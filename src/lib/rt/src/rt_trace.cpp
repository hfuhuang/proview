/* 
 * Proview   $Id: rt_trace.cpp,v 1.8 2008-12-03 12:00:38 claes Exp $
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

/* rt_trace.cpp -- trace in runtime environment */

#if !defined OS_ELN

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "pwr_privilege.h"

#include "flow_ctx.h"
#include "flow_api.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_wow.h"
#include "co_trace.h"
#include "rt_gdh.h"
#include "rt_trace.h"
#include "xtt_menu.h"

#define GOEN_F_GRID 0.05

void RtTrace::get_trace_attr( flow_tObject object, 
			       char *object_str, char *attr_str, flow_eTraceType *type,
			       int *inverted, unsigned int *options)
{
  pwr_tOName name;
  char *s;

  flow_GetTraceAttr( object, name, attr_str, type, inverted);
  if ( m_has_host) {
    /* Replace "$host" with hostname */
    if ( strncmp( name, "$host", 5) == 0) {
      strcpy( object_str, m_hostname);
      strcat( object_str, &name[5]);
    }
    else
      strcpy( object_str, name);
  }
  else
    strcpy( object_str, name);
  
  // Get options in attr_str after #
  if ( (s = strchr( attr_str, '#'))) {
    *s = 0;
    sscanf( s+1, "%u", options);
  }
  else
    *options = 0;
}

void RtTrace::get_save_filename( pwr_tObjid window_objid, char *filename)
{
  sprintf( filename, "$pwrp_load/pwr_%s.flwt", id_to_str( window_objid));
  dcli_translate_filename( filename, filename);
}


int RtTrace::get_filename( pwr_tObjid window_objid, char *filename,
			    int *has_host, char *hostname, char *plcconnect)
{
  FILE *fp;
  pwr_tOid host;
  pwr_tCid cid;
  pwr_tObjName cname;
  pwr_tFileName fname;
  int sts;
  pwr_tAName name;
  pwr_sAttrRef conar;

  sprintf( fname, "$pwrp_load/pwr_%s.flw", id_to_str( window_objid));
  dcli_translate_filename( fname, fname);
  
  *has_host = 0;
  fp =  fopen( fname, "r");
  if ( !fp) {
    /* Try class flowfile */
    sts = gdh_GetParent( window_objid, &host);
    if ( EVEN(sts)) return sts;

    sts = gdh_GetObjectClass( host, &cid);
    if ( EVEN(sts)) return sts;

    if ( cid == pwr_cClass_plc)
      return 0;
    
    sts = gdh_ObjidToName( cdh_ClassIdToObjid(cid), cname, sizeof(cname),
			   cdh_mName_object);
    if ( EVEN(sts)) return sts;

    cdh_ToLower( cname, cname);
    sprintf( fname, "$pwrp_load/pwr_%s.flw", cname);
    dcli_translate_filename( fname, fname);
    fp = fopen( fname, "r");
    if ( !fp) {
      /* Try base load */
      sprintf( fname, "$pwr_load/pwr_%s.flw", cname);
      dcli_translate_filename( fname, fname);
      fp = fopen( fname, "r");
      if ( !fp)
	return 0;
    }

    sts = gdh_ObjidToName( host, name, sizeof(name),
			   cdh_mName_volumeStrict);
    if ( EVEN(sts)) return sts;

    strcpy( hostname, name);
    *has_host = 1;
    
    strcat( name, ".PlcConnect");
    strcpy( plcconnect, "");
    sts = gdh_GetObjectInfo( name, &conar, sizeof(conar));
    if ( ODD(sts)) {
      sts = gdh_AttrrefToName( &conar, name, sizeof(name),
			       cdh_mName_volumeStrict);
      if ( ODD(sts))
	strcpy( plcconnect, name);
    }
  }
  fclose( fp);
  strcpy( filename, fname);
  return 1;
}

int RtTrace::connect_bc( flow_tObject object, char *name, char *attr, 
			  flow_eTraceType type, void **p)
{
  pwr_tAName   	attr_str;
  int		size;
  pwr_tSubid	*subid_p, subid;
  int		sts;
  RtTrace	*tractx;
  char 		*s;

  flow_GetCtxUserData( flow_GetCtx( object), (void **)&tractx);

/*  printf( "Connecting %s.%s\n", name, attr);  */

  if ( strcmp( name, "") == 0 || strcmp( attr, "") == 0)
    return 1;

  if ( type != flow_eTraceType_Boolean &&
       flow_GetNodeGroup( object) != flow_eNodeGroup_Trace)
    return 1;

  switch( type)
  {
    case flow_eTraceType_Boolean:
      size = sizeof(pwr_tBoolean);
      break;
    case flow_eTraceType_Int32:
      size = sizeof(pwr_tInt32);
      break;
    case flow_eTraceType_Float32:
      size = sizeof(pwr_tFloat32);
      break;
    default:
      size = sizeof(pwr_tInt32);
  }

  if ( tractx->m_has_host) {
    if ( strncmp( name, "$host", 5) == 0) {
      /* Replace "$host" with hostname */
      strcpy( attr_str, tractx->m_hostname);
      strcat( attr_str, &name[5]);
    }
    else if ( strncmp( name, "$PlcFo:", 7) == 0) {
      /* Replace "$PlcFo:" with fo name */
      s = strchr( name, '.');
      if ( !s)
	strcpy( attr_str, tractx->m_hostname);
      else {
	strcpy( attr_str, tractx->m_hostname);
	strcat( attr_str, s);
      }
    }      
    else if ( strncmp( name, "$PlcMain:", 9) == 0) {
      /* Replace "$PlcMain:" with plcconnect name */
      s = strchr( name, '.');
      if ( !s)
	strcpy( attr_str, tractx->m_plcconnect);
      else {
	strcpy( attr_str, tractx->m_plcconnect);
	strcat( attr_str, s);
      }
    }      
    else
      strcpy( attr_str, name);
  }
  else
    strcpy( attr_str, name);
  strcat( attr_str, ".");
  strcat( attr_str, attr);  
  if ( (s = strchr( attr_str, '#')))
    *s = 0;
       

  if ( flow_GetObjectType( object) == flow_eObjectType_Node)
  {
    sts = gdh_RefObjectInfo( attr_str, (void **)p, &subid, size);
    if ( EVEN(sts)) return sts;
      
    subid_p = (pwr_tSubid *) malloc( sizeof(pwr_tSubid));
    *subid_p = subid;
    flow_SetUserData( object, (void *) subid_p);
  }
  return 1;
}

int RtTrace::disconnect_bc( flow_tObject object)
{
  pwr_tSubid	*subid_p;
  int 		sts;
  flow_tTraceObj name;
  flow_tTraceAttr attr;
  flow_eTraceType type;
  int		inverted;

  if ( flow_GetObjectType( object) == flow_eObjectType_Node) {

    flow_GetTraceAttr( object, name, attr, &type, &inverted);
    if ( type != flow_eTraceType_Boolean &&
	 flow_GetNodeGroup( object) != flow_eNodeGroup_Trace)
      return 1;

    if ( !( strcmp( name, "") == 0 || strcmp( attr, "") == 0)) {
      flow_GetUserData( object, (void **) &subid_p);
      if ( subid_p) {
	sts = gdh_UnrefObjectInfo( *subid_p);
	free( (char *) subid_p);
      }
    }
  }
  return 1;
}


void RtTrace::trace_scan( void *data)
{
  RtTrace *tractx = (RtTrace *)data;

  if ( tractx->trace_started) {
    flow_TraceScan( tractx->flow_ctx);

    tractx->trace_timerid->add( (int)(tractx->scan_time * 1000), trace_scan, tractx);
  }
}


int RtTrace::init_flow( FlowCtx *ctx, void *client_data)
{
  RtTrace *tractx = (RtTrace *)client_data;

  tractx->flow_ctx = ctx;
  flow_SetCtxUserData( tractx->flow_ctx, tractx);
  return 1;
}


void RtTrace::activate_close()
{
  if ( close_cb)
    (close_cb)( this);
}

void RtTrace::activate_print()
{
  flow_tCtx ctx = (flow_tCtx)flow_ctx;
  flow_tObject 	*list;
  int		cnt;
  double	ll_x, ll_y, ur_x, ur_y;
  flow_tNode	*np;
  int		j;
  int		i = 0;
  char		filename[200];
  char		cmd[200];

  /* Get selected object */
  flow_GetObjectList( ctx, &list, &cnt);
  np = list;
  for ( j = 0; j < cnt; j++) {
    if ( cnt > 0 && flow_GetObjectType( *np) == flow_eObjectType_Node &&
	 flow_GetNodeGroup( *np) == flow_eNodeGroup_Document) {
      sprintf( filename, "$pwrp_tmp/trace%d.ps", ++i);
      dcli_translate_filename( filename, filename);

      flow_MeasureNode( *np, &ll_x, &ll_y, &ur_x, &ur_y);
      flow_PrintRegion( ctx, ll_x, ll_y, ur_x, ur_y, filename);

      sprintf( cmd, "$pwr_exe/rt_print.sh %s 1", filename);
      system( cmd);
    }
    np++;
  }
}

void RtTrace::activate_printselect()
{
  flow_tCtx ctx = (flow_tCtx)flow_ctx;
  flow_tObject 	*list;
  int		cnt;
  double	ll_x, ll_y, ur_x, ur_y;
  flow_tNode	n;
  char		filename[200];
  int 		i = 0;
  char		cmd[200];
  
  /* Get selected object */
  flow_GetSelectList( ctx, &list, &cnt);
  if ( cnt > 0 && flow_GetObjectType( *list) == flow_eObjectType_Node &&
       flow_GetNodeGroup( *list) == flow_eNodeGroup_Document)  
  {
    sprintf( filename, "$pwrp_tmp/trace%d.ps", ++i);
    dcli_translate_filename( filename, filename);

    n = *list;
    flow_MeasureNode( n, &ll_x, &ll_y, &ur_x, &ur_y);
    flow_PrintRegion( ctx, ll_x, ll_y, ur_x, ur_y, filename);

    sprintf( cmd, "$pwr_exe/rt_print.sh %s 1", filename);
    system( cmd);
  }
  else
    printf("No such node\n");
}

void RtTrace::activate_savetrace()
{
  pwr_tFileName	filename;

  get_save_filename( objid, filename);
  flow_SaveTrace( flow_ctx, filename);
}

void RtTrace::activate_restoretrace()
{
  pwr_tFileName	filename;

  if ( !trace_started)
    return;

  get_save_filename( objid, filename);
  flow_OpenTrace( flow_ctx, filename);
}

void RtTrace::activate_cleartrace()
{
  if ( !trace_started)
    return;

  flow_RemoveTraceObjects( flow_ctx);
}

void RtTrace::activate_display_object()
{
  flow_tObject 	node;
  int		sts;
  pwr_tAttrRef	attrref;
  xmenu_eItemType itemtype;

  sts = get_selected_node( &node);
  if (EVEN(sts)) return;

  sts = get_attrref( node, &attrref);
  if (EVEN(sts)) return;

  if ( attrref.Flags.b.ObjectAttr)
    itemtype = xmenu_eItemType_AttrObject;
  else if ( attrref.Flags.b.Object)
    itemtype = xmenu_eItemType_Object;
  else
    itemtype = xmenu_eItemType_Attribute;

  if ( call_method_cb) {
    (call_method_cb)( parent_ctx,
		      "$Object-RtNavigator",
		      "$Object-RtNavigatorFilter",
		      attrref, 
		      itemtype,
		      xmenu_mUtility_Trace, NULL);
  }
}

void RtTrace::activate_collect_insert()
{
  flow_tObject 	node;
  int		sts;
  pwr_tAttrRef	attrref;
  xmenu_eItemType itemtype;

  sts = get_selected_node( &node);
  if (EVEN(sts)) return;

  sts = get_attrref( node, &attrref);
  if (EVEN(sts)) return;

  if ( attrref.Flags.b.ObjectAttr)
    itemtype = xmenu_eItemType_AttrObject;
  else if ( attrref.Flags.b.Object)
    itemtype = xmenu_eItemType_Object;
  else
    itemtype = xmenu_eItemType_Attribute;

  if ( call_method_cb) {

    (call_method_cb)( parent_ctx,
		      "$Object-Collect",
		      "$Object-CollectFilter",
		      attrref, 
		      itemtype,
		      xmenu_mUtility_Trace, NULL);
  }
}

void RtTrace::activate_open_object()
{
  flow_tObject 	node;
  int		sts;
  pwr_tObjid	objid;

  sts = get_selected_node( &node);
  if (EVEN(sts)) return;

  sts = get_objid( node, &objid);
  if (EVEN(sts)) return;

  if ( call_method_cb) {
    pwr_sAttrRef attrref = cdh_ObjidToAref( objid);

    (call_method_cb)( parent_ctx,
		      "$Object-OpenObject",
		      NULL,
		      attrref, 
		      xmenu_eItemType_Object,
		      xmenu_mUtility_Trace, NULL);
  }
}

void RtTrace::activate_open_subwindow()
{
  flow_tObject 		node;
  pwr_tObjid		objid;
  pwr_tStatus          	sts;
  RtTrace	      	*new_tractx;
  trace_tNode 		*nnode;

  sts = get_selected_node( &node);
  if (EVEN(sts)) return;

  sts = get_objid( node, &objid);
  if (EVEN(sts)) return;

  /* Should check for ordercond or orderact window... */
  sts = gdh_GetChild( objid, &objid);
  if ( EVEN(sts)) return;

  if ( subwindow_cb) {
    // The parent context will start the subwindow
    (subwindow_cb)( parent_ctx, objid);
  }
  else {
    new_tractx = subwindow_new( this, objid, &sts);
    if ( ODD(sts)) {
      new_tractx->close_cb = trace_close_cb;
      new_tractx->help_cb = help_cb;
      new_tractx->display_object_cb = display_object_cb;
      new_tractx->collect_insert_cb = collect_insert_cb;
      new_tractx->is_authorized_cb = is_authorized_cb;

      nnode = trace_list;
      trace_list = (trace_tNode *) malloc(sizeof(trace_tNode));
      trace_list->Next = nnode;
      trace_list->tractx = new_tractx;     
    }
    else
      delete new_tractx;
  }
}



void RtTrace::activate_show_cross()
{
  flow_tObject 	node;
  int		sts;
  pwr_tAttrRef  attrref;
  xmenu_eItemType itemtype;

  sts = get_selected_node( &node);
  if (EVEN(sts)) return;

  sts = get_attrref( node, &attrref);
  if (EVEN(sts)) return;

  if ( attrref.Flags.b.ObjectAttr)
    itemtype = xmenu_eItemType_AttrObject;
  else if ( attrref.Flags.b.Object)
    itemtype = xmenu_eItemType_Object;
  else
    itemtype = xmenu_eItemType_Attribute;

  if ( call_method_cb) {
    (call_method_cb)(parent_ctx,
		     "$Object-OpenCrossref",
		     "$Object-OpenCrossrefFilter",
		     attrref, 
		     itemtype,
		     xmenu_mUtility_Trace, NULL);
  }
}

void RtTrace::activate_open_classgraph()
{
  flow_tObject 	node;
  int		sts;
  pwr_tAttrRef	attrref;
  xmenu_eItemType itemtype;

  sts = get_selected_node( &node);
  if (EVEN(sts)) return;

  sts = get_attrref( node, &attrref);
  if (EVEN(sts)) return;

  if ( attrref.Flags.b.ObjectAttr)
    itemtype = xmenu_eItemType_AttrObject;
  else if ( attrref.Flags.b.Object)
    itemtype = xmenu_eItemType_Object;
  else
    itemtype = xmenu_eItemType_Attribute;

  if ( call_method_cb) {

    if ( itemtype == xmenu_eItemType_Attribute)
      (call_method_cb)( parent_ctx,
			"$Object-OpenTypeGraph",
			"$Object-OpenTypeGraphFilter",
			attrref, 
			itemtype,
			xmenu_mUtility_Trace, NULL);
    else
      (call_method_cb)( parent_ctx,
			"$Object-OpenObjectGraph",
			"$Object-OpenObjectGraphFilter",
			attrref, 
			itemtype,
			xmenu_mUtility_Trace, NULL);
  }
}

void RtTrace::activate_trace()
{
  trasetup();
  if ( !trace_started) {
    trace_start();
  }
}

void RtTrace::activate_simulate()
{
  simsetup();
  if ( !trace_started) {
    trace_start();
  }
}

void RtTrace::activate_view()
{
  if ( trace_started) {
    trace_stop();
    viewsetup();
  }
}

void RtTrace::activate_help()
{
  if ( help_cb)
    (help_cb)(this, "trace");
}

void RtTrace::activate_helpplc()
{
  pwr_tOName	name;
  int 		sts;

  sts = gdh_ObjidToName( objid, name, sizeof(name), cdh_mNName); 
  if (EVEN(sts)) return;

  if ( help_cb)
    (help_cb)(this, name);
}


/*************************************************************************
*
* Name:		int	trace_flow_cb()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Callback from flow.
**************************************************************************/
int RtTrace::flow_cb( FlowCtx *ctx, flow_tEvent event)
{
  char		name[80];
  RtTrace	*tractx;
  static int	idx = 0;

  flow_GetCtxUserData( ctx, (void **)&tractx);

  if ( event->any.type == flow_eEventType_CreateCon) {
    if ( flow_GetNodeGroup( event->con_create.source_object) == 
	 flow_eNodeGroup_Trace)
      return 1;

    if ( tractx->trace_started) {
      flow_tTraceObj	       	object_str;
      flow_tTraceAttr  		attr_str;
      flow_tTraceAttr	       	con_attr_str;
      flow_eTraceType		trace_type;
      int			inverted;
      flow_tNode		n1;
      flow_tCon			c1;
      int			sts;
      flow_eDirection 		direction;
      double			x, y;

      /* Create a trace object */
      sprintf( name, "Trace%d", idx++);

      /* Connect only output points */
      sts = flow_GetConPoint( event->con_create.source_object, 
			      event->con_create.source_conpoint, &x, &y, &direction); 
      if ( EVEN(sts)) return 1;

      switch( direction)
	{
        case flow_eDirection_Center:
	case flow_eDirection_Left:
	case flow_eDirection_Up:
	case flow_eDirection_Down:
          return 1;
        default:
          ;
	}

      /* Find the trace attributes */
      flow_GetTraceAttr( event->con_create.source_object, object_str, attr_str, 
			 &trace_type, &inverted);

      /* Get attribute from connection point */
      sts = flow_GetConPointTraceAttr( event->con_create.source_object,
				       event->con_create.source_conpoint, con_attr_str, &trace_type);
      /* If "$object", use object trace attribute */
      if ( strcmp( con_attr_str, "$object") != 0)
        strcpy( attr_str, con_attr_str);

      if ( strcmp( attr_str, "") == 0)
        return 1;

      flow_CreateNode( ctx, name, tractx->trace_analyse_nc, event->object.x, event->object.y, NULL, &n1);
      flow_SetTraceAttr( n1, object_str, attr_str, trace_type, 0);

      flow_CreateCon( ctx, name, tractx->trace_con_cc, 
		      event->con_create.source_object, n1,
		      event->con_create.source_conpoint, 0,
		      NULL, &c1, 0, NULL, NULL, &sts);
    }
  }
  switch ( event->event) {
  case flow_eEvent_Init:
    break;
  case flow_eEvent_MB3Down: {
    flow_SetClickSensitivity( ctx, flow_mSensitivity_MB3Press);
    break;
  }
  case flow_eEvent_MB1Click:
    /* Select */
    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      if ( flow_FindSelectedObject( ctx, event->object.object)) {
	flow_SelectClear( ctx);
      }
      else {
	// if ( flow_GetNodeGroup( event->object.object) == flow_eNodeGroup_Document)
	//  break;
	flow_SelectClear( ctx);
	flow_SetInverse( event->object.object, 1);
	flow_SelectInsert( ctx, event->object.object);
      }
      break;
    default:
      flow_SelectClear( ctx);
    }
    break;
  case flow_eEvent_MB2DoubleClick:
    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      if ( flow_GetNodeGroup( event->object.object) == 
	   flow_eNodeGroup_Trace) {
	flow_DeleteNodeCons( event->object.object);
	flow_DeleteNode( event->object.object);
      }
      break;
    default:
      ;
    }
    break;
  case flow_eEvent_MB1Press: {
    /* Object moved */
    break;
  }
  case flow_eEvent_MB3Press: {
    pwr_sAttrRef      attrref;
    int		sts;
    int		x, y;
    unsigned int      utility;
    xmenu_eItemType itemtype;

    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      sts = tractx->get_attrref( event->object.object, &attrref);
      if ( EVEN(sts)) return 1;

      if ( attrref.Flags.b.ObjectAttr)
	itemtype = xmenu_eItemType_AttrObject;
      else if ( attrref.Flags.b.Object)
	itemtype = xmenu_eItemType_Object;
      else
	itemtype = xmenu_eItemType_Attribute;

      if ( tractx->popup_menu_cb) {
	// Display popup menu
	utility = xmenu_mUtility_Trace;
	tractx->popup_menu_position( event->any.x_pixel + 8, event->any.y_pixel, &x, &y);
	(tractx->popup_menu_cb)( tractx->parent_ctx, attrref, itemtype,
				 utility, NULL, x, y);
      }
      break;
    default:
      ;
    }

    /* Object moved */
    break;
  }
  case flow_eEvent_MB1DoubleClick: {
    /* Open attribute editor */
    pwr_tAttrRef attrref;
    int		sts;

    /* Display object */
    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      sts = tractx->get_attrref( event->object.object, &attrref);
      if (EVEN(sts)) return 1;

      if ( tractx->call_method_cb) {
	// Display crossreferences
	unsigned long utility = xmenu_mUtility_Trace;
	
	(tractx->call_method_cb)(tractx->parent_ctx, 
				 "$Object-OpenCrossref",
				 "$Object-OpenCrossrefFilter",
				 attrref, 
				 xmenu_eItemType_Object,
				 utility, NULL);
      }
      break;
    default:
      ;
    }
    break;
  }
  case flow_eEvent_MB1DoubleClickShift: {
    pwr_tOName       		name;
    flow_tName       		object_name;
    pwr_tObjid		objid;
    pwr_tStatus      		sts;
    RtTrace			*new_tractx;
    trace_tNode 		*node;

    /* Open subwindow */
    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      if ( flow_GetNodeGroup( event->object.object) != 
	   flow_eNodeGroup_Trace) {
	sts = gdh_ObjidToName( tractx->objid, name, sizeof(name), cdh_mNName); 
	if (EVEN(sts)) return 1;

	flow_GetObjectName( event->object.object, object_name);
	strcat( name, "-");
	strcat( name, object_name);

	sts = gdh_NameToObjid( name, &objid);
	if ( EVEN(sts)) return 1;

	/* Should check for ordercond or orderact window... */
	sts = gdh_GetChild( objid, &objid);
	if ( EVEN(sts)) return 1;

	if ( tractx->subwindow_cb) {
	  // The parent context will start the subwindow
	  (tractx->subwindow_cb)( tractx->parent_ctx, objid);
	}
	else {
	  new_tractx = tractx->subwindow_new( tractx, objid, &sts);
	  if ( ODD(sts)) {
	    new_tractx->close_cb = trace_close_cb;
	    new_tractx->help_cb = tractx->help_cb;
	    new_tractx->display_object_cb = tractx->display_object_cb;
	    new_tractx->collect_insert_cb = tractx->collect_insert_cb;
	    new_tractx->is_authorized_cb = tractx->is_authorized_cb;

	    if (tractx) {
	      node = tractx->trace_list;
	      tractx->trace_list = (trace_tNode *) malloc(sizeof(trace_tNode));
	      tractx->trace_list->Next = node;
	      tractx->trace_list->tractx = new_tractx;
	    }     
	  }
	  else
	    delete new_tractx;
	}
      }
      break;
    default:
      ;
    }
    break;
  }
  case flow_eEvent_MB1ClickCtrl: {
    break;
  }
  case flow_eEvent_MB1DoubleClickShiftCtrl: {
    tractx->changevalue( event->object.object);
    break;
  }
  case flow_eEvent_SelectClear:
    flow_ResetSelectInverse( ctx);
    break;
  case flow_eEvent_ScrollDown:
    flow_RemoveTipText( ctx);
    flow_Scroll( ctx, 0, -0.05);
    break;
  case flow_eEvent_ScrollUp: 
    flow_RemoveTipText( ctx);
    flow_Scroll( ctx, 0, 0.05);
    break;
  case flow_eEvent_TipText: {
    flow_tTraceObj     	object_str;
    flow_tTraceAttr    	attr_str;
    flow_eTraceType	trace_type;
    pwr_tAName		aname;
    pwr_tAName		name;
    char		tiptext[512] = "";
    pwr_tStatus		sts;
    int			inverted;
    char		*s;
    bool		is_plcmain = false;
    bool		is_plcfo = false;
    unsigned int 	options;

    tractx->get_trace_attr( event->object.object, object_str, attr_str, &trace_type, &inverted, &options);

    if ( tractx->m_has_host) {
      if ( strncmp( object_str, "$host", 5) == 0) {
	/* Replace "$host" with hostname */
	strcpy( name, tractx->m_hostname);
	strcat( name, &object_str[5]);
      }
      else if ( strncmp( object_str, "$PlcFo:", 7) == 0) {
	/* Replace "$PlcFo:" with fo name */
	s = strchr( object_str, '.');
	if ( !s)
	  strcpy( name, tractx->m_hostname);
	else {
	  strcpy( name, tractx->m_hostname);
	  strcat( name, s);
	}
	is_plcfo = true;
      }      
      else if ( strncmp( object_str, "$PlcMain:", 9) == 0) {
	/* Replace "$PlcMain:" with plcconnect name */
	s = strchr( object_str, '.');
	if ( !s)
	  strcpy( name, tractx->m_plcconnect);
	else {
	  strcpy( name, tractx->m_plcconnect);
	  strcat( name, s);
	}
	is_plcmain = true;
      }      
      else
	strcpy( name, object_str);
    }
    else
      strcpy( name, object_str);

    if ( strcmp( name, "") != 0) {
      strcpy( aname, name);
      strcat( aname, ".Description");

      sts = gdh_GetObjectInfo( aname, tiptext, sizeof(tiptext)); 
      if (EVEN(sts)) {
	// Try PlcConnect
	pwr_tAttrRef aref;

	strcpy( aname, name);
	strcat( aname, ".PlcConnect");
	
	sts = gdh_GetObjectInfo( aname, &aref, sizeof(aref)); 
	if ( ODD(sts))
	  sts = gdh_AttrrefToName( &aref, aname, sizeof(aname),
				   cdh_mName_volumeStrict);
	if ( ODD(sts)) {
	  strcat( aname, ".Description");
	
	  sts = gdh_GetObjectInfo( aname, tiptext, sizeof(tiptext)); 
	}
      }

      if ( is_plcfo) {
	if ( strcmp( tiptext, "") != 0)
	  strcat( tiptext, "\n");
	if (( s = strchr( name, ':')))
	  s++;
	else
	  s = name;
	strncat( tiptext, s, sizeof(tiptext)-strlen(tiptext));
	if ( strcmp( attr_str, "") != 0) {
	  strcat( tiptext, ".");
	  strncat( tiptext, attr_str, sizeof(tiptext)-strlen(tiptext));
	}
      }
      else if ( is_plcmain) {
	if ( strcmp( tiptext, "") != 0)
	  strcat( tiptext, "\n");
	if (( s = strchr( name, ':')))
	  s++;
	else
	  s = name;
	strncat( tiptext, s, sizeof(tiptext)-strlen(tiptext));
	if ( strcmp( attr_str, "") != 0) {
	  strcat( tiptext, ".");
	  strncat( tiptext, attr_str, sizeof(tiptext)-strlen(tiptext));
	}

	// Write channel for signals
	pwr_tAttrRef aref;
	pwr_tTid tid;

	sts = gdh_NameToAttrref( pwr_cNObjid, name, &aref);
	if (EVEN(sts)) break;

	sts = gdh_GetAttrRefTid( &aref, &tid);
	if (EVEN(sts)) break;

	switch ( tid) {
	case pwr_cClass_Di:
	case pwr_cClass_Ai:
	case pwr_cClass_Ii:
	case pwr_cClass_Do:
	case pwr_cClass_Ao:
	case pwr_cClass_Io:
	case pwr_cClass_Co: {
	  strcpy( aname, name);
	  strcat( aname, ".SigChanCon");

	  sts = gdh_GetObjectInfo( aname, &aref, sizeof(aref)); 
	  if ( ODD(sts)) {
	    sts = gdh_AttrrefToName( &aref, aname, sizeof(aname),
				     cdh_mName_pathStrict);
	    if ( ODD(sts)) {
	      strcat( tiptext, "\n");
	      strncat( tiptext, aname, sizeof(tiptext)-strlen(tiptext));
	    }
	  }

	  break;
	}
	default: ;
	}
      }

      if ( strcmp( tiptext, "") != 0)
	flow_SetTipText( ctx, event->object.object, tiptext, 
			 event->any.x_pixel, event->any.y_pixel);
    }
    break;
  }
  default:
    ;
  }
  return 1;
}

void RtTrace::trace_close_cb( RtTrace *child_tractx)
{
  RtTrace *tractx;
  trace_tNode *node, *fnode;

  tractx = (RtTrace *) child_tractx->parent_ctx;

  node = tractx->trace_list;
  if (node && node->tractx == child_tractx) {
    fnode = node;
    tractx->trace_list = node->Next;
    free((char *)fnode);
  }
  else {
    for (; node; node = node->Next) {
      if (node->Next && node->Next->tractx == child_tractx) {
        fnode = node->Next;
        node->Next = fnode->Next;
        free((char *)fnode);
        break;
      }
    }
  }

  delete child_tractx;
}

int RtTrace::get_objid( flow_tObject node, pwr_tObjid *oid)
{
  pwr_tOName	        name;
  pwr_tOName   		object_name;
  int			sts;
  flow_tTraceAttr      	attr_str;
  flow_eTraceType	trace_type;
  int			inverted;
  unsigned int 		options;

  /* Try flow node name */
  sts = gdh_ObjidToName( objid, name, sizeof(name), cdh_mNName); 
  if (EVEN(sts)) return 1;

  flow_GetObjectName( node, object_name);
  strcat( name, "-");
  strcat( name, object_name);

  sts = gdh_NameToObjid( name, oid);
  if ( EVEN(sts)) {
    /* Try trace object */
    get_trace_attr( node, object_name, attr_str, &trace_type, &inverted, &options);

    sts = gdh_NameToObjid( object_name, oid);
    if ( EVEN(sts)) return sts;
  }
  return 1;
}

int RtTrace::get_attrref( flow_tObject node, pwr_tAttrRef *aref)
{
  flow_tTraceObj   	object_str;
  flow_tTraceAttr  	attr_str;
  flow_eTraceType	trace_type;
  int		inverted;
  pwr_sAttrRef      attrref;
  int		sts;
  pwr_tAName  name;
  char        *s;
  unsigned int options;

  if ( flow_GetNodeGroup( node) == flow_eNodeGroup_Trace)
    return 0;

  get_trace_attr( node, object_str, attr_str,
		  &trace_type, &inverted, &options);

  if ( m_has_host) {
    if ( strncmp( object_str, "$host", 5) == 0) {
      /* Replace "$host" with hostname */
      strcpy( name, m_hostname);
      strcat( name, &object_str[5]);
    }
    else if ( strncmp( object_str, "$PlcFo:", 7) == 0) {
      /* Replace "$PlcFo:" with fo name */
      s = strchr( object_str, '.');
      if ( !s)
	strcpy( name, m_hostname);
      else {
	strcpy( name, m_hostname);
	strcat( name, s);
      }
    }      
    else if ( strncmp( object_str, "$PlcMain:", 9) == 0) {
      /* Replace "$PlcMain:" with plcconnect name */
      s = strchr( object_str, '.');
      if ( !s)
	strcpy( name, m_plcconnect);
      else {
	strcpy( name, m_plcconnect);
	strcat( name, s);
      }
    }      
    else
      strcpy( name, object_str);
  }
  else
    strcpy( name, object_str);

  if ( options & trace_mAttrOptions_MenuAttr) {
    if ( strcmp( attr_str, "") != 0) {
      strcat( name, ".");
      strcat( name, attr_str);
      
      sts = gdh_NameToAttrref( pwr_cNObjid, name, &attrref);
      if ( EVEN(sts)) return sts;
    }
    else {
      sts = gdh_NameToAttrref( pwr_cNObjid, name, &attrref);
      if ( EVEN(sts)) return sts;
    }
  }
  else {
    sts = gdh_NameToAttrref( pwr_cNObjid, name, &attrref);
    if ( EVEN(sts)) return sts;
  }
  *aref = attrref;
  return 1;
}

int RtTrace::get_selected_node( flow_tObject *node)
{
  flow_tNode	*list;
  int		count;

  flow_GetSelectList( flow_ctx, &list, &count);
  if ( count == 0)
    return 0;
  *node = *list;
    return 1;
}

pwr_tStatus RtTrace::viewsetup() 
{
  flow_tCtx ctx = flow_ctx;

  flow_DisableEventAll( ctx);

  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClickShift, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_ScrollDown, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_ScrollUp, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_TipText, flow_eEventType_CallBack, 
	flow_cb);

  return 1;
}

pwr_tStatus RtTrace::simsetup() 
{
  flow_tCtx ctx = flow_ctx;

  flow_DisableEventAll( ctx);
  flow_EnableEvent( ctx, flow_eEvent_MB1Press, flow_eEventType_MoveNode, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB2Press, flow_eEventType_CreateCon, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB2DoubleClick, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClickShift, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB3Press, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB3Down, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1ClickCtrl, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClickShiftCtrl, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_ScrollDown, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_ScrollUp, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_TipText, flow_eEventType_CallBack, 
	flow_cb);
  return 1;
}

pwr_tStatus RtTrace::trasetup()
{
  flow_tCtx ctx = flow_ctx;

  flow_DisableEventAll( ctx);
  flow_EnableEvent( ctx, flow_eEvent_MB1Press, flow_eEventType_MoveNode, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB2Press, flow_eEventType_CreateCon, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB2DoubleClick, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1DoubleClickShift, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB3Press, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1ClickCtrl, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_ScrollDown, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_ScrollUp, flow_eEventType_CallBack, 
	flow_cb);
  flow_EnableEvent( ctx, flow_eEvent_TipText, flow_eEventType_CallBack, 
	flow_cb);
  return 1;
}

int RtTrace::trace_start()
{
  int	  	sts;
  double 	f_width, f_height;

  if ( !trace_started)
  {

    flow_ResetHighlightAll( flow_ctx);
    flow_SelectClear( flow_ctx);
    sts = flow_TraceInit( flow_ctx, connect_bc, disconnect_bc, NULL);
    if ( EVEN(sts))
      return sts;
    trace_started = 1;

    trace_scan( this);

    /* Create node and con classes for trace */
    if ( !trace_analyse_nc) {
      f_width = 4.5 * GOEN_F_GRID;
      f_height = GOEN_F_GRID;
      flow_CreateNodeClass( flow_ctx, "TraceNode", flow_eNodeGroup_Trace, 
		&trace_analyse_nc);
      flow_AddRect( trace_analyse_nc, 0, 0, f_width, f_height, 
		flow_eDrawType_Line, 1, flow_mDisplayLevel_1);
      flow_AddAnnot( trace_analyse_nc, 0.07 * f_width, 0.75 *f_height, 0,
		flow_eDrawType_TextHelvetica, 3, flow_eAnnotType_OneLine,
		flow_mDisplayLevel_1);
      flow_AddConPoint( trace_analyse_nc, 0, 0.5*f_height, 0, 
		flow_eDirection_Left);
      flow_AddConPoint( trace_analyse_nc, f_width, 0.5*f_height, 1,
		flow_eDirection_Right);

      flow_CreateConClass( flow_ctx, "TraceCon", 
		flow_eConType_Straight, flow_eCorner_Right,
		flow_eDrawType_Line, 1, 0, 0, 0, flow_eConGroup_Trace,
		&trace_con_cc);	  
    }
  }
  return 1;
}

int RtTrace::trace_stop()
{

  if ( trace_started) {
    flow_TraceClose( flow_ctx);
    flow_ResetHighlightAll( flow_ctx);
    flow_SelectClear( flow_ctx);
    flow_RemoveTraceObjects( flow_ctx);
    trace_started = 0;
    trace_timerid->remove();
  }
  return 1;
}


void RtTrace::changevalue( flow_tNode fnode)
{
  pwr_tStatus 		sts;
  char		       	name[200];
  pwr_tBoolean		value;
  flow_tTraceObj      	object_str;
  flow_tTraceAttr      	attr_str;
  flow_eTraceType	trace_type;
  int			inverted;
  unsigned int 		options;

  if ( is_authorized_cb) {
    if ( !(is_authorized_cb)(parent_ctx, 
	      pwr_mAccess_RtWrite | pwr_mAccess_System))
      return;
  }

  if ( flow_GetNodeGroup( fnode) == flow_eNodeGroup_Trace) {
    trace_changenode = fnode;

    /* Get a value */
/*
    foe_get_textinput( tractx, "Enter value : ", &trace_aanalyse_set_value);
*/
    return;
  }
  else {	    
    /* Toggle the value, start to get the current value */
    get_trace_attr( fnode, object_str, attr_str, &trace_type, &inverted, &options);
    strcpy( name, object_str);
    strcat( name, ".");
    strcat( name, attr_str);
    switch ( trace_type) {
      case flow_eTraceType_Boolean:
        sts = gdh_GetObjectInfo( name, &value, sizeof(value)); 
        if (EVEN(sts)) {
          return;
        }

        /* Toggle the value */
        if ( value == 0)
          value = 1;
        else
          value = 0;

        sts = gdh_SetObjectInfo( name, &value, sizeof(value));
        if (EVEN(sts)) {
          return;
        }
        break;
      default:
        ;
    }
  }
}

/*************************************************************************
*
* Name:		trace_aanalyse_set_value ()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
**************************************************************************/

#if 0
static pwr_tStatus	trace_aanalyse_set_value(
    RtTrace	*tractx,
    char	*valuestr
)
{
  pwr_tStatus 		sts;
  char			name[200];
  pwr_tBoolean		boolean_value;
  pwr_tFloat32		float_value;
  flow_tTraceObj       	object_str;
  flow_tTraceAttr      	attr_str;
  flow_eTraceType	trace_type;
  int			inverted;

  flow_GetTraceAttr( tractx->trace_changenode, object_str, attr_str, &trace_type, &inverted);
  strcpy( name, object_str);
  strcat( name, ".");
  strcat( name, attr_str);
  switch ( trace_type)
  {
    case flow_eTraceType_Boolean:
      /* Convert to Boolean */
      if ( sscanf( valuestr, "%d", &boolean_value) != 1)
      {
        return 0;
      }

      sts = gdh_SetObjectInfo( name, &boolean_value, sizeof(boolean_value));
      if (EVEN(sts))
      {
        return 1;
       }
       break;
    case flow_eTraceType_Float32:
      /* Convert to float */
      if ( sscanf( valuestr, "%f", &float_value) != 1)
      {
        return 0;
      }

      sts = gdh_SetObjectInfo( name, &float_value, sizeof(float_value));
      if (EVEN(sts))
      {
        return 1;
       }
       break;
     default:
       ;
  }
  return 1;
}
#endif

char *RtTrace::id_to_str( pwr_tObjid objid)
{
  static char	str[80];
  unsigned char	volid[4];

  memcpy( &volid, &objid.vid, sizeof(volid));
  sprintf( str, "%3.3u_%3.3u_%3.3u_%3.3u_%8.8x",
	   volid[3], volid[2], volid[1], volid[0], objid.oix);
  return str;
}

RtTrace::~RtTrace()
{
}

int RtTrace::search_object( char *object_str)
{
  flow_tObject 	object;
  int		sts;

  sts = flow_FindByNameNoCase( flow_ctx, object_str, &object);
  if ( EVEN(sts))
     return sts;

  flow_CenterObject( flow_ctx, object);
  flow_SelectClear( flow_ctx);
  flow_SetInverse( object, 1);
  flow_SelectInsert( flow_ctx, object);
  return 1;
}

void RtTrace::swap( int mode)
{
  pwr_tStatus sts;

  if ( mode == 0) {
    if ( trace_started) {
      flow_TraceClose( flow_ctx);
      trace_timerid->remove();
    }
  }
  else {
    if ( trace_started) {
      int fversion = 0;
#if defined OS_LINUX
      {
	pwr_tTime time;

	sts = dcli_file_ctime( filename, &time);
	if ( ODD(sts))
	  fversion = time.tv_sec;
      }
#endif
      if ( version != fversion) {
	flow_sAttributes attr;
	char tfile[200];
	char *s;

	// Temporary file to store trace objects
	
	strcpy( tfile, "/tmp/");
	if ( (s = strrchr( filename, '/')))
	  strcat( tfile, s+1);
	else
	  strcat( tfile, filename);

	flow_GetAttributes( flow_ctx, &attr);
	flow_SaveTrace( flow_ctx, tfile);
	version = fversion;
	trace_started = 0;
	flow_SetNodraw( flow_ctx);
	flow_DeleteAll( flow_ctx);
	flow_Open( flow_ctx, filename);
	flow_SetAttributes( flow_ctx, &attr, ~0);
	flow_Zoom( flow_ctx, 1);
	flow_ResetNodraw( flow_ctx);
	flow_Redraw( flow_ctx);
	trace_start();
	flow_OpenTrace( flow_ctx, tfile);
      }
      else {
	sts = flow_TraceInit( flow_ctx, connect_bc, 
			      disconnect_bc, NULL);
	if ( EVEN(sts))
	  return;
	trace_scan( this);
      }
    }
  }
}

RtTrace::RtTrace( void *tr_parent_ctx, pwr_tObjid tr_objid,
		  pwr_tStatus *status) :
  parent_ctx(tr_parent_ctx), trace_analyse_nc(0),
  trace_con_cc(0), trace_started(0), trace_timerid(0), trace_changenode(0),
  objid(pwr_cNObjid), scan_time(0.5), close_cb(0), help_cb(0), subwindow_cb(0),
  display_object_cb(0), collect_insert_cb(0), is_authorized_cb(0), popup_menu_cb(0),
  call_method_cb(0), trace_list(0), version(0), m_has_host(0)
{
}
#endif
