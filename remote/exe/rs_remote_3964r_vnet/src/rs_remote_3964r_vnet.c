/* 
 * Proview   $Id: rs_remote_3964r_vnet.c,v 1.1 2006-01-12 06:39:33 claes Exp $
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
*                       3 9 6 4 R
*                       ==========
**************************************************************************
*
* Filename:             remote_3964r_vnet.c
*
*                       Date    Pgm.    Read.   Remark
* Modified             
*
*
* Description:          Implements remote transport process 3964R.
*                       For poll off io and commondata from PSS7000(VNET).
*
**************************************************************************
**************************************************************************/

/*_Include files_________________________________________________________*/

#ifdef  OS_ELN
#include $vaxelnc
#include $exit_utility
#include $function_descriptor
#include $dda_utility
#include $elnmsg
#include $kernelmsg
#include ssdef
#include stdio
#include stdlib
#include string
#include math
#include iodef
#include descrip
#include psldef
#include libdtdef
#include starlet
#include lib$routines
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
#include <starlet.h>
#include <lib$routines.h>
#endif

#include "pwr_class.h"
#include "pwr_systemclasses.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "pwr_baseclasses.h"
#include "pwr_ssabclasses.h"
#include "rs_remote_msg.h"
#include "rs_remote.h"
#include "rs_remtrans_utils.h"
#include "rs_remio_utils.h"
#include "rs_remote_rad50.h"

/*_Function prototypes___________________________________________________*/

static void             exith(void);
static void             DeclareExitHandler(void);
static unsigned int     InitTimers(void);
static unsigned int     InitNet(void);
static unsigned int     remnode_send(remnode_item        *remnode,
                                     pwr_sClass_RemTrans *remtrans,
                                     char                *buf,
                                     int                 buf_size);
static unsigned int     send_it(char *buf, 
                                int  buf_size);
static unsigned int     Receive();
static unsigned int     ReceiveHandler();

#ifdef OS_VMS
int                     WaitFor_PLC_Change();
#endif

/*_defines_________________________________________________________________*/

#define NUMBER_OF_STOP_CHAR 3
#define NET_HEADER_SIZE_IO 6
#define NET_HEADER_SIZE_COMMON 8
#define MAX_SIZE_TELEGRAM 512

#define NUL 0
#define STX 2
#define ETX 3
#define DLE 16
#define NAK 21

/*_variables_______________________________________________________________*/

#ifdef  OS_ELN
/*
 * ELN specific variables
 */
PORT           virtual_serial_line_port;
PORT           job_port;
PROCESS        my_proc;
LARGE_INTEGER  timeout_ack;    /* Ack. time out on STX request */
LARGE_INTEGER  timeout_char;   /* Time out receive of next character */
LARGE_INTEGER  timeout_cycle;  /* Init. from remnode:CycleTime */

#else
/*
 * VMS specific variables
 */
unsigned int   receive_ef;
unsigned int   timer_ef;
unsigned int   ef_mask;

#endif

/*
 * Variables common for VMS and ELN
 */
remnode_item   remnode_struct;
remnode_item   *remnode = &remnode_struct;

/*************************************************************************
**************************************************************************
*
* Namn : exith
*
* Typ  : void
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Exit handler that is called before process termination
*
**************************************************************************
**************************************************************************/

static void exith(void)
{
#ifdef OS_ELN

  return;

#else

  return;

#endif
}

/*************************************************************************
**************************************************************************
*
* Namn : DeclareExitHandler
*
* Typ  : void
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Declares an exit handler that is called before
*               process termination
*
**************************************************************************
**************************************************************************/

static void DeclareExitHandler(void)
{
#ifdef OS_ELN

  FUNCTION_DESCRIPTOR fn_descriptor;

  eln$declare_exit_handler(ELN$PASS_FUNCTION_DESCRIPTOR(fn_descriptor, exith),
                           NULL);
#else

  atexit(exith);
#endif
}

/************************************************************************
**************************************************************************
*
* Namn : InitTimers
*
* Typ  : unsigned int
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Initiates the timers used in this process.
*
**************************************************************************
**************************************************************************/

static unsigned int InitTimers(void)
{
  unsigned int  sts;
  unsigned int  time_function_code    = LIB$K_DELTA_SECONDS_F;
  float         max_acknowledge_delay = 2.0;
  float         max_char_delay        = 0.22;

  /* Konvertera cykeltiden till intern deltatid */
  sts = lib$cvtf_to_internal_time(&time_function_code,
                                  &remnode->objp->CycleTime,
                                  &timeout_cycle);

  if ( ODD(sts) ){
    /* Set timeout for response character from receiver */
    sts = lib$cvtf_to_internal_time(&time_function_code,
                                    &max_acknowledge_delay,
                                    &timeout_ack);
  }

  if ( ODD(sts) ){
    /* Set timeout for max delay between receive of two characters */
    sts = lib$cvtf_to_internal_time(&time_function_code,
                                    &max_char_delay,
                                    &timeout_char);
  }

  return(sts);
}

/************************************************************************
**************************************************************************
*
* Namn : InitNet
*
* Typ  : unsigned int
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Assigns and sets up device
*
**************************************************************************
**************************************************************************/

static unsigned int InitNet(void)
{
  unsigned int            i, sts;
  char                    ch = '\0';
  char                    namestring[80];
  char                    portname[100];
  PORT                    DDA_destination_port;
  struct dsc$descriptor_s dataportname;

  for ( i=0 ; ( ch != ':' )&&( i < 80 ) ; i++ )
  {
    namestring[i] = ch = remnode->objp->NodeName[i];
  }
  namestring[--i] = '\0';

  sprintf(portname, "%s$ACCESS", namestring);
  dataportname.dsc$a_pointer = portname;
  dataportname.dsc$w_length  = strlen(portname);
  dataportname.dsc$b_class   = DSC$K_CLASS_S;
  dataportname.dsc$b_dtype   = DSC$K_DTYPE_T;

  /* Associate portnamne with serial line port ID */
  ker$translate_name(&sts, &DDA_destination_port, &dataportname, NAME$LOCAL);
  if ( ODD(sts) )
    /* Create a new port object */
    ker$create_port(&sts, &virtual_serial_line_port, NULL);
  if ( ODD(sts) )
    /* Connect the port object to the destination_port */
    ker$connect_circuit(&sts,
                        &virtual_serial_line_port,
                        &DDA_destination_port,
                        NULL, NULL, NULL, NULL);

  return(sts);
}


/*************************************************************************
**************************************************************************
*
* Namn : send_pollbuff
*
* Typ  : void
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Sends a poll or IO update message to Remote node
*
**************************************************************************
**************************************************************************/

void send_pollbuff(remnode_item *remnode, pssupd_buffer *buf)
{
  unsigned int sts, buf_size;
  pssupd_buffer_vnet *vnet_buf = buf;

  /* Fill in remaining data in poll telegram */
  AsciiToR50("PSSUPD", &vnet_buf->receive_task);
  vnet_buf->common_name[0] = remnode->objp->Address[0];
  vnet_buf->common_name[1] = remnode->objp->Address[1];

  buf_size = vnet_buf->length * 2; /*  Convert to bytes  */
  sts = send_it((char*)vnet_buf, buf_size);

  return;
}

/*************************************************************************
**************************************************************************
*
* Namn : remnode_send
*
* Typ  : unsigned int
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Sends a RemTrans message to Remote node
*
**************************************************************************
**************************************************************************/

static unsigned int remnode_send(remnode_item *remnode,
                                 pwr_sClass_RemTrans *remtrans,
                                 char *buf,
                                 int buffer_size)
{
  unsigned int  sts;

  sts = send_it(buf, buffer_size);
  return sts;
}

/*************************************************************************
**************************************************************************
*
* Namn : send_it
*
* Typ  : unsigned int
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Executes the sending of a message according to 3964r.
*
**************************************************************************
**************************************************************************/

static unsigned int send_it(char *buf, int buf_size)
{
  unsigned int          sts, i;
  unsigned int          size_of_telegram;
  unsigned int          number_of_DLE = 0;
  unsigned int          nbr_of_bytes_written = 0;
  unsigned int          nbr_of_bytes_read = 0;
  unsigned char         ch;
  unsigned char         BCC = DLE ^ ETX;
  unsigned char         received_char = '\0';
  unsigned char         *restore_buf_ptr = buf;
  unsigned char         telegram[MAX_SIZE_TELEGRAM];
  static unsigned char  sstx[2] = {STX, '\0'};
  static unsigned char  sdle[2] = {DLE, '\0'};
  static unsigned char  snak[2] = {NAK, '\0'};

/*************************************************************************/
/** Count DLE characters and calculate the size of the telegram.        **/
/*************************************************************************/
  for ( i=0 ; i<buf_size ; i++ )
  {
    if ( *buf++ == DLE )
      number_of_DLE += 1;
  }
  size_of_telegram = buf_size + number_of_DLE + NUMBER_OF_STOP_CHAR;

/*************************************************************************/
/**   Fill up telegram with contents of message and calculate BCC       **/
/*************************************************************************/
  buf = restore_buf_ptr;
  for ( i=0 ; i<(buf_size + number_of_DLE) ; i++ )
  {
    ch = telegram[i] = *buf++;
    BCC ^= ch;
    if ( ch == DLE ){
      telegram[++i] = DLE;
      BCC ^= ch;
    }
  }
  telegram[i++] = DLE;
  telegram[i++] = ETX;
  telegram[i]   = BCC;

/*************************************************************************/
/**    Execute the send procedure                                       **/
/*************************************************************************/
  /* Send STX and wait for answer */
  eln$tty_write(&sts, &virtual_serial_line_port, 1, &sstx,
                &nbr_of_bytes_written, NULL, NULL);
  if ( ODD(sts) ){
    /* wait for character or timeout */
    eln$tty_read(&sts, &virtual_serial_line_port, 1, &received_char,
                 &nbr_of_bytes_read, 2, NULL, &timeout_ack,
                 NULL, NULL, NULL, NULL);
  }
  if ( EVEN(sts) || (received_char != DLE) || (sts == ELN$_TIMEOUT) ){
    /* Try one more time to establish contact */
    eln$tty_write(&sts, &virtual_serial_line_port, 1, &sstx,
                  &nbr_of_bytes_written, NULL, NULL);
    if ( ODD(sts) ){
      /* wait for character or timeout */
      eln$tty_read(&sts, &virtual_serial_line_port, 1, &received_char,
                   &nbr_of_bytes_read, 2, NULL, &timeout_ack,
                   NULL, NULL, NULL, NULL);
    }
    if ( EVEN(sts) || (received_char != DLE) || (sts == ELN$_TIMEOUT) )
      sts = FALSE;
  }
  
  if ( ODD(sts) ){
    /* Contact is established. Send telegram */
    eln$tty_write(&sts, &virtual_serial_line_port, size_of_telegram,
                  &telegram,
                  &nbr_of_bytes_written, NULL, NULL);
    if ( ODD(sts) ){
      /* wait for character or timeout */
      eln$tty_read(&sts, &virtual_serial_line_port, 1, &received_char,
                   &nbr_of_bytes_read, 2, NULL, &timeout_ack,
                   NULL, NULL, NULL, NULL);
    }
    if ( EVEN(sts) || (received_char != DLE) || (sts == ELN$_TIMEOUT) )
      sts = FALSE;
  }

/*************************************************************************/
/**  Check final status.                                                **/
/*************************************************************************/
  if ( EVEN(sts) ){
    /* The send procedure has failed */
    eln$tty_write(NULL, &virtual_serial_line_port, 1, &snak,
                  &nbr_of_bytes_written, NULL, NULL);

    errh_CErrLog(REM__SNDFAIL3964R);
  }
  return(sts);
}

/*************************************************************************
**************************************************************************
*
* Namn : Receive
*
* Typ  : unsigned int
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Waits for STX on serial line
*
**************************************************************************
**************************************************************************/

static unsigned int Receive()
{
  static int   			sts;
  static int   			nbr_of_bytes_read = 0;
  static int          		nbr_of_bytes_written = 0;
  unsigned char         	received_char = NUL;
  static unsigned char  	snak[2] = {NAK, NUL};
  static int                    opt=2;
  static DDA$BREAK_MASK         code={0,0,0,0,0,0,0,0};
  static DDA$_EXTENDED_STATUS_INFO stsblk;
  static int			error_logged = 0;

  eln$tty_read(&sts, &virtual_serial_line_port, 1,
               &received_char, &nbr_of_bytes_read, opt, NULL,
               &timeout_cycle, 1, stsblk, NULL, NULL);

  if (sts == ELN$_SUCCESS){
    if ( error_logged) {
      errh_CErrLog(REM__RCVSUCC3964R);
      error_logged = 0;
    }
    if (received_char == STX){
      sts = ReceiveHandler();
    }
    else {
      /* We don't understand, Send NAK unless NAK is received */
      if (received_char != NAK)
        eln$tty_write(NULL, &virtual_serial_line_port, 1, &snak,
                      &nbr_of_bytes_written, NULL, NULL);
    }
  }

  else if (sts != ELN$_TIMEOUT){
    /* Wait a full cycle time to avoid loop */
    if ( !error_logged) {
      errh_CErrLog(REM__RCVFAIL3964R);
      error_logged = 1;
    }
    remnode->objp->ErrTransCount++;
    eln$tty_write(NULL, &virtual_serial_line_port, 1, &snak,
                  &nbr_of_bytes_written, NULL, NULL);
    ker$wait_any(NULL, NULL, &timeout_cycle);
  }

  return(1);
}


/*************************************************************************
**************************************************************************
*
* Namn : ReceiveHandler
*
* Typ  : unsigned int
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Invoked when a telegram is received.
*
**************************************************************************
**************************************************************************/

static unsigned int ReceiveHandler()
{
  unsigned int          sts, i;
  unsigned int          nbr_of_bytes_written = 0;
  unsigned int          nbr_of_bytes_read = 0;
  unsigned int          data_size = 0;
  unsigned int          common_offset = 0;
  unsigned int          length = 0;
  char                  search_remtrans = FALSE;
  char                  terminate = FALSE;
  char                  name[7];
  unsigned char         BCC = '\0';
  unsigned char         char_buffer, BCC_checksum;
  unsigned char         received_char = '\0';
  unsigned char         ReceiveBuffer[MAX_SIZE_TELEGRAM];
  unsigned char         *ReceiveBuffer_ptr;
  unsigned char         sstx[2] = {STX, '\0'};
  unsigned char         sdle[2] = {DLE, '\0'};
  unsigned char         snak[2] = {NAK, '\0'};
  remtrans_item         *remtrans;


/*************************************************************************/
/** We have received STX earlier, send DLE to responde                  **/
/*************************************************************************/
  eln$tty_write(&sts, &virtual_serial_line_port, 1, &sdle,
                &nbr_of_bytes_written, NULL, NULL);
  if ( EVEN(sts) )
    return(FALSE);

/*************************************************************************/
/**    Store the received telegram temporary                            **/
/*************************************************************************/
  eln$tty_read(&sts, &virtual_serial_line_port, 1, &received_char,
               &nbr_of_bytes_read, 2, NULL, &timeout_char,
               NULL, NULL, NULL, NULL);
  if ( EVEN(sts) || (sts == ELN$_TIMEOUT) )
    return(FALSE);

  ReceiveBuffer[0] = char_buffer = received_char;
  BCC ^= received_char;

  for ( i=1 ; (terminate==FALSE)&&(i<MAX_SIZE_TELEGRAM) ; i++ )
  {
    eln$tty_read(&sts, &virtual_serial_line_port, 1, &received_char,
                 &nbr_of_bytes_read, 2, NULL, &timeout_char,
                 NULL, NULL, NULL, NULL);
    if ( EVEN(sts) || (sts == ELN$_TIMEOUT) )
      return(FALSE);

    if ( (char_buffer == DLE) && (received_char == ETX) ){
      /* End of message. Read checksum and terminate. */
      ReceiveBuffer[i] = received_char;
      BCC ^= received_char;
      eln$tty_read(&sts, &virtual_serial_line_port, 1, &received_char,
                   &nbr_of_bytes_read, 2, NULL, &timeout_char,
                   NULL, NULL, NULL, NULL);
      if ( EVEN(sts) || (sts == ELN$_TIMEOUT) )
        return(FALSE);

      BCC_checksum = received_char;
      terminate = TRUE;
    }
    else{
      /* Store one more received character in the buffer */
      BCC ^= received_char;
      if ( (received_char != DLE) ||
           ((received_char == DLE) && (char_buffer != DLE)) ){
        ReceiveBuffer[i] = char_buffer = received_char;
      }
      else{
        /* This is a duplicated DLE character. Throw away. */
        i--;
        char_buffer = '\0';
      }
    }
  }    /* endfor */

/*************************************************************************/
/**  A complete message is received. Check BCC.                         **/
/*************************************************************************/
  if ( BCC == BCC_checksum ){
    eln$tty_write(&sts, &virtual_serial_line_port, 1, &sdle,
                  &nbr_of_bytes_written, NULL, NULL);
    if ( EVEN(sts) )
      return(FALSE);
  }
  else{
    /* Checksum in this telegram is wrong */
    return(FALSE);
  }

/*************************************************************************/
/**  Find out if the received message is a array of io updates.         **/
/**  Treat the message and exit.                                        **/
/*************************************************************************/
  {
    io_buffer_vnet *io_buf = (io_buffer_vnet *) &ReceiveBuffer;

    if ( io_buf->io_name[0] == remnode->objp->Address[0] &&
         io_buf->io_name[1] == remnode->objp->Address[1]    ){
      data_size = io_buf->length * 2;     /* Converted to bytes */
      sts = RemIO_Receive_3964R(remnode,
                                &io_buf->data,
                                data_size-NET_HEADER_SIZE_IO);
      return(sts);
    }
  }

/*************************************************************************/
/**  Find out if the received message is a array of common data         **/
/**  Search for the RemTrans object that is the target.                 **/
/*************************************************************************/
  {
    char type_name[4];
    common_buffer_vnet *c_buf = (common_buffer_vnet *) &ReceiveBuffer;

    R50ToAscii(&c_buf->common_name, &name);
    for ( i=0 ; i<4 ; i++ )
    {
      type_name[i] = name[i+3];
    }
    if ( (strncmp(type_name, "COM", 3)) == 0 ){
      search_remtrans = true;
      remtrans = remnode->remtrans;
      while(remtrans && search_remtrans)
      {
        if ( remtrans->objp->Address[0] == c_buf->common_name[0] &&
             remtrans->objp->Address[1] == c_buf->common_name[1]    )
          search_remtrans = false;
        if ( search_remtrans )
          remtrans = (remtrans_item *) remtrans->next;
      }   /* endwhile */

/*************************************************************************/
/**  Treat the common data update message and exit.                     **/
/*************************************************************************/
      if ( !search_remtrans ){
        data_size = c_buf->length * 2;     /* Converted to bytes */
        common_offset = c_buf->offset;

        if ( data_size > remtrans->objp->MaxLength ){
          remtrans->objp->ErrCount++;
          remtrans->objp->LastSts = STATUS_LENGTH;
          return(FALSE);
        }
        else{
          memcpy(&remtrans->datap[common_offset], c_buf, data_size);
	  clock_gettime( CLOCK_REALTIME, &remtrans->objp->TransTime);
          remtrans->objp->TransCount++;
          remtrans->objp->DataValid = TRUE;
          remtrans->objp->LastSts = STATUS_OK;
          remtrans->objp->DataLength = data_size;
        }
        return(TRUE);
      }
    }
    else{

/*************************************************************************/
/**   This message contains a ordinary remtrans                         **/
/**   Search for the RemTrans object that is the target.                **/
/*************************************************************************/
      search_remtrans = true;
      remtrans = remnode->remtrans;
      while(remtrans && search_remtrans)
      {
        if ( remtrans->objp->Address[0] == c_buf->common_name[0] &&
             remtrans->objp->Address[1] == c_buf->common_name[1]    )
          search_remtrans = false;
        if ( search_remtrans )
          remtrans = (remtrans_item *) remtrans->next;
      }   /* endwhile */

/*************************************************************************/
/**  Treat the remtrans message and exit.                               **/
/*************************************************************************/
      if ( !search_remtrans ){
        data_size = c_buf->length * 2;     /* Converted to bytes */
        sts = RemTrans_Receive(remtrans,
                               &ReceiveBuffer,
                               data_size);
        if ( EVEN(sts) ){
          remtrans->objp->ErrCount++;
          return (FALSE);
        }
      }
    }

    if (search_remtrans){
      /* No corresponding RemTrans object found */
      remnode->objp->ErrTransCount++;
      return (FALSE);
    }
  }
  return (TRUE);
}


#ifdef OS_ELN
/*************************************************************************
**************************************************************************
*
* Namn : WaitFor_PLC_Change
*
* Typ  : int
*
* Typ           Parameter              IOGF     Beskrivning
*
* Beskrivning : Subprocess  waits for message from REMOTEHANDLER on job_port,
*               that will kill transport when switch is done.
*
**************************************************************************
**************************************************************************/

int WaitFor_PLC_Change()
{
  pwr_tStatus sts;
  MESSAGE msg_id;
  char *mess;
  int size;

  while (TRUE) {
    ker$wait_any(&sts, NULL, NULL, &job_port);
    ker$receive(&sts, &msg_id, &mess, &size, &job_port, NULL, NULL);

    if (*mess == 1){
      /* Hot switch init. Do nothing but delete the message */
      ker$delete(&sts, msg_id);
    }
    else if (*mess == 2){
      /* Switch is done. Execute harakiri! */
      ker$delete(&sts, msg_id); /* Delete message */
      exith();
      ker$delete(&sts, my_proc);
    }
    else {
      printf("Fel telegram PLC-byte %%x%x\n",*mess);
      ker$delete(&sts, msg_id);
    }
  }
}
#endif


/*
 * Main routine
 */

main(int argc, char *argv[])
{
  unsigned int        sts, i;             /* Status from function calls etc. */
  pwr_tObjid          pwr_node;           /* Own Pwr node */
  pwr_sClass_PlcProcess *pwr_nodep;         /* Ref to own Pwr node */
  pwr_tTime           OriginChgTime;   /* LastChgTime at start */
  pwr_tClassId        remnode_class;      /* V�r remnod's class */
  char                first;              /* Send Hello first time */
  pssupd_buffer_vnet  buff;               /* Buffer for 'Hello' */
  pssupd_order_header *header;            /* Header for Hello */

#ifdef OS_ELN
  PROCESS id_p;
  static struct dsc$descriptor_s name_desc = {0, DSC$K_DTYPE_T,DSC$K_CLASS_S,0};
  NAME name_id;

  /* Get process-id to be able to kill myself.
     Create subprocess to kill or to signal switch. */

  ker$job_port(&sts, &job_port);
  ker$current_process(&sts, &my_proc);

  /* Create a name for my own process (first process in job) */

  name_desc.dsc$a_pointer = argv[3];
  name_desc.dsc$w_length = strlen(argv[3]);
  ker$create_name(&sts, &name_id, &name_desc, my_proc, NULL);

  ker$create_process( &sts, &id_p, WaitFor_PLC_Change,NULL);
#endif

  DeclareExitHandler();

  sts = gdh_Init("rs_remote_3964r_vnet");
  if (EVEN(sts)) exit(sts);

  /* H�mta remnodens objid fr�n argumentvektorn */
  if (argc >= 3)
  {
    sts = cdh_StringToObjid( argv[2], &remnode->objid);
    if (EVEN(sts)) exit(sts);
  }
  else
    remnode->objid = pwr_cNObjid;

  /* Kontrollera att objektet verkligen �r en remnod */
  sts = gdh_GetObjectClass(remnode->objid, &remnode_class);
  if (EVEN(sts)) exit(sts);
  if (remnode_class != pwr_cClass_RemNode) exit(0);

  /* H�mta en pekare till remnoden */
  sts = gdh_ObjidToPointer(remnode->objid, (pwr_tAddress *) &remnode->objp);
  if (EVEN(sts)) exit(sts);

  /* Kontrollera att det �r r�tt transport */
  if (remnode->objp->TransportType != REMNODE_TRANSPORT_3964R_VNET) exit(0);

  /* Get pointer to $Node-object */
  sts = gdh_GetClassList( pwr_cClass_PlcProcess, &pwr_node);
  sts = gdh_ObjidToPointer(pwr_node, (pwr_tAddress *) &pwr_nodep);

  /* Initialize remtrans objects */
  
  sts = RemTrans_Init(remnode, 0);
  if (EVEN(sts)) exit(sts);

  /* Initialize remote I/O  */
  sts = RemIO_Init(remnode);
  if (EVEN(sts)) exit(sts);

  /* Initialize device */
  sts = InitNet();
  if (EVEN(sts)) exit(sts);

  /* Initialize timers */
  sts = InitTimers();
  if (EVEN(sts)) exit(sts);

  remnode->Time_since_poll = 0;
  remnode->Time_since_IO   = 0;
  remnode->Time_since_send = 0;


  /* Loop forever */

  first = TRUE;
  while (!doomsday)
  {
    sts = Receive();

    /* Om LastChgTime �r �ndrad (PLC-programbyte), avsluta */
/*
    if (memcmp(&OriginChgTime, &pwr_nodep->LastChgTime,
               sizeof(pwr_tTime)) != 0)
      exit(0);
*/
    if (first && remnode->objp->Poll) {
      /* Send Hello to subsystem if we have poll */
      header = &buff.data;
      header->type = PSS_Switch_Done;
      header->size = 0;
      header->signal = 0;
      buff.no_of_updates = 1;
      buff.length = (sizeof(pssupd_buffer_vnet) -
      MAX_ORDER_BUFFERSIZE_VNET + sizeof(pssupd_order_header) + 1) / 2;
      send_pollbuff(remnode, &buff);
    }

    remnode->Time_since_poll += remnode->objp->CycleTime;
    remnode->Time_since_IO   += remnode->objp->CycleTime;
    remnode->Time_since_send += remnode->objp->CycleTime;

    sts = RemTrans_Cyclic(remnode, &remnode_send);

    if (((!remnode->objp->IOStallFlag || EVEN(remnode->objp->IOStallAction)) &&
         (remnode->Time_since_poll >= remnode->objp->IOCycleTime)) ||
         (remnode->objp->IOStallFlag && ODD(remnode->objp->IOStallAction) &&
         (remnode->Time_since_poll >= remnode->objp->ErrTime) &&
         (remnode->Time_since_poll >= remnode->objp->IOCycleTime)))
    {
      sts = RemIO_Cyclic_3964R(remnode, &send_pollbuff);
      remnode->Time_since_poll = 0.0;
    }

    if ((remnode->Time_since_IO >= remnode->objp->IOStallTime) &&
        (remnode->objp->IOStallTime > 0))
      sts = RemIO_Stall(remnode);

    first = FALSE;
  }
}
