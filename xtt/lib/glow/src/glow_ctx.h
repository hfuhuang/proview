#ifndef glow_ctx_h
#define glow_ctx_h

#include <string.h>

#include "glow.h"
#include "glow_pscript.h"
#include "glow_array.h"
#include "glow_tracedata.h"
#include "glow_tiptext.h"
#include "glow_color.h"

/*! \file glow_ctx.h
    \brief Contains the GlowCtx class. */
/*! \addtogroup Glow */
/*@{*/

//! Backcall data for scrollbar reconfiguration
typedef struct {
  void	*scroll_data;	//!< Scrollbar data (growwidget_sScroll).
  int	total_width;	//!< Total width of work area
  int	total_height;	//!< Total height of work area
  int	window_width;	//!< Window width
  int	window_height;	//!< Window height
  int	offset_x;	//!< Offset from workarea border to displayed window.
  int	offset_y;	//!< Offset from workarea border to displayed window.
} glow_sScroll;

typedef void (*glow_tUserDataSaveCb) ( ofstream *, void *, glow_eUserdataCbType);
typedef void (*glow_tUserDataOpenCb) ( ifstream *, void *, glow_eUserdataCbType);
typedef void (*glow_tUserDataCopyCb) ( void *, void *, void **, glow_eUserdataCbType);

//! Class for a drawing window populated with drawing objects and components.
/*! GlowCtx is the base class for the drawing area in Glow. It contains array with
  all the nodes and connections, arrays for selected objects, objects that are currently
  moved, and object in the paste buffer.
  The event handler detects event in each object and send enabled event to the caller.
*/
class GlowCtx {
 public:
  //! Constructor
  /*!
    \param ctx_name	Name of context.
    \param zoom_fact	Initial zoom factor.
    \param offs_x	Initial offset for x-coordinate.
    \param offs_y	Initial offset for y-coordinate.
  */
  GlowCtx( char *ctx_name, double zoom_fact = 100, int offs_x = 0, int offs_y = 0);

  glow_eCtxType ctx_type;		//!< Type of context
  double 	zoom_factor_x;		//!< Zoom factor in x direction.
  double 	zoom_factor_y;		//!< Zoom factor in y direction.
  double 	base_zoom_factor;	//!< Original zoom factor.
  int		offset_x;		//!< Offset in pixel between origo and displayed window in x direction.
  int		offset_y;		//!< Offset in pixel between origo and displayde window in y direction.
  double 	nav_zoom_factor_x;	//!< Zoom factor in navigation window in x direction.
  double 	nav_zoom_factor_y;	//!< Zoom factor in navigation window in y direction.
  double 	print_zoom_factor;	//!< Zoom factor when printing to postscript.
  int		nav_offset_x;		//!< Offset in navigation window in x direction.
  int		nav_offset_y;		//!< Offset in navigation window in y direction.
  double	x_right;		//!< Right border of work area.
  double	x_left;			//!< Left border of work area.
  double	y_high;			//!< High border of work area.
  double	y_low;			//!< Low border of work area.
  int		window_width;		//!< Window width in pixel.
  int		window_height;		//!< Window height in pixel.
  int		nav_window_width;	//!< Navigation window width in pixel.
  int		nav_window_height;	//!< Navigation window height in pixel.
  int		nav_rect_ll_x;		//!< x coordinate for lower left corner of navigation rectangle in nav window.
  int 		nav_rect_ll_y;		//!< y coordinate for lower left corner of navigation rectangle in nav window.
  int		nav_rect_ur_x;		//!< x coordinate for upper right corner of navigation rectangle in nav window.
  int		nav_rect_ur_y; 		//!< y coordinate for upper right corner of navigation rectangle in nav window.
  int		nav_rect_hot;		//!< Cursor is in navigation rectangle in navigation window.
  void		*draw_ctx;		//!< Draw context.
  void		*glow_window;		//!< Not used.

  //! Save context to file.
  /*!
    \param filename	Name of file.
    \param mode		Save mode. Save as graph or subgraph.
    \return		1 if save was successfull, else even return.

    Saves the context with all nodeclasses, conclasses and objects.
  */
  int save( char *filename, glow_eSaveMode mode);

  //! Open context from file.
  /*!
    \param filename	Name of file.
    \param mode		Not used.
    \return		1 if success, else even return.
  */
  int open( char *filename, glow_eSaveMode mode);

  //! Insert an object.
  /*!
    \param element	Object to insert.
    \return		Returns 1 if success, 0 if object already is inserted.
  */
  int insert( GlowArrayElem *element) 
		{ return a.insert( element);};

  //! Remove an object.
  /*! \param element	Object to remove. */
  void remove( GlowArrayElem *element)
		{ a.remove(element);};

  //! Remove and delete all objects.
  void delete_all();

  //! Delete object. Object is removed and deletet.
  /*! \param element	Object to delete. */
  void delete_object( GlowArrayElem *element) 
		{ remove(element); select_remove(element); move_remove(element);
		delete element;}

  //! Insert a nodeclass.
  /*! 
    \param element	Nodeclass to insert.
    \return		Returns 1 if success, 0 if nodeclass already is inserted.
  */
  int nodeclass_insert( GlowArrayElem *element) 
		{ return a_nc.insert( element);};

  //! Insert a connection class.
  /*! 
    \param element	Connection class to insert.
    \return		Returns 1 if success, 0 if conclass already is inserted.
  */
  int conclass_insert( GlowArrayElem *element) 
		{ return a_cc.insert( element);};

  //! Find an object.
  /*! 
    \param element	Object to search for.
    \return		1 if object is found, else 0.
  */
  int find( GlowArrayElem *element)
		{ return a.find(element);};

  //! Insert an object in select list.
  /*!
    \param element	Object to insert.
    \return		Returns 1 if success, 0 if object already is inserted.
  */
  int select_insert( GlowArrayElem *element) 
		{ return a_sel.insert( element);};

  //! Remove an object from selection list.
  /*! \param element	Object to remove. */
  void select_remove( GlowArrayElem *element)
		{ a_sel.remove(element);};

  //! Clear selection list.
  /*! Remove all objects from selection list. */
  void select_clear();

  //! Find an object in selection list.
  /*!
    \param element	Object to serach for.
    \return		Returns 1 if object is found, else 0.
  */
  int select_find( GlowArrayElem *element)
		{ return a_sel.find(element);};

  //! Insert an object in list of currently moved objects.
  /*!
    \param element	Object to insert.
    \return		Returns 1 if success, 0 if object already is inserted.
  */
  int move_insert( GlowArrayElem *element) 
		{ return a_move.insert( element);};

  //! Remove an object from list or currently moved objects.
  /*! \param element	Object to remove. */
  void move_remove( GlowArrayElem *element)
		{ a_move.remove(element);};

  //! Clear list of currently moved objects.
  /*! Remove all objects from list. */
  void move_clear()
		{ a_move.clear();};

  //! Find an object in list of currently moved objects.
  /*!
    \param element	Object to serach for.
    \return		Returns 1 if object is found, else 0.
  */
  int move_find( GlowArrayElem *element)
		{ return a_move.find(element);};


  //! Insert an object in paste list.
  /*!
    \param element	Object to insert.
    \return		Returns 1 if success, 0 if object already is inserted.
  */
  int paste_insert( GlowArrayElem *element) 
		{ return a_paste.insert( element);};

  //! Remove an object from paste list.
  /*! \param element	Object to remove. */
  void paste_remove( GlowArrayElem *element)
		{ a_paste.remove(element);};

  //! Clear selection list.
  /*! Remove all objects from paste list. */
  void paste_clear()
		{ a_paste.clear();};  // todo Delete objects also !!!


  //! Find an object in paste list.
  /*!
    \param element	Object to serach for.
    \return		Returns 1 if object is found, else 0.
  */
  int paste_find( GlowArrayElem *element)
		{ return a_paste.find(element);};

  //! Paste object in pastelist into the window.
  /*! Call paste execute now, or when the cursor enters the window. */
  void paste();

  //! Exectute the paste.
  /*! The cursor must have entered the window before the object is pasted.
    Copies all objects in paste list and inserts them into the context and into the move list.
  */
  void paste_execute();

  //! Cut selected objects.
  /*! Clear paste list and move obejcts from select list to the pastelist. Remove them from the context. */
  void cut();

  //! Copy selected object.
  /*! Copy selected objects into the paste list. */
  void copy();

  //! Set highlight on or off on all objects.
  /*! \param on		1 set highlight on, 0 set off. */
  void set_highlight( int on)
		{ a.set_highlight( on);};

  //! Set inverse on or off on all objects.
  /*! \param on		1 set inverse on, 0 set off. */
  void set_inverse( int on)
		{ a.set_inverse( on);};

  //! Set highlight on or off on all selected objects.
  /*! \param on		1 set highlight on, 0 set off. */
  void set_select_highlight( int on)
		{ a_sel.set_highlight( on);};

  //! Set inverse on or off on all objects.
  /*! \param on		1 set inverse on, 0 set off. */
  void set_select_inverse( int on)
		{ a_sel.set_inverse( on);};

  void node_movement( GlowArrayElem *node, int x, int y);

  //! Registration of the source node in a connection creation sequence.
  /*!
    \param node		The source node for the connection.
    \param cp_num	Connected connection point on node.
    \param cp_x		x coordinate of connection point.
    \param cp_y		y coordinate of connection point.
  */
  void con_create_source( GlowArrayElem *node, int cp_num, int cp_x, int cp_y); 

  //! Registration of the destination node in a connection creation sequence.
  /*!
    \param node		The destination node for the connection.
    \param cp_num	Connected connection point on node.
    \param event	Event.
    \param x		x coordinate of connection point.
    \param y		y coordinate of connection point.
  */
  void con_create_dest( GlowArrayElem *node, int cp_num, glow_eEvent event, int x, int y); 

  //! Get the selection list.
  /*!
    \param list		Returned pointer to the selection list.
    \param size		Number of objects in the selection list.
  */
  void get_selectlist( GlowArrayElem ***list, int *size)
		{ *list = a_sel.a; *size = a_sel.size();}; 

  //! Get the object list.
  /*!
    \param list		Returned pointer to the object list.
    \param size		Number of objects in the object list.
  */
  void get_objectlist( GlowArrayElem ***list, int *size)
		{ *list = a.a; *size = a.size();}; 

  //! Set gridsize.
  /*!
    \param size_x	Grid size in x direction.
    \param size_y	Grid size in y direction.
  */
  void set_gridsize( double size_x, double size_y) 
		{ grid_size_x = size_x; grid_size_y = size_y;};

  //! Set snap to grid on or off.
  /*! \param on		1 snap is on, 0 snap is off. */
  void set_grid( int on) { grid_on = on;};

  //! Find the closest grid point.
  /*!
    \param x		x coordinate.
    \param y		y coordinate.
    \param x_grid	x coordiante of closest grid point.
    \param y_grid	y coordinate of closest grid point.
  */
  void find_grid( double x, double y, double *x_grid, double *y_grid);

  GlowArrayElem *node_moved;			//!< Not used.
  int		node_move_last_x;		//!< Last x position of the cursor in a move sequence.
  int		node_move_last_y;		//!< Last y position of the cursor in a move sequence.
  int		node_movement_active;		//!< Movement sequence is active.
  int		node_movement_paste_active;	//!< Movement sequence of pasted node is active.
  int		node_movement_paste_pending;	//!< Execution of paste is pending.
  int		nav_rect_move_last_x;		//!< Last x position of the cursor when moving navigation rectangle in nav window.
  int		nav_rect_move_last_y;		//!< Last x position of the cursor when moving navigation rectangle in nav window.
  int		nav_rect_movement_active;	//!< Movement of navigation rectangle in nav window active.
  int		nav_rect_zoom_active;		//!< Zoom with navigation rectangle in nav window active.
  int		select_rect_active;		//!< Selection rectangle active.
  glow_eEvent	select_rect_event;		//!< Event for selection rectangle (select or add select).
  int		select_rect_last_x;		//!< Last x position of cursor for select rectangle in pixel.
  int		select_rect_last_y;		//!< Last y position of cursor for select rectangle in pixel.
  int		select_rect_start_x;		//!< x start position of cursor for select rectangle in pixel.
  int		select_rect_start_y;		//!< y start position of cursor for select rectangle in pixel.
  int		select_rect_ll_x;		//!< x coordinate for lower left corner of selection rectangle in pixel.
  int		select_rect_ll_y;		//!< y coordinate for lower left corner of selection rectangle in pixel.
  int		select_rect_ur_x;		//!< x coordinate for upper right corner of selection rectangle in pixel.
  int		select_rect_ur_y;		//!< y coordinate for upper right corner of selection rectangle in pixel.
  double	select_area_ll_x;		//!< x coordinate for lower left corner of last selected area.
  double	select_area_ll_y;		//!< y coordinate for lower left corner of last selected area.
  double	select_area_ur_x;		//!< x coordinate for upper right corner of last selected area.
  double	select_area_ur_y;		//!< y coordinate for upper right corner of last selected area.
  GlowArrayElem *con_create_node;		//!< Source node in a connection creation sequence.
  int		con_create_last_x;		//!< Last x position in cursor for connection creation sequence.
  int		con_create_last_y;		//!< Last y position in cursor for connection creation sequence.
  int		con_create_conpoint_num;	//!< Number for conpoint in source node for connection creation sequence.
  int		con_create_conpoint_x;		//!< x coordinate for conpoint in connection creation sequence.
  int		con_create_conpoint_y;		//!< y coordinate for conpoint in connection creation sequence.
  int		con_create_active;		//!< Connection creation sequence active.
  int		auto_scrolling_active;		//!< Auto scrolling active.
  void		*auto_scrolling_id;		//!< Autoscrolling timer id.

  //! Stop autoscrolling.
  /*! Autoscrolling started with auto_scrolling() fuction is stopped with this function. */
  void	auto_scrolling_stop();

  //! Zoom with a specifed zoom factor.
  /*! \param factor	Zoom factor. */
  void 	zoom( double factor);

  //! Zoom in x direction with a specified zoom factor.
  /*! \param factor	Zoom factor in x direction. */
  void 	zoom_x( double factor);

  //! Zoom in y direction with a specified zoom factor.
  /*! \param factor	Zoom factor in y direction. */
  void 	zoom_y( double factor);

  //! Zoom to an absolut zoom factor.
  /*! \param factor	Absolute zoom factor. */
  void 	zoom_absolute( double factor);

  //! Move the view of the working space.
  /*!
    \param x	Distance to move in x direction in pixel.
    \param y	Distance to move in y direction in pixel.
  */
  void 	traverse( int x, int y);

  //! Get the borders of the working space.
  /*! The borders are stored in x_right, x_left, y_low and y_high. */
  void 	get_borders();

  //! Set default connection class. This is used as default when a connection is created.
  /*! \param cc 	Connection class. */
  void	set_default_conclass( void *cc) { default_conclass=cc;};

  //! Get default connection class. This is used as default when a connection is created.
  /*! \return	The default connection class. */
  void	*get_default_conclass() {return default_conclass;};

  //! Print to postscript. Not implemented.
  void 	print( double  ll_x, double  ll_y, double  ur_x, double  ur_y);

  //! Print to postscript.
  void 	print( char *filename, double x0, double x1, int end);

  //! Redraw an area of the window.
  /*!
    \param ll_x		x coordiate for lower left corner of area to draw in pixel.
    \param ll_y		y coordiate for lower left corner of area to draw in pixel.
    \param ur_x		x coordiate for upper right corner of area to draw in pixel.
    \param ur_y		y coordiate for upper right corner of area to draw in pixel.
  */
  void 	draw( int ll_x, int ll_y, int ur_x, int ur_y);

  //! Redraw an area of the window. Arguments in double.
  /*!
    \param ll_x		x coordiate for lower left corner of area to draw in pixel.
    \param ll_y		y coordiate for lower left corner of area to draw in pixel.
    \param ur_x		x coordiate for upper right corner of area to draw in pixel.
    \param ur_y		y coordiate for upper right corner of area to draw in pixel.
  */
  void        draw( double ll_x, double ll_y, double ur_x, double ur_y)
                           {draw( (int)ll_x, (int)ll_y, (int)ur_x, (int)ur_y);};

  //! Clear the window.
  /*! Draw background color. */
  void 	clear();

  //! Update zoom of navigation window.
  /*! The zoomfactor of the navigation window is updated to match the extent of the working space. */
  void	nav_zoom();

  //! Zoom to appropriate scale for postscript output.
  void	print_zoom();

  //! Clear the navigation window.
  /*! Draw background color. */
  void	nav_clear();


  //! Redraw an area of the navigation window.
  /*!
    \param ll_x		x coordiate for lower left corner of area to draw in pixel.
    \param ll_y		y coordiate for lower left corner of area to draw in pixel.
    \param ur_x		x coordiate for upper right corner of area to draw in pixel.
    \param ur_y		y coordiate for upper right corner of area to draw in pixel.
  */
  void	nav_draw( int ll_x, int ll_y, int ur_x, int ur_y);

  //! Redraw an area of the navigation window. Arguments in double.
  /*!
    \param ll_x		x coordiate for lower left corner of area to draw in pixel.
    \param ll_y		y coordiate for lower left corner of area to draw in pixel.
    \param ur_x		x coordiate for upper right corner of area to draw in pixel.
    \param ur_y		y coordiate for upper right corner of area to draw in pixel.
  */
  void        nav_draw( double ll_x, double ll_y, double ur_x, double ur_y)
    		{nav_draw( (int)ll_x, (int)ll_y, (int)ur_x, (int)ur_y);};

  //! Handle events.
  /*! Calls the event handler of GrowCtx. */
  int	 	event_handler( glow_eEvent event, int x, int y,
			int w, int h);

  //! Handle events in the navigation window.
  /*! Handle the event for moving and scaling the navigation rectangle. */
  int	 	event_handler_nav( glow_eEvent event, int x, int y);

  //! Enable an event en register a backcall function for the event.
  /*!
    \param event	Event.
    \param event_type	Type of event.
    \param event_cb	Callback function for the event.
  */
  void	enable_event( glow_eEvent event, glow_eEventType event_type,
		  int (*event_cb)( GlowCtx *ctx, glow_tEvent event));

  //! Disable an event.
  /*! \param event	Event to disable. */
  void	disable_event( glow_eEvent event);

  //! Disable all events.
  /*! All events are disabled and all event callback functions removed. */
  void 	disable_event_all();

  //! Redraw the connections connected to a specific node.
  /*! \param node	Node. */
  void	redraw_node_cons( void *node);

  //! Delete the connections connected to a specific node.
  /*! \param node	Node.*/
  void	delete_node_cons( void *node);

  //! Set defered redraw.
  /*! The redraw will be deferd until a call to redraw_defered() is made. */
  void	set_defered_redraw();

  //! Execute the defered redrawings.
  /*! Execute redrawing the defered redrawing area since the call to set_defered_redraw(). */
  void	redraw_defered();

  int		defered_redraw_active;	//!< Defered redraw is active.
  int		defered_x_low;		//!< Left border of defered redrawing area.
  int		defered_x_high;		//!< Right border of defered redrawing area.
  int		defered_y_low;		//!< Low border of defered redrawing area.
  int		defered_y_high;		//!< High border of defered redrawing area.
  int		defered_x_low_nav;	//!< Left border of deferd redraing area in navigation window.
  int		defered_x_high_nav;	//!< Right border of deferd redraing area in navigation window.
  int		defered_y_low_nav;	//!< Low border of deferd redraing area in navigation window.
  int		defered_y_high_nav;	//!< High border of deferd redraing area in navigation window.
  GlowArray 	a_nc;			//!< Array of nodeclasses.
  GlowArray 	a_cc;			//!< Array of connection classes.
  GlowArray 	a;			//!< Object array.
  GlowArray 	a_sel;			//!< List of selected objects.
  GlowArray 	a_paste;		//!< List of objects in paste buffer.
  GlowArray 	a_move;			//!< List of currently moved objects.
  char		name[32];		//!< Context name.
  void		*default_conclass;	//!< Default connection class.
  int (*event_callback[glow_eEvent__])(GlowCtx *ctx, glow_tEvent); //!< Array with registred event callback functions.
  glow_eEvent	event_region_select;	//!< Event registred as region select event.
  glow_eEvent	event_region_add_select; //!< Event registred as region add select event.
  glow_eEvent	event_create_con;	//!< Event registred as create connection event.
  glow_eEvent	event_create_node;	//!< Event registred as create node event.
  glow_eEvent	event_move_node;	//!< Event registred as move node event.
  GlowArrayElem *callback_object;	//!< Current callback object. Object that is hit by the current event.
  glow_eObjectType callback_object_type; //!< Type of current callback object.

  //! Register the current callback object.
  /*!
    \param type		Object type.
    \param object	Current callback object.

    An object that is hit the current event will register itself, and will be inserted in 
    the callback of the event
  */
  void	register_callback_object( glow_eObjectType type, GlowArrayElem *object)
	{ callback_object = object; callback_object_type = type; };

  //! Insert objects in selection list that for the specified selection region.
  /*!
    \param ll_x		x coordinate of lower left corner of selection region.
    \param ll_y		y coordinate of lower left corner of selection region.
    \param ur_x		x coordinate of upper right corner of selection region.
    \param ur_y		y coordinate of upper right corner of selection region.
    \param policy	Selection policy.
  */
  void	select_region_insert( double ll_x, double ll_y, double ur_x,
                                      double ur_y, glow_eSelectPolicy policy)
        {a.select_region_insert( ll_x, ll_y, ur_x, ur_y, policy);}

  int		cursor_present;		//!< Cursor in present in window.
  int		cursor_x;		//!< x coordinate of cursor in pixel.
  int		cursor_y;		//!< y coordinate of cursor in pixel.
  int		user_highlight;		//!< Highlight is set by the application. Otherwise selected objects are highlighted.
  int		application_paste;	//!< The application handles the paste.

  //! Set user highlight.
  /*! \param mode 	1 the application controls the highlight, 0 selected objects are highlighted. */
  void	set_user_highlight( int mode) { user_highlight = mode;};

  double	grid_size_x;		//!< Grid size in x direction.
  double	grid_size_y;		//!< Grid size in y direction.
  int		grid_on;		//!< Snap to grid is active.
  int		show_grid;		//!< Gridpoints are dislayed in the window.
  GlowPscript 	*print_ps;		//!< Print postscript object.

  //! Print a region to postscript. Not implemented.
  void	print_region( double ll_x, double ll_y, double ur_x, 
		      double ur_y, char *filename);

  double	draw_delta;		//!< A quantity used when routing connections.
  double	grafcet_con_delta;	//!< A quantity used when drawing grafcet connections.
  int		refcon_cnt;		//!< Counter for reference connections numbers.
  double	refcon_width;		//!< Width of a reference connection.
  double	refcon_height;		//!< Height of a reference connection.
  int		refcon_textsize;	//!< Text size for a reference connection.
  int		refcon_linewidth;	//!< Line width for a reference connection.

  //! Redraw the reference connections for a specific node and connection point.
  /*!
    \param node		Node.
    \param conpoint	Connection point number.
  */
  void conpoint_refcon_redraw( void *node, int conpoint)
	{ a.conpoint_refcon_redraw( node, conpoint);};

  //! Erase the reference connections for a specific node and connection point.
  /*!
    \param node		Node.
    \param conpoint	Connection point number.
  */
  void conpoint_refcon_erase( void *node, int conpoint)
	{ a.conpoint_refcon_erase( node, conpoint);};

  //! Get object from name.
  /*!
    \param name		Object name.
    \return		Pointer to the object.
  */
  GlowArrayElem *get_node_from_name( char *name);

  //! Get nodeclass from name.
  /*!
    \param name		Nodeclass name.
    \return		Pointer to the nodeclass.
  */
  GlowArrayElem *get_nodeclass_from_name( char *name);

  //! Get connection class from name.
  /*!
    \param name		Connection class name.
    \return		Pointer to the connection class.
  */
  GlowArrayElem *get_conclass_from_name( char *name);

  int (*trace_connect_func)( void *, GlowTraceData *); //!< Backcall function for trace connect of an object.
  int (*trace_disconnect_func)( void *);	//!< Backcall function for trace disconnect of an object.
  int (*trace_scan_func)( void *, void *);	//!< Backcall function for trace scan of an object.
  int trace_started;				//!< Trace is started.

  void remove_trace_objects();

  //! Initialize trace.
  /*!
    \param connect_func		Backcall function for trace connect of an object.
    \param disconnect_func     	Backcall function for trace disconnect of an object.
    \param scan_func		Backcall function for trace scan of an object.
  */
  int trace_init( int (*connect_func)( void *, GlowTraceData *), 
	int (*disconnect_func)( void *),
	int (*scan_func)( void *, void *));

  //! Trace close.
  /*! Calls the disconnect backcall function for all connected objects. */
  void trace_close();

  //! Trace scan.
  /*! Calls the scan backcall function for all connected objects. */
  void trace_scan();

  void *user_data;		//!< User data.

  //! Set user data.
  /*! \param data	User data. */
  void set_user_data( void *data) { user_data = data;};

  //! Get user data.
  /*! \param data	User data. */
  void get_user_data( void **data) { *data = user_data;};

  //! Get list of selected nodes, i.e objects of type GlowNode.
  /*!
    \param nodes	List of selected nodes.
    \param num		Number of selected nodes.

    The list should be freed by the caller.
  */
  void get_selected_nodes( GlowArrayElem ***nodes, int *num);

  //! Get list of selected connections.
  /*!
    \param cons		List of selected connections.
    \param num		Number of selected connections.

    The list should be freed by the caller.
  */
  void get_selected_cons( GlowArrayElem ***cons, int *num);

  //! Convert coordinates for a point to pixel.
  /*!
    \param x		x coordinate.
    \param y		y coordinate.
    \param pix_x	x coordinate in pixel.
    \param pix_y	y coordinate in pixel.
  */
  void position_to_pixel( double x, double y, int *pix_x, int *pix_y)
	{ *pix_x = int( x * zoom_factor_x - offset_x);
	  *pix_y = int( y * zoom_factor_y - offset_y);};

  //! Unzoom.
  /*! Return to base zoom factor. */
  void unzoom() { zoom( base_zoom_factor / zoom_factor_x);};

  //! Position the view so that the specified object is in the center of the window.
  /*! \param object	Object to center. */
  void center_object( GlowArrayElem *object);

  GlowArrayElem *get_document( double x, double y);

  int unobscured;		//! Window visibility is unobscured.
  int nodraw;			//! No drawings are performed.
  int no_nav;			//! No navigation window is present.
  int widget_cnt;
  glow_eSelectPolicy select_policy; //! Select policy.

  //! Set nodraw.
  /*! No drawing is performed. */
  void set_nodraw() { nodraw++; };

  //! Reset nodraw.
  /*! Drawing is resumed when the nodraw counter is zero. */
  void reset_nodraw() { if ( nodraw) nodraw--; };

  void reconfigure();

  //! Redraw the window.
  /*! The window is cleard and all objects are redrawn. */
  void redraw();

  //! Send an object deleted event callback if this event is registred.
  /*! \param object	Object that is to be deleted. */
  void object_deleted( GlowArrayElem *object);

  //! Send a tooltip event callback if this event is registred.
  /*!
    \param object	Current tooltip object.
    \param x		x coordinate for event.
    \param y		y coordinate for event.
  */
  void tiptext_event( GlowArrayElem *object, int x, int y);

  void annotation_input_cb( GlowArrayElem *object, int number, 
	char *text);
  void radiobutton_cb( GlowArrayElem *object, int number, 
	int value);

  //! Register change in input focus for an object.
  /*!
    \param object	Object that has gained or lost input focus.
    \param focus	1 if focus is gained, 0 if focus is lost.
  */
  void register_inputfocus( GlowArrayElem *object, int focus);

  //! Get the type of the context.
  /*! \return 	Context type. */
  int type() { return ctx_type;};

  // Move input widgets.
  void move_widgets( int x, int y) { if ( widget_cnt) a.move_widgets( x, y);};

  int display_level;		//!< Current display level.

  //! Scroll the window the specified distance.
  /*!
    \param delta_x	Scroll distance in x direction in pixel.
    \param delta_y	Scroll distance in y direction in pixel.
  */
  void scroll( int delta_x, int delta_y);

  double scroll_size;		//!< Quantity to calculate size of scrollbars.
  void (*scroll_callback)( glow_sScroll *); //!< Backcall function to update the scrollbars.
  void *scroll_data;		//!< Data for scrollbar backcall.
  glow_eHotMode hot_mode;	//!< Hot mode.
  glow_eHotMode default_hot_mode; //!< Default hot mode.
  int hot_found;		//!< A hot object is found.
  int double_buffered;		//!< Double buffering is configured.
  int double_buffer_on;		//!< Double buffering is on.
  int draw_buffer_only;		//!< Draw in double buffering buffer only.
  glow_tUserDataSaveCb userdata_save_callback; //!< Callback function called when userdata is saved.
  glow_tUserDataOpenCb userdata_open_callback; //!< Callback function called when userdata is opened.
  glow_tUserDataCopyCb userdata_copy_callback; //!< Callback function called when userdata is copied.
  int version;			//!< Current glow version.
  GlowTipText *tiptext;		//!< Tip text object.
  GlowArrayElem *inputfocus_object; //!< Object that has input focus.
  int is_component;		//!< Ctx is a window component.

  //! Register scrollbar callback function
  /*!
    \param data		Data for the callback.
    \param callback	Callback function called when the scrollbars needs to update.
  */
  void register_scroll_callback( void *data, 
		void (*callback)( glow_sScroll *))
		{ scroll_data = data; scroll_callback = callback;};

  //! Update the scroobars.
  /*! The scrollbar callback function is called to update the scrollbars. */
  void change_scrollbar();

  //! Find an object by name.
  /*!
    \param name		Object name.
    \param element	Pointer to the object.
    \return		Returns 1 if the object is found, else 0.
  */
  int find_by_name( char *name, GlowArrayElem **element)
		{ return a.find_by_name( name, element);};

  //! Set hot mode.
  /*! \param mode	Hot mode. */
  void set_hot_mode( glow_eHotMode mode) { hot_mode = mode;};

  //! Set show grid.
  /*! \param show	1 display the grid points, 0 hide. */
  void set_show_grid( int show);

  //! Get show grid.
  /*! \return		1 grid points are displayed, 0 hidden. */
  int get_show_grid() { return show_grid;}

  //! Draw gridpoints in the specified area.
  /*!
    \param ll_x		x coordinate for lower left corner of area in pixel.
    \param ll_y		y coordinate for lower left corner of area in pixel.
    \param ur_x		x coordinate for upper right corner of area in pixel.
    \param ur_y		y coordinate for upper right corner of area in pixel.
  */
  void draw_grid( int ll_x, int ll_y, int ur_x, int ur_y);

  //! Draw in the double buffering buffer only.
  /*! Increase the draw_buffer_only count. As long as this is > 0, no drawing is made to the screen, only in the buffer. */
  void set_draw_buffer_only() { if (double_buffer_on) draw_buffer_only++;};

  //! Reset draw in the double buffering buffer only.
  /*! Decrease the draw_buffer_only count. As long as this is > 0, no drawing is made to the screen, only in the buffer. */
  void reset_draw_buffer_only() { if (double_buffer_on) draw_buffer_only--;};

  //! Register callback functions for userdata handling.
  /*!
    \param save_callback	Callback function that will be called when userdata is saved to file.
    \param open_callback	Callback function that will be called when userdata is opened from file.
    \param copy_callback	Callback function that will be called when userdata is copied.
  */
  void register_userdata_callbacks(
		glow_tUserDataSaveCb save_callback, 
		glow_tUserDataOpenCb open_callback,
		glow_tUserDataCopyCb copy_callback)
		{ userdata_save_callback = save_callback;
		  userdata_open_callback = open_callback;
		  userdata_copy_callback = copy_callback;}

  //! Destructor
  /*! Delete all objects in the context. */
  ~GlowCtx();
};

//! Start autoscrolling.
void	auto_scrolling( GlowCtx *ctx);

//! Scroll window horizontal.
/*! Called when the horizontal scrollbar is moved. */
void glow_scroll_horizontal( GlowCtx *ctx, int value, int bottom);

//! Scroll window vertical.
/*! Called when the vertical scrollbar is moved. */
void glow_scroll_vertical( GlowCtx *ctx, int value, int bottom);

//! Check if a file exist.
/*! The only justification of this function is that checking status of filestreams doesn't 
  work on some VMS compilers...
 */
int check_file( char *filename);


/*@}*/
#endif

