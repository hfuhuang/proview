#include "flow_std.h"


#include <iostream.h>
#include <fstream.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include "flow_ctx.h"
#include "flow_node.h"
#include "flow_point.h"
#include "flow_rect.h"
#include "flow_line.h"
#include "flow_arc.h"
#include "flow_text.h"
#include "flow_conpoint.h"
#include "flow_node.h"
#include "flow_nodeclass.h"
#include "flow_con.h"
#include "flow_conclass.h"
#include "flow_pushbutton.h"
#include "flow_draw.h"
#include "flow_browctx.h"
#include "flow_msg.h"

FlowCtx::~FlowCtx()
{
  set_nodraw();
  a_sel.clear();
  move_clear();
  paste_clear();
}

void FlowCtx::delete_all()
{
  int		i;
  FlowArrayElem	*element;

  set_nodraw();
  for ( i = 0; i < a.a_size; i++)
  {
    element = a.a[i];
    remove( element); 
    select_remove( element); 
    move_remove( element);
    delete element;
    i--;
  }
  reset_nodraw();
  clear();
}

int FlowCtx::save( char *filename, flow_eSaveMode mode)
{
  ofstream	fp;

  fp.open( filename);
#ifndef OS_VMS
  if ( !fp)
    return FLOW__FILEOPEN;
#endif

  fp << int(flow_eSave_Ctx) << endl;

  fp <<	int(flow_eSave_Ctx_zoom_factor) << FSPACE << zoom_factor << endl;
  fp <<	int(flow_eSave_Ctx_base_zoom_factor) << FSPACE << base_zoom_factor << endl;
  fp <<	int(flow_eSave_Ctx_offset_x) << FSPACE << offset_x << endl;
  fp <<	int(flow_eSave_Ctx_offset_y) << FSPACE << offset_y << endl;
  fp <<	int(flow_eSave_Ctx_nav_zoom_factor) << FSPACE << nav_zoom_factor << endl;
  fp <<	int(flow_eSave_Ctx_print_zoom_factor) << FSPACE << print_zoom_factor << endl;
  fp <<	int(flow_eSave_Ctx_nav_offset_x) << FSPACE << nav_offset_x << endl;
  fp <<	int(flow_eSave_Ctx_nav_offset_y) << FSPACE << nav_offset_y << endl;
  fp <<	int(flow_eSave_Ctx_x_right) << FSPACE << x_right << endl;
  fp <<	int(flow_eSave_Ctx_x_left) << FSPACE << x_left << endl;
  fp <<	int(flow_eSave_Ctx_y_high) << FSPACE << y_high << endl;
  fp <<	int(flow_eSave_Ctx_y_low) << FSPACE << y_low << endl;
//  fp << int(flow_eSave_Ctx_window_width) << FSPACE << window_width << endl;
//  fp << int(flow_eSave_Ctx_window_height) << FSPACE << window_height << endl;
//  fp << int(flow_eSave_Ctx_nav_window_width) << FSPACE << nav_window_width << endl;
//  fp << int(flow_eSave_Ctx_nav_window_height) << FSPACE << nav_window_height << endl;
  fp <<	int(flow_eSave_Ctx_nav_rect_ll_x) << FSPACE << nav_rect_ll_x << endl;
  fp <<	int(flow_eSave_Ctx_nav_rect_ll_y) << FSPACE << nav_rect_ll_y << endl;
  fp <<	int(flow_eSave_Ctx_nav_rect_ur_x) << FSPACE << nav_rect_ur_x << endl;
  fp <<	int(flow_eSave_Ctx_nav_rect_ur_y) << FSPACE << nav_rect_ur_y << endl;
  fp <<	int(flow_eSave_Ctx_nav_rect_hot) << FSPACE << nav_rect_hot << endl;
  fp <<	int(flow_eSave_Ctx_name) << FSPACE << name << endl;
  fp <<	int(flow_eSave_Ctx_user_highlight) << FSPACE << user_highlight << endl;
  fp <<	int(flow_eSave_Ctx_grid_size_x) << FSPACE << grid_size_x << endl;
  fp <<	int(flow_eSave_Ctx_grid_size_y) << FSPACE << grid_size_y << endl;
  fp <<	int(flow_eSave_Ctx_grid_on) << FSPACE << grid_on << endl;
  fp <<	int(flow_eSave_Ctx_draw_delta) << FSPACE << draw_delta << endl;
  fp <<	int(flow_eSave_Ctx_refcon_width) << FSPACE << refcon_width << endl;
  fp <<	int(flow_eSave_Ctx_refcon_height) << FSPACE << refcon_height << endl;
  fp <<	int(flow_eSave_Ctx_refcon_textsize) << FSPACE << refcon_textsize << endl;
  fp <<	int(flow_eSave_Ctx_refcon_linewidth) << FSPACE << refcon_linewidth << endl;
  fp <<	int(flow_eSave_Ctx_a_nc) << endl;
  a_nc.save( fp, mode);
  fp <<	int(flow_eSave_Ctx_a_cc) << endl;
  a_cc.save( fp, mode);
  fp <<	int(flow_eSave_Ctx_a) << endl;
  a.save( fp, mode);

  fp << int(flow_eSave_End) << endl;
  fp.close();
  return 1;
}

int FlowCtx::open( char *filename, flow_eSaveMode mode)
{
  ifstream	fp;
  int		type;
  int		end_found = 0;
  char		dummy[40];

  fp.open( filename);
#ifndef OS_VMS
  if ( !fp)
    return FLOW__FILEOPEN;
#endif

  for (;;)
  {
    fp >> type;
    switch( type) {
      case flow_eSave_Ctx: break;
      case flow_eSave_Ctx_zoom_factor: fp >> zoom_factor; break;
      case flow_eSave_Ctx_base_zoom_factor: fp >> base_zoom_factor; break;
      case flow_eSave_Ctx_offset_x: fp >> offset_x; break;
      case flow_eSave_Ctx_offset_y: fp >> offset_y; break;
      case flow_eSave_Ctx_nav_zoom_factor: fp >> nav_zoom_factor; break;
      case flow_eSave_Ctx_print_zoom_factor: fp >> print_zoom_factor; break;
      case flow_eSave_Ctx_nav_offset_x: fp >> nav_offset_x; break;
      case flow_eSave_Ctx_nav_offset_y: fp >> nav_offset_y; break;
      case flow_eSave_Ctx_x_right: fp >> x_right; break;
      case flow_eSave_Ctx_x_left: fp >> x_left; break;
      case flow_eSave_Ctx_y_high: fp >> y_high; break;
      case flow_eSave_Ctx_y_low: fp >> y_low; break;
//      case flow_eSave_Ctx_window_width: fp >> window_width; break;
//      case flow_eSave_Ctx_window_height: fp >> window_height; break;
//      case flow_eSave_Ctx_nav_window_width: fp >> nav_window_width; break;
//      case flow_eSave_Ctx_nav_window_height: fp >> nav_window_height; break;
      case flow_eSave_Ctx_nav_rect_ll_x: fp >> nav_rect_ll_x; break;
      case flow_eSave_Ctx_nav_rect_ll_y: fp >> nav_rect_ll_y; break;
      case flow_eSave_Ctx_nav_rect_ur_x: fp >> nav_rect_ur_x; break;
      case flow_eSave_Ctx_nav_rect_ur_y: fp >> nav_rect_ur_y; break;
      case flow_eSave_Ctx_nav_rect_hot: fp >> nav_rect_hot; break;
      case flow_eSave_Ctx_name: 
        fp.get();
        fp.getline( name, sizeof(name));
        break;
      case flow_eSave_Ctx_user_highlight: fp >> user_highlight; break;
      case flow_eSave_Ctx_grid_size_x: fp >> grid_size_x; break;
      case flow_eSave_Ctx_grid_size_y: fp >> grid_size_y; break;
      case flow_eSave_Ctx_grid_on: fp >> grid_on; break;
      case flow_eSave_Ctx_draw_delta: fp >> draw_delta; break;
      case flow_eSave_Ctx_refcon_width: fp >> refcon_width; break;
      case flow_eSave_Ctx_refcon_height: fp >> refcon_height; break;
      case flow_eSave_Ctx_refcon_textsize: fp >> refcon_textsize; break;
      case flow_eSave_Ctx_refcon_linewidth: fp >> refcon_linewidth; break;
      case flow_eSave_Ctx_a_nc: a_nc.open( this, fp); break;
      case flow_eSave_Ctx_a_cc: a_cc.open( this, fp); break;
      case flow_eSave_Ctx_a: a.open( this, fp); break;
      case flow_eSave_End: end_found = 1; break;
      default:
        cout << "Ctx:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
   }
   if ( end_found)
     break;
  }
  fp.close();
  get_borders();
  a.zoom();
  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();

  return 1;
}

void FlowCtx::node_movement( FlowArrayElem *node, int x, int y) 
{
  node_moved = node;
  node_movement_active = 1;
  node_move_last_x = x;
  node_move_last_y = y;
}

void FlowCtx::con_create_source( FlowArrayElem *node, int cp_num, int cp_x, int cp_y)
{
  con_create_node = node;
  con_create_conpoint_x = cp_x;
  con_create_conpoint_y = cp_y;
  con_create_conpoint_num = cp_num;
  con_create_last_x = cp_x;
  con_create_last_y = cp_y;
  con_create_active = 1;
}

void FlowCtx::redraw_node_cons( void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->redraw_node_cons( node);
  }
}

void FlowCtx::delete_node_cons( void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    i -= a.a[i]->delete_node_cons( node);
  }
}

void FlowCtx::con_create_dest( FlowArrayElem *node, int cp_num, 
		flow_eEvent event, int x, int y)
{
  if ( node == con_create_node && cp_num == con_create_conpoint_num)
  {
    con_create_active = 0;
    return;
  }

/*
  FlowCon *c1 =  new FlowCon( this, "", (FlowConClass *)default_conclass, 
	(FlowNode *)con_create_node, (FlowNode *)node, con_create_conpoint_num, cp_num);
  FlowCon *c1 =  new FlowCon( this, "", (FlowConClass *)default_conclass, 
	(FlowNode *)con_create_node, (FlowNode *)node, 
	con_create_conpoint_num, cp_num, 3, 5,5,8,8,15,15,0,0,0,0,0,0,0,0,0,0);
  insert( c1);
*/

  if ( event_callback[event_create_con] )
  {
    static flow_sEvent e;

    e.event = event;
    e.any.type = flow_eEventType_CreateCon;
    e.any.x_pixel = x;
    e.any.y_pixel = y;
    e.any.x = (x + offset_x) / zoom_factor;
    e.any.y = (y + offset_y) / zoom_factor;
    e.con_create.source_object = con_create_node;
    e.con_create.source_conpoint = con_create_conpoint_num;
    e.con_create.dest_object = node;
    e.con_create.dest_conpoint = cp_num;
    event_callback[event_create_con]( this, &e);
  }

  con_create_active = 0;
}

void FlowCtx::zoom( double factor)
{ 
  if ( fabs(factor) < DBL_EPSILON)
    return;

//  cout << "Before zoom zoom factor : " << zoom_factor << ", offset : " <<
//     offset_x << "," << offset_y << endl;
  zoom_factor *= factor;
  offset_x = int( (offset_x - window_width / 2.0 * ( 1.0/factor - 1)) * factor);
  offset_y = int( (offset_y  - window_height / 2.0 * ( 1.0/factor - 1)) * factor);
//  cout << "After  zoom zoom factor : " << zoom_factor << ", offset : " <<
//     offset_x << "," << offset_y << endl;
  a.zoom();
  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();
  change_scrollbar();
}

void FlowCtx::zoom_absolute( double factor)
{ 
  if ( fabs(factor) < DBL_EPSILON)
    return;

  zoom_factor = factor;
  offset_x = int( (offset_x - window_width / 2.0 * ( 1.0/factor - 1)) * factor);
  offset_y = int( (offset_y  - window_height / 2.0 * ( 1.0/factor - 1)) * factor);
  a.zoom();
  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();
}

void FlowCtx::select_clear()
{
  if ( event_callback[flow_eEvent_SelectClear] )
  {
    /* Send a selection clear callback */
    static flow_sEvent e;

    e.event = flow_eEvent_SelectClear;
    e.any.type = flow_eEventType_Object;
    e.any.x_pixel = 0;
    e.any.y_pixel = 0;
    e.any.x = 0;
    e.any.y = 0;
    e.object.object_type = flow_eObjectType_NoObject;
    e.object.object = 0;
    event_callback[flow_eEvent_SelectClear]( this, &e);
  }
  if ( !user_highlight) 
    set_select_highlight(0);
  a_sel.clear();
}

void FlowCtx::traverse( int x, int y)
{
//  cout << "Before trav zoom factor : " << zoom_factor << ", offset : " <<
//     offset_x << "," << offset_y << endl;
  offset_x -= x;
  offset_y -= y;
  if ( con_create_active)
  {
    con_create_conpoint_x += x;
    con_create_conpoint_y += y;
  }
  if ( select_rect_active)
  {
    if ( select_rect_ll_x == select_rect_start_x)
      select_rect_ll_x += x;
    if ( select_rect_ll_y == select_rect_start_y)
      select_rect_ll_y += y;
    if ( select_rect_ur_x == select_rect_start_x)
      select_rect_ur_x += x;
    if ( select_rect_ur_y == select_rect_start_y)
      select_rect_ur_y += y;
    select_rect_start_x += x;
    select_rect_start_y += y;
  }
//  cout << "After  trav zoom factor : " << zoom_factor << ", offset : " <<
//     offset_x << "," << offset_y << endl;
  a.traverse( x, y);
  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();
}

void FlowCtx::get_borders()
{ 
  x_right = -1e10;
  x_left = 1e10;
  y_high = -1e10;
  y_low = 1e10;
  a.get_borders( &x_right, &x_left, &y_high, &y_low);
}

void FlowCtx::set_defered_redraw()
{
  if ( defered_redraw_active)
    defered_redraw_active++;
  else
  {
    defered_x_low = window_width;
    defered_x_high = 0;
    defered_y_low = window_height;
    defered_y_high = 0;
    defered_x_low_nav = nav_window_width;
    defered_x_high_nav = 0;
    defered_y_low_nav = nav_window_height;
    defered_y_high_nav = 0;
    defered_redraw_active = 1;
  }
}

void FlowCtx::redraw_defered()
{
  defered_redraw_active--;
  if ( !defered_redraw_active )
  {
    draw( defered_x_low, defered_y_low, defered_x_high, defered_y_high);
    nav_draw( defered_x_low_nav, defered_y_low_nav, defered_x_high_nav, 
	defered_y_high_nav);
  }
}

void FlowCtx::print( double ll_x, double ll_y, double ur_x, double ur_y)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->print( ll_x, ll_y, ur_x, ur_y);
  }
}

void FlowCtx::draw( int ll_x, int ll_y, int ur_x, int ur_y)
{
  int		i;

  if ( defered_redraw_active)
  {
    if ( ll_x < defered_x_low)
      defered_x_low = ll_x;
    if ( ll_y < defered_y_low)
      defered_y_low = ll_y;
    if ( ur_x > defered_x_high)
      defered_x_high = ur_x;
    if ( ur_y > defered_y_high)
      defered_y_high = ur_y;
     return;
  }
  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->draw( ll_x, ll_y, ur_x, ur_y);
  }
  if ( select_rect_active)
  {
    flow_draw_rect( this, select_rect_ll_x, select_rect_ll_y, 
	  select_rect_ur_x - select_rect_ll_x,
	  select_rect_ur_y - select_rect_ll_y, flow_eDrawType_Line, 0, 0);
  }
}

void FlowCtx::clear()
{
  flow_draw_clear( this);
}

void FlowCtx::cut()
{
  int i;

  if ( !a_sel.size())
    return;
  paste_clear();
  a_paste.copy_from( a_sel);
  for ( i = 0; i < a_sel.size(); i++)
  {
    remove( a_sel[i]);
    a_sel[i]->remove_notify();
  }
  select_clear();
  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();
}

void FlowCtx::copy()
{
  if ( !a_sel.size())
    return;
  paste_clear();
  a_paste.copy_from(a_sel);
  select_clear();
}
void FlowCtx::paste()
{
  if ( cursor_present)
    paste_execute();
  else
    node_movement_paste_pending = 1;
}

void FlowCtx::paste_execute()
{
  int 	i;
  double ll_x, ll_y, ur_x, ur_y;
  int	delta_x, delta_y;

  a_paste.zoom();
  a_paste.nav_zoom();
  if ( application_paste)
    a_move.copy_from_common_objects( a_paste);
  else
  {
    move_clear();
    a_move.copy_from(a_paste);
  }
  for ( i = 0; i < a_move.size(); i++)
  {
    if ( a_move[i]->type() == flow_eObjectType_Con)
      ((FlowCon *)a_move[i])->set_movement_type( flow_eMoveType_Frozen);
  }

  select_clear();
  
  for ( i = 0; i < a_move.size(); i++)
  {
    a.insert( a_move[i]);
  }
  if ( application_paste)
    a_sel.copy_from_common_objects(a_move);
  else
    a_sel.copy_from(a_move);

  ur_x = -1e10;
  ll_x = 1e10;
  ur_y = -1e10;
  ll_y = 1e10;
  a_move.get_borders( &ur_x, &ll_x, &ur_y, &ll_y);

  delta_x = int( (ur_x + ll_x) / 2 * zoom_factor - offset_x - cursor_x);
  delta_y = int( (ur_y + ll_y) / 2  * zoom_factor - offset_y - cursor_y);
  node_movement_paste_active = 1;
  set_defered_redraw();
  a_move.move_noerase( -delta_x, -delta_y, 0);
  node_move_last_x = cursor_x;
  node_move_last_y = cursor_y;
  redraw_defered();
  draw_set_cursor( this, draw_eCursor_DiamondCross);
  nav_zoom();

//  if ( !user_highlight)
//    set_select_highlight(1);

  if ( event_callback[flow_eEvent_PasteSequenceStart] )
  {
    static flow_sEvent e;

    e.event = flow_eEvent_PasteSequenceStart;
    e.any.type = flow_eEventType_Object;
    e.any.x_pixel = 0;
    e.any.y_pixel = 0;
    e.any.x = 0;
    e.any.y = 0;
    e.object.object_type = flow_eObjectType_NoObject;
    e.object.object = 0;
    event_callback[flow_eEvent_PasteSequenceStart]( this, &e);
  }
}

void FlowCtx::nav_zoom()
{
  if ( nodraw)
    return;
  if ( a.size() > 0)
  {
    double x_nav_left, x_nav_right, y_nav_low, y_nav_high;

    get_borders();
//    cout << "Borders : <" << x_left << " > " << x_right << " ^ " << 
//		y_high << " Y " << y_low << endl;
    x_nav_left = min( x_left, offset_x / zoom_factor);
    x_nav_right = max( x_right, (offset_x + window_width) / zoom_factor);
    y_nav_low = min( y_low, offset_y / zoom_factor);
    y_nav_high = max( y_high, (offset_y + window_height) / zoom_factor);
    if ( x_nav_right - x_nav_left == 0 || y_nav_high - y_nav_low == 0)
      return;
    nav_zoom_factor = min( nav_window_width / (x_nav_right - x_nav_left),
	nav_window_height / (y_nav_high - y_nav_low));
    nav_offset_x = int( x_nav_left * nav_zoom_factor);
    nav_offset_y = int( y_nav_low * nav_zoom_factor);
//    nav_window_width = 200;
//    nav_window_height = y_nav_high * nav_zoom_factor - nav_offset_y;
//    if ( nav_window_height < 200) 
//      nav_window_height = 200;
//    flow_draw_set_nav_window_size( this, nav_window_width, nav_window_height);
    a.nav_zoom();
    nav_clear();
    nav_draw( 0, 0, nav_window_width, nav_window_height);
  }
}

void FlowCtx::print_zoom()
{
  a.print_zoom();
}

int FlowCtx::print_region( double ll_x, double ll_y, double ur_x, 
	double ur_y, char *filename)
{
  int sts;

  print_ps = new FlowPscript( filename, this, 0, &sts);
  if ( ODD(sts))
    print_ps->print_page( ll_x, ll_y, ur_x, ur_y);
  delete print_ps;

  return sts;
}

void FlowCtx::nav_clear()
{
  flow_draw_nav_clear( this);
}

void FlowCtx::nav_draw( int ll_x, int ll_y, int ur_x, int ur_y)
{
  int		i;

  if ( ll_x == ur_x)
     return;

  if ( defered_redraw_active)
  {
    if ( ll_x < defered_x_low_nav)
      defered_x_low_nav = ll_x;
    if ( ll_y < defered_y_low_nav)
      defered_y_low_nav = ll_y;
    if ( ur_x > defered_x_high_nav)
      defered_x_high_nav = ur_x;
    if ( ur_y > defered_y_high_nav)
      defered_y_high_nav = ur_y;
    return;
  }

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->nav_draw( ll_x, ll_y, ur_x, ur_y);
  }

  nav_rect_ll_x = int( nav_zoom_factor * offset_x / zoom_factor - nav_offset_x);
  nav_rect_ur_x = int( nav_zoom_factor * (offset_x + window_width) / 
		zoom_factor - nav_offset_x);
  nav_rect_ll_y = int( nav_zoom_factor * offset_y / zoom_factor - nav_offset_y);
  nav_rect_ur_y = int( nav_zoom_factor * (offset_y + window_height) / 
		zoom_factor - nav_offset_y);

//  cout << "Nav rect: ll : " << nav_rect_ll_x << "," << nav_rect_ll_y << 
//	" ur: " << nav_rect_ur_x << "," << nav_rect_ur_y << endl;
  flow_draw_nav_rect( this, nav_rect_ll_x, nav_rect_ll_y, 
	nav_rect_ur_x - nav_rect_ll_x, nav_rect_ur_y - nav_rect_ll_y,
	flow_eDrawType_Line, 0, 0);
}

void FlowCtx::find_grid( double x, double y, double *x_grid, double *y_grid)
{
  double x1, y1;

  if ( !grid_on)
  {
    *x_grid = x;
    *y_grid = y;
    return;
  }
  x1 = floor( x / grid_size_x) * grid_size_x;
  if ( x - x1 < 0.5 * grid_size_x)
    *x_grid = x1;
  else
    *x_grid = x1 + grid_size_x;

  y1 = floor( y / grid_size_y) * grid_size_y;
  if ( y - y1 < 0.5 * grid_size_y)
    *y_grid = y1;
  else
    *y_grid = y1 + grid_size_y;
}


int FlowCtx::event_handler( flow_eEvent event, int x, int y, int w, int h)
{
  int sts;
  int i;
  FlowCtx	*ctx;
  int node_move_event = 0;

  ctx = this;
//  cout << "Event: " << event << endl;

  callback_object_type = flow_eObjectType_NoObject;
  callback_object = 0;

  if ( event == event_create_con)
  {
    sts = 0;
    for ( i = 0; i < a.a_size; i++)
    {
      sts = a.a[i]->event_handler( event, x, y);
      if ( sts)
        break;
    }
  }
  else if ( event == event_create_node)
  {
  }
  else if ( event == event_move_node)
  {
    move_clear();

    sts = 0;
    for ( i = 0; i < a.a_size; i++)
    {
      sts = a.a[i]->event_handler( event, x, y);
      if ( sts)
        break;
    }
    if ( sts)
    {
      int j, node_cnt;

      // If trace is started, move only tracenodes
      if ( trace_started &&
	   a_move[0]->type() == flow_eObjectType_Node &&
	   ((FlowNode *)a_move[0])->get_group() != flow_eNodeGroup_Trace)
        return 1;

      if ( a_sel.size() > 1 && select_find( a_move[0]))
      {

        /* Move all selected nodes */
        move_clear();

	/* Insert nodes first and then all connections connected to the nodes */
        for ( i = 0; i < a_sel.size(); i++)
        {
	  if ( a_sel[i]->type() == flow_eObjectType_Node)
            move_insert( a_sel[i]);
	}
        node_movement_active = 1;
        node_move_last_x = x;
        node_move_last_y = y;
      }

      node_movement_active = 1;
      node_move_last_x = x;
      node_move_last_y = y;

      /* Insert all connnected cons for movement */
      node_cnt = a_move.size();
      for ( i = 0; i < node_cnt; i++)
      {
        for ( j = 0; j < a.size(); j++)
        {
	  if ( a[j]->type() == flow_eObjectType_Con &&
	       ((FlowCon *)a[j])->is_connected_to( (FlowNode *)a_move[i]))
	  {
            if ( move_insert( a[j]))
              ((FlowCon *)a[j])->set_movement_type( a_move.a, node_cnt);
	  }
	}
      }

      draw_set_cursor( ctx, draw_eCursor_DiamondCross);
      if ( event == event_region_select) 
        // Move and region select is defined as the same event
        node_move_event = 1;
    }
  }
  if ( (event == event_region_select && !node_move_event) || 
       event == event_region_add_select) 
  {
    select_rect_active = 1;
    select_rect_event = event;
    select_rect_start_x = x;
    select_rect_start_y = y;
    select_rect_last_x = x;
    select_rect_last_y = y;
    select_rect_ll_x = x;
    select_rect_ll_y = y; 
    select_rect_ur_x = x;
    select_rect_ur_y = y; 
    return 1;
  }

  switch ( event)
  {
    case flow_eEvent_MB1Click:
      if ( node_movement_paste_active)
      {
        if ( auto_scrolling_active)
          auto_scrolling_stop();
        set_defered_redraw();
        a_move.move( x - node_move_last_x, y - node_move_last_y, grid_on);
        node_move_last_x = x;
        node_move_last_y = y;
        redraw_defered();
        node_movement_paste_active = 0;
	nav_zoom();
        draw_set_cursor( ctx, draw_eCursor_Normal);

	/* Send callback for all move objects */
	if ( event_callback[flow_eEvent_ObjectMoved] )
	{
	  static flow_sEvent e;
	
          e.event = flow_eEvent_ObjectMoved;
	  e.any.type = flow_eEventType_Object;
	  e.any.x_pixel = x;
	  e.any.y_pixel = y;
	  e.any.x = 1.0 * (x + offset_x) / zoom_factor;
	  e.any.y = 1.0 * (y + offset_y) / zoom_factor;
          for ( i = 0; i < a_move.size(); i++)
          {
	    e.object.object = a_move[i];
	    e.object.object_type = a_move[i]->type();
	    event_callback[event_move_node]( this, &e);
          }
	}
        return 1;
      }
    case flow_eEvent_MB1DoubleClick:
    case flow_eEvent_MB1ClickShift:
    case flow_eEvent_MB1DoubleClickShift:
    case flow_eEvent_MB1ClickCtrl:
    case flow_eEvent_MB1DoubleClickCtrl:
    case flow_eEvent_MB1ClickShiftCtrl:
    case flow_eEvent_MB1DoubleClickShiftCtrl:
    case flow_eEvent_MB2Click:
    case flow_eEvent_MB2DoubleClick:
    case flow_eEvent_MB2ClickShift:
    case flow_eEvent_MB2DoubleClickShift:
    case flow_eEvent_MB2ClickCtrl:
    case flow_eEvent_MB2DoubleClickCtrl:
    case flow_eEvent_MB2ClickShiftCtrl:
    case flow_eEvent_MB2DoubleClickShiftCtrl:
    case flow_eEvent_MB3Click:
    case flow_eEvent_MB3Press:
      sts = 0;
      for ( i = 0; i < a.a_size; i++)
      {
        sts = a.a[i]->event_handler( event, x, y);
        if ( sts == FLOW__DESTROYED)
          return sts;
        if ( sts == FLOW__NO_PROPAGATE)
          break;
      }
      break;

    case flow_eEvent_Exposure:
      int width, height;

      flow_draw_get_window_size( ctx, &width, &height);
      if ( window_width != width || window_height != height) {
        window_width = width;
        window_height = height;
	
        if ( event_callback[flow_eEvent_Resized]) {
	  static flow_sEvent e;

          e.event = flow_eEvent_Resized;
	  e.any.type = flow_eEventType_CallBack;
	  event_callback[flow_eEvent_Resized]( this, &e);
        }
      }
      nav_zoom();
//      draw( 0, 0, window_width, window_height);
      draw( x, y, x + w, y + h);
      if ( ctx_type == flow_eCtxType_Brow)
        ((BrowCtx *)this)->change_scrollbar();
      break;
    case flow_eEvent_CursorMotion:
      cursor_present = 1;
      cursor_x = x;
      cursor_y = y;
      if ( node_movement_paste_active)
      {
        set_defered_redraw();
        a_move.move( x - node_move_last_x, y - node_move_last_y, 0);
        node_move_last_x = x;
        node_move_last_y = y;
        redraw_defered();
      }
      sts = 0;
      for ( i = 0; i < a.a_size; i++)
      {
        sts = a.a[i]->event_handler( event, x, y);
      }
      break;
    case flow_eEvent_ButtonMotion:
      if ( node_movement_active)
      {
        set_defered_redraw();
        a_move.move( x - node_move_last_x, y - node_move_last_y, 0);
        node_move_last_x = x;
        node_move_last_y = y;
        redraw_defered();
      }
      else if ( con_create_active)
      {
        flow_draw_line_erase( this, con_create_conpoint_x, 
		con_create_conpoint_y, con_create_last_x, con_create_last_y ,0);
        draw( con_create_conpoint_x, con_create_conpoint_y, 
		con_create_last_x, con_create_last_y);
        flow_draw_line( this, con_create_conpoint_x, con_create_conpoint_y,
		x, y, flow_eDrawType_Line, 0, 0);
        con_create_last_x = x;
        con_create_last_y = y;
        for ( i = 0; i < a.a_size; i++)
        {
          sts = a.a[i]->event_handler( flow_eEvent_CursorMotion, x, y);
        }
      }
      else if ( select_rect_active)
      {
        flow_draw_rect_erase( this, select_rect_ll_x, select_rect_ll_y,
	  select_rect_ur_x - select_rect_ll_x,
	  select_rect_ur_y - select_rect_ll_y, 0);

        select_rect_ll_x = min( x, select_rect_start_x);
        select_rect_ll_y = min( y, select_rect_start_y); 
        select_rect_ur_x = max( x, select_rect_start_x);
        select_rect_ur_y = max( y, select_rect_start_y); 

        draw( select_rect_ll_x, select_rect_ll_y,
	  select_rect_ur_x, select_rect_ur_y);

        select_rect_last_x = x;
        select_rect_last_y = y;
      }
      break;
    case flow_eEvent_ButtonRelease:
      if ( auto_scrolling_active)
        auto_scrolling_stop();
      if ( node_movement_active)
      {
        set_defered_redraw();
        a_move.move( x - node_move_last_x, y - node_move_last_y, grid_on);
        node_move_last_x = x;
        node_move_last_y = y;
        redraw_defered();
        node_movement_active = 0;
	nav_zoom();
        draw_set_cursor( ctx, draw_eCursor_CrossHair);

	/* Send callback for all move objects */
	if ( event_callback[flow_eEvent_ObjectMoved] )
	{
	  static flow_sEvent e;
	
          e.event = flow_eEvent_ObjectMoved;
	  e.any.type = flow_eEventType_Object;
	  e.any.x_pixel = x;
	  e.any.y_pixel = y;
	  e.any.x = 1.0 * (x + offset_x) / zoom_factor;
	  e.any.y = 1.0 * (y + offset_y) / zoom_factor;
          for ( i = 0; i < a_move.size(); i++)
          {
	    e.object.object = a_move[i];
	    e.object.object_type = a_move[i]->type();
	    event_callback[event_move_node]( this, &e);
          }
	}
      }
      else if ( select_rect_active)
      {
        select_rect_active = 0;
        select_rect_last_x = x;
        select_rect_last_y = y;
        select_rect_ll_x = min( x, select_rect_start_x);
        select_rect_ll_y = min( y, select_rect_start_y); 
        select_rect_ur_x = max( x, select_rect_start_x);
        select_rect_ur_y = max( y, select_rect_start_y);

        flow_draw_rect_erase( this, select_rect_ll_x, select_rect_ll_y,
	  select_rect_ur_x - select_rect_ll_x,
	  select_rect_ur_y - select_rect_ll_y, 0);

        draw( select_rect_ll_x, select_rect_ll_y,
	  select_rect_ur_x, select_rect_ur_y);

        /* Save the final select area */
        select_area_ll_x = (select_rect_ll_x + offset_x) / zoom_factor;
        select_area_ll_y = (select_rect_ll_y + offset_y) / zoom_factor;
        select_area_ur_x = (select_rect_ur_x + offset_x) / zoom_factor;
        select_area_ur_y = (select_rect_ur_y + offset_y) / zoom_factor;

        if ( select_rect_event == event_region_select)
        {
	  /* Insert selected objects to selectlist */

          select_clear();
          select_region_insert( select_area_ll_x, select_area_ll_y,
		select_area_ur_x, select_area_ur_y);
        }
        if ( select_rect_event == event_region_add_select)
        {
	  /* Add selected objects to selectlist */
          select_region_insert( select_area_ll_x, select_area_ll_y,
		select_area_ur_x, select_area_ur_y);
        }
        /* Send event backcall */
        if ( event_callback[select_rect_event] )
        {
          static flow_sEvent e;

          e.event = select_rect_event;
          e.any.type = flow_eEventType_Object;
          e.any.x_pixel = x;
          e.any.y_pixel = y;
          e.any.x = 1.0 * (x + offset_x) / zoom_factor;
          e.any.y = 1.0 * (y + offset_y) / zoom_factor;
          e.object.object_type = flow_eObjectType_NoObject;
          e.object.object = 0;
          event_callback[select_rect_event]( this, &e);
        }
      }
      else if ( con_create_active)
      {
        flow_draw_line_erase( this, con_create_conpoint_x,
		con_create_conpoint_y, con_create_last_x, con_create_last_y, 0);
        draw( con_create_conpoint_x, con_create_conpoint_y, 
		con_create_last_x, con_create_last_y);

	/* Find the destination node */
        for ( i = 0; i < a.a_size; i++)
        {
          sts = a.a[i]->event_handler( event, x, y);
          if ( sts)
            break;
        }
	if ( !sts)
	{
	  /* No hit */
	  if ( event_callback[event_create_con] )
	  {
	    static flow_sEvent e;
	
	    e.event = event;
	    e.any.type = flow_eEventType_CreateCon;
	    e.any.x_pixel = x;
	    e.any.y_pixel = y;
	    e.any.x = 1.0 * (x + offset_x) / zoom_factor;
	    e.any.y = 1.0 * (y + offset_y) / zoom_factor;
	    e.con_create.source_object = con_create_node;
	    e.con_create.source_conpoint = con_create_conpoint_num;
	    e.con_create.dest_object = 0;
	    e.con_create.dest_conpoint = 0;
	    event_callback[event_create_con]( this, &e);
	  }
	}
        con_create_active = 0;
      }
      break;
    case flow_eEvent_Enter:
      cursor_present = 1;
      cursor_x = x;
      cursor_y = y;
      if ( node_movement_paste_pending)
      {
        node_movement_paste_pending = 0;
        paste_execute();
      }
      if ( auto_scrolling_active)
        auto_scrolling_stop();
      break;
    case flow_eEvent_Leave:
      cursor_present = 0;
      if ( node_movement_active || con_create_active || select_rect_active ||
	   node_movement_paste_active)
      {
        if ( x < 0 || x > ctx->window_width || y < 0 || y > window_height)
        {
          /* Start auto scrolling */
          auto_scrolling( this);
        }
      }
      else if ( x < 0 || x > ctx->window_width || y < 0 || y > window_height)
        a.set_hot( 0);
      break;
    case flow_eEvent_VisibilityUnobscured:
      unobscured = 1;
      break;
    case flow_eEvent_VisibilityObscured:
      unobscured = 0;
      break;
    default:
      ;
  }

  if ( event_callback[event] &&
	sts != FLOW__NO_PROPAGATE && event != event_move_node)
  {
    static flow_sEvent e;
    e.event = event;
    e.any.type = flow_eEventType_Object;
    e.any.x_pixel = x;
    e.any.y_pixel = y;
    e.any.x = 1.0 * (x + offset_x) / zoom_factor;
    e.any.y = 1.0 * (y + offset_y) / zoom_factor;
    e.object.object_type = callback_object_type;
    if ( callback_object_type == flow_eObjectType_Node)
      e.object.object = callback_object;
    else if ( callback_object_type == flow_eObjectType_Con)
      e.object.object = callback_object;
    sts = event_callback[event]( this, &e);
    return sts;
  }
  return 1;
}

int FlowCtx::event_handler_nav( flow_eEvent event, int x, int y)
{
  FlowCtx	*ctx;

  ctx = this;

  switch ( event)
  {
    case flow_eEvent_MB1Press:
      if ( nav_rect_ll_x < x && x < nav_rect_ur_x &&
           nav_rect_ll_y < y && y < nav_rect_ur_y)
      {
        nav_rect_movement_active = 1;
        nav_rect_move_last_x = x;
        nav_rect_move_last_y = y;
      }
      break;

    case flow_eEvent_MB2Press:
      if ( nav_rect_ll_x < x && x < nav_rect_ur_x &&
           nav_rect_ll_y < y && y < nav_rect_ur_y)
      {
        nav_rect_zoom_active = 1;
        nav_rect_move_last_x = x;
        nav_rect_move_last_y = y;
      }
      break;

    case flow_eEvent_CursorMotion:
      if ( nav_rect_ll_x < x && x < nav_rect_ur_x &&
           nav_rect_ll_y < y && y < nav_rect_ur_y)
      {
        if ( !nav_rect_hot)
        {
          draw_set_nav_cursor( ctx, draw_eCursor_CrossHair);
          nav_rect_hot = 1;
        }
      }
      else
      {
        if ( nav_rect_hot)
        {
          draw_set_nav_cursor( ctx, draw_eCursor_Normal);
          nav_rect_hot = 0;
        }
      }
      break;
    case flow_eEvent_Exposure:
      flow_draw_get_nav_window_size( ctx, &nav_window_width, &nav_window_height);
      nav_zoom();
      break;
    case flow_eEvent_ButtonMotion:
      if ( nav_rect_movement_active)
      {
        int delta_x, delta_y, mainwind_delta_x, mainwind_delta_y;

        flow_draw_nav_rect_erase( this, nav_rect_ll_x, nav_rect_ll_y, 
		nav_rect_ur_x - nav_rect_ll_x, nav_rect_ur_y - nav_rect_ll_y, 0);

	delta_x = x - nav_rect_move_last_x;
	delta_y = y - nav_rect_move_last_y;
        nav_rect_ll_x += delta_x;
        nav_rect_ur_x += delta_x;
        nav_rect_ll_y += delta_y;
        nav_rect_ur_y += delta_y;
        nav_rect_move_last_x = x;
        nav_rect_move_last_y = y;
        nav_draw( nav_rect_ll_x - 10, nav_rect_ll_y - 10, 
		nav_rect_ur_x + 10, nav_rect_ur_y + 10);
//        flow_draw_nav_rect( this, nav_rect_ll_x, nav_rect_ll_y, 
//		nav_rect_ur_x - nav_rect_ll_x, nav_rect_ur_y - nav_rect_ll_y,
//		flow_eDrawType_Line, 0, 0);

        mainwind_delta_x = int( - zoom_factor / nav_zoom_factor * delta_x);
        mainwind_delta_y = int( - zoom_factor / nav_zoom_factor * delta_y);
        offset_x -= mainwind_delta_x;
        offset_y -= mainwind_delta_y;
        a.traverse( mainwind_delta_x, mainwind_delta_y);

        draw_copy_area( ctx, mainwind_delta_x, mainwind_delta_y);
//        clear();
        if ( !unobscured) 
          draw( 0, 0, window_width, window_height);
        else
        {
          if ( mainwind_delta_x >= 0 && mainwind_delta_y >= 0)
          {
            if ( mainwind_delta_x)
              draw( 0, 0, mainwind_delta_x, window_height);
            if ( mainwind_delta_y)
              draw( mainwind_delta_x, 0, window_width, mainwind_delta_y);
          }
          else if ( mainwind_delta_x <= 0 && mainwind_delta_y <= 0)
          {
            if ( mainwind_delta_x)
              draw( window_width+mainwind_delta_x, 0, window_width, window_height);
            if ( mainwind_delta_y)
              draw( 0, window_height + mainwind_delta_y, 
		window_width + mainwind_delta_x, window_height);
          }
          else if ( mainwind_delta_x <= 0 && mainwind_delta_y >= 0)
          {
            if ( mainwind_delta_x)
              draw( window_width + mainwind_delta_x, 0, window_width, 
			window_height);
            if ( mainwind_delta_y)
              draw( 0, 0, window_width + mainwind_delta_x, mainwind_delta_y);
          }
          else
          {
            if ( mainwind_delta_x)
              draw( 0, 0, mainwind_delta_x, window_height);
            if ( mainwind_delta_y)
              draw( mainwind_delta_x, window_height+ mainwind_delta_y, 
		window_width, window_height);
          }
        }
        change_scrollbar();
      }
      else if ( nav_rect_zoom_active)
      {
        int delta_x, delta_y;
	double zoom_f;
        double center_x, center_y;
        double center_dist, center_dist_last;

        flow_draw_nav_rect_erase( this, nav_rect_ll_x, nav_rect_ll_y, 
		nav_rect_ur_x - nav_rect_ll_x, nav_rect_ur_y - nav_rect_ll_y, 0);

	delta_x = x - nav_rect_move_last_x;
	delta_y = y - nav_rect_move_last_y;

	center_x = 0.5 * (nav_rect_ur_x + nav_rect_ll_x);
	center_y = 0.5 * (nav_rect_ur_y + nav_rect_ll_y);
	center_dist_last = sqrt( (nav_rect_move_last_x - center_x) *
		       (nav_rect_move_last_x - center_x) + 
		       (nav_rect_move_last_y - center_y) *
		       (nav_rect_move_last_y - center_y));
        center_dist = sqrt((x - center_x)*(x - center_x) + 
	              (y - center_y)*(y - center_y));
        if ( center_dist < DBL_EPSILON)
          return 1;
        zoom_f =  center_dist_last / center_dist;
//        if ( fabs(zoom_f - 1) < 0.2)
//          return 1;
	zoom( zoom_f);
        nav_rect_move_last_x = x;
        nav_rect_move_last_y = y;
      }
      break;
    case flow_eEvent_ButtonRelease:
      if ( nav_rect_movement_active)
      {
        nav_rect_movement_active = 0;
	nav_zoom();
      }
      if ( nav_rect_zoom_active)
      {
        nav_rect_zoom_active = 0;
      }
      break;
    default:
      ;
  }
  return 1;
}

void FlowCtx::enable_event( flow_eEvent event, flow_eEventType event_type,
	int (*event_cb)( FlowCtx *ctx, flow_tEvent event))
{
  switch ( event_type)
  {
    case flow_eEventType_RegionSelect:
      event_region_select = event;
      break;
    case flow_eEventType_RegionAddSelect:
      event_region_add_select = event;
      break;
    case flow_eEventType_CreateCon:
      event_create_con = event;
      break;
    case flow_eEventType_CreateNode:
      event_create_node = event;
      break;
    case flow_eEventType_MoveNode:
      event_move_node = event;
      break;
    default:
      ;
  }
  event_callback[event] = event_cb;
}

void FlowCtx::disable_event( flow_eEvent event)
{
  if ( event_region_select == event)
    event_region_select = flow_eEvent_Null;
  else if ( event_create_con == event)
    event_create_con = flow_eEvent_Null;
  else if ( event_create_node == event)
    event_create_node = flow_eEvent_Null;
  else if ( event_move_node == event)
    event_move_node = flow_eEvent_Null;

  event_callback[event] = 0;
}

void FlowCtx::disable_event_all()
{
  event_region_select = flow_eEvent_Null;
  event_create_con = flow_eEvent_Null;
  event_create_node = flow_eEvent_Null;
  event_move_node = flow_eEvent_Null;

  for ( int i = 0; i < flow_eEvent__; i++)
    event_callback[i] = 0;
}

FlowArrayElem *FlowCtx::get_node_from_name( char *name)
{
  int i;
        
  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Node &&
	 strcmp( ((FlowNode *) a.a[i])->n_name, name) == 0)
      return a.a[i];
  }
  return 0;
}

FlowArrayElem *FlowCtx::get_nodeclass_from_name( char *name)
{
  int i;
        
  for ( i = 0; i < a_nc.a_size; i++)
  {
    if ( strcmp( ((FlowNodeClass *) a_nc.a[i])->nc_name, name) == 0)
      return a_nc.a[i];
  }
  return 0;
}

FlowArrayElem *FlowCtx::get_conclass_from_name( char *name)
{
  int i;
        
  for ( i = 0; i < a_cc.a_size; i++)
  {
    if ( strcmp( ((FlowConClass *) a_cc.a[i])->cc_name, name) == 0)
      return a_cc.a[i];
  }
  return 0;
}

void FlowCtx::remove_trace_objects()
{
  int i;
        
  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Node &&
	 ((FlowNode *) a.a[i])->nc->group == flow_eNodeGroup_Trace)
    {
      remove( a.a[i]);
      select_remove( a.a[i]);
      move_remove( a.a[i]);
      i--;
    }
    if ( a.a[i]->type() == flow_eObjectType_Con &&
	 ((FlowCon *) a.a[i])->cc->group == flow_eConGroup_Trace)
    {
      remove( a.a[i]);
      select_remove( a.a[i]);
      move_remove( a.a[i]);
      i--;
    }
  }
  for ( i = 0; i < a_nc.a_size; i++)
  {
    if ( ((FlowNodeClass *) a_nc.a[i])->group == flow_eNodeGroup_Trace)
    {
      a_nc.remove( a_nc.a[i]);
      i--;
    }
  }
  for ( i = 0; i < a_cc.a_size; i++)
  {
    if ( ((FlowConClass *) a_cc.a[i])->group == flow_eConGroup_Trace)
    {
      a_cc.remove( a_cc.a[i]);
      i--;
    }
  }
  a.zoom();
  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();
}

int FlowCtx::trace_init( int (*connect_func)( void *, char *, char *, flow_eTraceType, 
	void **), int (*disconnect_func)( void *), 
	int (*scan_func)( void *, void *))
{
  int i, sts;

  trace_connect_func = connect_func;
  trace_disconnect_func = disconnect_func;
  trace_scan_func = scan_func;
  sts = 1;
  for ( i = 0; i < a.a_size; i++)
  {
    sts = a.a[i]->trace_init();
    if ( !sts)
      break;
  }
  trace_started = 1;
  return sts;
}

void FlowCtx::trace_close()
{
  int i;

  for ( i = 0; i < a.a_size; i++)
    a.a[i]->trace_close();
  trace_started = 0;
}

void FlowCtx::trace_scan()
{
  int i;

  if ( nodraw)
     return;

  for ( i = 0; i < a.a_size; i++)
    a.a[i]->trace_scan();
}

void FlowCtx::get_selected_nodes( FlowArrayElem ***nodes, int *num)
{
  int i;

  *num = 0;
  if ( a_sel.size() == 0)
    return;
  *nodes = (FlowArrayElem **) calloc( a_sel.size(), sizeof(nodes));
  for ( i = 0; i < a_sel.size(); i++)
  {
    if ( a_sel[i]->type() == flow_eObjectType_Node)
    {
      (*nodes)[*num] = a_sel[i];
      (*num)++;
    }
  }
  if ( !*num)
     free( *nodes);
}

void FlowCtx::get_selected_cons( FlowArrayElem ***cons, int *num)
{
  int i;

  *num = 0;
  *cons = (FlowArrayElem **) calloc( a_sel.size(), sizeof(cons));
  for ( i = 0; i < a_sel.size(); i++)
  {
    if ( a_sel[i]->type() == flow_eObjectType_Con)
    {
      (*cons)[*num] = a_sel[i];
      (*num)++;
    }
  }
  if ( !*num)
     free( *cons);
}

void FlowCtx::center_object( FlowArrayElem *object)
{
  double ll_x, ll_y, ur_x, ur_y;

  flow_draw_get_window_size( this, &window_width, &window_height);

  ur_x = -1e10;
  ll_x = 1e10;
  ur_y = -1e10;
  ll_y = 1e10;
  object->get_borders( &ur_x, &ll_x, &ur_y, &ll_y);
  offset_x = int( ((ur_x + ll_x)/2 ) * zoom_factor - window_width/2);
  offset_y = int( ((ur_y + ll_y)/2 ) * zoom_factor - window_height/2);

  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();
  change_scrollbar();
}

FlowArrayElem *FlowCtx::get_document( double x, double y)
{
  int i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Node &&
	 ((FlowNode *)a.a[i])->nc->group == flow_eNodeGroup_Document &&
	 ((FlowNode *)a.a[i])->x_left < x &&
	 ((FlowNode *)a.a[i])->x_right > x &&
	 ((FlowNode *)a.a[i])->y_low < y &&
	 ((FlowNode *)a.a[i])->y_high > y)
      return a.a[i];
  }
  return 0;
}

void FlowCtx::reconfigure()
{
  int 		i;
  flow_sEvent 	e;
        
  set_nodraw();
  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Node)
    {
      if ( grid_on)
      {
        ((FlowNode *) a.a[i])->move( 0, 0, 1);
	if ( event_callback[flow_eEvent_ObjectMoved] )
	{
          e.event = flow_eEvent_ObjectMoved;
	  e.any.type = flow_eEventType_Object;
	  e.any.x_pixel = 0;
	  e.any.y_pixel = 0;
	  e.any.x = 0;
	  e.any.y = 0;
	  e.object.object = a[i];
	  e.object.object_type = flow_eObjectType_Node;
	  event_callback[event_move_node]( this, &e);
	}
      }
    }
  }
  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Con)
    {
      ((FlowCon *) a.a[i])->reconfigure();
      if ( event_callback[flow_eEvent_ObjectMoved] )
      {
        e.event = flow_eEvent_ObjectMoved;
	e.any.type = flow_eEventType_Object;
	e.any.x_pixel = 0;
	e.any.y_pixel = 0;
	e.any.x = 0;
	e.any.y = 0;
	e.object.object = a[i];
	e.object.object_type = flow_eObjectType_Con;
	event_callback[event_move_node]( this, &e);
      }
    }
  }
  reset_nodraw();
  a.zoom();
  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();
}

void FlowCtx::redraw()
{
  flow_draw_get_window_size( this, &window_width, &window_height);
  get_borders();
  clear();
  draw( 0, 0, window_width, window_height);
  nav_zoom();
  change_scrollbar();
  // Fix to correct calculations of node borders..
  a.get_borders();
}

void FlowCtx::object_deleted( FlowArrayElem *object)
{
  
  if ( event_callback[flow_eEvent_ObjectDeleted] )
  {
    /* Send an object deleted callback */
    static flow_sEvent e;

    e.event = flow_eEvent_ObjectDeleted;
    e.any.type = flow_eEventType_Object;
    e.any.x_pixel = 0;
    e.any.y_pixel = 0;
    e.any.x = 0;
    e.any.y = 0;
    e.object.object_type = object->type();
    e.object.object = object;
    event_callback[flow_eEvent_ObjectDeleted]( this, &e);
  }
}

void FlowCtx::annotation_input_cb( FlowArrayElem *object, int number, 
	char *text)
{
  
  if ( event_callback[flow_eEvent_AnnotationInput] )
  {
    /* Send an annotation input callback */
    static flow_sEvent e;

    e.event = flow_eEvent_AnnotationInput;
    e.any.type = flow_eEventType_AnnotationInput;
    e.any.x_pixel = 0;
    e.any.y_pixel = 0;
    e.any.x = 0;
    e.any.y = 0;
    e.annot_input.object_type = object->type();
    e.annot_input.object = object;
    e.annot_input.number = number;
    e.annot_input.text = text;
    event_callback[flow_eEvent_AnnotationInput]( this, &e);
  }
}

int FlowCtx::radiobutton_cb( FlowArrayElem *object, int number, 
	int value)
{
  
  if ( event_callback[flow_eEvent_Radiobutton] )
  {
    /* Send an radiobutton callback */
    static flow_sEvent e;

    e.event = flow_eEvent_Radiobutton;
    e.any.type = flow_eEventType_Radiobutton;
    e.any.x_pixel = 0;
    e.any.y_pixel = 0;
    e.any.x = 0;
    e.any.y = 0;
    e.radiobutton.object_type = object->type();
    e.radiobutton.object = object;
    e.radiobutton.number = number;
    e.radiobutton.value = value;
    return event_callback[flow_eEvent_Radiobutton]( this, &e);
  }
  return 1;
}

void FlowCtx::change_scrollbar()
{
  flow_sScroll data;

  if ( scroll_callback == NULL)
    return;

  scroll_size = window_width / 100 / zoom_factor;
  if ( scroll_size == 0)
    return;

  data.scroll_data = scroll_data;
  data.total_width = int( (x_right - x_left) / scroll_size);
  data.total_height = int( (y_high - y_low) / scroll_size);
  data.window_width = int( window_width / scroll_size / zoom_factor);
  data.window_height = int( window_height / scroll_size / zoom_factor + 1);
  data.offset_x = int( offset_x / scroll_size / zoom_factor - x_left / 
	scroll_size);
  data.offset_y = int( offset_y /scroll_size / zoom_factor - y_low / 
	scroll_size);

  (scroll_callback)( &data);
}

void FlowCtx::scroll( int delta_x, int delta_y)
{
  offset_x -= delta_x;
  offset_y -= delta_y;
  a.traverse( delta_x, delta_y);

  move_widgets( delta_x, delta_y);

  if ( window_width <= abs( delta_x) || window_height <= abs( delta_y) )
  {
    // Entirely new area
    clear();
    draw( 0, 0, window_width, window_height);
  }
  else
  {
    draw_copy_area( this, delta_x, delta_y);
    if ( !unobscured || widget_cnt) 
      draw( 0, 0, window_width, window_height);
    else
    {
      if ( delta_x >= 0 && delta_y >= 0)
      {
        if ( delta_x)
          draw( 0, 0, delta_x, window_height);
        if ( delta_y)
          draw( delta_x, 0, window_width, delta_y);
      }
      else if ( delta_x <= 0 && delta_y <= 0)
      {
        if ( delta_x)
          draw( window_width+delta_x, 0, window_width, window_height);
        if ( delta_y)
          draw( 0, window_height + delta_y, 
		window_width + delta_x, window_height);
      }
      else if ( delta_x <= 0 && delta_y >= 0)
      {
        if ( delta_x)
          draw( window_width + delta_x, 0, window_width, 
		window_height);
        if ( delta_y)
          draw( 0, 0, window_width + delta_x, delta_y);
      }
      else
      {
        if ( delta_x)
          draw( 0, 0, delta_x, window_height);
        if ( delta_y)
          draw( delta_x, window_height+ delta_y, 
		window_width, window_height);
      }
    }
  }
  nav_clear();
  nav_draw( 0, 0, nav_window_width, nav_window_height);
}

void FlowCtx::auto_scrolling_stop()
{
  auto_scrolling_active = 0;
  draw_cancel_timer( auto_scrolling_id);
}

void auto_scrolling( FlowCtx *ctx)
{
  int delta_x, delta_y;

  ctx->auto_scrolling_active = 1;
  if ( ctx->node_movement_active || ctx->node_movement_paste_active)
  {
    delta_x = - (ctx->node_move_last_x - ctx->window_width / 2) / 3;
    delta_y = - (ctx->node_move_last_y - ctx->window_height / 2) / 3;

    ctx->set_defered_redraw();
    ctx->a_move.move( -delta_x, -delta_y, 0);
    ctx->redraw_defered();
  }
  else if ( ctx->select_rect_active)
  {
    delta_x = - (ctx->select_rect_last_x - ctx->window_width / 2) / 3;
    delta_y = - (ctx->select_rect_last_y - ctx->window_height / 2) / 3;
  }
  else if ( ctx->con_create_active)
  {
    delta_x = - (ctx->con_create_last_x - ctx->window_width / 2) / 3;
    delta_y = - (ctx->con_create_last_y - ctx->window_height / 2) / 3;
  }
 
  ctx->traverse( delta_x,delta_y);
  ctx->nav_draw( 0, 0, ctx->nav_window_width, ctx->nav_window_height);
  ctx->change_scrollbar();
  draw_set_timer( ctx, 300, auto_scrolling, &ctx->auto_scrolling_id);
}

void flow_scroll_horizontal( FlowCtx *ctx, int value, int bottom)
{
  int x_pix;

  x_pix = int( - value * ctx->scroll_size * ctx->zoom_factor + 
	(ctx->offset_x - ctx->x_left * ctx->zoom_factor));
  ctx->scroll( x_pix, 0);
}

void flow_scroll_vertical( FlowCtx *ctx, int value, int bottom)
{
  int y_pix;

  y_pix = int( - value * ctx->scroll_size * ctx->zoom_factor + 
	(ctx->offset_y - ctx->y_low * ctx->zoom_factor));
  ctx->scroll( 0, y_pix);
}
