/* 
 * Proview   $Id: flow_array.h,v 1.5 2008-10-31 12:51:32 claes Exp $
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

#ifndef flow_array_h
#define flow_array_h

#include <iostream>
#include <fstream>

#include "flow_array_elem.h"

class FlowArray {
  public:
    FlowArray( int allocate, int incr);
    FlowArray() {};
    void new_array( const FlowArray& array);
    FlowArrayElem *operator[] ( int idx);
    void copy_from( const FlowArray& array);
    void copy_from_common_objects( FlowArray& array);
    void move_from( FlowArray& array);
    int size() { return a_size;};
    int insert( FlowArrayElem *element);
    void remove( FlowArrayElem *element);
    int find( FlowArrayElem *element);
    int find_by_name( char *name, FlowArrayElem **element);
    int find_by_name_no_case( char *name, FlowArrayElem **element);
    void clear() { a_size = 0;}
    void zoom();
    void nav_zoom();
    void print_zoom();
    void print( void *pos, void *node, int highlight);
    void save( ofstream& fp, flow_eSaveMode mode);
    void open( void *ctx, ifstream& fp);
    void draw( void *pos, int highlight, int hot, void *node);
    void erase( void *pos, int hot, void *node); 
    void draw_inverse( void *pos, int hot, void *node);
    void nav_draw( void *pos, int highlight, void *node);
    void nav_erase( void *pos, void *node);
    void traverse( int x, int y);
    void get_borders(
	double *x_right, double *x_left, double *y_high, double *y_low);
    void get_borders();
    void get_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node);
    int	event_handler( flow_eEvent event, int x, int y);
    int	event_handler( void *pos, flow_eEvent event, int x, int y, void *node);
    int event_handler( void *pos, flow_eEvent event, int x, int y, int num);
    void conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp);
    int get_conpoint( int num, double *x, double *y, flow_eDirection *dir);
    void set_highlight( int on);
    void set_hot( int on);
    void select_region_insert( double ll_x, double ll_y, double ur_x, 
		double ur_y);
    void shift( void *pos, double delta_x, double delta_y, int highlight, 
		int hot);
    void move( int delta_x, int delta_y, int grid);
    void move_noerase( int delta_x, int delta_y, int grid);
    void conpoint_refcon_redraw( void *node, int conpoint);
    void conpoint_refcon_erase( void *node, int conpoint);
    void set_inverse( int on);
    void configure();
    int brow_insert( FlowArrayElem *element, FlowArrayElem *destination, 
	flow_eDest code);
    void brow_remove( void *ctx, FlowArrayElem *element);
    void brow_close( void *ctx, FlowArrayElem *element);
    int brow_get_parent( FlowArrayElem *element, FlowArrayElem **parent);
    int brow_get_child( FlowArrayElem *element, FlowArrayElem **child);
    int brow_get_next_sibling( FlowArrayElem *element, 
		FlowArrayElem **sibling);
    int brow_get_previous_sibling( FlowArrayElem *element, 
		FlowArrayElem **sibling);
    void move_widgets( int x, int y);
    int get_first( FlowArrayElem **first);
    int get_last( FlowArrayElem **last);
    int get_previous( FlowArrayElem *element, FlowArrayElem **prev);
    int get_next( FlowArrayElem *element, FlowArrayElem **next);
    ~FlowArray();
    friend class FlowNodeClass;
    friend class FlowCtx;
  private:
    int	allocated;
    int alloc_incr;
    int a_size;
    FlowArrayElem **a;
};

#endif
