/* 
 * Proview   $Id: ge_motif.h,v 1.1 2007-01-04 08:22:16 claes Exp $
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

#ifndef ge_motif_h
#define ge_motif_h

#ifndef ge_h
#include "ge.h"
#endif

/* ge_motif.h -- Simple graphic editor */

class GeMotif : public Ge {
  Widget	parent_wid;
  Widget	grow_widget;
  Widget	form_widget;
  Widget	main_pane;
  Widget	palette_pane;
  Widget	colorpalette_widget;
  Widget	colpal_main_widget;
  Widget	plant_widget;
  Widget	subpalette_widget;
  Widget	subgraphs_widget;
  Widget	toplevel;
  Widget	india_widget;
  Widget	india_label;
  Widget	india_text;
  Widget	cursor_position;
  Widget	msg_label;
  Widget	cmd_prompt;
  Widget	cmd_input;
  Widget	graph_form;
  Widget	confirm_widget;
  Widget	yesnodia_widget;
  Widget	grid_on_w;
  Widget	grid_size_w;
  Widget	grid_size_10_w;
  Widget	grid_size_05_w;
  Widget	grid_size_02_w;
  Widget	grid_size_01_w;
  Widget	show_grid_w;
  Atom		graph_atom;
  char		text_recall[30][160];
  int		text_current_recall;
  char		name_recall[30][160];
  int		name_current_recall;
  char		value_recall[30][160];
  int		value_current_recall;
  char		cmd_recall[30][160];
  int		cmd_current_recall;

 public:
  GeMotif( void *parent_ctx, Widget parent_widget,
      ldh_tSesContext ldhses, int exit_when_close,
      char *graph_name);
  ~GeMotif();

  virtual void set_title( char *title);
  virtual void open_input_dialog( char *text, char *title,
				  char *init_text,
				  void (*india_ok_cb)( Ge *, char *));
  virtual void message( char severity, char *message);
  virtual void status_msg( char *pos_str);
  virtual void open_yesnodia( char *text, char *title, 
			      void (*yes_cb)( Ge *), void (*no_cb)( Ge *));
  virtual void set_prompt( char *prompt);
  virtual void subgraphs_new();
  virtual void update();
  virtual int get_plant_select( char *name);
  virtual void create_list( char *title, char *texts,
			    void (action_cb)( void *, char *), void *ctx);
  virtual void plant_del( void *plantctx);
  virtual int plant_get_select( void *plantctx, pwr_sAttrRef *attrref, int *is_attr);

  static void valchanged_cmd_input( Widget w, XEvent *event);
  static void change_text_cb( void *ge_ctx, void *text_object, char *text);
  static void change_name_cb( void *ge_ctx, void *text_object, char *text);
  static void change_value_cb( void *ge_ctx, void *value_object, char *text);
  static void confirm_cb( void *ge_ctx, void *confirm_object, char *text);

  static void action_inputfocus( Widget w, XmAnyCallbackStruct *data);

  static void create_cursor_position( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_msg_label( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_cmd_prompt( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_cmd_input( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_graph_form( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_main_pane( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_palette_pane( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_widget_cb( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_india_label( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void create_india_text( Widget w, Ge *gectx, XmAnyCallbackStruct *data);

  static void activate_change_text( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_change_name( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_preview_start( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_preview_stop( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_cut( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_copy( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_rotate( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_rotate90( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_flip_vert( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_flip_horiz( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_pop( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_push( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_edit_polyline( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_scale_equal( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_move_horizontal( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_move_vertical( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_move_reset( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_align_horiz_up( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_align_horiz_down( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_align_horiz_center( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_align_vert_left( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_align_vert_right( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_align_vert_center( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_equid_vert_up( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_equid_vert_down( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_equid_vert_center( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_equid_horiz_left( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_equid_horiz_right( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_equid_horiz_center( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_select_cons( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_select_objects( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_group( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_ungroup( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_connect( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_connectsecond( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_objectattributes( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_show_grid( Widget w, Ge *gectx, XmToggleButtonCallbackStruct *data);
  static void activate_paste( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_command( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_exit( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_print( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_new( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_save( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_save_as( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_export_javabean( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_export_javabean_as( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_export_gejava( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_export_gejava_as( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_export_java( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_export_java_as( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_generate_web( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_creanextpage( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_nextpage( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_prevpage( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_graph_attr( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_open( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_subgraphs( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_rect( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_rectrounded( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_line( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_polyline( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_circle( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_text( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_annot( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_conpoint( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_fill( Widget w, Ge *gectx, XmToggleButtonCallbackStruct *data);
  static void activate_border( Widget w, Ge *gectx, XmToggleButtonCallbackStruct *data);
  static void activate_shadow( Widget w, Ge *gectx, XmToggleButtonCallbackStruct *data);
  static void activate_incr_lightness( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_decr_lightness( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_incr_intensity( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_decr_intensity( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_incr_shift( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_decr_shift( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_scale( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_grid( Widget w, Ge *gectx, XmToggleButtonCallbackStruct *data);
  static void activate_linewidth_1( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linewidth_2( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linewidth_3( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linewidth_4( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linewidth_5( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linewidth_6( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linewidth_7( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linewidth_8( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linetype1( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linetype2( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linetype3( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linetype4( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linetype5( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linetype6( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_linetype7( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_gridsize_4( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_gridsize_3( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_gridsize_2( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_gridsize_1( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_textsize_0( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_textsize_1( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_textsize_2( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_textsize_3( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_textsize_4( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_textsize_5( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_textbold( Widget w, Ge *gectx, XmToggleButtonCallbackStruct *data);
  static void activate_zoom_in( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_zoom_out( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_zoom_reset( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_concorner_right( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_concorner_rounded( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_round_amount_1( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_round_amount_2( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_round_amount_3( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_round_amount_4( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_round_amount_5( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_contype_straight( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_contype_routed( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_contype_stronearr( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_contype_stepdiv( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_contype_stepconv( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_contype_transdiv( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_contype_transconv( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_condir_center( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_condir_left( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_condir_right( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_condir_up( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_condir_down( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_background_color( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_help( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_india_ok( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_india_cancel( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_yesnodia_yes( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_yesnodia_no( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_yesnodia_cancel( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_confirm_ok( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
  static void activate_confirm_cancel( Widget w, Ge *gectx, XmAnyCallbackStruct *data);
};

#endif