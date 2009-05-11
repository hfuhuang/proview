/* 
 * Proview   $Id: wb_wnav_command.cpp,v 1.54 2008-10-31 12:51:32 claes Exp $
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

/* wb_wnav_command.cpp 
   This module contains routines for handling of command line in wnav. */


#include "flow_std.h"


# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <ctype.h>

#include "pwr_class.h"
#include "pwr_version.h"
#include "co_cdh.h"
#include "co_ccm.h"
#include "co_time.h"
#include "co_dcli.h"
#include "pwr_baseclasses.h"
#include "co_ccm_msg.h"
#include "co_regex.h"
#include "co_user.h"
#include "rt_load.h"
#include "wb_ldh.h"
#include "wb_ldhload.h"
#include "co_login.h"
#include "wb_utl_api.h"
#include "wb_genstruct.h"
#include "wb_lfu.h"
#include "wb_dir.h"
#include "wb_trv.h"
#include "wb_crrgen.h"

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"

#include "wb_wnav.h"
#include "wb_wnav_item.h"
#include "co_dcli_msg.h"
#include "wb_wnav_msg.h"
#include "wb_foe_msg.h"
#include "wb_wda.h"
#include "wb_wge.h"
#include "co_xhelp.h"
#include "wb_wccm.h"
#include "co_api_user.h"
#include "wb_env.h"
#include "wb_erep.h"
#include "wb_vrepwbl.h"
#include "wb_vrepmem.h"
#include "wb_vrepdb.h"
#include "wb_pkg.h"
#include "wb_build.h"
#include "wb_wtt.h"
#include "ge.h"
#include "wb_utl.h"

#define	WNAV_MENU_CREATE	0
#define	WNAV_MENU_ADD		1

static char wtt_version[] = pwrv_cPwrVersionStr;
static WNav *current_wnav[20];
static int wnav_cnt = 0;

static int wnav_wccm_get_ldhsession_cb( void *ctx, ldh_tWBContext *wbctx);
static int wnav_wccm_get_wbctx_cb( void *ctx, ldh_tSesContext *ldhses);
static char	*wnav_VolumeIdToStr( pwr_tVolumeId volumeid);

static int	wnav_help_func(		void		*client_data,
					void		*client_flag);
static int	wnav_define_func(	void		*client_data,
					void		*client_flag);
static int	wnav_logout_func(	void		*client_data,
					void		*client_flag);
static int	wnav_login_func(	void		*client_data,
					void		*client_flag);
static int	wnav_two_func(		void		*client_data,
					void		*client_flag);
static int	wnav_one_func(		void		*client_data,
					void		*client_flag);
static int	wnav_edit_func(		void		*client_data,
					void		*client_flag);
static int	wnav_noedit_func(	void		*client_data,
					void		*client_flag);
static int	wnav_show_func(		void		*client_data,
					void		*client_flag);
static int	wnav_compile_func(	void		*client_data,
					void		*client_flag);
static int	wnav_print_func(	void		*client_data,
					void		*client_flag);
static int	wnav_redraw_func(	void		*client_data,
					void		*client_flag);
static int	wnav_list_func(		void		*client_data,
					void		*client_flag);
static int	wnav_configure_func(	void		*client_data,
					void		*client_flag);
static int	wnav_sort_func(		void		*client_data,
					void		*client_flag);
static int	wnav_copy_func(		void		*client_data,
					void		*client_flag);
static int	wnav_cut_func(		void		*client_data,
					void		*client_flag);
static int	wnav_paste_func(	void		*client_data,
					void		*client_flag);
static int	wnav_move_func(		void		*client_data,
					void		*client_flag);
static int	wnav_open_func(		void		*client_data,
					void		*client_flag);
static int	wnav_close_func(	void		*client_data,
					void		*client_flag);
static int	wnav_create_func( 	void		*client_data,
					void		*client_flag);
static int	wnav_new_func( 		void		*client_data,
					void		*client_flag);
static int	wnav_delete_func( 	void		*client_data,
					void		*client_flag);
static int	wnav_connect_func( 	void		*client_data,
					void		*client_flag);
static int	wnav_disconnect_func( 	void		*client_data,
					void		*client_flag);
static int	wnav_wb_func( 		void		*client_data,
					void		*client_flag);
static int	wnav_exit_func(		void		*client_data,
					void		*client_flag);
static int	wnav_setup_func(	void		*client_data,
					void		*client_flag);
static int	wnav_set_func(		void		*client_data,
					void		*client_flag);
static int	wnav_search_func(	void		*client_data,
					void		*client_flag);
static int	wnav_save_func(		void		*client_data,
					void		*client_flag);
static int	wnav_revert_func(	void		*client_data,
					void		*client_flag);
static int	wnav_display_func(	void		*client_data,
					void		*client_flag);
static int	wnav_generate_func(     void		*client_data,
				        void		*client_flag);
static int	wnav_crossref_func(     void		*client_data,
				        void		*client_flag);
static int	wnav_distribute_func(   void		*client_data,
				        void		*client_flag);
static int	wnav_release_func(	void		*client_data,
					void		*client_flag);
static int	wnav_build_func(	void		*client_data,
					void		*client_flag);
static int	wnav_check_func(	void		*client_data,
					void		*client_flag);
static int	wnav_update_func(	void		*client_data,
					void		*client_flag);

dcli_tCmdTable	wnav_command_table[] = {
		{
			"SHOW",
			&wnav_show_func,
			{ "dcli_arg1", "dcli_arg2", "/NAME", "/CLASS", 
			"/HIERARCHY", "/PARAMETER" , "/OBJID", 
			"/FILE", "/LOCAL", "/INITSTEP", 
			"/MAXOBJECTS", "/VOLUME", "/ALL", "/TYPE", 
			"/OPTION", "/TERMINAL", "/OUTPUT", 
			"/FULL", "/APPEND", "/EXACTORDER", ""}
		},
		{
			"COMPILE",
			&wnav_compile_func,
			{ "/MODIFIED", "/DEBUG", 
			"/HIERARCHY", "/PLCPGM" , "/WINDOW", 
			"/FROM_PLCPGM", "/ALLVOLUMES", "/VOLUMES", 
			""}
		},
		{
			"PRINT",
			&wnav_print_func,
			{ "/ALL", "/NODOCUMENT", "/NOOVERVIEW", 
			"/HIERARCHY", "/PLCPGM", "/FROM_PLCPGM", "/PDF",
			""}
		},
		{
			"REDRAW",
			&wnav_redraw_func,
			{ "/ALL",
			"/HIERARCHY", "/PLCPGM", "/FROM_PLCPGM", "/TEMPLATE",
			""}
		},
		{
			"LIST",
			&wnav_list_func,
			{ "dcli_arg1", "/OUTPUT",
			"/HIERARCHY", "/OBJECT", "/ALLVOLUMES", "/VOLUMES",
			"/DESCRIPTOR", "/SHORTNAME", "/NODE", "/PLCPGM",
			""}
		},
		{
			"CONFIGURE",
			&wnav_configure_func,
			{ "dcli_arg1", "/RACK", "/CARDNAME", "/CARDCLASS",
			"/CHANNELNAME", "/CHANIDENTITY", 
			"/CHANDESCRIPTION", "/TABLE",
			""}
		},
		{
			"SORT",
			&wnav_sort_func,
			{ "/SIGNALS", "/CLASS", "/PARENT",
			""}
		},
		{
			"COPY",
			&wnav_copy_func,
			{ "dcli_arg1", "/FIRST", "/LAST", "/AFTER", "/BEFORE",
			"/HIERARCHY", "/SOURCE", "/DESTINATION", "/NAME",
			"/KEEPREFERENCES", "/IGNORE_ERRORS", ""}
		},
		{
			"CUT",
			&wnav_cut_func,
			{ "/KEEPREFERENCES", ""}
		},
		{
			"PASTE",
			&wnav_paste_func,
			{ "dcli_arg1", "/INTO", "/BUFFER", "/KEEPOID", 
			  "/TOPLEVEL", ""}
		},
		{
			"MOVE",
			&wnav_move_func,
			{ "dcli_arg1", "/FIRST", "/LAST", "/AFTER", "/BEFORE",
			"/SOURCE", "/DESTINATION", "/RENAME",
			""}
		},
		{
			"OPEN",
			&wnav_open_func,
			{ "dcli_arg1", "dcli_arg2", "/NAME", "/FILE", 
			"/SCROLLBAR", "/WIDTH", "/HEIGHT", "/MENU", 
			"/NAVIGATOR", "/CENTER", "/OBJECT", "/MODAL", "/INSTANCE", "/TITLE", 
			"/VOLUME", ""}
		},
		{
			"CLOSE",
			&wnav_close_func,
			{ "dcli_arg1", "dcli_arg2", "/FILE", "/INSTANCE", ""}
		},
		{
			"CREATE",
			&wnav_create_func,
			{ "dcli_arg1", "dcli_arg2", "/TEXT", "/MENU", 
                        "/DESTINATION",
			"/COMMAND", "/AFTER", "/BEFORE", "/FIRSTCHILD", 
			"/LASTCHILD", "/VOLUME", "/ALL", 
			"/CLASS", "/DEBUG", "/NODECONFIG",
			"/NAME", "/IDENTITY", "/FILES", "/OUT", "/IGNORE",
			"/DIRECTORY", "/BUILDVERSION", "/DATABASE", "/SERVER", 
			  "/PLCPGM", "/HIERARCHY", "/FROM_PLCPGM", "/TEMPLATE", ""}
		},
		{
			"NEW",
			&wnav_new_func,
			{ "dcli_arg1", "/NAME",  ""}
		},
		{
			"DELETE",
			&wnav_delete_func,
			{ "dcli_arg1", "/NAME", "/CLASS", "/HIERARCHY",
			"/NOCONFIRM", "/NOLOG", ""}
		},
		{
			"CONNECT",
			&wnav_connect_func,
			{ "/SOURCE", "/DESTINATION", "/RECONNECT",
			""}
		},
		{
			"DISCONNECT",
			&wnav_disconnect_func,
			{ "/SOURCE",
			""}
		},
		{
			"WB",
			&wnav_wb_func,
			{ "dcli_arg1", "/OUTPUT", "/HIERARCHY", "/LOADFILE", "/NOINDEX",
			  "/KEEPNAME", "/NOFOCODE",
			""}
		},
		{
			"EXIT",
			&wnav_exit_func,
			{ ""}
		},
		{
			"QUIT",
			&wnav_exit_func,
			{ ""}
		},
		{
			"DEFINE",
			&wnav_define_func,
			{ "dcli_arg1", "dcli_arg2", "dcli_arg3", "dcli_arg4", ""}
		},
		{
			"HELP",
			&wnav_help_func,
			{ "dcli_arg1", "dcli_arg2", "dcli_arg3", "dcli_arg4",
			"/HELPFILE", "/POPNAVIGATOR", "/BOOKMARK", 
			"/INDEX", "/BASE", "/PROJECT", "/STRICT", "/VERSION", ""}
		},
		{
			"LOGOUT",
			&wnav_logout_func,
			{ ""}
		},
		{
			"LOGIN",
			&wnav_login_func,
			{ "dcli_arg1", "dcli_arg2", "/ADMINISTRATOR", ""}
		},
		{
			"TWO",
			&wnav_two_func,
			{ ""}
		},
		{
			"ONE",
			&wnav_one_func,
			{ ""}
		},
		{
			"EDIT",
			&wnav_edit_func,
			{ ""}
		},
		{
			"NOEDIT",
			&wnav_noedit_func,
			{ ""}
		},
		{
			"SET",
			&wnav_set_func,
			{ "dcli_arg1", "dcli_arg2", "/LOCAL", 
			"/WIDTH", "/HEIGHT", 
			"/TERMINAL", "/OUTPUT", "/NOCONFIRM", "/LOG",
			"/CLASS", "/HIERARCHY", "/NAME", "/ATTRIBUTE", 
			"/SIGNALOBJECTSEG", "/SIGCHANCONSEG", 
			"/SHOSIGCHANCON", "/SHODETECTTEXT", "/VOLUMENAME",
			"/VALUE", "/SOURCE", "/MODAL", ""}
		},
		{
			"SETUP",
			&wnav_setup_func,
			{"",}
		},
		{
			"SEARCH",
			&wnav_search_func,
			{ "dcli_arg1", "/REGULAREXPRESSION", "/NEXT", ""}
		},
		{
			"SAVE",
			&wnav_save_func,
			{ "/QUIET", ""}
		},
		{
			"REVERT",
			&wnav_revert_func,
			{ "/NOCONFIRM", "/CONFIRM", ""}
		},
		{
			"DISPLAY",
			&wnav_display_func,
			{ "dcli_arg1", "/NAME", "/CLASS", 
			"/ROOT", "/DEPTH", "/HIERARCHY", "/ATTRIBUTE", 
			""}
		},
		{
			"GENERATE",
			&wnav_generate_func,
			{ "dcli_arg1", ""}
		},
		{
			"CROSSREFERENCE",
			&wnav_crossref_func,
			{ "dcli_arg1", "/NAME", "/FILE", "/STRING", "/BRIEF",
			"/FUNCTION", "/CASE_SENSITIVE", ""}
		},
		{
			"DISTRIBUTE",
			&wnav_distribute_func,
			{ "/NODE", ""}
		},
		{
			"RELEASE",
			&wnav_release_func,
			{ "dcli_arg1", "dcli_arg2", ""}
		},
		{
			"BUILD",
			&wnav_build_func,
			{ "dcli_arg1", "dcli_arg2", "/FORCE", "/DEBUG", "/CROSSREFERENCE", 
			  "/MANUAL", "/NAME", ""}
		},
		{
			"CHECK",
			&wnav_check_func,
			{ "dcli_arg1", ""}
		},
		{
			"UPDATE",
			&wnav_update_func,
			{ "dcli_arg1", ""}
		},
		{"",}};


static void wnav_store_wnav( WNav *wnav)
{
  if ( wnav_cnt >= 20)
    return;
  current_wnav[wnav_cnt] = wnav;  
  wnav_cnt++;
}

static void wnav_free_stored_wnav()
{
  wnav_cnt--;
}

static void wnav_get_stored_wnav( WNav **wnav)
{
  *wnav = current_wnav[wnav_cnt -1];
}

static void wnav_wge_close_cb( void *ctx)
{
  WGe *wge = (WGe *)ctx;
  ((WNav *)wge->parent_ctx)->appl.remove( (void *)wge);
}

static int wnav_wge_command_cb( void *ctx, char *cmd)
{
  WGe *wge = (WGe *)ctx;
  return ((WNav *)wge->parent_ctx)->command( cmd);
}

static int	wnav_help_func(		void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  int	sts;
  char	arg_str[80];
  char	file_str[80];
  char	bookmark_str[80];
  char	key[80];
  int	pop;
  int   strict;
	

  if ( ODD( dcli_get_qualifier( "/INDEX", file_str, sizeof(file_str))))
  {
    if ( ODD( dcli_get_qualifier( "/HELPFILE", file_str, sizeof(file_str))))
    {
      sts = CoXHelp::dhelp_index( navh_eHelpFile_Other, file_str);
      if ( EVEN(sts))
        wnav->message('E', "Unable to find file");
    }
    else
    {
      if ( ODD( dcli_get_qualifier( "/BASE", file_str, sizeof(file_str))))
        sts = CoXHelp::dhelp_index( navh_eHelpFile_Base, NULL);
      else
        sts = CoXHelp::dhelp_index( navh_eHelpFile_Project, NULL);
    }
    return sts;
  }

  if ( ODD( dcli_get_qualifier( "/VERSION", 0, 0))) {
    sts = CoXHelp::dhelp( "version", "", navh_eHelpFile_Other, "$pwr_exe/wtt_version_help.dat", strict);
    if ( EVEN(sts))
      wnav->message('E', "No help on this subject");
    return sts;
  }
  if ( EVEN( dcli_get_qualifier( "dcli_arg1", arg_str, sizeof(arg_str))))
  {
    sts = CoXHelp::dhelp( "help command", "", navh_eHelpFile_Base, NULL, strict);
    return sts;
  }
  strict = ODD( dcli_get_qualifier( "/STRICT", 0, 0));
  pop = ODD( dcli_get_qualifier( "/POPNAVIGATOR", 0, 0));
  if ( pop)
    wnav->brow_push_all();

  if ( EVEN( dcli_get_qualifier( "/BOOKMARK", bookmark_str, sizeof(bookmark_str))))
    strcpy( bookmark_str, "");

  strcpy( key, arg_str);
  if ( ODD( dcli_get_qualifier( "dcli_arg2", arg_str, sizeof(arg_str))))
  {
    strcat( key, " ");
    strcat( key, arg_str);
    if ( ODD( dcli_get_qualifier( "dcli_arg3", arg_str, sizeof(arg_str))))
    {
      strcat( key, " ");
      strcat( key, arg_str);
      if ( ODD( dcli_get_qualifier( "dcli_arg3", arg_str, sizeof(arg_str))))
      {
        strcat( key, " ");
        strcat( key, arg_str);
        if ( ODD( dcli_get_qualifier( "dcli_arg4", arg_str, sizeof(arg_str))))
        {
          strcat( key, " ");
          strcat( key, arg_str);
        }
      }
    }
  }
  if ( ODD( dcli_get_qualifier( "/HELPFILE", file_str, sizeof(file_str)))) {
    sts = CoXHelp::dhelp( key, bookmark_str, navh_eHelpFile_Other, file_str, strict);
    if ( EVEN(sts))
      wnav->message('E', "No help on this subject");
  }
  else if ( ODD( dcli_get_qualifier( "/BASE", 0, 0))) {
    sts = CoXHelp::dhelp( key, bookmark_str, navh_eHelpFile_Base, NULL, strict);
    if ( EVEN(sts))
      wnav->message('E', "No help on this subject");
  }
  else if ( ODD( dcli_get_qualifier( "/PROJECT", 0, 0))) {
    sts = CoXHelp::dhelp( key, bookmark_str, navh_eHelpFile_Project, NULL, strict);
    if ( EVEN(sts))
      wnav->message('E', "No help on this subject");
  }
  else {
    sts = CoXHelp::dhelp( key, bookmark_str, navh_eHelpFile_Base, NULL, strict);
    if ( EVEN(sts))
    {
      sts = CoXHelp::dhelp( key, bookmark_str, navh_eHelpFile_Project, NULL, strict);
      if ( EVEN(sts))
        wnav->message('E', "No help on this subject");
    }
  }
//  if ( pop)
//    wnav->pop();

  return sts;
}

static int	wnav_define_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  int	sts;
  char	arg1_str[200];
  char	arg2_str[200];
  char	arg3_str[80];
  char	arg4_str[80];
  char 	*arg3_ptr;
  char 	*arg4_ptr;
	
  if ( EVEN( dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str))))
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }
  if ( EVEN( dcli_get_qualifier( "dcli_arg2", arg2_str, sizeof(arg2_str))))
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }
  if ( EVEN( dcli_get_qualifier( "dcli_arg3", arg3_str, sizeof(arg3_str))))
    arg3_ptr = 0;
  else
    arg3_ptr = arg3_str;
  if ( EVEN( dcli_get_qualifier( "dcli_arg4", arg4_str, sizeof(arg4_str))))
    arg4_ptr = 0;
  else
    arg4_ptr = arg4_str;
  sts = dcli_define_symbol( arg1_str, arg2_str, arg3_ptr, arg4_ptr);
//if (ODD(sts))
//  wnav->message('I',"Symbol defined");
  return sts;
}

static int	wnav_two_func(		void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
	
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  if ( wnav->set_twowindows_cb)
    wnav->set_twowindows_cb( wnav->parent_ctx, 1, 0, 0);
  return WNAV__SUCCESS;
}

static int	wnav_one_func(		void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
	
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  if ( wnav->set_twowindows_cb)
    wnav->set_twowindows_cb( wnav->parent_ctx, 0, 0, 0);
  return WNAV__SUCCESS;
}

static int	wnav_display_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  char	arg1_str[80];
	
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  if ( EVEN( dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str))))
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }

  if ( strncmp( arg1_str, "W1", strlen( arg1_str)) == 0)
  {
    if ( wnav->set_twowindows_cb)
      wnav->set_twowindows_cb( wnav->parent_ctx, 0, 1, 0);
  }
  else if ( strncmp( arg1_str, "W2", strlen( arg1_str)) == 0)
  {
    if ( wnav->set_twowindows_cb)
      wnav->set_twowindows_cb( wnav->parent_ctx, 0, 0, 1);
  }
  else if ( strncmp( arg1_str, "ATTRIBUTE", strlen( arg1_str)) == 0)
  {
    char classstr[80];
    pwr_tOName hierstr;
    pwr_tObjName attributestr;
    pwr_tClassId classid;
    int sts;
    pwr_tObjid hier_objid;

    if (( ODD( dcli_get_qualifier( "/CLASS" , classstr, sizeof(classstr))))) {
      sts = ldh_ClassNameToId( wnav->ldhses, &classid, classstr);
      if ( EVEN(sts)) {
        wnav->message('E', "Class found");
        return sts;
      }
    }
    else {
      wnav->message('E', "Enter class");
      return WNAV__QUAL;
    }

    if (( ODD( dcli_get_qualifier( "/HIERARCHY" , hierstr, sizeof(hierstr))))) {
      sts = ldh_NameToObjid( wnav->ldhses, &hier_objid, hierstr);
      if ( EVEN(sts)) {
        wnav->message('E', "Hierarchy object not found");
        return sts;
      }
    }
    else
      hier_objid = pwr_cNObjid;

    if ( EVEN( dcli_get_qualifier( "/ATTRIBUTE" , attributestr, sizeof(attributestr)))) {
      wnav->message('E', "Enter attribute");
      return WNAV__QUAL;
    }
    wnav->wda_new( hier_objid, classid, attributestr, wnav->editmode, 
		   wnav->gbl.advanced_user, 1);
    
  }
  else if ( strncmp( arg1_str, "OBJECTS", strlen( arg1_str)) == 0) {
    char	class_str[80];
    pwr_tOName	name_str;
    pwr_tOName	root_str;
    int		sts;
    pwr_tCid	*cid_p;
    pwr_tCid 	cidvect[10];
    pwr_tOid	root;
    char	*name_p;
    char 	depth_str[80];
    int		depth;

    // Command is "DISPLAY OBJECTS" 

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    if ( ODD( dcli_get_qualifier( "/CLASS", class_str, sizeof(class_str)))) {
      char cvect[10][80];
      int nr;
      
      nr = dcli_parse( class_str, ",", "", (char *)cvect, 
		sizeof( cvect) / sizeof( cvect[0]), sizeof( cvect[0]), 0);
      if ( nr == 0) {
	wnav->message('E',"Syntax error");
	return WNAV__SYNTAX;
      }

      for ( int i = 0; i < nr; i++) {
	sts = ldh_ClassNameToId( wnav->ldhses, &cidvect[i], cvect[i]);
	if ( EVEN(sts)) {
	  wnav->message('E',"Class not found");
	  return WNAV__SYNTAX;
	}
      }
      cidvect[nr] = 0;
      cid_p = cidvect;
    }
    else
      cid_p = 0;

    if ( ODD( dcli_get_qualifier( "/ROOT", root_str, sizeof(root_str)))) {
      sts = ldh_NameToObjid( wnav->ldhses, &root, root_str);
      if ( EVEN(sts)) {
        wnav->message('E', "Object not found");
        return sts;
      }
    }
    else
      root = pwr_cNObjid;

    if ( ODD( dcli_get_qualifier( "/NAME", name_str, sizeof(name_str))))
      name_p = name_str;
    else
      name_p = 0;
    
    if ( ODD( dcli_get_qualifier( "/DEPTH", depth_str, sizeof(depth_str)))) {
      if ( cdh_NoCaseStrcmp( depth_str, "self") == 0)
	depth = trv_eDepth_Self;
      else if ( cdh_NoCaseStrcmp( depth_str, "children") == 0)
	depth = trv_eDepth_Children;
      else if ( cdh_NoCaseStrcmp( depth_str, "deep") == 0)
	depth = trv_eDepth_Deep;
      else {
        wnav->message('E',"Depth should be Self, Children or Deep");
        return WNAV__SYNTAX;
      }
    }
    else
      depth = trv_eDepth_Deep;

    wnav->display_objects( cid_p, name_p, root, depth);
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }

  return WNAV__SUCCESS;
}

static int	wnav_edit_func(		void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
	
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  if ( wnav->set_configure_cb)
    wnav->set_configure_cb( wnav->parent_ctx, 1);
  return WNAV__SUCCESS;
}

static int	wnav_noedit_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
	
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  if ( wnav->set_configure_cb)
    wnav->set_configure_cb( wnav->parent_ctx, 0);
  return WNAV__SUCCESS;
}

static void wnav_login_success_bc( void *ctx)
{
  WNav *wnav = (WNav *)ctx;
  char	msg[80];

  CoLogin::get_login_info( 0, 0, wnav->user, (unsigned long *)&wnav->priv, 0);
  if ( wnav->admin_login) {
    wnav->admin_login = 0;
    wnav->priv = pwr_mPrv_Administrator;
    CoLogin::insert_login_info( "", "", wnav->user, wnav->priv, 0);
  }
  sprintf( msg, "User %s logged in", wnav->user);
  wnav->message('I', msg);
}

static void wnav_login_cancel_bc(void *wnav)
{  
  ((WNav *)wnav)->admin_login = 0;
}

static int	wnav_login_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  int	sts;
  char	arg1_str[80];
  char	arg2_str[80];
  char	systemname[80];
  char	systemgroup[80];
  unsigned int	priv;
  char	msg[80];
  int 	administrator;
	
  administrator = ODD(dcli_get_qualifier( "/ADMINISTRATOR", 0, 0));

  if ( administrator) {
    sts = user_CheckSystemGroup( "administrator");
    if ( EVEN(sts)) {
      // Username and password are not required
      strcpy( wnav->user, "Administrator");
      wnav->priv = pwr_mPrv_Administrator;
      CoLogin::insert_login_info( "", "", "Administrator", wnav->priv, 0);
      wnav->message('I', "Administrator logged in");
      return WNAV__SUCCESS;
    }
    else {
      // Check user and password in systemgroup "aministrator"
      if ( EVEN( dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str)))) {
	wnav->admin_login = 1;
	wnav->login_new( "PwR Login", "administrator", wnav_login_success_bc, 
			 wnav_login_cancel_bc, &sts);
	return WNAV__SUCCESS;
      }
      if ( EVEN( dcli_get_qualifier( "dcli_arg2", arg2_str, sizeof(arg2_str)))) {
	wnav->message('E',"Password required");
	return WNAV__SYNTAX;
      }
      cdh_ToLower( arg1_str, arg1_str);
      cdh_ToLower( arg2_str, arg2_str);
      strcpy( arg2_str, UserList::pwcrypt(arg2_str));
      sts = user_CheckUser( "administrator", arg1_str, arg2_str, &priv);
      if ( EVEN(sts))
	wnav->message('E',"Login failure");
      else {
	strcpy( wnav->user, arg1_str);
	wnav->priv = pwr_mPrv_Administrator;
	CoLogin::insert_login_info( "", "", arg1_str, wnav->priv, 0);
	sprintf( msg, "User %s logged in", arg1_str);
	wnav->message('I', msg);
      }
      return WNAV__SUCCESS;
    }
  }

  sts = utl_get_systemname( systemname, systemgroup);
  if ( EVEN(sts)) return sts;

  if ( EVEN( dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str)))) {
    wnav->login_new( "PwR Login", systemgroup, wnav_login_success_bc, 
		     wnav_login_cancel_bc, &sts);
    wnav->admin_login = 0;
    return WNAV__SUCCESS;
  }
  if ( EVEN( dcli_get_qualifier( "dcli_arg2", arg2_str, sizeof(arg2_str)))) {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }
  cdh_ToLower( arg1_str, arg1_str);
  cdh_ToLower( arg2_str, arg2_str);
  strcpy( arg2_str, UserList::pwcrypt(arg2_str));
  sts = user_CheckUser( systemgroup, arg1_str, arg2_str, &priv);
  if ( EVEN(sts))
    wnav->message('E',"Login failure");
  else
  {
    strcpy( wnav->user, arg1_str);
    wnav->priv = priv;
    CoLogin::insert_login_info( "", "", arg1_str, wnav->priv, 0);
    sprintf( msg, "User %s logged in", arg1_str);
    wnav->message('I', msg);
  }
  return sts;
}

static int	wnav_logout_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  char	msg[80];
	
  if ( strcmp( wnav->base_user, "") == 0)
  {
    sprintf( msg, "User %s logged out", wnav->user);
    wnav->message('I', msg);
  }
  else
  {
    sprintf( msg, "Returned to user %s", wnav->base_user);
    wnav->message('I', msg);
  }
  strcpy( wnav->user, wnav->base_user);
  wnav->priv = wnav->base_priv;
  CoLogin::insert_login_info( "", "", wnav->base_user, wnav->priv, 0);
  return WNAV__SUCCESS;
}

static int	wnav_set_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  char	arg1_str[80];
  int	arg1_sts;
  char	arg2_str[80];
  int	arg2_sts;
	
  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "ADVANCEDUSER", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET ADVANCEDUSER");
    else
      wnav->gbl.advanced_user = 1;
  }
  else if ( strncmp( arg1_str, "NOADVANCEDUSER", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOADVANCEDUSER");
    else
      wnav->gbl.advanced_user = 0;
  }
  else if ( strncmp( arg1_str, "ALLTOPLEVEL", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET ALLTOPLEVEL");
    else
    {
      wnav->gbl.all_toplevel = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOALLTOPLEVEL", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOALLTOPLEVEL");
    else
    {
      wnav->gbl.all_toplevel = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "SHOWCLASS", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET SHOWCLASS");
    else
    {
      wnav->gbl.show_class = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOSHOWCLASS", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOSHOWCLASS");
    else
    {
      wnav->gbl.show_class = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "SHOWALIAS", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET SHOWALIAS");
    else
    {
      wnav->gbl.show_alias = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOSHOWALIAS", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOSHOWALIAS");
    else
    {
      wnav->gbl.show_alias = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "SHOWDESCRIPTION", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET SHOWDESCRIPTION");
    else
    {
      wnav->gbl.show_descrip = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOSHOWDESCRIPTION", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOSHOWDESCRIPTION");
    else
    {
      wnav->gbl.show_descrip = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "SHOWOBJREF", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET SHOWOBJREF");
    else
    {
      wnav->gbl.show_objref = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOSHOWOBJREF", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOSHOWOBJREF");
    else
    {
      wnav->gbl.show_objref = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "SHOWOBJXREF", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET SHOWOBJXREF");
    else
    {
      wnav->gbl.show_objxref = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOSHOWOBJXREF", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOSHOWOBJXREF");
    else
    {
      wnav->gbl.show_objxref = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "SHOWATTRREF", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET SHOATTRREF");
    else
    {
      wnav->gbl.show_attrref = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOSHOWATTRREF", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOSHOWATTRREF");
    else
    {
      wnav->gbl.show_attrref = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "SHOWATTRXREF", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET SHOWATTRXREF");
    else
    {
      wnav->gbl.show_attrxref = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOSHOWATTRXREF", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOSHOWATTRXREF");
    else
    {
      wnav->gbl.show_attrxref = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "SHOWTRUEDB", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET SHOWTRUEDB");
    else
    {
      wnav->gbl.show_truedb = 1;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "NOSHOWTRUEDB", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOSHOWTRUEDB");
    else
    {
      wnav->gbl.show_truedb = 0;
      wnav->ldh_refresh( pwr_cNObjid);
    }
  }
  else if ( strncmp( arg1_str, "BUILDFORCE", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET BUILDFORCE");
    else
      wnav->gbl.build.force = 1;
  }
  else if ( strncmp( arg1_str, "NOBUILDFORCE", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOBUILDFORCE");
    else
      wnav->gbl.build.force = 0;
  }
  else if ( strncmp( arg1_str, "BUILDDEBUG", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET BUILDDEBUG");
    else
      wnav->gbl.build.debug = 1;
  }
  else if ( strncmp( arg1_str, "NOBUILDDEBUG", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOBUILDDEBUG");
    else
      wnav->gbl.build.debug = 0;
  }
  else if ( strncmp( arg1_str, "BUILDCROSSREF", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET BUILDCROSSREF");
    else
      wnav->gbl.build.crossref = 1;
  }
  else if ( strncmp( arg1_str, "NOBUILDCROSSREF", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOBUILDCROSSREF");
    else
      wnav->gbl.build.crossref = 0;
  }
  else if ( strncmp( arg1_str, "BUILDMANUAL", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET BUILDMANUAL");
    else
      wnav->gbl.build.manual = 1;
  }
  else if ( strncmp( arg1_str, "NOBUILDMANUAL", strlen( arg1_str)) == 0)
  {
    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOBUILDMANUAL");
    else
      wnav->gbl.build.manual = 0;
  }
  else if ( strncmp( arg1_str, "VERIFY", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
    {
      wnav->gbl.verify = 1;
      return WNAV__SUCCESS;
    }

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET VERIFY");
    else
      wnav->gbl.verify = 1;
  }
  else if ( strncmp( arg1_str, "NOVERIFY", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
    {
      wnav->gbl.verify = 0;
      return WNAV__SUCCESS;
    }

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      // Global command to all windows
      (wnav->gbl_command_cb)( wnav->parent_ctx, "SET NOVERIFY");
    else
      wnav->gbl.verify = 0;
  }
  else if ( strncmp( arg1_str, "SYMBOLFILE", strlen( arg1_str)) == 0)
  {
    pwr_tFileName filename_str;
    pwr_tCmd cmd;

    if ( EVEN( dcli_get_qualifier( "dcli_arg2", filename_str, sizeof(filename_str)))) {
      wnav->message('E',"Syntax error");
      return WNAV__SYNTAX;
    }

    if ( wnav->window_type == wnav_eWindowType_No)
    {
      strcpy( wnav->gbl.symbolfilename, filename_str);
      return WNAV__SUCCESS;
    }

    if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0))) {
      // Global command to all windows
      sprintf( cmd, "set symbolfile \"%s\"", filename_str);
      (wnav->gbl_command_cb)( wnav->parent_ctx, cmd);
    }
    else
      strcpy( wnav->gbl.symbolfilename, filename_str);
  }
  else if ( strncmp( arg1_str, "INPUTFOCUS", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;
    
    arg2_sts = dcli_get_qualifier( "dcli_arg2", arg2_str, sizeof(arg2_str));
    if ( strncmp( arg2_str, "W1", strlen( arg2_str)) == 0)
    {
      if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      {
        // Global command to all windows
        (wnav->gbl_command_cb)( wnav->parent_ctx, "SET INPUTFOCUS W1");
      }
      else
      {
        if ( wnav->window_type == wnav_eWindowType_W1)
          if ( wnav->set_focus_cb)
            (wnav->set_focus_cb)( wnav->parent_ctx, wnav);
      }
    }
    else if ( strncmp( arg2_str, "W2", strlen( arg2_str)) == 0)
    {
      if ( EVEN( dcli_get_qualifier( "/LOCAL", 0, 0)))
      {
        // Global command to all windows
        (wnav->gbl_command_cb)( wnav->parent_ctx, "SET INPUTFOCUS W2");
      }
      else
      {
        if ( wnav->window_type == wnav_eWindowType_W2)
          if ( wnav->set_focus_cb)
            (wnav->set_focus_cb)( wnav->parent_ctx, wnav);
      }
    }
  }
  else if ( strncmp( arg1_str, "WINDOW", strlen( arg1_str)) == 0)
  {
    int width = 0;
    int height = 0;
    int	nr;

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( ODD( dcli_get_qualifier( "/WIDTH", arg2_str, sizeof(arg2_str))))
    {
      nr = sscanf( arg2_str, "%d", &width);
      if ( nr != 1)
      {
        wnav->message('E',"Syntax error");
        return WNAV__SYNTAX;
      }
    }

    if ( ODD( dcli_get_qualifier( "/HEIGHT", arg2_str, sizeof(arg2_str))))
    {
      nr = sscanf( arg2_str, "%d", &height);
      if ( nr != 1)
      {
        wnav->message('E',"Syntax error");
        return WNAV__SYNTAX;
      }
    }

    if ( wnav->set_window_char_cb)
      (wnav->set_window_char_cb)( wnav->parent_ctx, width, height);
  }
  else if ( strncmp( arg1_str, "ATTRIBUTE", strlen( arg1_str)) == 0)
  {
    char		file[80]; 
    int			termflg;
    pwr_tOName		parameterstr;
    char		valuestr[200];
    pwr_tObjName       	classstr;
    pwr_tOName		hierstr;
    pwr_tOName		namestr;
    char		*classstr_p;
    char		*hierstr_p;
    char		*namestr_p;
    char		*valuestr_p;
    int			sts_class, sts_hier, sts_name;
    pwr_tStatus		sts;
    int			confirm;
    int			log;

    if ( !wnav->editmode)
    {
      wnav->message('E', "Not in edit mode");
      return WNAV__SYNTAX;
    }

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    termflg = ODD( dcli_get_qualifier( "/TERMINAL", 0, 0));

    if (( EVEN( dcli_get_qualifier( "/OUTPUT", file, sizeof(file)))))
    {
      strcpy( file, "");
      termflg = 1;
    }

    confirm = EVEN( dcli_get_qualifier( "/NOCONFIRM", 0, 0));
    log = ODD( dcli_get_qualifier( "/LOG", 0, 0));
   
    if (( sts_class = ODD( dcli_get_qualifier( "/CLASS" , classstr, sizeof(classstr)))))
      classstr_p = classstr;
    else
      classstr_p = NULL;
    if (( sts_hier = ODD( dcli_get_qualifier( "/HIERARCHY" , hierstr, sizeof(hierstr)))))
      hierstr_p = hierstr;
    else
      hierstr_p = NULL;
    if (( sts_name = ODD( dcli_get_qualifier( "/NAME" , namestr, sizeof(namestr)))))
      namestr_p = namestr;
    else
      namestr_p = NULL;
    if ( ODD( dcli_get_qualifier( "/VALUE" , valuestr, sizeof(valuestr))))
      valuestr_p = valuestr;
    else
      valuestr_p = NULL;
    if ( EVEN( dcli_get_qualifier( "/ATTRIBUTE" , parameterstr, sizeof(parameterstr))))
    {
      wnav->message('E', "Enter attribute");
      return WNAV__QUAL;
    }

    if ( sts_class || sts_hier || sts_name )
    {
      sts = utl_set_object_parameter
        ( wnav->ldhses, hierstr_p , classstr_p ,namestr_p, parameterstr, 
	valuestr_p, termflg , file, confirm, log);
      if ( EVEN(sts))
        wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
    else
    {
      wnav->message('E',"Qualifier required");
      return WNAV__QUAL;
    }
  }
  else if ( strncmp( arg1_str, "TEMPLATE", strlen( arg1_str)) == 0)
  {
    char		str[80] ; 
    int		signalobjectsegments;
    int		sigchanconsegments;
    int		shosigchancon;
    int		shodetecttext;
    pwr_tStatus	sts;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    if ( ODD( dcli_get_qualifier( "/SIGNALOBJECTSEG" , str, sizeof(str))))
    {
      sts = sscanf(str, "%d", &signalobjectsegments);
      if ( sts != 1 || signalobjectsegments < 1)
      {
        wnav->message('E', "Syntax error");
        return WNAV__SYNTAX;
      }
    }
    else
      signalobjectsegments = 0;
    
    if ( ODD( dcli_get_qualifier( "/SIGCHANCONSEG" , str, sizeof(str))))
    {
      sts = sscanf(str, "%d", &sigchanconsegments);
      if ( sts != 1 || sigchanconsegments < 1)
      {
        wnav->message('E', "Syntax error");
        return WNAV__SYNTAX;
      }
    }
    else
      sigchanconsegments = 0;
    
    if ( ODD( dcli_get_qualifier( "/SHOSIGCHANCON" , str, sizeof(str))))
    {
      sts = sscanf(str, "%d", &shosigchancon);
      if ( sts != 1 || !(shosigchancon == 1 || shosigchancon == 0))
      {
        wnav->message('E', "Syntax error");
        return WNAV__SYNTAX;
      }
      shosigchancon += 1;
    }
    else
      shosigchancon = 0;
    
    if ( ODD( dcli_get_qualifier( "/SHODETECTTEXT" , str, sizeof(str))))
    {
      sts = sscanf(str, "%d", &shodetecttext);
      if ( sts != 1 || !(shodetecttext == 1 || shodetecttext == 0))
      {
        wnav->message('E', "Syntax error");
        return WNAV__SYNTAX;
      }
      shodetecttext += 1;
    }
    else
      shodetecttext = 0;
    
    sts = utl_set_template( wnav->ldhses, signalobjectsegments, shosigchancon,
		sigchanconsegments, shodetecttext);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "VOLUME", strlen( arg1_str)) == 0)
  {
    pwr_tVolumeId	volid;
    char		volumename[80];
    int			sts;
    ldh_sSessInfo	info;
    int			pop;

    if ( EVEN( dcli_get_qualifier( "/VOLUMENAME", volumename, sizeof(volumename))))
    {
      if ( EVEN( dcli_get_qualifier( "dcli_arg2", volumename, sizeof(volumename))))
      {
        wnav->message('E', "Qualifier required");
        return WNAV__QUAL;
      }
    }

    sts = wnav_wccm_get_wbctx_cb( wnav, &wnav->wbctx);
    if ( EVEN(sts)) return sts;

    sts = ldh_VolumeNameToId( wnav->wbctx, volumename, &volid);
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }

    sts = (wnav->attach_volume_cb)( wnav->parent_ctx, volid, 1);
    if ( sts == WNAV__VOLATTACHED)
    {
      if ( wnav->window_type == wnav_eWindowType_No)
        pop = 0;
      else
      {
        if ( wnav->brow_cnt == 1)
          // Keep brow ctx
          pop = 0;
        else
          pop = 1;
      }

      // Detach volume first
      sts = ldh_GetSessionInfo( wnav->ldhses, &info);
      if ( EVEN(sts)) return sts;

      if ( !info.Empty )
      {
        wnav->message('E', "Session is not saved");
        return WNAV__NOTSAVED;
      }
      sts = (wnav->detach_volume_cb)( wnav->parent_ctx);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
      sts = (wnav->attach_volume_cb)( wnav->parent_ctx, volid, pop);
    }
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "DB", strlen( arg1_str)) == 0)
  {
    wnav->message('E', "command \"set db\" is obsolete");
  }
  else if ( strncmp( arg1_str, "SUBWINDOW", strlen( arg1_str)) == 0)
  {
    char filenamestr[80];
    char sourcestr[80];
    char objectstr[80];
    int modal;
    WGe *wge;

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    modal = ODD(dcli_get_qualifier( "/MODAL", 0, 0));

    if ( EVEN( dcli_get_qualifier( "/SOURCE", sourcestr, sizeof(sourcestr)))) {
      wnav->message('E',"Syntax error");
      return WNAV__SYNTAX;
    }

    if ( EVEN( dcli_get_qualifier( "/NAME", objectstr, sizeof(objectstr)))) {
      wnav->message('E',"Syntax error");
      return WNAV__SYNTAX;
    }
    if ( EVEN( dcli_get_qualifier( "dcli_arg2", filenamestr, sizeof(filenamestr)))) {
      wnav->message('E',"Syntax error");
      return WNAV__SYNTAX;
    }
    cdh_ToLower( filenamestr, filenamestr);
    cdh_ToLower( sourcestr, sourcestr);

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( wnav->appl.find( applist_eType_Graph, filenamestr, (void **) &wge)) {
      wge->set_subwindow_source( objectstr, sourcestr, modal);
      if ( modal) {
	wnav->wge_subwindow_loop( wge);
      }
    }
    else {
      wnav->message('E', "Graph not found");
    }
  }
  else
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }
  return WNAV__SUCCESS;
}

static int	wnav_show_func(	void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  int	sts;
  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "VERSION", strlen( arg1_str)) == 0)
  {
    // Command is "SHOW VERSION"
    char	message_str[80];
	 
    strcpy( message_str, "Wtt version ");
    strcat( message_str, wtt_version);
    wnav->message('I', message_str);
    return WNAV__SUCCESS;
  }
  else if ( strncmp( arg1_str, "LICENSE", strlen( arg1_str)) == 0)
  {
    // Command is "SHOW LICENSE"
    if ( wnav->window_type == wnav_eWindowType_No)
      system( "cat $pwr_exe/en_us/license.txt");
    else
      wnav->wow->DisplayLicense();
    return WNAV__SUCCESS;
  }
  else if ( strncmp( arg1_str, "SYMBOL", strlen( arg1_str)) == 0)
  {
    /* Command is "SHOW SYMBOL" */
    char	arg2_str[80];
    char	message_str[400];
    char  	value[DCLI_SYM_VALUE_SIZE];
	 
    if ( ODD( dcli_get_qualifier( "/ALL", 0, 0)))
    {
      sts = wnav->show_symbols();
      return sts;
    }
    else
    {
      if ( EVEN( dcli_get_qualifier( "dcli_arg2", arg2_str, sizeof(arg2_str))))
      {
        sts = wnav->show_symbols();
        return sts;
      }
      sts = dcli_get_symbol( arg2_str, value);
      if (EVEN(sts))
      {
        wnav->message('E', "Symbol not found");
        return sts;
      }
      sprintf( message_str, "Symbol %s = %s", arg2_str, value);
      wnav->message('I', message_str);
      return WNAV__SUCCESS;
   }
  }
  else if ( strncmp( arg1_str, "VOLUMES", strlen( arg1_str)) == 0)
  {
    /* Command is "SHOW VOLUMES" */
    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    if ( wnav->window_type == wnav_eWindowType_No)
    {
      int all = ODD( dcli_get_qualifier( "/ALL", 0, 0));

      sts = utl_show_volumes( wnav->ldhses, all);
      if ( EVEN(sts))
        wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
    else
      wnav->show_database();
    return 1;
  }
  else if ( strncmp( arg1_str, "DEFAULT", strlen( arg1_str)) == 0)
  {
    /* Command is "SHOW DEFAULT" */
    char	message_str[80];
	 
    sprintf( message_str, "Default directory: %s", wnav->gbl.default_directory);
    wnav->message('I', message_str);
    return WNAV__SUCCESS;
  }
  else if ( strncmp( arg1_str, "USER", strlen( arg1_str)) == 0)
  {
    char msg[120];
    char priv_str[80];

    if ( strcmp( wnav->user, "") == 0)
    {
      wnav->message('I', "Not logged in");
    }
    else
    {
      user_DevPrivToString( CoLogin::privilege(), priv_str, sizeof(priv_str));
      sprintf( msg, "User %s (%s)", CoLogin::username(), priv_str);
      wnav->message('I', msg);
    }
  }
  else if ( strncmp( arg1_str, "CHILDREN", strlen( arg1_str)) == 0)
  {
    // Command is "SHOW CHILDREN" 
    pwr_tOName		name_str;
    pwr_tObjid		objid;
    int			size; 
    WItem		*item;

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    if ( ODD( dcli_get_qualifier( "/NAME", name_str, sizeof(name_str))))
    {
      sts = ldh_NameToObjid( wnav->ldhses, &objid, name_str);
      if ( EVEN(sts))
      {
        wnav->message('E', "Object not found");
        return sts;
      }
      sts = ldh_ObjidToName( wnav->ldhses, objid, ldh_eName_Hierarchy,
		name_str, sizeof(name_str), &size);
      if ( EVEN(sts)) return sts;
    }
    else
    {
      // Get the selected object
      sts = wnav->get_current_object( &objid, 
			name_str, sizeof( name_str), ldh_eName_Hierarchy);
      if ( EVEN(sts))
      {
        wnav->message('E', "Select an object or enter name");
        return sts;
      }
    }

    wnav->brow_push_to_volume();
    brow_SetNodraw( wnav->brow->ctx);
    sts = wnav->display_object( objid);
    if ( EVEN(sts)) 
    {
      brow_ResetNodraw( wnav->brow->ctx);
      brow_Redraw( wnav->brow->ctx, 0);
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }

    sts = wnav->find( objid, (void **) &item);
    if ( !sts) 
    {
      brow_ResetNodraw( wnav->brow->ctx);
      brow_Redraw( wnav->brow->ctx, 0);
      wnav->message('E', "Object not found");
      return sts;
    }
    switch( item->type)
    {
      case wnav_eItemType_Object: 
	sts = ((WItemBaseObject *)item)->open_children( wnav, 0, 0);
        break;
      default:
        ;
    }
    brow_ResetNodraw( wnav->brow->ctx);
    brow_Redraw( wnav->brow->ctx, 0);
    brow_CenterObject( wnav->brow->ctx, item->node, 0.0);
    wnav->last_selected = item->node;
    return WNAV__SUCCESS;
  }
  else if ( strncmp( arg1_str, "OBJECTS", strlen( arg1_str)) == 0)
  {
    char	objid_str[80];
    char	file[80]; 
    int		termflg;
    int		sts;

    // Command is "SHOW OBJECTS" 

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    if ( ODD( dcli_get_qualifier( "/OBJID", objid_str, sizeof(objid_str))))
    {
      termflg = ODD( dcli_get_qualifier( "/TERMINAL", 0, 0));

      if ( EVEN( dcli_get_qualifier( "/OUTPUT", file, sizeof(file))))
      {
        strcpy( file, "");
        termflg = 1;
      }

      sts = utl_show_object_objdid( wnav->ldhses, objid_str, termflg, file);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
    else
    {
      pwr_tObjName     	classstr;
      pwr_tOName       	hierstr;
      pwr_tOName	namestr;
      pwr_tOName       	parameterstr;
      char		volumestr[80] ;
      char		*classstr_p;
      char		*hierstr_p;
      char		*namestr_p;
      char		*parameterstr_p;
      char		*volumestr_p;
      int		sts_class, sts_hier, sts_name, sts_parameter;
      int		fullflg;
      int		allvolumes;
      int		append;
      int		exactorder;

      termflg = ODD( dcli_get_qualifier( "/TERMINAL", 0, 0));

      if (( EVEN( dcli_get_qualifier( "/OUTPUT", file, sizeof(file)))))
      {
        strcpy( file, "");
        termflg = 1;
      }

      fullflg = ODD( dcli_get_qualifier( "/FULL", 0, 0));
      allvolumes = ODD( dcli_get_qualifier( "/ALL", 0, 0));
      append = ODD( dcli_get_qualifier( "/APPEND", 0, 0));
      exactorder = ODD( dcli_get_qualifier( "/EXACTORDER", 0, 0));

      if (( sts_class = ODD( dcli_get_qualifier( "/CLASS", classstr, sizeof(classstr)))))
        classstr_p = classstr;
      else
        classstr_p = NULL;
      if (( sts_hier = ODD( dcli_get_qualifier( "/HIERARCHY", hierstr, sizeof(hierstr)))))
        hierstr_p = hierstr;
      else
        hierstr_p = NULL;
      if (( sts_name = ODD( dcli_get_qualifier( "/NAME", namestr, sizeof(namestr)))))
        namestr_p = namestr;
      else
        namestr_p = NULL;
      if (( sts_parameter = ODD( dcli_get_qualifier( "/PARAMETER", parameterstr, sizeof(parameterstr)))))
        parameterstr_p = parameterstr;
      else
        parameterstr_p = NULL;
      if ( ODD( dcli_get_qualifier( "/VOLUME", volumestr, sizeof(volumestr))))
        volumestr_p = volumestr;
      else
        volumestr_p = NULL;

      if ( sts_parameter && !sts_class)
      {
        wnav->message('E', "Class qualifier required");
        return WNAV__QUAL;
      }

      if ( sts_class || sts_hier || sts_name )
      {
        sts = utl_show_obj_hier_class_name( wnav->ldhses, hierstr_p, 
		classstr_p, namestr_p, parameterstr_p, volumestr_p,
		termflg, file, fullflg, allvolumes, append, exactorder);
        if ( EVEN(sts))
	{
          wnav->message(' ', wnav_get_message(sts));
	  return sts;
	}
      }
      else 
      {
        wnav->message('E', "Qualifier required");
        return WNAV__QUAL;
      }
    }
  }
  else if ( strncmp( arg1_str, "OBJID", strlen( arg1_str)) == 0)
  {
    // Command is "SHOW OBJID"
    pwr_tOName		name_str;
    pwr_tObjid		objid;
    char		msg[256];
    int			size; 

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    if ( ODD( dcli_get_qualifier( "/NAME", name_str, sizeof(name_str))))
    {
      sts = ldh_NameToObjid( wnav->ldhses, &objid, name_str);
      if ( EVEN(sts))
      {
        wnav->message('E', "Object not found");
        return sts;
      }
      sts = ldh_ObjidToName( wnav->ldhses, objid, ldh_eName_Hierarchy,
		name_str, sizeof(name_str), &size);
      if ( EVEN(sts)) return sts;
    }
    else
    {
      // Get the selected object
      sts = wnav->get_current_object( &objid, 
			name_str, sizeof( name_str), ldh_eName_Hierarchy);
      if ( EVEN(sts))
      {
        wnav->message('E', "Select an object or enter name");
        return sts;
      }
    }

    sprintf( msg, "Objid %s, Name %s", cdh_ObjidToString( NULL, objid, 0),
			name_str);
    wnav->message('I', msg);
    return WNAV__SUCCESS;
  }
  else if ( strncmp( arg1_str, "MODULES", strlen( arg1_str)) == 0)
  {
    // command is "show modules"
    pwr_tOName		namestr;
    pwr_tOName		hierstr;
    char		objidstr[80];
    char                *namestr_p;
    char                *hierstr_p;
    char                *objidstr_p;
    pwr_tFileName       file;
    int                 termflg;

    termflg = ODD( dcli_get_qualifier( "/TERMINAL", 0, 0));

    if (( EVEN( dcli_get_qualifier( "/OUTPUT", file, sizeof(file)))))
    {
      strcpy( file, "");
      termflg = 1;
    }

    if ( ODD( dcli_get_qualifier( "/HIERARCHY", hierstr, sizeof(hierstr))))
      hierstr_p = hierstr;
    else
      hierstr_p = NULL;
    if ( ODD( dcli_get_qualifier( "/NAME", namestr, sizeof(namestr))))
      namestr_p = namestr;
    else
      namestr_p = NULL;
    if ( ODD( dcli_get_qualifier( "/OBJID", objidstr, sizeof(objidstr))))
      objidstr_p = objidstr;
    else
      objidstr_p = NULL;

    // allvolumes = ODD( dcli_get_qualifier( "/ALLVOLUMES", 0, 0));

    //if ( ODD( dcli_get_qualifier( "/VOLUMES" , volumestr, sizeof(volumestr))))
    //  volumestr_p = volumestr;
    //else
    //  volumestr_p = NULL;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_show_modules( wnav->ldhses, objidstr_p, namestr_p, 
		hierstr_p, termflg, file);
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else if ( strncmp( arg1_str, "SCRIPT", strlen( arg1_str)) == 0)
  {
    // Command is "SHOW SCRIPT"
    char	arg2_str[80];
    char	title[80];
    char	filename[80];
    char	option_str[80];
    int		hide_dir = 0;

    if ( EVEN( dcli_get_qualifier( "dcli_arg2", arg2_str, sizeof(arg2_str))))
    {
      hide_dir = 1;
      dcli_get_defaultfilename( "*", filename, ".pwr_com");
      strcpy( title, "WNav scripts");
    }
    else
    {
      dcli_get_defaultfilename( arg2_str, filename, ".pwr_com");
      strcpy( title, "File list");
    }
    if ( ODD( dcli_get_qualifier( "/OPTION", option_str, sizeof(option_str))))
    {
      if ( strstr( option_str, "HIDE_DIR"))
        hide_dir = 1;
      else if ( strstr( option_str, "PROJECT"))
      {
        dcli_translate_filename( filename, "pwrp_login:*.pwr_com");
        sts = wnav->show_file( filename, "Project scripts", 1);
        if ( EVEN(sts))
          wnav->message('E', "No files found");
        return sts;
      }
      else if ( strstr( option_str, "BASE"))
      {
        dcli_translate_filename( filename, "pwr_exe:*.pwr_com");
        sts = wnav->show_file( filename, "Base scripts", 1);
        if ( EVEN(sts))
          wnav->message('E', "No files found");
        return sts;
      }
    }
    sts = wnav->show_file( filename, title, hide_dir);
    if ( EVEN(sts))
    {
      wnav->message('E', "No files found");
      return sts;
    }
    return sts;
  }
  else if ( strncmp( arg1_str, "METHODS", strlen( arg1_str)) == 0)
  {
    // Command is "SHOW METHODS"
    wb_erep *erep = *(wb_env *)wnav->wbctx;
    erep->printMethods();
  }
  else if ( strncmp( arg1_str, "NEXT_FREE_USER_VID", strlen( arg1_str)) == 0)
  {
    // Command is "SHOW NEXT_FREE_USER_VID"
    pwr_tVid next_vid;
    int sts;

    sts = wnav->get_next_free_vid( cdh_cUserVolMin, cdh_cUserVolMax, &next_vid);
    if ( EVEN(sts))
      wnav->message( 'E', wnav_get_message(sts));
    else {
      char msg[200];

      sprintf( msg, "%s is next free user volume identity\n", cdh_VolumeIdToString( 0, next_vid, 0, 0));
      wnav->message( 'I', msg);
    }
  }
  else if ( strncmp( arg1_str, "NEXT_FREE_USERCLASS_VID", strlen( arg1_str)) == 0)
  {
    // Command is "SHOW NEXT_FREE_CLASSUSER_VID"
    pwr_tVid next_vid;
    int sts;

    sts = wnav->get_next_free_vid( cdh_cUserClassVolMin, cdh_cUserClassVolMax, &next_vid);
    if ( EVEN(sts))
      wnav->message( 'E', wnav_get_message(sts));
    else {
      char msg[200];

      sprintf( msg, "%s is next free userclass volume identity\n", cdh_VolumeIdToString( 0, next_vid, 0, 0));
      wnav->message( 'I', msg);
    }
  }
  else
  {
    wnav->message('E', "Unknown qualifier");
    return WNAV__QUAL;
  }
  return WNAV__SUCCESS;
}

static int	wnav_compile_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  pwr_tOName   	hierstr;
  char		*hierstr_p;
  pwr_tOName   	plcpgmstr;
  char		*plcpgmstr_p;
  pwr_tOName   	windowstr;
  char		*windowstr_p;
  pwr_tOName   	fromstr;
  char		*fromstr_p;
  int		sts;
  int		modified;
  int		debug;
  int		allvolumes;
  char		volumestr[80] ;
  char		*volumestr_p;
  int		sts_plcpgm, sts_hier, sts_window, sts_from, sts_volumes;

  if ( !wnav->editmode)
  {
    wnav->message('E', "Not in edit mode");
    return WNAV__NOEDIT;
  }

  modified = ODD( dcli_get_qualifier( "/MODIFIED", 0, 0));
  debug = ODD( dcli_get_qualifier( "/DEBUG", 0, 0));

  if (( sts_hier = ODD( dcli_get_qualifier( "/HIERARCHY", hierstr, sizeof(hierstr)))))
    hierstr_p = hierstr;
  else
    hierstr_p = NULL;
  if (( sts_plcpgm = ODD( dcli_get_qualifier( "/PLCPGM", plcpgmstr, sizeof(plcpgmstr)))))
    plcpgmstr_p = plcpgmstr;
  else
    plcpgmstr_p = NULL;

  if (( sts_window = ODD( dcli_get_qualifier( "/WINDOW" , windowstr, sizeof(windowstr)))))
    windowstr_p = windowstr;
  else
    windowstr_p = NULL;
  if (( sts_from = ODD( dcli_get_qualifier( "/FROM_PLCPGM", fromstr, sizeof(fromstr)))))
    fromstr_p = fromstr;
  else
    fromstr_p = NULL;
 
  allvolumes = ODD( dcli_get_qualifier( "/ALLVOLUMES", 0, 0));

  if (( sts_volumes = ODD( dcli_get_qualifier( "/VOLUMES", volumestr, sizeof(volumestr)))))
    volumestr_p = volumestr;
  else
    volumestr_p = NULL;

  if ( !( sts_plcpgm || sts_hier || sts_window || sts_from || sts_volumes ||
	allvolumes))
  {
    // Compile selected plcpgm or window
    pwr_sAttrRef 	*sel_list;
    int                 *sel_is_attr;
    int			sel_cnt;
    pwr_tClassId	classid;
    int			i;
    int			size;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
    if ( EVEN(sts))
    {
      if ( modified) {
        // Compile all modified in this volume
        sts = utl_compile( wnav->ldhses, ldh_SessionToWB(wnav->ldhses), 
		plcpgmstr_p, windowstr_p, hierstr_p, fromstr_p, modified, 
		debug, allvolumes, volumestr_p);
        if ( EVEN(sts)) {
          wnav->message(' ', wnav_get_message(sts));
          return sts;
        }
        return WNAV__SUCCESS;
      }
      else {
        wnav->message( 'E', "Qualifier or selection required");
        return WNAV__QUAL;
      }
    }
    for ( i = 0; i < sel_cnt; i++)
    {
      ldh_GetObjectClass( wnav->ldhses, sel_list[i].Objid, &classid);
      if ( classid == pwr_cClass_plc)
      {
        sts = ldh_ObjidToName( wnav->ldhses, sel_list[i].Objid, 
		ldh_eName_Hierarchy, plcpgmstr, sizeof(plcpgmstr), &size);
        if ( EVEN(sts)) return sts;
        plcpgmstr_p = plcpgmstr;
        windowstr_p = NULL;
        hierstr_p = NULL;
      }
      else if ( classid == pwr_cClass_windowplc ||
                classid == pwr_cClass_windowcond ||
                classid == pwr_cClass_windoworderact ||
                classid == pwr_cClass_windowsubstep)
      {
        sts = ldh_ObjidToName( wnav->ldhses, sel_list[i].Objid, 
		ldh_eName_Hierarchy, windowstr, sizeof(windowstr), &size);
        if ( EVEN(sts)) return sts;
        windowstr_p = windowstr;
        hierstr_p = NULL;
        plcpgmstr_p = NULL;
      }
      else if ( classid == pwr_eClass_MountObject)
        continue;
      else
      {
        sts = ldh_ObjidToName( wnav->ldhses, sel_list[i].Objid, 
		ldh_eName_Hierarchy, hierstr, sizeof(hierstr), &size);
        if ( EVEN(sts)) return sts;
        hierstr_p = hierstr;
        windowstr_p = NULL;
        plcpgmstr_p = NULL;

      }
      sts = utl_compile( wnav->ldhses, ldh_SessionToWB(wnav->ldhses), 
		plcpgmstr_p, windowstr_p, hierstr_p, fromstr_p, modified, 
		debug, allvolumes, volumestr_p);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        free( sel_list);
        return sts;
      }
    }
    free( sel_list);
    free( sel_is_attr);
  }
  else
  {
    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_compile( wnav->ldhses, ldh_SessionToWB(wnav->ldhses), 
		plcpgmstr_p, windowstr_p, hierstr_p, fromstr_p, modified, 
		debug, allvolumes, volumestr_p);
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  return WNAV__SUCCESS;
}


static int	wnav_print_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char		plcpgm_str[120]; 
  char		hier_str[120]; 
  char		from_str[120];
  char		*fromstr_ptr;
  int		sts;
  int		document;
  int		overview;
  int		all;
  int		pdf;

  all = ODD( dcli_get_qualifier( "/ALL", 0, 0));
  pdf = ODD( dcli_get_qualifier( "/PDF", 0, 0));
  document = EVEN( dcli_get_qualifier( "/NODOCUMENT", 0, 0));
  overview = EVEN( dcli_get_qualifier( "/NOOVERVIEW", 0, 0));

  sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
  if ( EVEN(sts)) return sts;

  if ( ODD( dcli_get_qualifier( "/PLCPGM", plcpgm_str, sizeof(plcpgm_str)))) {
    wb_utl *utl = wnav->utl_new();
    if ( !wnav->has_window())
      utl->create_mainwindow( 0, NULL);

    sts = utl->print_plc( wnav->ldhses, ldh_SessionToWB(wnav->ldhses), 
			  plcpgm_str, document, overview, pdf);
    delete utl;
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else if ( all || ODD( dcli_get_qualifier( "/HIERARCHY", hier_str, sizeof(hier_str))))
  {
    if ( ODD( dcli_get_qualifier( "/FROM_PLCPGM", from_str, sizeof(from_str))))
      fromstr_ptr = from_str;
    else
      fromstr_ptr = NULL;
   
    wb_utl *utl = wnav->utl_new();
    if ( !wnav->has_window())
      utl->create_mainwindow( 0, NULL);

    sts = utl->print_plc_hier( wnav->ldhses, 
	ldh_SessionToWB(wnav->ldhses), hier_str, fromstr_ptr, document, 
	overview, all, pdf);
    delete utl;
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else
  {
    // Print selected plcpgm objects
    pwr_sAttrRef 	*sel_list;
    int                 *sel_is_attr;
    int			sel_cnt;
    pwr_tClassId	classid;
    int			i;
    int			size;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
    if ( EVEN(sts))
    {
      wnav->message( 'E', "Qualifier or selection required");
      return WNAV__QUAL;
    }

    for ( i = 0; i < sel_cnt; i++)
    {
      ldh_GetObjectClass( wnav->ldhses, sel_list[i].Objid, &classid);
      if ( classid == pwr_cClass_plc)
      {
        sts = ldh_ObjidToName( wnav->ldhses, sel_list[i].Objid, 
		ldh_eName_Hierarchy, plcpgm_str, sizeof(plcpgm_str), &size);
        if ( EVEN(sts)) return sts;

	wb_utl *utl = wnav->utl_new();
	if ( !wnav->has_window())
	  utl->create_mainwindow( 0, NULL);

        sts = utl->print_plc( wnav->ldhses,
		ldh_SessionToWB(wnav->ldhses), plcpgm_str, document, overview, 0);
	delete utl;
        if ( EVEN(sts)) {
          wnav->message(' ', wnav_get_message(sts));
          free( sel_list);
          return sts;
        }
      }
    }
    free( sel_list);
    free( sel_is_attr);
  }

  return WNAV__SUCCESS;
}

static int	wnav_redraw_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char		plcpgm_str[120]; 
  char		hier_str[120]; 
  char		from_str[120];
  char		*fromstr_ptr;
  int		sts;
  int           templ;

  if ( !wnav->editmode)
  {
    wnav->message('E', "Not in edit mode");
    return WNAV__NOEDIT;
  }

  if ( ODD( dcli_get_qualifier( "/ALL", 0, 0)))
  {
   
    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    templ = ODD( dcli_get_qualifier( "/TEMPLATE", from_str, sizeof(from_str)));

    if ( ODD( dcli_get_qualifier( "/FROM_PLCPGM", from_str, sizeof(from_str))))
      fromstr_ptr = from_str;
    else
      fromstr_ptr = NULL;
   
    wb_utl *utl = wnav->utl_new();
    if ( !wnav->has_window())
      utl->create_mainwindow( 0, NULL);

    sts = utl->redraw_plc_hier( wnav->ldhses, 
	ldh_SessionToWB(wnav->ldhses), NULL, fromstr_ptr, 1, templ);
    delete utl;
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else if ( ODD( dcli_get_qualifier( "/PLCPGM", plcpgm_str, sizeof(plcpgm_str))))
  {
    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    wb_utl *utl = wnav->utl_new();
    if ( !wnav->has_window())
      utl->create_mainwindow( 0, NULL);

    templ = ODD( dcli_get_qualifier( "/TEMPLATE", from_str, sizeof(from_str)));

    sts = utl->redraw_plc_hier( wnav->ldhses,
		ldh_SessionToWB(wnav->ldhses), plcpgm_str, 0, 0, templ);
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else if ( ODD( dcli_get_qualifier( "/HIERARCHY", hier_str, sizeof(hier_str))))
  {
   
    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    if ( ODD( dcli_get_qualifier( "/FROM_PLCPGM", from_str, sizeof(from_str))))
      fromstr_ptr = from_str;
    else
      fromstr_ptr = NULL;
   
    templ = ODD( dcli_get_qualifier( "/TEMPLATE", from_str, sizeof(from_str)));

    wb_utl *utl = wnav->utl_new();
    sts = utl->redraw_plc_hier( wnav->ldhses, 
	ldh_SessionToWB(wnav->ldhses), hier_str, fromstr_ptr, 0, templ);
    delete utl;
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else
  {
    // Redraw selected plcpgm objects
    pwr_sAttrRef 	*sel_list;
    int                 *sel_is_attr;
    int			sel_cnt;
    pwr_tClassId	classid;
    int			i;
    int			size;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
    if ( EVEN(sts))
    {
      wnav->message( 'E', "Qualifier or selection required");
      return WNAV__QUAL;
    }

    for ( i = 0; i < sel_cnt; i++)
    {
      ldh_GetObjectClass( wnav->ldhses, sel_list[i].Objid, &classid);
      if ( classid == pwr_cClass_plc)
      {
        sts = ldh_ObjidToName( wnav->ldhses, sel_list[i].Objid, 
		ldh_eName_Hierarchy, plcpgm_str, sizeof(plcpgm_str), &size);
        if ( EVEN(sts)) return sts;

	wb_utl *utl = wnav->utl_new();
        sts = utl->redraw_plc_hier( wnav->ldhses,
		ldh_SessionToWB(wnav->ldhses), plcpgm_str, 0, 0, 0);
	delete utl;
        if ( EVEN(sts)) {
          wnav->message(' ', wnav_get_message(sts));
          free( sel_list);
          return sts;
        }
      }
    }
    free( sel_list);
    free( sel_is_attr);
  }

  return WNAV__SUCCESS;
}

static int	wnav_generate_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char		arg1_str[80]; 

  if ( EVEN( dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str)))) {
    wnav->message( 'E', "Syntax error");
    return WNAV__SYNTAX;
  }

  if ( strncmp( arg1_str, "WEB", strlen( arg1_str)) == 0)
  {
    int sts;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = Ge::generate_web( wnav->ldhses);
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;    
  }

  return WNAV__SUCCESS;
}

static int	wnav_list_func(	void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  int	arg1_sts;
  char	file[80];
  pwr_tOName	hierstr;
  pwr_tOName	objectstr;
  char	*hierstr_p;
  char	*objectstr_p;
  int	sts;
  int	print;
  char	volumestr[80];
  char	*volumestr_p;
  int	allvolumes;
  pwr_tObjid	objid;
  pwr_tOName   	descriptor_str;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "DESCRIPTOR", strlen( arg1_str)) == 0)
  {
    char		descstr[80];

    // command is "LIST DESCRIPTOR"

    if ( ODD( dcli_get_qualifier( "/OUTPUT", file, sizeof(file))))
      print = 1;
    else
    {
      strcpy( file, "");
      print = 0;;
    }

    if ( ODD( dcli_get_qualifier( "/HIERARCHY", hierstr, sizeof(hierstr))))
      hierstr_p = hierstr;
    else
      hierstr_p = NULL;
    if ( ODD( dcli_get_qualifier( "/OBJECT", objectstr, sizeof(objectstr))))
      objectstr_p = objectstr;
    else
      objectstr_p = NULL;

    allvolumes = ODD( dcli_get_qualifier( "/ALLVOLUMES", 0, 0));

    if ( ODD( dcli_get_qualifier( "/VOLUMES" , volumestr, sizeof(volumestr))))
      volumestr_p = volumestr;
    else
      volumestr_p = NULL;


    if ( ODD( dcli_get_qualifier( "/DESCRIPTOR" , descstr, sizeof(descstr))))
    {
      // Add defaultvolume wb to descriptor
      if ( strchr( descstr, ':'))
        strcpy( descriptor_str, descstr);
      else
      {
        strcpy( descriptor_str, "wb:");
        strcat( descriptor_str, descstr);
      }

      sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
      if ( EVEN(sts)) return sts;

      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, objectstr_p, 
		file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
  }
  else if ( strncmp( arg1_str, "SIGNALS", strlen( arg1_str)) == 0)
  {
    int		shortname;

    // command is "LIST SIGNALS"

    if ( ODD( dcli_get_qualifier( "/OUTPUT", file, sizeof(file))))
      print = 1;
    else
    {
      strcpy( file, "");
      print = 0;;
    }

    if ( ODD( dcli_get_qualifier( "/HIERARCHY", hierstr, sizeof(hierstr))))
      hierstr_p = hierstr;
    else
      hierstr_p = NULL;
    objectstr_p = NULL;

    allvolumes = ODD( dcli_get_qualifier( "/ALLVOLUMES", 0, 0));
    shortname = ODD( dcli_get_qualifier( "/SHORTNAME", 0, 0));

    if ( ODD( dcli_get_qualifier( "/VOLUMES" , volumestr, sizeof(volumestr))))
      volumestr_p = volumestr;
    else
      volumestr_p = NULL;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    if ( !shortname)
    {
      // Search for local signal list descriptor
      strcpy(descriptor_str, "localWb:Lists-SignalList");
      sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
      if ( ODD(sts))
      {
        sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
        if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
        {
          wnav->message(' ', wnav_get_message(sts));
          return sts;
        }
      }
      if ( EVEN(sts))
      {
        // No local found, take global
        strcpy(descriptor_str, "wb:Lists-SignalList");
        sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
        if ( EVEN(sts))
        {
          wnav->message(' ', wnav_get_message(sts));
          return sts;
        }
      }
    }  
    else
    {
      // Search for local signal list descriptor

      strcpy( descriptor_str, "localWb:Lists-SignalListShort");
      sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
      if ( ODD(sts))
      {
        sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, objectstr_p, 
		file, print, allvolumes, volumestr_p, 0);
        if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
        {
          wnav->message(' ', wnav_get_message(sts));
          return sts;
        }
      }
      if ( EVEN(sts))
      {
        // No local found, take global
        strcpy( descriptor_str, "wb:Lists-SignalListShort");
        sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, objectstr_p, 
		file, print, allvolumes, volumestr_p, 0);
        if ( EVEN(sts))
        {
          wnav->message(' ', wnav_get_message(sts));
          return sts;
        }
      }  
    }
  }
  else if ( strncmp( arg1_str, "CHANNELS", strlen( arg1_str)) == 0)
  {

    // command is "LIST CHANNELS"

    if ( ODD( dcli_get_qualifier( "/OUTPUT", file, sizeof(file))))
      print = 1;
    else
    {
      strcpy( file, "");
      print = 0;;
    }

    if ( ODD( dcli_get_qualifier( "/HIERARCHY", hierstr, sizeof(hierstr))))
      hierstr_p = hierstr;
    else
      hierstr_p = NULL;
    if ( ODD( dcli_get_qualifier( "/NODE", objectstr, sizeof(objectstr))))
      objectstr_p = objectstr;
    else
      objectstr_p = NULL;

    allvolumes = ODD( dcli_get_qualifier( "/ALLVOLUMES", 0, 0));

    if ( ODD( dcli_get_qualifier( "/VOLUMES" , volumestr, sizeof(volumestr))))
      volumestr_p = volumestr;
    else
      volumestr_p = NULL;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    // Search for local signal list descriptor
    strcpy(descriptor_str, "localWb:Lists-ChannelList");
    sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
    if ( ODD(sts))
    {
      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
    if ( EVEN(sts))
    {
      // No local found, take global
      strcpy(descriptor_str, "wb:Lists-ChannelList");
      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
  }
  else if ( strncmp( arg1_str, "HIERARCHY", strlen( arg1_str)) == 0)
  {

    // command is "LIST HIERARCHY"

    if ( ODD( dcli_get_qualifier( "/OUTPUT", file, sizeof(file))))
      print = 1;
    else
    {
      strcpy( file, "");
      print = 0;;
    }

    if ( ODD( dcli_get_qualifier( "/HIERARCHY", hierstr, sizeof(hierstr))))
      hierstr_p = hierstr;
    else
      hierstr_p = NULL;
    objectstr_p = NULL;

    allvolumes = ODD( dcli_get_qualifier( "/ALLVOLUMES", 0, 0));

    if ( ODD( dcli_get_qualifier( "/VOLUMES" , volumestr, sizeof(volumestr))))
      volumestr_p = volumestr;
    else
      volumestr_p = NULL;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    // Search for local signal list descriptor
    strcpy(descriptor_str, "localWb:Lists-HierList");
    sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
    if ( ODD(sts))
    {
      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
    if ( EVEN(sts))
    {
      // No local found, take global
      strcpy(descriptor_str, "wb:Lists-HierList");
      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }

    // Write plcpgm list
    strcpy(descriptor_str, "localWb:Lists-PlcPgmList");
    sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
    if ( ODD(sts))
    {
      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
    if ( EVEN(sts))
    {
      // No local found, take global
      strcpy(descriptor_str, "wb:Lists-PlcPgmList");
      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
  }
  else if ( strncmp( arg1_str, "PLCPGM", strlen( arg1_str)) == 0)
  {

    // command is "LIST PLCPGM"

    if ( ODD( dcli_get_qualifier( "/OUTPUT", file, sizeof(file))))
      print = 1;
    else
    {
      strcpy( file, "");
      print = 0;;
    }

    if ( ODD( dcli_get_qualifier( "/HIERARCHY", hierstr, sizeof(hierstr))))
      hierstr_p = hierstr;
    else
      hierstr_p = NULL;
    if ( ODD( dcli_get_qualifier( "/PLCPGM", objectstr, sizeof(objectstr))))
      objectstr_p = objectstr;
    else
      objectstr_p = NULL;

    allvolumes = ODD( dcli_get_qualifier( "/ALLVOLUMES", 0, 0));

    if ( ODD( dcli_get_qualifier( "/VOLUMES" , volumestr, sizeof(volumestr))))
      volumestr_p = volumestr;
    else
      volumestr_p = NULL;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    // Search for local signal list descriptor
    strcpy(descriptor_str, "localWb:Lists-PlcModuleList");
    sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
    if ( ODD(sts))
    {
      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
    if ( EVEN(sts))
    {
      // No local found, take global
      strcpy(descriptor_str, "wb:Lists-PlcModuleList");
      sts = utl_list( wnav->ldhses, descriptor_str, hierstr_p, 
		objectstr_p, file, print, allvolumes, volumestr_p, 0);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
  }
  else if ( strncmp( arg1_str, "RTTLISTS", strlen( arg1_str)) == 0)
  {
    ldh_sVolumeInfo 	info;

    // command is "LIST RTTLISTS"

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    // Get the filename
    sts = ldh_GetVolumeInfo( ldh_SessionToVol(wnav->ldhses), &info);
    if ( EVEN(sts)) return sts;

    sprintf( file, "pwrp_load:rtt_crr_%s.dat", wnav_VolumeIdToStr( info.Volume));

    strcpy( descriptor_str, "localWb:Lists-RttSignalList");
    sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
    if ( ODD(sts))
    {
      sts = utl_list( wnav->ldhses, descriptor_str, NULL, NULL, file, 0, 0, NULL, 1);
      if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
    if ( EVEN(sts))
    {
      // No local found, take global
      strcpy(descriptor_str, "wb:Lists-RttSignalList");
      sts = utl_list( wnav->ldhses, descriptor_str, NULL, NULL, file, 0, 0, NULL, 1);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    } 

    // Create the object crossreferece list

    // Get the filename
    sts = ldh_GetVolumeInfo( ldh_SessionToVol(wnav->ldhses), &info);
    if ( EVEN(sts)) return sts;

    sprintf( file, "pwrp_load:rtt_crro_%s.dat", wnav_VolumeIdToStr( info.Volume));

    strcpy(descriptor_str, "localWb:Lists-RttObjectList");
    sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
    if ( ODD(sts))
    {
      sts = utl_list( wnav->ldhses, descriptor_str, NULL, NULL, file, 0, 0, NULL, 0);
      if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
    if ( EVEN(sts))
    {
      // No local found, take global
      strcpy(descriptor_str, "wb:Lists-RttObjectList");
      sts = utl_list( wnav->ldhses, descriptor_str, NULL, NULL, file, 0, 0, NULL, 0);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }

    // Create the code crossreference list

    // Get the filename
    sts = ldh_GetVolumeInfo( ldh_SessionToVol(wnav->ldhses), &info);
    if ( EVEN(sts)) return sts;

    sprintf( file, "pwrp_load:rtt_crrc_%s.dat", wnav_VolumeIdToStr( info.Volume));

    strcpy(descriptor_str, "localWb:Lists-RttCodeList");
    sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
    if ( ODD(sts))
    {
      sts = utl_list( wnav->ldhses, descriptor_str, NULL, NULL, file, 0, 0, NULL, 0);
      if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }
    if ( EVEN(sts))
    {
      // No local found, take global
      strcpy(descriptor_str, "wb:Lists-RttCodeList");
      sts = utl_list( wnav->ldhses, descriptor_str, NULL, NULL, file, 0, 0, NULL, 0);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }  

    // Create the plc list
    sprintf( file, "pwrp_load:rtt_plc_%s.dat", wnav_VolumeIdToStr( info.Volume));

    // Search for local descriptor
    strcpy(descriptor_str, "localWb:Lists-RttPlcList");
    sts = ldh_NameToObjid( wnav->ldhses, &objid, descriptor_str);
    if ( ODD(sts))
    {
      sts = utl_list( wnav->ldhses, descriptor_str, NULL, NULL, file, 0, 0, NULL, 0);
      if ( EVEN(sts) && ( sts != FOE__LISTOBJECT))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }

    if ( EVEN(sts))
    {
      // No local found, take global
      strcpy(descriptor_str, "wb:Lists-RttPlcList");
      sts = utl_list( wnav->ldhses, descriptor_str, NULL, NULL, file, 0, 0, NULL, 0);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        return sts;
      }
    }  
  }
  else
  {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return WNAV__SUCCESS;
}

static int	wnav_configure_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "CARD", strlen( arg1_str)) == 0)
  {
    char		rack[80]; 
    char		cardname[80];
    char		cardclass[80];
    char		channelname[80];
    char		identitystr[80];
    char		descriptionstr[80];
    char		tablestr[80];
    char		*identitystr_p;
    char		*descriptionstr_p;
    char		*tablestr_p;
    int			sts;

    // command "CONFIGURE CARD" 

    if ( !wnav->editmode)
    {
      wnav->message('E', "Not in edit mode");
      return WNAV__NOEDIT;
    }

    sts = 1;
    if ( EVEN( dcli_get_qualifier( "/RACK", rack, sizeof(rack))))
      sts = 0;
    if ( EVEN( dcli_get_qualifier( "/CARDNAME", cardname, sizeof(cardname))))
      sts = 0;
    if ( EVEN( dcli_get_qualifier( "/CARDCLASS", cardclass, sizeof(cardclass))))
      sts = 0;
    if ( EVEN( dcli_get_qualifier( "/CHANNELNAME", channelname, sizeof(channelname))))
      sts = 0;
    if ( EVEN(sts))
    {
      wnav->message('E', "Qualifier required");
      return WNAV__QUAL;
    }

    if ( ODD( dcli_get_qualifier( "/CHANIDENTITY", identitystr, sizeof(identitystr))))
      identitystr_p = identitystr;
    else
      identitystr_p = NULL;
    if ( ODD( dcli_get_qualifier( "/CHANDESCRIPTION", descriptionstr, sizeof(descriptionstr))))
      descriptionstr_p = descriptionstr;
    else
      descriptionstr_p = NULL;
    if ( ODD( dcli_get_qualifier( "/TABLE", tablestr, sizeof(tablestr))))
      tablestr_p = tablestr;
    else
      tablestr_p = NULL;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_configure_card( wnav->ldhses, rack, cardname, cardclass, 
	channelname, identitystr_p, descriptionstr_p, tablestr_p);
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else
  {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }

  return WNAV__SUCCESS;
}

static int	wnav_sort_func(	void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  pwr_tOName   	parentstr;
  int		sts;
  int		sorttype;

  if ( !wnav->editmode)
  {
    wnav->message('E', "Not in edit mode");
    return WNAV__NOEDIT;
  }

  sorttype = 0;
  if ( ODD( dcli_get_qualifier( "/SIGNALS", 0, 0)))
    sorttype = 2;
  else  if ( ODD( dcli_get_qualifier( "/CLASS", 0, 0)))
    sorttype = 3;


  if ( ODD( dcli_get_qualifier( "/PARENT", 0, 0)))
  {
    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_sortchildren( wnav->ldhses, parentstr, sorttype);
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }  
  else
  {
    // Sort children of selected objects
    pwr_sAttrRef 	*sel_list;
    int                 *sel_is_attr;
    int			sel_cnt;
    int			i;
    int			size;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
    if ( EVEN(sts))
    {
      wnav->message( 'E', "Qualifier or selection required");
      return WNAV__QUAL;
    }

    for ( i = 0; i < sel_cnt; i++)
    {
      sts = ldh_ObjidToName( wnav->ldhses, sel_list[i].Objid, 
		ldh_eName_Hierarchy, parentstr, sizeof(parentstr), &size);
      if ( EVEN(sts)) return sts;

      sts = utl_sortchildren( wnav->ldhses, parentstr, sorttype);
      if ( EVEN(sts))
      {
        wnav->message(' ', wnav_get_message(sts));
        free( sel_list);
        return sts;
      }
    }
    free( sel_list);
    free( sel_is_attr);
  }
  return WNAV__SUCCESS;
}

static int	wnav_copy_func(	void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));
  if ( EVEN(arg1_sts)) {
    pwr_sAttrRef	*sel_list;
    int                 *sel_is_attr;
    int			sel_cnt;
    int                 keepref;
    int                 ignore_errors;
    pwr_tStatus         sts;

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
    if ( EVEN(sts)) {
      wnav->message('E', "Nothing is selected in the current window");
      return sts;
    }

    keepref = ODD( dcli_get_qualifier( "/KEEPREFERENCES", 0, 0));
    ignore_errors = ODD( dcli_get_qualifier( "/IGNORE_ERRORS", 0, 0));

    sts = ldh_Copy( wnav->ldhses, sel_list, keepref, ignore_errors);
    if ( sel_cnt > 0) {
      free( sel_list);
      free( sel_is_attr);
    }
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0)
  {
    pwr_tOName	sourcestr;
    pwr_tOName	destinationstr;
    pwr_tObjName namestr;
    int		hier;
    int		last;
    int		first;
    int		after;
    int		before;
    pwr_tStatus	sts;

    // command  is "COPY OBJECT"

    if ( !wnav->editmode)
    {
      wnav->message('E', "Not in edit mode");
      return WNAV__NOEDIT;
    }

    first = ODD( dcli_get_qualifier( "/FIRST", 0, 0));
    last = ODD( dcli_get_qualifier( "/LAST", 0, 0));
    after = ODD( dcli_get_qualifier( "/AFTER", 0, 0));
    before = ODD( dcli_get_qualifier( "/BEFORE", 0, 0));
    hier = ODD( dcli_get_qualifier( "/HIERARCHY", 0, 0));

    sts = 1;
    if ( EVEN( dcli_get_qualifier( "/SOURCE", sourcestr, sizeof(sourcestr))))
      sts = 0;
    if ( EVEN( dcli_get_qualifier( "/DESTINATION", destinationstr, sizeof(destinationstr))))
      sts = 0;
    if ( EVEN( dcli_get_qualifier( "/NAME", namestr, sizeof(namestr))))
      sts = 0;

    if ( EVEN(sts))
    {
      wnav->message('E', "Qualifer required");
      return WNAV__QUAL;
    }

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_copy_objects( wnav->ldhses, sourcestr, destinationstr, namestr, 
	hier, first, last, after, before);
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else
  {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }

  return WNAV__SUCCESS;
}

static int	wnav_cut_func(	void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  pwr_sAttrRef	*sel_list;
  int                 *sel_is_attr;
  int			sel_cnt;
  int                 keepref;
  pwr_tStatus         sts;

  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
  if ( EVEN(sts)) {
    wnav->message('E', "Nothing is selected in the current window");
    return sts;
  }

  keepref = ODD( dcli_get_qualifier( "/KEEPREFERENCES", 0, 0));

  sts = ldh_Cut( wnav->ldhses, sel_list, keepref);
  if ( sel_cnt > 0) {
    free( sel_list);
    free( sel_is_attr);
  }
  if ( EVEN(sts)) {
    wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  return WNAV__SUCCESS;
}

static int	wnav_paste_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	buffer_str[80];
  char *buffer_ptr;

  pwr_sAttrRef	*sel_list;
  int                 *sel_is_attr;
  int		      sel_cnt;
  pwr_tStatus         sts;
  ldh_eDest	      dest;

  if ( ODD( dcli_get_qualifier( "/BUFFER", buffer_str, sizeof(buffer_str))))
    buffer_ptr = buffer_str;
  else if ( ODD( dcli_get_qualifier( "dcli_arg1", buffer_str, sizeof(buffer_str))))
    buffer_ptr = buffer_str;
  else
    buffer_ptr = 0;

  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  int keepoid = ODD( dcli_get_qualifier( "/KEEPOID", 0, 0));

  if ( ODD( dcli_get_qualifier( "/TOPLEVEL", 0, 0))) {
    ldh_sSessInfo info;
    pwr_tOid destoid;

    sts = ldh_GetSessionInfo( wnav->ldhses, &info);
    if ( EVEN(sts)) return sts;
    
    dest = ldh_eDest_IntoFirst;
    destoid.oix = 0;
    destoid.vid = info.Vid;
      
    sts = ldh_Paste( wnav->ldhses, destoid, dest, keepoid, 0, buffer_ptr);
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
    return WNAV__SUCCESS;
  }

  sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
  if ( EVEN(sts)) {
    wnav->message('E', "Nothing is selected in the current window");
    return sts;
  }
  else if ( sel_cnt > 1) {
    wnav->message('E', "More than one object is selected in the current window");
    return WNAV__SELTOMANY;
  }

  if ( ODD( dcli_get_qualifier( "/INTO", 0, 0)))
    dest = ldh_eDest_IntoFirst;
  else
    dest = ldh_eDest_After;

  sts = ldh_Paste( wnav->ldhses, sel_list->Objid, dest, keepoid, 0, buffer_ptr);
  if ( sel_cnt > 0) {
    free( sel_list);
    free( sel_is_attr);
  }
  if ( EVEN(sts)) {
    wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  return WNAV__SUCCESS;
}

static int	wnav_move_func(	void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0)
  {
    pwr_tOName	sourcestr;
    pwr_tOName	destinationstr;
    pwr_tObjName renamestr;
    char	*destinationstr_ptr;
    char	*rename_str_ptr;
    int		last;
    int		first;
    int		after;
    int		before;
    pwr_tStatus	sts;

    // command  is "MOVE OBJECT"

    if ( !wnav->editmode)
    {
      wnav->message('E', "Not in edit mode");
      return WNAV__NOEDIT;
    }

    first = ODD( dcli_get_qualifier( "/FIRST", 0, 0));
    last = ODD( dcli_get_qualifier( "/LAST", 0, 0));
    after = ODD( dcli_get_qualifier( "/AFTER", 0, 0));
    before = ODD( dcli_get_qualifier( "/BEFORE", 0, 0));

    sts = 1;
    if ( EVEN( dcli_get_qualifier( "/SOURCE", sourcestr, sizeof(sourcestr))))
      sts = 0;

    if ( ODD( dcli_get_qualifier( "/DESTINATION", destinationstr, sizeof(destinationstr))))
      destinationstr_ptr = destinationstr;
    else
      destinationstr_ptr = NULL;    
    if ( ODD( dcli_get_qualifier( "/RENAME", renamestr, sizeof(renamestr))))
      rename_str_ptr = renamestr;
    else
      rename_str_ptr = NULL;    

    if ( EVEN(sts))
    {
      wnav->message('E', "Qualifer required");
      return WNAV__QUAL;
    }

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_move_object( wnav->ldhses, sourcestr, destinationstr_ptr, 
		rename_str_ptr, first, last, after, before);
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else
  {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return WNAV__SUCCESS;
}

static int	wnav_open_func(	void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "BUFFERS", strlen( arg1_str)) == 0)
  {
    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( wnav->open_vsel_cb)
      (wnav->open_vsel_cb)( wnav->parent_ctx, wb_eType_Buffer, NULL, wow_eFileSelType_All);
  }
  else if ( strncmp( arg1_str, "DATABASE", strlen( arg1_str)) == 0)
  {
    char	volumestr[80];
    pwr_tFileName filename;
    pwr_tCmd cmd;
    int sts;

    if ( EVEN( dcli_get_qualifier( "/VOLUME", volumestr, sizeof(volumestr)))) {
      wnav->message('E', "Volume is missing");
      return WNAV__SYNTAX;
    }


    dcli_translate_filename( filename, "$pwr_exe/wb_open_db.sh");
    sprintf( cmd,
	     "%s \"%s\" \"%s\" \"%s\" \"%s\" &",
	     filename, CoLogin::username(), CoLogin::ucpassword(), volumestr, volumestr);

    sts = system( cmd);
    if ( sts == -1 || sts == 127) {
      wnav->message( 'E', "Error when creating process");
      return sts;
    }
  }
  else if ( strncmp( arg1_str, "CLASSEDITOR", strlen( arg1_str)) == 0)
  {
    pwr_tFileName	filenamestr;

    // Command is "OPEN CLASSEDITOR" 

    if ( ODD( dcli_get_qualifier( "/FILE", filenamestr, sizeof(filenamestr))))
      cdh_ToLower( filenamestr, filenamestr);
    else {
      wnav->message('E', "File is missing");
      return WNAV__SYNTAX;
    }

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( wnav->open_vsel_cb)
      (wnav->open_vsel_cb)( wnav->parent_ctx, wb_eType_ClassEditor, filenamestr, wow_eFileSelType_WblClass);
  }
  else if ( strncmp( arg1_str, "GRAPH", strlen( arg1_str)) == 0)
  {
    pwr_tFileName	filenamestr;
    char		titlestr[80];
    pwr_tAName		instancestr;
    char		*instance_p;
    WGe 		*wge;
    int			modal;
    char		*s;
    char 		applname[80];

    // Command is "OPEN GRAPH" 

    if ( EVEN( dcli_get_qualifier( "/FILE", filenamestr, sizeof(filenamestr)))) {
      if ( EVEN( dcli_get_qualifier( "dcli_arg2", filenamestr, sizeof(filenamestr)))) {
	wnav->message('E', "File is missing");
	return WNAV__SYNTAX;
      }
    }
    cdh_ToLower( filenamestr, filenamestr);

    if ( ODD( dcli_get_qualifier( "/INSTANCE", instancestr, sizeof(instancestr))))
      instance_p = instancestr;
    else
      instance_p = 0;

    if ( EVEN( dcli_get_qualifier( "/TITLE", titlestr, sizeof(titlestr))))
      strcpy( titlestr, "Pwr");

    modal = ODD( dcli_get_qualifier( "/MODAL", 0, 0));

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;


    if ( (s = strrchr( filenamestr, '/')))
      strcpy( applname, s + 1);
    else
      strcpy( applname, filenamestr);
    if ( (s = strchr( applname, '.')))
      *s = 0;

    wge = wnav->wge_new( titlestr, filenamestr, instance_p, modal);
    wge->command_cb = wnav_wge_command_cb;
    wnav->appl.insert( applist_eType_Graph, (void *)wge, pwr_cNObjid, applname);
    if ( modal) {
      wnav->wge_modal_loop( wge);
    }
    else {
      wge->close_cb = wnav_wge_close_cb;
    }
  }
  else
  {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return WNAV__SUCCESS;
}

static int	wnav_close_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "GRAPH", strlen( arg1_str)) == 0)
  {
    pwr_tFileName	filenamestr;
    pwr_tOName		instancestr;
    WGe 		*wge;

    // Command is "CLOSE GRAPH" 

    if ( EVEN( dcli_get_qualifier( "/FILE", filenamestr, sizeof(filenamestr)))) {
      if ( EVEN( dcli_get_qualifier( "dcli_arg2", filenamestr, sizeof(filenamestr)))) {
	wnav->message('E', "File is missing");
	return WNAV__SYNTAX;
      }
    }
    cdh_ToLower( filenamestr, filenamestr);
    if ( ODD( dcli_get_qualifier( "/INSTANCE", instancestr, sizeof(instancestr)))) {
    }

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( wnav->appl.find( applist_eType_Graph, filenamestr, (void **) &wge)) {
      wnav->appl.remove( (void *)wge);
      delete wge;
    }
  }
  else {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return WNAV__SUCCESS;
}

static int	wnav_create_func( void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  int	sts;
  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "ITEM", strlen( arg1_str)) == 0)
  {
    char text_str[80];
    char destination_str[80];
    char command_str[80];
    int dest_code;
    int item_type;
    wnav_sMenu *menu_item;

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    // Command is "CREATE ITEM"
    if ( ODD( dcli_get_qualifier( "dcli_arg2", text_str, sizeof(text_str))))
    {
      if ( text_str[0] == '/')
      {
        wnav->message('E', "Syntax error");
        return WNAV__SYNTAX;
      }
    }
    else if ( EVEN( dcli_get_qualifier( "/TEXT", text_str, sizeof(text_str))))
    {
      wnav->message('E',"Enter text");
      return WNAV__QUAL;
    }
    if ( EVEN( dcli_get_qualifier( "/DESTINATION", destination_str, sizeof(destination_str))))
    {
      wnav->message('E',"Enter destination");
      return WNAV__QUAL;
    }
    if ( ODD( dcli_get_qualifier( "/COMMAND", command_str, sizeof(command_str))))
      item_type = wnav_eItemType_Command;
    if ( ODD( dcli_get_qualifier( "/MENU", 0, 0)))
      item_type = wnav_eItemType_Menu;
    if ( ODD( dcli_get_qualifier( "/AFTER", 0, 0)))
      dest_code = wnav_eDestCode_After;
    else if ( ODD( dcli_get_qualifier( "/BEFORE", 0, 0)))
      dest_code = wnav_eDestCode_Before;
    else if ( ODD( dcli_get_qualifier( "/FIRSTCHILD", 0, 0)))
      dest_code = wnav_eDestCode_FirstChild;
    else if ( ODD( dcli_get_qualifier( "/LASTCHILD", 0, 0)))
      dest_code = wnav_eDestCode_LastChild;
    else
      dest_code = wnav_eDestCode_After;

    sts = wnav->menu_tree_insert( text_str, item_type, command_str,
		destination_str, dest_code, &menu_item);
    if ( EVEN(sts))
      wnav->message('E',"Destination not found");

    return sts;
  }
  else if ( strncmp( arg1_str, "STRUCTFILES", strlen( arg1_str)) == 0)
  {
    pwr_tFileName filenamestr;
    pwr_tStatus	sts;
    pwr_tCmd      cmd;

    // Command is "CREATE STRUCTFILES" 

    if ( ODD( dcli_get_qualifier( "/VOLUME", 0, 0)))
    {
      wnav->message('E',"Qualifier /VOLUME is obsolete");
      return WNAV__QUAL;
    }

    if ( ODD( dcli_get_qualifier( "/FILES", filenamestr, sizeof(filenamestr))))
      cdh_ToLower( filenamestr, filenamestr);
    else
      // userclasses.wb_load is default filename
      strcpy( filenamestr, "$pwrp_db/userclasses.wb_load");

    // Create structfiles
    sprintf( cmd, "co_convert -so -d $pwrp_inc/ %s", filenamestr);
    sts = system( cmd);
    if ( sts != 0) {
      wnav->message('E', "Conversion error, see terminal window for more information");
      return WNAV__CO_CONVERT;
    }

    // Create hpp file
    sprintf( cmd, "co_convert -po -d $pwrp_inc/ %s", filenamestr);
    sts = system( cmd);
    if ( sts != 0) {
      wnav->message('E', "Conversion error, see terminal window for more information");
      return WNAV__CO_CONVERT;
    }

    // Create html-files
    sprintf( cmd, "co_convert -wv -d $pwrp_web/ %s", filenamestr);
    sts = system( cmd);
    if ( sts != 0) {
      wnav->message('E', "Conversion error, see terminal window for more information");
      return WNAV__CO_CONVERT;
    }

    // Create xtt help-files
    sprintf( cmd, "co_convert -xv -d $pwrp_exe/ %s", filenamestr);
    sts = system( cmd);
    if ( sts != 0) {
      wnav->message('E', "Conversion error, see terminal window for more information");
      return WNAV__CO_CONVERT;
    }



    return WNAV__SUCCESS;
  }
  else if ( strncmp( arg1_str, "LOADFILES", strlen( arg1_str)) == 0)
  {
    char	volumestr[160];
    char	*volumestr_p;
    pwr_tStatus	sts;
    int		allvolumes;

    // Command is "CREATE LOADFILES"

    if ( !wnav->editmode)
    {
      wnav->message('E', "Not in edit mode");
      return WNAV__NOEDIT;
    }

    allvolumes = ODD( dcli_get_qualifier( "/ALL", 0, 0));

    if ( ODD( dcli_get_qualifier( "/VOLUME" , volumestr, sizeof(volumestr))))
      volumestr_p = volumestr;
    else
      volumestr_p = NULL;

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_create_loadfiles( wnav->ldhses, volumestr_p, allvolumes);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "BOOTFILES", strlen( arg1_str)) == 0)
  {
    int		debug;
    char	nodestr[160];
    char	*nodestr_p;
    pwr_tStatus	sts;
    int		allnodes;

    // Command is "CREATE BOOTFILES"

    allnodes = ODD( dcli_get_qualifier( "/ALL", 0, 0));
    debug = ODD( dcli_get_qualifier( "/DEBUG", 0, 0));

    if ( ODD( dcli_get_qualifier( "/NODECONFIG" , nodestr, sizeof(nodestr))))
      nodestr_p = nodestr;
    else if ( ODD( dcli_get_qualifier( "dcli_arg2", nodestr, sizeof(nodestr))))
      nodestr_p = nodestr;
    else
      nodestr_p = NULL;

    sts = lfu_create_bootfiles( nodestr_p, debug, allnodes);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0)
  {
    pwr_tOName	namestr;
    pwr_tObjName classtr;
    pwr_tOName	destinationstr;
    int		last;
    int		first;
    int		after;
    int		before;
    pwr_tStatus	sts;

    // Command is "CREATE OBJECT"
    first = ODD( dcli_get_qualifier( "/FIRST", 0, 0));
    last = ODD( dcli_get_qualifier( "/LAST", 0, 0));
    after = ODD( dcli_get_qualifier( "/AFTER", 0, 0));
    before = ODD( dcli_get_qualifier( "/BEFORE", 0, 0));

    sts = 1;
    if ( EVEN( dcli_get_qualifier( "/NAME", namestr, sizeof(namestr))))
      sts = 0;

    if ( EVEN( dcli_get_qualifier( "/CLASS", classtr, sizeof(classtr))))
      sts = 0;

    if ( EVEN( dcli_get_qualifier( "/DESTINATION", destinationstr, sizeof(destinationstr))))
      sts = 0;

    if ( EVEN(sts))
    {
      wnav->message('E', "Qualifer required");
      return WNAV__QUAL;
    }

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_create_object( wnav->ldhses, namestr, classtr, destinationstr, 
		first, last, after, before);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;

  }
  else if ( strncmp( arg1_str, "VOLUME", strlen( arg1_str)) == 0)
  {
    pwr_tObjName namestr;
    pwr_tObjName classtr;
    pwr_tString40 serverstr;
    char        databasestr[80];
    char	identitystr[80];
    pwr_tStatus	sts;
    pwr_tCid	cid;
    pwr_tVid	vid;
    ldh_eVolRep volrep;

    // Command is "CREATE VOLUME"

    if ( ODD( dcli_get_qualifier( "/DATABASE", databasestr, sizeof(databasestr)))) {
      if ( strcmp( databasestr, "BERKELEYDB") == 0)
	volrep = ldh_eVolRep_Db;
      else if ( strcmp( databasestr, "MYSQL") == 0)
	volrep = ldh_eVolRep_Dbms;
      else {
	wnav->message('E', "Syntax error in database");
	return WNAV__QUAL;
      }
    }
    else
      sts = 0;
    
    if ( volrep == ldh_eVolRep_Dbms) {
      if ( EVEN( dcli_get_qualifier( "/SERVER", serverstr, sizeof(serverstr))))
	strcpy( serverstr, "");
    }
    else
      strcpy( serverstr, "");
	

    if ( ODD( dcli_get_qualifier( "/DIRECTORY", 0, 0))) {
      strcpy( namestr, "directory");
      vid = ldh_cDirectoryVolume;
      cid = pwr_eClass_DirectoryVolume;

      sts = wnav_wccm_get_wbctx_cb( wnav, &wnav->wbctx);
      // sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
      if ( EVEN(sts)) return sts;
    }
    else {      
      sts = 1;
      if ( EVEN( dcli_get_qualifier( "/NAME", namestr, sizeof(namestr))))
	sts = 0;

      if ( EVEN( dcli_get_qualifier( "/CLASS", classtr, sizeof(classtr))))
	sts = 0;
      
      if ( EVEN( dcli_get_qualifier( "/IDENTITY", identitystr, sizeof(identitystr))))
	sts = 0;

      if ( EVEN(sts)) {
	wnav->message('E', "Qualifer required");
	return WNAV__QUAL;
      }

      sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
      if ( EVEN(sts)) return sts;

      sts = ldh_ClassNameToId( wnav->ldhses, &cid, classtr);
      if ( EVEN(sts)) {
	wnav->message(' ', wnav_get_message(sts));
	return sts;
      }
    
      sts = cdh_StringToVolumeId( identitystr, &vid);
      if ( EVEN(sts)) {
	wnav->message(' ', wnav_get_message(sts));
	return sts;
      }
    }

    try {
      wb_erep *erep = *(wb_env *)wnav->wbctx;
      erep->createVolume( &sts, vid, cid, namestr, volrep, serverstr);
    }
    catch ( wb_error &e) {
      sts = e.sts();
    }
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "RTTFILES", strlen( arg1_str)) == 0)
  {
    char cmd[80] = "CREATE CROSSREFERENCEFILES";
    sts = wnav->command( cmd);
    if ( EVEN(sts)) return sts;
  }
  else if ( strncmp( arg1_str, "CROSSREFERENCEFILES", strlen( arg1_str)) == 0)
  {
    pwr_tStatus sts;
    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    wb_crrgen crrgen( (wb_session *)wnav->ldhses);
    crrgen.load( &sts);
    crrgen.write( &sts);
    crrgen.write_code( &sts);
  }
  else if ( strncmp( arg1_str, "SNAPSHOT", strlen( arg1_str)) == 0)
  {
    pwr_tFileName	filestr;
    pwr_tFileName	outstr;
    char        *outstr_p;
    int         ignore;
    int		buildversion;
    pwr_tStatus	sts;
    pwr_tTime	buildtime, *timep;

    // Command is "CREATE SNAPSHOT"

    if ( EVEN( dcli_get_qualifier( "/FILES", filestr, sizeof(filestr))))
    {
      wnav->message('E', "Qualifer required");
      return WNAV__QUAL;
    }

    if ( ODD( dcli_get_qualifier( "/OUT", outstr, sizeof(outstr))))
      outstr_p = outstr;
    else
      outstr_p = 0;

    ignore = ODD( dcli_get_qualifier( "/IGNORE", 0, 0));
    buildversion = ODD( dcli_get_qualifier( "/BUILDVERSION", 0, 0));

    if ( buildversion) {
      time_AsciiToA( pwrv_cBuildTimeStr, &buildtime);
      timep = &buildtime;
    }
    else
      timep = 0;

    sts = wnav_wccm_get_wbctx_cb( wnav, &wnav->wbctx);
    if ( EVEN(sts)) return sts;

    sts = WNAV__SUCCESS;
    try {
      wb_erep *erep = *(wb_env *)wnav->wbctx;
      wb_vrepwbl *wbl = new wb_vrepwbl(erep);
      wbl->ref();
      sts = wbl->load( filestr);
      if ( ODD(sts) || ignore)
	wbl->createSnapshot( outstr_p, timep);
      delete wbl;
    }
    catch ( wb_error &e) {
      sts = e.sts();
    }
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "FLOWFILES", strlen( arg1_str)) == 0) {
    char		plcpgm_str[120]; 
    char		hier_str[120]; 
    char		from_str[120];
    char		*fromstr_ptr;
    int			sts;
    int			templ;

    if ( !wnav->editmode) {
      wnav->message('E', "Not in edit mode");
      return WNAV__NOEDIT;
    }

    if ( ODD( dcli_get_qualifier( "/ALL", 0, 0))) {
      templ = ODD( dcli_get_qualifier( "/TEMPLATE", 0, 0));
   
      sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
      if ( EVEN(sts)) return sts;

      if ( ODD( dcli_get_qualifier( "/FROM_PLCPGM", from_str, sizeof(from_str))))
	fromstr_ptr = from_str;
      else
	fromstr_ptr = NULL;
      
      wb_utl *utl = wnav->utl_new();
      if ( !wnav->has_window())
	utl->create_mainwindow( 0, NULL);

      sts = utl->create_flow_plc( wnav->ldhses, ldh_SessionToWB(wnav->ldhses), 
				  NULL, fromstr_ptr, 1, templ);
      delete utl;
      if ( EVEN(sts)) {
	wnav->message(' ', wnav_get_message(sts));
	return sts;
      }
    }
    else if ( ODD( dcli_get_qualifier( "/PLCPGM", plcpgm_str, sizeof(plcpgm_str)))) {
      sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
      if ( EVEN(sts)) return sts;

      wb_utl *utl = wnav->utl_new();
      if ( !wnav->has_window())
	utl->create_mainwindow( 0, NULL);

      sts = utl->create_flow_plc( wnav->ldhses, ldh_SessionToWB(wnav->ldhses), 
				  plcpgm_str, 0, 0, templ);
      if ( EVEN(sts)) {
	wnav->message(' ', wnav_get_message(sts));
	return sts;
      }
    }
    else if ( ODD( dcli_get_qualifier( "/HIERARCHY", hier_str, sizeof(hier_str)))) {

      templ = ODD( dcli_get_qualifier( "/TEMPLATE", 0, 0));
   
      sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
      if ( EVEN(sts)) return sts;

      if ( ODD( dcli_get_qualifier( "/FROM_PLCPGM", from_str, sizeof(from_str))))
	fromstr_ptr = from_str;
      else
	fromstr_ptr = NULL;
      
      wb_utl *utl = wnav->utl_new();
      sts = utl->create_flow_plc( wnav->ldhses, ldh_SessionToWB(wnav->ldhses), 
				  hier_str, fromstr_ptr, 0, templ);
      delete utl;
      if ( EVEN(sts)) {
	wnav->message(' ', wnav_get_message(sts));
	return sts;
      }
    }
    else {
      // Redraw selected plcpgm objects
      pwr_sAttrRef 	*sel_list;
      int                 *sel_is_attr;
      int			sel_cnt;
      pwr_tClassId	classid;
      int			i;
      int			size;

      templ = ODD( dcli_get_qualifier( "/TEMPLATE", 0, 0));

      sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
      if ( EVEN(sts)) return sts;

      sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
      if ( EVEN(sts)) {
	wnav->message( 'E', "Qualifier or selection required");
	return WNAV__QUAL;
      }

      for ( i = 0; i < sel_cnt; i++) {
	ldh_GetObjectClass( wnav->ldhses, sel_list[i].Objid, &classid);
	if ( classid == pwr_cClass_plc) {
	  sts = ldh_ObjidToName( wnav->ldhses, sel_list[i].Objid, 
				 ldh_eName_Hierarchy, plcpgm_str, sizeof(plcpgm_str), &size);
	  if ( EVEN(sts)) return sts;

	  wb_utl *utl = wnav->utl_new();
	  sts = utl->create_flow_plc( wnav->ldhses, ldh_SessionToWB(wnav->ldhses), 
				      plcpgm_str, 0, 0, templ);
	  delete utl;
	  if ( EVEN(sts)) {
	    wnav->message(' ', wnav_get_message(sts));
	    free( sel_list);
	    return sts;
	  }
	}
      }
      free( sel_list);
      free( sel_is_attr);
    }
  }
  else
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }

  return WNAV__SUCCESS;	
}

static int	wnav_new_func( void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  int	sts;
  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "BUFFER", strlen( arg1_str)) == 0)
  {
    char name_str[80];
    char *name_ptr = 0;

    sts = wnav_wccm_get_wbctx_cb( wnav, &wnav->wbctx);
    if ( EVEN(sts)) return sts;

    // Command is "NEW BUFFER"
    if ( ODD( dcli_get_qualifier( "dcli_arg2", name_str, sizeof(name_str))))
    {
      if ( name_str[0] == '/')
      {
        wnav->message('E', "Syntax error");
        return WNAV__SYNTAX;
      }
    }
    else if ( EVEN( dcli_get_qualifier( "/NAME", name_str, sizeof(name_str))))
    {
      // Use defaultname from erep
      name_ptr = name_str;
    }

    wb_erep *erep = *(wb_env *)wnav->wbctx;
    pwr_tVid vid = erep->nextVolatileVid( &sts, name_ptr);
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
    wb_vrepmem *mem = new wb_vrepmem(erep, vid);
    mem->name( name_str);
    erep->addBuffer( &sts, mem);

    return sts;	
  }
  else
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }

  return WNAV__SUCCESS;	
}

static int	wnav_delete_func( void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  int	sts;
  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "ITEM", strlen( arg1_str)) == 0)
  {
    char name_str[80];

    // Command is "DELETE ITEM"
    if ( ODD( dcli_get_qualifier( "dcli_arg2", name_str, sizeof(name_str))))
    {
      if ( name_str[0] == '/')
      {
        wnav->message('E', "Syntax error");
        return WNAV__SYNTAX;
      }
    }
    else if ( EVEN( dcli_get_qualifier( "/NAME", name_str, sizeof(name_str))))
    {
      wnav->message('E',"Enter name");
      return WNAV__QUAL;
    }

    sts = wnav->menu_tree_delete( name_str);
    if ( EVEN(sts))
      wnav->message('E',"WItem not found");

    return sts;	
  }
  else if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0)
  {
    pwr_tOName	namestr;
    pwr_tObjName classtr;
    pwr_tOName	hierstr;
    char	*namestr_ptr;
    char	*classtr_ptr;
    char	*hierstr_ptr;
    int		confirm;
    int		log;
    pwr_tStatus	sts;

    // Command is "DELETE OBJECT"

   if ( ODD( dcli_get_qualifier( "/NAME" , namestr, sizeof(namestr))))
      namestr_ptr = namestr;
    else
      namestr_ptr = NULL;    
   
    if ( ODD( dcli_get_qualifier( "/CLASS" , classtr, sizeof(classtr))))
      classtr_ptr = classtr;
    else
      classtr_ptr = NULL;    

    if ( ODD( dcli_get_qualifier( "/HIERARCHY" , hierstr, sizeof(hierstr))))
      hierstr_ptr = hierstr;
    else
      hierstr_ptr = NULL;    

    confirm = EVEN( dcli_get_qualifier( "/NOCONFIRM", 0, 0));
    log = EVEN( dcli_get_qualifier( "/NOLOG", 0, 0));

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_delete_objects( wnav->ldhses, hierstr_ptr, classtr_ptr, namestr_ptr, 
			confirm, log); 
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "TREE", strlen( arg1_str)) == 0)
  {
    pwr_tOName	namestr;
    char	*namestr_ptr;
    int		confirm;
    int		log;
    pwr_tStatus	sts;

    // Command is "DELETE TREE"

    if ( ODD( dcli_get_qualifier( "/NAME" , namestr, sizeof(namestr))))
      namestr_ptr = namestr;
    else
    {
      wnav->message('E', "Qualifer required");
      return WNAV__QUAL;
    }
   
    confirm = EVEN( dcli_get_qualifier( "/NOCONFIRM", 0, 0));
    log = EVEN( dcli_get_qualifier( "/NOLOG", 0, 0));

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    if ( EVEN(sts)) return sts;

    sts = utl_delete_tree( wnav->ldhses, namestr_ptr, confirm, log); 
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "VOLUME", strlen( arg1_str)) == 0)
  {
    pwr_tObjName namestr;
    char	*namestr_ptr;
    int		confirm;
    int		log;
    pwr_tStatus	sts;

    // Command is "DELETE VOLUME"

    if ( ODD( dcli_get_qualifier( "/NAME" , namestr, sizeof(namestr))))
      namestr_ptr = namestr;
    else
    {
      wnav->message('E', "Qualifer required");
      return WNAV__QUAL;
    }
   
    confirm = EVEN( dcli_get_qualifier( "/NOCONFIRM", 0, 0));
    log = EVEN( dcli_get_qualifier( "/NOLOG", 0, 0));

    sts = wnav_wccm_get_wbctx_cb( wnav, &wnav->wbctx);
    if ( EVEN(sts)) return sts;

    sts = utl_delete_volume( wnav->wbctx, namestr_ptr, 
			confirm, log); 
    if ( EVEN(sts))
    {
      wnav->message(' ', wnav_get_message(sts));
      return sts;
    }
  }
  else
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }

  return WNAV__SUCCESS;	
}

static int	wnav_connect_func( void		*client_data,
				void		*client_flag)
{
  WNav 		*wnav = (WNav *)client_data;

  char		source_str[80];
  char		dest_str[80];
  pwr_tStatus	sts;
  int		reconnect;

  // Command is "CONNECT"

  sts = 1;
  if ( EVEN( dcli_get_qualifier( "/SOURCE" , source_str, sizeof(source_str))))
    sts = 0;

  if ( EVEN( dcli_get_qualifier( "/DESTINATION" , dest_str, sizeof(dest_str))))
    sts = 0;
  if ( EVEN(sts))
  {
    wnav->message('E', "Qualifer required");
    return WNAV__QUAL;
  }

  reconnect = ODD( dcli_get_qualifier( "/RECONNECT" , 0, 0));

  sts = utl_connect( wnav->ldhses, source_str, dest_str, reconnect);
  if ( EVEN(sts))
    wnav->message(' ', wnav_get_message(sts));
  return sts;

}

static int	wnav_disconnect_func( void	*client_data,
				void		*client_flag)
{
  WNav 		*wnav = (WNav *)client_data;

  char		source_str[80];
  pwr_tStatus	sts;

  // Command is "DISCONNECT"

  if ( EVEN( dcli_get_qualifier( "/SOURCE" , source_str, sizeof(source_str))))
  {
    wnav->message('E', "Qualifer required");
    return WNAV__QUAL;
  }

  sts = utl_disconnect( wnav->ldhses, source_str);
  if ( EVEN(sts))
    wnav->message(' ', wnav_get_message(sts));
  return sts;
}

static int	wnav_wb_func( 	void		*client_data,
				void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "DUMP", strlen( arg1_str)) == 0)
  {
    char	hierarchystr[80]; 
    char	outputstr[200]; 
    char        *hierarchystr_p;
    pwr_tStatus	sts;
    int		keepname;
    int		noindex;
    int		nofocode;

    if ( EVEN( dcli_get_qualifier( "/OUTPUT" , outputstr, sizeof(outputstr))))
    {
      wnav->message('E', "Qualifer required");
      return WNAV__QUAL;
    }

    if ( ODD( dcli_get_qualifier( "/HIERARCHY" , hierarchystr, sizeof(hierarchystr))))
      hierarchystr_p = hierarchystr;
    else
      hierarchystr_p = 0;

    keepname = ODD( dcli_get_qualifier( "/KEEPNAME", 0, 0));
    noindex = ODD( dcli_get_qualifier( "/NOINDEX", 0, 0));
    nofocode = ODD( dcli_get_qualifier( "/NOFOCODE", 0, 0));

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
    
    sts = ldh_WbDump( wnav->ldhses, hierarchystr_p, outputstr, keepname, noindex,
		      nofocode);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( strncmp( arg1_str, "LOAD", strlen( arg1_str)) == 0)
  {
    pwr_tStatus sts;
    char loadfilestr[80];
    int noindex;

    if ( EVEN( dcli_get_qualifier( "/LOADFILE" , loadfilestr, sizeof(loadfilestr))))
    {
      wnav->message('E', "Qualifer required");
      return WNAV__QUAL;
    }
    noindex = ODD( dcli_get_qualifier( "/NOINDEX", 0, 0));

    sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);

    sts = ldh_WbLoad( wnav->ldhses, loadfilestr, noindex);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else
  {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return 1;
}

static int	wnav_exit_func(	void		*client_data,
				void		*client_flag)
{
  WNav 		*wnav = (WNav *)client_data;

  if ( wnav->close_cb)
    (wnav->close_cb)( wnav->parent_ctx);
  else
#if defined OS_VMS
    exit(1);
#else
    exit(0);
#endif
  return 1;
}

static int	wnav_setup_func(	void		*client_data,
					void		*client_flag)
{
  WNav 		*wnav = (WNav *)client_data;

  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  wnav->setup();
  return 1;
}

static int	wnav_search_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  int	sts;
  char	arg1_str[80];
  int	regexp;
	
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  if ( ODD( dcli_get_qualifier( "/NEXT", 0, 0)))
  {
    sts = wnav->search_next();
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else if ( ODD( dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str))))
  {
    regexp =  ODD( dcli_get_qualifier( "/REGULAREXPRESSION", 0, 0));

    sts = wnav->search( arg1_str, regexp);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
    return sts;
  }
  else
  {
    wnav->message('E',"Syntax error");
    return WNAV__SYNTAX;
  }
}

static int	wnav_save_func(		void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  int quiet;

  quiet =  ODD( dcli_get_qualifier( "/QUIET", 0, 0));

  (wnav->save_cb)( wnav->parent_ctx, quiet);
  return 1;
}

static int	wnav_revert_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;
  int	confirm;

  confirm = EVEN( dcli_get_qualifier( "/NOCONFIRM", 0, 0));

  (wnav->revert_cb)( wnav->parent_ctx, confirm);
  return 1;
}

static int	wnav_crossref_func(	void		*client_data,
					void		*client_flag)
{
  WNav 		*wnav = (WNav *)client_data;
  int		sts;
  pwr_tFileName	file_str;
  char		string_str[80];
  char		func_str[80];
  char		*file_ptr;
  pwr_tOName    name_str;

  if ( ODD( dcli_get_qualifier( "/FILE", file_str, sizeof(file_str))))
    file_ptr = file_str;
  else
    file_ptr = NULL;

  if ( ODD( dcli_get_qualifier( "/STRING", string_str, sizeof(string_str))))
  {
    int		brief;
    int		case_sens;

    if ( ODD( dcli_get_qualifier( "/FUNCTION", func_str, sizeof(func_str))))
    {
      wnav->message('E', "Syntax error");
      return WNAV__SUCCESS;
    }
    brief = ODD( dcli_get_qualifier( "/BRIEF", 0, 0));
    case_sens = ODD( dcli_get_qualifier( "/CASE_SENSITIVE", 0, 0));
    sts = wnav->crr_code( file_ptr, string_str, brief, 0, case_sens);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
  }
  else if ( ODD( dcli_get_qualifier( "/FUNCTION", func_str, sizeof(func_str))))
  {
    int		brief;
    int		case_sens;

    brief = ODD( dcli_get_qualifier( "/BRIEF", 0, 0));
    case_sens = ODD( dcli_get_qualifier( "/CASE_SENSITIVE", 0, 0));

    sts = wnav->crr_code( file_ptr, func_str, brief, 1, case_sens);
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
  }
  if ( ODD( dcli_get_qualifier( "/NAME", name_str, sizeof(name_str))))
  {
    pwr_sAttrRef objar;
    pwr_tClassId classid;
    char *namep;
    int size;

    sts = ldh_NameToAttrRef( wnav->ldhses, name_str, &objar);
    if ( EVEN(sts)) {
      wnav->message(' ', wnav_get_message(sts));
      return WNAV__SUCCESS;
    }
    sts = ldh_AttrRefToName( wnav->ldhses, &objar, ldh_eName_Hierarchy, 
	&namep, &size);
    if ( EVEN(sts)) return sts;

    printf( "Crossreferences for %s\n\n", namep);

    sts = ldh_GetAttrRefTid( wnav->ldhses, &objar, &classid);
    if ( EVEN(sts)) return sts;

    switch ( classid) {
      case pwr_cClass_Di:
      case pwr_cClass_Dv:
      case pwr_cClass_Do:
      case pwr_cClass_Po:
      case pwr_cClass_Av:
      case pwr_cClass_Ai:
      case pwr_cClass_Ao:
      case pwr_cClass_Iv:
      case pwr_cClass_Ii:
      case pwr_cClass_Io:
      case pwr_cClass_Co:
      case pwr_cClass_Sv:
      case pwr_cClass_ATv:
      case pwr_cClass_DTv:
        break;
      default:
        sts = wnav->crr_object( NULL, name_str, NULL);
    }
    if ( EVEN(sts))
      wnav->message(' ', wnav_get_message(sts));
  }
  else {
    /* Get the selected object */
    brow_tNode	*node_list;
    int		node_count;
    WItem		*item;

    brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
    if ( !node_count) {
      wnav->message('E', "Enter name or select an object");
      return WNAV__SUCCESS;
    }
    brow_GetUserData( *node_list, (void **)&item);

    switch( item->type)
    {
      case wnav_eItemType_Object: 
  	sts = ((WItemBaseObject *)item)->open_crossref( wnav, 0, 0);
        break;
      case wnav_eItemType_AttrObject: 
  	sts = ((WItemAttrObject *)item)->open_crossref( wnav, 0, 0);
        break;
      default:
        ;
    }
  }
  return sts;	  
}

static int	wnav_distribute_func(	void		*client_data,
					void		*client_flag)
{
  WNav 		*wnav = (WNav *)client_data;
  int		sts;
  char		*node_ptr;
  char		node_str[80];

  if ( ODD( dcli_get_qualifier( "/NODE", node_str, sizeof(node_str))))
    node_ptr = node_str;
  else
    node_ptr = NULL;

  sts = WNAV__SUCCESS;
  try {
    wb_pkg *pkg = new wb_pkg( node_ptr);
    delete pkg;
  }
  catch ( wb_error &e) {
    wnav->message(' ', (char *)e.what().c_str());
    sts = e.sts();
  }
  if ( EVEN(sts))
    return sts;
  return 1;
}

static int	wnav_release_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  int	arg1_sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));

  if ( strncmp( arg1_str, "SUBWINDOW", strlen( arg1_str)) == 0) {
    WGe *wge;
    char filenamestr[80];

    if ( EVEN( dcli_get_qualifier( "dcli_arg2", filenamestr, sizeof(filenamestr)))) {
      wnav->message('E', "Syntax error");
      return WNAV__SYNTAX;
    }
    cdh_ToLower( filenamestr, filenamestr);

    if ( wnav->window_type == wnav_eWindowType_No)
      return WNAV__CMDMODE;

    if ( wnav->appl.find( applist_eType_Graph, filenamestr, (void **) &wge))
      wge->set_subwindow_release();
    else
      wnav->message('E', "Graph not found");
  }
  else
  {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return 1;
}

static int	wnav_build_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  int	arg1_sts;
  pwr_tStatus sts;

  sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
  if ( EVEN(sts)) return sts;

  arg1_sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));
  if ( EVEN(arg1_sts)) {
    // build object is default
    strcpy( arg1_str, "OBJECT");
  }

  if ( strncmp( arg1_str, "NODE", strlen( arg1_str)) == 0) {
    char namestr[80];
    void *volumelist;
    int	volumecount;

    if ( EVEN( dcli_get_qualifier( "/NAME", namestr, sizeof(namestr)))) {
      if ( EVEN( dcli_get_qualifier( "dcli_arg2", namestr, sizeof(namestr)))) {
	wnav->message('E', "Syntax error");
	return WNAV__SYNTAX;
      }
    }

    // Load the bootlist
    sts = lfu_volumelist_load( load_cNameBootList, (lfu_t_volumelist **) &volumelist,
			       &volumecount);
    if ( sts == FOE__NOFILE) {
      wnav->message( 'E', "Project is not configured");
      return sts;
    }

    wb_build build( *(wb_session *)wnav->ldhses, wnav);
    build.opt.force = ODD( dcli_get_qualifier( "/FORCE", 0, 0));
    build.opt.debug = ODD( dcli_get_qualifier( "/DEBUG", 0, 0));
    build.opt.crossref = ODD( dcli_get_qualifier( "/CROSSREFERENCE", 0, 0));
    build.opt.manual = ODD( dcli_get_qualifier( "/MANUAL", 0, 0));

    build.node( namestr, volumelist, volumecount);
    wnav->message(' ', wnav_get_message(build.sts()));

    free( (char *) volumelist);
  }
  else if ( strncmp( arg1_str, "VOLUME", strlen( arg1_str)) == 0) {
    // Build current volume
    char namestr[80];

    if ( ODD( dcli_get_qualifier( "/NAME", namestr, sizeof(namestr)))) {
      wnav->message('E', "Syntax error");
      return WNAV__SYNTAX;
    }
    if ( ODD( dcli_get_qualifier( "dcli_arg2", namestr, sizeof(namestr)))) {
      wnav->message('E', "Syntax error");
      return WNAV__SYNTAX;
    }

    wb_build build( *(wb_session *)wnav->ldhses, wnav);
    build.opt.force = ODD( dcli_get_qualifier( "/FORCE", 0, 0));
    build.opt.debug = ODD( dcli_get_qualifier( "/DEBUG", 0, 0));
    build.opt.crossref = ODD( dcli_get_qualifier( "/CROSSREFERENCE", 0, 0));
    build.opt.manual = ODD( dcli_get_qualifier( "/MANUAL", 0, 0));

    build.volume();
    wnav->message(' ', wnav_get_message(build.sts()));
  }
  else if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0) {
    char 	namestr[80];
    pwr_tCid	cid;
    pwr_tAttrRef aref;
    int 	size;
    pwr_tOid 	menu_oid;
    pwr_tOName 	menu;
    char 	*methodstr_p;
    pwr_tString80 methodstr;
    pwr_sAttrRef *sel_list;
    int		*sel_is_attr;
    int		sel_cnt;
    int		name_found;
    wb_tMethodMenu method;
    wb_build_opt stored_opt = wnav->gbl.build;

    wnav->gbl.build.force = ODD( dcli_get_qualifier( "/FORCE", 0, 0));
    wnav->gbl.build.debug = ODD( dcli_get_qualifier( "/DEBUG", 0, 0));
    wnav->gbl.build.crossref = ODD( dcli_get_qualifier( "/CROSSREFERENCE", 0, 0));
    wnav->gbl.build.manual = ODD( dcli_get_qualifier( "/MANUAL", 0, 0));

    name_found = 1;
    if ( EVEN( dcli_get_qualifier( "/NAME", namestr, sizeof(namestr)))) {
      if ( EVEN( dcli_get_qualifier( "dcli_arg2", namestr, sizeof(namestr)))) {
	name_found = 0;
      }
    }

    if ( name_found) {
      sts = ldh_NameToAttrRef( wnav->ldhses, namestr, &aref);
      if ( EVEN(sts)) {
	wnav->message('E', "No such object");
	wnav->gbl.build = stored_opt;
	return WNAV__SYNTAX;
      }
      sel_list = (pwr_tAttrRef *) calloc( 1, sizeof( pwr_tAttrRef));
      sel_is_attr = (int *) calloc( 1, sizeof( int));
      sel_list[0] = aref;
      sel_cnt = 1;
    }
    else {
      // Get selected objects
      sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
      if ( EVEN(sts)) {
	wnav->message('E', "Syntax error");
	wnav->gbl.build = stored_opt;
	return WNAV__SYNTAX;
      }
    }

    for ( int i = 0; i < sel_cnt; i++) {
      // Find method
      sts = ldh_GetAttrRefTid( wnav->ldhses, &sel_list[i], &cid);
      if ( EVEN(sts)) return sts;
    
      sts = ldh_ObjidToName( wnav->ldhses, cdh_ClassIdToObjid( cid),
			     cdh_mName_volumeStrict, menu, sizeof(menu), &size);
      if ( EVEN(sts)) return sts;

      strcat( menu, "-NavigatorPoson-Pointed-Build");
      sts = ldh_NameToObjid( wnav->ldhses, &menu_oid, menu);
      if ( EVEN(sts)) {
	// No method
	continue;
      }
    
      sts = ldh_GetObjectPar( wnav->ldhses, menu_oid, "SysBody", "MethodName",
			      &methodstr_p, &size);
      if ( EVEN(sts)) return sts;

      strcpy( methodstr, methodstr_p);
      free( methodstr_p);

      sts = ldh_GetMethod( wnav->ldhses, methodstr, (wb_tMethod *)&method); 
      if ( EVEN(sts)) {
	continue;
      }

      ldh_sMenuCall menucall;
      memset( &menucall, 0, sizeof(menucall));
      menucall.PointedSession = wnav->ldhses;
      menucall.Pointed = sel_list[i];
      menucall.wnav = wnav;

      sts = (method)( &menucall);
      wnav->gbl.build = stored_opt;
      if ( EVEN(sts)) {
	wnav->message(' ', wnav_get_message(sts));
	return sts;
      }
    }
    free( sel_list);
    free( sel_is_attr);
  }
  else {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return 1;
}

static int	wnav_check_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  pwr_tStatus sts;

  sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
  if ( EVEN(sts)) return sts;

  sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));
  if ( EVEN(sts)) {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }

  if ( strncmp( arg1_str, "CLASSES", strlen( arg1_str)) == 0) {
    wb_volume *v = (wb_volume *) ldh_SessionToVol( wnav->ldhses);
    wb_vrepdb *vrep = (wb_vrepdb *)((wb_vrep *) *v);
    vrep->checkMeta();
  }
  else {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return 1;
}

static int	wnav_update_func(	void		*client_data,
					void		*client_flag)
{
  WNav *wnav = (WNav *)client_data;

  char	arg1_str[80];
  pwr_tStatus sts;

  sts = wnav_wccm_get_ldhsession_cb( wnav, &wnav->ldhses);
  if ( EVEN(sts)) return sts;

  sts = dcli_get_qualifier( "dcli_arg1", arg1_str, sizeof(arg1_str));
  if ( EVEN(sts)) {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }

  if ( strncmp( arg1_str, "CLASSES", strlen( arg1_str)) == 0) {
    if ( !wnav->editmode) {
      wnav->message('E', "Not in edit mode");
      return WNAV__NOEDIT;
    }

    wb_volume *v = (wb_volume *) ldh_SessionToVol( wnav->ldhses);
    wb_vrep *vrep = (wb_vrep *) *v;
    vrep->updateMeta();
    wnav->message('E', "Classvolumes updated");
  }
  else {
    wnav->message('E', "Syntax error");
    return WNAV__SYNTAX;
  }
  return 1;
}

int WNav::show_database()
{
  int		sts;
  pwr_tVolumeId	volume;
  int		volume_cnt;

  if ( !wbctx)
  {
    sts = (get_wbctx_cb)( parent_ctx, &wbctx);
    if ( EVEN(sts)) return sts;
  }

  // Get the volumelist
  sts = ldh_GetVolumeList( wbctx, &volume);
  volume_cnt = 0;
  while ( ODD(sts) )
  {
    if ( !volume_cnt)
    {
      brow_pop( wnav_eBrowType_Other);
      brow_SetNodraw( brow->ctx);
    }
    new WItemVolume( this, volume, NULL, flow_eDest_IntoLast);
    volume_cnt++;
    sts = ldh_GetNextVolume( wbctx, volume, &volume);
  }

  if ( !volume_cnt)
  {
    message( 'I', "No volumes found in database");
    return 1;
  }
  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);

  return 1;
}

int WNav::show_volume( int pop)
{
  if ( window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  // Create the root list
  if ( pop)
    brow_pop(wnav_eBrowType_Volume);
  brow_SetNodraw( brow->ctx);
  if ( !pop)
    brow_DeleteAll( brow->ctx);

  get_rootlist();

  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);

  return 1;
}

int WNav::get_rootlist()
{
  pwr_tObjid	root;
  int		sts;
  int		class_cnt;
  pwr_tClassId	valid_class[100];
  int		i;
  pwr_tClassId	classid;
  PalFileMenu     *menu_p;

  if ( gbl.all_toplevel || strcmp( layout, "") == 0)
  {
    // Display all toplevel objects
    sts = ldh_GetRootList( ldhses, &root);
    while ( ODD(sts))
    {
      new WItemObject( this, root, NULL, flow_eDest_IntoLast, 0);
      sts = ldh_GetNextSibling( ldhses, root, &root);
    }
  }
  else
  {
    // Get valid toplevel classes from config file
    if ( !menu) {
      menu = PalFile::config_tree_build( ldhses, pal_cPaletteFile,
	  pal_eNameType_TopObjects, layout, NULL); 
      if ( !menu) {
        printf( 
          "** WNav: topobjects entry \"%s\" not found in configuration file\n",
	  layout);
        return 0;
      }
      PalFile::config_tree_build( ldhses, pal_cLocalPaletteFile,
	  pal_eNameType_TopObjects, layout, menu);
    }

    class_cnt = 0;
    for ( menu_p = menu->child_list; menu_p; menu_p = menu_p->next) 
    {
      sts = ldh_ClassNameToId( ldhses, &valid_class[class_cnt],
			menu_p->title);
      if ( ODD(sts))
        class_cnt++;
      else
        printf( "** Unknown window root class: %s\n", menu_p->title);
    }
    if ( !class_cnt)
      return sts;

    // Create the root list
    sts = ldh_GetRootList( ldhses, &root);
    while ( ODD(sts))
    {
      sts = ldh_GetObjectClass( ldhses, root, &classid);
      if ( EVEN(sts)) return sts;

      for ( i = 0; i < class_cnt; i++)
      {
        if ( classid == valid_class[i])
        {
          new WItemObject( this, root, NULL, flow_eDest_IntoLast, 0);
          break;
        }
      }
      sts = ldh_GetNextSibling( ldhses, root, &root);
    }
  }
  return 1;
}

int WNav::check_toplevel_class( pwr_tCid cid)
{
  int		sts;
  pwr_tCid	valid_class;
  PalFileMenu     *menu_p;

  if ( !menu)
    return 0;

  if ( gbl.all_toplevel || strcmp( layout, "") == 0)
    // All toplevel objects are valid
    return 1;

  for ( menu_p = menu->child_list; menu_p; menu_p = menu_p->next) {
    sts = ldh_ClassNameToId( ldhses, &valid_class, menu_p->title);
    if ( EVEN(sts))
      continue;

    if ( valid_class == cid)
      return 1;
  }
  // Not found
  return 0;
}

int WNav::command( char* input_str)
{
  pwr_tCmd     	command;
  int		sts, sym_sts;
  char		symbol_value[DCLI_SYM_VALUE_SIZE];
  pwr_tFileName	filename;

  dcli_toupper( input_str, input_str);
  sts = dcli_replace_symbol( input_str, command, sizeof(command));
  if ( EVEN(sts)) return sts;

  if ( input_str[0] == '@')
  {
    /* Read command file */

    cdh_ToLower( filename, &command[1]);
    sts = readcmdfile( filename);
    if ( sts == DCLI__NOFILE)
    {
      message('E',"Unable to open file");
      return DCLI__SUCCESS;
    }
    else if ( EVEN(sts)) return sts;
    return DCLI__SUCCESS;
  }

  sts = dcli_cli( (dcli_tCmdTable *)&wnav_command_table, command, (void *) this, 0);
  if ( sts == DCLI__COM_NODEF)
  {
    /* Try to find a matching symbol */
    sym_sts = dcli_get_symbol_cmd( command, symbol_value);
    if ( ODD(sym_sts))
    {
      if ( symbol_value[0] == '@')
      {
        /* Read command file */
        sts = readcmdfile( &symbol_value[1]);
        if ( sts == DCLI__NOFILE)
        {
          message('E',"Unable to open file");
          return DCLI__SUCCESS;
        }
        else if ( EVEN(sts)) return sts;
        return DCLI__SUCCESS;
      }
      sts = dcli_cli( (dcli_tCmdTable *)&wnav_command_table, symbol_value, (void *) this, 0);
    }
    else if ( sym_sts == DCLI__SYMBOL_AMBIG)
      sts = sym_sts;
  }
  if (sts == DCLI__COM_AMBIG) message('E',"Ambiguous command");
  else if (sts == DCLI__COM_NODEF) message('E',"Undefined command");
  else if (sts == DCLI__QUAL_AMBIG) message('E',"Ambiguous qualifier");
  else if (sts == DCLI__QUAL_NODEF) message('E',"Undefined qualifier");
  else if (sts == DCLI__SYMBOL_AMBIG) message('E', "Ambiguous symbol abbrevation");

  return sts;
}

static int wnav_getcurrenttext_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  char text[80];
  brow_tNode	*node_list;
  int		node_count;

  if ( arg_count != 0)
    return CCM__ARGMISM;

  wnav_get_stored_wnav( &wnav);

  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
  if ( !node_count)
    strcpy( return_string, "");
  else
  {
    // Get annotation number 0
    brow_GetAnnotation( node_list[0], 0, text, sizeof(text));
    free( node_list);
    strcpy( return_string, text);
  }
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int wnav_getcurrentobject_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav			*wnav;
  pwr_tOName   		name;
  int			sts;
  pwr_sAttrRef		*sel_list;
  int                   *sel_is_attr;
  int			sel_cnt;
  int			size;

  if ( arg_count != 0)
    return CCM__ARGMISM;

  wnav_get_stored_wnav( &wnav);

  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  sts = wnav->get_select( &sel_list, &sel_is_attr, &sel_cnt);
  if ( ODD(sts))
  {
    sts = ldh_ObjidToName( wnav->ldhses, sel_list[0].Objid, ldh_eName_Hierarchy, 
	name, sizeof(name), &size);
    free( sel_list);
    free( sel_is_attr);
  }
  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int wnav_getcurrentvolume_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav			*wnav;
  char			name[80];
  int			sts;
  int			size;
  ldh_sVolumeInfo	info;

  if ( arg_count != 0)
    return CCM__ARGMISM;

  *return_decl = CCM_DECL_STRING;

  wnav_get_stored_wnav( &wnav);

  if ( !wnav->ldhses)
  {
    strcpy( return_string, "");
    return 1;
  }

  sts = ldh_GetVolumeInfo( ldh_SessionToVol(wnav->ldhses), &info);
  if ( EVEN(sts))
  {
    strcpy( return_string, "");
    return 1;
  }

  sts = ldh_VolumeIdToName( ldh_SessionToWB(wnav->ldhses),
	info.Volume, name, sizeof(name), &size);
  if ( EVEN(sts))
    strcpy( return_string, "");
  else
    strcpy( return_string, name);
  return 1;
}

static int wnav_messageerror_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  wnav_get_stored_wnav( &wnav);
  wnav->message('E', arg_list->value_string);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_messageinfo_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  wnav_get_stored_wnav( &wnav);
  wnav->message('I', arg_list->value_string);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_isw1_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;


  if ( arg_count != 0)
    return CCM__ARGMISM;

  wnav_get_stored_wnav( &wnav);

  *return_int = ( wnav->window_type == wnav_eWindowType_W1);
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_isw2_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;


  if ( arg_count != 0)
    return CCM__ARGMISM;

  wnav_get_stored_wnav( &wnav);

  *return_int = ( wnav->window_type == wnav_eWindowType_W2);
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_editmode_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;


  if ( arg_count != 0)
    return CCM__ARGMISM;

  wnav_get_stored_wnav( &wnav);

  *return_int = wnav->editmode;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_setdialogsize_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  ccm_s_arg *arg_p2, *arg_p3, *arg_p4; 

  if ( arg_count != 4)
    return CCM__ARGMISM;

  arg_p2 = arg_list->next;
  arg_p3 = arg_p2->next;
  arg_p4 = arg_p3->next;
  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__VARTYPE;
  if ( arg_p2->value_decl != CCM_DECL_INT)
    return CCM__VARTYPE;
  if ( arg_p3->value_decl != CCM_DECL_INT)
    return CCM__VARTYPE;
  if ( arg_p4->value_decl != CCM_DECL_INT)
    return CCM__VARTYPE;

  wnav_get_stored_wnav( &wnav);
  wnav->dialog_width = arg_list->value_int;
  wnav->dialog_height = arg_p2->value_int;
  wnav->dialog_x = arg_p3->value_int;
  wnav->dialog_y = arg_p4->value_int;

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_messagedialog_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  ccm_s_arg *arg_p2; 

  if ( arg_count != 2)
    return CCM__ARGMISM;

  arg_p2 = arg_list->next;
  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p2->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;

  wnav_get_stored_wnav( &wnav);

  if ( wnav->window_type == wnav_eWindowType_No)
    wnav->message('I', arg_p2->value_string);
  else
    wnav->message_dialog( arg_list->value_string, arg_p2->value_string);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_confirmdialog_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  ccm_s_arg *arg_p2, *arg_p3; 
  int sts;
  int cancel;
  char str[80];

  if ( !(arg_count == 2 || arg_count == 3))
    return CCM__ARGMISM;

  arg_p2 = arg_list->next;
  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p2->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_count == 3)
  {
    // Add cancel button and return if cancel is pressed
    arg_p3 = arg_p2->next;
    if ( arg_p3->value_decl != CCM_DECL_INT)
      return CCM__VARTYPE;

    wnav_get_stored_wnav( &wnav);
    if ( wnav->window_type == wnav_eWindowType_No)
    {
      printf( "%s", arg_p2->value_string);
      printf(" (y/n/q) : ");
      scanf( "%s", str);
      if ( strcmp( str, "Y") == 0 || strcmp( str, "y") == 0 ||
           strcmp( str, "") == 0)
      {
        sts = 1;
        cancel = 0;
      }
      else if ( strcmp( str, "Q") == 0 || strcmp( str, "q") == 0)
      {
        sts = 0;
        cancel = 1;
      }
      else
      {
        sts = 0;
        cancel = 0;
      }
    }
    else
      sts = wnav->confirm_dialog( arg_list->value_string, arg_p2->value_string,
		1, &cancel);
    arg_p3->value_int = cancel;
    arg_p3->value_returned = 1;
  }
  else
  {
    wnav_get_stored_wnav( &wnav);
    if ( wnav->window_type == wnav_eWindowType_No)
    {
      printf( "%s", arg_p2->value_string);
      printf(" (y/n) : ");
      scanf( "%s", str);
      if ( strcmp( str, "Y") == 0 || strcmp( str, "y") == 0 ||
           strcmp( str, "") == 0)
      {
        sts = 1;
        cancel = 0;
      }
      else
      {
        sts = 0;
        cancel = 0;
      }
    }
    else
      sts = wnav->confirm_dialog( arg_list->value_string, arg_p2->value_string,
		0, &cancel);
  }

  *return_int = sts;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_continuedialog_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  ccm_s_arg *arg_p2; 
  int sts;
  char str[80];

  if ( arg_count != 2)
    return CCM__ARGMISM;

  arg_p2 = arg_list->next;
  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p2->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;

  wnav_get_stored_wnav( &wnav);
  if ( wnav->window_type == wnav_eWindowType_No)
  {
    printf( "%s", arg_p2->value_string);
    printf("\nDo you want to continue ? (y/n) : ");
    scanf( "%s", str);
    if ( strcmp( str, "Y") == 0 || strcmp( str, "y") == 0 ||
         strcmp( str, "") == 0)
      sts = 1;
    else
      sts = 0;
  }
  else
    sts = wnav->continue_dialog( arg_list->value_string, arg_p2->value_string);

  *return_int = sts;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_promptdialog_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  ccm_s_arg *arg_p2, *arg_p3; 
  int sts;
  char *value;
  char str[80];

  if ( arg_count != 3)
    return CCM__ARGMISM;

  arg_p2 = arg_list->next;
  arg_p3 = arg_p2->next;
  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p2->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p3->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;

  wnav_get_stored_wnav( &wnav);
  if ( wnav->window_type == wnav_eWindowType_No)
  {
    printf( "%s", arg_p2->value_string);
    printf( " : ");
    scanf( "%s", str);
    value = str;
    sts = 1;
  }
  else
    sts = wnav->prompt_dialog( arg_list->value_string, arg_p2->value_string,
		&value);

  strncpy( arg_p3->value_string, value, sizeof( arg_p3->value_string));
  arg_p3->value_string[sizeof( arg_p3->value_string)-1] = 0;
  arg_p3->value_returned = 1;

  *return_int = sts;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_opengraph_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  ccm_s_arg *arg_p2; 
  ccm_s_arg *arg_p3; 
  int sts;
  pwr_tCmd cmd;

  if ( arg_count < 2 || arg_count > 3)
    return CCM__ARGMISM;

  arg_p2 = arg_list->next;
  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p2->value_decl != CCM_DECL_INT)
    return CCM__VARTYPE;
  if ( arg_count >= 3) {
    arg_p3 = arg_p2->next;
    if ( arg_p3->value_decl != CCM_DECL_STRING)
      return CCM__VARTYPE;
  }

  wnav_get_stored_wnav( &wnav);
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;
      
  strcpy( cmd, "open graph/file=\"");
  strcat( cmd, arg_list->value_string);
  strcat( cmd, "\"");
  if ( arg_p2->value_int)
    strcat( cmd, " /modal");
  if ( arg_count >= 3) {
    strcat( cmd, " /title=\"");
    strcat( cmd, arg_p3->value_string);
    strcat( cmd, "\"");
  }
  sts = wnav->command( cmd);

  *return_int = sts;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_closegraph_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  int sts;
  pwr_tCmd cmd;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;

  wnav_get_stored_wnav( &wnav);
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  strcpy( cmd, "close graph/file=");
  strcat( cmd, arg_list->value_string);

  sts = wnav->command( cmd);

  *return_int = sts;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_setsubwindow_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  WNav *wnav;
  ccm_s_arg *arg_p2; 
  ccm_s_arg *arg_p3; 
  ccm_s_arg *arg_p4; 
  int sts;
  pwr_tCmd cmd;

  if ( arg_count != 4)
    return CCM__ARGMISM;

  arg_p2 = arg_list->next;
  arg_p3 = arg_p2->next;
  arg_p4 = arg_p3->next;
  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p2->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p3->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_p4->value_decl != CCM_DECL_INT)
    return CCM__VARTYPE;

  wnav_get_stored_wnav( &wnav);
  if ( wnav->window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;
      
  strcpy( cmd, "set subwindow ");
  strcat( cmd, arg_list->value_string);
  strcat( cmd, "/name=");
  strcat( cmd, arg_p2->value_string);
  strcat( cmd, "/source=\"");
  strcat( cmd, arg_p3->value_string);
  strcat( cmd, "\"");
  if ( arg_p4->value_int)
    strcat( cmd, " /modal");

  sts = wnav->command( cmd);

  *return_int = sts;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int wnav_ccm_deffilename_func( char *outfile, char *infile, void *client_data)
{

  dcli_get_defaultfilename( infile, outfile, ".pwr_com");
  dcli_translate_filename( outfile, outfile);
  return 1;
}

static int wnav_ccm_errormessage_func( char *msg, int severity, void *client_data)
{
  WNav *wnav = (WNav *) client_data; 

  if ( EVEN(severity))
    wnav->message( 'I', msg);
  else
    wnav->message( 'E', msg);
  return 1;
}

int	wnav_externcmd_func( char *cmd, void *client_data)
{
  WNav *wnav = (WNav *) client_data; 
  int sts;

  wnav->scriptmode++;
  sts = wnav->command( cmd);
  wnav->scriptmode--;
  return sts;
}

/*************************************************************************
*
* Name:		readcmdfile()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

int WNav::readcmdfile( 	char		*incommand)
{
  char		input_str[160];
  int		sts;
  int		appl_sts;

  if ( !ccm_func_registred)
  {
    sts = ccm_register_function( "GetCurrentText", wnav_getcurrenttext_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "GetCurrentObject", wnav_getcurrentobject_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "GetCurrentVolume", wnav_getcurrentvolume_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "MessageError", wnav_messageerror_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "MessageInfo", wnav_messageinfo_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "IsW1", wnav_isw1_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "IsW2", wnav_isw2_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "EditMode", wnav_editmode_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "MessageDialog", wnav_messagedialog_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "ConfirmDialog", wnav_confirmdialog_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "ContinueDialog", wnav_continuedialog_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "PromptDialog", wnav_promptdialog_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "SetDialogSize", wnav_setdialogsize_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "OpenGraph", wnav_opengraph_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "CloseGraph", wnav_closegraph_func);
    if ( EVEN(sts)) return sts;
    sts = ccm_register_function( "SetSubwindow", wnav_setsubwindow_func);
    if ( EVEN(sts)) return sts;

    ccm_func_registred = 1;

    // Register wb standard functions
    wccm_register( wnav_wccm_get_wbctx_cb, wnav_wccm_get_ldhsession_cb);
  }

  strcpy( input_str, incommand);
  dcli_remove_blank( input_str, input_str);
  wnav_store_wnav( this);
  if ( ldhses)
    wccm_store_ldhses( this, ldhses);
  else
    wccm_store_client( this);

  // Read and execute the command file
  sts = ccm_file_exec( input_str, wnav_externcmd_func,
		wnav_ccm_deffilename_func, wnav_ccm_errormessage_func, 
		&appl_sts, gbl.verify, 0, NULL, 0, 0, NULL, (void *)this);
  if ( EVEN(sts)) return sts;

  wnav_free_stored_wnav();
  return 1;
}

int wnav_cut_segments (
  char	*outname,
  char	*name,
  int	segments
)
{
	char	*s[20];
	int	i, j, last_i;

	for( i = 0; i < segments; i++)
	{
	  s[i] = strrchr( name, '-');
	  if ( s[i] == 0)
	  {
	    last_i = i;
	    break;
	  }
	  *s[i] = '+';
	  last_i = i;
	}
	for ( j = 0; j <= last_i; j++)
	{
	  if ( s[j] != 0)
	    *s[j] = '-';
	}
	if ( s[last_i] == 0)
	  strcpy( outname, name);
	else
	  strcpy( outname, s[last_i] + 1);

	return 1;
}

int WNav::get_current_object(
			pwr_tObjid	*objid,
			char		*objectname,
			int		size,
			ldh_eName	nametype)
{
  int			sts;
  pwr_sAttrRef		*sel_list;
  int                   *sel_is_attr;
  int			sel_cnt;
  int			rsize;

  if ( window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  sts = get_select( &sel_list, &sel_is_attr, &sel_cnt);
  if ( ODD(sts))
  {
    if ( cdh_ObjidIsNull( sel_list[0].Objid))
      return WNAV__NOOBJECT;
    *objid = sel_list[0].Objid;
    sts = ldh_ObjidToName( ldhses, sel_list[0].Objid, nametype,
	objectname, size, &rsize);
    free( sel_list);
    free( sel_is_attr);
  }
  return sts;
}

int	WNav::find_name(
			char		*name,
			pwr_tObjid	*objid)
{
  int	sts;

  sts = ldh_NameToObjid( ldhses, objid, name);
  return sts;
}

int WNav::show_symbols()
{
  int sts;
  int i;
  char	key[DCLI_SYM_KEY_SIZE];
  char	value[DCLI_SYM_VALUE_SIZE];
  char	text[500];

  if ( window_type == wnav_eWindowType_No)
  {
    i = 0;
    while( 1)
    {
      sts = dcli_get_symbol_by_index( i, key, value);
      if (EVEN(sts)) break;

      sprintf( text, "%s = \"%s\"", key, value);
      if ( i == 0)
        printf( "Symbols\n\n");
      printf( "%s\n", text);
      i++;
    }
    if ( i == 0)
      message('I', "Symboltable is empty");
  }
  else
  {
    i = 0;
    while( 1)
    {
      sts = dcli_get_symbol_by_index( i, key, value);
      if (EVEN(sts)) break;

      sprintf( text, "%s = \"%s\"", key, value);
      if ( i == 0)
      {
        brow_pop( wnav_eBrowType_Other);
        brow_SetNodraw( brow->ctx);
        new WItemHeader( this, "Title", "Symbols", NULL, flow_eDest_IntoLast);
      }
      new WItemCommand( this, text, NULL,
		flow_eDest_IntoLast, key, 0, brow->pixmap_symbol);
      i++;
    }
    if ( i == 0)
      message('I', "Symboltable is empty");
    else
    {
      brow_ResetNodraw( brow->ctx);
      brow_Redraw( brow->ctx, 0);
    }
  }
  return WNAV__SUCCESS;
}

int WNav::search( char *search_str, int regexp)
{
  int sts;
  pwr_tObjid found_objid;
  char str[200];

  if ( regexp)
  {
    if ( search_compiled)
      regfree( &search_creg);

    sts = regcomp( &search_creg, search_str, REG_NOSUB | REG_ICASE);
    if (sts != 0)
    {
      search_compiled = 0;
      search_type = wnav_eSearchType_No;
      return WNAV__SEARCHCOMPILE;
    }
    search_compiled = 1;
    search_type = wnav_eSearchType_RegularExpr;
  }
  else
    search_type = wnav_eSearchType_Name;

  cdh_ToUpper( str, search_str);
  sts = search_root( str, &found_objid, 0);
  if ( EVEN(sts)) return sts;

  // Store for search next
  search_last = found_objid;
  strcpy( search_string, str);

  sts = display_object( found_objid);
  return sts;
}

int WNav::search_next()
{
  int sts;
  pwr_tObjid found_objid;

  if ( search_type == wnav_eSearchType_No)
    return WNAV__NOSEARCH;

  search_last_found = 0;
  sts = search_root( search_string, &found_objid, 1);
  if ( EVEN(sts)) return sts;

  // Store for search next
  search_last = found_objid;

  sts = display_object( found_objid);
  return sts;
}

int WNav::search_root( char *search_str, pwr_tObjid *found_objid, int next)
{
  pwr_tObjid	root;
  int		sts, search_sts;
  int		class_cnt;
  pwr_tClassId	valid_class[100];
  int		i;
  pwr_tClassId	classid;
  PalFileMenu     *menu_p;

  search_sts = 0;
  if ( gbl.all_toplevel || strcmp( layout, "") == 0)
  {
    // Display all toplevel objects
    sts = ldh_GetRootList( ldhses, &root);
    while ( ODD(sts))
    {
      search_sts = search_object( root, search_str, found_objid, next);
      if ( ODD(search_sts))
        break;
      sts = ldh_GetNextSibling( ldhses, root, &root);
    }
  }
  else
  {
    if ( !menu)
      return 0;
    class_cnt = 0;
    for ( menu_p = menu->child_list; menu_p; menu_p = menu_p->next) 
    {
      sts = ldh_ClassNameToId( ldhses, &valid_class[class_cnt],
			menu_p->title);
      if ( ODD(sts))
        class_cnt++;
    }
    if ( !class_cnt)
      return sts;

    // Create the root list
    sts = ldh_GetRootList( ldhses, &root);
    while ( ODD(sts))
    {
      sts = ldh_GetObjectClass( ldhses, root, &classid);
      if ( EVEN(sts)) return sts;

      for ( i = 0; i < class_cnt; i++)
      {
        if ( classid == valid_class[i])
        {
          search_sts = search_object( root, search_str, found_objid, next);
          break;
        }
      }
      if ( ODD(search_sts))
        break;
      sts = ldh_GetNextSibling( ldhses, root, &root);
    }
  }
  return search_sts;
}

int WNav::search_object( pwr_tObjid objid, char *search_str, 
	pwr_tObjid *found_objid, int next)
{
  char	name[80];
  int	sts, search_sts;
  pwr_tObjid child;
  int 	size;
  pwr_tClassId classid;

  if ( next && !search_last_found)
  {
    if ( cdh_ObjidIsEqual( objid, search_last))
      search_last_found = 1;
  }
  else
  {
    sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Object,
			name, sizeof(name), &size);
    if (EVEN(sts)) return sts;

    if ( search_type == wnav_eSearchType_Name)
    {
      cdh_ToUpper( name, name);
      if ( strcmp( name, search_str) == 0)
      {
        *found_objid = objid;
        return WNAV__SUCCESS;
      }
    }
    else
    {
      sts = regexec( &search_creg, name, 0, NULL, 0);
      if ( sts == REG_OK) 
      {
        *found_objid = objid;
        return WNAV__SUCCESS;
      }
    }
  }

  // If mountobject, don't search children
  sts = ldh_GetObjectClass( ldhses, objid, &classid);
  if ( EVEN(sts)) return sts;

  if ( classid == pwr_eClass_MountObject)
    return WNAV__NOMATCH;

  search_sts = WNAV__NOMATCH;
  sts = ldh_GetChild( ldhses, objid, &child);
  while( ODD(sts))
  {
    search_sts = search_object( child, search_str, found_objid, next);
    if (ODD(search_sts))
      break;
    sts = ldh_GetNextSibling( ldhses, child, &child);
  }
  return search_sts;  
}

static int wnav_wccm_get_ldhsession_cb( void *ctx, ldh_tSesContext *ldhses)
{
  WNav *wnav = (WNav *)ctx;
  int  sts;

  if ( wnav->ldhses)
    *ldhses = wnav->ldhses;
  else
  {
    // Attach first volume
    sts = (wnav->attach_volume_cb)( wnav->parent_ctx, 0, 1);
    if ( EVEN(sts))
      return sts;
    *ldhses = wnav->ldhses;
  }
  return 1;
}

static int wnav_wccm_get_wbctx_cb( void *ctx, ldh_tWBContext *wbctx)
{
  WNav *wnav = (WNav *)ctx;
  int	sts;

  if ( wnav->ldhses)
    *wbctx = ldh_SessionToWB( wnav->ldhses);
  else
  {
    sts = (wnav->get_wbctx_cb)( wnav->parent_ctx, wbctx);
    if ( EVEN(sts)) return sts;
  }
  return 1;
}

char	*wnav_VolumeIdToStr( pwr_tVolumeId volumeid)
{
	static char	str[80];
	unsigned char	volid[4];

	memcpy( &volid, &volumeid, sizeof(volid));
	sprintf( str, "%3.3u_%3.3u_%3.3u_%3.3u",
		volid[3], volid[2], volid[1], volid[0]);
	return str;
}

//
// Displays all files with matching a wilcard specification.
//

int	WNav::show_file(
			const char     	*filename,
			const char     	*intitle,
			int		hide_dir)
{
  int		sts;
  char		title[80] = "FILE LIST";
  pwr_tFileName	found_file;
  char		dev[80], dir[80], file[80], type[80];
  int		version;
  pwr_tFileName	file_spec;
  char		text[80];
  item_eFileType file_type;

  if ( intitle)
    strcpy( title, intitle);

  strcpy( file_spec, filename);

  sts = dcli_search_file( file_spec, found_file, DCLI_DIR_SEARCH_INIT);
  if ( EVEN(sts)) return sts;

  dcli_parse_filename( found_file, dev, dir, file, type, &version);
  if ( hide_dir)
  {
    strcpy( text, file);
    cdh_ToLower( text, text);
    text[0] = toupper( text[0]);
  }
  else
    strcpy( text, found_file);
  cdh_ToUpper( type, type);
  if ( strcmp( type, ".PWR_COM") == 0)
    file_type = item_eFileType_Script;
  else if ( strcmp( type, ".PWG") == 0)
    file_type = item_eFileType_Graph;
  else
    file_type = item_eFileType_Unknown;

  if ( window_type != wnav_eWindowType_No)
  {
    brow_pop( wnav_eBrowType_Other);
    brow_SetNodraw( brow->ctx);
    new WItemHeader( this, "Title", title, NULL, flow_eDest_IntoLast);
    new WItemFile( this, "", text, found_file, file_type, NULL, flow_eDest_IntoLast);
  }
  else
    printf( "%s\n\n",  title);

  while ( ODD(sts))
  {
    sts = dcli_search_file( file_spec, found_file, DCLI_DIR_SEARCH_NEXT);
    if ( ODD(sts))
    {
      dcli_parse_filename( found_file, dev, dir, file, type, &version);
      if ( hide_dir)
      {
        strcpy( text, file);
        cdh_ToLower( text, text);
        text[0] = toupper( text[0]);
      }
      else
        strcpy( text, found_file);
      cdh_ToUpper( type, type);
      if ( strcmp( type, ".PWR_COM") == 0)
        file_type = item_eFileType_Script;
      else if ( strcmp( type, ".PWG") == 0)
        file_type = item_eFileType_Graph;
      else
        file_type = item_eFileType_Unknown;
      if ( window_type == wnav_eWindowType_No)
        printf( "%s\n", found_file);
      else
        new WItemFile( this, "", text, found_file, file_type, NULL, flow_eDest_IntoLast);
    }
  }
  dcli_search_file( filename, found_file, DCLI_DIR_SEARCH_END);
  if ( window_type != wnav_eWindowType_No)
  {
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, 0);
  }
  return WNAV__SUCCESS;
}

static int wnav_display_objects_cb( pwr_sAttrRef *arp, void *a1, void *a2, 
				    void *a3, void *a4, void *a5)
{
  WNav *wnav = (WNav *) a1;
  pwr_tStatus sts;

  if ( arp->Flags.b.Object)
    new WItemObject( wnav, arp->Objid, NULL, flow_eDest_IntoLast, 0);
  else {
    char *namep;
    char *s;
    char body[20] = "RtBody";
    int size;

    sts = ldh_AttrRefToName( wnav->ldhses, arp, ldh_eName_Aref, &namep,
			     &size);
    if ( EVEN(sts)) return sts;
    s = strchr( namep, '.');
    if ( s)
      s++;
    else
      s = namep;

    new WItemAttrObject( wnav->brow, wnav->ldhses, arp->Objid, NULL, 
		flow_eDest_IntoLast, s,
		arp->Body,
		arp->Size, false, 0,
		arp->Flags.m,
		body, 1);
  }
  return WNAV__SUCCESS;
}

int WNav::display_objects( pwr_tCid *cidp, char *name, pwr_tObjid root,
			   int depth)
{
  pwr_tStatus sts;

  brow_pop( wnav_eBrowType_Other);
  brow_SetNodraw( brow->ctx);

  sts = trv_get_attrobjects( ldhses, root, cidp, name, (trv_eDepth) depth,
		       wnav_display_objects_cb, this, 0,0,0,0);

  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
  return sts;
}

int WNav::get_next_free_vid( pwr_tVid min_vid, pwr_tVid max_vid, pwr_tVid *next)
{
  lfu_t_volumelist *volumelist;
  int volumecount;
  pwr_tVid next_vid = 0;
  int found;
  int sts;

  sts = lfu_volumelist_load( load_cNameGblVolumeList,  &volumelist, 
			     &volumecount);
  if ( EVEN(sts)) return sts;

  for ( unsigned int j = min_vid; j <= max_vid; j++) {

    found = 0;
    for ( int i = 0; i < volumecount; i++) {
      if ( volumelist[i].volume_id == j) {
	found = 1;
	break;
      }
    }
    if ( !found) {
      next_vid = j;
      break;
    }
  }

  if ( volumecount > 0)
    free( (char *)volumelist);

  if ( !next_vid)
    return WNAV__NOFREEVID;

  *next = next_vid;
  return WNAV__SUCCESS;
}

int WNav::check_new_vid( pwr_tVid vid)
{
  lfu_t_volumelist *volumelist;
  int volumecount;
  int found = 0;
  int sts;

  sts = lfu_volumelist_load( load_cNameGblVolumeList,  &volumelist, 
			     &volumecount);
  if ( EVEN(sts)) return sts;

  for ( int i = 0; i < volumecount; i++) {
    if ( volumelist[i].volume_id == vid) {
      found = 1;
      break;
    }
  }

  if ( found)
    return WNAV__VIDEXIST;
  else
    return WNAV__SUCCESS;
}

int WNav::check_new_volumename( char *vname)
{
  lfu_t_volumelist *volumelist;
  int volumecount;
  int found = 0;
  int sts;

  sts = lfu_volumelist_load( load_cNameGblVolumeList,  &volumelist, 
			     &volumecount);
  if ( EVEN(sts)) return sts;

  for ( int i = 0; i < volumecount; i++) {
    if ( cdh_NoCaseStrcmp( vname, volumelist[i].volume_name) == 0) {
      found = 1;
      break;
    }
  }

  if ( found)
    return WNAV__VOLNAMEEXIST;
  else
    return WNAV__SUCCESS;
}


