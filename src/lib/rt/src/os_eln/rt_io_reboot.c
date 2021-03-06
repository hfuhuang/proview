/* 
 * Proview   $Id: rt_io_reboot.c,v 1.2 2005-09-01 14:57:56 claes Exp $
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

/* rt_io_reboot.c 
   Reboot the system PROVIEW/R.  */

#include $vaxelnc
#include $kerneldef
#include $mutex
#include descrip
#include prdef
#include $kernelmsg
#include $elnmsg
#include stdio
#include $dda_utility

#include "rt_io_reboot.h"

static void self_reboot ();


/* Execute routine self_reboot in kernal process mode.
   Rebooting a VAX system is currently done in one  of  two  
   basic  ways. The  traditional  method  for  initiating  a
   reboot was to write a special reboot code, 0xF02, into 
   processor register  35  (the  console  transmitter data
   buffer   register).   On  the  KA620  or  the  KA630,
   rebooting  is accomplished by writing a special "cold boot" 
   into a location in  the  time of  year  (TOY)  RAM referred 
   to as the processor's "mailbox" register, and then halting
   the processor.  */

void
io_reboot (
)
{
  ker$enter_kernel_context(NULL,self_reboot,NULL);
}

/****************************************************************************
* Name		: self_reboot
* 
* Type		: void
*
* TYPE		  PARAMETER	IOGF	DESCRIPTION
*
* Description:   This routine will reboot this node in a CPU dependent manner.
*                It is executed in kernel context because it will either
*                execute an MTPR instruction, or will halt the processor,
*                both of which can only be done in kernel mode.  For the KA620
*                and KA630, we map to the processor's mailbox register in 
*                the time of year RAM, and set "cold boot on halt" flag.
*                For other VAXen, we write a special reboot code in the
*                transmitter data buffer register for the console.
****************************************************************************/

static void self_reboot (
) {
#if 0
  char		  *cpmbx;
  char		  *cpmbxaddr;
  unsigned short  cpmbxoffset;
  unsigned char	  cpmbxvalue;

  switch (ker$gb_cpu_type) {
  case PR$_SID_TYP620:
  case PR$_SID_TYP630:
    cpmbxaddr = 0x200B8000;     	/* Beginning of TOY RAM */
    cpmbxoffset = 0x1C;         	/* Offset to mailbox register */
    cpmbxvalue = 2;          		/* "Cold boot on halt" flag */
    break;

  default:
    mtpr (PR$_TXDB, 0xF02);     	/* Write the reboot flag */
    break;
  }

  ker$allocate_memory (NULL, &cpmbx, 512, NULL, cpmbxaddr);
  cpmbx += cpmbxoffset;
  *cpmbx = cpmbxvalue;
#endif
  pas$halt ();

} /* END self_reboot */

