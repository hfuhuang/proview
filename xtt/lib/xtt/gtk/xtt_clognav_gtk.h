/* 
 * Proview   $Id: xtt_clognav_gtk.h,v 1.1 2007-01-04 08:29:32 claes Exp $
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

#ifndef xtt_clognav_gtk_h
#define xtt_clognav_gtk_h

/* xtt_clognav_gtk.h -- Console message window. */


// Status is defined as int i xlib...

#ifndef xtt_clognav_h
# include "xtt_clognav.h"
#endif

class CLogNavGtk : public CLogNav {
  public:
    CLogNavGtk( void *ev_parent_ctx,
	     GtkWidget *ev_parent_wid,
	     GtkWidget **w);
    ~CLogNavGtk();

    GtkWidget		*parent_wid;
    GtkWidget		*brow_widget;
    GtkWidget		*form_widget;
    GtkWidget		*toplevel;

    void set_input_focus();
};

#endif


