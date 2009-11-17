/* 
 * Proview   $Id: wb_wtt_motif.cpp,v 1.4 2008-10-31 12:51:31 claes Exp $
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

/* wb_wtt_motif.cpp -- Display plant and node hiererachy */

#include "flow_std.h"

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "co_dcli.h"
#include "pwr_baseclasses.h"
#include "wb_ldh.h"
#include "flow_x.h"
#include "cow_wow.h"
}

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/List.h>
#include <Xm/CascadeBG.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <X11/cursorfont.h>
#define XK_MISCELLANY
#include <X11/keysymdef.h>

#include "cow_mrm_util.h"
#include "wb_utl_api.h"
#include "wb_lfu.h"
#include "rt_load.h"
#include "wb_foe_msg.h"
#include "wb_pwrb_msg.h"
#include "wb_uted_motif.h"

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "cow_wow_motif.h"
#include "wb_wtt.h"
#include "wb_wnav_motif.h"
#include "wb_wnav_item.h"
#include "wb_pal_motif.h"
#include "wb_watt_motif.h"
#include "wb_wda_motif.h"
#include "wb_wtt_motif.h"
#include "wb_wnav_msg.h"
#include "wb_volume.h"
#include "wb_env.h"
#include "wb_wpkg_motif.h"
#include "cow_msgwindow.h"
#include "wb_wnav_selformat.h"
#include "wb_pwrs.h"
#include "wb_build.h"
#include "wb_wcast_motif.h"
#include "ge_motif.h"
#include "wb_uilutil.h"

#define WTT_PALETTE_WIDTH 160
#define MENU_BAR      1
#define MENU_PULLDOWN 2
#define MENU_POPUP    3
#define MENU_OPTION   4

void WttMotif::set_window_char( int width, int height)
{
  Arg args[5];
  int i;

  i = 0;
  if ( width) {
    short w = width;
    XtSetArg(args[i], XmNwidth, w);i++;
  }
  if ( height) {
    short h = height;
    XtSetArg(args[i], XmNheight, h);i++;
  }
  XtSetValues( toplevel, args,i);
}

void WttMotif::get_window_char( int *width, int *height)
{
  Arg args[2];
  short w, h;

  XtSetArg(args[0], XmNwidth, &w);
  XtSetArg(args[0], XmNheight, &h);
  XtGetValues( toplevel, args, 2);
  *width = w;
  *height = h;
}

void WttMotif::set_clock_cursor()
{
  if ( !clock_cursor)
    clock_cursor = XCreateFontCursor( flow_Display(toplevel), XC_watch);

  XDefineCursor( flow_Display(toplevel), flow_Window(toplevel), clock_cursor);
  XFlush( flow_Display(toplevel));
}

void WttMotif::reset_cursor()
{
  XUndefineCursor( flow_Display(toplevel), flow_Window(toplevel));
}

void WttMotif::free_cursor()
{
  if (clock_cursor)
    XFreeCursor( flow_Display(toplevel), clock_cursor);
}

void WttMotif::menu_setup()
{
  Arg	sensitive[1];
  Arg	nosensitive[1];

  XtSetArg( sensitive[0],XmNsensitive, 1);
  XtSetArg( nosensitive[0],XmNsensitive, 0);

  switch( wb_type) {
    case wb_eType_Directory:
      if ( editmode) {
        XtSetValues( menu_save_w, sensitive, 1);
        XtSetValues( menu_revert_w, sensitive, 1);
        XtSetValues( menu_cut_w, sensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, sensitive, 1);
        XtSetValues( menu_pasteinto_w, sensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, sensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, sensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, sensitive, 1);
      }
      else {
        XtSetValues( menu_save_w, nosensitive, 1);
        XtSetValues( menu_revert_w, nosensitive, 1);
        XtSetValues( menu_cut_w, nosensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, nosensitive, 1);
        XtSetValues( menu_pasteinto_w, nosensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, nosensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, sensitive, 1);
        XtSetValues( menu_distribute_w, sensitive, 1);
        XtSetValues( menu_change_value_w, nosensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, sensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      break;
    case wb_eType_Volume:
      if ( editmode) {
        XtSetValues( menu_save_w, sensitive, 1);
        XtSetValues( menu_revert_w, sensitive, 1);
        XtSetValues( menu_cut_w, sensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, sensitive, 1);
        XtSetValues( menu_pasteinto_w, sensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, sensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, sensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
	if ( ldhses && ldh_VolRepType( ldhses) == ldh_eVolRep_Dbs)
          XtSetValues( menu_updateclasses_w, nosensitive, 1);
	else
          XtSetValues( menu_updateclasses_w, sensitive, 1);
      }
      else {
        XtSetValues( menu_save_w, nosensitive, 1);
        XtSetValues( menu_revert_w, nosensitive, 1);
        XtSetValues( menu_cut_w, nosensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, nosensitive, 1);
        XtSetValues( menu_pasteinto_w, nosensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, nosensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, sensitive, 1);
        XtSetValues( menu_buildobject_w, sensitive, 1);
	if ( ldhses && ldh_VolRepType( ldhses) == ldh_eVolRep_Dbs)
          XtSetValues( menu_buildvolume_w, nosensitive, 1);
	else
          XtSetValues( menu_buildvolume_w, sensitive, 1);
        XtSetValues( menu_buildnode_w, sensitive, 1);
        XtSetValues( menu_distribute_w, sensitive, 1);
        XtSetValues( menu_change_value_w, nosensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      break; 
   case wb_eType_Class:
     if ( editmode) {
        XtSetValues( menu_save_w, sensitive, 1);
        XtSetValues( menu_revert_w, sensitive, 1);
        XtSetValues( menu_cut_w, sensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, sensitive, 1);
        XtSetValues( menu_pasteinto_w, sensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, sensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, sensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      else {
        XtSetValues( menu_save_w, nosensitive, 1);
        XtSetValues( menu_revert_w, nosensitive, 1);
        XtSetValues( menu_cut_w, nosensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, nosensitive, 1);
        XtSetValues( menu_pasteinto_w, nosensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, nosensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
	XtSetValues( menu_buildvolume_w, sensitive, 1);
        XtSetValues( menu_buildnode_w, sensitive, 1);
        XtSetValues( menu_distribute_w, sensitive, 1);
        XtSetValues( menu_change_value_w, nosensitive, 1);
	XtSetValues( menu_edit_w, nosensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      break;
   case wb_eType_ClassEditor:
     XtUnmanageChild( menu_buildobject_w);
     XtUnmanageChild( menu_utilities_w);
     XtUnmanageChild( menu_buildnode_w);
     XtUnmanageChild( menu_classeditor_w);
     if ( editmode) {
        XtSetValues( menu_save_w, sensitive, 1);
        XtSetValues( menu_revert_w, sensitive, 1);
        XtSetValues( menu_cut_w, sensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, sensitive, 1);
        XtSetValues( menu_pasteinto_w, sensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, sensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, sensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      else {
        XtSetValues( menu_save_w, nosensitive, 1);
        XtSetValues( menu_revert_w, nosensitive, 1);
        XtSetValues( menu_cut_w, nosensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, nosensitive, 1);
        XtSetValues( menu_pasteinto_w, nosensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, nosensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, sensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
	XtSetValues( menu_buildvolume_w, sensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, nosensitive, 1);
	XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      break;
    case wb_eType_Buffer:
      if ( editmode) {
        XtSetValues( menu_save_w, nosensitive, 1);
        XtSetValues( menu_revert_w, nosensitive, 1);
        XtSetValues( menu_cut_w, sensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, sensitive, 1);
        XtSetValues( menu_pasteinto_w, sensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, sensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, sensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      else {
        XtSetValues( menu_save_w, nosensitive, 1);
        XtSetValues( menu_revert_w, nosensitive, 1);
        XtSetValues( menu_cut_w, nosensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, nosensitive, 1);
        XtSetValues( menu_pasteinto_w, nosensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, nosensitive, 1);
        XtSetValues( menu_utilities_w, sensitive, 1);
        XtSetValues( menu_openplc_w, sensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, nosensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      break; 
    case wb_eType_ExternVolume:
      if ( editmode) {
        XtSetValues( menu_save_w, sensitive, 1);
        XtSetValues( menu_revert_w, sensitive, 1);
        XtSetValues( menu_cut_w, sensitive, 1);
        XtSetValues( menu_copy_w, sensitive, 1);
        XtSetValues( menu_paste_w, sensitive, 1);
        XtSetValues( menu_pasteinto_w, sensitive, 1);
        XtSetValues( menu_copykeep_w, sensitive, 1);
        XtSetValues( menu_rename_w, sensitive, 1);
        XtSetValues( menu_utilities_w, nosensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, sensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      else {
        XtSetValues( menu_save_w, nosensitive, 1);
        XtSetValues( menu_revert_w, nosensitive, 1);
        XtSetValues( menu_cut_w, nosensitive, 1);
        XtSetValues( menu_copy_w, nosensitive, 1);
        XtSetValues( menu_paste_w, nosensitive, 1);
        XtSetValues( menu_pasteinto_w, nosensitive, 1);
        XtSetValues( menu_copykeep_w, nosensitive, 1);
        XtSetValues( menu_rename_w, nosensitive, 1);
        XtSetValues( menu_utilities_w, nosensitive, 1);
        XtSetValues( menu_openplc_w, nosensitive, 1);
        XtSetValues( menu_buildobject_w, nosensitive, 1);
        XtSetValues( menu_buildvolume_w, nosensitive, 1);
        XtSetValues( menu_buildnode_w, nosensitive, 1);
        XtSetValues( menu_distribute_w, nosensitive, 1);
        XtSetValues( menu_change_value_w, nosensitive, 1);
        XtSetValues( menu_edit_w, sensitive, 1);
        XtSetValues( menu_classeditor_w, nosensitive, 1);
        XtSetValues( menu_updateclasses_w, nosensitive, 1);
      }
      break;
    default:
      ;
  }
}

void WttMotif::set_selection_owner()
{
  selection_timerid = XtAppAddTimeOut( XtWidgetToApplicationContext( toplevel), 200,
       (XtTimerCallbackProc)WttMotif::set_selection_owner_proc, this);  
}

void WttMotif::set_palette_selection_owner() 
{
  selection_timerid = XtAppAddTimeOut( XtWidgetToApplicationContext( toplevel), 200,
    	(XtTimerCallbackProc)WttMotif::set_palette_selection_owner_proc, this);
}

void WttMotif::set_selection_owner_proc( WttMotif *wtt)
{
  // Delay call to own selection, to make it possible to paste previous selection to value inputwith MB2
  wtt->selection_timerid = 0;
  if ( wtt->focused_wnav)
    wtt->focused_wnav->set_selection_owner();
}

void WttMotif::set_palette_selection_owner_proc( WttMotif *wtt)
{
  // Delay call to own selection, to make it possible to paste previous selection to value inputwith MB2
  wtt->selection_timerid = 0;
  wtt->palette->set_selection_owner();
}

int WttMotif::create_popup_menu( pwr_tAttrRef aref,
				 int x, int y)
{
  Widget popup;
  Arg args[5]; 
  int i;
  short x1, y1, x2, y2, x3, y3;
  short menu_x, menu_y;
  int sts;
  
  if ( !ldhses)
    return 0;

  XtSetArg( args[0], XmNx, &x1);
  XtSetArg( args[1], XmNy, &y1);
  XtGetValues( wnav_form, args, 2);

  XtSetArg( args[0], XmNx, &x2);
  XtSetArg( args[1], XmNy, &y2);
  XtGetValues( XtParent( wnav_form), args, 2);

  XtSetArg( args[0], XmNx, &x3);
  XtSetArg( args[1], XmNy, &y3);
  XtGetValues( toplevel, args, 2);

  menu_x = x + x1 + x2 + x3 + 8;
  menu_y = y + y1 + y2 + y3;
  sts = get_popup_menu_items( aref, pwr_cNCid);
  if ( EVEN(sts)) return sts;

  popup = build_menu(); 
  if ( !popup)
    return 0;

  i = 0;
  XtSetArg(args[i], XmNx, menu_x);i++;
  XtSetArg(args[i], XmNy, menu_y);i++;
  XtSetValues( popup ,args,i);

//    XmMenuPosition(popup, (XButtonPressedEvent *)data->event);
  XtManageChild(popup);
  return 1;
}

int WttMotif::create_pal_popup_menu( pwr_tCid cid,
				     int x, int y)
{
  Widget popup;
  Arg args[5]; 
  int i;
  short x1, y1, x2, y2, x3, y3;
  short menu_x, menu_y;
  int sts;

  if ( !ldhses)
    return 0;

  XtSetArg( args[0], XmNx, &x1);
  XtSetArg( args[1], XmNy, &y1);
  XtGetValues( palette_form, args, 2);

  XtSetArg( args[0], XmNx, &x2);
  XtSetArg( args[1], XmNy, &y2);
  XtGetValues( XtParent( palette_form), args, 2);

  XtSetArg( args[0], XmNx, &x3);
  XtSetArg( args[1], XmNy, &y3);
  XtGetValues( toplevel, args, 2);

  menu_x = x + x1 + x2 + x3 + 8;
  menu_y = y + y1 + y2 + y3;

  sts = get_popup_menu_items( pwr_cNAttrRef, cid);
  if ( EVEN(sts)) return sts;

  popup = build_menu(); 
  if ( !popup) 
    return 0;

  i = 0;
  XtSetArg(args[i], XmNx, menu_x);i++;
  XtSetArg(args[i], XmNy, menu_y);i++;
  XtSetValues( popup ,args,i);

//    XmMenuPosition(popup, (XButtonPressedEvent *)data->event);
  XtManageChild(popup);
  return 1;
}

void WttMotif::set_noedit_show()
{
  Arg args[5];
  int i;
  short width;

  if ( editmode == 0)
    return;

  i = 0;
  XtSetArg(args[i], XmNleftAttachment, XmATTACH_FORM);i++;
  XtSetValues( wnav_form ,args,i);

  XtUnmanageChild( palette_form);

  i = 0;
  XtSetArg(args[i], XmNwidth, &width);i++;
  XtGetValues( toplevel, args,i);

  i = 0;
  XtSetArg(args[i], XmNwidth, width - WTT_PALETTE_WIDTH);i++;
  XtSetValues( toplevel, args,i);
}

void WttMotif::set_edit_show()
{
  Arg args[5];
  int i;
  short width;

  XtManageChild( palette_form);

  i = 0;
  XtSetArg(args[i], XmNleftAttachment, XmATTACH_WIDGET);i++;
  XtSetArg(args[i], XmNleftWidget, palette_form);i++;
  XtSetValues( wnav_form ,args,i);

  i = 0;
  XtSetArg(args[i], XmNwidth, &width);i++;
  XtGetValues( toplevel, args,i);

  i = 0;
  XtSetArg(args[i], XmNwidth, width + WTT_PALETTE_WIDTH);i++;
  XtSetValues( toplevel, args,i);
}

void WttMotif::set_twowindows( int two, int display_wnav, int display_wnavnode)
{

  if ( display_wnav || display_wnavnode) {
    if ( display_wnav && ! wnav_mapped) {
      XtManageChild( wnav_brow_widget);
      set_focus( wnav);
      wnav_mapped = 1;
    }
    if ( display_wnavnode && ! wnavnode_mapped) {
      XtManageChild( wnavnode_brow_widget);
      set_focus( wnavnode);
      wnavnode_mapped = 1;
    }
    if ( !display_wnav && wnav_mapped) {
      XtUnmanageChild( wnav_brow_widget);
      set_focus( wnavnode);
      wnav_mapped = 0;
    }
    if ( !display_wnavnode && wnavnode_mapped) {
      XtUnmanageChild( wnavnode_brow_widget);
      set_focus( wnav);
      wnavnode_mapped = 0;
    }
    twowindows = display_wnav && display_wnavnode;
  }
  else {
    if ( two == twowindows)
      return;

    if ( !two) {
      // Keep the focused window
      if ( focused_wnav == wnavnode) {
        XtUnmanageChild( wnav_brow_widget);
        set_focus( wnavnode);
        wnav_mapped = 0;
        wnavnode_mapped = 1;
      }
      else {
        XtUnmanageChild( wnavnode_brow_widget);
        set_focus( wnav);
        wnav_mapped = 1;
        wnavnode_mapped = 0;
      }
    }
    else {
      if ( !wnav_mapped) {
        XtManageChild( wnav_brow_widget);
        set_focus( wnav);
        wnav_mapped = 1;
      }
      if ( !wnavnode_mapped) {
        XtManageChild( wnavnode_brow_widget);
        set_focus( wnavnode);
        wnavnode_mapped = 1;
      }
    }
    twowindows = two;
  }
}

void WttMotif::message( char severity, const char *message)
{
  Arg 		args[2];
  XmString	cstr;

  cstr=XmStringCreateLtoR( (char*) message, (char*) "ISO8859-1");
  XtSetArg(args[0],XmNlabelString, cstr);
  XtSetArg(args[1],XmNheight, 20);
  XtSetValues( msg_label, args, 2);
  XmStringFree( cstr);
}


void WttMotif::set_prompt( const char *prompt)
{
  Arg 		args[3];
  XmString	cstr;

  cstr=XmStringCreateLtoR( (char*) prompt, (char*) "ISO8859-1");
  XtSetArg(args[0],XmNlabelString, cstr);
  XtSetArg(args[1],XmNwidth, 50);
  XtSetArg(args[2],XmNheight, 30);
  XtSetValues( cmd_prompt, args, 3);
  XmStringFree( cstr);
}

void WttMotif::watt_new( pwr_tAttrRef aref)
{
  new WAttMotif( toplevel, this, ldhses,
	    aref, editmode,
	    wnavnode->gbl.advanced_user, 1);
}

void WttMotif::wda_new( pwr_tOid oid)
{
  new WdaMotif( toplevel, this, ldhses,
		oid, 0, "", editmode, wnav->gbl.advanced_user, 1);
}

void WttMotif::ge_new( char *graph_name)
{
  unsigned int opt = wnavnode->gbl.enable_comment ? ge_mOption_EnableComment : 0;
  new GeMotif( NULL, toplevel, ldhses, 0, opt, graph_name);
}

void WttMotif::wcast_new( pwr_tAttrRef aref, pwr_tStatus *sts) 
{
  new WCastMotif( this, toplevel, "Casting Window", ldhses, aref, sts);
}

wb_build *WttMotif::build_new()
{
  return new wb_build( *(wb_session *)ldhses, focused_wnav);
}

void WttMotif::wpkg_new()
{
  wpkg = new WPkgMotif( toplevel, this);
  wpkg->close_cb = Wtt::wpkg_quit_cb;
}

int WttMotif::ute_new( char *title)
{
  pwr_tStatus sts;
  utedctx = new WUtedMotif( this, toplevel, title, "Utilites",
			    ldh_SessionToWB(ldhses), ldhses, editmode, 
			    uted_quit_cb, &sts);
  return sts;
}

void WttMotif::close_change_value()
{
  if ( input_open) {
    XtUnmanageChild( cmd_input);
    set_prompt( "");
    input_open = 0;
  }
}

void WttMotif::open_change_value()
{
  int		sts;
  brow_tObject	*sellist;
  int		sel_cnt1, sel_cnt2;
  char 		*value = 0;
  int		multiline;
  int		input_size;

  if ( input_open) {
    XtUnmanageChild( cmd_input);
    set_prompt( "");
    input_open = 0;
    return;
  }

  sts = wnav->get_selected_nodes( &sellist, &sel_cnt1);
  if ( ODD(sts))
  {
    if ( sel_cnt1 != 1)
    {
      message('E', "Select one attribute");
      return;
    }

    sts = wnav->check_attr_value( sellist[0], &multiline, &value, &input_size);
    if ( EVEN(sts))
    {
      message( 'E', wnav_get_message( sts));
      return;
    }
    if ( multiline)
    {
      message( 'E', "Edit multiline attributes in the attribute editor");
      return;
    }

    input_node = sellist[0];
    input_wnav = wnav;
    wnav->node_to_objid( input_node, &input_objid);
    free( sellist);
  }
  else
  {
    sts = wnavnode->get_selected_nodes( &sellist, &sel_cnt2);
    if ( ODD(sts))
    {
      if ( sel_cnt2 != 1)
      {
        message('E', "Select one attribute");
        return;
      }

      sts = wnavnode->check_attr_value( sellist[0], &multiline, &value, &input_size);
      if ( EVEN(sts))
      {
        message( 'E', wnav_get_message( sts));
        return;
      }
      if ( multiline)
      {
        message( 'E', "Edit multiline attributes in the attribute editor");
        return;
      }

      input_node = sellist[0];
      input_wnav = wnavnode;
      wnavnode->node_to_objid( input_node, &input_objid);
      free( sellist);
    }
  }
  if ( sel_cnt1 == 0 && sel_cnt2 == 0)
  {
    message('E', "Select an attribute");
    return;
  }

  if ( command_open)
    command_open = 0;
  else
    XtManageChild( cmd_input);

  message( ' ', "");
  XtSetKeyboardFocus( toplevel, cmd_input);

  if ( value)
  {
    XmTextSetString( cmd_input, value);
    XmTextSetInsertionPosition( cmd_input, strlen(value));
    XmTextSetSelection( cmd_input, 0, strlen(value), CurrentTime);
  }
  else
    XmTextSetString( cmd_input, (char*) "");

  set_prompt( "value >");
  input_open = 1;
  input_mode = wtt_eInputMode_Attribute;
}

void WttMotif::open_change_name()
{
  int		sts;
  brow_tObject	*sellist;
  int		sel_cnt1, sel_cnt2;
  int           size;
  char          name[80];

  if ( input_open) 
  {
    XtUnmanageChild( cmd_input);
    set_prompt( "");
    input_open = 0;
    return;
  }

  sts = wnav->get_selected_nodes( &sellist, &sel_cnt1);
  if ( ODD(sts))
  {
    if ( sel_cnt1 != 1)
    {
      message('E', "Select one object");
      return;
    }

    sts = wnav->check_object_name( sellist[0]);
    if ( EVEN(sts))
    {
      message( 'E', wnav_get_message( sts));
      return;
    }

    input_node = sellist[0];
    input_wnav = wnav;
    wnav->node_to_objid( input_node, &input_objid);
    free( sellist);
  }
  else
  {
    sts = wnavnode->get_selected_nodes( &sellist, &sel_cnt2);
    if ( ODD(sts))
    {
      if ( sel_cnt2 != 1)
      {
        message('E', "Select one object");
        return;
      }

      sts = wnavnode->check_object_name( sellist[0]);
      if ( EVEN(sts))
      {
        message( 'E', wnav_get_message( sts));
        return;
      }

      input_node = sellist[0];
      input_wnav = wnavnode;
      wnavnode->node_to_objid( input_node, &input_objid);
      free( sellist);
    }
  }
  if ( sel_cnt1 == 0 && sel_cnt2 == 0)
  {
    message('E', "Select an object");
    return;
  }

  if ( command_open)
    command_open = 0;
  else
    XtManageChild( cmd_input);

  message( ' ', "");
  XtSetKeyboardFocus( toplevel, cmd_input);

  sts = ldh_ObjidToName( ldhses, input_objid, ldh_eName_Object, 
					   name, sizeof(name), &size); 

  XmTextSetString( cmd_input, name);
  XmTextSetInsertionPosition( cmd_input, strlen(name));
  XmTextSetSelection( cmd_input, 0, strlen(name), CurrentTime);
  set_prompt( "name >");
  input_open = 1;
  input_mode = wtt_eInputMode_ObjectName;
}

//
//  Callbackfunctions from menu entries
//
void WttMotif::activate_change_value( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->open_change_value();
}

void WttMotif::activate_command( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  if ( wtt->command_open) {
    XtUnmanageChild( ((WttMotif *)wtt)->cmd_input);
    wtt->set_prompt( "");
    wtt->command_open = 0;
    return;
  }

  if ( wtt->input_open)
    wtt->input_open = 0;
  else
    XtManageChild( ((WttMotif *)wtt)->cmd_input);
  wtt->message( ' ', "");
  XtSetKeyboardFocus( ((WttMotif *)wtt)->toplevel, ((WttMotif *)wtt)->cmd_input);

  XmTextSetString( ((WttMotif *)wtt)->cmd_input, (char*) "");
  wtt->set_prompt( "wtt >");
  wtt->command_open = 1;
}

void WttMotif::activate_exit( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  Wtt::close( wtt);
}

void WttMotif::activate_print( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_print();
}

void WttMotif::activate_collapse( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_collapse();
}

void WttMotif::activate_save( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_save();
}

void WttMotif::activate_revert( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_revert();
}

void WttMotif::activate_syntax( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_syntax();
}

void WttMotif::activate_find( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_find();
}

void WttMotif::activate_findregex( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_findregex();
}

void WttMotif::activate_findnext( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_findnext();
}

void WttMotif::activate_copy( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_copy();
}

void WttMotif::activate_cut( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_cut();
}

void WttMotif::activate_paste( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_paste();
}

void WttMotif::activate_pasteinto( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_pasteinto();
}

void WttMotif::activate_copykeep( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_copykeep();
}

void WttMotif::activate_rename( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->open_change_name();
}

void WttMotif::activate_configure( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_configure();
}

void WttMotif::activate_utilities( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  disable_set_focus( wtt, 2000);
  wtt->activate_utilities();
}

void WttMotif::activate_openobject( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_openobject();
}

void WttMotif::activate_openvolobject( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_openvolobject();
}

void WttMotif::activate_openplc( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  if ( ((WttMotif *)wtt)->avoid_deadlock)
    return;

  disable_set_focus( wtt, 2000);
  wtt->activate_openplc();
  set_avoid_deadlock( wtt, 2000);
}

void WttMotif::activate_buildobject( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_buildobject();
}

void WttMotif::activate_openvolume( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_openvolume();
}
 
void WttMotif::activate_openbuffer( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_openbuffer();
}
 
void WttMotif::activate_confproject( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_confproject();
}
 
void WttMotif::activate_openfile_dbs( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->set_clock_cursor();
  wtt->wnav->wow->CreateFileSelDia( "Loadfile Selection", (void *)wtt,
				    Wtt::file_selected_cb, wow_eFileSelType_Dbs);
  wtt->reset_cursor();
}

void WttMotif::activate_openfile_wbl( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->set_clock_cursor();
  wtt->wnav->wow->CreateFileSelDia( "Loadfile Selection", (void *)wtt,
				    Wtt::file_selected_cb, wow_eFileSelType_Wbl);
  wtt->reset_cursor();
}

void WttMotif::activate_openpl( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_openpl();
}

void WttMotif::activate_opengvl( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_opengvl();
}

void WttMotif::activate_openudb( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_openudb();
}

void WttMotif::activate_spreadsheet( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_spreadsheet();
}

void WttMotif::activate_openge( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_openge();
}

void WttMotif::activate_openclasseditor( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_openclasseditor();
}

void WttMotif::activate_buildvolume( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_buildvolume();
}

void WttMotif::activate_buildnode( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_buildnode();
}

void WttMotif::activate_distribute( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_distribute();
}

void WttMotif::activate_showcrossref( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_showcrossref();
}

void WttMotif::activate_updateclasses( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_updateclasses();
}

void WttMotif::activate_zoom_in( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_zoom_in();
}

void WttMotif::activate_zoom_out( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_zoom_out();
}

void WttMotif::activate_zoom_reset( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_zoom_reset();
}

void WttMotif::activate_twowindows( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_twowindows();
}

void WttMotif::activate_messages( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_messages();
}

void WttMotif::activate_view( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->update_options_form();
  XtManageChild( ((WttMotif *)wtt)->options_form);
}

void WttMotif::activate_savesettings( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->save_settings();
}

void WttMotif::activate_restoresettings( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->restore_settings();
}

void WttMotif::activate_scriptproj( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_scriptproj();
}

void WttMotif::activate_scriptbase( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_scriptbase();
}

void WttMotif::activate_help( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_help();
}

void WttMotif::activate_help_project( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_help_project();
}

void WttMotif::activate_help_proview( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->activate_help_proview();
}

void WttMotif::create_menubutton( Widget w, Wtt *wtt) 
{
  int key;

  XtVaGetValues (w, XmNuserData, &key, NULL);
  switch (key) 
  {
    case 1: ((WttMotif *)wtt)->menu_save_w = w; break;
    case 2: ((WttMotif *)wtt)->menu_revert_w = w; break;
    case 3: ((WttMotif *)wtt)->menu_cut_w = w; break;
    case 4: ((WttMotif *)wtt)->menu_rename_w = w; break;
    case 5: ((WttMotif *)wtt)->menu_utilities_w = w; break;
    case 6: ((WttMotif *)wtt)->menu_openplc_w = w; break;
    case 7: ((WttMotif *)wtt)->menu_buildvolume_w = w; break;
    case 8: ((WttMotif *)wtt)->menu_buildnode_w = w; break;
    case 9: ((WttMotif *)wtt)->menu_distribute_w = w; break;
    case 10: ((WttMotif *)wtt)->menu_change_value_w = w; break;
    case 11: ((WttMotif *)wtt)->menu_copy_w = w; break;
    case 12: ((WttMotif *)wtt)->menu_paste_w = w; break;
    case 13: ((WttMotif *)wtt)->menu_buildobject_w = w; break;
    case 14: ((WttMotif *)wtt)->menu_pasteinto_w = w; break;
    case 15: ((WttMotif *)wtt)->menu_edit_w = w; break;
    case 16: ((WttMotif *)wtt)->menu_classeditor_w = w; break;
    case 17: ((WttMotif *)wtt)->menu_copykeep_w = w; break;
    case 18: ((WttMotif *)wtt)->menu_updateclasses_w = w; break;
    default:
      ;
  }
}

void WttMotif::create_msg_label( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  ((WttMotif *)wtt)->msg_label = w;
}
void WttMotif::create_cmd_prompt( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  ((WttMotif *)wtt)->cmd_prompt = w;
}

static void wtt_valchanged_cmd_input( Widget w, XEvent *event)
{
  WttMotif 	*wtt;
  int 	sts;
  char 	*text;
  Arg 	args[2];

  XtSetArg(args[0], XmNuserData, &wtt);
  XtGetValues(w, args, 1);

  if ( wtt->input_open)
    sts = mrm_TextInput( w, event, (char *)wtt->value_recall, sizeof(wtt->value_recall[0]),
	sizeof( wtt->value_recall)/sizeof(wtt->value_recall[0]),
	&wtt->value_current_recall);
  else
    sts = mrm_TextInput( w, event, (char *)wtt->cmd_recall, sizeof(wtt->cmd_recall[0]),
	sizeof( wtt->cmd_recall)/sizeof(wtt->cmd_recall[0]),
	&wtt->cmd_current_recall);
  if ( sts)
  {
    text = XmTextGetString( w);
    if ( wtt->input_open)
    {
      switch( wtt->input_mode) 
      {
        case wtt_eInputMode_Attribute:
          wtt->input_wnav->select_object( wtt->input_node);
          sts = wtt->input_wnav->set_attr_value( wtt->input_node, 
		wtt->input_objid, text);
          if ( EVEN(sts))
            wtt->message( 'E', wnav_get_message( sts));
          break;
        case wtt_eInputMode_ObjectName:
          wtt->input_wnav->select_object( wtt->input_node);
          sts = wtt->input_wnav->set_object_name( wtt->input_node, 
		wtt->input_objid, text);
          if ( EVEN(sts))
            wtt->message( 'E', wnav_get_message( sts));
          break;
        default:
          ;
      }
      if ( wtt->input_open) {
	XtUnmanageChild( w);
	wtt->set_prompt( "");
	wtt->input_open = 0;
      }
    }
    else if ( wtt->command_open)
    {
      if ( !wtt->focused_wnav)
        wtt->set_focus_default();
      wtt->set_clock_cursor();
      sts = wtt->focused_wnav->command( text);
      wtt->reset_cursor();
      XtUnmanageChild( w);
      wtt->set_prompt( "");
      wtt->command_open = 0;
    }
  }
}

void WttMotif::create_cmd_input( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  Arg args[2];

  XtSetArg    (args[0], XmNuserData, wtt);
  XtSetValues (w, args, 1);

  mrm_TextInit( w, (XtActionProc) wtt_valchanged_cmd_input, mrm_eUtility_Wtt);

  ((WttMotif *)wtt)->cmd_input = w;
}
void WttMotif::create_wnav_form( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  ((WttMotif *)wtt)->wnav_form = w;
}
void WttMotif::create_palette_form( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  ((WttMotif *)wtt)->palette_form = w;
}
void WttMotif::create_india_label( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  ((WttMotif *)wtt)->india_label = w;
}
void WttMotif::create_india_text( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  ((WttMotif *)wtt)->india_text = w;
}
void WttMotif::activate_india_ok( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  char *value;

  value = XmTextGetString( ((WttMotif *)wtt)->india_text);
  XtUnmanageChild( ((WttMotif *)wtt)->india_widget);

  (wtt->india_ok_cb)( wtt, value);
}
void WttMotif::activate_india_cancel( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  XtUnmanageChild( ((WttMotif *)wtt)->india_widget);
}
void WttMotif::activate_confirm_ok( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  XtUnmanageChild( ((WttMotif *)wtt)->confirm_widget);
  wtt->confirm_open = 0;
  if ( wtt->confirm_ok_cb)
    (wtt->confirm_ok_cb)( wtt);
}
void WttMotif::activate_confirm_no( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  XtUnmanageChild( ((WttMotif *)wtt)->confirm_widget);
  wtt->confirm_open = 0;
  if ( wtt->confirm_no_cb)
    (wtt->confirm_no_cb)( wtt);
}
void WttMotif::activate_confirm_cancel( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  wtt->confirm_open = 0;
  XtUnmanageChild( ((WttMotif *)wtt)->confirm_widget);
}

void WttMotif::enable_set_focus( Wtt *wtt)
{
  ((WttMotif *)wtt)->set_focus_disabled--;
}

void WttMotif::disable_set_focus( Wtt *wtt, int time)
{
  ((WttMotif *)wtt)->set_focus_disabled++;
  ((WttMotif *)wtt)->disfocus_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext( ((WttMotif *)wtt)->toplevel), time,
	(XtTimerCallbackProc)WttMotif::enable_set_focus, wtt);
}

void WttMotif::reset_avoid_deadlock( Wtt *wtt)
{
  ((WttMotif *)wtt)->avoid_deadlock = 0;
}

void WttMotif::set_avoid_deadlock( Wtt *wtt, int time)
{
  ((WttMotif *)wtt)->avoid_deadlock = 1;
  ((WttMotif *)wtt)->deadlock_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext( ((WttMotif *)wtt)->toplevel), time,
	(XtTimerCallbackProc)WttMotif::reset_avoid_deadlock, wtt);
}

void WttMotif::action_inputfocus( Widget w, XmAnyCallbackStruct *data)
{
  Arg args[1];
  Wtt *wtt;

  XtSetArg    (args[0], XmNuserData, &wtt);
  XtGetValues (w, args, 1);

  if ( mrm_IsIconicState(w))
    return;

  if ( ((WttMotif *)wtt)->set_focus_disabled)
    return;

  if ( wtt->focused_wnav) {
    wtt->set_focus( wtt->focused_wnav);
    // wtt->focus_timerid = XtAppAddTimeOut( XtWidgetToApplicationContext( wtt->toplevel), 200,
    //	(XtTimerCallbackProc)wtt_set_focus_proc, wtt);
  }
  disable_set_focus( wtt, 400);
}

void WttMotif::open_input_dialog( const char *text, const char *title,
			     const char *init_text,
			     void (*ok_cb)( Wtt *, char *))
{
  Arg		args[10];
  int 		i;
  XmString	cstr;

  i = 0;
  XtSetArg(args[0], XmNlabelString,
		cstr=XmStringCreateLtoR( (char*) text, (char*) "ISO8859-1") ); i++;
  XtSetValues( india_label, args, i);
  XmStringFree( cstr);
  i = 0;
  XtSetArg(args[0], XmNdialogTitle,
		cstr=XmStringCreateLtoR( (char*) title, (char*) "ISO8859-1") ); i++;
  XtSetValues( india_widget, args, i);
  XmStringFree( cstr);

  XmTextSetString( india_text, (char*) init_text);

  XtManageChild( india_widget);

  XmProcessTraversal( india_text, XmTRAVERSE_CURRENT);

  india_ok_cb = ok_cb;
}

void WttMotif::open_confirm( const char *text, const char *title, 
	void (*ok_cb)( Wtt *), void (*no_cb)( Wtt *))
{
  Arg 		args[3];
  XmString	cstr;

  if ( confirm_open)  {
    XtUnmanageChild( confirm_widget);
    confirm_open = 0;
    return;
  }

  XtManageChild( confirm_widget);

  message( ' ', "");

  XtSetArg(args[0],XmNmessageString, XmStringCreateLtoR( (char*) text, (char*) "ISO8859-1"));
  XtSetArg(args[1], XmNdialogTitle,
		cstr=XmStringCreateLtoR( (char*) title, (char*) "ISO8859-1") );
  XtSetValues( confirm_widget, args, 2);
  XmStringFree( cstr);
  confirm_open = 1;
  confirm_ok_cb = ok_cb;
  confirm_no_cb = no_cb;
}

void WttMotif::activate_selmode( Widget w, Wtt *wtt, 
			    XmToggleButtonCallbackStruct *data)
{
  String name;
    
  name = XtName(w);
    
  if (!strcmp(name,"normalSyntax")) {
    if (!data->set) {
      XmToggleButtonSetState( w, 1, 0);
      return;
    }
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_gms_syntax, 0, 0);
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_extern_syntax, 0, 0);
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_extern_syntax, 0, 0);
    XtSetSensitive( ((WttMotif *)wtt)->cm_add_type, 1);
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_add_type, 0, 0);
    wtt->select_type = 0;
    XtSetSensitive( ((WttMotif *)wtt)->cm_add_volume, 1);
    wtt->select_syntax = wnav_eSelectionMode_Normal;
  }
  else if (!strcmp(name,"gmsSyntax")) {
    if (!data->set) {
      XmToggleButtonSetState( w, 1, 0);
      return;
    }
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_normal_syntax, 0, 0);
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_extern_syntax, 0, 0);
    XtSetSensitive( ((WttMotif *)wtt)->cm_add_type, 1);
    XtSetSensitive( ((WttMotif *)wtt)->cm_add_volume, 1);
    wtt->select_syntax = wnav_eSelectionMode_GMS;
  }
  else if (!strcmp(name,"externSyntax")) {
    if (!data->set) {
      XmToggleButtonSetState( w, 1, 0);
      return;
    }
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_normal_syntax, 0, 0);
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_gms_syntax, 0, 0);
    XtSetSensitive( ((WttMotif *)wtt)->cm_add_type, 0);
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_add_type, 0, 0);
    XtSetSensitive( ((WttMotif *)wtt)->cm_add_volume, 0);
    wtt->select_type = 0;
    XmToggleButtonSetState( ((WttMotif *)wtt)->cm_add_volume, 0, 0);
    wtt->select_syntax = wnav_eSelectionMode_Extern;
    wtt->select_volume = 0;
  }
  else if (!strcmp(name,"addVolume"))
    wtt->select_volume = data->set;
  else if (!strcmp(name,"addAttribute"))
    wtt->select_attr = data->set;
  else if (!strcmp(name,"addType")) {
    wtt->select_type = data->set;
    if ( data->set && !wtt->select_attr) {
      XmToggleButtonSetState( ((WttMotif *)wtt)->cm_add_attribute, 1, 0);
      wtt->select_attr = 1;
    }
  }
}

void WttMotif::create_selmode( Widget w, Wtt *wtt, 
			  XmToggleButtonCallbackStruct *data)
{
  XmToggleButtonCallbackStruct cbs;
  String name;

  name = XtName(w);
    
  if (!strcmp(name,"normalSyntax"))
  {
      ((WttMotif *)wtt)->cm_normal_syntax = w;
      XmToggleButtonSetState( w, 1, 0);
  }
  else if (!strcmp(name,"gmsSyntax"))
    ((WttMotif *)wtt)->cm_gms_syntax = w;
  else if (!strcmp(name,"externSyntax"))
    ((WttMotif *)wtt)->cm_extern_syntax = w;
  else if (!strcmp(name,"addVolume"))
    ((WttMotif *)wtt)->cm_add_volume = w;
  else if (!strcmp(name,"addAttribute"))
    ((WttMotif *)wtt)->cm_add_attribute = w;
  else if (!strcmp(name,"addType"))
  {
    ((WttMotif *)wtt)->cm_add_type = w;

    // Register button which is set
    cbs.reason = XmCR_ACTIVATE;
    cbs.event = NULL;
    cbs.set = True;
    activate_selmode( ((WttMotif *)wtt)->cm_normal_syntax, wtt, &cbs);
  }
}

// Open a Create Boot File window
void WttMotif::open_boot_window()
{
  if ( boot_dia != 0) {
    XRaiseWindow( flow_Display(boot_dia), flow_Window(XtParent(boot_dia)));
    return;
  }

  XmString cstr;  
  int i;
  lfu_t_volumelist *volumelist_ptr;
  pwr_tString40 nodename;
  pwr_tStatus 	sts;
  
/* DRM database hierarchy related variables */
  char		uid_filename[200] = {"pwr_exe:wb_wtt.uid"};
  char		*uid_filename_p = uid_filename;

  static MrmRegisterArg reglist[] =
  {
  /* First the context variable */
    	{(char*) "wtt_ctx", NULL },
	{(char*) "wtt_boot_list_cr",(caddr_t)WttMotif::boot_list_cr },
	{(char*) "wtt_boot_ok_cb",(caddr_t)WttMotif::boot_ok_cb },
	{(char*) "wtt_boot_cancel_cb",(caddr_t)WttMotif::boot_cancel_cb },
	{(char*) "wtt_boot_destroy_cb",(caddr_t)WttMotif::boot_destroy_cb },
	};
  static int reglist_num = XtNumber(reglist);

  // Load the bootlist
  sts = lfu_volumelist_load( load_cNameBootList, 
		(lfu_t_volumelist **) &boot_volumelist,
		&boot_volumecount);
  if (sts == FOE__NOFILE) {
    message( 'E', "Project is not configured");
    return;
  }
  else if (EVEN(sts)) {
    message( 'E', "Syntax error in bootlist file");
    return;
  }

  if (boot_dia == NULL) {
    sts = dcli_translate_filename( uid_filename, uid_filename);
    if ( EVEN(sts)) return;

    reglist[0].value = (caddr_t) this;
    uilutil_fetch( 
      	&uid_filename_p,
      	1,
      	reglist, 
      	reglist_num,
      	toplevel,
      	(char*) "bootFilesWindow", 
     	(char*) "bootFilesWindow", 
     	 NULL, 
     	 0,
     	 &boot_dia, 
     	 NULL 
     	 );

    XtVaSetValues(
      XtParent(boot_dia), 
      XmNdeleteResponse, XmDESTROY, 
      NULL
      ); 
  }

  strcpy( nodename, "");
  volumelist_ptr = (lfu_t_volumelist *) boot_volumelist;
  for ( i = 0; i < boot_volumecount; i++)
  {
    if ( strcmp(volumelist_ptr->p1, nodename))
    {
      strcpy( nodename, volumelist_ptr->p1);
      cstr = XmStringCreateSimple( nodename);
      XmListAddItemUnselected(boot_list, cstr, 0);
      XmStringFree(cstr);
    }
    volumelist_ptr++;
  }

  XtManageChild( boot_dia);
  return;
}

// Callbacks for the Create Boot Files window

// Create callback for the list widget
void WttMotif::boot_list_cr(Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
    ((WttMotif *)wtt)->boot_list = w;
}

// Widget callbacks for Cancel button
void WttMotif::boot_cancel_cb(Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
    XtDestroyWidget( ((WttMotif *)wtt)->boot_dia);
}

// Destroy callback
void WttMotif::boot_destroy_cb(Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
    if ( wtt->boot_volumelist && wtt->boot_volumecount) {
      free( (char *) wtt->boot_volumelist);
      wtt->boot_volumelist = 0;
    }
    ((WttMotif *)wtt)->boot_dia = 0;
}

// Widget callbacks for OK button
void WttMotif::boot_ok_cb(Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
    int *pos_list, pos_cnt;
    int i, j, nodecount;
    lfu_t_volumelist *volumelist_ptr;
    pwr_tString40 nodeconfigname;
    char msg[200];
    int bootfile_count;

    if ( !wtt->focused_wnav)
      wtt->set_focus_default();

    wtt->message( ' ', "");

    nodecount = 0;
    bootfile_count = 0;
    volumelist_ptr = (lfu_t_volumelist *)wtt->boot_volumelist;
    if (XmListGetSelectedPos( ((WttMotif *)wtt)->boot_list, &pos_list, &pos_cnt)) {
      for (i = 0; i < pos_cnt; i++) {
        strcpy( nodeconfigname, "");
        for ( j = 0; j < wtt->boot_volumecount; j++) {
          if ( strcmp(volumelist_ptr->p1, nodeconfigname)) {
            nodecount++;

            strcpy( nodeconfigname, volumelist_ptr->p1);
	    if ( nodecount == pos_list[i]) {
	      wb_build build( *(wb_session *)wtt->ldhses, wtt->focused_wnav);
	      build.opt = wtt->focused_wnav->gbl.build;
	      build.node( nodeconfigname, 
			  wtt->boot_volumelist, wtt->boot_volumecount);
	      if ( build.evenSts()) {
                XtDestroyWidget( ((WttMotif *)wtt)->boot_dia);
	        sprintf( msg, 
			"Error creating bootfile for NodeConfig-object %s",
			nodeconfigname);
		wtt->message( 'E', msg);
		return;
	      }
	      else if ( build.sts() != PWRB__NOBUILT)
		bootfile_count++;
              volumelist_ptr++;
	      break;
	    }
          }
          volumelist_ptr++;
        }
      }
    }

    XtDestroyWidget(((WttMotif *)wtt)->boot_dia);
    if ( !bootfile_count) {
      wtt->message( 'E', "Nothing to build");
      return;
    }
    sprintf( msg, "Bootfile%s created", (bootfile_count == 1) ? "":"s");
    wtt->message( 'I', msg);
	  
}

/************************************************************************
*
* Name: update_options_form
*
* Description: This routine must be called when the configurator/navigator is 
*	       managed.
* 
*************************************************************************/
void WttMotif::update_options_form()
{

  // Hierarchies
  XmToggleButtonSetState(show_plant_w, 
			  (Boolean) wnav_mapped, False);
  XmToggleButtonSetState(show_node_w, 
			  (Boolean) wnavnode_mapped, False);

  // entry components
  XmToggleButtonSetState( show_class_w, (Boolean) show_class, False);
  XmToggleButtonSetState( show_alias_w, (Boolean) show_alias, False);
  XmToggleButtonSetState( show_descrip_w, (Boolean) show_descrip, False);
  XmToggleButtonSetState( show_objref_w, (Boolean) show_objref, False);
  XmToggleButtonSetState( show_objxref_w, (Boolean) show_objxref, False);
  XmToggleButtonSetState( show_attrref_w, (Boolean) show_attrref, False);
  XmToggleButtonSetState( show_attrxref_w, (Boolean) show_attrxref, False);
  XmToggleButtonSetState( build_force_w, (Boolean) build_force, False);
  XmToggleButtonSetState( build_debug_w, (Boolean) build_debug, False);
  XmToggleButtonSetState( build_crossref_w, (Boolean) build_crossref, False);
  XmToggleButtonSetState( build_manual_w, (Boolean) build_manual, False);
} 


/************************************************************************
*
* Name: set_options
*
*
*************************************************************************/
void WttMotif::set_options()
{
  show_class = XmToggleButtonGetState(show_class_w);
  show_alias = XmToggleButtonGetState(show_alias_w);
  show_descrip = XmToggleButtonGetState(show_descrip_w);
  show_objref = XmToggleButtonGetState(show_objref_w);
  show_objxref = XmToggleButtonGetState(show_objxref_w);
  show_attrref = XmToggleButtonGetState(show_attrref_w);
  show_attrxref = XmToggleButtonGetState(show_attrxref_w);
  build_force = XmToggleButtonGetState(build_force_w);
  build_debug = XmToggleButtonGetState(build_debug_w);
  build_crossref = XmToggleButtonGetState(build_crossref_w);
  build_manual = XmToggleButtonGetState(build_manual_w);

  wnav->set_options( show_class, show_alias, 
	show_descrip, show_objref, show_objxref, 
	show_attrref, show_attrxref, build_force, build_debug,
	build_crossref, build_manual);
  wnavnode->set_options( show_class, show_alias, 
	show_descrip, show_objref, show_objxref, 
        show_attrref, show_attrxref, build_force, build_debug,
	build_crossref, build_manual);

}

// Callbacks from the options form

void WttMotif::options_act_but_cb( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  int key;
  int plant, node;
  
  wtt->message( ' ', "");

  plant = XmToggleButtonGetState( ((WttMotif *)wtt)->show_plant_w);
  node = XmToggleButtonGetState( ((WttMotif *)wtt)->show_node_w);

  XtVaGetValues(w, XmNuserData, &key, NULL);
  switch (key) {
  case 1 : /* OK */
    XtUnmanageChild( ((WttMotif *)wtt)->options_form);
  case 2 : /* Apply */
    wtt->set_twowindows( 0, plant, node);
    wtt->set_options();
    break;

  case 3 : /* Cancel */
    wtt->update_options_form();
    XtUnmanageChild( ((WttMotif *)wtt)->options_form);
    break;
  }


}

void WttMotif::options_form_cr( Widget w, Wtt *wtt, XmAnyCallbackStruct *data) 
{
  ((WttMotif *)wtt)->options_form = w;
}

void WttMotif::options_entry_tog_cr( Widget w, Wtt *wtt, XmAnyCallbackStruct *data) 
{
  int key;

  XtVaGetValues (w, XmNuserData, &key, NULL);
  switch (key) {
  case 1: /* Class */
    ((WttMotif *)wtt)->show_class_w = w;
    wtt->show_class = XmToggleButtonGetState(w);
    break;
  case 2: /* Alias */
    ((WttMotif *)wtt)->show_alias_w = w;
    wtt->show_alias = XmToggleButtonGetState(w);
    break;
  case 3: /* Description */
    ((WttMotif *)wtt)->show_descrip_w = w;
    wtt->show_descrip = XmToggleButtonGetState(w);
    break;
  case 4: /* Objref */
    ((WttMotif *)wtt)->show_objref_w = w;
    wtt->show_objref = XmToggleButtonGetState(w);
    break;
  case 5: /* Objxref */
    ((WttMotif *)wtt)->show_objxref_w = w;
    wtt->show_objxref = XmToggleButtonGetState(w);
    break;
  case 6: /* Attrref */
    ((WttMotif *)wtt)->show_attrref_w = w;
    wtt->show_attrref = XmToggleButtonGetState(w);
    break;
  case 7: /* Attrxref */
    ((WttMotif *)wtt)->show_attrxref_w = w;
    wtt->show_attrxref = XmToggleButtonGetState(w);
    break;
  case 8:
    ((WttMotif *)wtt)->build_force_w = w;
    wtt->build_force = XmToggleButtonGetState(w);
    break;
  case 9:
    ((WttMotif *)wtt)->build_debug_w = w;
    wtt->build_debug = XmToggleButtonGetState(w);
    break;
  case 10:
    ((WttMotif *)wtt)->build_crossref_w = w;
    wtt->build_crossref = XmToggleButtonGetState(w);
    break;
  case 11:
    ((WttMotif *)wtt)->build_manual_w = w;
    wtt->build_manual = XmToggleButtonGetState(w);
    break;
  default:
    break;
  }
}

void WttMotif::options_hier_tog_cr( Widget w, Wtt *wtt, XmAnyCallbackStruct *data) 
{
  int key;

  XtVaGetValues (w, XmNuserData, &key, NULL);
  switch (key) {
  case 1: /* Plant Hier */
    ((WttMotif *)wtt)->show_plant_w = w;
    break;
  case 2: /* Node Hier */
    ((WttMotif *)wtt)->show_node_w = w;
    break;
  default:
    ;
  }
}


WttMotif::WttMotif( 
	void	*wt_parent_ctx,
	Widget 	wt_parent_wid,
	const char 	*wt_name,
	const char	*iconname,
	void	*wt_wbctx,
	pwr_tVolumeId wt_volid,
	ldh_tVolume wt_volctx,
	wnav_sStartMenu *root_menu,
	pwr_tStatus *status
	) :
  Wtt(wt_parent_ctx, wt_name, iconname, wt_wbctx, wt_volid, wt_volctx, root_menu, status),
  parent_wid( wt_parent_wid), boot_dia(0),
  set_focus_disabled(0), disfocus_timerid(0), selection_timerid(0), avoid_deadlock(0),
  clock_cursor(0), cmd_current_recall(0), value_current_recall(0)
{
  char		uid_filename[200] = {"pwr_exe:wb_wtt.uid"};
  char		*uid_filename_p = uid_filename;
  Arg 		args[20];
  pwr_tStatus	sts;
  char 		title[80];
  int		i;
  MrmHierarchy 	s_DRMh;
  MrmType 	dclass;
  Widget	wtt_widget;
  char		volname[80];
  pwr_tClassId	volclass;
  int		size;
  char		layout_w1[80];
  char		layout_w2[80];
  char		layout_palette[80];
  char		title_w1[40];
  char		title_w2[40];
  int		hide_wnavnode = 0;

  static char translations[] = "\
<FocusIn>: wtt_inputfocus()\n";
  static XtTranslations compiled_translations = NULL;

  static XtActionsRec actions[] =
  {
    {(char*) "wtt_inputfocus",      (XtActionProc) WttMotif::action_inputfocus}
  };

  static MrmRegisterArg	reglist[] = {
        {(char*) "wtt_ctx", 0 },
	{(char*) "wtt_activate_exit",(caddr_t)WttMotif::activate_exit },
	{(char*) "wtt_activate_collapse",(caddr_t)WttMotif::activate_collapse },
	{(char*) "wtt_activate_save",(caddr_t)WttMotif::activate_save },
	{(char*) "wtt_activate_revert",(caddr_t)WttMotif::activate_revert },
	{(char*) "wtt_activate_syntax",(caddr_t)WttMotif::activate_syntax },
	{(char*) "wtt_activate_print",(caddr_t)WttMotif::activate_print },
	{(char*) "wtt_activate_find",(caddr_t)WttMotif::activate_find },
	{(char*) "wtt_activate_findregex",(caddr_t)WttMotif::activate_findregex },
	{(char*) "wtt_activate_findnext",(caddr_t)WttMotif::activate_findnext },
	{(char*) "wtt_activate_selmode",(caddr_t)WttMotif::activate_selmode },
	{(char*) "wtt_activate_cut",(caddr_t)WttMotif::activate_cut },
	{(char*) "wtt_activate_copy",(caddr_t)WttMotif::activate_copy },
	{(char*) "wtt_activate_paste",(caddr_t)WttMotif::activate_paste },
	{(char*) "wtt_activate_pasteinto",(caddr_t)WttMotif::activate_pasteinto },
	{(char*) "wtt_activate_copykeep",(caddr_t)WttMotif::activate_copykeep },
	{(char*) "wtt_activate_rename",(caddr_t)WttMotif::activate_rename },
	{(char*) "wtt_activate_configure",(caddr_t)WttMotif::activate_configure },
	{(char*) "wtt_activate_utilities",(caddr_t)WttMotif::activate_utilities },
	{(char*) "wtt_activate_openplc",(caddr_t)WttMotif::activate_openplc },
	{(char*) "wtt_activate_openobject",(caddr_t)WttMotif::activate_openobject },
	{(char*) "wtt_activate_openvolobject",(caddr_t)WttMotif::activate_openvolobject },
	{(char*) "wtt_activate_confproject",(caddr_t)WttMotif::activate_confproject },
	{(char*) "wtt_activate_openvolume",(caddr_t)WttMotif::activate_openvolume },
	{(char*) "wtt_activate_openbuffer",(caddr_t)WttMotif::activate_openbuffer },
	{(char*) "wtt_activate_openfile_dbs",(caddr_t)WttMotif::activate_openfile_dbs },
	{(char*) "wtt_activate_openfile_wbl",(caddr_t)WttMotif::activate_openfile_wbl },
	{(char*) "wtt_activate_openpl",(caddr_t)WttMotif::activate_openpl },
	{(char*) "wtt_activate_opengvl",(caddr_t)WttMotif::activate_opengvl },
	{(char*) "wtt_activate_openudb",(caddr_t)WttMotif::activate_openudb },
	{(char*) "wtt_activate_spreadsheet",(caddr_t)WttMotif::activate_spreadsheet },
	{(char*) "wtt_activate_openge",(caddr_t)WttMotif::activate_openge },
	{(char*) "wtt_activate_openclasseditor",(caddr_t)WttMotif::activate_openclasseditor },
	{(char*) "wtt_activate_buildobject",(caddr_t)WttMotif::activate_buildobject },
	{(char*) "wtt_activate_buildvolume",(caddr_t)WttMotif::activate_buildvolume },
	{(char*) "wtt_activate_buildnode",(caddr_t)WttMotif::activate_buildnode },
	{(char*) "wtt_activate_distribute",(caddr_t)WttMotif::activate_distribute },
	{(char*) "wtt_activate_updateclasses",(caddr_t)WttMotif::activate_updateclasses },
	{(char*) "wtt_activate_showcrossref",(caddr_t)WttMotif::activate_showcrossref },
	{(char*) "wtt_activate_change_value",(caddr_t)WttMotif::activate_change_value },
	{(char*) "wtt_activate_command",(caddr_t)WttMotif::activate_command },
	{(char*) "wtt_activate_zoom_in",(caddr_t)WttMotif::activate_zoom_in },
	{(char*) "wtt_activate_zoom_out",(caddr_t)WttMotif::activate_zoom_out },
	{(char*) "wtt_activate_zoom_reset",(caddr_t)WttMotif::activate_zoom_reset },
	{(char*) "wtt_activate_twowindows",(caddr_t)WttMotif::activate_twowindows },
	{(char*) "wtt_activate_messages",(caddr_t)WttMotif::activate_messages },
	{(char*) "wtt_activate_view",(caddr_t)WttMotif::activate_view },
	{(char*) "wtt_activate_savesettings",(caddr_t)WttMotif::activate_savesettings },
	{(char*) "wtt_activate_restoresettings",(caddr_t)WttMotif::activate_restoresettings },
	{(char*) "wtt_activate_scriptproj",(caddr_t)WttMotif::activate_scriptproj },
	{(char*) "wtt_activate_scriptbase",(caddr_t)WttMotif::activate_scriptbase },
	{(char*) "wtt_activate_help",(caddr_t)WttMotif::activate_help },
	{(char*) "wtt_activate_help_project",(caddr_t)WttMotif::activate_help_project },
	{(char*) "wtt_activate_help_proview",(caddr_t)WttMotif::activate_help_proview },
	{(char*) "wtt_create_menubutton",(caddr_t)WttMotif::create_menubutton },
	{(char*) "wtt_create_msg_label",(caddr_t)WttMotif::create_msg_label },
	{(char*) "wtt_create_cmd_prompt",(caddr_t)WttMotif::create_cmd_prompt },
	{(char*) "wtt_create_cmd_input",(caddr_t)WttMotif::create_cmd_input },
	{(char*) "wtt_create_palette_form",(caddr_t)WttMotif::create_palette_form },
	{(char*) "wtt_create_wnav_form",(caddr_t)WttMotif::create_wnav_form },
	{(char*) "wtt_create_selmode",(caddr_t)WttMotif::create_selmode },
	{(char*) "wtt_activate_india_ok",(caddr_t)WttMotif::activate_india_ok },
	{(char*) "wtt_activate_india_cancel",(caddr_t)WttMotif::activate_india_cancel },
	{(char*) "wtt_create_india_label",(caddr_t)WttMotif::create_india_label },
	{(char*) "wtt_create_india_text",(caddr_t)WttMotif::create_india_text },
	{(char*) "wtt_activate_confirm_ok",(caddr_t)WttMotif::activate_confirm_ok },
	{(char*) "wtt_activate_confirm_no",(caddr_t)WttMotif::activate_confirm_no },
	{(char*) "wtt_activate_confirm_cancel",(caddr_t)WttMotif::activate_confirm_cancel },
	{(char*) "wtt_options_form_cr",(caddr_t)WttMotif::options_form_cr },
	{(char*) "wtt_options_entry_tog_cr",(caddr_t)WttMotif::options_entry_tog_cr },
	{(char*) "wtt_options_hier_tog_cr",(caddr_t)WttMotif::options_hier_tog_cr },
	{(char*) "wtt_options_act_but_cb",(caddr_t)WttMotif::options_act_but_cb }
	};

  static int	reglist_num = (sizeof reglist / sizeof reglist[0]);

  for ( int i = 0; i < int(sizeof(cmd_recall)/sizeof(cmd_recall[0])); i++)
    cmd_recall[i][0] = 0;
  for ( int i = 0; i < int(sizeof(value_recall)/sizeof(value_recall[0])); i++)
    value_recall[i][0] = 0;

  sts = dcli_translate_filename( uid_filename, uid_filename);
  if ( EVEN(sts)) {
    printf( "** pwr_exe is not defined\n");
    exit(0);
  }

  if ( wbctx && volid) {
    // Get the volume class and decide what type of navigator */
    sts = ldh_GetVolumeClass( wbctx, volid, &volclass);
    if (EVEN(sts)) {
      *status = sts;
      return;
    }
    if ( volid == ldh_cDirectoryVolume) volclass = pwr_eClass_DirectoryVolume; // Fix

    sts = ldh_VolumeIdToName( wbctx, volid, volname,
		sizeof(volname), &size);
    if (EVEN(sts)) {
      *status = sts;
      return;
    }
    if ( !volctx) {
      sts = ldh_AttachVolume( wbctx, volid, &volctx);
      if (EVEN(sts)) {
        printf("-- Volume '%s' is already attached\n", volname);
        putchar( '\7' );
        *status = sts;
        return;
      }
    }

    sts = ldh_OpenSession( &ldhses, volctx, ldh_eAccess_ReadOnly,
    	ldh_eUtility_Navigator);
    if (EVEN(sts)) {
      printf("Navigator: Can't open session\n");
      *status = sts;
      return;
    }
    ldh_AddOtherSessionCallback( ldhses,  (void *)this, 
		Wtt::ldh_other_session_cb);

    switch( volclass) {
    case pwr_eClass_DirectoryVolume:
      wb_type = wb_eType_Directory;
      sprintf( title, "PwR Navigator Directory %s", name);
      strcpy( layout_w1, "ProjectNavigatorW1");
      strcpy( layout_w2, "ProjectNavigatorW2");
      strcpy( layout_palette, "ProjectNavigatorPalette");
      strcpy( title_w1, "Volume Configuration");
      strcpy( title_w2, "Node Configuration");
      break;
    case pwr_eClass_ClassVolume:
    case pwr_eClass_DetachedClassVolume:
      if ( ldh_VolRepType( ldhses) == ldh_eVolRep_Mem) {
	wb_type = wb_eType_ClassEditor;
	sprintf( title, "PwR ClassEditor Volume %s, %s", volname, name);
      }
      else {
	wb_type = wb_eType_Class;
	sprintf( title, "PwR Navigator Volume %s, %s", volname, name);
      }
      strcpy( layout_w1, "ClassNavigatorW1");
      strcpy( layout_w2, "ClassNavigatorW2");
      strcpy( layout_palette, "ClassNavigatorPalette");
      strcpy( title_w1, "Class Configuration");
      strcpy( title_w2, "Node Configuration");
      break;
    case pwr_eClass_VolatileVolume:
      wb_type = wb_eType_Buffer;
      strcpy( layout_w1, "");
      strcpy( layout_w2, "");
      strcpy( layout_palette, "NavigatorPalette");
      strcpy( title_w1, "Plant Configuration");
      strcpy( title_w2, "Node Configuration");
      sprintf( title, "PwR Navigator Buffer %s, %s", volname, name);
      hide_wnavnode = 1;
      break;
    case pwr_eClass_ExternVolume: {
      switch ( volid) {
      case ldh_cProjectListVolume:
	wb_type = wb_eType_ExternVolume;
	strcpy( layout_w1, "PrListNavigatorW1");
	strcpy( layout_w2, "PrListNavigatorW1");
	strcpy( layout_palette, "PrListNavigatorPalette");
	strcpy( title_w1, "Project List");
	strcpy( title_w2, "");
	sprintf( title, "PwR Project List");
	hide_wnavnode = 1;
	break;
      case ldh_cGlobalVolumeListVolume:
	wb_type = wb_eType_ExternVolume;
	strcpy( layout_w1, "GvListNavigatorW1");
	strcpy( layout_w2, "GvListNavigatorW1");
	strcpy( layout_palette, "GvListNavigatorPalette");
	strcpy( title_w1, "Global Volume List");
	strcpy( title_w2, "");
	sprintf( title, "PwR Global Volume List");
	hide_wnavnode = 1;
	break;
      case ldh_cUserDatabaseVolume:
	wb_type = wb_eType_ExternVolume;
	strcpy( layout_w1, "UserDbNavigatorW1");
	strcpy( layout_w2, "UserDbNavigatorW1");
	strcpy( layout_palette, "UserDbNavigatorPalette");
	strcpy( title_w1, "User Database");
	strcpy( title_w2, "");
	sprintf( title, "PwR User Database");
	hide_wnavnode = 1;
	break;
      default:
	wb_type = wb_eType_ExternVolume;
	strcpy( layout_w1, "NavigatorW1");
	strcpy( layout_w2, "NavigatorW2");
	strcpy( layout_palette, "NavigatorPalette");
	strcpy( title_w1, "Plant Configuration");
	strcpy( title_w2, "Node Configuration");
	sprintf( title, "PwR Navigator Volume %s, %s", volname, name);
      }
      break;
    }
    default:
      wb_type = wb_eType_Volume;
      strcpy( layout_w1, "NavigatorW1");
      strcpy( layout_w2, "NavigatorW2");
      strcpy( layout_palette, "NavigatorPalette");
      strcpy( title_w1, "Plant Configuration");
      strcpy( title_w2, "Node Configuration");
      sprintf( title, "PwR Navigator Volume %s, %s", volname, name);
    }
  }
  else {
    strcpy( layout_w1, "NavigatorW1");
    strcpy( layout_w2, "NavigatorW2");
    strcpy( title_w1, "Plant Configuration");
    strcpy( title_w2, "Node Configuration");
    strcpy( title, "PwR Wtt");
  }

  // Motif
  MrmInitialize();


  toplevel = XtCreatePopupShell( title, 
		topLevelShellWidgetClass, parent_wid, args, 0);

  reglist[0].value = (caddr_t) this;

  if (compiled_translations == NULL) 
    XtAppAddActions( XtWidgetToApplicationContext(toplevel), 
						actions, XtNumber(actions));
 
  // Save the context structure in the widget
  i = 0;
  XtSetArg( args[i], XmNuserData, (unsigned int) this);i++;

  sts = MrmOpenHierarchy( 1, &uid_filename_p, NULL, &s_DRMh);
  if (sts != MrmSUCCESS) printf("can't open %s\n", uid_filename);

  MrmRegisterNames(reglist, reglist_num);

  sts = MrmFetchWidgetOverride( s_DRMh, (char*) "wtt_window", toplevel,
			name, args, i, &wtt_widget, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch %s\n", name);

  sts = MrmFetchWidget(s_DRMh, (char*) "input_dialog", toplevel,
		&india_widget, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch input dialog\n");

  sts = MrmFetchWidget(s_DRMh, (char*) "confirm_dialog", toplevel,
		&confirm_widget, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch confirm dialog\n");

  sts = MrmFetchWidget(s_DRMh, (char*) "optionsForm", toplevel,
		&options_form, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch options dialog\n");

  MrmCloseHierarchy(s_DRMh);


  if (compiled_translations == NULL) 
    compiled_translations = XtParseTranslationTable(translations);
  XtOverrideTranslations( wtt_widget, compiled_translations);

  i = 0;
  XtSetArg(args[i],XmNwidth,600);i++;
  XtSetArg(args[i],XmNheight,600);i++;
  XtSetArg(args[i], XmNdeleteResponse, XmDO_NOTHING);i++;
  XtSetValues( toplevel ,args,i);
    
  XtManageChild( wtt_widget);
  XtUnmanageChild( cmd_input);

  wnav = new WNavMotif( this, wnav_form, title_w2, layout_w1,
		&wnav_brow_widget, ldhses, root_menu, 
		wnav_eWindowType_W1, &sts);
  wnav->message_cb = &Wtt::message_cb;
  wnav->close_cb = &Wtt::close;
  wnav->change_value_cb = &Wtt::change_value;
  wnav->get_wbctx_cb = &Wtt::get_wbctx;
  wnav->attach_volume_cb = &Wtt::attach_volume_cb;
  wnav->detach_volume_cb = &Wtt::detach_volume_cb;
  wnav->get_palette_select_cb = &Wtt::get_palette_select_cb;
  wnav->set_focus_cb = &Wtt::set_focus_cb;
  wnav->traverse_focus_cb = &Wtt::traverse_focus;
  wnav->set_twowindows_cb = &Wtt::set_twowindows_cb;
  wnav->set_configure_cb = &Wtt::configure_cb;
  wnav->gbl_command_cb = &Wtt::gbl_command_cb;
  wnav->create_popup_menu_cb = &Wtt::create_popup_menu_cb;
  wnav->save_cb = &Wtt::save_cb;
  wnav->revert_cb = &Wtt::revert_cb;
  wnav->script_filename_cb = &Wtt::script_filename_cb;
  wnav->format_selection_cb = Wtt::format_selection;
  wnav->get_global_select_cb = Wtt::get_global_select_cb;
  wnav->global_unselect_objid_cb = Wtt::global_unselect_objid_cb;
  wnav->set_window_char_cb = Wtt::set_window_char_cb;
  wnav->open_vsel_cb = Wtt::open_vsel_cb;
  focused_wnav = wnav;
  wnav_mapped = 1;

  i = 0;
  XtSetArg(args[i], XmNtraversalOn, True);i++;
  XtSetArg(args[i], XmNnavigationType, XmTAB_GROUP);i++;
  XtSetValues( wnav_brow_widget, args,i);

  wnavnode = new WNavMotif( this, wnav_form, title_w2, layout_w2,
		&wnavnode_brow_widget, ldhses, root_menu, 
		wnav_eWindowType_W2, &sts);
  wnavnode->message_cb = &Wtt::message_cb;
  wnavnode->close_cb = &Wtt::close;
  wnavnode->change_value_cb = &Wtt::change_value;
  wnavnode->get_wbctx_cb = &Wtt::get_wbctx;
  wnavnode->attach_volume_cb = &Wtt::attach_volume_cb;
  wnavnode->detach_volume_cb = &Wtt::detach_volume_cb;
  wnavnode->get_palette_select_cb = &Wtt::get_palette_select_cb;
  wnavnode->set_focus_cb = &Wtt::set_focus_cb;
  wnavnode->traverse_focus_cb = &Wtt::traverse_focus;
  wnavnode->set_twowindows_cb = &Wtt::set_twowindows_cb;
  wnavnode->set_configure_cb = &Wtt::configure_cb;
  wnavnode->gbl_command_cb = &Wtt::gbl_command_cb;
  wnavnode->create_popup_menu_cb = &Wtt::create_popup_menu_cb;
  wnavnode->save_cb = &Wtt::save_cb;
  wnavnode->revert_cb = &Wtt::revert_cb;
  wnavnode->script_filename_cb = &Wtt::script_filename_cb;
  wnavnode->format_selection_cb = Wtt::format_selection;
  wnavnode->get_global_select_cb = Wtt::get_global_select_cb;
  wnavnode->global_unselect_objid_cb = Wtt::global_unselect_objid_cb;
  wnavnode->set_window_char_cb = Wtt::set_window_char_cb;
  wnavnode->open_vsel_cb = Wtt::open_vsel_cb;

  i = 0;
  XtSetArg(args[i], XmNheight, 300);i++;
  XtSetArg(args[i], XmNpaneMinimum, 100);i++;
  XtSetArg(args[i], XmNnavigationType, XmTAB_GROUP);i++;
  XtSetArg(args[i], XmNtraversalOn, True);i++;
  XtSetValues( wnavnode_brow_widget, args,i);

  palette = new PalMotif( this, palette_form, "Objects",
		ldhses, layout_palette,
		&palette_widget, &sts);
  palette->set_focus_cb = &Wtt::set_focus_cb;
  palette->traverse_focus_cb = &Wtt::traverse_focus;
  palette->create_popup_menu_cb = &Wtt::create_pal_popup_menu_cb;

  i = 0;
  XtSetArg(args[i],XmNwidth, WTT_PALETTE_WIDTH);i++;
  XtSetValues( palette_widget,args,i);
    

  XtUnmanageChild( palette_form);
  XtUnmanageChild( wnavnode_brow_widget);
  XtPopup( toplevel, XtGrabNone);
//  XtRealizeWidget( toplevel);

//  XmProcessTraversal( wnav->wnav_brow_widget, XmTRAVERSE_CURRENT);
//  wnav->set_inputfocus();

  // Connect the window manager close-button to exit
  flow_AddCloseVMProtocolCb( toplevel, 
	(XtCallbackProc)activate_exit, this);

  wnav->get_options( &show_class, &show_alias, 
	&show_descrip, &show_objref, &show_objxref, 
	&show_attrref, &show_attrxref, &build_force, &build_debug,
	&build_crossref, &build_manual);

  if ( wbctx && volid) {
    wnav->volume_attached( wbctx, ldhses, 0);
    wnavnode->volume_attached( wbctx, ldhses, 0);
  }

  if ( wb_type == wb_eType_Directory) {
    // Start configuration wizard if volume is empty
    pwr_tOid oid;

    sts = ldh_GetRootList( ldhses, &oid);
    if ( EVEN( sts)) {
      wnav->wow->HideWarranty();  // Warranty window is hidden behind the wizard
      set_edit();
      XtAppAddWorkProc( XtWidgetToApplicationContext(toplevel),
			(XtWorkProc)start_wizard, this) ;
    }
  }

  menu_setup();
  *status = 1;
}


WttMotif::~WttMotif()
{
  if ( close_cb)
    (close_cb)( this);
  else
    exit(0);

  free_cursor();

  if ( utedctx)
    delete utedctx;

  if ( set_focus_disabled)
    XtRemoveTimeOut( disfocus_timerid);
  if ( selection_timerid)
    XtRemoveTimeOut( selection_timerid);

  if ( mcp)
    free(mcp);
  wnav->closing_down = 1;
  wnavnode->closing_down = 1;
  delete wnav;
  delete wnavnode;
  delete palette;

  XtDestroyWidget( toplevel);
}

void WttMotif::pop()
{
  flow_UnmapWidget( toplevel);
  flow_MapWidget( toplevel);
}

//
// Popupup Menu
//
Widget WttMotif::build_menu()
{
  Widget popup;
  int i = 0;

  popup = build_submenu( wnav_form, MENU_POPUP, (char*) "", mcp, 
			 popup_button_cb, (void *) this, 
			 (ldh_sMenuItem *) mcp->ItemList, &i);
  if (popup != NULL) 
    XtAddCallback( popup, XmNunmapCallback, 
		   (XtCallbackProc)popup_unmap_cb, this);

  return popup;
}

Widget WttMotif::build_submenu( Widget Parent, int MenuType,
				char *MenuTitle, void *MenuUserData,
				void (*Callback)( Widget, Wtt *, XmAnyCallbackStruct *),
				void *CallbackData, ldh_sMenuItem *Items, int *idx)
{
  Widget Menu, Cascade, W;
  int i;
  unsigned int Level;
  XmString Str;
  WidgetClass Class;
  Arg ArgList[5]; 
  XmFontList fontlist;
  XFontStruct *font;
  XmFontListEntry fontentry;

  // Set default fontlist
  font = XLoadQueryFont( flow_Display(Parent),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
  fontentry = XmFontListEntryCreate( (char*) "tag1", XmFONT_IS_FONT, font);
  fontlist = XmFontListAppendEntry( NULL, fontentry);
  XtFree( (char *)fontentry);

  i = 0;
  XtSetArg(ArgList[i], XmNuserData, MenuUserData); i++;
  XtSetArg(ArgList[i], XmNbuttonFontList, fontlist); i++;
  XtSetArg(ArgList[i], XmNlabelFontList, fontlist); i++;
  if (MenuType == MENU_PULLDOWN || MenuType == MENU_OPTION)
    Menu = XmCreatePulldownMenu(Parent, (char*) "_pulldown", ArgList, i);
  else if (MenuType == MENU_POPUP)
    Menu = XmCreatePopupMenu(Parent, (char*) "_popup", ArgList, i);  
  else  {
    XtWarning("Invalid menu type passed to BuildMenu().");
    return NULL;
  }

  if (MenuType == MENU_PULLDOWN)  {
    Str = XmStringCreateSimple(MenuTitle);	
    Cascade = XtVaCreateManagedWidget(MenuTitle,
	    xmCascadeButtonGadgetClass, Parent,
	    XmNsubMenuId,   Menu,
	    XmNlabelString, Str,
	    NULL);
    XmStringFree(Str);
  } 
  else if (MenuType == MENU_OPTION)  {
    Str = XmStringCreateSimple(MenuTitle);
    XtSetArg(ArgList[0], XmNsubMenuId, Menu);
    XtSetArg(ArgList[1], XmNlabelString, Str);
    Cascade = XmCreateOptionMenu(Parent, MenuTitle, ArgList, 2);
    XmStringFree(Str);
  }

  XmFontListFree( fontlist);

  Level = Items[*idx].Level;

  for (; Items[*idx].Level != 0 && Items[*idx].Level >= Level; (*idx)++) {
    if (Items[*idx].Item == ldh_eMenuItem_Cascade) {
      if (MenuType == MENU_OPTION)  {
        XtWarning("You can't have submenus from option menu items.");
        return NULL;
      } 
      else {
        i = *idx;
        (*idx)++;	
        build_submenu(Menu, MENU_PULLDOWN, Items[i].Name, MenuUserData, 
		      Callback, CallbackData, Items, idx);
        (*idx)--;
      }
    }
    else {
      if (Items[*idx].Item == ldh_eMenuItem_Separator)
        Class = xmSeparatorGadgetClass;
      else
        Class = xmPushButtonGadgetClass;
 
      W = XtVaCreateManagedWidget(Items[*idx].Name, 
		    Class, Menu,
		    XmNuserData, *idx,
		    XmNsensitive, (Boolean)(Items[*idx].Flags.f.Sensitive == 1),
		    NULL);

      if (Callback && Class == xmPushButtonGadgetClass)
        XtAddCallback(W, XmNactivateCallback, (XtCallbackProc) Callback, 
		  (XtPointer) CallbackData);
    }
  }

  return MenuType == MENU_POPUP ? Menu : Cascade;
}

void WttMotif::popup_unmap_cb(Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  XtDestroyWidget(w);
}

void WttMotif::popup_button_cb( Widget w, Wtt *wtt, XmAnyCallbackStruct *data)
{
  Widget menu;
  int idx;
  pwr_tStatus sts;

  // Find the menu widget
  menu = XtParent(w);
  while (1) {
    if (strcmp(XtName(menu), "_popup") == 0 || 
	  strcmp(XtName(menu), "_pulldown") == 0)
      break;
    menu = XtParent(menu);
  }

  XtVaGetValues (w, XmNuserData, &idx, NULL);

  wtt->mcp->ChosenItem = idx;
  wtt->set_clock_cursor();
  sts = ldh_CallMenuMethod( wtt->mcp, wtt->mcp->ChosenItem);
  if (EVEN(sts))
    wtt->message( 'E', wnav_get_message(sts));
  wtt->reset_cursor();

}

