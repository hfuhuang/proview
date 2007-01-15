/* 
 * Proview   $Id: wb_wnav_motif.cpp,v 1.1 2007-01-04 07:29:02 claes Exp $
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

/* wb_wnav.cpp -- Display plant and node hiererachy */

#include "flow_std.h"

#include <stdio.h>
#include <stdlib.h>

#include "pwr_privilege.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_msg.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "wb_wnav_msg.h"
#include "wb_ldh_msg.h"
#include "wb_ldh.h"
#include "wb_login.h"
#include "wb_wccm.h"

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget_motif.h"

#include "wb_wnav_motif.h"
#include "wb_wnav_item.h"

#include "flow_x.h"
#include "co_wow_motif.h"
#include "wb_wge_motif.h"
#include "ge_motif.h"
#include "wb_utl_motif.h"
#include "wb_wda_motif.h"
#include "wb_foe_motif.h"

#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))

static void wnav_reset_avoid_deadlock( WNavMotif *wnav);
static void wnav_set_avoid_deadlock( WNavMotif *wnav, int time);


//
//  Pop wnav window
//

void WNavMotif::pop()
{
  Widget parent, top;

  parent = XtParent( parent_wid);
  while( parent)
  {
    top = parent;
    parent = XtParent( parent);
  }
  flow_UnmapWidget( top);
  flow_MapWidget( top);
}

//
// Create the navigator widget
//
WNavMotif::WNavMotif(
	void *xn_parent_ctx,
	Widget	xn_parent_wid,
	char *xn_name,
	char *xn_layout,
	Widget *w,
        ldh_tSesContext	xn_ldhses,
	wnav_sStartMenu *root_menu,
	wnav_eWindowType xn_type,
	pwr_tStatus *status) :
  WNav( xn_parent_ctx, xn_name, xn_layout, xn_ldhses, root_menu, xn_type, status),
  parent_wid(xn_parent_wid), deadlock_timerid(0), trace_timerid(0), avoid_deadlock(0)
{
  Arg 		args[5];
  int		i;

  if ( window_type == wnav_eWindowType_No)
    return;

  form_widget = ScrolledBrowCreate( parent_wid, name, NULL, 0, 
	WNav::init_brow_base_cb, this, (Widget *)&brow_widget);
  XtManageChild( form_widget);

  i = 0;
  XtSetArg(args[i], XmNborderWidth, 1);i++;
  XtSetValues( form_widget, args,i);

  // Create the root item
  *w = form_widget;

  CoWowMotif::GetAtoms( form_widget, &graph_atom, &objid_atom, &attrref_atom);
  gbl.load_config( this);

  if ( root_menu && !ldhses)
    menu_tree_build( root_menu);

  wow = new CoWowMotif( parent_wid);

  *status = 1;
}

//
//  Delete a nav context
//
WNavMotif::~WNavMotif()
{
  closing_down = 1;

  if ( trace_started)
    XtRemoveTimeOut( trace_timerid);
  if ( avoid_deadlock)
    XtRemoveTimeOut( deadlock_timerid);

  menu_tree_free();
  PalFile::config_tree_free( menu);
  for ( int i = 1; i < brow_cnt; i++)
  {
    brow_DeleteSecondaryCtx( brow_stack[brow_cnt]->ctx);
    brow_stack[brow_cnt]->free_pixmaps();
    delete brow_stack[i];
  }
  delete brow;
  delete wow;
  XtDestroyWidget( form_widget);
}

void WNavMotif::set_inputfocus( int focus)
{
  Arg 		args[2];
  Pixel 	bg, fg;

  if ( !displayed)
    return;

  XtVaGetValues( form_widget, XmNbackground, &bg, XmNforeground, &fg, NULL);
  if ( !focus)
  {
    XtSetArg(args[0], XmNborderColor, bg);
    XtSetValues( form_widget, args, 1);
  }
  else
  {
    if ( flow_IsViewable( brow_widget)) {
      XtCallAcceptFocus( brow_widget, CurrentTime);
      XtSetArg(args[0], XmNborderColor, fg);
      XtSetValues( form_widget, args, 1);
    }
  }
}

void WNavMotif::trace_start()
{
  WNavMotif::trace_scan( this);
}


void WNavMotif::trace_scan( WNavMotif *wnav)
{
  int time = 1000;

  if ( wnav->trace_started)
  {
    brow_TraceScan( wnav->brow->ctx);

    wnav->trace_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext(wnav->brow_widget) , time,
	(XtTimerCallbackProc)WNavMotif::trace_scan, wnav);
  }
}

int WNavMotif::get_selection( char *str, int len)
{
  int sts;

  sts = ((CoWowMotif *)wow)->GetSelection( form_widget, str, len, objid_atom);
  if ( EVEN(sts))
    sts = ((CoWowMotif *)wow)->GetSelection( form_widget, str, len, XA_STRING);

  return sts;
}

void WNavMotif::set_selection_owner()
{
  int sts;

  sts = XtOwnSelection( brow_widget, XA_PRIMARY, 
	XtLastTimestampProcessed(flow_Display(brow_widget)),  
	WNavMotif::sel_convert_cb, WNavMotif::sel_lose_cb , NULL);
  if ( !sts)
  {
     message('E', "Failed attempting to become primary selection owner");
     brow_SelectClear( brow->ctx);
     return;
  }	
  selection_owner = 1;
}

void WNavMotif::create_popup_menu( pwr_tAttrRef aref, int x, int y)
{
  short x1, y1;
  Arg arg[2];

  if ( avoid_deadlock)
    return;

  XtSetArg( arg[0], XmNx, &x1);
  XtSetArg( arg[1], XmNy, &y1);
  XtGetValues( XtParent(brow_widget), arg, 2);

  (create_popup_menu_cb)( parent_ctx, aref, x + x1, y + y1);
  wnav_set_avoid_deadlock( this, 2000);
}

Ge *WNavMotif::ge_new( char *graph_name)
{
  GeMotif *ge = new GeMotif( NULL, parent_wid, ldhses, 0, graph_name);
  return ge;
}

WGe *WNavMotif::wge_new( char *name, char *filename, char *object_name, int modal) 
{ 
  WGe *wge = new WGeMotif( parent_wid, this, name, filename, 0,0,0,0,0,0,0, object_name,
			   modal);
  return wge;
}

wb_utl *WNavMotif::utl_new()
{
  wb_utl_motif *utl = new wb_utl_motif( parent_wid);
  return utl;
}

static void  wnav_reset_avoid_deadlock( WNavMotif *wnav)
{
  wnav->avoid_deadlock = 0;
}

static void  wnav_set_avoid_deadlock( WNavMotif *wnav, int time)
{
  wnav->avoid_deadlock = 1;
  wnav->deadlock_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext( wnav->brow_widget), time,
	(XtTimerCallbackProc)wnav_reset_avoid_deadlock, wnav);
}

Boolean WNavMotif::sel_convert_cb(
    Widget  w,
    Atom    *selection,
    Atom    *target,
    Atom    *type_return,
    XtPointer	*value_return,
    unsigned long   *length_return,
    int	    *format_return)
{
  WNavMotif    	*wnav;
  brow_tCtx	browctx;
  int 		sts;
  int		size;
  WItem		*item;
  pwr_tAName   	attr_str;
  pwr_sAttrRef attrref;
  char          name[200];
  char		*buffp;

  BrowCtxFromWidget( w, (void **) &browctx);
  brow_GetCtxUserData( browctx, (void **) &wnav);

  if (*target == XA_TARGETS(flow_Display(wnav->brow_widget))) {
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
    *targetP++ = XA_TEXT(flow_Display(wnav->brow_widget));
    *length_return = std_length + (targetP - (*(Atom **) value_return));
    bcopy((char *)std_targets, (char *)targetP, sizeof(Atom) * std_length);
    XtFree( (char *)std_targets);
    *type_return = XA_ATOM;
    *format_return = 32;
    return True;
  }

  if (*target == XA_STRING ||
      *target == XA_TEXT(flow_Display(wnav->brow_widget)) ||
      *target == XA_COMPOUND_TEXT(flow_Display(wnav->brow_widget)) ||
      *target == wnav->graph_atom ||
      *target == wnav->objid_atom ||
      *target == wnav->attrref_atom)
  {
    brow_tNode	*node_list;
    int		node_count;
    wnav_eSelectionFormat format;
  
    if ( *target == wnav->graph_atom)
      format = wnav_eSelectionFormat_Graph;
    else if ( *target == wnav->objid_atom)
      format = wnav_eSelectionFormat_Objid;
    else if ( *target == wnav->attrref_atom)
      format = wnav_eSelectionFormat_Attrref;
    else
      format = wnav_eSelectionFormat_User;
	

    brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
    if ( !node_count)
      return FALSE;

    brow_GetUserData( node_list[0], (void **)&item);

    switch( item->type) {
    case wnav_eItemType_Attr:
    case wnav_eItemType_AttrInput:
    case wnav_eItemType_AttrInputInv:
    case wnav_eItemType_AttrInputF:
    case wnav_eItemType_AttrOutput:
    case wnav_eItemType_AttrArray:
    case wnav_eItemType_AttrArrayOutput:
    case wnav_eItemType_AttrArrayElem:
    case wnav_eItemType_AttrObject: {
      WItemBaseAttr *aitem = (WItemBaseAttr *)item;

      sts = ldh_ObjidToName( wnav->ldhses, item->objid, ldh_eName_Hierarchy,
	    	attr_str, sizeof(attr_str), &size);
      if ( EVEN(sts)) return FALSE;

      strcat( attr_str, ".");
      strcat( attr_str, aitem->name);
      sts = ldh_NameToAttrRef( wnav->ldhses, attr_str, &attrref);
      if ( EVEN(sts)) return FALSE;
      sts = (wnav->format_selection_cb)( wnav->parent_ctx, attrref, 
					 &buffp, 0, 1, format);
      *value_return = XtNewString(buffp);      
      *length_return = strlen(buffp) + 1;
      if ( !sts) return FALSE;
      break;
    }
    case wnav_eItemType_Object:
      memset( &attrref, 0, sizeof(attrref));
      attrref.Objid = item->objid;
      sts = (wnav->format_selection_cb)( wnav->parent_ctx, attrref, 
					 &buffp, 0, 0, format);
      *value_return = XtNewString(buffp);      
      *length_return = strlen(buffp) + 1;
      if ( !sts) return FALSE;
        break;
    default:
      brow_GetAnnotation( node_list[0], 0, name, sizeof(name));
      *value_return = XtNewString(name);      
      *length_return = strlen(name) + 1;
    }
    free( node_list);

    if ( *target == XA_COMPOUND_TEXT(flow_Display(wnav->brow_widget)) ||
 	 *target == wnav->graph_atom ||
 	 *target == wnav->objid_atom ||
	 *target == wnav->attrref_atom)
      *type_return = *target;
    else
      *type_return = XA_STRING;
    *format_return = 8;

    return TRUE;
  }
  return FALSE;
}

void WNavMotif::sel_lose_cb( Widget w, Atom *selection)
{
  WNavMotif    	*wnav;
  brow_tCtx	browctx;

  BrowCtxFromWidget( w, (void **) &browctx);
  brow_GetCtxUserData( browctx, (void **) &wnav);

  brow_SelectClear( wnav->brow->ctx);
  wnav->selection_owner = 0;
}

int WNavMotif::open_foe( char *name, pwr_tOid plcpgm,
			 void **foectx, int map_window,
			 ldh_eAccess access, pwr_tOid oid)
{
  pwr_tStatus sts;
  WFoe *foe;

  foe = WFoe::get( plcpgm);
  if ( foe)
    foe->pop();
  else 
    foe = new WFoeMotif( (void *)parent_ctx, parent_wid, name,
			 plcpgm, wbctx, ldhses,
			 map_window, access, &sts);
  if ( EVEN(sts)) return sts;

  if ( cdh_ObjidIsNotNull( oid))
    foe->center_object(oid);
  *foectx = foe;
  return sts;
}

void WNavMotif::wda_new( pwr_tOid oid, pwr_tCid cid, char *attribute,
			 int edit_mode, int advuser, int display_objectname)
{
    new WdaMotif( parent_wid, this, ldhses, oid, cid, attribute, edit_mode, 
		  advuser, display_objectname);
}

void WNavMotif::wge_subwindow_loop( WGe *wge)
{
  XEvent 	Event;
  
  for (;;) {
    XtAppNextEvent( XtWidgetToApplicationContext( parent_wid), &Event);
    XtDispatchEvent( &Event);
	  
    if ( wge->subwindow_release) {
      wge->subwindow_release = 0;
      break;
    }
  }
}

void WNavMotif::wge_modal_loop( WGe *wge)
{
  XEvent 	Event;
	  
  for (;;) {
    XtAppNextEvent( XtWidgetToApplicationContext( parent_wid), &Event);
    XtDispatchEvent( &Event);

    if ( wge->terminated) {
      appl.remove( (void *)wge);
      delete wge;
      break;      
    }
  }
}

static char *wnav_dialog_convert_text( char *text)
{
  char *s, *t;
  static char new_text[200];

  for ( s = text, t = new_text; *s; s++, t++)
  {
    if ( *s == '\\' && *(s+1) == 'n')
    {
      *t = 10;
      s++;
    }
    else
     *t = *s;
    if ( t > &new_text[sizeof(new_text)-1])
      break;
  }
  *t = *s;
  return new_text;
}

static void wnav_message_dialog_ok( Widget w, XtPointer wnav, XtPointer data)
{
  ((WNav *)wnav)->dialog_ok = 1;
}

static void wnav_message_dialog_cancel( Widget w, XtPointer wnav, XtPointer data)
{
  ((WNav *)wnav)->dialog_cancel = 1;
}

void wnav_message_dialog_help( Widget w, XtPointer wnav, XtPointer data)
{
  ((WNav *)wnav)->dialog_help = 1;
}

static void wnav_message_dialog_read( Widget w, XtPointer client_data, XtPointer data)
{
  WNav *wnav = (WNav *)client_data;
  char *value;

  XmSelectionBoxCallbackStruct *cbs = (XmSelectionBoxCallbackStruct *)data;
  wnav->dialog_ok = 1;
  XmStringGetLtoR( cbs->value, XmSTRING_DEFAULT_CHARSET, &value);
  strncpy( wnav->dialog_value, value, sizeof(wnav->dialog_value));
  XtFree( value);
}

void WNavMotif::message_dialog( char *title, char *text)
{
  Widget 	dialog;
  XmString	text_str;
  XmString	title_str;
  Arg		args[9];
  int		i;
  XEvent 	Event;

  text_str = XmStringCreateLocalized( wnav_dialog_convert_text(text));
  title_str = XmStringCreateLocalized( title);
  i = 0;
  XtSetArg( args[i], XmNmessageString, text_str); i++;
  XtSetArg( args[i], XmNdialogTitle, title_str); i++;
  XtSetArg( args[i], XmNdialogType, XmDIALOG_MESSAGE); i++;
  if ( dialog_width && dialog_height)
  {
    XtSetArg( args[i], XmNwidth, dialog_width); i++;
    XtSetArg( args[i], XmNheight, dialog_height); i++;
    XtSetArg( args[i], XmNx, dialog_x); i++;
    XtSetArg( args[i], XmNy, dialog_y); i++;
  }

  dialog = XmCreateInformationDialog( parent_wid, "Info", args, i);
  XmStringFree( text_str);
  XmStringFree( title_str);

  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON));
  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_CANCEL_BUTTON));

  XtAddCallback( dialog, XmNokCallback, wnav_message_dialog_ok, this);

  // Connect the window manager close-button to exit
  flow_AddCloseVMProtocolCb( XtParent(dialog),
	(XtCallbackProc)wnav_message_dialog_ok, this);

  XtManageChild( dialog);
  XtPopup( XtParent(dialog), XtGrabNone);

  dialog_ok = 0;

  for (;;)
  {
    XtAppNextEvent( XtWidgetToApplicationContext( dialog), &Event);
    XtDispatchEvent( &Event);
    if ( dialog_ok)
      return;
  }
}

int WNavMotif::confirm_dialog( char *title, char *text, int display_cancel,
		int *cancel)
{
  Widget 	dialog;
  XmString	text_str;
  XmString	title_str;
  XmString	no_str;
  XmString	yes_str;
  XmString	cancel_str;
  Arg		args[14];
  int		i;
  XEvent 	Event;

  text_str = XmStringCreateLocalized( wnav_dialog_convert_text(text));
  title_str = XmStringCreateLocalized( title);
  no_str = XmStringCreateLocalized( "No");
  yes_str = XmStringCreateLocalized( "Yes");
  cancel_str = XmStringCreateLocalized( "Cancel");
  i = 0;
  XtSetArg( args[i], XmNmessageString, text_str); i++;
  XtSetArg( args[i], XmNdialogTitle, title_str); i++;
  XtSetArg( args[i], XmNcancelLabelString, no_str); i++;
  XtSetArg( args[i], XmNokLabelString, yes_str); i++;
  XtSetArg( args[i], XmNhelpLabelString, cancel_str); i++;
  XtSetArg( args[i], XmNdialogType, XmDIALOG_WARNING); i++;
  if ( dialog_width && dialog_height)
  {
    XtSetArg( args[i], XmNwidth, dialog_width); i++;
    XtSetArg( args[i], XmNheight, dialog_height); i++;
    XtSetArg( args[i], XmNx, dialog_x); i++;
    XtSetArg( args[i], XmNy, dialog_y); i++;
  }

  dialog = XmCreateInformationDialog( parent_wid, "Info", args, i);
  XmStringFree( text_str);
  XmStringFree( title_str);
  XmStringFree( no_str);
  XmStringFree( yes_str);
  XmStringFree( cancel_str);

  if ( !display_cancel)
    XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON));

  XtAddCallback( dialog, XmNokCallback, wnav_message_dialog_ok, this);
  XtAddCallback( dialog, XmNcancelCallback, wnav_message_dialog_cancel, this);
  XtAddCallback( dialog, XmNhelpCallback, wnav_message_dialog_help, this);

  // Connect the window manager close-button to exit
  if ( !display_cancel)
    flow_AddCloseVMProtocolCb( XtParent(dialog),
	(XtCallbackProc)wnav_message_dialog_cancel, this);
  else
    flow_AddCloseVMProtocolCb( XtParent(dialog),
	(XtCallbackProc)wnav_message_dialog_help, this);

  XtManageChild( dialog);
  XtPopup( XtParent(dialog), XtGrabNone);

  dialog_ok = 0;
  dialog_cancel = 0;
  dialog_help = 0;

  for (;;)
  {
    XtAppNextEvent( XtWidgetToApplicationContext( dialog), &Event);
    XtDispatchEvent( &Event);
    if ( dialog_ok)
    {
      if ( display_cancel)
        *cancel = 0;
      return 1;
    }
    if ( dialog_cancel)
    {
      if ( display_cancel)
        *cancel = 0;
      return 0;
    }
    if ( dialog_help)
    {
      *cancel = 1;
      XtDestroyWidget( dialog);
      return 0;
    }
  }
}

int WNavMotif::continue_dialog( char *title, char *text)
{
  Widget 	dialog;
  XmString	text_str;
  XmString	title_str;
  XmString	continue_str;
  XmString	quit_str;
  Arg		args[10];
  int		i;
  XEvent 	Event;

  text_str = XmStringCreateLocalized( wnav_dialog_convert_text(text));
  title_str = XmStringCreateLocalized( title);
  continue_str = XmStringCreateLocalized( "Continue");
  quit_str = XmStringCreateLocalized( "Quit");
  i = 0;
  XtSetArg( args[i], XmNmessageString, text_str); i++;
  XtSetArg( args[i], XmNdialogTitle, title_str); i++;
  XtSetArg( args[i], XmNcancelLabelString, quit_str); i++;
  XtSetArg( args[i], XmNokLabelString, continue_str); i++;
  XtSetArg( args[i], XmNdialogType, XmDIALOG_WARNING); i++;
  if ( dialog_width && dialog_height)
  {
    XtSetArg( args[i], XmNwidth, dialog_width); i++;
    XtSetArg( args[i], XmNheight, dialog_height); i++;
    XtSetArg( args[i], XmNx, dialog_x); i++;
    XtSetArg( args[i], XmNy, dialog_y); i++;
  }

  dialog = XmCreateInformationDialog( parent_wid, "Info", args, i);
  XmStringFree( text_str);
  XmStringFree( title_str);
  XmStringFree( quit_str);
  XmStringFree( continue_str);

  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON));

  XtAddCallback( dialog, XmNokCallback, wnav_message_dialog_ok, this);
  XtAddCallback( dialog, XmNcancelCallback, wnav_message_dialog_cancel, this);

  flow_AddCloseVMProtocolCb( XtParent(dialog),
	(XtCallbackProc)wnav_message_dialog_cancel, this);

  XtManageChild( dialog);
  XtPopup( XtParent(dialog), XtGrabNone);

  dialog_ok = 0;
  dialog_cancel = 0;
  dialog_help = 0;

  for (;;)
  {
    XtAppNextEvent( XtWidgetToApplicationContext( dialog), &Event);
    XtDispatchEvent( &Event);
    if ( dialog_ok)
      return 1;
    if ( dialog_cancel)
      return 0;
  }
}

int WNavMotif::prompt_dialog( char *title, char *text, char **value)
{
  Widget 	dialog;
  XmString	text_str;
  XmString	title_str;
  Arg		args[10];
  int		i;
  XEvent 	Event;

  text_str = XmStringCreateLocalized( wnav_dialog_convert_text(text));
  title_str = XmStringCreateLocalized( title);
  i = 0;
  XtSetArg( args[i], XmNselectionLabelString, text_str); i++;
  XtSetArg( args[i], XmNdialogTitle, title_str); i++;
  if ( dialog_width && dialog_height)
  {
    XtSetArg( args[i], XmNwidth, dialog_width); i++;
    XtSetArg( args[i], XmNheight, dialog_height); i++;
    XtSetArg( args[i], XmNx, dialog_x); i++;
    XtSetArg( args[i], XmNy, dialog_y); i++;
  }
//  XtSetArg( args[i], XmNautoUnmanage, False); i++;

  dialog = XmCreatePromptDialog( parent_wid, "Info", args, i);
  XmStringFree( text_str);
  XmStringFree( title_str);

  XtUnmanageChild( XmSelectionBoxGetChild( dialog, XmDIALOG_HELP_BUTTON));

  XtAddCallback( dialog, XmNokCallback, wnav_message_dialog_read, this);
  XtAddCallback( dialog, XmNcancelCallback, wnav_message_dialog_cancel, this);

  // Connect the window manager close-button to exit
  flow_AddCloseVMProtocolCb( XtParent(dialog),
	(XtCallbackProc)wnav_message_dialog_cancel, this);

  XtManageChild( dialog);
  XtPopup( XtParent(dialog), XtGrabNone);

  dialog_ok = 0;
  dialog_cancel = 0;
  dialog_help = 0;

  for (;;)
  {
    XtAppNextEvent( XtWidgetToApplicationContext( dialog), &Event);
    XtDispatchEvent( &Event);
    if ( dialog_ok)
    {
      *value = dialog_value;
      XtDestroyWidget( dialog);
      return 1;
    }
    if ( dialog_cancel)
    {
      strcpy( dialog_value, "");
      *value = dialog_value;
      XtDestroyWidget( dialog);
      return 0;
    }
  }
}
