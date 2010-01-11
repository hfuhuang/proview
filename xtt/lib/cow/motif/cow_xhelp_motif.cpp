/** 
 * Proview   $Id: co_xhelp_motif.cpp,v 1.2 2008-10-31 12:51:30 claes Exp $
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

#if defined OS_VMS && defined __ALPHA
# pragma message disable (NOSIMPINT,EXTROUENCUNNOBJ)
#endif

#if defined OS_VMS && !defined __ALPHA
# pragma message disable (LONGEXTERN)
#endif

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Xm/Protocols.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"

#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"
#include "flow_x.h"
#include "cow_mrm_util.h"

#include "co_lng.h"
#include "cow_xhelpnav_motif.h"
#include "cow_xhelp_motif.h"

void CoXHelpMotif::open_input_dialog( CoXHelpMotif *xhelp, const char *text, const char *title,
	char *init_text,
	void (*ok_cb)( CoXHelp *, char *))
{
  Arg		args[10];
  int 		i;
  XmString	cstr;

  i = 0;
  XtSetArg(args[0], XmNlabelString,
           cstr=XmStringCreateLtoR((char*)  text, (char*) "ISO8859-1") ); i++;
  XtSetValues( xhelp->india_label, args, i);
  XmStringFree( cstr);
  i = 0;
  XtSetArg(args[0], XmNdialogTitle,
           cstr=XmStringCreateLtoR( (char*) title, (char*) "ISO8859-1") ); i++;
  XtSetValues( xhelp->india_widget, args, i);
  XmStringFree( cstr);

  XmTextSetString( xhelp->india_text, init_text);

  XtManageChild( xhelp->india_widget);

  XmProcessTraversal( xhelp->india_text, XmTRAVERSE_CURRENT);

  xhelp->india_ok_cb = ok_cb;
}

void CoXHelpMotif::activate_exit( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  if ( xhelp->close_cb)
    (xhelp->close_cb)( xhelp->parent_ctx, (void *)xhelp);
  else {
    flow_UnmapWidget( xhelp->parent_wid);
    xhelp->displayed = 0;
  }
  //  delete xhelp;
}

void CoXHelpMotif::activate_zoom_in( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  double zoom_factor;

  xhelp->xhelpnav->get_zoom( &zoom_factor);
  if ( zoom_factor > 40)
    return;

  xhelp->xhelpnav->zoom( 1.18);
}

void CoXHelpMotif::activate_zoom_out( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  double zoom_factor;

  xhelp->xhelpnav->get_zoom( &zoom_factor);
  if ( zoom_factor < 15)
    return;

  xhelp->xhelpnav->zoom( 1.0 / 1.18);
}

void CoXHelpMotif::activate_zoom_reset( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  xhelp->xhelpnav->unzoom();
}

void CoXHelpMotif::activate_search( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  CoXHelpMotif::open_input_dialog( xhelp, "Search string", "Search string",
	(char*) "", &CoXHelp::find_ok);
}

void CoXHelpMotif::activate_searchnext( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  xhelp->xhelpnav->search_next();
}

void CoXHelpMotif::activate_searchprevious( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  xhelp->xhelpnav->search_next_reverse();
}

void CoXHelpMotif::create_india_label( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  xhelp->india_label = w;
}
void CoXHelpMotif::create_india_text( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  xhelp->india_text = w;
}
void CoXHelpMotif::activate_india_ok( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  char *value;

  value = XmTextGetString( xhelp->india_text);
  XtUnmanageChild( xhelp->india_widget);

  (xhelp->india_ok_cb)( xhelp, value);
}
void CoXHelpMotif::activate_india_cancel( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  XtUnmanageChild( xhelp->india_widget);
}

void CoXHelpMotif::activate_help( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  CoXHelp::dhelp( "helpwindow_refman", 0, navh_eHelpFile_Other, "$pwr_lang/man_dg.dat", 
		  true);
}

void CoXHelpMotif::create_xhelpnav_form( Widget w, CoXHelpMotif *xhelp, XmAnyCallbackStruct *data)
{
  xhelp->xhelpnav_form = w;
}

void CoXHelpMotif::enable_set_focus( CoXHelpMotif *xhelp)
{
  xhelp->set_focus_disabled--;
}

void CoXHelpMotif::disable_set_focus( CoXHelpMotif *xhelp, int time)
{
  xhelp->set_focus_disabled++;
  xhelp->focus_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext( xhelp->toplevel), time,
	(XtTimerCallbackProc)CoXHelpMotif::enable_set_focus, xhelp);
}

void CoXHelpMotif::action_inputfocus( Widget w, XmAnyCallbackStruct *data)
{
  Arg args[1];
  CoXHelpMotif *xhelp;

  XtSetArg    (args[0], XmNuserData, &xhelp);
  XtGetValues (w, args, 1);

  if ( !xhelp)
    return;

  if ( mrm_IsIconicState(w))
    return;

  if ( xhelp->set_focus_disabled)
    return;

  if ( xhelp->xhelpnav && xhelp->displayed) {
    xhelp->xhelpnav->set_inputfocus();
  }
  CoXHelpMotif::disable_set_focus( xhelp, 400);

}

void CoXHelpMotif::set_dimension( int width, int height)
{
  Arg 		args[3];

  int i = 0;
  if ( width) {
    XtSetArg( args[i], XmNwidth, width);i++;
  }
  if ( height) {
    XtSetArg( args[i], XmNheight, height);i++;
  }
  XtSetValues( toplevel, args,i);
}

CoXHelpMotif::~CoXHelpMotif()
{
  if ( set_focus_disabled)
    XtRemoveTimeOut( focus_timerid);

  delete xhelpnav;
  XtDestroyWidget( parent_wid);
}

CoXHelpMotif::CoXHelpMotif( 
	Widget 		xa_parent_wid,
	void 		*xa_parent_ctx,
	xhelp_eUtility	utility,
        int             *xa_sts) :
 	CoXHelp(xa_parent_ctx,utility,xa_sts), parent_wid(xa_parent_wid)
{
  char		uid_filename[120] = {"xtt_xhelp.uid"};
  char		*uid_filename_p = uid_filename;
  Arg 		args[20];
  pwr_tStatus	sts;
  char 		title[80];
  int		i;
  MrmHierarchy s_DRMh;
  MrmType dclass;
  char		name[] = "Proview/R Navigator";

  static char translations[] =
    "<FocusIn>: xhelp_inputfocus()\n";
  static XtTranslations compiled_translations = NULL;

  static XtActionsRec actions[] =
    {
      {(char*) "xhelp_inputfocus",      (XtActionProc) CoXHelpMotif::action_inputfocus}
    };

  static MrmRegisterArg	reglist[] = {
    {(char*) "xhelp_ctx", 0 },
    {(char*) "xhelp_activate_exit",(caddr_t)CoXHelpMotif::activate_exit },
    {(char*) "xhelp_activate_zoom_in",(caddr_t)CoXHelpMotif::activate_zoom_in },
    {(char*) "xhelp_activate_zoom_out",(caddr_t)CoXHelpMotif::activate_zoom_out },
    {(char*) "xhelp_activate_zoom_reset",(caddr_t)CoXHelpMotif::activate_zoom_reset },
    {(char*) "xhelp_activate_search",(caddr_t)CoXHelpMotif::activate_search },
    {(char*) "xhelp_activate_searchnext",(caddr_t)CoXHelpMotif::activate_searchnext },
    {(char*) "xhelp_activate_searchprevious",(caddr_t)CoXHelpMotif::activate_searchprevious },
    {(char*) "xhelp_activate_india_ok",(caddr_t)CoXHelpMotif::activate_india_ok },
    {(char*) "xhelp_activate_india_cancel",(caddr_t)CoXHelpMotif::activate_india_cancel },
    {(char*) "xhelp_create_india_label",(caddr_t)CoXHelpMotif::create_india_label },
    {(char*) "xhelp_create_india_text",(caddr_t)CoXHelpMotif::create_india_text },
    {(char*) "xhelp_activate_help",(caddr_t)CoXHelpMotif::activate_help },
    {(char*) "xhelp_create_xhelpnav_form",(caddr_t)CoXHelpMotif::create_xhelpnav_form }
	};

  static int	reglist_num = (sizeof reglist / sizeof reglist[0]);

  Lng::get_uid( uid_filename, uid_filename);

  // Motif
  MrmInitialize();

  strcpy( title, Lng::translate("Help"));
  reglist[0].value = (caddr_t) this;

  // Save the context structure in the widget
  i = 0;
  XtSetArg (args[i], XmNuserData, (XtPointer) this);i++;
  XtSetArg( args[i], XmNdeleteResponse, XmDO_NOTHING);i++;

  sts = MrmOpenHierarchy( 1, &uid_filename_p, NULL, &s_DRMh);
  if (sts != MrmSUCCESS) printf("can't open %s\n", uid_filename);

  MrmRegisterNames(reglist, reglist_num);

  parent_wid = XtCreatePopupShell( title, 
                                   topLevelShellWidgetClass, parent_wid, args, i);

  sts = MrmFetchWidgetOverride( s_DRMh, (char*) "xhelp_window", parent_wid,
                                name, args, 1, &toplevel, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch %s\n", name);

  sts = MrmFetchWidget(s_DRMh, (char*) "input_dialog", toplevel,
                       &india_widget, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch input dialog\n");

  MrmCloseHierarchy(s_DRMh);


  if (compiled_translations == NULL) 
    XtAppAddActions( XtWidgetToApplicationContext(toplevel), 
                     actions, XtNumber(actions));
 
  if (compiled_translations == NULL) 
    compiled_translations = XtParseTranslationTable(translations);
  XtOverrideTranslations( toplevel, compiled_translations);

  i = 0;
  XtSetArg(args[i],XmNwidth,500);i++;
  XtSetArg(args[i],XmNheight,700);i++;
  XtSetValues( toplevel ,args,i);
    
  XtManageChild( toplevel);

  xhelpnav = new CoXHelpNavMotif( (void *)this, xhelpnav_form, title, utility, &brow_widget, 
                             &sts);
  xhelpnav->open_URL_cb = CoXHelp::open_URL;

  // XtPopup( parent_wid, XtGrabNone);
  // displayed = 1;
  XtRealizeWidget( parent_wid);

  // Connect the window manager close-button to exit
  flow_AddCloseVMProtocolCb( parent_wid,
                             (XtCallbackProc)CoXHelpMotif::activate_exit, this);

  *xa_sts = 1;
}






