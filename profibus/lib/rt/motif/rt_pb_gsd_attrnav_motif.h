/* 
 * Proview   $Id: rt_pb_gsd_attrnav_motif.h,v 1.1 2007-01-04 08:43:47 claes Exp $
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

#ifndef rt_pb_gsd_attrnav_motif_h
#define rt_pb_gsd_attrnav_motif_h

/* rt_pb_gsd_attrnav_motif.h -- Profibus gsd configurator navigator */

#ifndef rt_pb_gsd_attrnav_h
# include "rt_pb_gsd_attrnav.h"
#endif

//! The navigation area of the attribute editor.
class GsdAttrNavMotif : public GsdAttrNav {
  public:
    GsdAttrNavMotif(
	void *xn_parent_ctx,
	Widget xn_parent_wid,
	char *xn_name,
	pb_gsd *xn_gsd,
	int xn_edit_mode,
	Widget *w,
	pwr_tStatus *status);
    ~GsdAttrNavMotif();
    void set_inputfocus();

    Widget		parent_wid;
    Widget		brow_widget;
    Widget		form_widget;
    Widget		toplevel;
};


#endif