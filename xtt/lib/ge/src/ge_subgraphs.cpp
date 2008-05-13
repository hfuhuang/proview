/* 
 * Proview   $Id: ge_subgraphs.cpp,v 1.7 2008-05-13 13:49:25 claes Exp $
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

/* ge_subgraphs.cpp -- Display object info */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector.h>

#include "co_cdh.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "co_dcli.h"
#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "glow.h"
#include "glow_growctx.h"
#include "glow_growapi.h"

#include "ge_attr.h"
#include "ge_subgraphs.h"
#include "ge_graph.h"

#include "xnav_bitmap_leaf12.h"
#include "xnav_bitmap_map12.h"
#include "xnav_bitmap_openmap12.h"
#include "xnav_bitmap_attr12.h"

#define SUBGRAPHS__INPUT_SYNTAX 2
#define SUBGRAPHS__OBJNOTFOUND 4
#define SUBGRAPHS__STRINGTOLONG 6
#define SUBGRAPHS__ITEM_NOCREA 8
#define SUBGRAPHS__SUCCESS 1

static char null_str[] = "";

static void subgraphs_attr_close_cb( Attr *attrctx);
static void subgraphs_attr_redraw_cb( Attr *attrctx);
static int subgraphs_trace_scan_bc( brow_tObject object, void *p);
static int subgraphs_trace_connect_bc( brow_tObject object, char *name, char *attr, 
	flow_eTraceType type, /* flow_eDrawType color, */ void **p);
static int subgraphs_trace_disconnect_bc( brow_tObject object);

//
// Convert attribute string to value
//
void SubGraphs::message( char sev, char *text)
{
  (message_cb)( parent_ctx, sev, text);
}

//
//  Free pixmaps
//
void SubGraphsBrow::free_pixmaps()
{
  brow_FreeAnnotPixmap( ctx, pixmap_leaf);
  brow_FreeAnnotPixmap( ctx, pixmap_map);
  brow_FreeAnnotPixmap( ctx, pixmap_openmap);
  brow_FreeAnnotPixmap( ctx, pixmap_attr);
}

//
//  Create pixmaps for leaf, closed map and open map
//
void SubGraphsBrow::allocate_pixmaps()
{
	flow_sPixmapData pixmap_data;
	int i;

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_leaf12_width;
	    pixmap_data[i].height =xnav_bitmap_leaf12_height;
	    pixmap_data[i].bits =xnav_bitmap_leaf12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_leaf);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_map12_width;
	    pixmap_data[i].height =xnav_bitmap_map12_height;
	    pixmap_data[i].bits =xnav_bitmap_map12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_map);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_openmap12_width;
	    pixmap_data[i].height =xnav_bitmap_openmap12_height;
	    pixmap_data[i].bits =xnav_bitmap_openmap12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_openmap);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_attr12_width;
	    pixmap_data[i].height =xnav_bitmap_attr12_height;
	    pixmap_data[i].bits =xnav_bitmap_attr12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attr);

}


//
// Create the navigator widget
//
SubGraphs::SubGraphs(
	void *xn_parent_ctx,
	char *xn_name,
	void *xn_growctx,
	pwr_tStatus *status) :
	parent_ctx(xn_parent_ctx),
	trace_started(0), message_cb(NULL), close_cb(NULL), 
	grow_ctx(xn_growctx), attrlist(NULL)
{
  strcpy( name, xn_name);
  *status = 1;
}

//
//  Delete a nav context
//
SubGraphs::~SubGraphs()
{
}

SubGraphsBrow::~SubGraphsBrow()
{
  free_pixmaps();
}

void SubGraphs::set_inputfocus()
{
  brow_SetInputFocus( brow->ctx);
}

//
//  Return associated class of selected object
//

int SubGraphs::get_select( void **subgraph_item)
{
  brow_tNode	*node_list;
  int		node_count;
  SubGraphBaseItem *item;
  
  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return 0;

  brow_GetUserData( node_list[0], (void **)&item);
  free( node_list);

  *subgraph_item = (void *)item;
  return 1;
}

int SubGraphs::set_all_extern( int eval)
{
  brow_tNode	*node_list;
  int		node_count;
  SubGraphBaseItem *item;

  brow_GetObjectList( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return 0;
		      
  for ( int i = 0; i < node_count; i++) {
    brow_GetUserData( node_list[i], (void **)&item);
    if ( item->type == subgraphs_eItemType_SubGraph)
      ((ItemSubGraph *)item)->set_extern( eval);
  }
  return 1;
}

//
// Callbacks from brow
//
static int subgraphs_brow_cb( FlowCtx *ctx, flow_tEvent event)
{
  SubGraphs		*subgraphs;
  ItemSubGraph 		*item;

  if ( event->event == flow_eEvent_ObjectDeleted)
  {
    brow_GetUserData( event->object.object, (void **)&item);
    delete item;
    return 1;
  }

  brow_GetCtxUserData( (BrowCtx *)ctx, (void **) &subgraphs);
  subgraphs->message( ' ', null_str);
  switch ( event->event)
  {
    case flow_eEvent_Key_Up:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( subgraphs->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
        sts = brow_GetLast( subgraphs->brow->ctx, &object);
        if ( EVEN(sts)) return 1;
      }
      else
      {
        sts = brow_GetPrevious( subgraphs->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          sts = brow_GetLast( subgraphs->brow->ctx, &object);
          if ( EVEN(sts))
	  {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      brow_SelectClear( subgraphs->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( subgraphs->brow->ctx, object);
      if ( !brow_IsVisible( subgraphs->brow->ctx, object, flow_eVisible_Full))
        brow_CenterObject( subgraphs->brow->ctx, object, 0.25);
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

      brow_GetSelectedNodes( subgraphs->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
        sts = brow_GetFirst( subgraphs->brow->ctx, &object);
        if ( EVEN(sts)) return 1;
      }
      else
      {
        sts = brow_GetNext( subgraphs->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          sts = brow_GetFirst( subgraphs->brow->ctx, &object);
          if ( EVEN(sts))
	  {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      brow_SelectClear( subgraphs->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( subgraphs->brow->ctx, object);
      if ( !brow_IsVisible( subgraphs->brow->ctx, object, flow_eVisible_Full))
        brow_CenterObject( subgraphs->brow->ctx, object, 0.75);
      if ( node_count)
        free( node_list);
      break;
    }
    case flow_eEvent_SelectClear:
      brow_ResetSelectInverse( subgraphs->brow->ctx);
      break;
    case flow_eEvent_MB1Click:
      // Select
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          if ( brow_FindSelectedObject( subgraphs->brow->ctx, event->object.object))
          {
            brow_SelectClear( subgraphs->brow->ctx);
          }
          else
          {
            brow_SelectClear( subgraphs->brow->ctx);
            brow_SetInverse( event->object.object, 1);
            brow_SelectInsert( subgraphs->brow->ctx, event->object.object);
          }
          break;
        default:
          brow_SelectClear( subgraphs->brow->ctx);
      }
      break;
    case flow_eEvent_Key_Return:
    case flow_eEvent_Key_Right:
    {
      brow_tNode	*node_list;
      int		node_count;

      brow_GetSelectedNodes( subgraphs->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        break;
      brow_GetUserData( node_list[0], (void **)&item);
      free( node_list);
      switch( item->type)
      {
        case subgraphs_eItemType_SubGraph: 
          break;
        default:
          ;
      }
      break;
    }
    case flow_eEvent_Key_PF4:
    case flow_eEvent_Key_Left:
    {
      brow_tNode	*node_list;
      int		node_count;

      brow_GetSelectedNodes( subgraphs->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        break;
      if ( !node_count)
        break;
      brow_GetUserData( node_list[0], (void **)&item);
      switch( item->type)
      {
        case subgraphs_eItemType_SubGraph: 
          break;
        default:
          ;
      }
      brow_SelectClear( subgraphs->brow->ctx);
      brow_SetInverse( node_list[0], 1);
      brow_SelectInsert( subgraphs->brow->ctx, node_list[0]);
      if ( !brow_IsVisible( subgraphs->brow->ctx, node_list[0], flow_eVisible_Full))
        brow_CenterObject( subgraphs->brow->ctx, node_list[0], 0.25);
      free( node_list);
      break;
    }
    case flow_eEvent_MB1DoubleClick:
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          switch( item->type)
          {
            case subgraphs_eItemType_SubGraph: 
              subgraphs->edit_attributes( item->nodeclass);
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


//
// Create nodeclasses
//
void SubGraphsBrow::create_nodeclasses()
{
  allocate_pixmaps();

  // Create common-class

  brow_CreateNodeClass( ctx, "NavigatorDefault", 
		flow_eNodeGroup_Common, &nc_object);
  brow_AddFrame( nc_object, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_object, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_object, 1, 1.1, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_object, 2, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_object, 9, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_object, 11, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create attribute nodeclass

  brow_CreateNodeClass( ctx, "NavigatorAttr", 
		flow_eNodeGroup_Common, &nc_attr);
  brow_AddFrame( nc_attr, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_attr, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_attr, 2, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_attr, 8, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create table nodeclass

  brow_CreateNodeClass( ctx, "NavigatorTable", 
		flow_eNodeGroup_Common, &nc_table);
  brow_AddFrame( nc_table, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_table, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_table, 1, 1.1, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_table, 2, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table, 8, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 12, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 16, 0.6, 3,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 20, 0.6, 4,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 24, 0.6, 5,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 28, 0.6, 6,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 32, 0.6, 7,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 35, 0.6, 8,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 38, 0.6, 9,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create Header

  brow_CreateNodeClass( ctx, "NavigatorHead", 
		flow_eNodeGroup_Common, &nc_header);
  brow_AddFrame( nc_header, 0, 0, 20, 0.8, flow_eDrawType_LineGray, 2, 1);
  brow_AddAnnotPixmap( nc_header, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_header, 2, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_header, 8, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create TableHeader

  brow_CreateNodeClass( ctx, "NavigatorTableHead", 
		flow_eNodeGroup_Common, &nc_table_header);
  brow_AddFrame( nc_table_header, 0, 0, 20, 0.8, flow_eDrawType_LineGray, 2, 1);
  brow_AddAnnotPixmap( nc_table_header, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_table_header, 2, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 8, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 12, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 16, 0.6, 3,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 20, 0.6, 4,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 24, 0.6, 5,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 28, 0.6, 6,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 32, 0.6, 7,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 35, 0.6, 8,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 38, 0.6, 9,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
}

int	SubGraphs::object_attr()
{
  int	i, j;
  grow_tNodeClass *list;
  grow_tNodeClass *list_p;
  int	list_cnt;
  char	name[80];

  grow_GetNodeClassList( (grow_tCtx) grow_ctx, &list, &list_cnt);
  brow_SetNodraw( brow->ctx);

  list_p = list;
  for ( i = 0; i < list_cnt; i++)
  {

    grow_sAttrInfo	*grow_info, *grow_info_p;
    int			grow_info_cnt;
    int			*extern_p;

    grow_GetObjectAttrInfo( (grow_tObject)*list_p, NULL, &grow_info, &grow_info_cnt);

    grow_info_p = grow_info;
    extern_p = 0;
    for ( j = 0; j < grow_info_cnt; j++)
    {
      if ( strcmp( "Extern", grow_info_p->name) == 0)
      {
        extern_p = (int *)grow_info_p->value_p;
        break;
      }
      grow_info_p++;
    }
    grow_FreeObjectAttrInfo( grow_info);

    grow_GetNodeClassName( *list_p, name);

    new ItemSubGraph( this, name, extern_p, *list_p,
	NULL, flow_eDest_IntoLast);
    list_p++;
  }
    
  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
  return SUBGRAPHS__SUCCESS;
}

void SubGraphsBrow::brow_setup()
{
  brow_sAttributes brow_attr;
  unsigned long mask;

  mask = 0;
  mask |= brow_eAttr_indentation;
  brow_attr.indentation = 0.5;
  mask |= brow_eAttr_annotation_space;
  brow_attr.annotation_space = 0.5;
  brow_SetAttributes( ctx, &brow_attr, mask); 
  brow_SetCtxUserData( ctx, subgraphs);

  brow_EnableEvent( ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PF4, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Return, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Right, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Left, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ObjectDeleted, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Up, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Down, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PF3, flow_eEventType_CallBack, 
	subgraphs_brow_cb);
}

//
// Backcall routine called at creation of the brow widget
// Enable event, create nodeclasses and insert the root objects.
//
int SubGraphs::init_brow_cb( FlowCtx *fctx, void *client_data)
{
  SubGraphs *subgraphs = (SubGraphs *) client_data;
  BrowCtx *ctx = (BrowCtx *)fctx;
  int sts;

  subgraphs->brow = new SubGraphsBrow( ctx, (void *)subgraphs);

  subgraphs->brow->brow_setup();
  subgraphs->brow->create_nodeclasses();

  // Create the root item
  subgraphs->object_attr();

  sts = brow_TraceInit( ctx, subgraphs_trace_connect_bc, 
		subgraphs_trace_disconnect_bc, subgraphs_trace_scan_bc);
  subgraphs->trace_started = 1;

  subgraphs->trace_start();

  return 1;
}

ItemSubGraph::ItemSubGraph( SubGraphs *subgraphs, 
	char *item_name, int *item_extern_p, void *item_nodeclass,
	brow_tNode dest, flow_eDest dest_code) :
	SubGraphBaseItem(subgraphs_eItemType_SubGraph), 
	nodeclass(item_nodeclass), extern_p( item_extern_p), 
	old_extern(0), first_scan(0)
	
{

  strcpy( name, item_name);

  brow_CreateNode( subgraphs->brow->ctx, item_name, 
		subgraphs->brow->nc_object, 
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, subgraphs->brow->pixmap_leaf);

  brow_SetAnnotation( node, 0, item_name, strlen(item_name));

  if ( *extern_p)
    brow_SetAnnotation( node, 1, "Extern", strlen("Extern"));
  else
    brow_SetAnnotation( node, 1, "Intern", strlen("Intern"));
  brow_SetTraceAttr( node, item_name, "", flow_eTraceType_User);
}

int SubGraphs::edit_attributes( void *object)
{
  attr_sItem	items[10];
  int		i;
  grow_sAttrInfo	*grow_info, *grow_info_p;
  int			grow_info_cnt;
  Attr		        *attr;
  subgraphs_tAttr 	attrlist_p;
  int			trace_type;
  int			dyn_action_type;

  grow_GetObjectAttrInfo( (grow_tObject)object, NULL, &grow_info, &grow_info_cnt);
  grow_GetNodeClassDynType( object, &trace_type, &dyn_action_type);

  grow_info_p = grow_info;
  for ( i = 0; i < grow_info_cnt; i++)
  {
    items[i].value = grow_info_p->value_p;
    strcpy( items[i].name, grow_info_p->name);
    if ( grow_info_p->type == glow_eType_TraceColor)
    {
      switch ( trace_type)
      {
        case graph_eTrace_DigTone:
        case graph_eTrace_DigToneWithError:
          items[i].type = glow_eType_ToneOrColor;
          break;
        default:
          items[i].type = glow_eType_Color;
      }
    }
    else
      items[i].type = grow_info_p->type;
    items[i].size = grow_info_p->size;
    items[i].minlimit = 0;
    items[i].maxlimit = 0;
    if ( strcmp( grow_info_p->name, "Extern") == 0)
      items[i].noedit = 0;
    else
      items[i].noedit = 1;
    items[i].multiline = grow_info_p->multiline;
    grow_info_p++;
  }

  attr = new_attr( object, items, i);
  attr->client_data = (void *)grow_info;
  attr->close_cb = subgraphs_attr_close_cb;
  attr->redraw_cb = subgraphs_attr_redraw_cb;

  // Insert in attrlist
  attrlist_p = (subgraphs_tAttr) calloc( 1, sizeof( *attrlist_p));
  attrlist_p->attrctx = attr;
  attrlist_p->next = attrlist;
  attrlist = attrlist_p;
  return 1;
}

static void subgraphs_attr_close_cb( Attr *attrctx)
{
  SubGraphs *subgraphs = (SubGraphs *) attrctx->parent_ctx;
  subgraphs_tAttr attrlist_p, prev_p;

//  if ( attrctx->object)
//    grow_FreeObjectAttrInfo( (grow_sAttrInfo *)attrctx->client_data);
//  else
//    grow_FreeSubGraphAttrInfo( (grow_sAttrInfo *)attrctx->client_data);

  // Remove from attrlist
  attrlist_p = subgraphs->attrlist;
  prev_p = NULL;
  while( attrlist_p)
  {
    if ( attrlist_p->attrctx == attrctx)
    {
      if ( prev_p)
        prev_p->next = attrlist_p->next;
      else
        subgraphs->attrlist = attrlist_p->next;
      free( (char *)attrlist_p);
      break;
    }
    prev_p = attrlist_p;
    attrlist_p = attrlist_p->next;    
  }

  delete attrctx;
}

static void subgraphs_attr_redraw_cb( Attr *attrctx)
{
  printf( "Here in attr redraw\n");
}


static int subgraphs_trace_scan_bc( brow_tObject object, void *p)
{
  SubGraphBaseItem	*base_item;
  char		buf[20];
  int		len;

  brow_GetUserData( object, (void **)&base_item);
  switch( base_item->type)
  {
    case subgraphs_eItemType_SubGraph:
    {
      ItemSubGraph	*item;

      item = (ItemSubGraph *)base_item;
      if ( !item->first_scan)
      {
        if ( *item->extern_p == item->old_extern)
          // No change since last time
          return 1;
      }
      else
        item->first_scan = 0;

      if ( *item->extern_p)
        len = sprintf( buf, "Extern");
      else
        len = sprintf( buf, "Intern");
      brow_SetAnnotation( object, 1, buf, len);
      item->old_extern = *item->extern_p;
      break;
    }
    default:
      ;
  }
  return 1;
}

static int subgraphs_trace_connect_bc( brow_tObject object, char *name, char *attr, 
	flow_eTraceType type, /* flow_eDrawType color, */ void **p)
{
  SubGraphBaseItem 	*base_item;

  if ( strcmp(name,"") == 0)
    return 1;

  brow_GetUserData( object, (void **)&base_item);
  switch( base_item->type)
  {
    case subgraphs_eItemType_SubGraph:
    {
      ItemSubGraph	*item;

      item = (ItemSubGraph *) base_item;
      *p = item->extern_p;
      break;
    }
    default:
      ;
  }      
  return 1;
}

static int subgraphs_trace_disconnect_bc( brow_tObject object)
{
  SubGraphBaseItem 		*base_item;

  brow_GetUserData( object, (void **)&base_item);
  switch( base_item->type)
  {
    default:
      ;
  }
  return 1;
}


