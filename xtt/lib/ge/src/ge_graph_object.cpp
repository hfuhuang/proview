/* rt_ge.cpp -- Display plant and node hiererachy

   PROVIEW/R  
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "rt_gdh.h"
#include "rt_types.h"
#include "co_dcli.h"
#include "ge_msg.h"
}
 
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "glow.h"
#include "glow_growctx.h"
#include "glow_growapi.h"
#include "glow_growwidget.h"

#include "ge_graph.h"
#include "ge_attr.h"
#include "ge_dyn.h"

typedef struct {
	pwr_tClassId	classid;
    	int 		(*func)( Graph *, pwr_tObjid);
	} graph_sObjectFunction;

static int graph_attr_float32( Graph *graph, pwr_sAttrRef *attrref);
static int graph_attr_int32( Graph *graph, pwr_sAttrRef *attrref);
static int graph_attr_boolean( Graph *graph, pwr_sAttrRef *attrref);
static int graph_object_ix( Graph *graph, pwr_tObjid objid);
static int graph_object_ax( Graph *graph, pwr_tObjid objid);
static int graph_object_dx( Graph *graph, pwr_tObjid objid);
static int graph_object_chanxx( Graph *graph, pwr_tObjid objid);
static int graph_object_PID( Graph *graph, pwr_tObjid objid);
static int graph_object_Mode( Graph *graph, pwr_tObjid objid);
static int graph_object_PlcThread( Graph *graph, pwr_tObjid objid);
static int graph_object_collect( Graph *graph, pwr_tObjid objid);
static int graph_object_collect_build( Graph *graph, pwr_tObjid objid);

static graph_sObjectFunction graph_object_functions[] = {
	{ pwr_cClass_Av, &graph_object_ax},
	{ pwr_cClass_Ai, &graph_object_ax},
	{ pwr_cClass_Ao, &graph_object_ax},
	{ pwr_cClass_Iv, &graph_object_ix},
	{ pwr_cClass_Ii, &graph_object_ix},
	{ pwr_cClass_Io, &graph_object_ix},
	{ pwr_cClass_Dv, &graph_object_dx},
	{ pwr_cClass_Di, &graph_object_dx},
	{ pwr_cClass_Do, &graph_object_dx},
	{ pwr_cClass_ChanAi, &graph_object_chanxx},
	{ pwr_cClass_ChanAo, &graph_object_chanxx},
	{ pwr_cClass_ChanIi, &graph_object_chanxx},
	{ pwr_cClass_ChanIo, &graph_object_chanxx},
	{ pwr_cClass_ChanDi, &graph_object_chanxx},
	{ pwr_cClass_ChanDo, &graph_object_chanxx},
	{ pwr_cClass_pid, &graph_object_PID},
	{ pwr_cClass_mode, &graph_object_Mode},
	{ pwr_cClass_PlcThread, &graph_object_PlcThread},
	{ 0, 0}};

graph_sLocalDb *Graph::localdb_add( char *name, int type)
{
  graph_sLocalDb *item_p;

  item_p = (graph_sLocalDb *) calloc( 1, sizeof(graph_sLocalDb));
  strcpy( item_p->name, name);
  item_p->type = type;
  item_p->next = local_db;
  local_db = item_p;

  return item_p;
}

int Graph::localdb_find( char *name, graph_sLocalDb **item)
{
  graph_sLocalDb *item_p;

  for ( item_p = local_db; item_p; item_p = item_p->next)
  {
    if ( strcmp( item_p->name, name) == 0)
    {
      *item = item_p;
      return 1;
    }
  }
  return 0;
}

int Graph::localdb_set_value( char *name, void *value, int size)
{
  graph_sLocalDb *item_p;

  for ( item_p = local_db; item_p; item_p = item_p->next)
  {
    if ( strcmp( item_p->name, name) == 0)
    {
      memcpy( item_p->value, value, size);
      return 1;
    }
  }
  return 0;
}

void Graph::localdb_free()
{
  graph_sLocalDb *item_p, *next_item_p;

  for ( item_p = local_db; item_p;)
  {
    next_item_p = item_p->next;
    free( item_p);
    item_p = next_item_p;
  }
}

void *Graph::localdb_ref_or_create( char *name, int type)
{
  graph_sLocalDb *item_p;
  
  if ( localdb_find( name, &item_p))
    return (void *) item_p->value;

  item_p = localdb_add( name, type);
  return (void *) item_p->value;
}

int Graph::init_object_graph( int mode)
{
  char 		classname[120];
  pwr_tClassId 	classid;
  char		*s;
  int 		sts;
  pwr_tObjid	objid;
  int		i;
  int		is_type = 0;
  pwr_sAttrRef	attrref;

  if ( mode == 0)
  {
    if ( strcmp( filename, "_none_.pwg") == 0)
    {
      if ( strcmp( object_name, "collect") == 0)
      {
        sts = graph_object_collect_build( this, pwr_cNObjid);
        return sts;
      }
    }
    return 1;
  }

  if ( strcmp( filename, "_none_.pwg") == 0)
  {
    if ( strcmp( object_name, "collect") == 0)
    {
      sts = graph_object_collect( this, pwr_cNObjid);
      return sts;
    }
  }

  // Get class from filename
  if ( (s = strrchr( filename, '/')) ||
       (s = strrchr( filename, '>')) ||
       (s = strrchr( filename, ']')) ||
       (s = strrchr( filename, ':')))
  {
    if ( strncmp( s+1, "pwr_t_", 6) == 0)
    {
      is_type = 1;
      strcpy( classname, s+7);
    }
    else if ( strncmp( s+1, "pwr_c_", 6) == 0)
      strcpy( classname, s+7);
    else
      strcpy( classname, s+1);
  }
  else
    strcpy( classname, filename);
  if ( (s = strrchr( classname, '.')))
    *s = 0;

  if ( is_type)
  {
    sts = gdh_NameToAttrref( pwr_cNObjid, object_name, &attrref);
    if ( EVEN(sts)) return sts;

    if ( strcmp( classname, "float32") == 0) {
      sts = graph_attr_float32( this, &attrref);
      return sts;
    }
    if ( strcmp( classname, "int32") == 0) {
      sts = graph_attr_int32( this, &attrref);
      return sts;
    }
    else if ( strcmp( classname, "boolean") == 0) {
      sts = graph_attr_boolean( this, &attrref);
      return sts;
    }
    else
      return 0;
   
  }
  else
  {
    sts = gdh_ClassNameToId( classname, &classid);
    if ( EVEN(sts)) return sts;

    sts = gdh_NameToObjid( object_name, &objid);
    if ( EVEN(sts)) return sts;

    for ( i = 0; graph_object_functions[i].classid; i++)
    {
      if ( classid == graph_object_functions[i].classid)
      {
        sts = (graph_object_functions[i].func)( this, objid); 
        return sts;
      }
    }
  }
  return 0;
}

//
// Attribute graph for Float32
// 

typedef struct {
	graph_sObjectTrend 	td;
	} graph_sAttrFloat32;

static void graph_attr_float32_scan( Graph *graph)
{
  graph_sAttrFloat32 *od = (graph_sAttrFloat32 *)graph->graph_object_data;

  graph->trend_scan( &od->td);
}

static void graph_attr_float32_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_attr_float32( Graph *graph, pwr_sAttrRef *attrref)
{
  int sts;
  graph_sAttrFloat32 *od;

  od = (graph_sAttrFloat32 *) calloc( 1, sizeof(graph_sAttrFloat32));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_attr_float32_close;
  graph->graph_object_scan = graph_attr_float32_scan;

  sts = graph->trend_init( &od->td, pwr_cNObjid);

  return 1;
}

//
// Attribute graph for Int32
// 

typedef struct {
	graph_sObjectTrend 	td;
	} graph_sAttrInt32;

static void graph_attr_int32_scan( Graph *graph)
{
  graph_sAttrInt32 *od = (graph_sAttrInt32 *)graph->graph_object_data;

  graph->trend_scan( &od->td);
}

static void graph_attr_int32_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_attr_int32( Graph *graph, pwr_sAttrRef *attrref)
{
  int sts;
  graph_sAttrInt32 *od;

  od = (graph_sAttrInt32 *) calloc( 1, sizeof(graph_sAttrInt32));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_attr_int32_close;
  graph->graph_object_scan = graph_attr_int32_scan;

  sts = graph->trend_init( &od->td, pwr_cNObjid);

  return 1;
}

//
// Attribute graph for Boolean
// 

typedef struct {
	graph_sObjectTrend 	td;
	} graph_sAttrBoolean;

static void graph_attr_boolean_scan( Graph *graph)
{
  graph_sAttrBoolean *od = (graph_sAttrBoolean *)graph->graph_object_data;

  graph->trend_scan( &od->td);
}

static void graph_attr_boolean_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_attr_boolean( Graph *graph, pwr_sAttrRef *attrref)
{
  int sts;
  graph_sAttrBoolean *od;

  od = (graph_sAttrBoolean *) calloc( 1, sizeof(graph_sAttrBoolean));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_attr_boolean_close;
  graph->graph_object_scan = graph_attr_boolean_scan;

  sts = graph->trend_init( &od->td, pwr_cNObjid);
  return 1;
}

//
// Object graph for Ai, Ao and Av
// 

typedef struct {
	char			object_name[120];
	graph_sObjectTrend 	td;
	} graph_sObjectAx;

static void graph_object_ax_scan( Graph *graph)
{
  graph_sObjectAx *od = (graph_sObjectAx *)graph->graph_object_data;

  graph->trend_scan( &od->td);
}

static void graph_object_ax_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_object_ax( Graph *graph, pwr_tObjid objid)
{
  pwr_sAttrRef attrref;
  int sts;
  grow_tObject object;
  graph_sObjectAx *od;
  pwr_tClassId	classid;
  pwr_tObjid	sigchancon;
  pwr_tClassId	chan_classid;
  char		classname[40];
  char		chan_name[120];
  char		cmd[200];

  od = (graph_sObjectAx *) calloc( 1, sizeof(graph_sObjectAx));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_object_ax_close;

  sts = gdh_GetObjectClass( objid, &classid);
  if ( EVEN(sts)) return sts;

  // Display object name in item "ObjectName"
  sts = gdh_ObjidToName( objid, od->object_name, 
		sizeof(od->object_name), cdh_mNName);
  if ( EVEN(sts)) return sts;

  // Find field for object name
  sts = grow_FindObjectByName( graph->grow->ctx, "ObjectName", &object);
  if ( ODD(sts))
  {
    GeDyn *dyn;
    grow_GetUserData( object, (void **)&dyn);
    dyn->set_p( (void *) od->object_name);
  }

  sts = graph->trend_init( &od->td, objid);

  // Register scan function
  graph->graph_object_scan = graph_object_ax_scan;

  // Add command to open channel graph

  sts = gdh_ClassAttrToAttrref( classid, ".SigChanCon", &attrref);
  if ( ODD(sts))
  {
    attrref.Objid = objid;
    sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&sigchancon, sizeof(sigchancon));
    if ( EVEN(sts)) return sts;

    sts = gdh_ObjidToName( sigchancon, chan_name, sizeof(chan_name), 
		cdh_mNName);
    if ( ODD(sts))
    {
      sts = gdh_GetObjectClass( sigchancon, &chan_classid);
      if ( EVEN(sts)) return sts;
      sts = gdh_ObjidToName( cdh_ClassIdToObjid( chan_classid),
		  classname, sizeof(classname), cdh_mName_object);
      if ( EVEN(sts)) return sts;
      cdh_ToLower( classname, classname);

      sprintf( cmd, "ope gr pwr_c_%s/ins=%s/nam=\"%s\"", 
		classname, chan_name, chan_name);

      sts = graph->set_button_command( "OpenChannel", cmd);
    }
  }

  return 1;
}

//
// Object graph for Ii, Io and Iv
// 

typedef struct {
	char			object_name[120];
	graph_sObjectTrend 	td;
	} graph_sObjectIx;

static void graph_object_ix_scan( Graph *graph)
{
  graph_sObjectIx *od = (graph_sObjectIx *)graph->graph_object_data;

  graph->trend_scan( &od->td);
}

static void graph_object_ix_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_object_ix( Graph *graph, pwr_tObjid objid)
{
  pwr_sAttrRef attrref;
  int sts;
  grow_tObject object;
  graph_sObjectIx *od;
  pwr_tClassId	classid;
  pwr_tObjid	sigchancon;
  pwr_tClassId	chan_classid;
  char		classname[40];
  char		chan_name[120];
  char		cmd[200];

  od = (graph_sObjectIx *) calloc( 1, sizeof(graph_sObjectIx));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_object_ix_close;

  sts = gdh_GetObjectClass( objid, &classid);
  if ( EVEN(sts)) return sts;

  // Display object name in item "ObjectName"
  sts = gdh_ObjidToName( objid, od->object_name, 
		sizeof(od->object_name), cdh_mNName);
  if ( EVEN(sts)) return sts;

  // Find field for object name
  sts = grow_FindObjectByName( graph->grow->ctx, "ObjectName", &object);
  if ( ODD(sts))
  {
    GeDyn *dyn;
    grow_GetUserData( object, (void **)&dyn);
    dyn->set_p( (void *) od->object_name);
  }

  sts = graph->trend_init( &od->td, objid);

  // Register scan function
  graph->graph_object_scan = graph_object_ix_scan;

  // Add command to open channel graph

  sts = gdh_ClassAttrToAttrref( classid, ".SigChanCon", &attrref);
  if ( ODD(sts))
  {
    attrref.Objid = objid;
    sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&sigchancon, sizeof(sigchancon));
    if ( EVEN(sts)) return sts;

    sts = gdh_ObjidToName( sigchancon, chan_name, sizeof(chan_name), 
		cdh_mNName);
    if ( ODD(sts))
    {
      sts = gdh_GetObjectClass( sigchancon, &chan_classid);
      if ( EVEN(sts)) return sts;
      sts = gdh_ObjidToName( cdh_ClassIdToObjid( chan_classid),
		  classname, sizeof(classname), cdh_mName_object);
      if ( EVEN(sts)) return sts;
      cdh_ToLower( classname, classname);

      sprintf( cmd, "ope gr pwr_c_%s/ins=%s/nam=\"%s\"", 
		classname, chan_name, chan_name);

      sts = graph->set_button_command( "OpenChannel", cmd);
    }
  }

  return 1;
}
//
// Object graph for Di, Do and Dv
// 

typedef struct {
	char		object_name[120];
	graph_sObjectTrend 	td;
	} graph_sObjectDx;

static void graph_object_dx_scan( Graph *graph)
{
  graph_sObjectDx *od = (graph_sObjectDx *)graph->graph_object_data;

  graph->trend_scan( &od->td);
}


static void graph_object_dx_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_object_dx( Graph *graph, pwr_tObjid objid)
{
  int sts;
  grow_tObject object;
  graph_sObjectDx *od;
  pwr_tClassId classid;
  pwr_sAttrRef attrref;
  pwr_tObjid sigchancon;
  pwr_tClassId	chan_classid;
  char		classname[40];
  char		chan_name[120];
  char		cmd[200];

  od = (graph_sObjectDx *) calloc( 1, sizeof(graph_sObjectDx));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_object_dx_close;

  // Display object name in item "ObjectName"
  sts = gdh_ObjidToName( objid, od->object_name, 
		sizeof(od->object_name), cdh_mNName);
  if ( EVEN(sts)) return sts;

  sts = gdh_GetObjectClass( objid, &classid);
  if ( EVEN(sts)) return sts;

  sts = grow_FindObjectByName( graph->grow->ctx, "ObjectName", &object);
  if ( ODD(sts))
  {
    GeDyn *dyn;
    grow_GetUserData( object, (void **)&dyn);
    dyn->set_p( (void *) od->object_name);
  }

  sts = graph->trend_init( &od->td, objid);

  // Register scan function
  graph->graph_object_scan = graph_object_dx_scan;

  // Add command to open channel graph

  sts = gdh_ClassAttrToAttrref( classid, ".SigChanCon", &attrref);
  if ( ODD(sts))
  {
    attrref.Objid = objid;
    sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&sigchancon, sizeof(sigchancon));
    if ( EVEN(sts)) return sts;

    sts = gdh_ObjidToName( sigchancon, chan_name, sizeof(chan_name), 
		cdh_mNName);
    if ( ODD(sts))
    {
      sts = gdh_GetObjectClass( sigchancon, &chan_classid);
      if ( EVEN(sts)) return sts;
      sts = gdh_ObjidToName( cdh_ClassIdToObjid( chan_classid),
		  classname, sizeof(classname), cdh_mName_object);
      if ( EVEN(sts)) return sts;
      cdh_ToLower( classname, classname);

      sprintf( cmd, "ope gr pwr_c_%s/ins=%s/nam=\"%s\"", 
		classname, chan_name, chan_name);

      sts = graph->set_button_command( "OpenChannel", cmd);
    }
  }
  return 1;
}


//
// Graph for channel
//
static int graph_object_chanxx( Graph *graph, pwr_tObjid objid)
{
  pwr_sAttrRef attrref;
  int sts;
  pwr_tClassId	classid;
  pwr_tObjid	sigchancon;
  pwr_tClassId	signal_classid;
  char		classname[40];
  char		signal_name[120];
  char		cmd[200];

  sts = gdh_GetObjectClass( objid, &classid);
  if ( EVEN(sts)) return sts;

  // Add command to open signal graph

  sts = gdh_ClassAttrToAttrref( classid, ".SigChanCon", &attrref);
  if ( ODD(sts))
  {
    attrref.Objid = objid;
    sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&sigchancon, sizeof(sigchancon));
    if ( EVEN(sts)) return sts;

    sts = gdh_ObjidToName( sigchancon, signal_name, sizeof(signal_name), 
		cdh_mNName);
    if ( ODD(sts))
    {
      sts = gdh_GetObjectClass( sigchancon, &signal_classid);
      if ( EVEN(sts)) return sts;
      sts = gdh_ObjidToName( cdh_ClassIdToObjid( signal_classid),
		  classname, sizeof(classname), cdh_mName_object);
      if ( EVEN(sts)) return sts;
      cdh_ToLower( classname, classname);

      sprintf( cmd, "ope gr pwr_c_%s/ins=%s/nam=\"%s\"", 
		classname, signal_name, signal_name);

      sts = graph->set_button_command( "OpenSignal", cmd);
    }
  }

  return 1;
}

//
// Object graph for PID
//

typedef struct {
	pwr_tFloat32	*set_max_show_p;
	pwr_tFloat32	*set_min_show_p;
	pwr_tFloat32	set_max_show_old;
	pwr_tFloat32	set_min_show_old;
	pwr_tFloat32	*out_max_show_p;
	pwr_tFloat32	*out_min_show_p;
	pwr_tFloat32	out_max_show_old;
	pwr_tFloat32	out_min_show_old;
	grow_tObject 	set_bar_object;
	grow_tObject 	proc_bar_object;
	grow_tObject 	out_bar_object;
	grow_tObject 	set_trend_object;
	grow_tObject 	out_trend_object;
	grow_tObject 	hold_button_object;
	float		*scan_time_p;
	float		old_scan_time;
	double		*data_set_scan_time_p;
	double		*data_out_scan_time_p;
	int		*hold_button_p;
	int		*hold_set_p;
	int		*hold_out_p;
	char		pid_alg_str[40];
	} graph_sObjectPID;

static void graph_object_PID_scan( Graph *graph)
{
  graph_sObjectPID *od = (graph_sObjectPID *)graph->graph_object_data;

  // Reconfigure bar and trend if limits are changed
  if ( od->set_max_show_p && od->set_min_show_p && 
       ( *od->set_max_show_p != od->set_max_show_old ||
	 *od->set_min_show_p != od->set_min_show_old))
  {
    if ( *od->set_max_show_p != *od->set_min_show_p)
    {
      grow_SetBarRange( od->set_bar_object, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
      grow_SetBarRange( od->proc_bar_object, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
      grow_SetTrendRange( od->set_trend_object, 0, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
      grow_SetTrendRange( od->set_trend_object, 1, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
    }
    od->set_min_show_old = *od->set_min_show_p;
    od->set_max_show_old = *od->set_max_show_p;
  }      

  if ( od->out_max_show_p && od->out_min_show_p && 
       ( *od->out_max_show_p != od->out_max_show_old ||
	 *od->out_min_show_p != od->out_min_show_old))
  {
    if ( *od->out_max_show_p != *od->out_min_show_p)
    {
      grow_SetBarRange( od->out_bar_object, double(*od->out_min_show_p), 
		double(*od->out_max_show_p));
      grow_SetTrendRange( od->out_trend_object, 0, double(*od->out_min_show_p), 
		double(*od->out_max_show_p));
    }
    od->out_min_show_old = *od->out_min_show_p;
    od->out_max_show_old = *od->out_max_show_p;
  }      

  // Reconfigure new scantime
  if ( od->scan_time_p && 
       *od->scan_time_p != od->old_scan_time)
  {
    if ( graph->scan_time > *od->scan_time_p/200)
    {
      graph->scan_time = *od->scan_time_p/200;
      graph->animation_scan_time = *od->scan_time_p/200;
    }
    grow_SetTrendScanTime( od->set_trend_object, double( *od->scan_time_p/200));
    grow_SetTrendScanTime( od->out_trend_object, double( *od->scan_time_p/200));
    od->old_scan_time = *od->scan_time_p;
    *od->data_set_scan_time_p = double(*od->scan_time_p)/200;
    *od->data_out_scan_time_p = double(*od->scan_time_p)/200;
  }

  if ( od->hold_button_p && *od->hold_button_p)
  {
    *od->hold_set_p = !*od->hold_set_p;
    *od->hold_out_p = !*od->hold_out_p;
    *od->hold_button_p = 0;
    if ( *od->hold_set_p && od->hold_button_object)
      grow_SetObjectColorTone( od->hold_button_object, glow_eDrawTone_Yellow);
    else
      grow_ResetObjectColorTone( od->hold_button_object);
  }

}

static void graph_object_PID_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_object_PID( Graph *graph, pwr_tObjid objid)
{
  pwr_sAttrRef attrref;
  int sts;
  grow_tObject object;
  graph_sObjectPID *od;
  pwr_tClassId	classid;
  pwr_tObjid	mode_objid;
  pwr_tClassId	mode_classid;
  char		classname[40];
  char		mode_name[120];
  char		cmd[200];
  pwr_tFloat32 max_limit = 100;
  pwr_tFloat32 min_limit = 0;
  double scan_time;
  unsigned int pid_alg;

  od = (graph_sObjectPID *) calloc( 1, sizeof(graph_sObjectPID));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_object_PID_close;

  sts = gdh_GetObjectClass( objid, &classid);
  if ( EVEN(sts)) return sts;

  // Get values for SetMinShow and SetMaxShow
  sts = gdh_ClassAttrToAttrref( classid, ".SetMaxShow", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&max_limit, sizeof(max_limit));
  if ( EVEN(sts)) return sts;

  sts = gdh_ClassAttrToAttrref( classid, ".SetMinShow", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&min_limit, sizeof(min_limit));
  if ( EVEN(sts)) return sts;

  od->set_max_show_old = max_limit;
  od->set_min_show_old = min_limit;

  // Configure ProcVal and SetVal bar
  sts = grow_FindObjectByName( graph->grow->ctx, "SetValBar", 
		&od->set_bar_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetBarRange( od->set_bar_object, double(min_limit), double(max_limit));

  sts = grow_FindObjectByName( graph->grow->ctx, "ProcValBar", 
		&od->proc_bar_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetBarRange( od->proc_bar_object, double(min_limit), double(max_limit));

  // Get pointers to max and min value
  sts = grow_FindObjectByName( graph->grow->ctx, "SetMaxShow", &object);
  if ( EVEN(sts)) return sts;

  GeDyn *dyn;
  grow_GetUserData( object, (void **)&dyn);
  od->set_max_show_p = (pwr_tFloat32 *) dyn->get_p();

  sts = grow_FindObjectByName( graph->grow->ctx, "SetMinShow", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->set_min_show_p = (pwr_tFloat32 *) dyn->get_p();

  // Configure SetVal and ProcVal trend
  sts = grow_FindObjectByName( graph->grow->ctx, "SetValTrend", 
		&od->set_trend_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
  {
    grow_SetTrendRange( od->set_trend_object, 0, double(min_limit), double(max_limit));
    grow_SetTrendRange( od->set_trend_object, 1, double(min_limit), double(max_limit));
  }

  // Get values for OutMinShow and OutMaxShow
  sts = gdh_ClassAttrToAttrref( classid, ".OutMaxShow", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&max_limit, sizeof(max_limit));
  if ( EVEN(sts)) return sts;

  sts = gdh_ClassAttrToAttrref( classid, ".OutMinShow", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&min_limit, sizeof(min_limit));
  if ( EVEN(sts)) return sts;

  od->out_max_show_old = max_limit;
  od->out_min_show_old = min_limit;

  // Configure OutVal bar
  sts = grow_FindObjectByName( graph->grow->ctx, "OutValBar", 
		&od->out_bar_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetBarRange( od->out_bar_object, double(min_limit), double(max_limit));

  // Get pointers to max and min value
  sts = grow_FindObjectByName( graph->grow->ctx, "OutMaxShow", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->out_max_show_p = (pwr_tFloat32 *) dyn->get_p();

  sts = grow_FindObjectByName( graph->grow->ctx, "OutMinShow", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->out_min_show_p = (pwr_tFloat32 *) dyn->get_p();

  // Configure OutVal trend
  sts = grow_FindObjectByName( graph->grow->ctx, "OutValTrend", 
		&od->out_trend_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetTrendRange( od->out_trend_object, 0, double(min_limit), double(max_limit));

  // Set scantime variable in local database
  grow_GetTrendScanTime( od->set_trend_object, &scan_time);
  od->scan_time_p = (float *) graph->localdb_ref_or_create( "ScanTime", 
		pwr_eType_Float32);
  od->old_scan_time = float( scan_time*200);
  *od->scan_time_p = od->old_scan_time;

  // Get Hold button
  sts = grow_FindObjectByName( graph->grow->ctx, "TrendHold", 
		&od->hold_button_object);
  if ( ODD(sts))
    od->hold_button_p = (int *) graph->localdb_ref_or_create( "TrendHold", 
		pwr_eType_Boolean);

  grow_GetUserData( od->set_trend_object, (void **)&dyn);
  od->data_set_scan_time_p = dyn->ref_trend_scantime();
  od->hold_set_p = dyn->ref_trend_hold();

  grow_GetUserData( od->out_trend_object, (void **)&dyn);
  od->data_out_scan_time_p = dyn->ref_trend_scantime();
  od->hold_out_p = dyn->ref_trend_hold();

  // Get and convert PidAlg
  sts = gdh_ClassAttrToAttrref( classid, ".PidAlg", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&pid_alg, sizeof(pid_alg));
  if ( EVEN(sts)) return sts;

  sts =  types_translate_enum( classid, "PidAlg", pid_alg, od->pid_alg_str);

  sts = grow_FindObjectByName( graph->grow->ctx, "PidAlg", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  dyn->set_p( (void *) od->pid_alg_str);

  // Register scan function
  graph->graph_object_scan = graph_object_PID_scan;

  // Add command to open mode graph

  sts = gdh_ClassAttrToAttrref( classid, ".ModeObjDid", &attrref);
  if ( ODD(sts))
  {
    attrref.Objid = objid;
    sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&mode_objid, sizeof(mode_objid));
    if ( EVEN(sts)) return sts;

    sts = gdh_ObjidToName( mode_objid, mode_name, sizeof(mode_name), 
		cdh_mNName);
    if ( ODD(sts))
    {
      sts = gdh_GetObjectClass( mode_objid, &mode_classid);
      if ( EVEN(sts)) return sts;
      sts = gdh_ObjidToName( cdh_ClassIdToObjid( mode_classid),
		  classname, sizeof(classname), cdh_mName_object);
      if ( EVEN(sts)) return sts;
      cdh_ToLower( classname, classname);

      sprintf( cmd, "ope gr pwr_c_%s/ins=%s/nam=\"%s\"", 
		classname, mode_name, mode_name);

      sts = graph->set_button_command( "OpenMode", cmd);
    }
  }

  return 1;
}

//
// Object graph for PID
//

typedef struct {
	char		object_name[120];
	pwr_tFloat32	*set_max_show_p;
	pwr_tFloat32	*set_min_show_p;
	pwr_tFloat32	set_max_show_old;
	pwr_tFloat32	set_min_show_old;
	pwr_tFloat32	*out_max_show_p;
	pwr_tFloat32	*out_min_show_p;
	pwr_tFloat32	out_max_show_old;
	pwr_tFloat32	out_min_show_old;
	grow_tObject 	set_bar_object;
	grow_tObject 	proc_bar_object;
	grow_tObject 	out_bar_object;
	grow_tObject 	set_slider_object;
	grow_tObject 	out_slider_object;
	grow_tObject 	set_slider_button_object;
	grow_tObject 	out_slider_button_object;
	int		*set_slider_button_p;
	int		*set_slider_disable_p;
	int		*out_slider_button_p;
	int		*out_slider_disable_p;
	int		*man_mode_button_p;
	int		*auto_mode_button_p;
	int		*cascade_mode_button_p;
	int		man_mode_old;
	int		auto_mode_old;
	int		cascade_mode_old;
	pwr_tUInt32	*auto_mode_p;
	pwr_tUInt32	*cascade_mode_p;
	int		man_mode;
        pwr_tInt32      acc_mod;
	} graph_sObjectMode;

	
static void graph_object_Mode_scan( Graph *graph)
{
  graph_sObjectMode *od = (graph_sObjectMode *)graph->graph_object_data;
  int value;
  int sts;
  char attr[120];

  // Calulate value for man mode indicator
  od->man_mode = !( *od->auto_mode_p || *od->cascade_mode_p); 

  // Reconfigure bar and trend if limits are changed
  if ( od->set_max_show_p && od->set_min_show_p && 
       ( *od->set_max_show_p != od->set_max_show_old ||
	 *od->set_min_show_p != od->set_min_show_old))
  {
    if ( *od->set_max_show_p != *od->set_min_show_p)
    {
      grow_SetBarRange( od->set_bar_object, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
      grow_SetBarRange( od->proc_bar_object, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
      if ( od->set_slider_object)
      {
        GeDyn *dyn;
        grow_SetSliderRange( od->set_slider_object, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
        grow_GetUserData( od->set_slider_object, (void **)&dyn);
        dyn->update();
      }
    }
    od->set_min_show_old = *od->set_min_show_p;
    od->set_max_show_old = *od->set_max_show_p;
  }      

  if ( od->out_max_show_p && od->out_min_show_p && 
       ( *od->out_max_show_p != od->out_max_show_old ||
	 *od->out_min_show_p != od->out_min_show_old))
  {
    if ( *od->out_max_show_p != *od->out_min_show_p)
    {
      grow_SetBarRange( od->out_bar_object, double(*od->out_min_show_p), 
		double(*od->out_max_show_p));
      if ( od->out_slider_object)
      {
        GeDyn *dyn;
        grow_SetSliderRange( od->out_slider_object, double(*od->out_min_show_p), 
		double(*od->out_max_show_p));
        grow_GetUserData( od->out_slider_object, (void **)&dyn);
        dyn->update();
      }
    }
    od->out_min_show_old = *od->out_min_show_p;
    od->out_max_show_old = *od->out_max_show_p;
  }      

  if ( od->set_slider_button_p && *od->set_slider_button_p)
  {
    *od->set_slider_disable_p = !*od->set_slider_disable_p;
    *od->set_slider_button_p = 0;
    if ( !*od->set_slider_disable_p && od->set_slider_button_object)
      grow_SetObjectColorTone( od->set_slider_button_object, glow_eDrawTone_Yellow);
    else
      grow_ResetObjectColorTone( od->set_slider_button_object);
  }
  if ( od->out_slider_button_p && *od->out_slider_button_p)
  {
    *od->out_slider_disable_p = !*od->out_slider_disable_p;
    *od->out_slider_button_p = 0;
    if ( !*od->out_slider_disable_p && od->out_slider_button_object)
      grow_SetObjectColorTone( od->out_slider_button_object, glow_eDrawTone_Yellow);
    else
      grow_ResetObjectColorTone( od->out_slider_button_object);
  }

  if ( od->man_mode_button_p && *od->man_mode_button_p)
  {
    *od->man_mode_button_p = 0;

    if ( od->acc_mod & 1) {
      strcpy( attr, od->object_name);
      strcat( attr, ".OpMod");
      value = 1;
      sts = gdh_SetObjectInfo( attr, &value, sizeof( value));
      if ( EVEN(sts)) printf( "Set Mode error\n");
    }
  }
  else if ( od->auto_mode_button_p && *od->auto_mode_button_p)
  {
    *od->auto_mode_button_p = 0;
    if ( od->acc_mod & 2) {
      strcpy( attr, od->object_name);
      strcat( attr, ".OpMod");
      value = 2;
      sts = gdh_SetObjectInfo( attr, &value, sizeof( value));
      if ( EVEN(sts)) printf( "Set Mode error\n");
    }
  }
  else if ( od->cascade_mode_button_p && *od->cascade_mode_button_p)
  {
    *od->cascade_mode_button_p = 0;
    if ( od->acc_mod & 4) {
      strcpy( attr, od->object_name);
      strcat( attr, ".OpMod");
      value = 4;
      sts = gdh_SetObjectInfo( attr, &value, sizeof( value));
      if ( EVEN(sts)) printf( "Set Mode error\n");
    }
  }
}

static void graph_object_Mode_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_object_Mode( Graph *graph, pwr_tObjid objid)
{
  pwr_sAttrRef attrref;
  int sts;
  grow_tObject object;
  graph_sObjectMode *od;
  pwr_tClassId	classid;
  pwr_tObjid	pid_objid;
  pwr_tClassId	pid_classid;
  char		classname[40];
  char		pid_name[120];
  char		cmd[200];
  pwr_tFloat32 max_limit = 100;
  pwr_tFloat32 min_limit = 0;

  od = (graph_sObjectMode *) calloc( 1, sizeof(graph_sObjectMode));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_object_Mode_close;

  sts = gdh_ObjidToName( objid, od->object_name, sizeof(od->object_name), 
		cdh_mNName);
  if ( EVEN(sts)) return sts;

  sts = gdh_GetObjectClass( objid, &classid);
  if ( EVEN(sts)) return sts;

  // Get values for AccMod
  sts = gdh_ClassAttrToAttrref( classid, ".AccMod", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&od->acc_mod, sizeof(od->acc_mod));
  if ( EVEN(sts)) return sts;

  // Get values for SetMinShow and SetMaxShow
  sts = gdh_ClassAttrToAttrref( classid, ".SetMaxShow", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&max_limit, sizeof(max_limit));
  if ( EVEN(sts)) return sts;

  sts = gdh_ClassAttrToAttrref( classid, ".SetMinShow", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&min_limit, sizeof(min_limit));
  if ( EVEN(sts)) return sts;

  od->set_max_show_old = max_limit;
  od->set_min_show_old = min_limit;

  // Configure ProcVal and SetVal bar
  sts = grow_FindObjectByName( graph->grow->ctx, "SetValBar", 
		&od->set_bar_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetBarRange( od->set_bar_object, double(min_limit), double(max_limit));

  sts = grow_FindObjectByName( graph->grow->ctx, "ProcValBar", 
		&od->proc_bar_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetBarRange( od->proc_bar_object, double(min_limit), double(max_limit));

  // Configure set slider
  sts = grow_FindObjectByName( graph->grow->ctx, "SetValSlider", 
		&od->set_slider_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetSliderRange( od->set_slider_object, double(min_limit), double(max_limit));

  GeDyn *dyn;
  grow_GetUserData( od->set_slider_object, (void **)&dyn);
  od->set_slider_disable_p = dyn->ref_slider_disabled();
  if ( od->set_slider_disable_p)
    *od->set_slider_disable_p = 1;

  // Get pointers to max and min value
  sts = grow_FindObjectByName( graph->grow->ctx, "SetMaxShow", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->set_max_show_p = (pwr_tFloat32 *) dyn->get_p();

  sts = grow_FindObjectByName( graph->grow->ctx, "SetMinShow", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->set_min_show_p = (pwr_tFloat32 *) dyn->get_p();

  // Get values for OutMinShow and OutMaxShow
  sts = gdh_ClassAttrToAttrref( classid, ".OutMaxShow", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&max_limit, sizeof(max_limit));
  if ( EVEN(sts)) return sts;

  sts = gdh_ClassAttrToAttrref( classid, ".OutMinShow", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&min_limit, sizeof(min_limit));
  if ( EVEN(sts)) return sts;

  od->out_max_show_old = max_limit;
  od->out_min_show_old = min_limit;

  // Configure OutVal bar
  sts = grow_FindObjectByName( graph->grow->ctx, "OutValBar", 
		&od->out_bar_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetBarRange( od->out_bar_object, double(min_limit), double(max_limit));

  // Get pointers to max and min value
  sts = grow_FindObjectByName( graph->grow->ctx, "OutMaxShow", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->out_max_show_p = (pwr_tFloat32 *) dyn->get_p();

  sts = grow_FindObjectByName( graph->grow->ctx, "OutMinShow", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->out_min_show_p = (pwr_tFloat32 *) dyn->get_p();

  // Configure out slider
  sts = grow_FindObjectByName( graph->grow->ctx, "OutValSlider", 
		&od->out_slider_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetSliderRange( od->out_slider_object, double(min_limit), double(max_limit));

  grow_GetUserData( od->out_slider_object, (void **)&dyn);
  od->out_slider_disable_p = dyn->ref_slider_disabled();
  if ( od->out_slider_disable_p)
    *od->out_slider_disable_p = 1;

  // Slider disable buttons  
  sts = grow_FindObjectByName( graph->grow->ctx, "SetSliderDisable", 
		&od->set_slider_button_object);
  if ( EVEN(sts)) return sts;
  
  od->set_slider_button_p = (int *) graph->localdb_ref_or_create( "SetSliderDisable", 
		pwr_eType_Boolean);

  sts = grow_FindObjectByName( graph->grow->ctx, "OutSliderDisable", 
		&od->out_slider_button_object);
  if ( EVEN(sts)) return sts;
  
  od->out_slider_button_p = (int *) graph->localdb_ref_or_create( "OutSliderDisable", 
		pwr_eType_Boolean);

  // Find values for Mode buttons
  od->man_mode_button_p = (int *) graph->localdb_ref_or_create( "ManMode", 
		pwr_eType_Boolean);
  od->auto_mode_button_p = (int *) graph->localdb_ref_or_create( "AutoMode", 
		pwr_eType_Boolean);
  od->cascade_mode_button_p = (int *) graph->localdb_ref_or_create( "CascadeMode", 
		pwr_eType_Boolean);

  // Get pointers to AutMode and CascMod for ManMode calculation
  sts = grow_FindObjectByName( graph->grow->ctx, "AutoInd", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->auto_mode_p = (pwr_tUInt32 *) dyn->get_p();

  sts = grow_FindObjectByName( graph->grow->ctx, "CascadeInd", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  od->cascade_mode_p = (pwr_tUInt32 *) dyn->get_p();

  sts = grow_FindObjectByName( graph->grow->ctx, "ManInd", &object);
  if ( EVEN(sts)) return sts;

  grow_GetUserData( object, (void **)&dyn);
  dyn->set_p( (void *) &od->man_mode);

  // Register scan function
  graph->graph_object_scan = graph_object_Mode_scan;

  // Add command to open PID graph
  sts = gdh_ClassAttrToAttrref( classid, ".PidObjDid", &attrref);
  if ( ODD(sts))
  {
    attrref.Objid = objid;
    sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&pid_objid, sizeof(pid_objid));
    if ( EVEN(sts)) return sts;

    sts = gdh_ObjidToName( pid_objid, pid_name, sizeof(pid_name), 
		cdh_mNName);
    if ( ODD(sts))
    {
      sts = gdh_GetObjectClass( pid_objid, &pid_classid);
      if ( EVEN(sts)) return sts;
      sts = gdh_ObjidToName( cdh_ClassIdToObjid( pid_classid),
		  classname, sizeof(classname), cdh_mName_object);
      if ( EVEN(sts)) return sts;
      cdh_ToLower( classname, classname);

      sprintf( cmd, "ope gr pwr_c_%s/ins=%s/nam=\"%s\"", 
		classname, pid_name, pid_name);

      sts = graph->set_button_command( "OpenPID", cmd);
    }
  }

  return 1;
}

int Graph::set_button_command( char *button_name, char *cmd)
{
  int 			sts;
  grow_tObject 		object;

  // Add command to open channel graph
  sts = grow_FindObjectByName( grow->ctx, button_name, 
		&object);
  if ( EVEN(sts)) return sts;

  GeDyn *dyn;
  grow_GetUserData( object, (void **)&dyn);
  dyn->set_command( cmd);

  return 1;
}

void Graph::trend_scan( graph_sObjectTrend *td)
{
  // Reconfigure bar and trend if limits are changed
  if ( td->pres_max_limit_p && td->pres_min_limit_p && 
       ( *td->pres_max_limit_p != td->pres_max_limit_old ||
	 *td->pres_min_limit_p != td->pres_min_limit_old))
  {
    if ( *td->pres_max_limit_p != *td->pres_min_limit_p)
    {
      if ( td->bar_object)
        grow_SetBarRange( td->bar_object, double(*td->pres_min_limit_p), 
		double(*td->pres_max_limit_p));
      if ( td->slider_object)
      {
        GeDyn *dyn;
        grow_SetSliderRange( td->slider_object, double(*td->pres_min_limit_p), 
		double(*td->pres_max_limit_p));
        grow_GetUserData( td->slider_object, (void **)&dyn);
        dyn->update();
      }
      grow_SetTrendRange( td->trend_object, 0, double(*td->pres_min_limit_p), 
		double(*td->pres_max_limit_p));
    }
    td->pres_min_limit_old = *td->pres_min_limit_p;
    td->pres_max_limit_old = *td->pres_max_limit_p;
  }      

  // Reconfigure new scantime
  if ( td->scan_time_p && 
       *td->scan_time_p != td->old_scan_time)
  {
    if ( scan_time > *td->scan_time_p/200)
    {
      scan_time = *td->scan_time_p/200;
      animation_scan_time = *td->scan_time_p/200;
    }
    grow_SetTrendScanTime( td->trend_object, double( *td->scan_time_p/200));
    td->old_scan_time = *td->scan_time_p;
    *td->data_scan_time_p = double(*td->scan_time_p)/200;
  }

  if ( td->hold_button_p && *td->hold_button_p)
  {
    *td->hold_p = !*td->hold_p;
    *td->hold_button_p = 0;
    if ( *td->hold_p && td->hold_button_object)
    {
      grow_SetObjectColorTone( td->hold_button_object, glow_eDrawTone_Yellow);
//      grow_SetAnnotation( td->hold_button_object, 1, "   Go", 6);
     }
    else
    {
      grow_ResetObjectColorTone( td->hold_button_object);
//      grow_SetAnnotation( td->hold_button_object, 1, "   Hold", 8);
   }
  }

  if ( td->slider_button_p && *td->slider_button_p)
  {
    *td->slider_disable_p = !*td->slider_disable_p;
    *td->slider_button_p = 0;
    if ( !*td->slider_disable_p && td->slider_button_object)
      grow_SetObjectColorTone( td->slider_button_object, glow_eDrawTone_Yellow);
    else
      grow_ResetObjectColorTone( td->slider_button_object);
  }
}

int Graph::trend_init( graph_sObjectTrend *td, pwr_tObjid objid)
{
  pwr_tClassId classid;
  pwr_tFloat32 max_limit = 100;
  pwr_tFloat32 min_limit = 0;
  int sts;
  pwr_sAttrRef attrref;
  grow_tObject object;
  int presminlimit_found = 0;
  int presmaxlimit_found = 0;
  double scan_time;

  if ( ! cdh_ObjidIsNull( objid))
  {
    sts = gdh_GetObjectClass( objid, &classid);
    if ( EVEN(sts)) return sts;

    // Try to find attributes PresMaxLimit and PresMinLimit
    sts = gdh_ClassAttrToAttrref( classid, ".PresMaxLimit", &attrref);
    if ( ODD(sts))
    {
      attrref.Objid = objid;
      sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&max_limit, sizeof(max_limit));
      if ( EVEN(sts)) return sts;
      presminlimit_found = 1;
    }
    sts = gdh_ClassAttrToAttrref( classid, ".PresMinLimit", &attrref);
    if ( ODD(sts))
    {
      attrref.Objid = objid;
      sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&min_limit, sizeof(min_limit));
      if ( EVEN(sts)) return sts;
      presmaxlimit_found = 1;
    }
  }
  td->pres_max_limit_old = max_limit;
  td->pres_min_limit_old = min_limit;

  // Configure bar
  sts = grow_FindObjectByName( grow->ctx, "ActualValueBar", 
		&td->bar_object);
  if ( ODD(sts))
  {
    if ( min_limit != max_limit)
      grow_SetBarRange( td->bar_object, double(min_limit), double(max_limit));
  }

  // Get pointers to max and min value
  sts = grow_FindObjectByName( grow->ctx, "PresMaxLimit", &object);
  if ( ODD(sts))
  {
    GeDyn *dyn;
    grow_GetUserData( object, (void **)&dyn);
    td->pres_max_limit_p = (pwr_tFloat32 *) dyn->get_p();
    if ( !presmaxlimit_found)
      // PresMaxLimit in local database, set value
      *td->pres_max_limit_p = max_limit;
  }

  sts = grow_FindObjectByName( grow->ctx, "PresMinLimit", &object);
  if ( ODD(sts))
  {
    GeDyn *dyn;
    grow_GetUserData( object, (void **)&dyn);
    td->pres_min_limit_p = (pwr_tFloat32 *) dyn->get_p();
    if ( !presminlimit_found)
      // PresMinLimit in local database, set value
      *td->pres_min_limit_p = min_limit;
  }

  // Configure trend
  sts = grow_FindObjectByName( grow->ctx, "ActualValueTrend", 
		&td->trend_object);
  if ( EVEN(sts)) return sts;

  if ( td->pres_min_limit_p && td->pres_max_limit_p)
  {
    if ( min_limit != max_limit)
      grow_SetTrendRange( td->trend_object, 0, double(min_limit), double(max_limit));
  }

  // Configure slider
  sts = grow_FindObjectByName( grow->ctx, "ActualValueSlider", 
		&td->slider_object);
  if ( ODD(sts))
  {
    if ( td->pres_min_limit_p && td->pres_max_limit_p)
    {
      if ( min_limit != max_limit)
        grow_SetSliderRange( td->slider_object, double(min_limit), double(max_limit));
    }
    GeDyn *dyn;
    grow_GetUserData( td->slider_object, (void **)&dyn);
    td->slider_disable_p = dyn->ref_slider_disabled();
    if ( td->slider_disable_p)
      *td->slider_disable_p = 1;
  }

  // Set scantime variable in local database
  grow_GetTrendScanTime( td->trend_object, &scan_time);
  td->scan_time_p = (float *) localdb_ref_or_create( "ScanTime", 
		pwr_eType_Float32);
  td->old_scan_time = float( scan_time*200);
  *td->scan_time_p = td->old_scan_time;

  // Get Hold button
  sts = grow_FindObjectByName( grow->ctx, "TrendHold", 
		&td->hold_button_object);
  if ( ODD(sts))
    td->hold_button_p = (int *) localdb_ref_or_create( "TrendHold", 
		pwr_eType_Boolean);

  GeDyn *dyn;
  grow_GetUserData( td->trend_object, (void **)&dyn);
  td->data_scan_time_p = dyn->ref_trend_scantime();
  td->hold_p = dyn->ref_trend_hold();

  sts = grow_FindObjectByName( grow->ctx, "SliderDisable", 
		&td->slider_button_object);
  if ( ODD(sts))
    td->slider_button_p = (int *) localdb_ref_or_create( "SliderDisable", 
		pwr_eType_Boolean);

  return 1;
}
//
// Object graph for collect
// 

#define MAX_TREND_OBJECTS 100

typedef struct {
	char		object_name[120];
	graph_sObjectTrend 	trend1;
        grow_tObject trend_objects[MAX_TREND_OBJECTS];
        double       *data_scan_time_p[MAX_TREND_OBJECTS];
        int          *hold_p[100];
        int          trend_cnt;
	} graph_sObjectCollect;

static void graph_object_collect_scan( Graph *graph)
{
  graph_sObjectCollect *od = (graph_sObjectCollect *)graph->graph_object_data;
  graph_sObjectTrend *td = &od->trend1;
  int i;

  // Reconfigure new scantime
  if ( td->scan_time_p && 
       *td->scan_time_p != td->old_scan_time)
  {
    if ( graph->scan_time > *td->scan_time_p/200)
    {
      graph->scan_time = *td->scan_time_p/200;
      graph->animation_scan_time = *td->scan_time_p/200;
    }
    for ( i = 0; i < od->trend_cnt; i++) {
      grow_SetTrendScanTime( od->trend_objects[i], double( *td->scan_time_p/200));
      *od->data_scan_time_p[i] = double(*td->scan_time_p)/200;
    }
    td->old_scan_time = *td->scan_time_p;
  }

  if ( td->hold_button_p && *td->hold_button_p)
  {
    for ( i = 0; i < od->trend_cnt; i++)
      *od->hold_p[i] = !*od->hold_p[i];
    *td->hold_button_p = 0;
    if ( *td->hold_p && td->hold_button_object)
      grow_SetObjectColorTone( td->hold_button_object, glow_eDrawTone_Yellow);
    else
      grow_ResetObjectColorTone( td->hold_button_object);
  }
}


static void graph_object_collect_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_object_collect( Graph *graph, pwr_tObjid objid)
{
  int sts;
  graph_sObjectCollect *od = (graph_sObjectCollect *)graph->graph_object_data;
  int i;

  if ( !od)
    return 1;

  sts = graph->trend_init( &od->trend1, pwr_cNObjid);

  for ( i = 0; i < od->trend_cnt; i++) {
    GeDyn *dyn;
    grow_GetUserData( od->trend_objects[i], (void **)&dyn);
    od->data_scan_time_p[i] = dyn->ref_trend_scantime();
    od->hold_p[i] = dyn->ref_trend_hold();
  }
  return 1;
}

static int graph_object_collect_build( Graph *graph, pwr_tObjid objid)
{
  pwr_sAttrRef *alist, *ap;
  int *is_attrp, *is_attr;
  int sts;
  char name[120];
  double x1, y1;
  graph_sObjectCollect *od;
  grow_sAttrInfo *grow_info, *grow_info_p;
  int grow_info_cnt;
  int i;
  grow_tObject scantime_button;
  grow_tObject hold_button;
  grow_tObject t1, l1;
  double z_width, z_height, z_descent;
  double name_width = 0;
  double trend_width = 28;
  double trend_height = 1.2;
  double y0 = 2.2;
  double x0 = 2;
  pwr_tTypeId attr_type;
  unsigned int attr_size, attr_offset, attr_dimension;
  GeDyn *dyn;
  char attr_name[120];


  printf("Here in object collect\n");
  if ( ! graph->get_current_objects_cb)
    return 0;

  sts = (graph->get_current_objects_cb) (graph->parent_ctx, &alist, &is_attr);
  if ( EVEN(sts)) return sts;
  
  if ( cdh_ObjidIsNull( alist->Objid))
    return 0;

  od = (graph_sObjectCollect *) calloc( 1, sizeof(graph_sObjectCollect));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_object_collect_close;

  grow_SetPath( graph->grow->ctx, 1, "pwr_exe:");

  // Scantime button
  graph->create_node( NULL, "pwr_valuereliefup", x0, y0 - 1, 4, y0 - 1 + 0.7, 
		      &scantime_button);

  dyn = new GeDyn( graph);
  grow_SetUserData( scantime_button, (void *)dyn);

  dyn->set_dyn( ge_mDynType_Value, ge_mActionType_ValueInput);
  dyn->update_elements();
  dyn->set_access( (glow_mAccess) 65535);
  dyn->set_attribute( scantime_button, "$local.ScanTime##Float32", 0);
  dyn->set_value_input( "%6.0f", 2, 10000000);

  // Hold button
  graph->create_node( "TrendHold", "pwr_smallbutton", x0 + trend_width/2 - 3./2, 
		      y0 - 1 , x0 + trend_width/2 + 3./2, y0 - 1 + 0.85,
		      &hold_button);
  grow_SetAnnotation( hold_button, 1, "  Hold", 6);

  dyn = new GeDyn( graph);
  grow_SetUserData( hold_button, (void *)dyn);

  // Set color tone
  grow_SelectClear( graph->grow->ctx);
  grow_SelectInsert( graph->grow->ctx, hold_button);
  grow_SetSelectOrigColorTone( graph->grow->ctx, glow_eDrawTone_Gray);
  grow_SelectRemove( graph->grow->ctx, hold_button);

  dyn->set_dyn( ge_mDynType_No, ge_mActionType_SetDig);
  dyn->update_elements();
  dyn->set_access( (glow_mAccess) 65535);
  dyn->set_attribute( hold_button, "$local.TrendHold##Boolean", 0);

  //  Zero text
  grow_CreateGrowText( graph->grow->ctx, "", "0",
	    x0 + trend_width - 0.2, y0 - 0.2,
	    glow_eDrawType_TextHelveticaBold, glow_eDrawType_Line, 2, glow_mDisplayLevel_1,
	    NULL, &t1);

  ap = alist;
  is_attrp = is_attr;
  x1 = x0;
  y1 = y0;
  while( cdh_ObjidIsNotNull( ap->Objid))
  {
    if ( *is_attrp)
    {
      sts = gdh_AttrrefToName( ap, name, sizeof(name), cdh_mNName);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetAttributeCharacteristics( name, &attr_type, &attr_size,
					     &attr_offset, &attr_dimension);
      if ( EVEN(sts)) return sts;

      switch ( attr_type) {
        case pwr_eType_Boolean:

          grow_CreateGrowTrend( graph->grow->ctx, "ActualValueTrend", 
			    x1, y1, trend_width, trend_height,
			    glow_eDrawType_Color37,
			    0, glow_mDisplayLevel_1, 1, 1,
			    glow_eDrawType_Color38, NULL, 
			    &od->trend_objects[od->trend_cnt]);
	  dyn = new GeDyn( graph);
	  dyn->dyn_type = ge_mDynType_Trend;
	  dyn->update_elements();
	  grow_SetUserData( od->trend_objects[od->trend_cnt], (void *)dyn);

          grow_GetObjectAttrInfo( od->trend_objects[od->trend_cnt], NULL, 
			      &grow_info, &grow_info_cnt);

	  strcpy( attr_name, name);
	  strcat( attr_name, "##Boolean");
	  grow_GetUserData( od->trend_objects[od->trend_cnt], (void **)&dyn);
	  dyn->set_attribute( od->trend_objects[od->trend_cnt], attr_name, 0);

          grow_info_p = grow_info;
          for ( i = 0; i < grow_info_cnt; i++)
          {
	    if ( strcmp( grow_info_p->name, "NoOfPoints") == 0)
	      *(int *) grow_info_p->value_p = 200;
	    else if ( strcmp( grow_info_p->name, "HorizontalLines") == 0)
	      *(int *) grow_info_p->value_p = 0;
	    else if ( strcmp( grow_info_p->name, "VerticalLines") == 0)
	      *(int *) grow_info_p->value_p = 9;
	    else if ( strcmp( grow_info_p->name, "CurveColor1") == 0)
	      *(int *) grow_info_p->value_p = glow_eDrawType_Color145;
	    else if ( strcmp( grow_info_p->name, "MaxValue1") == 0)
	      *(double *) grow_info_p->value_p = 1.2;
	    else if ( strcmp( grow_info_p->name, "MinValue1") == 0)
	      *(double *) grow_info_p->value_p = -0.1;

            grow_info_p++;
          }
          grow_FreeObjectAttrInfo( grow_info);

          // This will configure the curves
          grow_SetTrendScanTime( od->trend_objects[od->trend_cnt], 0.5);

          grow_GetTextExtent( graph->grow->ctx, name, 
	    strlen(name), glow_eDrawType_TextHelveticaBold,
	    2, &z_width, &z_height, &z_descent);

          grow_CreateGrowText( graph->grow->ctx, "", name,
	    x1 + trend_width + 1, y1 + trend_height/2 + z_height/2,
	    glow_eDrawType_TextHelveticaBold, glow_eDrawType_Line, 2, glow_mDisplayLevel_1,
	    NULL, &t1);
          if ( z_width > name_width)
            name_width = z_width;

          od->trend_cnt++;
          y1 += trend_height;
          break;
        default:
          ;
      }
      if ( od->trend_cnt >= MAX_TREND_OBJECTS)
        break;
    }

    ap++;
    is_attrp++;
  }
  free( alist);
  free( is_attr);

  // Draw separator lines between name texts
  y1 = y0;
  x1 = x0 + trend_width;
  grow_CreateGrowLine( graph->grow->ctx, "", 
	x0 + trend_width, y1, x0 + trend_width + name_width + 2, y1,
	glow_eDrawType_Color78, 1, 0, NULL, &l1);

  for ( i = 0; i < od->trend_cnt; i++) {
    y1 += trend_height;

    grow_CreateGrowLine( graph->grow->ctx, "", 
	x0 + trend_width, y1, x0 + trend_width + name_width + 2, y1,
	glow_eDrawType_Color71, 1, 0, NULL, &l1);
  }

  // Draw frame
  graph->create_node( "", "pwr_framethin", x0 - 0.5, 
		      y0 - 1.3 , x0 + trend_width + name_width + 2, 
		      y0 + od->trend_cnt * trend_height + 0.5,
		      &l1);
  dyn = new GeDyn( graph);
  grow_SetUserData( l1, (void *)dyn);

  grow_SetLayout( graph->grow->ctx, x0 - 1, y0 - 2.3, x0 + trend_width +
		  name_width + 3, y0 + od->trend_cnt * trend_height + 1.5);

  // Register scan function
  graph->graph_object_scan = graph_object_collect_scan;


  // Set graph attributes
  grow_sAttributes grow_attr;
  unsigned long mask = 0;

  mask |= grow_eAttr_double_buffer_on;
  grow_attr.double_buffer_on = 1;
  grow_SetAttributes( graph->grow->ctx, &grow_attr, mask);

  return 1;
}



int Graph::create_node( char *node_name, char *subgraph_str, double x1, 
			double y1, double x2, double y2, grow_tNode *node)
{
  int 	sts;
  grow_tNodeClass nc;
  double	sx, sy;
  double	ll_x, ll_y, ur_x, ur_y;
  char name[80];

  sts = grow_FindNodeClassByName( grow->ctx, subgraph_str, &nc);
  if ( EVEN(sts))
  {
    // Load the subgraph
    grow_OpenSubGraphFromName( grow->ctx, subgraph_str);
  }
  sts = grow_FindNodeClassByName( grow->ctx, subgraph_str, &nc);
  if ( EVEN(sts))
  {
    message( 'E', "Unable to open subgraph");
    return GE__SUBGRAPHLOAD;
  }

  if ( !node_name)
    sprintf( name, "O%d", grow_GetNextObjectNameNumber( grow->ctx));
  else
    strcpy( name, node_name);

  if ( !grow_IsSliderClass( nc))
    grow_CreateGrowNode( grow->ctx, name, nc, x1, y1, 
		NULL, node);
  else
    grow_CreateGrowSlider( grow->ctx, name, nc, x1, y1, 
		NULL, node);

  grow_MoveNode( *node, x1, y1);
    
  grow_MeasureNode( *node, &ll_x, &ll_y, &ur_x, &ur_y);
  if ( x2 != x1)
    sx = (x2 - x1)/(ur_x - ll_x);
  else
    sx = 1;
  if ( y2 != y1)
    sy = (y2 - y1)/(ur_y - ll_y);
  else
    sy = 1;
  grow_StoreTransform( *node);
  grow_SetObjectScale( *node, sx, sy, x1, y1, glow_eScaleType_LowerLeft);

  return 1;
}



//
// Object graph for PlcThread
//

typedef struct {
	pwr_tFloat32	*set_max_show_p;
	pwr_tFloat32	*set_min_show_p;
	pwr_tFloat32	set_max_show_old;
	pwr_tFloat32	set_min_show_old;
	grow_tObject 	set_bar_object;
	grow_tObject 	set_trend_object;
	grow_tObject 	hold_button_object;
	float		*scan_time_p;
	float		old_scan_time;
	double		*data_set_scan_time_p;
	int		*hold_button_p;
	int		*hold_set_p;
	} graph_sObjectPlcThread;

static void graph_object_PlcThread_scan( Graph *graph)
{
  graph_sObjectPlcThread *od = (graph_sObjectPlcThread *)graph->graph_object_data;

  // Reconfigure bar and trend if limits are changed
  if ( od->set_max_show_p && od->set_min_show_p && 
       ( *od->set_max_show_p != od->set_max_show_old ||
	 *od->set_min_show_p != od->set_min_show_old)) {
    if ( *od->set_max_show_p != *od->set_min_show_p) {
      grow_SetBarRange( od->set_bar_object, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
      grow_SetTrendRange( od->set_trend_object, 0, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
      grow_SetTrendRange( od->set_trend_object, 1, double(*od->set_min_show_p), 
		double(*od->set_max_show_p));
    }
    od->set_min_show_old = *od->set_min_show_p;
    od->set_max_show_old = *od->set_max_show_p;
  }      

  // Reconfigure new scantime
  if ( od->scan_time_p && 
       *od->scan_time_p != od->old_scan_time)
  {
    if ( graph->scan_time > *od->scan_time_p/200)
    {
      graph->scan_time = *od->scan_time_p/200;
      graph->animation_scan_time = *od->scan_time_p/200;
    }
    grow_SetTrendScanTime( od->set_trend_object, double( *od->scan_time_p/200));
    od->old_scan_time = *od->scan_time_p;
    *od->data_set_scan_time_p = double(*od->scan_time_p)/200;
  }

  if ( od->hold_button_p && *od->hold_button_p)
  {
    *od->hold_set_p = !*od->hold_set_p;
    *od->hold_button_p = 0;
    if ( *od->hold_set_p && od->hold_button_object)
      grow_SetObjectColorTone( od->hold_button_object, glow_eDrawTone_Yellow);
    else
      grow_ResetObjectColorTone( od->hold_button_object);
  }

}

static void graph_object_PlcThread_close( Graph *graph)
{
  free( graph->graph_object_data);
}

static int graph_object_PlcThread( Graph *graph, pwr_tObjid objid)
{
  pwr_sAttrRef attrref;
  int sts;
  graph_sObjectPlcThread *od;
  pwr_tClassId	classid;
  pwr_tFloat32 max_limit = 1;
  pwr_tFloat32 min_limit = 0;
  double scan_time;

  od = (graph_sObjectPlcThread *) calloc( 1, sizeof(graph_sObjectPlcThread));
  graph->graph_object_data = (void *) od;
  graph->graph_object_close = graph_object_PlcThread_close;

  sts = gdh_GetObjectClass( objid, &classid);
  if ( EVEN(sts)) return sts;

  // Get value for ScanTime
  sts = gdh_ClassAttrToAttrref( classid, ".ScanTime", &attrref);
  if ( EVEN(sts)) return sts;

  attrref.Objid = objid;
  sts = gdh_GetObjectInfoAttrref( &attrref, (void *)&max_limit, sizeof(max_limit));
  if ( EVEN(sts)) return sts;

  max_limit = max_limit * 2;

  od->set_max_show_old = max_limit;
  od->set_min_show_old = min_limit;

  // Configure ProcVal and SetVal bar
  sts = grow_FindObjectByName( graph->grow->ctx, "ActualScanTimeBar", 
		&od->set_bar_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit)
    grow_SetBarRange( od->set_bar_object, double(min_limit), double(max_limit));

  // Get pointers to max and min value
  od->set_max_show_p = (float *) graph->localdb_ref_or_create( "MaxShow", 
		pwr_eType_Float32);
  *od->set_max_show_p = od->set_max_show_old;

  od->set_min_show_p = (float *) graph->localdb_ref_or_create( "MinShow", 
		pwr_eType_Float32);
  *od->set_min_show_p = od->set_min_show_old;

  // Configure SetVal  trend
  sts = grow_FindObjectByName( graph->grow->ctx, "ActualScanTimeTrend", 
		&od->set_trend_object);
  if ( EVEN(sts)) return sts;

  if ( min_limit != max_limit) {
    grow_SetTrendRange( od->set_trend_object, 0, double(min_limit), double(max_limit));
    grow_SetTrendRange( od->set_trend_object, 1, double(min_limit), double(max_limit));
  }

  // Set scantime variable in local database
  grow_GetTrendScanTime( od->set_trend_object, &scan_time);
  od->scan_time_p = (float *) graph->localdb_ref_or_create( "ScanTime", 
		pwr_eType_Float32);
  od->old_scan_time = float( scan_time*200);
  *od->scan_time_p = od->old_scan_time;

  if ( graph->scan_time > *od->scan_time_p/200) {
    graph->scan_time = *od->scan_time_p/200;
    graph->animation_scan_time = *od->scan_time_p/200;
  }

  // Get Hold button
  sts = grow_FindObjectByName( graph->grow->ctx, "TrendHold", 
		&od->hold_button_object);
  if ( ODD(sts))
    od->hold_button_p = (int *) graph->localdb_ref_or_create( "TrendHold", 
		pwr_eType_Boolean);

  GeDyn *dyn;

  grow_GetUserData( od->set_trend_object, (void **)&dyn);
  od->data_set_scan_time_p = dyn->ref_trend_scantime();
  od->hold_set_p = dyn->ref_trend_hold();

  // Register scan function
  graph->graph_object_scan = graph_object_PlcThread_scan;

  return 1;
}

