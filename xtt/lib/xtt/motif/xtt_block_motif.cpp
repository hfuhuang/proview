/* 
 * Proview   $Id: xtt_block_motif.cpp,v 1.2 2008-10-31 12:51:36 claes Exp $
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

/* xtt_block_motif.cpp -- Alarm blocking window in xtt. */

#include "pwr_class.h"
#include "pwr_privilege.h"
#include "rt_gdh.h"
#include "rt_mh_outunit.h"
#include "co_cdh.h"


#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/ToggleB.h>

#include "flow_x.h"
#include "xtt_block_motif.h"
#include "co_lng.h"
#include "cow_wow_motif.h"
#include "co_msg.h"

int BlockMotif::execute()
{
  mh_eEventPrio prio;
  pwr_tStatus sts;

  if (XmToggleButtonGetState( toggleA))
    prio = mh_eEventPrio_A;
  else if (XmToggleButtonGetState( toggleB))
    prio = mh_eEventPrio_B;
  else if (XmToggleButtonGetState( toggleC))
    prio = mh_eEventPrio_C;
  else if (XmToggleButtonGetState( toggleD))
    prio = mh_eEventPrio_D;
  else
    prio = (mh_eEventPrio) 0;
  
  sts = mh_OutunitBlock( oar.Objid, prio);
  if (EVEN(sts)) {
    char msg[80];

    msg_GetMsg( sts, msg, sizeof(msg));
    wow->DisplayError( "Block Error", msg);
  }
  return sts;
}

void BlockMotif::update()
{
  pwr_tStatus    sts;
  mh_uEventInfo  block_level;

  sts = gdh_GetAlarmInfo( oar.Objid, NULL, NULL, (pwr_tUInt32 *) &block_level,
			  NULL, NULL);
  switch ( block_level.Event.Prio) {
  case mh_eEventPrio_A:
    XmToggleButtonSetState( toggleA, 1, 1);
    break;
  case mh_eEventPrio_B:
    XmToggleButtonSetState( toggleB, 1, 1);
    break;
  case mh_eEventPrio_C:
    XmToggleButtonSetState( toggleC, 1, 1);
    break;
  case mh_eEventPrio_D:
    XmToggleButtonSetState( toggleD, 1, 1);
    break;
  case 0:
    XmToggleButtonSetState( toggleNo, 1, 1);
    break;
  default:
    break;
  }
}

void BlockMotif::create_ok( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  ((BlockMotif *)blk)->buttonOk = w;
}

void BlockMotif::create_apply( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  ((BlockMotif *)blk)->buttonApply = w;
}

void BlockMotif::create_toggleA( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  ((BlockMotif *)blk)->toggleA = w;
}

void BlockMotif::create_toggleB( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
 ((BlockMotif *)blk)->toggleB = w;
}

void BlockMotif::create_toggleC( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  ((BlockMotif *)blk)->toggleC = w;
}

void BlockMotif::create_toggleD( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  ((BlockMotif *)blk)->toggleD = w;
}

void BlockMotif::create_toggleNo( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  ((BlockMotif *)blk)->toggleNo = w;
}

void BlockMotif::activate_apply( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  blk->execute();
}

void BlockMotif::activate_ok( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  pwr_tStatus sts;

  sts = blk->execute();
  if ( ODD(sts))
    delete blk;
}

void BlockMotif::activate_cancel( Widget w, Block *blk, XmAnyCallbackStruct *data)
{
  delete blk;
}

BlockMotif::~BlockMotif()
{
  delete wow;
  XtDestroyWidget( parent_wid);
}

BlockMotif::BlockMotif( void *b_parent_ctx,
			Widget b_parent_wid,
			pwr_sAttrRef *b_oar,
			char *name,
			unsigned int priv,
			pwr_tStatus *sts):
  Block( b_parent_ctx, b_oar, name, priv, sts), parent_wid(b_parent_wid)
{
  char		uid_filename[120] = {"xtt_block.uid"};
  char		*uid_filename_p = uid_filename;
  Arg 		args[20];
  int		msts;
  int		i;
  MrmHierarchy 	s_DRMh;
  MrmType 	dclass;
  char 		title[200];
  pwr_tAName	aname;

  static MrmRegisterArg	reglist[] = {
        {(char*) "blk_ctx", 0 },
	{(char*) "blk_activate_cancel",(caddr_t)activate_cancel },
	{(char*) "blk_activate_ok",(caddr_t)activate_ok },
	{(char*) "blk_activate_apply",(caddr_t)activate_apply },
	{(char*) "blk_create_ok",(caddr_t)create_ok },
	{(char*) "blk_create_apply",(caddr_t)create_apply },
	{(char*) "blk_create_toggleA",(caddr_t)create_toggleA },
	{(char*) "blk_create_toggleB",(caddr_t)create_toggleB },
	{(char*) "blk_create_toggleC",(caddr_t)create_toggleC },
	{(char*) "blk_create_toggleD",(caddr_t)create_toggleD },
	{(char*) "blk_create_toggleNo",(caddr_t)create_toggleNo }
	};
  static int	reglist_num = (sizeof reglist / sizeof reglist[0]);

  *sts = 1;

  Lng::get_uid( uid_filename, uid_filename);

  *sts = gdh_AttrrefToName( &oar, aname, sizeof(aname), cdh_mNName);
  if ( EVEN(*sts)) return;

  strcpy( title, name);
  strcat( title, "    ");
  strcat( title, aname);

  reglist[0].value = (caddr_t) this;

  // Motif
  MrmInitialize();

  // Save the context structure in the widget
  i = 0;
  XtSetArg(args[i], XmNuserData, (XtPointer) this);i++;
  XtSetArg(args[i], XmNdeleteResponse, XmDO_NOTHING);i++;

  msts = MrmOpenHierarchy( 1, &uid_filename_p, NULL, &s_DRMh);
  if (msts != MrmSUCCESS) printf("can't open %s\n", uid_filename);

  MrmRegisterNames(reglist, reglist_num);

  parent_wid = XtCreatePopupShell( title, 
	  topLevelShellWidgetClass, parent_wid, args, i);

  msts = MrmFetchWidgetOverride( s_DRMh, (char*) "blk_window", parent_wid,
			name, args, 1, &toplevel, &dclass);
  if (msts != MrmSUCCESS)  printf("can't fetch %s\n", name);

  MrmCloseHierarchy(s_DRMh);

  i = 0;
  XtSetArg(args[i],XmNwidth,500);i++;
  XtSetArg(args[i],XmNheight,200);i++;
  XtSetValues( toplevel, args, i);

  XtManageChild( toplevel);

  XtPopup( parent_wid, XtGrabNone);

  if ( !(priv & pwr_mPrv_RtEvents ||
	 priv & pwr_mPrv_System)) {
    Arg	sensitive[1];
    // No access to block
    // XtUnmanageChild( ok);
    // XtUnmanageChild( apply);

    XtSetArg( sensitive[0],XmNsensitive, 0);
    XtSetValues( buttonOk, sensitive, 1);
    XtSetValues( buttonApply, sensitive, 1);
    XtSetValues( toggleA, sensitive, 1);
    XtSetValues( toggleB, sensitive, 1);
    XtSetValues( toggleC, sensitive, 1);
    XtSetValues( toggleD, sensitive, 1);
    XtSetValues( toggleNo, sensitive, 1);
  }

  wow = new CoWowMotif( parent_wid);

  update();

  // Connect the window manager close-button to exit
  flow_AddCloseVMProtocolCb( parent_wid, 
	(XtCallbackProc)activate_cancel, this);
}









