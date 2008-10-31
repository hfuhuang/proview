/* 
 * Proview   $Id: flow_pushbutton.cpp,v 1.4 2008-10-31 12:51:33 claes Exp $
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
#include "flow.h"
#include "flow_ctx.h"
#include "flow_draw.h"
#include "flow_array.h"
#include "flow_pushbutton.h"
#include "flow_msg.h"

FlowPushButton::FlowPushButton( FlowCtx *flow_ctx, char *p_name,
	double x1, double y1, double width, double height) : 
	ctx(flow_ctx), pos(flow_ctx, x1,y1), 
	r(flow_ctx, x1, y1, width, height, flow_eDrawType_Line, 1), 
	t(flow_ctx, p_name, x1, y1+height/2, flow_eDrawType_TextHelvetica, 1)
{
  strcpy( name, p_name);
  zoom_factor = ctx->zoom_factor;
  draw(0,0,0,0);
}

void FlowPushButton::draw( int ll_x, int ll_y, int ur_x, int ur_y) 
{ 
  FlowPoint korr_pos(ctx);

//  pos.z_x = pos.x * zoom_factor + ctx->offset_x;
//  pos.z_y = pos.y * zoom_factor + ctx->offset_y;
//  korr_pos.z_x = pos.z_x + ctx->offset_x;
//  korr_pos.z_y = pos.z_y + ctx->offset_y;
//  r.draw( &pos, 0);
//  t.draw( &pos, 0);

  ctx->fdraw->rect( ctx, pos.z_x + r.ll.z_x, pos.z_y + r.ll.z_y, 
	  r.ur.z_x - r.ll.z_x, r.ur.z_y - r.ll.z_y, r.draw_type, 
	  r.line_width-1, 0);
  ctx->fdraw->text( ctx, pos.z_x + t.p.z_x, pos.z_y + t.p.z_y, t.text,
	  strlen(t.text), t.draw_type, t.text_size, 0, 0);
}

int FlowPushButton::event_handler( flow_eEvent event, int x, int y)
{
  int sts;

  sts = 0;
  switch ( event)
  {
    case flow_eEvent_MB1Click:

      if ( r.ll.z_x + pos.z_x <= x && 
         x <= r.ur.z_x  + pos.z_x  &&
         r.ll.z_y  + pos.z_y <= y && 
         y <= r.ur.z_y + pos.z_y)
      {
        sts = FLOW__NO_PROPAGATE;
        if ( strcmp( name, "Zoom in") == 0)
          ctx->zoom( 1.25);
        else if ( strcmp( name, "Zoom out") == 0)
          ctx->zoom( 0.8);
        else if ( strcmp( name, "Right") == 0)
          ctx->traverse( 50, 0);
        else if ( strcmp( name, "Left") == 0)
          ctx->traverse( -50, 0);
        else if ( strcmp( name, "Up") == 0)
          ctx->traverse( 0, -50);
        else if ( strcmp( name, "Down") == 0)
          ctx->traverse( 0, 50);
        else if ( strcmp( name, "Copy") == 0)
          ctx->copy();
        else if ( strcmp( name, "Cut") == 0)
          ctx->cut();
        else if ( strcmp( name, "Paste") == 0)
          ctx->paste();
        else if ( strcmp( name, "ConType") == 0)
        {
          int i;
          for ( i = 0; i < ctx->a_cc.size(); i++)
          {
             if ( ctx->default_conclass == ctx->a_cc[i])
               break;
          }
          if ( ++i >= ctx->a_cc.size())
            i = 0;
          ctx->set_default_conclass( ctx->a_cc[i]);
        }
        else
          cout << "Unknown pushbutton" << endl;
        break; 
     default:
       ;
   }
  }
  return sts;
}

ostream& operator<< ( ostream& o, const FlowPushButton p)
{
  o << "PushButton: " << p.name << endl;
  return o;
}

