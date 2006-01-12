/* 
 * Proview   $Id: rs_remote_dmq.c,v 1.1 2006-01-12 06:39:33 claes Exp $
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
*
* Filename:             remote_dmq.c
*
*                       Date    Pgm.    Read.   Remark
* Modified              971016  CJu
*
* Description:		Implements remote transport process DMQ
*
**************************************************************************/

#ifdef __DECC
#pragma message disable GLOBALEXT
#endif


/*_Include files_________________________________________________________*/

#include <ssdef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iodef.h>
#include <descrip.h>
#include <psldef.h>
#include <libdtdef.h>
#include <lib$routines.h>
#include "p_entry.h"
#include "p_proces.h"
#include "p_return.h"
#include "p_symbol.h"
#include "p_typecl.h"

#include "pwr_class.h"
#include "rs_remote_mq.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "pwr_baseclasses.h"
#include "pwr_ssabclasses.h"
#include "rs_remote_msg.h"
#include "rs_remote.h"
#include "rs_remtrans_utils.h"

#define CLASS_APL_RESP	101
#define CLASS_RESP	102

mq_uAddress my_queue;			/* Queue number */
char receive_buffer[32767];		/* Pointer to received message */
char *receive_msg = (char *) &receive_buffer;
short buffer_size = sizeof(receive_buffer);
char receive_prio = 0;			/* Prio for messages */
q_address receive_src;			/* group, process  received message */
short int receive_class;		/* Class of received message */
short int receive_type;			/* Type of received message */
short int receive_size;			/* Size of received message */
struct PSB receive_psb;			/* Status block from PSB */
char receive_force_j = PDEL_DEFAULT_JRN;	/* Journal flag */

remnode_item *remnode_list = NULL;	/* List of remnodes (internal structure) */
float cycle_time = 1.0;			/* Cycle time in seconds */
float min_cycle_time = 0.01;		/* Shortest scan time */

int32 timer_id = 1;

/*************************************************************************
**************************************************************************
*
* Namn : exith
*
* Typ  : void
*
* Typ		Parameter	       IOGF	Beskrivning
*
* Beskrivning : Exit handler that is called before process termination
*
**************************************************************************
**************************************************************************/

void exith(void)
{
  int32 sts;

  sts = pams_cancel_timer(&timer_id);
  sts = pams_exit();

  return;
}

/*************************************************************************
**************************************************************************
*
* Namn : DeclareExitHandler
*
* Typ  : void
*
* Typ		Parameter	       IOGF	Beskrivning
*
* Beskrivning : Declares an exit handler that is called before 
*		process termination
*
**************************************************************************
**************************************************************************/

void DeclareExitHandler(void)
{
  atexit(exith);
}

/*************************************************************************
**************************************************************************
*
* Namn : InitCycleTimer
*
* Typ  : unsigned int
*
* Typ		Parameter	       IOGF	Beskrivning
*
* Beskrivning : Sets DMQ scan timer
*
**************************************************************************
**************************************************************************/

unsigned int InitCycleTimer()
{
  unsigned int sts;
  int32 dmq_sts;
  unsigned int time_function_code = LIB$K_DELTA_SECONDS_F;
  char format = 'S';
  int32 delta_time[2];
  int32 tick_count = 0;

  /* Konvertera cykeltiden till intern deltatid */
  sts = lib$cvtf_to_internal_time(&time_function_code, 
				  &cycle_time,
				  &delta_time);
  if (EVEN(sts)) exit(sts);

  dmq_sts = pams_set_timer(&timer_id, &format, &tick_count, (int32 *) &delta_time);

  return(sts);
}

/*************************************************************************
**************************************************************************
*
* Namn : WaitEvent
*
* Typ  : unsigned int
*
* Typ		Parameter	       IOGF	Beskrivning
*
* Beskrivning : Waits for Net receive or timer expiration.
*
**************************************************************************
**************************************************************************/

unsigned int WaitEvent(void)
{
  int32 dmq_sts;
  int32 receive_timeout = 0;

  dmq_sts = pams_get_msgw(receive_msg, &receive_prio, &receive_src,
		          &receive_class, &receive_type, &buffer_size,
		          &receive_size, &receive_timeout, 0, &receive_psb,
		          0, 0, 0, 0, 0);

  if (ODD(dmq_sts))
  {
    if (receive_type == MSG_TYPE_TIMER_EXPIRED)
      return(RT_TIMEREXP);
    else
      return(RT_RECEIVE);
  }
  else
    return(dmq_sts);
}

/*************************************************************************
**************************************************************************
*
* Namn : send_pollbuff
*
* Typ  : void
*
* Typ		Parameter	       IOGF	Beskrivning
*
* Beskrivning : Sends a poll or IO update message to Remote node
*
**************************************************************************
**************************************************************************/

void send_pollbuff(	remnode_item *remnode,
			pssupd_buffer *buf)
{
  return;
}

/*************************************************************************
**************************************************************************
*
* Namn : remnode_send
*
* Typ  : unsigned int
*
* Typ		Parameter	       IOGF	Beskrivning
*
* Beskrivning : Sends a message to Remote node
*
**************************************************************************
**************************************************************************/

unsigned int remnode_send(remnode_item *remnode,
			  pwr_sClass_RemTrans *remtrans,
			  char *buf,
			  int buf_size)

{
  unsigned int sts;
  int32 dmq_sts;

  char put_prio = 0;			/* Prio for messages */
  q_address dest;
  short class;
  short type;
  short msg_size;
  int32 timeout = 100;
  struct PSB put_psb;
  char delivery = PDEL_MODE_WF_DQF;
  char put_uma = PDEL_UMA_SAF;

  dest.au.group = remnode->objp->Address[0];
  class = remtrans->Address[0];
  type = remtrans->Address[1];
  dest.au.queue = remtrans->Address[2];
  msg_size = buf_size;

  dmq_sts =  pams_put_msg(buf, 
		          &put_prio,
		          &dest,
		          &class,
		          &type,
		          &delivery,
		          &msg_size,
		          &timeout,
		          &put_psb,
		          &put_uma, 
		          0, 0, 0, 0);

  return(dmq_sts);
}

/*************************************************************************
**************************************************************************
*
* Namn : ReceiveComplete
*
* Typ  : unsigned int
*
* Typ		Parameter	       IOGF	Beskrivning
*
* Beskrivning : Invoked when a message is received on the ethernet.
*
**************************************************************************
**************************************************************************/

unsigned int ReceiveComplete()
{
  unsigned int sts;
  int32 dmq_sts;

  char search_remnode = TRUE;
  char search_remtrans = TRUE;
  char send_response = FALSE;
  remnode_item *remnode;
  remtrans_item *remtrans;

  char *response_buffer;			/* Pointer to response message */
  int response_prio = 0;			/* Prio for response */
  short int response_class = CLASS_RESP;	/* Class of response */
  int response_size = 4;			/* Size of response  = 4 bytes */


  search_remnode = TRUE;

  remnode = remnode_list;
  while (remnode && search_remnode)
  {
    if (receive_src.au.group == remnode->objp->Address[0])
      search_remnode = FALSE;
    else
      remnode = (remnode_item *) remnode->next;
  }

  if (search_remnode) 

    errh_CErrLog(REM__UNKNOWNSOURCE);

  else
  {
    search_remtrans = true;

    remtrans = remnode->remtrans;
    while(remtrans && search_remtrans)
    {
      if (remtrans->objp->Address[0] == receive_class &&
	  remtrans->objp->Address[1] == receive_type &&
	  remtrans->objp->Direction == REMTRANS_IN)
      {
	search_remtrans = false;
	sts = RemTrans_Receive(remtrans, receive_buffer, receive_size);
	break;
      }
      remtrans = (remtrans_item *) remtrans->next;
    }
    if (search_remtrans) 
    {
      remnode->objp->ErrTransCount++;
      errh_CErrLog(REM__NOREMTRANS, errh_ErrArgAF(remnode->objp->NodeName));
    }

  }	/* if search_remnode ... */

  /* Kvittera till MRS om s� beh�vs */

  if (receive_psb.del_psb_status == PAMS__CONFIRMREQ ||
      receive_psb.del_psb_status == PAMS__POSSDUPL)
    pams_confirm_msg(receive_psb.seq_number, &dmq_sts, &receive_force_j);

  return(1);
}

/*
 * Main routine
 */

main(int argc, char *argv[])
{
  remnode_item *remnode = NULL;			/* Temporary remnode */
  unsigned int sts;				/* Status from function calls etc. */
  unsigned int sts2;				/* Status from function calls etc. */
  pwr_tObjid node;				/* Loop node */
  pwr_sClass_RemNode *nodep;
  pwr_tObjid pwr_node;				/* Own Pwr node */
  pwr_sClass_PlcProcess *pwr_nodep;		/* Ref to own Pwr node */
  pwr_tTime OriginDnoChgTime;			/* LastDnoChgTime at start */
  int		mygroup;			/* DMQ group no */
 
  mq_uAddress gdhpams_process;

  DeclareExitHandler();

  /* Get our DMQ queue from argument list */

  if (argc >= 2)
    sscanf(argv[1], "%ld", &my_queue.All);
  else
    my_queue.All = 125;

  /* Declare us as a DMQ and GDH process. Max 5 retries */

    sts = mq_AttachQueue ( &my_queue, &gdhpams_process);
    if (EVEN(sts)) exit(sts);
    sts = gdh_Init("rs_remote_dmq");
    if (EVEN(sts)) exit(sts);

  /* Get pointer to $Node-object */

  sts = gdh_GetClassList(pwr_cClass_PlcProcess, &pwr_node);
  sts = gdh_ObjidToPointer(pwr_node, (pwr_tAddress *) &pwr_nodep);
  memcpy(&OriginDnoChgTime, &pwr_nodep->LastChgTime, sizeof(pwr_tTime));

  /* Get first remnode object, only on local node */

  sts = gdh_GetClassList(pwr_cClass_RemNode, &node);

  if (EVEN(sts)) exit(sts);

  while (ODD(sts))
  {

    sts = gdh_ObjidToPointer(node, (pwr_tAddress *) &nodep);

    /* Check if transport type is DMQ and if remnode's process number matches ours */

    if (nodep->TransportType == REMNODE_TRANSPORT_DMQ  &&
	nodep->Address[1] == my_queue.All)
    {

      /* Allocate a new item and link it */
      remnode = malloc(sizeof(remnode_item));
      if (!remnode) exit(REM__NOMEMORY); 

      remnode->objp = nodep;
      remnode->objid = node;

      remnode->next = (struct remnode_item *) remnode_list;
      remnode_list = remnode;

      remnode->Time_since_scan = 0;
      remnode->Time_since_poll = 0;
      remnode->Time_since_IO   = 0;
      remnode->Time_since_send = 0;

      /* Initialize remtrans objects */
      mygroup = gdhpams_process.au.Group;
      sts = RemTrans_Init(remnode, mygroup);

      if (EVEN(sts)) exit(sts);

      /* Check if any cycle time is the shortest so far */

      if (remnode->objp->CycleTime < cycle_time &&
	  remnode->objp->CycleTime >= min_cycle_time)
	     	cycle_time = remnode->objp->CycleTime;

    }		/* if transport_type... */

    sts = gdh_GetNextObject(node, &node);

  }		/* while */

  /* Init Cycle Timer */

  sts = InitCycleTimer();
  if (EVEN(sts)) exit(sts);

  /* Loop forever and wait for net message or timeout */

  while (!doomsday)
  {

    sts = WaitEvent();
    if (EVEN(sts)) exit(sts);

    /* Om LastDnoChgTime �r �ndrad (PLC-programbyte), avsluta */

    if (memcmp(&OriginDnoChgTime, &pwr_nodep->LastChgTime,
	sizeof(pwr_tTime)) != 0) exit(0);

    if (sts == RT_RECEIVE)
    {
      sts = ReceiveComplete();
      if (EVEN(sts)) exit(sts);
    }

    else if (sts == RT_TIMEREXP)
    {
      remnode = remnode_list;
      while(remnode)
      {
        remnode->Time_since_scan += cycle_time;
        remnode->Time_since_send += cycle_time;
	if (remnode->Time_since_scan >= remnode->objp->CycleTime)
	{
	  sts = RemTrans_Cyclic(remnode, &remnode_send);
	  remnode->Time_since_scan = 0.0;
	}
	remnode = (remnode_item *) remnode->next;
      }

      /*
       * Init a new timer
       */

      sts = InitCycleTimer();
      if (EVEN(sts)) exit(sts);
    }
  }
}
