/* 
 * Proview   $Id: wb_wge.cpp,v 1.3 2008-10-31 12:51:32 claes Exp $
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
 */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>

#include "glow_ctx.h"

#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"

#include "glow_growctx.h"
#include "glow_growapi.h"
#include "co_lng.h"
#include "wb_wge.h"
#include "ge_graph.h"

void WGe::graph_init_cb( void *client_data)
{
  WGe *ge = (WGe *) client_data;
  char fname[120];
  int		default_width;
  int		default_height;
  int		sts;

  cdh_ToLower( fname, ge->filename);
  if ( ! strrchr( fname, '.'))
    strcat( fname, ".pwg");
  ((Graph *)ge->graph)->open( fname);

  ((Graph *)ge->graph)->init_trace();

  if ( ge->width == 0 || ge->height == 0) {
    sts = ((Graph *)ge->graph)->get_default_size( &default_width, &default_height);
    if ( ODD(sts)) {
      ge->set_size( default_width, default_height);
    }
  }

  ((Graph *)ge->graph)->set_default_layout();

}

void WGe::graph_close_cb( void *client_data)
{
  WGe *ge = (WGe *) client_data;
  if ( ge->modal)
    ge->terminated = 1;
  else
    delete ge;
}

int WGe::wge_command_cb( void *ge_ctx, char *command)
{
  WGe	*ge = (WGe *)ge_ctx;
  int		sts;

  if ( ge->command_cb) {
    sts = (ge->command_cb)( ge, command);
    return sts;
  }
  return 0;
}

int WGe::wge_is_authorized_cb( void *ge_ctx, unsigned int access)
{
  return 1;
}

void WGe::message( void *ctx, char severity, const char *message)
{
  if ( strcmp( message, "") != 0)
    printf("** WGe: %s\n", message);
}

int WGe::set_object_focus( char *name, int empty)
{
  return graph->set_object_focus( name, empty);
}

int WGe::set_folder_index( char *name, int idx)
{
  return graph->set_folder_index( name, idx);
}

int WGe::set_subwindow_source( char *name, char *source, int modal)
{
  return graph->set_subwindow_source( name, source);
}

void WGe::set_subwindow_release()
{
  subwindow_release = 1;
}

WGe::~WGe()
{
}

void WGe::print()
{
  pwr_tFileName filename;
  pwr_tCmd cmd;

  dcli_translate_filename( filename, "$pwrp_tmp/graph.ps");
  graph->print( filename);

  sprintf( cmd, "$pwr_exe/rt_print.sh %s 1", filename);
  system(cmd);
}

WGe::WGe( void *wge_parent_ctx, char *wge_name, char *wge_filename,
	  int wge_scrollbar, int wge_menu, int wge_navigator, int wge_width, int wge_height, 
	  int x, int y, char *object_name, int wge_modal = 0) :
  parent_ctx(wge_parent_ctx), graph(0), scrollbar(wge_scrollbar),
  navigator(wge_navigator), menu(wge_menu), current_value_object(0), current_confirm_object(0),
  value_input_open(0), confirm_open(0), command_cb(0), close_cb(0),
  help_cb(0), is_authorized_cb(0), width(wge_width), height(wge_height),
  modal(wge_modal), terminated(0), subwindow_release(0)
{
  strcpy( filename, wge_filename);
  strcpy( name, wge_name);
}

