/* wb_wdanav.cpp -- Display object info

   PROVIEW/R  
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#if defined OS_VMS && defined __ALPHA
# pragma message disable (NOSIMPINT,EXTROUENCUNNOBJ)
#endif

#if defined OS_VMS && !defined __ALPHA
# pragma message disable (LONGEXTERN)
#endif

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "rt_types.h"
#include "pwr_baseclasses.h"
#include "wb_wda_msg.h"
#include "rt_mh_net.h"
#include "wb_trv.h"
}

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget.h"

#include "wb_wda.h"
#include "wb_wdanav.h"
#include "wb_wnav.h"
#include "wb_wnav_brow.h"
#include "wb_wnav_item.h"

static char null_str[] = "";

static int wdanav_init_brow_cb( FlowCtx *fctx, void *client_data);
static int wdanav_attr_found_cb( pwr_tObjid objid, void *a1, void *a2,
				 void *a3, void *a4, void *a5);

void WdaNav::message( char sev, char *text)
{
  (message_cb)( parent_ctx, sev, text);
}

int WdaNav::print( char *filename)
{
  return brow_Print( brow->ctx, filename);
}


//
// Create the navigator widget
//
WdaNav::WdaNav(
	void 		*wa_parent_ctx,
	Widget		wa_parent_wid,
	char 		*wa_name,
	ldh_tSesContext wa_ldhses,
	pwr_tObjid 	wa_objid,
	pwr_tClassId 	wa_classid,
        char            *wa_attribute,
	int 		wa_editmode,
	int 		wa_advanced_user,
	int		wa_display_objectname,
	wb_eUtility	wa_utility,
	Widget 		*w,
	pwr_tStatus 	*status) :
	parent_ctx(wa_parent_ctx), parent_wid(wa_parent_wid),
	ldhses(wa_ldhses), objid(wa_objid), classid(wa_classid),
	editmode(wa_editmode), 
	advanced_user(wa_advanced_user), 
	display_objectname(wa_display_objectname), bypass(0),
	trace_started(0), message_cb(NULL), utility(wa_utility), 
	displayed(0)
{
  strcpy( name, wa_name);
  strcpy( attribute, wa_attribute);

  form_widget = ScrolledBrowCreate( parent_wid, name, NULL, 0, 
	wdanav_init_brow_cb, this, (Widget *)&brow_widget);
  XtManageChild( form_widget);

  // Create the root item
  *w = form_widget;

  *status = 1;
}

//
//  Delete a nav context
//
WdaNav::~WdaNav()
{
  if ( trace_started)
    XtRemoveTimeOut( trace_timerid);

  delete brow;
  XtDestroyWidget( form_widget);
}

void WdaNav::set_inputfocus()
{
//  brow_SetInputFocus( brow->ctx);

  if ( !displayed)
    return;

  XtCallAcceptFocus( brow_widget, CurrentTime);
}

//
// Check that the current selected item is valid for change
//
int WdaNav::check_attr( int *multiline, brow_tObject *node,
	char *name, char **init_value, int *size)
{
  brow_tNode	*node_list;
  int		node_count;
  WItem		*base_item;
  int		sts;
  
  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return WDA__NOATTRSEL;

  brow_GetUserData( node_list[0], (void **)&base_item);
  free( node_list);

  switch( base_item->type)
  {
    case wnav_eItemType_Attr:
    case wnav_eItemType_AttrInput:
    case wnav_eItemType_AttrInputF:
    case wnav_eItemType_AttrInputInv:
    case wnav_eItemType_AttrOutput:
    case wnav_eItemType_AttrArrayElem:
    {
      WItemBaseAttr *item = (WItemBaseAttr *)base_item;

      if ( !editmode)
        return WDA__NOEDIT;
      if ( item->flags & PWR_MASK_NOEDIT && !bypass)
        return WDA__FLAGNOEDIT;
      if ( item->flags & PWR_MASK_STATE && !bypass)
        return WDA__FLAGSTATE;
      if ( item->type_id == pwr_eType_Text)
      {
        *multiline = 1;
        *size = item->size;
        sts = item->get_value( init_value);
        if ( EVEN(sts)) return sts;
      }
      else if ( item->type_id == pwr_eType_String)
      {
        *multiline = 0;
        *size = item->size;
        sts = item->get_value( init_value);
        if ( EVEN(sts)) return sts;
      }
      else
      {
        *multiline = 0;
        *init_value = NULL;
        *size = 40;
      }
      *node = item->node;
      strcpy( name, item->attr);
      break;
    }
    case wnav_eItemType_ObjectName:
    {
      if ( !editmode)
        return WDA__NOEDIT;
      sts = ((WItemObjectName *)base_item)->get_value( init_value);
      *node = ((WItemObjectName *)base_item)->node;
      *multiline = 0;
      *size = 32;
      break;
    }
    default:
      *multiline = 0;
      return WDA__ATTRNOEDIT;
  }
  return WDA__SUCCESS;
}


//
// Callbacks from brow
//
static int wdanav_brow_cb( FlowCtx *ctx, flow_tEvent event)
{
  WdaNav		*wdanav;
  WItem	 		*item;

  if ( event->event == flow_eEvent_ObjectDeleted)
  {
    brow_GetUserData( event->object.object, (void **)&item);
    delete item;
    return 1;
  }

  brow_GetCtxUserData( (BrowCtx *)ctx, (void **) &wdanav);
  wdanav->message( ' ', null_str);
  switch ( event->event)
  {
    case flow_eEvent_Key_Up:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( wdanav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
        sts = brow_GetLast( wdanav->brow->ctx, &object);
        if ( EVEN(sts)) return 1;
      }
      else
      {
        sts = brow_GetPrevious( wdanav->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          sts = brow_GetLast( wdanav->brow->ctx, &object);
          if ( EVEN(sts))
	  {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      brow_SelectClear( wdanav->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( wdanav->brow->ctx, object);
      if ( !brow_IsVisible( wdanav->brow->ctx, object))
        brow_CenterObject( wdanav->brow->ctx, object, 0.25);
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

      brow_GetSelectedNodes( wdanav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
        sts = brow_GetFirst( wdanav->brow->ctx, &object);
        if ( EVEN(sts)) return 1;
      }
      else
      {
        sts = brow_GetNext( wdanav->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          sts = brow_GetFirst( wdanav->brow->ctx, &object);
          if ( EVEN(sts))
	  {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      brow_SelectClear( wdanav->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( wdanav->brow->ctx, object);
      if ( !brow_IsVisible( wdanav->brow->ctx, object))
        brow_CenterObject( wdanav->brow->ctx, object, 0.75);
      if ( node_count)
        free( node_list);
      break;
    }
    case flow_eEvent_SelectClear:
      brow_ResetSelectInverse( wdanav->brow->ctx);
      break;
    case flow_eEvent_MB1Click:
    {
      // Select
      double ll_x, ll_y, ur_x, ur_y;
      int		sts;

      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_MeasureNode( event->object.object, &ll_x, &ll_y,
			&ur_x, &ur_y);
	  if ( event->object.x < ll_x + 1.0)
          {
            // Simulate doubleclick
            flow_tEvent doubleclick_event;

            doubleclick_event = (flow_tEvent) calloc( 1, sizeof(*doubleclick_event));
            memcpy( doubleclick_event, event, sizeof(*doubleclick_event));
            doubleclick_event->event = flow_eEvent_MB1DoubleClick;
            sts = wdanav_brow_cb( ctx, doubleclick_event);
            free( (char *) doubleclick_event);
            return sts;
          }

          if ( brow_FindSelectedObject( wdanav->brow->ctx, event->object.object))
          {
            brow_SelectClear( wdanav->brow->ctx);
          }
          else
          {
            brow_SelectClear( wdanav->brow->ctx);
            brow_SetInverse( event->object.object, 1);
            brow_SelectInsert( wdanav->brow->ctx, event->object.object);
          }
          break;
        default:
          brow_SelectClear( wdanav->brow->ctx);
      }
      break;
    }
    case flow_eEvent_Key_Left:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( wdanav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        return 1;

      if ( brow_IsOpen( node_list[0]))
        // Close this node
        object = node_list[0];
      else
      {
        // Close parent
        sts = brow_GetParent( wdanav->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          free( node_list);
          return 1;
        }
      }
      brow_GetUserData( object, (void **)&item);
      switch( item->type)
      {
        case wnav_eItemType_Attr: 
	  ((WItemAttr *)item)->close( 0, 0);
          break;
        case wnav_eItemType_AttrArrayElem: 
	  ((WItemAttrArrayElem *)item)->close( 0, 0);
          break;
        case wnav_eItemType_AttrArray: 
	  ((WItemAttrArray *)item)->close( 0, 0);
          break;
        case wnav_eItemType_AttrArrayOutput: 
	  ((WItemAttrArrayOutput *)item)->close( 0, 0);
          break;
        default:
          ;
      }
      brow_SelectClear( wdanav->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( wdanav->brow->ctx, object);
      if ( !brow_IsVisible( wdanav->brow->ctx, object))
        brow_CenterObject( wdanav->brow->ctx, object, 0.25);
      free( node_list);
      break;
    }
    case flow_eEvent_Key_Right:
    {
      brow_tNode	*node_list;
      int		node_count;
      int		sts;

      brow_GetSelectedNodes( wdanav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        return 1;

      brow_GetUserData( node_list[0], (void **)&item);
      switch( item->type)
      {
        case wnav_eItemType_Attr:
	  sts = ((WItemAttr *)item)->open_children( 0, 0);
	  if ( ODD(sts))
            break;

          if ( wdanav->advanced_user && wdanav->change_value_cb)
            (wdanav->change_value_cb)( wdanav->parent_ctx);
          break;
        case wnav_eItemType_AttrInput:
        case wnav_eItemType_AttrInputF:
        case wnav_eItemType_AttrInputInv:
        case wnav_eItemType_AttrOutput:
          if ( wdanav->advanced_user && wdanav->change_value_cb)
            (wdanav->change_value_cb)( wdanav->parent_ctx);
          break;
        case wnav_eItemType_AttrArrayElem:
	  sts = ((WItemAttrArrayElem *)item)->open_children( 0, 0);
	  if ( ODD(sts))
            break;

          if ( wdanav->advanced_user && wdanav->change_value_cb)
            (wdanav->change_value_cb)( wdanav->parent_ctx);
          break;
        case wnav_eItemType_ObjectName:
          if ( wdanav->advanced_user && wdanav->change_value_cb)
            (wdanav->change_value_cb)( wdanav->parent_ctx);
          break;
        case wnav_eItemType_AttrArray: 
	  ((WItemAttrArray *)item)->open_attributes( 0, 0);
          break;
        case wnav_eItemType_AttrArrayOutput: 
	  ((WItemAttrArrayOutput *)item)->open_attributes( 0, 0);
          break;
        case wnav_eItemType_Enum: 
        {
          int value;

          if ( !wdanav->advanced_user)
            break;
          brow_GetRadiobutton( node_list[0], 0, &value);
          if ( !value)
	    ((WItemEnum *)item)->set();
          break;
        }
        case wnav_eItemType_Mask: 
        {
          int value;

          if ( !wdanav->advanced_user)
            break;
          brow_GetRadiobutton( node_list[0], 0, &value);
	  ((WItemMask *)item)->set( !value);
          break;
        }
        default:
          ;
      }
      free( node_list);
      break;
    }
    case flow_eEvent_Key_ShiftRight:
    {
      brow_tNode	*node_list;
      int		node_count;
      int value;

      if ( wdanav->utility != wb_eUtility_PlcEditor)
        return 1;

      brow_GetSelectedNodes( wdanav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        return 1;

      brow_GetUserData( node_list[0], (void **)&item);
      switch( item->type)
      {
        case wnav_eItemType_AttrArray: 
	  ((WItemAttrArray *)item)->open_attributes( 0, 0);
          break;
        case wnav_eItemType_AttrInput:
        {
          brow_GetRadiobutton( node_list[0], 0, &value);
	  ((WItemAttrInput *)item)->set_mask( 0, !value);
          break;
        }
        case wnav_eItemType_AttrInputF:
        {
          brow_GetRadiobutton( node_list[0], 0, &value);
	  ((WItemAttrInputF *)item)->set_mask( 0, !value);
          break;
        }
        case wnav_eItemType_AttrOutput:
        {
          brow_GetRadiobutton( node_list[0], 0, &value);
	  ((WItemAttrOutput *)item)->set_mask( 0, !value);
          break;
        }
        case wnav_eItemType_AttrArrayOutput:
        {
          brow_GetRadiobutton( node_list[0], 0, &value);
	  ((WItemAttrArrayOutput *)item)->set_mask( 0, !value);
          break;
        }
        default:
          ;
      }
      break;
    }
    case flow_eEvent_Key_ShiftLeft:
    {
      brow_tNode	*node_list;
      int		node_count;

      if ( wdanav->utility != wb_eUtility_PlcEditor)
        return 1;

      brow_GetSelectedNodes( wdanav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        return 1;

      brow_GetUserData( node_list[0], (void **)&item);
      switch( item->type)
      {
        case wnav_eItemType_AttrInput:
        {
          int value;

          brow_GetRadiobutton( node_list[0], 1, &value);
	  ((WItemAttrInput *)item)->set_mask( 1, !value);
          break;
        }
        case wnav_eItemType_AttrInputInv:
        {
          int value;

          brow_GetRadiobutton( node_list[0], 0, &value);
	  ((WItemAttrInputInv *)item)->set_mask( 0, !value);
          break;
        }
        default:
          ;
      }
      break;
    }
    case flow_eEvent_MB1DoubleClick:
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          switch( item->type)
          {
            case wnav_eItemType_Attr:
	      ((WItemAttr *)item)->open_children(
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_AttrArray:
	      ((WItemAttrArray *)item)->open_attributes(
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_AttrArrayOutput:
	      ((WItemAttrArrayOutput *)item)->open_attributes(
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_AttrArrayElem:
	      ((WItemAttrArrayElem *)item)->open_children(
			event->object.x, event->object.y);
              break;
            default:
              ;
          }
          break;
        default:
          ;
      }
      break;
    case flow_eEvent_Radiobutton:
    {
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          switch( item->type)
          {
            case wnav_eItemType_AttrInput: 
              if ( wdanav->utility != wb_eUtility_PlcEditor)
                break;
	      ((WItemAttrInput *)item)->set_mask( event->radiobutton.number,
			!event->radiobutton.value);
              break;
            case wnav_eItemType_AttrInputInv: 
              if ( wdanav->utility != wb_eUtility_PlcEditor)
                break;
	      ((WItemAttrInputInv *)item)->set_mask( event->radiobutton.number,
			!event->radiobutton.value);
              break;
            case wnav_eItemType_AttrInputF: 
              if ( wdanav->utility != wb_eUtility_PlcEditor)
                break;
	      ((WItemAttrInputF *)item)->set_mask( event->radiobutton.number,
			!event->radiobutton.value);
              break;
            case wnav_eItemType_AttrOutput: 
              if ( wdanav->utility != wb_eUtility_PlcEditor)
                break;
	      ((WItemAttrOutput *)item)->set_mask( event->radiobutton.number,
			!event->radiobutton.value);
              break;
            case wnav_eItemType_AttrArrayOutput: 
              if ( wdanav->utility != wb_eUtility_PlcEditor)
                break;
	      ((WItemAttrArrayOutput *)item)->set_mask( event->radiobutton.number,
			!event->radiobutton.value);
              break;
            case wnav_eItemType_Enum: 
              if ( !event->radiobutton.value)
	        ((WItemEnum *)item)->set();
              break;
            case wnav_eItemType_Mask: 
	      ((WItemMask *)item)->set( !event->radiobutton.value);
              break;
            default:
              ;
          }
          break;
        default:
          ;
      }


      break;
    }
    case flow_eEvent_Map:
    {
      wdanav->displayed = 1;
      break;
    }
    case flow_eEvent_Resized:
    {
      brow_Redraw( wdanav->brow->ctx, 0);
      break;
    }

    default:
      ;
  }
  return 1;
}

static void wdanav_trace_scan( WdaNav *wdanav)
{
  int time = 200;

  if ( wdanav->trace_started)
  {
    brow_TraceScan( wdanav->brow->ctx);

    wdanav->trace_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext(wdanav->brow_widget) , time,
	(XtTimerCallbackProc)wdanav_trace_scan, wdanav);
  }
}

int	WdaNav::get_attr()
{
  int	i, j;
  char	body[20];
  ldh_sParDef 	*bodydef;
  int sts;
  int rows;
  int elements;
  int found;
  pwr_tClassId classid_vect[2];

  if ( classid == 0)
    return WDA__SUCCESS;

  brow_SetNodraw( brow->ctx);

  found = 0;
  for ( i = 0; i < 3; i++)
  {
    if ( i == 0)
      strcpy( body, "RtBody");
    else if ( i == 1)
      strcpy( body, "DevBody");
    else
      strcpy( body, "SysBody");

    sts = ldh_GetObjectBodyDef( ldhses, classid, body, 1,
		&bodydef, &rows);
    if ( EVEN(sts))
      continue;
    for ( j = 0; j < rows; j++)
    {
      if ( cdh_NoCaseStrcmp( attribute, bodydef[j].ParName) == 0) {
        found = 1;
        break;
      }
    }
    if ( found)
      break;

    XtFree((char *) bodydef);
  }

  if ( !found) {
    brow_ResetNodraw( brow->ctx);
    return WDA__ATTRNOTFOUND;
  }

  if ( bodydef[j].Par->Output.Info.Type == pwr_eType_Buffer) {
    XtFree((char *) bodydef);
    brow_ResetNodraw( brow->ctx);
    return WDA__ATTRINVALID;
  }

  if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_ARRAY ) {
    if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_INVISIBLE ) {
      XtFree((char *) bodydef);
      brow_ResetNodraw( brow->ctx);
      return WDA__ATTRINVISIBLE;
    }
  }

  elements = bodydef[j].Par->Output.Info.Elements;

  classid_vect[0] = classid;
  classid_vect[1] = 0;
  trv_get_objects_hcn( ldhses, objid, classid_vect, NULL, wdanav_attr_found_cb,
		       (void *)this, (void *) &bodydef[j], (void *) body,
		       NULL, NULL);

  XtFree((char *) bodydef);	

  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
  return WDA__SUCCESS;
}

static int wdanav_attr_found_cb( pwr_tObjid objid, void *a1, void *a2, 
				 void *a3, void *a4, void *a5)
{
  WdaNav *wdanav = (WdaNav *) a1;
  ldh_sParDef *bodydef = (ldh_sParDef *) a2;
  char	*body = (char *) a3;


  if ( bodydef[0].Par->Output.Info.Elements <= 1)
    new WItemAttr( wdanav->brow, wdanav->ldhses, objid, NULL,
		flow_eDest_IntoLast, bodydef[0].ParName,
		bodydef[0].Par->Output.Info.Type, 
		bodydef[0].Par->Output.Info.Size,
		bodydef[0].Par->Output.Info.Flags,
		body, 1);
  else
    new WItemAttrArray( wdanav->brow, wdanav->ldhses, objid, NULL, 
		flow_eDest_IntoLast, bodydef[0].ParName,
		bodydef[0].Par->Output.Info.Elements, 
		bodydef[0].Par->Output.Info.Type, 
		bodydef[0].Par->Output.Info.Size, 
		bodydef[0].Par->Output.Info.Flags,
		body, 1);

  return 1;
}

void WdaNav::enable_events()
{
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_ObjectDeleted, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Up, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Down, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Right, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_ShiftRight, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Left, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_ShiftLeft, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_PF3, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Radiobutton, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Map, flow_eEventType_CallBack, 
	wdanav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Resized, flow_eEventType_CallBack, 
	wdanav_brow_cb);
}

//
// Backcall routine called at creation of the brow widget
// Enable event, create nodeclasses and insert the root objects.
//
static int wdanav_init_brow_cb( FlowCtx *fctx, void *client_data)
{
  WdaNav *wdanav = (WdaNav *) client_data;
  BrowCtx *ctx = (BrowCtx *)fctx;

  wdanav->brow = new WNavBrow( ctx, (void *)wdanav);

  wdanav->brow->ldh_cb_used = 0;
  wdanav->brow->brow_setup();
  wdanav->brow->create_nodeclasses();
  wdanav->enable_events();

  // Create the root item
  wdanav->get_attr();

  return 1;
}

int WdaNav::object_exist( brow_tObject object)
{
  brow_tObject 	*object_list;
  int		object_cnt;
  int		i;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    if ( object_list[i] == object)
      return 1;
  }
  return 0;
}

int WdaNav::find_by_objid( pwr_tObjid oi, brow_tObject *object)
{
  brow_tObject 	*object_list;
  int		object_cnt;
  int		i;
  WItem         *base_item;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&base_item);

    if ( cdh_ObjidIsEqual( base_item->objid, oi)) {
      *object = object_list[i];
      return 1;
    }
  }
  return 0;
}

int WdaNav::update( pwr_tObjid new_objid, pwr_tClassId new_classid,
		    char *new_attribute)
{
  int sts;
  int keep_select;
  pwr_tObjid    keep_objid;
  brow_tNode	*node_list;
  int		node_count;
  WItem		*base_item;
  brow_tObject  object;

  if ( cdh_ObjidIsEqual( objid, new_objid) &&
       classid == new_classid &&
       strcmp( attribute, new_attribute) == 0)
    return WDA__SUCCESS;

  if ( cdh_ObjidIsEqual( objid, new_objid) &&
       classid == new_classid )
    keep_select = 1;
  else
    keep_select = 0;

  if ( keep_select) {  
    brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
    if ( !node_count)
      keep_objid = pwr_cNObjid;
    else {
      brow_GetUserData( node_list[0], (void **)&base_item);
      free( node_list);

      keep_objid = base_item->objid;
    }
  }

  objid = new_objid;
  classid = new_classid;
  strcpy( attribute, new_attribute);

  brow_SetNodraw( brow->ctx);
  brow_DeleteAll( brow->ctx);
  sts = get_attr();
  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);

  if ( keep_select) {
    // Select previous selected objid
    if ( cdh_ObjidIsNotNull( keep_objid) &&
	 find_by_objid( keep_objid, &object)) {
      brow_SetInverse( object, 1);
      brow_SelectInsert( brow->ctx, object);
    }
  }
  else {
    // Scroll to top position
    if ( brow_GetFirst( brow->ctx, &object))
      brow_CenterObject( brow->ctx, object, 0.0);
  }

  return sts;
}


//
// Set attribute value
//
int WdaNav::set_attr_value( brow_tObject node, char *name, char *value_str)
{
  WItem		*base_item;
  int		sts;
  char 		buff[1024];
  int		size;

  // Check that object still exist
  if ( !object_exist( node))
    return WDA__DISAPPEARD;

  brow_GetUserData( node, (void **)&base_item);

  switch( base_item->type)
  {
    case wnav_eItemType_Attr:
    case wnav_eItemType_AttrInput:
    case wnav_eItemType_AttrInputF:
    case wnav_eItemType_AttrInputInv:
    case wnav_eItemType_AttrOutput:
    {
      WItemBaseAttr *item = (WItemBaseAttr *)base_item;

      // Check that objid is still the same
      if ( strcmp(item->attr, name) != 0)
        return WDA__DISAPPEARD;

      sts =  wnav_attr_string_to_value( ldhses, item->type_id, 
	value_str, buff, sizeof(buff), item->size);
      if ( EVEN(sts))
        message( 'E', "Input syntax error");
      else
      {
        sts = ldh_SetObjectPar( ldhses, item->objid, item->body,
		item->attr, buff, item->size);
        switch( base_item->type) {
          case wnav_eItemType_Attr:
            ((WItemAttr *)item)->update();
            break;
          case wnav_eItemType_AttrInput:
            ((WItemAttrInput *)item)->update();
            break;
          case wnav_eItemType_AttrInputF:
            ((WItemAttrInputF *)item)->update();
            break;
          case wnav_eItemType_AttrInputInv:
            ((WItemAttrInputInv *)item)->update();
            break;
          case wnav_eItemType_AttrOutput:
            ((WItemAttrOutput *)item)->update();
            break;
          default:
            ;
        }
      }
      return sts;
    }
    case wnav_eItemType_AttrArrayElem:
    {
      char *value;

      WItemAttrArrayElem *item = (WItemAttrArrayElem *)base_item;

      // Check that objid is still the same
      if ( strcmp( item->attr, name) != 0)
        return WDA__DISAPPEARD;

      sts =  wnav_attr_string_to_value( ldhses, item->type_id, 
	value_str, buff, sizeof(buff), item->size);
      if ( EVEN(sts))
        message( 'E', "Input syntax error");
      else
      {
        sts = ldh_GetObjectPar( ldhses, item->objid, item->body,
		item->attr, (char **)&value, &size); 
        if (EVEN(sts)) return sts;

        memcpy( value + item->element * item->size, buff, item->size);
        sts = ldh_SetObjectPar( ldhses, item->objid, item->body,
		item->attr, value, size);
        XtFree( (char *)value);

        item->update();
      }
      return sts;
    }
    case wnav_eItemType_ObjectName:
    {
      WItemObjectName *item = (WItemObjectName *)base_item;

      // Check that objid is still the same
      if ( ! cdh_ObjidIsEqual( objid, base_item->objid))
        return WDA__DISAPPEARD;

      sts = ldh_ChangeObjectName( ldhses, item->objid, value_str);
      if ( EVEN(sts)) return sts;
      item->update();
      return sts;
    }
    default:
      ;
  }
  return 1;
}

void WdaNav::redraw()
{
    brow_Redraw( brow->ctx, 0);
}

int WdaNav::select_by_name( char *name)
{
  WItem		*base_item;
  brow_tObject 	*object_list;
  int		object_cnt;
  int		i;
  int           found;
  brow_tObject  object;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  found = 0;
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&base_item);

    switch( base_item->type) {
      case wnav_eItemType_Attr:
      case wnav_eItemType_AttrInput:
      case wnav_eItemType_AttrInputF:
      case wnav_eItemType_AttrInputInv:
      case wnav_eItemType_AttrOutput:
      case wnav_eItemType_AttrArrayElem:
      {
        WItemBaseAttr *item = (WItemBaseAttr *)base_item;

	if ( strcmp( name, item->attr) == 0) {
          object = object_list[i];
          found = 1;
        }
        break;
      }
      default:
        ;
    }
    if ( found)
      break;
  }
  if ( !found)
    return WDA__ATTRNOTFOUND;

  brow_SelectClear( brow->ctx);
  brow_SetInverse( object, 1);
  brow_SelectInsert( brow->ctx, object);
  return WDA__SUCCESS;
}








