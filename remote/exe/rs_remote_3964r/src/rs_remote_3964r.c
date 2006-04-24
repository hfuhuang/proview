/* 
 * Proview   $Id: rs_remote_3964r.c,v 1.2 2006-04-24 13:22:23 claes Exp $
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
* Filename:             rs_remote_3964r.c
*
*                       Date    Pgm.    Read.   Remark
* Modified              010401  ulflj   -       for lynx
*			010815  ulflj		fixed for pwr 3.3a
*			020530	ulflj		modified serial parameter
*						read in lynx version
*			030814	ulflj		Linux version improved
*			030830	ulflj		fixed timeouts for DLE-answer/characters
*			031118	ulflj		set longer char timeout to get magnemag marker to work (longer then specs)
*
* Description:          Implements remote transport process 3964R.
*
**************************************************************************
**************************************************************************/

/*_Include files_________________________________________________________*/

/*System includes*/

#include <time.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
//#include <uio.h>
#include <termios.h>
#include <termio.h>
#include <sgtty.h>
#include <sys/ioctl.h>

/*PWR includes*/

#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "pwr_remoteclasses.h"
#include "remote_mq.h"
#include "co_cdh.h"
#include "rt_gdh.h"
#include "rt_aproc.h"
#include "rt_pwr_msg.h"
#include "remote.h"
#include "remote_utils.h"
#include "remote_remtrans_utils.h"
#include "remote_remio_utils.h"


#include "pwr.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "rt_plc_utl.h"
#include "rt_errh.h"
#include "co_time.h"

/*_Function prototypes___________________________________________________*/

void 			send_pollbuff(remnode_item *remnode, pssupd_buffer_vnet *buf);
static unsigned int     remnode_send(remnode_item        *remnode,
                                     pwr_sClass_RemTrans *remtrans,
                                     char                *buf,
                                     int                 buffer_size);
static unsigned int     send_it(char                *buf,
                                int                 buffer_size);
static unsigned int     Receive();
static unsigned int     ReceiveHandler();

/*_defines_________________________________________________________________*/

#define NUMBER_OF_STOP_CHAR 3
#define NET_HEADER_SIZE_IO 6
#define NET_HEADER_SIZE_COMMON 8
#define MAX_SIZE_TELEGRAM 2048

//#define TIMEOUT_REC_ANSWER_SEC 2
//#define TIMEOUT_REC_ANSWER_USEC 0

//#define TIMEOUT_REC_CHAR_SEC 0
//#define TIMEOUT_REC_CHAR_USEC 900000

//#define TIMEOUT_SND_ANSWER_SEC 2
//#define TIMEOUT_SND_ANSWER_USEC 0

//#define TIMEOUT_SND_CHAR_SEC 0
//#define TIMEOUT_SND_CHAR_USEC 900000

#define NUL 0
#define STX 2
#define ETX 3
#define DLE 16
#define NAK 21

#define DLE_BITMASK 0x10000

/*_variables_______________________________________________________________*/

remnode_item rn;
pwr_sClass_Remnode3964R *rn_3964R;
int ser_fd;				/* file domininator for serial port */
unsigned char debug=0;			/* 1 if debug mode activated */

float time_since_poll;
float time_since_io;
float time_since_scan;

int stall_action = 1;
int use_remote_io;

void load_timeval(struct timeval *tv, float t)
{
  tv->tv_sec = t;
  tv->tv_usec = (t-(float)tv->tv_sec) * 1000000;
}

short poll_id[2];

/*_variables_______________________________________________________________*/

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

void send_pollbuff(remnode_item *remnode, pssupd_buffer_vnet *buf)
{
  unsigned int sts, buf_size;

  /* Fill in remaining data in poll telegram */
  RemUtils_AsciiToR50("PSSUPD", (short *) &buf->receive_task);
  buf->common_name[0] = poll_id[0];
  buf->common_name[1] = poll_id[1];

  buf_size = buf->length * 2; /*  Convert to bytes  */
  sts = send_it((char*)buf, buf_size);

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
* Beskrivning : Sends a buffer
*
**************************************************************************
**************************************************************************/

static unsigned int send_it(char *buf, int buffer_size)
{
  int          		sts, i;
  unsigned int          size_of_telegram;
  unsigned int          number_of_DLE = 0;
  unsigned char         ch;
  unsigned char         BCC = DLE ^ ETX;
  unsigned char         *restore_buf_ptr = (unsigned char *)buf;
  unsigned char         telegram[MAX_SIZE_TELEGRAM];
  unsigned char		buff;
  static unsigned char  sstx[2] = {STX, '\0'};
  //static unsigned char  sdle[2] = {DLE, '\0'};
  static unsigned char  snak[2] = {NAK, '\0'};

  fd_set read_fd;
  struct timeval tv;

/*************************************************************************/
/** Count DLE characters and calculate the size of the telegram.        **/
/*************************************************************************/
  for ( i=0 ; i < buffer_size ; i++ )
  {
    if ( *buf++ == DLE )
      number_of_DLE += 1;
  }
  size_of_telegram = buffer_size + number_of_DLE + NUMBER_OF_STOP_CHAR;
  
/*************************************************************************/
/**   Fill up telegram with contents of message and calculate BCC       **/
/*************************************************************************/
  buf = (char *)restore_buf_ptr;
  for ( i=0 ; i<(buffer_size + number_of_DLE) ; i++ )
  {
    ch = telegram[i] = *buf++;
    BCC ^= ch;
    if ( ch == DLE )
    {
      telegram[++i] = DLE;
      BCC ^= ch;
    }
  }
  telegram[i++] = DLE;
  telegram[i++] = ETX;
  telegram[i]   = BCC;

/*************************************************************************/
/**    Execute the send procedure      **/
/*************************************************************************/

/**** set up timeout ****/

//  tv.tv_sec = TIMEOUT_SND_ANSWER_SEC;
//  tv.tv_usec = TIMEOUT_SND_ANSWER_USEC;
  load_timeval(&tv, rn_3964R->AckTimeout);

  FD_ZERO(&read_fd);
  FD_SET(ser_fd, &read_fd);

  sts=TRUE;
  
  write(ser_fd, sstx, 1); 	/*send stx and wait for answer*/
    
  select(ser_fd+1, &read_fd, NULL, NULL, &tv);	/*wait for char or timeout*/
  sts=read(ser_fd,&buff,1);			/*read port*/
  
  if(sts < 1)				/*if timeout*/
  {
    errh_Error("Remtrans 3964R, s�ndning, inget svar fr�n mottagaren, f�rs�ker igen");
    write(ser_fd, sstx, 1);	/*try once more*/

    FD_ZERO(&read_fd);
    FD_SET(ser_fd, &read_fd);
    load_timeval(&tv, rn_3964R->AckTimeout);
    select(ser_fd+1, &read_fd, NULL, NULL, &tv);
    sts=read(ser_fd,&buff,1);
  }
  
  if(sts > 0 && buff==DLE)			/*if DLE ok send telegram*/
  {
    write(ser_fd, telegram, size_of_telegram);

//    tv.tv_sec = TIMEOUT_SND_ANSWER_SEC;
//    tv.tv_usec = TIMEOUT_SND_ANSWER_USEC;
    load_timeval(&tv, rn_3964R->AckTimeout);
    FD_ZERO(&read_fd);
    FD_SET(ser_fd, &read_fd);
    select(ser_fd+1, &read_fd, NULL, NULL, &tv);
    sts=read(ser_fd,&buff,1); 
  }
  
  if(sts < 1 || buff!=DLE)			/*if somthing went wrong*/
  {
    write(ser_fd,snak, 1);
    if(sts<1)
      errh_Error("Remtrans 3964R s�ndning misslyckades, inget initierande DLE-tecken fr�n mottagaren");
    else if(buff!=DLE)
      errh_Error("Remtrans 3964R s�ndning misslyckades, inget avslutande DLE-tecken fr�n mottagaren");
    sts=FALSE;
  }
  return sts;				/*and return status*/
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
  unsigned char         	received_char = NUL;
  static unsigned char  	snak[2] = {NAK, NUL};
  static unsigned char  	sdle[2] = {DLE, NUL};
  //static int			error_logged = 0;

  fd_set read_fd;
  struct timeval tv;

/**** set up timeout ****/

  load_timeval(&tv, 0);		// poll
//  load_timeval(&tv, rn_3964R->ScanTime);
//  tv.tv_sec = TIMEOUT_REC_ANSWER_SEC;
//  tv.tv_usec = TIMEOUT_REC_ANSWER_USEC;

/**routine**/  

  FD_ZERO(&read_fd);
  FD_SET(ser_fd, &read_fd);
  sts=select(ser_fd+1, &read_fd, NULL, NULL, &tv);
  sts=read(ser_fd, &received_char, 1);

  if (sts > 0)
  {
    if (received_char == STX)
    {
      /* Write DLE to respond */
      if(!write(ser_fd, sdle, 1))
        return 0;
      sts = ReceiveHandler(ser_fd);
    }
    else
    {
      /* We don't understand, Send NAK unless NAK is received */
      if (received_char != NAK)
        write(ser_fd, snak, 1);
      errh_Error("3964R felaktigt meddelande i mottagning, annat starttecken �n STX (0x%x)", received_char);
      sts=0;			//felstatus
    }

    if (sts < 1)
    {
      rn_3964R->ErrCount++;
    }
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

static unsigned int ReceiveHandler(int fd)
{
  unsigned int          sts;
  //unsigned int          nbr_of_bytes_written = 0;
  //unsigned int          nbr_of_bytes_read = 0;
  unsigned int          data_size = 0;
  unsigned int		io_size = 0;
  unsigned int		common_offset = 0;
  char                  cont = TRUE;
  char			search_remtrans = FALSE;
  char                  name[7];
  unsigned char         BCC = '\0';
  unsigned char         receive_buffer[MAX_SIZE_TELEGRAM];
  unsigned char         sdle = DLE;
  unsigned char		received_char;
  remtrans_item         *remtrans;
  int 			i;

  fd_set read_fd;
  struct timeval tv;
  char type_name[4];
  common_buffer_vnet *c_buf;

  /**** set up timeout,****/

  load_timeval(&tv, rn_3964R->CharTimeout);

//  tv.tv_sec = TIMEOUT_REC_CHAR_SEC;
//  tv.tv_usec = TIMEOUT_REC_CHAR_USEC;


/*************************************************************************/
/**    Read telegram	                                                **/
/*************************************************************************/

  cont = TRUE;
  data_size = 0;
  
  while (cont)
  {
    /* Read until DLE is received */

    received_char=0;

    while(received_char != DLE)
    {
      load_timeval(&tv, rn_3964R->CharTimeout);
      FD_ZERO(&read_fd);
      FD_SET(fd, &read_fd);
      select(fd+1, &read_fd, NULL, NULL, &tv);
      if (read(fd, &received_char, 1) > 0) { //om det inte var timeout
        // Prevent writing oob
        if (data_size > MAX_SIZE_TELEGRAM - 10) return (FALSE);
        receive_buffer[data_size++]=received_char;
      }
      else				//timeout g� tillbaka
      {
        errh_Error("3964R mottagning, character timeout");
        return(FALSE);  
      }
    }  

    /* Read one more */
    load_timeval(&tv, rn_3964R->CharTimeout);
    FD_ZERO(&read_fd);
    FD_SET(fd, &read_fd);
    select(fd+1, &read_fd, NULL, NULL, &tv);
    if (read(fd, &receive_buffer[data_size], 1) < 1) {
      errh_Error("3964R mottagning, character timeout");
      return(FALSE);  
    }
 
    if (receive_buffer[data_size] == ETX)
    {
      data_size++;

      /* Read one more, should be checksum */
      load_timeval(&tv, rn_3964R->CharTimeout);
      FD_ZERO(&read_fd);
      FD_SET(fd, &read_fd);
      select(fd+1, &read_fd, NULL, NULL, &tv);
      if (read(fd, &receive_buffer[data_size], 1) < 1) {
        errh_Error("3964R mottagning, character timeout");
        return(FALSE);  
      }
  
      data_size++;
      cont = FALSE;
    }
    else
      if (receive_buffer[data_size] != DLE ) data_size++;
  }

/*************************************************************************/
/**  A complete message is received. Check BCC.                         **/
/*************************************************************************/
  BCC = DLE ^ ETX;

  for (i=0; i<data_size-3; i++)
    if (receive_buffer[i] != DLE)
      BCC ^= receive_buffer[i];
  


  if ( BCC == receive_buffer[data_size-1] ) {
    if(!write(fd,&sdle, 1)) return(FALSE);
  }
  else
  {
    /* Checksum in this telegram is wrong */
    errh_Error("3964R mottagning, felaktig checksumma, %d, %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", 
    	data_size, receive_buffer[0], receive_buffer[1], receive_buffer[2], receive_buffer[3], receive_buffer[4], receive_buffer[5],
    	receive_buffer[6], receive_buffer[7], receive_buffer[8], receive_buffer[9], receive_buffer[10], receive_buffer[11]);
    if (debug) printf("  Checksum error\n");
       
    if (use_remote_io) {
      if(!write(fd,&sdle, 1)) return(FALSE);
    }
    else
      return(FALSE);
  }


/*************************************************************************/
/**  Find out if the received message is a array of io updates.         **/
/**  Treat the message and exit.                                        **/
/*************************************************************************/
  {
    io_buffer_vnet *io_buf = (io_buffer_vnet *) &receive_buffer;
    RemUtils_R50ToAscii((unsigned short *) &io_buf->io_name, (char *) &name);

    if (strstr(name, "PSS")) {
      io_size = io_buf->length * 2;     /* Converted to bytes */
      sts = RemIO_Receive_3964R(&rn,
                                (unsigned char *) &io_buf->data,
                                io_size-NET_HEADER_SIZE_IO);
      if (debug) printf("  Receiving I/O area\n");

      time_since_io = 0;
      rn_3964R->LinkUp = 1;

      return(sts);
    }
  }

/*************************************************************************/
/**  Find out if the received message is a array of common data         **/
/**  Search for the RemTrans object that is the target.                 **/
/*************************************************************************/
  c_buf = (common_buffer_vnet *) &receive_buffer;

  RemUtils_R50ToAscii((unsigned short *) &c_buf->common_name, (char *) &name);
  for ( i=0 ; i<4 ; i++ )
  {
    type_name[i] = name[i+3];
  }
  if ( (strncmp(type_name, "COM", 3)) == 0 ){
    search_remtrans = true;
    remtrans = rn.remtrans;
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
      io_size = c_buf->length * 2;     /* Converted to bytes */
      common_offset = c_buf->offset;

      if (io_size > remtrans->objp->MaxLength ){
        remtrans->objp->ErrCount++;
        remtrans->objp->LastSts = STATUS_LENGTH;
        return(FALSE);
      }
      else { 
        memcpy(&remtrans->datap[common_offset], c_buf, io_size);
	clock_gettime( CLOCK_REALTIME, &remtrans->objp->TransTime);
        remtrans->objp->TransCount++;
        remtrans->objp->DataValid = TRUE;
        remtrans->objp->LastSts = STATUS_OK;
        remtrans->objp->DataLength = data_size;
      }
      return(TRUE);
    }
  }
  
/*************************************************************************/
/**   This message contains a ordinary remtrans                         **/
/**   Search for the RemTrans object that is the target.                **/
/**   If remote I/O is not used, the message is stored in the first     **/
/**   found (ingoing) remtrans object. If we use remote I/O we check for**/
/**   matching message header                                           **/
/*************************************************************************/
  search_remtrans = true;
  remtrans = rn.remtrans;
  while(remtrans && search_remtrans)
  {
    if (remtrans->objp->Direction == REMTRANS_IN) {
      if (!use_remote_io) {
        search_remtrans = false;
      }
      else {
        if (remtrans->objp->Address[0] == c_buf->common_name[0] &&
	    remtrans->objp->Address[1] == c_buf->common_name[1])
              search_remtrans = false;
      }
    }
    if ( search_remtrans ) remtrans = (remtrans_item *) remtrans->next;
  }
 
/*************************************************************************/
/**  Treat the remtrans message and exit.                               **/
/*************************************************************************/
  if (!search_remtrans)
  {
    sts = RemTrans_Receive(remtrans, (char *)receive_buffer, data_size-3);
    if ( EVEN(sts) )
    {
      remtrans->objp->ErrCount++;
      return (FALSE);
    }
  }
  else
  {
    /* No remtrans */
    errh_Error("Remtrans 3964R no remtrans for this message");
    rn_3964R->ErrCount++;
    return (FALSE);
  }
  return (TRUE);
}


/***************************************************
 ******        Main routine        *****************
 ***************************************************/

int main(int argc, char *argv[]) /*argv[2]=remnode name*/
{
  unsigned int sts;			/* Status from function calls etc. */
  unsigned char id[32];
  unsigned char pname[32];
  remtrans_item *remtrans;
  int i;
  char first;
  pssupd_buffer_vnet buff;		/* Buffer for 'hello' */
  pssupd_order_header *header;		/* Header for 'hello' */
  char name[80];

  /* Read arg number 2, should be id for this instance */

  if (argc >= 2)
    strcpy((char *) id, argv[1]);
  else
    strcpy((char *) id, "0");
    
  /* Build process name with id */

  sprintf((char *) pname, "rs_rem3964r_%s", id);

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

  /* Get pointer to Remnode3964R object and store locally */
  
  sts = gdh_ObjidToPointer(rn.objid, (pwr_tAddress *) &rn_3964R);
  if ( EVEN(sts)) {
    errh_Error("cdh_ObjidToPointer, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }
  
  /* Get name of object to use in sending poll */
  
  sts = gdh_ObjidToName(rn.objid, name, sizeof(name), cdh_mName_object);
  name[3] = 'P';
  name[4] = 'S';
  name[5] = 'S';
  name[6] = 0;
  RemUtils_AsciiToR50((char *) &name, (short *) &poll_id);
  
  if (debug) printf("%s, %d %d\n", name, poll_id[0], poll_id[1]);  
      
  /* Initialize some internal data and make standard remtrans init */

  rn.next = NULL;
  rn.local = NULL;		// We dont use local structure since we only have one remnode
  rn.retransmit_time = 10.0;	// Not used, but initialize anyway
  rn_3964R->ErrCount = 0;

  sts = RemTrans_Init(&rn);
  
  if ( EVEN(sts)) {
    errh_Error("RemTrans_Init, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }

  sts = RemIO_Init_3964R(&rn);

  if ( EVEN(sts)) {
    errh_Error("RemIO_Init, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }
  
  if (rn.remio_data == NULL) 
    use_remote_io = 0;
  else
    use_remote_io = 1;  

  time_since_poll = 0.0;
  time_since_io = 0.0;
  time_since_scan = 0.0;

  /* Store remtrans objects objid in remnode_3964R object */
  remtrans = rn.remtrans;
  i = 0;
  while(remtrans) {
    rn_3964R->RemTransObjects[i++] = remtrans->objid;
    if ( i >= (int)(sizeof(rn_3964R->RemTransObjects)/sizeof(rn_3964R->RemTransObjects[0])))
      break;
    remtrans = (remtrans_item *) remtrans->next;
  }

  /* Initialize device */

  ser_fd = RemUtils_InitSerialDev(rn_3964R->DevName, 
  				  rn_3964R->Speed, 
				  rn_3964R->DataBits,
				  rn_3964R->StopBits,
				  rn_3964R->Parity);
				  
  if (!ser_fd) {
    errh_Error("InitDev, %d", ser_fd);
    errh_SetStatus(PWR__SRVTERM);
    exit(0);
  }    

  first = TRUE;
  rn_3964R->LinkUp = 1;
  
  /* Loop forever */

  while (1)
  {

    if (rn_3964R->Disable == 1) {
      errh_Fatal("Disabled, exiting");
      errh_SetStatus(PWR__SRVTERM);
      exit(0);
    }   
    
    // Wait micro time  
    // Wait cycle time
//    timer = (int) (rn_3964R->ScanTime * 1000000.0);
    usleep(30000);
    
    Receive();
    
//    time_since_poll += rn_3964R->ScanTime;
//    time_since_io += rn_3964R->ScanTime;
    time_since_poll += 0.03;
    time_since_io += 0.03;
    time_since_scan += 0.03;

    if (first && use_remote_io) {
      /* Send Hello to subsystem if we have poll */
      header = (pssupd_order_header *) &buff.data;
      header->type = PSS_Switch_Done;
      header->size = 0;
      header->signal = 0;
      buff.no_of_updates = 1;
      buff.length = (sizeof(pssupd_buffer_vnet) -
      MAX_ORDER_BUFFERSIZE_VNET + sizeof(pssupd_order_header) + 1) / 2;
      send_pollbuff(&rn, &buff);
    }
        
//    if (debug) printf("Remtrans Cyclic\n");
    if (time_since_scan >= rn_3964R->ScanTime) {
      if (debug) printf("Remtrans Cyclic\n");
      RemTrans_Cyclic(&rn, &remnode_send);
      time_since_scan = 0.0;
    }

    if (use_remote_io) {
    
      if ((rn_3964R->LinkUp && time_since_poll >= rn_3964R->ScanTime*2.0) ||
          (!rn_3964R->LinkUp && time_since_poll >= rn_3964R->ScanTime*10.0))
      {
        if (debug) printf("RemIO Cyclic\n");
        sts = RemIO_Cyclic_3964R(&rn, &send_pollbuff);
        time_since_poll = 0.0;
      }

      if (time_since_io >= rn_3964R->LinkTimeout && rn_3964R->LinkTimeout > 0) {
        if (debug) printf("RemIO Stall\n");
        sts = RemIO_Stall_3964R(&rn, stall_action);
      }
    }

    first = FALSE;    
  }
}
