/* xtt_evlist.cpp -- Alarm or event list in xtt

   PROVIEW/R  
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "rt_gdh.h"
#include "rt_mh.h"
#include "rt_mh_outunit.h"
}

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// Status is defined as int i xlib...
#ifdef Status
# undef Status
#endif

extern "C" {
#include "flow_x.h"
#include "co_mrm_util.h"
}
#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget.h"
#include "xtt_evlist.h"
#include "xtt_menu.h"

#include "xnav_bitmap_leaf12.h"
#include "xnav_bitmap_map12.h"
#include "xnav_bitmap_openmap12.h"
#include "xnav_bitmap_attr12.h"
#include "xnav_bitmap_attrarra12.h"
#include "xnav_bitmap_ack12.h"
#include "xnav_bitmap_alarm12.h"
#include "xnav_bitmap_eventalarm12.h"
#include "xnav_bitmap_eventacked12.h"
#include "xnav_bitmap_eventreturn12.h"
#include "xnav_bitmap_blockr_12.h"
#include "xnav_bitmap_blockl_12.h"

typedef union alau_Event ala_uEvent;
union alau_Event 
{
    mh_sMsgInfo Info;
    mh_sAck	Ack;
    mh_sMessage Msg;
    mh_sReturn  Return;
};

static int evlist_init_brow_cb( FlowCtx *fctx, void *client_data);
static int evlist_brow_cb( FlowCtx *ctx, flow_tEvent event);

//
//  Free pixmaps
//
void EvListBrow::free_pixmaps()
{
  brow_FreeAnnotPixmap( ctx, pixmap_leaf);
  brow_FreeAnnotPixmap( ctx, pixmap_map);
  brow_FreeAnnotPixmap( ctx, pixmap_openmap);
  brow_FreeAnnotPixmap( ctx, pixmap_attr);
  brow_FreeAnnotPixmap( ctx, pixmap_alarm);
  brow_FreeAnnotPixmap( ctx, pixmap_ack);
  brow_FreeAnnotPixmap( ctx, pixmap_eventalarm);
  brow_FreeAnnotPixmap( ctx, pixmap_eventacked);
  brow_FreeAnnotPixmap( ctx, pixmap_eventreturn);
  brow_FreeAnnotPixmap( ctx, pixmap_blockr);
  brow_FreeAnnotPixmap( ctx, pixmap_blockl);
}

//
//  Create pixmaps for leaf, closed map and open map
//
void EvListBrow::allocate_pixmaps()
{
	flow_sPixmapData pixmap_data;
	int i;

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_leaf12_width;
	    pixmap_data[i].height =xnav_bitmap_leaf12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_leaf12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_leaf);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_map12_width;
	    pixmap_data[i].height =xnav_bitmap_map12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_map12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_map);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_openmap12_width;
	    pixmap_data[i].height =xnav_bitmap_openmap12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_openmap12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_openmap);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_attr12_width;
	    pixmap_data[i].height =xnav_bitmap_attr12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_attr12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attr);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_attrarra12_width;
	    pixmap_data[i].height =xnav_bitmap_attrarra12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_attrarra12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attrarray);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_alarm12_width;
	    pixmap_data[i].height =xnav_bitmap_alarm12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_alarm12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_alarm);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_ack12_width;
	    pixmap_data[i].height =xnav_bitmap_ack12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_ack12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_ack);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_eventalarm12_width;
	    pixmap_data[i].height =xnav_bitmap_eventalarm12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_eventalarm12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_eventalarm);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_eventacked12_width;
	    pixmap_data[i].height =xnav_bitmap_eventacked12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_eventacked12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_eventacked);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_eventreturn12_width;
	    pixmap_data[i].height =xnav_bitmap_eventreturn12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_eventreturn12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_eventreturn);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_blockr_12_width;
	    pixmap_data[i].height =xnav_bitmap_blockr_12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_blockr_12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_blockr);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_blockl_12_width;
	    pixmap_data[i].height =xnav_bitmap_blockl_12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_blockl_12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_blockl);
}

//
// Create nodeclasses
//
void EvListBrow::create_nodeclasses()
{
  allocate_pixmaps();

  // Create common-class

  brow_CreateNodeClass( ctx, "EventDefault", 
		flow_eNodeGroup_Common, &nc_event);
  brow_AddFrame( nc_event, 0, 0, 35, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnot( nc_event, 0.8, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnotPixmap( nc_event, 0, 1.6, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_event, 1, 2.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_event, 3.3, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_event, 10, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_event, 26, 0.6, 3,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Nodeclass for A-alarm
  brow_CreateNodeClass( ctx, "A_Alarm", 
		flow_eNodeGroup_Common, &nc_a_alarm);
  brow_AddFrame( nc_a_alarm, 0, 0, 35, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddFilledRect( nc_a_alarm, 0.2, 0.15, 0.4, 0.4, flow_eDrawType_LineRed);
  brow_AddRect( nc_a_alarm, 0.2, 0.15, 0.4, 0.4, flow_eDrawType_Line, 0, 0);
  brow_AddAnnot( nc_a_alarm, 0.8, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnotPixmap( nc_a_alarm, 0, 1.6, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_a_alarm, 1, 2.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_a_alarm, 3.3, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_a_alarm, 10, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_a_alarm, 26, 0.6, 3,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Nodeclass for B-alarm
  brow_CreateNodeClass( ctx, "B_Alarm", 
		flow_eNodeGroup_Common, &nc_b_alarm);
  brow_AddFrame( nc_b_alarm, 0, 0, 35, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddFilledRect( nc_b_alarm, 0.2, 0.15, 0.4, 0.4, flow_eDrawType_Yellow);
  brow_AddRect( nc_b_alarm, 0.2, 0.15, 0.4, 0.4, flow_eDrawType_Line, 0, 0);
  brow_AddAnnot( nc_b_alarm, 0.8, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnotPixmap( nc_b_alarm, 0, 1.6, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_b_alarm, 1, 2.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_b_alarm, 3.3, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_b_alarm, 10, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_b_alarm, 26, 0.6, 3,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Nodeclass for Info
  brow_CreateNodeClass( ctx, "Info", 
		flow_eNodeGroup_Common, &nc_info);
  brow_AddFrame( nc_info, 0, 0, 35, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddFilledRect( nc_info, 0.2, 0.15, 0.4, 0.4, flow_eDrawType_Green);
  brow_AddRect( nc_info, 0.2, 0.15, 0.4, 0.4, flow_eDrawType_Line, 0, 0);
  brow_AddAnnot( nc_info, 0.8, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnotPixmap( nc_info, 0, 1.6, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_info, 1, 2.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_info, 3.3, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_info, 10, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_info, 26, 0.6, 3,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

}

void EvListBrow::brow_setup()
{
  brow_sAttributes brow_attr;
  unsigned long mask;

  mask = 0;
  mask |= brow_eAttr_indentation;
  brow_attr.indentation = 0.5;
  mask |= brow_eAttr_annotation_space;
  brow_attr.annotation_space = 0.5;
  brow_SetAttributes( ctx, &brow_attr, mask); 
  brow_SetCtxUserData( ctx, evlist);

  brow_EnableEvent( ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB3Press, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB3Down, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ObjectDeleted, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Up, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Down, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Right, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Left, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PF3, flow_eEventType_CallBack, 
	evlist_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Radiobutton, flow_eEventType_CallBack, 
	evlist_brow_cb);
}

//
// Backcall routine called at creation of the brow widget
// Enable event, create nodeclasses and insert the root objects.
//
static int evlist_init_brow_cb( FlowCtx *fctx, void *client_data)
{
  EvList *evlist = (EvList *) client_data;
  BrowCtx *ctx = (BrowCtx *)fctx;

  evlist->brow = new EvListBrow( ctx, (void *)evlist);

  evlist->brow->brow_setup();
  evlist->brow->create_nodeclasses();

  return 1;
}

EvList::EvList(
	void *ev_parent_ctx,
	Widget	ev_parent_wid,
	ev_eType ev_type, 
	int ev_size,
	Widget *w) :
	parent_ctx(ev_parent_ctx), parent_wid(ev_parent_wid),
	type(ev_type), size(0), max_size(ev_size), display_hundredth(0),
	hide_object(0), hide_text(0),
	start_trace_cb(NULL), display_in_xnav_cb(NULL), acc_beep_time(0),
	beep_interval(4)
{
  if ( max_size <= 0) {
    switch ( type) {
    case ev_eType_AlarmList:
      max_size = 1000;
      break;
    case ev_eType_BlockList:
      max_size = 200;
      break;
    default:
      max_size = 500;
    }
  }
  form_widget = ScrolledBrowCreate( parent_wid, "EvList", NULL, 0, 
	evlist_init_brow_cb, this, (Widget *)&brow_widget);
  XtManageChild( form_widget);

  *w = form_widget;
}


//
//  Delete ev
//
EvList::~EvList()
{
  delete brow;
  XtDestroyWidget( form_widget);
}

EvListBrow::~EvListBrow()
{
  free_pixmaps();
}

void EvList::event_info( mh_sMessage *msg)
{
  ala_uEvent 	*event = (ala_uEvent *) msg;
  int		sts;
  ItemAlarm	*dest;
  flow_eDest	dest_code;
  brow_tNode	dest_node;

  if ( type == ev_eType_AlarmList &&
       !( msg->Status & mh_mEventStatus_NotAck ||
          msg->Status & mh_mEventStatus_NotRet))
    return;

  if ( type != ev_eType_HistList )
  {
    sts = get_destination( event->Info.EventTime, (void **)&dest);
    if ( EVEN(sts))
    {
      dest_code = flow_eDest_IntoLast;
      dest_node = NULL;
    }
    else
    {
      dest_code = flow_eDest_Before;
      dest_node = dest->node;
    }
  }
  else
  {
    dest_code = flow_eDest_IntoLast;
    dest_node = NULL;
  }

  new ItemAlarm( this, "Alarm",
	event->Info.EventTime, event->Msg.EventText,
	event->Info.EventName, event->Info.EventFlags,
	event->Info.EventPrio, event->Info.Id,
	event->Info.Object, msg->Status, evlist_eEventType_Info,
	dest_node, dest_code);
  size++;
}

void EvList::event_alarm( mh_sMessage *msg)
{
  ala_uEvent 	*event = (ala_uEvent *) msg;
  int		sts;
  ItemAlarm	*dest;
  flow_eDest	dest_code;
  brow_tNode	dest_node;

  if ( type == ev_eType_AlarmList &&
       !( msg->Status & mh_mEventStatus_NotAck ||
          msg->Status & mh_mEventStatus_NotRet))
    return;

  if ( type != ev_eType_HistList )
  {
    sts = get_destination( event->Info.EventTime, (void **)&dest);
    if ( EVEN(sts))
    {
      dest_code = flow_eDest_IntoLast;
      dest_node = NULL;
    }
    else
    {
      dest_code = flow_eDest_Before;
      dest_node = dest->node;
    }
  }
  else
  {
    dest_code = flow_eDest_IntoLast;
    dest_node = NULL;
  }

  new ItemAlarm( this, "Alarm",
	event->Info.EventTime, event->Msg.EventText,
	event->Info.EventName, event->Info.EventFlags,
	event->Info.EventPrio, event->Info.Id,
	event->Info.Object, msg->Status, evlist_eEventType_Alarm,
	dest_node, dest_code);
  size++;
}

void EvList::event_block( mh_sBlock *msg)
{
  ala_uEvent 	*event = (ala_uEvent *) msg;
  int		sts;

  if ( type == ev_eType_BlockList) {

    switch( msg->Info.EventType) {
    case mh_eEvent_Block:
    case mh_eEvent_Reblock: {
      ItemAlarm 	*item;
      ItemAlarm	*dest;
      flow_eDest	dest_code;
      brow_tNode	dest_node;
      char name[80];
      char text[100];

      if ( oid_to_item( msg->Info.Object, (void **)&item)) {
	brow_DeleteNode( brow->ctx, item->node);
	size--;
      }
      sts = get_destination( event->Info.EventTime, (void **)&dest);
      if ( EVEN(sts)) {
	dest_code = flow_eDest_IntoLast;
	dest_node = NULL;
      }
      else {
	dest_code = flow_eDest_Before;
	dest_node = dest->node;
      }
      sts = gdh_ObjidToName( msg->Outunit, name, sizeof(name), 
			     cdh_mName_path | cdh_mName_object);
      if ( ODD(sts)) {
	strcpy( text, "User: ");
	strcat( text, name);
      }
      else
	strcpy( text, "Unknown");
      
      new ItemAlarm( this, "Alarm",
		     event->Info.EventTime, event->Info.EventName,
		     text, event->Info.EventFlags,
		     event->Info.EventPrio, event->Info.Id,
		     event->Info.Object, 0, evlist_eEventType_Block,
		     dest_node, dest_code);
      size++;
      break;
    }
    case mh_eEvent_CancelBlock:
    case mh_eEvent_Unblock: {
      ItemAlarm 	*item;
      
      if ( !oid_to_item( msg->Info.Object, (void **)&item))
	break;
      
      brow_DeleteNode( brow->ctx, item->node);
      size--;
      break;
    }
    default:
      ;
    }
  }
  else if ( type == ev_eType_EventList) {
    char text[100];
    char name[80];
    flow_eDest	dest_code;
    brow_tNode	dest_node;

    sts = gdh_ObjidToName( msg->Outunit, name, sizeof(name), 
			     cdh_mName_path | cdh_mName_object);
    if ( EVEN(sts))
      strcpy( name, "");

    switch( msg->Info.EventType) {
    case mh_eEvent_Block:
      strcpy( text, "Blocked by: ");
      break;
    case mh_eEvent_Reblock:
      strcpy( text, "Reblocked by: ");
      break;
    case mh_eEvent_CancelBlock:
      strcpy( text, "CancelBlocked by: ");
      break;
    case mh_eEvent_Unblock:
      strcpy( text, "Unblocked by: ");
      break;
    default:
      strcpy( text, "Unknown ");
    }
    strcat( text, name);

    dest_code = flow_eDest_IntoLast;
    dest_node = NULL;

    ItemAlarm *item = new ItemAlarm( this, "Alarm",
				     event->Info.EventTime, text, 
				     event->Info.EventName, event->Info.EventFlags,
				     event->Info.EventPrio, event->Info.Id,
				     event->Info.Object, 0, evlist_eEventType_Block,
				     dest_node, dest_code);

    switch( msg->Info.EventType) {
    case mh_eEvent_Block:
    case mh_eEvent_Reblock:
      brow_SetAnnotPixmap( item->node, 0, brow->pixmap_blockr);
      break;
    case mh_eEvent_CancelBlock:
    case mh_eEvent_Unblock:
      brow_SetAnnotPixmap( item->node, 0, brow->pixmap_blockl);
      break;
    default: ;
    }
    size++;
  }
}

void EvList::block_remove()
{
  brow_tNode	*node_list;
  int		node_count;
  ItemAlarm	*item;
  mh_eEventPrio prio = (mh_eEventPrio) 0;
  pwr_tStatus 	sts;

  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return;

  for ( int i = 0; i < node_count; i++) {
    brow_GetUserData( node_list[i], (void **)&item);

    switch( item->type) {
    case evlist_eItemType_Alarm:
      sts = mh_OutunitBlock( item->object, prio);
      break;
    default: ;
    }
  }
  free( node_list);
}

void EvList::event_cancel( mh_sReturn *msg)
{
  if ( type != ev_eType_AlarmList)
    return;

  ItemAlarm 	*item;

  if ( !id_to_item( &msg->TargetId, (void **)&item))
    return;

  brow_DeleteNode( brow->ctx, item->node);
  size--;
}

void EvList::event_ack( mh_sAck *msg)
{
  ala_uEvent 	*event = (ala_uEvent *) msg;
  int		sts;
  ItemAlarm	*dest;
  flow_eDest	dest_code;
  brow_tNode	dest_node;

  if ( type == ev_eType_EventList || type == ev_eType_HistList)
  {
    if(type == ev_eType_EventList)
    {
      sts = get_destination( event->Info.EventTime, (void **)&dest);
      if ( EVEN(sts))
      {
        dest_code = flow_eDest_IntoLast;
        dest_node = NULL;
      }
      else
      {
        dest_code = flow_eDest_Before;
        dest_node = dest->node;
      }
    }
    else
    {   
      dest_code = flow_eDest_IntoLast;
      dest_node = NULL;
    }
    new ItemAlarm( this, "Alarm",
	event->Info.EventTime, "",
	event->Info.EventName, event->Info.EventFlags,
	event->Info.EventPrio, event->Info.Id,
	event->Info.Object, 0, evlist_eEventType_Ack,
	dest_node, dest_code);
    size++;
  }
  
  else
  {
    // Alarmlist
    ItemAlarm 	*item;

    if ( !id_to_item( &msg->TargetId, (void **)&item))
      return;

    switch( item->event_type)
    {
      case evlist_eEventType_Alarm:
        if ( item->status & mh_mEventStatus_NotRet)
        {
          item->status &= ~mh_mEventStatus_NotAck;
          item->update_text();
        }
        else
        {
          brow_DeleteNode( brow->ctx, item->node);
          size--;
        }
        break;
      case evlist_eEventType_Info:
        if ( item->eventflags & mh_mEventFlags_InfoWindow)
        {
          brow_DeleteNode( brow->ctx, item->node);
          size--;
        }
        break;
      default:
        ;
    }
  }
}

void EvList::event_return( mh_sReturn *msg)
{
  ala_uEvent 	*event = (ala_uEvent *) msg;
  int		sts;
  ItemAlarm	*dest;
  flow_eDest	dest_code;
  brow_tNode	dest_node;

  if ( type == ev_eType_EventList || type == ev_eType_HistList)
  {
    if(type == ev_eType_EventList)
    {
      sts = get_destination( event->Info.EventTime, (void **)&dest);
      if ( EVEN(sts))
      {
        dest_code = flow_eDest_IntoLast;
        dest_node = NULL;
      }
      else
      {
        dest_code = flow_eDest_Before;
        dest_node = dest->node;
      }
    }
    else
    {
      dest_code = flow_eDest_IntoLast;
      dest_node = NULL;
    }
    new ItemAlarm( this, "Alarm",
	event->Info.EventTime, event->Msg.EventText,
	event->Info.EventName, event->Info.EventFlags,
	event->Info.EventPrio, event->Info.Id,
	event->Info.Object, 0, evlist_eEventType_Return,
	dest_node, dest_code);
    size++;
  }

  else
  {
    // Alarmlist
    ItemAlarm 	*item;

    if ( !id_to_item( &msg->TargetId, (void **)&item))
      return;

    if ( item->status & mh_mEventStatus_NotAck)
    {
      item->status &= ~mh_mEventStatus_NotRet;
      item->update_text();
    }
    else
    {
      brow_DeleteNode( brow->ctx, item->node);
      size--;
    }
  }
}

void EvList::print( char *filename)
{
  brow_Print( brow->ctx, filename);
}

void EvList::event_clear_alarmlist( pwr_tNodeIndex nix)
{
  int		i;
  brow_tObject 	*object_list, *stored_object_list;
  int		object_cnt;
  ItemAlarm	*object_item;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  if ( object_cnt == 0)
    return;

  // Copy the objectlist, to avoid changes when object are deleted
  stored_object_list = (brow_tObject *) calloc( object_cnt, sizeof(brow_tObject));
  memcpy( stored_object_list, object_list, object_cnt * sizeof(brow_tObject));

  // Remove all items with the present node index
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( stored_object_list[i], (void **)&object_item);
    switch( object_item->type)
    {
      case evlist_eItemType_Alarm:
        if ( object_item->eventid.Nix == nix)
          brow_DeleteNode( brow->ctx, object_item->node);
        break;
      default:
        ;
    }
  }
  free( (char *)stored_object_list);
}

void EvList::set_input_focus()
{
  if ( flow_IsViewable( brow_widget))
    XtCallAcceptFocus( brow_widget, CurrentTime);
}

//
//  Zoom
//
void EvList::zoom( double zoom_factor)
{
  brow_Zoom( brow->ctx, zoom_factor);
}

//
//  Return to base zoom factor
//
void EvList::unzoom()
{
  brow_UnZoom( brow->ctx);
}

void EvList::beep( double scantime)
{
  int sts;
  mh_sEventId *id;

  if ( acc_beep_time >= beep_interval)
     acc_beep_time = 0;

  sts = get_last_not_acked_beep( &id);
  if ( ODD(sts))
  {
    if ( acc_beep_time == 0)
      flow_Bell( form_widget);
    acc_beep_time += scantime;
  }
  else
    acc_beep_time = 0;
}

void EvList::set_nodraw()
{
  brow_SetNodraw( brow->ctx);
}

void EvList::reset_nodraw()
{
  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
}

void EvList::start_trace()
{
  brow_tNode	*node_list;
  int		node_count;
  ItemAlarm	*item;
  char		name[80];
  pwr_tObjid	parent;
  pwr_tClassId	classid;
  int		sts;

  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return;

  brow_GetUserData( node_list[0], (void **)&item);
  free( node_list);

  switch( item->type)
  {
    case evlist_eItemType_Alarm:
      
      sts = gdh_GetParent( item->object, &parent);
      if ( EVEN(sts)) return;
      
      sts = gdh_GetObjectClass( parent, &classid);
      if ( EVEN(sts)) return;

      if ( ! (classid == pwr_cClass_windowplc ||
              classid == pwr_cClass_windowcond ||
              classid == pwr_cClass_windoworderact ||
              classid == pwr_cClass_windowsubstep))
        return;

      sts = gdh_ObjidToName( item->object, 
	    	name, sizeof(name), cdh_mName_object);
      if ( EVEN(sts)) return;

      if ( start_trace_cb)
        (start_trace_cb)( parent_ctx, parent, name);

      break;
    default:
      ;
  }
}

void EvList::display_in_xnav()
{
  brow_tNode	*node_list;
  int		node_count;
  ItemAlarm	*item;
  pwr_sAttrRef	aref;

  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return;

  brow_GetUserData( node_list[0], (void **)&item);
  free( node_list);

  switch( item->type)
  {
    case evlist_eItemType_Alarm:
      if ( cdh_ObjidIsNull( item->object))
        return;
      if ( display_in_xnav_cb) {
	aref = cdh_ObjidToAref( item->object);
        (display_in_xnav_cb)( parent_ctx, &aref);
      }
      break;
    default:
      ;
  }
}

//
// Callbacks from brow
//
static int evlist_brow_cb( FlowCtx *ctx, flow_tEvent event)
{
  EvList		*evlist;
  ItemAlarm 		*item;

  if ( event->event == flow_eEvent_ObjectDeleted)
  {
    brow_GetUserData( event->object.object, (void **)&item);
    delete item;
    return 1;
  }

  brow_GetCtxUserData( (BrowCtx *)ctx, (void **) &evlist);
  switch ( event->event)
  {
    case flow_eEvent_Key_Up:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( evlist->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
        sts = brow_GetLast( evlist->brow->ctx, &object);
        if ( EVEN(sts)) return 1;
      }
      else
      {
        sts = brow_GetPrevious( evlist->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          sts = brow_GetLast( evlist->brow->ctx, &object);
          if ( EVEN(sts))
	  {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      brow_SelectClear( evlist->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( evlist->brow->ctx, object);
      if ( !brow_IsVisible( evlist->brow->ctx, object))
        brow_CenterObject( evlist->brow->ctx, object, 0.25);
      if ( node_count)
        free( node_list);
      break;
    }
    case flow_eEvent_Key_Down:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( evlist->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
        sts = brow_GetFirst( evlist->brow->ctx, &object);
        if ( EVEN(sts)) return 1;
      }
      else
      {
        sts = brow_GetNext( evlist->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          sts = brow_GetFirst( evlist->brow->ctx, &object);
          if ( EVEN(sts))
	  {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      brow_SelectClear( evlist->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( evlist->brow->ctx, object);
      if ( !brow_IsVisible( evlist->brow->ctx, object))
        brow_CenterObject( evlist->brow->ctx, object, 0.75);
      if ( node_count)
        free( node_list);
      break;
    }
    case flow_eEvent_SelectClear:
      brow_ResetSelectInverse( evlist->brow->ctx);
      break;
    case flow_eEvent_MB1Click:
      // Select
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          if ( brow_FindSelectedObject( evlist->brow->ctx, event->object.object))
          {
            brow_SelectClear( evlist->brow->ctx);
          }
          else
          {
            brow_SelectClear( evlist->brow->ctx);
            brow_SetInverse( event->object.object, 1);
            brow_SelectInsert( evlist->brow->ctx, event->object.object);
          }
          break;
        default:
          brow_SelectClear( evlist->brow->ctx);
      }
      break;
    case flow_eEvent_MB3Down:
    {
      brow_SetClickSensitivity( evlist->brow->ctx, 
				      flow_mSensitivity_MB3Press);
      break;
    }
    case flow_eEvent_MB3Press:
    {            
      // Popup menu
      Widget popup;

      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          if ( evlist->popup_menu_cb) {
            brow_GetUserData( event->object.object, (void **)&item);
            if ( cdh_ObjidIsNotNull( item->object)) {
              pwr_sAttrRef attrref = cdh_ObjidToAref( item->object);
              (evlist->popup_menu_cb)( evlist->parent_ctx, attrref,
		     (unsigned long)xmenu_eItemType_Object, 
		     (unsigned long)xmenu_mUtility_EventList, NULL, &popup);
              if ( !popup)
                break;

              mrm_PositionPopup( popup, evlist->brow_widget, 
			       event->any.x_pixel + 8, event->any.y_pixel);
              XtManageChild(popup);
            }
          }
          break;
        default:
          ;
      }
      break;
    }
    case flow_eEvent_Key_Left:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( evlist->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        return 1;

      if ( brow_IsOpen( node_list[0]))
        // Close this node
        object = node_list[0];
      else
      {
        // Close parent
        sts = brow_GetParent( evlist->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          free( node_list);
          return 1;
        }
      }
      brow_GetUserData( object, (void **)&item);
      switch( item->type)
      {
        case evlist_eItemType_Alarm:
//	  ((ItemLocal *)item)->close( evlist, 0, 0);
          break;
        default:
          ;
      }
      brow_SelectClear( evlist->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( evlist->brow->ctx, object);
      if ( !brow_IsVisible( evlist->brow->ctx, object))
        brow_CenterObject( evlist->brow->ctx, object, 0.25);
      free( node_list);
      break;
    }
    case flow_eEvent_Key_Right:
    {
      brow_tNode	*node_list;
      int		node_count;

      brow_GetSelectedNodes( evlist->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        return 1;

      brow_GetUserData( node_list[0], (void **)&item);
      switch( item->type)
      {
        case evlist_eItemType_Alarm:
//	  ((ItemAlarm *)item)->open_children( evlist, 0, 0);
          break;
        default:
          ;
      }
    }
    case flow_eEvent_MB1DoubleClick:
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          switch( item->type)
          {
            case evlist_eItemType_Alarm: 
//	      ((ItemAlarm *)item)->open_children( evlist,
//			event->object.x, event->object.y);
              break;
            default:
              ;
          }
          break;
        default:
          ;
      }
      break;
    default:
      ;
  }
  return 1;
}

ItemAlarm::ItemAlarm( EvList *item_evlist, char *item_name, pwr_tTime item_time,
	char *item_eventtext, char *item_eventname, int item_eventflags,
	unsigned long item_eventprio, mh_sEventId item_eventid,
	pwr_tObjid item_object, unsigned long item_status,
	evlist_eEventType item_event_type,
	brow_tNode dest, flow_eDest dest_code):
	event_type(item_event_type), evlist(item_evlist), time(item_time), 
	eventflags(item_eventflags), eventprio(item_eventprio),
	eventid(item_eventid), object(item_object), status(item_status)
{
  type = evlist_eItemType_Alarm;
  brow_tNodeClass 	nc;

  strcpy( name, item_name);
  strncpy( eventtext, item_eventtext, sizeof(eventtext));
  eventtext[sizeof(eventtext)-1] = 0;
  strncpy( eventname, item_eventname, sizeof(eventname));
  eventname[sizeof(eventname)-1] = 0;

  switch ( event_type) { 
  case evlist_eEventType_Alarm:
  case evlist_eEventType_Block:
    switch ( eventprio) {
    case mh_eEventPrio_A:
      nc = evlist->brow->nc_a_alarm;
      break;
    case mh_eEventPrio_B:
      nc = evlist->brow->nc_b_alarm;
      break;
    default:
      nc = evlist->brow->nc_event;
    }
    break;
  case evlist_eEventType_Info:
    nc = evlist->brow->nc_info;
    break;
  default:
    nc = evlist->brow->nc_event;
    break;
  }

  brow_CreateNode( evlist->brow->ctx, item_name, nc,
		   dest, dest_code, (void *) this, 1, &node);

//  brow_SetAnnotPixmap( node, 0, evlist->brow->pixmap_leaf);
  update_text();

  if ( evlist->size > evlist->max_size)
  {
    // Delete last
    brow_tObject last_node;
    int sts;

    sts = brow_GetLast( evlist->brow->ctx, &last_node);
    if ( ODD(sts))
    {
      brow_DeleteNode( evlist->brow->ctx, last_node);
      evlist->size--;
    }
  }
}

void ItemAlarm::update_text()
{
  char type_str[8];
  char info_str[8];
  char time_str[40];

  strcpy( info_str, "");
  if ( evlist->type == ev_eType_AlarmList)
  {
    if ( status & mh_mEventStatus_NotAck) 
      brow_SetAnnotPixmap( node, 0, evlist->brow->pixmap_ack);
    else
      brow_RemoveAnnotPixmap( node, 0);
    if ( status & mh_mEventStatus_NotRet) 
      brow_SetAnnotPixmap( node, 1, evlist->brow->pixmap_alarm);
    else
      brow_RemoveAnnotPixmap( node, 1);
  }
  else
  {
    switch ( event_type)
    {
      case evlist_eEventType_Return:
        brow_SetAnnotPixmap( node, 0, evlist->brow->pixmap_eventreturn);
        break;
      case evlist_eEventType_Ack:
        brow_SetAnnotPixmap( node, 0, evlist->brow->pixmap_eventacked);
        break;
      case evlist_eEventType_Info:
      case evlist_eEventType_Alarm:
        brow_SetAnnotPixmap( node, 0, evlist->brow->pixmap_eventalarm);
        break;
      case evlist_eEventType_Block:
        break;
      default:
        ;
    }
  }

  switch ( event_type)
  {
    case evlist_eEventType_Info:
      strcpy( type_str, "I");
      brow_SetAnnotation( node, 1, type_str, strlen(type_str));
      break;
    case evlist_eEventType_Block:
    case evlist_eEventType_Alarm:
      switch ( eventprio)
      {
        case mh_eEventPrio_A:
          strcpy( type_str, "A");
          break;
        case mh_eEventPrio_B:
          strcpy( type_str, "B");
          break;
        case mh_eEventPrio_C:
          strcpy( type_str, "C");
          break;
        case mh_eEventPrio_D:
          strcpy( type_str, "D");
          break;
      }
      brow_SetAnnotation( node, 0, type_str, strlen(type_str));
      break;
    default:
      ;
  }

  time_AtoAscii( &time, time_eFormat_ComprDateAndTime, time_str, 
	sizeof(time_str));
  if ( !evlist->display_hundredth)
    time_str[17] = 0;
  brow_SetAnnotation( node, 1, time_str, strlen(time_str));

  switch ( type)
  {
    case evlist_eEventType_Info:
    case evlist_eEventType_Alarm:
      if ( ! evlist->hide_text)
        brow_SetAnnotation( node, 2, eventtext, strlen(eventtext));
      else
        brow_SetAnnotation( node, 2, "", 0);
      break;
    default:
      brow_SetAnnotation( node, 2, "", 0);
  }

  if ( !evlist->hide_object && evlist->hide_text)
  {
    brow_SetAnnotation( node, 2, eventname, strlen(eventname));
    brow_SetAnnotation( node, 3, "", 0);
  }
  else if ( !evlist->hide_object)
    brow_SetAnnotation( node, 3, eventname, strlen(eventname));
  else
    brow_SetAnnotation( node, 3, "", 0);
}

int EvList::get_last_not_acked( mh_sEventId **id)
{
  int		i;
  brow_tObject 	*object_list;
  int		object_cnt;
  ItemAlarm	*object_item;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&object_item);
    switch( object_item->type)
    {
      case evlist_eItemType_Alarm:
        if ( object_item->status & mh_mEventStatus_NotAck) 
        {
          *id = &object_item->eventid;
          return 1;
        }
        break;
      default:
        ;
    }
  }
  return 0;
}

int EvList::get_last_not_acked_beep( mh_sEventId **id)
{
  int		i;
  brow_tObject 	*object_list;
  int		object_cnt;
  ItemAlarm	*object_item;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&object_item);
    switch( object_item->type)
    {
      case evlist_eItemType_Alarm:
        if ( object_item->status & mh_mEventStatus_NotAck &&
	     object_item->eventflags & mh_mEventFlags_Bell)  {
          *id = &object_item->eventid;
          return 1;
        }
        break;
      default:
        ;
    }
  }
  return 0;
}

int EvList::get_last_not_acked_prio( mh_sEventId **id, unsigned long type, 
	unsigned long prio)
{
  int		i;
  brow_tObject 	*object_list;
  int		object_cnt;
  ItemAlarm	*object_item;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&object_item);
    switch( object_item->type)
    {
      case evlist_eItemType_Alarm:
        switch( object_item->event_type)
        {
          case evlist_eEventType_Alarm:
            if ( object_item->status & mh_mEventStatus_NotAck &&
	         object_item->event_type == (evlist_eEventType) type &&
	         object_item->eventprio == prio)
            {
              *id = &object_item->eventid;
              return 1;
            }
            break;
          case evlist_eEventType_Info:
            if ( object_item->status & mh_mEventStatus_NotAck &&
	         object_item->event_type == (evlist_eEventType) type)
            {
              *id = &object_item->eventid;
              return 1;
            }
            break;
          default:
            ;
        }
        break;
      default:
        ;
    }
  }
  return 0;
}

int EvList::get_alarm_info( evlist_sAlarmInfo *info)
{
  int		i;
  brow_tObject 	*object_list;
  int		object_cnt;
  ItemAlarm	*object_item;
  int		a_cnt = 0;
  int		b_cnt = 0;
  int		c_cnt = 0;
  int		d_cnt = 0;
  int		i_cnt = 0;

  memset( info, 0, sizeof(*info));

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&object_item);
    switch( object_item->type)
    {
      case evlist_eItemType_Alarm:
        switch( object_item->event_type)
        {
          case evlist_eEventType_Alarm:
            if ( object_item->status & mh_mEventStatus_NotRet)
              info->alarms_total++;

            if ( object_item->status & mh_mEventStatus_NotAck) 
            {
              info->alarms_notacked++;
              switch( object_item->eventprio)
              {
                case mh_eEventPrio_A:
                  if ( a_cnt >= ALARM_INFO_A_SIZE)
                    break;
                  strcpy( info->a_alarm_text[a_cnt], object_item->eventtext);
                  info->a_alarm_active[a_cnt] = object_item->status & mh_mEventStatus_NotRet;
                  info->a_alarm_exist[a_cnt] = 1;
                  a_cnt++;
                  break;
                case mh_eEventPrio_B:
                  if ( b_cnt >= ALARM_INFO_B_SIZE)
                    break;
                  strcpy( info->b_alarm_text[b_cnt], object_item->eventtext);
                  info->b_alarm_active[b_cnt] = object_item->status & mh_mEventStatus_NotRet;
                  info->b_alarm_exist[b_cnt] = 1;
                  b_cnt++;
                  break;
                case mh_eEventPrio_C:
                  if ( c_cnt >= ALARM_INFO_C_SIZE)
                    break;
                  strcpy( info->c_alarm_text[c_cnt], object_item->eventtext);
                  info->c_alarm_active[c_cnt] = object_item->status & mh_mEventStatus_NotRet;
                  info->c_alarm_exist[c_cnt] = 1;
                  c_cnt++;
                  break;
                case mh_eEventPrio_D:
                  if ( d_cnt >= ALARM_INFO_D_SIZE)
                    break;
                  strcpy( info->d_alarm_text[d_cnt], object_item->eventtext);
                  info->d_alarm_active[d_cnt] = object_item->status & mh_mEventStatus_NotRet;
                  info->d_alarm_exist[d_cnt] = 1;
                  d_cnt++;
                  break;
                default:
                  ;
              }
            }
            break;
          case evlist_eEventType_Info:
            if ( object_item->eventflags & mh_mEventFlags_InfoWindow && 
                 object_item->status & mh_mEventStatus_NotAck) 
            {
              if ( i_cnt >= ALARM_INFO_I_SIZE)
                break;
              strcpy( info->i_alarm_text[i_cnt], object_item->eventtext);
              info->i_alarm_active[i_cnt] = object_item->status & mh_mEventStatus_NotRet;
              info->i_alarm_exist[i_cnt] = 1;
              i_cnt++;
            }
            break;
          default:
            ;
        }
        break;
      default:
        ;
    }
  }
  return 1;
}

void EvList::set_display_hundredth( int value)
{
  if ( type == ev_eType_BlockList)
    return;

  display_hundredth = value;
  update_text();
}

void EvList::set_hide_object( int value)
{
  if ( type == ev_eType_BlockList)
    return;

  hide_object = value;
  update_text();
}

void EvList::set_hide_text( int value)
{
  if ( type == ev_eType_BlockList)
    return;

  hide_text = value;
  update_text();
}

void EvList::update_text()
{
  int		i;
  brow_tObject 	*object_list;
  int		object_cnt;
  ItemAlarm	*object_item;

  brow_SetNodraw( brow->ctx);
  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&object_item);
    switch( object_item->type)
    {
      case evlist_eItemType_Alarm:
        object_item->update_text();
        break;
      default:
        ;
    }
  }
  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
}

int EvList::id_to_item( mh_sEventId *id, void **item)
{
  int		i;
  brow_tObject 	*object_list;
  int		object_cnt;
  ItemAlarm	*object_item;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);

  for ( i = 0; i < object_cnt; i++) {
    brow_GetUserData( object_list[i], (void **)&object_item);
    switch( object_item->type) {
    case evlist_eItemType_Alarm:
      if ( memcmp( &object_item->eventid, id, sizeof(object_item->eventid))
	   == 0) {
	*item = (void *)object_item;
	return 1;
      }
      break;
    default:
      ;
    }
  }
  return 0;
}

int EvList::oid_to_item( pwr_tOid oid, void **item)
{
  int		i;
  brow_tObject 	*object_list;
  int		object_cnt;
  ItemAlarm	*object_item;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);

  for ( i = 0; i < object_cnt; i++) {
    brow_GetUserData( object_list[i], (void **)&object_item);
    switch( object_item->type) {
    case evlist_eItemType_Alarm:
      if ( cdh_ObjidIsEqual( object_item->object, oid)) {
	*item = (void *)object_item;
	return 1;
      }
      break;
    default:
      ;
    }
  }
  return 0;
}

void EvList::ack( mh_sEventId *id)
{
  ItemAlarm 	*item;

  if ( !id_to_item( id, (void **)&item))
    return;

  if ( item->status & mh_mEventStatus_NotAck) 
    item->status &= ~mh_mEventStatus_NotAck;

  if ( item->status & mh_mEventStatus_NotRet) 
    // Remove the not ack marks in the item text
    item->update_text();
  else if ( type == ev_eType_AlarmList)
  {
    // Detete the item from the alarm list
    brow_DeleteNode( brow->ctx, item->node);
    size--;
  }
}

int EvList::get_selected_event( char *eventname)
{
  brow_tNode	*node_list;
  int		node_count;
  ItemAlarm	*item;

  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return 0;

  brow_GetUserData( node_list[0], (void **)&item);
  free( node_list);
  switch( item->type)
  {
    case evlist_eItemType_Alarm:
      strcpy( eventname, item->eventname);
      break;
    default:
      return 0;
  }
  return 1;
}

int EvList::get_destination( pwr_tTime time, void **dest)
{
  int		i;
  brow_tObject 	*object_list;
  int		object_cnt;
  ItemAlarm	*object_item;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&object_item);
    switch( object_item->type)
    {
      case evlist_eItemType_Alarm:
        if ( time_Acomp( &time, &object_item->time) >= 0)
        {
          *dest = (void *) object_item;
          return 1;
        }
        break;
      default:
        ;
    }
  }
  return 0;
}

