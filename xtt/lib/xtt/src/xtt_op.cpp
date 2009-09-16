/* 
 * Proview   $Id: xtt_op.cpp,v 1.9 2008-04-01 14:19:30 claes Exp $
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

/* xtt_op.cpp -- Alarm and event window in xtt */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>

#include "co_cdh.h"
#include "co_time.h"
#include "co_syi.h"
#include "pwr_baseclasses.h"
#include "rt_gdh.h"
#include "rt_mh.h"
#include "rt_mh_outunit.h"
#include "co_wow.h"
#include "co_lng.h"
#include "co_xhelp.h"
#include "xtt_op.h"
#include "rt_xnav_msg.h"

#define OP_HEIGHT_MIN 80
#define OP_HEIGHT_INC 20
#define OP_HEIGHT_MAX (OP_HEIGHT_MIN + 3 * OP_HEIGHT_INC)


Op::Op( void *op_parent_ctx,
	char *opplace,
	pwr_tStatus *status) :
  parent_ctx(op_parent_ctx), start_jop(0), 
  jop(NULL), command_cb(NULL), map_cb(NULL), help_cb(NULL), 
  close_cb(NULL), get_alarm_info_cb(NULL), ack_last_cb(NULL), sup_timerid(0)
{
  sup_init();
}


//
//  Delete op
//
Op::~Op()
{
  if ( jop)
    jop->close();
}

int Op::jop_command( char *command) 
{ 
  if ( jop) 
    return jop->command( command);
  return 0;
}

void Op::scan()
{
  if ( jop)
    jop->scan();
}

int Op::appl_action( int idx)
{
  char cmd[80];
  char name[80];
  int sts;

  if ( command_cb) {
    sts = gdh_AttrrefToName( &button_aref[idx], name, sizeof(name), cdh_mNName);
    strcpy( cmd, "ope gra/obj=");
    strcat( cmd, name);

    command_cb( parent_ctx, cmd);
  }
  return XNAV__SUCCESS;
}


void Op::activate_exit()
{
  if ( close_cb)
    (close_cb)(parent_ctx);
  else
    delete this;
}

void Op::activate_aalarm_ack()
{

  if ( ack_last_cb)
    (ack_last_cb)( parent_ctx, evlist_eEventType_Alarm, mh_eEventPrio_A);
}

void Op::activate_balarm_ack()
{
  if ( ack_last_cb)
    (ack_last_cb)( parent_ctx, balarm_type, balarm_prio);
}

void Op::activate_alarmlist()
{
  char cmd[20] = "show alarm";
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_eventlist()
{
  char cmd[20] = "show event";
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_eventlog()
{
  char cmd[20] = "show histlist";
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_blocklist()
{
  char cmd[20] = "show blocklist";
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_trend()
{
  char cmd[200] = "show objectlist/class=dstrend/title=\"Trend List\"";

  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_fast()
{
  char cmd[200] = "show objectlist/class=dsfastcurve/title=\"Fast Curve List\"";

  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_history()
{
  char cmd[200] = "show objectlist/class=sevhist/title=\"Process History List\"";
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_graph()
{
  char cmd[200] = "show objectlist/class=xttgraph/title=\"Process Graphic List\"";

  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_navigator()
{
  if ( map_cb)
    map_cb( parent_ctx);
}

void Op::activate_switch_user()
{
  char cmd[200] ="login";
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_show_user()
{
  char cmd[200] ="show user/window";
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_logout()
{
  char cmd[200] ="logout /messagewindow";
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_cmd_menu_item( char *cmd)
{
  if ( command_cb)
    command_cb( parent_ctx, cmd);
}

void Op::activate_sup_node( void *id)
{
  pwr_tCmd cmd;

  for ( unsigned int i = 0; i < sup_vect.size(); i++) {
    if ( sup_vect[i].buttonw == id) {
      sprintf( cmd, "open graph/class/inst=%s", sup_vect[i].object_name);
      if ( command_cb)
	command_cb( parent_ctx, cmd);
      break;
    }
  }
}

void Op::activate_help()
{
  if ( help_cb)
    help_cb( parent_ctx, "index");
}

void Op::activate_help_overview()
{
  if ( help_cb)
    help_cb( parent_ctx, "overview");
}

void Op::activate_help_opwin()
{
  CoXHelp::dhelp("opg_opwindow", "", navh_eHelpFile_Other, "$pwr_lang/man_opg.dat", 0);
}

void Op::activate_help_proview()
{
  CoXHelp::dhelp( "version", "", navh_eHelpFile_Other, "$pwr_exe/xtt_version_help.dat", 0);
}

void Op::jop_command_cb( void *op, char *command)
{
  if ( ((Op *)op)->command_cb)
    ((Op *)op)->command_cb( ((Op *)op)->parent_ctx, command);
}

int Op::sup_init()
{
  pwr_tOid node_oid;
  pwr_tOid sup_oid;
  pwr_tAName aname;
  pwr_tStatus sts;
  

  // Index 0 is current node
  sts = gdh_GetNodeObject( 0, &node_oid);
  if ( EVEN(sts)) return sts;

  OpSup sup;
  sup.node_oid = node_oid;
  sts = gdh_ObjidToName( node_oid, sup.object_name, sizeof(sup.object_name), cdh_mName_volumeStrict);
  if ( EVEN(sts)) return sts;

  strcpy( aname, sup.object_name);
  strcat( aname, ".SystemStatus");
  sts = gdh_RefObjectInfo( aname, (void **)&sup.p, &sup.refid, sizeof(pwr_tStatus));
  if ( EVEN(sts)) return sts;

  syi_NodeName( &sts, sup.node_name, sizeof(sup.node_name));

  sup_vect.push_back(sup);			 

  // Add nodes in NodeLinkSup objects
  for ( sts = gdh_GetClassList( pwr_cClass_NodeLinkSup, &sup_oid);
	ODD(sts);
	sts = gdh_GetNextObject( sup_oid, &sup_oid)) {
    pwr_sClass_NodeLinkSup *sup_p;
    qcom_sNode 		qnode;
    pwr_tNid		nid;

    sts = gdh_ObjidToPointer( sup_oid, (void **)&sup_p);

    OpSup nsup;

    nsup.node_oid = sup_p->Node;

    sts = gdh_ObjidToName( nsup.node_oid, nsup.object_name, sizeof(nsup.object_name), 
			   cdh_mName_volumeStrict);
    if ( EVEN(sts)) strcpy( nsup.object_name, "");
    
    sts = gdh_ObjidToName( sup_oid, aname, sizeof(aname), cdh_mName_volumeStrict);
    if ( EVEN(sts)) return sts;
    
    strcat( aname, ".SystemStatus");
    sts = gdh_RefObjectInfo( aname, (void **)&nsup.p, &sup.refid, sizeof(pwr_tStatus));
    if ( EVEN(sts)) return sts;

    int found = 0;
    for (nid = qcom_cNNid; qcom_NextNode(&sts, &qnode, nid); nid = qnode.nid) {
      if ( qnode.nid == nsup.node_oid.vid) {
	strcpy( nsup.node_name, qnode.name);
	found = 1;
	break;
      }
    }
    if ( !found)
      strcpy( nsup.node_name, "Unknown");

    sup_vect.push_back(nsup);			 
  }
  return 1;
}

void Op::sup_scan( void *data)
{
  Op *op = (Op *) data;
  int time = 1000;

  for ( unsigned int i = 0; i < op->sup_vect.size(); i++) {
    op_eSupColor color;
    pwr_tStatus status = *op->sup_vect[i].p;

    if ( status == 0)
      color = op_eSupColor_Gray;
    else if ( errh_SeveritySuccess( status) || errh_SeverityInfo( status))
      color = op_eSupColor_Green;
    else if ( errh_SeverityWarning( status))
      color = op_eSupColor_Yellow;
    else if ( errh_SeverityError( status))
      color = op_eSupColor_Red;
    else if ( errh_SeverityFatal( status)) {
      if ( op->sup_vect[i].old_color == op_eSupColor_Red)
	color = op_eSupColor_Black;
      else
	color = op_eSupColor_Red;
    }

    if ( color != op->sup_vect[i].old_color) {
      op->sup_vect[i].old_color = color;
      op->change_sup_color( op->sup_vect[i].imagew, color);
    }
  }
  op->sup_timerid->add( time, sup_scan, op);
}

