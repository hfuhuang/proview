/* rt_rtt_command.c -- <short description>

   PROVIEW/R
   Copyright (C) 1996-98 by Mandator AB.

   This module contains routines for handling of command line in rtt. */

/*_Include files_________________________________________________________*/



#if defined(OS_ELN)
# include $vaxelnc
# include stdio
# include string
# include stdlib
# include ctype
# include chfdef
# include descrip
# include starlet
#elif defined (OS_VMS)
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <ctype.h>
# include <chfdef.h>
# include <descrip.h>
# include <lib$routines.h>
# include <clidef.h>
# include <ssdef.h>
# include <starlet.h>
#else
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <ctype.h>
#endif

#if defined OS_LINUX
# include <time.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "co_time.h"
#include "co_cdh.h"
#include "co_ccm.h"
#include "rt_gdh.h"
#include "rt_ini_alias.h"
#include "rt_rtt.h"
#include "rt_rtt_global.h"
#include "rt_rtt_msg.h"
#include "rt_rtt_functions.h"
#include "dtt_rttsys_functions.h"
#include "co_ccm_msg.h"
#include "rt_gdh_msg.h"
#include "rt_rtt_helptext.h"
#include "rt_load.h"

#define r_toupper(c) (((c) >= 'a' && (c) <= 'z') ? (c) & 0xDF : (c))
#define r_tolower(c) (((c) >= 'A' && (c) <= 'Z') ? (c) | 0x20 : (c))

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define IF_NOGDH_RETURN \
if ( !rtt_gdh_started)\
{\
rtt_message('E',"Rtt is not connected to nethandler");\
return RTT__NOPICTURE;\
}

static int		zero = 0;
static int		one = 1;
char			rtt_searchbuffer[80] = "";
static	FILE		*rtt_learn_file;
static void		*rtt_ccmctx = 0;
static char		rtt_ccmcmd[256] = "";
static int		rtt_ccmcmd_fetched = 0;
static int		rtt_ccmcmd_quit = 0;
static int		rtt_ccm_func_registred = 0;
/* Local function prototypes */

static int rtt_command_toupper( 	char	*str_upper,
					char	*str);
static int	monitor_func(	menu_ctx	ctx,
				int		*flag);
static int	show_func(	menu_ctx	ctx,
				int		*flag);
static int	debug_func(	menu_ctx	ctx,
				int		*flag);
static int	add_func(	menu_ctx	ctx,
				int		*flag);
static int	crossref_func(	menu_ctx	ctx,
				int		*flag);
static int	collect_func(	menu_ctx	ctx,
				int		*flag);
static int	logging_func(	menu_ctx	ctx,
				int		*flag);
static int	directory_func(	menu_ctx	ctx,
				int		*flag);
static int	rtt_get_func(	menu_ctx	ctx,
				int		*flag);
static int	rtt_set_func(	menu_ctx	ctx,
				int		*flag);
static int	plcscan_func(	menu_ctx	ctx,
				int		*flag);
static int	store_func(	menu_ctx	ctx,
				int		*flag);
static int	alarm_func(	menu_ctx	ctx,
				int		*flag);
static int	rtt_create_func(	menu_ctx	ctx,
					int		*flag);
static int	rtt_delete_func(	menu_ctx	ctx,
					int		*flag);
static int	rtt_view_func(	menu_ctx	ctx,
				int		*flag);
static int	wait_func(	menu_ctx	ctx,
				int		*flag);
static int	search_func(	menu_ctx	ctx,
				int		*flag);
static int	top_func(	menu_ctx	ctx,
				int		*flag);
static int	page_func(	menu_ctx	ctx,
				int		*flag);
static int	help_func(	menu_ctx	ctx,
				int		*flag);
static int	exit_func(	menu_ctx	ctx,
				int		*flag);
static int	classhier_func(	menu_ctx	ctx,
				int		*flag);
static int	rtt_show_object_add(
			pwr_tObjid	objid,
			rtt_t_menu	**menulist,
			int		*index,
			void		*dum2,
			void		*dum3,
			void		*dum4);
static int	rtt_show_parameter_add(
			pwr_tObjid	objid,
			rtt_t_menu_upd	**menulist,
			char		*parname,
			int		*index,
			int		*element,
			void		*dum4);
#if 0
static int	rtt_show_class_hier_class_name(
			menu_ctx	parent_ctx,
			char		*hiername,
			char		*classname,
			char		*name);
#endif
static int	rtt_show_par_hier_class_name(
			menu_ctx	parent_ctx,
			char		*parametername,
			char		*hiername,
			char		*classname,
			char		*name,
			int		add,
			int		global,
			int		max_objects);
static int	rtt_debug_obj_hier_class_name(
			menu_ctx	parent_ctx,
			char		*hiername,
			char		*classname,
			char		*name,
			int		add,
			int		global);
static int	rtt_get_child_object_hi_cl_na(
			menu_ctx	ctx,
			pwr_tClassId	class,
			char		*name,
			pwr_tObjid	objid,
			int		*obj_counter,
			int		max_count,
			int		global,
			int		(*backcall)(),
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4,
			void		*arg5);
#if 0
static int	rtt_get_class_hier_class_name(
			menu_ctx	ctx,
			pwr_tObjid	hierobjid,
			pwr_tClassId	class,
			char		*name,
			int		max_count,
			int		(*backcall)(),
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4,
			void		*arg5);
#endif
static int	rtt_find_name(
			menu_ctx	ctx,
			char		*name,
			pwr_tObjid	*objid);
static int	rtt_find_hierarchy(
			menu_ctx	ctx,
			pwr_tObjid	*objid);
#if 0
static int rtt_menu_sort_compare( const void *p1, const void *p2);
static int rtt_menu_upd_sort_compare( const void *p1, const void *p2);
#endif
static int	rtt_menulist_search(
			menu_ctx	ctx,
			char		*searchstr,
			int		index,
			int		*foundindex);
static int	rtt_store(
			menu_ctx	ctx,
			char		*filename,
			int		collect);
static int	rtt_set_parameter(
			char		*name_str,
			char		*value_str,
			int		bypass);
static int rtt_get_current_object(
			menu_ctx	ctx,
			pwr_tObjid	*objid,
			char		*objectname,
			int		size,
			pwr_tBitMask	nametype);
static int	rtt_set_plcscan(
			pwr_tObjid	objid,
			int		*on,
			void		*dum1,
			void		*dum2,
			void		*dum3,
			void		*dum4);
static int	rtt_plcscan(
			menu_ctx	ctx,
			int		on,
			int		all,
			char		*hiername,
			int		global);
static int	rtt_create_object(
			menu_ctx	parent_ctx,
			char		*classname,
			char		*name);
static int	rtt_delete_object(
			menu_ctx	parent_ctx,
			char		*name);
static int	rtt_show_step(
			menu_ctx	parent_ctx,
			char		*hiername,
			int		initstep);
static int	rtt_print_picture(
			menu_ctx	ctx,
			char		*filename, 
			int		append,
			int		text_size,
			int		parameter_size);
static int	rtt_print_item_upd(
			char		*text,
			char		*parameter,
			char		*value_ptr,
			int		value_type,
			int		flags,
			FILE		*outfile,
			int		text_size,
			int		parameter_size);
static int	rtt_set_conversion(	pwr_tObjid	objid,
				int		on,
				int		show_only);
static int	rtt_set_invert(	pwr_tObjid	objid,
			int		on,
			int		show_only);
static int	rtt_get_invert(	pwr_tObjid	objid,
			int		*on);
static int	rtt_get_conversion(	pwr_tObjid	objid,
				int		*on);
static int	rtt_print_picture_restore(
			menu_ctx	ctx,
			char		*filename);
static int	rtt_print_restore_item_upd(
			char		*parameter,
			char		*value_ptr,
			int		value_type,
			int		flags,
			FILE		*outfile);
static int	rtt_print_text(
			char		*filename, 
			char		*str,
			int		append);
static int	rtt_confirm( char	*text);
static int	rtt_qual_to_time( 	char 		*in_str, 
					pwr_tTime 	*time);


/*************************************************************************
*
* Name:		rtt_wildcard()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*wildname	I	wildcard name.
* char		*name		I	object name.
*
* Description:
*	Checks if the object name can be described by the
*	wildcard string.
*	Returns 0 if ok, else 1.
*
**************************************************************************/

int rtt_wildcard(	char	*wildname,
			char	*name)
{
	int	len;
	char	*s;
	char	*t;
	char	*u;
	char	checkstr[80];
	char	upper_name[120];
	char	upper_wildname[120];

	/* Convert to upper case */
	rtt_toupper( upper_name, name);
	rtt_toupper( upper_wildname, wildname);

	t = upper_wildname;
	u = upper_name;
	s = strchr( t, '*');
	if ( s == 0)
	{
	  if (strcmp( t, u) == 0)
	    return 0;
	  else 
	    return 1;
	}

	len = s - t;
	if ( len > 0)
	{
	  strncpy ( checkstr, t, len);
	  checkstr[len] = '\0';
	  if ( strncmp ( checkstr, u, len) != 0)
	    return 1;
	  u += len;
	}	  
	t += len + 1;
	s = strchr( t, '*');

	while ( s != 0)
	{
	  len = s - t;
	  if ( len > 0)
	  {
	    strncpy ( checkstr, t, len);
	    checkstr[len] = '\0';
	    u = strstr( u, checkstr);
	    if ( u == 0)
	      return 1;
	    u += len;
	  }	  
	  t += len + 1;
	  s = strchr( t, '*');
	}
	strcpy ( checkstr, t);
	u = u + strlen(u) - strlen (checkstr);
	if ( strcmp ( checkstr, u ) != 0)
	      return 1;

	return 0;
}

/*************************************************************************
*
* Name:		rtt_toupper()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*str		I	input string.
* char		*upper_str	I	string converted to upper case.
*
* Description:
*	Converts a string to upper case.
*
**************************************************************************/

int rtt_toupper( 	char	*str_upper,
			char	*str)
{
	cdh_ToUpper( str_upper, str);
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_toupper()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*str		I	input string.
* char		*upper_str	I	string converted to upper case.
*
* Description:
*	Converts a string to upper case.
*	Text between two '"' will not be affekted.
*
**************************************************************************/

static int rtt_command_toupper( 	char	*str_upper,
					char	*str)
{
	char	namechar;
	char	*u;
	char	*t; 
	int	convert;
	unsigned char prev_char;

	/* Convert to upper case */
	u = str;
	t = str_upper;
	prev_char = 0;
	convert = 1;
	while ( *u != '\0')
	{
	  namechar = *(u++);
	  if ( namechar == '"' && prev_char != '\\')
	    convert = !convert;
	  if (convert)
	  {
	    *t = (char) r_toupper( namechar);
	    if (*t == '�') *t = (char) '�';
	    else if (*t == '�') *t = (char) '�';
	    else if (*t == '�') *t = (char) '�';
	  }
	  else
	    *t = namechar;
	  prev_char = (unsigned char) *t;
	  t++;
	}
	*t = '\0';

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		monitor_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "monitor" command i recieved.
*
**************************************************************************/

static int	monitor_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;

	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "GRAFCET", strlen( arg1_str)) == 0)
	{
	  /* Command is "MONITOR GRAFCET" */
	  pwr_tObjid	objid;
	  pwr_tClassId	class;
	  char		classname[80];
	  int		found;
	  char		name_str[160];

	  /* Get the selected object */
	  sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	  found = 0;
	  while( ODD(sts)) 
	  {
	    sts = gdh_GetObjectClass ( objid, &class);
	    if ( EVEN(sts)) return sts;
	    sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	    /* Check that this is a or a plc */	
	    if ( strcmp( classname, "pwrb:Class-PlcPgm") == 0)
	    {
	      found = 1;
	      break;
	    }
	    sts = gdh_GetParent( objid, &objid);
	  }
	  if ( !found )
	  {
	    rtt_message('E', "Select a plcpgm object or an object in a plcpgm");
	    return RTT__NOPICTURE;
	  }
	  sts = rttsys_start_grafcet_monitor( ctx, objid);
	  return sts;
	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}
	return RTT__SUCCESS;	
}

/*************************************************************************
*
* Name:		show_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "show" command i recieved.
*
**************************************************************************/

static int	show_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;

	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "VERSION", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW VERSION" */
	  char	message_str[80];
	 
	  strcpy( message_str, "Rtt version is ");
	  strcat( message_str, rtt_version);
	  rtt_message('I', message_str);
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "ALARMLIST", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW ALARMS" */
	  sts = rtt_cli( rtt_command_table, "ALARM SHOW", (void *) ctx, 0);
	  return sts;
	}
	else if ( strncmp( arg1_str, "EVENTLIST", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW EVENTLIST" */
	  sts = rtt_cli( rtt_command_table, "ALARM LIST", (void *) ctx, 0);
	  return sts;
	}
	else if ( strncmp( arg1_str, "FILE", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW FILE" */
	  char	arg2_str[80];
	  char	title[80];
	  char	filename[80];

	  if ( EVEN( rtt_get_qualifier( "rtt_arg2", arg2_str)))
	  {
	    filename[0] = '!';
	    rtt_get_defaultfilename( "*", &filename[1], ".rtt_com");
#if defined OS_ELN
	    /* VAXELN selects all versions of files, select the last version */
	    strcat( filename, ".0");
#endif
	    strcpy( title, "STORED PICTURES AND COMMANDFILES");
	  }
	  else
	  {
	    rtt_get_defaultfilename( arg2_str, filename, ".rtt_com");
	    strcpy( title, "FILE LIST");
	  }

	  sts = rtt_show_file( ctx, filename, NULL, title);
	  if ( EVEN(sts))
	  {
	    rtt_message('E', "No files found");
	    return RTT__NOPICTURE;
	  }
	  return sts;
	}
	else if ( strncmp( arg1_str, "SYMBOL", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW SYMBOL" */
	  char	arg2_str[80];
	  char	message_str[80];
	  char  value[80];
	 
	  if ( ODD( rtt_get_qualifier( "/ALL", value)))
	  {
	    sts = rtt_show_symbols( ctx);
	    return sts;
	  }
	  else
	  {
	    if ( EVEN( rtt_get_qualifier( "rtt_arg2", arg2_str)))
	    {
	      sts = rtt_show_symbols( ctx);
	      return sts;
	    }
	    sts = rtt_get_symbol( arg2_str, value);
	    if (EVEN(sts))
	    {
	      rtt_message('E', "Symbol not found");
	      return RTT__HOLDCOMMAND;
	    }
	    sprintf( message_str, "Symbol %s = %s", arg2_str, value);
	    rtt_message('I', message_str);
	    return RTT__NOPICTURE;
	  }
	}
	else if ( strncmp( arg1_str, "TIME", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW TIME" */
	  char	message_str[80];
	 
	  rtt_update_time();
	  sprintf( message_str, "Time is %s", rtt_time);
	  rtt_message('I', message_str);
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "CLOCK", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW CLOCK" */
	 
#ifdef OS_VMS
	  rtt_message('E' ,"Not implemented in VMS");
	  return RTT__NOPICTURE;
#endif
#ifdef OS_ELN
	  sts = rtt_rtc( RTT_RTC_SHOW);
	  return sts;
#endif
	}
	else if ( strncmp( arg1_str, "DEFAULT", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW DEFAULT" */
	  char	message_str[80];
	 
	  sprintf( message_str, "Default directory: %s", rtt_default_directory);
	  rtt_message('I', message_str);
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "MENU", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW MENU" */
	  char	name_str[80];

	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] == '/')
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    if ( EVEN( rtt_get_qualifier( "/NAME", name_str)))
	    {
	      rtt_message('E', "Syntax error, name not found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_show_menu( ctx, name_str);
	  return sts;
	}
	else if ( strncmp( arg1_str, "HIERARCHY", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW HIERARCHY" */
	  IF_NOGDH_RETURN;
	  sts = rtt_hierarchy( ctx, pwr_cNObjid, 0, 0, 0, 0);
	  return sts;
	}
	else if ( strncmp( arg1_str, "CHILDREN", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW CHILDREN" */
	  char		name_str[80];
	  pwr_tObjid	objid;

	  IF_NOGDH_RETURN;
	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	    sts = gdh_ObjidToName ( objid, name_str, sizeof(name_str),
			cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Select an object or enter name");
	      return RTT__NOPICTURE;
	    }
	  }
	  sts = rtt_hierarchy_child( ctx, objid, 0, 0, 0, 0);
	  return sts;
	}
	else if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW OBJECT" */
	  char	class_str[80];
	  char	name_str[80];
	  char	hierarchy_str[80];
	  char	objdid_str[80];
	  char	maxobjects_str[80];
	  char	type_str[80];
	  char	file_str[80];
	  int	maxobjects;
	  char	*class_ptr;
	  char	*name_ptr;
	  char	*hierarchy_ptr;
	  int	global;
	  char	str[80];
	  int	nr;

	  IF_NOGDH_RETURN;
	  if ( ODD( rtt_get_qualifier( "/OBJID", objdid_str)))
	  {
	    pwr_tObjid		objid;
	    pwr_tClassId	class;

	    /* Convert to objid */
	    sts = cdh_StringToObjid( objdid_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Syntax error in objid");
	      return RTT__HOLDCOMMAND;
	    }
	    sts = gdh_GetObjectClass (objid, &class);
	    if ( EVEN(sts)) 
	    {
	      rtt_message('E', "Objid not found");
	      return RTT__HOLDCOMMAND;
	    }
	    /* Get the object name */
	    sts = gdh_ObjidToName ( objid, name_str, sizeof(name_str), 
			cdh_mName_volumeStrict);

	    if ( EVEN(sts)) return sts;

	    name_ptr = name_str;
	    sts = rtt_show_obj_hier_class_name( ctx, 0, 0,
			name_ptr, 0, 0);
	    return sts;
	  }
	  else if ( ODD( rtt_get_qualifier( "/TYPE", type_str)))
	  {
	    pwr_tObjid		objid;

	    if ( EVEN( rtt_get_qualifier( "/FILE", file_str)))
	    {
	      rtt_message('E', "Enter filename");
	      return RTT__HOLDCOMMAND;
	    }
	    if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	    {
	      sts = gdh_NameToObjid ( name_str, &objid);
	      if ( EVEN(sts))
	      {
	        rtt_message('E', "Object not found");
	        return RTT__NOPICTURE;
	      }
	      sts = gdh_ObjidToName ( objid, name_str, sizeof(name_str),
			cdh_mName_volumeStrict);
	      if ( EVEN(sts)) return sts;
	    }
	    else
	    {
	      /* Get the selected object */
	      sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	      if ( EVEN(sts))
	      {
	        rtt_message('E', "Select an object or enter name");
	        return RTT__NOPICTURE;
	      }
	    }
	    sts = rtt_show_object_as_struct( ctx, objid, type_str, file_str);
	    return sts;
	  }

	  /* Get maxobjects qualifier */
	  if ( ODD( rtt_get_qualifier( "/MAXOBJECTS", maxobjects_str)))
	  {
	    /* Convert to objid */
	    nr = sscanf( maxobjects_str, "%d", &maxobjects);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error in maxobjects");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    /* Default value */
	    maxobjects = 0;

	  /* Get the name qualifier */
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] != '/')
	      /* Assume that this is the namestring */
	      name_ptr = name_str;
	    else
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else
	  {
	    if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	      name_ptr = name_str;
	    else
	      name_ptr = NULL;
	  }

	  if ( ODD( rtt_get_qualifier( "/CLASS", class_str)))
	    class_ptr = class_str;
	  else
	    class_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/HIERARCHY", hierarchy_str)))
	    hierarchy_ptr = hierarchy_str;
	  else
	    hierarchy_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/LOCAL", str)))
	    global = 0;
	  else
	    global = 1;

	  sts = rtt_show_obj_hier_class_name( ctx, hierarchy_ptr, class_ptr,
			name_ptr, global, maxobjects);
	  return sts;
	}

	else if ( strncmp( arg1_str, "OBJID", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW OBJID" */
	  char		name_str[80];
	  pwr_tObjid	objid;
	  char		msg[160];

	  IF_NOGDH_RETURN;
	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	    sts = gdh_ObjidToName ( objid, name_str, sizeof(name_str),
			cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Select an object or enter name");
	      return RTT__NOPICTURE;
	    }
	  }

	  sprintf( msg, "Objid %s, Name %s", cdh_ObjidToString( NULL, objid, 0),
			name_str);
	  rtt_message('I', msg);
	  return RTT__NOPICTURE;
	}

	else if ( strncmp( arg1_str, "SIGNALS", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW SIGNALS" */
	  char		file_str[80];
	  char		name_str[80];
	  char		*file_ptr;
	  char		*name_ptr;
	  pwr_tObjid	objid;
	  pwr_tObjid	parentobjid;
	  pwr_tClassId	class;
	  char		classname[80];

	  IF_NOGDH_RETURN;
	  if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	    file_ptr = file_str;
	  else
	    file_ptr = NULL;

	  /* Get the name qualifier */
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] != '/')
	      /* Assume that this is the namestring */
	      name_ptr = name_str;
	    else
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	    name_ptr = name_str;
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, name_str, 
			sizeof( name_str), cdh_mName_path | cdh_mName_object);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	    name_ptr = name_str;

	    sts = gdh_GetObjectClass ( objid, &class);
	    if ( EVEN(sts)) return sts;
	    sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	    /* Check that this is a or a plc */	
	    if ( strcmp( classname, "pwrb:Class-PlcPgm") == 0)
	    {
	      /* Get all the windows in the plc */
	      strcat( name_str, "-W*");
	    }
	    else if (!( (strcmp( classname, "pwrb:Class-WindowPlc") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowOrderact") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowCond") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowSubstep") == 0)))
	    {
	      /* Try with the parent */
	      sts = gdh_GetParent( objid, &parentobjid);
	      if ( EVEN(sts)) return sts;
	      sts = gdh_GetObjectClass ( parentobjid, &class);
	      if ( EVEN(sts)) return sts;
	      sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	      if ( EVEN(sts)) return sts;

	      if (!( (strcmp( classname, "pwrb:Class-WindowPlc") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowOrderact") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowCond") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowSubstep") == 0)))
	      {
	        rtt_message('E', "Selected object has to be in a plcpgm or a plcpgm");
	        return RTT__HOLDCOMMAND;
	      }
	      else
	      {
	        sts = gdh_ObjidToName ( parentobjid, name_str, sizeof(name_str), 
			cdh_mName_volumeStrict);
	        if ( EVEN(sts)) return sts;
	      }
	    }
	  }
	  sts = rtt_show_signals( ctx, file_ptr, name_ptr, 0);
	  return sts;
	}

	else if ( strncmp( arg1_str, "STEP", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW STEP" */
	  char		str[80];
	  int		initstep;
	  char		name_str[80];
	  char		*name_ptr;
	  pwr_tObjid	objid;
	  pwr_tObjid	parentobjid;
	  pwr_tClassId	class;
	  char		classname[80];

	  IF_NOGDH_RETURN;
	  if ( ODD( rtt_get_qualifier( "/INITSTEP", str)))
	    initstep = 1;
	  else
	    initstep = 0;

	  /* Get the name qualifier */
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] != '/')
	      /* Assume that this is the namestring */
	      name_ptr = name_str;
	    else
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else if ( ODD( rtt_get_qualifier( "/HIERARCHY", name_str)))
	  {
	    name_ptr = name_str;
	    if ( *name_ptr == 0)
	    {
	      /* Get the selected object */
	      sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	      if ( EVEN(sts))
	      {
	        rtt_message('E', "Enter hierarchy or select an object");
	        return RTT__HOLDCOMMAND;
	      }
	      name_ptr = name_str;

  	      sts = gdh_GetObjectClass ( objid, &class);
	      if ( EVEN(sts)) return sts;
	      sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	      if ( EVEN(sts)) return sts;
	      /* Check that this is a or a plc */	
	      if (( strcmp( classname, "pwrb:Class-PlcPgm") == 0) ||
	    	( strcmp( classname, "pwrs:Class-$PlantHier") == 0))
	      {
	        /* This is ok */
	      }
	      else if (!( (strcmp( classname, "pwrb:Class-WindowPlc") == 0) ||
	         (strcmp( classname, "pwrb:Class-WindowOrderact") == 0) ||
	         (strcmp( classname, "pwrb:Class-WindowCond") == 0) ||
	         (strcmp( classname, "pwrb:Class-WindowSubstep") == 0)))
	      {
	        /* Try with the parent */
	        sts = gdh_GetParent( objid, &parentobjid);
	        if ( EVEN(sts)) return sts;
	        sts = gdh_GetObjectClass ( parentobjid, &class);
	        if ( EVEN(sts)) return sts;
	        sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	        if ( EVEN(sts)) return sts;

	        if (!( (strcmp( classname, "pwrb:Class-WindowPlc") == 0) ||
	         (strcmp( classname, "pwrb:Class-WindowOrderact") == 0) ||
	         (strcmp( classname, "pwrb:Class-WindowCond") == 0) ||
	         (strcmp( classname, "pwrb:Class-WindowSubstep") == 0)))
	        {
	          rtt_message('E', "Selected object has to be a planthier, plcpgm or in a plcpgm");
	          return RTT__NOPICTURE;
	        }
	        else
	        {
	          sts = gdh_ObjidToName ( parentobjid, name_str, sizeof(name_str),
			cdh_mName_volumeStrict);
	          if ( EVEN(sts)) return sts;
	        }
	      }
	    }
	  }
	  else
	    name_ptr = NULL;
	  sts = rtt_show_step( ctx, name_ptr, initstep);
	  return sts;
	}

	else if ( strncmp( arg1_str, "CLASS", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW CLASS" */
	  char	volume_str[80];
	  char	name_str[80];
	  pwr_tClassId class;
	  int		index = 0;
	  rtt_t_menu	*menulist = 0;
	  char		title[80] = "Show Class";
	  pwr_tObjid	objid;

	  IF_NOGDH_RETURN;

	  if (  ODD( rtt_get_qualifier( "/VOLUME", volume_str)))
	  {
	    if ( !strchr( volume_str, ':'))
	      strcat( volume_str, ":");
	    strcat( volume_str, "Class");

	    sts = gdh_NameToObjid ( volume_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Volume not found");
	      return RTT__NOPICTURE;
	    }

	    sts = rtt_hierarchy_child( ctx, objid, 0, 0, 0, 0);
	    return sts;
	  }
	  else 
	  {
	    if ( EVEN( rtt_get_qualifier( "/NAME", name_str)))
	      if ( EVEN( rtt_get_qualifier( "rtt_arg2", name_str)))
	      {
	        rtt_message('E',"Enter name");	
	        return RTT__HOLDCOMMAND;
	      }

	    sts = gdh_ClassNameToNumber( name_str, &class);
	    if ( EVEN(sts))
	    {
	      rtt_message('E',"No such class");	
	      return RTT__HOLDCOMMAND;
	    }

	    sts = rtt_show_object_add( cdh_ClassIdToObjid( class), &menulist,
			&index, 0, 0, 0);
	    if ( EVEN(sts)) return sts;
	    sts = rtt_menu_new( ctx, pwr_cNObjid, &menulist, title, 0, 
	  				RTT_MENUTYPE_DYN);
	    return sts;
	  }

#if 0  /* NYDB */
	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	    name_ptr = name_str;
	  else
	    name_ptr = NULL;
	  class_ptr = class_str;
	  if ( EVEN( rtt_get_qualifier( "/CLASS", class_str)))
	    strcpy( class_str, "$CLASSDEF");
	  hierarchy_ptr = hierarchy_str;
	  if ( EVEN( rtt_get_qualifier( "/HIERARCHY", hierarchy_str)))
	    strcpy( hierarchy_str, "pwrb:Class");

	  sts = rtt_show_class_hier_class_name( ctx, hierarchy_ptr, 
			class_ptr, name_ptr);
	  return sts;
#endif
	}

	else if ( strncmp( arg1_str, "PARAMETER", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW PARAMETER" */
	  char	parameter_str[80];
	  char	class_str[80];
	  char	name_str[80];
	  char	hierarchy_str[80];
	  char	*class_ptr;
	  char	*name_ptr;
	  char	*hierarchy_ptr;
	  char	*parameter_ptr;
	  int	global;
	  char		str[80];
	  int	nr;
	  char	maxobjects_str[80];
	  int	maxobjects;

	  IF_NOGDH_RETURN;
	  /* Get maxobjects qualifier */
	  if ( ODD( rtt_get_qualifier( "/MAXOBJECTS", maxobjects_str)))
	  {
	    /* Convert to objid */
	    nr = sscanf( maxobjects_str, "%d", &maxobjects);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error in maxobjects");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    /* Default value */
	    maxobjects = 0;

	  if ( ODD( rtt_get_qualifier( "/PARAMETER", parameter_str)))
	    parameter_ptr = parameter_str;
	  else
	    parameter_ptr = NULL;

	  /* Get the name qualifier */
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] != '/')
	      /* Assume that this is the namestring */
	      name_ptr = name_str;
	    else
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else
	  {
	    if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	      name_ptr = name_str;
	    else
	      name_ptr = NULL;
	  }
	  if ( ODD( rtt_get_qualifier( "/CLASS", class_str)))
	    class_ptr = class_str;
	  else
	    class_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/HIERARCHY", hierarchy_str)))
	    hierarchy_ptr = hierarchy_str;
	  else
	    hierarchy_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/LOCAL", str)))
	    global = 0;
	  else
	    global = 1;

	  sts = rtt_show_par_hier_class_name( ctx, parameter_ptr,  
		hierarchy_ptr, class_ptr, name_ptr, RTT_MENU_CREATE, global,
		maxobjects); 
	  return sts; 
	} 
	else if ( strncmp( arg1_str, "CONVERSION", strlen( arg1_str)) == 0) 
	{ 
	  int	on = 0;
	  pwr_tObjid	objid; 
	  char		name_str[80];

	  IF_NOGDH_RETURN;

	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_set_conversion( objid, on, 1);
	  return sts;
	}
	else if ( strncmp( arg1_str, "INVERT", strlen( arg1_str)) == 0)
	{
	  int	on = 0;
	  pwr_tObjid	objid;
	  char		name_str[80];

	  IF_NOGDH_RETURN;

	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_set_invert( objid, on, 1);
	  return sts;
	}
	else if ( strncmp( arg1_str, "DOTEST", strlen( arg1_str)) == 0)
	{
	  int	on = 0;
	  pwr_tObjid	objid;
	  char		name_str[80];

	  IF_NOGDH_RETURN;

	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_set_do_test( objid, on, 1);
	  return sts;
	}
	else if ( strncmp( arg1_str, "TESTVALUE", strlen( arg1_str)) == 0)
	{
	  int	on = 0;
	  pwr_tObjid	objid;
	  char		name_str[80];

	  IF_NOGDH_RETURN;

	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str) ,cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_set_do_testvalue( objid, on, 1);
	  return sts;
	}
	else
	{
	  /* This might be a system picture */
	  sts = rttsys_start_system_picture( ctx, arg1_str);
	  if ( sts == 0)
	  {
	    rtt_message('E', "Unknown qualifier");
	    return RTT__HOLDCOMMAND;
	  }
	  return sts;
	}
	  	
	return RTT__SUCCESS;	
}

  /*************************************************************************
*
* Name:		debug_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "debug" command i recieved.
*
**************************************************************************/

static int	debug_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;

	IF_NOGDH_RETURN;
	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0)
	{
	  /* Command is "SHOW OBJECT" */
	  char	class_str[80];
	  char	name_str[80];
	  char	hierarchy_str[80];
	  char	*class_ptr;
	  char	*name_ptr;
	  char	*hierarchy_ptr;
	  int	global;
	  char		str[80];

	  /* Get the name qualifier */
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] != '/')
	      /* Assume that this is the namestring */
	      name_ptr = name_str;
	    else
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else
	  {
	    if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	      name_ptr = name_str;
	    else
	      name_ptr = NULL;
	  }
	  if ( ODD( rtt_get_qualifier( "/CLASS", class_str)))
	    class_ptr = class_str;
	  else
	    class_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/HIERARCHY", hierarchy_str)))
	    hierarchy_ptr = hierarchy_str;
	  else
	    hierarchy_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/LOCAL", str)))
	    global = 0;
	  else
	    global = 1;

	  sts = rtt_debug_obj_hier_class_name( ctx, hierarchy_ptr, class_ptr,
			name_ptr, RTT_MENU_CREATE, global);
	  return sts;
	}
	else if ( strncmp( arg1_str, "CHILDREN", strlen( arg1_str)) == 0)
	{
	  /* Command is "DEBUG CHILDEN" */
	  char		name_str[80];
	  pwr_tObjid	objid;

	  /* Get the name qualifier */
	  if ( EVEN( rtt_get_qualifier( "rtt_arg2", name_str)))
	    if ( EVEN( rtt_get_qualifier( "/NAME", name_str)))
	    {
	      rtt_message('E',"Enter name");	
	      return RTT__HOLDCOMMAND;
	    }

	  if ( name_str[0] == 0)
	  {
	    rtt_message('E',"Enter name");	
	    return RTT__HOLDCOMMAND;
	  }
	  /* Convert name to objid */
	  sts = rtt_find_name( ctx, name_str, &objid);
	  if ( EVEN(sts))
	  {
     	    rtt_message('E',"Object does not exist");
	    return RTT__HOLDCOMMAND;
	  }
	  sts = rtt_debug_child( ctx, objid, 0, 0, 0, 0);
	  return sts;	  
	}

	else if ( strncmp( arg1_str, "SIGNALS", strlen( arg1_str)) == 0)
	{
	  /* Command is "DEBUG SIGNALS" */
	  char		file_str[80];
	  char		name_str[80];
	  char		*file_ptr;
	  char		*name_ptr;
	  pwr_tObjid	objid;
	  pwr_tObjid	parentobjid;
	  pwr_tClassId	class;
	  char		classname[80];

	  if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	    file_ptr = file_str;
	  else
	    file_ptr = NULL;

	  /* Get the name qualifier */
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] != '/')
	      /* Assume that this is the namestring */
	      name_ptr = name_str;
	    else
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	    name_ptr = name_str;
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, name_str, 
			sizeof( name_str), cdh_mName_path | cdh_mName_object);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	    name_ptr = name_str;

	    sts = gdh_GetObjectClass ( objid, &class);
	    if ( EVEN(sts)) return sts;
	    sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	    /* Check that this is a or a plc */	
	    if ( strcmp( classname, "pwrb:Class-PlcPgm") == 0)
	    {
	      /* Get all the windows in the plc */
	      strcat( name_str, "-W*");
	    }
	    else if (!( (strcmp( classname, "pwrb:Class-WindowPlc") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowOrderact") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowCond") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowSubstep") == 0)))
	    {
	      /* Try with the parent */
	      sts = gdh_GetParent( objid, &parentobjid);
	      if ( EVEN(sts)) return sts;
	      sts = gdh_GetObjectClass ( parentobjid, &class);
	      if ( EVEN(sts)) return sts;
	      sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	      if ( EVEN(sts)) return sts;

	      if (!( (strcmp( classname, "pwrb:Class-WindowPlc") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowOrderact") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowCond") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowSubstep") == 0)))
	      {
	        rtt_message('E', "Selected object has to be in a plcpgm or a plcpgm");
	        return RTT__HOLDCOMMAND;
	      }
	      else
	      {
	        sts = gdh_ObjidToName ( parentobjid, name_str, sizeof(name_str), 
				cdh_mName_volumeStrict);
	        if ( EVEN(sts)) return sts;
	      }
	    }
	  }
	  sts = rtt_show_signals( ctx, file_ptr, name_ptr, 1);
	  return sts;
	}

	return RTT__SUCCESS;	
}

/*************************************************************************
*
* Name:		add_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "add" command i recieved.
*
**************************************************************************/

static int	add_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;

	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "PARAMETER", strlen( arg1_str)) == 0)
	{
	  /* Command is "ADD PARAMETER" */
	  char	parameter_str[80];
	  char	class_str[80];
	  char	name_str[80];
	  char	hierarchy_str[80];
	  char	*class_ptr;
	  char	*name_ptr;
	  char	*hierarchy_ptr;
	  char	*parameter_ptr;
	  int	global;
	  char	str[80];

	  IF_NOGDH_RETURN;

	  /* Check that this is a update menu */
	  if ( !(ctx->menutype & RTT_MENUTYPE_UPD) )
	  {
	    rtt_message('E',"Previous command has to be show parameter");
	    return RTT__HOLDCOMMAND;
	  }

	  if ( ODD( rtt_get_qualifier( "/PARAMETER", parameter_str)))
	    parameter_ptr = parameter_str;
	  else
	    parameter_ptr = NULL;

	  /* Get the name qualifier */
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] != '/')
	      /* Assume that this is the namestring */
	      name_ptr = name_str;
	    else
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else
	  {
	    if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	      name_ptr = name_str;
	    else
	      name_ptr = NULL;
	  }
	  if ( ODD( rtt_get_qualifier( "/CLASS", class_str)))
	    class_ptr = class_str;
	  else
	    class_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/HIERARCHY", hierarchy_str)))
	    hierarchy_ptr = hierarchy_str;
	  else
	    hierarchy_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/LOCAL", str)))
	    global = 0;
	  else
	    global = 1;

	  sts = rtt_show_par_hier_class_name( ctx, parameter_ptr, 
			hierarchy_ptr, class_ptr, name_ptr, RTT_MENU_ADD,
			global, 0);
	  return sts;
	}

	if ( strncmp( arg1_str, "DEBUG", strlen( arg1_str)) == 0)
	{
	  /* Command is "ADD DEBUG" */
	  char	class_str[80];
	  char	name_str[80];
	  char	hierarchy_str[80];
	  char	*class_ptr;
	  char	*name_ptr;
	  char	*hierarchy_ptr;
	  int	global;
	  char	str[80];

	  IF_NOGDH_RETURN;

	  /* Check that this is a update menu */
	  if ( !(ctx->menutype & RTT_MENUTYPE_UPD) )
	  {
	    rtt_message('E',"Previous command has to be show parameter");
	    return RTT__HOLDCOMMAND;
	  }

	  /* Get the name qualifier */
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", name_str)))
	  {
	    if ( name_str[0] != '/')
	      /* Assume that this is the namestring */
	      name_ptr = name_str;
	    else
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else
	  {
	    if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	      name_ptr = name_str;
	    else
	      name_ptr = NULL;
	  }
	  if ( ODD( rtt_get_qualifier( "/CLASS", class_str)))
	    class_ptr = class_str;
	  else
	    class_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/HIERARCHY", hierarchy_str)))
	    hierarchy_ptr = hierarchy_str;
	  else
	    hierarchy_ptr = NULL;
	  if ( ODD( rtt_get_qualifier( "/LOCAL", str)))
	    global = 0;
	  else
	    global = 1;

	  sts = rtt_debug_obj_hier_class_name( ctx,  
			hierarchy_ptr, class_ptr, name_ptr, RTT_MENU_ADD,
			global);
	  return sts;
	}
	else if ( strncmp( arg1_str, "MENU", strlen( arg1_str)) == 0)
	{
	  /* Command is "ADD MENU" */
	  int		sts;
	  char		object_str[80];
	  char		text_str[80];
	  char		command_str[80];
	  rtt_t_menu	*menu_ptr;
	  int		i;
	  int		command;
	  int		object;
	  pwr_tObjid	objid;

	  if ( EVEN( rtt_get_qualifier( "/TEXT", text_str)))
	  {
	    rtt_message('E',"Enter text");	
	    return RTT__HOLDCOMMAND;
	  }
	  if ( ODD( rtt_get_qualifier( "/COMMAND", command_str)))
	    command = 1;
	  else
	    command = 0;
	  if ( ODD( rtt_get_qualifier( "/OBJECT", object_str)))
	  {
	    IF_NOGDH_RETURN;
	    object = 1;
	    sts = gdh_NameToObjid ( object_str, &objid);
	    if (EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    object = 0;

	  i = 0;
	  for ( menu_ptr = ctx->menu; menu_ptr->text[0]; menu_ptr++)
	    i++;
	  sts = rtt_menu_list_add( &ctx->menu, i, 0, text_str,
		NULL, NULL, NULL,
		pwr_cNObjid,0,0,0,0);
	  if ( EVEN(sts)) return sts;

	  if ( command)
	  {
	    menu_ptr = ctx->menu + i;
	    menu_ptr->func = &rtt_menu_command;
	    menu_ptr->arg1 = menu_ptr->argstr;
	    strcpy( menu_ptr->arg1, command_str);
	  }
	  else if ( object)
	  {
	    menu_ptr = ctx->menu + i;	    
	    menu_ptr->func = &rtt_hierarchy_child;
	    menu_ptr->func2 = &rtt_object_parameters;
	    menu_ptr->func3 = &rtt_debug_child;
	    menu_ptr->argoi = objid;
	  }

	  rtt_menu_configure(ctx);
	  return RTT__SUCCESS;	
	}

	return RTT__SUCCESS;	
}

/*************************************************************************
*
* Name:		crossref_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "crossreference" command i recieved.
*	
**************************************************************************/

static int	crossref_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char		file_str[80];
	char		name_str[80];
	char		string_str[80];
	char		func_str[80];
	char		*file_ptr;
	char		*name_ptr;
	pwr_tObjid	objid;
	pwr_tClassId	class;
	int		brief;
	int		case_sens;

	  IF_NOGDH_RETURN;

	  if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	    file_ptr = file_str;
	  else
	    file_ptr = NULL;

	  if ( ODD( rtt_get_qualifier( "/STRING", string_str)))
	  {
	    if ( ODD( rtt_get_qualifier( "/FUNCTION", func_str)))
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND; 	
	    }
	    brief = ODD( rtt_get_qualifier( "/BRIEF", name_str));
	    case_sens = ODD( rtt_get_qualifier( "/CASE_SENSITIVE", name_str));
	    sts = rtt_crr_code( file_ptr, string_str, brief, 0, case_sens);
	  }
	  else if ( ODD( rtt_get_qualifier( "/FUNCTION", func_str)))
	  {
	    brief = ODD( rtt_get_qualifier( "/BRIEF", name_str));
	    case_sens = ODD( rtt_get_qualifier( "/CASE_SENSITIVE", name_str));
	    sts = rtt_crr_code( file_ptr, func_str, brief, 1, case_sens);
	  }
	  else
	  {
	    /* Get the name qualifier */
	    if ( ODD( rtt_get_qualifier( "rtt_arg1", name_str)))
	    {
	      if ( name_str[0] != '/')
	        /* Assume that this is the namestring */
	        name_ptr = name_str;
	      else
	      {
	        rtt_message('E', "Syntax error");
	        return RTT__HOLDCOMMAND; 	
	      } 
	    }
	    else if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	      name_ptr = name_str;
	    else
	    {
	      /* Get the selected object */
	      sts = rtt_get_current_object( ctx, &objid, name_str, 
		sizeof( name_str), cdh_mName_path | cdh_mName_object);
	      if ( EVEN(sts))
	      {
	        rtt_message('E', "Enter name or select an object");
	        return RTT__HOLDCOMMAND;
	      }
	      name_ptr = name_str;
	    }

	    sts = gdh_GetObjectClass ( objid, &class);
	    if ( EVEN(sts)) return sts;

	    switch ( class)
	    {
	      case pwr_cClass_Di:
	      case pwr_cClass_Dv:
	      case pwr_cClass_Do:
	      case pwr_cClass_Po:
	      case pwr_cClass_Av:
	      case pwr_cClass_Ai:
	      case pwr_cClass_Ao:
	        sts = rtt_crr_signal( file_ptr, name_ptr);
	        break;
	      default:
	        /* Not a signal */
	        sts = rtt_crr_object( file_ptr, name_ptr);
	    }
	  }
	  return sts;	  
}

/*************************************************************************
*
* Name:		print_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "print" command i recieved.
*	
**************************************************************************/

int	rttcmd_print_func(	menu_ctx	ctx,
				int		*flag)
{
	int		sts;
	char		file_str[80];
	char		*file_ptr;
	int		append;
	int		text_size;
	int		parameter_size;
	char		str[80];
	int		nr;

	  if ( ODD( rtt_get_qualifier( "/TERMINAL", str)))
	  {
            rtt_print_screen();
            rtt_message('I', "Screen printed");
            return RTT__NOPICTURE;
          }
	  else if ( ODD( rtt_get_qualifier( "/RESTORE", str)))
          {
	    /* Print as a restore command file */
	    if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	      file_ptr = file_str;
	    else if ( ODD( rtt_get_qualifier( "rtt_arg1", file_str)))
	    {
	      if ( file_str[0] != '/')
	        /* Assume that this is the filestring */
	        file_ptr = file_str;
	      else
	      {
	        rtt_message('E', "Enter filename");
	        return RTT__HOLDCOMMAND;
	      } 
	    }
            sts = rtt_print_picture_restore( ctx, file_ptr);
	    return RTT__NOPICTURE;
	  }
          else
          {
	    append = ODD( rtt_get_qualifier( "/APPEND", file_str));

	    if ( ODD( rtt_get_qualifier( "/TEXT", str)))
	    {
	      /* Print a simple text in a file */
	      if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	        file_ptr = file_str;
	      else if ( ODD( rtt_get_qualifier( "rtt_arg1", file_str)))
	      {
	        if ( file_str[0] != '/')
	          /* Assume that this is the filestring */
	          file_ptr = file_str;
	        else if ( rtt_file_on)
	          file_ptr = 0;
	        else
	        {
	          rtt_message('E', "Enter filename");
	          return RTT__HOLDCOMMAND;
	        } 
	      }
	      else if ( rtt_file_on)
	        file_ptr = 0;
	      else
	      {
	        rtt_message('E',"Enter filename");
	        return RTT__HOLDCOMMAND; 	
	      } 
	      sts = rtt_print_text( file_ptr, str, append);
	      return sts;
	    }

	    if ( ODD( rtt_get_qualifier( "/TSIZE", str)))
	    {
	      nr = sscanf( str, "%d", &text_size);
	      if ( nr != 1)
	      {
	        rtt_message('E', "Syntax error in textsize");
	        return RTT__HOLDCOMMAND;
	      }
	    }
	    else
	      text_size = 15;

	    if ( ODD( rtt_get_qualifier( "/PSIZE", str)))
	    {
	      nr = sscanf( str, "%d", &parameter_size);
	      if ( nr != 1)
	      {
	        rtt_message('E', "Syntax error in parsize");
	        return RTT__HOLDCOMMAND;
	      }
	    }
	    else
	      parameter_size = 25;

	    if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	      file_ptr = file_str;
	    else if ( ODD( rtt_get_qualifier( "rtt_arg1", file_str)))
	    {
	      if ( file_str[0] != '/')
	        /* Assume that this is the filestring */
	        file_ptr = file_str;
	      else
	      {
	        rtt_message('E', "Enter filename");
	        return RTT__HOLDCOMMAND; 	
	      } 
	    }
	    else if ( rtt_file_on)
	      file_ptr = 0;
	    else
	    {
	      rtt_message('E', "Enter filename");
	      return RTT__HOLDCOMMAND; 	
	    } 
	    sts = rtt_print_picture( ctx, file_ptr, append, text_size,
			parameter_size);
	    return sts;	  
	  }
}
/*************************************************************************
*
* Name:		say_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "say" command i recieved.
*	
**************************************************************************/

int	rttcmd_say_func( 	menu_ctx	ctx,
				int		*flag)
{
	char		str[80];

	  if ( ODD( rtt_get_qualifier( "/TEXT", str)))
	  {
	    printf( "\n%s", str);
	    return RTT__NOPICTURE;
	  }
	  else
	  {
	    rtt_message('E', "Enter text");
	    return RTT__HOLDCOMMAND;
	  }
}

/*************************************************************************
*
* Name:		collect_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "collect" command i recieved.
*	
**************************************************************************/

static int	collect_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	char	name_str[80];
	char	*name_ptr;

	IF_NOGDH_RETURN;

	if ( EVEN( rtt_get_qualifier( "rtt_arg1", arg1_str)))
	{
	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	    name_ptr = name_str;
	  else
	    name_ptr = NULL;

	  sts = rtt_collect_insert( ctx, name_ptr);
	  return sts;
	}
	else if ( strncmp( arg1_str, "SHOW", strlen( arg1_str)) == 0)
	{
	  sts = rtt_collect_show( ctx);
	  return sts;
	}
	else if ( strncmp( arg1_str, "CLEAR", strlen( arg1_str)) == 0)
	{
	  menu_ctx	dummyctx;

	  if ( ctx->menu == (rtt_t_menu *) rtt_collectionmenulist)
	  {
	    /* The current menu is the collectionmenu */
	    rtt_collectionmenulist = 0;
	    return RTT__BACK;
	  }
	  else
	  {
	    if ( rtt_collectionmenuctx != 0)
	      rtt_message('E', "Return to collection before clear");
	    else
	    {
	      /* Create a dummy ctx for the collection picture, then delete it */ 
	      sts = rtt_menu_create_ctx( &dummyctx, 0, 
		(rtt_t_menu *) rtt_collectionmenulist, "dummy", 0);
	      if ( EVEN(sts)) return sts;
	
	      rtt_collectionmenulist = 0;
	      if ( rtt_collectionmenuctx != 0)
	        rtt_collectionmenuctx->menu = 0;
	      rtt_menu_delete( dummyctx);
	    }
	  }	
	  return RTT__NOPICTURE;
	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}

	return RTT__SUCCESS;	
}

/*************************************************************************
*
* Name:		logging_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "logging" command i recieved.
*
**************************************************************************/

static int	logging_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;

	IF_NOGDH_RETURN;

	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "SET", strlen( arg1_str)) == 0)
	{
	  /* Command is "LOGGING SET" */

	  char	entry_str[80];
	  char	line_size_str[80];
	  char	parameter_str[80];
	  char	time_str[80];
	  char	file_str[80];
	  char	condition_str[80];
	  char	buffer_size_str[80];
	  char	priority_str[80];
	  char	str[80];
	  int	stop;
	  int	priority;
	  int	entry;
	  int	line_size;
	  char	*parameter_ptr;
	  int	logg_time;
	  char	*file_ptr;
	  char	*condition_ptr;
	  int	buffer_size;
	  int	nr;
	  int	logg_type;
	  int	insert;
	  int	create;
	  int	shortname;
	  char	title[80];

	  if ( ODD( rtt_get_qualifier( "/ENTRY", entry_str)))
	  {
	    if ( !strcmp( entry_str, "CURRENT"))
	    {
	      rtt_toupper( title, ctx->title);
	      if ( strcmp( title, "LOGGING"))
	      {
	        rtt_message('E', "No current logging entry, enter logging picture");
	        return RTT__NOPICTURE;
	      }
	      sts = rttsys_get_current_logg_entry( &entry);
	    }
	    else
	    {
	      /* convert to integer */
	      nr = sscanf( entry_str, "%d", &entry);
	      if ( nr != 1)
	      {
	        rtt_message('E', "Entry syntax error");
	        return RTT__HOLDCOMMAND;
	      }
	    }
	  }
	  else
	  {
	    rtt_message('E',"Enter entry");	
	    return RTT__HOLDCOMMAND;
	  }

	  if ( ODD( rtt_get_qualifier( "/TIME", time_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( time_str, "%d", &logg_time);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Time syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    logg_time = 0;

	  if ( ODD( rtt_get_qualifier( "/BUFFER_SIZE", buffer_size_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( buffer_size_str, "%d", &buffer_size);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Buffer size syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    buffer_size = 0;

	  if ( ODD( rtt_get_qualifier( "/LINE_SIZE", line_size_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( line_size_str, "%d", &line_size);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Line size syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    line_size = 0;

	  if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	    file_ptr = file_str;
	  else
	    file_ptr = NULL;

	  if ( ODD( rtt_get_qualifier( "/PARAMETER", parameter_str)))
	    parameter_ptr = parameter_str;
	  else
	    parameter_ptr = NULL;

	  if ( ODD( rtt_get_qualifier( "/CONDITION", condition_str)))
	    condition_ptr = condition_str;
	  else
	    condition_ptr = NULL;

	  if ( ODD( rtt_get_qualifier( "/TYPE", str)))
	  {
	    if ( strncmp( str, "EVENT", strlen( str)) == 0)
	      logg_type = RTT_LOGG_MOD;
	    else if ( strncmp( str, "CONTINOUS", strlen( str)) == 0)
	      logg_type = RTT_LOGG_CONT;
	  }
	  else
	    logg_type = 0;

	  if ( ODD( rtt_get_qualifier( "/PRIORITY", priority_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( priority_str, "%d", &priority);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Priority syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    priority = -1;

	  if ( ODD( rtt_get_qualifier( "/STOP", str)))
	    stop = 1;
	  else if ( ODD( rtt_get_qualifier( "/NOSTOP", str)))
	    stop = 0;
	  else
	    stop = -1;

	  if ( ODD( rtt_get_qualifier( "/INSERT", str)))
	    insert = 1;
	  else
	    insert = 0;

	  if ( ODD( rtt_get_qualifier( "/SHORTNAME", str)))
	    shortname = 1;
	  else if ( ODD( rtt_get_qualifier( "/NOSHORTNAME", str)))
	    shortname = 0;
	  else 
	    shortname = -1;

	  if ( ODD( rtt_get_qualifier( "/CREATE", str)))
	    create = 1;
	  else
	    create = 0;

	  sts = rtt_logging_set( ctx, entry, logg_time, file_ptr, 
		parameter_ptr, condition_ptr, logg_type, insert, buffer_size,
		stop, priority, create, line_size, shortname);
	  return sts;
	}

	else if ( strncmp( arg1_str, "CREATE", strlen( arg1_str)) == 0)
	{
	  /* Command is "LOGGING CREATE" */

	  char	entry_str[80];
	  char	line_size_str[80];
	  char	parameter_str[80];
	  char	time_str[80];
	  char	file_str[80];
	  char	condition_str[80];
	  char	buffer_size_str[80];
	  char	priority_str[80];
	  char	str[80];
	  int	stop;
	  int	priority;
	  int	entry;
	  int	line_size;
	  char	*parameter_ptr;
	  int	logg_time;
	  char	*file_ptr;
	  char	*condition_ptr;
	  int	buffer_size;
	  int	nr;
	  int	logg_type;
	  int	insert;
	  int	shortname;

	  if ( ODD( rtt_get_qualifier( "/ENTRY", entry_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( entry_str, "%d", &entry);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Entry syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	    if ( entry == 0)
	    {
	      rtt_message('E', "Entry is out of range");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    entry = 0;

	  if ( ODD( rtt_get_qualifier( "/TIME", time_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( time_str, "%d", &logg_time);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Time syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    logg_time = 0;

	  if ( ODD( rtt_get_qualifier( "/LINE_SIZE", line_size_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( line_size_str, "%d", &line_size);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Line size syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    line_size = 0;

	  if ( ODD( rtt_get_qualifier( "/BUFFER_SIZE", buffer_size_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( buffer_size_str, "%d", &buffer_size);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Buffer size syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    buffer_size = 0;

	  if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	    file_ptr = file_str;
	  else
	    file_ptr = NULL;

	  if ( ODD( rtt_get_qualifier( "/PARAMETER", parameter_str)))
	    parameter_ptr = parameter_str;
	  else
	    parameter_ptr = NULL;

	  if ( ODD( rtt_get_qualifier( "/CONDITION", condition_str)))
	    condition_ptr = condition_str;
	  else
	    condition_ptr = NULL;

	  if ( ODD( rtt_get_qualifier( "/TYPE", str)))
	  {
	    if ( strncmp( str, "EVENT", strlen( str)) == 0)
	      logg_type = RTT_LOGG_MOD;
	    else if ( strncmp( str, "CONTINOUS", strlen( str)) == 0)
	      logg_type = RTT_LOGG_CONT;
	  }
	  else 
	    logg_type = 0;

	  if ( ODD( rtt_get_qualifier( "/PRIORITY", priority_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( priority_str, "%d", &priority);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Priority syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    priority = -1;

	  if ( ODD( rtt_get_qualifier( "/STOP", str)))
	    stop = 1;
	  else if ( ODD( rtt_get_qualifier( "/NOSTOP", str)))
	    stop = 0;
	  else
	    stop = -1;

	  if ( ODD( rtt_get_qualifier( "/INSERT", str)))
	    insert = 1;
	  else
	    insert = 0;

	  if ( ODD( rtt_get_qualifier( "/SHORTNAME", str)))
	    shortname = 1;
	  else if ( ODD( rtt_get_qualifier( "/NOSHORTNAME", str)))
	    shortname = 0;
	  else 
	    shortname = -1;

	  sts = rtt_logging_create( ctx, entry, logg_time, file_ptr, 
		parameter_ptr, condition_ptr, logg_type, insert, buffer_size, 
		stop, priority, line_size, shortname);
	  return sts;
	}

	else if ( strncmp( arg1_str, "DELETE", strlen( arg1_str)) == 0)
	{
	  /* Command is "LOGGING DELETE" */

	  char	entry_str[80];
	  char	parameter_str[80];
	  char	*parameter_ptr;
	  int	entry;
	  int	nr;

	  if ( ODD( rtt_get_qualifier( "/ENTRY", entry_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( entry_str, "%d", &entry);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Entry syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    rtt_message('E',"Enter entry");	
	    return RTT__HOLDCOMMAND;
	  }
	  if ( ODD( rtt_get_qualifier( "/PARAMETER", parameter_str)))
	    parameter_ptr = parameter_str;
	  else
	    parameter_ptr = NULL;

	  sts = rtt_logging_delete( ctx, entry, parameter_ptr);
	  return sts;
	}

	else if ( strncmp( arg1_str, "START", strlen( arg1_str)) == 0)
	{
	  /* Command is "LOGGING START" */

	  char	entry_str[80];
	  int	entry;
	  int	nr;

	  if ( ODD( rtt_get_qualifier( "/ENTRY", entry_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( entry_str, "%d", &entry);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Entry syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    rtt_message('E',"Enter entry");	
	    return RTT__HOLDCOMMAND;
	  }
	  sts = rtt_logging_start( ctx, entry);
	  return sts;
	}

	else if ( strncmp( arg1_str, "STOP", strlen( arg1_str)) == 0)
	{
	  /* Command is "LOGGING STOP" */

	  char	entry_str[80];
	  int	entry;
	  int	nr;

	  if ( ODD( rtt_get_qualifier( "/ENTRY", entry_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( entry_str, "%d", &entry);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Entry syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    rtt_message('E',"Enter entry");	
	    return RTT__HOLDCOMMAND;
	  }
	  sts = rtt_logging_stop( ctx, entry);
	  return sts;
	}
	else if ( strncmp( arg1_str, "SHOW", strlen( arg1_str)) == 0)
	{
	  /* Command is "LOGGING SHOW" */

	  char	entry_str[80];
	  int	entry;
	  int	nr;

	  if ( ODD( rtt_get_qualifier( "/ENTRY", entry_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( entry_str, "%d", &entry);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Entry syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    entry = 0;
	  }
	  sts = rtt_logging_show( ctx, entry);
	  return sts;
	}
	else if ( strncmp( arg1_str, "STORE", strlen( arg1_str)) == 0)
	{
	  /* Command is "LOGGING STORE" */

	  char	entry_str[80];
	  char	file_str[80];
	  char  *file_ptr;
	  int	entry;
	  int	nr;

	  if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	    file_ptr = file_str;
	  else if ( ODD( rtt_get_qualifier( "rtt_arg2", file_str)))
	  {
	    if ( file_str[0] != '/')
	      /* Assume that this is the filestring */
	      file_ptr = file_str;
	    else
	    {
	      rtt_message('E',"Enter file");	
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else
	  {
	    rtt_message('E',"Enter file");	
	    return RTT__HOLDCOMMAND; 	
	  } 

	  if ( ODD( rtt_get_qualifier( "/ALL", entry_str)))
	  {
	    sts = rtt_logging_store_all( file_str);
	    return sts;
	  }
	  else
	  {
	    if ( ODD( rtt_get_qualifier( "/ENTRY", entry_str)))
	    {
	      /* convert to integer */
	      nr = sscanf( entry_str, "%d", &entry);
	      if ( nr != 1)
	      {
	        rtt_message('E', "Entry syntax error");
	        return RTT__HOLDCOMMAND;
	      }
	    }
	    else
	    {
	      /* Take the current entry as default */
	      sts = rttsys_get_current_logg_entry( &entry);
	    }
	    sts = rtt_logging_store_entry( entry, file_str);
	    return sts;
	  }
	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}

	return RTT__SUCCESS;	
}


/*************************************************************************
*
* Name:		set_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "set" command i recieved.
*
**************************************************************************/

int	rttcmd_define_func( 	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	char	arg2_str[80];
	char	arg3_str[80];
	char	arg4_str[80];
	char 	*arg3_ptr;
	char 	*arg4_ptr;
	
	if ( EVEN( rtt_get_qualifier( "rtt_arg1", arg1_str)))
	{
	  rtt_message('E',"Syntax error");
	  return RTT__HOLDCOMMAND;
	}
	if ( EVEN( rtt_get_qualifier( "rtt_arg2", arg2_str)))
	{
	  rtt_message('E',"Syntax error");
	  return RTT__HOLDCOMMAND;
	}
	if ( EVEN( rtt_get_qualifier( "rtt_arg3", arg3_str)))
	  arg3_ptr = 0;
	else
	  arg3_ptr = arg3_str;
	if ( EVEN( rtt_get_qualifier( "rtt_arg4", arg4_str)))
	  arg4_ptr = 0;
	else
	  arg4_ptr = arg4_str;

	sts = rtt_define_symbol( arg1_str, arg2_str, arg3_ptr, arg4_ptr);
	return sts;
}

/*************************************************************************
*
* Name:		get_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "get" command i recieved.
*
**************************************************************************/

static int	rtt_get_func(	menu_ctx	ctx,
				int		*flag)
{
	char	arg1_str[80];
	int	arg1_sts;

	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "CLOCK", strlen( arg1_str)) == 0)
	{
	  /* Command is "GET CLOCK" */
#ifdef OS_VMS
	  rtt_message('E' ,"Not implemented in VMS");
	  return RTT__NOPICTURE;
#endif
#ifdef OS_ELN
	  int sts;

	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }
	  sts = rtt_confirm( "Do you really want to set system time from clock");
	  if ( EVEN(sts)) return RTT__NOPICTURE;

	  sts = rtt_rtc( RTT_RTC_GET);
	  return sts;
#endif
	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		directory_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "directory" command i recieved.
*
**************************************************************************/

static int	directory_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	char	filespec[80];
	char	title[80] = "FILE LIST";

	if ( ODD( rtt_get_qualifier( "rtt_arg1", arg1_str)))
	  rtt_get_defaultfilename( arg1_str, filespec, "");
	else
	  rtt_get_defaultfilename( "*", filespec, ".*");

	sts = rtt_show_file( ctx, filespec, NULL, title);
	if ( EVEN(sts))
	{
	  rtt_message('E', "No files found");
	   return RTT__HOLDCOMMAND;
	}
	return sts;
}

/*************************************************************************
*
* Name:		setup_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "setup" command i recieved.
*
**************************************************************************/

static int	rtt_setup_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;

	/* Command is "SETUP" */
	sts = rtt_setup( ctx);
	return sts;
}
/*************************************************************************
*
* Name:		set_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "set" command i recieved.
*
**************************************************************************/

static int	rtt_set_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	char	arg2_str[80];
	int	arg1_sts;

	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "FILE", strlen( arg1_str)) == 0)
	{
	  char str[80];

	  if ( ODD( rtt_get_qualifier( "/ON", str)))
	  {
	    if ( rtt_file_on)
	    {
	      rtt_message('E', "File is already on");
	      return RTT__NOPICTURE;
	    }
	    if ( EVEN( rtt_get_qualifier( "/NAME", str)))
	    {
	      rtt_message('E', "Enter name");
	      return RTT__HOLDCOMMAND;
	    }

	    rtt_get_defaultfilename( str, str, ".lis");
	    rtt_outfile = fopen( str, "w");
	    if ( rtt_outfile == 0)
	    {
	      rtt_message('E', "Unable to open file");
	      return RTT__HOLDCOMMAND;
	    }
	    if ( ODD( rtt_get_qualifier( "/MESSAGE", str)))
	      rtt_print_message = 1;
	    else
	      rtt_print_message = 0;
	      
	    if ( ODD( rtt_get_qualifier( "/COMMAND", str)))
	      rtt_print_command = 1;
	    else
	      rtt_print_command = 0;
	      
	    rtt_file_on = 1;
	    return RTT__NOPICTURE;
	  }
	  else if ( ODD( rtt_get_qualifier( "/OFF", str)))
	  {
	    if ( !rtt_file_on)
	    {
	      rtt_message('E', "File is not on");
	      return RTT__NOPICTURE;
	    }
	    fclose( rtt_outfile);
	    rtt_print_message = 0;
	    rtt_print_command = 0;
	    rtt_file_on = 0;
	    return RTT__NOPICTURE;
	  }
	  else
	  {
	    rtt_message('E', "Syntax error");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( strncmp( arg1_str, "VERIFY", strlen( arg1_str)) == 0)
	{
	  rtt_verify = 1;
	  rtt_message('I', "Verify set on");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "NOVERIFY", strlen( arg1_str)) == 0)
	{
	  rtt_verify = 0;
	  rtt_message('I', "Verify set off");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "PWRP_ALIAS", strlen( arg1_str)) == 0)
	{
	  char alias_node[20];
	  char alias_file[80];

#ifdef OS_ELN
          sprintf( alias_file, "%spwrp_alias.dat", "<sys0.sysexe>");
#else
          sprintf( alias_file, "%spwrp_alias.dat", rtt_pwr_dir("pwrp_load"));
#endif
          sts = ini_GetAlias( alias_file, rtt_node, alias_node);
	  if (EVEN(sts))
	    strcpy( alias_node, rtt_node);

	  rtt_display_erase();
	  rtt_cursor_abs( 0, 0);
	  r_print("Execution of %s", alias_file);
	  rtt_cursor_abs( 0, 2);
	  r_print_buffer();

	  sts = ini_SetAttribute( alias_file, alias_node, 1);
	  if (EVEN(sts))
	  {
	    rtt_message('E', "pwrp_alias error");
	    return RTT__NOPICTURE;
	  }
#ifdef OS_ELN
	  sts = ini_SetAttributeAfterPlc( alias_file, alias_node, 1);
	  if (EVEN(sts))
	  {
	    rtt_message('E', "pwrp_alias error");
	    return RTT__NOPICTURE;
	  }
#endif
	  rtt_wait_for_return();
	  return RTT__SUCCESS;
	}
	else if ( strncmp( arg1_str, "PRIORITY", strlen( arg1_str)) == 0)
	{
	  /* Command is "SET PRIORITY" */
	  /* Set job priority for the rtt job */
	  char	priority_str[80];
	  int	nr;
	  int	priority;
	  int	sts;

	  if ( ODD( rtt_get_qualifier( "/PRIORITY", priority_str)))
	  {

	    /* convert to integer */
	    nr = sscanf( priority_str, "%d", &priority);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	    /* Set job priority */
	    sts = 0;
#ifdef OS_ELN
	    ker$set_job_priority( &sts, priority);
#endif
#ifdef OS_VMS
	    sts = sys$setpri(NULL, NULL, priority, NULL, NULL, NULL);
#endif
	    if ( EVEN(sts))
	    {
	      rtt_message('E',"Unable to set priority");	
	      return RTT__HOLDCOMMAND;
	    }
	    else
	    {
	      rtt_message('I',"Priority set");	
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    rtt_message('E',"Enter priority");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( strncmp( arg1_str, "TIME", strlen( arg1_str)) == 0)
	{
	  char text[80];
	  pwr_tTime time;
	  char			timstr[64];

	  arg1_sts = rtt_get_qualifier( "rtt_arg2", arg2_str);
	  if ( EVEN(arg1_sts))
	  {
	    rtt_message('E', "Syntax error");
	    return RTT__HOLDCOMMAND;
	  }
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }
	  sts = rtt_qual_to_time( arg2_str, &time);
	  if ( EVEN(sts))
	  {
	    rtt_message('E', "Syntax error");
	    return RTT__HOLDCOMMAND;
	  }
	  time_AtoAscii( &time, time_eFormat_DateAndTime,
			timstr, sizeof(timstr));
	  timstr[strlen(timstr) - 3] = '\0';
	  sprintf( text, "Do you really want to set time to %s", timstr);
	  sts = rtt_confirm( text);
	  if ( EVEN(sts)) return RTT__NOPICTURE;

	  sts = 0;
#if defined (OS_ELN) || defined (OS_VMS)
	  sts = time_SetTime( &time);
#endif
	  if ( EVEN(sts))
	  {
	    rtt_message('E', "Unable to set time");
	    return RTT__NOPICTURE;
	  }
	  /* Show new time */
	  rtt_update_time();
	  sprintf( text, "Time is %s", rtt_time);
	  rtt_message('I', text);
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "CLOCK", strlen( arg1_str)) == 0)
	{
	  /* Command is "SET CLOCK" */
#ifdef OS_VMS
	  rtt_message('E' ,"Not implemented in VMS");
	  return RTT__NOPICTURE;
#endif
#ifdef OS_ELN
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }
	  sts = rtt_confirm( "Do you really want to set clock from system time");
	  if ( EVEN(sts)) return RTT__NOPICTURE;

	  sts = rtt_rtc( RTT_RTC_SET);
	  return sts;
#endif
	}
	else if ( strncmp( arg1_str, "DEFAULT", strlen( arg1_str)) == 0)
	{
	  if ( ODD( rtt_get_qualifier( "rtt_arg2", arg2_str)))
	  {
	    if ( strcmp( arg2_str, "") != 0)
	    {
#if defined(OS_VMS) || defined(OS_ELN)
	      if ( !( arg2_str[strlen(arg2_str)-1] == ':' ||
	              arg2_str[strlen(arg2_str)-1] == ']' ||
	              arg2_str[strlen(arg2_str)-1] == '>' ))
	        strcat( arg2_str, ":");
#elif defined(OS_LYNX) || defined(OS_LINUX)
	      cdh_ToLower( arg2_str, arg2_str);
#endif
	    }
	    strcpy( rtt_default_directory, arg2_str);
	    return RTT__HOLDCOMMAND;
	  }
	  else
	  {
	    rtt_message('E', "Syntax error");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( strncmp( arg1_str, "PARAMETER", strlen( arg1_str)) == 0)
	{
	  /* Command is "SET PARAMETER" */
	  char	name_str[80];
	  char	value_str[80];
	  int	sts;
	  int	bypass;

	  IF_NOGDH_RETURN;

	  bypass = ODD( rtt_get_qualifier( "/BYPASS", name_str));

	  if ( EVEN( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    rtt_message('E', "Enter name of parameter");
	    return RTT__HOLDCOMMAND;
	  }
	  if ( EVEN( rtt_get_qualifier( "/VALUE", value_str)))
	  {
	    rtt_message('E', "Enter value");
	    return RTT__HOLDCOMMAND;
	  }
	  sts = rtt_set_parameter( name_str, value_str, bypass);
	  if ( EVEN(sts))
	  {
	    rtt_message('E',"Unable to set parameter");	
	    return RTT__HOLDCOMMAND;
	  }
	  else
	    return sts;
	}
	else if ( strncmp( arg1_str, "RTDB_OFFSET", strlen( arg1_str)) == 0)
	{
	  /* Command is "SET RTDB_OFFSET" */
	  /* Set rtdb_offset for the rtt job */
	  char	offset_str[80];
	  int	nr;
	  unsigned long	offset;

	  IF_NOGDH_RETURN;
	  if ( ODD( rtt_get_qualifier( "/RTDB_OFFSET", offset_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( offset_str, "%ld", &offset);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	    rtt_rtdb_offset = offset;
	    return RTT__NOPICTURE;
	  }
	  else
	  {
	    rtt_message('E', "Syntax is: pwr_rtt> set rtdb_offset/offset=...");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( strncmp( arg1_str, "MODE", strlen( arg1_str)) == 0)
	{
	  arg1_sts = rtt_get_qualifier( "rtt_arg2", arg2_str);

	  /* Command is "SET MODE" */
	  if ( strncmp( arg2_str, "ADDRESS", strlen( arg2_str)) == 0)
	  {
	    /* Check authorization */
	    if ( !(rtt_priv & RTT_PRIV_SYS) )
	    {
	      rtt_message('E',"Not authorized for this operation");
	      return RTT__NOPICTURE;
	    }
	    rtt_mode_address = 1;
	    return RTT__NOPICTURE;
	  }
	  else if ( strncmp( arg2_str, "NOADDRESS", strlen( arg2_str)) == 0)
	  {
	    rtt_mode_address = 0;
	    return RTT__NOPICTURE;
	  }
	  else if ( strcmp( arg2_str, "ACCVIO") == 0)
	  {
	    char *s = 0;

	    /* Test of exception handler... */
	    strcpy( s, "The end is close");
	  }
	  else if ( strncmp( arg2_str, "DUMP", strlen( arg2_str)) == 0)
	  {
	    /* Check authorization */
	    if ( !(rtt_priv & RTT_PRIV_SYS) )
	    {
	      rtt_message('E',"Not authorized for this operation");
	      return RTT__NOPICTURE;
	    }
	    rtt_mode_dump = 1;
	    return RTT__NOPICTURE;
	  }
	  else if ( strncmp( arg2_str, "NODUMP", strlen( arg2_str)) == 0)
	  {
	    rtt_mode_dump = 0;
	    return RTT__NOPICTURE;
	  }
	  else
	  {
	    rtt_message('E', "Unknown qualifier");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( strncmp( arg1_str, "ALARMMESSAGE", strlen( arg1_str)) == 0)
	{
	  rtt_AlarmMessage = 1;
	  rtt_message('I', "Alarm message set on");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "NOALARMMESSAGE", strlen( arg1_str)) == 0)
	{
	  rtt_AlarmMessage = 0;
	  rtt_message('I', "Alarm message set off");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "ALARMBEEP", strlen( arg1_str)) == 0)
	{
	  rtt_AlarmBeep = 1;
	  rtt_message('I', "Alarm beep set on");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "NOALARMBEEP", strlen( arg1_str)) == 0)
	{
	  rtt_AlarmBeep = 0;
	  rtt_message('I', "Alarm beep set off");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "CONVERSION", strlen( arg1_str)) == 0)
	{
	  int	on;
	  pwr_tObjid	objid;
	  char		name_str[80];

	  IF_NOGDH_RETURN;

	  /* Check authorization */
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }
	  if ( ODD( rtt_get_qualifier( "/ON", name_str)))
	    on = 1;
	  else if ( ODD( rtt_get_qualifier( "/OFF", name_str)))
	    on = 0;
	  else
	  {
	    rtt_message('E', "Enter on/off");
	    return RTT__HOLDCOMMAND;
	  }

	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_set_conversion( objid, on, 0);
	  return sts;
	}
	else if ( strncmp( arg1_str, "INVERT", strlen( arg1_str)) == 0)
	{
	  int	on;
	  pwr_tObjid	objid;
	  char		name_str[80];

	  IF_NOGDH_RETURN;

	  /* Check authorization */
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }
	  if ( ODD( rtt_get_qualifier( "/ON", name_str)))
	    on = 1;
	  else if ( ODD( rtt_get_qualifier( "/OFF", name_str)))
	    on = 0;
	  else
	  {
	    rtt_message('E', "Enter on/off");
	    return RTT__HOLDCOMMAND;
	  }

	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_set_invert( objid, on, 0);
	  return sts;
	}
	else if ( strncmp( arg1_str, "DOTEST", strlen( arg1_str)) == 0)
	{
	  int	on;
	  pwr_tObjid	objid;
	  char		name_str[80];

	  IF_NOGDH_RETURN;

	  /* Check authorization */
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }
	  if ( ODD( rtt_get_qualifier( "/ON", name_str)))
	    on = 1;
	  else if ( ODD( rtt_get_qualifier( "/OFF", name_str)))
	    on = 0;
	  else
	  {
	    rtt_message('E', "Enter on/off");
	    return RTT__HOLDCOMMAND;
	  }

	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_set_do_test( objid, on, 0);
	  return sts;
	}
	else if ( strncmp( arg1_str, "TESTVALUE", strlen( arg1_str)) == 0)
	{
	  int	on;
	  pwr_tObjid	objid;
	  char		name_str[80];

	  IF_NOGDH_RETURN;

	  /* Check authorization */
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }
	  if ( ODD( rtt_get_qualifier( "/ON", name_str)))
	    on = 1;
	  else if ( ODD( rtt_get_qualifier( "/OFF", name_str)))
	    on = 0;
	  else
	  {
	    rtt_message('E', "Enter on/off");
	    return RTT__HOLDCOMMAND;
	  }

	  if ( ODD( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    sts = gdh_NameToObjid ( name_str, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &objid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Enter name or select an object");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  sts = rtt_set_do_testvalue( objid, on, 0);
	  return sts;
	}
	else if ( strncmp( arg1_str, "DESCRIPTION", strlen( arg1_str)) == 0)
	{
	  rtt_description_on = 1;
	  rtt_message( 'I', "Description set on");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "NODESCRIPTION", strlen( arg1_str)) == 0)
	{
	  rtt_description_on = 0;
	  rtt_message( 'I', "Description set off");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "SCANTIME", strlen( arg1_str)) == 0)
	{
	  /* Command is "SET SCANTIME" */
	  /* Set rtdb_offset for the rtt job */
	  char	time_str[80];
	  int	nr;
	  float	f_time;

	  IF_NOGDH_RETURN;
	  if ( ODD( rtt_get_qualifier( "/TIME", time_str)))
	  {
	    /* convert to integer */
	    nr = sscanf( time_str, "%f", &f_time);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	    rtt_scantime = 1000 * f_time;
	    return RTT__NOPICTURE;
	  }
	  else
	  {
	    rtt_message('E', "Syntax is: pwr_rtt> set scantime/time=...");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( strncmp( arg1_str, "MESSAGE", strlen( arg1_str)) == 0)
	{
	  rtt_message_off = 0;
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "NOMESSAGE", strlen( arg1_str)) == 0)
	{
	  rtt_message_off = 1;
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "NOREDRAW", strlen( arg1_str)) == 0)
	{
	  rtt_noredraw = 1;
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "DRAW", strlen( arg1_str)) == 0)
	{
	  rtt_quiet = 0;
	  rtt_noredraw = 1;
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "NODRAW", strlen( arg1_str)) == 0)
	{
	  rtt_quiet = RTT_QUIET_ALL;
	  rtt_noredraw = 0;
	  return RTT__NOPICTURE;
	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		plcscan_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "plcscan" command i recieved.
*
**************************************************************************/

static int	plcscan_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	int	on, all;
	char	hierarchy_str[80];
	char	str[80];
	char	*hierarchy_ptr;
	int	global;

	IF_NOGDH_RETURN;

	/* Check authorization */
	if ( !(rtt_priv & RTT_PRIV_SYS) )
	{
	      rtt_message('E',"Not authorized for this operation");
	      return RTT__NOPICTURE;
	}

	on = 0;
	if ( ODD( rtt_get_qualifier( "/ON", str)))
	{
	  on = 1;
	}
	else
	{
	  if ( EVEN( rtt_get_qualifier( "/OFF", str)))
	  {
	    rtt_message('E',"Syntax error");
	    return RTT__HOLDCOMMAND;
	  }
	}

	if ( ODD( rtt_get_qualifier( "/ALL", str)))
	  all = 1;
	else
	  all = 0;
	if ( ODD( rtt_get_qualifier( "/GLOBAL", str)))
	  global = 1;
	else
	  global = 0;

	if ( ODD( rtt_get_qualifier( "/HIERARCHY", hierarchy_str)))
	  hierarchy_ptr = hierarchy_str;
	else
	  hierarchy_ptr = NULL;

	sts = rtt_plcscan( ctx, on, all, hierarchy_ptr, global);
	if ( EVEN(sts)) return sts;

	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rttcmd_learn_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "learn" command i recieved.
*
**************************************************************************/

int	rttcmd_learn_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;
	char	msg[120];
	
	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "START", strlen( arg1_str)) == 0)
	{
	  /* Command is "LEARN START" */

	  char	file_str[80];

	  if ( EVEN( rtt_get_qualifier( "/FILE", file_str)))
	    strcpy( file_str, "rtt_learn_file");

	  rtt_get_defaultfilename( file_str, file_str, ".rtt_com");
	  sts = rtt_learn_start( file_str);
	  if ( EVEN(sts)) 
	  {
	    sprintf( msg, "Unable to open file %s", file_str);
	    rtt_message('E', msg);
	    return RTT__HOLDCOMMAND;
	  }
	  rtt_message('I', "Enter key sequence to learn");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "STOP", strlen( arg1_str)) == 0)
	{
	  /* Command is "LEARN STOP" */

	  sts = rtt_learn_stop();
	  if ( EVEN(sts)) 
	  {
	    rtt_message('E', "Start learn before stop");
	    return RTT__HOLDCOMMAND;
	  }
	  rtt_message('I', "Key sequence stored");
	  return RTT__NOPICTURE;
	}
	else if ( strncmp( arg1_str, "RECALL", strlen( arg1_str)) == 0)
	{
	  /* Command is "LEARN RECALL" */

	  char	file_str[80];
	  char 	command[80];

	  if ( EVEN( rtt_get_qualifier( "/FILE", file_str)))
	    strcpy( file_str, "rtt_learn_file");

	  rtt_get_defaultfilename( file_str, file_str, ".rtt_com");

	  strcpy( command, "@");
	  strcat( command, file_str);
	  sts = rtt_menu_command( ctx, pwr_cNObjid, command, 0, 0, 0);
	  return sts;
	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}

}


/*************************************************************************
*
* Name:		store_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "store" command i receieved.
*
**************************************************************************/

static int	store_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;

	char	*file_ptr;
	char	file_str[80];
	char	str[80];
	int	collect;

	  IF_NOGDH_RETURN;

	  if ( ODD( rtt_get_qualifier( "/SYMBOLS", str)))
	  {
	    if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	      file_ptr = file_str;
	    else if ( ODD( rtt_get_qualifier( "rtt_arg1", file_str)))
	    {
	      if ( file_str[0] != '/')
	        /* Assume that this is the filestring */
	        file_ptr = file_str;
	      else
	        file_ptr = rtt_symbolfilename;
	    }
	    else
	      file_ptr = rtt_symbolfilename;

	    sts = rtt_store_symbols( file_ptr);
	    return sts;
	  }
	  else
	  {

	    if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	      file_ptr = file_str;
	    else if ( ODD( rtt_get_qualifier( "rtt_arg1", file_str)))
	    {
	      if ( file_str[0] != '/')
	        /* Assume that this is the filestring */
	        file_ptr = file_str;
	      else
	      {
	        rtt_message('E',"Enter file");	
	        return RTT__HOLDCOMMAND; 	
	      } 
	    }
	    else
	    {
	      rtt_message('E',"Enter file");	
	      return RTT__HOLDCOMMAND; 	
	    } 

	    /* Store a menue */
	    if ( ODD( rtt_get_qualifier( "/COLLECT", str)))
	      collect = 1;
	    else
	      collect = 0;

	    sts = rtt_store( ctx, file_str, collect);
	    return sts;
	  }
}

/*************************************************************************
*
* Name:		alarm_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "alarm" command i recieved.
*
**************************************************************************/

static int	alarm_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;

	IF_NOGDH_RETURN;
	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "SEND", strlen( arg1_str)) == 0)
	{
	  /* Command is "ALARM SEND" */

	  char	text_str[80];
	  char	prio_str[80];
	  int	priority;


	  if ( EVEN( rtt_get_qualifier( "/TEXT", text_str)))
	  {
	    rtt_message('E',"Enter text");	
	    return RTT__HOLDCOMMAND;
	  }

	  if ( ODD( rtt_get_qualifier( "/PRIORITY", prio_str)))
	  {
	    if ( strcmp( prio_str, "A") == 0)
	      priority = 'A';
	    else if ( strcmp( prio_str, "B") == 0)
	      priority = 'B';
	    else if ( strcmp( prio_str, "C") == 0)
	      priority = 'C';
	    else if ( strcmp( prio_str, "D") == 0)
	      priority = 'D';
	    else if ( strcmp( prio_str, "I") == 0)
	      priority = 'I';
	    else
	    {
	      rtt_message('E',"Unknown priority");	
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    priority = 'A';

	  sts = rtt_alarm_send( text_str, priority);
	  if ( EVEN(sts)) 
	  {
	    rtt_message('E', "Unable to send alarm");
	    return RTT__NOPICTURE;
	  }
	  return RTT__NOPICTURE;
	}

	else if ( strncmp( arg1_str, "PRINT", strlen( arg1_str)) == 0)
	{
	  /* Command is "ALARM PRINT" */

	  char	file_str[80];
	  char	dum_str[80];
	  int	notext;
	  int	noname;

	  if ( EVEN( rtt_get_qualifier( "/FILE", file_str)))
	  {
	      rtt_message('E',"Enter file");	
	      return RTT__HOLDCOMMAND;
	  }
	  if ( ODD( rtt_get_qualifier( "/NOTEXT", dum_str)))
	    notext = 1;
	  else
	    notext = 0;

	  if ( ODD( rtt_get_qualifier( "/NONAME", dum_str)))
	    noname = 1;
	  else
	    noname = 0;

	  rtt_get_defaultfilename( file_str, file_str, ".lis");
	  sts = rtt_event_print( file_str, notext, noname);
	  return sts;
	}

	else if ( strncmp( arg1_str, "LOG", strlen( arg1_str)) == 0)
	{
	  /* Command is "ALARM LOG" */

	  char	file_str[80];
	  char	dum_str[80];

	  if ( ODD( rtt_get_qualifier( "/START", dum_str)))
	  {
	    if ( EVEN( rtt_get_qualifier( "/FILE", file_str)))
	    {
	      rtt_message('E',"Enter file");	
	      return RTT__HOLDCOMMAND;
	    }
	    rtt_get_defaultfilename( file_str, file_str, ".log");
	    sts = rtt_alarmlog_start( file_str);
	    return sts;
	  }
	  else if ( ODD( rtt_get_qualifier( "/STOP", dum_str)))
	  {
	    sts = rtt_alarmlog_stop();
	    return sts;
	  }
	  else
	  {
	    rtt_message('E',"Syntax error");	
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( strncmp( arg1_str, "LIST", strlen( arg1_str)) == 0)
	{
	  /* Command is "ALARM LIST" */

	  char	user_str[80];
	  pwr_tObjid	user_objid;
	  char	maxalarm_str[80];
	  int	maxalarm;
	  char	maxevent_str[80];
	  int	maxevent;
	  char	dum_str[80];
	  int	nr;
	  int	acknowledge;
	  int	returned;
	  int	beep;


	  if ( ODD( rtt_get_qualifier( "/MAXALARM", maxalarm_str)))
	  {
	    nr = sscanf( maxalarm_str, "%d", &maxalarm);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    maxalarm = 0;

	  if ( ODD( rtt_get_qualifier( "/MAXEVENT", maxevent_str)))
	  {
	    nr = sscanf( maxevent_str, "%d", &maxevent);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    maxevent = 0;

	  if ( ODD( rtt_get_qualifier( "/ACKNOWLEDGE", dum_str)) || rtt_AlarmAck)
	    acknowledge = 1;
	  else
	    acknowledge = 0;

	  if ( ODD( rtt_get_qualifier( "/RETURN", dum_str)) || rtt_AlarmReturn)
	    returned = 1;
	  else
	    returned = 0;

	  if ( ODD( rtt_get_qualifier( "/BEEP", dum_str)) || rtt_AlarmBeep)
	    beep = 1;
	  else
	    beep = 0;

	  if ( ODD( rtt_get_qualifier( "/USER", user_str)))
	  {
	    sts = gdh_NameToObjid ( user_str, &user_objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E',"Userobject not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	    user_objid = pwr_cNObjid;

	  sts = rtt_alarm_connect( user_objid, maxalarm, maxevent, 
			acknowledge, returned, beep);
	  if ( EVEN(sts))
	  {
	    rtt_message('E',"Unable to connect to user");	
	    return RTT__HOLDCOMMAND;
	  }

	  sts = rtt_menu_alarm_new( ctx, rtt_event_ctx);
	  if ( EVEN(sts))
	  {
	    rtt_message('E', "Unable to show alarm");
	    return RTT__NOPICTURE;
	  }
	}
	else if ( strncmp( arg1_str, "SHOW", strlen( arg1_str)) == 0)
	{
	  /* Command is "ALARM SHOW" */

	  char	user_str[80];
	  pwr_tObjid	user_objid;
	  char	maxalarm_str[80];
	  int	maxalarm;
	  char	maxevent_str[80];
	  int	maxevent;
	  char	dum_str[80];
	  int	nr;
	  int	returned;
	  int	acknowledge;
	  int	beep;

	  if ( ODD( rtt_get_qualifier( "/MAXALARM", maxalarm_str)))
	  {
	    nr = sscanf( maxalarm_str, "%d", &maxalarm);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    maxalarm = 0;

	  if ( ODD( rtt_get_qualifier( "/MAXEVENT", maxevent_str)))
	  {
	    nr = sscanf( maxevent_str, "%d", &maxevent);
	    if ( nr != 1)
	    {
	      rtt_message('E', "Syntax error");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    maxevent = 0;

	  if ( ODD( rtt_get_qualifier( "/ACKNOWLEDGE", dum_str)) || rtt_AlarmAck)
	    acknowledge = 1;
	  else
	    acknowledge = 0;

	  if ( ODD( rtt_get_qualifier( "/RETURN", dum_str)) || rtt_AlarmReturn)
	    returned = 1;
	  else
	    returned = 0;

	  if ( ODD( rtt_get_qualifier( "/BEEP", dum_str)) || rtt_AlarmBeep)
	    beep = 1;
	  else
	    beep = 0;

	  if ( ODD( rtt_get_qualifier( "/USER", user_str)))
	  {
	    sts = gdh_NameToObjid ( user_str, &user_objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E',"Userobject not found");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	    user_objid = pwr_cNObjid;

	  sts = rtt_alarm_connect( user_objid, maxalarm, maxevent, 
			acknowledge, returned, beep);
	  if ( EVEN(sts))
	  {
	    rtt_message('E',"Unable to connect to user");
	    return RTT__HOLDCOMMAND;
	  }
	  sts = rtt_menu_alarm_new( ctx, rtt_alarm_ctx);
	  if ( EVEN(sts))
	  {
	    rtt_message('E', "Unable to show alarm");
	    return RTT__NOPICTURE;
	  }
	}
	else if ( strncmp( arg1_str, "ACKNOWLEDGE", strlen( arg1_str)) == 0)
	{
	  rtt_alarm_ack( rtt_alarm_ctx);
	  return RTT__NOPICTURE;
	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_create_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "create" command i recieved.
*
**************************************************************************/

static int	rtt_create_func(	menu_ctx	ctx,
					int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;

	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0)
	{
	  /* Command is "CREATE OBJECT" */

	  char	name_str[80];
	  char	class_str[80];

	  IF_NOGDH_RETURN;

	  /* Check authorization */
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }

	  if ( EVEN( rtt_get_qualifier( "/CLASS", class_str)))
	  {
	    rtt_message('E',"Enter class");	
	    return RTT__HOLDCOMMAND;
	  }

	  if ( EVEN( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    rtt_message('E',"Enter name");	
	    return RTT__HOLDCOMMAND;
	  }


	  sts = rtt_create_object( ctx, class_str, name_str);
	  return sts;
	}
	else if ( strncmp( arg1_str, "MENU", strlen( arg1_str)) == 0)
	{
	  /* Command is "CREATE MENU" */
	  int		sts;
	  char		title_str[80];
	  char		text_str[80];
	  char		object_str[80];
	  char		command_str[80];
	  rtt_t_menu	*menulist = 0;
	  rtt_t_menu	*menu_ptr;
	  int		i;
	  int		command;
	  int		object;
	  pwr_tObjid	objid;

	  if ( EVEN( rtt_get_qualifier( "/TEXT", text_str)))
	  {
	    rtt_message('E',"Enter text");	
	    return RTT__HOLDCOMMAND;
	  }
	  if ( EVEN( rtt_get_qualifier( "/TITLE", title_str)))
	  {
	    rtt_message('E',"Enter title");	
	    return RTT__HOLDCOMMAND;
	  }
	  if ( ODD( rtt_get_qualifier( "/COMMAND", command_str)))
	    command = 1;
	  else
	    command = 0;
	  if ( ODD( rtt_get_qualifier( "/OBJECT", object_str)))
	  {
	    IF_NOGDH_RETURN;
	    object = 1;
	    sts = gdh_NameToObjid ( object_str, &objid);
	    if (EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	    object = 0;

	  menulist = 0;
	  i = 0;
	  sts = rtt_menu_list_add( &menulist, i, 0, text_str,
		NULL, NULL, NULL,
		pwr_cNObjid,0,0,0,0);
	  if ( EVEN(sts)) return sts;

	  if ( command)
	  {
	    menu_ptr = menulist + i;
	    menu_ptr->func = &rtt_menu_command;
	    menu_ptr->arg1 = command_str;
	  }
	  else if ( object)
	  {
	    menu_ptr = menulist + i;	    
	    menu_ptr->func = &rtt_hierarchy_child;
	    menu_ptr->func2 = &rtt_object_parameters;
	    menu_ptr->func3 = &rtt_debug_child;
	    menu_ptr->argoi = objid;
	  }

	  sts = rtt_menu_new( ctx, pwr_cNObjid, &menulist, title_str, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if (EVEN(sts)) return sts;

	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_delete_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "delete" command i recieved.
*
**************************************************************************/

static int	rtt_delete_func(	menu_ctx	ctx,
					int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	arg1_sts;

	IF_NOGDH_RETURN;
	arg1_sts = rtt_get_qualifier( "rtt_arg1", arg1_str);

	if ( strncmp( arg1_str, "OBJECT", strlen( arg1_str)) == 0)
	{
	  /* Command is "DELETE OBJECT" */

	  char	name_str[80];

	  /* Check authorization */
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__NOPICTURE;
	  }

	  if ( EVEN( rtt_get_qualifier( "/NAME", name_str)))
	  {
	    rtt_message('E',"Enter name");	
	    return RTT__HOLDCOMMAND;
	  }


	  sts = rtt_delete_object( ctx, name_str);
	  return sts;
	}
	else
	{
	  rtt_message('E', "Unknown qualifier");
	  return RTT__HOLDCOMMAND;
	}
}

/*************************************************************************
*
* Name:		rtt_delete_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "delete" command i recieved.
*
**************************************************************************/

static int	rtt_view_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	*file_ptr;
	char	file_str[80];

	  if ( ODD( rtt_get_qualifier( "/FILE", file_str)))
	    file_ptr = file_str;
	  else if ( ODD( rtt_get_qualifier( "rtt_arg1", file_str)))
	  {
	    if ( file_str[0] != '/')
	      /* Assume that this is the filestring */
	      file_ptr = file_str;
	    else
	    {
	      rtt_message('E',"Enter file");	
	      return RTT__HOLDCOMMAND; 	
	    } 
	  }
	  else
	  {
	    rtt_message('E',"Enter file");	
	    return RTT__HOLDCOMMAND; 	
	  } 

	  sts = rtt_view( ctx, file_ptr, 0, 0, RTT_VIEWTYPE_FILE);
	  return sts;
}

/*************************************************************************
*
* Name:		wait_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "wait" command i recieved.
*
**************************************************************************/

static int	wait_func(	menu_ctx	ctx,
				int		*flag)
{
	int		nr;
	char		time_str[80];
	char		dum_str[80];
	int		time;

	if ( ODD( rtt_get_qualifier( "/PLCPGM", dum_str)))
	{
#ifdef OS_ELN
	  /* Wait for the plcpgm has flagged initizated */
	  plc_UtlWaitForPlc();
#elif OS_VMS
	  rtt_message('E', "Not implemented for VMS");
#endif
	  return RTT__NOPICTURE;
	}

	else if ( ODD( rtt_get_qualifier( "/TIME", time_str)))
	{

	  /* convert to integer */
	  nr = sscanf( time_str, "%d", &time);
	  if ( nr != 1)
	  {
	    rtt_message('E', "Time syntax error");
	    return RTT__HOLDCOMMAND;
	  }

	  rtt_sleep( ctx, time);
	  return RTT__NOPICTURE;
	}
	else
	{
	  rtt_message('E',"Enter time");
	  return RTT__HOLDCOMMAND;
	}

}

/*************************************************************************
*
* Name:		search_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Searches for a string in the menu text.
*	
**************************************************************************/

static int	search_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	int	index;
	int	startindex;
	int	page;

	if ( EVEN( rtt_get_qualifier( "rtt_arg1", arg1_str)))
	{
	  /* Use the contents in the search buffer */
	  if ( rtt_searchbuffer[0] == 0)
	  {
	    rtt_message('E', "Enter search string");
	    return RTT__HOLDCOMMAND;
	  }
	  startindex = ctx->current_item + 1;
	}
	else
	{
	  /* the search string is in arg1 */
	  strcpy( rtt_searchbuffer, arg1_str);
	  startindex = ctx->current_item + 1;
	}

	if ( ctx->ctx_type == RTT_CTXTYPE_VIEW)
	{
	  sts = rtt_view_search( (view_ctx) ctx, rtt_searchbuffer);
	  return sts;
	}
	else
	{
	  if ( rtt_menulist_search( ctx, rtt_searchbuffer, 
			startindex, &index))
	  {
	    rtt_item_to_page( ctx, index, &page);
	    if ( page == ctx->current_page)
	    {
	      /* Select the found item */
              if ( ctx->menutype & RTT_MENUTYPE_EDIT)
              {
                rtt_edit_unselect( ctx);
                ctx->current_item = index;
                rtt_edit_select( ctx);
              }
              else
              {
                rtt_menu_unselect( ctx);
                ctx->current_item = index;
                rtt_menu_select( ctx);
              }
	      r_print_buffer();
	      return RTT__NOPICTURE;
	    }
	    else
	    {
	      /* Redraw the menu with new item and page */
	      ctx->current_page = page;
	      ctx->current_item = index;
	      return RTT__SUCCESS;	
	    }
	  }
	  else
	  {
	    rtt_message('E', "String not found");
	    return RTT__HOLDCOMMAND;
	  }
	}

	return RTT__SUCCESS;	
}
/*************************************************************************
*
* Name:		top_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Position at top of menu.
*	
**************************************************************************/

static int	top_func(	menu_ctx	ctx,
				int		*flag)
{
	if ( ctx->current_page == 0)
	{
	  /* Select the found item */
	  if ( ctx->menutype & RTT_MENUTYPE_EDIT)
	  {
	    rtt_edit_unselect( ctx);
	    ctx->current_item = 0;
	    rtt_edit_select( ctx);
	  }
	  else
	  {
	    rtt_menu_unselect( ctx);
	    ctx->current_item = 0;
	    rtt_menu_select( ctx);
	  }
	  r_print_buffer();
	  return RTT__NOPICTURE;
	}
	else
	{
	  /* Redraw the menu with new item and page */
	  ctx->current_page = 0;
	  ctx->current_item = 0;
	  return RTT__SUCCESS;	
	}
	return RTT__SUCCESS;	
}
/*************************************************************************
*
* Name:		page_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Set page.
*	
**************************************************************************/

static int	page_func(	menu_ctx	ctx,
				int		*flag)
{
	int	nr;
	char	arg1_str[80];
	int	page;

	if ( EVEN( rtt_get_qualifier( "rtt_arg1", arg1_str)))
	{
	  rtt_message('E', "Page number is missing");
	  return RTT__HOLDCOMMAND;
	}
	else
	{
	  /* the page nr is in arg1, convert to integer */
	  nr = sscanf( arg1_str, "%d", &page);
	  if ( nr != 1)
	  {
	    rtt_message('E', "Page number syntax error");
	    return RTT__HOLDCOMMAND;
	  }
	  page--;
	  if ( page == ctx->current_page)
	    /* do nothing */
	    return RTT__NOPICTURE;
	  if ( page <= 0)
	    page = 0;
	  if ( page > (ctx->no_pages - 1))
	    page = ctx->no_pages - 1;
	  if ( page == ctx->current_page)
	    /* do nothing */
	    return RTT__NOPICTURE;
	  ctx->current_page = page;
	  ctx->current_item = page * ctx->page_len;
	}
	return RTT__SUCCESS;	
}

/*************************************************************************
*
* Name:		help_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	This function is called when a "help" command i recieved.
*
**************************************************************************/

static int	help_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;
	char	arg1_str[80];
	char	arg2_str[80];
	char	arg3_str[80];
	char	arg4_str[80];
	char 	help_str[100] = "";

	while ( 1)
	{
	  if ( EVEN( rtt_get_qualifier( "rtt_arg1", arg1_str)))
	    break;
	  strcpy( help_str, arg1_str);
	  if ( EVEN( rtt_get_qualifier( "rtt_arg2", arg2_str)))
	    break;
	  strncat( help_str, " ", 
			sizeof(help_str) - strlen(help_str) - 1 );
	  strncat( help_str, arg2_str, 
			sizeof(help_str) - strlen(help_str) - 1 );
	  if ( EVEN( rtt_get_qualifier( "rtt_arg3", arg3_str)))
	    break;
	  strncat( help_str, " ", 
			sizeof(help_str) - strlen(help_str) - 1 );
	  strncat( help_str, arg3_str, 
			sizeof(help_str) - strlen(help_str) - 1 );
	  if ( EVEN( rtt_get_qualifier( "rtt_arg4", arg4_str)))
	    break;
	  strncat( help_str, " ", 
			sizeof(help_str) - strlen(help_str) - 1 );
	  strncat( help_str, arg4_str, 
			sizeof(help_str) - strlen(help_str) - 1 );
	}
	
	if ( help_str[0] == '\0')
	  sts = rtt_help( ctx, "HELP", (rtt_t_helptext *) rtt_command_helptext);
	else
	{
	  sts = rtt_help( ctx, help_str, (rtt_t_helptext *) rtt_command_helptext);
	  if ( EVEN(sts)) 
	  {
	    rtt_message('E', "No help on this subject");
	    return RTT__HOLDCOMMAND;
	  }
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		exit_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "exit" command i recieved.
*
**************************************************************************/

static int	exit_func(	menu_ctx	ctx,
				int		*flag)
{
	rtt_exit_now(0, RTT__SUCCESS);
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		classhier_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	This function is called when a "classhier" command i recieved.
*
**************************************************************************/

static int	classhier_func(	menu_ctx	ctx,
				int		*flag)
{
	int	sts;

	IF_NOGDH_RETURN;
	sts = rtt_class_hierarchy( ctx, pwr_cNObjid, 0, 0, 0, 0);
	return sts;
}


/*************************************************************************
*
* Description:
*	Command table.
*	The commands in pwr_rtt is defined here.
*	First parameter is the command verb.
*	Second is the function to be called.
*	Third and on are the qualifiers.
*	If the first arguments are named rtt_arg1, rtt_arg2 etc
*	parameters are allowed and can be fetched by the user.
*
**************************************************************************/


extern	rtt_t_comtbl	rtt_command_table[];
rtt_t_comtbl	rtt_command_table[] = {
		{
			"SHOW",
			&show_func,
			{ "rtt_arg1", "rtt_arg2", "/NAME", "/CLASS", 
				"/HIERARCHY", "/PARAMETER" , "/OBJID", 
				"/FILE", "/LOCAL", "/INITSTEP", 
				"/MAXOBJECTS", "/VOLUME", "/ALL", "/TYPE", ""}
		},
		{
			"MONITOR",
			&monitor_func,
			{ "rtt_arg1", ""}
		},
		{
			"ADD",
			&add_func,
			{ "rtt_arg1", "rtt_arg2", "/NAME", "/CLASS", 
				"/HIERARCHY", "/PARAMETER", "/LOCAL",
				"/TEXT", "/OBJECT", "/COMMAND", ""}
		},
		{
			"COLLECT",
			&collect_func,
			{ "rtt_arg1", "/NAME", ""}
		},
		{
			"DEBUG",
			&debug_func,
			{ "rtt_arg1", "rtt_arg2", "/NAME", "/CLASS", 
			"/HIERARCHY", "/FILE", "/LOCAL", ""}
		},
		{
			"CROSSREFERENCE",
			&crossref_func,
			{ "rtt_arg1", "/NAME", "/FILE", "/STRING", "/BRIEF",
			"/FUNCTION", "/CASE_SENSITIVE", ""}
		},
		{
			"PRINT",
			&rttcmd_print_func,
			{ "rtt_arg1", "/FILE", "/APPEND", "/TSIZE",
			"/PSIZE", "/TEXT", "/TERMINAL", "/RESTORE", ""}
		},
		{
			"SAY",
			&rttcmd_say_func,
			{ "rtt_arg1", "/TEXT", ""}
		},
		{
			"SEARCH",
			&search_func,
			{ "rtt_arg1", ""}
		},
		{
			"LOGGING",
			&logging_func,
			{ "rtt_arg1", "rtt_arg2", "/FILE", "/TIME", "/ENTRY", 
			"/TYPE", "/PARAMETER", "/CONDITION", "/INSERT", 
			"/BUFFER_SIZE", "/PRIORITY", "/STOP", "/NOSTOP", 
			"/CREATE", "/ALL", "/LINE_SIZE", "/SHORTNAME", 
			"/NOSHORTNAME", ""}
		},
		{
			"ALARM",
			&alarm_func,
			{ "rtt_arg1", "/TEXT", "/PRIORITY", "/USER",
				"/MAXALARM", "/MAXEVENT", "/FILE", 
				"/NONAME", "/NOTEXT", "/ACKNOWLEDGE", 
				"/RETURN", "/BEEP", "/START", "/STOP", ""}
		},
		{
			"LEARN",
			&rttcmd_learn_func,
			{ "rtt_arg1", "/FILE", ""}
		},
		{
			"STORE",
			&store_func,
			{ "rtt_arg1", "/COLLECT", "/FILE", "/SYMBOLS", ""}
		},
		{
			"WAIT",
			&wait_func,
			{ "/TIME", "/PLCPGM", ""}
		},
		{
			"SETUP",
			&rtt_setup_func,
			{ ""}
		},
		{
			"SET",
			&rtt_set_func,
			{ "rtt_arg1", "rtt_arg2", "/PRIORITY", "/NAME", 
			"/VALUE", "/RTDB_OFFSET", "/BYPASS", "/ON", "/OFF", 
			"/MESSAGE", "/COMMAND", "/TIME", ""}
		},
		{
			"GET",
			&rtt_get_func,
			{ "rtt_arg1", ""}
		},
		{
			"DIRECTORY",
			&directory_func,
			{ "rtt_arg1", ""}
		},
		{
			"DEFINE",
			&rttcmd_define_func,
			{ "rtt_arg1", "rtt_arg2", "rtt_arg4", "rtt_arg4", ""}
		},
		{
			"PLCSCAN",
			&plcscan_func,
			{ "/ON", "/OFF", "/ALL", "/HIERARCHY", "/GLOBAL", ""}
		},
		{
			"PAGE",
			&page_func,
			{ "rtt_arg1", ""}
		},
		{
			"TOP",
			&top_func,
			{ ""}
		},
		{
			"HELP",
			&help_func,
			{ "rtt_arg1", "rtt_arg2", "rtt_arg3", "rtt_arg4",""}
		},
		{
			"EXIT",
			&exit_func,
			{ "",}
		},
		{
			"QUIT",
			&exit_func,
			{ "",}
		},
		{
			"CLASSHIER",
			&classhier_func,
			{ ""}
		},
		{
			"CREATE",
			&rtt_create_func,
			{ "rtt_arg1", "/NAME", "/CLASS", "/TITLE", "/TEXT",
			"/COMMAND", "/OBJECT", ""}
		},
		{
			"DELETE",
			&rtt_delete_func,
			{ "rtt_arg1", "/NAME"}
		},
		{
			"VIEW",
			&rtt_view_func,
			{ "rtt_arg1", "/FILE"}
		},
		{"",}};


/*************************************************************************
*
* Name:		rtt_get_command()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* unsigned long	*chn		I	channel.
* unsigned long	recall		I	recall buffer.
* int		timeout		I	timeout time.
* int		(* timeout_func) () I	function called at timeout.
* unsigned long	timeout_arg	I	argument passed to timeout_func.
* char		*prompt		I	input prompt.
* int		x		I	x koordinate for input prompt.
* int		y		I	y koordinate for input prompt.
*
* Description:
*	This function writes a prompt and waits for a input string.
*	If a terminator key is not recieved within the timeout time
*	the timeoutfunction is called with the timeout argument.
*	The function continues to read the input string and calls the
*	timeout function with timeout inverval until a terminator key 
*	is recieved.
*
**************************************************************************/

int	rtt_get_command(
			menu_ctx	ctx,
			char		*chn,
			rtt_t_recall	*recall,
			int		timeout,
			int		(* timeout_func) (),
			void		*timeout_arg,
			char		*prompt,
			int		x,
			int		y,
			rtt_t_comtbl	*command_table)
{

	unsigned long	terminator;
	unsigned long	option;
	char		input_str[160];
	char		command[160];
	int		maxlen = 30;
	int		sts, sym_sts;
	char		symbol_value[80];

	if ( timeout_func != NULL )
	  option = RTT_OPT_TIMEOUT | RTT_OPT_NOSCROLL;
	else
	  option = RTT_OPT_NOSCROLL;

	while (1)
	{

	  if ( rtt_commandmode & RTT_COMMANDMODE_FILE)
	  {
	    sts = rtt_commandmode_getnext( input_str, &terminator);
	    if ( EVEN(sts))
	    {
	      /* End of file */
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    rtt_cursor_abs( x, y);
	    rtt_eofline_erase();
	    rtt_get_input_string( chn, input_str, &terminator, 
		maxlen, (rtt_t_recall *) recall, option, timeout, 
		timeout_func, timeout_arg, prompt); 
	    rtt_message('S',"");
	    rtt_command_toupper( input_str, input_str);
	  }


	  if ( rtt_commandmode & RTT_COMMANDMODE_LEARN &&
	       input_str[0] != '@')
	    rtt_learn_store( input_str);

	  /* Find symbols */
	  sts = rtt_replace_symbol( input_str, command);

	  /* Print command on terminal if verify */
	  if ( rtt_commandmode & RTT_COMMANDMODE_FILE && rtt_verify)
	    printf( "\n%%RTT-I-CMD, %s", command);
 
	  /* Print command on file if file on */
	  if ( rtt_commandmode & RTT_COMMANDMODE_FILE && 
			rtt_file_on && rtt_print_command)
	  {
	    /* Don't print the print and say commands */
	    if ( strncmp( command, "PRINT", 5) &&
		 strncmp( command, "SAY", 3))
	      fprintf( rtt_outfile, "%%RTT-I-CMD, %s\n", command);
	  }

	  if ( input_str[0] == '@')
	  {
	    /* Read command file */
	    sts = rtt_commandmode_start( &command[1], 0);
	    if ( sts == RTT__NOFILE)
	    {
	      rtt_message('E',"Unable to open file");
	      continue;
	    }
	    else if ( EVEN(sts)) return sts;
	    return RTT__NOPICTURE;
	  }

	  sts = rtt_cli( command_table, command, (void *) ctx, 0);
	  if ( sts == RTT__COM_NODEF)
	  {
	    /* Try to find a matching symbol */
	    sym_sts = rtt_get_symbol_cmd( command, symbol_value);
            if ( ODD(sym_sts))
	    {
	      if ( symbol_value[0] == '@')
	      {
	        /* Read command file */
	        sts = rtt_commandmode_start( &symbol_value[1], 0);
	        if ( sts == RTT__NOFILE)
	        {
	          rtt_message('E',"Unable to open file");
	          continue;
	        }
	        else if ( EVEN(sts)) return sts;
	        return RTT__NOPICTURE;
	      }
	      sts = rtt_cli( command_table, symbol_value, (void *) ctx, 0);
	    }
	    else if ( sym_sts == RTT__SYMBOL_AMBIG)
	      sts = sym_sts;
	  }

	  rtt_cursor_abs( x, y);
	  rtt_eofline_erase();
	  if (sts == RTT__COM_AMBIG) rtt_message('E',"Ambiguous command");
	  else if (sts == RTT__COM_NODEF) rtt_message('E',"Undefined command");
	  else if (sts == RTT__QUAL_AMBIG) rtt_message('E',"Ambiguous qualifier");
	  else if (sts == RTT__QUAL_NODEF) rtt_message('E',"Undefined qualifier");
	  else if (sts == RTT__SYMBOL_AMBIG) rtt_message('E', "Ambiguous symbol abbrevation");
	  else if ( (ODD(sts)) && (sts != RTT__HOLDCOMMAND)) return sts;
	  else if ( EVEN(sts)) return sts;
	  if ((terminator >= RTT_K_PF1) && (terminator <= RTT_K_PF4))
	    break;
	  if ( rtt_commandmode & RTT_COMMANDMODE_FILE)
	    break;
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_execute_file()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
*
* Description:
*
**************************************************************************/

int	rtt_menu_execute_file(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*filename,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	char	command[256];
	int	sts;

	if ( strstr( filename, ".RTT_COM") ||
	     strstr( filename, ".rtt_com"))
	{
	  strcpy( command, "@");
	  strcat( command, filename);
	  sts = rtt_menu_command( ctx, argoi, command, arg2, arg3, arg4);
	  return sts;
	}
	else if ( strstr( filename, ".TXT") ||
	          strstr( filename, ".txt"))
	{
	  sts = rtt_view( ctx, filename, 0, 0, RTT_VIEWTYPE_FILE);
	  return sts;
	}
	else
	{
	  rtt_message( 'E', "Function not defined for this file");
	  return RTT__NOPICTURE;
	}

	return RTT__NOPICTURE;
}
/*************************************************************************
*
* Name:		rtt_menu_command()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
*
* Description:
*
**************************************************************************/

int	rtt_menu_command(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*incommand,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{

	int		sts, sym_sts;
	char		*file;
	char		command[256];
	char		symbol_value[80];

	rtt_command_toupper( command, incommand);

	if ( *command == '@')
	{
	  /* Read command file */
	  file = command + 1;
	  sts = rtt_commandmode_start( file, 0);
	  if ( sts == RTT__NOFILE)
	  {
	    rtt_message('E',"Unable to open file");
	    return RTT__NOPICTURE;
	  }
	  else if ( EVEN(sts)) return sts;
	  return RTT__NOPICTURE;
	}
	else
	{
	  sts = rtt_cli( (rtt_t_comtbl *) rtt_command_table, command, 
			(void *) ctx, &one);
	  if ( sts == RTT__COM_NODEF)
	  {
	    /* Try to find a matching symbol */
	    sym_sts = rtt_get_symbol_cmd( command, symbol_value);
            if ( ODD(sym_sts))
	      sts = rtt_cli( (rtt_t_comtbl *) rtt_command_table, symbol_value,
			 (void *) ctx, &one);
	    else if ( sym_sts == RTT__SYMBOL_AMBIG)
	      sts = sym_sts;
	  }
	  if (sts == RTT__COM_AMBIG) rtt_message('E',"Ambiguous command");
	  else if (sts == RTT__COM_NODEF) rtt_message('E',"Undefined command");
	  else if (sts == RTT__QUAL_AMBIG) rtt_message('E',"Ambiguous qualifier");
	  else if (sts == RTT__QUAL_NODEF) rtt_message('E',"Undefined qualifier");
	  else if (sts == RTT__SYMBOL_AMBIG) rtt_message('E', "Ambiguous symbol abbrevation");
	  else return sts;
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_commandhold()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
*
* Description:
*
**************************************************************************/

int	rtt_menu_commandhold(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*incommand,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	int		sts;

	rtt_noredraw = 1;
	sts = rtt_menu_command( ctx, argoi, incommand, arg2, arg3, arg4);
        return sts;
}

/*************************************************************************
*
* Name:		rtt_menu_vmscommand()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
*
* Description:
*
**************************************************************************/

int	rtt_menu_vmscommand(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*incommand,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
#if defined OS_ELN
	rtt_message('E', "Function not defined in ELN");
#else
	int		sts;
	char		command[256];

	rtt_command_toupper( command, incommand);

	/* Place cursor in message position */
	rtt_cursor_abs( 0, RTT_ROW_COMMAND);
	r_print_buffer();
        qio_reset( (int *) &rtt_chn);
	sts = system( command);
        qio_set_attr( (int *) &rtt_chn);
	if ( EVEN(sts))
	{
	  rtt_message('E', "Error in executing command");
	}
#endif
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_vmscommand_nowait()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
*
* Description:
*
**************************************************************************/

int	rtt_menu_vmscommand_nowait(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*command,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
#if defined OS_VMS
	int		sts;
	char	cmd[100];
	struct dsc$descriptor_s	cmd_desc;
        unsigned long cli_flag = CLI$M_NOWAIT;

	cmd_desc.dsc$w_length = sizeof(cmd) - 1;
	cmd_desc.dsc$b_dtype = DSC$K_DTYPE_T;
	cmd_desc.dsc$b_class = DSC$K_CLASS_S;
	cmd_desc.dsc$a_pointer = cmd;

	rtt_command_toupper( cmd, command);

	/* Place cursor in message position */
	rtt_cursor_abs( 0, RTT_ROW_COMMAND);
	r_print_buffer();
	sts = lib$spawn (&cmd_desc , 0 , 0 , &cli_flag );
	if ( sts != SS$_NORMAL )
	  rtt_message('E',"Unable to create subprocess");
#elif OS_ELN
	rtt_message('E', "Function not defined in ELN");
#endif
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_vmscommandconf()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
*
* Description:
*
**************************************************************************/

int	rtt_menu_vmscommandconf(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*incommand,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{

#if defined OS_ELN
	rtt_message('E', "Function not defined in ELN");
#else
	int		sts;
	char		message[120];
	rtt_t_menu	*menu_ptr;
	char		command[256];

	/* Confirm */
	menu_ptr = ctx->menu;
	menu_ptr += ctx->current_item;
	strcpy( message, "Confirm \"");
	strcat( message, menu_ptr->text);
	strcat( message, "\"");
	sts = rtt_confirm( message);
	if ( sts != RTT__SUCCESS)
	  return RTT__NOPICTURE;

	rtt_message('S',"");
	rtt_cursor_abs( 0, RTT_ROW_COMMAND);
	rtt_eofline_erase();

	rtt_command_toupper( command, incommand);

	/* Place cursor in message position */
	rtt_cursor_abs( 0, RTT_ROW_COMMAND);
	r_print_buffer();

        qio_reset( (int *) &rtt_chn);
	sts = system( command);
        qio_set_attr( (int *) &rtt_chn);
	if ( EVEN(sts))
	{
	  rtt_message('E', "Error in executing command");
	}
#endif
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_vmscommandhold()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
*
* Description:
*
**************************************************************************/

int	rtt_menu_vmscommandhold(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*incommand,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{

#if defined OS_ELN
	rtt_message('E', "Function not defined in ELN");
#else
	int		sts;
	char		command[256];

	rtt_command_toupper( command, incommand);

	/* Place cursor in message position */
	rtt_cursor_abs( 0, RTT_ROW_COMMAND);
	r_print_buffer();
        qio_reset( (int *) &rtt_chn);
	sts = system( command);
        qio_set_attr( (int *) &rtt_chn);
	if ( EVEN(sts))
	{
	  rtt_message('E', "Error in executing command");
	}
#endif
	return RTT__NOPICTURE;
}


/*************************************************************************
*
* Name:		rtt_show_object_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	objid		I	objid of object to insert.
* rtt_t_menu	**menulist	I	menulist.
* int		*index		I	index in menulist.
* int		dum2		I	
* int		dum3		I	
* int		dum4		I	
*
* Description:
*	Inserts an object in the menu list at a 'show object' command. 
*	This function is called when an object that fits the description
*	is found by rtt_get_objects_hier_class_name.
*
**************************************************************************/

static int	rtt_show_object_add(
			pwr_tObjid	objid,
			rtt_t_menu	**menulist,
			int		*index,
			void		*dum2,
			void		*dum3,
			void		*dum4)
{
	char	objname[80];
	int	sts;
	char	title[120];
	char	classname[32];
	pwr_tObjid	childobjid;
	int		j;

	/* Get the object name */
	sts = gdh_ObjidToName ( objid, objname, sizeof(objname), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get class name */	
	sts = rtt_objidtoclassname( objid, classname);
	if (EVEN(sts)) return sts;

	/* Add class name to objname in title */
	strcpy( title, objname);
	for ( j = strlen( title); j < 25; j++)
	  strcat( title, " ");
	strcat( title, " ");
	strcat( title, classname);

	/* Mark if the object has children */
	sts = gdh_GetChild( objid, &childobjid);
	if ( ODD(sts))
	  strcat( title, " *");

	sts = rtt_menu_list_add( menulist, *index, 0, title,
		&rtt_hierarchy_child,
		&rtt_object_parameters, 
		&rtt_debug_child,
		objid,0,0,0,0);
	if ( EVEN(sts)) return sts;

	(*index)++;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_show_parameter_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	objid		I	objid of found object.
* rtt_t_menu_upd **menulist	I	menulist.
* char		*parname	I	parameter name.
* int		*index		I	index in menulist.
* int		dum3		I	
* int		dum4		I	
*
* Description:
*	Inserts an object and parameter in the menu list at a 
*	'show parameter' command. 
*	This function is called when an object that fits the description
*	is found by rtt_get_objects_hier_class_name.
*	
*
**************************************************************************/

static int	rtt_show_parameter_add(
			pwr_tObjid	objid,
			rtt_t_menu_upd	**menulist,
			char		*parname,
			int		*index,
			int		*elem,
			void		*dum4)
{
	int		j;
	char		objname[100];
	int		sts;
	char		hiername[80];
	pwr_tClassId	class;
	int		elements;
	char 		*parameter_ptr;
	SUBID 		subid;
	pwr_sParInfo	parinfo;
	char		title[120];
	char		classname[80];
	pwr_tObjid	childobjid;
	pwr_tTypeId	attrtype;
	unsigned int	attrsize, attroffs, attrelem;
	char		*s;
        int		element;

	element = *elem;

	/* Get the object name */
	sts = gdh_ObjidToName ( objid, objname, sizeof(objname), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( objname, ".");
	strcat( objname, parname);

	sts = gdh_GetAttributeCharacteristics ( objname,
		&attrtype, &attrsize, &attroffs, &attrelem);
	if ( EVEN(sts)) return sts;

	/* Get rtdb pointer */
	sts = gdh_RefObjectInfo ( objname, (pwr_tAddress *) &parameter_ptr, 
			&subid, attrsize);
	if ( (EVEN(sts))  /****NYGDH || ( sts == (GDH__NODEDOWN -1)) ****/)
	{
	  parameter_ptr = 0;
	}

	/* Get class name */	
	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;
	sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername,
			sizeof(hiername), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Remove index if element in an array */
	if ( (s = strrchr( parname, '[')))
	  *s = 0;
	strcat( hiername, "-RTBODY-");
	strcat( hiername, parname);

	sts = gdh_GetObjectInfo ( hiername, &parinfo, sizeof(parinfo)); 
	if (EVEN(sts))
	{
	  /* Try with sysbody */
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
			sizeof(hiername), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;

	  strcat( hiername, "-SYSBODY-");
	  strcat( hiername, parname);

	  sts = gdh_GetObjectInfo ( hiername, &parinfo, sizeof(parinfo)); 
	  if (EVEN(sts)) return RTT__ITEM_NOCREA;
	}	

	elements = 1;
	if ( parinfo.Flags & PWR_MASK_ARRAY )
	{
	  elements = parinfo.Elements;
	  if ( element > (elements -1))
	    element = elements -1;
	  else if ( element < 0)
	    element = 0;
	}
	else
	  element = -1;

	/* gdh returns the pointer (not a pointer to a pointer) 
	   remove the pointer bit in Flags */
	if ( parinfo.Flags & PWR_MASK_POINTER )
	  parinfo.Flags -= PWR_MASK_POINTER;


	if ( element >= 0)
	  parameter_ptr += element * parinfo.Size / elements;

	/* Get class name */	
	sts = rtt_objidtoclassname( objid, classname);
	if (EVEN(sts)) return sts;

	/* Create at title with objname without volume and classname */
	s = strchr( objname, ':');
	if ( s)
	  s++;
	else
	  s = objname;
	strcpy( title, s);
	  	
	for ( j = strlen( title); j < 45; j++)
	  strcat( title, " ");
	strcat( title, " ");
	strcat( title, classname);

	/* Mark if the object has children */
	sts = gdh_GetChild( objid, &childobjid);
	if ( ODD(sts))
	  strcat( title, " *");

	sts = rtt_menu_upd_list_add( menulist, *index, 0, title, 
		&rtt_hierarchy_child,
		&rtt_object_parameters, 
		&rtt_debug_child,
		objid,0,0,0,0, objname,
		RTT_PRIV_NOOP, parameter_ptr, parinfo.Type, parinfo.Flags, 
		parinfo.Size / elements, subid, 0, 0, 0, 0, 0.0, 0.0, 
		RTT_DATABASE_GDH, 0);
	if ( EVEN(sts)) return sts;
	(*index)++;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_debug_object_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid	objid		I	objid of object.
* rtt_t_menu_upd **menulist	I	menulist.
* int		*index		I	index in menulist.
* void		*dum2		I		
* void		*dum3		I		
* void		*dum4		I	
*
* Description:
*	Inserts an object and parameter in the menu list at a 
*	'debug object' command. 
*	This function is called when an object that fits the description
*	is found by rtt_get_objects_hier_class_name.
*
**************************************************************************/

int	rtt_debug_object_add(
			pwr_tObjid	objid,
			rtt_t_menu_upd	**menulist,
			int		*index,
			int		*crossref,
			void		*dum3,
			void		*dum4)
{
	int		j;
	char		objname[100];
	int		sts;
	char		*s;
	char		hiername[80];
	pwr_tClassId	class;
	unsigned long	elements;
	char 		*parameter_ptr;
	SUBID 		subid;
	pwr_sParInfo	parinfo;
	char		parname[80];
	char		title[120];
	char		classname[80];
	pwr_tObjid	childobjid;
	pwr_tTypeId	attrtype;
	unsigned int	attrsize, attroffs, attrelem;

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Di:
	  case pwr_cClass_Dv:
	  case pwr_cClass_Do:
	  case pwr_cClass_Po:
	  case pwr_cClass_Av:
	  case pwr_cClass_Ai:
	  case pwr_cClass_Ao:
	  case pwr_cClass_Ii:
	  case pwr_cClass_Io:
	  case pwr_cClass_Iv:
	    strcpy( parname, "ActualValue");	
	    break;   
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	  case pwr_cClass_ChanAi:
	  case pwr_cClass_ChanAo:
	  case pwr_cClass_ChanIi:
	  case pwr_cClass_ChanIo:
	    sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	    strcat( objname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( objname, &objid, sizeof( objid));
	    if (EVEN(sts)) return sts;
	    strcpy( parname, "ActualValue");
	    break;
	  case pwr_cClass_ChanCo:
	    sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	    strcat( objname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( objname, &objid, sizeof( objid));
	    if (EVEN(sts)) return sts;
	    strcpy( parname, "AbsValue");	
	    break;
	  case pwr_cClass_trans :
	    strcpy( parname, "Cond");
	    break;   
	  case pwr_cClass_order :
	  case pwr_cClass_dorder :
	  case pwr_cClass_porder :
	  case pwr_cClass_lorder :
	  case pwr_cClass_sorder :
	    strcpy( parname, "Status"); /* Status[0] */
	    break;   
	  case pwr_cClass_ssbegin :
	  case pwr_cClass_ssend :
	  case pwr_cClass_step :
	  case pwr_cClass_initstep :
	  case pwr_cClass_substep :
	    strcpy( parname, "Order"); /* Order[0] */
	    break;   
	  case pwr_cClass_cstoao :
	  case pwr_cClass_cstoav :
	  case pwr_cClass_cstoap :
	    strcpy( parname, "Cond");
	    break;   
	  case pwr_cClass_and :
	  case pwr_cClass_or :
	  case pwr_cClass_xor :
	  case pwr_cClass_edge :
	  case pwr_cClass_sr_s :
	  case pwr_cClass_sr_r :
	  case pwr_cClass_pulse :
	  case pwr_cClass_wait :
	  case pwr_cClass_timer :
	  case pwr_cClass_inv :
	  case pwr_cClass_waith :
	  case pwr_cClass_darithm :
	    strcpy( parname, "Status");
	    break;   
	  case pwr_cClass_DSup :
	  case pwr_cClass_ASup :
	    strcpy( parname, "Action");
	    break;   
	  case pwr_cClass_csub :
	    strcpy( parname, "in");
	    break;   
	  case pwr_cClass_sum :
	  case pwr_cClass_limit :
	  case pwr_cClass_select :
	  case pwr_cClass_ramp :
	  case pwr_cClass_filter :
	  case pwr_cClass_speed :
	  case pwr_cClass_curve :
	  case pwr_cClass_adelay :
	  case pwr_cClass_aarithm :
	  case pwr_cClass_timint :
	    strcpy( parname, "ActVal");
	    break;   
	  case pwr_cClass_maxmin :
	    strcpy( parname, "MaxVal");
	    break;   
	  case pwr_cClass_comph :
	    strcpy( parname, "High");
	    break;   
	  case pwr_cClass_compl :
	    strcpy( parname, "Low");
	    break;   
	  case pwr_cClass_pid :
	    strcpy( parname, "OutVal");
	    break;   
	  case pwr_cClass_mode :
	    strcpy( parname, "OutVal");
	    break;   
	  default:
	    return RTT__ITEM_NOCREA;
	}

	/* Get the object name */
	sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( objname, ".");
	strcat( objname, parname);

	/* Get rtdb pointer */
	sts = gdh_GetAttributeCharacteristics ( objname,
		&attrtype, &attrsize, &attroffs, &attrelem);
	if ( EVEN(sts)) return sts;

	sts = gdh_RefObjectInfo ( objname, (pwr_tAddress *) &parameter_ptr, &subid, attrsize);
	if ( (EVEN(sts))  /****NYGDH || ( sts == (GDH__NODEDOWN -1)) ****/)
	{
	  parameter_ptr = 0;
	}
	/* Get class name */	
	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
		sizeof(hiername), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	strcat( hiername, "-RTBODY-");
	strcat( hiername, parname);

	sts = gdh_GetObjectInfo ( hiername, &parinfo, sizeof(parinfo)); 
	if (EVEN(sts)) return RTT__ITEM_NOCREA;
	
	elements = 1;
	if ( parinfo.Flags & PWR_MASK_ARRAY )
	  elements = parinfo.Elements;

	/* gdh returns the pointer (not a pointer to a pointer) 
	   remove the pointer bit in Flags */
	if ( parinfo.Flags & PWR_MASK_POINTER )
	  parinfo.Flags -= PWR_MASK_POINTER;

/*	parameter_ptr += j * parinfo.Size / elements;*/

	/* Get class name */	
	sts = rtt_objidtoclassname( objid, classname);
	if (EVEN(sts)) return sts;

	/* Add class name to objname in title */
	s = strchr( objname, ':');
	if ( s)
	  s++;
	else
	  s = objname;
	strcpy( title, s);
	for ( j = strlen( title); j < 45; j++)
	  strcat( title, " ");
	strcat( title, " ");
	strcat( title, classname);

	/* Mark if the object has children */
	sts = gdh_GetChild( objid, &childobjid);
	if ( ODD(sts))
	  strcat( title, " *");

	if ( *crossref)
	{
	  sts = rtt_menu_upd_list_add( menulist, *index, 0, title, 
		&rtt_hierarchy_child,
		&rtt_object_parameters, 
		&rtt_crossref_signal,
		objid,0,0,0,0, objname,
		RTT_PRIV_NOOP, parameter_ptr, parinfo.Type, parinfo.Flags, 
		parinfo.Size / elements, subid, 0, 0, 0, 0, 0.0, 0.0, 
		RTT_DATABASE_GDH, 0);
	  if ( EVEN(sts)) return sts;
	}
	else
	{
	  sts = rtt_menu_upd_list_add( menulist, *index, 0, title, 
		&rtt_hierarchy_child,
		&rtt_object_parameters, 
		&rtt_debug_child,
		objid,0,0,0,0, objname,
		RTT_PRIV_NOOP, parameter_ptr, parinfo.Type, parinfo.Flags, 
		parinfo.Size / elements, subid, 0, 0, 0, 0, 0.0, 0.0, 
		RTT_DATABASE_GDH, 0);
	  if ( EVEN(sts)) return sts;
	}
	(*index)++;

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_debug_child_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid	objid		I	objid of object to add in menulist.
* rtt_t_menu_upd **menulist	I	menulist.
* int		*index		I	index in menulist.
* void		*dum2		I	
* void		*dum3		I	
* void		*dum4		I	
*
* Description:
*	Inserts an object and parameter in the menu list at a 
*	'debug children' command (PF2). 
*
**************************************************************************/

int	rtt_debug_child_add(
			pwr_tObjid	objid,
			rtt_t_menu_upd	**menulist,
			int		*index,
			int		*allocated,
			void		*dum3,
			void		*dum4)
{
	int	j;
	char	objname[100];
	int	sts;
	char		hiername[80];
	pwr_tClassId	class;
	unsigned long	elements;
	char 		*parameter_ptr;
	SUBID 		subid;
	pwr_sParInfo	parinfo;
	char		parname[32];
	char		full_parname[120];
	char		con_parname[120];
	char		title[120];
	char		spectitle[80] = "";
	char		classname[80];
	char		*s, *t;
	pwr_tObjid	con_obj;
	pwr_tObjid	childobjid;
	pwr_tTypeId	attrtype;
	unsigned int	attrsize, attroffs, attrelem;
	int		conv_on;
	int		invert_on;
	int		dotest_on, dotestvalue_on;

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	/* Get the object name */
	sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	strcpy( full_parname, objname);
	strcat( full_parname, ".");


	switch ( class)
	{
	  case pwr_cClass_Dv:
	  case pwr_cClass_Av:
	    strcpy( parname, "ActualValue");	
	    strcat( full_parname, parname);	
	    break;
	  case pwr_cClass_Di:
	  case pwr_cClass_Po:
	  case pwr_cClass_Do:
	  case pwr_cClass_Ai:
	  case pwr_cClass_Ao:
	  case pwr_cClass_Co:
	    if ( class == pwr_cClass_Co)
	      strcpy( parname, "AbsValue");	
	    else
	      strcpy( parname, "ActualValue");	

	    strcat( full_parname, parname);
	    /* Create a special title */
	    strcpy( con_parname, objname);
	    strcat( con_parname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( con_parname, &con_obj, sizeof(con_obj));
	    if (EVEN(sts)) break;
	    sts = gdh_ObjidToName ( con_obj, hiername, sizeof(hiername), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) break;
	    /* We want the two last parts of the name */
	    s = strrchr( hiername, '-');
	    if ( s == 0)
	      strcpy( spectitle, hiername);
	    else
	    {
	      *s = '+';
	      t = strrchr( hiername, '-');
	      *s = '-';
	      if ( t == 0)
	        strcpy( spectitle, hiername);
	      else
	        strcpy( spectitle, t + 1);
	    }
	    break;   
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	  case pwr_cClass_ChanAi:
	  case pwr_cClass_ChanAo:
	  case pwr_cClass_ChanCo:
	    /* Look at the value of the connected object */
	    strcpy( con_parname, objname);
	    if ( class == pwr_cClass_ChanDi) 
	    {
	      class = pwr_cClass_Di;
	      strcpy( parname, "ActualValue");	
	      sts = rtt_get_conversion(	objid, &conv_on);
	      if (EVEN(sts))
	        strcpy( spectitle, " ");
	      else if ( conv_on)
	        strcpy( spectitle, "C");
	      else
	        strcpy( spectitle, " ");
	      sts = rtt_get_invert( objid, &invert_on);
	      if (EVEN(sts))
	        strcat( spectitle, "  ");
	      else if ( invert_on)
	        strcat( spectitle, "I ");
	      else
	        strcat( spectitle, "  ");
	    }
	    else if ( class == pwr_cClass_ChanDo) 
	    {
	      class = pwr_cClass_Do;
	      strcpy( parname, "ActualValue");	
	      sts = rtt_get_invert( objid, &invert_on);
	      if (EVEN(sts))
	        strcpy( spectitle, " ");
	      else if ( invert_on)
	        strcpy( spectitle, "I");
	      else
	        strcpy( spectitle, " ");
	      sts = rtt_get_do_test( objid, &dotest_on);
	      if (EVEN(sts))
	        strcat( spectitle, " ");
	      else if ( dotest_on)
	      {
	        strcat( spectitle, "T");
	        sts = rtt_get_do_testvalue( objid, &dotestvalue_on);
	        if (EVEN(sts))
	          strcat( spectitle, " ");
	        else if ( dotestvalue_on)
	          strcat( spectitle, "1 ");
	        else
	          strcat( spectitle, "0 ");
	      }
	      else
	        strcat( spectitle, " ");
	    }
	    else if ( class == pwr_cClass_ChanAi) 
	    {
	      class = pwr_cClass_Ai;
	      strcpy( parname, "SigValue");	
	      strcpy( spectitle, "");
	    }
	    else if ( class == pwr_cClass_ChanAo) 
	    {
	      class = pwr_cClass_Ao;
	      strcpy( parname, "SigValue");	
	      strcpy( spectitle, "");
	    }
	    else if ( class == pwr_cClass_ChanCo) 
	    {
	      class = pwr_cClass_Co;
	      strcpy( parname, "AbsValue");	
	      sts = rtt_get_conversion(	objid, &conv_on);
	      if (EVEN(sts))
	        strcpy( spectitle, "  ");
	      else if ( conv_on)
	        strcpy( spectitle, "C ");
	      else
	        strcpy( spectitle, "  ");
	    }
	    strcat( con_parname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( con_parname, &con_obj, sizeof(con_obj));
	    if (EVEN(sts)) break;
	    sts = gdh_ObjidToName ( con_obj, hiername, sizeof(hiername), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) break;
	    /* The rtdbpointer is actualvalue of connected object */
	    strcpy( full_parname, hiername);	
	    strcat( full_parname, ".");	
	    strcat( full_parname, parname);	
	    /* We want the two last parts of the name */
	    s = strrchr( hiername, '-');
	    if ( s == 0)
	      strcat( spectitle, hiername);
	    else
	    {
	      *s = '+';
	      t = strrchr( hiername, '-');
	      *s = '-';
	      if ( t == 0)
	        strcat( spectitle, hiername);
	      else
	        strcat( spectitle, t + 1);
	    }
	    if ( class == pwr_cClass_ChanDi) class = pwr_cClass_Di;
	    else if ( class == pwr_cClass_ChanDo) class = pwr_cClass_Do;
	    else if ( class == pwr_cClass_ChanAi) class = pwr_cClass_Ai;
	    else if ( class == pwr_cClass_ChanAo) class = pwr_cClass_Ao;
	    break;   
	  case pwr_cClass_trans :
	    strcpy( parname, "Cond");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_order :
	  case pwr_cClass_dorder :
	  case pwr_cClass_porder :
	  case pwr_cClass_lorder :
	  case pwr_cClass_sorder :
	    strcpy( parname, "Status"); /* Status[0] */
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_ssbegin :
	  case pwr_cClass_ssend :
	  case pwr_cClass_step :
	  case pwr_cClass_initstep :
	  case pwr_cClass_substep :
	    strcpy( parname, "Order"); /* Order[0] */
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_cstoao :
	  case pwr_cClass_cstoav :
	  case pwr_cClass_cstoap :
	    strcpy( parname, "Cond");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_and :
	  case pwr_cClass_or :
	  case pwr_cClass_xor :
	  case pwr_cClass_edge :
	  case pwr_cClass_sr_s :
	  case pwr_cClass_sr_r :
	  case pwr_cClass_pulse :
	  case pwr_cClass_wait :
	  case pwr_cClass_timer :
	  case pwr_cClass_inv :
	  case pwr_cClass_waith :
	  case pwr_cClass_darithm :
	    strcpy( parname, "Status");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_DSup :
	  case pwr_cClass_ASup :
	    strcpy( parname, "Action");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_csub :
	    strcpy( parname, "in");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_sum :
	  case pwr_cClass_limit :
	  case pwr_cClass_select :
	  case pwr_cClass_ramp :
	  case pwr_cClass_filter :
	  case pwr_cClass_speed :
	  case pwr_cClass_curve :
	  case pwr_cClass_adelay :
	  case pwr_cClass_aarithm :
	  case pwr_cClass_timint :
	    strcpy( parname, "ActVal");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_maxmin :
	    strcpy( parname, "MaxVal");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_comph :
	    strcpy( parname, "High");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_compl :
	    strcpy( parname, "Low");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_pid :
	    strcpy( parname, "OutVal");
	    strcat( full_parname, parname);	
	    break;   
	  case pwr_cClass_mode :
	    strcpy( parname, "OutVal");
	    strcat( full_parname, parname);	
	    break;   
	  default:
	    return RTT__ITEM_NOCREA;
	}

	/* Get rtdb pointer */
	sts = gdh_GetAttributeCharacteristics ( full_parname,
		&attrtype, &attrsize, &attroffs, &attrelem);
	if ( EVEN(sts))
	{
	  /* Not connected channel */
	  return RTT__SUCCESS;
	}

	sts = gdh_RefObjectInfo ( full_parname, (pwr_tAddress *) &parameter_ptr, &subid,
		attrsize);
	if ( (EVEN(sts))  /****NYGDH || ( sts == (GDH__NODEDOWN -1)) ****/)
	{
	  parameter_ptr = 0;
	}

	/* Get class name */	
	sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
			sizeof(hiername), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	strcat( hiername, "-RTBODY-");
	strcat( hiername, parname);

	sts = gdh_GetObjectInfo ( hiername, &parinfo, sizeof(parinfo)); 
	if (EVEN(sts)) return RTT__ITEM_NOCREA;
	
	elements = 1;
	if ( parinfo.Flags & PWR_MASK_ARRAY )
	  elements = parinfo.Elements;

	/* gdh returns the pointer (not a pointer to a pointer) 
	   remove the pointer bit in Flags */
	if ( parinfo.Flags & PWR_MASK_POINTER )
	  parinfo.Flags -= PWR_MASK_POINTER;

/*	parameter_ptr += j * parinfo.Size / elements;*/

	/* Get class name */	
	sts = rtt_objidtoclassname( objid, classname);
	if (EVEN(sts)) return sts;

	/* Create a title, last part of the name is ok */
	s = strrchr( objname, '-');
	if ( s == 0)
	  strcpy( title, objname);
	else
	  strcpy( title, s + 1);

	/* Add class specific title to title */
	for ( j = strlen( title); j < 25; j++)
	  strcat( title, " ");
	strcat( title, " ");
	strcat( title, spectitle);

	/* Add class name to objname in title */
	for ( j = strlen( title); j < 45; j++)
	  strcat( title, " ");
	strcat( title, " ");
	strcat( title, classname);

	/* Mark if the object has children */
	sts = gdh_GetChild( objid, &childobjid);
	if ( ODD(sts))
	  strcat( title, " *");

	sts = rtt_menu_upd_list_add( menulist, *index, *allocated, title, 
		&rtt_hierarchy_child,
		&rtt_object_parameters, 
		&rtt_debug_child,
		objid,0,0,0,0, full_parname,
		RTT_PRIV_NOOP, parameter_ptr, parinfo.Type, parinfo.Flags, 
		parinfo.Size / elements, subid, 0, 0, 0, 0, 0.0, 0.0, 
		RTT_DATABASE_GDH, 0);
	if ( EVEN(sts)) return sts;
	(*index)++;

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_debug_child_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid	objid		I	objid of object to add in menulist.
*
* Description:
*	Check is there is a debug parameter defined for an object.
*
**************************************************************************/

int	rtt_debug_child_check(
			pwr_tObjid	objid)
{
	int	sts;
	pwr_tClassId	class;

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Dv:
	  case pwr_cClass_Av:
	  case pwr_cClass_Di:
	  case pwr_cClass_Do:
	  case pwr_cClass_Po:
	  case pwr_cClass_Ai:
	  case pwr_cClass_Ao:
	  case pwr_cClass_Co:
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	  case pwr_cClass_ChanAi:
	  case pwr_cClass_ChanAo:
	  case pwr_cClass_ChanCo:
	  case pwr_cClass_trans :
	  case pwr_cClass_order :
	  case pwr_cClass_dorder :
	  case pwr_cClass_porder :
	  case pwr_cClass_lorder :
	  case pwr_cClass_sorder :
	  case pwr_cClass_ssbegin :
	  case pwr_cClass_ssend :
	  case pwr_cClass_step :
	  case pwr_cClass_initstep :
	  case pwr_cClass_substep :
	  case pwr_cClass_cstoao :
	  case pwr_cClass_cstoav :
	  case pwr_cClass_cstoap :
	  case pwr_cClass_and :
	  case pwr_cClass_or :
	  case pwr_cClass_xor :
	  case pwr_cClass_edge :
	  case pwr_cClass_sr_s :
	  case pwr_cClass_sr_r :
	  case pwr_cClass_pulse :
	  case pwr_cClass_wait :
	  case pwr_cClass_timer :
	  case pwr_cClass_inv :
	  case pwr_cClass_waith :
	  case pwr_cClass_darithm :
	  case pwr_cClass_DSup :
	  case pwr_cClass_ASup :
	  case pwr_cClass_csub :
	  case pwr_cClass_sum :
	  case pwr_cClass_limit :
	  case pwr_cClass_select :
	  case pwr_cClass_ramp :
	  case pwr_cClass_filter :
	  case pwr_cClass_speed :
	  case pwr_cClass_curve :
	  case pwr_cClass_adelay :
	  case pwr_cClass_aarithm :
	  case pwr_cClass_timint :
	  case pwr_cClass_maxmin :
	  case pwr_cClass_comph :
	  case pwr_cClass_compl :
	  case pwr_cClass_pid :
	  case pwr_cClass_mode :
	    break;   
	  default:
	    return RTT__ITEM_NOCREA;
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_show_obj_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	rtt context.
* char		*hiername	I	hierarchy object name.
* char		*classname	I	class name.
* char		*name		I	object name descripion.
*
* Description:
*	This function is called when a 'show object' command is recieved.
*	All object under the hierarchy object, with the specified class
*	that fits in the name description is inserted in a menulist
*	and displayed on the screen.
*
**************************************************************************/

int	rtt_show_obj_hier_class_name(
			menu_ctx	parent_ctx,
			char		*hiername,
			char		*classname,
			char		*name,
			int		global,
			int		max_objects)
{
	int		sts;
	int		index = 0;
	pwr_tObjid	objid;
	pwr_tClassId	class;
	pwr_tObjid	hierobjid;
	rtt_t_menu	*menulist = 0;
	char		title[80] = "SEARCH LIST";
	char		*s;

	
	if ( max_objects == 0)
	  max_objects = 300;

	if ( name != NULL)
	{
	  /* Check if name does not include a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Get objid for the object */
	    sts = rtt_find_name( parent_ctx, name, &objid);
/*	    sts = gdh_NameToObjid ( name, &objid);  */
	    if ( EVEN(sts))
	    {
     	      rtt_message('E',"Object does not exist");
	      return RTT__HOLDCOMMAND;
	    }
	    sts = rtt_object_parameters( parent_ctx, objid,0,0,0,0);
	    if ( sts == RTT__NOPICTURE)
	    {
	      /* This object could not be opened, show it in an ordinary
		 menu */
	      sts = rtt_show_object_add( objid, &menulist, &index, 0, 0, 0);
	      if ( EVEN(sts)) return sts;
	      sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
				RTT_MENUTYPE_DYN);
	      return sts;
	    }
	    else
	      return sts;
	  }
	  else
	    /* Convert name to upper case */
	    rtt_toupper( name, name);
	}

	/* Check if class */
	if ( classname != NULL )
	{
	  /* Get classid for the class */
	  sts = gdh_ClassNameToId ( classname, &class);
	  if ( EVEN(sts))
	  {
	    /* Class not found */
	    rtt_message('E',"Unknown class");
	    return RTT__HOLDCOMMAND;
	  }
	}	
	else
	  class = 0;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  if ( *hiername == '\0')
	  {
	    /* No value is given, take the title as default */
	    sts = rtt_find_hierarchy( parent_ctx, &hierobjid);
	    if (EVEN(sts)) 
	    {
	      rtt_message('E', "No hierarchy found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    /* Get objid for the hierarchy object */
	    sts = gdh_NameToObjid ( hiername, &hierobjid);
	    if (EVEN(sts))
	    {
	      rtt_message('E',"Hierarchy object not found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	}
	else
	  hierobjid = pwr_cNObjid;

	sts = rtt_get_objects_hier_class_name( parent_ctx, hierobjid, 
		class, name, max_objects, global,
		&rtt_show_object_add, &menulist, 
		&index, 0, 0, 0);
	if ( sts == RTT__MAXCOUNT)
	  rtt_message('E',"To many object, all objects could not be shown");
	else if ( EVEN (sts)) return sts;

	if ( menulist != 0)
	{
	  sts = rtt_menu_bubblesort( menulist);
	  sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
			RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if (EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "No objects found");
	  return RTT__HOLDCOMMAND;
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_show_class_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	rtt context.
* char		*hiername	I	class hierarchy object name.
* char		*classname	I	system class name.
* char		*name		I	description of class name.
*
* Description:
*	This function is called when a 'show class' command is recieved.
*	All objects under the class hierarchy object, with the specified class
*	that fits in the name description is inserted in a menulist
*	and displayed on the screen.
*
**************************************************************************/

#if 0
static int	rtt_show_class_hier_class_name(
			menu_ctx	parent_ctx,
			char		*hiername,
			char		*classname,
			char		*name)
{
	int		sts;
	int		index = 0;
	pwr_tObjid	objid;
	pwr_tClassId	class;
	pwr_tObjid	class_objid;
	pwr_tObjid	hierobjid;
	rtt_t_menu	*menulist = 0;
	char		title[80] = "SEARCH LIST";
	int		max_objects = 300;
	char		full_classname[100];
	char		*s;

	
	if ( name != NULL)
	{
	  /* Check if name does not include a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Get objid for the object */
	    sts = rtt_find_name( parent_ctx, name, &objid);
/*	    sts = gdh_NameToObjid ( name, &objid);  */
	    if ( EVEN(sts))
	    {
     	      rtt_message('E',"Object does not exist");
	      return RTT__HOLDCOMMAND;
	    }
	    sts = rtt_object_parameters( parent_ctx, objid, 0,0,0,0);
	    return sts;
	  }
	  else
	    /* Convert name to upper case */
	    rtt_toupper( name, name);
	}

	/* Check if class */
	if ( classname != NULL )
	{
	  /* Get objid for the class */
	  strcpy( full_classname, "pwrs:Class-");
	  strcat( full_classname, classname);
	  sts = gdh_NameToObjid ( full_classname, &class_objid);
	  if (EVEN(sts))
	  {
	    rtt_message('E',"Unknown class");
	    return RTT__HOLDCOMMAND;
	  }
	  class = cdh_ClassObjidToId ( class_objid);
	}	
	else
	  class = 0;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  if ( *hiername == '\0')
	  {
	    /* No value is given, take the title as default */
	    sts = rtt_find_hierarchy( parent_ctx, &hierobjid);
	    if (EVEN(sts)) 
	    {
	      rtt_message('E', "No hierarchy found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    /* Get objid for the hierarchy object */
	    sts = gdh_NameToObjid ( hiername, &hierobjid);
	    if (EVEN(sts))
	    {
	      rtt_message('E',"Hierarchy object not found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	}
	else
	  hierobjid = pwr_cNObjid;

	sts = rtt_get_class_hier_class_name( parent_ctx, hierobjid, 
		class, name, max_objects, 
		&rtt_show_object_add, &menulist, 
		&index, 0, 0, 0);
	if ( sts == RTT__MAXCOUNT)
	  rtt_message('E',"To many object, all objects could not be shown");
	else if ( EVEN (sts)) return sts;

	if ( menulist != 0)
	{
	  sts = rtt_menu_bubblesort( menulist);
	  sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
			RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if (EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "No objects found");
	  return RTT__HOLDCOMMAND;
	}
	return RTT__SUCCESS;
}
#endif

/*************************************************************************
*
* Name:		rtt_show_par_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	rtt context.
* char		*parametername	I	name of parameter.
* char		*hiername	I	name of hierarchy object.
* char		*classname	I	name of class.
* char		*name		I	name description.
* int		add		I	if added to existing menulist or
*					createing a new menu.
*
* Description:
*	This function is called when a 'show parameter' command is recieved.
*	All object under the hierarchy object, with the specified class
*	that fits in the name description is inserted in a menulist
*	and displayed on the screen.
*
**************************************************************************/

static int	rtt_show_par_hier_class_name(
			menu_ctx	parent_ctx,
			char		*parametername,
			char		*hiername,
			char		*classname,
			char		*name,
			int		add,
			int		global,
			int		max_objects)
{
	char		parametername_str[80];
	char		name_str[80];
	int		sts;
	int		index = 0;
	pwr_tClassId	class;
	pwr_tObjid	hierobjid;
	rtt_t_menu_upd	*menulist = 0;
	char		title[80] = "SEARCH LIST";
	pwr_tObjid	objid;
	char		*s;
	int		single_object = 0;
	char		*t;
	char		elementstr[10];
	int		len;
	int		element;
	char		name_array[2][80];
	int		names;

	if ( max_objects == 0)
	  max_objects = 300;

	if ( (parametername == NULL) && (name != NULL))
	{
	  /* Parse the parameter name to get object name and
	   parameter name */
	   names = rtt_parse( name, ".", "",
		(char *) name_array, sizeof( name_array)/sizeof( name_array[0]), 
		sizeof( name_array[0]), 0);
	  if ( names != 2 )
	  {
	    rtt_message('E',"Name syntax error");
	    return RTT__NOPICTURE;
	  }
	  strncpy( name_str, name_array[0], sizeof(name_str));
	  strncpy( parametername_str, name_array[1], sizeof(parametername_str));
	  parametername = parametername_str;
	  name = name_str;
	}
	else if (parametername == NULL)
	{
	  rtt_message('E', "Enter parameter");
	  return RTT__HOLDCOMMAND;
	}

	if ( name != NULL)
	{
	  /* Check if name does not include a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Get objid for the object */
	    sts = rtt_find_name( parent_ctx, name, &objid);
/*	    sts = gdh_NameToObjid ( name, &objid);  */
	    if ( EVEN(sts))
	    {
     	      rtt_message('E',"Object does not exist");
	      return RTT__HOLDCOMMAND;
	    }
	    single_object = 1;
	  }
	  else
	    /* Convert name to upper case */
	    rtt_toupper( name, name);
	}

	/* Check if class */
	if ( classname != NULL )
	{
	  /* Get classid for the class */
	  sts = gdh_ClassNameToId ( classname, &class);
	  if ( EVEN(sts))
	  {
	    /* Class not found */
	    rtt_message('E',"Unknown class");
	    return RTT__HOLDCOMMAND;
	  }
	}	
	else
	  class = 0;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  if ( *hiername == '\0')
	  {
	    /* No value is given, take the title as default */
	    sts = rtt_find_hierarchy( parent_ctx, &hierobjid);
	    if (EVEN(sts)) 
	    {
	      rtt_message('E', "No hierarchy found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    /* Get objid for the hierarchy object */
	    sts = gdh_NameToObjid ( hiername, &hierobjid);
	    if (EVEN(sts))
	    {
	      rtt_message('E',"Hierarchy object not found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	}
	else
	  hierobjid = pwr_cNObjid;

	/* Check index in parameter */
	s = strchr( parametername, '[');
	if ( s == 0)
	  element = -1;
	else
	{
	  t = strchr( parametername, ']');
	  if ( t == 0)
	  {
	    rtt_message('E',"Syntax error in parameter name");
	    return RTT__HOLDCOMMAND;
	  }
	  else
	  {
	    len = t - s - 1;
	    strncpy( elementstr, s + 1, len);
	    elementstr[ len] = 0;
	    sscanf( elementstr, "%d", &element);
	    *s = '\0';
	    if ( (element < 0) || (element > 1000) )
	    {
	      rtt_message('E',"Syntax error in parameter name");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	}


	if ( add == RTT_MENU_ADD)
	{
	  rtt_get_menusize(parent_ctx, &index);
	  menulist = (rtt_t_menu_upd *) parent_ctx->menu;
	}

	if ( single_object)
	  sts = rtt_show_parameter_add( objid, &menulist, 
		parametername, &index, &element, 0);
	else
	  sts = rtt_get_objects_hier_class_name( parent_ctx, hierobjid, 
		class, name, max_objects, global,
		&rtt_show_parameter_add, (void *) &menulist, 
		(void *) parametername, (void *) &index, 
		(void *) &element, 0);
	if ( sts == RTT__MAXCOUNT)
	  rtt_message('E',"To many object, all objects could not be shown");
	else if ( EVEN (sts)) return sts;

	if ( add == RTT_MENU_ADD)	
	{
	  /* Reconfigure and redraw the menu */
	  parent_ctx->menu = (rtt_t_menu *) menulist;
	  sts = rtt_menu_upd_bubblesort( menulist);
	  rtt_menu_upd_configure( parent_ctx);
	}
	else if ( menulist != 0)
	{
	  sts = rtt_menu_upd_bubblesort( menulist);
	  sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if (EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "No objects found");
	  return RTT__HOLDCOMMAND;
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_debug_obj_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	rtt context.
* char		*hiername	I	name of hierarchy object.
* char		*classname	I	name of class.
* char		*name		I	name description.
* int		add		I	if add to existing menulist or
*					creating a new menu.
*
* Description:
*	This function is called when a 'debug object' command is recieved.
*	All object under the hierarchy object, with the specified class
*	that fits in the name description is inserted in a menulist
*	and displayed on the screen.
*
**************************************************************************/

static int	rtt_debug_obj_hier_class_name(
			menu_ctx	parent_ctx,
			char		*hiername,
			char		*classname,
			char		*name,
			int		add,
			int		global)
{
	int		sts;
	int		index = 0;
	pwr_tClassId	class;
	pwr_tObjid	hierobjid;
	rtt_t_menu_upd	*menulist = 0;
	char		title[80] = "SEARCH LIST";
	int		max_objects = 300;
	pwr_tObjid	objid;
	char		*s;
	int		single_object = 0;

	if ( name != NULL)
	{
	  /* Check if name does not include a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Get objid for the object */
	    sts = rtt_find_name( parent_ctx, name, &objid);
/*	    sts = gdh_NameToObjid ( name, &objid);  */
	    if ( EVEN(sts))
	    {
     	      rtt_message('E',"Object does not exist");
	      return RTT__HOLDCOMMAND;
	    }
	    single_object = 1;
	  }
	  else
	    /* Convert name to upper case */
	    rtt_toupper( name, name);
	}

	/* Check if class */
	if ( classname != NULL )
	{
	  /* Get classid for the class */
	  sts = gdh_ClassNameToId ( classname, &class);
	  if ( EVEN(sts))
	  {
	    /* Class not found */
	    rtt_message('E',"Unknown class");
	    return RTT__HOLDCOMMAND;
	  }
	}	
	else
	  class = 0;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  if ( *hiername == '\0')
	  {
	    /* No value is given, take the title as default */
	    sts = rtt_find_hierarchy( parent_ctx, &hierobjid);
	    if (EVEN(sts)) 
	    {
	      rtt_message('E', "No hierarchy found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else
	  {
	    /* Get objid for the hierarchy object */
	    sts = gdh_NameToObjid ( hiername, &hierobjid);
	    if (EVEN(sts))
	    {
	      rtt_message('E',"Hierarchy object not found");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	}
	else
	  hierobjid = pwr_cNObjid;

	if ( add == RTT_MENU_ADD)
	{
	  rtt_get_menusize(parent_ctx, &index);
	  menulist = (rtt_t_menu_upd *) parent_ctx->menu;
	}

	if ( single_object)
	  sts = rtt_debug_object_add( objid, &menulist, &index, &zero, 
			0, 0);
	else
	  sts = rtt_get_objects_hier_class_name( parent_ctx, hierobjid, 
		class, name, max_objects, global,
		&rtt_debug_object_add, (void *) &menulist, 
		(void *) &index, &zero,
		0, 0);
	if ( sts == RTT__MAXCOUNT)
	  rtt_message('E',"To many object, all objects could not be shown");
	else if ( EVEN (sts)) return sts;

	if ( add == RTT_MENU_ADD)	
	{
	  /* Reconfigure and redraw the menu */
	  parent_ctx->menu = (rtt_t_menu *) menulist;
	  sts = rtt_menu_upd_bubblesort( menulist);
	  rtt_menu_upd_configure( parent_ctx);
	}
	else if ( menulist != 0)
	{
	  sts = rtt_menu_upd_bubblesort( menulist);
	  sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if (EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "No objects found");
	  return RTT__HOLDCOMMAND;
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_collect_insert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	This function inserts an object in the rtt_collectmenulist.
*	If the current item is an object the debug parameter is inserted.
*	If the current item is a parameter in a 'show object' picture
*	this parameter is inserted.
*
**************************************************************************/

int	rtt_collect_insert(
			menu_ctx	ctx,
			char		*full_name)
{
	  int 		sts;
	  rtt_t_menu_upd	*menu_ptr_upd;
	  rtt_t_menu	*menu_ptr_stat;
	  char		objname[160];
	  int		index;
	  pwr_tObjid	objid;
	  char		parametername[32];
	  char		name_array[2][160];
	  int		names;
	  char		*s;

	/* Get next index in the collection picture */
	menu_ptr_upd = rtt_collectionmenulist;
	index = 0;
	if ( menu_ptr_upd != 0)
  	{
	  while ( menu_ptr_upd->text[0] != 0)
	  {
	    menu_ptr_upd++;
	    index++;
	  }
  	}

	/* Check parametername if there is one */
	if ( full_name != NULL)
	{
	  /* Check if parametername or only objektname */
	  s = strchr( full_name, '.');
	  if ( s == 0)
	  {
	    /* Only object name, debug object */
	    sts = gdh_NameToObjid ( full_name, &objid);
	    if (EVEN(sts))
	    {
	      rtt_message('E',"Object not found");
	      return RTT__HOLDCOMMAND;
	    }
	    sts = rtt_debug_object_add( objid, &rtt_collectionmenulist, 
		&index, &zero, 0, 0);
	    if ( (EVEN(sts)) || ( sts == RTT__ITEM_NOCREA))
	    {
	      rtt_message('E',"No debug on this item");
	      return RTT__NOPICTURE;
	    }
   	  }
	  else
	  {
	    /* Object and parameter in the full_name */
	    /* Parse the parameter name to get object name and
	     parameter name */
	     names = rtt_parse( full_name, ".", "",
		(char *) name_array, sizeof( name_array)/sizeof( name_array[0]), 
		sizeof( name_array[0]), 0);
	    if ( names != 2 )
	    {
	      rtt_message('E',"No debug on this item");
	      return RTT__NOPICTURE;
	    }
	    strncpy( objname, name_array[0], sizeof(objname));
	    strncpy( parametername, name_array[1], sizeof(parametername));

	    sts = gdh_NameToObjid ( objname, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E',"No debug on this item");
	      return RTT__NOPICTURE;
	    }

	    sts = rtt_show_parameter_add( objid, &rtt_collectionmenulist, 
		parametername, &index, &zero, 0);
	    if ( (EVEN(sts)) || ( sts == RTT__ITEM_NOCREA))
     	    {
	      rtt_message('E',"No debug on this item");
	      return RTT__NOPICTURE;
	    }
	  }
	}
	else
	{
	  /* Add current item to the rtt_collectionmenulist */
	  if ( ctx->menutype & RTT_MENUTYPE_UPD)
	  {
	    /* Get objid for the item, assume that it is in the first argument */
	    menu_ptr_upd = (rtt_t_menu_upd *) ctx->menu;
	    menu_ptr_upd += ctx->current_item;
	    objid = menu_ptr_upd->argoi;
	
	    /* Parse the parameter name for the item to get object name and
	       parameter name */
	    names = rtt_parse( menu_ptr_upd->parameter_name, ".", "",
		(char *) name_array, sizeof( name_array)/sizeof( name_array[0]), 
		sizeof( name_array[0]), 0);
	    if ( names == 2 )
	    {
	      strncpy( objname, name_array[0], sizeof(objname));
	      strncpy( parametername, name_array[1], sizeof(parametername));

	      sts = gdh_NameToObjid ( objname, &objid);
	      if ( EVEN(sts))
	      {
	        rtt_message('E',"No debug on this item");
	        return RTT__NOPICTURE;
	      }

	      sts = rtt_show_parameter_add( objid, &rtt_collectionmenulist, 
		parametername, &index, &zero, 0);
	      if ( (EVEN(sts)) || ( sts == RTT__ITEM_NOCREA))
	      {
	        rtt_message('E',"No debug on this item");
	        return RTT__NOPICTURE;
	      }
	    }
	    else
	    {
	      sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mNName);
	      if ( EVEN(sts))
	      {
	        rtt_message('E',"No debug on this item");
	        return RTT__NOPICTURE;
	      }
	      sts = rtt_debug_object_add( objid, &rtt_collectionmenulist, 
		&index, &zero, 0, 0);
	      if ( (EVEN(sts)) || ( sts == RTT__ITEM_NOCREA))
	      {
	        rtt_message('E',"No debug on this item");
	        return RTT__NOPICTURE;
	      }
	    }
	  }
	  else if ( ctx->menutype & RTT_MENUTYPE_MENU)
	  {
	    /* Get objid for the item, assume that it is in the first argument */
	    menu_ptr_stat = ctx->menu;
	    menu_ptr_stat += ctx->current_item;
	    objid = menu_ptr_stat->argoi;

	    sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mNName);
	    if ( EVEN(sts))
	    {
	      rtt_message('E',"No debug on this item");
	      return RTT__NOPICTURE;
	    }
	    sts = rtt_debug_object_add( objid, &rtt_collectionmenulist, 
		&index, &zero, 0, 0);
	    if ( (EVEN(sts)) || ( sts == RTT__ITEM_NOCREA))
	    {
	      rtt_message('E',"No debug on this item");
	      return RTT__NOPICTURE;
	    }
	  }
	  else
	  {
	    rtt_message('E',"No debug on this item");
	    return RTT__NOPICTURE;
	  }

	}
	rtt_message('I',"Object inserted");

	/* Update the menu in the context, if it exists */
	if ( rtt_collectionmenuctx != 0)
	{
	  rtt_collectionmenuctx->menu = (rtt_t_menu *) rtt_collectionmenulist;
	  sts = rtt_menu_upd_bubblesort( rtt_collectionmenulist);
	  rtt_menu_upd_configure( rtt_collectionmenuctx);
	}
	return RTT__NOPICTURE;
}	

/*************************************************************************
*
* Name:		rtt_get_child_object_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* pwr_tObjid	hierobjid	I	ancestor of wanted objects.
* pwr_tClassId	class		I	class of the wanted objects.
* char		*name		I	wildcard name of wanted objects.
* pwr_tObjid	objid		I	objid of the object
* int		(*backcall)()	I 	backcallroutine called for every object.
* void		*arg1		I	argument passed to the backcall routine.
* void		*arg2		I	argument passed to the backcall routine.
* void		*arg3		I	argument passed to the backcall routine.
* void		*arg4		I	argument passed to the backcall routine.
* void		*arg5		I	argument passed to the backcall routine.
*
* Description:
*	Routine used by rtt_get_objects_hier_class
*	to find all objects in a system of
*	a specified class that has a specific object as ancestor.
*	Calls a backcallroutine with the given arguments for every object 
*	found of the specified class below the specified hierarchy object.
*	This is  a recursiv functions that calls itself for all children
*	found to the given objdid.
*
**************************************************************************/

static int	rtt_get_child_object_hi_cl_na(
			menu_ctx	ctx,
			pwr_tClassId	class,
			char		*name,
			pwr_tObjid	objid,
			int		*obj_counter,
			int		max_count,
			int		global,
			int		(*backcall)(),
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4,
			void		*arg5)
{
	int			sts;
	pwr_tClassId		obj_class;
	pwr_tObjid		childobjid;
	int			name_ok, class_ok;
	char			obj_name[80];
	unsigned int		location;

	if ( !global)
	{
	  sts = gdh_GetObjectLocation( objid, &location);
	  if ( EVEN(sts)) return sts;
	  if ( !location)
	    return RTT__SUCCESS;
	}

	/* Check class if class is specified */
	class_ok = 1;
	if ( class != 0 )
	{
	  sts = gdh_GetObjectClass ( objid, &obj_class);
	  if ( EVEN(sts)) return sts;
	    
	  if ( obj_class != class)
	    class_ok = 0;
	}
	/* Check name if name is specified */
	name_ok = 1;
	if ( class_ok && (name != NULL) )
	{
	  /* Get the name of the object */
	  sts = gdh_ObjidToName ( objid, obj_name, sizeof(obj_name), 
		cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;

	  if ( rtt_wildcard( name, obj_name) != 0)
	    name_ok = 0;	
	}

	if ( class_ok && name_ok)
	{
	  sts = (backcall) (objid, arg1, arg2, arg3, arg4, arg5);
	  if ( EVEN(sts)) return sts;
	  if ( sts != RTT__ITEM_NOCREA )
	    (*obj_counter)++;
	  if ( *obj_counter >= max_count)
	    return RTT__MAXCOUNT;
 	}

	/* Get the first child to the object */
	sts = gdh_GetChild( objid, &childobjid);
	while ( ODD(sts) )
 	{
	  sts = rtt_get_child_object_hi_cl_na( ctx,  
		class, name, childobjid, obj_counter, max_count, global, 
		backcall, arg1, arg2, arg3, arg4, arg5);
	  if ( EVEN(sts)) return sts;
	  sts = gdh_GetNextSibling( childobjid, &childobjid);
	}

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_get_objects_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* pwr_tObjid	hierobjid	I	ancestor of wanted objects.
* pwr_tClassId	class		I	class of the wanded objects.
* char		*name		I	wildcard name of wanted objects.
* int		(*backcall)()	I 	backcallroutine called for every object.
* void		*arg1		I	argument passed to the backcall routine.
* void		*arg2		I	argument passed to the backcall routine.
* void		*arg3		I	argument passed to the backcall routine.
* void		*arg4		I	argument passed to the backcall routine.
* void		*arg5		I	argument passed to the backcall routine.
*
* Description:
*	Traverses the objects in the plant and nodehierarchy.
*	Calls a backcallroutine with the given arguments for every found 
*	object of the specified class that is found below the hierobjid in
*	the hierarchy and that  fits in a wildcard description. 
*	If hierobjid is eq 0 the hierarchy is not tested.
*	If class is eq 0 the class is not tested.
*	If name is eq NULL the name is not tested.
*	The objid of the found object and arguments will be 
*	passed to the backcallroutine. The backcallroutine should be
*	declared as:
*
*	int	'backcallroutine name'( objid, arg1, arg2, arg3, arg4, arg5)
*	pwr_tObjid	objid;
*	void	*arg1;
*	void	*arg2;
*	void	*arg3;
*	void	*arg4;
*	void	*arg5;
*	...
*	Calls a backcall routine for every object in the plathierarchy
*	of a specified class under a specific object in the hierarchy.
*
**************************************************************************/

int	rtt_get_objects_hier_class_name(
			menu_ctx	ctx,
			pwr_tObjid	hierobjid,
			pwr_tClassId	class,
			char		*name,
			int		max_count,
			int		global,
			int		(*backcall)(),
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4,
			void		*arg5)
{
	int			sts;
	pwr_tObjid		objid;
	pwr_tClassId		obj_class;
	int			obj_counter;
	char			hiername[80];
	unsigned int		location;

	obj_counter = 0;
	if ( cdh_ObjidIsNotNull( hierobjid))
	{
	  /* Check if the children */
	  sts = rtt_get_child_object_hi_cl_na( ctx, 
		class, name, hierobjid, &obj_counter, max_count, global,
		backcall, arg1, arg2, arg3, arg4, arg5);
	  if ( EVEN(sts)) return sts;
	}
	else
	{
	  /* Get all objects */
	  sts = gdh_GetRootList( &objid);
	  while ( ODD(sts) )
	  {
	    if ( !global)
	    {
	      sts = gdh_GetObjectLocation( objid, &location);
	      if ( EVEN(sts)) return sts;
	      if ( !location)
	      {
	        sts = gdh_GetNextSibling ( objid, &objid);
	        continue;
	      }
	    }

	    sts = gdh_GetObjectClass( objid, &obj_class);
	    if ( EVEN(sts)) return sts;
	    sts = gdh_ObjidToName ( cdh_ClassIdToObjid(obj_class), hiername,
			sizeof(hiername), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;

	    /* Check that the class of the node object is correct */
	    if ( (strcmp( hiername, "pwrs:Class-$NodeHier") == 0) ||
	       (strcmp( hiername, "pwrs:Class-$PlantHier") == 0) ||
	       (strcmp( hiername, "pwrs:Class-$System") == 0))
	    {
	      /* Check if the children */
	      sts = rtt_get_child_object_hi_cl_na( ctx, 
		class, name, objid, &obj_counter, max_count, global,
		backcall, arg1, arg2, arg3, arg4, arg5);
	      if ( EVEN(sts)) return sts;
	    }
	    sts = gdh_GetNextSibling ( objid, &objid);
	  }
	}
	return RTT__SUCCESS;
}



/*************************************************************************
*
* Name:		trv_get_class_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* pwr_tObjid	hierobjid	I	ancestor of wanted objects.
* pwr_tClassId	class		I	class of the wanded objects.
* char		*name		I	wildcard name of wanted objects.
* int		(*backcall)()	I 	backcallroutine called for every object.
* void		*arg1		I	argument passed to the backcall routine.
* void		*arg2		I	argument passed to the backcall routine.
* void		*arg3		I	argument passed to the backcall routine.
* void		*arg4		I	argument passed to the backcall routine.
* void		*arg5		I	argument passed to the backcall routine.
*
* Description:
*	Traverses the objects in the class hierarchy.
*	Calls a backcallroutine with the given arguments for every found 
*	object of the specified class that is found below the hierobjid in
*	the hierarchy and that  fits in a wildcard description. 
*	If hierobjid is eq 0 the hierarchy is not tested.
*	If class is eq 0 the class is not tested.
*	If name is eq NULL the name is not tested.
*	The objid of the found object and arguments will be 
*	passed to the backcallroutine. The backcallroutine should be
*	declared as:
*
*	int	'backcallroutine name'( objid, arg1, arg2, arg3, arg4, arg5)
*	pwr_tObjid	objid;
*	void		*arg1;
*	void		*arg2;
*	void		*arg3;
*	void		*arg4;
*	void		*arg5;
*	...
*	Calls a backcall routine for every object in the plathierarchy
*	of a specified class under a specific object in the hierarchy.
*
**************************************************************************/

#if 0
static int	rtt_get_class_hier_class_name(
			menu_ctx	ctx,
			pwr_tObjid	hierobjid,
			pwr_tClassId	class,
			char		*name,
			int		max_count,
			int		(*backcall)(),
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4,
			void		*arg5)
{
	int			sts;
	pwr_tObjid		objid;
	pwr_tClassId		obj_class;
	int			obj_counter;
	char			hiername[80];

	obj_counter = 0;
	if ( cdh_ObjidIsNotNull( hierobjid))
	{
	  /* Check if the children */
	  sts = rtt_get_child_object_hi_cl_na( ctx, 
		class, name, hierobjid, &obj_counter, max_count, 0,
		backcall, arg1, arg2, arg3, arg4, arg5);
	  if ( EVEN(sts)) return sts;
	}
	else
	{
	  /* Get all objects */
	  sts = gdh_GetRootList( &objid);
	  while ( ODD(sts) )
	  {
	    sts = gdh_GetObjectClass ( objid, &obj_class);
	    if ( EVEN(sts)) return sts;

	    sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
			sizeof(hiername), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;

	    /* Check that the class of the node object is correct */
	    if ( strcmp( hiername, "pwrs:Class-$ClassHier") == 0)
	    {
	      /* Check if the children */
	      sts = rtt_get_child_object_hi_cl_na( ctx, 
		class, name, objid, &obj_counter, max_count, 0,
		backcall, arg1, arg2, arg3, arg4, arg5);
	      if ( EVEN(sts)) return sts;
	    }
	    sts = gdh_GetNextSibling ( objid, &objid);
	  }
	}
	return RTT__SUCCESS;
}
#endif

/*************************************************************************
*
* Name:		rtt_find_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* char		*name		I	name string.
* pwr_tObjid	*objid		O	name objid.
*
* Description:
*	Returns the objid of a namestring.
*	If the name is not found the the routine adds the title
*	that used to be the hierarchy to the name. 
*	If it is not found on this level, higher levels of the
*	hierarchy is serarched.
*
**************************************************************************/

static int	rtt_find_name(
			menu_ctx	ctx,
			char		*name,
			pwr_tObjid	*objid)
{
	char	title[100];
	char	*s;
	int	sts;

	sts = gdh_NameToObjid ( name, objid);
	if ( ODD(sts))
	  return RTT__SUCCESS;

	while ( (EVEN(sts)) && ( ctx != 0))
	{
	  /* Examine the first part of the title */
	  strcpy( title, ctx->title);
	  s = strchr( title, ' ');
	  if ( s != 0)
	    *s = 0;
	  strcat( title, "-");
	  strcat( title, name);
	  sts = gdh_NameToObjid ( title, objid);
	  ctx = ctx->parent_ctx;
	}
	return sts;
}


/*************************************************************************
*
* Name:		rtt_find_hierarchy()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* pwr_tObjid	*objid		O	objid of object.
*
* Description:
*	Returns the objid of a namestring.
*	If the name is not found the the routine adds the title
*	that used to be the hierarchy to the name. 
*	If it is not found on this level, higher levels of the
*	hierarchy is searched.
*
**************************************************************************/

static int	rtt_find_hierarchy(
			menu_ctx	ctx,
			pwr_tObjid	*objid)
{
	char	title[100];
	char	*s;
	int	sts = 0;

	while ( (EVEN(sts)) && ( ctx != 0))
	{
	  /* Examine the first part of the title */
	  strcpy( title, ctx->title);
	  s = strchr( title, ' ');
	  if ( s != 0)
	    *s = 0;
	  sts = gdh_NameToObjid ( title, objid);
	  ctx = ctx->parent_ctx;
	}
	return sts;
}


/*************************************************************************
*
* Name:		rtt_menu_classort()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	*menulist	I	menulist.
* int		redo		I	do or redo.
*
* Description:
*	This function modifies the menulist so bubblesort sort in class also.
*
**************************************************************************/

int	rtt_menu_classort(
			rtt_t_menu	*menulist,
			int		redo)
{
	int		sts;
	rtt_t_menu	*menu_ptr;
	pwr_tClassId	class;
	char		classname[80];
	char		dummytxt[120];

	/* Get the size of the menu */
	menu_ptr = menulist;
	while ( menu_ptr->text[0] != 0)
	{
	  /* Objid is in arg1 */
	  sts = gdh_GetObjectClass( menu_ptr->argoi, &class);
	  if ( EVEN(sts)) return sts;
	  sts = gdh_ObjidToName( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;

	  if (strcmp( classname, "pwrs:Class-$PlantHier") == 0)
	  {
	    if ( !redo)
	    {
	      strcpy( dummytxt, menu_ptr->text);
	      strcpy( menu_ptr->text, "000");
	      strcat( menu_ptr->text, dummytxt);
	    }
	    else
	    {
	      strcpy( menu_ptr->text, &menu_ptr->text[3]);
	    }
	  }
	  else if (strcmp( classname, "pwrb:Class-PlcPgm") == 0)
	  {
	    if ( !redo)
	    {
	      strcpy( dummytxt, menu_ptr->text);
	      strcpy( menu_ptr->text, "001");
	      strcat( menu_ptr->text, dummytxt);
	    }
	    else
	    {
	      strcpy( menu_ptr->text, &(menu_ptr->text[3]));
	    }
	  }	
	  menu_ptr++;
	}

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_menu_bubblesort()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	*menulist	I	menulist.
*
* Description:
*	This function sorts the items in a nondynamic menulist in 
*	characther order.
*
**************************************************************************/

#if 0  /* qsort bug in ELN and VMS */
static int rtt_menu_sort_compare( const void *p1, const void *p2)
{
	char	*s1;
	char	*s2;

	for ( s1 = ( ((rtt_t_menu *) p1)->text), 
	      s2 = ( ((rtt_t_menu *) p2)->text); 
	      *s1 != 0; s1++)
	{
	   if  ( r_toupper(*s1) != r_toupper(*s2)) break;
	   s2++;
	}
	return r_toupper(*s1) - r_toupper(*s2);
}
#endif

int	rtt_menu_bubblesort(
			rtt_t_menu	*menulist)
{
	int	i, j, size;
	char	*str1, *str2;
	rtt_t_menu	dum;
	rtt_t_menu	*menu_ptr;

	/* Get the size of the menu */
	menu_ptr = menulist;
	size = 0;
	while ( menu_ptr->text[0] != 0)
	{
	  menu_ptr++;
	  size++;
	}
#if 0
	qsort( menulist, size, sizeof( rtt_t_menu), rtt_menu_sort_compare);
#endif
	for ( i = size - 1; i > 0; i--)
	{
	  menu_ptr = menulist;
	  for ( j = 0; j < i; j++)
	  {
	    str1 = menu_ptr->text;
	    str2 = (menu_ptr + 1)->text;
	    if ( *str2 == 0)
	      break;
	    if ( strcmp( str1, str2) > 0)
	    {
	      memcpy( &dum, menu_ptr + 1, sizeof( rtt_t_menu));
	      memcpy( menu_ptr + 1, menu_ptr, sizeof( rtt_t_menu));
	      memcpy( menu_ptr, &dum, sizeof( rtt_t_menu));
	    }
	    menu_ptr++;
	  }
	}

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_menu_upd_bubblesort()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu_upd *menulist	I	dynamic menulist.
*
* Description:
*	This function sorts the items in a dynamic menulist in 
*	characther order.
*
**************************************************************************/

#if 0  /* qsort bug in ELN and VMS */
static int rtt_menu_upd_sort_compare( const void *p1, const void *p2)
{
	char	*s1;
	char	*s2;

	for ( s1 = ((rtt_t_menu_upd *) p1)->text, 
	      s2 = ((rtt_t_menu_upd *) p2)->text; 
	      *s1 != 0; s1++)
	{
	   if  ( r_toupper(*s1) != r_toupper(*s2)) break;
	   s2++;
	}
	return r_toupper(*s1) - r_toupper(*s2);
}
#endif
int	rtt_menu_upd_bubblesort(
			rtt_t_menu_upd	*menulist)
{
	int	i, j, size;
	char	*str1, *str2;
	rtt_t_menu_upd	dum;
	rtt_t_menu_upd	*menu_ptr;

	/* Get the size of the menu */
	menu_ptr = menulist;
	size = 0;
	while ( menu_ptr->text[0] != 0)
	{
	  menu_ptr++;
	  size++;
	}
#if 0
	qsort( menulist, size, sizeof( rtt_t_menu_upd), rtt_menu_upd_sort_compare);
#endif
	for ( i = size - 1; i > 0; i--)
	{
	  menu_ptr = menulist;
	  for ( j = 0; j < i; j++)
	  {
	    str1 = menu_ptr->text;
	    str2 = (menu_ptr + 1)->text;
	    if ( *str2 == 0)
	      break;
	    if ( strcmp( str1, str2) > 0)
	    {
	      memcpy( &dum, menu_ptr + 1, sizeof( rtt_t_menu_upd));
	      memcpy( menu_ptr + 1, menu_ptr, sizeof( rtt_t_menu_upd));
	      memcpy( menu_ptr, &dum, sizeof( rtt_t_menu_upd));
	    }
	    menu_ptr++;
	  }
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_collect_show()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Displays the collection picture.
*
**************************************************************************/

int	rtt_collect_show(
			menu_ctx	ctx)
{
	  char	title[] = "COLLECTION PICTURE";
	  int	sts;

/*	  if ( rtt_collectionmenuctx != 0)
	    return RTT__BACKTOCOLLECT;
*/
	  if ( ctx == rtt_collectionmenuctx )
	    return RTT__NOPICTURE;

	  if ( rtt_collectionmenulist != 0)
	  {
	    sts = rtt_menu_upd_bubblesort( rtt_collectionmenulist);
	    sts = rtt_menu_upd_new( ctx, pwr_cNObjid, &rtt_collectionmenulist,
		title, 0, RTT_MENUTYPE_DYN);
	    if ( sts == RTT__FASTBACK) return sts;
	    else if (EVEN(sts)) return sts;
	  }
	  else
	  {
	    rtt_message('E', "No objects found in collection picture");
	    return RTT__NOPICTURE;
	  }

	  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menulist_search()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Search for a string in the menu items text.
*
**************************************************************************/

static int	rtt_menulist_search(
			menu_ctx	ctx,
			char		*searchstr,
			int		index,
			int		*foundindex)
{
	char 	*menu_ptr;	
	int	size;
	int	i;
	char	text[120];

	if ( ctx->menutype & RTT_MENUTYPE_MENU )
	  size = sizeof(rtt_t_menu);
	else if ( ctx->menutype & RTT_MENUTYPE_UPD)
	  size = sizeof(rtt_t_menu_upd);
	
	menu_ptr = (char *) ctx->menu;
	menu_ptr += index * size;
	while ( ((rtt_t_menu *) menu_ptr)->text[0] != 0)
	{
	  /* Convert menu text to upper case (no case sensitive) */
	  rtt_toupper( text, ((rtt_t_menu *) menu_ptr)->text);
	  	
	  if ( strlen( text) >= strlen( searchstr))
	  {
	    for ( i = 0; i <= (int)(strlen( text) - strlen( searchstr)); i++)
	    {
	      if ( strncmp( &text[i], searchstr, strlen(searchstr)) == 0)
	      {
	        /* the string matches */
	        *foundindex = index;
	        return 1;
	      }
	    }
	  }
	  menu_ptr += size;
	  index++;
	}
	return 0;
}


/*************************************************************************
*
* Name:		rtt_commandmode_start()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Start the commandmode. Open the commanfile and put the content
*	in a command array.
*	Set the global variable rtt_commandmode to RTT_COMMANDMODE_FILE.
*	Command from now on will be fetched from the command array.
*
**************************************************************************/

static int	rtt_deffilename_func( char *outfile, char *infile, void *client_data)
{
	rtt_get_defaultfilename( infile, outfile, ".rtt_com");
        return 1;
}

static int rtt_getinput_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  ccm_s_arg 		*arg_p, *arg_p2, *arg_p3; 
  int			maxlen = 79;
  char			input_str[80];
  int			sts;
  unsigned long		option;
  unsigned long		terminator;
  menu_ctx		ctx;
  rtt_t_menu		*menu_ptr;
  int			(* timeout_func) ();
  char			format[80];
  char			prompt[80];

  option = RTT_OPT_TIMEOUT | RTT_OPT_NOSCROLL;

  if ( arg_count < 2 || arg_count > 4)
    return CCM__ARGMISM;
  arg_p = arg_list->next;
  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__VARTYPE;
  if ( arg_count > 2)
  {
    arg_p2 = arg_p->next;
    if ( arg_p2->value_decl != CCM_DECL_STRING)
      return CCM__VARTYPE;
    strcpy( prompt, arg_p2->value_string);
  }
  else
    strcpy( prompt, "");

  if ( arg_count > 3)
  {
    arg_p3 = arg_p2->next;
    if ( arg_p3->value_decl != CCM_DECL_INT)
      return CCM__VARTYPE;
  }
  strcpy( format, arg_list->value_string);

  ctx = rtt_current_ctx();
  switch ( ctx->ctx_type)
  {
    case RTT_CTXTYPE_MENU:
      if ( ctx->menu == 0)
        timeout_func = rtt_scan;
      else
      {
        menu_ptr =  ctx->menu;
        if ( ctx->menutype & RTT_MENUTYPE_EDIT)
          timeout_func = rtt_menu_edit_update;
        else if ( ctx->menutype & RTT_MENUTYPE_UPD)
          timeout_func = rtt_menu_upd_update;
        else
          timeout_func = rtt_scan;
      }
      break;
    default:
      timeout_func = rtt_scan;
  }

  sts = rtt_get_input_string( (char *) &rtt_chn, input_str, &terminator,
		maxlen, rtt_value_recallbuff, option, rtt_scantime, 
		timeout_func, (void *) ctx, 
		prompt);
  if ( arg_p->value_decl == CCM_DECL_FLOAT)
    sts = sscanf( input_str, format, &arg_p->value_float);
  else if ( arg_p->value_decl == CCM_DECL_INT)
    sts = sscanf( input_str, format, &arg_p->value_int);
  else if ( arg_p->value_decl == CCM_DECL_STRING)
    sts = sscanf( input_str, format, arg_p->value_string);
  arg_p->value_returned = 1;
  arg_p->var_decl = arg_p->value_decl;
  if ( arg_count > 3)
  {
    arg_p3->value_int = terminator;
    arg_p3->value_returned = 1;
    arg_p3->var_decl = arg_p3->value_decl;
  }
  *return_decl = CCM_DECL_INT;
  *return_int = sts;
  return 1;
}

static int rtt_placecursor_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  ccm_s_arg 	*arg_p2; 

  if ( arg_count != 2)
    return CCM__ARGMISM;
  arg_p2 = arg_list->next;

  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;
  if ( arg_p2->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  rtt_cursor_abs( arg_list->value_int, 
	arg_p2->value_int);
  r_print_buffer();

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int rtt_lineerase_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 0)
    return CCM__ARGMISM;

  rtt_eofline_erase();
  r_print_buffer();

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int rtt_getcurrenttitle_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  menu_ctx	ctx;

  if ( arg_count != 0)
    return CCM__ARGMISM;

  ctx = rtt_current_ctx();
  switch ( ctx->ctx_type)
  {
    case RTT_CTXTYPE_MENU:
      strcpy( return_string, ctx->title);
      break;
    case RTT_CTXTYPE_VIEW:
      strcpy( return_string, ((view_ctx)ctx)->title);
      break;
    default:
      strcpy( return_string, "");
  }
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getcurrenttext_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  menu_ctx		ctx;
  rtt_t_menu		*menu_ptr;
  rtt_t_menu_upd 	*menu_upd_ptr;
  rtt_t_menu_alarm 	*menu_alarm_ptr;

  if ( arg_count != 0)
    return CCM__ARGMISM;

  ctx = rtt_current_ctx();
  switch ( ctx->ctx_type)
  {
    case RTT_CTXTYPE_MENU:
      if ( ctx->menu == 0)
      {
        strcpy( return_string, "");
        break;
      }
      menu_ptr =  ctx->menu;
      if ( ctx->menutype & RTT_MENUTYPE_UPD)
      {
        menu_upd_ptr = (rtt_t_menu_upd *) menu_ptr; 
	menu_upd_ptr += ctx->current_item;
        strcpy( return_string, menu_upd_ptr->text);
      }
      else if ( ctx->menutype & RTT_MENUTYPE_ALARM)
      {
        menu_alarm_ptr = (rtt_t_menu_alarm *) menu_ptr;
	menu_alarm_ptr += ctx->current_item;
        strcpy( return_string, menu_alarm_ptr->text);
      }
      else
      {
	menu_ptr += ctx->current_item;
        strcpy( return_string, menu_ptr->text);
      }
      break;
    default:
      strcpy( return_string, "");
  }
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getcurrentobject_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  menu_ctx		ctx;
  pwr_tObjid		objid;
  char			name_str[80];
  int			sts;

  if ( arg_count != 0)
    return CCM__ARGMISM;

  ctx = rtt_current_ctx();
  switch ( ctx->ctx_type)
  {
    case RTT_CTXTYPE_MENU:
      sts = rtt_get_current_object( ctx, &objid, 
		name_str, sizeof( name_str), cdh_mName_volumeStrict);
      if ( EVEN(sts))
        strcpy( return_string, "");
      else
        strcpy( return_string, name_str);
      break;
    default:
      strcpy( return_string, "");
  }
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getchild_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  int		sts;
  char		name[80];
  pwr_tObjid	parent_objid;
  pwr_tObjid	child_objid;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  sts = gdh_NameToObjid( arg_list->value_string, &parent_objid);
  if ( ODD(sts))
  {
    sts = gdh_GetChild( parent_objid, &child_objid);
    if (ODD(sts))
      sts = gdh_ObjidToName( child_objid, name, sizeof(name), cdh_mNName);
  }
  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getparent_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  int		sts;
  char		name[80];
  pwr_tObjid	parent_objid;
  pwr_tObjid	child_objid;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  sts = gdh_NameToObjid( arg_list->value_string, &child_objid);
  if ( ODD(sts))
  {
    sts = gdh_GetParent( child_objid, &parent_objid);
    if (ODD(sts))
      sts = gdh_ObjidToName( parent_objid, name, sizeof(name), cdh_mNName);
  }
  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getnextsibling_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  int		sts;
  char		name[80];
  pwr_tObjid	objid;
  pwr_tObjid	next_objid;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  sts = gdh_NameToObjid( arg_list->value_string, &objid);
  if ( ODD(sts))
  {
    sts = gdh_GetNextSibling( objid, &next_objid);
    if (ODD(sts))
      sts = gdh_ObjidToName( next_objid, name, sizeof(name), cdh_mNName);
  }
  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getclasslist_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  int		sts;
  char		name[80];
  pwr_tClassId	class;
  pwr_tObjid	objid;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  sts = gdh_ClassNameToId ( arg_list->value_string, &class);
  if ( ODD(sts))
  {
    sts = gdh_GetClassList ( class, &objid);
    if (ODD(sts))
      sts = gdh_ObjidToName( objid, name, sizeof(name), cdh_mNName);
  }
  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getrootlist_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  int		sts;
  char		name[80];
  pwr_tObjid	objid;

  if ( arg_count != 0)
    return CCM__ARGMISM;

  sts = gdh_GetRootList( &objid);
  if (ODD(sts))
    sts = gdh_ObjidToName( objid, name, sizeof(name), cdh_mNName);

  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getnodeobject_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  int		sts;
  char		name[80];
  pwr_tObjid	objid;

  if ( arg_count != 0)
    return CCM__ARGMISM;

  sts = gdh_GetNodeObject( 0, &objid);
  if (ODD(sts))
    sts = gdh_ObjidToName( objid, name, sizeof(name), cdh_mNName);

  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_getnextobject_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  int		sts;
  char		name[80];
  pwr_tObjid	objid;
  pwr_tObjid	next_objid;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  sts = gdh_NameToObjid( arg_list->value_string, &objid);
  if ( ODD(sts))
  {
    sts = gdh_GetNextObject( objid, &next_objid);
    if (ODD(sts))
      sts = gdh_ObjidToName( next_objid, name, sizeof(name), cdh_mNName);
  }
  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}


static int rtt_getobjectclass_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  int		sts;
  char		name[80];
  pwr_tObjid	objid;

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  sts = gdh_NameToObjid( arg_list->value_string, &objid);
  if ( ODD(sts))
    sts = rtt_objidtoclassname( objid, name);

  if ( ODD(sts))
    strcpy( return_string, name);
  else
    strcpy( return_string, "");
  *return_decl = CCM_DECL_STRING;
  
  return 1;
}

static int rtt_messageerror_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  rtt_message('E', arg_list->value_string);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int rtt_messageinfo_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;

  rtt_message('I', arg_list->value_string);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

static int rtt_cutobjectname_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  ccm_s_arg 	*arg_p2; 

  if ( arg_count != 2)
    return CCM__ARGMISM;
  arg_p2 = arg_list->next;

  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;
  if ( arg_p2->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  rtt_cut_segments( return_string, arg_list->value_string, 
	arg_p2->value_int);

  *return_decl = CCM_DECL_STRING;
  
  return 1;
}


static int rtt_getattribute_func( 
  void *filectx,
  ccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  ccm_s_arg	*arg_p2;
  int	sts;
  int	value_decl;
  int	value_int;
  float	value_float;
  char	value_string[80];

  if ( !(arg_count == 2 || arg_count == 1))
    return CCM__ARGMISM;

  arg_p2 = arg_list->next;
  if ( arg_list->value_decl != CCM_DECL_STRING)
    return CCM__ARGMISM;
  if ( arg_count == 2 && arg_p2->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  sts = rtt_attribute_func( arg_list->value_string, &value_decl,
	&value_float, &value_int, value_string);
  if ( EVEN(sts))
  {
    if ( arg_count == 2)
    {
      arg_p2->value_int = 0;
      arg_p2->value_returned = 1;
      arg_p2->var_decl = arg_p2->value_decl;
    }
    *return_decl = CCM_DECL_UNKNOWN;
  }
  else
  {
    if ( value_decl == CCM_DECL_INT)
    {
      *return_int = value_int;
      *return_decl = CCM_DECL_INT;
    }
    else if ( value_decl == CCM_DECL_FLOAT)
    {
      *return_float = value_float;
      *return_decl = CCM_DECL_FLOAT;
    }
    else if ( value_decl == CCM_DECL_STRING)
    {
      strcpy( return_string, value_string);
      *return_decl = CCM_DECL_STRING;
    }
    if ( arg_count == 2)
    {
      arg_p2->value_int = 1;
      arg_p2->value_returned = 1;
      arg_p2->var_decl = arg_p2->value_decl;
    }
  }

  return 1;
}

static int	rtt_externcmd_func( char *cmd, void *client_data)
{
	int		sts;
	menu_ctx	ctx;

	/* Create a dummy menu context */
	sts = rtt_menu_create_ctx( &ctx, NULL, NULL, 
			"Dummy", RTT_MENUTYPE_UPD);
	
	sts = rtt_menu_command( ctx, pwr_cNObjid, cmd, 0, 0, 0);
	free( (char *) ctx);
	return sts;
}

static int	rtt_errormessage_func( char *msg, int severity, void *client_data)
{

	if ( EVEN(severity))
	  rtt_message('E', msg);
	else
	  rtt_message('I', msg);
	return 1;
}

int	rtt_commandmode_start(
			char			*incommand,
			int			quit)
{
	int	appl_sts;
	int	sts;
	
	if ( rtt_commandmode & RTT_COMMANDMODE_SINGLE)
	  rtt_ccmcmd_quit = 0;

	if ( !rtt_ccm_func_registred)
	{
	  sts = ccm_register_function( "GetInput", rtt_getinput_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "PlaceCursor", rtt_placecursor_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "LineErase", rtt_lineerase_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetCurrentTitle", rtt_getcurrenttitle_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetCurrentText", rtt_getcurrenttext_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetCurrentObject", rtt_getcurrentobject_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetChild", rtt_getchild_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetParent", rtt_getparent_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetNextSibling", rtt_getnextsibling_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetClassList", rtt_getclasslist_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetRootList", rtt_getrootlist_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetNodeObject", rtt_getnodeobject_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetNextObject", rtt_getnextobject_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetObjectClass", rtt_getobjectclass_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "MessageError", rtt_messageerror_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "MessageInfo", rtt_messageinfo_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "CutObjectName", rtt_cutobjectname_func);
	  if ( EVEN(sts)) return sts;
	  sts = ccm_register_function( "GetAttribute", rtt_getattribute_func);
	  if ( EVEN(sts)) return sts;
	  rtt_ccm_func_registred = 1;
	}

	sts = ccm_file_exec( incommand, rtt_externcmd_func,
		rtt_deffilename_func, rtt_errormessage_func , &appl_sts, 
		rtt_verify, 1, &rtt_ccmctx, 1, 
		0, rtt_ccmcmd, NULL);
	if ( sts == CCM__EXTERNFUNC)
	{
	  rtt_command_toupper( rtt_ccmcmd, rtt_ccmcmd);
	  rtt_ccmcmd_fetched = 1;
          if ( quit)
	    rtt_ccmcmd_quit = 1;
	  rtt_commandmode |= RTT_COMMANDMODE_FILE;
	  rtt_quiet = RTT_QUIET_ALL;
	}
	else
	{
	  if ( quit ||
	       rtt_commandmode & RTT_COMMANDMODE_SINGLE)
	    rtt_exit_now(0, RTT__SUCCESS);
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_commandmode_single()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Start the commandmode when a single command is given as an argument
*	at start. Put the commandstring in a command array and terminates
*	the commandarray with 'quit' to force an exit.
*	Set the global variable rtt_commandmode to RTT_COMMANDMODE_FILE.
*	Command from now on will be fetched from the command array.
*
**************************************************************************/

int	rtt_commandmode_single(
			char			*command)
{
	strcpy( rtt_ccmcmd, command);
	rtt_ccmcmd_fetched = 1;
	rtt_ccmcmd_quit = 1;
	rtt_commandmode |= RTT_COMMANDMODE_FILE;
	rtt_commandmode |= RTT_COMMANDMODE_SINGLE;
	rtt_quiet = RTT_QUIET_MESSAGE | RTT_QUIET_ALL;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_commandmode_getnext()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get next command.
*
**************************************************************************/

int	rtt_commandmode_getnext(
			char		*command,
			unsigned long	*terminator)
{
	int	sts;
	int	appl_sts;

	if ( !(rtt_commandmode & RTT_COMMANDMODE_FILE))
	{
	  /* Wrong mode */
	  *terminator = RTT_K_PF1;
	  return RTT__EOC;
	}

	for (;;)
	{
	  if ( rtt_ccmcmd_fetched)
	  {
	    /* A command was return att init of ccm */
	    rtt_ccmcmd_fetched = 0;
	    sts = CCM__EXTERNFUNC;
	  }
	  else
	  {
	    if ( rtt_ccmcmd_quit)
	      rtt_exit_now(0, RTT__SUCCESS);

	    sts = ccm_file_exec( NULL, NULL,
		NULL, NULL, &appl_sts, rtt_verify, 1, &rtt_ccmctx, 1,
		1, rtt_ccmcmd, NULL);
	    rtt_command_toupper( rtt_ccmcmd, rtt_ccmcmd);
	  }
	  if ( sts == CCM__EXTERNFUNC)
	  {
	    /* Check if som intern learn commands */
	    if ( strcmp( rtt_ccmcmd, "NOQUIET") == 0)
	    {
	      r_print_buffer();
	      rtt_quiet = 0;
	    }
	    else
	      break;
	  }
	  else
	    break;
	}
	if ( sts == CCM__EXTERNFUNC)
	{
	  strcpy( command, rtt_ccmcmd);
	  *terminator = RTT_K_RETURN;
	  return RTT__SUCCESS;
	}
	else
	{
	  rtt_commandmode &= ~RTT_COMMANDMODE_FILE;
	  rtt_message_off = 0;
	  if ( rtt_ccmcmd_quit)
	    rtt_exit_now(0, RTT__SUCCESS);
	    
	  if ( rtt_quiet != 0)
	  {
	    /* Clear the output buffer and simulate a redraw command */
	    r_print_buffer();
	    rtt_quiet = 0;
	    if ( rtt_noredraw)
	    {
	      rtt_noredraw = 0;
	      *terminator = RTT_K_PF1;
	      return RTT__EOC;
	    }
	    else
	    {
	      strcpy( command, "K_CTRLW");
	      *terminator = RTT_K_RETURN;
	      return RTT__SUCCESS;
  	    }
	  }
	  else
	  {
	    /* No more commands */
	    *terminator = RTT_K_PF1;
	    return RTT__EOC;
	  }
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_commandmode_rewind()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Decrement the rtt_commandmode_counter.
*
**************************************************************************/

int	rtt_commandmode_rewind( char *command)
{
	if ( rtt_commandmode & RTT_COMMANDMODE_FILE)
	{
	  strcpy( rtt_ccmcmd, command);
	  rtt_ccmcmd_fetched = 1;
	}
	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_read_line()
*
* Type		void
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Read a line for a file.
*
**************************************************************************/

int rtt_read_line(
			char	*line,
			int	maxsize,
			FILE	*file)
{ 
	char	*s;

	if (fgets( line, maxsize, file) == NULL)
	  return 0;
	line[maxsize-1] = 0;
	s = strchr( line, 10);
	if ( s != 0)
	  *s = 0;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_command_get_input_string()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*chn		I	channel.
* char		*out_string	O	input string
* unsigned long	*out_terminator	O	terminator
* int		out_maxlen	I	max charachters.
* unsigned long	recall		I	recall buffer.
* unsigned long	option		I	option mask.
* int		timeout		I	timeout time
* int		(* timeout_func) () I	timeout function
* void		*timeout_arg	I	timeout function argument
* char		*prompt		I	prompt string.
*
* Description:
*	Read a input string.
*
**************************************************************************/

int	rtt_command_get_input_string(
			char		*chn,
			char		*out_string,
			unsigned long	*out_terminator,
			int		out_maxlen,
			rtt_t_recall	*recall,
			unsigned long	option,
			int		timeout,
			int		(* timeout_func) (),
			void		*timeout_arg,
			char		*prompt,
			int		function)
{
	int		sts;
	unsigned long	terminator;
	char		command[160];

	if ( rtt_commandmode & RTT_COMMANDMODE_FILE)
	{
	
	  sts = rtt_commandmode_getnext( command, &terminator);
	  if ( EVEN(sts))
	  {
	    *out_terminator = RTT_K_NONE;
	    return RTT__SUCCESS;
	  }

	  if ( function == RTT_COMMAND_VALUE)
	  {
	    /* This is a value */
	    strcpy( out_string, command);
	    *out_terminator = RTT_K_RETURN;
	  }
	  else
	  {	  
	    if ( strcmp( command, "K_RETURN") == 0)
	      *out_terminator = RTT_K_RETURN;
	    else if ( strcmp( command, "K_PF1") == 0)
	      *out_terminator = RTT_K_PF1;
  	    else if ( strcmp( command, "K_PF2") == 0)
	      *out_terminator = RTT_K_PF2;
	    else if ( strcmp( command, "K_PF3") == 0)
	      *out_terminator = RTT_K_PF3;
	    else if ( strcmp( command, "K_PF4") == 0)
	      *out_terminator = RTT_K_PF4;
	    else if ( strcmp( command, "K_ARROW_UP") == 0)
	      *out_terminator = RTT_K_ARROW_UP;
	    else if ( strcmp( command, "K_ARROW_DOWN") == 0)
	      *out_terminator = RTT_K_ARROW_DOWN;
	    else if ( strcmp( command, "K_ARROW_RIGHT") == 0)
	      *out_terminator = RTT_K_ARROW_RIGHT;
	    else if ( strcmp( command, "K_ARROW_LEFT") == 0)
	      *out_terminator = RTT_K_ARROW_LEFT;
	    else if ( strcmp( command, "K_PREVIOUS_PAGE") == 0)
	      *out_terminator = RTT_K_PREVPAGE;
	    else if ( strcmp( command, "K_NEXT_PAGE") == 0)
	      *out_terminator = RTT_K_NEXTPAGE;
	    else if ( strcmp( command, "K_CTRLV") == 0)
	      *out_terminator = RTT_K_CTRLV;
	    else if ( strcmp( command, "K_DELETE") == 0)
	      *out_terminator = RTT_K_DELETE;
	    else if ( strcmp( command, "K_CTRLN") == 0)
	      *out_terminator = RTT_K_CTRLN;
	    else if ( strcmp( command, "K_CTRLZ") == 0)
	      *out_terminator = RTT_K_CTRLZ;
	    else if ( strcmp( command, "K_CTRLW") == 0)
	      *out_terminator = RTT_K_CTRLW;
	    else if ( strcmp( command, "K_CTRLL") == 0)
	      *out_terminator = RTT_K_CTRLL;
	    else if ( strcmp( command, "K_HELP") == 0)
	      *out_terminator = RTT_K_HELP;
	    else
	    {
	      /* Nothing for us */	  
	      rtt_commandmode_rewind(command);
	      *out_terminator = RTT_K_COMMAND;
	    }
	  }
	  sts = RTT__SUCCESS;
	}
	else
	{
	  sts = rtt_get_input_string( chn, out_string, out_terminator, out_maxlen,
		recall, option, timeout, timeout_func, timeout_arg, prompt);
	}

	if ( rtt_commandmode & RTT_COMMANDMODE_LEARN)
	{
	  if ( function == RTT_COMMAND_VALUE)
	  {
	    /* This is a value */
	    rtt_learn_store( out_string);
	  }
	  else
	  {
	    if ( *out_terminator == RTT_K_RETURN)
	      rtt_learn_store( "K_RETURN");
	    if ( *out_terminator == RTT_K_PF1)
	      rtt_learn_store( "K_PF1");
	    if ( *out_terminator == RTT_K_PF2)
    	      rtt_learn_store( "K_PF2");
	    if ( *out_terminator == RTT_K_PF3)
	      rtt_learn_store( "K_PF3");
	    if ( *out_terminator == RTT_K_PF4)
	      rtt_learn_store( "K_PF4");
	    if ( *out_terminator == RTT_K_ARROW_UP)
	      rtt_learn_store( "K_ARROW_UP");
	    if ( *out_terminator == RTT_K_ARROW_DOWN)
	      rtt_learn_store( "K_ARROW_DOWN");
	    if ( *out_terminator == RTT_K_ARROW_RIGHT)
	      rtt_learn_store( "K_ARROW_RIGHT");
	    if ( *out_terminator == RTT_K_ARROW_LEFT)
	      rtt_learn_store( "K_ARROW_LEFT");
	    if ( *out_terminator == RTT_K_PREVPAGE)
	      rtt_learn_store( "K_PREVIOUS_PAGE");
	    if ( *out_terminator == RTT_K_NEXTPAGE)
	      rtt_learn_store( "K_NEXT_PAGE");
	    if ( *out_terminator == RTT_K_CTRLV)
	      rtt_learn_store( "K_CTRLV");
	    if ( *out_terminator == RTT_K_DELETE)
	      rtt_learn_store( "K_DELETE");
	    if ( *out_terminator == RTT_K_CTRLN)
	      rtt_learn_store( "K_CTRLN");
	    if ( *out_terminator == RTT_K_CTRLZ)
	      rtt_learn_store( "K_CTRLZ");
	    if ( *out_terminator == RTT_K_CTRLW)
	      rtt_learn_store( "K_CTRLW");
	    if ( *out_terminator == RTT_K_CTRLL)
	      rtt_learn_store( "K_CTRLL");
	    if ( *out_terminator == RTT_K_HELP)
	      rtt_learn_store( "K_HELP");
	  }
	}

	return sts;
}

/*************************************************************************
*
* Name:		rtt_learn_start()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*file		I	file name.
*
* Description:
*	Starts a learn session.
*	Opens a file in which all the following commands will be stored.
*
**************************************************************************/

int	rtt_learn_start(
			char		*filename)
{
	char	filename_str[80];

	/* Add default extention '.rtt_com' if there is no extention given */
	rtt_get_defaultfilename( filename, filename_str, ".rtt_com");

	/* Open the file */
	rtt_learn_file = fopen( filename_str, "w");
	if ( rtt_learn_file == 0)
	  return RTT__NOFILE;

	/* Set learn mode */
	rtt_commandmode |= RTT_COMMANDMODE_LEARN;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_learn_stop()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Stops a learn session.
*	Close the file in which all the commands is stored.
*
**************************************************************************/

int	rtt_learn_stop()
{

	if ( !(rtt_commandmode & RTT_COMMANDMODE_LEARN))
	  return RTT__NOFILE;

	/* Close the file */
	fclose( rtt_learn_file);

	/* Reset learn mode */
	rtt_commandmode &= ~RTT_COMMANDMODE_LEARN;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_learn_store()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Store a command during a learn session.
*
**************************************************************************/

int	rtt_learn_store( char	*command)
{
	char	out_str[5][80];
	int	nr;

	if ( !(rtt_commandmode & RTT_COMMANDMODE_LEARN))
	  return RTT__NOFILE;

	/* Do not store the 'learn start' command */
	nr = rtt_parse( command, " ", "/", (char *)out_str, 
		sizeof( out_str) / sizeof( out_str[0]), sizeof( out_str[0]), 0);
	if ( nr == 2 &&
             strncmp( out_str[0], "LEARN", strlen(out_str[0])) == 0 &&
	     strncmp( out_str[1], "STOP", strlen(out_str[1])) == 0)
	  return RTT__SUCCESS;

	fprintf( rtt_learn_file, "%s\n", command);

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_store()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Store a command during a learn session.
*
**************************************************************************/

static int	rtt_store(
			menu_ctx	ctx,
			char		*filename,
			int		collect)
{
	rtt_t_menu_upd	*menu_ptr;
	char	filename_str[120];
	char	msg[120];
	FILE	*outfile;
	int	first;

	if ( !(ctx->menutype & RTT_MENUTYPE_UPD ))
	{
	  rtt_message('E', "Nothing to store in this picture");
	  return RTT__NOPICTURE;
	}

	/* Add default extention '.rtt_com' if there is no extention given */
	rtt_get_defaultfilename( filename, filename_str, ".rtt_com");

	/* Open the file */
	outfile = fopen( filename_str, "w");
	if ( outfile == 0)
	{
	  rtt_message('E', "Unable to open file");
	  return RTT__HOLDCOMMAND;
	}

	/* Loop through the menu item */
	
	first = 1;
	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	while ( menu_ptr->text[0] != 0)
	{
	  if ( menu_ptr->database == RTT_DATABASE_GDH)
	  {
	    /* Store only gdh parameters */
	    if ( first)
	    {
	      if ( collect)
	      {
	        fprintf( outfile, "collect show\ncollect clear\n");
	        fprintf( outfile, "collect/name=%s\n",
				menu_ptr->parameter_name);
	      }
	      else
	        fprintf( outfile, "show parameter/name=%s\n", 
				menu_ptr->parameter_name);
	      first = 0;
	    }
	    else
	    {
	      if ( collect)
	        fprintf( outfile, "collect/name=%s\n", 
				menu_ptr->parameter_name);
	      else
	        fprintf( outfile, "add parameter/name=%s\n", 
			menu_ptr->parameter_name);
	    }
	  }
	  menu_ptr++;
	}
	if ( collect)
	  fprintf( outfile, "collect show\n");

	rtt_fgetname( outfile, filename_str, filename_str);
	fclose( outfile);	

	sprintf( msg, "Picture stored %s", filename_str);
	rtt_message('I', msg);
	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_set_parameter()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Set value of a parameter in rtdb.
*
**************************************************************************/

static int	rtt_set_parameter(
			char		*name_str,
			char		*value_str,
			int		bypass)
{
	/* Get type of the parameter from info in the class object */

	int		sts;
	char		hiername[80];
	char		parname[80];
	pwr_tClassId	class;
	pwr_sParInfo	parinfo;
	char		objname[80];
	char		name_array[2][80];
	int		nr;
	pwr_tObjid	objid;
	pwr_tObjid	value_objid;
	pwr_eType	parameter_type;
	unsigned long	parameter_size;
	char		buffer[80];
	char		*buffer_ptr = buffer;
	int		value_syntax_error;
	char		*s;
	
	/* Check authorization, sys is required */
	if ( !bypass)
	{
	  if ( !(rtt_priv & RTT_PRIV_SYS) )
	  {
	    rtt_message('E',"Not authorized for this operation");
	    return RTT__SUCCESS;
	  }
	}

	/* Parse the parameter name into a object and a parameter name */
	nr = rtt_parse( name_str, ".", "",
		(char *) name_array, 
		sizeof( name_array)/sizeof( name_array[0]), 
		sizeof( name_array[0]), 0);
	if ( nr != 2)
	{
	  rtt_message('E', "Syntax error in name");
	  return RTT__HOLDCOMMAND;
	}
	strcpy( objname, name_array[0]);
	strcpy( parname, name_array[1]);

	/* Get objid */
	sts = gdh_NameToObjid ( objname, &objid);
	if ( EVEN(sts)) 
	{
	  rtt_message('E',"Object does not exist");
	  return RTT__HOLDCOMMAND;
	}

	/* Remove index if element in an array */
	if ( (s = strrchr( parname, '[')))
	  *s = 0;

	/* Get objid of rtbody */
	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;
	sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
		sizeof(hiername), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( hiername, "-RtBody-");
	strcat( hiername, parname);
	
	sts = gdh_GetObjectInfo ( hiername, &parinfo, sizeof(parinfo)); 
	if ( EVEN(sts)) 
	{
	  /* Try with sysbody */
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
			sizeof(hiername), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;
	  strcat( hiername, "-SysBody-");
	  strcat( hiername, parname);
	  sts = gdh_GetObjectInfo ( hiername, &parinfo, sizeof(parinfo)); 
	  if ( EVEN(sts)) 
	  {
	    rtt_message('E',"Parameter does not exist");
	    return RTT__HOLDCOMMAND;
	  }
	}
	
	parameter_type = parinfo.Type;
	parameter_size = parinfo.Size;
	if ( parinfo.Elements > 1)
	{
	  /* Check that attribute name includes index */
	  if ( !strrchr( name_str, '['))
	  {
	    rtt_message('E', "Attribute is an array, index is missing");
	    return RTT__HOLDCOMMAND;
	  }
	  parameter_size = parinfo.Size / parinfo.Elements;
	}
	value_syntax_error = 0;
	switch ( parameter_type )
	{
	  case pwr_eType_Boolean:
	  {
	    if ( sscanf( value_str, "%d", (pwr_tBoolean *)buffer_ptr) != 1)
	      value_syntax_error = 1;
	    if ( (*buffer_ptr < 0) || (*buffer_ptr > 1))
	      value_syntax_error = 1;
	    break;
	  }
	  case pwr_eType_Float32:
	  {
	    if ( sscanf( value_str, "%f", (pwr_tFloat32 *)buffer_ptr) != 1)

	      value_syntax_error = 1;
	    break;
	  }
	  case pwr_eType_Float64:
	  {
	    pwr_tFloat64 d;
	    pwr_tFloat32 f;

	    /* There is no input conversion specifier for double... */
	    if ( sscanf( value_str, "%f", &f) != 1)
	      value_syntax_error = 1;
	    else
	    {
	      d = f;
	      memcpy( buffer_ptr, (char *) &d, sizeof(d));
	    }
	    break;
	  }
	  case pwr_eType_Char:
	  {
	    if ( sscanf( value_str, "%c", buffer_ptr) != 1)
	      value_syntax_error = 1;
	    break;
	  }
	  case pwr_eType_Int8:
	  {
	    pwr_tInt16 	i16 ;
	    pwr_tInt8 	i8;
	    if ( sscanf( value_str, "%hd", &i16) != 1)
	      value_syntax_error = 1;
	    else
	    {
	      i8 = i16;
	      memcpy( buffer_ptr, (char *) &i8, sizeof(i8));
	    }
	    break;
	  }
	  case pwr_eType_Int16:
	  {
	    if ( sscanf( value_str, "%hd", (pwr_tInt16 *)buffer_ptr) != 1)
	      value_syntax_error = 1;
	    break;
	  }
	  case pwr_eType_Int32:
	  {
	    if ( sscanf( value_str, "%d", (int *)buffer_ptr) != 1)
	      value_syntax_error = 1;
	    break;
	  }
	  case pwr_eType_UInt8:
	  {
	    pwr_tUInt16 	ui16 ;
	    pwr_tUInt8 		ui8;
	    if ( sscanf( value_str, "%hd", &ui16) != 1)
	      value_syntax_error = 1;
	    else
	    {
	      ui8 = ui16;
	      memcpy( buffer_ptr, (char *) &ui8, sizeof(ui8));
	    }
	    break;
	  }
	  case pwr_eType_UInt16:
	  {
	    if ( sscanf( value_str, "%hd", (pwr_tUInt16 *)buffer_ptr) != 1)
	      value_syntax_error = 1;
	    break;
	  }
	  case pwr_eType_UInt32:
	  {
	    if ( sscanf( value_str, "%d", (pwr_tUInt32 *)buffer_ptr) != 1)
	      value_syntax_error = 1;
	    break;
	  }
	  case pwr_eType_String:
	  {
	    if ( strlen( value_str) >= parameter_size)
	    {
	      rtt_message('E',"String is to long");
	      return RTT__HOLDCOMMAND;
	    }
	    strncpy( buffer_ptr, value_str, 
			min( parameter_size, sizeof(buffer)));
	    break;
	  }
	  case pwr_eType_Objid:
	  {
	    sts = gdh_NameToObjid ( value_str, &value_objid);
	    if (EVEN(sts)) 
	    {
	      rtt_message('E',"Object does not exist");
	      return RTT__HOLDCOMMAND;
	    }

	    memcpy( buffer_ptr, &value_objid, sizeof( value_objid));
	    break;
	  }
	  default: ;
	}
	if ( value_syntax_error)
	{
	  rtt_message('E', "Syntax error in value");
	  return RTT__HOLDCOMMAND;
	}

	sts = gdh_SetObjectInfo ( name_str, buffer_ptr, parameter_size);
	if ( EVEN(sts)) 
	{
	  rtt_message('E',"Unable to set parameter");
	  return RTT__HOLDCOMMAND;
	}

	/* If bypass this is probably a fastkey, don't message */
	if ( !bypass)
	  rtt_message('I',"Parameter set");	
	return RTT__NOPICTURE;
}


/*************************************************************************
*
* Name:		rtt_get_current_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get the selected object in the current picture.
*
**************************************************************************/

static int rtt_get_current_object(
			menu_ctx	ctx,
			pwr_tObjid	*objid,
			char		*objectname,
			int		size,
			pwr_tBitMask	nametype)
{
	  rtt_t_menu_upd	*menu_ptr_upd;
	  rtt_t_menu		*menu_ptr_stat;
	  int			sts;

	  if ( ctx->menutype & RTT_MENUTYPE_UPD)
	  {
	    /* Get objid for the item, 
			assume that it is in the first argument */
	    menu_ptr_upd = (rtt_t_menu_upd *) ctx->menu;
	    menu_ptr_upd += ctx->current_item;
	    *objid = menu_ptr_upd->argoi;
	  }
	  else if ( ctx->menutype & RTT_MENUTYPE_MENU)
	  {
	    /* Get objid for the item, assume that it is in the first argument */
	    menu_ptr_stat = ctx->menu;
	    menu_ptr_stat += ctx->current_item;
	    *objid = menu_ptr_stat->argoi;
	  }
	  sts = gdh_ObjidToName ( *objid, objectname, size, nametype);
	  if ( EVEN(sts)) return sts;

	  return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_set_plcscan()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid	objid		I	objid of object.
* int		on		I	on or off.
* int		dum1		I
* int		dum2		I		
* int		dum3		I		
* int		dum4		I	
*
* Description:
*
**************************************************************************/

static int	rtt_set_plcscan(
			pwr_tObjid	objid,
			int		*on,
			void		*dum1,
			void		*dum2,
			void		*dum3,
			void		*dum4)
{
	char		parname[120];
	unsigned char	scanoff;
	int		sts;

	sts = gdh_ObjidToName ( objid, parname, sizeof(parname), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( parname, ".ScanOff");

	if ( *on)
	  scanoff = 0;
	else
	  scanoff = 1;

	sts = gdh_SetObjectInfo ( parname, &scanoff, sizeof( scanoff));
	if ( EVEN(sts)) return sts;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_plcscan()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	rtt context.
* char		*hiername	I	hierarchy object name.
*
* Description:
*
**************************************************************************/

static int	rtt_plcscan(
			menu_ctx	ctx,
			int		on,
			int		all,
			char		*hiername,
			int		global)
{
	int		sts;
	pwr_tClassId	class;
	pwr_tObjid	class_objid;
	pwr_tObjid	hierobjid;
	int		max_objects = 100000;
	char		name_str[100];
	char		*name = NULL;

	pwr_tString40	classname[]	= { 
				"WindowPlc", 
				"WindowCond", 
				"WindowOrderact",
				"WindowSubstep", 
				""};
	pwr_tString40	*classname_p;
	pwr_tString256	objectname;
	pwr_tObjid	objid;
	pwr_tBoolean	value;

	if ( !all)
	{
	  /* Check if hierarchy */
	  if ( hiername != NULL )
	  {
	    /* Get objid for the hierarchy object */
	    sts = gdh_NameToObjid ( hiername, &hierobjid);
	    if (EVEN(sts))
	    {
	      rtt_message('E',"Hierarchy object not found");
	      return RTT__HOLDCOMMAND;
	    }
 	  }
	  else
	  {
	    /* Get the selected object */
	    sts = rtt_get_current_object( ctx, &hierobjid, 
			name_str, sizeof( name_str), cdh_mName_volumeStrict);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Select an object or enter hierarchy");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	}
	else
	  hierobjid = pwr_cNObjid;


	if ( all && !global)
	{
	  /* Get all window classes */
	  classname_p = classname;
	  while ( strcmp( (char *)classname_p, "") != 0)
	  {
	  
	    sts = gdh_ClassNameToNumber ( (char *)classname_p, &class);
	    if ( EVEN(sts)) return sts;

	    /* Get every object of each class */
	    sts = gdh_GetClassList ( class, &objid);
	    while ( ODD(sts))
	    {
	    
	      /* Set attribute ScanOff to true */
	      sts = gdh_ObjidToName ( objid, objectname, sizeof(objectname), 
			cdh_mName_volumeStrict);
	      if ( EVEN(sts)) return sts;

	      strcat( objectname, ".ScanOff");

	      value = !on;
	      sts = gdh_SetObjectInfo ( objectname, &value, sizeof( value));
	      if ( EVEN(sts)) return sts;

	      sts = gdh_GetNextObject ( objid, &objid);
	    }
	    classname_p++;
	  }
	}	
	else
	{
	  sts = gdh_NameToObjid ( "pwrb:Class-WINDOWPLC", &class_objid);
	  if ( EVEN(sts)) return sts;
	  class = cdh_ClassObjidToId ( class_objid);

	  sts = rtt_get_objects_hier_class_name( ctx, hierobjid, 
		class, name, max_objects, global,
		&rtt_set_plcscan, &on, 0, 0, 0, 0);
	  if ( EVEN (sts)) return sts;

	  sts = gdh_NameToObjid ( "pwrb:Class-WINDOWCOND", &class_objid);
	  if ( EVEN(sts)) return sts;
	  class = cdh_ClassObjidToId ( class_objid);

	  sts = rtt_get_objects_hier_class_name( ctx, hierobjid, 
		class, name, max_objects, global,
		&rtt_set_plcscan, &on, 0, 0, 0, 0);
	  if ( EVEN (sts)) return sts;

	  sts = gdh_NameToObjid ( "pwrb:Class-WINDOWSUBSTEP", &class_objid);
	  if ( EVEN(sts)) return sts;
	  class = cdh_ClassObjidToId ( class_objid);

	  sts = rtt_get_objects_hier_class_name( ctx, hierobjid, 
		class, name, max_objects, global, 
		&rtt_set_plcscan, &on, 0, 0, 0, 0);
	  if ( EVEN (sts)) return sts;

	  sts = gdh_NameToObjid ( "pwrb:Class-WINDOWORDERACT", &class_objid);
	  if ( EVEN(sts)) return sts;
	  class = cdh_ClassObjidToId ( class_objid);

	  sts = rtt_get_objects_hier_class_name( ctx, hierobjid, 
		class, name, max_objects, global, 
		&rtt_set_plcscan, &on, 0, 0, 0, 0);
	  if ( EVEN (sts)) return sts;
	}
	if ( on)
	  rtt_message('I', "Plcscan set on");
	else
	  rtt_message('I', "Plcscan set off");


	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_create_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*classname	I	class name.
* char		*name		I	object name descripion.
*
* Description:
*	This function is called when a 'create object' command is recieved.
*
**************************************************************************/

static int	rtt_create_object(
			menu_ctx	parent_ctx,
			char		*classname,
			char		*name)
{
	int		sts;
	pwr_tObjid	objid;
	pwr_tClassId	class;

	
	/* Check if class */
	if ( classname != NULL )
	{
	  /* Get classid for the class */
	  sts = gdh_ClassNameToId ( classname, &class);
	  if ( EVEN(sts))
	  {
	    /* Class not found */
	    rtt_message('E',"Unknown class");
	    return RTT__HOLDCOMMAND;
	  }
	}

	sts = gdh_CreateObject( name, class, 0, &objid, pwr_cNObjid, 0, pwr_cNObjid);
	if ( EVEN(sts)) 	
	{
	  rtt_message('E',"Error in object name");
	  return RTT__HOLDCOMMAND;
	}

	rtt_message('I',"Object created");
	return RTT__NOPICTURE;
}


/*************************************************************************
*
* Name:		rtt_delete_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*name		I	object name descripion.
*
* Description:
*	This function is called when a 'delete object' command is recieved.
*
**************************************************************************/

static int	rtt_delete_object(
			menu_ctx	parent_ctx,
			char		*name)
{
	int		sts;
	pwr_tObjid	objid;

	
	/* Get objid for the object */
/*	sts = rtt_find_name( parent_ctx, name, &objid);*/
	sts = gdh_NameToObjid ( name, &objid);
	if ( EVEN(sts))
	{
     	  rtt_message('E',"Object does not exist");
	  return RTT__HOLDCOMMAND;
	}

	sts = gdh_DeleteObject( objid);
	if ( EVEN(sts)) 	
	{
	  rtt_message('E',"Unable to delete object");
	  return RTT__NOPICTURE;
	}

	rtt_message('I',"Object deleted");
	return RTT__NOPICTURE;
}


/*************************************************************************
*
* Name:		rtt_debug_obj_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	rtt context.
* char		*hiername	I	name of hierarchy object.
* char		*classname	I	name of class.
* char		*name		I	name description.
* int		add		I	if add to existing menulist or
*					creating a new menu.
*
* Description:
*	This function is called when a 'debug object' command is recieved.
*	All object under the hierarchy object, with the specified class
*	that fits in the name description is inserted in a menulist
*	and displayed on the screen.
*
**************************************************************************/

static int	rtt_show_step(
			menu_ctx	parent_ctx,
			char		*hiername,
			int		initstep)
{
	int		sts;
	int		index = 0;
	rtt_t_menu_upd	*menulist = 0;
	char		title[80] = "ACTIVE STEP LIST";
	char		*s;
	pwr_tString40	classname[]	= { 
				"Step", 
				"SsBegin", 
				"SsEnd",
				"InitStep",
				""};
	pwr_tString40	*classname_p;
	pwr_tString256	objectname;
	pwr_tString256	hierarchyname;
	pwr_tClassId	class;
	pwr_tObjid	objid;
	int		abort;
	pwr_tBoolean	value;


	if ( !initstep)
	{
	  /* Don't look for initsteps */
	  strcpy( classname[3], "");
	}


	if ( hiername != NULL)
	{
	  /* Check if name does not include a wildcard */
	  s = strchr( hiername, '*');
	  if ( s != 0)
	  {
     	    rtt_message('E',"Wildcard is not allowed");
	    return RTT__HOLDCOMMAND;
	  }

	  /* Get objid for the object */
	  sts = gdh_NameToObjid ( hiername, &objid);
	  if ( EVEN(sts))
	  {
     	    rtt_message('E',"Object does not exist");
	    return RTT__HOLDCOMMAND;
	  }

	  sts = gdh_ObjidToName ( objid, hierarchyname, sizeof(hierarchyname), 
			cdh_mNName);
	  if ( EVEN(sts)) return sts;
	}


	/* Get all step classes */
	abort = 0;
	classname_p = classname;
	while ( strcmp( (char *)classname_p, "") != 0)
	{
	  
	  sts = gdh_ClassNameToNumber ( (char *)classname_p, &class);
	  if ( EVEN(sts)) return sts;

	  /* Get every object of each class */
	  sts = gdh_GetClassList ( class, &objid);
	  while ( ODD(sts))
	  {
	    
	    /* Check if hierarchy i ok */
	    sts = gdh_ObjidToName ( objid, objectname, sizeof(objectname), cdh_mNName);
	    if ( EVEN(sts)) return sts;

	    if ( hiername != 0)
	    {
	      if ( strncmp( hierarchyname, objectname, strlen(hierarchyname)) !=
			0)
	      {
	        /* Next object */
	        sts = gdh_GetNextObject ( objid, &objid);
	        continue;
	      }	        
	    }

	    strcat( objectname, ".Order[0]");

	    sts = gdh_GetObjectInfo ( objectname, &value, sizeof( value));
	    if ( EVEN(sts)) return sts;

	    if ( value)
	    {
	      /* Insert in list */
	      sts = rtt_debug_object_add( objid, &menulist, &index, &zero, 0, 0);
	      if ( EVEN(sts) || sts == RTT__MAXCOUNT)
	      {
	        abort = 1;
	        break;
	      }
	    }
	    sts = gdh_GetNextObject ( objid, &objid);
	  }
	  if ( abort)
	    break;
	  classname_p++;
	}
	
	if ( sts == RTT__MAXCOUNT)
	  rtt_message('E',"To many object, all objects could not be shown");
	else if ( abort && EVEN (sts)) return sts;

	if ( menulist != 0)
	{
	  sts = rtt_menu_upd_bubblesort( menulist);
	  sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if (EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "No objects found");
	  return RTT__HOLDCOMMAND;
	}

	return RTT__SUCCESS;
}



/*************************************************************************
*
* Name:		rtt_print_picture()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* char		*filename	I	filename.
*
* Description:
*	Prints the content of a picture on a file.
*
**************************************************************************/

static int	rtt_print_picture(
			menu_ctx	ctx,
			char		*filename, 
			int		append,
			int		text_size,
			int		parameter_size)
{
	FILE	*outfile;
	int		sts;
	rtt_t_menu_upd	*menu_upd_ptr;
	rtt_t_menu	*menu_ptr;
	char		filename_str[80] = "";
	char		msg[200];
	view_ctx	viewctx;
	

	if ( filename != 0)
        {
	  /* Open the file */
	  rtt_get_defaultfilename( filename, filename_str, ".lis");
	  if ( append)
	    outfile = fopen( filename_str, "a");
	  else 
	    outfile = fopen( filename_str, "w");
	  if ( outfile == 0)
	  {
	    rtt_message('E', "Unable to open file");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else 
	  outfile = rtt_outfile;

	if ( ctx->ctx_type == RTT_CTXTYPE_MENU)
	{
	  if ( ctx->menutype & RTT_MENUTYPE_UPD)
	  {
	    menu_upd_ptr = (rtt_t_menu_upd *) ctx->menu;
	    fprintf( outfile, "    %s\n\n", ctx->title);
	    while ( menu_upd_ptr->text[0] != 0)
	    {
	      /* Print this item */
	      sts = rtt_print_item_upd( menu_upd_ptr->text, 
		menu_upd_ptr->parameter_name, menu_upd_ptr->value_ptr, 
		menu_upd_ptr->value_type, menu_upd_ptr->flags,
		outfile, text_size, parameter_size);
	      menu_upd_ptr++;
	    }

	    rtt_fgetname( outfile, filename_str, filename_str);
	    sprintf( msg, "Picture printed %s", filename_str);
	    if ( filename != 0)
	      rtt_message('I', msg);
	  }
	  else if ( ctx->menutype & RTT_MENUTYPE_MENU)
	  {
	    menu_ptr = ctx->menu;
	    fprintf( outfile, "    %s\n\n", ctx->title);
	    while ( menu_ptr->text[0] != 0)
	    {
	      /* Print this item */
	      fprintf( outfile, "%s\n", menu_ptr->text);
	      menu_ptr++;
	    }

	    rtt_fgetname( outfile, filename_str, filename_str);
	    sprintf( msg, "Picture printed %s", filename_str);
	    if ( filename != 0)
	      rtt_message('I', msg);
	  }
	  else
	  {
	    rtt_message('E', "Unable to print this picture");
	    return RTT__NOPICTURE;
	  }
	}
	else if (ctx->ctx_type == RTT_CTXTYPE_VIEW)
	{
	  viewctx = (view_ctx) ctx;

	  fprintf( outfile, "    %s\n\n", viewctx->title);

	  /* Print the buffer */
	  fprintf( outfile, "%s\n", viewctx->buff);

	  rtt_fgetname( outfile, filename_str, filename_str);
	  sprintf( msg, "Buffer printed %s", filename_str);
	  if ( filename != 0)
	    rtt_message('I', msg);

	}

	if ( filename != 0)
	  fclose( outfile);	
	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_print_picture_restore()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* char		*filename	I	filename.
*
* Description:
*	Prints the content of a picture in a command file so that the values
*	of the attributes in the picture can be restored.
*
**************************************************************************/

static int	rtt_print_picture_restore(
			menu_ctx	ctx,
			char		*filename)
{
	FILE	*outfile;
	int		sts;
	rtt_t_menu_upd	*menu_upd_ptr;
	char		filename_str[80];
	char		msg[200];
	char		outfilename[80];
	

	/* Open the file */
	rtt_get_defaultfilename( filename, filename_str, ".rtt_com");
	outfile = fopen( filename_str, "w");
	if ( outfile == 0)
	{
	  rtt_message('E', "Unable to open file");
	  return RTT__HOLDCOMMAND;
	}

	if ( ctx->ctx_type == RTT_CTXTYPE_MENU)
	{
	  if ( ctx->menutype & RTT_MENUTYPE_UPD)
	  {
	    menu_upd_ptr = (rtt_t_menu_upd *) ctx->menu;
	    while ( menu_upd_ptr->text[0] != 0)
	    {
	      /* Print this item */
	      sts = rtt_print_restore_item_upd(
		menu_upd_ptr->parameter_name, menu_upd_ptr->value_ptr, 
		menu_upd_ptr->value_type, menu_upd_ptr->flags,
		outfile);
	      menu_upd_ptr++;
	    }

	    rtt_fgetname( outfile, outfilename, filename_str);
	    sprintf( msg, "Picture printed %s", outfilename);
	    if ( filename != 0)
	      rtt_message('I', msg);
	  }
	}

	if ( filename != 0)
	  fclose( outfile);	
	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_print_text()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* char		*filename	I	filename.
*
* Description:
*	Prints a text on a file.
*
**************************************************************************/

static int	rtt_print_text(
			char		*filename, 
			char		*str,
			int		append)
{
	FILE	*outfile;
	char		filename_str[80] = "";
	char		msg[200];

	if ( filename != 0)
        {
	  /* Open the file */
	  rtt_get_defaultfilename( filename, filename_str, ".lis");
	  if ( append)
	    outfile = fopen( filename_str, "a");
	  else
	    outfile = fopen( filename_str, "w");
	  if ( outfile == 0)
	  {
	    rtt_message('E', "Unable to open file");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else
	  outfile = rtt_outfile;

	fprintf( outfile, "%s\n", str);

	rtt_fgetname( outfile, filename_str, filename_str);
	sprintf( msg, "Text printed %s", filename_str);
	if ( filename != 0)
	  rtt_message('I', msg);

	if ( filename != 0)
	  fclose( outfile);
	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_print_restore_item_upd()
*
* Type		int
*
* Description:
*	Prints an item in a file.
*
**************************************************************************/

static int	rtt_print_restore_item_upd(
			char		*parameter,
			char		*value_ptr,
			int		value_type,
			int		flags,
			FILE		*outfile)
{
	char		hiername[80];
	pwr_tObjid	objid;
	int		sts;

	if ( value_ptr == 0)
	{
	  return RTT__SUCCESS;
	}
	if ( value_ptr == (char *)RTT_ERASE )
	{
	  return RTT__SUCCESS;
	}
	if ( flags & PWR_MASK_POINTER )
 	{
	  return RTT__SUCCESS;
	}


	/* Print parameter */
	fprintf( outfile, "set par/name=%s/value=", parameter);

	switch ( value_type )
	{
	  case pwr_eType_Boolean:
	  {
	    fprintf( outfile,  "%d\n", *value_ptr);
	    break;
	  }
	  case pwr_eType_Float32:
	  {
	    fprintf( outfile,  "%f\n", *(float *)value_ptr);
	    break;
	  }
	  case pwr_eType_Float64:
	  {
	    fprintf( outfile,  "%f\n", *(double *)value_ptr);
	    break;
	  }
	  case pwr_eType_Char:
	  {
	    fprintf( outfile,  "%c\n", *value_ptr);
	    break;
	  }
	  case pwr_eType_Int8:
	  {
	    fprintf( outfile,  "%d\n", *value_ptr);
	    break;
	  }
	  case pwr_eType_Int16:
	  {
	    fprintf( outfile,  "%d\n", *(short *)value_ptr);
	    break;
	  }
	  case pwr_eType_Int32:
	  {
	    fprintf( outfile,  "%d\n", *(int *)value_ptr);
	    break;
	  }
	  case pwr_eType_UInt8:
	  {
	    fprintf( outfile,  "%d\n", *(unsigned char *)value_ptr);
	    break;
	  }
	  case pwr_eType_UInt16:
	  {
	    fprintf( outfile,  "%d\n", *(unsigned short *)value_ptr);
	    break;
	  }
	  case pwr_eType_UInt32:
	  {
	    fprintf( outfile,  "%lu\n", *(unsigned long *)value_ptr);
	    break;
	  }
	  case pwr_eType_String:
	  {
	    fprintf( outfile,  "%s\n", value_ptr);
	    break;
	  }
	  case pwr_eType_Objid:
	  {
	    objid = *(pwr_tObjid *)value_ptr;
	    sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	    if (EVEN(sts)) break;

	    fprintf( outfile,  "%s\n", hiername);
	    break;
	  }
	  default:
	    fprintf( outfile,  "Unknown type\n");
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_print_value()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*value_ptr	I	pointer to value
* int		value_type	I	type of value
* int		flags		I	flags of value
* int		size		I	size of value
* FILE		*outfile	I	output file
*
* Description:
*	Prints an item in a file.
*
**************************************************************************/

static int	rtt_print_item_upd(
			char		*text,
			char		*parameter,
			char		*value_ptr,
			int		value_type,
			int		flags,
			FILE		*outfile,
			int		text_size,
			int		parameter_size)
{
	char		hiername[80];
	pwr_tObjid	objid;
	int		sts;
	int		spaces;
	int		i;
	char		str[80];

	/* Print text */
	if ( text_size > 0)
	{
	  strcpy( str, text);
	  str[text_size] = 0;
	  fprintf( outfile, "%s", str);

	  spaces = text_size + 1 - strlen( str);
	  for ( i = 0; i < spaces; i++)
	    fprintf(outfile, " ");
	}
	
	/* Print parameter */
	if ( parameter_size > 0)
	{
	  strcpy( str,  parameter);
	  str[parameter_size] = 0;
	  fprintf( outfile, "%s", str);
	
	  spaces = parameter_size + 1 - strlen( str);
	  for ( i = 0; i < spaces; i++)
	    fprintf(outfile, " ");
	}

	if ( value_ptr == 0)
	{
	  fprintf( outfile, "UNDEFINED\n");
	  return RTT__SUCCESS;
	}
	if ( value_ptr == (char *)RTT_ERASE )
	{
	  fprintf( outfile, "\n");
	  return RTT__SUCCESS;
	}

	/* If this is a pointer, get the pointer */
	if ( flags & PWR_MASK_POINTER )
 	{
	  /* This is not a rtdb pointer */
	  fprintf( outfile, "UNDEFINED POINTER\n");
	  return RTT__SUCCESS;
	}


	  switch ( value_type )
	  {
	    case pwr_eType_Boolean:
	    {
	      fprintf( outfile,  "%d\n", *value_ptr);
	      break;
	    }
	    case pwr_eType_Float32:
	    {
	      fprintf( outfile,  "%f\n", *(float *)value_ptr);
	      break;
	    }
	    case pwr_eType_Float64:
	    {
	      fprintf( outfile,  "%f\n", *(double *)value_ptr);
	      break;
	    }
	    case pwr_eType_Char:
	    {
	      fprintf( outfile,  "%c\n", *value_ptr);
	      break;
	    }
	    case pwr_eType_Int8:
	    {
	      fprintf( outfile,  "%d\n", *value_ptr);
	      break;
	    }
	    case pwr_eType_Int16:
	    {
	      fprintf( outfile,  "%d\n", *(short *)value_ptr);
	      break;
	    }
	    case pwr_eType_Int32:
	    {
	      fprintf( outfile,  "%d\n", *(int *)value_ptr);
	      break;
	    }
	    case pwr_eType_UInt8:
	    {
	      fprintf( outfile,  "%d\n", *(unsigned char *)value_ptr);
	      break;
	    }
	    case pwr_eType_UInt16:
	    {
	      fprintf( outfile,  "%d\n", *(unsigned short *)value_ptr);
	      break;
	    }
	    case pwr_eType_UInt32:
	    {
	      fprintf( outfile,  "%lu\n", *(unsigned long *)value_ptr);
	      break;
	    }
	    case pwr_eType_String:
	    {
	      fprintf( outfile,  "%s\n", value_ptr);
	      break;
	    }
	    case pwr_eType_Objid:
	    {
	      objid = *(pwr_tObjid *)value_ptr;
	      sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	      if (EVEN(sts)) break;

	      fprintf( outfile,  "%s\n", hiername);
	      break;
	    }
	    default:
	      fprintf( outfile,  "Unknown type\n");

	  }

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_print_value()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Starts debug signals for current item in an edit picture.
*
**************************************************************************/

int	rtt_edit_debug_signals(
			menu_ctx	ctx,
			pwr_tObjid	objid,
			void		*dum1,
			void		*dum2,
			void		*dum3,
			void		*dum4)
{
	rtt_t_menu_upd	*menu_ptr;
	char		hiername[80];
	int		sts;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;

	sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
			cdh_mName_path | cdh_mName_object);
	if ( EVEN(sts)) 
	{
	  rtt_message('E', "No signal debug on this item");
	  return RTT__NOPICTURE;
	}

	/* Get all the windows in the plc */
	strcat( hiername, "-W*");
	sts = rtt_show_signals( ctx, NULL, hiername, 1);
	return sts;

}
/*************************************************************************
*
* Name:		rtt_set_conversion()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Set conversin for a channel on or off.
*
**************************************************************************/

static int	rtt_set_conversion(	pwr_tObjid	objid,
				int		on,
				int		show_only)
{
	int		sts;
	pwr_tObjid	chan_objid;
	pwr_tObjid	card_objid;
	pwr_tClassId	class;
	pwr_tUInt16	chan_number;
	pwr_tUInt32	conv_mask;
	char		name[80];
	char		card_name[80];
	char		attrname[120];
	pwr_tBoolean	conversion_on;

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Ai:
	  case pwr_cClass_Di:
	  case pwr_cClass_Co:
	    /* This is a signal, get the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( attrname, &chan_objid, 
			sizeof(chan_objid));
	    if (EVEN(sts)) return sts;
	    sts = gdh_GetObjectClass ( chan_objid, &class);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object is not connected");
	      return RTT__NOPICTURE;
	    }
	    switch ( class)
	    {
	      case pwr_cClass_ChanAi:
	      case pwr_cClass_ChanDi:
	      case pwr_cClass_ChanCo:
		sts = gdh_ObjidToName ( chan_objid, name, sizeof(name), 
				cdh_mName_volumeStrict);
		if ( EVEN(sts)) return sts;
	        break;
	      default:
	        rtt_message('E', "Connected object is not a channel");
	        return RTT__NOPICTURE;
	    }
	    break;
	  case pwr_cClass_ChanAi:
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanCo:
	    /* Channel is ok */
	    chan_objid = objid;
	    break;
	  default:
	    rtt_message('E', "Object has to be a Di or Co channel or signal");
	    return RTT__NOPICTURE;
	}

	switch ( class)
	{
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanCo:
	    /* Get the card the channel */
	    sts = gdh_GetParent ( chan_objid, &card_objid);
	    if ( EVEN(sts)) return sts;

	    sts = gdh_ObjidToName ( card_objid, card_name, sizeof(name), 
				cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;

	    /* Get the number of the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".Number");	
	    sts = gdh_GetObjectInfo ( attrname, &chan_number, 
			sizeof(chan_number));
	    if (EVEN(sts)) return sts;

	    strcpy( attrname, card_name);
	    switch( class)
	    {
	      case pwr_cClass_ChanDi:
	        if ( chan_number >= 16)
	        {
	          strcat( attrname, ".ConvMask2");
	          chan_number -= 16;
	        }
	        else
	          strcat( attrname, ".ConvMask1");
	         break;
	      case pwr_cClass_ChanCo:
	        strcat( attrname, ".ConvMask");
	        break;
	    }
	    sts = gdh_GetObjectInfo ( attrname, &conv_mask, 
			sizeof(conv_mask));
	    if (EVEN(sts))
	    {
	      rtt_message('E', "Unable to set conversion");
	      return RTT__NOPICTURE;
	    }

	    if ( show_only)
	    {
	      if ( conv_mask & 1 << chan_number)
	        rtt_message('I', "Conversion is on");
	      else
	        rtt_message('I', "Conversion is off");
	      return RTT__NOPICTURE;
	    }

	    if ( on)
	      conv_mask |= 1 << chan_number;
	    else
	      conv_mask &= ~(1 << chan_number);

	    sts = gdh_SetObjectInfo ( attrname, &conv_mask, sizeof(conv_mask));
	    if ( EVEN(sts)) return sts;
	    break;
	  case pwr_cClass_ChanAi:
	    /* Get the number of the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".ConversionOn");	
	    if ( show_only)
	    {
	      sts = gdh_GetObjectInfo ( attrname, &conversion_on, 
			sizeof(conversion_on));
	      if (EVEN(sts)) return sts;
	      if ( conversion_on)
	        rtt_message('I', "Conversion is on");
	      else
	        rtt_message('I', "Conversion is off");
	      return RTT__NOPICTURE;
	    }
	    conversion_on = on;
	    sts = gdh_SetObjectInfo ( attrname, &conversion_on, 
			sizeof(conversion_on));
	    if (EVEN(sts)) return sts;
	    break;
	}
	if ( on)
	  rtt_message('I', "Conversion set on");
	else
	  rtt_message('I', "Conversion set off");

	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_set_invert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Set invert for a channel on or off.
*
**************************************************************************/

static int	rtt_set_invert(	pwr_tObjid	objid,
			int		on,
			int		show_only)
{
	int		sts;
	pwr_tObjid	chan_objid;
	pwr_tObjid	card_objid;
	pwr_tClassId	class;
	pwr_tUInt16	chan_number;
	pwr_tUInt32	inv_mask;
	char		name[80];
	char		card_name[80];
	char		attrname[120];
	pwr_tBoolean	invert_on;

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Di:
	  case pwr_cClass_Do:
	  case pwr_cClass_Po:
	    /* This is a signal, get the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( attrname, &chan_objid, 
			sizeof(chan_objid));
	    if (EVEN(sts)) return sts;
	    sts = gdh_GetObjectClass ( chan_objid, &class);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object is not connected");
	      return RTT__NOPICTURE;
	    }
	    switch ( class)
	    {
	      case pwr_cClass_ChanDi:
	      case pwr_cClass_ChanDo:
		sts = gdh_ObjidToName ( chan_objid, name, sizeof(name), 
				cdh_mName_volumeStrict);
		if ( EVEN(sts)) return sts;
	        break;
	      default:
	        rtt_message('E', "Connected object is not a channel");
	        return RTT__NOPICTURE;
	    }
	    break;
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	    /* Channel is ok */
	    chan_objid = objid;
	    break;
	  default:
	    rtt_message('E', "Object has to be a Do or Di channel or a signal");
	    return RTT__NOPICTURE;
	}

	/* Get the card the channel */
	sts = gdh_GetParent ( chan_objid, &card_objid);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( card_objid, card_name, sizeof(card_name), 
		cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get the number of the channel */
	strcpy( attrname, name);
	strcat( attrname, ".Number");	
	sts = gdh_GetObjectInfo ( attrname, &chan_number, 
			sizeof(chan_number));
	if (EVEN(sts)) return sts;

	strcpy( attrname, card_name);
	switch( class)
	{
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	    if ( chan_number >= 16)
	    {
	      strcat( attrname, ".InvMask2");
	      chan_number -= 16;
	    }
	    else
	      strcat( attrname, ".InvMask1");
	    break;
	}
	sts = gdh_GetObjectInfo ( attrname, &inv_mask, 
			sizeof(inv_mask));
	if (EVEN(sts))
	{
	  rtt_message('E', "Unable to get invertmask");
	  return RTT__NOPICTURE;
	}

	if ( show_only)
	{
	  if ( inv_mask & 1 << chan_number)
	    rtt_message('I', "Invert is on");
	  else
	    rtt_message('I', "Invert is off");
	  return RTT__NOPICTURE;
	}

	if ( on)
	  inv_mask |= 1 << chan_number;
	else
	  inv_mask &= ~(1 << chan_number);

	sts = gdh_SetObjectInfo ( attrname, &inv_mask, sizeof(inv_mask));
	if ( EVEN(sts)) return sts;


	/* Set the flag in the channelobject */
	sts = gdh_ObjidToName ( chan_objid, attrname, sizeof(attrname), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( attrname, ".InvertOn");	
	invert_on = on;
	sts = gdh_SetObjectInfo ( attrname, &invert_on, sizeof(invert_on));
	if ( EVEN(sts)) return sts;

	if ( on)
	  rtt_message('I', "Invert set on");
	else
	  rtt_message('I', "Invert set off");
	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_get_invert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get invert for a channel on or off.
*
**************************************************************************/

static int	rtt_get_invert(	pwr_tObjid	objid,
			int		*on)
{
	int		sts;
	pwr_tObjid	chan_objid;
	pwr_tObjid	card_objid;
	pwr_tClassId	class;
	pwr_tUInt16	chan_number;
	pwr_tUInt32	inv_mask;
	char		name[80];
	char		card_name[80];
	char		attrname[120];

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Di:
	  case pwr_cClass_Do:
	  case pwr_cClass_Po:
	    /* This is a signal, get the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( attrname, &chan_objid, 
			sizeof(chan_objid));
	    if (EVEN(sts)) return sts;
	    sts = gdh_GetObjectClass ( chan_objid, &class);
	    if ( EVEN(sts)) return sts;
	    switch ( class)
	    {
	      case pwr_cClass_ChanDi:
	      case pwr_cClass_ChanDo:
		sts = gdh_ObjidToName ( chan_objid, name, sizeof(name), 
			cdh_mName_volumeStrict);
		if ( EVEN(sts)) return sts;
	        break;
	      default:
	        return RTT__NOPICTURE;
	    }
	    break;
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	    /* Channel is ok */
	    chan_objid = objid;
	    break;
	  default:
	    return 0;
	}

	/* Get the card the channel */
	sts = gdh_GetParent ( chan_objid, &card_objid);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( card_objid, card_name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get the number of the channel */
	strcpy( attrname, name);
	strcat( attrname, ".Number");	
	sts = gdh_GetObjectInfo ( attrname, &chan_number, 
			sizeof(chan_number));
	if (EVEN(sts)) return sts;

	strcpy( attrname, card_name);
	switch( class)
	{
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	    if ( chan_number >= 16)
	    {
	      strcat( attrname, ".InvMask2");
	      chan_number -= 16;
	    }
	    else
	      strcat( attrname, ".InvMask1");
	    break;
	}
	sts = gdh_GetObjectInfo ( attrname, &inv_mask, 
			sizeof(inv_mask));
	if (EVEN(sts)) return sts;

	*on = inv_mask & 1 << chan_number;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_conversion()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get conversin for a channel on or off.
*
**************************************************************************/

static int	rtt_get_conversion(	pwr_tObjid	objid,
				int		*on)
{
	int		sts;
	pwr_tObjid	chan_objid;
	pwr_tObjid	card_objid;
	pwr_tClassId	class;
	pwr_tUInt16	chan_number;
	pwr_tUInt32	conv_mask;
	char		name[80];
	char		card_name[80];
	char		attrname[120];

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Di:
	  case pwr_cClass_Co:
	    /* This is a signal, get the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( attrname, &chan_objid, 
			sizeof(chan_objid));
	    if (EVEN(sts)) return sts;
	    sts = gdh_GetObjectClass ( chan_objid, &class);
	    if ( EVEN(sts)) return sts;
	    switch ( class)
	    {
	      case pwr_cClass_ChanDi:
	      case pwr_cClass_ChanCo:
		sts = gdh_ObjidToName ( chan_objid, name, sizeof(name), 
				cdh_mName_volumeStrict);
		if ( EVEN(sts)) return sts;
	        break;
	      default:
	        return 0;
	    }
	    break;
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanCo:
	    /* Channel is ok */
	    chan_objid = objid;
	    break;
	  default:
	    return 0;
	}

	/* Get the card the channel */
	sts = gdh_GetParent ( chan_objid, &card_objid);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( card_objid, card_name, sizeof(name), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get the number of the channel */
	strcpy( attrname, name);
	strcat( attrname, ".Number");	
	sts = gdh_GetObjectInfo ( attrname, &chan_number, 
			sizeof(chan_number));
	if (EVEN(sts)) return sts;

	strcpy( attrname, card_name);
	switch( class)
	{
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanAi:
	    if ( chan_number >= 16)
	    {
	      strcat( attrname, ".ConvMask2");
	      chan_number -= 16;
	    }
	    else
	      strcat( attrname, ".ConvMask1");
	    break;
	  case pwr_cClass_ChanCo:
	    strcat( attrname, ".ConvMask");
	    break;
	}
	sts = gdh_GetObjectInfo ( attrname, &conv_mask, 
			sizeof(conv_mask));
	if (EVEN(sts))
	{
	  rtt_message('E', "Unable to set conversion");
	  return RTT__NOPICTURE;
	}

	*on = conv_mask & 1 << chan_number;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_do_test()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get test for a do channel on or off.
*
**************************************************************************/

int	rtt_get_do_test(	pwr_tObjid	objid,
				int		*on)
{
	int		sts;
	pwr_tObjid	chan_objid;
	pwr_tObjid	card_objid;
	pwr_tClassId	class;
	pwr_tUInt16	chan_number;
	pwr_tUInt32	test_mask;
	char		name[80];
	char		card_name[80];
	char		attrname[120];

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Do:
	  case pwr_cClass_Po:
	    /* This is a signal, get the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( attrname, &chan_objid, 
			sizeof(chan_objid));
	    if (EVEN(sts)) return sts;
	    sts = gdh_GetObjectClass ( chan_objid, &class);
	    if ( EVEN(sts)) return sts;
	    switch ( class)
	    {
	      case pwr_cClass_ChanDo:
		sts = gdh_ObjidToName ( chan_objid, name, sizeof(name), 
			cdh_mName_volumeStrict);
		if ( EVEN(sts)) return sts;
	        break;
	      default:
	        return 0;
	    }
	    break;
	  case pwr_cClass_ChanDo:
	    /* Channel is ok */
	    chan_objid = objid;
	    break;
	  default:
	    return 0;
	}

	/* Get the card the channel */
	sts = gdh_GetParent ( chan_objid, &card_objid);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( card_objid, card_name, sizeof(name), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get the number of the channel */
	strcpy( attrname, name);
	strcat( attrname, ".Number");	
	sts = gdh_GetObjectInfo ( attrname, &chan_number, 
			sizeof(chan_number));
	if (EVEN(sts)) return sts;

	strcpy( attrname, card_name);
	if ( chan_number >= 16)
	{
	  strcat( attrname, ".TestMask2");
	  chan_number -= 16;
	}
	else
	  strcat( attrname, ".TestMask1");

	sts = gdh_GetObjectInfo ( attrname, &test_mask, 
			sizeof(test_mask));
	if (EVEN(sts))
	{
	  rtt_message('E', "Unable to get testmask");
	  return RTT__NOPICTURE;
	}

	*on = test_mask & 1 << chan_number;
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_set_do_test()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Set invert for a channel on or off.
*
**************************************************************************/

int	rtt_set_do_test( pwr_tObjid	objid,
			int		on,
			int		show_only)
{
	int		sts;
	pwr_tObjid	chan_objid;
	pwr_tObjid	card_objid;
	pwr_tClassId	class;
	pwr_tUInt16	chan_number;
	pwr_tUInt32	test_mask;
	char		name[80];
	char		card_name[80];
	char		attrname[120];
	pwr_tBoolean	test_on;

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Do:
	  case pwr_cClass_Po:
	    /* This is a signal, get the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( attrname, &chan_objid, 
			sizeof(chan_objid));
	    if (EVEN(sts)) return sts;
	    sts = gdh_GetObjectClass ( chan_objid, &class);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object is not connected");
	      return RTT__NOPICTURE;
	    }
	    switch ( class)
	    {
	      case pwr_cClass_ChanDo:
		sts = gdh_ObjidToName ( chan_objid, name, sizeof(name), 
				cdh_mName_volumeStrict);
		if ( EVEN(sts)) return sts;
	        break;
	      default:
	        rtt_message('E', "Connected object is not a channel");
	        return RTT__NOPICTURE;
	    }
	    break;
	  case pwr_cClass_ChanDo:
	    /* Channel is ok */
	    chan_objid = objid;
	    break;
	  default:
	    rtt_message('E', "Object has to be a channel or a signal");
	    return RTT__NOPICTURE;
	}

	/* Get the card the channel */
	sts = gdh_GetParent ( chan_objid, &card_objid);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( card_objid, card_name, sizeof(card_name), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get the number of the channel */
	strcpy( attrname, name);
	strcat( attrname, ".Number");	
	sts = gdh_GetObjectInfo ( attrname, &chan_number, 
			sizeof(chan_number));
	if (EVEN(sts)) return sts;

	strcpy( attrname, card_name);
	switch( class)
	{
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	    if ( chan_number >= 16)
	    {
	      strcat( attrname, ".TestMask2");
	      chan_number -= 16;
	    }
	    else
	      strcat( attrname, ".TestMask1");
	    break;
	}
	sts = gdh_GetObjectInfo ( attrname, &test_mask, 
			sizeof(test_mask));
	if (EVEN(sts))
	{
	  rtt_message('E', "Unable to get testmask");
	  return RTT__NOPICTURE;
	}

	if ( show_only)
	{
	  if ( test_mask & 1 << chan_number)
	    rtt_message('I', "Test is on");
	  else
	    rtt_message('I', "Test is off");
	  return RTT__NOPICTURE;
	}

	if ( on)
	  test_mask |= 1 << chan_number;
	else
	  test_mask &= ~(1 << chan_number);

	sts = gdh_SetObjectInfo ( attrname, &test_mask, sizeof(test_mask));
	if ( EVEN(sts)) return sts;

	/* Set the flag in the channelobject */
	sts = gdh_ObjidToName ( chan_objid, attrname, sizeof(attrname), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( attrname, ".TestOn");	
	test_on = on;
	sts = gdh_SetObjectInfo ( attrname, &test_on, sizeof(test_on));
	if ( EVEN(sts)) return sts;

	if ( on)
	  rtt_message('I', "Test set on");
	else
	  rtt_message('I', "Test set off");
	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_get_do_testvalue()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get testvalue for a do channel (true or false).
*
**************************************************************************/

int	rtt_get_do_testvalue(	pwr_tObjid	objid,
				int		*on)
{
	int		sts;
	pwr_tObjid	chan_objid;
	pwr_tObjid	card_objid;
	pwr_tClassId	class;
	pwr_tUInt16	chan_number;
	pwr_tUInt32	testvalue_mask;
	char		name[80];
	char		card_name[80];
	char		attrname[120];

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Do:
	  case pwr_cClass_Po:
	    /* This is a signal, get the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( attrname, &chan_objid, 
			sizeof(chan_objid));
	    if (EVEN(sts)) return sts;
	    sts = gdh_GetObjectClass ( chan_objid, &class);
	    if ( EVEN(sts)) return sts;
	    switch ( class)
	    {
	      case pwr_cClass_ChanDo:
		sts = gdh_ObjidToName ( chan_objid, name, sizeof(name), 
				cdh_mName_volumeStrict);
		if ( EVEN(sts)) return sts;
	        break;
	      default:
	        return 0;
	    }
	    break;
	  case pwr_cClass_ChanDo:
	    /* Channel is ok */
	    chan_objid = objid;
	    break;
	  default:
	    return 0;
	}

	/* Get the card the channel */
	sts = gdh_GetParent ( chan_objid, &card_objid);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( card_objid, card_name, sizeof(name), 
		cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get the number of the channel */
	strcpy( attrname, name);
	strcat( attrname, ".Number");	
	sts = gdh_GetObjectInfo ( attrname, &chan_number, 
			sizeof(chan_number));
	if (EVEN(sts)) return sts;

	strcpy( attrname, card_name);
	if ( chan_number >= 16)
	{
	  strcat( attrname, ".TestValue2");
	  chan_number -= 16;
	}
	else
	  strcat( attrname, ".TestValue1");

	sts = gdh_GetObjectInfo ( attrname, &testvalue_mask, 
			sizeof(testvalue_mask));
	if (EVEN(sts))
	{
	  rtt_message('E', "Unable to get testvalue");
	  return RTT__NOPICTURE;
	}

	*on = testvalue_mask & 1 << chan_number;
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_set_do_testvalue()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Set testvalue of do true or false
*
**************************************************************************/

int	rtt_set_do_testvalue(	pwr_tObjid	objid,
			int		on,
			int		show_only)
{
	int		sts;
	pwr_tObjid	chan_objid;
	pwr_tObjid	card_objid;
	pwr_tClassId	class;
	pwr_tUInt16	chan_number;
	pwr_tUInt32	testvalue_mask;
	char		name[80];
	char		card_name[80];
	char		attrname[120];
	pwr_tBoolean	test_value;

	sts = gdh_GetObjectClass ( objid, &class);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_Do:
	  case pwr_cClass_Po:
	    /* This is a signal, get the channel */
	    strcpy( attrname, name);
	    strcat( attrname, ".SigChanCon");
	    sts = gdh_GetObjectInfo ( attrname, &chan_objid, 
			sizeof(chan_objid));
	    if (EVEN(sts)) return sts;
	    sts = gdh_GetObjectClass ( chan_objid, &class);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object is not connected");
	      return RTT__NOPICTURE;
	    }
	    switch ( class)
	    {
	      case pwr_cClass_ChanDo:
		sts = gdh_ObjidToName ( chan_objid, name, sizeof(name), 
				cdh_mName_volumeStrict);
		if ( EVEN(sts)) return sts;
	        break;
	      default:
	        rtt_message('E', "Connected object is not a channel");
	        return RTT__NOPICTURE;
	    }
	    break;
	  case pwr_cClass_ChanDo:
	    /* Channel is ok */
	    chan_objid = objid;
	    break;
	  default:
	    rtt_message('E', "Object has to be a channel or a signal");
	    return RTT__NOPICTURE;
	}

	/* Get the card the channel */
	sts = gdh_GetParent ( chan_objid, &card_objid);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( card_objid, card_name, sizeof(name), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get the number of the channel */
	strcpy( attrname, name);
	strcat( attrname, ".Number");	
	sts = gdh_GetObjectInfo ( attrname, &chan_number, 
			sizeof(chan_number));
	if (EVEN(sts)) return sts;

	strcpy( attrname, card_name);
	switch( class)
	{
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	    if ( chan_number >= 16)
	    {
	      strcat( attrname, ".TestValue2");
	      chan_number -= 16;
	    }
	    else
	      strcat( attrname, ".TestValue1");
	    break;
	}
	sts = gdh_GetObjectInfo ( attrname, &testvalue_mask, 
			sizeof(testvalue_mask));
	if (EVEN(sts))
	{
	  rtt_message('E', "Unable to get testvalue");
	  return RTT__NOPICTURE;
	}

	if ( show_only)
	{
	  if ( testvalue_mask & 1 << chan_number)
	    rtt_message('I', "Testvalue is true");
	  else
	    rtt_message('I', "Testvalue is false");
	  return RTT__NOPICTURE;
	}

	if ( on)
	  testvalue_mask |= 1 << chan_number;
	else
	  testvalue_mask &= ~(1 << chan_number);

	sts = gdh_SetObjectInfo ( attrname, &testvalue_mask, sizeof(testvalue_mask));
	if ( EVEN(sts)) return sts;


	/* Set the flag in the channelobject */
	sts = gdh_ObjidToName ( chan_objid, attrname, sizeof(attrname), 
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( attrname, ".TestValue");	
	test_value = on;
	sts = gdh_SetObjectInfo ( attrname, &test_value, sizeof(test_value));
	if ( EVEN(sts)) return sts;

	if ( on)
	  rtt_message('I', "Testvalue is set to true");
	else
	  rtt_message('I', "Testvalue is set to false");
	return RTT__NOPICTURE;
}


/*************************************************************************
*
* Name:		rtt_show_menu()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Show a menu specified by a string.
*
**************************************************************************/

int	rtt_show_menu( 	menu_ctx	ctx, 
			char 		*menu_name)
{
	char		name_array[20][32];
	int		i;
	rtt_t_menu	*menu_ptr;
	char		title[80];
	int		nr;
	int		found;
	int		sts;

	/* Parse the menu_name */
	nr = rtt_parse( menu_name, "-", "",
		(char *) name_array, 
		sizeof( name_array)/sizeof( name_array[0]), 
		sizeof( name_array[0]), 0);

	menu_ptr = rtt_mainmenu;
	for ( i = 0; i < nr; i++)
	{

	  rtt_toupper( name_array[i], name_array[i]);
	  found = 0;
	  while ( menu_ptr->text[0])
	  {
	    rtt_toupper( title, menu_ptr->text);
	    if ( !strcmp( title, name_array[i]))
	    {
	      if ( i < nr -1)
	      {
	        /* Check that the menu type is correct */
	        if ( !( menu_ptr->func == rtt_menu_new ||
		        menu_ptr->func == rtt_menu_keys_new ))
	        {
	          rtt_message('E',"Error in menu type");
	          return RTT__NOPICTURE;
	        }
	        if ( !menu_ptr->arg1)
	        {
	          rtt_message('E',"Menu not found");
	          return RTT__NOPICTURE;
	        }
	        menu_ptr = * (rtt_t_menu **) (menu_ptr->arg1);
	        found = 1;
	        break;
	      }
	      else
	      {
	        found = 1;
	        break;
	      }
	    }
	    menu_ptr++;
	  }
	  if ( !found)
	  {
	    rtt_message('E',"Menu not found");
	    return RTT__HOLDCOMMAND;
	  }
	}

	if ( menu_ptr->func)
	{
	  sts = (menu_ptr->func)( ctx, menu_ptr->argoi, menu_ptr->arg1, 
			menu_ptr->arg2, menu_ptr->arg3, menu_ptr->arg4);
	  return sts;
	}
	else
	{
	  rtt_message('E',"Function in menu not defined");
	  return RTT__NOPICTURE;
	}
}

int	rtt_remove_blank( char *out_str, char *in_str)
{
	char *s;

	s = in_str;
	/* Find first not blank */
	while ( *s)
	{
	  if ( !(*s == 9 || *s == 32)) 
	    break;
	  s++;
	}
	strcpy( out_str, s);
	/* Remove at end */
	s = out_str + strlen(out_str);
	s--;
	while( s >= out_str)
	{
	  if ( !(*s == 9 || *s == 32)) 
	    break;
	  s--;
	}
	s++;
	*s = 0;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_confirm()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Print a confirm text and wait for confirm.
*
**************************************************************************/
static int	rtt_confirm( char	*text)
{
	unsigned long	option;
	unsigned long	terminator;
	char		input_str[80];
	char		message[120];
	int		maxlen = 1;

	/* Confirm */
	option = RTT_OPT_NOSCROLL;
	strcpy( message, text);
	strcat( message, "  (Yes:PF1/No:PF4) ? ");

	rtt_cursor_abs( 0, RTT_ROW_COMMAND);
	rtt_eofline_erase();
	rtt_command_get_input_string( (char *) rtt_chn, input_str, &terminator,
		maxlen, (rtt_t_recall *) rtt_recallbuff, option, rtt_scantime,
		rtt_scan, 0, message, RTT_COMMAND_VALUE);
	if (terminator != RTT_K_PF1)
	  return RTT__CONFABO;
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		tlog_qual_to_time()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Convert a commandline input time to VMS-time.
*
**************************************************************************/

static int	rtt_qual_to_time( 	char 		*in_str, 
					pwr_tTime 	*time)
{
	pwr_tStatus	sts;
	char		*s;
	pwr_tDeltaTime	one_day_time;
	pwr_tTime	current_time;
	char			str[64];
	char			timstr[64];


	if ( !strcmp( in_str, "") ||
	     !strncmp( in_str, "TODAY", strlen(in_str)))
	{
	  clock_gettime( CLOCK_REALTIME, &current_time);
	  time_AtoAscii( &current_time, time_eFormat_DateAndTime,
			timstr, sizeof(timstr));
	  timstr[12] = 0;
	  strcat( timstr, " 00:00:00.00");
	  sts = time_AsciiToA( timstr, time);
	}
	else if ( !strncmp( in_str, "YESTERDAY", strlen(in_str)))
	{
	  clock_gettime( CLOCK_REALTIME, &current_time);
	  time_AtoAscii( &current_time, time_eFormat_DateAndTime,
			timstr, sizeof(timstr));
	  timstr[12] = 0;
	  strcat( timstr, " 00:00:00.00");
	  sts = time_AsciiToA( timstr, &current_time);
	  strcpy( timstr, "1 00:00:00");
	  sts = time_AsciiToD( timstr, &one_day_time);
	  time_Dneg( &one_day_time, &one_day_time);
	  time_Aadd( time, &current_time, &one_day_time);
	}
	else
	{
	  strcpy( str, in_str);
	  if ( (s = strchr( str, '-')))
	  {
	    /* Date is supplied, replace ':' to space */
	    if ( (s = strchr( str, ':')))
	      *s = ' ';
	    strcpy( timstr, str);
	  }
	  else
	  {
	    /* No date is supplied, add current date as default */
	    clock_gettime( CLOCK_REALTIME, &current_time);
	    time_AtoAscii( &current_time, time_eFormat_DateAndTime,
			timstr, sizeof(timstr));
	    timstr[12] = 0;
 	    strcat( timstr, " ");
 	    strcat( timstr, str);
	  }
	  sts = time_AsciiToA( timstr, time);
	  if (EVEN(sts)) return sts;
	}
	return RTT__SUCCESS;
}
