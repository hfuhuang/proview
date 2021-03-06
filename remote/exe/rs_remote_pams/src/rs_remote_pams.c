/* 
 * Proview   $Id: rs_remote_pams.c,v 1.1 2006-01-12 06:39:33 claes Exp $
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
* Filename:             remote_pams.c
*
*                       Date    Pgm.    Read.   Remark
* Modified              950307  CJu
*
* Description:		Implements remote transport process PAMS
*
**************************************************************************/

#ifdef __DECC
#pragma message disable GLOBALEXT
#endif


/*_Include files_________________________________________________________*/

#ifdef  OS_ELN
#include $vaxelnc
#include $exit_utility
#include $function_descriptor
#include ssdef
#include stdio
#include stdlib
#include string
#include math
#include iodef
#include descrip
#include psldef
#include libdtdef
#include lib$routines
/***
#include pams_c_process
#include pams_c_group
#include pams_c_type_class
#include pams_c_return_status_def
#include pams_c_symbol_def
#include pams_c_entry_point
#include sbs_msg_def
***/
#endif

#ifdef  OS_VMS
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
#include <pams_c_process.h>
#include <pams_c_type_class.h>
#include <pams_c_return_status.h>
#include <sbs_msg_def.h>
#endif

#include "pwr_class.h"
#include "pwrs_c_node.h"
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

#ifdef OS_ELN
  PROCESS my_proc;
  PORT job_port;
#endif

mq_uAddress my_pams_process;		/* Pams processnumber for this job */
char receive_buffer[32767];			/* Pointer to received message */
int receive_prio = 0;			/* Prio for messages */
mq_uAddress receive_src;		/* group, process  received message */
short int receive_class;		/* Class of received message */
short int receive_type;			/* Type of received message */
int receive_size;			/* Size of received message */

remnode_item *remnode_list = NULL;		/* List of remnodes (internal structure) */
float cycle_time = 1.0;				/* Cycle time in seconds */
float min_cycle_time = 0.01;			/* Shortest scan time */

unsigned int timer_id = 1;

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
  unsigned int sts;

  sts = pams_cancel_timer(&timer_id);
  sts = pams_exit(&my_pams_process);
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
#ifdef OS_ELN

  FUNCTION_DESCRIPTOR fn_descriptor;

  eln$declare_exit_handler(ELN$PASS_FUNCTION_DESCRIPTOR(fn_descriptor, exith),
			   NULL);
#else

  atexit(exith);
#endif
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
* Beskrivning : Sets PAMS scan timer
*
**************************************************************************
**************************************************************************/

unsigned int InitCycleTimer()
{
  unsigned int sts;
  unsigned int sts2;
  unsigned int time_function_code = LIB$K_DELTA_SECONDS_F;
  char format = 'S';
  unsigned short delta_time[4];

  /* Konvertera cykeltiden till intern deltatid */
  sts = lib$cvtf_to_internal_time(&time_function_code, 
				  &cycle_time,
				  &delta_time);
  if (EVEN(sts)) exit(sts);

  sts = pams_set_timer(&timer_id, &format, &0, &delta_time);

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
  unsigned int sts;
  unsigned int receive_timeout = 0;
  short buffer_size;

  buffer_size = sizeof(receive_buffer);

  sts = pams_get_msgw(&receive_buffer, &receive_prio, &receive_src,
		      &receive_class, &receive_type, &buffer_size,
		      &receive_size, &receive_timeout);

  if (sts == SS$_NORMAL)
  {
    if (receive_type == MSG_TYPE_TIMER_EXPIRED)
      return(RT_TIMEREXP);
    else
      return(RT_RECEIVE);
  }
  else
    return(RT_ERROR);
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

  char *send_buffer;			/* Pointer to message */
  int send_prio = 0;			/* Prio for messages */
  mq_uAddress dest;			/* group, process for message */

  sts = pams_alloc_msg(&buf_size, &send_buffer);
  if (EVEN(sts)) exit(sts);

  memcpy(send_buffer, buf, buf_size);
  dest.au.Group = remnode->objp->Address[0];
  dest.au.Queue = remtrans->Address[2];

  sts = pams_send_msg(&send_buffer, 
		      &send_prio,
		      &dest,
		      &remtrans->Address[0],
		      &remtrans->Address[1],
		      &0);

  if (ODD(sts) && (remtrans->Address[0] == CLASS_APL_RESP))
    return(STATUS_BUFACK);
  else
    return(sts);
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
    if (receive_src.au.Group == remnode->objp->Address[0])
      search_remnode = FALSE;
    else
      remnode = (remnode_item *) remnode->next;
  }

  if (search_remnode) 

    errh_CErrLog(REM__UNKNOWNSOURCE);

  else
  {
    if (receive_class == CLASS_RESP)		/* Response message */
    {
      rem_t_transbuff *transbuff;			

      transbuff = remnode->transbuff;

      if (transbuff && 
          transbuff->remtrans->objp->Address[0] == CLASS_APL_RESP &&
          transbuff->remtrans->objp->Address[1] == receive_type &&
	  (memcmp(&transbuff->data, receive_buffer, response_size ) == 0))
      {
	remnode->transbuff = (rem_t_transbuff *) transbuff->next;
	transbuff->remtrans->objp->Buffers--;
	remnode->Time_since_send = remnode->objp->ErrTime;
	free(transbuff);

	transbuff = remnode->transbuff;
	if (transbuff)
	{
	  sts = remnode_send(remnode, 
	       		     transbuff->remtrans->objp,
	       		     &transbuff->data, transbuff->size);
	  if (ODD(sts))
	  {
	    remnode->transbuff = (rem_t_transbuff *) transbuff->next;
	    transbuff->remtrans->objp->Buffers--;
	    free(transbuff);
	  }
	}
      }
    }

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
	  if (ODD(sts) && receive_class == CLASS_APL_RESP) send_response = TRUE;
	  break;
	}
	remtrans = (remtrans_item *) remtrans->next;
      }
      if (search_remtrans) remnode->objp->ErrTransCount++;
    }

    if (send_response)
    {
      /* Send a response message back to source with class response */

      sts = pams_alloc_msg(&response_size, &response_buffer);
      if (EVEN(sts)) exit(sts);

      memcpy(response_buffer, receive_buffer, response_size);

      sts = pams_send_msg(&response_buffer, 
		          &response_prio, 
		          &receive_src, 
		          &response_class,
		          &receive_type, 
		          &0);
      if (EVEN(sts)) exit(sts);
    }
  }	/* if search_remnode ... */

  return(1);
}

#ifdef OS_ELN
/*************************************************************************
**************************************************************************
*
* Namn : WaitFor_PLC_Change
*
* Typ  : int
*
* Typ		Parameter	       IOGF	Beskrivning
*
* Beskrivning : Subprocess  waits for message from REMOTEHANDLER on job_port,
*		that will kill the process at PLC-change.
*
**************************************************************************
**************************************************************************/

int WaitFor_PLC_Change()
{
  pwr_tStatus sts;

  ker$wait_any(&sts, NULL, NULL, &job_port);
  exith();
  ker$delete(&sts, my_proc);
}
#endif

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
  pwr_sClass_PlcProcess *pwr_nodep;		/* Ref to own Pwr plcprocess */
  pwr_tTime OriginDnoChgTime;			/* LastDnoChgTime at start */
  int		mygroup;			/* PAMS group no */
 
  mq_uAddress gdhpams_process;

#ifdef OS_ELN
  PROCESS id;
  static struct dsc$descriptor_s name_desc = {0, DSC$K_DTYPE_T,DSC$K_CLASS_S,0};
  NAME name_id;

  /* Get process-id to be able to kill myself. Create subprocess to kill. */
  ker$job_port(&sts, &job_port);
  ker$current_process(&sts, &my_proc);

  /* Create a name for my own process (first process in job) */

  name_desc.dsc$a_pointer = argv[3];
  name_desc.dsc$w_length = strlen(argv[3]);
  ker$create_name(&sts, &name_id, &name_desc, my_proc, NULL);

  /* Create process that waits for PLC-change (hotswitch) */

  ker$create_process( &sts, &id, WaitFor_PLC_Change, NULL);
#endif

  DeclareExitHandler();

  /* Get our PAMS process number from argument list */

  if (argc >= 2)
    sscanf(argv[1], "%ld", &my_pams_process.All);
  else
    my_pams_process.All = 125;

  /* Declare us as a PAMS and GDH process. Max 5 retries */

    sts = mq_AttachQueue ( &my_pams_process, &gdhpams_process);
    if (EVEN(sts)) exit(sts);
    sts = gdh_Init("rs_remote_pams");
    if (EVEN(sts)) exit(sts);

  /* Get pointer to $Node-object */

  sts = gdh_GetClassList( pwr_cClass_PlcProcess, &pwr_node);
  sts = gdh_ObjidToPointer(pwr_node, (pwr_tAddress *) &pwr_nodep);
  memcpy(&OriginDnoChgTime, &pwr_nodep->LastChgTime, sizeof(pwr_tTime));

  /* Get first remnode object, only on local node */

  sts = gdh_GetClassList(pwr_cClass_RemNode, &node);

  if (EVEN(sts)) exit(sts);

  while (ODD(sts))
  {

    sts = gdh_ObjidToPointer(node, (pwr_tAddress *) &nodep);

    /* Check if transport type is PAMS and if remnode's process number matches ours */

    if (nodep->TransportType == REMNODE_TRANSPORT_PAMS  &&
	nodep->Address[1] == my_pams_process.All)
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
