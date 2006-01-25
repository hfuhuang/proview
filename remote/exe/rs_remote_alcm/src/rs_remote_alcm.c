/* 
 * Proview   $Id: rs_remote_alcm.c,v 1.1 2006-01-12 06:39:33 claes Exp $
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
* 			===============
* 			P r o v i e w
* 			===============
**************************************************************************
*
* Filename:             rs_remote_alcm.c
*
*                       Date    Pgm.    Read.   Remark
* Modified              010102  CJ		F�rsta version f�r Lynx/linux 
*			030227	CJ		Remote I/O inf�rt
*			040504	CJ		v4.0.0
*
* Description:		Remote transport ALCM f�r plattformarna Lynx och Linux.
*			ALCM �r ett l�gniv� ethernetprotokoll. P� Linux anv�nds 
*			en "raw ethernet socket" f�r att hantera ethernetpaket
*			med protokoll-id 60-06 som �r DEC's gamla "owner protocol"
*
**************************************************************************
**************************************************************************/

#include "rt_gdh.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <fcntl.h>
#include <netinet/in.h>

#include "rt_errh.h"
#include "rt_pwr_msg.h"
#include "rt_aproc.h"
#include "pwr_class.h"
#include "pwr_systemclasses.h"
#include "pwr_baseclasses.h"
#include "pwr_remoteclasses.h"
#include "remote.h"
#include "remote_utils.h"
#include "remote_remtrans_utils.h"
#include "remote_remio_utils.h"
#include "rs_remote_alcm.h"

#define TIME_INCR 0.02

typedef struct {
  pwr_sClass_RemnodeALCM *ref;
  unsigned char address[6];
  float time_since_scan;
  float time_since_rcv;
  float time_since_poll;
  float time_since_io;
} remnode_alcm;

remnode_item *rnl = NULL;		// Lista med remnoder

int my_socket;				// Egen socket
unsigned char my_mac_address[6];	// Egen nods MAC-adress

typedef struct {
  unsigned char dst[6];
  unsigned char src[6];
  unsigned short type;
  unsigned short size;
} eth_header;

typedef struct {
  unsigned char type;
  unsigned char seqnum;
  unsigned char filler[46];
} alcm_header;

/*************************************************************************
**************************************************************************
*
* Namn 		: RemoteSleep
*
* Beskrivning 	: V�ntar angiven tid (sekunder)
*
**************************************************************************
**************************************************************************/

void RemoteSleep(float seconds)
{
  struct timespec rqtp, rmtp;

  rqtp.tv_sec = (int) seconds;
  rqtp.tv_nsec = (seconds - (float) rqtp.tv_sec) * 1000000000;
  nanosleep(&rqtp, &rmtp);
  return;
}

/*************************************************************************
**************************************************************************
*
* Namn 		: CalcAddress
*
* Beskrivning 	: Ber�knar ethernetadress fr�n DECnet-area och -nod
*
**************************************************************************
**************************************************************************/

int CalcAddress(unsigned char *str, unsigned char *address)
{
  int i1, i2, i3, i4, i5, i6;
  unsigned short int node, area;

  if (strchr(str, '.')) {  
    sscanf(str, "%d.%d", &i1, &i2);
    area = (unsigned short int) i1;
    node = (unsigned short int) i2;

    if (area > 64 || node > 1024) return(-1);
    
    address[0] = 0xAA;
    address[1] = 0x00;
    address[2] = 0x04;
    address[3] = 0x00;

    node = node | (area << 10);
      
    address[4] = (unsigned char) (node & 255);
    address[5] = (unsigned char) (node >> 8);
    
    return 1;
  }
  else if (strchr(str, ':')) {
    sscanf(str, "%x:%x:%x:%x:%x:%x", &i1, &i2, &i3, &i4, &i5, &i6);
    address[0] = (unsigned char) i1;
    address[1] = (unsigned char) i2;
    address[2] = (unsigned char) i3;
    address[3] = (unsigned char) i4;
    address[4] = (unsigned char) i5;
    address[5] = (unsigned char) i6;
    return 1;
  }
  
  else {
    return -1;
  }

}

/*************************************************************************
**************************************************************************
*
* Namn 		: InitNet
*
* Beskrivning 	: Skapar raw socket och g�r bind till interfacet
*
**************************************************************************
**************************************************************************/

void InitNet()
{
  struct ifreq ifr;
  struct sockaddr_ll my_addr;

  // Skapa en raw ethernet socket

  my_socket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUST));

  if (my_socket < 0) {
    errh_Error("Error in socket()");
    exit(0);
  }

  // S�tt noblock
  
  fcntl(my_socket, F_SETFL, O_NONBLOCK);

  // L�s egen nods MAC-adress fr�n interfacet (eth0)

  strcpy(ifr.ifr_name, "eth0");

  ioctl(my_socket, SIOCGIFHWADDR, &ifr);
  memcpy(&my_mac_address, &ifr.ifr_hwaddr.sa_data, 6);

  // H�mta interface-index g�r bind

  ioctl(my_socket, SIOCGIFINDEX, &ifr);

  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sll_family = AF_PACKET;
  my_addr.sll_protocol = htons(ETH_P_CUST);
  my_addr.sll_ifindex = ifr.ifr_ifindex;

  bind(my_socket, (struct sockaddr *) &my_addr, sizeof(my_addr));

  return;
}

/*************************************************************************
**************************************************************************
*
* Namn 		: SendAck
*
* Beskrivning 	: Skickar ack-meddelande till remote-nod
*
**************************************************************************
**************************************************************************/

unsigned int SendAck(remnode_item *remnode, unsigned char seqnum)
{
  
  struct {
    eth_header eh;
    alcm_header ah;
  } snd;

  remnode_alcm *local = (remnode_alcm *) remnode->local;

  memcpy(&snd.eh.dst, &local->address, sizeof(snd.eh.dst));
  memcpy(&snd.eh.src, &my_mac_address, sizeof(snd.eh.src));
  snd.eh.type = htons(ETH_P_CUST);
  snd.eh.size = sizeof(alcm_header);

  snd.ah.type = ALCM_MTYP_RESP;
  snd.ah.seqnum = seqnum;

  write(my_socket, &snd, sizeof(snd));

  return(1);
}

/*************************************************************************
**************************************************************************
*
* Namn 		: SendPoll
*
* Beskrivning 	: Skickar poll eller IO-uppdatering till remote-noderna
*
**************************************************************************
**************************************************************************/

static void SendPoll(remnode_item *remnode, pssupd_buffer *buf)
{
  char common_name[] = "   PSS";

  struct {
    eth_header eh;
    alcm_header ah;
    apl_buffer apl;
  } snd;
  
  remnode_alcm *local = (remnode_alcm *) remnode->local;

  memcpy(&snd.eh.dst, &local->address, sizeof(snd.eh.dst));
  memcpy(&snd.eh.src, &my_mac_address, sizeof(snd.eh.src));
  snd.eh.type = htons(ETH_P_CUST);
  snd.eh.size = sizeof(alcm_header) + buf->length;

  snd.ah.type = ALCM_MTYP_DATA;
  snd.ah.seqnum = 0;
  
  RemUtils_AsciiToR50("PSSUPD", buf->receive_task);
  memcpy(&common_name, &local->ref->RemoteHostname, 3);
  RemUtils_AsciiToR50(common_name, buf->common_name);
  buf->transcode = ALCM_TRACO_APLNAK;

  memcpy(&snd.apl, buf, buf->length);
  
  if (write(my_socket, &snd, sizeof(eth_header)+sizeof(alcm_header)+buf->length) < 0) {
    errh_Error("Send failure");
      local->ref->ErrCount++;
  }

  return;
}

/*************************************************************************
**************************************************************************
*
* Namn 		: SendAppl
*
* Beskrivning 	: Skickar applikationstransen som beskrivs av remtransen
*
**************************************************************************
**************************************************************************/

unsigned int SendAppl(remnode_item *remnode,
		      pwr_sClass_RemTrans *remtrans,
		      char *buf,
		      int buf_size)

{
  static unsigned char message_counter = 0;

  struct {
    eth_header eh;
    alcm_header ah;
    apl_buffer apl;
  } snd;
  
  csp_buffer *c_buf;
  
  remnode_alcm *local = (remnode_alcm *) remnode->local;

  if (message_counter++ >= 255) message_counter = 1;

  memcpy(&snd.eh.dst, &local->address, sizeof(snd.eh.dst));
  memcpy(&snd.eh.src, &my_mac_address, sizeof(snd.eh.src));
  snd.eh.type = htons(ETH_P_CUST);
  snd.eh.size = sizeof(alcm_header) + buf_size;

  snd.ah.type = ALCM_MTYP_DATA;
  snd.ah.seqnum = message_counter;

  memcpy(&snd.apl, buf, buf_size);
  
  c_buf = (csp_buffer *) &snd.apl;

  if (snd.apl.trans_code == ALCM_TRACO_APLACK ||
      snd.apl.trans_code == ALCM_TRACO_APLNAK ||
      snd.apl.trans_code == ALCM_TRACO_CSPNAK ||
      snd.apl.trans_code == ALCM_TRACO_COFNAK) 
  {
    if (snd.apl.trans_code == ALCM_TRACO_APLACK ||
        snd.apl.trans_code == ALCM_TRACO_APLNAK) {
      RemUtils_AsciiToR50(remtrans->TransName, snd.apl.receive_task);
      // L�gg tillbaka headern till buf s� folk ser vad vi pillat p�!
      memcpy(buf, &snd.apl, 10);
    }
    else {
      RemUtils_AsciiToR50(remtrans->TransName, c_buf->common_name);
    }
    
    if (write(my_socket, &snd, sizeof(eth_header)+sizeof(alcm_header)+buf_size) < 0)
    {
      errh_Error("Send failure");
      return(-1);
    }

    if (snd.apl.trans_code == ALCM_TRACO_APLACK)
      return(STATUS_BUFACK);
    else
      return(1);
  }
  else
    return(-1);
}

/*************************************************************************
**************************************************************************
*
* Namn 		: Receive
*
* Beskrivning 	: Tar emot ethernetpaket fr�n interfacet
*
**************************************************************************
**************************************************************************/

unsigned short int Receive()
{
  remnode_item *remnode;
  unsigned char search_remnode = TRUE;
  remtrans_item *remtrans;
  unsigned char search_remtrans = TRUE;
  unsigned char send_response = FALSE;
  int msg_size;
  unsigned int sts;
  char name[7];
  unsigned int apl_size;
  unsigned char reject;
  int loop_count;
  remtrans_item *transp;

  struct {
    eth_header eh;
    alcm_header ah;
    apl_buffer apl;
  } rcv;
  
  csp_buffer *csp;		// F�r speglad commonarea
  int common_offset, common_length;

  reject = true;
  loop_count = 0;
  
  remnode_alcm *local;
  
  // Avvisa multicast- och broadcastpaket

  while (reject) {
    msg_size = read(my_socket, &rcv, sizeof(rcv));
    if (msg_size < 0) {
      return(1);
    }
    if (rcv.eh.dst[0] & 1) {
      reject = true;
    }
    else {
      reject = false;
    }
    if (loop_count++ > 100) return(1);
  }    

  remnode = rnl;
  search_remnode = TRUE;

  while (remnode && search_remnode)
  {
    local = (remnode_alcm *) remnode->local;
    if (memcmp(rcv.eh.src, local->address, 6) == 0)
      search_remnode = FALSE;
    else
      remnode = (remnode_item *) remnode->next;
  }

  if (search_remnode)
  {
    if (rcv.eh.dst[0] != 0xAB)
      errh_Info("Message from unknown source %02x-%02x-%02x-%02x-%02x-%02x",
        rcv.eh.src[0], rcv.eh.src[1], rcv.eh.src[2],
        rcv.eh.src[3], rcv.eh.src[4], rcv.eh.src[5]);
  }

  else

  {
        
    if (rcv.ah.type == ALCM_MTYP_DATA && !local->ref->Disable)
    {
    
      apl_size = msg_size-sizeof(eth_header)-sizeof(alcm_header);

      // Applikationstrans

      if (rcv.apl.trans_code == ALCM_TRACO_APLACK ||
   	  rcv.apl.trans_code == ALCM_TRACO_APLNAK)
      {
 	RemUtils_R50ToAscii( (unsigned short *) &rcv.apl.receive_task, name);
	name[6] = '\0';
	
 	search_remtrans = TRUE;

        remtrans = remnode->remtrans;
	while(remtrans && search_remtrans)
	{
	  if ((strncmp(name, remtrans->objp->TransName, 6) == 0) &&
		remtrans->objp->Direction == REMTRANS_IN)
	  {
	    search_remtrans = FALSE;
	    sts = RemTrans_Receive(remtrans, (char *) &rcv.apl, apl_size);
	    if ((rcv.apl.trans_code == ALCM_TRACO_APLACK) && ODD(sts))
	      send_response = TRUE;
	    break;
	  }
	  remtrans = (remtrans_item *) remtrans->next;
	}
	if (search_remtrans) {
	  local->ref->ErrCount++;
          errh_Info("Unknown message %s from %s", name, 
	  	((remnode_alcm *) (remnode->local))->ref->RemoteHostname);	  
	}
	else {
	  // Skicka alltid kvittens p� DUMMY0 (om APLACK)	
	  if (strncmp(name, "DUMMY0", 6) == 0 && rcv.apl.trans_code == ALCM_TRACO_APLACK) 
	    send_response = TRUE;
	}
      }

      // Spegling av common-area. Behandlas i princip som en remtrans
      // Tyv�rr lite special eftersom det finns m�jlighet att spegla med offset

      else if (rcv.apl.trans_code == ALCM_TRACO_CSPACK ||
	       rcv.apl.trans_code == ALCM_TRACO_CSPNAK ||
	       rcv.apl.trans_code == ALCM_TRACO_COFACK ||
	       rcv.apl.trans_code == ALCM_TRACO_COFNAK) {
	       
	csp = (csp_buffer *) &rcv.apl;
	
 	RemUtils_R50ToAscii((unsigned short *)csp->common_name, name);
	name[6] = '\0';

	if (csp->trans_code == ALCM_TRACO_COFACK ||
	    csp->trans_code == ALCM_TRACO_COFNAK)
	  common_offset = csp->offset;
	else
	  common_offset = 0;

 	search_remtrans = true;

        remtrans = remnode->remtrans;
	while(remtrans && search_remtrans)
	{
	  if ((strncmp(name, remtrans->objp->TransName, 6) == 0) &&
		remtrans->objp->Direction == REMTRANS_IN)
	  {
	    search_remtrans = false;
	    common_length = msg_size-sizeof(eth_header)-sizeof(alcm_header)+common_offset;
	    if (common_length > remtrans->objp->MaxLength)
	    {
	      remtrans->objp->ErrCount++;
	      remtrans->objp->LastSts = STATUS_LENGTH;
	    }
	    else
	    {
	      memcpy(&remtrans->datap[common_offset], csp, 
	      	msg_size-sizeof(eth_header)-sizeof(alcm_header));
	      clock_gettime( CLOCK_REALTIME, &remtrans->objp->TransTime);
	      remtrans->objp->TransCount++;
	      remtrans->objp->DataValid = TRUE;
	      remtrans->objp->LastSts = STATUS_OK;
	      if (common_length > remtrans->objp->DataLength) 
		remtrans->objp->DataLength = common_length;
	      if (csp->trans_code == ALCM_TRACO_CSPACK ||
		  csp->trans_code == ALCM_TRACO_COFACK) send_response = TRUE;
	    }
	  }
	  remtrans = (remtrans_item *) remtrans->next;
	}
	if (search_remtrans) {
	  local->ref->ErrCount++; 
          errh_Info("Unknown message %s from %s", name, 
	  	((remnode_alcm *) (remnode->local))->ref->RemoteHostname);	  
	}       
      }

      // I/O-spegling

      else if (rcv.apl.trans_code == ALCM_TRACO_BSPACK || rcv.apl.trans_code == ALCM_TRACO_BSPNAK) {
      
        sts = RemIO_Receive_ALCM(remnode, (bsp_buffer *) &rcv.apl, apl_size);
	local->time_since_io = 0;
      
        if (rcv.apl.trans_code == ALCM_TRACO_BSPACK) send_response = TRUE;
      }

    }


    else if (rcv.ah.type == ALCM_MTYP_RESP)
    {
      // Applikationskvittens

      rem_t_transbuff *transbuff;

      transbuff = remnode->transbuff;

      if (transbuff)
      {
      	transp = (remtrans_item *) transbuff->remtrans;
	remnode->transbuff = (rem_t_transbuff *) transbuff->next;
	transp->objp->Buffers--;
	free(transbuff);
      }
      send_response = FALSE;		// Skicka ingen kvittens
    }

    else if (rcv.ah.type == ALCM_MTYP_INIT && !local->ref->Disable)
    {
      send_response = TRUE;		// Svara alltid init-meddelande med kvittens
    }

    if (send_response) {
      sts = SendAck(remnode, rcv.ah.seqnum);
    }
        
	
    if (local->ref->LinkUp != 1) {
      local->ref->LinkUp = 1;
      errh_Error("ALCM link up to node %s", local->ref->RemoteHostname, 0);
    }
    
    local->time_since_rcv = 0.0;

  }

  return(1);
}

/*************************************************************************
**************************************************************************
*
* Main
*
**************************************************************************
**************************************************************************/

int main(int argc, char *argv[]) {

  remnode_item *rn = NULL;
  remnode_alcm *rn_local = NULL;
  remtrans_item *remtrans;
  pwr_tObjid objid;				// F�r loop
  unsigned int sts;                             // Status fr�n funktionsanrop
  unsigned char pname[32];
  pwr_tTime tmptime;
  int i;

  // Build process name
  
  sprintf(pname, "rs_remalcm");

  // Init of errh

  errh_Init(pname, errh_eAnix_remote);
  errh_SetStatus(PWR__SRVSTARTUP);

  // Init of gdh

  sts = gdh_Init(pname);
  if ( EVEN(sts)) {
    errh_Error("gdh_Init, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }
  
  // Leta reda p� samtliga RemnodeALCM objekt, skapa l�nkad lista med lokala 

  sts = gdh_GetClassList(pwr_cClass_RemnodeALCM, &objid);

  if ( EVEN(sts)) {
    errh_Error("gdh_GetClassList, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }

  // Loopa �ver alla remnoder och skapa en intern l�nkad lista

  while (ODD(sts))
  {
    // Allokera lokala datastrukturer
    rn = malloc(sizeof(remnode_item));
    rn_local = malloc(sizeof(remnode_alcm));
    rn->local = rn_local;
    if (!rn || !rn_local) {
      errh_Error("Malloc");
      errh_SetStatus(PWR__SRVTERM);
      exit(0);
    }

    // L�nka in
    rn->next = rnl;
    rnl = rn;

    // Initiera data
    sts = gdh_ObjidToPointer(objid, (pwr_tAddress *) &rn_local->ref);
    rn->objid = objid;
    rn->retransmit_time = rn_local->ref->RetransmitTime;
    rn_local->time_since_scan = 0;
    rn_local->time_since_rcv = 0;
    rn_local->time_since_poll   = 0;
    rn_local->time_since_io   = 0;

    sts = RemTrans_Init(rn);
    if ( EVEN(sts)) {
      errh_Error("RemTrans_Init, %d", sts);
      errh_SetStatus(PWR__SRVTERM);
      exit(sts);
    }

    sts = RemIO_Init_ALCM(rn);
    if ( EVEN(sts)) {
      errh_Error("RemIO_Init, %d", sts);
      errh_SetStatus(PWR__SRVTERM);
      exit(sts);
    }
      
    // Ber�kna ethernet-adress fr�n adress-str�ng i objektet

    sts = CalcAddress(rn_local->ref->RemoteAddress, rn_local->address);

    sts = gdh_GetNextObject(objid, &objid);
  }

  // Initialize socket

  InitNet();

  // Set running status
  
  errh_SetStatus(PWR__SRUN);

  // Set (re)start time in all remnode objects
  
  clock_gettime(CLOCK_REALTIME, &tmptime);
  rn = rnl;
  while (rn) {
    rn_local = (remnode_alcm *) rn->local;
    rn_local->ref->RestartTime = tmptime;
    rn = rn->next;
  }
  
  /* Store remtrans objects objid in every remnode_alcm object */

  rn = rnl;
  i = 0;
  while (rn) {
    rn_local = (remnode_alcm *) rn->local;
    remtrans = rn->remtrans;
    while(remtrans) {
      rn_local->ref->RemTransObjects[i++] = remtrans->objid;
      if (i >= (int)(sizeof(rn_local->ref->RemTransObjects)/sizeof(rn_local->ref->RemTransObjects[0])))
        break;
      remtrans = (remtrans_item *) remtrans->next;
    }
    rn = rn->next;
  }

  // Loopa nu f�r evigt och �vervaka remnoderna

  while (!doomsday)
  {
  
    /* Timestamp */
    
    aproc_TimeStamp();
    
    RemoteSleep(TIME_INCR);

    sts = Receive();

    rn = rnl;
    
    while (rn) {
    
      rn_local = (remnode_alcm *) rn->local;

      /* Increase time counters in local remnode and prevent big counters */

      rn_local->time_since_scan += TIME_INCR;
      rn_local->time_since_rcv += TIME_INCR;
      rn_local->time_since_poll += TIME_INCR;
      rn_local->time_since_io += TIME_INCR;
      rn_local->time_since_scan = min(rn_local->time_since_scan, rn_local->ref->ScanTime + 1.0);
      rn_local->time_since_rcv = min(rn_local->time_since_rcv, rn_local->ref->LinkTimeout + 1.0);
      rn_local->time_since_poll = min(rn_local->time_since_poll, rn_local->ref->IOPollTimeSlow + 1.0);
      rn_local->time_since_io = min(rn_local->time_since_io, rn_local->ref->IOStallTime + 1.0);

      /* Increase send timer for every remtrans */
      remtrans = rn->remtrans;
      while(remtrans) {
        remtrans->time_since_send += TIME_INCR;
        /* Prevent big counter */
        remtrans->time_since_send = min(remtrans->time_since_send, rn->retransmit_time + 1.0);
        remtrans = (remtrans_item *) remtrans->next;
      }

      // Ber�kna ethernet-adress fr�n objektet
      CalcAddress(rn_local->ref->RemoteAddress, rn_local->address);

      // Update retransmit time, could have been changed
      rn->retransmit_time = rn_local->ref->RetransmitTime;
      
      if (rn_local->ref->IOPoll && !rn_local->ref->Disable) {

        // Dags att polla remote I/O ?
	
	if (!rn_local->ref->Disable) {
          if (((!rn_local->ref->IOStallFlag || EVEN(rn_local->ref->IOStallAction)) &&
      	  	(rn_local->time_since_poll >= rn_local->ref->IOPollTime)) ||
	      (rn_local->ref->IOStallFlag && ODD(rn_local->ref->IOStallAction) &&
		(rn_local->time_since_poll >= rn_local->ref->IOPollTimeSlow) &&
		(rn_local->time_since_poll >= rn_local->ref->IOPollTime))) {
	    sts = RemIO_Cyclic_ALCM(rn, &SendPoll);
	    rn_local->time_since_poll = 0;
          }
	}

        // Dags f�r stall ?

        if ((rn_local->time_since_io >= rn_local->ref->IOStallTime) && (rn_local->ref->IOStallTime > 0)) {
	  sts = RemIO_Stall_ALCM(rn);
        }
      
      }
      else {
        rn_local->ref->IOStallFlag = 0;
	rn_local->time_since_poll = 0;
	rn_local->time_since_io = 0;
      }

      // Dags att scanna remtransar ?

      if (rn_local->time_since_scan >= rn_local->ref->ScanTime) {
	if (!rn_local->ref->Disable) sts = RemTrans_Cyclic(rn, &SendAppl);
	rn_local->time_since_scan = 0;
      }
      
      // Link up ?

      if (rn_local->time_since_rcv >= rn_local->ref->LinkTimeout && rn_local->ref->LinkTimeout > 0) {
        if (rn_local->ref->LinkUp != 0) {
	  rn_local->ref->LinkUp = 0;
	  errh_Error("ALCM link down to node %s", rn_local->ref->RemoteHostname, 0);
	}
      }

      rn = rn->next;
    }
  }
}
