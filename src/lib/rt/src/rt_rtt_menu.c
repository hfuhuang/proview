/* 
 * Proview   $Id: rt_rtt_menu.c,v 1.14 2008-06-25 07:50:14 claes Exp $
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

/* rt_rtt_menu.c 
   This module contains routines for handling of menues in rtt. */


#define RTT_MENU_MODULE
#define RTT_ISAREF 0x99990000

/*_Include files_________________________________________________________*/



#if defined(OS_ELN)
# include stdio
# include stdarg
# include stdlib
# include chfdef
# include signal
# include string
# include float
# include descrip
# include starlet
# include lib$routines
# include $vaxelnc
# include $kernelmsg
# include $elnmsg
# include $get_message_text
#elif defined(OS_VMS)
# include <stdio.h>
# include <stdarg.h>
# include <stdlib.h>
# include <chfdef.h>
# include <signal.h>
# include <string.h>
# include <float.h>
# include <descrip.h>
# include <starlet.h>
# include <lib$routines.h>
# include <processes.h>
#else
# include <stdarg.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <float.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "pwr_privilege.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"
#include "co_syi.h"
#include "co_msg.h"
#include "co_api_user.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "rt_mh.h"
#include "rt_mh_outunit.h"
#include "rt_rtt.h"
#include "rt_rtt_edit.h"
#include "rt_rtt_global.h"
#include "rt_rtt_functions.h"
#include "rt_rtt_dir.h"
#include "rt_rtt_msg.h"
#include "dtt_rttsys_functions.h"
#include "rt_ini_event.h"
#include "rt_qcom.h"
#include "rt_qcom_msg.h"


/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define RTT_HIDE_ELEMENTS	2

#define IF_NOGDH_RETURN \
if ( !rtt_gdh_started)\
{\
rtt_message('E',"Rtt is not connected to nethandler");\
return RTT__NOPICTURE;\
}

typedef struct {
	void		*key;
	void		*ctx;
	} rtt_t_store_menuctx;

/* Local static variables */

#define CTXLIST_SIZE 100

static menu_ctx		rtt_ctxlist[CTXLIST_SIZE];
static unsigned int	rtt_ctxlist_count = 0;
static pwr_tNodeId	rtt_nodidx;
static char		rtt_qual_str[10][2][80];
static char		rtt_print_buffer[2000];
static int		rtt_print_buffer_len;
static rtt_t_store_menuctx	*rtt_menuctx_store = 0;
static rtt_t_menu	*rtt_root_menu = 0;
static rtt_t_menu	*rtt_menu_deletebuf = 0;
static qcom_sQid	my_q = {0, 0};
static qcom_sGet	get;

static unsigned short state_table[10][256];

/*** Local function prototypes **********************************************/

static int	rtt_rttconfig();
static int	rtt_recall_insert( 	rtt_t_recall 	*recall,
					char		*command);
static int	rtt_recall_getcommand( 	rtt_t_recall 	*recall,
					int		nr,
					char		*command);
static int	rtt_menu_upd_value_koord( menu_ctx	ctx);
static int	rtt_edit_configure( 	menu_ctx	ctx);
static int	rtt_edit_draw_item( 	menu_ctx	ctx,
					int		item);
static int	rtt_menu_draw( menu_ctx	ctx);
static int	rtt_get_next_item_down_e( menu_ctx	ctx);
static int	rtt_get_next_item_up_e( menu_ctx	ctx);
static int	rtt_get_next_item_left_e( menu_ctx	ctx);
static int	rtt_get_next_item_right_e( menu_ctx	ctx);
static int	rtt_print_value(
			char		*value_ptr,
			int		value_type,
			int		flags,
			int		size,
			char		*old_value,
			unsigned long	init,
			int		x,
			int		y,
			unsigned long	priv);
static int	rtt_draw_bar(
			rtt_t_menu_upd	*menu_ptr,
			unsigned long	init);
static int	rtt_edit_print_value(
			rtt_t_menu_upd	*menu_ptr,
			unsigned long	init);
static int	rtt_objdid_parameter(
			menu_ctx	ctx,
			pwr_tObjid	objid,
			void		*arg1,
			void		*arg2,
			void 		*arg3,
			void 		*arg4);
static int	rtt_attribute_elements(
			menu_ctx	parent_ctx,
			pwr_tObjid	objid,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4);
static int	rtt_set_value(
			menu_ctx	ctx,
			char		*value_str);
static int	rtt_get_value(
			menu_ctx	ctx,
			int		timeout,
			int		(* timeout_func) (),
			void 		*timeout_arg,
			char		*prompt,
			int		x,
			int		y);
static int	rtt_help_getinfoline(
			char		*subject,
			rtt_t_helptext	*helptext,
			char		**infoline);
static int	rtt_logon( unsigned long *chn,
			   unsigned long	*priv,
			   char		*username,
			   char		*password);
static int	rtt_menu_new_update_add(
			menu_ctx	parent_ctx,
			rtt_t_menu_upd	**menulist,
			char		*title,
			int		(* func1) (),
			int		(* func2) (),
			char		*parameter_name,
			char		*dualparameter_name,
			unsigned long	priv,
			char		characters,
			char		decimals,
			float		maxlimit,
			float		minlimit,
			int		database,
			int		declaration,
			int		x,
			int		y,
			void		*userdata,
			unsigned long	flag,
			int		*index,
			int		item_count);
static int	rtt_RefObjectInfo (
			char	*parameter_name,
			char	**parameter_ptr);
static int	rtt_RttsysRefObjectInfo (
			char	*parameter_name,
			char	**parameter_ptr);
static int	rtt_store_menuctx( 
			void		*ctx,
			void		*key);
static int	rtt_get_stored_menuctx(
				void		**ctx,
				void		*key);
static int	rtt_get_system_name( char *system_name, int size);
static int	rtt_parse_mainmenu( char *mainmenu_title);
static int	rtt_help_show_all( 
			menu_ctx	parent_ctx,
			rtt_t_helptext	*helptext);


/*************************************************************************
*
* Name:		rtt_init_state_table()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Initialization of the stat_table used by rtt_get_input.
*
**************************************************************************/

int	rtt_init_state_table()
{
	state_table[0][RTT_K_RETURN] = RTT_TERM + RTT_K_RETURN;
	state_table[0][RTT_K_CTRLZ] = RTT_TERM + RTT_K_CTRLZ;
	state_table[0][RTT_K_CTRLV] = RTT_TERM + RTT_K_CTRLV;
	state_table[0][RTT_K_CTRLN] = RTT_TERM + RTT_K_CTRLN;
	state_table[0][RTT_K_CTRLC] = RTT_TERM + RTT_K_CTRLC;
	state_table[0][RTT_K_CTRLW] = RTT_TERM + RTT_K_CTRLW;
	state_table[0][RTT_K_CTRLK] = RTT_TERM + RTT_K_CTRLK;
	state_table[0][RTT_K_CTRLL] = RTT_TERM + RTT_K_CTRLL;
	state_table[0][RTT_K_DELETE] = RTT_TERM + RTT_K_DELETE;
	state_table[0][RTT_K_BACKSPACE] = RTT_TERM + RTT_K_BACKSPACE;
	state_table[0][RTT_K_CTRLB] = RTT_TERM + RTT_K_COMMAND;
	state_table[0][RTT_K_CTRLD] = RTT_TERM + RTT_K_PREVPAGE;
	state_table[0][RTT_K_CTRLF] = RTT_TERM + RTT_K_NEXTPAGE;
	state_table[0][RTT_K_CTRLH] = RTT_TERM + RTT_K_HELP;
	state_table[0][RTT_K_CTRLA] = RTT_TERM + RTT_K_PF1;
	state_table[0][RTT_K_CTRLT] = RTT_TERM + RTT_K_PF2;
	state_table[0][RTT_K_CTRLE] = RTT_TERM + RTT_K_PF3;
	state_table[0][RTT_K_CTRLR] = RTT_TERM + RTT_K_PF4;
	state_table[0][RTT_K_ESCAPE] = 1;
	state_table[0][155] = 2;
	state_table[0][143] = 3;
	state_table[1][91] = 2;		/* state ESCAPE */
	state_table[1][79] = 3;		/* state ESCAPE */
	state_table[1][42] = 9;		/* state ESCAPE */
	state_table[9][97] = RTT_TERM +  RTT_K_FAST_1;
	state_table[9][98] = RTT_TERM +  RTT_K_FAST_2;
	state_table[9][99] = RTT_TERM +  RTT_K_FAST_3;
	state_table[9][100] = RTT_TERM +  RTT_K_FAST_4;
	state_table[9][101] = RTT_TERM +  RTT_K_FAST_5;
	state_table[9][102] = RTT_TERM +  RTT_K_FAST_6;
	state_table[9][103] = RTT_TERM +  RTT_K_FAST_7;
	state_table[9][104] = RTT_TERM +  RTT_K_FAST_8;
	state_table[9][105] = RTT_TERM +  RTT_K_FAST_9;
	state_table[9][106] = RTT_TERM +  RTT_K_FAST_10;
	state_table[9][107] = RTT_TERM +  RTT_K_FAST_11;
	state_table[9][108] = RTT_TERM +  RTT_K_FAST_12;
	state_table[9][109] = RTT_TERM +  RTT_K_FAST_13;
	state_table[9][110] = RTT_TERM +  RTT_K_FAST_14;
	state_table[9][111] = RTT_TERM +  RTT_K_FAST_15;
	state_table[9][112] = RTT_TERM +  RTT_K_FAST_16;
	state_table[9][113] = RTT_TERM +  RTT_K_FAST_17;
	state_table[9][114] = RTT_TERM +  RTT_K_FAST_18;
	state_table[9][115] = RTT_TERM +  RTT_K_FAST_19;
	state_table[9][116] = RTT_TERM +  RTT_K_FAST_20;
	state_table[9][117] = RTT_TERM +  RTT_K_FAST_21;
	state_table[9][118] = RTT_TERM +  RTT_K_FAST_22;
	state_table[2][65] = RTT_TERM + RTT_K_ARROW_UP;
	state_table[2][66] = RTT_TERM + RTT_K_ARROW_DOWN;
	state_table[2][67] = RTT_TERM + RTT_K_ARROW_RIGHT;
	state_table[2][68] = RTT_TERM + RTT_K_ARROW_LEFT;
	state_table[2][53] = 4;
	state_table[2][54] = 5;
	state_table[2][50] = 6;
	state_table[3][80] = RTT_TERM + RTT_K_PF1;
	state_table[3][81] = RTT_TERM + RTT_K_PF2;
	state_table[3][82] = RTT_TERM + RTT_K_PF3;
	state_table[3][83] = RTT_TERM + RTT_K_PF4;
	state_table[4][126] = RTT_TERM + RTT_K_PREVPAGE;
	state_table[5][126] = RTT_TERM + RTT_K_NEXTPAGE;
	state_table[6][56] = 7;
	state_table[6][57] = 8;
	state_table[6][67] = RTT_TERM + RTT_K_SHIFT_ARROW_RIGHT;
	state_table[6][68] = RTT_TERM + RTT_K_SHIFT_ARROW_LEFT;
	state_table[7][126] = RTT_TERM + RTT_K_HELP;
	state_table[8][126] = RTT_TERM + RTT_K_COMMAND;
	return RTT__SUCCESS;
}		

/*************************************************************************
*
* Name:		rtt_initialize()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	This function initializes gdh and rtt.
*
**************************************************************************/

int	rtt_initialize( char	*username,
			char	*password,
			char	*commandfile,
			char	*mainmenu_title)
{
	int	sts;
	int	noneth = 0;

	rtt_init_state_table();

  	qio_assign( "stdin", (int *) &rtt_chn);

	if ( strcmp( username, "NONETH_SYS") == 0) {
	  noneth = 1;
	  rtt_priv = RTT_PRV_SYS;
	}

	if ( !noneth) {
	  sts = rtt_gdh_init();

	  rtt_logon( rtt_chn, &rtt_priv, username, password);

	  if ( rtt_gdh_started)
	    sts = rtt_rttconfig();

          if (!qcom_CreateQ(&sts, &my_q, NULL, "events")) {
            exit(sts);
          }
          if (!qcom_Bind(&sts, &my_q, &qcom_cQini)) {
            exit(-1);
          }
	}

	sts = rtt_recall_create( &rtt_recallbuff);
	if (EVEN(sts)) return sts;
	sts = rtt_recall_create( &rtt_value_recallbuff);
	if (EVEN(sts)) return sts;

	if ( rtt_AlarmAutoLoad) {
	  /* Load alarm list */
	  sts  = rtt_alarm_connect( rtt_UserObject, 0, 0, rtt_AlarmAck,
			rtt_AlarmReturn, rtt_AlarmBeep);
	  if ( EVEN(sts))
	    rtt_message('E', "Unable to connect to alarm handler");
	}

	/* Init som global variables */
	sts = rtt_get_platform( rtt_platform);
	sts = rtt_get_hw( rtt_hw);
	sts = rtt_get_nodename( rtt_node, sizeof(rtt_node));
	if ( rtt_gdh_started)
	  sts = rtt_get_system_name( rtt_sys, sizeof(rtt_sys));
#ifdef OS_VMS
	{
	  unsigned long		ident;
	  sts = rttvms_get_uic( (int *)&ident);
          sts = rttvms_get_identname( ident, rtt_ident);
	}
#endif

	rtt_parse_mainmenu( mainmenu_title);

	if ( *commandfile != 0) {
	  if ( rtt_args >= 5 && !strcmp( rtt_arg[4], "EXIT"))
	    sts = rtt_commandmode_start( commandfile, 1);
	  else
	    sts = rtt_commandmode_start( commandfile, 0);

	  if ( sts == RTT__NOFILE) {
	    rtt_message('E',"Unable to open file");
	  }
	}

	return RTT__SUCCESS;

}
/*************************************************************************
*
* Name:		rtt_gdh_init()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	This function initializes gdh for rtt.
*
**************************************************************************/

int	rtt_gdh_init()
{
	int	sts;

	if ( rtt_gdh_started )
	  return RTT__SUCCESS;

	sts = gdh_Init("rt_rtt");
	if (EVEN(sts)) return sts;

 	sts = gdh_GetNodeIndex( &rtt_nodidx);
	if (EVEN(sts)) return sts;

	rtt_gdh_started = 1;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_configure()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	This function configures rtt.
*
**************************************************************************/

static int	rtt_rttconfig()
{
	pwr_tObjid	objid;
	pwr_tOName     	hiername;
	int		found;
	int		sts;
	pwr_tFileName  	filename;
	pwr_sClass_OpPlace *opplace_p;

	if ( rtt_ConfigureObject[0])
	{
	  strcpy( hiername, rtt_ConfigureObject);
	  sts = gdh_NameToObjid ( hiername, &objid);
	  if ( EVEN(sts)) return sts;
	}
	else
	{
	  // Look for default opplace
	  pwr_tOid oid;
	  pwr_tOName name;

	  found = 0;
	  for ( sts = gdh_GetClassList( pwr_cClass_OpPlace, &oid); 
		ODD(sts);
		sts = gdh_GetNextObject( oid, &oid)) {
	    sts = gdh_ObjidToName( oid, name, sizeof(name), cdh_mName_object);
	    if ( ODD(sts) && cdh_NoCaseStrcmp( name, "opdefault") == 0) {
	      sts = gdh_ObjidToName( oid, name, sizeof(name), cdh_mNName);
	      if ( EVEN(sts)) exit(sts);

	      strncpy( rtt_ConfigureObject, name, sizeof(rtt_ConfigureObject));
	      found = 1;
	      break;
	    }
	  }
	  if ( found == 0)
	    return RTT__OBJNOTFOUND;
	}

	sts = gdh_NameToPointer( rtt_ConfigureObject, (void **)&opplace_p);
	if ( EVEN(sts)) return sts;

	sts = gdh_NameToObjid( rtt_ConfigureObject, &rtt_UserObject);
	if ( EVEN(sts)) return sts;

	rtt_AlarmBeep = opplace_p->AlarmBell;
	rtt_AlarmReturn = (opplace_p->EventListEvents & pwr_mEventListMask_AlarmReturn) != 0;
	rtt_AlarmAck = (opplace_p->EventListEvents & pwr_mEventListMask_AlarmAck) != 0;
	strncpy( rtt_symbolfilename, opplace_p->SetupScript, sizeof(rtt_symbolfilename));

	/* Execute the symbolfile */
	if ( strcmp(rtt_symbolfilename, "") != 0)
	{
	  rtt_get_defaultfilename( rtt_symbolfilename, filename, ".rtt_com");
	  sts = rtt_commandmode_start( filename, 0);
	  if ( sts == RTT__NOFILE)
	    rtt_message('E',"Unable to open symbolfile");
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_parse()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*string		I	string to be parsed.
* char		*parse_char	I	parse charachter(s).
* char		*inc_parse_char	I	parse charachter(s) that will be
*					included in the parsed string.
* char		*outstr		O	parsed strings.
* int		max_rows	I	maximum number of chars in a parsed
*					string.
* int 		max_cols	I	maximum number of parsed elements.
*
* Description:
*	Parses a string.
*
**************************************************************************/

int	rtt_parse( 	char	*string,
			char	*parse_char,
			char	*inc_parse_char,
			char	*outstr,
			int	max_rows,
			int 	max_cols,
			int	keep_quota)
{
	int	row;
	int	col;
	char	*char_ptr;	
	char	*inc_char_ptr;	
	int	parsechar_found;
	int	inc_parsechar_found;
	int	next_token;
	int	char_found;
	int	one_token = 0;
	unsigned char	prev_char;
	int	nullstr;

	prev_char = 0;
	row = 0;
	col = 0;
	char_found = 0;
	next_token = 0;
	nullstr = 0;
	while ( *string != '\0')	
	{
	  char_ptr = parse_char;
	  inc_char_ptr = inc_parse_char;
	  parsechar_found = 0;
	  inc_parsechar_found = 0;
	  if ( *string == '"' && prev_char != '\\')
	  {
	    one_token = !one_token;
	    prev_char = (unsigned char) *string;
 	    if ( !one_token && col == 0)
 	      nullstr = 1;
	    else
	      nullstr = 0;
	    if ( !keep_quota)
	    {
	      string++; 
	      continue;
	    }
	  }
	  else if ( *string == '"' && prev_char == '\\')
	    col--;
	  if ( !one_token)
	  {
	    while ( *char_ptr != '\0')
	    {
	      /* Check if this is a parse charachter */
	      if ( *string == *char_ptr)
	      {
	        parsechar_found = 1;
	        /* Next token */
	        if ( col > 0)	
	        {
	          *(outstr + row * max_cols + col) = '\0';
	          row++;
	          if ( row >= max_rows )
	            return row;
	          col = 0;
	          next_token = 0;
	        }
	        break;
	      }
	      char_ptr++;	    
	    }
	    while ( *inc_char_ptr != '\0')
	    {
	      /* Check if this is a parse charachter */
	      if ( *string == *inc_char_ptr)
	      {
	        parsechar_found = 1;
	        inc_parsechar_found = 1;
	        /* Next token */
	        if ( col > 0 || nullstr)	
	        {
	          *(outstr + row * max_cols + col) = '\0';
	          row++;
	          if ( row >= max_rows )
	            return row;
	          col = 0;
	          next_token = 0;
	        }
	        break;
	      }
	      inc_char_ptr++;	    
	    }
	  }
	  if ( !parsechar_found && !next_token)
	  {
	    char_found++;
	    *(outstr + row * max_cols + col) = *string;
	    col++;
	  }
	  if ( inc_parsechar_found)
	  {
	    *(outstr + row * max_cols + col) = *inc_char_ptr;
	    col++;
	  }
	  prev_char = (unsigned char) *string;
	  string++; 
	  if ( col >= (max_cols - 1))
	    next_token = 1;	    
	  nullstr = 0;
	}
	*(outstr + row * max_cols + col) = '\0';
	row++;

	if ( char_found == 0)
	  return 0;

	return row;
}


/*************************************************************************
*
* Name:		rtt_cli()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_comtbl	*command_table	I	command table.
* char		*string		I	command string.
* unsigned long	userdata	I	argument passed to the command
*					function.
*
* Description:
*	This function identifies a command described int the command table
*	in the command string and calls the command function for this
*	command with the userdata as argument.
*	Qualifiers and parameters are stored and can be fetched by
*	rtt_get_qualifier.
*
**************************************************************************/

int	rtt_cli( 	rtt_t_comtbl	*command_table,
			char		*string,
			void		*userdata1,
			void		*userdata2)
{
#define CLI_SUBST_SLASH 3
#define CLI_SUBST_EQUAL 4

	char	out_str[20][160];
	char	value_str[10][160];
	int	nr, i, j, valuenr;
	int	hitnr, sts;
	char	command[160];
	int	(*func) ();
	rtt_t_comtbl	*comtbl_ptr;
	rtt_t_comtbl	*current_comtbl;
	int		arg_count;
	char	qual[80];
	char	*s, *t;
	int	is_arg;
	int	quota_mode;


	/* Fill spaces around '=' with '=' , this is a trick to avoid
	   parsing of the qualifier and value in the first step */
	s = string;
	quota_mode = 0;
	while ( *s != 0)
	{
	  if ( *s == '"')
	    quota_mode = !quota_mode;
	  if ( quota_mode)
	  {
	    /* Replace / and = to avoid parsing */
	    if ( *s == '/')
	      *s = CLI_SUBST_SLASH;
	    if ( *s == '=')
	      *s = CLI_SUBST_EQUAL;
	    s++;
	    continue;
	  }
	  if ( *s == '=')
	  {
	    t = s - 1;
	    while ( (*t == ' ') && (*t != 0))
	    {
	      *t = '=';	
	      t--;
	    }
	    t = s + 1; 
	    while ( (*t == ' ') && (*t != 0))
	    {
	      *t = '=';	
	      t++;
	    }
	    s = t - 1;	   
	  }
	  s++;
	}


	/* Parse the command string */
	nr = rtt_parse( string, " ", "/", (char *)out_str, 
		sizeof( out_str) / sizeof( out_str[0]), sizeof( out_str[0]), 0);

	if ( nr == 0)
	  return RTT__NOCOMMAND;

	/* Find the command in the command table */
	comtbl_ptr = command_table;
	hitnr = 0;
	while ( comtbl_ptr->command[0] != '\0')
	{
	  strcpy( command, comtbl_ptr->command);
	  if ( strcmp( out_str[0], command) == 0)
	  {
	    /* Perfect hit */
	    func = comtbl_ptr->func;
	    hitnr = 1;
	    current_comtbl = comtbl_ptr;
	    break;
	  }
	  else
	  {
	    command[ strlen( out_str[0])] = '\0';
	    if ( strcmp( out_str[0], command) == 0)
	    {
	      /* Hit */
	      func = comtbl_ptr->func;
	      hitnr++;
	      current_comtbl = comtbl_ptr;
	    }
	  }
	  comtbl_ptr++;
	}	   

	if ( hitnr > 1)
	{
	  /* Command not unic */
	  return RTT__COM_AMBIG;
	}
	else if ( hitnr < 1)
	{
	  /* Command not defined */
	  return RTT__COM_NODEF;
	}

	/* Identify the qualifiers */
	arg_count = 0;
	for ( i = 1; i < nr; i++)
	{
	  valuenr = rtt_parse( (char *)out_str[i], "=", "",
		(char *) value_str, sizeof( value_str)/sizeof( value_str[0]), 
		sizeof( value_str[0]), 1);
	  if ( valuenr > 1)
	  {
	    strcpy( rtt_qual_str[i-1][1], value_str[1]);
	    for ( j = 2; j < valuenr; j++)
	    {
	      strcat( rtt_qual_str[i-1][1], "=");	  
	      strcat( rtt_qual_str[i-1][1], value_str[j]);	  
	    }
	  }
	  else
	    strcpy( rtt_qual_str[i-1][1], "");	  

	  /* Check if this qualifier is ok */
	  j = 0;
	  hitnr = 0;
	  is_arg = 0;
	  if ( value_str[0][0] == 0 )
	    /* Null string sent as an argument */
	    is_arg = 1;
	  else
          {
	    while ( current_comtbl->qualifier[j][0] != 0)
	    {	  
	      strcpy( qual, current_comtbl->qualifier[j]);
	      qual[ strlen( value_str[0])] = '\0';
	      if ( strcmp( qual, value_str[0]) == 0)
	      {
	        /* Hit */
	        strcpy( rtt_qual_str[i-1][0], current_comtbl->qualifier[j]);
	        hitnr++;
	      }
	      j++;
	    }
	  }
	  if ( hitnr == 0 || is_arg)
	  {
	    /* This might be a argument, look for a argument */
	    if ( strncmp( current_comtbl->qualifier[arg_count], 
			"rtt_arg", 7) == 0)
	    {	  
	      sprintf( rtt_qual_str[i-1][0], "rtt_arg%d", arg_count+1);	
	      strcpy( rtt_qual_str[i-1][1], value_str[0]); 
	      arg_count++;
	    }
	    else
	    {
	      /* qualifier not found */
	      return RTT__QUAL_NODEF;
	    }
	  }
	  else if ( hitnr > 1)
	  {
	    /* qualifier not unic */
	    return RTT__QUAL_AMBIG;
	  }
	  /* Place back the / and = within quotes */
	  for ( s = rtt_qual_str[i-1][1]; *s != 0; s++)
	  {
	    if ( *s == CLI_SUBST_SLASH)
	      *s = '/';
	    if ( *s == CLI_SUBST_EQUAL)
	      *s = '=';
	  }
	}
	strcpy( rtt_qual_str[nr-1][0], "");

	sts = (func) (userdata1, userdata2);
	return sts;
}

/*************************************************************************
*
* Name:		rtt_get_qualifier()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*qualifier	I	
* char		*value		O	
*
* Description:
*	Returns the value of a specified qualifier.
*	If no value is found a NULL string is returned.
*	If the qualifier is not found RTT__QUAL_NOFOUND is returned as
*	statuscode.
*
**************************************************************************/

int	rtt_get_qualifier( 	char	*qualifier,
				char	*value)
{
	int	i, found;

	i = 0;	
	found = 0;
	while ( rtt_qual_str[i][0][0] != '\0')
	{
	  if ( strcmp( qualifier, (char *) rtt_qual_str[i]) == 0)
	  {
	    /* Hit */
	    strcpy( value, (char *) &rtt_qual_str[i][1]);
	    found = 1;
	  }
	  i++;
	}	   
	if ( !found)
	{
	  /* qualifier is not found */
	  *value = 0;
	  return RTT__QUAL_NOFOUND;
	}
	  
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_recall_create()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_recall **recall		O	recall buffer.
*
* Description:
*	Create a recall buffer.	
*		
**************************************************************************/

int	rtt_recall_create( rtt_t_recall **recall)
{
	*recall = calloc( 1, sizeof(rtt_t_recall)); 
	if ( *recall == 0)
	  return RTT__NOMEMORY;
	(*recall)->first_command = -1;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_recall_insert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_recall 	*recall		I	recall buffer.
* char		*command	I	string to insert in recall.
*
* Description:
*	Inserts a command in the recall buffer.
*
**************************************************************************/

static int	rtt_recall_insert( 	rtt_t_recall 	*recall,
					char		*command)
{
	if ( *command == 0)
	  return RTT__SUCCESS;
	if ( strcmp( (char *) recall->command[recall->last_command], command) == 0)
	  return RTT__SUCCESS;
	
	recall->last_command++;
	if ( recall->last_command >= RTT_RECALL_MAX)
	  recall->last_command = 0;
	strncpy( &(recall->command[recall->last_command][0]), command, 200);
	if ( recall->first_command == recall->last_command)
	  recall->first_command++;
	if ( recall->first_command > RTT_RECALL_MAX)
	  recall->first_command = 0;
	if ( recall->first_command == -1)
	  recall->first_command = 0;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_recall_getcommand()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_recall 	*recall		I	recall buffer.
* int		nr		I	index of returned command.
* char		*command	O	command.
*
* Description:
*	Returns a command from the recall buffer.
*
**************************************************************************/

static int	rtt_recall_getcommand( 	rtt_t_recall 	*recall,
					int		nr,
					char		*command)
{
	int	index;
	
	if ( (nr >= RTT_RECALL_MAX) || ( nr < 0))
	{
	  *command = 0;
	  return 1;
	}
	index = recall->last_command - nr;
	if ( index < 0)
	  index += RTT_RECALL_MAX; 
	strcpy( command, &(recall->command[index][0]));	
	return 1;
}

/*************************************************************************
*
* Name:		rtt_get_input()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*chn		I	channel.
* char		*input_str	O	input string.
* unsigned long	*terminator	O	terminator.
* int		maxlen		I	max nr of charachters
* unsigned long	option		I	option mask.
* int		timeout		I	timeout time.
*
* Description:
*	Reads a input string from a terminal.
*
**************************************************************************/

int	rtt_get_input( 	char		*chn,
			char		*input_str,
			unsigned long	*terminator,
			int		maxlen,
			unsigned long	option,
			int		timeout)
{
	unsigned char	c;
	char	*input_ptr;
	int	i;
	int	sts;
	int	state;

	  
	input_ptr = input_str;

	for ( i = 0; i < maxlen; i++)
	{
	  if ( (option & RTT_OPT_TIMEOUT) == 0)
	    qio_readw( (int *) chn, (char *) &c, 1);
	  else
	  {
	    sts = qio_read( (int *) chn, timeout, (char *) &c, 1);
	    if ( !sts)
	    {
	      /* Timeout */
	      *terminator = RTT_K_TIMEOUT;
	      *input_ptr = '\0';
	      return 1;
	    }
	  }	   
 

	  state = RTT_TERM;
	  while ( state > 0 )
	  {
	    if ( state == RTT_TERM)
	      /* first time */
	      state = 0;

	    state = state_table[ state ][ c ];	  
	    if ( state > RTT_TERM )
	    {
	      *terminator = state - RTT_TERM;
	      switch ( *terminator)
	      {
	        case RTT_K_RETURN:
	        case RTT_K_CTRLC:
	        case RTT_K_CTRLZ:
	        {
	          *input_ptr = '\0';
	          if ( ((option & RTT_OPT_NOECHO) == 0) &&
		       ((option & RTT_OPT_NOSCROLL) == 0)) 
	            rtt_printf("\n\r");
	          return 1;
	        }	        
	        default:
	        {
	          *input_ptr = '\0';
	          return 1;
	        }	        
	      }
	    }
	    else if ( state > 0)
	      qio_readw( (int *) chn, (char *) &c, 1);
	  }

	  if ( c > 31)
	  {
	    /* Some ordinary charachter */
	    *input_ptr = c;
	    input_ptr++;
	    if ( (option & RTT_OPT_NOECHO) == 0)
	    {
	      rtt_char_insert_nob( 1);
/*	      putchar( c); */
	      rtt_printf( "%c", c);
	    }
	  }
	}
	*terminator = RTT_K_MAXLEN;
	*input_ptr = '\0';
	if ( (option & RTT_OPT_NOECHO) == 0)
	  rtt_printf("\n\r");
	return 1;
}

/*************************************************************************
*
* Name:		rtt_cursor_rel()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* int		x		I	
* int		y		I	
*
* Description:
*	Positions the cursor x and y relativ to current position.
*
**************************************************************************/

void	rtt_cursor_rel( 	int	x,
				int	y)
{
	char	cursor_f[]={27, 91, 0, 0, 67, 0};
	char	cursor_b[]={27, 91, 0, 0, 68, 0};
	char	cursor_u[]={27, 91, 0, 0, 65, 0};
	char	cursor_d[]={27, 91, 0, 0, 66, 0};
	
	if ( y > 0 )
	{
	  cursor_f[2] = y / 10 + 48;
	  cursor_f[3] = y - (y / 10) * 10 + 48;
	  r_print("%s", cursor_f);
	}
	else if ( y < 0 )
	{
	  cursor_b[2] = - y / 10 + 48;
	  cursor_b[3] = - y + (y / 10) * 10 + 48;
	  r_print("%s", cursor_b);
	}
	if ( x > 0 )
	{
	  cursor_u[2] = x / 10 + 48;
	  cursor_u[3] = x - (x / 10) * 10 + 48;
	  r_print("%s", cursor_u);
	}
	else if ( x < 0 )
	{
	  cursor_d[2] = - x / 10 + 48;
	  cursor_d[3] = - x + (x / 10) * 10 + 48;
	  r_print("%s", cursor_d);
	}
}
/*************************************************************************
*
* Name:		rtt_cursor_abs()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* int		x		I	
* int		y		I	
*
* Description:
*	Positions the cursor at x, y.
*
**************************************************************************/

void	rtt_cursor_abs( 	int	x,
				int	y)
{
	char	cursor_a[]={27, 91, 0, 0, 59, 0, 0, 102, 0};
	
	cursor_a[5] = x / 10 + 48;
	cursor_a[6] = x - (x / 10) * 10 + 48;
	cursor_a[2] = y / 10 + 48;
	cursor_a[3] = y - (y / 10) * 10 + 48;
	r_print("%s", cursor_a);
}

void	rtt_cursor_abs_force( 	int	x,
				int	y)
{
	char	cursor_a[]={27, 91, 0, 0, 59, 0, 0, 102, 0};
	int	sts;	

	cursor_a[5] = x / 10 + 48;
	cursor_a[6] = x - (x / 10) * 10 + 48;
	cursor_a[2] = y / 10 + 48;
	cursor_a[3] = y - (y / 10) * 10 + 48;
	printf("%s", cursor_a);
	sts = qio_writew( (int *) rtt_chn, cursor_a,
			strlen(cursor_a));
}

/*************************************************************************
*
* Name:		rtt_char_delete()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* int		n		I	
*
* Description:
*	Character delete.
*
**************************************************************************/

void	rtt_char_delete( int	n)
{
	char	char_del[5]={27, 91, 0, 80, 0};
	
	char_del[2] = n;
	r_print("%s", char_del);
}
/*************************************************************************
*
* Name:		rtt_char_insert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* int		n		I	char to insert.
*
* Description:
*	Charachter insert
*
**************************************************************************/

void	rtt_char_insert( int n)
{
	char	char_ins[]={27, 91, 0, 0, 64, 0};
	
	char_ins[2] = n / 10 + 48;
	char_ins[3] = n - (n / 10) * 10 + 48;
	r_print("%s", char_ins);
}
void	rtt_char_insert_nob( int	n)
{
	char	char_ins[]={27, 91, 0, 0, 64, 0};
	
	char_ins[2] = n / 10 + 48;
	char_ins[3] = n - (n / 10) * 10 + 48;
	rtt_printf("%s", char_ins);
}
/*************************************************************************
*
* Name:		rtt_eofline_erase()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Erase end of line.
*
**************************************************************************/

void	rtt_eofline_erase()
{
	char	char_ins[]={27, 91, 48, 75, 0};

	r_print("%s", char_ins);
}
/*************************************************************************
*
* Name:		rtt_display_erase()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Erase the screen.
*
**************************************************************************/

void	rtt_display_erase()
{
	char	char_ins[]={27, 91, 50, 74, 0};
	
	r_print("%s", char_ins);
}
/*************************************************************************
*
* Name:		rtt_print_screen()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Print the screen.
*
**************************************************************************/

void	rtt_print_screen()
{
	char	char_ins[]={27, 91, 48, 105, 0};
	
	r_print("%s", char_ins);
}
/*************************************************************************
*
* Name:		rtt_char_inverse_start()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Invers mode on.
*
**************************************************************************/

void	rtt_char_inverse_start()
{
	char	char_inv[]={27, 91, 55, 109, 0};
	
	r_print("%s", char_inv);
}
/*************************************************************************
*
* Name:		rtt_char_inverse_end()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Invers mod off.
*
**************************************************************************/

void	rtt_char_inverse_end()
{
	char	char_inv[]={27, 91, 50, 55, 109, 0};
	
	r_print("%s", char_inv);
}
/*************************************************************************
*
* Name:		rtt_store_cursorpos()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Store the current cursor position.
*
**************************************************************************/

void	rtt_store_cursorpos()
{
	char	char_ins[]={27, 55, 0};
	
	r_print("%s", char_ins);
}
/*************************************************************************
*
* Name:		rtt_restore_cursorpos()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Place the cursor at the previously stored cursor position.
*
**************************************************************************/

void	rtt_restore_cursorpos()
{
	char	char_ins[]={27, 56, 0};
	
	r_print("%s", char_ins);
}

/*************************************************************************
*
* Name:		rtt_charset_ascii()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Set ascii charachter set.
*
**************************************************************************/

void	rtt_charset_ascii()
{
	char	char_ins[]={27, 40, 66, 0};
	r_print("%s", char_ins);
}
/*************************************************************************
*
* Name:		rtt_charset_linedrawing()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Set ascii charachter set.
*
**************************************************************************/

void	rtt_charset_linedrawing()
{
	char	char_ins[]={27, 40, 48, 0};
	r_print("%s", char_ins);
}
/*************************************************************************
*
* Name:		rtt_charset_mosaic()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Set ascii charachter set.
*
**************************************************************************/

void	rtt_charset_mosaic()
{
	char	char_ins[]={27,78, 0};
	r_print("%s", char_ins);
}
/*************************************************************************
*
* Name:		rtt_get_input_string()
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
* unsigned long	timeout_arg	I	timeout function argument
* char		*prompt		I	prompt string.
*
* Description:
*	Read a input string.
*
**************************************************************************/

int	rtt_get_input_string( 	char		*chn,
				char		*out_string,
				unsigned long	*out_terminator,
				int		out_maxlen,
				rtt_t_recall 	*recall,
				unsigned long	option,
				int		timeout,
				int		(* timeout_func) (),
				void		*timeout_arg,
				char		*prompt)
{
	char		input_str[200];
	char		out_str[200];
	char		dum_str[200];
	int		maxlen = 199;
	unsigned long	terminator;
	int		index;
	int		recall_index = 0;
	
	if ( prompt != NULL)
	  r_print("%s", prompt);

	terminator = 0;
	index = 0;
	out_str[0] = 0;
	while ( 1)
	{	
	  r_print_buffer();
	  rtt_get_input( chn, input_str, &terminator, maxlen, option, timeout);

	  if (terminator == RTT_K_RETURN)
	    break;
	  if ((terminator == RTT_K_FAST_1) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_2) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_3) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_4) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_5) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_6) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_7) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_8) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_9) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_10) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_11) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_12) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_13) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_14) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_15) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_16) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_17) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_18) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_19) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_20) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_21) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_FAST_22) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_ARROW_LEFT) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_ARROW_RIGHT) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_SHIFT_ARROW_LEFT) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_SHIFT_ARROW_RIGHT) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_DELETE) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_BACKSPACE) && ((option & RTT_OPT_NOEDIT) != 0))
	    break;
	  if ((terminator == RTT_K_ARROW_UP) && ((option & RTT_OPT_NORECALL) != 0))
	    break;
	  if ((terminator == RTT_K_ARROW_DOWN) && ((option & RTT_OPT_NORECALL) != 0))
	    break;
	  if ((terminator == RTT_K_PF1) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_PF2) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_PF3) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_CTRLZ) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_CTRLW) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_CTRLV) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_CTRLN) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_CTRLK) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_CTRLL) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_PF4) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_PREVPAGE) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_NEXTPAGE) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_COMMAND) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_HELP) && ((option & RTT_OPT_NOPFTAN) == 0))
	    break;
	  if ((terminator == RTT_K_TIMEOUT) && ((option & RTT_OPT_NOEDIT) != 0))
	  {
	    if ( timeout_func != NULL)
	      (timeout_func) ( timeout_arg);
	    else
	      break;
	  }

	  if ( (option & RTT_OPT_NOEDIT) == 0)
	  {
	    switch ( terminator) 
	    {
	      case RTT_K_TIMEOUT:
	        strcpy( dum_str, (char *) &out_str[index]);
	        strcpy( (char *) &out_str[index], input_str);
	        index += strlen(input_str);
	        strcpy( (char *) &out_str[index], dum_str);
	        if ( timeout_func != NULL)
	        {
	 	  rtt_store_cursorpos();
	          (timeout_func) ( timeout_arg);
		  rtt_restore_cursorpos();
	        }
	        break;
	      case RTT_K_ARROW_LEFT:
	        strcpy( dum_str, (char *) &out_str[index]);
	        strcpy( &out_str[index], input_str);
	        index += strlen(input_str);
	        strcpy( &out_str[index], dum_str);
	        if ( index > 0)
	        {
	          index--;
	          rtt_cursor_rel( 0, -1);
	        }
	        break;
	      case RTT_K_ARROW_RIGHT:
	        strcpy( dum_str, (char *) &out_str[index]);
	        strncpy( (char *) &out_str[index], input_str, strlen(input_str));
	        index += strlen(input_str);
	        strcpy( (char *) &out_str[index], dum_str);
	        if ( index < (int)strlen( out_str))
	        {
	          index++;
	          rtt_cursor_rel( 0, 1);
	        }
	        break;
	      case RTT_K_BACKSPACE:
	        strcpy( dum_str, (char *) &out_str[index]);
	        strncpy( &out_str[index], input_str, strlen(input_str));
	        index += strlen(input_str);
	        strcpy( &out_str[index], dum_str);
	        if ( index > 0)
	        {
	          rtt_cursor_rel( 0, - index);
	          index = 0;
	        }
	        break;
	      case RTT_K_DELETE:
	        strcpy( dum_str, (char *) &out_str[index]);
	        strncpy( &out_str[index], input_str, strlen(input_str));
	        index += strlen(input_str);
	        strcpy( &out_str[index], dum_str);
	        if ( index > 0)
	        {
	          strcpy( dum_str, &out_str[index]);
	          index--;
	          rtt_cursor_rel( 0, -1);
	          strcpy( &out_str[index], dum_str);
	          rtt_char_delete( 1);

  	        }
	        break;
	    }
	  }
	  if ( (option & RTT_OPT_NORECALL) == 0)
	  {
	    switch ( terminator) 
	    {
	      case RTT_K_ARROW_UP:
	        index += strlen(input_str);
	        recall_index++;
	        if ( recall_index > RTT_RECALL_MAX)
	          recall_index = RTT_RECALL_MAX + 1;
	        rtt_recall_getcommand( recall, recall_index - 1, out_str);
	        rtt_cursor_rel( 0, -index);
	        rtt_eofline_erase();
	        index = strlen(out_str);
	        r_print("%s", out_str);
	        break;
	      case RTT_K_ARROW_DOWN:
	        index += strlen(input_str);
	        recall_index--;
	        if ( recall_index < 0)
	          recall_index = 0;
	        rtt_recall_getcommand( recall, recall_index - 1, out_str);
	        rtt_cursor_rel( 0, -index);
	        rtt_eofline_erase();
	        index = strlen( out_str);
	        r_print("%s", out_str);
	        break;
	    }
	  }
	}
	strcpy( dum_str, (char *) &out_str[index]);
	strncpy( &out_str[index], input_str, strlen(input_str));
	index += strlen(input_str);
	strcpy( &out_str[index], dum_str);
	strcpy( out_string, out_str);
	if ( (option & RTT_OPT_NORECALL) == 0)
	{	
	  /* Save in recall buffer */
	  recall_index = 0;
	  rtt_recall_insert( recall, out_string);
	}
	*out_terminator = terminator;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_create_ctx()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	*ctx		O	
* menu_ctx	parent_ctx	I	
* rtt_t_menu	*menu		I	
* char		*title		I	
* int		menutype	I	
*
* Description:
*	Creates a rtt context.
*
**************************************************************************/

int	rtt_menu_create_ctx( 
			menu_ctx	*ctx,
			menu_ctx	parent_ctx,
			rtt_t_menu	*menu,
			char		*title,
			int		menutype)
{
	*ctx = calloc(  1, sizeof(rtt_t_menu_ctx));
	if ( *ctx == 0)
	  return RTT__NOMEMORY;
	(*ctx)->parent_ctx = (void *) parent_ctx;
	(*ctx)->menu = menu;
	(*ctx)->menutype = menutype;
	cdh_StrncpyCutOff( (*ctx)->title, title, 80, 1); 

	if ( rtt_collectionmenulist && 
	     (rtt_t_menu_upd *) ((*ctx)->menu) == rtt_collectionmenulist)
	{
	  /* Store the context if this is the collection picture */
	  rtt_collectionmenuctx = *ctx;
	}

	rtt_ctx_push( *ctx);

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_menu_configure()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Configures a menu.
*	Calculates number of pages, rows, columns etc.
*
**************************************************************************/

int	rtt_menu_configure( menu_ctx	ctx)
{
	rtt_t_menu	*menu_ptr;
	int		item_maxsize;
	int		item_size;

	/* Calculate no of items */
	menu_ptr = ctx->menu;
	ctx->no_items = 0;
	item_maxsize = 0;

	while ( menu_ptr->text[0] != '\0' )
	{
	  ctx->no_items++;
	  item_size = strlen( menu_ptr->text);
	  if ( item_size > item_maxsize)
	    item_maxsize = item_size; 
	  menu_ptr++;
	}

	item_maxsize = min( item_maxsize, RTT_MENU_MAXCOLS);
	ctx->max_item_size = item_maxsize;

	/* Calculate rows and columns and pages */
	if ( (ctx->no_items * 2) < RTT_MENU_MAXROWS )
	{
	  ctx->no_pages = 1;
	  ctx->rows = ctx->no_items;
	  ctx->cols = 1;
	  ctx->page_len = ctx->no_items;
	  ctx->row_size = 2;	
	  ctx->col_size = item_maxsize + 2;	
	  ctx->left_margin = (RTT_MENU_MAXCOLS - item_maxsize) / 2; 
	  ctx->up_margin = 
		(RTT_MENU_MAXROWS - ctx->no_items * ctx->row_size ) / 2;
	}	  
	else if ( (RTT_MENU_MAXCOLS / (item_maxsize + 2) * RTT_MENU_MAXROWS) >
		ctx->no_items)
	{
	  /* Just one page */
	  ctx->no_pages = 1;
	  ctx->cols = (ctx->no_items - 1) / RTT_MENU_MAXROWS + 1;
	  ctx->page_len = ctx->no_items;
	  ctx->row_size = 1;	
	  ctx->rows = ctx->no_items / ctx->cols + 1;
	  ctx->col_size = (RTT_MENU_MAXCOLS / ctx->cols + item_maxsize + 2)/2;
	  ctx->left_margin = (RTT_MENU_MAXCOLS - ctx->col_size * ctx->cols)/2;
	  ctx->left_margin = max( ctx->left_margin, 2);
	  ctx->up_margin = (RTT_MENU_MAXROWS - ctx->rows ) / 2;
	}	  
	else
	{
	  /* Just one page */
	  ctx->page_len = max( RTT_MENU_MAXCOLS / (item_maxsize + 2), 1)
		* RTT_MENU_MAXROWS;
	  ctx->no_pages = (ctx->no_items - 1) / ctx->page_len + 1;
	  ctx->cols = ctx->page_len / RTT_MENU_MAXROWS;
	  ctx->row_size = 1;	
	  ctx->rows = (ctx->page_len - 1)/ ctx->cols + 1;
	  ctx->col_size = (RTT_MENU_MAXCOLS / ctx->cols + item_maxsize + 2)/2;
	  ctx->left_margin = (RTT_MENU_MAXCOLS - ctx->col_size * ctx->cols)/2;
	  ctx->left_margin = max( ctx->left_margin, 2);
	  ctx->up_margin = (RTT_MENU_MAXROWS - ctx->rows ) / 2;
	}	  

	ctx->current_page = 0;
	ctx->current_item = 0;
	ctx->current_row = 0;
	ctx->current_col = 0;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_upd_value_koord()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Calculates the coordinates for the value en a menu.
*
**************************************************************************/

static int	rtt_menu_upd_value_koord( menu_ctx	ctx)
{
	int		item;
	int		page;
	int		row, col;
	rtt_t_menu_upd	*menu_ptr;

	menu_ptr = (rtt_t_menu_upd *) (ctx->menu);
	for ( page = 0; page < ctx->no_pages; page++)
	{
	  for ( item = page * ctx->page_len; 
		(item < ctx->no_items) && ( item < (page + 1) *
		ctx->page_len); item++)
	  {

	    col = (item - page * ctx->page_len) / ctx->rows;
	    row = item - page * ctx->page_len - col * ctx->rows;
	    menu_ptr->value_x = ctx->left_margin + col * ctx->col_size + 
		ctx->max_item_size + 2;
	    menu_ptr->value_y = (22 - RTT_MENU_MAXROWS) + 
			ctx->up_margin + row * ctx->row_size;
	    menu_ptr++;
	  }	
	}
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_menu_upd_configure()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	menu context.
*
* Description:
*	Configures an update menu.
*	Calculates pages, rows etc for a menu  picture.
*
**************************************************************************/

int	rtt_menu_upd_configure( menu_ctx	ctx)
{
	rtt_t_menu_upd	*menu_ptr;
	int		item_maxsize;
	int		item_size;

	/* Calculate no of items */
	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	ctx->no_items = 0;
	item_maxsize = 0;

	while ( menu_ptr->text[0] != '\0' )
	{
	  ctx->no_items++;
	  item_size = strlen( menu_ptr->text);
	  if ( item_size > item_maxsize)
	    item_maxsize = item_size; 
	  menu_ptr++;
	}

	ctx->max_item_size = item_maxsize;

	/* Calculate rows and columns and pages */
	ctx->page_len = RTT_MENU_MAXROWS;
	ctx->no_pages = (ctx->no_items - 1) / ctx->page_len + 1;
	ctx->cols = ctx->page_len / RTT_MENU_MAXROWS;
	ctx->row_size = 1;	
	ctx->rows = (ctx->page_len - 1)/ ctx->cols + 1;
/*	ctx->col_size = (RTT_MENU_MAXCOLS / ctx->cols + item_maxsize + 2)/2;*/
	ctx->col_size = item_maxsize;
	ctx->left_margin = 2;
	ctx->up_margin = 
		(RTT_MENU_MAXROWS - ctx->rows ) / 2;

	ctx->current_page = 0;
	ctx->current_item = 0;
	ctx->current_row = 0;
	ctx->current_col = 0;

	/* Calculate koordinates for the value */
	rtt_menu_upd_value_koord( ctx);
	
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_edit_configure()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	menu context.
*
* Description:
*	Configures an edited picture.
*	Calculates pages, rows etc for a menu  picture.
*
**************************************************************************/

static int	rtt_edit_configure( menu_ctx	ctx)
{
	rtt_t_menu_upd	*menu_ptr;
	int		item_maxsize;
	int		item_size;

	if ( ctx->menu == 0)
	  return RTT__SUCCESS;

	/* Calculate no of items */
	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	ctx->no_items = 0;
	item_maxsize = 0;

	while ( menu_ptr->text[0] != '\0' )
	{
	  ctx->no_items++;
	  item_size = strlen( menu_ptr->text);
	  if ( item_size > item_maxsize)
	    item_maxsize = item_size; 
	  menu_ptr++;
	}

	ctx->max_item_size = item_maxsize;

	/* Calculate rows and columns and pages */
	ctx->page_len = ctx->no_items;
	ctx->no_pages = 1;
	ctx->cols = 1;
	ctx->row_size = 1;	
	ctx->rows = ctx->no_items;
	ctx->col_size = (RTT_MENU_MAXCOLS / ctx->cols + item_maxsize + 2)/2;
	ctx->left_margin = 8;
	ctx->up_margin = 
		(RTT_MENU_MAXROWS - ctx->rows ) / 2;

	ctx->current_page = 0;
	ctx->current_item = 0;
	ctx->current_row = 0;
	ctx->current_col = 0;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_draw_item()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* int		item		I	index of menu item.
*
* Description:
*	Draw a menu item.
*
**************************************************************************/

int	rtt_menu_draw_item( 
			menu_ctx	ctx,
			int		item)
{
	int		x, y, row, col;
	rtt_t_menu	*menu_ptr;
	char		*menu_charptr;

	if ( ctx->no_items == 0)
	  return RTT__SUCCESS;

	if ( (item < ctx->current_page * ctx->page_len) ||
	     (item > (ctx->current_page + 1) * ctx->page_len) ||
	     (item > ctx->no_items))
	  return 0;

	col = (item - ctx->current_page * ctx->page_len) / ctx->rows;
	row = item - ctx->current_page * ctx->page_len - col * ctx->rows;
	x = ctx->left_margin + col * ctx->col_size;
	y = (22 - RTT_MENU_MAXROWS) + ctx->up_margin + row * ctx->row_size;
	rtt_cursor_abs( x, y);

	menu_charptr = (char *) ctx->menu;
	if ( ctx->menutype & RTT_MENUTYPE_MENU )
	  menu_charptr += item * sizeof(rtt_t_menu);
	else if ( ctx->menutype & RTT_MENUTYPE_UPD)
	  menu_charptr += item * sizeof(rtt_t_menu_upd);
	else if ( ctx->menutype & RTT_MENUTYPE_ALARM)
	  menu_charptr += item * sizeof(rtt_t_menu_alarm);

	menu_ptr = (rtt_t_menu *) menu_charptr;

	r_print("%s", &(menu_ptr->text));
	
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_draw_item()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
* int		item		I	index of menu item.
*
* Description:
*	Draw a menu item.
*
**************************************************************************/

static int	rtt_edit_draw_item( 
			menu_ctx	ctx,
			int		item)
{
	int		x, y;
	rtt_t_menu_upd	*menu_ptr;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += item;

	/* if text == "%", the sign for no text written, return */
	if( strcmp( menu_ptr->text, "%") == 0)
	  return RTT__SUCCESS;

	x = menu_ptr->value_x - strlen( menu_ptr->text) - 1;
	y = menu_ptr->value_y;
	rtt_cursor_abs( x, y);

	r_print("%s", &(menu_ptr->text));
	
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_draw_title()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Draw the menu title.
*
**************************************************************************/

int	rtt_menu_draw_title( menu_ctx	ctx)
{
	int		x, y;
	char		*infoline;
	int		sts;
	char		default_infoline[] = 
" RETURN down | Ctrl/A open | Ctrl/T debug | Ctrl/E change value | Ctrl/R back";

	/* Print the title */
	y = 1;
	x = 1;
	if ( rtt_title_prefix[0] != 0)
	{
	  rtt_cursor_abs( x, y);
	  rtt_char_inverse_start();
	  r_print("%s", rtt_title_prefix);
	  rtt_char_inverse_end();
	}
	x = (72 - strlen( ctx->title)) /2;
	if ( x < (int)strlen(rtt_title_prefix) + 2)
	  x = strlen(rtt_title_prefix) + 2;
	if ( strlen(ctx->title) > 71 - strlen(rtt_title_prefix))
	  ctx->title[71-strlen(rtt_title_prefix)] = 0;
/*	y = ctx->up_margin / 2;*/
	rtt_cursor_abs( x, y);
	r_print("%s", &(ctx->title));
	
	/* Print page nr */
	rtt_cursor_abs( 71, 1);
	rtt_char_inverse_start();
	r_print( "page %2d:%2d", ctx->current_page + 1, ctx->no_pages);
	rtt_char_inverse_end();

	/* Print info line */
	rtt_cursor_abs( 1, RTT_ROW_INFO);
	/* Get the info line from the application help if there is one */
	sts = rtt_help_getinfoline( ctx->title, rtt_appl_helptext, &infoline);
	if ( sts == RTT__NOHELPSUBJ)
	  sts = rtt_help_getinfoline( ctx->title, rtt_command_helptext, &infoline);
	rtt_char_inverse_start();
	if ( sts == RTT__NOHELPSUBJ)
	  r_print( "%80s", &default_infoline);
	else 
	  r_print( "%-80s", infoline);
	rtt_char_inverse_end();

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_draw()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	menu ctx.
*
* Description:
*	Draw a menu.
*
**************************************************************************/

static int	rtt_menu_draw( menu_ctx	ctx)
{
	rtt_t_menu	*menu_ptr;
	int		i;

	rtt_display_erase();
	rtt_menu_draw_title( ctx);
	menu_ptr = ctx->menu;
	for ( i = ctx->current_page * ctx->page_len; 
		(i < ctx->no_items) && ( i < (ctx->current_page + 1) *
		ctx->page_len); i++)
	{
	  rtt_menu_draw_item( ctx, i);
	}
	rtt_message('S',"");
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_edit_draw()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	menu ctx.
*
* Description:
*	Draw an edited picture.
*
**************************************************************************/

int	rtt_edit_draw( 
			menu_ctx	ctx,
			rtt_t_backgr	*picture)
{
	rtt_t_menu	*menu_ptr;
	int		i;

	rtt_display_erase();
	rtt_edit_draw_background( picture);

	if ( ctx->menu == 0)
	  return RTT__SUCCESS;

	menu_ptr = ctx->menu;
	for ( i = ctx->current_page * ctx->page_len; 
		(i < ctx->no_items) && ( i < (ctx->current_page + 1) *
		ctx->page_len); i++)
	{
	  rtt_edit_draw_item( ctx, i);
	}
	rtt_message('S',"");
	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_menu_select()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Draw the current item in inverse mode.
*
**************************************************************************/

int	rtt_menu_select( menu_ctx	ctx)
{
	rtt_char_inverse_start();
	rtt_menu_draw_item( ctx, ctx->current_item);
	rtt_char_inverse_end();
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_unselect()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Draw the current item in no inverse mode.
*
**************************************************************************/

int	rtt_menu_unselect( menu_ctx	ctx)
{
	rtt_menu_draw_item( ctx, ctx->current_item);
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_edit_select()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Draw the current item in inverse mode.
*
**************************************************************************/

int	rtt_edit_select( menu_ctx	ctx)
{
	if ( ctx->menu == 0)
	  return RTT__SUCCESS;

	rtt_char_inverse_start();
	rtt_edit_draw_item( ctx, ctx->current_item);
	rtt_char_inverse_end();
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_edit_unselect()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Draw the current item in no inverse mode.
*
**************************************************************************/

int	rtt_edit_unselect( menu_ctx	ctx)
{
	if ( ctx->menu == 0)
	  return RTT__SUCCESS;

	rtt_edit_draw_item( ctx, ctx->current_item);
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_next_item_down()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Assign the next item to current item.
*
**************************************************************************/

int	rtt_get_next_item_down( menu_ctx	ctx)
{
	ctx->current_item++;
	if ( ctx->current_item >= ctx->no_items )
	  ctx->current_item = ctx->current_page * ctx->page_len;
	if   ( ctx->current_item >= (ctx->current_page + 1) * ctx->page_len)
	  ctx->current_item = ctx->current_page * ctx->page_len;

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_get_next_item_up()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign the previous item to current item.
*
**************************************************************************/

int	rtt_get_next_item_up( menu_ctx	ctx)
{
	ctx->current_item--;
	if ( ctx->current_item < ctx->current_page * ctx->page_len)
	  ctx->current_item = (ctx->current_page + 1) * ctx->page_len - 1;
	if ( ctx->current_item >= ctx->no_items )
	  ctx->current_item = ctx->no_items - 1;

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_get_next_item_left()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign the item in the column to the left to be current item.
*
**************************************************************************/

int	rtt_get_next_item_left( menu_ctx	ctx)
{
	int	item;
	
	item = ctx->current_item - ctx->rows;
	if ( item < ctx->current_page * ctx->page_len)
	  return 1;
	else
	  ctx->current_item = item;

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_get_next_item_right()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign the item in the column to the right to be current item.
*
**************************************************************************/

int	rtt_get_next_item_right( menu_ctx	ctx)
{
	int	item;
	
	item = ctx->current_item + ctx->rows;
	if ( item >= ctx->no_items )
	  return 1;
	else if   ( item >= (ctx->current_page + 1) * ctx->page_len)
	  return 1;
	else
	  ctx->current_item = item;
	
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_next_item_down()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.
*
* Description:
*	Assign the next item to current item.
*
**************************************************************************/

static int	rtt_get_next_item_down_e( menu_ctx	ctx)
{
	rtt_t_menu_upd	*menu_ptr;
	int		i;

	for ( i = 0; i < ctx->page_len; i++)
	{
	  ctx->current_item++;
	  if ( ctx->current_item >= ctx->no_items )
	    ctx->current_item = ctx->current_page * ctx->page_len;
	  if ( ctx->current_item >= (ctx->current_page + 1) * ctx->page_len)
	    ctx->current_item = ctx->current_page * ctx->page_len;
	  menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	  menu_ptr += ctx->current_item;
	  if ( strcmp( menu_ptr->text, "%") != 0)
	    break;
	}
	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_get_next_item_up_e()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign the previous item to current item.
*
**************************************************************************/

static int	rtt_get_next_item_up_e( menu_ctx	ctx)
{
	rtt_t_menu_upd	*menu_ptr;
	int		i;

	for ( i = 0; i < ctx->page_len; i++)
	{
	  ctx->current_item--;
	  if ( ctx->current_item < ctx->current_page * ctx->page_len)
	    ctx->current_item = (ctx->current_page + 1) * ctx->page_len - 1;
	  if ( ctx->current_item >= ctx->no_items )
	    ctx->current_item = ctx->no_items - 1;
	  menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	  menu_ptr += ctx->current_item;
	  if ( strcmp( menu_ptr->text, "%") != 0)
	    break;
	}
	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_get_next_item_right_e()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign the left item to current item.
*
**************************************************************************/

static int	rtt_get_next_item_right_e( menu_ctx	ctx)
{
	rtt_t_menu_upd	*menu_ptr;
	int		i;
	int		closer;
	unsigned long	current_x, current_y, found_x, found_y;
	int		found_i;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;
	current_x = menu_ptr->value_x;
	current_y = menu_ptr->value_y;
	found_x = menu_ptr->value_x;
	found_y = menu_ptr->value_y;
	found_i = ctx->current_item;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	i = 0;
	while ( menu_ptr->text[0] != '\0')
	{
	  closer = 0;
	  if ( strcmp( menu_ptr->text, "%") != 0)
	  {
	    if ( menu_ptr->value_y == current_y)
	    {
	      if ( found_y == current_y)
	      {
	        if ( menu_ptr->value_x > current_x &&
	             menu_ptr->value_x < found_x)
	          closer = 1;
	        else if ( menu_ptr->value_x < current_x &&
	                  menu_ptr->value_x > found_x)
	          closer = 1;
	      }
	      else if ( menu_ptr->value_x > current_x)
	        closer = 1;
	    }
	    else
	    {
	      /* y is not equal */	
	      if ( menu_ptr->value_y == found_y && 
		   menu_ptr->value_x < found_x)
	        closer = 1;
	      else
	      {
	        if ( found_y < current_y &&
	             menu_ptr->value_y < found_y &&
	             menu_ptr->value_y < current_y)
	          closer = 1;
	        else if ( found_y < current_y &&
	                  menu_ptr->value_y > current_y)
	          closer = 1;
	        else if ( found_y > current_y &&
	                  menu_ptr->value_y < found_y &&
	                  menu_ptr->value_y > current_y)
	          closer = 1;
	        else if ( found_y == current_y &&
	                  found_x <= current_x)
	          closer = 1;
	      }
	    }
	  }
	  if ( closer)
	  {
	    found_x = menu_ptr->value_x;
	    found_y = menu_ptr->value_y;
	    found_i = i;
	  }
	  menu_ptr++;
	  i++;
	}
	ctx->current_item = found_i;
	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_get_next_item_right_e()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign the left item to current item.
*
**************************************************************************/

static int	rtt_get_next_item_left_e( menu_ctx	ctx)
{
	rtt_t_menu_upd	*menu_ptr;
	int		i;
	int		closer;
	unsigned long	current_x, current_y, found_x, found_y;
	int		found_i;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;
	current_x = menu_ptr->value_x;
	current_y = menu_ptr->value_y;
	found_x = menu_ptr->value_x;
	found_y = menu_ptr->value_y;
	found_i = ctx->current_item;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	i = 0;
	while ( menu_ptr->text[0] != '\0')
	{
	  closer = 0;
	  if ( strcmp( menu_ptr->text, "%") != 0)
	  {
	    if ( menu_ptr->value_y == current_y)
	    {
	      if ( found_y == current_y)
	      {
	        if ( found_x < current_x)
	        {
	          if ( menu_ptr->value_x < current_x &&
	               menu_ptr->value_x > found_x)
	            closer = 1;
	        }
	        else 
	        {
	          if ( menu_ptr->value_x < current_x)
	            closer = 1;
	          else if ( menu_ptr->value_x > current_x &&
	               menu_ptr->value_x > found_x)
	            closer = 1;
	        }
	      }
	      else
	      {
	        if ( menu_ptr->value_x < current_x)
	          closer = 1;
	      }
	    }
	    else
	    {
	      /* y is not equal */
	      if ( menu_ptr->value_y == found_y && 
		   menu_ptr->value_x > found_x)
	        closer = 1;
	      else
	      {
	        if ( found_y > current_y &&
	             menu_ptr->value_y > found_y)
	          closer = 1;
	        else if ( found_y > current_y &&
	                  menu_ptr->value_y < current_y)
	          closer = 1;
	        else if ( found_y < current_y &&
	                  menu_ptr->value_y > found_y &&
	                  menu_ptr->value_y < current_y)
	          closer = 1;
	        else if ( found_y == current_y &&
	                  found_x >= current_x)
	          closer = 1;
	      }
	    }
	  }
	  if ( closer)
	  {
	    found_x = menu_ptr->value_x;
	    found_y = menu_ptr->value_y;
	    found_i = i;
	  }
	  menu_ptr++;
	  i++;
	}
	ctx->current_item = found_i;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_previous_page()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign previous page to be current page.
*
**************************************************************************/

int	rtt_get_previous_page( menu_ctx	ctx)
{
	
	if ( ctx->current_page == 0)
	  return 0;
	ctx->current_page--;
	ctx->current_item = ctx->current_page * ctx->page_len;
	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_get_next_page()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign next page to be current page.
*
**************************************************************************/

int	rtt_get_next_page( menu_ctx	ctx)
{
	
	if ( ctx->current_page == (ctx->no_pages - 1))
	  return 0;
	ctx->current_page++;
	ctx->current_item = ctx->current_page * ctx->page_len;
	return 1;
}

/*************************************************************************
*
* Name:		rtt_item_to_page()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Returns the page of an item.
*
**************************************************************************/

int	rtt_item_to_page(
			menu_ctx	ctx,
			int		item,
			int		*page)
{
	*page = item / ctx->page_len;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_delete()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Delete the menu.
*	Free the allocated memory for the context, and for the menulist.
*
**************************************************************************/

int	rtt_menu_delete( menu_ctx	ctx)
{
	rtt_t_menu_upd	*menu_ptr;
	int		sts;

	rtt_ctx_pop();

	if ( ctx == rtt_collectionmenuctx || 
	     ctx == rtt_event_ctx ||
	     ctx == rtt_alarm_ctx)
	{
	  /* Keep the collection ctx */
	  return RTT__SUCCESS;
	}

	if ( ctx->menutype & RTT_MENUTYPE_UPD)
	{
	  /* take away the gdh prenumerations */
	  menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	  while ( menu_ptr->text[0] != '\0')
	  {
	    if ( memcmp( &menu_ptr->subid, &pwr_cNDlid, sizeof(gdh_tSubid)))
	      sts = gdh_UnrefObjectInfo ( menu_ptr->subid);
	    menu_ptr++;
	  }
	}

	/* free the menu */
	if ( ctx->menutype & RTT_MENUTYPE_DYN )
	  free ( ctx->menu);
	
	/* free the context */	
	free( ctx);

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_new()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	parent_ctx	I	parents rtt context.
* rtt_t_menu	**menu_p	I	menu list.
* char		*title		I	menu title.
* unsigned long	userdata	I	...
* unsigned long	flag		I	menu type
*
* Description:
*	Create a new menu.
*
**************************************************************************/

 int	rtt_menu_keys_new(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			rtt_t_menu	**menu_p,
			char		*title,
			void 		*userdata,
			unsigned long	flag)
{
	int	sts;

	sts = rtt_menu_new( parent_ctx, argoi, menu_p, title, userdata, flag);
	return sts;
}

/*************************************************************************
*
* Name:		rtt_menu_new()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	parent_ctx	I	parents rtt context.
* rtt_t_menu	**menu_p	I	menu list.
* char		*title		I	menu title.
* unsigned long	userdata	I	...
* unsigned long	flag		I	menu type
*
* Description:
*	Create a new menu.
*
**************************************************************************/

int	rtt_menu_new(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			rtt_t_menu	**menu_p,
			char		*title,
			void		*userdata,
			unsigned long	flag)
{
	rtt_t_menu	*menu;
	menu_ctx	ctx;
	unsigned long	terminator;
	char		input_str[80];
	int		maxlen = 30;
	unsigned long	option;
	rtt_t_menu	*menu_ptr;
	int		sts;
	unsigned long	menutype;
	int		fastpicture_drawn;
	
	menu = *menu_p;
	menutype = RTT_MENUTYPE_MENU | flag;
	if (  (rtt_t_menu_upd *) menu == rtt_collectionmenulist && 
	      rtt_collectionmenuctx != 0)
	{
	  /* The collection ctx is already created */
	  ctx = rtt_collectionmenuctx;
	  rtt_ctx_push( ctx);
	}
	else
	{
	  sts = rtt_menu_create_ctx( &ctx, parent_ctx, menu, title, menutype);
	  if ( EVEN(sts)) return sts;
	  rtt_menu_configure( ctx);
	}
	rtt_menu_draw( ctx);
	rtt_menu_select( ctx);
	menu_ptr = ctx->menu;
	if ( ctx->parent_ctx == 0)
	  rtt_root_menu = ctx->menu;

	option = RTT_OPT_NORECALL | RTT_OPT_NOEDIT | RTT_OPT_NOECHO | 
		RTT_OPT_TIMEOUT;

	while (1)
	{
	  rtt_command_get_input_string( (char *)&rtt_chn, input_str, 
		&terminator, maxlen, 
		rtt_recallbuff, option, rtt_scantime, &rtt_scan,
		(void *) ctx, 
		NULL, RTT_COMMAND_PICTURE);
	  rtt_message('S',"");

	  switch ( terminator)
	  {
	    case RTT_K_ARROW_UP:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_up( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_ARROW_DOWN:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_down( ctx);
	      rtt_menu_select( ctx);
	      break;
#if 0
	    case RTT_K_ARROW_LEFT:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_left( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_ARROW_RIGHT:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_right( ctx);
	      rtt_menu_select( ctx);
	      break;
#endif
	    case RTT_K_NEXTPAGE:
	      /* Next page */
	      sts = rtt_get_next_page( ctx);	
	      if ( ODD(sts))
	      {
	        rtt_menu_draw( ctx);
	        rtt_menu_select( ctx);
	      }
	      break;
	    case RTT_K_PREVPAGE:
	      /* Previous page */
	      sts = rtt_get_previous_page( ctx);	
	      if ( ODD(sts))
	      {
	        rtt_menu_draw( ctx);
	        rtt_menu_select( ctx);
	      }
	      break;
	    case RTT_K_RETURN:
	    case RTT_K_ARROW_RIGHT:
	      if ( (menu_ptr + ctx->current_item)->func != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func) ( ctx,
			(menu_ptr + ctx->current_item)->argoi,
			(menu_ptr + ctx->current_item)->arg1,
			(menu_ptr + ctx->current_item)->arg2,
			(menu_ptr + ctx->current_item)->arg3,
			(menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        menu_ptr = ctx->menu;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else
	          {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	            if ( rtt_fastkey)
		      sts = RTT__NOPICTURE;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_SHIFT_ARROW_RIGHT:
	    case RTT_K_PF1:
	      if ( (menu_ptr + ctx->current_item)->func2 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func2) ( ctx,
			(menu_ptr + ctx->current_item)->argoi,
			(menu_ptr + ctx->current_item)->arg1,
			(menu_ptr + ctx->current_item)->arg2,
			(menu_ptr + ctx->current_item)->arg3,
			(menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else
	          {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	        }
	      } 
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF2:
	      if ( (menu_ptr + ctx->current_item)->func3 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func3) ( ctx,
			(menu_ptr + ctx->current_item)->argoi,
			(menu_ptr + ctx->current_item)->arg1,
			(menu_ptr + ctx->current_item)->arg2,
			(menu_ptr + ctx->current_item)->arg3,
			(menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else
	          {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	        }
	      } 
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF3:
	      rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_ARROW_LEFT:
	    case RTT_K_SHIFT_ARROW_LEFT:
	    case RTT_K_PF4:
	      if ( ctx->parent_ctx != 0)
	      {	
	        rtt_menu_delete(ctx);
	        return RTT__SUCCESS;
	      }
	      break;
	    case RTT_K_FAST_1:
	    case RTT_K_FAST_2:
	    case RTT_K_FAST_3:
	    case RTT_K_FAST_4:
	    case RTT_K_FAST_5:
	    case RTT_K_FAST_6:
	    case RTT_K_FAST_7:
	    case RTT_K_FAST_8:
	    case RTT_K_FAST_9:
	    case RTT_K_FAST_10:
	    case RTT_K_FAST_11:
	    case RTT_K_FAST_12:
	    case RTT_K_FAST_13:
	    case RTT_K_FAST_14:
	    case RTT_K_FAST_15:
	    case RTT_K_FAST_16:
	    case RTT_K_FAST_17:
	    case RTT_K_FAST_18:
	    case RTT_K_FAST_19:
	    case RTT_K_FAST_20:
	    case RTT_K_FAST_21:
	    case RTT_K_FAST_22:
	      rtt_fastkey = terminator - RTT_K_FAST;
	      sts = rtt_get_fastkey_type();
	      if ( sts == RTT__NOPICTURE)
	      {
	        sts = rtt_get_fastkey_picture( ctx);
	        if ( EVEN(sts)) return sts;
	        break;
	      }
	    case RTT_K_CTRLZ:
	      if ( ctx->parent_ctx != 0)
	      {	
	        rtt_menu_delete(ctx);
	        return RTT__FASTBACK;
	      }
	      else
	      {
	        rtt_menu_unselect( ctx);
	        ctx->current_page = 0;
	        ctx->current_item = 0;
	        rtt_menu_select( ctx);
	      }
	      break;
	    case RTT_K_CTRLW:
	      rtt_menu_draw( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_CTRLK:
	      /* Acknowledge last alarm */
	      sts = rtt_alarm_ack_last();
	      break;
	    case RTT_K_CTRLL:
	      /* Change description mode */
	      if ( !rtt_description_on)
	        sts = rtt_cli( rtt_command_table, "SET DESCRIPTION", (void *) ctx, 0);
	      else
	        sts = rtt_cli( rtt_command_table, "SET NODESCRIPTION", (void *) ctx, 0);
	      break;
	    case RTT_K_CTRLV:
	      /* Insert current item in collection picture */
	      sts = rtt_collect_insert( ctx, NULL);
	      break;
	    case RTT_K_CTRLN:
	      /* Show collection picture */
	      sts = rtt_collect_show( ctx);
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      else
	      {
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	        } 
	      }
	      break;
	    case RTT_K_DELETE:
	      /* Delete current item */
	      /* Check that this is a dynamic menu */
	      if ( ctx->menutype && RTT_MENUTYPE_DYN)
	      {
	        /* Delete current item */
	        rtt_menu_item_delete( ctx, ctx->current_item);
	        /* Reconfigure the menu */
	        rtt_menu_configure( ctx);
	        rtt_menu_draw( ctx);
	        rtt_menu_select( ctx);
	      }
	      else
	        rtt_message('E',"Unable to delete an item in a static menu");
	      break;
	    case RTT_K_COMMAND:
	      sts = rtt_get_command( ctx, (char *) &rtt_chn, rtt_recallbuff, 
			0, 0, (void *) ctx, 
	    		"pwr_rtt> ", 0, RTT_ROW_COMMAND, rtt_command_table);
	      /* menu_ptr might have been changed */
	      if ( EVEN(sts)) return sts;
	      menu_ptr = ctx->menu;
	      if ( sts == RTT__FASTBACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__FASTBACK;
	        }
	        else
	        {
	          ctx->current_page = 0;
	          ctx->current_item = 0;
	        }
	      }
	      if ( sts == RTT__BACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__SUCCESS;
	        }
	      }
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      if ( sts != RTT__NOPICTURE)
	      {
	        rtt_menu_draw( ctx);
	        rtt_menu_select( ctx);
	      } 
	      break;
	    case RTT_K_HELP:
	      /* Try to find subject in application help */
	      sts = rtt_help( ctx, title, rtt_appl_helptext);
	      if ( sts == RTT__NOHELPSUBJ)
	      {
	        sts = rtt_help( ctx, title, rtt_command_helptext);
	        if ( sts == RTT__NOHELPSUBJ)
	          /* Not found, show the 'object menu' help */
	          rtt_help( ctx, "OBJECT MENU", rtt_command_helptext);
	      }
	      rtt_menu_draw( ctx);
	      rtt_menu_select( ctx);
	      break;
	  } 

	  if ( ctx->parent_ctx == 0)
	  {
	    fastpicture_drawn = 0;
	    while ( rtt_fastkey)
	    {
	      fastpicture_drawn = 1;
	      sts = rtt_get_fastkey_picture( ctx);
	      if ( EVEN(sts)) return sts;
	    }
	    if ( fastpicture_drawn)
	    {
	      rtt_menu_draw( ctx);
	      rtt_menu_select( ctx);
	    }
	  }
	}

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_menu_upd_new()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	parent_ctx	I	parents rtt context.
* rtt_t_menu_upd **menu_p	I	menu list.
* char		*title		I	menu title.
* unsigned long	userdata	I	...
* unsigned long	flag		I	menu type
*
* Description:
*	Create a new update menu.
*
**************************************************************************/

int	rtt_menu_upd_new(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			rtt_t_menu_upd	**menu_p,
			char		*title,
			void		*userdata,
			unsigned long	flag)
{
	rtt_t_menu_upd	*menu;
	menu_ctx	ctx;
	unsigned long	terminator;
	unsigned long	option;
	char		input_str[80];
	int		maxlen = 30;
	rtt_t_menu_upd	*menu_ptr;
	int		sts;
	unsigned long	menutype;
	
	menu = *menu_p;
	menutype = RTT_MENUTYPE_UPD | flag;
	sts = rtt_menu_create_ctx( &ctx, parent_ctx, (rtt_t_menu *) menu, 
			title, menutype);
	if ( EVEN(sts)) return sts;
	rtt_menu_upd_configure( ctx);
	rtt_menu_draw( ctx);
	rtt_menu_select( ctx);
	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	ctx->update_init = 1;
	rtt_menu_upd_update( ctx);

	option = RTT_OPT_NORECALL | RTT_OPT_NOEDIT | RTT_OPT_NOECHO | 
		RTT_OPT_TIMEOUT;

	while (1)
	{
	  rtt_command_get_input_string( (char *) &rtt_chn, input_str, 
		&terminator, maxlen, 
		rtt_recallbuff, option, rtt_scantime, &rtt_menu_upd_update, 
		(void *) ctx, 
		NULL, RTT_COMMAND_PICTURE);
	  rtt_message('S',"");

	  switch( terminator)
	  {
	    case RTT_K_ARROW_UP:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_up( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_ARROW_DOWN:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_down( ctx);
	      rtt_menu_select( ctx);
	      break;
#if 0
	    case RTT_K_ARROW_LEFT:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_left( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_ARROW_RIGHT:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_right( ctx);
	      rtt_menu_select( ctx);
	      break;
#endif
	    case RTT_K_NEXTPAGE:
	      /* Next page */
	      sts = rtt_get_next_page( ctx);	
	      if ( ODD(sts))
	      {
	        rtt_menu_draw( ctx);
	        rtt_menu_select( ctx);
	        ctx->update_init = 1;
	        rtt_menu_upd_update( ctx);
	      }
	      break;
	    case RTT_K_PREVPAGE:
	      /* Previous page */
	      sts = rtt_get_previous_page( ctx);	
	      if ( ODD(sts))
	      {
	        rtt_menu_draw( ctx);
	        rtt_menu_select( ctx);
 	        ctx->update_init = 1;
	        rtt_menu_upd_update( ctx);
	      }
	      break;
	    case RTT_K_RETURN:
	      if ( (menu_ptr + ctx->current_item)->func != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else
	          {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_upd_update( ctx);
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF1:
	    case RTT_K_SHIFT_ARROW_RIGHT:
	      if ( (menu_ptr + ctx->current_item)->func2 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func2) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else
	          {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_upd_update( ctx);
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF2:
	      if ( (menu_ptr + ctx->current_item)->func3 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func3) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else
	          {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_upd_update( ctx);
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF3:
	    case RTT_K_ARROW_RIGHT:
	      if ((long int) (menu_ptr + ctx->current_item)->value_ptr != RTT_ERASE) {
		rtt_get_value( ctx, rtt_scantime, &rtt_menu_upd_update,
			       (void *) ctx, 
			       "Enter value: ", 0, RTT_ROW_COMMAND);
		rtt_menu_upd_update( ctx);
	      }
	      else if ( (menu_ptr + ctx->current_item)->func2 != NULL) {
	        sts = ((menu_ptr + ctx->current_item)->func2) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK) {
	          if ( ctx->parent_ctx != 0) {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT) {
	          if ( ctx != rtt_collectionmenuctx) {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE) {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_upd_update( ctx);
	        }
	      }
	      break;
	    case RTT_K_PF4:
	    case RTT_K_ARROW_LEFT:
	    case RTT_K_SHIFT_ARROW_LEFT:
	      if ( ctx->parent_ctx != 0)
	      {	
	        rtt_menu_delete(ctx);
	        return RTT__SUCCESS;
	      }
	      break;
	    case RTT_K_FAST_1:
	    case RTT_K_FAST_2:
	    case RTT_K_FAST_3:
	    case RTT_K_FAST_4:
	    case RTT_K_FAST_5:
	    case RTT_K_FAST_6:
	    case RTT_K_FAST_7:
	    case RTT_K_FAST_8:
	    case RTT_K_FAST_9:
	    case RTT_K_FAST_10:
	    case RTT_K_FAST_11:
	    case RTT_K_FAST_12:
	    case RTT_K_FAST_13:
	    case RTT_K_FAST_14:
	    case RTT_K_FAST_15:
	    case RTT_K_FAST_16:
	    case RTT_K_FAST_17:
	    case RTT_K_FAST_18:
	    case RTT_K_FAST_19:
	    case RTT_K_FAST_20:
	    case RTT_K_FAST_21:
	    case RTT_K_FAST_22:
	      rtt_fastkey = terminator - RTT_K_FAST;
	      sts = rtt_get_fastkey_type();
	      if ( sts == RTT__NOPICTURE)
	      {
	        sts = rtt_get_fastkey_picture( ctx);
	        if ( EVEN(sts)) return sts;
	        break;
	      }
	    case RTT_K_CTRLZ:
	      if ( ctx->parent_ctx != 0)
	      {	
	        rtt_menu_delete(ctx);
	        return RTT__FASTBACK;
	      }
	      break;
	    case RTT_K_CTRLW:
	      rtt_menu_draw( ctx);
	      rtt_menu_select( ctx);
	      ctx->update_init = 1;
	      rtt_menu_upd_update( ctx);
	      break;
	    case RTT_K_CTRLK:
	      /* Acknowledge last alarm */
	      sts = rtt_alarm_ack_last();
	      break;
	    case RTT_K_CTRLL:
	      /* Change description mode */
	      if ( !rtt_description_on)
	        sts = rtt_cli( rtt_command_table, "SET DESCRIPTION", (void *) ctx, 0);
	      else
	        sts = rtt_cli( rtt_command_table, "SET NODESCRIPTION", (void *) ctx, 0);
	      break;
	    case RTT_K_CTRLV:
	      /* Insert current item in collection picture */
	      sts = rtt_collect_insert( ctx, NULL);
	      break;
	    case RTT_K_CTRLN:
	      /* Show collection picture */
	      sts = rtt_collect_show( ctx);
	      if ( EVEN(sts)) return sts;
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      else
	      {
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_draw( ctx);
	          rtt_menu_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_upd_update( ctx);
	        } 
	      }
	      break;
	    case RTT_K_DELETE:
	      /* Delete current item */
	      /* Check that this is a dynamic menu */
	      if ( ctx->menutype && RTT_MENUTYPE_DYN)
	      {
	        /* Delete current item */
	        rtt_menu_item_delete( ctx, ctx->current_item);
	        /* Reconfigure the menu */
	        rtt_menu_upd_configure( ctx);
	        rtt_menu_draw( ctx);
	        rtt_menu_select( ctx);
	        ctx->update_init = 1;
	        rtt_menu_upd_update( ctx);
	      }
	      else
	        rtt_message('E',"Unable to delete an item in a static menu");
	      break;
	    case RTT_K_COMMAND:
	      sts = rtt_get_command( ctx, (char *) &rtt_chn, 
		rtt_recallbuff, rtt_scantime, 
		&rtt_menu_upd_update, (void *) ctx, 
	    	"pwr_rtt> ", 0, RTT_ROW_COMMAND, rtt_command_table);
	      /* menu_ptr might have been changed */
	      if ( EVEN(sts)) return sts;
	      menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	      if ( sts == RTT__FASTBACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__FASTBACK;
	        }
	        else
	        {
	          ctx->current_page = 0;
	          ctx->current_item = 0;
	        }
	      }
	      if ( sts == RTT__BACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__SUCCESS;
	        }
	      }
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      if ( sts != RTT__NOPICTURE)
	      {
	        rtt_menu_draw( ctx);
	        rtt_menu_select( ctx);
	        ctx->update_init = 1;
	        rtt_menu_upd_update( ctx);
	      } 
	      break;
	    case RTT_K_HELP:
	      /* Try to find help in application help */
	      sts = rtt_help( ctx, title, rtt_appl_helptext);
	      if ( sts == RTT__NOHELPSUBJ)
	      {
	        sts = rtt_help( ctx, title, rtt_command_helptext);
	        if ( sts == RTT__NOHELPSUBJ)
	          /* Not found, show the 'object menu' help */
	          rtt_help( ctx, "OBJECT MENU", rtt_command_helptext);
	      }

  	      rtt_menu_draw( ctx);
	      rtt_menu_select( ctx);
	      ctx->update_init = 1;
	      rtt_menu_upd_update( ctx);
	      break;
	  } 
	}

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_menu_upd_new()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parents rtt context.
* rtt_t_menu_upd **menu_p	I	menu list.
* char		*title		I	menu title.
* void		*userdata	I	...
* unsigned long	flag		I	menu type
*
* Description:
*	Create a new update menu.
*
**************************************************************************/

int	rtt_menu_edit_new(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			rtt_t_menu_upd	**menu_p,
			char		*title,
			rtt_t_backgr	*picture,
			int		(* appl_func) ())
{
	rtt_t_menu_upd	*menu;
	menu_ctx	ctx;
	unsigned long	terminator;
	unsigned long	option;
	char		input_str[80];
	int		maxlen = 30;
	rtt_t_menu_upd	*menu_ptr;
	int		sts;
	unsigned long	menutype;
	
	menu = *menu_p;
	menutype = RTT_MENUTYPE_UPD | RTT_MENUTYPE_DYN | RTT_MENUTYPE_EDIT;
	sts = rtt_menu_create_ctx( &ctx, parent_ctx, (rtt_t_menu *) menu, 
			title, menutype);
	if ( EVEN(sts)) return sts;
	rtt_edit_configure( ctx);
	ctx->appl_func = appl_func;
	if ( appl_func != NULL)
	{
	  sts = (appl_func) ( ctx, RTT_APPL_INIT, 0);
	  if ( EVEN(sts)) return sts;
	}
	rtt_edit_draw( ctx, picture);
	rtt_edit_select( ctx);
	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	ctx->update_init = 1;
	rtt_menu_edit_update( ctx);

	option = RTT_OPT_NORECALL | RTT_OPT_NOEDIT | RTT_OPT_NOECHO | 
		RTT_OPT_TIMEOUT;

	while (1)
	{
	  rtt_command_get_input_string( (char *)&rtt_chn, input_str, 
		&terminator, maxlen, 
		rtt_recallbuff, option, rtt_scantime, &rtt_menu_edit_update, 
		(void *) ctx, 
		NULL, RTT_COMMAND_PICTURE);
	  rtt_message('S',"");

	  switch ( terminator)
	  {
	    case RTT_K_ARROW_UP:
	      rtt_edit_unselect( ctx);
	      rtt_get_next_item_up_e( ctx);
	      rtt_edit_select( ctx);
	      break;
	    case RTT_K_ARROW_DOWN:
	      rtt_edit_unselect( ctx);
	      rtt_get_next_item_down_e( ctx);
	      rtt_edit_select( ctx);
	      break;
	    case RTT_K_ARROW_LEFT:
	      rtt_edit_unselect( ctx);
	      rtt_get_next_item_left_e( ctx);
	      rtt_edit_select( ctx);
	      break;
	    case RTT_K_ARROW_RIGHT:
	      rtt_edit_unselect( ctx);
	      rtt_get_next_item_right_e( ctx);
	      rtt_edit_select( ctx);
	      break;
	    case RTT_K_NEXTPAGE:
	      /* Next page */
	      if ( appl_func != NULL)
	      {
	        sts = (appl_func) ( ctx, RTT_APPL_NEXTPAGE, 0);
	        if ( EVEN(sts)) return sts;
	      }
	      else
	      {
	        sts = rtt_get_next_page( ctx);	
	        if ( ODD(sts))
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	      } 
	      break;
	    case RTT_K_PREVPAGE:
	      /* Previous page */
	      if ( appl_func != NULL)
	      {
	        sts = (appl_func) ( ctx, RTT_APPL_PREVPAGE, 0);
	        if ( EVEN(sts)) return sts;
	      }
	      else
	      {
	        sts = rtt_get_previous_page( ctx);
	        if ( ODD(sts))
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
 	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	      }
	      break;
	    case RTT_K_RETURN:
	      if ( (menu_ptr + ctx->current_item)->func != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF1:
	      if ( (menu_ptr + ctx->current_item)->func2 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func2) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE && sts != RTT__NOVALUE)
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	        if ( appl_func != NULL)
	        {
	          if ( sts != RTT__NOVALUE)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_VALUECHANGED, 
			  (menu_ptr + ctx->current_item)->value_ptr);
	            if ( EVEN(sts)) return sts;
	            if ( sts == RTT__REDRAW)
	            {
	              rtt_edit_draw( ctx, picture);
	              rtt_edit_select( ctx);
	              ctx->update_init = 1;
	              rtt_menu_edit_update( ctx);
	            }
	          }
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF2:
	      if ( (menu_ptr + ctx->current_item)->func3 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func3) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE && sts != RTT__NOVALUE)
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	        if ( appl_func != NULL)
	        {
	          if ( sts != RTT__NOVALUE)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_VALUECHANGED, 
			  (menu_ptr + ctx->current_item)->value_ptr);
	            if ( EVEN(sts)) return sts;
	            if ( sts == RTT__REDRAW)
	            {
	              rtt_edit_draw( ctx, picture);
	              rtt_edit_select( ctx);
	              ctx->update_init = 1;
	              rtt_menu_edit_update( ctx);
	            }
	          }
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF3:
	      sts = rtt_get_value( ctx, rtt_scantime, &rtt_menu_edit_update, 
		  (void *) ctx,
	    	  "Enter value: ", 0, RTT_ROW_COMMAND);
	      if ( ODD(sts) && sts != RTT__NOVALUE && appl_func != NULL)
	      {
	        sts = (appl_func) ( ctx, RTT_APPL_VALUECHANGED, 
			  (menu_ptr + ctx->current_item)->value_ptr);
	        if ( EVEN(sts)) return sts;
	        rtt_menu_edit_update( ctx);
	      }
	      break;
	    case RTT_K_PF4:
	      if ( ctx->parent_ctx != 0)
	      {	
	        if ( appl_func != NULL)
	        {
	          sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	          if ( EVEN(sts)) return sts;
	        }
	        free(ctx);
	        return RTT__SUCCESS;
	      }
	      break;
	    case RTT_K_FAST_1:
	    case RTT_K_FAST_2:
	    case RTT_K_FAST_3:
	    case RTT_K_FAST_4:
	    case RTT_K_FAST_5:
	    case RTT_K_FAST_6:
	    case RTT_K_FAST_7:
	    case RTT_K_FAST_8:
	    case RTT_K_FAST_9:
	    case RTT_K_FAST_10:
	    case RTT_K_FAST_11:
	    case RTT_K_FAST_12:
	    case RTT_K_FAST_13:
	    case RTT_K_FAST_14:
	    case RTT_K_FAST_15:
	    case RTT_K_FAST_16:
	    case RTT_K_FAST_17:
	    case RTT_K_FAST_18:
	    case RTT_K_FAST_19:
	    case RTT_K_FAST_20:
	    case RTT_K_FAST_21:
	    case RTT_K_FAST_22:
	      rtt_fastkey = terminator - RTT_K_FAST;
	      sts = rtt_get_fastkey_type();
	      if ( sts == RTT__NOPICTURE)
	      {
	        sts = rtt_get_fastkey_picture( ctx);
	        if ( EVEN(sts)) return sts;
	        break;
	      }
	    case RTT_K_CTRLZ:
	      if ( ctx->parent_ctx != 0)
	      {	
	        if ( appl_func != NULL)
	        {
	          sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	          if ( EVEN(sts)) return sts;
	        }
	        free(ctx);
	        return RTT__FASTBACK;
	      }
	      else 
	        rtt_message('I', "This is the root menu");
	      break;
	    case RTT_K_CTRLW:
	      rtt_edit_draw( ctx, picture);
	      rtt_edit_select( ctx);
	      ctx->update_init = 1;
	      rtt_menu_edit_update( ctx);
	      break;
	    case RTT_K_CTRLK:
	      /* Acknowledge last alarm */
	      sts = rtt_alarm_ack_last();
	      break;
	    case RTT_K_CTRLL:
	      /* Change description mode */
	      if ( !rtt_description_on)
	        sts = rtt_cli( rtt_command_table, "SET DESCRIPTION", (void *) ctx, 0);
	      else
	        sts = rtt_cli( rtt_command_table, "SET NODESCRIPTION", (void *) ctx, 0);
	      break;
	    case RTT_K_CTRLV:
	      /* Insert current item in collection picture */
	      sts = rtt_collect_insert( ctx, NULL);
	      break;
	    case RTT_K_CTRLN:
	      /* Show collection picture */
	      sts = rtt_collect_show( ctx);
	      if ( EVEN(sts)) return sts;
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          if ( appl_func != NULL)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	            if ( EVEN(sts)) return sts;
	          }
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      else
	      {
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        } 
	      }
	      break;
	    case RTT_K_DELETE:
	      break;
	    case RTT_K_COMMAND:
	      sts = rtt_get_command( ctx, (char *) &rtt_chn, rtt_recallbuff, 
			rtt_scantime, 
		  	&rtt_menu_edit_update, (void *) ctx, 
	    	  	"pwr_rtt> ", 0, RTT_ROW_COMMAND, rtt_command_table);
	      /* menu_ptr might have been changed */
	      if ( EVEN(sts)) return sts;
	      menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	      if ( sts == RTT__FASTBACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          if ( appl_func != NULL)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	            if ( EVEN(sts)) return sts;
	          }
	          rtt_menu_delete(ctx);
	          return RTT__FASTBACK;
	        }
	      }
	      if ( sts == RTT__BACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          if ( appl_func != NULL)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	            if ( EVEN(sts)) return sts;
	          }
	          rtt_menu_delete(ctx);
	          return RTT__SUCCESS;
	        }
	      }
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          if ( appl_func != NULL)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0);
	            if ( EVEN(sts)) return sts;
	          }
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      if ( sts != RTT__NOPICTURE)
	      {
	        rtt_edit_draw( ctx, picture);
	        rtt_edit_select( ctx);
	        ctx->update_init = 1;
	        rtt_menu_edit_update( ctx);
	      } 
	      break;
	    case RTT_K_HELP:
	      /* Try to find help in application help */
	      sts = rtt_help( ctx, title, rtt_appl_helptext);
	      if ( sts == RTT__NOHELPSUBJ)
	      {
	        sts = rtt_help( ctx, title, rtt_command_helptext);
	        if ( sts == RTT__NOHELPSUBJ)
	          /* Not found, show the 'object menu' help */
	          rtt_help( ctx, "OBJECT MENU", rtt_command_helptext);
	      }

  	      rtt_edit_draw( ctx, picture);
	      rtt_edit_select( ctx);
	      ctx->update_init = 1;
	      rtt_menu_edit_update( ctx);
	      break;
	  } 
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_upd_new()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Create a new update menu.
*
**************************************************************************/

int	rtt_menu_sysedit_new(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			rtt_t_menu_upd	**menu_p,
			char		*title,
			char		*objectname,
			int		(* appl_func) ())
{
	rtt_t_backgr	*picture;
	rtt_t_menu_upd	*menu;
	menu_ctx	ctx = NULL;
	unsigned long	terminator;
	unsigned long	option;
	char		input_str[80];
	int		maxlen = 30;
	rtt_t_menu_upd	*menu_ptr;
	int		sts;
	unsigned long	menutype;
	
	/* Get the picture */
	sts = (appl_func) ( ctx, RTT_APPL_PICTURE, 0, objectname, &picture);
	if ( EVEN(sts)) return sts;

	menu = *menu_p;
	menutype = RTT_MENUTYPE_UPD | RTT_MENUTYPE_DYN | RTT_MENUTYPE_EDIT;
	sts = rtt_menu_create_ctx( &ctx, parent_ctx, (rtt_t_menu *) menu, 
			title, menutype);
	if ( EVEN(sts)) return sts;
	rtt_edit_configure( ctx);
	ctx->appl_func = appl_func;
	if ( appl_func != NULL)
	{
	  sts = (appl_func) ( ctx, RTT_APPL_INIT, 0, objectname, NULL);
	  if ( EVEN(sts)) return sts;
	}
	rtt_edit_draw( ctx, picture);
	rtt_edit_select( ctx);
	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	ctx->update_init = 1;
	rtt_menu_edit_update( ctx);

	option = RTT_OPT_NORECALL | RTT_OPT_NOEDIT | RTT_OPT_NOECHO | 
		RTT_OPT_TIMEOUT;

	while (1)
	{
	  rtt_command_get_input_string( (char *) &rtt_chn, input_str, 
		&terminator, maxlen, 
		rtt_recallbuff, option, rtt_scantime, &rtt_menu_edit_update, 
		(void *) ctx, 
		NULL, RTT_COMMAND_PICTURE);
	  rtt_message('S',"");

	  switch ( terminator)
	  {
	    case RTT_K_ARROW_UP:
	      rtt_edit_unselect( ctx);
	      rtt_get_next_item_up_e( ctx);
	      rtt_edit_select( ctx);
	      break;
	    case RTT_K_ARROW_DOWN:
	      rtt_edit_unselect( ctx);
	      rtt_get_next_item_down_e( ctx);
	      rtt_edit_select( ctx);
	      break;
	    case RTT_K_ARROW_LEFT:
	      rtt_edit_unselect( ctx);
	      rtt_get_next_item_left_e( ctx);
	      rtt_edit_select( ctx);
	      break;
	    case RTT_K_ARROW_RIGHT:
	      rtt_edit_unselect( ctx);
	      rtt_get_next_item_right_e( ctx);
	      rtt_edit_select( ctx);
	      break;
	    case RTT_K_NEXTPAGE:
	      /* Next page */
	      if ( appl_func != NULL)
	      {
	        sts = (appl_func) ( ctx, RTT_APPL_NEXTPAGE, 0, objectname, NULL);
	        if ( EVEN(sts)) return sts;
 	        ctx->update_init = 1;
	        rtt_menu_edit_update( ctx);
	      }
	      else
	      {
	        sts = rtt_get_next_page( ctx);	
	        if ( ODD(sts))
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	      } 
	      break;
	    case RTT_K_PREVPAGE:
	      /* Previous page */
	      if ( appl_func != NULL)
	      {
	        sts = (appl_func) ( ctx, RTT_APPL_PREVPAGE, 0, objectname, NULL);
	        if ( EVEN(sts)) return sts;
 	        ctx->update_init = 1;
	        rtt_menu_edit_update( ctx);
	      }
	      else
	      {
	        sts = rtt_get_previous_page( ctx);
	        if ( ODD(sts))
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
 	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	      }
	      break;
	    case RTT_K_RETURN:
	      if ( (menu_ptr + ctx->current_item)->func != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF1:
	      if ( (menu_ptr + ctx->current_item)->func2 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func2) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE && sts != RTT__NOVALUE)
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	        if ( appl_func != NULL)
	        {
	          if ( sts != RTT__NOVALUE)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_VALUECHANGED, 
			  (menu_ptr + ctx->current_item)->value_ptr, objectname,
			  NULL);
	            if ( EVEN(sts)) return sts;
	            if ( sts == RTT__REDRAW)
	            {
	              rtt_edit_draw( ctx, picture);
	              rtt_edit_select( ctx);
	              ctx->update_init = 1;
	              rtt_menu_edit_update( ctx);
	            }
	          }
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF2:
	      if ( (menu_ptr + ctx->current_item)->func3 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func3) ( ctx,
		  (menu_ptr + ctx->current_item)->argoi,
		  (menu_ptr + ctx->current_item)->arg1,
		  (menu_ptr + ctx->current_item)->arg2,
		  (menu_ptr + ctx->current_item)->arg3,
		  (menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            if ( appl_func != NULL)
	            {
	              sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	              if ( EVEN(sts)) return sts;
	            }
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE && sts != RTT__NOVALUE)
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        }
	        if ( appl_func != NULL)
	        {
	          if ( sts != RTT__NOVALUE)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_VALUECHANGED, 
			  (menu_ptr + ctx->current_item)->value_ptr, objectname, 
			  NULL);
	            if ( EVEN(sts)) return sts;
	            if ( sts == RTT__REDRAW)
	            {
	              rtt_edit_draw( ctx, picture);
	              rtt_edit_select( ctx);
	              ctx->update_init = 1;
	              rtt_menu_edit_update( ctx);
	            }
	          }
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF3:
	      sts = rtt_get_value( ctx, rtt_scantime, &rtt_menu_edit_update, 
		  (void *) ctx, 
	    	  "Enter value: ", 0, RTT_ROW_COMMAND);
	      if ( ODD(sts) && sts != RTT__NOVALUE && appl_func != NULL)
	      {
	        sts = (appl_func) ( ctx, RTT_APPL_VALUECHANGED, 
			  (menu_ptr + ctx->current_item)->value_ptr, objectname, 
			  NULL);
	        if ( EVEN(sts)) return sts;
	        rtt_menu_edit_update( ctx);
	      }
	      break;
	    case RTT_K_PF4:
	      if ( ctx->parent_ctx != 0)
	      {	
	        if ( appl_func != NULL)
	        {
	          sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	          if ( EVEN(sts)) return sts;
	        }
	        rtt_menu_delete(ctx);
	        return RTT__SUCCESS;
	      }
	      break;
	    case RTT_K_FAST_1:
	    case RTT_K_FAST_2:
	    case RTT_K_FAST_3:
	    case RTT_K_FAST_4:
	    case RTT_K_FAST_5:
	    case RTT_K_FAST_6:
	    case RTT_K_FAST_7:
	    case RTT_K_FAST_8:
	    case RTT_K_FAST_9:
	    case RTT_K_FAST_10:
	    case RTT_K_FAST_11:
	    case RTT_K_FAST_12:
	    case RTT_K_FAST_13:
	    case RTT_K_FAST_14:
	    case RTT_K_FAST_15:
	    case RTT_K_FAST_16:
	    case RTT_K_FAST_17:
	    case RTT_K_FAST_18:
	    case RTT_K_FAST_19:
	    case RTT_K_FAST_20:
	    case RTT_K_FAST_21:
	    case RTT_K_FAST_22:
	      rtt_fastkey = terminator - RTT_K_FAST;
	      sts = rtt_get_fastkey_type();
	      if ( sts == RTT__NOPICTURE)
	      {
	        sts = rtt_get_fastkey_picture( ctx);
	        if ( EVEN(sts)) return sts;
	        break;
	      }
	    case RTT_K_CTRLZ:
	      if ( ctx->parent_ctx != 0)
	      {	
	        if ( appl_func != NULL)
	        {
	          sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL,
			  objectname, NULL);
	          if ( EVEN(sts)) return sts;
	        }
	        rtt_menu_delete(ctx);
	        return RTT__FASTBACK;
	      }
	      else 
	        rtt_message('I', "This is the root menu");
	      break;
	    case RTT_K_CTRLW:
	      rtt_edit_draw( ctx, picture);
	      rtt_edit_select( ctx);
	      ctx->update_init = 1;
	      rtt_menu_edit_update( ctx);
	      break;
	    case RTT_K_CTRLK:
	      /* Acknowledge last alarm */
	      sts = rtt_alarm_ack_last();
	      break;
	    case RTT_K_CTRLL:
	      /* Change description mode */
	      if ( !rtt_description_on)
	        sts = rtt_cli( rtt_command_table, "SET DESCRIPTION", (void *) ctx, 0);
	      else
	        sts = rtt_cli( rtt_command_table, "SET NODESCRIPTION", (void *) ctx, 0);
	      break;
	    case RTT_K_CTRLV:
	      /* Insert current item in collection picture */
	      sts = rtt_collect_insert( ctx, NULL);
	      break;
	    case RTT_K_CTRLN:
	      /* Show collection picture */
	      sts = rtt_collect_show( ctx);
	      if ( EVEN(sts)) return sts;
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          if ( appl_func != NULL)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	            if ( EVEN(sts)) return sts;
	          }
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      else
	      {
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_edit_draw( ctx, picture);
	          rtt_edit_select( ctx);
	          ctx->update_init = 1;
	          rtt_menu_edit_update( ctx);
	        } 
	      }
	      break;
	    case RTT_K_DELETE:
	      break;
	    case RTT_K_COMMAND:
	      sts = rtt_get_command( ctx, (char *) &rtt_chn, rtt_recallbuff, 
			rtt_scantime, 
		  	&rtt_menu_edit_update, (void *) ctx, 
	    	  "pwr_rtt> ", 0, RTT_ROW_COMMAND, rtt_command_table);
	      /* menu_ptr might have been changed */
	      if ( EVEN(sts)) return sts;
	      menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	      if ( sts == RTT__FASTBACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          if ( appl_func != NULL)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	            if ( EVEN(sts)) return sts;
	          }
	          rtt_menu_delete(ctx);
	          return RTT__FASTBACK;
	        }
	      }
	      if ( sts == RTT__BACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          if ( appl_func != NULL)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	            if ( EVEN(sts)) return sts;
	          }
	          rtt_menu_delete(ctx);
	          return RTT__SUCCESS;
	        }
	      }
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          if ( appl_func != NULL)
	          {
	            sts = (appl_func) ( ctx, RTT_APPL_EXIT, 0, objectname, NULL);
	            if ( EVEN(sts)) return sts;
	          }
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      if ( sts != RTT__NOPICTURE)
	      {
	        rtt_edit_draw( ctx, picture);
	        rtt_edit_select( ctx);
	        ctx->update_init = 1;
	        rtt_menu_edit_update( ctx);
	      } 
	      break;
	    case RTT_K_HELP:
	      /* Try to find help in application help */
	      sts = rtt_help( ctx, title, rtt_command_helptext);
	      if ( sts == RTT__NOHELPSUBJ)
	        /* Not found, show the 'object menu' help */
	        rtt_help( ctx, "OBJECT MENU", rtt_command_helptext);

  	      rtt_edit_draw( ctx, picture);
	      rtt_edit_select( ctx);
	      ctx->update_init = 1;
	      rtt_menu_edit_update( ctx);
	      break;
	  } 
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_list_add_malloc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	**menulist	I	menulist.
* int		index		I	number of items to allocate memory.
*
* Description:
*	Allocates memory for a menu list.
*	Allocated memory is number of items + 1.
*
**************************************************************************/

int	rtt_menu_list_add_malloc(
			rtt_t_menu	**menulist,
			int		index)
{
	*menulist = calloc( index + 1 , sizeof(rtt_t_menu));
	if ( *menulist == 0)
	  return RTT__NOMEMORY;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_list_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	**menulist	I	menulist.
* int		index		I	index in menulist
* int		allocated	I	previlously allocated items.
* char		*text		I	menu text.
* int		(* func) ()	I	function to be called at RETURN.
* int		(* func2) ()	I	function to be called at PF1
* int		(* func3) ()	I	function to be called at PF2
* pwr_tObjid	argoi		I	argument passed to the functions
* void 		*arg1		I	argument passed to the functions
* void 		*arg2		I	argument passed to the functions
* void 		*arg3		I	argument passed to the functions
* void 		*arg4		I	argument passed to the functions
*
* Description:
*	Adds an item to a menu list.
*
**************************************************************************/

int	rtt_menu_list_add(
			rtt_t_menu	**menulist,
			int		index,
			int		allocated,
			char		*text,
			int		(* func) (),
			int		(* func2) (),
			int		(* func3) (),
			pwr_tObjid	argoi,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	rtt_t_menu	*menu_ptr;
	rtt_t_menu	*new_menulist;

	if ( allocated == 0)
	{
	  if ( index == 0)
	  {
	    *menulist = calloc( 1 , 2 * sizeof(rtt_t_menu));
	    if ( *menulist == 0)
	      return RTT__NOMEMORY;
	  }
	  else
	  {
/*	    *menulist = realloc( *menulist, (index + 2) * sizeof(rtt_t_menu));
*/
	    new_menulist = calloc( index + 2 , sizeof(rtt_t_menu));
	    if ( new_menulist == 0)
	      return RTT__NOMEMORY;
	    memcpy( new_menulist, *menulist, (index + 1) * sizeof(rtt_t_menu));
	    free( *menulist);
	    *menulist = new_menulist;
	  }
	}
	menu_ptr = *menulist + index;
	strncpy( menu_ptr->text, text, sizeof(menu_ptr->text)-RTT_CLASSORT_SIZE);
	menu_ptr->text[sizeof(menu_ptr->text)-RTT_CLASSORT_SIZE-1] = 0;
	menu_ptr->func = func;
	menu_ptr->func2 = func2;
	menu_ptr->func3 = func3;
	menu_ptr->argoi = argoi;
	menu_ptr->arg1 = arg1;
	menu_ptr->arg2 = arg2;
	menu_ptr->arg3 = arg3;
	menu_ptr->arg4 = arg4;
	menu_ptr++;
	strcpy( menu_ptr->text, "");

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_menu_list_insert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	**menulist	I	menulist.
* int		index		I	index in menulist
* char		*text		I	menu text.
* int		(* func) ()	I	function to be called at RETURN.
* int		(* func2) ()	I	function to be called at PF1
* int		(* func3) ()	I	function to be called at PF2
* pwr_tObjid	argoi		I	argument passed to the functions
* void		*arg1		I	argument passed to the functions
* void		*arg2		I	argument passed to the functions
* void		*arg3		I	argument passed to the functions
* void		*arg4		I	argument passed to the functions
*
* Description:
*	Inserts an item to a menu list.
*
**************************************************************************/

int	rtt_menu_list_insert(
			rtt_t_menu	**menulist,
			int		index,
			char		*text,
			int		(* func) (),
			int		(* func2) (),
			int		(* func3) (),
			pwr_tObjid	argoi,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	rtt_t_menu	*menu_ptr;
	rtt_t_menu	*new_menulist;
	int		size = 0;

	if ( *menulist == 0)
	{
	  *menulist = calloc( 1 , 2 * sizeof(rtt_t_menu));
	  if ( *menulist == 0)
	    return RTT__NOMEMORY;
	}
	else
	{
/*	  *menulist = realloc( *menulist, (index + 2) * sizeof(rtt_t_menu));
*/
	  /* Get size */
	  menu_ptr = *menulist;
	  while ( menu_ptr->text[0] != '\0')
	  {
	    menu_ptr++;
	    size++;
	  }
	  new_menulist = calloc( size + 2 , sizeof(rtt_t_menu));
	  if ( new_menulist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_menulist, *menulist, index * sizeof(rtt_t_menu));
	  menu_ptr = *menulist + index;
	  memcpy( new_menulist + index + 1, menu_ptr, 
		(size - index + 1) * sizeof(rtt_t_menu));
	  free( *menulist);
	  *menulist = new_menulist;
	}
	menu_ptr = *menulist + index;
	strncpy( menu_ptr->text, text, sizeof(menu_ptr->text));
	menu_ptr->func = func;
	menu_ptr->func2 = func2;
	menu_ptr->func3 = func3;
	menu_ptr->argoi = argoi;
	menu_ptr->arg1 = arg1;
	menu_ptr->arg2 = arg2;
	menu_ptr->arg3 = arg3;
	menu_ptr->arg4 = arg4;
	menu_ptr = *menulist + size + 2;
	strcpy( menu_ptr->text, "");

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_list_add_malloc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	**menulist	I	menulist.
* int		index		I	number of items to allocate memory.
*
* Description:
*	Allocates memory for a menu list.
*	Allocated memory is number of items + 1.
*
**************************************************************************/

int	rtt_menu_upd_list_add_malloc(
			rtt_t_menu_upd	**menulist,
			int		index)
{
	*menulist = calloc( index + 1 , sizeof(rtt_t_menu_upd));
	if ( *menulist == 0)
	  return RTT__NOMEMORY;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_upd_list_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	**menulist	I	menulist.
* int		index		I	index in menulist
* char		*text		I	menu text.
* int		(* func) ()	I	function to be called at RETURN.
* int		(* func2) ()	I	function to be called at PF1
* int		(* func3) ()	I	function to be called at PF2
* pwr_tObjid	argoi		I	argument passed to the functions
* void		*arg1		I	argument passed to the functions
* void		*arg2		I	argument passed to the functions
* void		*arg3		I	argument passed to the functions
* void		*arg4		I	argument passed to the functions
* char		*parameter_name	I	full name of parameter.
* unsigned long	priv		I	privileges to change value
* char		*value_ptr	I	rtdb pointer to parameter.
* unsigned long	value_type	I	type of parameter.
* unsigned long	flags		I	flags of parameter.
* unsigned long	size		I	size of parameter.
* ghd_tSubid	subid		I	subid from gdh_refobjinfo
*
*
* Description:
*	Adds an item to an update menu list.
*
**************************************************************************/

int	rtt_menu_upd_list_add( 
			rtt_t_menu_upd	**menulist,
			int		index,
			int		allocated,
			char		*text,
			int		(* func) (),
			int		(* func2) (),
			int		(* func3) (),
			pwr_tObjid	argoi,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4,
			char		*parameter_name,
			unsigned long	priv,
			char		*value_ptr,
			unsigned long	value_type,
			unsigned long	flags,
			unsigned long	size,
			gdh_tSubid	subid,
			int		x,
			int		y,
			char		characters,
			char		decimals,
			float		maxlimit,
			float		minlimit,
			int		database,
			char		*output_text)
{
	rtt_t_menu_upd	*menu_ptr;
	rtt_t_menu_upd	*new_menulist;

	if ( allocated == 0)
	{
	  if ( index == 0)
	  {
	    *menulist = calloc( 1 , 2 * sizeof(rtt_t_menu_upd));
	    if ( *menulist == 0)
	      return RTT__NOMEMORY;
	  }
	  else
	  {
/*	    *menulist = realloc( *menulist, (index + 2) * sizeof(rtt_t_menu));
*/
	    new_menulist = calloc( index + 2 , sizeof(rtt_t_menu_upd));
	    if ( new_menulist == 0)
	      return RTT__NOMEMORY;
	    memcpy( new_menulist, *menulist, 
		(index + 1) * sizeof(rtt_t_menu_upd));
	    free( *menulist);
	    *menulist = new_menulist;
	  }
	}
	menu_ptr = *menulist + index;
	strncpy( menu_ptr->text, text, sizeof(menu_ptr->text));
	menu_ptr->func = func;
	menu_ptr->func2 = func2;
	menu_ptr->func3 = func3;
	menu_ptr->argoi = argoi;
	menu_ptr->arg1 = arg1;
	menu_ptr->arg2 = arg2;
	menu_ptr->arg3 = arg3;
	menu_ptr->arg4 = arg4;
	menu_ptr->priv = priv;
	strncpy( menu_ptr->parameter_name, parameter_name, 
		sizeof(menu_ptr->parameter_name));
	menu_ptr->value_ptr = value_ptr;
	menu_ptr->value_type = value_type;
	menu_ptr->flags = flags;
	menu_ptr->size = size;
	menu_ptr->subid = subid;
	menu_ptr->value_x = x;
	menu_ptr->value_y = y;
	menu_ptr->characters = characters;
	menu_ptr->decimals = decimals;
	menu_ptr->maxlimit = maxlimit;
	menu_ptr->minlimit = minlimit;
	menu_ptr->database = database;
	if ( output_text)
	  strncpy( menu_ptr->output_text, output_text, sizeof(menu_ptr->output_text));
	else
	  menu_ptr->output_text[0] = 0;
	menu_ptr->old_value[0] = 255;
	menu_ptr->old_value[1] = 255;
	menu_ptr++;
	strcpy( menu_ptr->text, "");

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
* char		*old_value	I	old value
* unsigned long	init		I	write everything or just changed values
* int		x		I	x koordinate for value
* int		y		I	y koordinate for value
*
* Description:
*	Prints a value on the screen.
*
**************************************************************************/

static int	rtt_print_value(
			char		*value_ptr,
			int		value_type,
			int		flags,
			int		size,
			char		*old_value,
			unsigned long	init,
			int		x,
			int		y,
			unsigned long	priv)
{
	pwr_tObjid		objid;
	pwr_sAttrRef		*attrref;
	int			sts;
	char			timstr[64];
	unsigned long		*dump_ptr;
	int			i;

	if ( value_ptr == 0)
	{
	  /* This was probalby a null pointer in rtdb */
	  if ( init || (strcmp( old_value, "UNDEF PTR") != 0))
	  {
	    rtt_cursor_abs( x, y);
	    rtt_eofline_erase();
	    r_print("UNDEFINED");
	    strcpy( old_value, "UNDEF PTR");
	  }
	  return RTT__SUCCESS;
	}
	else if ( (unsigned long) value_ptr == RTT_ERASE)
	  return RTT__SUCCESS;

	/* If this is a pointer, get the pointer */
	if ( flags & PWR_MASK_POINTER )
 	{
	  /* Check that it is a rtdb pointer */
	  if ( value_ptr == 0)
	  {
	    /* This is not a rtdb pointer */
	    if ( init || (strcmp( old_value, "UNDEF PTR") != 0))
	    {
	      rtt_cursor_abs( x, y);
	      rtt_eofline_erase();
	      r_print("UNDEFINED POINTER");
	      strcpy( old_value, "UNDEF PTR");
	    }
	    return RTT__SUCCESS;
	  }
	}


	if ( !init)
	  if ( memcmp( old_value, value_ptr, size) == 0)
	    /* No change since last time, return */
	    return RTT__SUCCESS;

	rtt_cursor_abs( x, y);
	rtt_eofline_erase();

	if ( rtt_mode_dump)
	{
	  dump_ptr = (unsigned long *) value_ptr;
	  for ( i = 0; 4 * i < size; i++)
	  {
	    if ( size - i * sizeof( *dump_ptr) >= 4)
	      r_print( "%lx ", *dump_ptr);
	    else if ( size - i * sizeof( *dump_ptr) >= 2)
	      r_print( "%hx ", *(unsigned short *) dump_ptr);
	    else
	      r_print( "%hx ", *(unsigned char *)dump_ptr);
	    dump_ptr++;
	  }
	}
	else if ( priv & RTT_OUTPUT_ONOFF )
	{
	  if ( *value_ptr ) 
	  {
	    rtt_char_inverse_start();
	    r_print( "   ON   ");
	    rtt_char_inverse_end();
	  }
	  else
	    r_print( "OFF");
	}
	else if ( priv & RTT_OUTPUT_TRUEFALSE )
	{
	  if ( *value_ptr ) 
	  {
	    rtt_char_inverse_start();
	    r_print( "  TRUE  ");
	    rtt_char_inverse_end();
	  }
	  else
	    r_print( "FALSE");
	}
	else if ( priv & RTT_OUTPUT_OPENCLOSED )
	{
	  if ( *value_ptr ) 
	  {
	    rtt_char_inverse_start();
	    r_print( "  OPEN  ");
	    rtt_char_inverse_end();
	  }
	  else
	    r_print( "CLOSED");
	}
	else if ( priv & RTT_OUTPUT_AUTOMAN )
	{
	  if ( *value_ptr ) 
	  {
	    rtt_char_inverse_start();
	    r_print( "  AUTO  ");
	    rtt_char_inverse_end();
	  }
	  else
	    r_print( "MAN");
	}
	else
	{

	  switch ( value_type )
	  {
	    case pwr_eType_Boolean:
	    {
	      r_print( "%d", *value_ptr);
	      break;
	    }
	    case pwr_eType_Float32:
	    {
	      r_print( "%f", *(float *)value_ptr);
	      break;
	    }
	    case pwr_eType_Float64:
	    {
	      r_print( "%f", *(double *)value_ptr);
	      break;
	    }
	    case pwr_eType_Char:
	    {
	      r_print( "%c", *value_ptr);
	      break;
	    }
	    case pwr_eType_Int8:
	    {
	      r_print( "%d", *value_ptr);
	      break;
	    }
	    case pwr_eType_Int16:
	    {
	      r_print( "%d", *(short *)value_ptr);
	      break;
	    }
	    case pwr_eType_Int32:
	    {
	      r_print( "%d", *(int *)value_ptr);
	      break;
	    }
	    case pwr_eType_Int64:
	    {
	      r_print( pwr_dFormatInt64, *(pwr_tInt64 *)value_ptr);
	      break;
	    }
	    case pwr_eType_UInt8:
	    {
	      r_print( "%d", *(unsigned char *)value_ptr);
	      break;
	    }
	    case pwr_eType_UInt16:
	    {
	      r_print( "%d", *(unsigned short *)value_ptr);
	      break;
	    }
	    case pwr_eType_UInt32:
	    case pwr_eType_Mask:
	    case pwr_eType_Enum:
	    {
	      r_print( "%d", *(unsigned long *)value_ptr);
	      break;
	    }
	    case pwr_eType_UInt64:
	    {
	      r_print( pwr_dFormatUInt64, *(pwr_tUInt64 *)value_ptr);
	      break;
	    }
	    case pwr_eType_String:
	    {
	      r_print( "%s", value_ptr);
	      break;
	    }
	    case pwr_eType_ObjDId:
	    {
	      pwr_tOName     		hiername;

	      objid = *(pwr_tObjid *)value_ptr;
	      if ( !objid.oix)
	        sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
			 cdh_mName_volumeStrict);
	      else
	        sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
			cdh_mNName);
	      if (EVEN(sts)) break;

	      r_print( "%s", &hiername);
	      break;
	    }
	    case pwr_eType_AttrRef:
	    {
	      pwr_tAName     		hiername;

	      attrref = (pwr_sAttrRef *) value_ptr;
	      sts = gdh_AttrrefToName ( attrref, hiername, sizeof(hiername), cdh_mNName);
	      if ( EVEN(sts)) break;

	      r_print( "%s", &hiername);
	      break;
	    }
	    case pwr_eType_Time:
	    {
	      sts = time_AtoAscii( (pwr_tTime *) value_ptr, time_eFormat_DateAndTime, 
			timstr, sizeof(timstr));
	      if ( EVEN(sts))
	        strcpy( timstr, "Undefined time");
	      r_print( "%s", timstr);
	      break;
	    }
	    case pwr_eType_DeltaTime:
	    {
	      sts = time_DtoAscii( (pwr_tDeltaTime *) value_ptr, 1, 
			timstr, sizeof(timstr));
	      if ( EVEN(sts))
	        strcpy( timstr, "Undefined time");
	      r_print( "%s", timstr);
	      break;
	    }
	    case pwr_eType_ObjectIx:
	    {
	      r_print( "%s", cdh_ObjectIxToString( NULL, 
			*(pwr_tObjectIx *) value_ptr, 1));
	      break;
	    }
	    case pwr_eType_ClassId:
	    {
	      pwr_tOName     		hiername;

	      objid = cdh_ClassIdToObjid( *(pwr_tClassId *) value_ptr);
	      sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	      if (EVEN(sts)) break;

	      r_print( "%s", &hiername);
	      break;
	    }
	    case pwr_eType_TypeId:
	    {
	      pwr_tOName     		hiername;

	      objid = cdh_TypeIdToObjid( *(pwr_tTypeId *) value_ptr);
	      sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	      if (EVEN(sts)) break;

	      r_print( "%s", &hiername);
	      break;
	    }
	    case pwr_eType_VolumeId:
	    {
	      r_print( "%s", cdh_VolumeIdToString( NULL, 
					*(pwr_tVolumeId *) value_ptr, 1, 0));
	      break;
	    }
	    case pwr_eType_RefId:
	    {
	      r_print( "%s", cdh_SubidToString( NULL, 
					*(pwr_tSubid *) value_ptr, 1));
	      break;
	    }
	  }
	}
	memcpy( old_value, value_ptr, min(size, 80));

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_upd_update()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.	
*
* Description:
*	Update values in a update menu.
*
**************************************************************************/

int	rtt_menu_upd_update(
			menu_ctx	ctx)
{
	int		item;
	rtt_t_menu_upd	*menu_ptr;
	int		sts;

	/* Get new alarm messages */
	sts = rtt_scan ( 0); 
	if (EVEN(sts)) return sts;

	rtt_update_time();
	for ( item = ctx->current_page * ctx->page_len; 
		(item < ctx->no_items) && ( item < (ctx->current_page + 1) *
		ctx->page_len); item++)
	{
	  menu_ptr = (rtt_t_menu_upd *)	ctx->menu;
	  menu_ptr += item;
	  rtt_print_value( menu_ptr->value_ptr, menu_ptr->value_type, 
		menu_ptr->flags, menu_ptr->size, menu_ptr->old_value,
		ctx->update_init, menu_ptr->value_x, menu_ptr->value_y,
		menu_ptr->priv);
	}	
	r_print_buffer();
	ctx->update_init = 0;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_draw_bar()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Draw a horizontal bar.
*
**************************************************************************/

static int	rtt_draw_bar(
			rtt_t_menu_upd	*menu_ptr,
			unsigned long	init)
{
	int	length;
	int	old_length;
	int	dif_length;

	if ( menu_ptr->maxlimit == menu_ptr->minlimit)
	  return RTT__SUCCESS;

	/* Calculate lenght of the bar */
	length = ( *(float *)menu_ptr->value_ptr - menu_ptr->minlimit) /
		 ( menu_ptr->maxlimit - menu_ptr->minlimit) * 
		 menu_ptr->characters + 0.5;
	if ( length < 0 )
	  length = 0;
	if ( length > menu_ptr->characters)
	  length = menu_ptr->characters;
	
	if ( init)
	{
	  rtt_cursor_abs( menu_ptr->value_x, menu_ptr->value_y);
	  r_print( "%.*s", menu_ptr->characters, 
"                                                                              ");
	  rtt_cursor_abs( menu_ptr->value_x, menu_ptr->value_y);
	  rtt_char_inverse_start();
	  r_print( "%.*s", length, 
"                                                                              ");
	  rtt_char_inverse_end();
	}
	else
	{
	  /* Calculate present length from old_value */
	  old_length = ( *(float *)menu_ptr->old_value - menu_ptr->minlimit) /
		 ( menu_ptr->maxlimit - menu_ptr->minlimit) * 
		 menu_ptr->characters + 0.5;
	  if ( old_length < 0 )
	    old_length = 0;
	  if ( old_length > menu_ptr->characters)
	    old_length = menu_ptr->characters;

	  dif_length = length - old_length;
	  if ( dif_length > 0)
	  {
	    rtt_cursor_abs( menu_ptr->value_x+old_length, menu_ptr->value_y);
	    rtt_char_inverse_start();
	    r_print( "%.*s", dif_length, 
"                                                                              ");
	    rtt_char_inverse_end();
	  }
	  else if ( dif_length < 0)
	  {
	    rtt_cursor_abs( menu_ptr->value_x+length, menu_ptr->value_y);
	    r_print( "%.*s", -dif_length, 
"                                                                              ");
	  }
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
* char		*old_value	I	old value
* unsigned long	init		I	write everything or just changed values
* int		x		I	x koordinate for value
* int		y		I	y koordinate for value
*
* Description:
*	Prints a value on the screen.
*
**************************************************************************/

static int	rtt_edit_print_value(
			rtt_t_menu_upd	*menu_ptr,
			unsigned long	init)
{
	pwr_tObjid	objid;
	int		sts;
	char		*s;
	char		text[120];
	pwr_sAttrRef		*attrref;
	char			timstr[64];

	if ( menu_ptr->priv & RTT_OUTPUT_NO )
	  return RTT__SUCCESS;

	if ( menu_ptr->value_ptr == 0)
	{
	  /* This was probalby a null pointer in rtdb */
	  if ( init || (strcmp( menu_ptr->old_value, "UNDEF PTR") != 0))
	  {
	    rtt_cursor_abs( menu_ptr->value_x, menu_ptr->value_y);
	    rtt_eofline_erase();
	    r_print("UNDEFINED");
	    strcpy( menu_ptr->old_value, "UNDEF PTR");
	  }
	  return RTT__SUCCESS;
	}
	else if ( (unsigned long) menu_ptr->value_ptr == RTT_ERASE)
	{
	  /* This was probalby a null pointer in rtdb */
	  if ( init || ( *(unsigned long *) menu_ptr->old_value != RTT_ERASE))
	  {
	    rtt_cursor_abs( menu_ptr->value_x, menu_ptr->value_y);
	    r_print( "%.*s", menu_ptr->characters, 
			"                             ");
	    *(unsigned long *)menu_ptr->old_value = RTT_ERASE;
	  }
	  return RTT__SUCCESS;
	}

	/* If this is a pointer, get the pointer */
	if ( menu_ptr->flags & PWR_MASK_POINTER )
 	{
	  /* Check that it is a rtdb pointer */
	  if ( menu_ptr->value_ptr == 0)
	  {
	    /* This is not a rtdb pointer */
	    if ( init || (strcmp( menu_ptr->old_value, "UNDEF PTR") != 0))
	    {
	      rtt_cursor_abs( menu_ptr->value_x, menu_ptr->value_y);
	      rtt_eofline_erase();
	      r_print("UNDEFINED POINTER");
	      strcpy( menu_ptr->old_value, "UNDEF PTR");
	    }
	    return RTT__SUCCESS;
	  }
	}

        if ( menu_ptr->priv & RTT_OUTPUT_FLASH)
        {
          if ( rtt_flash)
          {
            /* Erase */
            rtt_cursor_abs( menu_ptr->value_x, menu_ptr->value_y);
            r_print( "%.*s", menu_ptr->characters,
"                                                                               ");
            return RTT__SUCCESS;
          }
          else
            init = 1;
        }

	if ( !init)
	  if ( memcmp( menu_ptr->old_value, menu_ptr->value_ptr, 
			menu_ptr->size) == 0)
	    /* No change since last time, return */
	    return RTT__SUCCESS;

	rtt_cursor_abs( menu_ptr->value_x, menu_ptr->value_y);
	if ( menu_ptr->priv & RTT_OUTPUT_ONOFF )
	{
	  if ( *menu_ptr->value_ptr ) 
	  {
	    rtt_char_inverse_start();
	    r_print( "ON ");
	    rtt_char_inverse_end();
	  }
	  else
	    r_print( "OFF");
	}
	else if ( menu_ptr->priv & RTT_OUTPUT_TRUEFALSE )
	{
	  if ( *menu_ptr->value_ptr ) 
	  {
	    rtt_char_inverse_start();
	    r_print( "TRUE ");
	    rtt_char_inverse_end();
	  }
	  else
	    r_print( "FALSE");
	}
	else if ( menu_ptr->priv & RTT_OUTPUT_OPENCLOSED )
	{
	  if ( *menu_ptr->value_ptr ) 
	  {
	    rtt_char_inverse_start();
	    r_print( "OPEN  ");
	    rtt_char_inverse_end();
	  }
	  else
	    r_print( "CLOSED");
	}
	else if ( menu_ptr->priv & RTT_OUTPUT_AUTOMAN )
	{
	  if ( *menu_ptr->value_ptr ) 
	    r_print( "AUTO");
	  else
	  {
	    rtt_char_inverse_start();
	    r_print( "MAN ");
	    rtt_char_inverse_end();
	  }
	}
	else if ( menu_ptr->priv & RTT_OUTPUT_TEXT )
	{
	  if ( *menu_ptr->value_ptr ) 
	  {
	    strcpy( text, menu_ptr->output_text);
	    if ( (s = strchr( text, '/')))
	      *s = 0;
	  }
	  else
	  {
	    if ( (s = strchr( menu_ptr->output_text, '/')))
	    {
	      s++;
	      strcpy( text, s);
	    }
	    else
	      text[0] = 0;
	  }
	  if ( text[0] == '!')
	  {
	    rtt_char_inverse_start();
	    if ( menu_ptr->characters > 0)
	      r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			&text[1]);
	    else
	    {
	      rtt_eofline_erase();
	      r_print( "%s", &text[1]);
	    }
	    rtt_char_inverse_end();
	  }
          else if ( text[0] == '_' && text[1] == 'L' && text[2] == '_')
          {
            rtt_charset_linedrawing();
            if ( menu_ptr->characters > 0)
              r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters,
                        &text[3]);
            else
            {
              rtt_eofline_erase();
              r_print( "%s", &text[3]);
            }
            rtt_charset_ascii();
          }
 	  else
	  {
	    if ( menu_ptr->characters > 0)
	      r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			text);
	    else
	    {
	      rtt_eofline_erase();
	      r_print( "%s", text);
	    }
	  }
	}
	else
	{
	  if ( menu_ptr->output_text[0] == '!')
	    rtt_char_inverse_start();
	  switch ( menu_ptr->value_type )
	  {
	    case pwr_eType_Boolean:
	    {
	      r_print( "%d", *menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_Float32:
	    {
	      if ( menu_ptr->priv & RTT_OUTPUT_BAR )
	        rtt_draw_bar( menu_ptr, init);
	      else
	      {
	        if ( menu_ptr->characters > 0)
	          r_print( "%*.*f", menu_ptr->characters, menu_ptr->decimals, 
			*(float *)menu_ptr->value_ptr);
	        else
	          r_print( "%g", *(float *)menu_ptr->value_ptr);
	      }
	      break;
	    }
	    case pwr_eType_Float64:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%*.*f", menu_ptr->characters, menu_ptr->decimals, 
			*(double *)menu_ptr->value_ptr);
	      else
	        r_print( "%g", *(double *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_Char:
	    {
	      if ( menu_ptr->characters > 1)
	        r_print( "%*c", menu_ptr->characters, 
			*(char *)menu_ptr->value_ptr);
	      else
	        r_print( "%c", *(char *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_Int8:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%*d", menu_ptr->characters, 
			*(char *)menu_ptr->value_ptr);
	      else
	        r_print( "%d", *(char *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_Int16:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%*d", menu_ptr->characters, *(short *)menu_ptr->value_ptr);
	      else
	        r_print( "%d", *(short *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_Int32:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%*d", menu_ptr->characters, *(int *)menu_ptr->value_ptr);
	      else
	        r_print( "%d", *(int *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_Int64:
	    {
	      if ( menu_ptr->characters > 0)
#if defined OS_LINUX && defined HW_X86_64
	        r_print( "%*ld", menu_ptr->characters, *(pwr_tInt64 *)menu_ptr->value_ptr);
#else
	        r_print( "%*lld", menu_ptr->characters, *(pwr_tInt64 *)menu_ptr->value_ptr);
#endif
	      else
	        r_print( pwr_dFormatInt64, *(pwr_tInt64 *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_UInt8:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%*d", menu_ptr->characters, 
			*(unsigned char *)menu_ptr->value_ptr);
	      else
	        r_print( "%d", *(unsigned char *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_UInt16:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%*d", menu_ptr->characters, 
			*(unsigned short *)menu_ptr->value_ptr);
	      else
	        r_print( "%d", *(unsigned short *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_UInt32:
	    case pwr_eType_Mask:
	    case pwr_eType_Enum:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%*d", menu_ptr->characters, 
			*(unsigned long *)menu_ptr->value_ptr);
	      else
	        r_print( "%d", *(unsigned long *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_UInt64:
	    {
	      if ( menu_ptr->characters > 0)
#if defined OS_LINUX && defined HW_X86_64
	        r_print( "%*lu", menu_ptr->characters, *(pwr_tUInt64 *)menu_ptr->value_ptr);
#else
	        r_print( "%*llu", menu_ptr->characters, *(pwr_tUInt64 *)menu_ptr->value_ptr);
#endif
	      else
	        r_print( pwr_dFormatUInt64, *(pwr_tUInt64 *)menu_ptr->value_ptr);
	      break;
	    }
	    case pwr_eType_String:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			menu_ptr->value_ptr);
	      else
	      {
	        rtt_eofline_erase();
	        r_print( "%s", menu_ptr->value_ptr);
	      }
	      break;
	    }
	    case pwr_eType_ObjDId:
	    {
	      pwr_tOName     	hiername;

	      objid = *(pwr_tObjid *)menu_ptr->value_ptr;
	      if ( !objid.oix)
	        sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
			 cdh_mName_volumeStrict);
	      else
	        sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
			cdh_mNName);
	      if (EVEN(sts))
	      {
	        if ( menu_ptr->characters > 0)
	          r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			"");
	        else
	          rtt_eofline_erase();
	        break;
	      }
	      if ( menu_ptr->decimals)
	      {
	        /* Last segment only */
	        if ( (s = strrchr( hiername, '-')))
	          strcpy( hiername, s+1);
	      }

	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			hiername);
	      else
	      {
	        rtt_eofline_erase();
	        r_print( "%s", &hiername);
	      }
	      break;
	    }
	    case pwr_eType_AttrRef:
	    {
	      pwr_tAName     	hiername;

	      attrref = (pwr_sAttrRef *) menu_ptr->value_ptr;
	      sts = gdh_AttrrefToName ( attrref, hiername, sizeof(hiername), cdh_mNName);
	      if ( EVEN(sts)) break;

	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			hiername);
	      else
	        r_print( "%s", &hiername);
	      break;
	    }
	    case pwr_eType_Time:
	    {
	      if ( menu_ptr->decimals)
	        sts = time_AtoAscii( (pwr_tTime *) menu_ptr->value_ptr, 
			time_eFormat_Time, timstr, sizeof(timstr));
	      else
	        sts = time_AtoAscii( (pwr_tTime *) menu_ptr->value_ptr, 
			time_eFormat_DateAndTime, timstr, sizeof(timstr));
	      if ( EVEN(sts))
	        strcpy( timstr, "Undefined time");
	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			timstr);
	      else
	        r_print( "%s", timstr);
	      break;
	    }
	    case pwr_eType_DeltaTime:
	    {
	      sts = time_DtoAscii( (pwr_tDeltaTime *) menu_ptr->value_ptr, 1,
			timstr, sizeof(timstr));
	      if ( EVEN(sts))
	        strcpy( timstr, "Undefined time");
	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			timstr);
	      else
	        r_print( "%s", timstr);
	      break;
	    }
	    case pwr_eType_ObjectIx:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			cdh_ObjectIxToString( NULL, 
			*((pwr_tObjectIx *) menu_ptr->value_ptr), 1));
	      else
	      {
	        rtt_eofline_erase();
	        r_print( "%s", cdh_ObjectIxToString( NULL, *((pwr_tObjectIx *) 
			menu_ptr->value_ptr), 1));
	      }
	      break;
	    }
	    case pwr_eType_ClassId:
	    {
	      pwr_tOName     	hiername;

	      objid = cdh_ClassIdToObjid( 
			*((pwr_tClassId *) menu_ptr->value_ptr));
	      sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	      if (EVEN(sts))
	      {
	        if ( menu_ptr->characters > 0)
	          r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			"");
	        else
	          rtt_eofline_erase();
	        break;
	      }

	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			hiername);
	      else
	      {
	        rtt_eofline_erase();
	        r_print( "%s", &hiername);
	      }
	      if (EVEN(sts)) break;
	    }
	    case pwr_eType_TypeId:
	    {
	      pwr_tOName     	hiername;

	      objid = cdh_TypeIdToObjid( *((pwr_tTypeId *) menu_ptr->value_ptr));
	      sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	      if (EVEN(sts))
	      {
	        if ( menu_ptr->characters > 0)
	          r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			"");
	        else
	          rtt_eofline_erase();
	        break;
	      }

	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters, 
			hiername);
	      else
	      {
	        rtt_eofline_erase();
	        r_print( "%s", &hiername);
	      }
	      if (EVEN(sts)) break;

	      r_print( "%s", &hiername);
	      break;
	    }
	    case pwr_eType_VolumeId:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters,
			cdh_VolumeIdToString( NULL,
			*((pwr_tVolumeId *) menu_ptr->value_ptr), 1, 0));
	      else
	      {
	        rtt_eofline_erase();
	        r_print( "%s", cdh_VolumeIdToString( NULL,
			*((pwr_tVolumeId *) menu_ptr->value_ptr), 1, 0));
	      }
	      break;
	    }
	    case pwr_eType_RefId:
	    {
	      if ( menu_ptr->characters > 0)
	        r_print( "%-*.*s", menu_ptr->characters, menu_ptr->characters,
			cdh_SubidToString( NULL,
			*((pwr_tSubid *) menu_ptr->value_ptr), 1));
	      else
	      {
	        rtt_eofline_erase();
	        r_print( "%s", cdh_SubidToString( NULL, *((pwr_tSubid *)
			menu_ptr->value_ptr), 1));
	      }
	      break;
	    }
	  }
	  if ( menu_ptr->output_text[0] == '!')
	    rtt_char_inverse_end();
	}
	memcpy( menu_ptr->old_value, menu_ptr->value_ptr,
			min(menu_ptr->size, 80));

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_edit_update()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.	
*
* Description:
*	Update values in a picture.
*
**************************************************************************/

int	rtt_menu_edit_update(
			menu_ctx	ctx)
{
	int		item;
	rtt_t_menu_upd	*menu_ptr;
	int		sts;

        rtt_flash = !rtt_flash;

	/* Get new alarm messages */
	sts = rtt_scan ( 0); 
	if ( EVEN(sts)) return sts;

	if ( ctx->menu == 0)
	  return RTT__SUCCESS;

	/* Call update entry in application function */
	if ( ctx->appl_func != NULL)
	{
	  sts = (ctx->appl_func) ( ctx, RTT_APPL_UPDATE, 0);
	  if ( EVEN(sts)) return sts;
	}

	rtt_update_time();
	for ( item = ctx->current_page * ctx->page_len; 
		(item < ctx->no_items) && ( item < (ctx->current_page + 1) *
		ctx->page_len); item++)
	{
	  menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	  menu_ptr += item;
	  rtt_edit_print_value( menu_ptr, ctx->update_init);
	}	
	r_print_buffer();
	ctx->update_init = 0;

	return RTT__SUCCESS;
}


int dummy( int ctx, pwr_tObjid argoi, void *arg1, void *arg2, void *arg3, void *arg4) 
  { rtt_printf("HELLO in  DUMMY\n"); return RTT__SUCCESS;}

/*************************************************************************
*
* Name:		rtt_objdid_parameter()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context
* pwr_tObjid	objid		I	objid of object
* void		*arg1		I	
* void		*arg2		I	
* void		*arg3		I	
* void		*arg4		I	
*
* Description:
*	Show the object refered to in a parameter of type objdid
*
**************************************************************************/

static int	rtt_objdid_parameter(
			menu_ctx	ctx,
			pwr_tObjid	objid,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	int 		sts;
	rtt_t_menu_upd	*menu_ptr;
	pwr_tObjid	par_objid;
	pwr_tObjid	*objdid_ptr;
	pwr_tClassId	class;

	/* Get current item */
	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;
	
	/* Check that this parameter is an objdid */
	if ( menu_ptr->value_type != pwr_eType_ObjDId)
	{
	  rtt_message('E', "Function not defined");
	  return RTT__NOPICTURE;
	}
	objdid_ptr = (pwr_tObjid *) menu_ptr->value_ptr;
	par_objid = *objdid_ptr;
	sts = gdh_GetObjectClass( par_objid, &class);
	if ( EVEN(sts)) 
	{
	  rtt_message('E', "Object not found");
	  return RTT__NOPICTURE;
	}
	sts = rtt_object_parameters( ctx, par_objid, arg1, arg2, arg3, arg4);
	return sts;
}

/*************************************************************************
*
* Name:		rtt_attribute_elements()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* pwr_tObjid	objid		I	objid of object.
* void		*arg1		I	
* void		*arg2		I	
* void		*arg3		I	
* void		*arg4		I	
*
* Description:
*	Presents an attribute of type array.
*
**************************************************************************/

static int	rtt_attribute_elements(
			menu_ctx	parent_ctx,
			pwr_tObjid	objid,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	int		sts;
	char		*parname;
	char		parnameindex[80];
	char		title[80];
	rtt_t_menu_upd	*menulist = 0;
	rtt_t_menu_upd	*menu_ptr;
	int		i, j;
	unsigned int	elements;
	char		nr[10];
	char 		*parameter_ptr;
	SUBID 		subid;
	pwr_tOName     	parameter_name;
	pwr_tAName     	objname;
	char		classname[80];
	pwr_tObjid	childobjid;
	int		parameter_count = 0;
	pwr_sAttrRef	objar;
	pwr_sAttrRef	aref;
	unsigned int	asize, aoffset;
	pwr_tTypeId	atype;
	int		aflags;
	pwr_tCid	cid;

	objar.Objid = objid;
	objar.Body = (pwr_tCid) ((unsigned long)arg1);
	objar.Offset = (pwr_tUInt32) ((unsigned long)arg2);
	objar.Size = (pwr_tUInt32) ((unsigned long)arg3);
	objar.Flags.m = (pwr_tBitMask) ((unsigned long)arg4);
	
	/* Get object name */
	sts = gdh_AttrrefToName( &objar, objname, sizeof(objname), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Create a title */
	/* Get class name */
	strcpy( title, objname);

	/* Mark if the object has children */
	sts = gdh_GetChild( objid, &childobjid);
	if ( ODD(sts))
	  strcat( title, " *");

	sts = rtt_objidtoclassname( objid, classname);
	if ( EVEN(sts)) return sts;

	strcat( title, "   ");
	strcat( title, classname);

	sts = gdh_GetAttributeCharAttrref( &objar, &atype, &asize, &aoffset, &elements);
	if ( EVEN(sts)) return sts;
	aflags = 0;

	/* Allocate memory for menu list */
	sts = rtt_menu_upd_list_add_malloc( &menulist, elements);
	if ( EVEN(sts)) return sts;

	/* Get rtdb pointer */
	sts = gdh_RefObjectInfo ( objname, (void *) &parameter_ptr, 
		&subid, asize);
	if ( EVEN(sts)) 
	  parameter_ptr = 0;

	i = 0;

	strcpy( title, objname);
	parname = strchr( objname, '.');
	if ( parname == 0) return 0;
	parname++;

	for ( j = 0; j < (int)elements; j++) {
	  if ( rtt_mode_address) {
	    sprintf( parnameindex,"%8lu    ", 
			(unsigned long)(parameter_ptr + rtt_rtdb_offset));
	    strcat( parnameindex, parname);
	  }
	  else
	    strcpy( parnameindex, parname);

	  strcpy( parameter_name, objname);
	  if ( elements > 1) {
	    if ( parameter_ptr != 0)
	      if ( j != 0)
		parameter_ptr += asize / elements;
	    sprintf( nr, "[%d]", j);
	    strcat( parnameindex, nr);
	    strcat( parameter_name, nr);
	  }

#if 0
	  if ( parameter_class == pwr_eClass_Input && rtt_mode_address) {
	    /* Add the content of the pointer */
	    sprintf( parnameindex,"%8u    ", 
			(unsigned int)(parameter_ptr - 4 + rtt_rtdb_offset));
	    strcat( parnameindex, parname);
	    strcat( parnameindex, "P");

	    sts = rtt_menu_upd_list_add( &menulist, i, parameter_count,
		parnameindex,
		0,
		&rtt_objdid_parameter,
		0, objid, 0, 0, 0, 0, 
		parameter_name, RTT_PRIV_NOOP, parameter_ptr - 4, 
		pwr_eType_Int32, 
		aflags, 4, pwr_cNDlid, 0, 0, 0, 0,
		0.0, 0.0, RTT_DATABASE_GDH, 0);
	      if ( EVEN(sts)) return sts;
	    i++;
	  }
#endif

	  /* store subid only for the first element */
	  if ( (j > 0) || (parameter_ptr == 0))
	    subid = pwr_cNDlid;

	  if ( objar.Flags.b.ObjectAttr) {
	    sts = gdh_NameToAttrref( pwr_cNObjid, parameter_name, &aref);
	    if ( EVEN(sts)) return sts;

	    sts = gdh_GetAttrRefTid( &aref, &cid);
	    if ( EVEN(sts)) return sts;
	    
	    sts = gdh_ObjidToName( cdh_ClassIdToObjid( cid),
			       classname, sizeof(classname), cdh_mName_object);
	    if ( EVEN(sts)) return sts;
	    strcat( parnameindex, " ");
	    strcat( parnameindex, classname);
	    strcat( parnameindex, " *");

	    sts = rtt_menu_upd_list_add( &menulist, i, parameter_count, 
		parnameindex, 
		0, 
		&rtt_object_parameters,
		0, objid, (void *)(long)aref.Body, (void *)(long)aref.Offset, 
	        (void *)(long)aref.Size, (void *)(long)(aref.Flags.m | RTT_ISAREF),
		parameter_name, RTT_PRIV_NOOP, parameter_ptr, atype, 
		aflags, asize / elements, subid, 0, 0, 0, 0,
		0.0, 0.0, RTT_DATABASE_GDH, 0);
	    if ( EVEN(sts)) return sts;

            menu_ptr = menulist + i;
	    menu_ptr->value_ptr = (char *) RTT_ERASE;
	  }
	  else {
	    sts = rtt_menu_upd_list_add( &menulist, i, parameter_count, 
		parnameindex, 
		0, 
		&rtt_objdid_parameter,
		0, objid, 0, 0, 0, 0,
		parameter_name, RTT_PRIV_NOOP, parameter_ptr, atype, 
		aflags, asize / elements, subid, 0, 0, 0, 0,
		0.0, 0.0, RTT_DATABASE_GDH, 0);
	    if ( EVEN(sts)) return sts;
	  }
	  i++;
	}

	if ( menulist != 0)
	{
	  sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if ( EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "Unable to open attribute");	  
	  return RTT__NOPICTURE;
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_show_object_as_struct()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Presents the object as a struct described in an includefile.
*
**************************************************************************/
int	rtt_show_object_as_struct( 
			menu_ctx	parent_ctx, 
			pwr_tObjid	objid, 
			char		*type_str, 
			char		*file_str)
{
	dcli_sStructElement *e_list;
	dcli_sStructElement *e_ptr;
	int		sts;
	char		*s;
	char		title[80];
	rtt_t_menu_upd	*menulist = 0;
	int		i;
	char 		*parameter_ptr;
	char 		*object_ptr;
	SUBID 		subid;
	pwr_tOName     	objname;
	char		classname[80];
	pwr_tObjid	childobjid;
	int		parameter_count;
	char		attr_str[80];
        char            *msg;
        char            message[80];

	/* Get object name */
	sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Create a title with objname without volume and classname */
	s = strchr( objname, ':');
	if ( s)
	  s++;
	else
	  s = objname;
	strcpy( title, s);

	/* Mark if the object has children */
	sts = gdh_GetChild( objid, &childobjid);
	if ( ODD(sts))
	  strcat( title, " *");

	sts = rtt_objidtoclassname( objid, classname);
	if ( EVEN(sts)) return sts;

	strcat( title, "   ");
	strcat( title, classname);

	/* Link to object */
	sts = gdh_RefObjectInfo( objname, 
		(pwr_tAddress *) &object_ptr, 
		&subid, 0);
	if ( EVEN(sts)) 
	{
	  rtt_message('E', "Unable to link to object");
	  return RTT__NOPICTURE;
	}	

	/* Find the struct */
	sts = dcli_readstruct_find( file_str, type_str, &e_list);
        if ( EVEN(sts) && dcli_readstruct_get_message( &msg)) {
          strncpy( message, msg, sizeof(message));
          message[sizeof(message)-1] = 0;
          rtt_message( 'E', message);
        }
	if ( EVEN(sts))
	  return RTT__NOPICTURE;

	parameter_count = 0;
	for ( e_ptr = e_list; e_ptr; e_ptr = e_ptr->next)
	{
	  if ( e_ptr->struct_begin)
	    continue;
	  parameter_count++;
	}

	/* Allocate memory for menu list */
	sts = rtt_menu_upd_list_add_malloc( &menulist, parameter_count);
	if ( EVEN(sts)) return sts;

	i = 0;
	parameter_ptr = object_ptr;
	for ( e_ptr = e_list; e_ptr; e_ptr = e_ptr->next)
	{
	  if ( e_ptr->struct_begin)
	    continue;
	  if ( i != 0)
	    subid = pwr_cNDlid;
	  sprintf( attr_str, "_A_ %d %d %d %d", objid.vid, objid.oix,
		(int)(parameter_ptr - object_ptr), e_ptr->size);

	  sts = rtt_menu_upd_list_add( &menulist, i, parameter_count, 
		e_ptr->name, 
		0,
		0,
		0, objid, 0, 0, 0, 0,
		attr_str, RTT_PRIV_NO, parameter_ptr, e_ptr->type, 
		e_ptr->mask, e_ptr->size, subid, 0, 0, 0, 0,
		0.0, 0.0, RTT_DATABASE_GDH, 0);
	      if ( EVEN(sts)) return sts;
	  i++;
	  parameter_ptr += e_ptr->size;
	}
	dcli_readstruct_free( e_list);

	if ( menulist != 0)
	{
	  sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if ( EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "Unable to open object");	  
	  return RTT__NOPICTURE;
	}

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_object_parameters()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* pwr_tObjid	objid		I	objid of object.
* void		*arg1		I	
* void		*arg2		I	
* void		*arg3		I	
* void		*arg4		I	
*
* Description:
*	Presents all parameters and the values of the parameter
*	in an menu.
*
**************************************************************************/

int	rtt_object_parameters(
			menu_ctx	parent_ctx,
			pwr_tObjid	objid,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	int		sts;
	char		parnameindex[80];
	char		classname[80];
	char		*s;
	char		title[250];
	rtt_t_menu_upd	*menulist = 0;
	int		i, j;
	pwr_tClassId	class;
	unsigned long	elements;
	char		nr[10];
	char 		*parameter_ptr;
	pwr_tOid	childobjid;
	pwr_tSubid	subid;
	pwr_tOName     	parameter_name;
	pwr_tAName     	objname;
	int		parameter_count;
	int		hide_elements;
	rtt_t_menu_upd	*menu_ptr;
	int		rows;
	gdh_sAttrDef	*bd;
	int		idx;
	pwr_sAttrRef	objar;
	pwr_sAttrRef	aref;
	int		flags;
	pwr_tCid	cid;

	if ( ((unsigned int) ((unsigned long)arg4) & 0xffff0000) == RTT_ISAREF) {
	  objar.Objid = objid;
	  objar.Body = (pwr_tCid) ((unsigned long)arg1);
	  objar.Offset = (pwr_tUInt32) ((unsigned long)arg2);
	  objar.Size = (pwr_tUInt32) ((unsigned long)arg3);
	  objar.Flags.m = (pwr_tBitMask) ((unsigned long)arg4) & 0xffff;
	}
	else
	  objar = cdh_ObjidToAref( objid);
	
	/* Get object name */
	sts = gdh_AttrrefToName( &objar, objname, sizeof(objname), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Create a title with objname without volume and classname */
	s = strchr( objname, ':');
	if ( s)
	  s++;
	else
	  s = objname;
	strcpy( title, s);

	/* Mark if the object has children */
	if ( objar.Flags.b.Object) {
	  sts = gdh_GetChild( objid, &childobjid);
	  if ( ODD(sts))
	    strcat( title, " *");
	}

	sts = gdh_GetAttrRefTid( &objar, &class);
	if ( EVEN(sts)) return sts;
	
	sts = gdh_ObjidToName( cdh_ClassIdToObjid( class),
			       classname, sizeof(classname), cdh_mName_object);
	if ( EVEN(sts)) return sts;

	strcat( title, "   ");
	strcat( title, classname);

	sts = gdh_GetObjectBodyDef( class, &bd, &rows, objid);
	if ( EVEN(sts)) return sts;

	/* Count the parameters */
	parameter_count = 0;

	for ( i = 0; i < rows; i++) {
	  if ( bd[i].attr->Param.Info.Flags & PWR_MASK_RTVIRTUAL || 
	       bd[i].attr->Param.Info.Flags & PWR_MASK_PRIVATE)
	    /* This parameter does not contain any useful information */
	    continue;

	  elements = 1;
	  if ( bd[i].attr->Param.Info.Flags & PWR_MASK_ARRAY )
	    elements = bd[i].attr->Param.Info.Elements;

	  if ( bd[i].attr->Param.Info.Elements > RTT_HIDE_ELEMENTS ||
	       bd[i].attr->Param.Info.Flags & PWR_MASK_CLASS)
	    elements = 1;

	  for ( j = 0; j < (int)elements; j++)
	    parameter_count++;

	  if ( bd[i].attrClass == pwr_eClass_Input && rtt_mode_address)
	    parameter_count++;
	}

	if ( parameter_count == 0) {
	  rtt_message('E', "Unable to open object");	  
	  return RTT__NOPICTURE;
	}

	/* Allocate memory for menu list */
	sts = rtt_menu_upd_list_add_malloc( &menulist, parameter_count);
	if ( EVEN(sts)) return sts;

	idx = 0;
	for ( i = 0; i < rows; i++) {
	  if ( bd[i].attr->Param.Info.Flags & PWR_MASK_RTVIRTUAL || 
	       bd[i].attr->Param.Info.Flags & PWR_MASK_PRIVATE)
	    /* This parameter does not contain any useful information */
	    continue;

	  /* Get the pointer to the parameter */
	  strcpy( parameter_name, objname);
	  strcat( parameter_name, ".");
	  strcat( parameter_name, bd[i].attrName);
	  /* Get rtdb pointer */
	  sts = gdh_RefObjectInfo ( parameter_name, 
		(pwr_tAddress *) &parameter_ptr, 
		&subid, bd[i].attr->Param.Info.Size);
	  if ( EVEN(sts))
	    parameter_ptr = 0;

	  /* gdh returns the pointer (not a pointer to a pointer) 
	     remove the pointer bit in Flags */
	  flags = bd[i].attr->Param.Info.Flags & ~PWR_MASK_POINTER;

	  elements = 1;
	  if ( bd[i].attr->Param.Info.Flags & PWR_MASK_ARRAY )
	    elements = bd[i].attr->Param.Info.Elements;

	  hide_elements = 0;
	  if ( bd[i].attr->Param.Info.Elements > RTT_HIDE_ELEMENTS) {
 	    elements = 1;
	    hide_elements = 1;
	  }

	  for ( j = 0; j < (int)elements; j++) {
	    if ( rtt_mode_address) {
	      sprintf( parnameindex,"%8lu    ", 
			(unsigned long)(parameter_ptr + rtt_rtdb_offset));
	      strcat( parnameindex, bd[i].attrName);
	    }
	    else
	      strcpy( parnameindex, bd[i].attrName);
	    strcpy( parameter_name, objname);
	    strcat( parameter_name, ".");
	    strcat( parameter_name, bd[i].attrName);
	    if ( elements > 1) {
	      if ( parameter_ptr != 0)
	        if ( j != 0)
	          parameter_ptr += bd[i].attr->Param.Info.Size / elements;
	      sprintf( nr, "%d", j);
	      strcat( parnameindex,"[");
	      strcat( parnameindex, nr);
	      strcat( parnameindex,"]");
	      strcat( parameter_name,"[");
	      strcat( parameter_name, nr);
	      strcat( parameter_name,"]");
	    }

	    if ( bd[i].attrClass == pwr_eClass_Input && rtt_mode_address) {
	      /* Add the content of the pointer */
	      sprintf( parnameindex,"%8lu    ",
			(unsigned long)(parameter_ptr - 4 + rtt_rtdb_offset));
	      strcat( parnameindex, bd[i].attrName);
	      strcat( parnameindex, "P");

	      sts = rtt_menu_upd_list_add( &menulist, idx, parameter_count,
			parnameindex,
			0,
			&rtt_objdid_parameter,
			0, objid, 0, 0, 0, 0,
			parameter_name, RTT_PRIV_NOOP, parameter_ptr - 4, 
			pwr_eType_Int32, 
			flags, 4, pwr_cNDlid, 0, 0, 0, 0,
			0.0, 0.0, RTT_DATABASE_GDH, 0);
	      if ( EVEN(sts)) return sts;
	      idx++;
	    }

	    /* store subid only for the first element */
	    if ( (j > 0) || (parameter_ptr == 0))
	      subid = pwr_cNDlid;

	    if ( bd[i].attr->Param.Info.Flags & PWR_MASK_CLASS &&
		 !(bd[i].attr->Param.Info.Flags & PWR_MASK_ARRAY)) {
 	      sts = gdh_NameToAttrref( pwr_cNObjid, parameter_name, &aref);
	      if ( EVEN(sts)) return sts;

	      sts = gdh_GetAttrRefTid( &aref, &cid);
	      if ( EVEN(sts)) return sts;
	    
	      sts = gdh_ObjidToName( cdh_ClassIdToObjid( cid),
				     classname, sizeof(classname), cdh_mName_object);
	      if ( EVEN(sts)) return sts;
	      strcat( parnameindex, " ");
	      strcat( parnameindex, classname);
	      strcat( parnameindex, " *");

	      sts = rtt_menu_upd_list_add( &menulist, idx, parameter_count,
			parnameindex,
			0,
			&rtt_object_parameters,
			0, objid, (void *)(long)aref.Body, (void *)(long)aref.Offset, 
		        (void *)(long)aref.Size, (void *)(long)(aref.Flags.m | RTT_ISAREF),
			parameter_name, RTT_PRIV_NO, parameter_ptr - 4,
			pwr_eType_Int32,
			flags, 4, pwr_cNDlid, 0, 0, 0, 0,
			0.0, 0.0, RTT_DATABASE_USER, 0);
	      if ( EVEN(sts)) return sts;
	      menu_ptr = menulist + idx;
	      menu_ptr->value_ptr = (char *) RTT_ERASE;
	    }
	    else if ( !hide_elements) {
	      sts = rtt_menu_upd_list_add( &menulist, idx, parameter_count, 
		parnameindex, 
		0, 
		&rtt_objdid_parameter,
		0, objid, 0, 0, 0, 0,
		parameter_name, RTT_PRIV_NOOP, parameter_ptr, 
		bd[i].attr->Param.Info.Type, 
		flags, 
		bd[i].attr->Param.Info.Size / elements, subid, 0, 0, 0, 0,
		0.0, 0.0, RTT_DATABASE_GDH, 0);
	      if ( EVEN(sts)) return sts;
 	    }
	    else 
	    {
 	      sts = gdh_NameToAttrref( pwr_cNObjid, parameter_name, &aref);
	      if ( EVEN(sts)) return sts;

	      strcat( parnameindex, " *");
	      sts = rtt_menu_upd_list_add( &menulist, idx, parameter_count,
			parnameindex,
			0,
			&rtt_attribute_elements,
		        0, objid, (void *)(long)aref.Body, (void *)(long)aref.Offset, 
			(void *)(long)aref.Size, (void *)(long)aref.Flags.m,
			parameter_name, RTT_PRIV_NO, parameter_ptr - 4,
			pwr_eType_Int32,
			flags, 4, pwr_cNDlid, 0, 0, 0, 0,
			0.0, 0.0, RTT_DATABASE_USER, 0);
	      if ( EVEN(sts)) return sts;
	      menu_ptr = menulist + idx;
	      if ( (s = strrchr( menu_ptr->parameter_name, '.'))) {
	        menu_ptr->value_ptr = (char *) RTT_ERASE;
	      }
	    }
	    idx++;
	  }
	}
	free( (char *)bd);

	if ( menulist != 0)
	{
	  sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if ( EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "Unable to open object");	  
	  return RTT__NOPICTURE;
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_show_file()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Displays all files with matching a wilcard specification.
*
**************************************************************************/

int	rtt_show_file(
			menu_ctx	parent_ctx,
			char		*filename,
			char		*command,
			char		*intitle)
{
	int		sts;
	char		title[80] = "FILE LIST";
	pwr_tFileName  	found_file;
	rtt_t_menu	*menulist = 0;
	rtt_t_menu	*menu_ptr;
	int		i;
	int		file_count;
	char		dev[80], dir[80], file[80], type[80];
	int		version;
	int		hide_dir;
        pwr_tFileName  	file_spec;
	char		text[80];
	char		*arg1_ptr[500];
	int		arg1_ptr_size = sizeof(arg1_ptr)/sizeof(arg1_ptr[0]);
	int		arg1_ptr_count;

	if ( intitle)
	  strcpy( title, intitle);

	if ( *filename == '!')
	{
	  hide_dir = 1;
	  strcpy( file_spec, filename + 1);
	}
	else
	{
	  hide_dir = 0;
	  strcpy( file_spec, filename);
	}
	if ( command)
	{
	  /* The title should be the text of the parent item */
	  rtt_menu_get_parent_text( parent_ctx, title);
	}

	sts = rtt_search_file( file_spec, found_file, RTT_DIR_SEARCH_INIT);
	if ( EVEN(sts)) return sts;

	menulist = 0;
	file_count = 0;
	arg1_ptr_count = 0;
	if ( hide_dir)
	{
	  rtt_parse_filename( found_file, dev, dir, file, type, &version);
	  strcpy( text, file);
	}
	else
	  strcpy( text, found_file);
	sts = rtt_menu_list_add( &menulist, file_count, 0, text, 
		NULL, NULL, NULL,
		pwr_cNObjid,0,0,0,0);
	if ( EVEN(sts)) return sts;
	if ( command)
	{
	  menu_ptr = menulist + file_count;
	  menu_ptr->func = &rtt_menu_command;
	  menu_ptr->arg1 = (void *) command;
	}
	else if ( strstr( found_file, ".RTT_COM") ||
		  strstr( found_file, ".TXT") ||
		  strstr( found_file, ".rtt_com") ||
		  strstr( found_file, ".txt"))
	{
	  if ( arg1_ptr_count < arg1_ptr_size)
	  {
	    menu_ptr = menulist + file_count;
	    menu_ptr->func = &rtt_menu_execute_file;
	    menu_ptr->arg1 = calloc( 1, 256);
	    strcpy( menu_ptr->arg1, found_file);
	    arg1_ptr[arg1_ptr_count++] = (char *) menu_ptr->arg1;
	  }
	}
	file_count++;

	while ( ODD(sts))
	{
	  sts = rtt_search_file( file_spec, found_file, RTT_DIR_SEARCH_NEXT);
	  if ( ODD(sts))
	  {
	    if ( hide_dir)
	    {
	      rtt_parse_filename( found_file, dev, dir, file, type, &version);
	      strcpy( text, file);
	    }
	    else
	      strcpy( text, found_file);
	    sts = rtt_menu_list_add( &menulist, file_count, 0, text, 
		NULL, NULL, NULL, pwr_cNObjid,0,0,0,0);
	    if ( EVEN(sts)) return sts;
	    if ( command)
	    {
	      menu_ptr = menulist + file_count;
	      menu_ptr->func = &rtt_menu_command;
	      menu_ptr->arg1 = (void *) command;
	    }
	    else if ( strstr( found_file, ".RTT_COM") ||
		      strstr( found_file, ".TXT") ||
		      strstr( found_file, ".rtt_com") ||
		      strstr( found_file, ".txt"))
	    {
	      if ( arg1_ptr_count < arg1_ptr_size)
	      {
	        menu_ptr = menulist + file_count;
	        menu_ptr->func = &rtt_menu_execute_file;
	        menu_ptr->arg1 = calloc( 1, 256);
	        strcpy( menu_ptr->arg1, found_file);
	        arg1_ptr[arg1_ptr_count++] = (char *) menu_ptr->arg1;
	      }
	    }
	    file_count++;
	  }
	}
	rtt_search_file( filename, found_file, RTT_DIR_SEARCH_END);

	sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	for ( i = 0; i < arg1_ptr_count; i++)
	  free( arg1_ptr[i]);
	if ( sts == RTT__FASTBACK) return sts;
	else if ( sts == RTT__BACKTOCOLLECT) return sts;
	else if (EVEN(sts)) return sts;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_hierarchy_child()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* pwr_tObjid	parent_objid	I	objid of parent object.
* void		*arg1		I	
* void		*arg2		I	
* void		*arg3		I	
* void		*arg4		I	
*
* Description:
*	Displays all children of an object in a menu.
*
**************************************************************************/

int	rtt_hierarchy_child(
			menu_ctx	parent_ctx,
			pwr_tObjid	parent_objid,
			void		*arg1,
			void		*arg2,
			void 		*arg3,
			void		*arg4)
{
	int		sts;
	pwr_tObjid	objid;
	pwr_tOName     	hiername;
	char		classname[80];
	char		description[80];
	pwr_tAName     	objname;
	char		*s;
	char     	title[250];
	rtt_t_menu	*menulist = 0;
	int		i, j;
	pwr_tObjid	childobjid;
	int		object_count;

	sts = gdh_ObjidToName ( parent_objid, title, sizeof(title), cdh_mNName);
	if ( EVEN(sts)) return sts;

	/* Count the children */
	object_count = 0;
	sts = gdh_GetChild( parent_objid, &objid);
	while ( ODD(sts))
	{
	  object_count++;
	  sts = gdh_GetNextSibling ( objid, &objid);
	}

	if ( object_count == 0)
	{
	  rtt_message('E', "End of hierarchy");
	  return RTT__NOPICTURE;
	}

	/* Allocate memory */
	sts = rtt_menu_list_add_malloc( &menulist, object_count);
	if ( EVEN(sts)) return sts;

	i = 0;
	sts = gdh_GetChild( parent_objid, &objid);
	while ( ODD(sts))
	{
	  sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	  if ( EVEN(sts)) return sts;

	  /* Skip hierarchy */
	  s = strrchr( hiername, '-');
	  if ( s == 0)
	    strcpy( objname, hiername);
	  else
	    strcpy( objname, s + 1);
	
	  /* Get class name */	
	  sts = rtt_objidtoclassname( objid, classname);
	  if (EVEN(sts)) return sts;

	  /* Add class name to objname in title */
	  for ( j = strlen( objname); j < 15; j++)
	    strcat( objname, " ");
	  strcat( objname, " ");
	  strcat( objname, classname);

	  /* Mark if the object has children */
	  sts = gdh_GetChild( objid, &childobjid);
	  if ( ODD(sts))
	    strcat( objname, " *");
	  if ( rtt_description_on)
	  {
	    /* Get the description attribute if there is one */
	    strcat( hiername, ".Description");
	    sts = gdh_GetObjectInfo ( hiername, &description, 
			sizeof(description)); 
	    if ( EVEN(sts))
	      strcpy( description, "");
	    for ( j = strlen( objname); j < 30; j++)
	      strcat( objname, " ");
	    strcat( objname, " ");
	    strncat( objname, description, sizeof(objname)-strlen(objname));
	    objname[sizeof(objname)-1] = 0;
	  }

	  sts = rtt_menu_list_add( &menulist, i, object_count, objname, 
		&rtt_hierarchy_child,
		&rtt_object_parameters, 
		&rtt_debug_child,
		objid,0,0,0,0);
	  if ( EVEN(sts)) return sts;

	  sts = gdh_GetNextSibling ( objid, &objid);
	  i++;
	}

	sts = rtt_menu_classort( menulist, 0);
	sts = rtt_menu_bubblesort( menulist);
	sts = rtt_menu_classort( menulist, 1);
	sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	if ( sts == RTT__FASTBACK) return sts;
	else if ( sts == RTT__BACKTOCOLLECT) return sts;
	else if (EVEN(sts)) return sts;

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_hierarchy()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	parent_ctx	I	
* pwr_tObjid	argoi		I	
* void		*arg1		I	
* void		*arg2		I	
* void		*arg3		I	
* void		*arg4		I	
*
* Description:
*	Displays the plant and node hierarchy roots in rtdb in a menu.
*
**************************************************************************/

int	rtt_hierarchy(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	int		sts;
	pwr_tOName     	objname;
	pwr_tOName     	hiername;
	char		title[] = "OBJECT HIERARCHY";
	pwr_tObjid	objid;
	rtt_t_menu	*menulist = 0;
	int		i, j;
	pwr_tClassId	class;
	char		*s;
	pwr_tObjid	childobjid;

	IF_NOGDH_RETURN;
	i = 0;
	sts = gdh_GetRootList( &objid);
	while ( ODD(sts))
	{
	  sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mNName);
	  if ( EVEN(sts)) return sts;

	  /* Get class name */	
	  sts = gdh_GetObjectClass ( objid, &class);
	  if ( EVEN(sts))
	  {
	    sts = gdh_GetNextSibling ( objid, &objid);
	    continue;
	  }	  
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
		sizeof(hiername), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;

	  /* Skip hierarchy of classname */
	  for ( j = strlen( objname); j < 15; j++)
	    strcat( objname, " ");
	  strcat( objname, " ");
	  s = strrchr( hiername, '-');
	  if ( s == 0)
	    strcat( objname, hiername);
	  else
	    strcat( objname, s + 1);

	  /* Mark if the object has children */
	  sts = gdh_GetChild( objid, &childobjid);
	  if ( ODD(sts))
	     strcat( objname, " *");
/* NYDATABAS
	  else if ( (strcmp( hiername, "pwrs:Class-$NodeHier") == 0) ||
	              (strcmp( hiername, "pwrs:Class-$PlantHier") == 0))
	  {
	    sts = gdh_GetNextSibling ( objid, &objid);
	    continue;
	  }
*/
	  sts = rtt_menu_list_add( &menulist, i, 0, objname, 
		&rtt_hierarchy_child,
		&rtt_object_parameters, 
		&rtt_debug_child,
		objid,0,0,0,0);
	  if ( EVEN(sts)) return sts;
	  i++;

	  sts = gdh_GetNextSibling ( objid, &objid);
	}

	if ( menulist != 0)
	{
	  sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if ( EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "End of hierarchy");
	  return RTT__NOPICTURE;
	}

	return 1;
}

/*************************************************************************
*
* Name:		rtt_class_hierarchy()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* pwr_tObjid 	argoi		I	
* void 		*arg1		I	
* void 		*arg2		I	
* void		*arg3		I	
* void		*arg4		I	
*
* Description:
*	Display the roots of the class hierarchy in a menu.
*
**************************************************************************/

int	rtt_class_hierarchy(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	int		sts;
	pwr_tOName     	objname;
	pwr_tOName     	hiername;
	char		title[] = "CLASS HIERARCHY";
	pwr_tObjid	objid;
	rtt_t_menu	*menulist = 0;
	int		i, j;
	pwr_tClassId	class;
	char		*s;

	i = 0;
	sts = gdh_GetRootList( &objid);
	while ( ODD(sts))
	{
	  sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mNName);
	  if ( EVEN(sts)) return sts;
	  if ( 1 /* Should be if the object is on this node */) 
	  {
	    /* Get class name */	
	    sts = gdh_GetObjectClass( objid, &class);
	    if ( EVEN(sts)) return sts;
	    sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
			sizeof(hiername), cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	  

	    if ( (strcmp( hiername, "pwrs:Class-$ClassHier") == 0) ||
	       (strcmp( hiername, "pwrs_Class-$TypeHier") == 0))
	    {
	      /* Skip hierarchy of classname */
	      for ( j = strlen( objname); j < 15; j++)
	        strcat( objname, " ");
	      strcat( objname, " ");
	      s = strrchr( hiername, '-');
	      if ( s == 0)
	        strcat( objname, hiername);
	      else
	        strcat( objname, s + 1);

	      sts = rtt_menu_list_add( &menulist, i, 0, objname, 
	 	&rtt_hierarchy_child,
		&rtt_object_parameters, 
		0, objid,0,0,0,0);
	      if ( EVEN(sts)) return sts;
	      i++;
	    }
	  }
	  sts = gdh_GetNextSibling ( objid, &objid);
	}

	if ( menulist != 0)
	{
	  sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
			RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if ( EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "End of hierarchy");
	  return RTT__NOPICTURE;
	}

	return 1;
}

/*************************************************************************
*
* Name:		rtt_debug_child()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	
* pwr_tObjid	parent_objid	I	
* void		*arg1		I	
* void		*arg2		I	
* void		*arg3		I	
* void		*arg4		I	
*
* Description:
*	Display all children and the value of the debug parameter in
*	a menu. If no debugparameter is displayed, the object is not
*	displayed.
*
**************************************************************************/

int	rtt_debug_child(
			menu_ctx	parent_ctx,
			pwr_tObjid	parent_objid,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	int		sts;
	char		title[250];
	rtt_t_menu_upd	*menulist = 0;
	int		index = 0;
	pwr_tObjid	objid;
	pwr_tClassId	parent_class;
	pwr_tClassId	class;
	char		classname[80];
	pwr_tObjid	window_objid;
	int		childs = 0;
	int		object_count;

	sts = gdh_GetObjectClass ( parent_objid, &parent_class);
	if ( EVEN(sts)) return sts;

	/* Check if this object has a window as a child, and only one child */
	window_objid = pwr_cNObjid;
	sts = gdh_GetChild( parent_objid, &objid);
	if (ODD(sts))
	{
	  sts = gdh_GetObjectClass ( objid, &class);
	  if ( EVEN(sts)) return sts;
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), classname, 
			sizeof(classname), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;

	  if ( (strcmp( classname, "pwrb:Class-WindowPlc") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowOrderact") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowCond") == 0) ||
	       (strcmp( classname, "pwrb:Class-WindowSubstep") == 0))
	  {
	    childs++;
	    window_objid = objid;
	  }
	}
	sts = gdh_GetNextSibling ( objid, &objid);
	if ( cdh_ObjidIsNotNull( window_objid) && ( EVEN(sts)))
	  parent_objid = window_objid;

	sts = gdh_ObjidToName ( parent_objid, title, sizeof(title), cdh_mNName);
	if ( EVEN(sts)) return sts;

	/* Count the children with debugparameter defined */
	object_count = 0;
	sts = gdh_GetChild( parent_objid, &objid);
	while ( ODD(sts))
	{
	  sts = rtt_debug_child_check( objid);
	  if ( ( ODD(sts)) && (sts != RTT__ITEM_NOCREA))
	    /* There is a debug parameter */
	    object_count++;
	  else if ( EVEN(sts)) return sts;
	  sts = gdh_GetNextSibling ( objid, &objid);
	}

	if ( object_count == 0)
	{
	  rtt_message('E', "No debug objects found");
	  return RTT__NOPICTURE;
	}

	/* Allocate memory for menu list */
	sts = rtt_menu_upd_list_add_malloc( &menulist, object_count);
	if ( EVEN(sts)) return sts;

	sts = gdh_GetChild( parent_objid, &objid);
	while ( ODD(sts))
	{

	  sts = rtt_debug_child_add( objid, &menulist, &index, 
			&object_count, 0, 0);
	  if ( EVEN(sts)) return sts;
	  sts = gdh_GetNextSibling ( objid, &objid);
	}

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
	  rtt_message('E', "No debug objects found");
	  return RTT__NOPICTURE;
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_exit()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Exit.
*
**************************************************************************/

int 	rtt_exit( int ctx, pwr_tObjid argoi, void *arg1, void *arg2, 
		void *arg3, void *arg4) 
{
	rtt_exit_now(0, RTT__SUCCESS);
	return 1;
}


/*************************************************************************
*
* Name:		rtt_set_value()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
* char		*value_str	I	
*
* Description:
*	Set the value of current item in an update menu.
*
**************************************************************************/

static int	rtt_set_value(
			menu_ctx	ctx,
			char		*value_str)
{
	rtt_t_menu_upd	*menu_ptr;
	char		buffer[80];
	char		*buffer_ptr = buffer;
	pwr_tObjid	objid;
	pwr_sAttrRef	attrref;
	pwr_tTime	time;
	pwr_tDeltaTime	deltatime;
	pwr_tRefId	subid;
	pwr_tObjectIx	objectix;
	pwr_tTypeId	typeid;
	pwr_tVolumeId	volumeid;
	pwr_tClassId	class;
	int		sts;
	int		size;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;

	/* Check authorization */
	if ( !(rtt_priv & menu_ptr->priv) )
	{
	  rtt_message('E',"Not authorized for this operation");
	  return RTT__NOPRIV;
	}

	/* Check if input is allowed */
	if ( (RTT_MENU_NOINPUT	& menu_ptr->priv) )
	{
	  rtt_message('E',"No change of value allowed");
	  return RTT__SUCCESS;
	}

	switch ( menu_ptr->value_type )
	{
	  case pwr_eType_Boolean:
	  {
	    size = sizeof( pwr_tBoolean);
	    if ( sscanf( value_str, "%d", (pwr_tBoolean *)buffer_ptr) != 1)
	      return RTT__INPUT_SYNTAX;
	    if ( (*buffer_ptr < 0) || (*buffer_ptr > 1))
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) && (menu_ptr->minlimit != 0.0))
	    {
	      if ( *buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_Float32:
	  {
	    size = sizeof(pwr_tFloat32);
	    if ( sscanf( value_str, "%f", (float *)buffer_ptr) != 1)
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(float *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(float *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_Float64:
	  {
	    pwr_tFloat32 f;
	    pwr_tFloat64 d;
	    size = sizeof(pwr_tFloat64);
	    if ( sscanf( value_str, "%f", &f) != 1)
	      return RTT__INPUT_SYNTAX;
	    d = f;
	    memcpy( buffer_ptr, (char *) &d, sizeof(d));
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(double *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(double *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_Char:
	  {
	    size = sizeof(pwr_tChar);
	    if ( sscanf( value_str, "%c", buffer_ptr) != 1)
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_Int8:
	  {
	    pwr_tInt8 	i8;
	    pwr_tInt16	i16;
	    size = sizeof(pwr_tInt8);
#if defined OS_ELN
	    if ( sscanf( value_str, "%d", &i16) != 1)
#else
	    if ( sscanf( value_str, "%hd", &i16) != 1)
#endif
	      return RTT__INPUT_SYNTAX;
	    i8 = i16;
	    memcpy( buffer_ptr, (char *)&i8, sizeof(i8));
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_Int16:
	  {
	    size = sizeof(pwr_tInt16);
#if defined OS_ELN
	    if ( sscanf( value_str, "%d", (short *)buffer_ptr) != 1)
#else
	    if ( sscanf( value_str, "%hd", (short *)buffer_ptr) != 1)
#endif
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(short *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(short *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_Int32:
	  {
	    size = sizeof(pwr_tInt32);
	    if ( sscanf( value_str, "%d", (int *)buffer_ptr) != 1)
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(int *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(int *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_Int64:
	  {
	    size = sizeof(pwr_tInt64);
	    if ( sscanf( value_str, pwr_dFormatInt64, (pwr_tInt64 *)buffer_ptr) != 1)
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(int *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(int *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_UInt8:
	  {
	    pwr_tUInt8 	i8;
	    pwr_tUInt16	i16;
	    size = sizeof(pwr_tUInt8);
#if defined OS_ELN
	    if ( sscanf( value_str, "%d", (unsigned char *) &i16) != 1)
#else
	    if ( sscanf( value_str, "%hu", &i16) != 1)
#endif
	      return RTT__INPUT_SYNTAX;
	    i8 = i16;
	    memcpy( buffer_ptr, (char *)&i8, sizeof(i8));
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(unsigned char *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(unsigned char *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_UInt16:
	  {
	    size = sizeof(pwr_tUInt16);
#if defined OS_ELN
	    if ( sscanf( value_str, "%d", (unsigned short *)buffer_ptr) != 1)
#else
	    if ( sscanf( value_str, "%hu", (unsigned short *)buffer_ptr) != 1)
#endif
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(unsigned short *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(unsigned short *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_UInt32:
	  case pwr_eType_Mask:
	  case pwr_eType_Enum:
	  {
	    size = sizeof(pwr_tUInt32);
#if defined OS_ELN
	    if ( sscanf( value_str, "%d", (unsigned long *)buffer_ptr) != 1)
#else
	    if ( sscanf( value_str, "%lu", (unsigned long *)buffer_ptr) != 1)
#endif
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(unsigned long *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(unsigned long *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_UInt64:
	  {
	    size = sizeof(pwr_tUInt64);
	    if ( sscanf( value_str, pwr_dFormatUInt64, (pwr_tUInt64 *)buffer_ptr) != 1)
	      return RTT__INPUT_SYNTAX;
	    if ( (menu_ptr->maxlimit != 0.0) || (menu_ptr->minlimit != 0.0))
	    {
	      if ( *(unsigned long *)buffer_ptr > menu_ptr->maxlimit)
	        return RTT__MAXLIMIT;
	      if ( *(unsigned long *)buffer_ptr < menu_ptr->minlimit)
	        return RTT__MINLIMIT;
	    }
	    break;
	  }
	  case pwr_eType_String:
	  {
	    size = menu_ptr->size;
	    if ( strlen( value_str) >= menu_ptr->size)
	      return RTT__STRINGTOLONG;
	    strncpy( buffer_ptr, value_str, 
			min(menu_ptr->size, sizeof(buffer)));
	    break;
	  }
	  case pwr_eType_ObjDId:
	  {
	    size = sizeof(pwr_tObjid);
	    sts = gdh_NameToObjid ( value_str, &objid);
	    if (EVEN(sts)) return RTT__OBJNOTFOUND;

	    memcpy( buffer_ptr, &objid, sizeof(objid));
	    break;
	  }
	  case pwr_eType_ClassId:
	  {
	    size = sizeof(pwr_tClassId);
	    sts = gdh_NameToObjid ( value_str, &objid);
	    if (EVEN(sts)) return RTT__OBJNOTFOUND;

	    class = cdh_ClassObjidToId( objid);
	    memcpy( buffer_ptr, (char *) &class, sizeof(class));
	    break;
	  }
	  case pwr_eType_TypeId:
	  {
	    size = sizeof(pwr_tTypeId);
	    sts = gdh_NameToObjid ( value_str, &objid);
	    if (EVEN(sts)) return RTT__OBJNOTFOUND;

	    typeid = cdh_TypeObjidToId( objid);
	    memcpy( buffer_ptr, (char *) &typeid, sizeof(typeid));
	    break;
	  }
	  case pwr_eType_ObjectIx:
	  {
	    sts = cdh_StringToObjectIx( value_str, &objectix); 
	    if (EVEN(sts)) return RTT__OBJNOTFOUND;

	    memcpy( buffer_ptr, (char *) &objectix, sizeof(objectix));
	    break;
	  }
	  case pwr_eType_VolumeId:
	  {
	    sts = cdh_StringToVolumeId( value_str, &volumeid); 
	    if (EVEN(sts)) return RTT__OBJNOTFOUND;

	    memcpy( buffer_ptr, (char *) &volumeid, sizeof(volumeid));
	    break;
	  }
	  case pwr_eType_RefId:
	  {
	    sts = cdh_StringToSubid( value_str, &subid);
	    if (EVEN(sts)) return RTT__OBJNOTFOUND;

	    memcpy( buffer_ptr, (char *) &subid, sizeof(subid));
	    break;
	  }
	  case pwr_eType_AttrRef:
	  {
	    size = sizeof( pwr_sAttrRef);
	    sts = gdh_NameToAttrref ( pwr_cNObjid, value_str, &attrref);
	    if (EVEN(sts)) return RTT__OBJNOTFOUND;

	    memcpy( buffer_ptr, &attrref, sizeof(attrref));
	    break;
	  }
	  case pwr_eType_Time:
	  {
	    size = sizeof( pwr_tTime);
	    sts = time_AsciiToA( value_str, &time);
	    if (EVEN(sts)) return RTT__INPUT_SYNTAX;

	    memcpy( buffer_ptr, (char *) &time, sizeof(time));
	    break;
	  }
	  case pwr_eType_DeltaTime:
	  {
	    size = sizeof( pwr_tDeltaTime);
	    sts = time_AsciiToD( value_str, &deltatime);
	    if (EVEN(sts)) return RTT__INPUT_SYNTAX;

	    memcpy( buffer_ptr, (char *) &deltatime, sizeof(deltatime));
	    break;
	  }
	}

	if ( menu_ptr->database == RTT_DATABASE_GDH)
	{
/*	  if ( menu_ptr->subid == 0)
 	  {
	     This is an element of an array, and not the first element 
	    memcpy( menu_ptr->value_ptr, buffer_ptr, menu_ptr->size);
	    rtt_message('W', 
	      "Warning, if this is a remote object the pararmeter will not be set"
	      );
	  }
	  else
*/
	  if ( strncmp( menu_ptr->parameter_name, "_A_ ", 4) == 0)
	  {
	    /* Parameter is given as an attref */
	    sscanf( &menu_ptr->parameter_name[4], "%d %d %d %d",
		&attrref.Objid.vid, &attrref.Objid.oix, 
		&attrref.Offset, &attrref.Size);
	    sts = gdh_SetObjectInfoAttrref( &attrref, buffer_ptr, size);
	    if ( EVEN(sts)) rtt_message_sts( sts);
	  }
	  else
	  {
	    sts = gdh_SetObjectInfo ( menu_ptr->parameter_name, 
		buffer_ptr, size);
	    if ( EVEN(sts)) rtt_message_sts( sts);
	  }
	}
	else
	{
	  memcpy( menu_ptr->value_ptr, buffer_ptr, menu_ptr->size);
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_value()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
* int		timeout		I	
* int		(* timeout_func) () I	
* void		*timeout_arg	I	
* char		*prompt		I	
* int		x		I	
* int		y		I	
*
* Description:
*	Read an input value.
*
**************************************************************************/

static int	rtt_get_value(
			menu_ctx	ctx,
			int		timeout,
			int		(* timeout_func) (),
			void 		*timeout_arg,
			char		*prompt,
			int		x,
			int		y)
{
	unsigned long	terminator;
	unsigned long	option;
	char		input_str[80];
	int		maxlen = 30;
	rtt_t_menu_upd	*menu_ptr;
	int		sts;

	option = RTT_OPT_TIMEOUT | RTT_OPT_NOSCROLL;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;

	/* Check authorization */
	if ( menu_ptr->priv == RTT_PRIV_NO )
	{
	  rtt_message('E',"Value can not be changed");
	  return RTT__NOPRIV;
	}

	while (1)
	{
	  rtt_cursor_abs( x, y);
	  rtt_eofline_erase();
	  rtt_command_get_input_string( (char *) &rtt_chn, input_str, 
		&terminator,
		maxlen, rtt_value_recallbuff, option, timeout, 
		timeout_func, timeout_arg, prompt, RTT_COMMAND_VALUE); 
	  if ( terminator == RTT_K_NONE)
		 /* && !(rtt_commandmode & RTT_COMMANDMODE_FILE)) */
	    /* K_PF3 last command in a command file */
            continue;
	  rtt_message('S',"");
	  rtt_cursor_abs( x, y);
	  rtt_eofline_erase();
	  if ( (terminator >= RTT_K_PF1) && (terminator <= RTT_K_PF4))
	  {
	    sts = RTT__NOVALUE;
	    break;
	  }

	  sts = rtt_set_value( ctx, input_str);
	  if (ODD(sts)) break;
	  if ( sts == RTT__NOPRIV)
	  {
	    sts = RTT__NOVALUE;
	    break;
	  }
	  if ( sts == RTT__INPUT_SYNTAX)
	    rtt_message('E',"Input syntax error");
	  else if ( sts == RTT__OBJNOTFOUND)
	    rtt_message('E',"Object not found");
	  else if ( sts == RTT__STRINGTOLONG)
	    rtt_message('E',"String to long");
	  else if ( sts == RTT__UNDEF_POINTER)
	    rtt_message('E',"Undefined pointer");
	  else if ( sts == RTT__MAXLIMIT)
	    rtt_message('E',"Max limit of value exceeded");
	  else if ( sts == RTT__MINLIMIT)
	    rtt_message('E',"Min limit of value exceeded");
	  else rtt_message('E',"Input error");
	}
	return sts;
}


/*************************************************************************
*
* Name:		rtt_message()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		severity	I
* char		*message	I	
*
* Description:
*	Display a message on the screen.
*
**************************************************************************/

int	rtt_message(	char	severity,
			char	*message)
{
	if ( rtt_message_off)
	  return RTT__SUCCESS;

	rtt_cursor_abs( 0, RTT_ROW_MESSAGE);
	rtt_eofline_erase();
	if ( *message != '\0')
	{
	  if ( rtt_file_on && rtt_print_message)
	    fprintf( rtt_outfile, "%%RTT-%c-MSG, %s\n", severity, message);
	  if ( (rtt_quiet & RTT_QUIET_MESSAGE) || (rtt_quiet && rtt_verify))
	    printf( "\n%%RTT-%c-MSG, %s", severity, message);
	  else if ( severity == 'E')
	    rtt_printf( "%c", '\7');
	  r_print("%%RTT-%c-MSG, %s", severity, message);
	}
	else if ( rtt_AlarmMessage && rtt_AlarmLastMessage[0] )
	{
	  r_print("%s", rtt_AlarmLastMessage);
	}
	return RTT__SUCCESS;
}

int rtt_message_sts( int sts)
{
  char msg[256];

  msg_GetMsg( sts, msg, sizeof(msg));
  r_print( "%s", msg);
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_help()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*subject	I	
* rtt_t_helptext *helptext	I	
*
* Description:
*	Display the helptext of the specified subject.
*	The function seraches in the helptext table for the subject.
*
**************************************************************************/

int	rtt_help(
			menu_ctx	parent_ctx,
			char		*subject,
			rtt_t_helptext	*helptext)
{
	rtt_t_helptext	*helptext_ptr;
	char		subj_array[5][80];
	char		ht_subj_array[5][80];
	int		subjectnr;
	int		ht_subjectnr;
	int		i;
	int		no_match;
	int		subject_found = 0;
	int		sts;
	char		title[100];

	if ( *subject == '\0' || strncmp( subject, "HELP", strlen(subject)) == 0)
	{
	  /* No subject is given take help as default */
	  sts = rtt_help_show_all( parent_ctx, helptext);
	  return sts;
	}
	else if ( *subject == '\0' || strncmp( subject, "SCRIPT", strlen(subject)) == 0)
	{
	  /* No subject is given take help as default */
	  sts = rtt_help_show_all( parent_ctx, rtt_script_helptext);
	  return sts;
	}
	else
	{
	  helptext_ptr = helptext;
	  /* Parse the input subject */
	  subjectnr = rtt_parse( subject, " ", "",
		(char *) subj_array, sizeof( subj_array)/sizeof( subj_array[0]),
		sizeof( subj_array[0]), 0);
	  for ( i = 0; i < subjectnr; i++)
	    rtt_toupper( subj_array[i], subj_array[i]);
	}

	/* Search for the help text for this subject */
	while ( helptext_ptr->subject[0] != '\0')
	{
	  /* Parse the helptext subject */
	  ht_subjectnr = rtt_parse( helptext_ptr->subject, " ", "",
		(char *) ht_subj_array, 
		sizeof( ht_subj_array)/sizeof( ht_subj_array[0]), 
		sizeof( ht_subj_array[0]), 0);
	  if ( subjectnr <= ht_subjectnr)
	  {
	    no_match = 0;
	    for ( i = 0; i < subjectnr; i++)
	    {
	      rtt_toupper( ht_subj_array[i], ht_subj_array[i]);
	      if ( strncmp( subj_array[i], ht_subj_array[i], 
			strlen( subj_array[i])) != 0)
	      {
	        no_match = 1;
	        break;
	      }
	    }
	    if ( !no_match)
	    {
	      subject_found = 1;
	      break;
	    }
	  }
	  helptext_ptr++;
	}
	if ( !subject_found)
	  return RTT__NOHELPSUBJ;

	rtt_display_erase();
	rtt_cursor_abs( 0, 0);
	r_print_buffer();

/**
	rtt_printf("	%s\n\r\n\r", helptext_ptr->subject);
	rtt_printf("%s", helptext_ptr->text);
	rtt_message('S',"");

	rtt_wait_for_return();
**/
	strcpy( title, "HELP ");
	strcat( title, helptext_ptr->subject);
	sts = rtt_view( 0, 0, helptext_ptr->text, title, 
			RTT_VIEWTYPE_BUF);
	return sts;
	return 1;
}


/*************************************************************************
*
* Name:		rtt_help_get_infoline()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*subject	I	
* rtt_t_helptext *helptext	I	
*
* Description:
*	Display the helptext of the specified subject.
*	The function seraches in the helptext table for the subject.
*
**************************************************************************/

static int	rtt_help_getinfoline(
			char		*subject,
			rtt_t_helptext	*helptext,
			char		**infoline)
{
	rtt_t_helptext	*helptext_ptr;
	char		subj_array[5][80];
	char		ht_subj_array[5][80];
	int		subjectnr;
	int		ht_subjectnr;
	int		i;
	int		no_match;
	int		subject_found = 0;

	if ( *subject == '\0')
	{
	  /* No subject is given take help as default */
	  subjectnr = 1;
	  strcpy( subj_array[0], "HELP");
	}
	else
	{
	  /* Parse the input subject */
	  subjectnr = rtt_parse( subject, " ", "",
		(char *) subj_array, sizeof( subj_array)/sizeof( subj_array[0]), 
		sizeof( subj_array[0]), 0);
	  for ( i = 0; i < subjectnr; i++)
	    rtt_toupper( subj_array[i], subj_array[i]);
	}

	/* Search for the help text for this subject */
	helptext_ptr = helptext;	
	while ( helptext_ptr->subject[0] != '\0')
	{
	  /* Parse the helptext subject */
	  ht_subjectnr = rtt_parse( helptext_ptr->subject, " ", "",
		(char *) ht_subj_array,
		sizeof( ht_subj_array)/sizeof( ht_subj_array[0]), 
		sizeof( ht_subj_array[0]), 0);
	  if ( subjectnr <= ht_subjectnr)
	  {
	    no_match = 0;
	    for ( i = 0; i < subjectnr; i++)
	    {
	      rtt_toupper( ht_subj_array[i], ht_subj_array[i]);
	      if ( strncmp( subj_array[i], ht_subj_array[i], 
			strlen( subj_array[i])) != 0)
	      {
	        no_match = 1;
	        break;
	      }
	    }
	    if ( !no_match)
	    {
	      subject_found = 1;
	      break;
	    }
	  }
	  helptext_ptr++;
	}
	if ( !subject_found)
	  return RTT__NOHELPSUBJ;

	*infoline = helptext_ptr->infoline;

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_get_menusize()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
* int		*size		O	
*
* Description:
*	Return the number of items in a menu.
*
**************************************************************************/

int	rtt_get_menusize(
			menu_ctx	ctx,
			int		*size)
{
	rtt_t_menu	*menu_ptr;
	char		*menu_charptr;

	menu_charptr = (char *) ctx->menu;
	menu_ptr = (rtt_t_menu *) menu_charptr;
	*size = 0;
	while ( menu_ptr->text[0] != 0)
	{
	  if ( ctx->menutype & RTT_MENUTYPE_MENU )
	    menu_charptr += sizeof(rtt_t_menu);
	  else if ( ctx->menutype & RTT_MENUTYPE_UPD)
	    menu_charptr += sizeof(rtt_t_menu_upd);

	  menu_ptr = (rtt_t_menu *) menu_charptr;
	  (*size)++;
	}
	return RTT__SUCCESS;
}	

/*************************************************************************
*
* Name:		rtt_menu_item_undelete()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
* int		item		I	
*
* Description:
*	Undelete an item in a menu.
*
**************************************************************************/

int	rtt_menu_item_undelete(
			rtt_t_menu	**menulist,
			int		index)
{
	rtt_t_menu	*menu_ptr;
	int		sts;

	if ( !rtt_menu_deletebuf)
	  return 0;

	sts = rtt_menu_list_insert( menulist,
		index, "",
		0, 0, 0, pwr_cNObjid,0,0,0,0);
	if ( EVEN(sts)) return sts;

	menu_ptr = *menulist + index;
	memcpy( menu_ptr, rtt_menu_deletebuf, sizeof( rtt_t_menu));
	free( rtt_menu_deletebuf);
	rtt_menu_deletebuf = 0;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_item_delete()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
* int		item		I	
*
* Description:
*	Delete an item in a menu.
*
**************************************************************************/

int	rtt_menu_item_delete(
			menu_ctx	ctx,
			int		item)
{
	char		*menu_charptr;
	char		*menu_charptr_next;
	rtt_t_menu_upd	*menu_upd_ptr;
	rtt_t_menu	*menu_ptr;
	int		size;
	int		numberof_items;
	int		itemsize;
	int		sts;

	rtt_get_menusize( ctx, &numberof_items);
	if ( numberof_items == 1)
	  return RTT__SUCCESS;

	if ( ctx->menutype & RTT_MENUTYPE_MENU )
	{
	  itemsize = sizeof(rtt_t_menu);
	  menu_ptr = ctx->menu;
	  /* Store in undelete buffer */
	  if ( !rtt_menu_deletebuf )
	    rtt_menu_deletebuf = calloc( 1, sizeof(rtt_t_menu));
	  menu_ptr += item;
	  memcpy( rtt_menu_deletebuf, menu_ptr, sizeof( rtt_t_menu));
	}
	else if ( ctx->menutype & RTT_MENUTYPE_UPD)
	{
	  itemsize = sizeof(rtt_t_menu_upd);
	  menu_upd_ptr = (rtt_t_menu_upd *) ctx->menu;
	  menu_upd_ptr += item;
	  if ( memcmp( &menu_upd_ptr->subid, &pwr_cNDlid, sizeof( gdh_tSubid)))
	    sts = gdh_UnrefObjectInfo ( menu_upd_ptr->subid);
	}

	menu_charptr = (char *) ctx->menu;
	menu_charptr += item * itemsize;
	menu_charptr_next = (char *) ctx->menu;
	menu_charptr_next += (item + 1) * itemsize;
	size = (numberof_items - item) * itemsize;

	memcpy( menu_charptr, menu_charptr_next, size);	

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_logon()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	*chn		I	
* unsigned long	*priv		O	
*
* Description:
*	Logon of pwr_rtt.
*
**************************************************************************/

static int	rtt_logon( unsigned long *chn,
			   unsigned long *priv,
			   char		*username,
			   char		*password)
{
	int		sts;
	unsigned int privilege;
	char	systemgroup[80];
	pwr_sSecurity sec;
	char opsys_username[80];

	sts = gdh_GetObjectInfo( "pwrNode-System.SystemGroup", &systemgroup, 
				 sizeof(systemgroup));
	if ( EVEN(sts)) return sts;

	if ( strcmp(username, "") != 0 && strcmp( password, "") != 0) {
	  sts = user_CheckUser( systemgroup, username, user_PwCrypt(password), &privilege);
	  if ( ODD(sts) && privilege | pwr_mAccess_AllRt) {
	    if ( privilege & pwr_mPrv_System)
	      *priv = RTT_PRV_SYS;
	    else if ( privilege & pwr_mPrv_Maintenance)
	      *priv = RTT_PRV_EL;
	    else if ( privilege & pwr_mPrv_Process ||
		      privilege & pwr_mPrv_Instrument)
	      *priv = RTT_PRV_PROC;
	    else
	      *priv = RTT_PRV_OP;
	    strncpy( rtt_user, username, sizeof(rtt_user));
	    return RTT__SUCCESS;
	  }
	  sts = rtt_logon_pict( chn, priv);
	  if ( EVEN(sts))
	    exit(0);
	  return sts;
	}

	sts = gdh_GetSecurityInfo( &sec);
	if ( ODD(sts) && sec.XttUseOpsysUser) {
	  syi_UserName( opsys_username, sizeof(opsys_username));
  
	  sts = user_GetUserPriv( systemgroup, opsys_username, &privilege);
	  if ( ODD(sts) && privilege | pwr_mAccess_AllRt) {
	    if ( privilege & pwr_mPrv_System)
	      *priv = RTT_PRV_SYS;
	    else if ( privilege & pwr_mPrv_Maintenance)
	      *priv = RTT_PRV_EL;
	    else if ( privilege & pwr_mPrv_Process ||
		      privilege & pwr_mPrv_Instrument)
	      *priv = RTT_PRV_PROC;
	    else if ( privilege | pwr_mAccess_AllRt)
	      *priv = RTT_PRV_OP;
	    strncpy( rtt_user, opsys_username, sizeof(rtt_user));
	    return RTT__SUCCESS;
	  }
	}
	else if ( ODD(sts) && sec.DefaultXttPriv) {
	  privilege = sec.DefaultXttPriv;
	  if ( privilege & pwr_mPrv_System)
	    *priv = RTT_PRV_SYS;
	  else if ( privilege & pwr_mPrv_Maintenance)
	    *priv = RTT_PRV_EL;
	  else if ( privilege & pwr_mPrv_Process ||
		    privilege & pwr_mPrv_Instrument)
	    *priv = RTT_PRV_PROC;
	  else if ( privilege | pwr_mAccess_AllRt)
	    *priv = RTT_PRV_OP;
	  strcpy( rtt_user, "DefaultUser");
	  return RTT__SUCCESS;
	}
	sts = rtt_logon_pict( chn, priv);
	if ( EVEN(sts))
	  exit(0);
	return sts;
}

/*************************************************************************
*
* Name:		rtt_logon_pict()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	*chn		I	
* unsigned long	*priv		O	
*
* Description:
*	Logon of pwr_rtt.
*
**************************************************************************/

int	rtt_logon_pict( unsigned long *chn,
			unsigned long *priv)
{
	unsigned long	terminator;
	unsigned long	option;
	char		user_str[80];
	char		passw_str[80];
	int		maxlen = 30;
	int		attempts = 0;
	char		nodename[80];
	int		sts;
	rtt_t_backgr	*picture;
	unsigned int privilege;
	char	systemgroup[80];

	sts = gdh_GetObjectInfo( "pwrNode-System.SystemGroup", &systemgroup, 
				 sizeof(systemgroup));
	if ( EVEN(sts)) return sts;

	sts = rtt_get_nodename( nodename, sizeof(nodename));
	rttsys_get_login_picture( &picture);
	rtt_display_erase();
	rtt_edit_draw_background( picture);

	rtt_cursor_abs( 34, 9);
	r_print( nodename);
	rtt_cursor_abs( 34, 11);
	r_print(rtt_version);
	r_print_buffer();
	

	while ( attempts < 3) {
	  rtt_cursor_abs( 32, 20);
	  rtt_eofline_erase();
	  r_print_buffer();
	  option = RTT_OPT_NOSCROLL | RTT_OPT_NORECALL;
	  rtt_get_input_string( (char *) chn, user_str, &terminator, 
				maxlen, 0, option, 0, 
				0, 0, "Username: "); 
	  rtt_toupper( user_str, user_str);
	  rtt_message('S',"");
	  if ( terminator >= RTT_K_RETURN ) {
	    rtt_cursor_abs( 32, 20);
	    rtt_eofline_erase();
	    r_print_buffer();
	    option = RTT_OPT_NOSCROLL | RTT_OPT_NORECALL | RTT_OPT_NOECHO;
	    rtt_get_input_string( (char *) chn, passw_str, &terminator, 
				  maxlen, 0, option, 0, 
				  0, 0, "Password: "); 
	    cdh_ToLower( passw_str, passw_str);
	    rtt_message('S',"");
	    if ( terminator >= RTT_K_RETURN ) {

	      sts = user_CheckUser( systemgroup, user_str, user_PwCrypt(passw_str), &privilege);
	      if ( EVEN(sts)) {
		attempts++;
		rtt_message('E',"User not authorized");
		continue;
	      }

	      if ( privilege & pwr_mPrv_System)
		*priv = RTT_PRV_SYS;
	      else if ( privilege & pwr_mPrv_Maintenance)
		*priv = RTT_PRV_EL;
	      else if ( privilege & pwr_mPrv_Process ||
			privilege & pwr_mPrv_Instrument)
		*priv = RTT_PRV_PROC;
	      else if ( privilege | pwr_mAccess_AllRt)
		*priv = RTT_PRV_OP;
	      else {
		attempts++;
		rtt_message('E',"User not authorized");
		continue;
	      }
	      strncpy( rtt_user, user_str, sizeof(rtt_user));
	      return RTT__SUCCESS;
	    }
	  }
	}
	r_print_buffer();
	return 0;
}

/*************************************************************************
*
* Name:		rtt_error_msg()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* unsigned long sts		I	
*
* Description:
*	Write an error message on the terminal.
*
**************************************************************************/

#ifdef OS_VMS
void rtt_error_msg(
			unsigned long 	sts)
{
	static int msgsts;
	static int msglen;
	static char msg[256];
	static $DESCRIPTOR(msgdesc, msg);

	if (EVEN(sts))
	{
	  msgsts = sts;
	  lib$sys_getmsg(&msgsts, &msglen, &msgdesc, 0, 0);
	  msg[msglen]='\0';
	  rtt_printf("%s\n\r", msg);
	}
}
#elif OS_ELN
void rtt_error_msg( unsigned long 	sts)
{
	VARYING_STRING(256) msg;

	if (EVEN(sts))
	{
	  eln$get_status_text(sts, 0, &msg);
	  rtt_printf("%s\n\r", &msg.data);
	}
}
#else
void rtt_error_msg( unsigned long 	sts)
{
	if (EVEN(sts))
	{
	  rtt_printf("%%RTT-E-UNKNWN, unknown message %d\n\r", sts);
	}
}
#endif

/*************************************************************************
*
* Name:		rtt_objidtoclassname()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid	objid		I	objid of an object.
* char		*name		O	name of class.
*
* Description:
*	Return the name of the class of the specified objid.
*
**************************************************************************/

int	rtt_objidtoclassname(
			pwr_tObjid	objid,
			char		*name)
{
	pwr_tClassId	class;
	pwr_tOName     	hiername;
	char		*s;
	int		sts;

	sts = gdh_GetObjectClass( objid, &class);
	if ( EVEN(sts)) return sts;
	sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, sizeof(hiername), cdh_mNName);
	if ( EVEN(sts)) return sts;

	/* Skip hierarchy of classname */
	s = strrchr( hiername, '-');
	if ( s == 0)
	  strcpy( name, hiername);
	else
	  strcpy( name, s + 1);

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_menu_new_update()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Presents all parameters and the values of the parameter
*	in an menu.
*
**************************************************************************/

int	rtt_menu_new_update(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			rtt_t_menu_update **menu_p,
			char		*title,
			void		*userdata,
			unsigned long	flag)
{
	rtt_t_menu_update *menu_ptr;
	rtt_t_menu_upd	*menulist = 0;
	int		sts;
	int		index = 0;

	menu_ptr = *menu_p;
	while( menu_ptr->text[0] != 0)
	{
	  sts = rtt_menu_new_update_add( parent_ctx, &menulist,
		menu_ptr->text, menu_ptr->func1, menu_ptr->func2, 
		menu_ptr->parameter_name, 
		menu_ptr->dualparameter_name, menu_ptr->priv, 
		0, 0, 0.0, 0.0, RTT_DATABASE_GDH, 0, 0, 0, userdata, flag,
		&index, 0);
	  if (EVEN(sts)) return sts;

	  menu_ptr++;
	}
	if ( menulist != 0)
	{
	  sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if ( EVEN(sts)) return sts;
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_new_upedit()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Presents all parameters and the values of the parameter
*	in an menu.
*
**************************************************************************/

int	rtt_menu_new_upedit(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			rtt_t_menu_update **menu_p,
			char		*title,
			rtt_t_backgr	*picture,
			int		(* function) ())
{
	rtt_t_menu_update *menu_ptr;
	rtt_t_menu_upd	*menulist = 0;
	rtt_t_menu_upd	*menu;
	int		sts;
	int		index = 0;
	int		x;
	int		item_count;
	int		unknown_object;

	/* Count the update items in the picture */
	item_count = 0;
	menu_ptr = *menu_p;
	while( menu_ptr->text[0] != 0)
	{
	  item_count++;
	  menu_ptr++;
	}
	/* Allocate memory for menu list */
	if ( item_count > 0)
	{
	  sts = rtt_menu_upd_list_add_malloc( &menulist, item_count);
	  if ( EVEN(sts)) return sts;
	}

	unknown_object = 0;
	menu_ptr = *menu_p;
	while( menu_ptr->text[0] != 0)
	{
	  x = menu_ptr->x;
	  if ( strcmp( menu_ptr->text, "%") == 0)
	    x -= 2;
	  sts = rtt_menu_new_update_add( parent_ctx, &menulist,
		menu_ptr->text, menu_ptr->func1, menu_ptr->func2, 
		menu_ptr->parameter_name, 
		menu_ptr->dualparameter_name, menu_ptr->priv, 
		menu_ptr->characters, menu_ptr->decimals,
		menu_ptr->maxlimit, menu_ptr->minlimit,
		menu_ptr->database, menu_ptr->declaration,
		x, menu_ptr->y, 0, RTT_MENUTYPE_DYN,
		&index, item_count);
	  if ( sts == RTT__ITEMUNKNOWN)
	    unknown_object = 1;
	  else if (EVEN(sts)) return sts;

	  menu_ptr++;
	}
	if ( unknown_object)
        {
  	  rtt_printf("\n\n\n");
	  rtt_wait_for_return();
        }

	sts = rtt_menu_edit_new( parent_ctx, pwr_cNObjid, &menulist, title, 
		picture, function);

	/* take away the gdh prenumerations */
	if ( menulist != 0)
	{
	  menu = menulist;
	  while ( menu->text[0] != '\0')
	  {
	    if ( memcmp( &menu->subid, &pwr_cNDlid, sizeof( gdh_tSubid)))
	    	gdh_UnrefObjectInfo ( menu->subid);
	    menu++;
	  }
	  /* free the menu */
	  free ( menulist);
	}
	if ( sts == RTT__FASTBACK) return sts;
	else if ( sts == RTT__BACKTOCOLLECT) return sts;
	else if ( EVEN(sts)) return sts;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_new_upeditperm()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Presents all parameters and the values of the parameter
*	in an menu.
*
**************************************************************************/

int	rtt_menu_new_upeditperm(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			rtt_t_menu_update **menu_p,
			char		*title,
			rtt_t_backgr	*picture,
			int		(* function) ())
{
	rtt_t_menu_update *menu_ptr;
	rtt_t_menu_upd	*menulist = 0;
	int		sts;
	int		index = 0;
	int		x;
	int		item_count;
	int		unknown_object;

	/* Get the menulist */
	sts = rtt_get_stored_menuctx( (void **) &menulist, 
			(void *) picture);
	if ( EVEN(sts))
	{
	
	  /* Count the update items in the picture */
	  item_count = 0;
	  menu_ptr = *menu_p;
	  while( menu_ptr->text[0] != 0)
	  {
	    item_count++;
	    menu_ptr++;
	  }
	  /* Allocate memory for menu list */
	  if ( item_count > 0)
	  {
	    sts = rtt_menu_upd_list_add_malloc( &menulist, item_count);
	    if ( EVEN(sts)) return sts;
	  }

	  unknown_object = 0;
	  menu_ptr = *menu_p;
	  while( menu_ptr->text[0] != 0)
	  {
	    x = menu_ptr->x;
	    if ( strcmp( menu_ptr->text, "%") == 0)
	      x -= 2;
	    sts = rtt_menu_new_update_add( parent_ctx, &menulist,
		menu_ptr->text, menu_ptr->func1, menu_ptr->func2, 
		menu_ptr->parameter_name, 
		menu_ptr->dualparameter_name, menu_ptr->priv, 
		menu_ptr->characters, menu_ptr->decimals,
		menu_ptr->maxlimit, menu_ptr->minlimit,
		menu_ptr->database, menu_ptr->declaration,
		x, menu_ptr->y, 0, RTT_MENUTYPE_DYN,
		&index, item_count);
	    if ( sts == RTT__ITEMUNKNOWN)
	      unknown_object = 1;
	    else if (EVEN(sts)) return sts;

	    menu_ptr++;
	  }
	  if ( unknown_object)
          {
  	    rtt_printf("\n\n\n");
	    rtt_wait_for_return();
          }

	  /* Store the menulist */
	  rtt_store_menuctx( (void *)menulist, (void *)picture);
	}
	sts = rtt_menu_edit_new( parent_ctx, pwr_cNObjid, &menulist, title, 
		picture, function);
	if ( sts == RTT__FASTBACK) return sts;
	else if ( sts == RTT__BACKTOCOLLECT) return sts;
	else if ( EVEN(sts)) return sts;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_object_parameters()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

int	rtt_menu_new_sysedit(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			char		*objectname,
			char		*title,
			void		*dummy,
			int		(* function) ())
{
	rtt_t_menu_update *menu_p;
	rtt_t_menu_update *menu_ptr;
	rtt_t_menu_upd	*menulist = 0;
	int		sts;
	int		index = 0;
	int		x;
	int		item_count;

	/* Get the items in the picture list */
	sts = (function) ( 0, RTT_APPL_MENU, 0, objectname, &menu_p);
        if ( sts == RTT__NOPICTURE)
          return RTT__NOPICTURE;
	else if ( EVEN(sts)) return sts;

	/* Count the update items in the picture */
	item_count = 0;
	menu_ptr = menu_p;
	while( menu_ptr->text[0] != 0)
	{
	  item_count++;
	  menu_ptr++;
	}
	/* Allocate memory for menu list */
	if ( item_count > 0)
	{
	  sts = rtt_menu_upd_list_add_malloc( &menulist, item_count);
	  if ( EVEN(sts)) return sts;
	}

	menu_ptr = menu_p;
	while( menu_ptr->text[0] != 0)
	{
	  x = menu_ptr->x;
	  if ( strcmp( menu_ptr->text, "%") == 0)
	    x -= 2;
	  sts = rtt_menu_new_update_add( parent_ctx, &menulist,
		menu_ptr->text, menu_ptr->func1, menu_ptr->func2, 
		menu_ptr->parameter_name, 
		menu_ptr->dualparameter_name, menu_ptr->priv, 
		menu_ptr->characters, menu_ptr->decimals,
		menu_ptr->maxlimit, menu_ptr->minlimit,
		menu_ptr->database, menu_ptr->declaration,
		x, menu_ptr->y, 0, RTT_MENUTYPE_DYN,
		&index, item_count);
	  if (EVEN(sts)) return sts;

	  menu_ptr++;
	}
	sts = rtt_menu_sysedit_new( parent_ctx, pwr_cNObjid, &menulist, 
		title, objectname, function);
	if ( sts == RTT__FASTBACK) return sts;
	else if ( sts == RTT__BACKTOCOLLECT) return sts;
	else if ( EVEN(sts)) return sts;

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_menu_new_update_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Presents all parameters and the values of the parameter
*	in an menu.
*
**************************************************************************/

static int	rtt_menu_new_update_add(
			menu_ctx	parent_ctx,
			rtt_t_menu_upd	**menulist,
			char		*title,
			int		(* func1) (),
			int		(* func2) (),
			char		*parameter_name,
			char		*dualparameter_name,
			unsigned long	priv,
			char		characters,
			char		decimals,
			float		maxlimit,
			float		minlimit,
			int		database,
			int		declaration,
			int		x,
			int		y,
			void		*userdata,
			unsigned long	flag,
			int		*index,
			int		item_count)
{
	int		sts;
	pwr_tOName     	hiername;
	pwr_tOName     	parname;
	pwr_tClassId	class;
	unsigned long	elements;
	char 		*parameter_ptr;
	SUBID 		subid;
	pwr_sParInfo
	parinfo;
	pwr_tOName     	objname;
	char		*s;
	pwr_tOName     	name_array[2];
	int		nr;
	pwr_tObjid	objid;


	if ( database == RTT_DATABASE_GDH)
	{
	  /* Parse the parameter name into a object and a parameter name */
	  nr = rtt_parse( parameter_name, ".", "",
		(char *) name_array, 
		sizeof( name_array)/sizeof( name_array[0]), 
		sizeof( name_array[0]), 0);
	  strcpy( objname, name_array[0]);
	  strcpy( parname, name_array[1]);

	  /* Get objid */
	  sts = gdh_NameToObjid ( objname, &objid);
	  if ( EVEN(sts)) 
	  {
	    rtt_printf("\n\rObject unknown: %s", parameter_name);
	    return RTT__ITEMUNKNOWN;
	  }

	  /*Remove index if array */
	  if ( (s = strrchr( parname, '[')))
	    *s = 0;

	  /* Get objid of rtbody */
	  sts = gdh_GetObjectClass( objid, &class);
	  if ( EVEN(sts)) return sts;
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
			sizeof(hiername),  cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;
	  strcat( hiername, "-RtBody-");
	  strcat( hiername, parname);
	
	  sts = gdh_GetObjectInfo ( hiername, &parinfo, 
			sizeof(parinfo)); 
	  if ( EVEN(sts)) 
	  {
	    /* Try with sysbody */
	    sts = gdh_ObjidToName ( cdh_ClassIdToObjid(class), hiername, 
			sizeof(hiername),  cdh_mName_volumeStrict);
	    if ( EVEN(sts)) return sts;
	    strcat( hiername, "-SysBody-");
	    strcat( hiername, parname);
	    sts = gdh_GetObjectInfo ( hiername, &parinfo, sizeof(parinfo)); 
	    if ( EVEN(sts)) 
	    {
	      rtt_printf("\n\rAttribute unknown: %s", parameter_name);
	      return RTT__ITEMUNKNOWN;
	    }
	  }

	  if ( parinfo.Flags & PWR_MASK_ARRAY )
	    elements = parinfo.Elements;
	  else
	    elements = 1;

	  /* Get rtdb pointer */
	  sts = gdh_RefObjectInfo ( parameter_name, (void *) &parameter_ptr,
		&subid, parinfo.Size / elements);
	  if ( EVEN(sts)) 
	  {
	    parameter_ptr = 0;
	  }

	  /* gdh returns the pointer (not a pointer to a pointer) 
	     remove the pointer bit in Flags */
	  if ( parinfo.Flags & PWR_MASK_POINTER )
	    parinfo.Flags -= PWR_MASK_POINTER;


	  /* store subid if the pointer if found */
	    if ( parameter_ptr == 0)
	      subid = pwr_cNDlid;

	}
	else if ( database == RTT_DATABASE_RTT)
	{
	  if ( declaration == RTT_DECL_INT)
	  {
	    parinfo.Type = pwr_eType_Int32;
	    parinfo.Size = 4;
	  }
	  else if ( declaration == RTT_DECL_SHORT)
	  {
	    parinfo.Type = pwr_eType_Int16;
	    parinfo.Size = 2;
	  }
	  else if ( declaration == RTT_DECL_BOOLEAN)
	  {
	    parinfo.Type = pwr_eType_Boolean;
	    parinfo.Size = sizeof(pwr_tBoolean);
	  }
	  else if ( declaration == RTT_DECL_FLOAT)
	  {
	    parinfo.Type = pwr_eType_Float32;
	    parinfo.Size = sizeof(pwr_tFloat32);
	  }
	  else if ( declaration == RTT_DECL_CHAR)
	  {
	    parinfo.Type = pwr_eType_Char;
	    parinfo.Size = sizeof(pwr_tChar);
	  }
	  else if ( declaration == RTT_DECL_STRING)
	  {
	    parinfo.Type = pwr_eType_String;
	    parinfo.Size = 80;
	  }
	  else if ( declaration == RTT_DECL_OBJID)
	  {
	    parinfo.Type = pwr_eType_Objid;
	    parinfo.Size = sizeof( pwr_tObjid);
	  }
	  else if ( declaration == RTT_DECL_ATTRREF)
	  {
	    parinfo.Type = pwr_eType_AttrRef;
	    parinfo.Size = sizeof( pwr_sAttrRef);
	  }
	  else if ( declaration == RTT_DECL_TIME)
	  {
	    parinfo.Type = pwr_eType_Time;
	    parinfo.Size = sizeof( pwr_tTime);
	  }
	  parinfo.Flags = 0;
	  elements = 1;
	  subid = pwr_cNDlid;
	  objid = pwr_cNObjid;
	  
	  /* Get the pointer */
	  sts = rtt_RefObjectInfo ( parameter_name, &parameter_ptr);
	  if ( EVEN(sts)) return sts;
	}
	else if ( database == RTT_DATABASE_RTTSYS)
	{
	  if ( declaration == RTT_DECL_INT)
	  {
	    parinfo.Type = pwr_eType_Int32;
	    parinfo.Size = sizeof(pwr_tInt32);
	  }
	  else if ( declaration == RTT_DECL_SHORT)
	  {
	    parinfo.Type = pwr_eType_Int16;
	    parinfo.Size = sizeof(pwr_tInt16);
	  }
	  else if ( declaration == RTT_DECL_BOOLEAN)
	  {
	    parinfo.Type = pwr_eType_Boolean;
	    parinfo.Size = sizeof(pwr_tBoolean);
	  }
	  else if ( declaration == RTT_DECL_FLOAT)
	  {
	    parinfo.Type = pwr_eType_Float32;
	    parinfo.Size = sizeof(pwr_tFloat32);
	  }
	  else if ( declaration == RTT_DECL_CHAR)
	  {
	    parinfo.Type = pwr_eType_Char;
	    parinfo.Size = sizeof(pwr_tChar);
	  }
	  else if ( declaration == RTT_DECL_STRING)
	  {
	    parinfo.Type = pwr_eType_String;
	    parinfo.Size = 80;
	  }
	  else if ( declaration == RTT_DECL_OBJID)
	  {
	    parinfo.Type = pwr_eType_Objid;
	    parinfo.Size = sizeof( pwr_tObjid);
	  }
	  else if ( declaration == RTT_DECL_ATTRREF)
	  {
	    parinfo.Type = pwr_eType_AttrRef;
	    parinfo.Size = sizeof( pwr_sAttrRef);
	  }
	  else if ( declaration == RTT_DECL_TIME)
	  {
	    parinfo.Type = pwr_eType_Time;
	    parinfo.Size = sizeof( pwr_tTime);
	  }
	  parinfo.Flags = 0;
	  elements = 1;
	  subid = pwr_cNDlid;
	  objid = pwr_cNObjid;
	  
	  /* Get the pointer */
	  sts = rtt_RttsysRefObjectInfo ( parameter_name, &parameter_ptr);
	  if ( EVEN(sts)) return sts;
	}
	else if ( database == RTT_DATABASE_USER)
	{
	  if ( declaration == RTT_DECL_INT)
	  {
	    parinfo.Type = pwr_eType_Int32;
	    parinfo.Size = sizeof(pwr_tInt32);
	  }
	  else if ( declaration == RTT_DECL_SHORT)
	  {
	    parinfo.Type = pwr_eType_Int16;
	    parinfo.Size = sizeof(pwr_tInt16);
	  }
	  else if ( declaration == RTT_DECL_BOOLEAN)
	  {
	    parinfo.Type = pwr_eType_Boolean;
	    parinfo.Size = sizeof(pwr_tBoolean);
	  }
	  else if ( declaration == RTT_DECL_FLOAT)
	  {
	    parinfo.Type = pwr_eType_Float32;
	    parinfo.Size = sizeof(pwr_tFloat32);
	  }
	  else if ( declaration == RTT_DECL_CHAR)
	  {
	    parinfo.Type = pwr_eType_Char;
	    parinfo.Size = sizeof(pwr_tChar);
	  }
	  else if ( declaration == RTT_DECL_STRING)
	  {
	    parinfo.Type = pwr_eType_String;
	    parinfo.Size = 80;
	  }
	  else if ( declaration == RTT_DECL_OBJID)
	  {
	    parinfo.Type = pwr_eType_Objid;
	    parinfo.Size = sizeof( pwr_tObjid);
	  }
	  else if ( declaration == RTT_DECL_ATTRREF)
	  {
	    parinfo.Type = pwr_eType_AttrRef;
	    parinfo.Size = sizeof( pwr_tObjid);
	  }
	  else if ( declaration == RTT_DECL_TIME)
	  {
	    parinfo.Type = pwr_eType_Time;
	    parinfo.Size = sizeof( pwr_tTime);
	  }
	  parinfo.Flags = 0;
	  elements = 1;
	  parameter_ptr = 0;
	  subid = pwr_cNDlid;
	}

	sts = rtt_menu_upd_list_add( menulist, *index, item_count, title, 
		0, 
		func1, func2,
		objid, parameter_name,
		dualparameter_name, (void *) priv, 0,
		parameter_name, priv, parameter_ptr, parinfo.Type,
		parinfo.Flags, parinfo.Size / elements, subid, x, y,
		characters, decimals, maxlimit, minlimit, database,
		dualparameter_name);
	if ( EVEN(sts)) return sts;
	(*index)++;

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_menu_parameter_set()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* void		*arg1		I	dummy
* char		*parameter_name	I	name of parameter to set
* void		*arg3		I	
* int		priv		I	previliges to change value
*
* Description:
*	Set of a boolean parameter in rtdb.
*
**************************************************************************/

int	rtt_menu_parameter_set(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*parameter_name,
			void		*arg3,
			unsigned long	priv,
			void		*arg4)
{
	pwr_tBoolean	value = 1;
	int		sts;
	rtt_t_menu_upd	*menu_ptr;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;
	
	if ( !(priv & rtt_priv) )
	{
	  rtt_message('E',"Not authorized for this operation");
	  return RTT__NOVALUE;
	}

	/* Set the value */
	if ( menu_ptr->database == RTT_DATABASE_GDH)
	{
	  sts = gdh_SetObjectInfo ( parameter_name, &value, sizeof(value));
	  if ( EVEN(sts))
	  {
	    rtt_message('E', "Unable to set value");
	    return RTT__NOPICTURE;
	  }
	}
	else if ( menu_ptr->value_ptr != 0)
	{
	  memcpy( menu_ptr->value_ptr, &value, sizeof(value));
	}
	else
	 rtt_message('E', "Unable to set value");

	return RTT__NOPICTURE;
}


/*************************************************************************
*
* Name:		rtt_menu_parameter_reset()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* void		*arg1		I	dummy
* char		*parameter_name	I	name of parameter to set
* void		*arg3		I	
* int		priv		I	previliges to change value
*
* Description:
*	Reset of a boolean parameter in rtdb.
*
**************************************************************************/

int	rtt_menu_parameter_reset(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*parameter_name,
			void		*arg3,
			unsigned long	priv,
			void		*arg4)
{
	pwr_tBoolean	value = 0;
	int		sts;
	rtt_t_menu_upd	*menu_ptr;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;
	

	if ( !(priv & rtt_priv) )
	{
	  rtt_message('E',"Not authorized for this operation");
	  return RTT__NOVALUE;
	}

	/* Set the value */
	if ( menu_ptr->database == RTT_DATABASE_GDH)
	{
	  sts = gdh_SetObjectInfo ( parameter_name, &value, sizeof(value));
	  if ( EVEN(sts))
	  {
	    rtt_message('E', "Unable to set value");
	    return RTT__NOPICTURE;
	  }
	}
	else if ( menu_ptr->value_ptr != 0)
	{
	  memcpy( menu_ptr->value_ptr, &value, sizeof(value));
	}
	else
	 rtt_message('E', "Unable to set value");

	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_menu_parameter_toggle()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* void		*arg1		I	dummy
* char		*parameter_name	I	name of parameter to toggle
* void		*arg3		I	
* int		priv		I	previliges to change value
*
* Description:
*	Toggles a boolean parameter in rtdb.
*
**************************************************************************/

int	rtt_menu_parameter_toggle(
			menu_ctx	ctx,
			pwr_tObjid	argoi,
			char		*parameter_name,
			void		*arg3,
			unsigned long	priv,
			void		*arg4)
{
	pwr_tBoolean	value;
	int		sts;
	rtt_t_menu_upd	*menu_ptr;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;

	if ( !(priv & rtt_priv) )
	{
	  rtt_message('E',"Not authorized for this operation");
	  return RTT__NOVALUE;
	}

	/* Get the current value */
	if ( menu_ptr->database == RTT_DATABASE_GDH)
	{
	  sts = gdh_GetObjectInfo ( parameter_name, &value, sizeof(value)); 
	  if (EVEN(sts))
	  {
	    rtt_message('E', "Unable to set value");
	    return RTT__NOPICTURE;
	  }
	}
	else if ( menu_ptr->value_ptr != 0)
	{
	  memcpy( &value, menu_ptr->value_ptr, sizeof(value));
	}
	else
	 rtt_message('E', "Unable to set value");

	/* Toggle the value */
	if ( value == 0)
	  value = 1;
	else
	  value = 0;

	/* Set the toggled value */
	if ( menu_ptr->database == RTT_DATABASE_GDH)
	{
	  sts = gdh_SetObjectInfo ( parameter_name, &value, sizeof(value));
	  if ( EVEN(sts))
	  {
	    rtt_message('E', "Unable to set value");
	    return RTT__NOPICTURE;
	  }
	}
	else if ( menu_ptr->value_ptr != 0)
	{
	  memcpy( menu_ptr->value_ptr, &value, sizeof(value));
	}

	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_menu_parameter_dual_set()
* Name:		rtt_menu_parameter_dual_reset()
* Name:		rtt_menu_parameter_dual_toggle()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* void		*arg1		I	dummy
* void		*arg2		I	
* char		*parameter_name	I	name of parameter to toggle
* int		priv		I	previliges to change value
*
* Description:
*	The same as rtt_menu_parameter... without 'dual' exept for the 
*	parameter name is in the third arg instead of second.
*
**************************************************************************/

int	rtt_menu_parameter_dual_set(
			menu_ctx	parent_ctx,
			pwr_tObjid	argoi,
			void		*arg1,
			char		*parameter_name,
			unsigned long	priv,
			void		*arg4)
{
	return ( rtt_menu_parameter_set( (menu_ctx) parent_ctx, argoi,
	parameter_name, arg1, priv, arg4));
}

int	rtt_menu_parameter_dual_reset(
				menu_ctx	parent_ctx,
				pwr_tObjid	argoi,
				void		*arg1,
				char		*parameter_name,
				unsigned long	priv,
				void		*arg4)
{
	return ( rtt_menu_parameter_reset( (menu_ctx) parent_ctx, argoi,
	parameter_name, arg1, priv, arg4));
}

int	rtt_menu_parameter_dual_toggle(
				menu_ctx	parent_ctx,
				pwr_tObjid	argoi,
				void		*arg1,
				char		*parameter_name,
				unsigned long	priv,
				void		*arg4)
{
	return ( rtt_menu_parameter_toggle( (menu_ctx) parent_ctx, argoi,
	parameter_name, arg1, priv, arg4));
}

/*************************************************************************
*
* Name:		rtt_menu_parameter_command()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* void		*argoi		I	objid
* void		*arg1		I	dummy
* char		*command	I	command to execute.
* int		priv		I	previliges to execute command
* void		*arg4		I	dummy
*
* Description:
*	Execute a command stored in the dual_parameter field.
*
**************************************************************************/

int	rtt_menu_parameter_command(
				menu_ctx	parent_ctx,
				pwr_tObjid	argoi,
				void		*arg1,
				char		*command,
				unsigned long	priv,
				void		*arg4)
{
	int sts;

	if ( !(priv & rtt_priv) )
	{
	  rtt_message('E',"Not authorized for this operation");
	  return RTT__NOPICTURE;
	}

	sts = rtt_menu_command( parent_ctx, argoi, command,0,0,0);
	return sts;
}

/*************************************************************************
*
* Name:		rtt_menu_exec_filecommand()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* void		*argoi		I	dummy
* char		*command	I	command
* void		*arg2		I	dummy
* void		*arg3		I	dummy
* void		*arg4		I	dummy
*
* Description:
*	Execute a command for a found file.
*
**************************************************************************/

int	rtt_menu_exec_filecommand(
				menu_ctx	parent_ctx,
				pwr_tObjid	argoi,
				char		*command,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	int sts;

	sts = rtt_menu_command( parent_ctx, argoi, command,0,0,0);
	return sts;
}

/*************************************************************************
*
* Name:		rtt_menu_filecommand()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
* void		*argoi		I	dummy
* char		*filespec	I	file_specification
* char		*command	I	command
* int		*arg3		I	dummy
* void		*arg4		I	dummy
*
* Description:
*	Show the specified files and execute the command as return-action.
*
**************************************************************************/

int	rtt_menu_filecommand(
				menu_ctx	parent_ctx,
				pwr_tObjid	argoi,
				char		*filespec,
				char		*command,
				void		*arg3,
				void		*arg4)
{
	int sts;

	sts = rtt_show_file( parent_ctx, filespec, command, NULL);
	if ( EVEN(sts))
	{
	  rtt_message('E', "No files found");
	  return RTT__NOPICTURE;
	}
	return sts;
}

/*************************************************************************
*
* Name:		rtt_edit_draw_background()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_backgr	*chartable	I	character table of picture.
*
* Description:
*	Draw the background picture.
*
**************************************************************************/

int	rtt_edit_draw_background(
			rtt_t_backgr	*chartable)
{
	int		i, j;
	unsigned long	current_charset;
	unsigned long	current_inverse;
	int		current_x = 0;
	int		current_y = 0;

	rtt_charset_ascii();
	rtt_char_inverse_end();
	current_charset = RTT_CHARSET_ASCII;
	current_inverse = RTT_CHARSET_NOINVERSE;
	rtt_cursor_abs( 1, 1);
	for ( i = 0; i < 22; i++)
	{
	  for ( j = 0; j < 80; j++)
	  {
	    if ( (*chartable)[0][ j][ i] != 0)
	    {
	      /* Set cursor if you really have to */
	      if ( (current_x++ != j) || (current_y != i))
	      {
	        rtt_cursor_abs( j + 1, i + 1);
	        current_x = j + 1;
	        current_y = i;
	      }
	      if (( (*chartable)[1][ j][ i] & RTT_CHARSET_LINE) &&
	         ( current_charset != RTT_CHARSET_LINE))	         
	      {
	        rtt_charset_linedrawing();
	        current_charset = RTT_CHARSET_LINE;
	      }
	      else if ( !((*chartable)[1][ j][ i] & RTT_CHARSET_LINE) &&
	         ( current_charset != RTT_CHARSET_ASCII))	         
	      {
	        rtt_charset_ascii();
	        current_charset = RTT_CHARSET_ASCII;
	      }
	      if (( (*chartable)[1][ j][ i] & RTT_CHARSET_INVERSE) &&
	         ( current_inverse != RTT_CHARSET_INVERSE))	         
	      {
	        rtt_char_inverse_start();
	        current_inverse = RTT_CHARSET_INVERSE;
	      }
	      else if ( !((*chartable)[1][ j][ i] & RTT_CHARSET_INVERSE) &&
	         ( current_inverse != RTT_CHARSET_NOINVERSE))	         
	      {
	        rtt_char_inverse_end();
	        current_inverse = RTT_CHARSET_NOINVERSE;
	      }
	      r_print("%c", (*chartable)[0][ j][ i]);
	    }
	  }
	}
	rtt_charset_ascii();
	rtt_char_inverse_end();
	r_print_buffer();
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		r_print()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Print. Equivalent to printf but the character string is put
*	in a buffer and printed when the buffer size is exceeded or 
*	when r_print_buffer is called.
*	The max size of the character string is 200.
*
**************************************************************************/

int	r_print(	char		*format, ...)
{
	char	buff[200];
	int	sts;
	va_list ap;

	va_start( ap, format);
	sts = vsprintf( buff, format, ap);
	va_end( ap);
	strcpy( &rtt_print_buffer[rtt_print_buffer_len], buff);
	rtt_print_buffer_len += strlen( buff);

	if ( rtt_print_buffer_len > (int)( sizeof( rtt_print_buffer) -
		sizeof( buff)))
	  r_print_buffer();

	return sts;
}

/*************************************************************************
*
* Name:		r_print_buffer()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

int	r_print_buffer()
{
	int sts;

	if ( !rtt_quiet)
	  sts = qio_writew( (int *) rtt_chn, rtt_print_buffer, 
			rtt_print_buffer_len);
	else
	  sts = RTT__SUCCESS;
	rtt_print_buffer_len = 0;
	rtt_print_buffer[0] = 0;

	return sts;
}


/*************************************************************************
*
* Name:		rtt_RefObjectInfo()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

static int	rtt_RefObjectInfo (
			char	*parameter_name,
			char	**parameter_ptr)
{
	rtt_t_db	*db_ptr;

	if ( strcmp( parameter_name, "RTT_TIME") == 0) 
	{
	  *parameter_ptr = &(rtt_time[12]);
	  return RTT__SUCCESS;
	} 
	else if ( strcmp( parameter_name, "RTT_TIME_FULL") == 0) 
	{
	  *parameter_ptr = rtt_time;
	  return RTT__SUCCESS;
	} 
	else if ( strcmp( parameter_name, "RTT_ALARMTEXT1") == 0) 
	{
	  *parameter_ptr = rtt_AlarmText1;
	  return RTT__SUCCESS;
	} 
	else if ( strcmp( parameter_name, "RTT_ALARMTEXT2") == 0) 
	{
	  *parameter_ptr = rtt_AlarmText2;
	  return RTT__SUCCESS;
	} 
	else if ( strcmp( parameter_name, "RTT_ALARMTEXT3") == 0) 
	{
	  *parameter_ptr = rtt_AlarmText3;
	  return RTT__SUCCESS;
	} 
	else if ( strcmp( parameter_name, "RTT_ALARMTEXT4") == 0) 
	{
	  *parameter_ptr = rtt_AlarmText4;
	  return RTT__SUCCESS;
	} 
	else if ( strcmp( parameter_name, "RTT_ALARMTEXT5") == 0) 
	{
	  *parameter_ptr = rtt_AlarmText5;
	  return RTT__SUCCESS;
	} 
	

	db_ptr = &rtt_appl_db[0];
	while ( db_ptr->parameter[0] != 0) 
	{
	  if ( strcmp( parameter_name, db_ptr->parameter) == 0) 
	  {
	    *parameter_ptr = db_ptr->parameter_ptr;
	    return RTT__SUCCESS;
	  } 
	  db_ptr++;
	}
	return RTT__OBJNOTFOUND;
}

/*************************************************************************
*
* Name:		rtt_RttsysRefObjectInfo()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

static int	rtt_RttsysRefObjectInfo (
			char	*parameter_name,
			char	**parameter_ptr)
{
	rtt_t_db	*db_ptr;

	db_ptr = &rtt_rttsys_db[0];
	while ( db_ptr->parameter[0] != 0) 
	{
	  if ( strcmp( parameter_name, db_ptr->parameter) == 0) 
	  {
	    *parameter_ptr = db_ptr->parameter_ptr;
	    return RTT__SUCCESS;
	  } 
	  db_ptr++;
	}
	return RTT__OBJNOTFOUND;
}

/*************************************************************************
*
* Name:		rtt_update_time()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

int	rtt_update_time ()
{
	char	timstr[64];

	time_AtoAscii( NULL, time_eFormat_DateAndTime, timstr, sizeof(timstr));
	timstr[strlen(timstr) - 3] = '\0';
	strcpy( rtt_time, timstr);
	return RTT__SUCCESS;
}

 /*************************************************************************
*
* Name:		rtt_wait_for_return()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Wait for a return from the user befor continuing.
*
**************************************************************************/

int	rtt_wait_for_return()
{
	unsigned long	option;
	unsigned long	terminator;
	char		input_str[80];
	int		sts;

	option = RTT_OPT_NORECALL | RTT_OPT_NOEDIT | RTT_OPT_NOECHO | 
		RTT_OPT_TIMEOUT;

	rtt_cursor_abs( 0, 23);
	r_print_buffer();
	rtt_printf("	Use the return or the PF4 key to continue");
	rtt_message('S',"");
	while ( 1)
	{	
	  rtt_get_input( (char *) rtt_chn, input_str, &terminator, 1, option,
		rtt_scantime);

	  if (terminator == RTT_K_TIMEOUT)
	  {
	    /* Get new alarm messages */
	    sts = rtt_scan ( 0);
	    if ( EVEN(sts)) return sts;
	  }
	  else if ( terminator == RTT_K_CTRLK)
	    sts = rtt_alarm_ack_last();
	  else if ((terminator == RTT_K_RETURN) || (terminator == RTT_K_PF4))
	    break;
	}
	return 1;
}

 /*************************************************************************
*
* Name:		rtt_clear_screen()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Clear the screen.
*
**************************************************************************/

int	rtt_clear_screen()
{
	rtt_display_erase();
	rtt_cursor_abs( 0, 0);
	r_print_buffer();
	return RTT__SUCCESS;
}



/*************************************************************************
*
* Name:		rtt_scan()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.	
*
* Description:
*	This routine is called every second.
*
**************************************************************************/

int	rtt_scan(
			menu_ctx	ctx)
{
	int	sts;

        /* Check if rt_ini left us a message */
        get.data = NULL;
        qcom_Get(&sts, &my_q, &get, 0);
        if (sts != QCOM__TMO && sts != QCOM__QEMPTY) {
          if (get.type.b == qcom_eBtype_event) {
	    if (get.type.s == qcom_cIini) {
	      qcom_sEvent  *ep = (qcom_sEvent*) get.data;
	      ini_mEvent   new_event;
	      
	      new_event.m = ep->mask;
	      if (new_event.b.terminate) {
	        rtt_logging_close_files();
	        qio_reset((int *) rtt_chn);
	        exit(0);
	      }
	    }
          }
          qcom_Free(&sts, get.data);
        }

	/* Get new alarm messages */
	sts = rtt_alarm_update ( ctx); 
	return sts;

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_sleep()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	rtt context.	
*
* Description:
*	Wait time ms.
*
**************************************************************************/

int	rtt_sleep(
			menu_ctx	ctx,
			int	time)
{
#if defined OS_VMS
	int		i;
	int		num;
	unsigned char	c;

	num = (1.0 / rtt_scantime + FLT_EPSILON) * time;
	for ( i = 0; i < num; i++)
	{
	  qio_read( (int *) rtt_chn, rtt_scantime, (char *) &c, 1);
	  rtt_scan( ctx);
	}
#elif defined OS_ELN
	int		i;
	int		num;
	LARGE_INTEGER	l_time;

	l_time.high = -1;
	l_time.low = - rtt_scantime * 10000;
	num = (1.0 / rtt_scantime + FLT_EPSILON) * time;
	for ( i = 0; i < num; i++)
	{
	  ker$wait_any( NULL, NULL, &l_time);
	  rtt_scan( ctx);
	}
#elif defined OS_LYNX || defined OS_LINUX
	int		i;
	int		num;
	pwr_tDeltaTime	p_time;
	struct timespec ts;

	time_FloatToD( &p_time, 0.001 *rtt_scantime);
	ts.tv_sec = p_time.tv_sec;
	ts.tv_nsec = p_time.tv_nsec;
	num = (1.0 / rtt_scantime + FLT_EPSILON) * time;
	for ( i = 0; i < num; i++)
	{
	  nanosleep( &ts, NULL);
	  rtt_scan( ctx);
	}
	
#endif

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_defaultfilename()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

int	rtt_get_defaultfilename(
			char	*inname,
			char	*outname,
			char	*ext)
{
	pwr_tFileName	filename;
	char 		*s, *s2;

#if defined(OS_ELN)
	/* Add default vmsnode if no node is suplied */
	s = strstr( inname, "::");
	if ( s == 0)
	{
	  if ( !strcmp( rtt_DefaultVMSNode, ""))
	  {
	    /* Use the system directory as default */
	    if ( strchr( inname, '<') || strchr( inname, '[') || 
		 strchr( inname, ':'))
	      strcpy( filename, "");
	    else
	      strcpy( filename, "disk$sys:[sys0.sysexe]");
	  }
	  else
	    strcpy( filename, rtt_DefaultVMSNode);
	  strcat( filename, inname);
	  strcpy( outname, filename);
	}
	else
	  strcpy( outname, inname);
	
#elif defined(OS_VMS)
	if ( strchr( inname, '<') || strchr( inname, '[') || 
	     strchr( inname, ':'))
	  strcpy( outname, inname);
	else
	{
	  strcpy( filename, rtt_default_directory);
	  strcat( filename, inname);
	  strcpy( outname, filename);
	}
#elif defined(OS_LYNX) || defined(OS_LINUX)
	if ( strchr( inname, '/'))
	  strcpy( outname, inname);
	else
	{
          if ( strcmp( rtt_default_directory, "") != 0)
          {
	    strcpy( filename, rtt_default_directory);
	    if ( (filename[strlen(filename)-1] != '/') &&
	         (inname[0] != '/'))
	      strcat( filename, "/");
	    strcat( filename, inname);
          }
          else
	    strcpy( filename, inname);
	  rtt_replace_env( filename, outname);
	}
#endif

	/* Look for extention in filename */
	if ( ext != NULL)
	{
	  s = strrchr( inname, ':');
	  if ( s == 0)
	    s = inname;

	  s2 = strrchr( s, '>');
	  if ( s2 == 0)
	  {
	    s2 = strrchr( s, ']');
	    if ( s2 == 0)
	      s2 = s;
	  }

	  s = strrchr( s2, '.');
	  if ( s == 0)
	  {
	    /* No extention found, add extention */
	    strcat( outname, ext);
	  }
	}

#if defined(OS_LYNX) || defined(OS_LINUX)
	cdh_ToLower( outname, outname);
#endif
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_exit_now()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

void	rtt_exit_now( int disconnected, pwr_tStatus exit_sts)
{
	int	sts;

	sts = rtt_alarm_disconnect();
	if ( !disconnected)
	{
	  rtt_cursor_abs( 0, 23);
	  r_print_buffer();
	}
	if ( rtt_gdh_started)
	  gdh_UnrefObjectInfoAll();

	rtt_logging_close_files();
	qio_reset((int *) rtt_chn);

#if defined(OS_LYNX) || defined(OS_LINUX)
	/* Returnstatus 0 is OK for UNIX commands */
	if (EVEN(exit_sts))
	  exit( exit_sts);
	else
	  exit( 0);
#endif
#if defined(OS_VMS) || defined(OS_ELN)
	exit( exit_sts);
#endif
}

/*************************************************************************
*
* Name:		rtt_print()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Print. Equivalent to printf but the character string is put
*	in a buffer and qio_write i called.
*	The max size of the character string is 2000.
*
**************************************************************************/

int	rtt_printf(	char		*format, ... )
{
	char	buff[2000];
	int	sts = 1;
	va_list ap;

	va_start( ap, format);
	vsprintf( buff, format, ap);
	va_end( ap);
	qio_writew( (int *) rtt_chn, buff, strlen(buff) );
	return sts;
}

/*************************************************************************
*
* Name:		rtt_get_fastkey_picture()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

int	rtt_get_fastkey_picture( menu_ctx ctx)
{
	rtt_t_menu	*menu_ptr;
	int		found;
	int		key;
	int		sts;

	menu_ptr = rtt_root_menu;

	found = 0;
	while ( menu_ptr->text[0] != '\0' )
	{
	  if ( menu_ptr->func == &rtt_menu_keys_new)
	  {
	    found = 1;
	    break;
	  }
	  menu_ptr++;
	}
	if ( !found)
	{
	  rtt_message('E', "Picture not defined");
	  return RTT__NOPICTURE;
	}	
	menu_ptr = *(rtt_t_menu **) menu_ptr->arg1;

	key = rtt_fastkey - 1;
	rtt_fastkey = 0;
	if ( (menu_ptr + key)->func != NULL)
	{
	  sts = ((menu_ptr + key)->func) ( ctx,
			(menu_ptr + key)->argoi,
			(menu_ptr + key)->arg1,
			(menu_ptr + key)->arg2,
			(menu_ptr + key)->arg3,
			(menu_ptr + key)->arg4);
	  return sts;
	}
	else
	{
	  rtt_message('E', "Picture not defined");
	  return RTT__NOPICTURE;
	}
}

/*************************************************************************
*
* Name:		rtt_get_fastkey_picture()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

int	rtt_get_fastkey_type( )
{
	rtt_t_menu	*menu_ptr;
	rtt_t_menu	*fastkey_menu_ptr;
	int		found;
	int		key;
	char		command[200];
	int		i;

	menu_ptr = rtt_root_menu;

	found = 0;
	while ( menu_ptr->text[0] != '\0' )
	{
	  if ( menu_ptr->func == &rtt_menu_keys_new)
	  {
	    found = 1;
	    break;
	  }
	  menu_ptr++;
	}
	if ( !found)
	{
	  rtt_message('E', "Picture not defined");
	  return RTT__NOPICTURE;
	}

	fastkey_menu_ptr = *(rtt_t_menu **) menu_ptr->arg1;

	/* Count items in the menu */
	menu_ptr = fastkey_menu_ptr;
	i = 0;
	while ( menu_ptr->text[0] != '\0' )
	{
	  i++;
	  menu_ptr++;
	}

	if ( rtt_fastkey > i)
	{
	  rtt_message('E', "Key not defined");
	  return RTT__NOPICTURE;
	}
	key = rtt_fastkey - 1;
	if ( (fastkey_menu_ptr + key)->func == &rtt_menu_command)
	{
	  strcpy( command, (char *)((fastkey_menu_ptr + key)->arg1));
	  rtt_toupper( command, command);
	  if ( !strncmp( command, "SET", 3))
	    return RTT__NOPICTURE;
	}
	else if ( (fastkey_menu_ptr + key)->func == &rtt_menu_commandhold)
	{
	  return RTT__NOPICTURE;
	}
	else if ( (fastkey_menu_ptr + key)->func == &rtt_menu_vmscommandhold)
	{
	  return RTT__NOPICTURE;
	}
	else
	  return RTT__SUCCESS;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_store_menuctx()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ctx		ctx		I	
* char		*key		I	
*
* Description:
*	Stores a ctx.
*
**************************************************************************/

static int	rtt_store_menuctx( 
			void		*ctx,
			void		*key)
{
	rtt_t_store_menuctx	*store_ptr;
	int	i = 0;
	int	sts;
	void	*some_ctx;

	/* Check if already stored */
	sts = rtt_get_stored_menuctx( &some_ctx, key);
	if ( ODD(sts)) return sts;

	if (rtt_menuctx_store == 0)
	{
	  rtt_menuctx_store = calloc(  400, sizeof(rtt_t_store_menuctx));
	}
	store_ptr = rtt_menuctx_store;

	while ( store_ptr->ctx != 0)
	{
	  store_ptr++;	
	  i++;
	}
	if ( i > 398)
	  return 0;

	store_ptr->ctx = ctx;
	store_ptr->key = key;

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_get_stored_menuctx()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ctx		ctx		I	
* char		*key		I	
*
* Description:
*	Returns a stored ctx.
*
**************************************************************************/

static int	rtt_get_stored_menuctx(
				void		**ctx,
				void		*key)
{
	rtt_t_store_menuctx	*store_ptr;

	if (rtt_menuctx_store == 0)
	  return 0;
	/* Zero is not defined as a key */
	if ( key == 0)
	  return 0;

	store_ptr = rtt_menuctx_store;
	while ( store_ptr->ctx != 0)
	{
	  if ( store_ptr->key == key)
	  {
	    *ctx = store_ptr->ctx;
	    return RTT__SUCCESS;
	  }
	  store_ptr++;	
	}
	return 0;
}

unsigned int	rtt_exception(
	void	*signalP,
	void	*mechanismP)
{
#ifdef OS_VMS
	exit( ((struct chf$signal_array *) signalP)->chf$l_sig_name);
#elif OS_ELN
	ker$exit( NULL, ((struct chf$signal_array *) signalP)->chf$l_sig_name);
#endif
	return 1;
}

/*************************************************************************
*
* Name:		rtt_get_system_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*system_name	O	Pointer to buffer where SystemName
[					will be returned
* int		size		I	size of the buffer.
*
* Description:
*	Returns the content of the SystemName attribute in the
*	System object.
*
**************************************************************************/
static int	rtt_get_system_name( char *system_name, int size)
{
	pwr_tStatus	sts;
	pwr_tObjid	objid;
	pwr_tAttrRef	aref;
	pwr_sSystem	sys;	

	sts = gdh_GetClassList( pwr_eClass_System, &objid);
	if (EVEN(sts)) {
	  strcpy( system_name, "");
	  return sts;
	}

	aref = cdh_ObjidToAref( objid);
	sts = gdh_GetObjectInfoAttrref( &aref, &sys, sizeof(sys));
	if ( EVEN(sts)) return sts;

	strncpy( system_name, sys.SystemName, size);
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_parse_mainmenu()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Parse the mainmenu_title into a title and an prefix.
*	The title will be shown on the first page, the prefix
*	in every menu page.
*
**************************************************************************/
static int	rtt_parse_mainmenu( char *mainmenu_title)
{
	char	*s;

	s = strchr( mainmenu_title, '/');
	if ( s == 0)
	  strcpy( rtt_mainmenu_title, mainmenu_title);
	else
	{
	  *s = 0;
	  rtt_replace_symbol( mainmenu_title, rtt_mainmenu_title);
	  rtt_replace_symbol( s + 1, rtt_title_prefix);
	  if ( strlen(rtt_title_prefix) > 50)
	    rtt_title_prefix[50] = 0;

	}
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_cut_segments()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

int rtt_cut_segments (
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

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_menu_get_parent_text()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*		Get the text of the parent menuitem.
**************************************************************************/

int	rtt_menu_get_parent_text( 	menu_ctx parent_ctx, 
					char *text)
{
	rtt_t_menu	*menu_ptr;

	if ( parent_ctx == NULL)
	{
	  strcpy( text, "");
	  return RTT__SUCCESS;
	}	
	menu_ptr = parent_ctx->menu + parent_ctx->current_item;
	strcpy( text, menu_ptr->text);
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_setup()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
*
* Description:
*	Display setup parameters.
*
**************************************************************************/

int	rtt_setup( menu_ctx	parent_ctx)
{
	int		sts;
	char		title[80];
	rtt_t_menu_upd	*menulist = 0;
	int		i;
	unsigned long	elements;
	char 		*parameter_ptr;
	char		text[80];
	int		size;
	unsigned int	type;
	unsigned int	priv;
	float		maxlimit;
	float		minlimit;

	elements = 13;

	strcpy( title, "Rtt Setup");

	/* Allocate memory for menu list */
	sts = rtt_menu_upd_list_add_malloc( &menulist, elements);
	if ( EVEN(sts)) return sts;

	for ( i = 0; i < (int)elements; i++)
	{
	  maxlimit = 0;
	  minlimit = 0;
	  if ( i == 0)
	  {
	    strcpy( text, "Platform");
	    parameter_ptr = (char *) &rtt_platform;
	    size = sizeof( rtt_platform);
	    type = pwr_eType_String;
	    priv = RTT_PRIV_NO;
	  }
	  else if ( i == 1)
	  {
	    strcpy( text, "ConfigureObject");
	    parameter_ptr = (char *) &rtt_ConfigureObject;
	    size = sizeof( rtt_ConfigureObject);
	    type = pwr_eType_String;
	    priv = RTT_PRIV_NO;
	  }
	  else if ( i == 2)
	  {
	    strcpy( text, "DefaultVMSNode");
	    parameter_ptr = (char *) &rtt_DefaultVMSNode;
	    size = sizeof( rtt_DefaultVMSNode);
	    type = pwr_eType_String;
	    priv = RTT_PRIV_OP;
	  }
	  else if ( i == 3)
	  {
	    strcpy( text, "DescriptionOn");
	    parameter_ptr = (char *) &rtt_description_on;
	    size = sizeof( rtt_description_on);
	    type = pwr_eType_Boolean;
	    priv = RTT_PRIV_OP;
	  }	  
	  else if ( i == 4)
	  {
	    strcpy( text, "DefaulDirectory");
	    parameter_ptr = (char *) &rtt_default_directory;
	    size = sizeof( rtt_default_directory);
	    type = pwr_eType_String;
	    priv = RTT_PRIV_OP;
	  }	  
	  else if ( i == 5)
	  {
	    strcpy( text, "Scantime (ms)");
	    parameter_ptr = (char *) &rtt_scantime;
	    size = sizeof( rtt_scantime);
	    type = pwr_eType_Int32;
	    priv = RTT_PRIV_OP;
	    maxlimit = 10000;
	    minlimit = 1000;
	  }	  
	  else if ( i == 6)
	  {
	    strcpy( text, "AlarmMessage");
	    parameter_ptr = (char *) &rtt_AlarmMessage;
	    size = sizeof( rtt_AlarmMessage);
	    type = pwr_eType_Boolean;
	    priv = RTT_PRIV_OP;
	  }	  
	  else if ( i == 7)
	  {
	    strcpy( text, "AlarmBeep");
	    parameter_ptr = (char *) &rtt_AlarmBeep;
	    size = sizeof( rtt_AlarmBeep);
	    type = pwr_eType_Boolean;
	    priv = RTT_PRIV_OP;
	  }	  
	  else if ( i == 8)
	  {
	    strcpy( text, "AlarmReturn");
	    parameter_ptr = (char *) &rtt_AlarmReturn;
	    size = sizeof( rtt_AlarmReturn);
	    type = pwr_eType_Boolean;
	    priv = RTT_PRIV_OP;
	  }	  
	  else if ( i == 9)
	  {
	    strcpy( text, "AlarmAck");
	    parameter_ptr = (char *) &rtt_AlarmAck;
	    size = sizeof( rtt_AlarmAck);
	    type = pwr_eType_Boolean;
	    priv = RTT_PRIV_OP;
	  }	  
	  else if ( i == 10)
	  {
	    strcpy( text, "SymbolFilename");
	    parameter_ptr = (char *) &rtt_symbolfilename;
	    size = sizeof( rtt_symbolfilename);
	    type = pwr_eType_String;
	    priv = RTT_PRIV_OP;
	  }	  
	  else if ( i == 11)
	  {
	    strcpy( text, "Verify");
	    parameter_ptr = (char *) &rtt_verify;
	    size = sizeof( rtt_verify);
	    type = pwr_eType_Boolean;
	    priv = RTT_PRIV_OP;
	  }	  
	  else if ( i == 12)
	  {
	    strcpy( text, "SignalTestModeOn");
	    parameter_ptr = (char *) &rtt_signal_test_mode;
	    size = sizeof( rtt_signal_test_mode);
	    type = pwr_eType_Boolean;
	    priv = RTT_PRIV_EL;
	  }	  

	  sts = rtt_menu_upd_list_add( &menulist, i, elements,
		text, 
		0, 
		0,
		0, pwr_cNObjid, 0, 0, 0, 0,
		text, priv, parameter_ptr, type, 
		0, size, pwr_cNDlid, 0, 0, 0, 0,
		maxlimit, minlimit, RTT_DATABASE_USER, 0);
	  if ( EVEN(sts)) return sts;
	}

	sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	if ( sts == RTT__FASTBACK) return sts;
	else if ( sts == RTT__BACKTOCOLLECT) return sts;
	else if ( EVEN(sts)) return sts;

	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_help_show_all()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parent rtt context.
*
* Description:
*	Display setup parameters.
*
**************************************************************************/

static int	rtt_help_show_all( 
			menu_ctx	parent_ctx,
			rtt_t_helptext	*helptext)
{
	int		sts;
	char		title[80];
	rtt_t_menu	*menulist = 0;
	int		i;
	rtt_t_helptext	*helptext_ptr;
	int		subject_count;


	strcpy( title, "Help subjects");

	helptext_ptr = helptext;
	subject_count = 0;
	while ( helptext_ptr->subject[0] != '\0')
	{
	  if ( helptext_ptr->view_in_list)
	    subject_count++;
	  helptext_ptr++;
	}

	/* Allocate memory for menu list */
	sts = rtt_menu_list_add_malloc( &menulist, subject_count);
	if ( EVEN(sts)) return sts;

	helptext_ptr = helptext;
	i = 0;
	while ( helptext_ptr->subject[0] != '\0')
	{
	  if ( helptext_ptr->view_in_list)
	  {
	    sts = rtt_menu_list_add( &menulist, i, subject_count, 
		helptext_ptr->subject,
		&rtt_view_buffer,
		NULL, NULL, pwr_cNObjid, 0, helptext_ptr->text,
		helptext_ptr->subject,(void *)RTT_VIEWTYPE_BUF);
	    if ( EVEN(sts)) return sts;
	    i++;
	  }
	  helptext_ptr++;
        }

	sts = rtt_menu_bubblesort( menulist);
	sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	if ( sts == RTT__FASTBACK) return sts;
	else if ( sts == RTT__BACKTOCOLLECT) return sts;
	else if (EVEN(sts)) return sts;

	return RTT__SUCCESS;
}

char	*rtt_pwr_dir( char *dir)
{
	static char pwr_dir[120];
	char 	*s;

#if defined(OS_ELN)
	if ( strcmp( rtt_DefaultVMSNode, "") == 0)
	  strcpy( pwr_dir, "[sys0.sysexe]");
	else
	{
	  strcpy( pwr_dir, rtt_DefaultVMSNode);
	  strcat( pwr_dir, dir);
	  strcat( pwr_dir, ":");
	}
#elif defined(OS_VMS)
	strcpy( pwr_dir, dir);
	strcat( pwr_dir, ":");
#elif defined(OS_LYNX) || defined(OS_LINUX)
	if ( (s = getenv( dir)) == NULL)
	  strcpy( pwr_dir, "");
        else
	{
	  strcpy( pwr_dir, s);
	  strcat( pwr_dir, "/");
	}
#endif
	return pwr_dir;
}

int rtt_ctx_push( menu_ctx ctx)
{
  if ( rtt_ctxlist_count > CTXLIST_SIZE-2)
    return 0;
  rtt_ctxlist[rtt_ctxlist_count] = ctx;
  rtt_ctxlist_count++;
  return 1;
}

int rtt_ctx_pop()
{
  rtt_ctxlist_count--;
  return 1;
}

menu_ctx rtt_current_ctx()
{
  return rtt_ctxlist[rtt_ctxlist_count-1];
}

