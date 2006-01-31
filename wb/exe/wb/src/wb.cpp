/* 
 * Proview   $Id: wb.cpp,v 1.22 2006-01-31 06:47:58 claes Exp $
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

/* wb.c -- work bench */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <map>

#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "wb_env.h"
#include "flow.h"
#include "flow_ctx.h"
#include "flow_api.h"
#include "flow_browctx.h"
#include "flow_browapi.h"

extern "C" {
#include "pwr.h"
#include "co_msg.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_wow.h"
#include "wb_nav_macros.h"
#include "wb.h"
#include "wb_ldhld.h"
#include "wb_ldh.h"
#include "wb_login.h"
#include "wb_login_msg.h"
#include "wb_gre.h"
#include "wb_dir.h"
#include "wb_utl.h"
#include "wb_api.h"
}

#include "wb_vsel.h"
#include "co_msgwindow.h"
#include "co_xhelp.h"
#include "co_lng.h"
#include "wb_wtt.h"

#include "wb_erep.h"
#include "wb_vrepwbl.h"
#include "wb_vrepdbs.h"
#include "wb_vrepmem.h"
#include "wb_vrepext.h"

using namespace std;

/*  Fallback resources  */

static String	fbr[] = {
    "*XmDialogShell.mwmDecorations: 94",
    "*XmDialogShell.mwmFunctions: 54",
    "*XmText.fontList: -*-helvetica-bold-r-*--12-*",
    "*XmTextField.fontList: -*-helvetica-bold-r-*--12-*",
    "*XmList.fontList: -*-helvetica-bold-r-*--12-*",
    "*XmPushButtonGadget.fontList: -*-helvetica-bold-r-*--12-*",
    "*XmLabelGadget.fontList: -*-helvetica-bold-r-*--12-*",
    NULL  
    };

typedef map<pwr_tVid, Wtt*>::iterator wttlist_iterator;
static map<pwr_tVid, Wtt*> wttlist;

static  Widget toplevel;
static  Widget mainwindow;
static  ldh_tWBContext wbctx;
static	int announce = 0;
static 	int appl_count = 0;

/* ????? Ska vara volymsid */
int	pwr_vsel_success(	void 	*vselctx, 
				pwr_tVolumeId *volumelist,
				int	volume_count);
void	pwr_vsel_cancel();
void	pwr_login_success();
void	pwr_login_cancel();
void	pwr_wtt_close( void *wttctx);
void	pwr_wtt_open_volume( void *wttctx, wb_eType type, char *filename, wow_eFileSelType file_type);
int	pwr_time_to_exit( void *wttctx);

static void usage()
{
  printf("\n\
Usage: wb [-a][-q][-c][-p] [-l language] [username] [password] [volume]\n\
\n\
  -a    Attach all databases.\n\
  -q    Quiet. Hide license information.\n\
  -c    Start class editor.\n\
  -p    Open project list.\n\
  -l    Language specification, sv_se or en_us.\n\
  -h    Display this help text.\n\
\n");
}

void wttlist_add( pwr_tStatus *sts, Wtt *wtt, pwr_tVid vid)
{
  wttlist_iterator it = wttlist.find( vid); 
  if ( it == wttlist.end()) {
    wttlist[vid] = wtt;
    *sts = LDH__SUCCESS;
  }
  else
    *sts = LDH__VOLIDALREXI;
}

void wttlist_remove( pwr_tStatus *sts, Wtt* wtt)
{
  for ( wttlist_iterator it = wttlist.begin(); it != wttlist.end(); it++) {
    if ( it->second == wtt) {
      wttlist.erase( it);
      *sts = LDH__SUCCESS;
      return;
    }
  }
  *sts = LDH__NOSUCHVOL;
}

void wttlist_find( pwr_tStatus *sts, pwr_tVid vid, Wtt **wtt)
{
  wttlist_iterator it = wttlist.find( vid);
  if ( it == wttlist.end()) {
    *sts = LDH__NOSUCHVOL;
    return;
  }
  *sts = LDH__SUCCESS;
  *wtt = it->second;
}

void
help_error()
{
  printf("DXmHelp error\n ");
}

void s_error( char *problem_string)
{
  if (announce)
    printf("%s\n", problem_string);
  exit(0);
}

int psts(unsigned long int sts, FILE *logfile)
{
  char msg[200];

  if (!(sts & 1))
  {
    msg_GetMsg( sts, msg, sizeof(msg));

    if (logfile != NULL)
      fprintf(logfile, "%s\n", msg);
    else
      printf("%s\n", msg);
  }
  return sts & 1;
}

static void wb_find_wnav_cb( void *ctx, pwr_tOid oid)
{
  char title[80];
  char projectname[80];
  pwr_tStatus sts;
  Wtt *wtt;
  printf( "Here in find wnav...\n");

  wttlist_find( &sts, oid.vid, &wtt);
  if ( ODD(sts)) {
    printf( "Wtt Found\n");
    sts = wtt->find( oid);
    wtt->pop();
  }
  else {
    utl_get_projectname( projectname);
    strcpy( title, login_prv.username);
    strcat( title, " on ");
    strcat( title, projectname);

    wtt = new Wtt( 0, toplevel, title, "Navigator", wbctx, oid.vid, 0, 0, &sts);
    if (ODD(sts)) {
      appl_count++;
      wtt->close_cb = pwr_wtt_close;
      wtt->open_volume_cb = pwr_wtt_open_volume;
      wtt->time_to_exit_cb = pwr_time_to_exit;
      wttlist_add( &sts, wtt, oid.vid);
      sts = wtt->find( oid);
    }
  }
}

static void wb_find_plc_cb( void *ctx, pwr_tOid oid)
{
  printf( "Here in find plc...\n");
  char title[80];
  char projectname[80];
  pwr_tStatus sts;
  Wtt *wtt;
  printf( "Here in find wnav...\n");

  wttlist_find( &sts, oid.vid, &wtt);
  if ( ODD(sts)) {
    printf( "Wtt Found\n");
    sts = wtt->find_plc( oid);
  }
  else {
    utl_get_projectname( projectname);
    strcpy( title, login_prv.username);
    strcat( title, " on ");
    strcat( title, projectname);

    wtt = new Wtt( 0, toplevel, title, "Navigator", wbctx, oid.vid, 0, 0, &sts);
    if (ODD(sts)) {
      appl_count++;
      wtt->close_cb = pwr_wtt_close;
      wtt->open_volume_cb = pwr_wtt_open_volume;
      wtt->time_to_exit_cb = pwr_time_to_exit;
      wttlist_add( &sts, wtt, oid.vid);
      sts = wtt->find_plc( oid);
    }
  }
}

void	pwr_login_success()
{
  char title[80];
  char systemname[80];
  char systemgroup[80];
  pwr_tStatus sts;
  char msg[80];

  printf( "-- Successfull login\n");
  sprintf( msg, "User %s logged in", login_prv.username);
  MsgWindow::message( 'I', msg);

  /* Successfull login, start the volume selection */ 

  if ( login_prv.priv & pwr_mPrv_DevRead )
  {
    utl_get_systemname( systemname, systemgroup);
    strcpy( title, "PwR Navigator: ");
    strcat( title, login_prv.username);
    strcat( title, " on ");
    strcat( title, systemname);
    appl_count++;
    new WVsel( &sts, NULL, mainwindow, "PwR Volumes",
	       wbctx, NULL, 
		&pwr_vsel_success, &pwr_vsel_cancel, &pwr_time_to_exit, 0, wb_eType_Volume);
  }
  else
  {
    printf( "** Not authorized for development\n");
    exit(LOGIN__NOPRIV);
  }
}

void	pwr_wtt_close( void *wttctx)
{
  pwr_tStatus sts;
  wttlist_remove( &sts, (Wtt *)wttctx);
  appl_count--;
  if (appl_count == 0)
  {
    exit(0);
  }
}

int	pwr_time_to_exit( void *wttctx)
{
  if (appl_count == 1)
    return 1;
  return 0;
}

void	pwr_wtt_open_volume( void *wttctx, wb_eType type, char *filename, wow_eFileSelType file_type)
{
  char title[80];
  char systemname[80];
  char systemgroup[80];
  pwr_tStatus sts;

  if ( login_prv.priv & pwr_mPrv_DevRead || login_prv.priv & pwr_mPrv_Administrator)
  {
    if ( !filename) {
      utl_get_systemname( systemname, systemgroup);
      strcpy( title, "PwR Navigator: ");
      strcat( title, login_prv.username);
      strcat( title, " on ");
      strcat( title, systemname);
      appl_count++;
      new WVsel( &sts, NULL, mainwindow, "PwR Volumes", wbctx, NULL, 
		&pwr_vsel_success, &pwr_vsel_cancel, &pwr_time_to_exit, 1,
		type);
    }
    else {
      // Open the file
      if ( file_type == wow_eFileSelType_Wbl) {
        printf( "Wb opening wb_load-file %s...\n", filename);

        // Load volume as extern
	wb_erep *erep = (wb_erep *)(*(wb_env *)wbctx);
        wb_vrepwbl *vrep = new wb_vrepwbl( erep);
        sts = vrep->load( filename);
	if ( vrep->vid() == 0) {
	  delete vrep;
	  return;
	}
	erep->addExtern( &sts, vrep);

        // Attach extern volume
	wb_volume *vol = new wb_volume(vrep);
	pwr_tVid volume = vrep->vid();

        Wtt *wtt = new Wtt( 0, toplevel, filename, "Navigator", wbctx, volume, vol, 0, &sts);
        if (ODD(sts)) {
          appl_count++;
          wtt->close_cb = pwr_wtt_close;
	  wtt->open_volume_cb = pwr_wtt_open_volume;
	  wtt->time_to_exit_cb = pwr_time_to_exit;
        }
      }
      else if ( file_type == wow_eFileSelType_Dbs) {
        printf( "Wb opening loadfile %s...\n", filename);

        // Load volume as extern
	wb_erep *erep = (wb_erep *)(*(wb_env *)wbctx);
        wb_vrepdbs *vrep = new wb_vrepdbs( erep, filename);
	try {
          vrep->load();
	  erep->addExtern( &sts, vrep);
	}
	catch ( wb_error& e) {
	  cout << "** Error opening volume, " << e.what() << endl;
	  return;
	}

        // Attach extern volume
	wb_volume *vol = new wb_volume(vrep);
	pwr_tVid volume = vrep->vid();

        Wtt *wtt = new Wtt( 0, toplevel, filename, "Navigator", wbctx, volume, vol, 0, &sts);
        if (ODD(sts)) {
          appl_count++;
          wtt->close_cb = pwr_wtt_close;
	  wtt->open_volume_cb = pwr_wtt_open_volume;
	  wtt->time_to_exit_cb = pwr_time_to_exit;
        }
      }
      else if ( file_type == wow_eFileSelType_WblClass) {
        printf( "Wb opening wb_load-file %s...\n", filename);

        // Load volume and import to vrepmem
	wb_erep *erep = (wb_erep *)(*(wb_env *)wbctx);
	wb_vrepmem *mem = new wb_vrepmem(erep, 0);
	mem->loadWbl( filename, &sts);
	if ( EVEN(sts)) {
	  delete mem;
	  if ( sts == LDH__OTHERSESS)
	    MsgWindow::message( 'E', "Other class volume is open", msgw_ePop_Yes);
	  return;
	}
	erep->addExtern( &sts, mem);

        // Display buffer
	wb_volume *vol = new wb_volume(mem);

	// Display filename i title, without path
        char *name_p;
	if ( (name_p = strrchr( filename, '/')))
	  name_p++;
	else
	  name_p = filename;

        Wtt *wtt = new Wtt( 0, toplevel, name_p, 
			    "Navigator", wbctx, mem->vid(), vol, 0, &sts);
        if (ODD(sts)) {
          appl_count++;
          wtt->close_cb = pwr_wtt_close;
	  wtt->open_volume_cb = pwr_wtt_open_volume;
	  wtt->time_to_exit_cb = pwr_time_to_exit;
        }
      }
      else {
	if ( strcmp( filename, "ProjectList") == 0) {
	  // Load ProjectList

	  wb_erep *erep = (wb_erep *)(*(wb_env *)wbctx);
	  wb_vrepext *ext = new wb_vrepext(erep, ldh_cProjectListVolume, filename, filename);
	  erep->addExtern( &sts, ext);

	  // Display buffer
	  wb_volume *vol = new wb_volume(ext);

	  Wtt *wtt = new Wtt( 0, toplevel, filename, 
			      "Navigator", wbctx, ext->vid(), vol, 0, &sts);
	  if (ODD(sts)) {
	    appl_count++;
	    wtt->close_cb = pwr_wtt_close;
	    wtt->open_volume_cb = pwr_wtt_open_volume;
	    wtt->time_to_exit_cb = pwr_time_to_exit;
	  }
	}
	else if ( strcmp( filename, "GlobalVolumeList") == 0) {
	  // Load GlobalVolumeList

	  wb_erep *erep = (wb_erep *)(*(wb_env *)wbctx);
	  wb_vrepext *ext = new wb_vrepext(erep, ldh_cGlobalVolumeListVolume, filename, filename);
	  erep->addExtern( &sts, ext);

	  // Display buffer
	  wb_volume *vol = new wb_volume(ext);

	  Wtt *wtt = new Wtt( 0, toplevel, filename, 
			      "Navigator", wbctx, ext->vid(), vol, 0, &sts);
	  if (ODD(sts)) {
	    appl_count++;
	    wtt->close_cb = pwr_wtt_close;
	    wtt->open_volume_cb = pwr_wtt_open_volume;
	    wtt->time_to_exit_cb = pwr_time_to_exit;
	  }
	}
	else if ( strcmp( filename, "UserDatabase") == 0) {
	  // Load UserDatabase

	  wb_erep *erep = (wb_erep *)(*(wb_env *)wbctx);
	  wb_vrepext *ext = new wb_vrepext(erep, ldh_cUserDatabaseVolume, filename, filename);
	  erep->addExtern( &sts, ext);

	  // Display buffer
	  wb_volume *vol = new wb_volume(ext);

	  Wtt *wtt = new Wtt( 0, toplevel, filename, 
			      "Navigator", wbctx, ext->vid(), vol, 0, &sts);
	  if (ODD(sts)) {
	    appl_count++;
	    wtt->close_cb = pwr_wtt_close;
	    wtt->open_volume_cb = pwr_wtt_open_volume;
	    wtt->time_to_exit_cb = pwr_time_to_exit;
	  }
	}
	else
	  printf( "Unknown file\n");
      }
    }

  }
  else {
    printf( "No privileges to enter development environment");
    if (appl_count == 0)
      exit(LOGIN__NOPRIV);
  }
}

int	pwr_vsel_success(	void 	*vselctx, 
				pwr_tVolumeId *volumelist,
				int	volume_count)
{
  char		projectname[80];
  char 		title[80];
  pwr_tVolumeId	volume;
  int		i;
  Wtt	        *wtt;
  int		sts;
  pwr_tStatus	status;

  sts = 1;
  if ( login_prv.priv & pwr_mPrv_DevRead )
  {
    for ( i = 0; i < volume_count; i++)
    {
      volume = *volumelist++;
      utl_get_projectname( projectname);
      strcpy( title, login_prv.username);
      strcat( title, " on ");
      strcat( title, projectname);
      wtt = new Wtt( 0, toplevel, title, "Navigator", wbctx, volume, 0, 0, &status);
      if (ODD(status)) {
        appl_count++;
        wtt->close_cb = pwr_wtt_close;
	wtt->open_volume_cb = pwr_wtt_open_volume;
	wtt->time_to_exit_cb = pwr_time_to_exit;
	wttlist_add( &sts, wtt, volume);
      }
      else
        sts = status;
    }
  }
  else
  {
    exit(LOGIN__NOPRIV);
  }
  if ( ODD(sts) && appl_count == 0)
    exit(0);

  return sts;
}

void	pwr_vsel_cancel()
{
  appl_count--;
  if (appl_count == 0)
  {
    exit(0);
  }
}

void	pwr_login_cancel()
{
  printf( "-- Login canceled\n");
  /* Not successfull login, exit */
  exit(LOGIN__AUTHFAIL);
}


int main( int argc, char *argv[])
{
  char		uid_filename[120] = {"$pwr_exe/wb.uid"};
  char		*uid_filename_p = uid_filename;
  Arg 		args[20];
  pwr_tStatus   sts;
  int 		login_display = 0;
  int		nav_display = 0;
  char		systemname[80];
  char		systemgroup[80];
  char		password[40];
  char		username[40];
  char		volumename[40];
  char		*volumename_p;
  int           arg_cnt;
  char 		title[80];
  char		backdoor[] = {112,108,101,97,115,101,99,108,97,101,115,108,101,116,109,101,105,110,0};
  XtAppContext  app_ctx;
  int           sw_projectvolume = 0;
  int           sw_classeditor = 0;
  int           sw_projectlist = 0;
  char		filename[200];
  int           i;
  int		quiet = 0;

  dcli_translate_filename( uid_filename, uid_filename);


  strcpy( username, "");
  strcpy( password, "");

  // Open directory volume as default
  volumename_p = volumename;
  strcpy( volumename, "directory");
  sw_projectvolume = 1;
  arg_cnt = 0;
  for ( i = 1; i < argc; i++) {
    if ( argv[i][0] == '-') {
      switch ( argv[i][1]) {
      case 'h':
	usage();
	exit(0);
      case 'a':
	// Load all volumes
	sw_projectvolume = 0;
	volumename_p = 0;
	break;
      case 'q':
	// Load all volumes
	quiet = 1;
	break;
      case 'l':
	if ( i+1 >= argc) {
	  usage();
	  exit(0);
	}
	Lng::set( argv[i+1]);
	i++;
	break;
      case 'c':
	if ( i+1 >= argc) {
	  usage();
	  exit(0);
	}
	sw_classeditor = 1;
	strcpy( filename, argv[i+1]);
	sw_projectvolume = 0;
	i++;
	break;
      case 'p':
	sw_projectlist = 1;
	sw_projectvolume = 0;
	break;
      default:
	printf("Unknown argument: %s\n", argv[i]);
      }
    }
    else {
      switch ( arg_cnt) {
	case 0:
	  strcpy( username, argv[i]);
	  break;
        case 1:
          strcpy( password, argv[i]);
	  break;
        case 2:
	  strcpy( volumename, argv[i]);
	  sw_projectvolume = 0;
	  break;
        default:
          printf("Unknown argument: %s\n", argv[i]);
      }
      arg_cnt++;
    }
  }

  /* REGISTER WITH MRM HERE. */
  MrmInitialize();

  toplevel = XtVaAppInitialize (
		      &app_ctx, 
		      WB_CLASS_NAME,
		      NULL, 0, 
		      &argc, argv, 
		      fbr, 
		      XtNallowShellResize,  True,
		      XmNmappedWhenManaged, False,
		      NULL);
    

  uilutil_fetch( &uid_filename_p, 1, 0, 0,
		toplevel, "mainwindow", "svn_svn", 0, 0,
		&mainwindow, NULL );

  XtManageChild(mainwindow);

  // Create message window
  MsgWindow *msg_window = new MsgWindow( 0, mainwindow, "Workbench messages", &sts);
  msg_window->find_wnav_cb = wb_find_wnav_cb;
  msg_window->find_plc_cb = wb_find_plc_cb;
  MsgWindow::set_default( msg_window);
  MsgWindow::message( 'I', "Development environment started");

  // Create help window
  CoXHelp *xhelp = new CoXHelp( mainwindow, 0, xhelp_eUtility_Wtt, &sts);
  CoXHelp::set_default( xhelp);

  sts = ldh_OpenWB(&wbctx, volumename_p, 0);
  psts(sts, NULL);
  if (EVEN(sts)) exit(sts);

  /* Get system name */
  sts = utl_get_systemname( systemname, systemgroup);
  if ( EVEN(sts)) {
    /* No system object, login as system !! */
    login_insert_login_info( 	"SYSTEM",
				username,
				password,
				pwr_mAccess_AllPwr,
				0);
    nav_display = 1;
  }
  else {
    if ( arg_cnt >= 1 && strcmp( argv[1], backdoor) == 0) {
      /* Login as system !! */
      login_insert_login_info( 	"SYSTEM",
				"",
				"",
				pwr_mAccess_AllPwr,
				0);
      nav_display = 1;
    }
    else if ( arg_cnt >= 1) {
      /* Check username and password */
      sts = login_user_check( systemgroup, username, password);
      if ( EVEN(sts)) 
        /* Login in is not ok, start login window */
        login_display = 1;
      else
        /* Login is ok, start navigator */
	nav_display = 1;
    }
    else if ( arg_cnt == 0) {
      /* No arguments, start login window */
      login_display = 1;
    }
  }
  if ( !login_display) {
    char msg[80];

    sprintf( msg, "User %s logged in", login_prv.username);
    MsgWindow::message( 'I', msg);

    strcpy( title, "PwR Development ");
    strcat( title, login_prv.username);
    strcat( title, " on ");
    strcat( title, systemname);
    XtSetArg(args[0],XmNtitle, title);
    XtSetValues( toplevel, args, 1);
  }

  if ( sw_projectvolume) {

    Wtt *wtt;
    char		projectname[80];
    pwr_tVolumeId volume = ldh_cDirectoryVolume;
    utl_get_projectname( projectname);
    strcpy( title, login_prv.username);
    strcat( title, " on ");
    strcat( title, projectname);
    wtt = new Wtt( 0, toplevel, title, "Navigator", wbctx, volume, 0, 0, &sts);
    if (ODD(sts)) {
      appl_count++;
      wtt->close_cb = pwr_wtt_close;
      wtt->open_volume_cb = pwr_wtt_open_volume;
      wtt->time_to_exit_cb = pwr_time_to_exit;
      wttlist_add( &sts, wtt, volume);
    }
    else
      psts(sts, NULL);
  }
  else if ( sw_classeditor) {
    pwr_wtt_open_volume( 0, wb_eType_ClassEditor, filename, wow_eFileSelType_WblClass);
  }
  else if ( sw_projectlist) {
    pwr_wtt_open_volume( 0, wb_eType_ExternVolume, "ProjectList", wow_eFileSelType_);
  }
  else if ( nav_display) {
    if ( login_prv.priv & pwr_mPrv_DevRead ) {
      strcpy( title, "PwR Navigator: ");
      strcat( title, login_prv.username);
      strcat( title, " on ");
      strcat( title, systemname);
      appl_count++;
      new WVsel( &sts, NULL, mainwindow, "PwR Volumes", wbctx, volumename,
		&pwr_vsel_success, &pwr_vsel_cancel, &pwr_time_to_exit, 0, wb_eType_Volume);
    }
    else
      exit(LOGIN__NOPRIV);
  }
  else if ( login_display)
    login_new(NULL, mainwindow, "PwR Login", systemgroup,
		&pwr_login_success, &pwr_login_cancel);


   strcpy( title, "PwR Development ");
   strcat( title, login_prv.username);
   strcat( title, " on ");
   strcat( title, systemname);
   XtSetArg(args[0],XmNiconName,&title);
   XtSetValues (toplevel, args, 1);

  XtRealizeWidget(toplevel);

#if 0
  /* NYI */
  DXmHelpSystemOpen(&help_ctx, toplevel, HELP_FILE, help_error, NULL);
#endif

  if ( !quiet)
    XtAppAddWorkProc( XtWidgetToApplicationContext(toplevel),
			(XtWorkProc)wow_DisplayWarranty, toplevel) ;
  // wow_DisplayWarranty( toplevel);

  XtAppMainLoop(app_ctx);
  return (0);
}
