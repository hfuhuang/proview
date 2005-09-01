/* 
 * Proview   $Id: glow_pscript.h,v 1.2 2005-09-01 14:57:54 claes Exp $
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

#ifndef glow_pscript_h
#define glow_pscript_h

#include <stdio.h>
#include "glow.h"

class GlowPscript {
  public:
    GlowPscript( char *filename, void *glow_ctx, int page_border);
    ~GlowPscript();
     int print_page( double ll_x, double ll_y, double ur_x, double ur_y);
     int rect( double x, double y, double width, double height, glow_eDrawType type, int idx);
     int filled_rect( double x, double y, double width, double height, glow_eDrawType type, int idx);
     int arc( double x, double y, double width, double height, int angel1, int angel2,
		glow_eDrawType type, int idx);
     int line( double x1, double y1, double x2, double y2, glow_eDrawType type, int idx);
     int text( double x, double y, char *text, int len,  glow_eDrawType type, int size);
     int pixmap( double x, double y, glow_sPixmapDataElem *data, glow_eDrawType type);
     int arrow( double x1, double y1, double x2, double y2, 
	double x3, double y3, glow_eDrawType type, int idx);
     void move( double x, double y);
     void setlinewidth( int idx);
     void rowbreak();
     double offset_x;
     double offset_y;
     FILE *file;
     char fname[80];
     int border;
     void *ctx;
};

#endif
