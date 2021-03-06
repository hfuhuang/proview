/* 
 * Proview   $Id: wb_pal_motif.cpp,v 1.2 2008-10-31 12:51:31 claes Exp $
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
 **/

/* wb_pal_motif.cpp -- Palette of configurator or plc-editor */

#if defined OS_VMS && defined __ALPHA
# pragma message disable (NOSIMPINT,EXTROUENCUNNOBJ)
#endif

#if defined OS_VMS && !defined __ALPHA
# pragma message disable (LONGEXTERN)
#endif

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "wb_ldh.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "pwr_baseclasses.h"
}

#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#ifndef _XtIntrinsic_h
#include <X11/Intrinsic.h>
#endif
#include <X11/IntrinsicP.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget_motif.h"
#include "wb_pal_motif.h"

#include "glow.h"
#include "glow_growctx.h"

extern "C" {
#include "flow_x.h"
}

static void pal_sel_lose_cb( Widget w, Atom *selection);
static Boolean pal_sel_convert_cb(
    Widget  w,
    Atom    *selection,
    Atom    *target,
    Atom    *type_return,
    XtPointer	*value_return,
    unsigned long   *length_return,
    int	    *format_return);
static void pal_set_avoid_deadlock( PalMotif *pal, int time);
static void pal_reset_avoid_deadlock( PalMotif *pal);

//
// Create the palette widgets
//
PalMotif::PalMotif(
	void *pal_parent_ctx,
	Widget	pal_parent_wid,
	const char *pal_name,
	ldh_tSesContext pal_ldhses,
	const char *pal_root_name,
	Widget *w,
	pwr_tStatus *status) : 

	Pal( pal_parent_ctx, pal_name, pal_ldhses, pal_root_name, status),
	parent_wid(pal_parent_wid), avoid_deadlock(0)
{
  int i;
  Arg 		args[5];

  form_widget = ScrolledBrowCreate( parent_wid, name, NULL, 0, 
	Pal::init_brow_cb, this, (Widget *)&brow_widget);
  XtManageChild( form_widget);

  i = 0;
  XtSetArg(args[i], XmNborderWidth, 1);i++;
  XtSetValues( form_widget, args,i);
  set_inputfocus(0);

  *w = form_widget;
  *status = 1;
}

//
//  Delete a pal context
//
PalMotif::~PalMotif()
{
  if ( avoid_deadlock)
    XtRemoveTimeOut( deadlock_timerid);

  PalFile::config_tree_free( menu);
  free_pixmaps();
  XtDestroyWidget( form_widget);
}

void PalMotif::create_popup_menu( pwr_tCid cid, int x, int y)
{
  int x1, y1;
  Arg arg[2];

  if ( avoid_deadlock)
    return;


  XtSetArg( arg[0], XmNx, &x1);
  XtSetArg( arg[1], XmNy, &y1);
  XtGetValues( XtParent(brow_widget), arg, 2);

  (create_popup_menu_cb)( parent_ctx, cid, x + x1, y + y1);
  pal_set_avoid_deadlock( this, 2000);

}

void PalMotif::set_inputfocus( int focus)
{
  Arg 		args[2];
  Pixel 	bg, fg;

  if ( !displayed)
    return;

  XtVaGetValues( form_widget, XmNbackground, &bg, XmNforeground, &fg, NULL);
  if ( focus) {
    XtCallAcceptFocus( brow_widget, CurrentTime);
    XtSetArg(args[0], XmNborderColor, fg);
    XtSetValues( form_widget, args, 1);
  }
  else {
    XtSetArg(args[0], XmNborderColor, bg);
    XtSetValues( form_widget, args, 1);
  }
//  brow_SetInputFocus( brow->ctx);
}

void PalMotif::set_selection_owner()
{
  int sts;

  sts = XtOwnSelection( brow_widget, XA_PRIMARY, 
	XtLastTimestampProcessed(flow_Display(brow_widget)),  
	pal_sel_convert_cb, pal_sel_lose_cb , NULL);
  if ( !sts) {
     brow_SelectClear( brow_ctx);
     return;
  }	
  selection_owner = 1;
}

static Boolean pal_sel_convert_cb(
    Widget  w,
    Atom    *selection,
    Atom    *target,
    Atom    *type_return,
    XtPointer	*value_return,
    unsigned long   *length_return,
    int	    *format_return)
{
  PalMotif     	*pal;
  brow_tCtx	browctx;
  char		name[200];
  PalItem	*item;

  BrowCtxFromWidget( w, (void **) &browctx);
  brow_GetCtxUserData( browctx, (void **) &pal);

  if (*target == XA_TARGETS(flow_Display(pal->brow_widget))) {
    Atom *targetP;
    Atom *std_targets;
    unsigned long std_length;
    XSelectionRequestEvent *req = XtGetSelectionRequest( w, *selection, 
       (XtRequestId)NULL);

    XmuConvertStandardSelection( w, req->time, selection, target, type_return,
		(caddr_t *)&std_targets, &std_length, format_return);

    *value_return = XtMalloc( sizeof(Atom) * (std_length + 2));
    targetP = *(Atom **) value_return;
    *targetP++ = XA_STRING;
    *targetP++ = XA_TEXT(flow_Display(pal->brow_widget));
    *length_return = std_length + (targetP - (*(Atom **) value_return));
    bcopy((char *)std_targets, (char *)targetP, sizeof(Atom) * std_length);
    XtFree( (char *)std_targets);
    *type_return = XA_ATOM;
    *format_return = 32;
    return True;
  }

  if (*target == XA_STRING ||
      *target == XA_TEXT(flow_Display(pal->brow_widget)) ||
      *target == XA_COMPOUND_TEXT(flow_Display(pal->brow_widget))) {
    brow_tNode	*node_list;
    int		node_count;
  
    brow_GetSelectedNodes( pal->brow_ctx, &node_list, &node_count);
    if ( !node_count)
      return FALSE;

    brow_GetUserData( node_list[0], (void **)&item);

    switch( item->type) {
      case pal_ePalItemType_ClassVolume: 
      case pal_ePalItemType_Class: 
      case pal_ePalItemType_Object: 
      default:
        brow_GetAnnotation( node_list[0], 0, name, sizeof(name));
        *value_return = XtNewString(name);      
        *length_return = strlen(name) + 1;
    }
    free( node_list);

    *type_return = XA_STRING;
    *format_return = 8;

    return TRUE;
  }
  return FALSE;
}

static void pal_sel_lose_cb( Widget w, Atom *selection)
{
  PalMotif     	*pal;
  brow_tCtx	browctx;

  BrowCtxFromWidget( w, (void **) &browctx);
  brow_GetCtxUserData( browctx, (void **) &pal);

  brow_SelectClear( pal->brow_ctx);
  pal->selection_owner = 0;
}

static void pal_reset_avoid_deadlock( PalMotif *pal)
{
  pal->avoid_deadlock = 0;
}

static void pal_set_avoid_deadlock( PalMotif *pal, int time)
{
  pal->avoid_deadlock = 1;
  pal->deadlock_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext( pal->brow_widget), time,
	(XtTimerCallbackProc)pal_reset_avoid_deadlock, pal);
}









