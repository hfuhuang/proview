/* 
 * Proview   $Id: glow_exportjbean.h,v 1.13 2008-10-31 12:51:35 claes Exp $
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

#ifndef glow_exportjbean_h
#define glow_exportjbean_h

#include <iostream>
#include <fstream>

#include <X11/Xlib.h>
#include "glow.h"
#include "glow_ctx.h"
#include "glow_nodeclass.h"

class GlowNodeClass;

class GlowExportJBean {
  public:
    GlowExportJBean( GlowCtx *glow_ctx, GlowNodeClass *nodeclass = 0) : 
      ctx(glow_ctx), nc(nodeclass), page(1), func_cnt(0), frc_created(0)
    {
      is_nodeclass = (nc != NULL);
    };
    void growctx( glow_eExportPass pass, ofstream& fp);
    void nodeclass( GlowNodeClass *nc, glow_eExportPass pass, 
	ofstream& fp, int page, int pages);
    void polyline( glow_sPoint *points, int point_cnt, int fill, int border, 
	glow_eDrawType fill_drawtype, glow_eDrawType border_drawtype,
	int fill_eq_border, int fill_eq_light, int fill_eq_shadow, int line_width, 
        int print_shadow, int shadow, int drawtype_incr, glow_sShadowInfo *sp, int sp_num,
		   glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void line( double x1, double y1, double x2, double y2,
	glow_eDrawType border_drawtype,
	int line_width, glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void rect( double x0, double y0, double width, double height,
	int fill, int border,
	glow_eDrawType fill_drawtype, glow_eDrawType border_drawtype,
	int line_width, double shadow_width, int shadow, 
	int drawtype_incr, glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void rectrounded( double x0, double y0, double width, double height,
	int fill, int border,
	glow_eDrawType fill_drawtype, glow_eDrawType border_drawtype,
	int line_width, double roundamount, double shadow_width, int shadow, 
	int drawtype_incr, glow_eExportPass pass, int *shape_cnt, 
	int node_cnt, ofstream& fp);
    void arc( double x0, double y0, double width, double height,
	double angle1, double angle2, int fill, int border,
	glow_eDrawType fill_drawtype, glow_eDrawType border_drawtype,
	int line_width, double shadow_width, int shadow, 
	int drawtype_incr, glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void text( int x0, int y0, char *text,
	glow_eDrawType drawtype, glow_eDrawType color_drawtype, int bold,
	int idx, glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void annot( int x0, int y0, int number,
		glow_eDrawType drawtype, glow_eDrawType text_drawtype, int bold,
		glow_eAdjustment adjustment, int idx, glow_eExportPass pass, 
		int *shape_cnt, int node_cnt, ofstream& fp);
    void annot_font( int number, glow_eDrawType drawtype, 
	glow_eDrawType background, int bold,
	int idx, glow_eExportPass pass, ofstream& fp);
    void node( double x1, double y1, double x2, double y2,
	char *class_name,
    	glow_eDrawType border_drawtype,  
    	glow_eDrawType	fill_drawtype,
    	glow_eDrawType	text_drawtype,
  	glow_eDrawTone	color_tone,
    	int		color_lightness,
    	int		color_intensity,
    	int		color_shift,
    	int		line_width,
	double		rotate,
	int 		shadow,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream& fp);
    void image( double x1, double y1, double x2, double y2,
	char *filename, int transparent,
    	glow_eDrawTone	color_tone,
    	int		color_lightness,
    	int		color_intensity,
    	int		color_shift,
	double		rotate,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream& fp);
    void bar( double x1, double y1, double x2, double y2,
    	glow_eDrawType border_drawtype,
    	glow_eDrawType	fill_drawtype,
    	glow_eDrawType	bar_drawtype,
    	glow_eDrawType	bar_bordercolor,
    	int		fill,
	int		border,
	double		min_value,
	double		max_value,
	int		bar_border_width,
    	int		line_width,
	double		rotate,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void trend( double x1, double y1, double x2, double y2,
    	glow_eDrawType border_drawtype,
    	glow_eDrawType	fill_drawtype,
    	glow_eDrawType	curve_drawtype1,
    	glow_eDrawType	curve_drawtype2,
    	glow_eDrawType	curve_fill_drawtype1,
    	glow_eDrawType	curve_fill_drawtype2,
    	int		fill,
	int		border,
	double		min_value1,
	double		max_value1,
	double		min_value2,
	double		max_value2,
	int		curve_width,
	int		no_of_points,
	double		scan_time,
	int		horizontal_lines,
	int		vertical_lines,
    	int		line_width,
	double		rotate,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void xycurve( double x1, double y1, double x2, double y2,
    	glow_eDrawType border_drawtype,
    	glow_eDrawType	fill_drawtype,
    	int		fill,
	int		border,
	int		curve_width,
	int		no_of_points,
	int		horizontal_lines,
	int		vertical_lines,
    	int		line_width,
	double		rotate,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void axis( double x1, double y1, double x2, double y2,
    	glow_eDrawType border_drawtype,
    	glow_eDrawType text_drawtype,
	double		min_value,
	double		max_value,
	int	        lines,
	int		longquotient,
	int		valuequotient,
	int		line_length,
    	int		line_width,
	double		rotate,
	int             bold,
	int             text_idx,
	char            *format,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void window( double x1, double y1, double x2, double y2,
	char *filename,
        int vertical_scrollbar, 
	int horizontal_scrollbar,
	char *owner,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void folder( double x1, double y1, double x2, double y2,
	int folders,
	char *folder_file_names, char *folder_text,
	int *folder_v_scrollbar, int *folder_h_scrollbar, char *folder,
        glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void table( double x1, double y1, double x2, double y2,
    	glow_eDrawType	fill_drawtype, int fill,
	int rows, int columns, int header_row, int header_column,
	int text_idx, glow_eDrawType text_drawtype, double header_row_height,
	double row_height, double *column_width, char *header_text,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, ofstream& fp);
    void slider( double x1, double y1, double x2, double y2,
	char *class_name,
    	glow_eDrawType border_drawtype,
    	glow_eDrawType	fill_drawtype,
    	glow_eDrawType	text_drawtype,
    	glow_eDrawTone	color_tone,
    	int		color_lightness,
    	int		color_intensity,
    	int		color_shift,
    	int		line_width,
	double		rotate,
        int		shadow,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream& fp);
    GlowCtx *ctx;
    GlowNodeClass *nc;
    int is_nodeclass;
    int page;
    int func_cnt;
    int frc_created;
};

#endif
