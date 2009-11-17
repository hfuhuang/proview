/* 
 * Proview   $Id: wb_cmd_main.cpp,v 1.4 2008-10-15 06:06:14 claes Exp $
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

/* wb_cmd.c -- command file processing
   The main program of pwrc.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pwr.h"
#include "pwr_class.h"
#include "co_dcli.h"

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
#include "cow_login.h"
#include "wb_wnav.h"
#include "wb_wnav_item.h"
#include "wb_pal.h"
#include "wb_watt.h"
#include "wb_wnav_msg.h"
#include "wb_cmdc.h"
#include "wb.h"
#include "cow_msgwindow.h"

char Cmd::cmd_volume[80];
char *Cmd::cmd_volume_p = 0;
unsigned int Cmd::cmd_options = 0;

void Cmd::usage()
{
  cout << endl << endl <<
    "pwrc        Proview workbench commands" << endl << endl << 
    "Arguments:" << endl <<
    "  -a             Load all configured databases." << endl <<
    "  -v 'volume'    Load volume 'volume'." << endl <<
    "  -h             Print usage." << endl << endl <<
    "  Other arguments are treated as a command and passed to the command parser" << endl <<
    "  If a command is given as an argument, the command will be executed and the" << endl <<
    "  program is then terminated." << endl <<
    "  If no command is given, pwrc will prompt for a command." << endl << endl <<
    "Examples:" << endl <<
    "  $ pwrc -v MyVolume" << endl <<
    "  pwrc>" << endl << endl <<
    "  $ pwrc -a show volume" << endl <<
    "  directory     Attached Db  $DirectoryVolume 254.254.254.253" << endl <<
    "  MyVolume               Db  $RootVolume        0.1.99.20" << endl <<
    "  $" << endl;
}

int Cmd::get_wbctx( void *ctx, ldh_tWBContext *wbctx)
{
  Cmd	*cmd = (Cmd *) ctx;
  int	sts;

  if ( cmd->wbctx)
  {
    *wbctx = cmd->wbctx;
    sts = 1;
  }
  else
  {
    sts = ldh_OpenWB( &cmd->wbctx, cmd_volume_p, cmd_options);
    if ( ODD(sts))
      *wbctx = cmd->wbctx;
  }
  return sts;
}

int Cmd::attach_volume_cb( void *ctx,
			 pwr_tVolumeId	volid,
			 int pop)
{
  Cmd *cmd = (Cmd *) ctx;
  int	sts;
  pwr_tVolumeId	vid;
  pwr_tClassId	classid;

  if ( cmd->ldhses)
  {
//    cmd->wnav->message( 'E', "Other volume is already attached");
    return WNAV__VOLATTACHED;
  }

  if ( !cmd->wbctx)
  {
    sts = get_wbctx( (void *)cmd, &cmd->wbctx);
    if ( EVEN(sts)) return sts;
  }

  if ( volid == 0)
  {
    if ( cmd_volume_p != 0) {
      // Attach argument volume
      sts = ldh_VolumeNameToId( cmd->wbctx, cmd_volume_p, &volid);
    }
    if ( cmd_volume_p == 0 || EVEN(sts)) {
      // Attach first rootvolume, or if no rootvolume exist some other volume
      sts = ldh_GetVolumeList( cmd->wbctx, &vid);
      while ( ODD(sts)) {
	volid = vid;
	sts = ldh_GetVolumeClass( cmd->wbctx, vid, &classid);
	if ( EVEN(sts)) return sts;

	if ( classid == pwr_eClass_RootVolume)
	  break;
	sts = ldh_GetNextVolume( cmd->wbctx, vid, &vid);
      }
      if ( volid == 0) return sts;
    }
  }

  cmd->volid = volid;

  // Open ldh session
  sts = ldh_AttachVolume( cmd->wbctx, cmd->volid, &cmd->volctx);
  if ( EVEN(sts)) return sts;

  sts = ldh_OpenSession( &cmd->ldhses,
    cmd->volctx,
    ldh_eAccess_ReadWrite,
    ldh_eUtility_Pwr);
  if ( EVEN(sts)) {
    // Try read access
    sts = ldh_OpenSession( &cmd->ldhses,
			   cmd->volctx,
			   ldh_eAccess_ReadOnly,
			   ldh_eUtility_Pwr);
    if ( EVEN(sts)) return sts;
  }

  cmd->wnav->volume_attached( cmd->wbctx, cmd->ldhses, pop);

  return 1;
}

int Cmd::detach_volume()
{
  int sts;

  if ( !ldhses)
    return WNAV__NOVOLATTACHED;

  sts = ldh_CloseSession( ldhses);
  if ( EVEN(sts))
  {
    wnav->message( 'E', wnav_get_message( sts));
    return 0;
  }

  sts = ldh_DetachVolume( wbctx, volctx);
  if ( EVEN(sts))
  {
    wnav->message( 'E', wnav_get_message( sts));
    return 0;
  }
  volid = 0;
  volctx = 0;
  ldhses = 0;

  wnav->volume_detached();
  return 1;
}

int Cmd::detach_volume_cb( void *ctx)
{
  Cmd *cmd = (Cmd *) ctx;

  return cmd->detach_volume();
}

void Cmd::save_cb( void *ctx, int quiet)
{
  Cmd *cmd = (Cmd *) ctx;
  int sts;

  if ( !cmd->ldhses) {
    cmd->wnav->message( 'E', "Cmd is not attached to a volume");
    return;
  }
  sts = ldh_SaveSession( cmd->ldhses);
  if ( EVEN(sts)) {
    cmd->wnav->message( 'E', wnav_get_message( sts));
    return;
  }

  ldh_sVolumeInfo info;
  pwr_tCid volcid;

  ldh_GetVolumeInfo( ldh_SessionToVol( cmd->ldhses), &info);
  ldh_GetVolumeClass( cmd->wbctx, info.Volume, &volcid);
  
  if ( volcid == pwr_eClass_DirectoryVolume) {
    sts = lfu_SaveDirectoryVolume( cmd->ldhses, 0, quiet);
    if ( EVEN(sts)) {
      cmd->wnav->message( 'E', "Syntax error");
      return;
    }
  }
  cmd->wnav->message( 'E', "Session saved");
}

void Cmd::revert_ok( Cmd *cmd)
{
  int sts;

  sts = ldh_RevertSession ( cmd->ldhses);
  if ( EVEN(sts))
    cmd->wnav->message( 'E', wnav_get_message( sts));
  else
    cmd->wnav->message( 'E', "Session reverted");
}

void Cmd::revert_cb( void *ctx, int confirm)
{
  Cmd *cmd = (Cmd *) ctx;

  if ( !cmd->ldhses)
  {
    cmd->wnav->message( 'E', "Cmd is not attached to a volume");
    return;
  }
  revert_ok( cmd);
}

void Cmd::close_cb( void *ctx)
{
  Cmd *cmd = (Cmd *) ctx;

  dcli_input_end( &cmd->chn, cmd->recall_buf);
  exit(0);
}

Cmd::Cmd() :
  ctx_type(wb_eUtility_Cmd), ldhses(0), wbctx(0), volctx(0), volid(0), wnav(0), 
  wb_type(0)
{
}
