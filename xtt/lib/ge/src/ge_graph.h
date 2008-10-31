/* 
 * Proview   $Id: ge_graph.h,v 1.35 2008-10-31 12:51:34 claes Exp $
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

#ifndef ge_graph_h
#define ge_graph_h

/* ge_graph.h -- Simple graphic editor */

#include <vector>

#ifndef pwr_h
# include "pwr.h"
#endif
#ifndef glow_h
#include "glow.h"
#endif

#ifndef glow_growctx_h
#include "glow_growapi.h"
#endif

#ifndef ge_graph_ccm_h
#include "ge_graph_ccm.h"
#endif

#ifndef ge_graph_journal_h
#include "ge_graph_journal.h"
#endif

#ifndef ge_attr_h
#include "ge_attr.h"
#endif

#if defined OS_VMS || defined OS_LINUX
#define LDH 1
#endif

#if LDH
# ifndef wb_ldh_h
#  include "wb_ldh.h"
# endif
#else
  typedef void *ldh_tSesContext;
  typedef void *ldh_tWBContext;
  typedef void *ldh_tVolContext;
#endif

/*! \file ge_graph.h
    \brief Contains the Graph class and some related classes GraphApplList, GraphRecallBuff, GraphGbl and GraphGrow. */
/*! \addtogroup Ge */
/*@{*/


#define graph_cVersion	"X3.0b"
#define GRAPH_GROW_MAX	25
#define glow_cJBean_Offset 2

//! Type of attributes. Should not collide with glow_eType
typedef enum {
  ge_eAttrType_DynType 		= glow_eType_DynType,  		//!< DynType.
  ge_eAttrType_DynTypeTone 	= 1001,				//!< DynType with color tone.
  ge_eAttrType_ActionType 	= glow_eType_ActionType,	//!< ActionType.
  ge_eAttrType_AnimSequence    	= 1003,				//!< Animation sequence.
  ge_eAttrType_LimitType    	= 1004,				//!< Limit type (Gt or Lt).
  ge_eAttrType_InstanceMask    	= 1005,				//!< Instance mask.
  ge_eAttrType_InputFocus 	= 1006,				//!< Initial input focus mask.
  ge_eAttrType_Dyn	 	= 1007,				//!< Dynamic data.
  ge_eAttrType_ScaleType      	= 1008,				//!< Scale type.
  ge_eAttrType_CurveDataType   	= 1009				//!< Data type for XY_Curve.
} ge_eAttrType;

//! Graph mode.
typedef enum {
  graph_eMode_Development,	//!< Editing mode.
  graph_eMode_Runtime		//!< Runtime mode.
} graph_eMode;

//! Databases
typedef enum {
  graph_eDatabase_Gdh,		//!< Database rtdb.
  graph_eDatabase_User,		//!< User defined database.
  graph_eDatabase_Local,	//!< Graph local database.
  graph_eDatabase_Ccm		//!< Scrip external variable database.
} graph_eDatabase;

//! Extension of proview type pwr_eType. Should not collide with any pwr_eType.
typedef enum {
  graph_eType_Bit = (1 << 15) + 1 //!< Type for a bit in a bitmask
} graph_eType;

typedef enum {
	graph_eTrace_Inherit		= 0,
	graph_eTrace_Dig		= 1,
	graph_eTrace_DigWithError	= 2,
	graph_eTrace_DigTone		= 3,
	graph_eTrace_DigToneWithError	= 4,
	graph_eTrace_Annot		= 5,
	graph_eTrace_DigWithText	= 6,
	graph_eTrace_Bar		= 7,
	graph_eTrace_Trend		= 8,
	graph_eTrace_DigBorder		= 9,
	graph_eTrace_AnnotWithTone 	= 10,
	graph_eTrace_DigTwo		= 11,
	graph_eTrace_DigToneTwo		= 12,
	graph_eTrace_Invisible		= 13,
	graph_eTrace_Rotate		= 14,
	graph_eTrace_AnalogShift	= 15,
	graph_eTrace_Animation		= 16,
	graph_eTrace_DigAnimation	= 17,
	graph_eTrace_AnimationForwBack	= 18,
	graph_eTrace_DigShift		= 19,
	graph_eTrace_Move		= 20,
	graph_eTrace_SliderBackground	= 21,
	graph_eTrace_Video		= 22,
	graph_eTrace_No		        = 999,
	graph_eTrace_SetDig		= 1000,
	graph_eTrace_ResetDig		= 1001,
	graph_eTrace_ToggleDig		= 1002,
	graph_eTrace_Slider		= 1003,
	graph_eTrace_AnnotInput		= 1004,
	graph_eTrace_Command		= 1005,
	graph_eTrace_CommandConfirm	= 1006,
	graph_eTrace_SetDigConfirm	= 1007,
	graph_eTrace_ResetDigConfirm	= 1008,
	graph_eTrace_ToggleDigConfirm	= 1009,
	graph_eTrace_SetDigWithTone	= 1010,
	graph_eTrace_ResetDigWithTone	= 1011,
	graph_eTrace_ToggleDigWithTone	= 1012,
	graph_eTrace_AnnotInputWithTone	= 1013,
	graph_eTrace_SetDigConfirmWithTone = 1014,
	graph_eTrace_ResetDigConfirmWithTone = 1015,
	graph_eTrace_ToggleDigConfirmWithTone = 1016,
	graph_eTrace_DigWithCommand	= 1017,
	graph_eTrace_DigWithErrorAndCommand = 1018,
	graph_eTrace_DigToneWithCommand	= 1019,
	graph_eTrace_DigToneWithErrorAndCommand	= 1020,
	graph_eTrace_StoDigWithTone	= 1021,
	graph_eTrace_DigTwoWithCommand	= 1022,
	graph_eTrace_DigToneTwoWithCommand = 1023,
	graph_eTrace_IncrAnalog 	= 1024,
	graph_eTrace_RadioButton 	= 1025,
	graph_eTrace_DigShiftWithToggleDig = 1026
	} graph_eTrace;

typedef struct {
	pwr_tFloat32	*pres_max_limit_p;
	pwr_tFloat32	*pres_min_limit_p;
	pwr_tFloat32	pres_max_limit_old;
	pwr_tFloat32	pres_min_limit_old;
	grow_tObject 	bar_object;
	grow_tObject 	trend_object;
	grow_tObject 	slider_object;
	grow_tObject 	slider_button_object;
	grow_tObject 	hold_button_object;
	float		*scan_time_p;
	float		old_scan_time;
	double		*data_scan_time_p;
	int		*hold_button_p;
	int		*go_button_p;
	int		*slider_button_p;
	int		*hold_p;
	int		*slider_disable_p;
	} graph_sObjectTrend;

typedef struct {
	pwr_tSubid	subid[6];
	char		format[16];
	graph_eTrace	type;
	glow_eDrawType	color;
	glow_eDrawType	color2;
	void		*p[6];
	int		inverted[6];
	char		old_value[6][80];
	int		size[6];
	graph_eDatabase	db[6];
	int		first_scan;
	double		scan_time;
	double		acc_time;
 	int		annot_typeid;
 	int		annot_size;
	unsigned int	access;
        glow_eCycle     cycle;
	char		high_text[80];
	char		low_text[80];
	double		x0;
	double		y0;
	double		x_orig;
	double		y_orig;
	double		factor;
	glow_eRotationPoint rotation_point;
	int		animation_count;
	int		animation_direction;
	int		trend_boolean;
	int		trend_hold;
	int		slider_disabled;
	} graph_sTraceData;


//! Variable item in local database.
typedef struct s_LocalDb {
  char	name[80];		//!< Variable name.
  int	type;			//!< Variable type (pwr_eType).
  char	value[80];		//!< Value.
  char	old_value[80];		//!< Old value.
  s_LocalDb *next;		//!< Next item.
} graph_sLocalDb;

#define RECALL_BUFF_SIZE 20

//! Recall buffer for component dynamics.
/*! Buffer used to store dynamics for groups when they are dissolved.
  Also used for store and recall of dynamics in attribute editor.
 */
class GraphRecallBuff {
  public:
	//! Constructor
	GraphRecallBuff() : cnt(0), size(RECALL_BUFF_SIZE)
             { memset(key,0,sizeof(key)); };
        GeDyn  		*buff[RECALL_BUFF_SIZE];	//!< Buffer with pointers to stored dynamic.
        GeDyn		*temporary;
        char            key[RECALL_BUFF_SIZE][80];	//!< Key to stored dynamic. Group or object name.
        int             cnt;				//!< Number or stored dynamics.
        int             size;				//!< Size of buffer.

	//! Store dynamics
	/*!
	  \param data	Dynamic data.
	  \param key    Key to data. Usually group or object name. Can be zero.
	  \param object	Object that is the owner of the dynamic data.
	*/
        void insert( GeDyn *data, const char *key, grow_tObject object);

	//! Get stored data with specified index.
	/*!
	  \param data	Stored data.
	  \param idx	Index.
	*/
        int get( GeDyn **data, int idx);

	//! Get stored data with specified key.
	/*!
	  \param data	Stored data.
	  \param key	Key for the data.
	*/
        int get( GeDyn **data, char *key);

	//! Destructor
	~GraphRecallBuff();
};

//! Entry in a list of opened applications.
/*! 
  The attribute editor is one type of application that is stored in the application list.
*/
class GraphApplList {
 public:
  
  //! Constructor
  /*!
    \param appl_key	Application key.
    \param appl_ctx	Application object.
  */
  GraphApplList( void *appl_key, void *appl_ctx) :
    key(appl_key), ctx(appl_ctx), prev(NULL), next(NULL) {};

  void	       		*key;		//!< Key.
  void			*ctx;		//!< Application object.
  GraphApplList		*prev;		//!< Previous element in list.
  GraphApplList		*next;		//!< Next element in list.

  //! Insert an application in list.
  /*! \param key	Key for application.
      \param ctx	Application object to insert. */
  void 		insert( void *key, void *ctx);

  //! Remove an application from list.
  /*! \param ctx	Application object to remove. */
  void 		remove( void *ctx);

  //! Find an applications with specified key.
  /*! \param key	Key to search for. */
  /*! \param ctx	Ctx for found key. */
  /*! \return		Returns 1 if key is found, else 0. */
  int find( void *key, void **ctx);

  //! Get the first element (after the root element)
  /*!
    \param ctx	The object of the first element.
    \return	1 if there is a first element, else 0.
  */
  int		get_first( void **ctx);
};


//! Not used
class GraphGbl {
  public:
    GraphGbl()
	{ 
	  strcpy( version, graph_cVersion);
	};
    char		version[10];
    int			load_config( void *graph);
};

class GraphRef {
 public:
  GraphRef( pwr_tAName name, pwr_tRefId *id, int size, glow_eCycle cycle, void **data) :
    m_id(id), m_size(size), m_cycle(cycle), m_data(data) {
    strcpy( m_name, name);
  }
  pwr_tAName m_name;
  pwr_tRefId *m_id;
  int m_size;
  glow_eCycle m_cycle;
  void **m_data;
};

//! Handling of a grow context.
/*! This class is originally made to handle multiple grow contexts in a stack,
  but for the moment graph only handles one context. 
*/
class GraphGrow {
 public:
  //! Constructor.
  /*!
    \param grow_ctx	Grow context.
    \param xn		Graph.
  */
  GraphGrow( GrowCtx *grow_ctx, void *xn) : ctx(grow_ctx), graph(xn), stack_cnt(0) {};

  //! Destructor.
  ~GraphGrow();

  GrowCtx	*ctx;		//!< Grow context.
  GrowCtx	*ctx_stack[10];	//!< Base grow context.
  void		*graph;		//!< Graph than owns the GraphGrow.
  int		stack_cnt; 	//!< Graph context is pushed.
 
  //! Setup grow for editmode.
  /*! Set attribute and enable events for edit mode. */
  void grow_setup();
 
  //! Setup grow for runtime mode.
  /*! Set attribute and enable events for runtime mode. */
  void grow_trace_setup();

  void pop( GrowCtx *context) { 
    if ( stack_cnt >= 10) {
      printf( "** Graph->grow stack overflow\n");
      return;
    }
    ctx_stack[stack_cnt++] = ctx;
    ctx = context; 
  }
  void push() {
    if ( stack_cnt > 0)
      ctx = ctx_stack[--stack_cnt];
  }
};


//! Class for the drawing area of Ge.
/*! ...
*/

class Graph {
 public:
  //! Constructor
  /*!
    \param xn_parent_ctx	Parent context.
    \param xn_parent_wid	Parent widget.
    \param xn_name		Name.
    \param w			Graph widget.
    \param status		Returned status.
    \param xn_default_path	Default path for .pwg files.
    \param graph_mode		Mode, development or runtime.
    \param scrollbar		Use scrollbars.
    \param xn_gdh_init_done	Gdh is alread initialized.
    \param xn_object_name	Object name for a class graph. If zero, this is not a class graph.
    \param xn_use_default_access Use the default access and not the access of the current user.
    \param xn_default_access	Default access. Can be used to override the access of the current user.
  */
  Graph(
	void *xn_parent_ctx,
	const char *xn_name,
	const char *xn_default_path,
	graph_eMode graph_mode = graph_eMode_Development,
	int xn_gdh_init_done = 0,
	const char *xn_object_name = 0,
	int xn_use_default_access = 0,
	unsigned int xn_default_access = 0);

  virtual void trace_timer_remove() {}
  virtual void trace_timer_add( int time) {}
  virtual Attr *attr_new( void *parent_ctx, void *object, attr_sItem *itemlist, int item_cnt) 
    { return 0;}
  virtual void popup_position( int event_x, int event_y, int *x, int *y) {}
  static void trace_scan( Graph *graph);

  GraphGbl		gbl;
  GraphApplList		attr_list;		//! List of opened applications, i.e. Attr windows.
  GraphRecallBuff     	recall;			//! Recall buffer for dynamics.
  void 			*parent_ctx;		//! Parent context.
  char 			name[300];		//! Name.
  pwr_tAName 	       	object_name;		//! Name of object for class graphs.
  GraphGrow		*grow;			//! GraphGrow
  GraphGrow		*grow_stack[GRAPH_GROW_MAX]; //! Grow stack. Not used.
  int			grow_cnt;		//! Number of grow in stack. Not used.
  ldh_tSesContext	ldhses;			//! Ldh session.
  GraphJournal		*journal;	       	//! Journal file.

  void 		(*message_cb)( void *, char, const char *);
  int 		(*get_current_subgraph_cb)( void *, char *, char *);
  void 		(*close_cb)( void *);
  void 		(*get_current_colors_cb)( void *, glow_eDrawType *, glow_eDrawType *, glow_eDrawType *);
  void 		(*set_current_colors_cb)( void *, glow_eDrawType, glow_eDrawType, glow_eDrawType);
  void 		(*init_cb)( void *);
  void 		(*cursor_motion_cb)( void *, double, double);
  void 		(*change_text_cb)( void *, void *, const char *);
  void 		(*change_name_cb)( void *, void *, char *);
  void 		(*change_value_cb)( void *, void *, char *);
  void 		(*confirm_cb)( void *, void *, char *);
  int 		(*command_cb)( void *, char *);
  void 		(*load_graph_cb)( void *, char *);
  int 		(*get_plant_select_cb)( void *, char *attr_name);
  void 		(*display_in_xnav_cb)( void *, pwr_sAttrRef *arp);
  void 		(*message_dialog_cb)( void *, const char *);
  int 		(*is_authorized_cb)( void *, unsigned int);
  int 		(*traverse_focus_cb)( void *, void *);
  int 		(*set_focus_cb)( void *, void *);
  int	       	(*get_ldhses_cb)( void *, ldh_tSesContext *, int);
  int	       	(*get_current_objects_cb)( void *, pwr_sAttrRef **, int **);
  void     	(*popup_menu_cb)(void *, pwr_sAttrRef, unsigned long,
				 unsigned long, char *, int x, int y); 
  int         	(*call_method_cb)(void *, char *, char *, pwr_sAttrRef,
				  unsigned long, unsigned long, char *);
  int         	(*sound_cb)(void *, pwr_tAttrRef *);
  int 		(*create_modal_dialog_cb)( void *, const char *, const char *, const char *, const char *, 
					   const char *, const char *);
  int			linewidth;		//!< Selected linewidth.
  glow_eLineType	linetype;		//!< Selected linetype.
  int			textsize;		//!< Selected text size.
  int			textbold;		//!< Text bold selected.
  glow_eFont	       	textfont;		//!< Text font selected.
  int			border_color;		//!< Selected border color.
  int			fill_color;		//!< Selected fill color.
  int			fill;			//!< Fill selected.
  int			border;			//!< Border selected.
  int			shadow;			//!< Shadow selected.
  int			grid;			//!< Snap to grid selected.
  double		grid_size_x;		//!< Grid size in x direction.
  double		grid_size_y;		//!< Grid size in y direction.
  glow_eConType		con_type;		//!< Selected connection type.
  glow_eCorner		con_corner;		//!< Selected connection corner style.
  glow_eDirection	conpoint_direction;	//!< Default conpoint direction.
  grow_tObject		current_polyline;	//!< Currently created polyline.
  grow_tObject		current_slider;		//!< Currnetly moved slider.
  int			trace_started;		//!< Trace is started.
  int			gdh_init_done;		//!< Gdh is initialized.
  gccm_s_arglist	arglist_stack[20]; 
  int			arglist_cnt;
  double		corner_round_amount;	//!< Selected corner round amount.
  graph_eMode		mode;			//!< Current edit mode.
  double		scan_time;		//!< Scantime for slow cycle.
  double		fast_scan_time;		//!< Scantime for fast cycle.
  double		animation_scan_time;	//!< Scantime for animations.
  char			default_path[80];	//!< Default path for .pwg files.
  char			filename[120];		
  int			closing_down;		//!< Desctructor is called.
  grow_tObject		current_mb1_down;	//!< Object for last MB1 down.
  int			slow_scan_cnt;		//!< Counter to calculate next slow scan.
  int			fast_scan_cnt;		//!< Counter to calculate next fast scan.
  int			displayed;		//!< Window is mapped.
  int			ccm_func_registred;	//!< ccm functions are registred.
  int			verify;			//!< Execute commandfiles with verify.
  int			scriptmode;		//!< Script is executed.
  grow_tObject		current_cmd_object;	//!< Current command object.
  void			*graph_object_data;	//!< Data for an object graph.
  void 			(*graph_object_scan)( Graph *graph); //!< Scan backcall for an object graph.
  void 			(*graph_object_close)( Graph *graph); //!< Close backcall for an object graph.
  graph_sLocalDb	*local_db;		//!< Local database.
  char                	systemname[80];		//!< System name
  int                 	use_default_access;	//!< Use default access an not the access of the current user.
  unsigned int        	default_access;		//!< Default access. Can be used to override the access of the current user.
  bool			keep_mode;		//!< Do not reset the edit mode when an object is created.
  char			confirm_text[200];	//!< Stored confirm text.
  GeDyn			*subgraph_dyn;		//!< Subgraph default dynamics.
  int			was_subgraph;		//!< Parameter to detect graph<->subgraph change.
  char			java_path[80];		//!< Path for generated java code for baseclasses
  char			java_package[80];      	//!< Package for generated java code for baseclasses
  vector<GraphRef>      reflist;		//!< List with stored references

  //! Print to postscript file.
  /*! \param filename	Name of postscript file. */
  void print( char *filename);

  //! Zoom with a specifed zoom factor.
  /*! \param zoom_factor	Zoom factor. */
  void zoom( double zoom_factor);

  //! Unzoom.
  /*! Return to base zoom factor. */
  void unzoom();

  //! Set edit mode.
  /*!
    \param mode		Edit mode.
    \param keep	        Do not reset the edit mode when an object is created.
  */
  void set_mode( grow_eMode mode, bool keep);

  //! Get edit mode.
  /*!
    \return	Edit mode.
  */
  grow_eMode get_mode()
    { return grow->ctx->mode();}

  //! Print a message
  /*!
    \param sev	Severity. 'E' for error, 'I' for information, 'W' for warning.
    \param text	Message text.
  */
  void message( char sev, const char *text);

  //! Print a status message
  /*!
    \param sts Status code.
  */
  void message( pwr_tStatus sts);

  int grow_pop();
  int grow_push();
  int grow_push_all();

  //! Set input focus to the window. Focus is marked by displaying the window border.
  /*!
    \param focus	1 set focus, 0 focus is removed.
  */
  virtual void set_inputfocus( int focus) {}

  int setup();

  //! Set snap to grid.
  /*! /param grid_on	Snap to grid. */
  void set_grid( int grid_on);

  //! Set linewidth.
  /*! \param width	Line width. */
  void set_linewidth( int width) { linewidth = width;};

  //! Set linetype.
  /*! \param type	Line type. */
  void set_linetype( glow_eLineType type) { linetype = type;};

  //! Set textsize.
  /*! \param size	Text size. */
  void set_textsize( int size) { textsize = size;};

  //! Set text bold.
  /*! \param bold	Bold text. */
  void set_textbold( int bold) { textbold = bold;};

  //! Set text font.
  /*! \param font	Text font. */
  void set_textfont( glow_eFont font) { textfont = font;};

  //! Set border color.
  /*! \param color_idx	Border color. */
  void set_border_color( int color_idx) { border_color = color_idx;};

  //! Set fill color.
  /*! \param color_idx	Fill color. */
  void set_fill_color( int color_idx) { fill_color = color_idx;};

  //! Set fill.
  /*! \param fill_on	Fill. */
  void set_fill( int fill_on) { fill = fill_on;};

  //! Set border.
  /*! \param border_on	Border. */
  void set_border( int border_on) { border = border_on;};

  //! Set shadow.
  /*! \param shadow_on	Draw object with shadow. */
  void set_shadow( int shadow_on) { shadow = shadow_on;};

  //! Set fill color on all selected objects.
  /*! Set currently selected fillcolor on all selected objects. */
  void set_select_fill_color();

  //! Set border color on all selected objects.
  /*! Set currently selected bordercolor on all selected objects. */
  void set_select_border_color();

  //! Set text color on all selected objects.
  /*! Set currently selected textcolor on all selected objects. */
  void set_select_text_color();

  //! Set color tone on all selected objects.
  /*! Set currently selected color tone on all selected objects. */
  void set_select_color_tone( glow_eDrawTone tone);

  //! Increase color lightness on all selected objects.
  /*! /param lightness 	Lightness increment. */
  void incr_select_color_lightness( int lightness);

  //! Increase color intensity on all selected objects.
  /*! /param intensity 	Intensity increment. */
  void incr_select_color_intensity( int intensity);

  //! Increase color shift on all selected objects.
  /*! /param shift      Shift increment. */
  void incr_select_color_shift( int shift);

  //! Set linewidth on all selected objects.
  /*! /param linewidth 	Linewidth. */
  void set_select_linewidth( int linewidth);

  //! Set linetype on all selected objects.
  /*! /param linetype 	Linewidth. */
  void set_select_linetype( glow_eLineType type);

  //! Set textsize on all selected objects.
  /*! /param textsize 	Text size. */
  void set_select_textsize( int textsize);

  //! Set text bold on all selected objects.
  /*! /param textbold	Text bold. */
  void set_select_textbold( int textbold);

  //! Set text font on all selected objects.
  /*! /param textfont	Text font. */
  void set_select_textfont( glow_eFont textfont);

  //! Set fill on all selected objects.
  /*! /param fill 	Fill. */
  void set_select_fill( int fill);

  //! Set border on all selected objects.
  /*! /param border 	Border. */
  void set_select_border( int border);

  //! Set shadow on all selected objects.
  /*! /param shadow 	Shadow. */
  void set_select_shadow( int shadow);

  //! Set backgound color.
  /*! Background color is set to the currently selected fill color. */
  void set_background_color();

  //! Set default backgound color.
  /*! Background color is set to a default color. */
  void set_default_background_color();

  //! Display gridpoints.
  /*! \param show	1 gridpoints are displayed, 0 gridpoints are hidden. */
  void set_show_grid( int show);

  //! Checi if gridpoints are displayed.
  /*! \return	1 gridpoints are displayed, 0 gridpoints are hidden. */
  int get_show_grid();

  //! Set backgound color in the navigation window.
  /*! Background color is set to the currently selected fill color. */
  void set_nav_background_color();

  //! Set name of the grow context.
  /*! \param name	Grow context name. */
  void set_name( char *name);

  //! Get the name of the grow context.
  /*! \param name	Grow context name. */
  void get_name( char *name);

  //! Save current graph to file.
  /*! \param filename	Name of file to save the graph in. */
  int save( char *filename);

  //! Save current subgraph to file.
  /*! \param filename	Name of file to save the graph in. */
  int save_subgraph( char *filename);

  //! Open a graph from file.
  /*! \param filename	Name of file to open graph from. */
  void open( char *filename);

  //! Open a subgraph from file.
  /*! \param filename	Name of file to open subgraph from. */
  void open_subgraph( char *filename);


  //! Set search path for subgraphs and images.
  /*!
    \param path_cnt		Number of paths in path_vect.
    \param path			Array of paths char[10][80].
  */
  void set_subgraph_path( int path_cnt, char *path);

  //! Find or create a conclass with the specified attributes.
  /*!
    \param drawtype	Color.
    \param linewidth	Linewidth.
    \param contype	Type of connection.
    \param corner	Type of corners, round or straight.
    \param round_amount	Size of arc in rounded corners.
    \param cc		Found or created connections class.
  */
  int get_conclass( glow_eDrawType drawtype, int linewidth,
	glow_eConType contype, glow_eCorner corner, double round_amount, 
	grow_tConClass *cc);

  //! Get the selected border color.
  /*! \return 	The selected border color. */
  glow_eDrawType get_border_drawtype();

  //! Get the selected fill color.
  /*! \return 	The selected fill color. */
  glow_eDrawType get_fill_drawtype();

  //! Get the selected text color.
  /*! \return 	The selected text color. */
  glow_eDrawType get_text_drawtype();

  //! Set connectionpoint direction.
  /*! \param dir	Connectionpoint direction. */
  void set_condir( glow_eDirection dir) { conpoint_direction = dir;};

  //! Set connection type.
  /*! \param type	Connection type. */
  void set_contype( glow_eConType type) { con_type = type;};

  //! Set connection corner.
  /*! \param corner	Connection corner, rounded or straight. */
  void set_concorner( glow_eCorner corner) { con_corner = corner;};

  //! Set connection corner round amount.
  /*! \param round_amount	Size of the arc in rounded corner. */
  void set_corner_round_amount( double round_amount) 
    { corner_round_amount = round_amount;};

  //! Open attribute editor for an object.
  /*! \param object	Object. */
  int edit_attributes( grow_tObject object);

  //! Get list of attributes for an object.
  /*! 
    \param object	Object.
    \param itemlist	List of attributes. 
    \param item_cnt	Number of attributes in list.
    \param client_data	Pointer to grow info list.
  */
  int get_attr_items( grow_tObject object, attr_sItem **itemlist,
		      int *item_cnt, void **client_data);

  //! Get list of attributes for a subgraph.
  /*! 
    \param itemlist	List of attributes. 
    \param item_cnt	Number of attributes in list.
    \param client_data	Pointer to grow info list.
  */
  int get_subgraph_attr_items( attr_sItem **itemlist,
		      int *item_cnt, void **client_data);

  //! Get list of attributes for a graph.
  /*! 
    \param itemlist	List of attributes. 
    \param item_cnt	Number of attributes in list.
    \param client_data	Pointer to grow info list.
  */
  int get_graph_attr_items( attr_sItem **itemlist,
		      int *item_cnt, void **client_data);

  //! Open attribute editor for a subgraph.
  /*! /return 	Always 1 */
  int edit_subgraph_attributes();

  //! Open attribute editor for a graph.
  /*! /return 	Always 1 */
  int edit_graph_attributes();

  //! Clear the graph.
  /*! Remove all objects and reset the graph. */
  void clear_all();

  //! Rotate selected objects.
  /*! \param angle	Rotation angle in degrees. */
  void rotate( double angle);

  //! Delete the selected object.
  /*! The selected objects are deleted. */
  void delete_select();

  //! Cut the selected object.
  /*! The selected objects are removed and put in the paste buffer. */
  void cut();

  //! Copy the seleted objects.
  /*! The selected objects are copied to the paste buffer. */
  void copy();

  //! Paste the last objects in cut or copy operation.
  /*! Copy objects from the paste list into the graph. */
  void paste();

  //! Initialize trace.
  /*! Initialize Gdh and trace. Execute the first scan. */
  int init_trace();

  //! Close trace.
  /*! \param reload	Refresh the graph by loading it from file. */
  void close_trace( int reload);

  //! Push selected objects.
  /*! The selected objects are pushed behind the other objects. */
  void push_select();

  //! Pop selected objects.
  /*! The selected objects are poped on top of the other objects. */
  void pop_select();

  //! Align selected objects.
  /*! \param direction	Alignment direction. */
  void align_select( glow_eAlignDirection direction);

  //! Set equal distance between selected objects.
  /*! \param direction 	Direction in which to order objects. */
  void equidistance_select( glow_eAlignDirection direction);

  //! Set default layout.
  /*! Adjust zoom factor to the current size of the window. */
  void set_default_layout();

  //! Set grid size.
  /*! \param gridsize	Distance between gridpoints. */
  void set_gridsize( double gridsize);

  //! Select all connections.
  /*! Select all connection objects in the graph. */
  void select_all_cons();

  //! Select all objects.
  /*! Select all objects in the graph. */
  void select_all_objects();

  //! Select next object. */
  void select_nextobject( glow_eDirection dir);

  //! Change the text of a text object.
  /*!
    \param object	Text object.
    \param text		New text.
  */
  void change_text( grow_tObject object, char *text);

  //! Change the name of an object.
  /*!
    \param object	Object to change name of.
    \param name		New object name.
  */
  void change_name( grow_tObject object, char *name);

  //! Change value for an object with ValueInput dyntype.
  /*!
    \param object	Object with ValueInput dynamic.
    \param text		Input text.
  */
  void change_value( grow_tObject object, char *text);

  //! Change text on the selected text object.
  /*! Exactly one text object has to be selected. */
  void change_select_text();

  //! Change name on the selected object.
  /*! Exactly one object has to be selected. */
  void change_select_name();

  //! Continue execution of an action after a confirm.
  /*! /param object	Object for thet current action execution. */
  void confirm_ok( grow_tObject object);

  void scale( double scalex, double scaley);

  //! Restrict movement of moved objects.
  /*! \param restriction	Type of restriction. */
  void set_move_restriction( glow_eMoveRestriction restriction);

  //! Get current movement restrictions.
  /*! \return	Type of restriction. */
  glow_eMoveRestriction get_move_restriction()
    { return grow_GetMoveRestrictions( grow->ctx);}


  //! Set or reset scaletype to equal scale.
  /*!
    \param equal	Scaleing of objects are equal in x and y direction.

    Scale equal means that when the scale of an objects is changed, the proportions in x and y
    direction is kept.
  */
  void set_scale_equal( int equal);

  //! Get current scaletype.
  /*!
    \return	If 1, scaleing of objects are equal in x and y direction.
  */
  int get_scale_equal()
    { return grow_GetScaleEqual( grow->ctx);}

  //! Check if object is a subgraph.
  /*! \return 		Returns 1 if object is a subgraph, else 0. */
  int is_subgraph();

  //! Connect an dynamic attribute to an attribute is rtdb.
  /*!
    \param object	Object which dynamic attributes is connected.
    \param attr_name	Rtdb attribute to connect to.
    \param second	Connect the second dynamic attribute. If 0, connect the first.
  */
  void connect( grow_tObject object, char *attr_name, int second);

  //! Get filename for a graph name. Att default path and .pwg.
  /*!
    \param inname	Graph name.
    \param outname	File name.
  */
  void get_filename( char *inname, char *outname);

  //! Check if the current user is authorized to activate an item with the specified access.
  /*! \param access	Access to check. */
  int is_authorized( unsigned int access);

  //! Get the selected object.
  /*!
    \param object	The seleted object.
    \return		GE__SUCCESS on success, GE_NOSELECT and GE__MANYSELECT if no or many object is selected,
  */
  int get_selected_object( grow_tObject *object);

  //! Check if graph is modified since last save.
  /*! \return 	1 if modified, 0 if not modified. */
  int is_modified();

  //! Set scantime for slow cycle.
  /*! \param time	Scantime in seconds. */
  void set_scantime( double time) { scan_time = time;};

  //! Parse a reference to a database attribute.
  /*!
    \param name		Database attribute to parse.
    \param parsed_name	Parsed name.
    \param inverted	The attribute is inverted.
    \param type		Type of attribute.
    \param size		Size of attribute.
    \param elem		Array element.

    The attribute reference is of type !VKV-P1-Str1.ActualValue##String80 which
    is inverted (!), of type pwr_eType_String with size 80.
  */
  graph_eDatabase parse_attr_name( char *name, char *parsed_name,
		int *inverted, int *type, int *size, int *elem = 0);

  //! Get the default window size
  /*!
    \param width	Window width in pixel.
    \param height	Window height in pixel.
    \return		Returns 1 if size is configured, otherwise 0.

    The window size is configured in x0, y0, x1 and y1. From these the window
    size in pixel is calculated.
  */
  int get_default_size( int *width, int *height);

  //! Convert a Proview type to string
  /*!
    \param type		Proview type.
    \param type_buf	The type as a string, i.e. "String", "Int".
    \param size		Size of the type.
  */
  int type_to_string( pwr_eType type, char *type_buf, int  *size);

  //! Convert a string to a Proview type.
  /*!
    \param type_str	Type string.
    \param type		Proview type.
    \param size		Size of type.
    \param elements	Elements.
  */
  void string_to_type( char *type_str, pwr_eType *type,
		int *size, int *elements);

  //! Get the stored system name.
  /*! \param name	System name. */
  void get_systemname( char *name) { strcpy( name, systemname);};

  //! Store the system name.
  /*! \param name	System name. */
  void set_systemname( char *name) { strcpy( systemname, name);};

  int get_argnames( char *code, char *argnames, int *argtypes, int *argcnt);
  int exec_dynamic( grow_tObject object, char *code, glow_eDynamicType type);

  //! Export subgraph as a javabean. This function probably needs some update.
  int export_javabean( char *filename, char *bean_name);

  //! Export graph as a java frame. This function probably nedds some update.
  int export_javaframe( char *filename, char *bean_name, int applet, int html);

  //! Export java code for an object's dynamics and annotations.
  /*!
    \param fp		Output file.
    \param object	Object.
    \param cnt		Index for javabean name.
  */
  int export_GejavaObjectTraceAttr( ofstream& fp, grow_tObject object, int cnt);

  //! Export java code for an object's dynamics and annotations in a java frame. Needs update.
  int export_ObjectTraceAttr( ofstream& fp, grow_tObject object, int cnt);

  //! Export java code for dynamics of a bar object.
  /*!
    \param fp		Output file.
    \param object	Object.
    \param cnt		Index for javabean name.
  */
  int export_BarTraceAttr( ofstream& fp, grow_tObject object, int cnt);

  //! Export java code for dynamics of a trend object.
  /*!
    \param fp		Output file.
    \param object	Object.
    \param cnt		Index for javabean name.
  */
  int export_TrendTraceAttr( ofstream& fp, grow_tObject object, int cnt);

  //! Export java code for dynamics of a table object.
  /*!
    \param fp		Output file.
    \param object	Object.
    \param cnt		Index for javabean name.
  */
  int export_TableTraceAttr( ofstream& fp, grow_tObject object, int cnt);

  //! Export java code for dynamics of a slider object.
  /*!
    \param fp		Output file.
    \param object	Object.
    \param cnt		Index for javabean name.
  */
  int export_SliderTraceAttr( ofstream& fp, grow_tObject object, int cnt);

  //! Export java for a ge graph.
  /*!
    \param filename    	Filname for java code.
    \param bean_name	Name of java class.
    \param applet	1 export as applet, 0 export as frame.
    \param html		Creata a html page for applet.
  */
  int export_gejava( char *filename, char *bean_name, int applet, int html);

  //! Export java for a nodeclass in a ge graph.
  /*!
    \param fp		Output file.
    \param nodeclass	Nodeclass to export.
  */
  int export_gejava_nodeclass( ofstream& fp, grow_tNodeClass nodeclass);

  //! Set java class name for the graph
  /*! \param name	Java class name. */
  void set_java_name( const char *name);

  //! Set java class name for the graph
  /*! \param name	Java class name. */
  int get_java_name( char *name);

  //! Get the name of the next subgraph (subgraph of next page).
  /*! \param name	Name of next subgraph. */
  void get_next_subgraph( char *name);

  //! Set the name of the next subgraph (subgraph of next page).
  /*! \param name	Name of next subgraph. */
  void set_next_subgraph( const char *name);

  //! Store the current zoomfactor and offsets.
  /*! The stored geometry is restored with restore_geometry(); */
  void store_geometry();

  //! Restore the stored geometry.
  /*! Restore the zoomfactor and offsets previously stored with store_geometry(). */
  void restore_geometry();

  //! Set no draw
  /*!
    Nothing will be drawn in the window until reset_nodraw() is called.
    After reset_nodraw() the function redraw() should be called to update the window.
  */
  void set_nodraw() { grow_SetNodraw( grow->ctx);};

  //! Reset no draw.
  /*! Reset of a previos call to set_nodraw. */
  void reset_nodraw() { grow_ResetNodraw( grow->ctx);};

  //! Redraw the window.
  /*! Redraw the background and all objects. */
  void redraw() { grow_Redraw( grow->ctx);};

  //! Group the selected objects.
  /*!
    \param object	Created group.
    \param last_group   Name of last group.

    The last group is the group name registred as last group among the majority of the
    group members
  */
  int group_select( grow_tObject *object, char *last_group);

  int set_recall_data( grow_tObject object, char *name);
 
  //! Ungroup selected groups.
  /*!
    \param force	Ungroup even if the group has dynamic data.
    \return		GE__GROUPDYNDATA if force is 0 and the group has dynamic data, else 1.
  */
  int ungroup_select( int force);

  //! Open an input field for an object with ValueInput action.
  /*!
    \param name		Name of the object.
    \param empty	Input field should be empty.
  */
  int set_object_focus( const char *name, int empty);

  //! Replace the string $object with the object name for the graph.
  /*!
    \param in	Input command.
    \param out	Converted command.
    \param dyn  Dynamics.
  */
  void get_command( char *in, char *out, GeDyn *dyn);

  //! Check if graph is configured to be exported as a java applet.
  /*! \return		1 if graph can be exported as a java applet, else 0. */
  int is_javaapplet();

  //! Check if graph is configured to be exported as a java frame.
  /*! \return		1 if graph can be exported as a java frame, else 0. */
  int is_javaapplication();

  //! Translate a reference name.
  /*!
    \param name		Attribute name.
    \param tname	Converted name.
  */
  int get_reference_name( char *name, char *tname);

  //! Subscribe or link to an attribute in rtdb.
  /*!
    \param cycle	Cycle for the dynamics. This sets the subscription time.
    \param name		Attribute name.
    \param data		Pointer to data of the attribute.
    \param subid	Subid.
    \param size		Size of the attribute.
  */
  int ref_object_info( glow_eCycle cycle, char *name, void **data,
		       pwr_tSubid *subid, unsigned int size, bool now = false);

  //! Subscribe all stored subscriptions.
  int ref_object_info_all();

  //! Flip the selected objects.
  /*! \param dir	Flip direction, vertical or horizontal. */
  void flip( glow_eFlipDirection dir);

  //! Create a trend object.
  /*!
    \param object	Created trend object.
    \param x		x coordinate for object.
    \param y		y coordinate for object.
    \param dyn_type	Dyntype of the created object.
  */
  void create_trend( grow_tObject *object, double x, double y, unsigned int dyn_type);

  //! Create a xy curve object.
  /*!
    \param object	Created xy curve object.
    \param x		x coordinate for object.
    \param y		y coordinate for object.
    \param dyn_type	Dyntype of the created object.
  */
  void create_xycurve( grow_tObject *object, double x, double y, unsigned int dyn_type);

  //! Create a bar object.
  /*!
    \param object	Created bar object.
    \param x		x coordinate for object.
    \param y		y coordinate for object.
  */
  void create_bar( grow_tObject *object, double x, double y);

  //! Create a window object.
  /*!
    \param object	Created bar object.
    \param x		x coordinate for object.
    \param y		y coordinate for object.
  */
  void create_window( grow_tObject *object, double x, double y);

  //! Create a table object.
  /*!
    \param object	Created bar object.
    \param x		x coordinate for object.
    \param y		y coordinate for object.
  */
  void create_table( grow_tObject *object, double x, double y);

  //! Create a folder object.
  /*!
    \param object	Created bar object.
    \param x		x coordinate for object.
    \param y		y coordinate for object.
  */
  void create_folder( grow_tObject *object, double x, double y);

  //! Create an axis object.
  /*!
    \param object	Created axis object.
    \param x		x coordinate for object.
    \param y		y coordinate for object.
  */
  void create_axis( grow_tObject *object, double x, double y);

  //! Set displayed folder in a folder object.
  /*!
    \param name		Object name of folder object.
    \param idx		Index of folder to display.
  */
  int set_folder_index( const char *name, int idx);

  //! Set graph source file for a window object.
  /*!
    \param name		Object name of window object.
    \param source      	Name of source graph.
  */
  int set_subwindow_source( const char *name, char *source);

  //! Play a sound.
  /*!
    \param aref		Pointer to sound object aref.
  */
  int sound( pwr_tAttrRef *aref);

  //! Export as plc functionobject.
  /*!
    \param filename    	Filename.
  */
  int export_plcfo( char *filename);

  //! Store in journal file.
  int journal_store( journal_eAction a, grow_tObject o) { 
    if (journal) return journal->store( a, o);
    else return 1;
  }


  //
  // Command module
  //

  //! Exectue a command.
  /*! \param input_str	Command. */
  int command( char* input_str);

  //! Execute a script file.
  /*!  \param incommand		Scriptfile with arguments. */
  int readcmdfile( 	char		*incommand);

  //
  // Object graph module
  //

  //! Initialize an object graph.
  /*!
    \param mode		0: call before trace is initialized, 1: after.

    The function is called twice, before trace is initialized, and after.
  */
  int init_object_graph( int mode);

  //! Set a command to a command button.
  /*!
    \param button_name	Name of command button object.
    \param cmd		Command to set in the button.
  */
  int set_button_command( const char *button_name, const char *cmd);

  //! Initialize trend, bar, slider hold and disable buttons, min and maxlimit buttons, for an object graph.
  /*!
    \param td		Trend data.
    \param arp		Pointer to attrref.

    The funcion looks for an bar named "ActualValueBar, a trend named "ActualValueTrend", a slider
    named "ActualValueSlider", buttons named "TrendHold" and "SliderDisable", and more buttons
    named "PresMaxLimit" and "PresMinLimit".
  */
  int trend_init( graph_sObjectTrend *td, pwr_sAttrRef *arp);

  //! Scan a trend object and check for changes in configuration.
  /*!
    \param td	Trend data.

    Reconfigures the bar and trend objects if limits are changed. Reconfigures
    the trend if new scantime is set. Changes color of hold an disable slider buttons.
  */
  void trend_scan( graph_sObjectTrend *td);

  //! Add an variable to local database.
  /*!
    \param name		Variable name.
    \param type		Variable type (pwr_eType).
  */
  graph_sLocalDb *localdb_add( const char *name, int type);

  //! Get a variable in local database.
  /*!
    \param name		Variable name.
    \param item		Variable item.
  */
  int localdb_find( const char *name, graph_sLocalDb **item);

  //! Free whole local database.
  void localdb_free();

  //! Reference or create a variable i local database.
  /*!
    \param name		Variable name.
    \param type		Variable type (pwr_eType).
  */
  void *localdb_ref_or_create( const char *name, int type);

  //! Set the value of a variable in local database.
  /*!
    \param name		Variable name.
    \param value	Value to set.
    \param size		Size of value (max size is 80 bytes).
  */
  int localdb_set_value( char *name, void *value, int size);
  int localdb_toggle_value( char *name);

  //! Create a GrowNode or GrowSlider object.
  /*!
    \param node_name	Name of node. If zero, a standard objectname is taken for the node.
    \param subgraph_str	Name of subgraph.
    \param x1		x coordinate of lower left corner.
    \param y1		y coordinate of lower left corner.
    \param x2		x coordinate of upper right corner. 
    \param y2		y coordinate of upper right corner.
    \param node		Created node object.
  */
 int create_node( const char *node_name, const char *subgraph_str, double x1, double y1, 
		     double x2, double y2, grow_tNode *node);


 //
 // Web module
 //

 //! Generate web site from configuration in database.
 /*! \param ldhses	Ldh session. */
 static int generate_web( ldh_tSesContext ldhses);

 //! Generate web help from xtt_help.
 static int generate_web_help();

 //
 // Convert module
 //

 //! Conversion

 //! Conversion between different versions.
 /*! Convert a graph to lates revision of Ge. */
 int convert();

 //! Conversion of an object between different versions.
 /*! \param object	Object to convert. */
 int convert_object( grow_tObject object);

 //! Soft restart
 /*! \param mode  0: swap starting, 1: swap done. */
 void swap( int mode);
 
 int ccm_set_variable( char *name, int type, void *data);
 int ccm_get_variable( char *name, int type, void *data);
 int ccm_ref_variable( char *name, int type, void **data);

 //! Destructor
 /*! Stop trace (if started), delete open attribute editors, free local database, delete grow and
   destroy the widget.
 */
 virtual ~Graph();
};

int graph_init_grow_base_cb( GlowCtx *fctx, void *client_data);

//! Convert a string to a value
/*!
  \param type_id	Type of value (pwr_eType).
  \param value_str	String to convert.
  \param buffer_ptr	Buffer to put the value in.
  \param buff_size	Size of buffer.
  \param attr_size	Attribute size.
*/
int  graph_attr_string_to_value( int type_id, const char *value_str, 
	void *buffer_ptr, int buff_size, int attr_size);

/*@}*/
#endif
