#include "glow_std.h"


#include <iostream.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include "glow_growannot.h"
#include "glow_draw.h"
#include "glow_growctx.h"
#include "glow_node.h"
#include "glow_grownode.h"


void GrowAnnot::save( ofstream& fp, glow_eSaveMode mode) 
{ 

  if ( mode == glow_eSaveMode_SubGraph)
    GlowAnnot::save( fp, mode);
  else
  {
    fp << int(glow_eSave_GrowAnnot) << endl;
    fp << int(glow_eSave_GrowAnnot_annot_part) << endl;
    GlowAnnot::save( fp, mode);
    fp << int(glow_eSave_GrowAnnot_trf) << endl;
    trf.save( fp, mode);
    fp << int(glow_eSave_End) << endl;
  }
}

void GrowAnnot::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case glow_eSave_GrowAnnot: break;
      case glow_eSave_GrowAnnot_annot_part: 
        GlowAnnot::open( fp);
        break;
      case glow_eSave_GrowAnnot_trf: trf.open( fp); break;
      case glow_eSave_End: end_found = 1; break;
      default:
        cout << "GrowAnnot:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

void GrowAnnot::draw( GlowTransform *t, int highlight, int hot, void *node, 
		      void *colornode)
{
  int x1, y1;

  if ( !(display_level & ctx->display_level))
    return;

  if ( !((GlowNode *) node)->annotv[number])
    return;

  glow_eDrawType color;
  int rot;
  double trf_scale = trf.vertical_scale( t);
  int idx = int( trf_scale * ctx->zoom_factor_y / ctx->base_zoom_factor * (text_size +4) - 4);
  if ( idx < 0)
    return;
  idx = min( idx, DRAW_TYPE_SIZE-1);

  if (!t)
  {
    x1 = int( trf.x( p.x, p.y) * ctx->zoom_factor_x) - ctx->offset_x;
    y1 = int( trf.y( p.x, p.y) * ctx->zoom_factor_y) - ctx->offset_y;
    rot = (int) trf.rot();
  }
  else
  {
    x1 = int( trf.x( t, p.x, p.y) * ctx->zoom_factor_x) - ctx->offset_x;
    y1 = int( trf.y( t, p.x, p.y) * ctx->zoom_factor_y) - ctx->offset_y;
    rot = (int) trf.rot( t);
  }
  rot = rot < 0 ? rot % 360 + 360 : rot % 360;

  switch ( annot_type) {
  case glow_eAnnotType_OneLine: {
    int width, height, descent;

    color = ((GrowCtx *)ctx)->get_drawtype( color_drawtype, glow_eDrawType_LineHighlight,
					    highlight, (GrowNode *)colornode, 2);


    if ( rot < 45 || rot >= 315) {
      if ( ((GlowNode *) node)->annotv_inputmode[number] &&
	   ((GrowNode *) node)->input_selected) {
	draw_get_text_extent( ctx, ((GlowNode *) node)->annotv[number],
			      strlen(((GlowNode *) node)->annotv[number]),
			      draw_type, idx,
			      &width, &height, &descent);
	glow_draw_fill_rect( ctx, x1, y1 - height + descent, width, height, 
			     glow_eDrawType_MediumGray);
      }
    }
    else {
      // Text is rotated, adjust the coordinates
      draw_get_text_extent( ctx, ((GlowNode *) node)->annotv[number],
			  strlen(((GlowNode *) node)->annotv[number]),
			  draw_type, idx,
			  &width, &height, &descent);

      if ( 45 <= rot && rot < 135) {
	y1 += height - descent;
      }
      else if ( 135 <= rot && rot < 225) {
	x1 -= width;
	y1 += height - descent;
      }
      else {
	x1 -= width;
      }
    } 
    glow_draw_text( ctx, x1, y1,
		    ((GlowNode *) node)->annotv[number], 
		    strlen(((GlowNode *) node)->annotv[number]), draw_type, color, idx, 
		    highlight, 0);
    if ( ((GlowNode *) node)->annotv_inputmode[number])
      glow_draw_text_cursor( ctx, x1, y1,
			     ((GlowNode *) node)->annotv[number],
			     strlen(((GlowNode *) node)->annotv[number]),
			     draw_type, color, idx, highlight, 
			     ((GrowNode *)node)->input_position);
#if 0
    glow_draw_move_input( ctx, 
			  ((GlowNode *) node)->annotv_input[number],
			  x1, y1,
			  glow_ePosition_Absolute);
#endif
    break;
  }
  case glow_eAnnotType_MultiLine: {
    int z_width, z_height, z_descent;
    int len = 0;
    int line_cnt = 0;
    char *line = ((GlowNode *) node)->annotv[number];
    char *s;
    color = ((GrowCtx *)ctx)->get_drawtype( color_drawtype, glow_eDrawType_LineHighlight,
					    highlight, (GrowNode *)colornode, 2);

    draw_get_text_extent( ctx, "", 0, draw_type, idx, &z_width, &z_height,
			  &z_descent);
    for ( s = ((GlowNode *) node)->annotv[number]; *s; s++) {
      if ( *s == 10) {
	if ( len)
	  glow_draw_text( ctx, x1, y1 + line_cnt * z_height, line, 
			  len, draw_type, color, idx, highlight, 0);
	len = 0;
	line = s+1;
	line_cnt++;
      }
      else
	len++;
    }
    if ( len)
      glow_draw_text( ctx, x1, y1 + line_cnt * z_height, line, 
		      len, draw_type, color, idx, highlight, 0);
    break;
  }
  }
}

void GrowAnnot::erase( GlowTransform *t, int hot, void *node)
{
  int x1, y1;

  if ( !(display_level & ctx->display_level))
    return;

  if ( !((GlowNode *) node)->annotv[number])
    return;
  double trf_scale = trf.vertical_scale( t);
  int idx = int( trf_scale * ctx->zoom_factor_y / ctx->base_zoom_factor * (text_size +4) - 4);
  if ( idx < 0)
    return;
  idx = min( idx, DRAW_TYPE_SIZE-1);

  if (!t)
  {
    x1 = int( trf.x( p.x, p.y) * ctx->zoom_factor_x) - ctx->offset_x;
    y1 = int( trf.y( p.x, p.y) * ctx->zoom_factor_y) - ctx->offset_y;
  }
  else
  {
    x1 = int( trf.x( t, p.x, p.y) * ctx->zoom_factor_x) - ctx->offset_x;
    y1 = int( trf.y( t, p.x, p.y) * ctx->zoom_factor_y) - ctx->offset_y;
  }
  switch ( annot_type) {
    case glow_eAnnotType_OneLine:
    {
      if ( ((GlowNode *) node)->annotv_inputmode[number] &&
	   ((GrowNode *) node)->input_selected) {
	int width, height, descent;
	draw_get_text_extent( ctx, ((GlowNode *) node)->annotv[number],
			      strlen(((GlowNode *) node)->annotv[number]),
			      draw_type, idx,
			      &width, &height, &descent);
	glow_draw_fill_rect( ctx, x1, y1 - height + descent, width, height, glow_eDrawType_LineErase);
      }
      glow_draw_text_erase( ctx, x1, y1,
		((GlowNode *) node)->annotv[number], 
		strlen(((GlowNode *) node)->annotv[number]), draw_type, idx, 0);
      break;
    }
    case glow_eAnnotType_MultiLine:
    {
      int z_width, z_height, z_descent;
      int len = 0;
      int line_cnt = 0;
      char *line = ((GlowNode *) node)->annotv[number];
      char *s;
      draw_get_text_extent( ctx, "", 0, draw_type, idx, &z_width, &z_height,
		&z_descent);
      for ( s = ((GlowNode *) node)->annotv[number]; *s; s++)
      {
        if ( *s == 10)
	{
	  if ( len)
            glow_draw_text_erase( ctx, x1, y1 + line_cnt * z_height, line, 
	  	len, draw_type, idx, 0);
	  len = 0;
	  line = s+1;
	  line_cnt++;
	}
	else
	  len++;
      }
      if ( len)
        glow_draw_text_erase( ctx, x1, y1 + line_cnt * z_height, line, 
	  	len, draw_type, idx, 0);
      break;
    }
  }
}

void GrowAnnot::erase_background( GlowTransform *t, int hot, void *node)
{
  int x1, y1;

  if ( !(display_level & ctx->display_level))
    return;

  if ( !((GlowNode *) node)->annotv[number])
    return;
  double trf_scale = trf.vertical_scale( t);
  int idx = int( trf_scale * ctx->zoom_factor_y / ctx->base_zoom_factor * (text_size +4) - 4);
  if ( idx < 0)
    return;
  idx = min( idx, DRAW_TYPE_SIZE-1);

  if (!t)
  {
    x1 = int( trf.x( p.x, p.y) * ctx->zoom_factor_x) - ctx->offset_x;
    y1 = int( trf.y( p.x, p.y) * ctx->zoom_factor_y) - ctx->offset_y;
  }
  else
  {
    x1 = int( trf.x( t, p.x, p.y) * ctx->zoom_factor_x) - ctx->offset_x;
    y1 = int( trf.y( t, p.x, p.y) * ctx->zoom_factor_y) - ctx->offset_y;
  }
  switch ( annot_type) {
    case glow_eAnnotType_OneLine:
    {
      int sts;

      sts = ((GrowNode *)node)->draw_annot_background( t, 0, 0);
      if ( ODD(sts))
      {
        // There is not any gc for color text...
//        glow_draw_text( ctx, x1, y1,
//		((GlowNode *) node)->annotv[number], 
//		strlen(((GlowNode *) node)->annotv[number]), background, idx, 
//		0, 0);
      }
      else
        glow_draw_text_erase( ctx, x1, y1,
		((GlowNode *) node)->annotv[number], 
		strlen(((GlowNode *) node)->annotv[number]), draw_type, idx, 0);
      break;
    }
    case glow_eAnnotType_MultiLine:
    {
      int z_width, z_height, z_descent;
      int len = 0;
      int line_cnt = 0;
      char *line = ((GlowNode *) node)->annotv[number];
      char *s;
      draw_get_text_extent( ctx, "", 0, draw_type, idx, &z_width, &z_height,
		&z_descent);
      for ( s = ((GlowNode *) node)->annotv[number]; *s; s++)
      {
        if ( *s == 10)
	{
	  if ( len)
            glow_draw_text_erase( ctx, x1, y1 + line_cnt * z_height, line, 
	  	len, draw_type, idx, 0);
	  len = 0;
	  line = s+1;
	  line_cnt++;
	}
	else
	  len++;
      }
      if ( len)
        glow_draw_text_erase( ctx, x1, y1 + line_cnt * z_height, line, 
	  	len, draw_type, idx, 0);
      break;
    }
  }
}

void GrowAnnot::nav_draw( GlowTransform *t, int highlight, void *node, void *colornode)
{
  int x1, y1;

  if ( !(display_level & ctx->display_level))
    return;
  if ( !((GlowNode *) node)->annotv[number])
    return;
  double trf_scale = trf.vertical_scale( t);
  int idx = int( trf_scale * ctx->nav_zoom_factor_y / ctx->base_zoom_factor * (text_size +4) - 4);
  if ( idx < 0)
    return;
  idx = min( idx, DRAW_TYPE_SIZE-1);

  if (!t)
  {
    x1 = int( trf.x( p.x, p.y) * ctx->nav_zoom_factor_x) - ctx->nav_offset_x;
    y1 = int( trf.y( p.x, p.y) * ctx->nav_zoom_factor_y) - ctx->nav_offset_y;
  }
  else
  {
    x1 = int( trf.x( t, p.x, p.y) * ctx->nav_zoom_factor_x) - ctx->nav_offset_x;
    y1 = int( trf.y( t, p.x, p.y) * ctx->nav_zoom_factor_y) - ctx->nav_offset_y;
  }
  switch ( annot_type) {
    case glow_eAnnotType_OneLine:
      glow_draw_nav_text( ctx, x1, y1,
	((GlowNode *) node)->annotv[number], 
	strlen(((GlowNode *) node)->annotv[number]), draw_type, idx, 
	highlight, 0);
      break;
    case glow_eAnnotType_MultiLine:
    {
      int z_width, z_height, z_descent;
      int len = 0;
      int line_cnt = 0;
      char *line = ((GlowNode *) node)->annotv[number];
      char *s;
      draw_get_text_extent( ctx, "", 0, draw_type, idx, &z_width, &z_height,
		&z_descent);
      for ( s = ((GlowNode *) node)->annotv[number]; *s; s++)
      {
        if ( *s == 10)
	{
	  if ( len)
            glow_draw_nav_text( ctx, x1, y1 + line_cnt * z_height, line, 
	  	len, draw_type, idx, highlight, 0);
	  len = 0;
	  line = s+1;
	  line_cnt++;
	}
	else
	  len++;
      }
      if ( len)
        glow_draw_nav_text( ctx, x1, y1 + line_cnt * z_height, line, 
	  	len, draw_type, idx, highlight, 0);
      break;
    }
  }
}

void GrowAnnot::nav_erase( GlowTransform *t, void *node)
{
  int x1, y1;

  if ( !(display_level & ctx->display_level))
    return;
  if ( !((GlowNode *) node)->annotv[number])
    return;
  double trf_scale = trf.vertical_scale( t);
  int idx = int( trf_scale * ctx->nav_zoom_factor_y / ctx->base_zoom_factor * (text_size +4) - 4);
  if ( idx < 0)
    return;
  idx = min( idx, DRAW_TYPE_SIZE-1);

  if (!t)
  {
    x1 = int( trf.x( p.x, p.y) * ctx->nav_zoom_factor_x) - ctx->nav_offset_x;
    y1 = int( trf.y( p.x, p.y) * ctx->nav_zoom_factor_y) - ctx->nav_offset_y;
  }
  else
  {
    x1 = int( trf.x( t, p.x, p.y) * ctx->nav_zoom_factor_x) - ctx->nav_offset_x;
    y1 = int( trf.y( t, p.x, p.y) * ctx->nav_zoom_factor_y) - ctx->nav_offset_y;
  }
  switch ( annot_type) {
    case glow_eAnnotType_OneLine:
      glow_draw_nav_text_erase( ctx, x1, y1, 
	((GlowNode *) node)->annotv[number], 
	strlen(((GlowNode *) node)->annotv[number]), draw_type, idx, 0);
      break;
    case glow_eAnnotType_MultiLine:
    {
      int z_width, z_height, z_descent;
      int len = 0;
      int line_cnt = 0;
      char *line = ((GlowNode *) node)->annotv[number];
      char *s;
      draw_get_text_extent( ctx, "", 0, draw_type, idx, &z_width, &z_height,
		&z_descent);
      for ( s = ((GlowNode *) node)->annotv[number]; *s; s++)
      {
        if ( *s == 10)
	{
	  if ( len)
            glow_draw_nav_text_erase( ctx, x1, y1 + line_cnt * z_height, line, 
	  	len, draw_type, idx, 0);
	  len = 0;
	  line = s+1;
	  line_cnt++;
	}
	else
	  len++;
      }
      if ( len)
        glow_draw_nav_text_erase( ctx, x1, y1 + line_cnt * z_height, line, 
	  	len, draw_type, idx, 0);
      break;
    }
  }
}


void GrowAnnot::get_borders( GlowTransform *t, double *x_right, double *x_left, 
	double *y_high, double *y_low)
{
  double x1, y1;

  if ( !t)
  {
    x1 = trf.x( p.x, p.y);
    y1 = trf.y( p.x, p.y);
  }
  else
  {
    x1 = trf.x( t, p.x, p.y);
    y1 = trf.y( t, p.x, p.y);
  }

  if ( x1 < *x_left)
    *x_left = x1;
  if ( x1 > *x_right)
    *x_right = x1;
  if ( y1 < *y_low)
    *y_low = y1;
  if ( y1 > *y_high)
    *y_high = y1;
}

void GrowAnnot::export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp)
{
  int x1, y1;
  int bold;

  double trf_scale = trf.vertical_scale( t);
  int idx = int( trf_scale * ctx->zoom_factor_y / ctx->base_zoom_factor * (text_size +4) - 4);
  if ( idx < 0)
    return;
  idx = min( idx, DRAW_TYPE_SIZE-1);

  if (!t)
  {
    x1 = int( trf.x( p.x, p.y) * ctx->zoom_factor_x) - ctx->offset_x;
    y1 = int( trf.y( p.x, p.y) * ctx->zoom_factor_y) - ctx->offset_y;
  }
  else
  {
    x1 = int( trf.x( t, p.x, p.y) * ctx->zoom_factor_x) - ctx->offset_x;
    y1 = int( trf.y( t, p.x, p.y) * ctx->zoom_factor_y) - ctx->offset_y;
  }

  bold = (draw_type == glow_eDrawType_TextHelveticaBold);

  ((GrowCtx *)ctx)->export_jbean->annot( x1, y1, number,
	draw_type, color_drawtype, bold, idx, pass, shape_cnt, node_cnt, fp);
//  (*shape_cnt)++;
}

void GrowAnnot::export_javabean_font( GlowTransform *t, void *node,
	glow_eExportPass pass, ofstream &fp)
{
  int bold;
  glow_eDrawType background;

  double trf_scale = trf.vertical_scale( t);
  int idx = int( trf_scale * ctx->zoom_factor_y / ctx->base_zoom_factor * (text_size +4) - 4);
  if ( idx < 0)
    return;
  idx = min( idx, DRAW_TYPE_SIZE-1);

  bold = (draw_type == glow_eDrawType_TextHelveticaBold);

  ((GrowCtx *)ctx)->export_jbean->nc->get_annot_background( NULL, NULL,
	&background);

  ((GrowCtx *)ctx)->export_jbean->annot_font( number,
	draw_type, background, bold, idx, pass, fp);
}

void GrowAnnot::convert( glow_eConvert version) 
{
  switch ( version) {
  case glow_eConvert_V34: {
    // Conversion of colors
    draw_type = GlowColor::convert( version, draw_type);

    break;
  }
  }  
}

void GrowAnnot::get_annotation_info( void *node, int *t_size, glow_eDrawType *t_drawtype,
				    glow_eDrawType *t_color)
{
#if 0
  double trf_scale = trf.vertical_scale( &((GrowNode *)node)->trf);
  int idx = int( trf_scale * ctx->zoom_factor_y / ctx->base_zoom_factor * (text_size +4) - 4);
  if ( idx < 0)
    idx = 0;
  idx = min( idx, DRAW_TYPE_SIZE-1);
#endif
  *t_color = ((GrowCtx *)ctx)->get_drawtype( color_drawtype, glow_eDrawType_LineHighlight,
		 0, (GrowNode *)node, 2);
  *t_size = text_size;
  *t_drawtype = draw_type;
}
