/* 
 * Proview   $Id: rs_remote_serial.c,v 1.2 2006-04-24 13:22:24 claes Exp $
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
*                       S E R I A L
*                       ===========
**************************************************************************
*
* Filename:             remote_serial.c unified serial driver for lynx/linux
*
*                       Date    Pgm.    Read.   Remark
* Modified              980610	CJu	-	-
* Modified              010426	ulflj	-	for lynx
*			010814  ulflj	-	fixed for pwr 3.3a
*			020530	ulflj	-	modified serial parameter read
*						in lynx version
*			021219	ulflj	-	fixed tv_timeout in recive()
*			040528	CJu	-	v4.0.0
*
* Description:          Implements remote transport process for a general
*			serial communications able to read and write ASCII
*			messages. Termination sequence for read is 
*			configured in Remnode object. 
*			
*
**************************************************************************
**************************************************************************/

/*_Include files_________________________________________________________*/

/*LynxOS system includes*/

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
//#include <ioctl.h>

/*PWR includes*/

#include "pwr_class.h"
#include "pwr_systemclasses.h"
#include "remote_mq.h"
#include "co_cdh.h"
#include "rt_gdh.h"
#include "rt_aproc.h"
#include "rt_pwr_msg.h"
#include "pwr_baseclasses.h"
#include "pwr_remoteclasses.h"
#include "remote.h"
#include "remote_utils.h"
#include "remote_remtrans_utils.h"

#include "pwr.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "rt_plc_utl.h"
#include "rt_errh.h"
#include "co_time.h"

/*_variables_______________________________________________________________*/

remnode_item rn;
pwr_sClass_RemnodeSerial *rn_serial;
int ser_fd;				/* file domininator for serial port */
unsigned char debug=0;			/* 1 if debug mode activated */

float time_since_scan;
float time_since_rcv;

void load_timeval(struct timeval *tv, float t)
{
  tv->tv_sec = t;
  tv->tv_usec = (t-(float)tv->tv_sec) * 1000000;
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
  int          sts;

/*************************************************************************/
/**    Execute the send procedure                                       **/
/*************************************************************************/
  //printf("sending\n");
  //printf("telegram:%s,%i\n",buf,buffer_size);

  sts=write(ser_fd, buf, buffer_size); //write returnerar antalet skrivna tecken
  if(sts != buffer_size)
  {
    errh_Error("S�ndfel, %d", sts);
    return(STATUS_FELSEND);  //j�mna returns vvisar p� fel i VMS
  }
  else
    return (STATUS_OK);
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
* Beskrivning : Waits for message on serial line
*
**************************************************************************
**************************************************************************/

static unsigned int Receive()
{
  int sts;
  int nbr_of_bytes_read = 0;
  unsigned char telegram[512];
  char search_remtrans = FALSE;
  remtrans_item *remtrans;
 
  fd_set read_fd;
  struct timeval tv;

  load_timeval(&tv, rn_serial->ScanTime);
  
  nbr_of_bytes_read = 0;
  telegram[nbr_of_bytes_read] = 0;
  sts = 0;
  
  while (nbr_of_bytes_read < sizeof(telegram)-2)
  {
    // Om vi �r under mottagning, v�nta tills l�stimeout
    if (nbr_of_bytes_read > 0) load_timeval(&tv, rn_serial->ReadTimeout);
    FD_ZERO(&read_fd);
    FD_SET(ser_fd, &read_fd);
    sts = select(ser_fd+1, &read_fd, NULL, NULL, &tv);
    if (sts > 0) { // Tecken finns att l�sa
      sts = read(ser_fd, &telegram[nbr_of_bytes_read], 1);
      if(sts > 0) {
        nbr_of_bytes_read++;
        if (telegram[nbr_of_bytes_read-1]==rn_serial->TermChar[0] ||
            telegram[nbr_of_bytes_read-1]==rn_serial->TermChar[1] ||
            telegram[nbr_of_bytes_read-1]==rn_serial->TermChar[2] ||
            telegram[nbr_of_bytes_read-1]==rn_serial->TermChar[3] ||
            telegram[nbr_of_bytes_read-1]==rn_serial->TermChar[4] ||
            telegram[nbr_of_bytes_read-1]==rn_serial->TermChar[5] ||
            telegram[nbr_of_bytes_read-1]==rn_serial->TermChar[6] ||
            telegram[nbr_of_bytes_read-1]==rn_serial->TermChar[7] )
          break;
      }
      else {  // L�sfel
        rn_serial->ErrCount++;        
        break;
      } 
    }
    else {  // Timeout
      break;
    }      
  }
   
  if (nbr_of_bytes_read > 0)  // Vi tog emot ett eller flera tecken
  {
    telegram[nbr_of_bytes_read] = 0;
    if (debug)
    {
      printf("Vi tog emot en trans med terminering 0x%x\n", telegram[nbr_of_bytes_read-1]);
      printf("Telegramet var:%s\n", telegram);
    }
    
    search_remtrans = true;
    remtrans = rn.remtrans;

    while (remtrans && search_remtrans)
    {
      if (remtrans->objp->Direction == REMTRANS_IN )
         search_remtrans = false;
      if ( search_remtrans ) remtrans = (remtrans_item *) remtrans->next;
    }   /* endwhile */

    if ( !search_remtrans )
    {
      sts = RemTrans_Receive(remtrans, (char *)telegram, nbr_of_bytes_read);
      if ( EVEN(sts) )
      {
        remtrans->objp->ErrCount++;
        return (FALSE);
      }
    }

    if (search_remtrans)
    {
      /* No corresponding RemTrans object found */
      rn_serial->ErrCount++;
      return (FALSE);
    }
  }

  return(1);
}

/***************************************************
 *****************   Main routine   ****************
 ***************************************************/

int main(int argc, char *argv[])
{
  unsigned int sts;			/* Status from function calls etc. */
  unsigned char id[32];
  unsigned char pname[32];
  remtrans_item *remtrans;
  int i;

  /* Read arg number 2, should be id for this instance */

  if (argc >= 2)
    strcpy((char *)id, argv[1]);
  else
    strcpy((char *)id, "0");
    
  /* Build process name with id */

  sprintf((char *) pname, "rs_remser_%s", id);

  /* Init of errh */

  errh_Init((char *) pname, errh_eAnix_remote);
  errh_SetStatus(PWR__SRVSTARTUP);

  /* Set debug mode if arg number 4 is "debug" (started manually) */

  debug = 0;
  if (argc >= 4) {
    if (!strncmp(argv[3],"debug",5)) debug = 1;
  }

  if (debug) printf("debugmode valt\n");

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

  /* Get pointer to RemnodeSerial object and store locally */
  
  sts = gdh_ObjidToPointer(rn.objid, (pwr_tAddress *) &rn_serial);
  if ( EVEN(sts)) {
    errh_Error("cdh_ObjidToPointer, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }

  /* Initialize some internal data and make standard remtrans init */

  rn.next = NULL;
  rn.local = NULL;		// We dont use local structure since we only have one remnode
  rn.retransmit_time = 10.0;	// Not used, but initialize anyway
  rn_serial->ErrCount = 0;

  sts = RemTrans_Init(&rn);

  if ( EVEN(sts)) {
    errh_Error("RemTrans_Init, %m", sts);
    errh_SetStatus(PWR__SRVTERM);
    exit(sts);
  }

  time_since_scan = 0;
  time_since_rcv = 0;

  /* Store remtrans objects objid in remnode_serial object */
  remtrans = rn.remtrans;
  i = 0;
  while(remtrans) {
    rn_serial->RemTransObjects[i++] = remtrans->objid;
    if ( i >= (int)(sizeof(rn_serial->RemTransObjects)/sizeof(rn_serial->RemTransObjects[0])))
      break;
    remtrans = (remtrans_item *) remtrans->next;
  }

  /* Initialize device */

  ser_fd = RemUtils_InitSerialDev(rn_serial->DevName, 
  				  rn_serial->Speed, 
				  rn_serial->DataBits,
				  rn_serial->StopBits,
				  rn_serial->Parity);
				  
  if (!ser_fd) {
    errh_Error("InitDev, %d", ser_fd);
    errh_SetStatus(PWR__SRVTERM);
    exit(0);
  }    

  /* Loop forever */

  while (1)
  {
    if (rn_serial->Disable == 1) {
      errh_Fatal("Disabled, exiting");
      errh_SetStatus(PWR__SRVTERM);
      exit(0);
    }   
    sts = Receive();
    sts = RemTrans_Cyclic(&rn, &remnode_send);
  }
}
