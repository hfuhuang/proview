/* 
 * Proview   $Id: glow_node.h,v 1.6 2008-10-31 12:51:36 claes Exp $
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

#ifndef glow_node_h
#define glow_node_h

#include <iostream>
#include <fstream>
#include "glow_growctx.h"
#include "glow_point.h"
#include "glow_array_elem.h"
#include "glow_nodeclass.h"
#include "glow_tracedata.h"

#define MAX_CONPOINTS 32

/*! \file glow_node.h
    \brief Contains the GlowNode class. */
/*! \addtogroup Glow */
/*@{*/


//! Base class for a node.
/*! The full implementation of a node is in the GrowNode class. Many functions are superseeded by this class
  and left undocumented.
*/

class GlowNode : public GlowArrayElem {
 public:

  //! Constuctor
  /*!
    \param glow_ctx 	The glow context.
    \param name		Name (max 31 char).
    \param node_class	Pointer to NodeClass object.
    \param x1		x coordinate for position.
    \param y1		y coordinate for position.
    \param nodraw	Don't draw the object now.
    \param rel_annot_pos Not used.
  */
  GlowNode( GrowCtx *glow_ctx, const char *name, GlowNodeClass *node_class,
	double x1, double y1, int nodraw = 0, int rel_annot_pos = 0);

  // Noargs constructor.
  GlowNode() {};

  // Destructor.
  ~GlowNode();

  void zoom() { nc->zoom(); pos.zoom();};
  void nav_zoom() { nc->nav_zoom(); pos.nav_zoom();};
  void get_borders(
	double *x1_right, double *x1_left, double *y1_high, double *y1_low)
	{ if ( x_left < *x1_left) *x1_left = x_left;
	  if ( x_right > *x1_right) *x1_right = x_right;
	  if ( y_high > *y1_high) *y1_high = y_high;
	  if ( y_low < *y1_low) *y1_low = y_low;};
  void get_node_borders( )
	{ nc->get_borders(pos.x, pos.y, &x_right, &x_left, &y_high, &y_low,
	(void *)this);};
  void get_node_obstacle_borders( )
	{ nc->get_obstacle_borders(pos.x, pos.y, &obst_x_right, &obst_x_left, 
		&obst_y_high, &obst_y_low, (void *)this);};

  //! Save the content of the object to file.
  /*!
    \param fp	Ouput file.
    \param mode	Save as graph or subgraph.
  */
  void save( ofstream& fp, glow_eSaveMode mode);

  //! Read the content of the object from file.
  /*!
    \param fp	Input file.
  */
  void open( ifstream& fp);

  void store_position() { stored_pos = pos;};
  void restore_position() { pos = stored_pos;};
  int get_conpoint( int num, double *x, double *y, glow_eDirection *dir);
  void	redraw_node_cons( void *node) {};
  int		delete_node_cons( void *node) {return 0;};
  void select_region_insert( double ll_x, double ll_y, double ur_x, 
		double ur_y, glow_eSelectPolicy select_policy);
  glow_eObjectType type() { return glow_eObjectType_Node;};
  void	set_annotation( int num, char *text, int size, int nodraw, int brief = 0);
  void	get_annotation( int num, char *text, int size);

  //! Get width and height for the text of an annotation.
  /*!
    \param num		Annotation number.
    \param text		Text of the annotation.
    \param width	Width of the text.
    \param height	Hight of the text.
  */
  void 	measure_annotation( int num, char *text, double *width, 
			double *height);

  void	measure( double *ll_x, double *ll_y, double *ur_x, double *ur_y)
	{ *ll_x = x_left; *ll_y = y_low; *ur_x = x_right; *ur_y = y_high;};
  
  double		x_right;	//!< Right border of object.
  double		x_left;		//!< Left border of object.
  double		y_high;		//!< High border of object.
  double		y_low;		//!< Low border of object.
  double		obst_x_right;	//!< Right border of object used for routing of connections.
  double		obst_x_left;	//!< Left border of object used for routing of connections.
  double		obst_y_high;	//!< High border of object used for routing of connections.
  double		obst_y_low;	//!< Low border of object used for routing of connections.
  int			hot;		//!< Object is hot.
  GrowCtx 		*ctx;		//!< Glow context.
  GlowNodeClass 	*nc;		//!< Pointer to nodeclass.
  GlowNodeClass 	*nc_root;	//!< Root nodeclass, i.e. the nodeclass of the first page.
  GlowPoint		pos;
  GlowPoint		stored_pos;
  char			n_name[80];	//!< Name of the object.
  int			highlight;	//!< The object is drawn with highlight color.
  int			inverse;
  char			*annotv[10];	//!< Array with pointers to annotation texts.
  int			annotsize[10];	//!< The size of the annotation text.
  int			refcon_cnt[MAX_CONPOINTS]; //!< Number of reference connections for each connection point.
  GlowTraceData		trace;
  GlowNode		*link;		//!< Link in list used for routing of connections.

  //! Insert in list used for routing of connections.
  /*!
    \param start	Start of list.
  */
  virtual void	link_insert( void **start)
	{link = *(GlowNode **)start; *start = (void *)this;};

  //! Check if object is inside an area with some margin (draw_delta).
  /*!
    \param ll_x		x coordinate for lower left corner of area.
    \param ll_y		y coordinate for lower left corner of area.
    \param ur_x		x coordinate for upper right corner of area.
    \param ur_y		y coordinate for upper right corner of area.
    \return 		Returns 1 if object is inside the area, else 0.
  */
  int			in_area( double ll_x, double ll_y, double ur_x, double ur_y)
      	{return ((obst_x_left - ctx->draw_delta) < ur_x && 
	         (obst_x_right + ctx->draw_delta) > ll_x &&
	         (obst_y_low - ctx->draw_delta) < ur_y && 
	         (obst_y_high + ctx->draw_delta) > ll_y);};

  //! Check if object is inside an area.
  /*!
    \param ll_x		x coordinate for lower left corner of area.
    \param ll_y		y coordinate for lower left corner of area.
    \param ur_x		x coordinate for upper right corner of area.
    \param ur_y		y coordinate for upper right corner of area.
    \return 		Returns 1 if object is inside the area, else 0.
  */
  int			in_area_exact( double ll_x, double ll_y, double ur_x, double ur_y)
      	{return (obst_x_left < ur_x && obst_x_right > ll_x && 
		 obst_y_low < ur_y && obst_y_high > ll_y);};

  //! Check if object crosses a vertical line.
  /*!
    \param x		x coordinate for the line.
    \param l_y		y coordinate for lower endpoint of line.
    \param u_y		y coordinate for upper endpoint of line.
    \return 		Returns 1 if object crosses the line, else 0.
  */
  int			in_vert_line( double x, double l_y, double u_y)
      	{return ((obst_x_left - ctx->draw_delta) < x && 
	         (obst_x_right + ctx->draw_delta) > x && 
                 (obst_y_low - ctx->draw_delta) < u_y && 
	         (obst_y_high + ctx->draw_delta) > l_y);};

  //! Check if object crosses a horizontal line.
  /*!
    \param y		y coordinate for the line.
    \param l_x		x coordinate for lower endpoint of line.
    \param u_x		x coordinate for upper endpoint of line.
    \return 		Returns 1 if object crosses the line, else 0.
  */
  int			in_horiz_line( double y, double l_x, double u_x)
      	{return ((obst_x_left - ctx->draw_delta) < u_x && 
	         (obst_x_right + ctx->draw_delta) > l_x && 
	         (obst_y_low - ctx->draw_delta) < y && 
	         (obst_y_high + ctx->draw_delta) > y);};

  //! Reconfigure reference connections for a connections point.
  /*!
    \param conpoint	Number of conpoint.
  */
  void conpoint_refcon_reconfig( int conpoint);

  void conpoint_refcon_redraw( void *node, int conpoint) {};
  void conpoint_refcon_erase( void *node, int conpoint) {};
  void remove_notify();

  void *user_data;	//!< User data.

  //! Set user data.
  /*!
    \param data User data.
  */
  void set_user_data( void *data) { user_data = data;};

  //! Get user data.
  /*!
    \param data User data.
  */
  void get_user_data( void **data) { *data = user_data;};

  void set_trace_attr( GlowTraceData *attr);
  void get_trace_attr( GlowTraceData **attr);
  void set_trace_data( void *data) { trace.p = data;};

  //! Scan trace
  /*! Calls the trace scan callback.
   */
  void trace_scan();

  //! Scan trace
  /*! Calls the trace connect callback.
   */
  int trace_init();

  void trace_close();

  //! Get grow context.
  /*!
    \return The context.
  */
  void *get_ctx() { return this->ctx;};

  void get_node_position( double *x, double *y) {*x = pos.x; *y = pos.y;};
  glow_eNodeGroup get_group() {return nc->group;};

  //! Get object name
  /*!
    \param name		Object name.
  */
  void get_object_name( char *name) { strcpy( name, n_name);};

  //! Set object name
  /*!
    \param name		Object name.
  */
  void set_object_name( char *name) { strcpy( n_name, name);};

//  brow stuff
    void set_level( int lev) { level = lev;};
    int get_level() { return level;};
    int is_open() { return node_open;};
    void set_open( int mask) { node_open |= mask;};
    void reset_open( int mask) { node_open &= ~mask;};
    void open_annotation_input( int num);
    int annotation_input_is_open( int num) { return annotv_inputmode[num];};
    void close_annotation_input( int num);
    int	level;
    int node_open;
    int relative_annot_pos;
    double relative_annot_x;
    double rel_annot_x[10];
    int annotv_inputmode[10];
    void *annotv_input[10];
    int rbuttonv[10];
    int input_active;
    int input_focus;
};

/*@}*/
#endif








