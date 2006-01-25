/* 
 * Proview   $Id: rs_remote_udpip.c,v 1.2 2006-01-13 06:38:27 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*************************************************************************
*		===============
*                P r o v i e w
*               ===============
**************************************************************************
*
* Filename:             rs_remote_udpip.c
*
*                       Date    Pgm.    Read.   Remark
* Modified              000301  CJu
*			000330	CJu		v3.0
*			040504	CJu		v4.0.0
*
* Description:		Remote transport process UDP/IP
*			Implements transport protocol UDP/IP, connectionless
*			datagram protocol on the IP-stack. The major difference
*			between TCP and UDP is that TCP uses connected 
*			sockets while UDP sends datagrams to any unconnected
*			socket.
*
**************************************************************************
**************************************************************************/

/*_Include files_________________________________________________________*/
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "pwr_class.h"
#include "pwr_systemclasses.h"
#include "rt_gdh.h"
#include "co_cdh.h"
#include "rt_errh.h"
#include "pwr_baseclasses.h"
#include "pwr_remoteclasses.h"
#include "rt_pwr_msg.h"
#include "rt_aproc.h"
#include "remote.h"
#include "remote_remtrans_utils.h"

#define STX 2
#define ETB 15
#define ENQ 5
#define ACK 6
#define UDP_MAX_SIZE 32768
#define TIME_INCR 0.02

fd_set fds;				/* For select call */

typedef struct
{
  unsigned char protocol_id[2];
  short int msg_size;
  short int msg_id[2];
} remote_udp_header;

remnode_item rn;
pwr_sClass_RemnodeUDP *rn_udp;

float time_since_scan;
float time_since_rcv;
float time_since_keepalive;

int my_socket;				/* My socket */
struct sockaddr_in my_addr;		/* My named socket description */
struct sockaddr_in their_addr;		/* Remote socket description */
struct sockaddr_in dual_addr;		/* Maybe a dual socket description */

/*************************************************************************
**************************************************************************
*
* RemoteSleep
*
**************************************************************************
**************************************************************************/

void RemoteSleep(float time)
{
  struct timespec rqtp, rmtp;

  rqtp.tv_sec = 0;
  rqtp.tv_nsec = (long int) (time * 1000000000);
  nanosleep(&rqtp, &rmtp);
  return;
}

/*************************************************************************
**************************************************************************
*
* SendAck
*
**************************************************************************
**************************************************************************/

void SendAck(short int id0, short int id1)

{
  int status;
  remote_udp_header header;  


  header.protocol_id[0] = STX;
  header.protocol_id[1] = ACK;
  header.msg_size = htons(sizeof(remote_udp_header));
  header.msg_id[0] = htons(id0);
  header.msg_id[1] = htons(id1);

  status = sendto(my_socket, &header, sizeof(header), 0, 
		 (struct sockaddr *) &their_addr, sizeof(struct sockaddr));

  return;
}

/*************************************************************************
**************************************************************************
*
* Receive
*
**************************************************************************
**************************************************************************/

short int Receive()
{
  static char buf[UDP_MAX_SIZE];
  char unknown[24];
  unsigned char badr[24];
  char *pos;
  struct sockaddr_in from;
  unsigned int fromlen;
  int size;
  unsigned int sts;
  remote_udp_header header;  
  remtrans_item *remtrans;
  unsigned char search_remtrans;
  rem_t_transbuff *transbuff;
  rem_t_transbuff *transbuff_behind;
  remtrans_item *transp;

  /* We have made a select, so we're pretty sure there is something to take */

  fromlen = sizeof(struct sockaddr);

  size = recvfrom(my_socket, &buf, sizeof(buf), 0, 
			(struct sockaddr *) &from, &fromlen);

  if (size < 0) { 		/* Definitly error */
    errh_Info("UDP Receive fail %s", rn_udp->RemoteHostname);
    rn_udp->ErrCount++;
    return(-1);	
  }

  if (rn_udp->Disable) return(1);

  if (memcmp(&from.sin_addr, 
	     &their_addr.sin_addr, 
	     sizeof(struct in_addr)) != 0) {  /*from.sin_port != their_addr.sin_port*/
    memcpy(&badr, &from.sin_addr, 4);
    sprintf(unknown, "%d.%d.%d.%d", badr[0], badr[1], badr[2], badr[3]);
    errh_Info("UDP Receive from unknown source %s", unknown);
    rn_udp->ErrCount++;
    return(1);
  }

  /* Set link up */

  time_since_rcv = 0;
  if (rn_udp->LinkUp == 0) {
    errh_Info("UDP link up %s", rn_udp->RemoteHostname);
    rn_udp->LinkUp = 1;
  }

  if (size > 0 && rn_udp->DisableHeader) {
 
    /* Header disabled, take the first receive remtrans object */ 

    remtrans = rn.remtrans;
    search_remtrans = true;

    while(remtrans && search_remtrans) {
      /* Match? */
      if (remtrans->objp->Direction == REMTRANS_IN) {
        search_remtrans = false;
        sts = RemTrans_Receive(remtrans, buf, size); 
      }
      remtrans = (remtrans_item *) remtrans->next;
    }
    if (search_remtrans) {
      rn_udp->ErrCount++;
      errh_Info("UDP Receive no remtrans %s", rn_udp->RemoteHostname);
    }
  }

  else if (size >= 8) {
    memcpy(&header, &buf, sizeof(remote_udp_header));

    /* Convert the header to host byte order */
    header.msg_size = ntohs(header.msg_size);
    header.msg_id[0] = ntohs(header.msg_id[0]);
    header.msg_id[1] = ntohs(header.msg_id[1]);

    if (header.protocol_id[0] == STX && size == header.msg_size) {
      /* This is a valid remtrans */
      if (header.protocol_id[1] == ETB || header.protocol_id[1] == ENQ) {
        if (header.msg_id[0] == 0 && header.msg_id[1] == 0) {
	  /* Keepalive */
          rn_udp->KeepaliveDiff--;
        }
        else {
          /* Data */
          remtrans = (remtrans_item *) rn.remtrans;
          search_remtrans = true;
          while(remtrans && search_remtrans) {
            /* Match? */
            if (remtrans->objp->Address[0] == header.msg_id[0] && 
                remtrans->objp->Address[1] == header.msg_id[1] && 
                remtrans->objp->Direction == REMTRANS_IN) {
              search_remtrans = false;
              pos = ((char *) &buf) + sizeof(remote_udp_header);
              sts = RemTrans_Receive(remtrans, pos, 
			         header.msg_size-sizeof(remote_udp_header));
              if (header.protocol_id[1] == ENQ) {
                SendAck(header.msg_id[0], header.msg_id[1]);
              }
            }
            remtrans = (remtrans_item *) remtrans->next;
          }
          if (search_remtrans) {
            rn_udp->ErrCount++;
            errh_Info("UDP Receive no remtrans %s", rn_udp->RemoteHostname);
          }
        }
      }
      else if (header.protocol_id[1] == ACK) {
	/* Acknowledge message */ 
        transbuff = (rem_t_transbuff *) rn.transbuff;
        transbuff_behind = (rem_t_transbuff *) rn.transbuff;

        while (transbuff) {
	  transp = (remtrans_item *) transbuff->remtrans;
          if (header.msg_id[0] == transp->objp->Address[0] &&
              header.msg_id[1] == transp->objp->Address[1]) {
            /* This is it, unlink the transbuff */
            if (transbuff == transbuff_behind)
              rn.transbuff = (rem_t_transbuff *) transbuff->next;
            else
              transbuff_behind->next = (rem_t_transbuff *) transbuff->next;
            /* Decrement buffer counter */
            transp->objp->Buffers--;
            /* Free transbuff */
            free(transbuff);
            /* Break the while loop */
            break;
          }
          transbuff_behind = transbuff;
          transbuff = (rem_t_transbuff *) transbuff->next;
        }
      }
      else {
        /* Weird header */
        rn_udp->ErrCount++;
        errh_Info("UDP receive weird header %s, %02x %02x %04x %04x %04x", 
        rn_udp->RemoteHostname,
	header.protocol_id[0],
	header.protocol_id[1],
	header.msg_size,
	header.msg_id[0],
	header.msg_id[1]);
      }
    }
                       
    else {
      /* Weird header */
      rn_udp->ErrCount++;
        errh_Info("UDP receive weird header %s, %02x %02x %04x %04x %04x", 
        rn_udp->RemoteHostname,
	header.protocol_id[0],
	header.protocol_id[1],
	header.msg_size,
	header.msg_id[0],
	header.msg_id[1]);
    }
  }

  else {
    /* Not a remtrans UPD message */
    rn_udp->ErrCount++;
    errh_Info("UDP receive weird message %s", rn_udp->RemoteHostname);
  }

  return(1);

}

/*************************************************************************
**************************************************************************
*
* SendKeepalive
*
**************************************************************************
**************************************************************************/

int SendKeepalive(void)
{
  int sts;
  remote_udp_header header;  

  /* Fill in application header and convert to network byte order */

  header.protocol_id[0] = STX;
  header.protocol_id[1] = ETB;
  header.msg_size = htons(sizeof(header));
  header.msg_id[0] = 0;
  header.msg_id[1] = 0;

  sts = sendto(my_socket, &header, sizeof(header), 0, 
	       (struct sockaddr *) &their_addr, sizeof(struct sockaddr));

  if (sts >= 0) rn_udp->KeepaliveDiff++;

  return(sts);
}

/*************************************************************************
**************************************************************************
*
* CreateSocket
*
**************************************************************************
**************************************************************************/

void CreateSocket()
{
  int sts;
  unsigned char badr[4];
  unsigned int iadr[4] = {-1, -1, -1, -1};
  struct hostent *he;
  struct sockaddr_in address;
  socklen_t address_len = sizeof(struct sockaddr_in);

  /* Create a socket for UDP */

  my_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (my_socket < 0) { 
    errh_Error("Socket, %d", my_socket);
    errh_SetStatus(PWR__SRVTERM);
    exit(0);
  }

  if (rn_udp->LocalPort != 0) {

    /* Set local port */
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(rn_udp->LocalPort);

    /* Bind the created socket */
    sts = bind(my_socket, (struct sockaddr *) &my_addr, sizeof(my_addr));
    if (sts != 0) {
      errh_Error("Bind, %d", sts);
      errh_SetStatus(PWR__SRVTERM);
      exit(0);
    }
  }
  else {
    getsockname(my_socket, (struct sockaddr *) &address, &address_len);
    rn_udp->LocalPort = ntohs(address.sin_port);
  }   

/*
  problem, you have to be root user to do this!!
  sts = setsockopt(my_socket, SOL_SOCKET, SO_BINDTODEVICE, rn_udp->DevName, strlen(en_udp->DevName)); 
  printf("Setsockopt, %d\n", sts);
  if (sts != 0) errh_Error("Setsockopt bindtodevice, %d", sts);
*/
  
  /* Initialize remote address structure */

  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(rn_udp->RemotePort);
  sscanf((char *) &(rn_udp->RemoteAddress), "%d.%d.%d.%d", 
	&iadr[0], &iadr[1], &iadr[2], &iadr[3]);
	
  /* If none or invalid ip-address is given, use hostname to get hostent struct,
     otherwise use the given ip address directly */
  
  if ((iadr[0] < 0 || iadr[0] > 255) ||
      (iadr[0] < 0 || iadr[0] > 255) ||
      (iadr[0] < 0 || iadr[0] > 255) ||
      (iadr[0] < 0 || iadr[0] > 255)) {
    he = gethostbyname(rn_udp->RemoteHostname);
    if (he) {
      memcpy(&their_addr.sin_addr, he->h_addr, 4);
      sprintf(rn_udp->RemoteAddress, "%s", inet_ntoa(their_addr.sin_addr));
    }
    else {
      errh_Error("Unknown host, %s", rn_udp->RemoteHostname);
      errh_SetStatus(PWR__SRVTERM);
      exit(0);
    }
  }
  else {
    badr[0] = (unsigned char) iadr[0];
    badr[1] = (unsigned char) iadr[1];
    badr[2] = (unsigned char) iadr[2];
    badr[3] = (unsigned char) iadr[3];
    memcpy(&their_addr.sin_addr, &badr, 4);
  }

  /* If there is a multicast address configured, create a socket for a dual remote node */

  if (rn.multicast) {
    dual_addr.sin_family = AF_INET;
    dual_addr.sin_port = htons(rn_udp->RemotePort);
	
    badr[0] = (unsigned char) rn.multicast->Address[0];
    badr[1] = (unsigned char) rn.multicast->Address[1];
    badr[2] = (unsigned char) rn.multicast->Address[2];
    badr[3] = (unsigned char) rn.multicast->Address[3];
    memcpy(&dual_addr.sin_addr, &badr, 4);
  }

  rn_udp->LinkUp = 0;
  rn_udp->KeepaliveDiff = 0;

  time_since_scan = 0;
  time_since_rcv = 0;
  time_since_keepalive = 0;
}

/*************************************************************************
**************************************************************************
*
* RemnodeSend
*
**************************************************************************
**************************************************************************/

unsigned int RemnodeSend(remnode_item *remnode,
			 pwr_sClass_RemTrans *remtrans,
			 char *buf,
			 int buf_size)

{
  int status;
  int want_ack;

  static struct message_s {
    remote_udp_header header;  
    char data[UDP_MAX_SIZE];
  } message;

  message.header.protocol_id[0] = STX;

  /* Will we have ack? */
  
  want_ack = 0;

  if (remtrans->MaxBuffers > 0) {
    message.header.protocol_id[1] = ENQ;
    want_ack = 1;
  }
  else
    message.header.protocol_id[1] = ETB;

  message.header.msg_size = htons(buf_size+sizeof(remote_udp_header));
  message.header.msg_id[0] = htons(remtrans->Address[0]);
  message.header.msg_id[1] = htons(remtrans->Address[1]);

  memcpy(&message.data, buf, buf_size);

  if (rn_udp->DisableHeader)
    status = sendto(my_socket, &message.data, buf_size, 0,
	            (struct sockaddr *) &their_addr, sizeof(struct sockaddr));
  else
    status = sendto(my_socket, &message, buf_size + sizeof(remote_udp_header), 0,
	            (struct sockaddr *) &their_addr, sizeof(struct sockaddr));

  /* Send to dual node aswell if this is configured, we never want ack on this though */
  if (rn.multicast && remtrans->Address[3] & 1) {
    message.header.protocol_id[1] = ETB;
    if (rn_udp->DisableHeader)
      status = sendto(my_socket, &message.data, buf_size, 0,
	            (struct sockaddr *) &dual_addr, sizeof(struct sockaddr));
    else
      status = sendto(my_socket, &message, buf_size + sizeof(remote_udp_header), 0,
	            (struct sockaddr *) &dual_addr, sizeof(struct sockaddr));
  }

  if (want_ack)
    return (STATUS_BUFACK);
  else
    return (STATUS_OK);
}

/*************************************************************************
**************************************************************************
*
* Main
*
**************************************************************************
**************************************************************************/

int main(int argc, char *argv[])
{
  remtrans_item *remtrans;
  unsigned char id[32];
  unsigned char pname[32];

  pwr_tStatus sts;
  struct timeval tv;
  int i;
  
  /* Read arg number 2, should be id for this instance */

  if (argc >= 2)
    strcpy(id, argv[1]);
  else
    strcpy(id, "0");

  /* Build process name with id */

  sprintf((char *) pname, "rs_remudp_%s", id);

  /* Init of errh */

  errh_Init((char *) pname, errh_eAnix_remote);
  errh_SetStatus(PWR__SRVSTARTUP);
  
  /* Init of gdh */

  sts = gdh_Init((char *) pname);
  if ( EVEN(sts)) {
    errh_Error("gdh_Init, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }

  /* Arg number 3 should be my remnodes objid in string representation,
     read it, convert to real objid and store in remnode_item */

  sts = 0;
  if (argc >= 3) sts = cdh_StringToObjid(argv[2], &rn.objid);
  if ( EVEN(sts)) {
    errh_Error("cdh_StringToObjid, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }
  
  /* Get pointer to RemnodeUDP object and store locally */
  
  sts = gdh_ObjidToPointer(rn.objid, (pwr_tAddress *) &rn_udp);
  if ( EVEN(sts)) {
    errh_Error("cdh_ObjidToPointer, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }

  /* Initialize some internal data and make standard remtrans init */

  rn.next = NULL;
  rn.local = NULL;		// We dont use local structure since we only have one remnode
  rn.retransmit_time = rn_udp->RetransmitTime;
  rn_udp->ErrCount = 0;

  sts = RemTrans_Init(&rn);

  /* Log that we will multicast */
    
  if (rn.multicast) {
    errh_Info("Will send to dual address: %d.%d.%d.%d",
      rn.multicast->Address[0],
      rn.multicast->Address[1],
      rn.multicast->Address[2],
      rn.multicast->Address[3]);
  }

  if ( EVEN(sts)) {
    errh_Error("RemTrans_Init, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }
  
  /* Store remtrans objects objid in remnode_udp object */
  remtrans = rn.remtrans;
  i = 0;
  while(remtrans) {
    rn_udp->RemTransObjects[i++] = remtrans->objid;
    if ( i >= (int)(sizeof(rn_udp->RemTransObjects)/sizeof(rn_udp->RemTransObjects[0])))
      break;
    remtrans = (remtrans_item *) remtrans->next;
  }

  /* Initialize no timeout value (poll) for select call */

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  /* Create UDP socket and init adress structures */

  CreateSocket();

  /* Wait for one cycle */
  
  RemoteSleep(rn_udp->ScanTime);

  /* Set running status */
  
  errh_SetStatus(PWR__SRUN);
  
  /* Set (re)start time in remnode object */
  
  clock_gettime(CLOCK_REALTIME, &rn_udp->RestartTime);
  
  /* Loop forever */

  while (!doomsday)
  {
    /* Check disable flag */
    
    if (rn_udp->Disable == 1) {
      errh_Fatal("Disabled, exiting");
      errh_SetStatus(PWR__SRVTERM);
      exit(0);
    }   
    
    /* Timestamp */
    
    aproc_TimeStamp();

    RemoteSleep(TIME_INCR);

    /* Increase time counters in local remnode and prevent big counters */
    
    time_since_scan += TIME_INCR;
    time_since_keepalive += TIME_INCR;
    time_since_rcv += TIME_INCR;
    time_since_scan = min(time_since_scan, rn_udp->ScanTime + 1.0);
    time_since_keepalive = min(time_since_keepalive, rn_udp->KeepaliveTime + 1.0);
    time_since_rcv = min(time_since_rcv, rn_udp->LinkTimeout + 1.0);

    /* Update retransmit time, could have been changed */
    
    rn.retransmit_time = rn_udp->RetransmitTime;

    remtrans = rn.remtrans;
    while(remtrans) {
      remtrans->time_since_send += TIME_INCR;
      /* Prevent big counter */
      remtrans->time_since_send = min(remtrans->time_since_send, rn.retransmit_time + 1.0);
      remtrans = (remtrans_item *) remtrans->next;
    }

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(my_socket, &fds);
    sts = select(32, &fds, NULL, NULL, &tv);

    if (sts > 0) Receive();

    if (sts < 0) {
      errh_Error("Select, %d", sts);
      errh_SetStatus(PWR__SRVTERM);
      exit(0);
    }
    
    if (time_since_scan >= rn_udp->ScanTime) {
      if (!rn_udp->Disable) RemTrans_Cyclic(&rn, &RemnodeSend);
      time_since_scan = 0;
    }

    if (time_since_keepalive >= rn_udp->KeepaliveTime) {
      if (!rn_udp->Disable && rn_udp->UseKeepalive) SendKeepalive();
      time_since_keepalive = 0;
    }

    if (time_since_rcv >= rn_udp->LinkTimeout && rn_udp->LinkTimeout > 0) {
      if (rn_udp->LinkUp) {
        errh_Info("UDP link down %s", rn_udp->RemoteHostname);
        rn_udp->LinkUp = 0;
      }
    }
  }
}