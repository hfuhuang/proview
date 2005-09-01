/* 
 * Proview   $Id: wb_adm.cpp,v 1.2 2005-09-01 14:57:48 claes Exp $
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

#include "flow_std.h"

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

extern "C" {
#include "pwr.h"
#include "pwr_privilege.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "wb_login.h"
}

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget.h"
#include "wb_admin.h"



/*  Fallback resources  */

static String	fbr[] = {
    NULL  
    };

static  Widget toplevel;
static  Widget mainwindow;

void adm_close_cb( void *ctx)
{
  exit(0);
}

int main( int argc, char *argv[])
{
  char		uid_filename[120] = {"pwr_exe:wb.uid"};
  char		*uid_filename_p = uid_filename;
  MrmHierarchy 	s_DRMh;
  MrmType 	dclass;
  pwr_tStatus 	sts;
  XtAppContext  app_ctx;
  char		title[80];
  Admin		*admin;
  
  dcli_translate_filename( uid_filename, uid_filename);

  // First argument is username, and second password
  strcpy( login_prv.username, "-");
  strcpy( login_prv.password, "-");
  if ( argc > 0)
    cdh_ToLower( login_prv.username, argv[1]);
  if ( argc > 1)
    cdh_ToLower( login_prv.password, argv[2]); 

  MrmInitialize();

  strcpy( title, "PwR Administrator");

  toplevel = XtVaAppInitialize (
		      &app_ctx, 
		      "PWR_ADM",
		      NULL, 0, 
		      &argc, argv, 
		      fbr, 
		      XtNallowShellResize,  True,
		      XtNtitle, title,
		      XmNmappedWhenManaged, False,
		      NULL);
    

  sts = MrmOpenHierarchy( 1, &uid_filename_p, NULL, &s_DRMh);
  if (sts != MrmSUCCESS) printf("can't open %s\n", uid_filename);

  sts = MrmFetchWidget(s_DRMh, "mainwindow", toplevel,
		&mainwindow, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch mainwindow\n");

  MrmCloseHierarchy( s_DRMh);

  XtManageChild( mainwindow);

  admin = new Admin( toplevel, 0);
  admin->close_cb = &adm_close_cb;

  XtRealizeWidget( toplevel);

  XtAppMainLoop(app_ctx);
  return (0);
}

