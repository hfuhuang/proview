/* 
 * Proview   $Id: xtt_ev_motif.h,v 1.1 2007-01-04 08:30:03 claes Exp $
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

#ifndef xtt_ev_motif_h
#define xtt_ev_motif_h

/* xtt_ev_motif.h -- Alarm and event windows in xtt */

#ifndef xtt_ev_h
# include "xtt_ev.h"
#endif

#ifndef cow_wow_motif_h
# include "cow_wow_motif.h"
#endif

class EvMotif : public Ev {
  public:
    EvMotif(
	void *ev_parent_ctx,
	Widget	ev_parent_wid,
	char *eve_name,
	char *ala_name,
	char *blk_name,
	pwr_tObjid ev_user,
	int display_ala,
	int display_eve,
	int display_blk,
	int display_return,
	int display_ack,
	int ev_beep,
	pwr_tMask ev_pop_mask,
	pwr_tStatus *status);
    ~EvMotif();

    Widget		parent_wid;
    Widget		parent_wid_eve;
    Widget		parent_wid_ala;
    Widget		parent_wid_blk;
    Widget		toplevel_ala;
    Widget		toplevel_eve;
    Widget		toplevel_blk;
    Widget		form_ala;
    Widget		form_eve;
    Widget		form_blk;
    Widget		eve_widget;
    Widget		ala_widget;
    Widget		blk_widget;
    CoWowFocusTimerMotif eve_focustimer;
    CoWowFocusTimerMotif ala_focustimer;
    CoWowFocusTimerMotif blk_focustimer;

    void map_eve();
    void map_ala();
    void map_blk();
    void unmap_eve();
    void unmap_ala();
    void unmap_blk();

    static void eve_action_inputfocus( Widget w, XmAnyCallbackStruct *data);
    static void ala_action_inputfocus( Widget w, XmAnyCallbackStruct *data);
    static void blk_action_inputfocus( Widget w, XmAnyCallbackStruct *data);
    static void eve_activate_exit( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_exit( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_exit( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_print( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_print( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_print( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_ack_last( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_ack_last( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_zoom_in( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_zoom_in( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_zoom_in( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_zoom_out( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_zoom_out( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_zoom_out( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_zoom_reset( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_zoom_reset( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_zoom_reset( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_block_remove( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_open_plc( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_open_plc( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_open_plc( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_display_in_xnav( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_display_in_xnav( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_display_in_xnav( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_disp_hundredth( Widget w, Ev *ev, XmToggleButtonCallbackStruct *data);
    static void ala_activate_disp_hundredth( Widget w, Ev *ev, XmToggleButtonCallbackStruct *data);
    static void eve_activate_hide_object( Widget w, Ev *ev, XmToggleButtonCallbackStruct *data);
    static void ala_activate_hide_object( Widget w, Ev *ev, XmToggleButtonCallbackStruct *data);
    static void eve_activate_hide_text( Widget w, Ev *ev, XmToggleButtonCallbackStruct *data);
    static void ala_activate_hide_text( Widget w, Ev *ev, XmToggleButtonCallbackStruct *data);
    static void eve_activate_help( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_help( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_activate_help( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_activate_helpevent( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_activate_helpevent( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void eve_create_form( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void ala_create_form( Widget w, Ev *ev, XmAnyCallbackStruct *data);
    static void blk_create_form( Widget w, Ev *ev, XmAnyCallbackStruct *data);
};

#endif



