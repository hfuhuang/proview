/* 
 * Proview   $Id: rt_io_m_ssab_remoterack.c,v 1.2 2006-04-12 10:14:49 claes Exp $
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

/* rt_io_m_ssab_remoterack.c -- io methods for ssab remote rack objects. */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "pwr_basecomponentclasses.h"
#include "pwr_ssaboxclasses.h"
#include "rt_io_base.h"
#include "rt_errh.h"
#include "rt_io_rack_init.h"
#include "rt_io_m_ssab_locals.h"
#include "rt_io_msg.h"


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/

static pwr_tStatus IoRackInit (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  io_sRackLocal *local;
  io_sCard *cardp;
  pwr_tCid cid;
  pwr_sClass_Ssab_RemoteRack *op;
  int sts, in_offset, out_offset;

  pwr_sClass_Di_DIX2 *dicp;
  pwr_sClass_Do_HVDO32 *docp;
  pwr_sClass_Ai_AI32uP *aicp;
  pwr_sClass_Co_PI24BO *cocp;
  
  op = (pwr_sClass_Ssab_RemoteRack *) rp->op;
  local = calloc( 1, sizeof(*local));
  rp->Local = local;
  
  /* Create socket, store in local struct */
  
  local->s = socket(AF_INET, SOCK_DGRAM, 0);
  if (local->s < 0) { 
    errh_Error( "Error creating socket for IO remote rack %s, %d", rp->Name, local->s);
    op->Status = IO__INITFAIL;
    return IO__ERRINIDEVICE;
  }
  
  /* Set local port same as remote port and bind the created socket */
  
  local->my_addr.sin_family = AF_INET;
  local->my_addr.sin_port = htons(op->Port);
  sts = bind(local->s, (struct sockaddr *) &local->my_addr, sizeof(local->my_addr));
  if (sts != 0) { 
    errh_Error( "Error binding local socket for IO remote rack %s, %d", rp->Name, sts);
    op->Status = IO__INITFAIL;
    return IO__ERRINIDEVICE;
  }
  
  /* Initialize remote address structure */

  local->rem_addr.sin_family = AF_INET;
  local->rem_addr.sin_port = htons(op->Port);
  local->rem_addr.sin_addr.s_addr = inet_addr((char *) &(op->Address));
  
  /* Connect to remote address */
  
  sts = connect(local->s, (struct sockaddr *) &local->rem_addr, sizeof(local->rem_addr)); 
  if (sts != 0) { 
    errh_Error( "Error binding remote socket for IO remote rack %s, %d", rp->Name, sts);
    op->Status = IO__INITFAIL;
    return IO__ERRINIDEVICE;
  }
  
  /* Loop all cards */
  cardp = rp->cardlist;
  in_offset = 0;
  out_offset = 0;
  local->in_items = 0;
  local->out_items = 0;
  
  while (cardp) {
    cid = cardp->Class;
    
    switch (cid) {

      case pwr_cClass_Di_DIX2:
        dicp = (pwr_sClass_Di_DIX2 *) cardp->op;
	cardp->offset = in_offset;
	in_offset += 2;
        printf("Di card found, address: %d\n", dicp->RegAddress);
	break;
      case pwr_cClass_Do_HVDO32:
        docp = (pwr_sClass_Do_HVDO32 *) cardp->op;
        printf("Do card found, address: %d\n", docp->RegAddress);
	cardp->offset = out_offset;
	out_offset += 2;
	break;
      case pwr_cClass_Ai_AI32uP:
        aicp = (pwr_sClass_Ai_AI32uP *) cardp->op;
        printf("Ai card found, address: %d\n", aicp->RegAddress);
	break;
      case pwr_cClass_Co_PI24BO:
        cocp = (pwr_sClass_Co_PI24BO *) cardp->op;
        printf("Co card found, address: %d\n", cocp->RegAddress);
	break;
    }
    
    cardp = cardp->next;
  }

  local->in_items = in_offset;
  local->out_items = out_offset;
  
  local->in.service = 1;
  local->in.length = local->in_items*4 + 4;
  
  op->RX_packets = 0;
  op->TX_packets = 0;
    
  /* Log initialization */
  
  errh_Info( "Init of IO remote rack %s, %d/%s:%d",
  	     rp->Name, ntohs(local->my_addr.sin_port), 
	     inet_ntoa(local->rem_addr.sin_addr), ntohs(local->rem_addr.sin_port));
  op->Status = IO__NORMAL;
  return IO__SUCCESS;
}

static pwr_tStatus IoRackClose (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  if (rp->Local) free (rp->Local);
  return IO__SUCCESS;
}

static pwr_tStatus IoRackRead (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  int sts;
  fd_set fds;
  struct timeval tv;
  io_sRackLocal *local = (io_sRackLocal *) rp->Local;
  pwr_sClass_Ssab_RemoteRack *op = (pwr_sClass_Ssab_RemoteRack *) rp->op;

  sts = 1;
  while (sts > 0) {
    FD_ZERO(&fds);
    FD_SET(local->s, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    sts = select(32, &fds, NULL, NULL, &tv);
    if (sts > 0) {
      recv(local->s, &local->in, sizeof(local->in), 0);
      op->RX_packets++;
    }
  } 
  
  return IO__SUCCESS;
}

static pwr_tStatus IoRackWrite (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  int sts;
  io_sRackLocal *local = (io_sRackLocal *) rp->Local;
  pwr_sClass_Ssab_RemoteRack *op = (pwr_sClass_Ssab_RemoteRack *) rp->op;

  // Send write request
  local->out.service = 2;
  local->out.length = 8;
  
  local->out.item[0].address = 64512;
  local->out.item[0].data = 0xffff;
  sts = send(local->s, &local->out, local->out.length, 0);

  op->TX_packets++;
  
  // Send read request
  
  sts = send(local->s, &local->in, local->in.length, 0);

  op->TX_packets++;

  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Ssab_RemoteRack) = {
  pwr_BindIoMethod(IoRackInit),
  pwr_BindIoMethod(IoRackClose),
  pwr_BindIoMethod(IoRackRead),
  pwr_BindIoMethod(IoRackWrite),
  pwr_NullMethod
};


