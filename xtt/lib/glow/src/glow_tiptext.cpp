/* 
 * Proview   $Id: glow_tiptext.cpp,v 1.2 2005-09-01 14:57:54 claes Exp $
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

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream.h>
#include <math.h>
#include "glow_tiptext.h"
#include "glow_ctx.h"
#include "glow_draw.h"
#include "glow_growctx.h"

static void tiptext_timer_cb( GlowCtx *ctx)
{
  ctx->tiptext->timer_id = 0;
  ctx->tiptext->active = true;

  ctx->tiptext->draw();
}
GlowTipText::~GlowTipText()
{
  if ( timer_id)
    draw_remove_timer( timer_id);
}

void GlowTipText::draw_text( GlowArrayElem *e, char *text, int x, int y) 
{
  int	z_width, z_height, z_descent;

  if ( active)
    remove_text( text_object);
  if ( timer_id) {
    draw_remove_timer( timer_id);
    timer_id = 0;
  }

  draw_get_text_extent( ctx, text, strlen(text), glow_eDrawType_TextHelvetica, 2, &z_width,
			&z_height, &z_descent);

  text_x = x;
  text_y = y;
  text_width = z_width + 4;
  text_height = z_height + 4;
  text_descent = z_descent;
  strncpy( tiptext, text, sizeof(tiptext));
  text_object = e;

  if ( text_x + text_width > ctx->window_width)
    text_x = ctx->window_width - text_width;
  if ( text_x < 0)
    text_x = 0;
  if ( text_y + text_height > ctx->window_height)
    text_y = ctx->window_height - text_height;
  if ( text_y < 0)
    text_y = 0;

  draw_set_timer( ctx, 1000, tiptext_timer_cb, &timer_id);
}

void GlowTipText::draw()
{
  if ( !active)
    return;

  glow_draw_fill_rect( ctx, text_x, text_y, text_width, text_height,
		       glow_eDrawType_Color22);
  glow_draw_rect( ctx, text_x, text_y, text_width, text_height,
		       glow_eDrawType_Line, 0, 0);
  glow_draw_text( ctx, text_x + 2, text_y + text_height - text_descent - 2, tiptext, 
		  strlen(tiptext), glow_eDrawType_TextHelvetica, glow_eDrawType_Line, 2, 0, 0);

}

void GlowTipText::remove_text( GlowArrayElem *e)
{
  if ( e != text_object)
    return;

  if ( timer_id) {
    draw_remove_timer( timer_id);
    timer_id = 0;
    return;
  }

  if ( active) {
    active = false;
    glow_draw_fill_rect( ctx, text_x, text_y, text_width + 1, text_height + 1,
		       glow_eDrawType_LineErase);
    ctx->draw( text_x, text_y, text_x + text_width + 1, text_y + text_height + 1);
  }
}

void GlowTipText::remove()
{
  if ( timer_id) {
    draw_remove_timer( timer_id);
    timer_id = 0;
  }

  if ( active)
    remove_text( text_object);
}












