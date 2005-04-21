/* ge.c -- graphical editor

   PROVIEW/R
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#include <stdio.h>
#include <stdlib.h>

#include "pwr.h"

#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef void *ldh_tSesContext;

#include "ge.h"
#include "co_lng.h"
#include "co_xhelp.h"


/*  Fallback resources  */

static String	fbr[] = {
    NULL  
    };

static  Widget toplevel;
static  Widget mainwindow;

static void usage()
{
  printf("\nUsage: wb_ge [-l language] [graphname]\n");
}

#if 0
static void ge_close_cb( void *gectx)
{
  ge_del( gectx);
  exit(0);
}
#endif

int main( int argc, char *argv[])
{
  char		uid_filename[120] = {"pwr_exe:wb.uid"};
  char		*uid_filename_p = uid_filename;
  unsigned long sts;
  char 		title[80];
  XtAppContext  app_ctx;
  MrmHierarchy 	s_DRMh;
  MrmType 	dclass;
  char		name[] = "Proview/R Ge";
  char		graph_name[80];
  char		file[120];
  int		i;
  
#if !defined OS_VMS
  {
    char *s;

    if ( (s = getenv( "pwr_exe")) == NULL)
    {
      printf( "** pwr_exe is not defined\n");
      exit(0);
    }
    sprintf( uid_filename, "%s/wb.uid", s);
  }
#endif

  strcpy( title, "PROVIEW/R Ge ");

  MrmInitialize();

  toplevel = XtVaAppInitialize (
		      &app_ctx, 
		      "mainwindow",
		      NULL, 0, 
		      &argc, argv, 
		      fbr, 
		      XtNallowShellResize,  True,
		      XtNtitle, title,
		      XmNmappedWhenManaged, False,
		      NULL);
    

  sts = MrmOpenHierarchy( 1, &uid_filename_p, NULL, &s_DRMh);
  if (sts != MrmSUCCESS) printf("can't open %s\n", uid_filename);

  sts = MrmFetchWidgetOverride( s_DRMh, "mainwindow", toplevel,
			name, NULL, 0, &mainwindow, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch %s\n", name);

  MrmCloseHierarchy(s_DRMh);

  XtManageChild(mainwindow);

  // Create help window
  CoXHelp *xhelp = new CoXHelp( mainwindow, 0, xhelp_eUtility_Wtt, (int *)&sts);
  CoXHelp::set_default( xhelp);

  printf( "Argc: %d\n", argc);
  if ( argc > 1) {

    for ( i = 1; i < argc; i++) {
      if ( strcmp( argv[i], "-l") == 0) {
	if ( i+1 >= argc) {
	  usage();
	  exit(0);
	}
	Lng::set( argv[i+1]);
	i++;
      }
      else
	strcpy( file, argv[i]);
    }

    printf( "Arg: %s\n", file);
    if ( file[0] == '@') {
      // Execute script
      void *gectx;
      pwr_tStatus sts;

      gectx = ge_new( NULL, mainwindow, 0, 1, NULL);
      sts = ge_command( gectx, file);
      if ( EVEN(sts))
	ge_message( gectx, sts);
    }
    else {
      

      // Open graph
      strcpy( graph_name, file);
      ge_new( NULL, mainwindow, 0, 1, graph_name);
    }
  }
  else
    ge_new( NULL, mainwindow, 0, 1, NULL);

  XtRealizeWidget(toplevel);

  XtAppMainLoop(app_ctx);
  return (0);
}
