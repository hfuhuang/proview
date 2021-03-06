/* 
 * Proview   $Id: flow_browctx.h,v 1.7 2008-10-31 12:51:33 claes Exp $
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

#ifndef flow_browctx_h
#define flow_browctx_h

//#include <iostream>
//#include <fstream>

#ifndef flow_h
#include "flow.h"
#endif

#ifndef flow_ctx_h
#include "flow_ctx.h"
#endif

class BrowCtx : public FlowCtx {
  public:
    BrowCtx( const char *ctx_name, double zoom_fact = 100) :
    	FlowCtx( ctx_name, zoom_fact), 
	indentation(0.8), frame_x_right(0), annotation_space(0.3)
	{ ctx_type = flow_eCtxType_Brow; };
    int insert( FlowArrayElem *element, FlowArrayElem *destination, 
		flow_eDest destination_code);
    void close( FlowArrayElem *element);
    void remove( FlowArrayElem *element);
    void configure( double y_redraw);
    void change_scrollbar();
    void redraw( double y_redraw);
    void zoom( double factor);
    void unzoom() { zoom( base_zoom_factor / zoom_factor);};
    int print( char *filename);
    int get_first( FlowArrayElem **first) { return a.get_first( first);};
    int get_last( FlowArrayElem **last) { return a.get_last( last);};
    int get_previous( FlowArrayElem *element, FlowArrayElem **prev)
    		{ return a.get_previous( element, prev);};
    int get_next( FlowArrayElem *element, FlowArrayElem **next)
    		{ return a.get_next( element, next);};
    int get_parent( FlowArrayElem *element, FlowArrayElem **parent)
    		{ return a.brow_get_parent( element, parent);};
    int get_child( FlowArrayElem *element, FlowArrayElem **child)
    		{ return a.brow_get_child( element, child);};
    int get_next_sibling( FlowArrayElem *element, FlowArrayElem **sibling)
    		{ return a.brow_get_next_sibling( element, sibling);};
    int get_previous_sibling( FlowArrayElem *element, FlowArrayElem **sibling)
    		{ return a.brow_get_previous_sibling( element, sibling);};
    int is_visible( FlowArrayElem *element, flow_eVisible type);
    int get_first_visible( FlowArrayElem **element);
    int get_last_visible( FlowArrayElem **element);
    void center_object( FlowArrayElem *object, double factor);
    int page( double factor);

    ~BrowCtx() {};
    double indentation;
    double frame_x_right;
    double annotation_space;
};

void brow_scroll_horizontal( BrowCtx *ctx, int value, int bottom);
void brow_scroll_vertical( BrowCtx *ctx, int value, int bottom);

#endif
