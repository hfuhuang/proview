/* 
 * Proview   $Id: flow_rect.h,v 1.5 2008-10-31 12:51:33 claes Exp $
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

#ifndef flow_rect_h
#define flow_rect_h

#include <iostream>
#include <fstream>
#include "flow.h"
#include "flow_ctx.h"
#include "flow_point.h"
#include "flow_array_elem.h"


class FlowRect : public FlowArrayElem {
  public:
    FlowRect( FlowCtx *flow_ctx, double x = 0, double y = 0, double w = 0, 
		double h = 0, flow_eDrawType d_type = flow_eDrawType_Line, 
		int line_w = 1, int fix_line_w = 0, 
		flow_mDisplayLevel display_lev = flow_mDisplayLevel_1,
		int fill_rect = 0) : 
	ctx(flow_ctx), ll(flow_ctx,x,y), ur(flow_ctx,x+w,y+h), 
	draw_type(d_type), line_width(line_w), fix_line_width(fix_line_w),
	display_level(display_lev), fill(fill_rect) {};
    friend ostream& operator<< ( ostream& o, const FlowRect r);
    void zoom();
    void nav_zoom();
    void print_zoom();
    void traverse( int x, int y);
    int	event_handler( void *pos, flow_eEvent event, int x, int y, void *node);
    void conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp) {};
    void print( void *pos, void *node, int highlight);
    void save( ofstream& fp, flow_eSaveMode mode);
    void open( ifstream& fp);
    void draw( void *pos, int hightlight, int hot, void *node);
    void nav_draw( void *pos, int highlight, void *node);
    void draw_inverse( void *pos, int hot, void *node);
    void erase( void *pos, int hot, void *node);
    void nav_erase( void *pos, void *node);
    void get_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node);
    void move( void *pos, double x, double y, int highlight, int hot);
    void shift( void *pos, double delta_x, double delta_y,
	int highlight, int hot);
    int get_conpoint( int num, double *x, double *y, flow_eDirection *dir) 
		{ return 0;};
    flow_eObjectType type() { return flow_eObjectType_Rect;};
    FlowCtx *ctx;    
    double width() { return ur.x - ll.x;};
    double height() { return ur.y - ll.y;};
    FlowPoint ll;
    FlowPoint ur;
    flow_eDrawType draw_type;
    int	line_width;
    int  fix_line_width;
    flow_mDisplayLevel display_level;
    int fill;
};

#endif
