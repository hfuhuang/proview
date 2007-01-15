/* 
 * Proview   $Id: xtt_ge_motif.h,v 1.1 2007-01-04 08:30:03 claes Exp $
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

#ifndef xtt_ge_motif_h
#define xtt_ge_motif_h

#ifndef xtt_ge_h
# include "xtt_ge.h"
#endif

class XttGeMotif : public XttGe {
 public:
  Widget	parent_wid;
  Widget	grow_widget;
  Widget	form_widget;
  Widget	toplevel;
  Widget	nav_shell;
  Widget	nav_widget;
  Widget	menu_widget;
  Widget	graph_form;
  Widget	value_input;
  Widget	value_dialog;
  Widget	confirm_widget;
  Widget	message_dia_widget;
  int		set_focus_disabled;
  XtIntervalId 	focus_timerid;

  XttGeMotif( Widget parent_wid, void *parent_ctx, char *name, char *filename,
	      int scrollbar, int menu, int navigator, int width, int height,
	      int x, int y, double scan_time, char *object_name, int use_default_access,
	      unsigned int access,
	      int (*xg_command_cb) (XttGe *, char *),
	      int (*xg_get_current_objects_cb) (void *, pwr_sAttrRef **, int **),
	      int (*xg_is_authorized_cb) (void *, unsigned int));
  ~XttGeMotif();

  void pop();
  void set_size( int width, int height);

  static void enable_set_focus( XttGeMotif *ge);
  static void disable_set_focus( XttGeMotif *ge, int time);
  static void ge_change_value_cb( void *ge_ctx, void *value_object, char *text);
  static void confirm_cb( void *ge_ctx, void *confirm_object, char *text);
  static void message_dialog_cb( void *ge_ctx, char *text);

  static void action_inputfocus( Widget w, XmAnyCallbackStruct *data);
  static void activate_value_input( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void activate_confirm_ok( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void activate_confirm_cancel( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void activate_exit( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void activate_zoom_in( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void activate_zoom_out( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void activate_zoom_reset( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void activate_help( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void create_graph_form( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void create_message_dia( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void create_menu( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void create_value_input( Widget w, XttGe *ge, XmAnyCallbackStruct *data);
  static void action_resize( Widget w, XmAnyCallbackStruct *data);

};

#endif





