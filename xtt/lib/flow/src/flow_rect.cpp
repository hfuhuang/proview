#include "flow_std.h"


#include <iostream.h>
#include "flow_rect.h"
#include "flow_draw.h"

void FlowRect::zoom()
{
  ll.zoom();
  ur.zoom();
}

void FlowRect::nav_zoom()
{
  ll.nav_zoom();
  ur.nav_zoom();
}

void FlowRect::print_zoom()
{
  ll.print_zoom();
  ur.print_zoom();
}

void FlowRect::traverse( int x, int y)
{
  ll.traverse( x, y);
  ur.traverse( x, y);
}

void FlowRect::print( void *pos, void *node)
{
  if ( !(display_level & ctx->display_level))
    return;
  if ( fix_line_width && line_width < 0)
    return;
  if ( draw_type == flow_eDrawType_LineErase)
    return;
  int idx = int( ctx->print_zoom_factor / ctx->base_zoom_factor * line_width - 
		1);
  idx = MAX( 0, idx);
  idx = MIN( idx, DRAW_TYPE_SIZE-1);
  if ( !fill)
    ctx->print_ps->rect( ll.print_z_x + ((FlowPoint *)pos)->print_z_x, 
	ll.print_z_y + ((FlowPoint *)pos)->print_z_y,
	ur.print_z_x - ll.print_z_x, ur.print_z_y - ll.print_z_y, 
	draw_type, idx);
  else
    ctx->print_ps->filled_rect( ll.print_z_x + ((FlowPoint *)pos)->print_z_x, 
	ll.print_z_y + ((FlowPoint *)pos)->print_z_y,
	ur.print_z_x - ll.print_z_x, ur.print_z_y - ll.print_z_y, 
	draw_type, idx);
}

void FlowRect::save( ofstream& fp, flow_eSaveMode mode)
{
  fp << int(flow_eSave_Rect) << endl;
  fp << int(flow_eSave_Rect_draw_type) << FSPACE << int(draw_type) << endl;
  fp << int(flow_eSave_Rect_line_width) << FSPACE << line_width << endl;
  fp << int(flow_eSave_Rect_display_level) << FSPACE << int(display_level) << endl;
  fp << int(flow_eSave_Rect_ll) << endl;
  ll.save( fp, mode);
  fp << int(flow_eSave_Rect_ur) << endl;
  ur.save( fp, mode);
  fp << int(flow_eSave_End) << endl;
}

void FlowRect::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case flow_eSave_Rect: break;
      case flow_eSave_Rect_draw_type: fp >> tmp; draw_type = (flow_eDrawType)tmp; break;
      case flow_eSave_Rect_line_width: fp >> line_width; break;
      case flow_eSave_Rect_display_level: fp >> tmp; display_level = (flow_mDisplayLevel)tmp; break;
      case flow_eSave_Rect_ll: ll.open( fp); break;
      case flow_eSave_Rect_ur: ur.open( fp); break;
      case flow_eSave_End: end_found = 1; break;
      default:
        cout << "FlowRect:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

void FlowRect::draw( void *pos, int highlight, int hot, void *node)
{
  if ( !(display_level & ctx->display_level))
    return;
  int idx;
  if ( fix_line_width)
  {
    idx = line_width;
    idx += hot;
    if ( idx < 0)
    {
      erase( pos, hot, node);
      return;
    }
  }
  else
  {
    idx = int( ctx->zoom_factor / ctx->base_zoom_factor * line_width - 1);
    idx += hot;
  }
  idx = MAX( 0, idx);
  idx = MIN( idx, DRAW_TYPE_SIZE-1);
  if ( !fill)
    flow_draw_rect( ctx, ll.z_x + ((FlowPoint *)pos)->z_x - ctx->offset_x, ll.z_y + 
	((FlowPoint *)pos)->z_y - ctx->offset_y, 
	ur.z_x - ll.z_x, ur.z_y - ll.z_y, draw_type, idx, highlight);
  else
    flow_draw_fill_rect( ctx, ll.z_x + ((FlowPoint *)pos)->z_x - ctx->offset_x, ll.z_y + 
	((FlowPoint *)pos)->z_y - ctx->offset_y, 
	ur.z_x - ll.z_x, ur.z_y - ll.z_y, draw_type);
}

void FlowRect::erase( void *pos, int hot, void *node)
{
  if ( !(display_level & ctx->display_level))
    return;
  int idx;
  if ( fix_line_width)
  {
    idx = line_width;
    idx += hot;
    if ( idx < 0) return;
  }
  else
  {
    idx = int( ctx->zoom_factor / ctx->base_zoom_factor * line_width - 1);
    idx += hot;
  }
  idx = MAX( 0, idx);
  idx = MIN( idx, DRAW_TYPE_SIZE-1);
  if ( !fill)
    flow_draw_rect_erase( ctx, ll.z_x + ((FlowPoint *)pos)->z_x - ctx->offset_x, ll.z_y + 
	((FlowPoint *)pos)->z_y - ctx->offset_y, 
	ur.z_x - ll.z_x, ur.z_y - ll.z_y, idx);
  else
    flow_draw_fill_rect( ctx, ll.z_x + ((FlowPoint *)pos)->z_x - ctx->offset_x, ll.z_y + 
	((FlowPoint *)pos)->z_y - ctx->offset_y, 
	ur.z_x - ll.z_x, ur.z_y - ll.z_y, flow_eDrawType_LineErase);
}

void FlowRect::nav_draw( void *pos, int highlight, void *node)
{
  if ( !(display_level & ctx->display_level))
    return;
  int idx;
  if ( fix_line_width)
  {
    idx = line_width;
    if ( idx < 0) return;
  }
  else
  {
    idx = int( ctx->nav_zoom_factor / ctx->base_zoom_factor * line_width - 1);
  }
  idx = MAX( 0, idx);
  idx = MIN( idx, DRAW_TYPE_SIZE-1);
  if ( !fill)
    flow_draw_nav_rect( ctx, ll.nav_z_x + ((FlowPoint *)pos)->nav_z_x - 
	ctx->nav_offset_x, ll.nav_z_y + 
	((FlowPoint *)pos)->nav_z_y - ctx->nav_offset_y, 
	ur.nav_z_x - ll.nav_z_x, ur.nav_z_y - ll.nav_z_y,
	draw_type, idx, highlight);
}

void FlowRect::nav_erase( void *pos, void *node)
{
  if ( !(display_level & ctx->display_level))
    return;
  int idx;
  if ( fix_line_width)
  {
    idx = line_width;
    if ( idx < 0) return;
  }
  else
  {
    idx = int( ctx->nav_zoom_factor / ctx->base_zoom_factor * line_width - 1);
  }
  idx = MAX( 0, idx);
  idx = MIN( idx, DRAW_TYPE_SIZE-1);
  if ( !fill)
    flow_draw_nav_rect_erase( ctx, ll.nav_z_x + ((FlowPoint *)pos)->nav_z_x - 
	ctx->nav_offset_x, ll.nav_z_y + 
	((FlowPoint *)pos)->nav_z_y - ctx->nav_offset_y, 
	ur.nav_z_x - ll.nav_z_x, ur.nav_z_y - ll.nav_z_y,
	idx);
}

int FlowRect::event_handler( void *pos, flow_eEvent event, int x, int y,
	void *node)
{
  FlowPoint *p;

  p = (FlowPoint *) pos;
  if ( ll.z_x + ((FlowPoint *)pos)->z_x - ctx->offset_x <= x && 
       x <= ur.z_x  + ((FlowPoint *)pos)->z_x - ctx->offset_x &&
       ll.z_y  + ((FlowPoint *)pos)->z_y - ctx->offset_y <= y && 
       y <= ur.z_y + ((FlowPoint *)pos)->z_y - ctx->offset_y)
  {
//    cout << "Event handler: Hit in rect" << endl;
    return 1;
  }  
  else
    return 0;
}

void FlowRect::get_borders( double pos_x, double pos_y, double *x_right, 
	double *x_left, double *y_high, double *y_low, void *node)
{
  if ( display_level != flow_mDisplayLevel_1)
    return;
  if ( pos_x + ll.x < *x_left)
    *x_left = pos_x + ll.x;
  if ( pos_x + ur.x > *x_right)
    *x_right = pos_x + ur.x;
  if ( pos_y + ll.y < *y_low)
    *y_low = pos_y + ll.y;
  if ( pos_y + ur.y > *y_high)
    *y_high = pos_y + ur.y;
}

void FlowRect::move( void *pos, double x, double y, int highlight, int hot)
{
  double width, height;

  width = ur.x - ll.x;
  height = ur.y - ll.y;
  erase( pos, hot, NULL);
  nav_erase( pos, NULL);
  ll.x = x;
  ll.y = y;
  ur.x = x + width;
  ur.y = y + height;
  zoom();
  nav_zoom();
  draw( pos, highlight, hot, NULL);
  nav_draw( pos, highlight, NULL);
}

void FlowRect::shift( void *pos, double delta_x, double delta_y,
	int highlight, int hot)
{
  erase( pos, hot, NULL);
  nav_erase( pos, NULL);
  ll.x += delta_x;
  ll.y += delta_y;
  ur.x += delta_x;
  ur.y += delta_y;
  zoom();
  nav_zoom();

  draw( pos, highlight, hot, NULL);
  nav_draw( pos, highlight, NULL);
}

ostream& operator<< ( ostream& o, const FlowRect r)
{
  o << 
  '(' << r.ll.x << ',' << r.ll.y << ')' << 
  '(' << r.ur.x << ',' << r.ur.y << ')' << 
  '[' << r.ll.z_x << ',' << r.ll.z_y << ']' <<
  '[' << r.ur.z_x << ',' << r.ur.z_y << ']' ;
  return o;
}

