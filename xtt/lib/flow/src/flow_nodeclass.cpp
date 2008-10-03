/* 
 * Proview   $Id: flow_nodeclass.cpp,v 1.8 2008-10-03 14:19:19 claes Exp $
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

#include "flow_std.h"

#include <vector.h>
#include <float.h>
#include <iostream.h>
#include <fstream.h>
#include "flow_nodeclass.h"
#include "flow_annot.h"
#include "flow_annotpixmap.h"
#include "flow_conpoint.h"
#include "flow_msg.h"

class NextConPoint {
 public:
  FlowConPoint *cp;
  double distance;
  double angle;
  double rank;
};

FlowNodeClass::FlowNodeClass( FlowCtx *flow_ctx, char *name, 
	flow_eNodeGroup grp)
  : ctx(flow_ctx), a(10,10), group(grp), no_con_obstacle(0)
{
  strcpy( nc_name, name);
}

void FlowNodeClass::print( FlowPoint *pos, void *node, int highlight)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->print( pos, node, highlight);
  }
}

void FlowNodeClass::save( ofstream& fp, flow_eSaveMode mode)
{

  if ( (mode == flow_eSaveMode_Trace && group != flow_eNodeGroup_Trace) ||
       (mode == flow_eSaveMode_Edit && group == flow_eNodeGroup_Trace) )
    return;
  fp <<	int(flow_eSave_NodeClass) << endl;
  fp <<	int(flow_eSave_NodeClass_nc_name) << FSPACE << nc_name << endl;
  fp <<	int(flow_eSave_NodeClass_a) << endl;
  a.save( fp, mode);
  fp <<	int(flow_eSave_NodeClass_group) << FSPACE << int(group) << endl;
  fp << int(flow_eSave_NodeClass_no_con_obstacle) << FSPACE << no_con_obstacle << endl;
  fp <<	int(flow_eSave_End) << endl;
}

void FlowNodeClass::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case flow_eSave_NodeClass: break;
      case flow_eSave_NodeClass_nc_name:
        fp.get();
        fp.getline( nc_name, sizeof(nc_name));
        break;
      case flow_eSave_NodeClass_a: a.open( ctx, fp); break;
      case flow_eSave_NodeClass_group: fp >> tmp; group = (flow_eNodeGroup)tmp; break;
      case flow_eSave_NodeClass_no_con_obstacle: fp >> no_con_obstacle; break;
      case flow_eSave_End: end_found = 1; break;
      default:
        cout << "FlowNodeClass:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

void FlowNodeClass::draw( FlowPoint *pos, int highlight, int hot, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->draw( pos, highlight, hot, node);
  }
}

void FlowNodeClass::nav_draw( FlowPoint *pos, int highlight, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->nav_draw( pos, highlight, node);
  }
}

void FlowNodeClass::draw_inverse( FlowPoint *pos, int hot, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    switch ( a.a[i]->type())
    { 
      case flow_eObjectType_Radiobutton:
      case flow_eObjectType_Image:
        a.a[i]->draw( pos, 0, hot, node);
        break;
      default:
        a.a[i]->draw_inverse( pos, hot, node);
    }
  }
}

void FlowNodeClass::erase( FlowPoint *pos, int hot, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->erase( pos, hot, node);
  }
}

void FlowNodeClass::nav_erase( FlowPoint *pos, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->nav_erase( pos, node);
  }
}

int FlowNodeClass::get_conpoint( int num, double *x, double *y, 
	flow_eDirection *dir)
{
  int		i, sts;

  for ( i = 0; i < a.a_size; i++)
  {
    sts = a.a[i]->get_conpoint( num, x, y, dir);
    if ( sts)
      return sts;
  }
  return FLOW__NOCONPOINT;
}

int FlowNodeClass::get_conpoint_trace_attr( int num, char *trace_attr, 
	flow_eTraceType *type)
{
  int		i, inverted;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_ConPoint &&
         ((FlowConPoint *)a.a[i])->number == num)
    {
      a.a[i]->get_trace_attr( NULL, trace_attr, type, &inverted);
      return FLOW__SUCCESS;
    }
  }
  return FLOW__NOCONPOINT;
}

int FlowNodeClass::event_handler( void *pos, flow_eEvent event, int x, int y,
	void *node)
{
  return a.event_handler( pos, event, x, y, node);
}

void FlowNodeClass::erase_annotation( void *pos, int highlight, int hot, 
	void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Annot &&
         ((FlowAnnot *)a.a[i])->number == num)
    {
      a.a[i]->erase( pos, hot, node);
      a.a[i]->nav_erase( pos, node);
      break;
    }
  }
}

void FlowNodeClass::draw_annotation( void *pos, int highlight, int hot, 
	void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Annot &&
         ((FlowAnnot *)a.a[i])->number == num)
    {
      a.a[i]->draw( pos, highlight, hot, node);
      a.a[i]->nav_draw( pos, highlight, node);
      break;
    }
  }
}

void FlowNodeClass::open_annotation_input( void *pos, void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Annot &&
         ((FlowAnnot *)a.a[i])->number == num)
    {
      ((FlowAnnot *)a.a[i])->open_annotation_input( pos, node);
      break;
    }
  }
}

void FlowNodeClass::close_annotation_input( void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Annot &&
         ((FlowAnnot *)a.a[i])->number == num)
    {
      ((FlowAnnot *)a.a[i])->close_annotation_input( node);
      break;
    }
  }
}

int FlowNodeClass::get_annotation_input( void *node, int num, char **text)
{
  int		i, sts;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Annot &&
         ((FlowAnnot *)a.a[i])->number == num)
    {
      sts = ((FlowAnnot *)a.a[i])->get_annotation_input( node, text);
      break;
    }
  }
  return sts;
}

void FlowNodeClass::move_widgets( void *node, int x, int y)
{
  for ( int i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Annot )
      ((FlowAnnot *)a.a[i])->move_widgets( node, x, y);
  }
}

void FlowNodeClass::configure_annotations( void *pos, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Annot)
    {
      ((FlowAnnot *)a.a[i])->configure_annotations( pos, node);
    }
    else if ( a.a[i]->type() == flow_eObjectType_AnnotPixmap)
    {
      ((FlowAnnotPixmap *)a.a[i])->configure_annotations( pos, node);
    }
  }
}

void FlowNodeClass::measure_annotation( int num, char *text, double *width,
	double *height)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == flow_eObjectType_Annot &&
         ((FlowAnnot *)a.a[i])->number == num)
    {
      ((FlowAnnot *)a.a[i])->measure_annot( text, width, height);
      break;
    }
  }
}

void FlowNodeClass::get_obstacle_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node)
{ 
  int i;

  if ( group == flow_eNodeGroup_Document || no_con_obstacle) {
    for ( i = 0; i < a.a_size; i++) {
      if ( a.a[i]->type() == flow_eObjectType_Rect)
	a.a[i]->get_borders(pos_x, pos_y, x_right, x_left, y_high, y_low, node);
    }
  }
  else
    a.get_borders(pos_x, pos_y, x_right, x_left, y_high, y_low, node);
}

void FlowNodeClass::get_object_name( char *name)
{
  strcpy( name, nc_name);
}

int FlowNodeClass::load( char *filename)
{
  ifstream fp;

  fp.open( filename);
  if ( !fp)
    return FLOW__FILEOPEN;

  open( fp);

  fp.close();
  return FLOW__SUCCESS;
}

int FlowNodeClass::get_next_conpoint( int cp_num, flow_eDirection dir, double x0, double y0, int *next_cp_num)
{
  double x, y, a_x, a_y;
  double dir_angle;
  vector<NextConPoint> a0;

  switch ( dir) {
  case flow_eDirection_Left:
    dir_angle = M_PI;
    break;
  case flow_eDirection_Right:
    dir_angle = 0;
    break;
  case flow_eDirection_Up:
    dir_angle = -M_PI/2;
    break;
  case flow_eDirection_Down:
    dir_angle = M_PI/2;
    break;
  default: ;
  }
  
  if ( cp_num == -1) {
    x = x0;
    y = y0;
  }
  else {
    int found = 0;
    for ( int i = 0; i < a.a_size; i++) {
      if ( a.a[i]->type() == flow_eObjectType_ConPoint &&
	   ((FlowConPoint *)a.a[i])->number == cp_num) {
	x = ((FlowConPoint *)a.a[i])->p.x;
	y = ((FlowConPoint *)a.a[i])->p.y;
	found = 1;
	break;
      }
    }
    if ( !found)
      return 0;
  }

  for ( int i = 0; i < a.a_size; i++) {
    if ( a.a[i]->type() == flow_eObjectType_ConPoint &&
	 ((FlowConPoint *)a.a[i])->number != cp_num) {
      
      a_x = ((FlowConPoint *)a.a[i])->p.x;
      a_y = ((FlowConPoint *)a.a[i])->p.y;
	
      NextConPoint n;
      n.cp = (FlowConPoint *)a[i];
      n.distance = sqrt((a_x - x)*(a_x - x) + (a_y - y)*(a_y - y));
      if ( fabs( a_y - y) < DBL_EPSILON) {
	if ( a_x > x)
	  n.angle = 0;
	else
	  n.angle = M_PI;
      }
      else {
	n.angle = atan((a_x - x)/(a_y - y)) + M_PI / 2;
	if ( (a_y - y) > 0)
	  n.angle -= M_PI;
      }
      
      double rank_angel = n.angle + dir_angle;
      double rank_distance = n.distance;
      if ( rank_angel > M_PI)
	rank_angel -= 2 * M_PI;
      if ( rank_angel < -M_PI)
	rank_angel += 2 * M_PI;
      rank_angel = fabs( rank_angel) / M_PI;
      if ( rank_angel >= 0.5 - DBL_EPSILON)
	continue;
      n.rank = rank_angel + ( 0.3 + rank_distance);
      a0.push_back( n);
    }
  }

  if ( a0.size() == 0)
    return 0;

  // Find best object
  double rank_min = 1E37;
  FlowConPoint *rank_elem = 0;
  for ( int i = 0; i < (int)a0.size(); i++) {

    if ( a0[i].rank < rank_min) {
      rank_min = a0[i].rank;
      rank_elem = a0[i].cp;
    }
  }
  if ( !rank_elem)
    return 0;
  *next_cp_num = rank_elem->number;
  return 1;
}
