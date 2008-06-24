/* 
 * Proview   $Id: wb_uted.cpp,v 1.2 2008-06-24 07:52:21 claes Exp $
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

/* wb_uted.cpp */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>

#include "pwr.h"
#include "flow_ctx.h"
#include "co_time.h"
#include "co_msg.h"
#include "co_dcli.h"
#include "co_dcli.h"
#include "wb_ldh.h"
#include "wb_foe.h"
#include "wb_foe_msg.h"
#include "wb_vldh_msg.h"
#include "wb_cmd_msg.h"
#include "wb_vldh.h"
#include "wb_uted.h"
#include "wb_wtt.h"

#define	BEEP	    putchar( '\7' );

#define UTED_INDEX_NOCOMMAND -1
#define UTED_TEXT_NOCOMMAND "No command selected"

uted_sCommand WUted::commands[ UTED_MAX_COMMANDS ] = {
{ "List Descriptor", UTED_PROC_PWRPLC, 1, 3,	{
/*** QUALIFIER	***** INSERT *** INSERT HIER ** VALUE * TOGGLE *** TYPE ****/  
  {"Descriptor",	1, 	UTED_INS_NODE,	1, 	0, UTED_QUAL_QUAL},
  {"Hierarchy",		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Object",		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Volumes", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"AllVolumes", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Output",		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "List Signals", UTED_PROC_PWRPLC,  1, 3,	{
  {"Hierarchy",		1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"Volumes", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"AllVolumes", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Output",		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Shortname", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "List Channels", UTED_PROC_PWRPLC,  1, 3,	{
  {"Node",		1, 	UTED_INS_NODE,	1, 	0, UTED_QUAL_QUAL},
  {"Output",		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "List Hierarchy", UTED_PROC_PWRPLC,  1, 3,	{
  {"Hierarchy",		1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"Volumes", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"AllVolumes", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Output",		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "List Plcpgm", UTED_PROC_PWRPLC,  1, 3,	{
  {"Plcpgm",		1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"Hierarchy",		1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"Volumes", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"AllVolumes", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Output",		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Show Class", UTED_PROC_PWRPLC, 1, 3,{
  {"Classhier", 	0, 	0, 		1, 	0, UTED_QUAL_QUAL},
  {"Name", 		0, 	0, 		1, 	0, UTED_QUAL_QUAL},
  {"Full", 		0, 	0, 		0, 	1, UTED_QUAL_QUAL},
  {"All", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Contents", 		0, 	0,		0,	1, UTED_QUAL_QUAL},
  {"Output", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Show Objects", UTED_PROC_PWRPLC, 1, 3,{
  {"Hierarchy", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Class", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Name", 		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Parameter", 	0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Volumes", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"AllVolumes", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Objid", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Full", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Output", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Show Modules", UTED_PROC_PWRPLC, 1, 1,{
  {"Name", 		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Hierarchy",		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Objid", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Output", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Show Security", UTED_PROC_PWRPLC, 1, 1,{
  {"",}}},
{ "Compile", UTED_PROC_PWRPLC, 0, 3,{
  {"Window", 		1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"Plcpgm", 		1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"Hierarchy", 	1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"Volumes", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"AllVolumes", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Modified", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Debug", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"From_plcpgm", 	1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Crossreference", UTED_PROC_PWRPLC, 1, 3,{
  {"Hierarchy", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Class", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Name", 		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Sort", UTED_PROC_PWRPLC,  0, 1,	{
  {"Parent",		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Class", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Signals", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Print", UTED_PROC_PWRPLC, 1, 3,{
  {"Hierarchy", 	1, 	UTED_INS_PLANT,	1,	0, UTED_QUAL_QUAL},
  {"Plcpgm", 		1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"All", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Nodocument", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Nooverview", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"From_plcpgm", 	1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Copy Objects", UTED_PROC_PWRPLC, 0, 1,{
  {"Source",	 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Destination", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Name", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Hierarchy", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"First", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Last", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Before", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"After", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Set Attribute", UTED_PROC_PWRPLC, 0, 1,{
  {"Hierarchy", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Class", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Name", 		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Attribute", 	0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Value", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Noconfirm", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Log", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Output", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Set Template", UTED_PROC_PWRPLC, 0, 1,{
  {"SignalObjectSeg",	0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"ShoSigChanCon", 	0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"SigChanConSeg",     0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"ShoDetectText", 	0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Configure Card", UTED_PROC_PWRPLC, 0, 1,{
  {"Rack", 		1, 	UTED_INS_NODE,	1, 	0, UTED_QUAL_QUAL},
  {"Cardname", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Cardclass", 	0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Channelname",	0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Chanidentity",	0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Chandescription",	0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Table",		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Export PlcPgm", UTED_PROC_PWRPLC, 1, 1,{
  {"PlcPgm", 		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Window", 		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Output", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Redraw", UTED_PROC_PWRPLC, 0, 3,{
  {"Hierarchy", 	1, 	UTED_INS_PLANT,	1,	0, UTED_QUAL_QUAL},
  {"Plcpgm", 		1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"All", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"From_plcpgm", 	1, 	UTED_INS_PLANT,	1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Move Object", UTED_PROC_PWRPLC, 0, 1,{
  {"Source",	 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Destination", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Rename", 		1, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"First", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Last", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Before", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"After", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Create Object", UTED_PROC_PWRPLC, 0, 1,{
  {"Destination", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Name", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Class", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"First", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Last", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Before", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"After", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Create StructFiles", UTED_PROC_PWRPLC, 0, 1,{
  {"Files", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Create LoadFiles", UTED_PROC_PWRPLC, 0, 1,{
  {"Volume", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"All",	 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Create BootFiles", UTED_PROC_PWRPLC, 0, 1,{
  {"NodeConfig", 	0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"All", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Debug", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Create RttFiles", UTED_PROC_PWRPLC,  0, 1,	{
  {"",}}},
{ "Delete Object", UTED_PROC_PWRPLC, 0, 1,{
  {"Hierarchy", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Class", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Name", 		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Noconfirm", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Log", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Delete Tree", UTED_PROC_PWRPLC, 0, 1,{
  {"Name", 		1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Noconfirm", 	0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Log", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Wb Dump", UTED_PROC_PWRPLC, 0, 1,{
  {"Output", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Hierarchy", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"Keepname", 		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "Wb Load", UTED_PROC_PWRPLC, 0, 1,{
  {"Loadfile", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"NoIndex",		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Output", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Wb Export", UTED_PROC_PWRPLC, 0, 1,{
  {"Output", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Hierarchy", 	1, 	UTED_INS_PLNO,	1, 	0, UTED_QUAL_QUAL},
  {"",}}},
{ "Wb Import", UTED_PROC_PWRPLC, 0, 1,{
  {"Loadfile", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Root", 		0, 	0,		1, 	0, UTED_QUAL_QUAL},
  {"Ignore",		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Output", 		0, 	0, 		1,	0, UTED_QUAL_QUAL},
  {"Full",		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"Announce",		0, 	0,		0, 	1, UTED_QUAL_QUAL},
  {"",}}},
{ "",}};

void WUted::set_editmode( int edit, ldh_tSesContext ldhses)
{
  ldhses = ldhses;
  enable_entries( edit);
}

//
//	Create a new ute window
//
WUted::WUted( void	       	*wu_parent_ctx,
	      char	       	*wu_name,
	      char	       	*wu_iconname,
	      ldh_tWBContext	wu_ldhwb,
	      ldh_tSesContext	wu_ldhses,
	      int	       	wu_editmode,
	      void 	       	(*wu_quit_cb)(void *),
	      pwr_tStatus     	*status) :
  parent_ctx(wu_parent_ctx), current_index(0), ldhwb(wu_ldhwb), ldhses(wu_ldhses),
  quit_cb(wu_quit_cb)
{
  strcpy( name, wu_name);
  *status = FOE__SUCCESS;
}

//
//	Destroys a ute instance.
//      Destroys the widget and frees allocated memory for the 
//	context.
//
WUted::~WUted()
{
}


int WUted::execute( int show)
{
  uted_sCommand	*command_ptr;
  uted_sQual	*qual_ptr;
  char		cmd[256];
  char		command[280];
  char		value[500];
  pwr_tStatus	sts;
  int		i;
  int		index;
  static char 	str[256] ;

  if ( current_index == UTED_INDEX_NOCOMMAND) {
    message( "Select a command in the menu");
    BEEP;
    return FOE__SUCCESS;
  }

  index = current_index;
  command_ptr = &commands[index];
  strcpy(cmd, command_ptr->command);
  
  i = 0;
  qual_ptr = &commands[index].qualifier[0];
  while ( qual_ptr->qual[0] != 0) {
    if ( qual_ptr->present) {
      if ( present_sts[i]) {
	if ( qual_ptr->type == UTED_QUAL_QUAL ) {
	  strcat(cmd, "/");
	  strcat(cmd, qual_ptr->qual);
	}
	else if ( qual_ptr->type == UTED_QUAL_PAR ) {
	  strcat(cmd, " ");
	  strcat(cmd, qual_ptr->qual);
	}
      }
    }
    else {
      /* Get the value */
      get_value( i, value, sizeof(value));
      if ( value[0] != 0) {
	if ( qual_ptr->type == UTED_QUAL_QUAL ) {
	  strcat(cmd, "/");
	  strcat(cmd, qual_ptr->qual);
	  strcat(cmd, "=");
	  strcat(cmd, value);
	}
	else if ( qual_ptr->type == UTED_QUAL_PAR ) {
	  strcat(cmd, " ");
	  strcat(cmd, value);
	}
      }
    }
    qual_ptr++;
    i++;
  }
  if ( show) {
  }
  
  /* Execute something... */
  if ( command_ptr->process == UTED_PROC_PWRPLC) {
    if ( show) {
      strcpy(command, "Command: pwr_plc ");
      strcat(command, cmd);
      message( command);
      
      /* Put in command window */
      set_command_window( cmd);
      return FOE__SUCCESS;
    }
    if (batch_sts == UTED_BATCH_CURRSESS) {
      /* Call pwr_plc in the current session */
      strcpy( str, cmd);
      sts = ((Wtt *)parent_ctx)->wnav->command( str);
      if ( EVEN(sts))
	message_error( sts);
      else {
	message( "Done");
	BEEP;
      }
    }
    else if (batch_sts == UTED_BATCH_BATCH) {
      message( "Batch not implemented in this environment");
    }
  }
  else if ( command_ptr->process == UTED_PROC_DCL) {
    if (batch_sts == UTED_BATCH_CURRSESS) {
      /* Call dcl in the current session */
    }
    else if (batch_sts == UTED_BATCH_BATCH) {
      /* Submit dcl */
    }
  }
  return FOE__SUCCESS;
}

pwr_tStatus WUted::get_command_index( char *label, int *index)
{
  uted_sCommand	*command_ptr;
  int		i;
  int		found;

  i = 0;
  found = 0;
  command_ptr = commands;
  while ( command_ptr->command[0] != 0) {
    if ( strcmp( command_ptr->command, label) == 0) {
      *index = i;
      found = 1;
      break;
    }
    i++;
    command_ptr++;
  }
  if ( !found)
    return 0;

  return FOE__SUCCESS;
}

void WUted::get_filename( char *filename)
{
  int 	val;
  pwr_tTime	time;

  clock_gettime(CLOCK_REALTIME, &time);
  srand( time.tv_sec);
  val = rand();
  sprintf( filename, "pwrp_tmp:uted_batch_%d.com", val);
}

//
//	Show a message.
//
void WUted::message_error( pwr_tStatus sts)
{
  char msg[256];

  msg_GetMsg( sts, msg,sizeof(msg));
  message( msg);
  BEEP;
}

//
// Convert msg status to string
//
void WUted::get_message_error( pwr_tStatus sts, char *str)
{
  char msg[256];

  msg_GetMsg( sts, msg, sizeof(msg));
  strncpy(str, msg, 80);
}

