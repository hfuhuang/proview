/* 
 * Proview   $Id: flow_point.cpp,v 1.4 2008-10-31 12:51:33 claes Exp $
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


#include <iostream>
#include "flow_point.h"


FlowPoint::FlowPoint( FlowCtx *flow_ctx, double x1, double y1)
 : ctx(flow_ctx), x(x1), y(y1)
{
  (*this).zoom();
}

void FlowPoint::posit( double x1, double y1)
{
  x = x1;
  y = y1;
  z_x = int( x * ctx->zoom_factor + 0.5);
  z_y = int( y * ctx->zoom_factor + 0.5);
  nav_z_x = int( x * ctx->nav_zoom_factor + 0.5);
  nav_z_y = int( y * ctx->nav_zoom_factor + 0.5);
}

void FlowPoint::posit_z( int x1, int y1)
{
  x = 1.0 * x1 / ctx->zoom_factor;
  y = 1.0 * y1 / ctx->zoom_factor;
  z_x = x1;
  z_y = y1;
  nav_z_x = int( x * ctx->nav_zoom_factor);
  nav_z_y = int( y * ctx->nav_zoom_factor);
}

void FlowPoint::zoom()
{
  z_x = int( x * ctx->zoom_factor + 0.5);
  z_y = int( y * ctx->zoom_factor + 0.5);
  current_zoom_factor = ctx->zoom_factor;
}

void FlowPoint::nav_zoom()
{
  nav_z_x = int( x * ctx->nav_zoom_factor + 0.5);
  nav_z_y = int( y * ctx->nav_zoom_factor + 0.5);
}

void FlowPoint::print_zoom()
{
  print_z_x = x * ctx->print_zoom_factor;
  print_z_y = y * ctx->print_zoom_factor;
}

void FlowPoint::traverse( int x, int y)
{
}

void FlowPoint::save( ofstream& fp, flow_eSaveMode mode)
{
  fp << int(flow_eSave_Point) << endl;
  fp << int(flow_eSave_Point_x) << FSPACE << x << endl;
  fp << int(flow_eSave_Point_y) << FSPACE << y << endl;
  fp << int(flow_eSave_End) << endl;
}

void FlowPoint::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case flow_eSave_Point: break;
      case flow_eSave_Point_x: fp >> x; break;
      case flow_eSave_Point_y: fp >> y; break;
      case flow_eSave_End: end_found = 1; break;
      default:
        cout << "FlowPoint:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

const FlowPoint& FlowPoint::operator+= (const FlowPoint p)
{
  x += p.x;  
  y += p.y;
  z_x += p.z_x;  
  z_y += p.z_y;
  return *this;
}

FlowPoint FlowPoint::operator+ (const FlowPoint p)
{
  FlowPoint tmp(*this);
  tmp += p;
  return tmp;
}

const FlowPoint& FlowPoint::operator-= (const FlowPoint p)
{
  x -= p.x;  
  y -= p.y;
  z_x -= p.z_x;  
  z_y -= p.z_y;
  return *this;
}

FlowPoint FlowPoint::operator- (const FlowPoint p)
{
  FlowPoint tmp(*this);
  tmp -= p;
  return tmp;
}

ostream& operator<< ( ostream& o, const FlowPoint p)
{
  o << '(' << p.x << ',' << p.y << ')' << '[' << p.z_x << ',' << p.z_y << ']' ;
  return o;
}

