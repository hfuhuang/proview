/* 
 * Proview   $Id: wb_wtt.h,v 1.22 2008-10-31 12:51:32 claes Exp $
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

#ifndef wb_wtt_h
#define wb_wtt_h

/* wb_wtt.h -- Simple navigator */

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef wb_utility_h
# include "wb_utility.h"
#endif

#ifndef flow_std_h
#include "flow_std.h"
#endif

#ifndef flow_h
#include "flow.h"
#endif

#ifndef flow_browctx_h
#include "flow_browapi.h"
#endif

#ifndef wb_ldh
# include "wb_ldh.h"
#endif

#ifndef wb_pal
# include "wb_pal.h"
#endif

#ifndef wb_wnav_h
# include "wb_wnav.h"
#endif

#ifndef wb_h
# include "wb.h"
#endif

#ifndef wb_uted_h
#include "wb_uted.h"
#endif

#ifndef wb_wda_h
#include "wb_wda.h"
#endif

#ifndef co_wow_h
#include "co_wow.h"
#endif

class WPkg;

typedef enum {
  	wtt_eNoEdit_Save,
  	wtt_eNoEdit_Revert,
	} wtt_eNoEditMode;

typedef enum {
  	wtt_eNoEdit_DetachVolume,
  	wtt_eNoEdit_KeepVolume,
	} wtt_eNoEditVolMode;

typedef enum {
  	wtt_eInputMode_Attribute,
  	wtt_eInputMode_ObjectName
	} wtt_eInputMode;


class wb_build;
class wb_session;

class WttApplListElem {
  public:
    WttApplListElem( wb_eUtility al_type, void *al_ctx, pwr_tObjid al_objid,
	const char *al_name);
    wb_eUtility		type;
    void		*ctx;
    pwr_tObjid		objid;
    char		name[80];
    WttApplListElem 	*next;
};

class WttApplList {
  public:
    WttApplList() :
       root(NULL) {};
    ~WttApplList();

    WttApplListElem *root;
    void insert( wb_eUtility type, void *ctx, 
	pwr_tObjid objid, const char *name);
    void remove( void *ctx);
    int find( wb_eUtility type, const char *name, void **ctx);
    int find( wb_eUtility type, pwr_tObjid objid, void **ctx);
    void set_editmode( int editmode, ldh_tSesContext ldhses);
};

class Wtt : public WUtility {
  public:
    Wtt( 
	void	*wt_parent_ctx,
	const char 	*wt_name,
	const char	*iconname,
	ldh_tWBContext wt_wbctx,
	pwr_tVolumeId wt_volid,
	ldh_tVolume wt_volctx,
	wnav_sStartMenu *root_menu,
	pwr_tStatus *status);
    Wtt() : WUtility(wb_eUtility_Wtt) {};

    void 	*parent_ctx;
    char 	name[80];
    wb_eType	wb_type;
    WttApplList	appl;
    WNav	*wnav;
    WNav	*wnavnode;
    WNav	*focused_wnav;
    Pal		*palette;
    void	*root_item;
    int		input_open;
    int		command_open;
    ldh_tWBContext wbctx;
    ldh_tVolContext volctx;
    pwr_tVolumeId	volid;
    ldh_tSesContext ldhses;
    int		editmode;
    int		twowindows;
    int		confirm_open;
    void		(*india_ok_cb)( Wtt *, char *);
    void		(*confirm_ok_cb)( Wtt *);
    void		(*confirm_no_cb)( Wtt *);
    void	*boot_volumelist;
    int		boot_volumecount;
    int		select_syntax;
    int		select_volume;
    int		select_attr;
    int		select_type;
    int		show_class;
    int		show_alias;
    int		show_descrip;
    int		show_objref;
    int		show_objxref;
    int		show_attrref;
    int		show_attrxref;
    int		build_force;
    int		build_debug;
    int		build_crossref;
    int		build_manual;
    int		wnav_mapped;
    int		wnavnode_mapped;
    WUted	*utedctx;
    WPkg	*wpkg;
    WNav	*input_wnav;
    brow_tObject input_node;
    pwr_tObjid	input_objid;
    wtt_eInputMode input_mode;
    void	(*close_cb)(void *ctx);
    void	(*open_volume_cb)(void *ctx, wb_eType, const char *, wow_eFileSelType);
    void	(*open_project_volume_cb)(void *ctx);
    int		(*time_to_exit_cb)(void *ctx);
    ldh_sMenuCall *mcp;
    int		disable_w2;

    int restore_settings();
    int save_settings();
    void set_editmode( int edit);
    int set_edit();
    int set_noedit( wtt_eNoEditMode save, wtt_eNoEditVolMode detach);
    int set_focus( void *component);
    int is_saved();
    int detach_volume();
    int get_select_first( pwr_sAttrRef *attrref, int *is_attr);
    void register_utility( void *ctx, wb_eUtility utility);
    void set_focus_default();
    int find( pwr_tOid oid);
    int find_plc( pwr_tOid oid);
    char *script_filename();
    int get_popup_menu_items( pwr_sAttrRef aref, pwr_tCid cid);

    void activate_print();
    void activate_collapse();
    void activate_save();
    void activate_revert();
    void activate_syntax();
    void activate_find();
    void activate_findregex();
    void activate_findnext();
    void activate_copy();
    void activate_cut();
    void activate_paste();
    void activate_pasteinto();
    void activate_copykeep();
    void activate_configure();
    void activate_utilities();
    void activate_openobject();
    void activate_creaobj( ldh_eDest dest);
    void activate_moveobj( wnav_eDestCode dest);
    void activate_deleteobj();
    void activate_openvolobject();
    void activate_openplc();
    void activate_buildobject();
    void activate_openvolume();
    void activate_openbuffer();
    void activate_confproject();
    void activate_openpl();
    void activate_opengvl();
    void activate_openudb();
    void activate_spreadsheet();
    void activate_openge();
    void activate_openclasseditor();
    void activate_buildvolume();
    void activate_buildnode();
    void activate_distribute();
    void activate_showcrossref();
    void activate_updateclasses();
    void activate_zoom_in();
    void activate_zoom_out();
    void activate_zoom_reset();
    void activate_twowindows();
    void activate_messages();
    void activate_scriptproj();
    void activate_scriptbase();
    void activate_help();
    void activate_help_project();
    void activate_help_proview();

    virtual void set_clock_cursor() {}
    virtual void reset_cursor() {}
    virtual void free_cursor() {}
    virtual void set_window_char( int width, int height) {}
    virtual void get_window_char( int *width, int *height) {}
    virtual void menu_setup() {}
    virtual void set_selection_owner() {}
    virtual void set_palette_selection_owner() {}
    virtual int create_popup_menu( pwr_tAttrRef attrref, int x, int y) {return 0;}
    virtual int create_pal_popup_menu( pwr_tCid cid, int x, int y) {return 0;}
    virtual void set_noedit_show() {}
    virtual void set_edit_show() {}
    virtual void set_twowindows( int two, int display_wnav, int display_wnavnode) {}
    virtual void message( char severity, const char *message) {}
    virtual void set_prompt( const char *prompt) {}
    virtual void open_change_value() {}
    virtual void close_change_value() {}
    virtual void open_change_name() {}
    virtual void watt_new( pwr_tAttrRef aref) {}
    virtual void wda_new( pwr_tOid oid) {}
    virtual void ge_new( char *graphname) {}
    virtual void wcast_new( pwr_tAttrRef aref, pwr_tStatus *sts) {}
    virtual wb_build *build_new() { return NULL;}
    virtual void wpkg_new() {}
    virtual int ute_new( char *title) {return 0;}
    virtual void open_input_dialog( const char *text, const char *title,
				    const char *init_text,
				    void (*ok_cb)( Wtt *, char *)) {}
    virtual void open_confirm( const char *text, const char *title, 
			       void (*ok_cb)( Wtt *), void (*no_cb)( Wtt *)) {}
    virtual void open_boot_window() {}
    virtual void update_options_form() {}
    virtual void set_options() {}
    virtual void pop() {}
    virtual void disable_focus() {}


    static int format_selection( void *ctx, pwr_sAttrRef attrref, 
				 char **value_return, int is_class, int is_attr,
				 wnav_eSelectionFormat format);
    static int start_wizard( Wtt *wtt, pwr_tCid vcid);
    static void set_twowindows_cb( void *wtt, int two, int display_w1,
				   int display_w2);
    static pwr_tStatus ldh_this_session_cb( void *ctx, ldh_sEvent *event);
    static pwr_tStatus ldh_other_session_cb( void *ctx, ldh_sEvent *event);
    static void open_vsel_cb( void *ctx, wb_eType type, char *filename, 
			      wow_eFileSelType file_type);
    static void set_window_char_cb( void *ctx, int width, int height);
    static char *script_filename_cb( void *ctx);
    static int traverse_focus( void *ctx, void *component);
    static int get_global_select_cb( void *ctx, pwr_sAttrRef **sel_list,
				     int **sel_is_attr, int *sel_cnt);
    static int global_unselect_objid_cb( void *ctx, pwr_tObjid objid);
    static int set_focus_cb( void *ctx, void *comp);
    static void create_popup_menu_cb( void *wtt, pwr_tAttrRef aref,
				      int x, int y);
    static void create_pal_popup_menu_cb( void *wtt, pwr_tCid cid,
				      int x, int y);
    static void gbl_command_cb( void *ctx, const char *cmd);
    static void configure_cb( void *ctx, int edit);
    static void findregex_ok( Wtt *wtt, char *search_str);
    static void find_ok( Wtt *wtt, char *search_str);
    static void file_selected_cb( void *ctx, char *filename, wow_eFileSelType file_type);
    static void save_cb( void *ctx, int quiet);
    static void revert_ok( Wtt *wtt);
    static void revert_cb( void *ctx, int confirm);
    static int attach_volume_cb( void *ctx, pwr_tVolumeId volid, int pop);
    static void detach_save_ok( Wtt *wtt);
    static void detach_save_no( Wtt *wtt);
    static void save_ok( Wtt *wtt);
    static void save_no( Wtt *wtt);
    static void close_ok( Wtt *wtt);
    static void close_no( Wtt *wtt);
    static void close_now_ok( Wtt *wtt);
    static int detach_volume_cb( void *ctx);
    static int get_palette_select_cb( void *ctx, pwr_tCid *classid);
    static void message_cb( void *ctx, char severity, const char *msg);
    static void close( void *ctx);
    static void change_value( void *ctx);
    static int get_wbctx( void *ctx, ldh_tWBContext *wbctx);
    static void wpkg_quit_cb( void *ctx);
    static void uted_quit_cb( void *ctx);
    static void delete_ok( Wtt *wtt);


    virtual ~Wtt();
};

#endif
