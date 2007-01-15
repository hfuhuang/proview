/* 
 * Proview   $Id: remote_pvd_udp.c,v 1.1 2006-09-14 14:17:44 claes Exp $
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
 **/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "rs_remote_msg.h"
#include "remote_pvd_udp.h"

#define STX 2
#define ETB 15
#define ENQ 5
#define ACK 6
#define UDP_MAX_SIZE 32768
#define TIME_INCR 0.02

#define errh_Error printf
#define errh_Info printf

fd_set fds;				/* For select call */

typedef struct
{
  unsigned char protocol_id[2];
  unsigned short int msg_size;
  unsigned short int msg_id[2];
} udp_header;

int my_socket;				/* My socket */
struct sockaddr_in my_addr;		/* My named socket description */
struct sockaddr_in their_addr;		/* Remote socket description */
struct sockaddr_in dual_addr;		/* Maybe a dual socket description */

typedef struct {
  int LocalPort;
  int RemotePort;
  char RemoteHostName[32];
  char RemoteAddress[32];
  int LinkUp;
  int KeepaliveDiff;  
  int MaxBuffers;
  int DisableHeader;
  int Receive;
  int ErrCount;
  int Disable;
} *udp_tCtx;

static udp_tCtx udp_ctx = 0;

#if 0
static void RemoteSleep(float time)
{
  struct timespec rqtp, rmtp;

  rqtp.tv_sec = 0;
  rqtp.tv_nsec = (long int) (time * 1000000000);
  nanosleep(&rqtp, &rmtp);
  return;
}
#endif

void udp_Disable()
{
  udp_ctx->Disable = 1;
}

void udp_Enable()
{
  udp_ctx->Disable = 0;
}

static int CreateSocket( udp_tCtx ctx)
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
    return 0;
  }

  if (ctx->LocalPort != 0) {

    /* Set local port */
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(ctx->LocalPort);

    /* Bind the created socket */
    sts = bind(my_socket, (struct sockaddr *) &my_addr, sizeof(my_addr));
    if (sts != 0) {
      errh_Error("Bind, %d", sts);
      return 0;
    }
  }
  else {
    getsockname(my_socket, (struct sockaddr *) &address, &address_len);
    ctx->LocalPort = ntohs(address.sin_port);
  }   

  /* Initialize remote address structure */

  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(ctx->RemotePort);
  sscanf((char *) &(ctx->RemoteAddress), "%d.%d.%d.%d", 
	&iadr[0], &iadr[1], &iadr[2], &iadr[3]);
	
  /* If none or invalid ip-address is given, use hostname to get hostent struct,
     otherwise use the given ip address directly */
  
  if ((iadr[0] < 0 || iadr[0] > 255) ||
      (iadr[0] < 0 || iadr[0] > 255) ||
      (iadr[0] < 0 || iadr[0] > 255) ||
      (iadr[0] < 0 || iadr[0] > 255)) {
    he = gethostbyname(ctx->RemoteHostName);
    if (he) {
      memcpy(&their_addr.sin_addr, he->h_addr, 4);
      sprintf(ctx->RemoteAddress, "%s", inet_ntoa(their_addr.sin_addr));
    }
    else {
      errh_Error("Unknown host, %s", ctx->RemoteHostName);
      return 0;
    }
  }
  else {
    badr[0] = (unsigned char) iadr[0];
    badr[1] = (unsigned char) iadr[1];
    badr[2] = (unsigned char) iadr[2];
    badr[3] = (unsigned char) iadr[3];
    memcpy(&their_addr.sin_addr, &badr, 4);
  }

  ctx->LinkUp = 0;
  ctx->KeepaliveDiff = 0;

  return 1;
}

pwr_tStatus udp_Request( char *sendbuf, int sendbuf_size, 
			 char **rcvbuf)
{
  int i;
  pwr_tStatus sts;
  int tmo = 100;

  if (udp_ctx->Disable) return REM__DISABLED;

  for ( i = 0; i < 5; i++) {
    sts = udp_Send( sendbuf, sendbuf_size);
    if ( EVEN(sts)) return sts;

    sts = udp_Receive( rcvbuf, tmo);
    if ( sts != REM__TIMEOUT)
      return sts;

    tmo = tmo * 2;
  }
  if ( udp_ctx->LinkUp) {
    printf( "UDP link Down to node %s\n", udp_ctx->RemoteHostName);
    udp_ctx->LinkUp = 0;
  }
  return sts;
}

pwr_tStatus udp_Send( char *buf, int buf_size)
{
  int status;
  udp_tCtx ctx = udp_ctx;
  static struct message_s {
    udp_header header;  
    char data[UDP_MAX_SIZE];
  } message;

  if (ctx->Disable) return REM__DISABLED;

  message.header.protocol_id[0] = STX;

  message.header.protocol_id[1] = ETB;

  message.header.msg_size = htons(buf_size+sizeof(udp_header));

  memcpy(&message.data, buf, buf_size);

  if ( ctx->DisableHeader)
    status = sendto(my_socket, &message.data, buf_size, 0,
	            (struct sockaddr *) &their_addr, sizeof(struct sockaddr));
  else
    status = sendto(my_socket, &message, buf_size + sizeof(udp_header), 0,
	            (struct sockaddr *) &their_addr, sizeof(struct sockaddr));

  return REM__SUCCESS;
}

pwr_tStatus udp_Receive( char **buff, int tmo)
{
  static char buf[UDP_MAX_SIZE];
  char unknown[24];
  unsigned char badr[24];
  char *datapos;
  int datasize;
  struct sockaddr_in from;
  unsigned int fromlen;
  int size;
  unsigned int sts;
  udp_header header;  
  udp_tCtx ctx = udp_ctx;
  struct timeval tv;

  if (ctx->Disable) return REM__DISABLED;

  tv.tv_sec = tmo / 1000;
  tv.tv_usec = (tmo - tv.tv_sec * 1000) * 1000;

  FD_ZERO(&fds);
  FD_SET(my_socket, &fds);
  sts = select(32, &fds, NULL, NULL, &tv);
  switch ( sts) {
  case 0:
    /* Timeout */
    return REM__TIMEOUT;
  case -1:
    errh_Error("Select, %d", sts);
    exit(0);
  default: ;
  }

  /* We have made a select, so we're pretty sure there is something to take */

  fromlen = sizeof(struct sockaddr);

  size = recvfrom(my_socket, &buf, sizeof(buf), 0, 
			(struct sockaddr *) &from, &fromlen);

  if (size < 0) { 		/* Definitly error */
    errh_Info("UDP Receive fail %s",ctx->RemoteHostName);
    ctx->ErrCount++;
    return(-1);
  }


  if (memcmp(&from.sin_addr, 
	     &their_addr.sin_addr, 
	     sizeof(struct in_addr)) != 0) {  /*from.sin_port != their_addr.sin_port*/
    memcpy(&badr, &from.sin_addr, 4);
    sprintf(unknown, "%d.%d.%d.%d", badr[0], badr[1], badr[2], badr[3]);
    errh_Info("UDP Receive from unknown source %s", unknown);
    ctx->ErrCount++;
    return(1);
  }

  /* Set link up */

  if ( ctx->LinkUp == 0) {
    errh_Info("UDP link up %s", ctx->RemoteHostName);
    ctx->LinkUp = 1;
  }

  if (size > 0 && ctx->DisableHeader) { 
    datapos = buf;
    datasize = sizeof(buf);
  }
  else if (size >= 8) {
    memcpy(&header, &buf, sizeof(udp_header));

    /* Convert the header to host byte order */
    header.msg_size = ntohs(header.msg_size);
    header.msg_id[0] = ntohs(header.msg_id[0]);
    header.msg_id[1] = ntohs(header.msg_id[1]);

    if (header.protocol_id[0] == STX && size == header.msg_size) {
      if (header.protocol_id[1] == ETB || header.protocol_id[1] == ENQ) {
        if (header.msg_id[0] == 0 && header.msg_id[1] == 0) {
	  /* Keepalive */
          ctx->KeepaliveDiff--;
        }
        else {
          /* Data */
	  datapos = ((char *) &buf) + sizeof(udp_header);
	  datasize = header.msg_size-sizeof(udp_header);
	  printf( "%s", datapos);
	}
      }
      else {
        /* Weird header */
        ctx->ErrCount++;
        errh_Info("UDP receive weird header %s, %02x %02x %04x %04x %04x", 
        ctx->RemoteHostName,
	header.protocol_id[0],
	header.protocol_id[1],
	header.msg_size,
	header.msg_id[0],
	header.msg_id[1]);
      }
    }
                       
    else {
      /* Weird header */
      ctx->ErrCount++;
        errh_Info("UDP receive weird header %s, %02x %02x %04x %04x %04x", 
        ctx->RemoteHostName,
	header.protocol_id[0],
	header.protocol_id[1],
	header.msg_size,
	header.msg_id[0],
	header.msg_id[1]);
    }
  }

  else {
    /* Not a remtrans UPD message */
    ctx->ErrCount++;
    errh_Info("UDP receive weird message %s", ctx->RemoteHostName);
  }
  *buff = datapos;

  return 1;
}

pwr_tStatus udp_Init( char *remote_address, char *remote_host_name, int port)
{
  pwr_tStatus sts;
  udp_tCtx ctx = calloc( 1, sizeof(*ctx));
  ctx->LocalPort = port;
  ctx->RemotePort = port;
  ctx->DisableHeader = 1;

  strcpy(ctx->RemoteAddress, remote_address);
  strcpy(ctx->RemoteHostName, remote_host_name);

  sts = CreateSocket( ctx);
  if ( EVEN(sts)) return sts;

  udp_ctx = ctx;
  return 1;
}

#if 0
int main(int argc, char *argv[])
{
  struct timeval tv;
  int sts;

  udp_tCtx ctx = calloc( 1, sizeof(*ctx));
  ctx->LocalPort = 3051;
  ctx->RemotePort = 3051;
  ctx->DisableHeader = 1;

  /* Read arguments */
  if (argc >= 2)
    strcpy(ctx->RemoteAddress, argv[1]);
  else {
    printf( "Invalid arguments\n");
    exit(0);
  }

  if (argc >= 3)
    strcpy(ctx->RemoteHostName, argv[2]);

  if ( argc >= 4)
    ctx->Receive = 1;

  CreateSocket( ctx);

  for (;;) {

    RemoteSleep(TIME_INCR);

    if ( ctx->Receive) {
      tv.tv_sec = 0;
      tv.tv_usec = 0;

      FD_ZERO(&fds);
      FD_SET(my_socket, &fds);
      sts = select(32, &fds, NULL, NULL, &tv);

      if (sts > 0) UdpReceive( ctx);

      if (sts < 0) {
	errh_Error("Select, %d", sts);
	exit(0);
      }
    }
    else {
      UdpSend( ctx, "Hej hopp", 8);
    }
  }
}
#endif