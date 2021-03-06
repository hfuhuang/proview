/* 
 * Proview   $Id: xtt_tblnav_gtk.h,v 1.1 2008-07-17 11:18:31 claes Exp $
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

#ifndef xtt_tblnav_gtk_h
#define xtt_tblnav_gtk_h

/* xtt_tblnav_gtk.h -- Simple navigator */

#include <gtk/gtk.h>

#ifndef xtt_tblnav_h
# include "../src/xtt_tblnav.h"
#endif

//! The navigation area of the attribute editor.
class TblNavGtk : public TblNav {
  public:
    TblNavGtk(
	void *xn_parent_ctx,
	GtkWidget	*xn_parent_wid,
	sevcli_sHistItem  *xn_itemlist,
	int xn_item_cnt,
	GtkWidget **w,
	pwr_tStatus *status);
    ~TblNavGtk();

    GtkWidget		*parent_wid;
    GtkWidget		*brow_widget;
    GtkWidget		*form_widget;
    GtkWidget		*toplevel;

    void set_inputfocus();
};

#endif
