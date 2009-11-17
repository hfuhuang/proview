/* 
 * Proview   $Id: xtt_xattnav_motif.cpp,v 1.2 2008-10-31 12:51:36 claes Exp $
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

/* wb_xattnav.cpp -- Display object info */

#include <stdio.h>
#include <stdlib.h>

#include "co_cdh.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "rt_xatt_msg.h"
#include "rt_mh_net.h"

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget_motif.h"

#include "xtt_xatt.h"
#include "xtt_xattnav_motif.h"
#include "xtt_xnav.h"
#include "xtt_xnav_brow.h"
#include "xtt_xnav_crr.h"
#include "xtt_item.h"
#include "pwr_privilege.h"
#include "cow_wow_motif.h"

#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))

//
// Create the navigator widget
//
XAttNavMotif::XAttNavMotif(
	void 		*xa_parent_ctx,
	Widget		xa_parent_wid,
	xattnav_eType   xa_type,
	const char     	*xa_name,
	pwr_sAttrRef 	*xa_objar,
	int 		xa_advanced_user,
	Widget 		*w,
	pwr_tStatus 	*status) :
  XAttNav( xa_parent_ctx, xa_type, xa_name, xa_objar, xa_advanced_user, status),
  parent_wid(xa_parent_wid)
{
  form_widget = ScrolledBrowCreate( parent_wid, name, NULL, 0, 
	init_brow_cb, this, (Widget *)&brow_widget);
  XtManageChild( form_widget);

  // Create the root item
  *w = form_widget;

  wow = new CoWowMotif( form_widget);
  trace_timerid = wow->timer_new();
  *status = 1;
}

//
//  Delete a nav context
//
XAttNavMotif::~XAttNavMotif()
{
  if ( trace_started)
    trace_timerid->remove();

  delete trace_timerid;
  delete wow;
  delete brow;
  XtDestroyWidget( form_widget);
}

void XAttNavMotif::set_inputfocus()
{
//  brow_SetInputFocus( brow->ctx);

  if ( !displayed)
    return;

  XtCallAcceptFocus( brow_widget, CurrentTime);
}

void XAttNavMotif::popup_position( int x_event, int y_event, int *x, int *y)
{
  CoWowMotif::PopupPosition( brow_widget, x_event, y_event, x, y);
}





