/* 
 * Proview   $Id: wb_cmd_motif.cpp,v 1.2 2008-06-25 07:54:54 claes Exp $
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

/* wb_cmd_motif.c -- command file processing
   The main program of pwrc.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#include <X11/cursorfont.h>
#define XK_MISCELLANY
#include <X11/keysymdef.h>

#include "pwr.h"
#include "pwr_class.h"
#include "co_dcli.h"
#include "co_mrm_util.h"

#include "ge.h"
#include "rt_load.h"
#include "wb_foe_msg.h"
#include "co_dcli_input.h"

#include "flow.h"
#include "flow_ctx.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "wb_utl.h"
#include "wb_lfu.h"
#include "co_login.h"
#include "wb_wnav_motif.h"
#include "wb_wnav_item.h"
#include "wb_pal.h"
#include "wb_watt.h"
#include "wb_wnav_msg.h"
#include "wb_cmdc_motif.h"
#include "wb.h"
#include "co_msgwindow.h"

CmdMotif::CmdMotif()
{
  Widget	w;
  pwr_tStatus	sts;

  wnav = new WNavMotif( (void *)this, (Widget)0,"","",&w, ldhses, 
	(wnav_sStartMenu *)0, wnav_eWindowType_No, &sts);
  wnav->attach_volume_cb = attach_volume_cb;
  wnav->detach_volume_cb = detach_volume_cb;
  wnav->get_wbctx_cb = get_wbctx;
  wnav->save_cb = save_cb;
  wnav->revert_cb = revert_cb;
  wnav->close_cb = close_cb;

}

int main(int argc, char *argv[])
{

  pwr_tStatus	sts;
  int		i;
  char 		str[256] ;
  CmdMotif     	*cmd;
  int 		quiet = 0;
  
  cmd = new CmdMotif();

  /* If arguments, treat them as a command and then exit */
  // Open directory volume as default
  strcpy( Cmd::cmd_volume, "directory");
  Cmd::cmd_volume_p = Cmd::cmd_volume;

  str[0] = 0;
  for ( i = 1; i < argc; i++) {
    if ( argv[i][0] == '-') {
      switch ( argv[i][1]) {
      case 'h':
	Cmd::usage();
	exit(0);
      case 'a':
	// Load all volumes
	Cmd::cmd_volume_p = 0;
	break;
      case 'v':
	// Load specified volume
	if ( argc >= i) {
	  strcpy( Cmd::cmd_volume, argv[i+1]);
	  Cmd::cmd_volume_p = Cmd::cmd_volume;
	  i++;
	  continue;
	}
	else
	  cout << "Syntax error, volume is missing" << endl;
	break;
      case 'q':
	// Quiet
	quiet = 1;
	break;
      case 'i':
	// Ignore errors for dbs files not yet created
	Cmd::cmd_options = ldh_mWbOption_IgnoreDLoadError;
	MsgWindow::hide_info_messages( 1);
	break;
      default:
	cout << "Unknown argument: " << argv[i] << endl;
      }
    }
    else {
      if ( str[0] != 0)
	strcat( str, " ");
      strcat( str, argv[i]);
    }
  }

  if ( !quiet)
    cout << "\n\
Proview is free software; covered by the GNU General Public License.\n\
You can redistribute it and/or modify it under the terms of this license.\n\
\n\
Proview is distributed in the hope that it will be useful \n\
but WITHOUT ANY WARRANTY; without even the implied warranty of \n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the \n\
GNU General Public License for more details.\n\n";

  if ( str[0] != 0) {
    dcli_remove_blank( str, str);
    sts = cmd->wnav->command(str);
    if ( ODD(sts))
      return 0;
    exit(sts);
  }
  sts = dcli_input_init( &cmd->chn, &cmd->recall_buf);
  if ( EVEN(sts)) exit(sts);

  // Init input

  while ( 1 )
  {
    /* get and parse the command */

    /* get input */
    dcli_qio_set_attr( &cmd->chn);
    sts = dcli_get_input_command( &cmd->chn, "pwr> ", str, 
		sizeof(str), cmd->recall_buf);
    dcli_qio_reset( &cmd->chn);

//    sts = scanf( "%s", str);
      
    if ( strcmp( str, "") == 0)
      continue;

    dcli_remove_blank( str, str);
    sts = cmd->wnav->command(str);

  }
  dcli_input_end( &cmd->chn, cmd->recall_buf);
}
