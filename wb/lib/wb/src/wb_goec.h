/* 
 * Proview   $Id: wb_goec.h,v 1.4 2007-01-04 07:29:03 claes Exp $
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

#ifndef wb_goec_h
#define wb_goec_h

#ifndef wb_gre_h
#include "wb_gre.h"
#endif


int
goec_con_draw(
  WGre          *gre,
  vldh_t_con    con_object,
  int           create_flag,
  vldh_t_node   node
);

int
goec_con_delete( 
  WGre         *gre,
  vldh_t_con   con_object
);

int
goec_con_delete_noredraw( 
  WGre          *gre,
  vldh_t_con   con_object
);

int
goec_con_sethighlight(
  WGre            *gre,
  vldh_t_con      con_object,
  unsigned long   highlight_flag
);

int
goec_con_gethighlight(
  WGre             *gre,
  vldh_t_con       con_object,
  unsigned long   *highlight_flag
);

int
goec_con_draw_ref( 
  WGre          *gre,
  vldh_t_con    con_object,
  int           create_flag,
  vldh_t_node   node
);

int
goec_get_refnodetypeid(
  WGre             *gre,
  unsigned long   *refnode_type_id
);

int
goec_con_delete_ref( 
  WGre         *gre,
  vldh_t_con   con_object
);

int
goec_con_delete_ref_noredraw(
  WGre         *gre,
  vldh_t_con   con_object
);

int
goec_con_redraw_ref(
  WGre          *gre,
  vldh_t_node   node
);

int
goec_con_reconfig_ref(
  WGre          *gre,
  vldh_t_node   node
);

int
goec_conpoint_get_refcons( 
  vldh_t_node      node,
  unsigned long    point,
  unsigned long    flag,
  unsigned long   *cons
);

#endif
