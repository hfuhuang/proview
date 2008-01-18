/* 
 * Proview   $Id: flow_tiptext.cpp,v 1.1 2008-01-18 13:55:06 claes Exp $
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
#include "flow_tiptext.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream.h>
#include <math.h>
#include "flow_ctx.h"
#include "flow_draw.h"

static void tiptext_timer_cb( FlowCtx *ctx)
{
  ctx->tiptext->timer_id = 0;
  ctx->tiptext->active = true;

  ctx->tiptext->draw();
}
FlowTipText::~FlowTipText()
{
  if ( timer_id)
    ctx->fdraw->cancel_timer( ctx, timer_id);
}

void FlowTipText::draw_text( FlowArrayElem *e, char *text, int x, int y) 
{
  int	z_width, z_height;
  int   row;
  char  *s, *t;

  if ( active)
    remove_text( text_object);
  if ( timer_id) {
    ctx->fdraw->cancel_timer( ctx, timer_id);
    timer_id = 0;
  }

  row = 0;
  for ( s = text, t = tiptext[0]; *s; s++) {
    if ( *s == 10) {
      *t = 0;
      row++;
      if ( row >= TIPTEXT_ROWS)
	break;
      t = tiptext[row];
    }
    else {
      *t = *s;
      t++;
      if ( t - tiptext[0] >= (int)sizeof(tiptext))
	break;
    }
  }
  *t = 0;

  tiptext_rows = row + 1;
  for ( int i = row + 1; i < TIPTEXT_ROWS; i++)
    tiptext[i][0] = 0;

  text_width = 0;
  text_height = 0;
  for ( int i = 0; i < tiptext_rows; i++) {
    ctx->fdraw->get_text_extent( ctx, tiptext[i], strlen(tiptext[i]), flow_eDrawType_TextHelvetica, 2, 
				 &z_width, &z_height);
    if ( z_width > text_width)
      text_width = z_width;
    text_height += z_height + 3;
  }
  text_x = x;
  text_y = y;
  text_width += 8;
  text_height += 4;
  text_descent = 4;
  text_object = e;

  if ( text_x + text_width > ctx->window_width)
    text_x = ctx->window_width - text_width;
  if ( text_x < 0)
    text_x = 0;
  if ( text_y + text_height > ctx->window_height)
    text_y = ctx->window_height - text_height;
  if ( text_y < 0)
    text_y = 0;

  ctx->fdraw->set_timer( ctx, 1000, tiptext_timer_cb, &timer_id);
}

void FlowTipText::draw()
{
  if ( !active || !tiptext_rows)
    return;

  ctx->fdraw->fill_rect( ctx, text_x, text_y, text_width, text_height,
		       flow_eDrawType_LineErase);
  ctx->fdraw->rect( ctx, text_x, text_y, text_width, text_height,
		       flow_eDrawType_Line, 0, 0);

  int y = text_y + 4 + (text_height-4)/tiptext_rows;
  for ( int i = 0; i < tiptext_rows; i++) {
    ctx->fdraw->text( ctx, text_x + 6, y - text_descent - 2, 
		      tiptext[i], strlen(tiptext[i]), flow_eDrawType_TextHelvetica, 2, 
		      0, 0);
    y += (text_height-3)/tiptext_rows;
  }
}

void FlowTipText::remove_text( FlowArrayElem *e)
{
  if ( e != text_object)
    return;

  if ( timer_id) {
    ctx->fdraw->cancel_timer( ctx, timer_id);
    timer_id = 0;
    return;
  }

  if ( active) {
    active = false;
    ctx->fdraw->fill_rect( ctx, text_x, text_y, text_width + 1, text_height + 1,
		       flow_eDrawType_LineErase);
    ctx->draw( text_x, text_y, text_x + text_width + 1, text_y + text_height + 1);
  }
}

void FlowTipText::remove()
{
  if ( timer_id) {
    ctx->fdraw->cancel_timer( ctx, timer_id);
    timer_id = 0;
  }

  if ( active)
    remove_text( text_object);
}












