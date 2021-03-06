/* 
 * Proview   $Id: wb_utl_gtk.h,v 1.2 2008-10-31 12:51:31 claes Exp $
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

#include "wb_utl.h"

#ifndef wb_utl_gtk_h
#define wb_utl_gtk_h


class wb_utl_gtk : public wb_utl {
 public:
  GtkWidget *widget;

  wb_utl_gtk( GtkWidget *w) : widget(w) {}
  ~wb_utl_gtk() {}
  int create_mainwindow( int argc, char **argv);
  int destroy_mainwindow();
  int utl_foe_new( const char *name, pwr_tOid	plcpgm,
		   ldh_tWBContext ldhwbctx, ldh_tSesContext ldhsesctx,
		   WFoe **foectx, int map_window, ldh_eAccess access);
  int utl_foe_new_local( WFoe *foectx, const char *name, pwr_tOid plcpgm, 
			 ldh_tWBContext ldhwbctx, ldh_tSesContext ldhsesctx, 
			 vldh_t_node nodeobject, unsigned long windowindex, 
			 unsigned long new_window, WFoe **return_foectx, 
			 int map_window, ldh_eAccess access, 
			 foe_eFuncAccess function_access);
};

#endif
