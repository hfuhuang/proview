/* 
 * Proview   $Id: glow_array.h,v 1.7 2008-10-31 12:51:35 claes Exp $
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

#ifndef glow_array_h
#define glow_array_h

#include <iostream>
#include <fstream>

#include "glow_array_elem.h"
#include "glow_transform.h"

/*! \file glow_array.h
    \brief Contains the GlowArray class. */
/*! \addtogroup Glow */
/*@{*/


//! A container for Glow objects and component.
/*! A container for objects of type GlowArrayElem, which is the baseclass for all objects, nodes, groups
  and connections. The class has functions to insert and remove objects, but also corresponding functions
  to all the functions of GlowArrayElem.
*/

class GrowCtx;
class GlowExportFlow;

class GlowArray {
 public:
  //! Constructor.
  /*!
    \param allocate	Initial allocation.
    \param incr		Additional allocation when the array needs to be extended.
  */
  GlowArray( int allocate, int incr);

  //! Noargs constructor.
  GlowArray() {};

  //! Initiates an array to be a copy of another array.
  /*!
    \param array	Array to make a copy of.
  */
  void new_array( const GlowArray& array);

  //! Index operator. Returns the object at the specified index.
  /*!
    \param idx	Array index.
  */
  GlowArrayElem *operator[] ( int idx);

  //! Makes this array a copy of another array with new copies of all objects.
  /*!
    \param array	Array to make a copy of.
  */
  void copy_from( const GlowArray& array);

  //! Makes this array a copy of another array, and share the objects.
  /*!
    \param array	Array to make a copy of.
  */
  void copy_from_common_objects( GlowArray& array);

  //! Move all object from an array to this array.
  /*!
    \param array	Array to move object from.
  */
  void move_from( GlowArray& array);

  //! Get size of array.
  /*!
    \return 	The number of objects in the array.
  */
  int size() { return a_size;};

  //! Insert an element last.
  /*!
    \param element	The element to insert.
    \return		Returns 1 if success, 0 if element already is inserted.
  */
  int insert( GlowArrayElem *element);

  //! Remove a specific element.
  /*!
    \param element	The element to remove.
  */
  void remove( GlowArrayElem *element);

  //! Check if an object is in the array.
  /*!
    \param element	Element to search for.
    \return		Returns 1 if the object is found, else 0.
  */
  int find( GlowArrayElem *element);

  //! Find an object by name.
  /*!
    \param name		The name of the object.
    \param element	Returned pointer to the object.
    \return		Returns 1 if the object is found, else 0.
  */
  int find_by_name( const char *name, GlowArrayElem **element);

  //! Clear the array.
  void clear() { a_size = 0;}
  void delete_all();

  void zoom();
  void nav_zoom();
  void print_zoom();
  void print( void *pos, void *node);
  void save( ofstream& fp, glow_eSaveMode mode);
  void open( GrowCtx *ctx, ifstream& fp);
  void draw( GlowWind *w, void *pos, int highlight, int hot, void *node);
  void erase( GlowWind *w, void *pos, int hot, void *node); 
  void draw( GlowWind *w, GlowTransform *t, int highlight, int hot, void *node, void *colornode);
  void erase( GlowWind *w, GlowTransform *t, int hot, void *node); 
  void draw_inverse( void *pos, int hot, void *node);
  void nav_draw( void *pos, int highlight, void *node);
  void nav_draw( GlowTransform *t, int highlight, void *node, void *colornode);
  void nav_erase( void *pos, void *node);
  void nav_erase( GlowTransform *t, void *node);
  void traverse( int x, int y);
  void get_borders(
	double *x_right, double *x_left, double *y_high, double *y_low);
  void get_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node);
  void get_borders( GlowTransform *t, double *x_right, 
		double *x_left, double *y_high, double *y_low);
  int event_handler( GlowWind *w, glow_eEvent event, int x, int y);
  int event_handler( GlowWind *w, glow_eEvent event, int x, int y, double fx, double fy);
  int event_handler( GlowWind *w, glow_eEvent event, double fx, double fy);
  int event_handler( GlowWind *w, void *pos, glow_eEvent event, int x, int y, void *node);
  int event_handler( GlowWind *w, void *pos, glow_eEvent event, int x, int y, int num);
  void conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp);
  void conpoint_select( GlowTransform *t, int x, int y, double *distance, 
		void **cp, int *pix_x, int *pix_y);
  int get_conpoint( int num, double *x, double *y, glow_eDirection *dir);
  void set_highlight( int on);
  void set_hot( int on);
  void select_region_insert( double ll_x, double ll_y, double ur_x, 
		double ur_y, glow_eSelectPolicy select_policy);
  void shift( void *pos, double delta_x, double delta_y, int highlight, 
		int hot);
  void move( double delta_x, double delta_y, int grid);
  void move_noerase( int delta_x, int delta_y, int grid);
  void conpoint_refcon_redraw( void *node, int conpoint);
  void conpoint_refcon_erase( void *node, int conpoint);
  void set_inverse( int on);
  void configure();
  int brow_insert( GlowArrayElem *element, GlowArrayElem *destination, 
		   glow_eDest code);
  int move( GlowArrayElem *element, GlowArrayElem *destination, 
	    glow_eDest code);
  void brow_remove( void *ctx, GlowArrayElem *element);
  void brow_close( void *ctx, GlowArrayElem *element);
  int brow_get_parent( GlowArrayElem *element, GlowArrayElem **parent);
  void move_widgets( int x, int y);
  int get_first( GlowArrayElem **first);
  int get_last( GlowArrayElem **last);
  int get_previous( GlowArrayElem *element, GlowArrayElem **prev);
  int get_next( GlowArrayElem *element, GlowArrayElem **next);
  void exec_dynamic();
  void set_fill_color( glow_eDrawType drawtype);
  void reset_fill_color();
  void set_border_color( glow_eDrawType drawtype);
  void reset_border_color();
  void set_original_fill_color( glow_eDrawType drawtype);
  void set_original_border_color( glow_eDrawType drawtype);
  void set_original_text_color( glow_eDrawType drawtype);
  void set_color_tone( glow_eDrawTone tone);
  void set_original_color_tone( glow_eDrawTone tone);
  void reset_color_tone();
  void set_color_lightness( int lightness);
  void incr_original_color_lightness( int lightness);
  void set_original_color_lightness( int lightness);
  void reset_color_lightness();
  void set_color_intensity( int intensity);
  void incr_original_color_intensity( int intensity);
  void set_original_color_intensity( int intensity);
  void reset_color_intensity();
  void set_color_shift( int shift);
  void incr_original_color_shift( int shift);
  void incr_color_shift( int shift);
  void set_original_color_shift( int shift);
  void reset_color_shift();
  void set_linewidth( int linewidth);
  void set_fill( int fill);
  void set_border( int border);
  void set_shadow( int shadow);
  void set_drawtype( glow_eDrawType drawtype);
  void set_transform( GlowTransform *t);
  void set_transform_from_stored(  GlowTransform *t);
  void store_transform();
  void push( GlowArrayElem *element);
  void pop( GlowArrayElem *element);
  void get_node_borders();
  void align( double x, double y, glow_eAlignDirection direction);
  int find_nc( GlowArrayElem *nc);
  int find_cc( GlowArrayElem *cc);
  void trace_scan();
  int trace_init();
  void trace_close();
  void get_nodegroups( void *a);
  void set_last_group( char *name);
  char *get_last_group();
  int get_background_object_limits( GlowTransform *t, glow_eTraceType type,
		    double x, double y, GlowArrayElem **background,
		    double *min, double *max, glow_eDirection *direction);
  void flip( double x0, double y0, glow_eFlipDirection dir);
  void convert( glow_eConvert version);
  void set_rootnode( void *node);
  void set_linetype( glow_eLineType type);
  void export_flow( GlowExportFlow *ef);

  friend class GlowNodeClass;
  friend class GlowCtx;

  int	allocated;
  int alloc_incr;
  int a_size;
  GlowArrayElem **a;
  ~GlowArray();
};

/*@}*/
#endif




