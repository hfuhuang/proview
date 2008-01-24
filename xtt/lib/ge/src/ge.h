/* 
 * Proview   $Id: ge.h,v 1.11 2008-01-24 09:28:01 claes Exp $
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

#ifndef ge_h
#define ge_h

#ifndef wb_ldh_h
#include "wb_ldh.h"
#endif

#ifndef ge_subpalette_h
#include "ge_subpalette.h"
//class SubPalette;
#endif

#ifndef ge_subgraphs_h
//#include "ge_subgraphs.h"
class SubGraphs;
#endif

#ifndef glow_h
#include "glow.h"
#endif
#ifndef glow_colpalctx_h
//#include "glow_colpalctx.h"
#endif
#ifndef glow_colpalapi_h
//#include "glow_colpalapi.h"
#endif
#ifndef glow_growapi_h
//#include "glow_growapi.h"
typedef void *grow_tObject;
typedef void *GlowCtx;
#endif

/* ge.h -- Simple graphic editor */

typedef struct {
    char	name[40];
    char	prev[40];
} ge_tPrevPage;

class ColPalCtx;
class Graph;
class Nav;
class CoWow;

class Ge {
 public:
  void 		*parent_ctx;
  char 		name[80];
  Graph   	*graph;
  SubPalette	*subpalette;
  SubGraphs	*subgraphs;
  ColPalCtx	*colorpalette_ctx;
  int		text_input_open;
  int		name_input_open;
  int		value_input_open;
  int		command_open;
  int		confirm_open;
  int		yesnodia_open;
  void		(*yesnodia_yes_cb)( Ge *);
  void		(*yesnodia_no_cb)( Ge *);
  void		(*india_ok_cb)( Ge *, char *);
  void		*current_text_object;
  void		*current_value_object;
  void		*current_confirm_object;
  ldh_tSesContext ldhses;
  Nav     	*plantctx;
  int		exit_when_close;
  ge_tPrevPage	prev_table[40];
  int		prev_count;
  void		*focused_component;
  grow_tObject  recover_object;
  char          recover_name[80];
  int		plant_mapped;
  int		subpalette_mapped;
  CoWow		*wow;

  Ge( void *parent_ctx,
      ldh_tSesContext ldhses, int exit_when_close);
  virtual ~Ge();
  void open( char *name);
  void save( char *name);
  void clear();
  static int generate_web( ldh_tSesContext ldhses);
  int command( char *cmd);
  void message( pwr_tStatus sts);

  virtual void set_title( char *title) {}
  virtual void open_input_dialog( char *text, char *title,
				  char *init_text,
				  void (*india_ok_cb)( Ge *, char *)) {}
  virtual void message( char severity, char *message) {}
  virtual void status_msg( char *pos_str) {}
  virtual void open_yesnodia( char *text, char *title, 
			      void (*yes_cb)( Ge *), void (*no_cb)( Ge *)) {}
  virtual void set_prompt( char *prompt) {}
  virtual void subgraphs_new() {}
  virtual void update() {}
  virtual int get_plant_select( char *name) { return 0;}
  virtual void create_list( char *title, char *texts,
			    void (action_cb)( void *, char *), void *ctx) {}
  virtual void plant_del( void *plantctx) {}
  virtual int plant_get_select( void *plantctx, pwr_sAttrRef *attrref, int *is_attr) {return 0;}

  void set_title();
  void prevtable_insert( char *name, char *prev);
  int prevtable_get( char *name, char *prev);
  void prevtable_clear();
  void save_and_close();

  void clear_all();
  void open_graph( char *name);
  int set_focus( void *component);


  void activate_change_text();
  void activate_change_name();
  void activate_preview_start();
  void activate_preview_stop();
  void activate_cut();
  void activate_copy();
  void activate_rotate();
  void activate_rotate90();
  void activate_flip_vert();
  void activate_flip_horiz();
  void activate_pop();
  void activate_push();
  void activate_edit_polyline();
  void activate_scale_equal();
  void activate_move_horizontal();
  void activate_move_vertical();
  void activate_move_reset();
  void activate_align_horiz_up();
  void activate_align_horiz_down();
  void activate_align_horiz_center();
  void activate_align_vert_left();
  void activate_align_vert_right();
  void activate_align_vert_center();
  void activate_equid_vert_up();
  void activate_equid_vert_down();
  void activate_equid_vert_center();
  void activate_equid_horiz_left();
  void activate_equid_horiz_right();
  void activate_equid_horiz_center();
  void activate_select_cons();
  void activate_select_objects();
  void activate_select_nextobject( glow_eDirection dir);
  void activate_group();
  void activate_ungroup();
  void activate_connect();
  void activate_connectsecond();
  void activate_objectattributes();
  void activate_show_grid(int set);
  void activate_paste();
  void activate_command();
  void activate_exit();
  void activate_print();
  void activate_new();
  void activate_save();
  void activate_save_as();
  void activate_export_javabean();
  void activate_export_javabean_as();
  void activate_export_gejava();
  void activate_export_gejava_as();
  void activate_export_java();
  void activate_export_java_as();
  void activate_export_plcfo();
  void activate_export_plcfo_as();
  void activate_generate_web();
  void activate_creanextpage();
  void activate_nextpage();
  void activate_prevpage();
  void activate_graph_attr();
  void activate_open();
  void activate_subgraphs();
  void activate_rect( bool keep);
  void activate_rectrounded( bool keep);
  void activate_line( bool keep);
  void activate_polyline( bool keep);
  void activate_circle( bool keep);
  void activate_text( bool keep);
  void activate_annot( bool keep);
  void activate_conpoint( bool keep);
  void activate_fill(int set);
  void activate_border(int set);
  void activate_shadow(int set);
  void activate_incr_lightness();
  void activate_decr_lightness();
  void activate_incr_intensity();
  void activate_decr_intensity();
  void activate_incr_shift();
  void activate_decr_shift();
  void activate_scale();
  void activate_scale( double factor);
  void activate_grid(int set);
  void activate_linewidth( int width);
  void activate_linetype1();
  void activate_linetype2();
  void activate_linetype3();
  void activate_linetype4();
  void activate_linetype5();
  void activate_linetype6();
  void activate_linetype7();
  void activate_gridsize( double size);
  void activate_textsize( int size);
  void activate_textfont( glow_eFont font);
  void activate_textbold(int set);
  void activate_zoom_in();
  void activate_zoom_out();
  void activate_zoom_reset();
  void activate_concorner_right();
  void activate_concorner_rounded();
  void activate_round_amount( double amount);
  void activate_contype_straight();
  void activate_contype_routed();
  void activate_contype_stronearr();
  void activate_contype_stepdiv();
  void activate_contype_stepconv();
  void activate_contype_transdiv();
  void activate_contype_transconv();
  void activate_condir_center();
  void activate_condir_left();
  void activate_condir_right();
  void activate_condir_up();
  void activate_condir_down();
  void activate_background_color();
  void activate_help();
  void activate_help_subgraph();
  void activate_india_ok( char *value);
  void activate_india_cancel();
  void activate_yesnodia_yes();
  void activate_yesnodia_no();
  void activate_yesnodia_cancel();
  void activate_confirm_ok();
  void activate_confirm_cancel();

  static int get_plant_select_cb( void *ge_ctx, char *select_name);
  static void load_graph_cb( void *ge_ctx, char *name);
  static void save_graph( Ge *gectx, char *name);
  static void save_graph_and_close( Ge *gectx, char *name);
  static void ungroup_yes_cb( Ge *gectx);
  static void ungroup_no_cb( Ge *gectx);
  static void recover_dynprop_yes_cb( Ge *gectx);
  static void recover_dynprop_no_cb( Ge *gectx);
  static void exit_save_cb( Ge *gectx);
  static void exit_nosave_cb( Ge *gectx);
  static void export_javabean( Ge *gectx, char *name);
  static void export_gejava( Ge *gectx, char *name);
  static void export_plcfo( Ge *gectx, char *filename);
  static void rotate( Ge *gectx, char *value_str);
  static int subpalette_get_select( void *gectx, char *text, char *filename);
  static void colorpalette_get_current( void *gectx, glow_eDrawType *fill_color,
					glow_eDrawType *border_color, 
					glow_eDrawType *text_color);
  static void colorpalette_set_current( void *gectx, glow_eDrawType fill_color,
					glow_eDrawType border_color, 
					glow_eDrawType text_color);
  static void subgraphs_close_cb( SubGraphs *subgraphs);
  static void status_msg( void *ge_ctx, double x, double y);
  static int command_cb( void *ge_ctx, char *command);
  static void open_list_cb( void *ctx, char *text);
  static int sort_files( const void *file1, const void *file2);

  static int colorpalette_cb( GlowCtx *ctx, glow_tEvent event);
  static int init_colorpalette_cb( GlowCtx *fctx, void *client_data);
  static int get_ldhses_cb( void *ctx, ldh_tSesContext *ldhses, int load);
  static int traverse_focus( void *ctx, void *component);
  static int set_focus_cb( void *ctx, void *component);
  static void message_cb( void *ctx, char severity, char *message);
  static void help_cb( void *ctx, char *topic, char *helpfile);
};

#endif
