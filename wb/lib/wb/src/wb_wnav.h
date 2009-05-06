/* 
 * Proview   $Id: wb_wnav.h,v 1.19 2008-10-31 12:51:32 claes Exp $
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

#ifndef wb_wnav_h
#define wb_wnav_h

/* wtt_wnav.h -- Simple navigator */

#ifndef pwr_h
//# include "pwr.h"
#endif

#ifndef wb_utility_h
# include "wb_utility.h"
#endif

#ifndef co_regex_h
# include "co_regex.h"
#endif

#ifndef co_nav_help_h
//# include "co_nav_help.h"
#endif

#ifndef flow_h
//#include "flow.h"
#endif

#ifndef wb_wnav_brow_h
#include "wb_wnav_brow.h"
#endif

#ifndef wb_palfile_h
# include "wb_palfile.h"
#endif

#ifndef wb_build_opt_h
#include "wb_build_opt.h"
#endif

#ifndef co_wow_h
# include "co_wow.h"
#endif

//#ifndef co_xhelp_h
//#include "co_xhelp.h"
//#endif

#ifndef wb_h
#include "wb.h"
#endif


#define wnav_cVersion	"X3.3a"
#define wnav_cScriptDescKey	"!** Description"
#define wnav_cScriptInvisKey	"!** Invisible"
#define wnav_cInitFile "pwrp_login:wtt_init"
#define wnav_cSymbolFile "pwrp_login:wtt_symbols.pwr_com"
#define WNAV_BROW_MAX	25

typedef enum {
  wnav_eSelectionFormat_User,
  wnav_eSelectionFormat_Graph,
  wnav_eSelectionFormat_Objid,
  wnav_eSelectionFormat_Attrref
} wnav_eSelectionFormat;

typedef enum {
	wnav_mOpen_All		= ~0,
	wnav_mOpen_Children	= 1 << 0,
	wnav_mOpen_Attributes	= 1 << 1,
	wnav_mOpen_Crossref	= 1 << 2
	} wnav_mOpen;

typedef enum {
	wnav_eType_GdbNodeFlags	= 10000,
	wnav_eType_Empty	= 10001,
	wnav_eType_FixStr	= 10002,
	wnav_eType_ShortTime	= 10003
	} wnav_eType;

typedef enum {
	wnav_eDestCode_After,
	wnav_eDestCode_Before,
	wnav_eDestCode_FirstChild,
	wnav_eDestCode_LastChild
	} wnav_eDestCode;

typedef enum {
	wnav_eSearchType_No,
	wnav_eSearchType_Name,
	wnav_eSearchType_RegularExpr
	} wnav_eSearchType;

typedef enum {
	wnav_eWindowType_No,
	wnav_eWindowType_W1,
	wnav_eWindowType_W2
	} wnav_eWindowType;

typedef struct {
	char	title[80];
	int	item_type;
	void	*action;
} wnav_sStartMenu;

typedef struct wnav_s_Menu {
	char			title[80];
	int			item_type;
	char			command[256];
	struct wnav_s_Menu	*child_list;
	struct wnav_s_Menu	*parent;
	struct wnav_s_Menu	*next;
} wnav_sMenu;

typedef struct s_trace_node t_trace_node;

typedef enum {
	applist_eType_Trace,
	applist_eType_Graph,
	applist_eType_Attr
} applist_eType;

class Ge;
class WGe;
class wb_utl;
class CoLogin;

class ApplListElem {
  public:
    ApplListElem( applist_eType al_type, void *al_ctx, pwr_tObjid al_objid,
	const char *al_name);
    applist_eType	type;
    void		*ctx;
    pwr_tObjid		objid;
    char		name[80];
    ApplListElem 	*next;
};

class ApplList {
  public:
    ApplList() :
       root(NULL) {};
    
    ApplListElem *root;
    void insert( applist_eType type, void *ctx, 
	pwr_tObjid objid, const char *name);
    void remove( void *ctx);
    int find( applist_eType type, char *name, void **ctx);
    int find( applist_eType type, pwr_tObjid objid, void **ctx);
};

class WNavGbl {
  public:
    WNavGbl() :
      priv(0), verify(0), advanced_user(0), all_toplevel(0), bypass(0),
      show_class(1), show_alias(0), show_descrip(1), show_attrref(0), 
      show_attrxref(0), show_objref(0), show_objxref(0), show_truedb(0)
      {
	strcpy( version, wnav_cVersion);
	strcpy( platform, ""); strcpy( os, ""); strcpy( hw, "");
	strcpy( node, ""); strcpy( sys, ""); strcpy( default_directory, "");
	strcpy( symbolfilename, "");
      };
    char		version[10];
    unsigned long	priv;
    char		platform[20];
    char		os[20];
    char		hw[20];
    int			verify;
    char		node[20];
    char		sys[20];
    char		default_directory[80];
    char		symbolfilename[80];
    int			advanced_user;
    int			all_toplevel;
    int			bypass;
    int			show_class;
    int			show_alias;
    int			show_descrip;
    int			show_attrref;
    int			show_attrxref;
    int			show_objref;
    int			show_objxref;
    int			show_truedb;
    wb_build_opt 	build;

    int			load_config( void *wnav);
    int			symbolfile_exec( void *wnav);
};

class WNav : public WUtility{
  public:
    WNav(
	void *xn_parent_ctx,
	const char *xn_name,
	const char *xn_layout,
        ldh_tSesContext	xn_ldhses,
	wnav_sStartMenu *root_menu,
	wnav_eWindowType xn_type,
	pwr_tStatus *status);
    virtual ~WNav();

    WNavGbl		gbl;
    ApplList		appl;
    void 		*parent_ctx;
    char 		name[80];
    wnav_eWindowType	window_type;
    ldh_tSesContext	ldhses;
    ldh_tWBContext	wbctx;
    WNavBrow		*brow;
    WNavBrow		*collect_brow;
    WNavBrow		*brow_stack[WNAV_BROW_MAX];
    int			brow_cnt;
    pwr_tObjid		root_objid;
    void		*root_item;
    int			trace_started;
    void 		(*message_cb)( void *, char, const char *);
    void 		(*close_cb)( void *);
    void 		(*map_cb)( void *);
    void 		(*change_value_cb)( void *);
    int 		(*get_wbctx_cb)( void *, ldh_tWBContext *);
    int 		(*attach_volume_cb)( void *, pwr_tVolumeId, int);
    int 		(*detach_volume_cb)( void *);
    int 		(*get_palette_select_cb)( void *, pwr_tClassId *);
    int 		(*set_focus_cb)( void *, void *);
    int 		(*traverse_focus_cb)( void *, void *);
    void 		(*set_configure_cb)( void *, int);
    void 		(*set_twowindows_cb)( void *, int, int, int);
    void 		(*gbl_command_cb)( void *, const char *);
    void 		(*create_popup_menu_cb)( void *, pwr_sAttrRef, int, int);
    void 		(*save_cb)( void *, int);
    void 		(*revert_cb)( void *, int confirm);
    char 		*(*script_filename_cb)( void *);
    int 		(*format_selection_cb)( void *, pwr_sAttrRef, char **,
						int, int, wnav_eSelectionFormat);
    int 		(*get_global_select_cb)( void *, pwr_sAttrRef **, 
						 int **, int *);
    int 		(*global_unselect_objid_cb)( void *, pwr_tObjid objid);
    void 		(*set_window_char_cb)( void *, int, int);
    void                (*open_vsel_cb)( void *, wb_eType, char *, wow_eFileSelType);
    int			ccm_func_registred;
    wnav_sMenu 		*menu_tree;
    int			closing_down;
    char		base_user[80];
    char		user[80];
    unsigned int	base_priv;
    unsigned int	priv;
    int			editmode;
    char		layout[120];
    pwr_tObjid		layout_objid;
    pwr_tObjid		search_last;
    char		search_string[200];
    int			search_last_found;
    regex_t		search_creg;
    int			search_compiled;
    wnav_eSearchType	search_type;
    int			selection_owner;
    brow_tObject	last_selected;
    int			displayed;
    int			scriptmode;
    int			dialog_ok;
    int			dialog_cancel;
    int			dialog_help;
    char		dialog_value[200];
    int			dialog_width;
    int			dialog_height;
    int			dialog_x;
    int			dialog_y;
    PalFileMenu         *menu;
    int                 init_help;
    CoWow		*wow;
    int			admin_login;
    int			nodraw;

    virtual void pop() {}
    virtual void set_inputfocus( int focus) {}
    virtual void trace_start() {}
    virtual void set_selection_owner() {}
    virtual Ge *ge_new( char *graph_name) { return 0;}
    virtual WGe *wge_new( char *name, char *filename, char *object_name,
			  int modal) { return 0;}
    virtual void create_popup_menu( pwr_tAttrRef aref, int x, int y) {}
    virtual int get_selection( char *str, int len) {return 0;}
    virtual int open_foe( const char *name, pwr_tOid plcpgm, void **foectx, int map_window,
			  ldh_eAccess access, pwr_tOid oid) {return 0;}
    virtual void wda_new( pwr_tOid oid, pwr_tCid cid, char *attribute,
			  int edit_mode, int advuser, int display_objectname) {}
    virtual void message_dialog( char *title, char *text) {}
    virtual int confirm_dialog( char *title, char *text, int display_cancel,
				 int *cancel) {return 0;}
    virtual int continue_dialog( char *title, char *text) {return 0;}
    virtual int prompt_dialog( char *title, char *text, char **value) {return 0;}
    virtual void wge_subwindow_loop( WGe *wge) {}
    virtual void wge_modal_loop( WGe *wge) {}
    virtual bool has_window() {return false;}
    virtual wb_utl *utl_new() {return 0;}
    virtual CoLogin *login_new( const char *name, const char *groupname,
				void (* bc_success)( void *), void (* bc_cancel)( void *), 
				pwr_tStatus *status) { return 0;}

    static int brow_cb( FlowCtx *ctx, flow_tEvent event);
    static int init_brow_base_cb( FlowCtx *fctx, void *client_data);
    static int init_brow_cb( BrowCtx *ctx, void *client_data);
    static int trace_connect_bc( brow_tObject object, char *name, char *attr, 
				 flow_eTraceType type, void **p);
    static int trace_disconnect_bc( brow_tObject object);
    static int trace_scan_bc( brow_tObject object, void *p);

    int create_object_item( pwr_tObjid objid, 
		brow_tNode dest, flow_eDest dest_code, void **item,
		int is_root);
    void print( char *filename);
    void zoom( double zoom_factor);
    void get_zoom( double *zoom_factor);
    void unzoom();
    int set_attr_value( brow_tObject node, pwr_tObjid objid, char *value_str);
    int check_attr_value( brow_tObject node, int *multiline,
	char **init_value, int *size);
    int set_object_name( brow_tObject node, pwr_tObjid objid, char *value_str);
    int check_object_name( brow_tObject node);
    int get_select( pwr_sAttrRef **attrref, int **is_attr, int *cnt);
    int get_selected_nodes( brow_tObject **sellist, int *sel_cnt);
    void message( char sev, const char *text);
    int brow_pop( wnav_eBrowType type);
    int brow_push();
    int brow_push_all();
    int brow_push_to_volume();
    int brow_push_volume();
    int object_exist( brow_tObject object);
    int is_empty();
    int find( pwr_tObjid objid, void **item);
    int display_object( pwr_tObjid objid);
    int setup();
    void force_trace_scan();
    void menu_tree_build( wnav_sStartMenu *root);
    wnav_sMenu *menu_tree_build_children( wnav_sStartMenu *first_child,
		wnav_sMenu *parent);
    void menu_tree_free();
    void menu_tree_free_children( wnav_sMenu *first_child);
    int menu_tree_search( char *name, wnav_sMenu **menu_item);
    int menu_tree_search_children( char *name, wnav_sMenu *child_list,
		wnav_sMenu **menu_item);
    int menu_tree_insert( char *title, int item_type, char *command,
		char *destination, int dest_code, wnav_sMenu **menu_item);
    int menu_tree_delete( char *name);
    int open_plc();
    int open_plc( pwr_tOid oid);
    int is_editmode() { return editmode;};
    void set_editmode( int value);
    int volume_attached( ldh_tWBContext wbcontext, ldh_tSesContext ldhsession, int pop);
    int volume_detached();
    void ldh_event( ldh_sEvent *event);
    void ldh_refresh( pwr_tObjid new_open);
    void refresh();
    void collapse();
    void set_options( int sh_class, int sh_alias, int sh_descrip, 
	int sh_objref, int sh_objxref, int sh_attrref, int sh_attrxref,
        int bu_force, int bu_debug, int bu_crossref, int bu_manual);
    void get_options( int *sh_class, int *sh_alias, int *sh_descrip, 
	 int *sh_objref, int *sh_objxref, int *sh_attrref, int *sh_attrxref,
         int *bu_force, int *bu_debug, int *bu_crossref, int *bu_manual);
    int save_settnings( ofstream& fp);
    int node_to_objid( brow_tNode node, pwr_tObjid *objid);
    int unselect_objid( pwr_tObjid objid);
    void enable_events( WNavBrow *brow);

    int show_database();
    int show_volume( int pop);
    int get_rootlist();
    int check_toplevel_class( pwr_tCid cid);
    int command( char* input_str);
    int readcmdfile( 	char		*incommand);
    int get_current_object(
			pwr_tObjid	*objid,
			char		*objectname,
			int		size,
			ldh_eName	nametype);
    int	find_name(
			char		*name,
			pwr_tObjid	*objid);
    int show_symbols();
    int search( char *search_str, int regexp);
    int search_root( char *search_str, pwr_tObjid *found_objid, int next);
    int search_object( pwr_tObjid objid, char *search_str, 
	pwr_tObjid *found_objid, int next);
    int search_next();
    int show_file( const char *filename, const char *intitle, int hide_dir);
    void select_object( brow_tObject object);
    int select_object( pwr_tOid oid);
    int get_next( pwr_tOid oid, wnav_eDestCode dest, pwr_tOid *next_oid, wnav_eDestCode *d);
    void set_select_visible();
    static int get_next_free_vid( pwr_tVid min_vid, pwr_tVid max_vid, pwr_tVid *next);
    static int check_new_vid( pwr_tVid vid);
    static int check_new_volumename( char *vname);

    // Crr module member functions
    int	crr_signal( char *filename, char *signalname, brow_tNode parent_node);
    int	crr_object( char *filename, char *objectname, brow_tNode parent_node);
    int	crr_code( char *filename, char *str, int brief, int func,
		  int case_sensitive);
    int display_objects( pwr_tCid *cidp, char *name, pwr_tObjid root,
			 int depth);
    void set_nodraw() { nodraw = 1;}
    void reset_nodraw() { nodraw = 0; refresh();}
};

int wnav_cut_segments (
  char	*outname,
  char	*name,
  int	segments);

int  wnav_attr_string_to_value( ldh_tSesContext ldhses, int type_id, 
	char *value_str, void *buffer_ptr, int buff_size, int attr_size);
void  wnav_attrvalue_to_string( ldh_tSesContext ldhses, int type_id, 
	void *value_ptr, char **buff, int *len);
char *wnav_get_message( int sts);

#endif
