/* wb_wtt.cpp -- Display plant and node hiererachy

   PROVIEW/R  
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#include "flow_std.h"

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "wb_ldh.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_types.h"
#include "pwr_baseclasses.h"
#include "co_dcli.h"
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
#include "wb_wtt.h"
#include "wb_wnav.h"
#include "wb_wnav_item.h"
#include "wb_wnav_msg.h"
#include "co_nav_msg.h"


//
// Definition of item classes
// The following classes are defined:
//    WItem		superclass.
//    WItemObject	Common object.

//
// Member functions for WItem classes
//




//
// Member functions for WItemObject
//
WItemObject::WItemObject( WNav *wnav, pwr_tObjid item_objid, 
	brow_tNode dest, flow_eDest dest_code, int item_is_root) :
	WItemBaseObject( item_objid, item_is_root)
{
  int sts;
  char	segname[120];
  pwr_tObjid child;
  pwr_tClassId classid;
  char	*descr;
  int	size;
  int	next_annot, next_pixmap;
  ldh_sRefInfo	ref_info;
  char	buf[80];

  type = wnav_eItemType_Object;

  sts = ldh_ObjidToName( wnav->ldhses, objid, ldh_eName_Hierarchy, 
	name, sizeof(name), &size);
  if ( !is_root)
  {
    sts = ldh_GetObjectClass( wnav->ldhses, objid, &classid);
    sts = ldh_ObjidToName( wnav->ldhses, objid, ldh_eName_Object, 
	segname, sizeof(segname), &size);
    brow_CreateNode( wnav->brow->ctx, segname, wnav->brow->nc_multiobject, 
		dest, dest_code, (void *) this, 1, &node);

    // Set pixmap
    sts = ldh_GetChildMnt( wnav->ldhses, objid, &child);
    if( ODD(sts))
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_map);
    else
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_leaf);
    // Set object name annotation
    brow_SetAnnotation( node, 0, segname, strlen(segname));

    next_annot = 1;

    // Set class annotation
    if ( wnav->gbl.show_class)
    {
      sts = ldh_ObjidToName( wnav->ldhses, cdh_ClassIdToObjid( classid), ldh_eName_Object,
		  segname, sizeof(segname), &size);
      brow_SetAnnotation( node, next_annot++, segname, strlen(segname));
    }

    // Set description annotation
    if ( wnav->gbl.show_descrip)
    {
      sts = ldh_GetObjectPar( wnav->ldhses, objid, "RtBody", "Description",
		&descr, &size);
      if ( EVEN(sts))
        sts = ldh_GetObjectPar( wnav->ldhses, objid, "DevBody", "Description",
		&descr, &size);
      if ( EVEN(sts))
        sts = ldh_GetObjectPar( wnav->ldhses, objid, "SysBody", "Description",
		&descr, &size);
      if ( ODD(sts))
      {
        brow_SetAnnotation( node, next_annot++, descr, strlen(descr));
        free( descr);
      }
    }
    if ( wnav->gbl.show_alias && classid == pwr_eClass_Alias)
    {
       char alias_name[80];
       pwr_tObjid *ref_object;

       sts = ldh_GetObjectPar( wnav->ldhses, objid, "RtBody", 
		"Object", (char **)&ref_object, &size);
       if (ODD(sts)) 
       {
         sts = ldh_ObjidToName( wnav->ldhses, *ref_object, ldh_eName_Hierarchy,
		alias_name, sizeof(alias_name), &size);
         if ( EVEN(sts))
           strcpy( alias_name, "-");
         free((char *) ref_object);
         brow_SetAnnotation( node, next_annot++, alias_name, strlen(alias_name));
       }
    }

    next_pixmap = 2;
    next_annot = next_pixmap + 2;

    if ( wnav->gbl.show_objref || wnav->gbl.show_objxref || 
	 wnav->gbl.show_attrref || wnav->gbl.show_attrxref) 
      sts = ldh_GetReferenceInfo( wnav->ldhses, objid, &ref_info);

    if ( wnav->gbl.show_objref)
    {
      if ( ref_info.ObjRef.Total)
      {
        if ( ref_info.ObjRef.Errors)
          brow_SetAnnotPixmap( node, next_pixmap++, wnav->brow->pixmap_ref_err);
        else
          brow_SetAnnotPixmap( node, next_pixmap++, wnav->brow->pixmap_ref);
        sprintf(buf, "%d:%d", ref_info.ObjRef.Used, ref_info.ObjRef.Total);
        brow_SetAnnotation( node, next_annot++, buf, strlen(buf));
      }
      else
      {
        next_pixmap++;
        next_annot++;
      }
    }
    if ( wnav->gbl.show_objxref)
    {
      if ( ref_info.ObjXRef.Total)
      {
        if ( ref_info.ObjXRef.Errors)
          brow_SetAnnotPixmap( node, next_pixmap++, wnav->brow->pixmap_crossref_err);
        else
          brow_SetAnnotPixmap( node, next_pixmap++, wnav->brow->pixmap_crossref);
        sprintf(buf, "%d:%d", ref_info.ObjXRef.Used, ref_info.ObjXRef.Total);
        brow_SetAnnotation( node, next_annot++, buf, strlen(buf));
      }
      else
      {
        next_pixmap++;
        next_annot++;
      }
    }
    if ( wnav->gbl.show_attrref)
    {
      if ( ref_info.AttrRef.Total)
      {
        if ( ref_info.AttrRef.Errors)
          brow_SetAnnotPixmap( node, next_pixmap++, wnav->brow->pixmap_attrref_err);
        else
          brow_SetAnnotPixmap( node, next_pixmap++, wnav->brow->pixmap_attrref);
        sprintf(buf, "%d:%d", ref_info.AttrRef.Used, ref_info.AttrRef.Total);
        brow_SetAnnotation( node, next_annot++, buf, strlen(buf));
      }
      else
      {
        next_pixmap++;
        next_annot++;
      }
    }
    if ( wnav->gbl.show_attrxref)
    {
      if ( ref_info.AttrXRef.Total)
      {
        if ( ref_info.AttrXRef.Errors)
          brow_SetAnnotPixmap( node, next_pixmap++, wnav->brow->pixmap_attrxref_err);
        else
          brow_SetAnnotPixmap( node, next_pixmap++, wnav->brow->pixmap_attrxref);
        sprintf(buf, "%d:%d", ref_info.AttrXRef.Used, ref_info.AttrXRef.Total);
        brow_SetAnnotation( node, next_annot++, buf, strlen(buf));
      }
      else
      {
        next_pixmap++;
        next_annot++;
      }
    }
  }
}

int WItemBaseObject::open_children( WNav *wnav, double x, double y)
{
  double	node_x, node_y;
  pwr_tObjid	child;
  int		child_exist;
  int		sts;

  if ( cdh_ObjidIsNull( objid))
    return 1;

  if ( !is_root)
    brow_GetNodePosition( node, &node_x, &node_y);
  else 
    node_y = 0;

  if ( !is_root && brow_IsOpen( node))
  {
    // Close
    brow_SetNodraw( wnav->brow->ctx);
    brow_CloseNode( wnav->brow->ctx, node);
    if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & wnav_mOpen_Children)
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_map);
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( wnav->brow->ctx);
    brow_Redraw( wnav->brow->ctx, node_y);
  }
  else
  {
    // Create some children
    brow_SetNodraw( wnav->brow->ctx);

    child_exist = 0;
    sts = ldh_GetChildMnt( wnav->ldhses, objid, &child);
    while ( ODD(sts))
    {
      child_exist = 1;
      new WItemObject( wnav, child, node, flow_eDest_IntoLast, 0);
      sts = ldh_GetNextSibling( wnav->ldhses, child, &child);
    }

    if ( child_exist && !is_root)
    {
      brow_SetOpen( node, wnav_mOpen_Children);
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_openmap);
    }
    brow_ResetNodraw( wnav->brow->ctx);
    if ( child_exist)
      brow_Redraw( wnav->brow->ctx, node_y);
    else
     return WNAV__NOCHILD;
  }
  return 1;
}

int WItemBaseObject::open_attributes( WNav *wnav, double x, double y)
{
  double	node_x, node_y;

  if ( cdh_ObjidIsNull( objid))
    return 1;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
  {
    // Attributes is open, close
    brow_SetNodraw( wnav->brow->ctx);
    brow_CloseNode( wnav->brow->ctx, node);
    brow_ResetOpen( node, wnav_mOpen_Attributes);
    brow_RemoveAnnotPixmap( node, 1);
    brow_ResetNodraw( wnav->brow->ctx);
    brow_Redraw( wnav->brow->ctx, node_y);
  }
  else 
  {
    int			sts;
    char		parname[80];
    pwr_tClassId	classid;
    unsigned long	elements;
    ldh_sParDef 	*bodydef;
    int			rows;
    WItem 		*item;
    int			i, j;
    char		body[20];
    int			attr_exist = 0;
    int			input_cnt = 0;
    int			output_cnt = 0;
    char		*block;
    int			size;

    if ( brow_IsOpen( node) & wnav_mOpen_Children ||
	 brow_IsOpen( node) & wnav_mOpen_Crossref)
    {
      // Close children first
      brow_SetNodraw( wnav->brow->ctx);
      brow_CloseNode( wnav->brow->ctx, node);
      brow_ResetOpen( node, wnav_mOpen_Children);
      brow_ResetOpen( node, wnav_mOpen_Crossref);
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_map);
      brow_ResetNodraw( wnav->brow->ctx);
      brow_Redraw( wnav->brow->ctx, node_y);
    }

    // Create some attributes
    brow_SetNodraw( wnav->brow->ctx);


    // Display object name
    if ( wnav->editmode)
    {
      item = (WItem *) new WItemObjectName( wnav->brow, wnav->ldhses, objid, 
		node, flow_eDest_IntoLast);
      attr_exist = 1;
    }
    // Get bodydef for rtbody, devbody or sysbody

    sts = ldh_GetObjectClass( wnav->ldhses, objid, &classid);
    if ( EVEN(sts)) return sts;

    for ( i = 0; i < 3; i++)
    {
      if ( i == 0)
        strcpy( body, "RtBody");
      else if ( i == 1)
        strcpy( body, "DevBody");
      else
        strcpy( body, "SysBody");

      sts = ldh_GetObjectBodyDef( wnav->ldhses, classid, body, 1,
		&bodydef, &rows);
      if ( EVEN(sts))
        continue;
      for ( j = 0; j < rows; j++)
      {
        strcpy( parname, bodydef[j].ParName);
//        sts = ldh_GetObjectPar( ldhses, objid, body, parname,
//		(char **)&attr_p, &size);

        if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_POINTER )
          continue;
        if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_INVISIBLE )
          continue;

        if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_ARRAY )
        {
          elements = bodydef[j].Par->Output.Info.Elements;

          if ( bodydef[j].ParClass == pwr_eClass_Output)
          {
            new WItemAttrArrayOutput( wnav->brow, wnav->ldhses, objid, node, 
		flow_eDest_IntoLast,
		parname,
		bodydef[j].Par->Output.Info.Elements, 
		bodydef[j].Par->Output.Info.Type, 
		bodydef[j].Par->Output.Info.Size, 
		bodydef[j].Par->Output.Info.Flags,
		body, output_cnt);
             output_cnt++;
          }
          else
            new WItemAttrArray( wnav->brow, wnav->ldhses, objid, node, 
		flow_eDest_IntoLast,
		parname,
		bodydef[j].Par->Output.Info.Elements, 
		bodydef[j].Par->Output.Info.Type, 
		bodydef[j].Par->Output.Info.Size, 
		bodydef[j].Par->Output.Info.Flags,
		body, 0);
          attr_exist = 1;
        }
        else if ( bodydef[j].ParClass == pwr_eClass_Input)
        {
          if ( bodydef[j].Par->Input.Info.Type == pwr_eType_Boolean)
          {
	    if ( bodydef[j].Par->Input.Info.Flags & PWR_MASK_NOREMOVE &&
	         bodydef[j].Par->Input.Info.Flags & PWR_MASK_NOINVERT)
              new WItemAttr( wnav->brow, wnav->ldhses, objid, node, 
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Input.Info.Type, 
		bodydef[j].Par->Input.Info.Size,
		bodydef[j].Par->Input.Info.Flags,
		body, 0);
	    else if ( bodydef[j].Par->Input.Info.Flags & PWR_MASK_NOREMOVE)
              new WItemAttrInputInv( wnav->brow, wnav->ldhses, objid, node, 
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Input.Info.Type, 
		bodydef[j].Par->Input.Info.Size,
		bodydef[j].Par->Input.Info.Flags,
		body, input_cnt);
	    else if ( bodydef[j].Par->Input.Info.Flags & PWR_MASK_NOINVERT)
              new WItemAttrInputF( wnav->brow, wnav->ldhses, objid, node, 
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Input.Info.Type, 
		bodydef[j].Par->Input.Info.Size,
		bodydef[j].Par->Input.Info.Flags,
		body, input_cnt);
	   else
              new WItemAttrInput( wnav->brow, wnav->ldhses, objid, node,
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Input.Info.Type, 
		bodydef[j].Par->Input.Info.Size,
		bodydef[j].Par->Input.Info.Flags,
		body, input_cnt);
          }
          else
          {
	    if ( bodydef[j].Par->Input.Info.Flags & PWR_MASK_NOREMOVE)
              new WItemAttr( wnav->brow, wnav->ldhses, objid, node,
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Input.Info.Type, 
		bodydef[j].Par->Input.Info.Size,
		bodydef[j].Par->Input.Info.Flags,
		body, 0);
	    else
              new WItemAttrInputF( wnav->brow, wnav->ldhses, objid, node,
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Input.Info.Type, 
		bodydef[j].Par->Input.Info.Size,
		bodydef[j].Par->Input.Info.Flags,
		body, input_cnt);
          }
          attr_exist = 1;
          input_cnt++;
        }
        else if ( bodydef[j].ParClass == pwr_eClass_Output)
        {
	  if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_NOREMOVE)
            new WItemAttr( wnav->brow, wnav->ldhses, objid, node, 
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Output.Info.Type, 
		bodydef[j].Par->Output.Info.Size,
		bodydef[j].Par->Output.Info.Flags,
		body, 0);
          else
            new WItemAttrOutput( wnav->brow, wnav->ldhses, objid, node, 
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Output.Info.Type, 
		bodydef[j].Par->Output.Info.Size,
		bodydef[j].Par->Output.Info.Flags,
		body, output_cnt);
          attr_exist = 1;
          output_cnt++;
        }
        else
        {
          new WItemAttr( wnav->brow, wnav->ldhses, objid, node, 
		flow_eDest_IntoLast, parname,
		bodydef[j].Par->Output.Info.Type, 
		bodydef[j].Par->Output.Info.Size,
		bodydef[j].Par->Output.Info.Flags,
		body, 0);
          attr_exist = 1;
        }
      }
      free((char *) bodydef);	

      sts = ldh_GetDocBlock( wnav->ldhses, objid, &block, &size);
      if ( ODD(sts)) {
	new WItemDocBlock( wnav->brow, wnav->ldhses, objid, block, size,
			   node, flow_eDest_IntoLast);
	attr_exist = 1;
      }
    }

    if ( attr_exist && !is_root)
    {
      brow_SetOpen( node, wnav_mOpen_Attributes);
      brow_SetAnnotPixmap( node, 1, wnav->brow->pixmap_openattr);
    }
    brow_ResetNodraw( wnav->brow->ctx);
    brow_Redraw( wnav->brow->ctx, node_y);
  }
  return 1;
}

int WItemBaseObject::open_crossref( WNav *wnav, double x, double y)
{
  double	node_x, node_y;
  int		crossref_exist;
  int		sts;
  pwr_tClassId	classid;

  if ( cdh_ObjidIsNull( objid))
    return 1;

  if ( !is_root)
    brow_GetNodePosition( node, &node_x, &node_y);
  else 
    node_y = 0;

  if ( !is_root && brow_IsOpen( node))
  {
    // Close
    brow_SetNodraw( wnav->brow->ctx);
    brow_CloseNode( wnav->brow->ctx, node);
    if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & wnav_mOpen_Children)
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_map);
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( wnav->brow->ctx);
    brow_Redraw( wnav->brow->ctx, node_y);
  }
  else
  {
    // Fetch the cross reference list
    crossref_exist = 0;
    brow_SetNodraw( wnav->brow->ctx);

    sts = ldh_GetObjectClass( wnav->ldhses, objid, &classid);
    if ( EVEN(sts)) return sts;

    switch ( classid)
    {
      case pwr_cClass_Di:
      case pwr_cClass_Dv:
      case pwr_cClass_Do:
      case pwr_cClass_Po:
      case pwr_cClass_Av:
      case pwr_cClass_Ai:
      case pwr_cClass_Ao:
        sts = wnav->crr_signal( NULL, name, node);
        break;
      default:
        sts = wnav->crr_object( NULL, name, node);
    }
    if ( sts == NAV__OBJECTNOTFOUND)
      wnav->message('E', "Object not found in crossreferens file");
    else if ( sts == NAV__NOCROSSREF)
      wnav->message('I', "There is no crossreferences for this object");
    else if ( ODD(sts))
    {
      brow_SetOpen( node, wnav_mOpen_Crossref);
      crossref_exist = 1;
    }
    brow_ResetNodraw( wnav->brow->ctx);
    if ( crossref_exist)
      brow_Redraw( wnav->brow->ctx, node_y);
  }
  return 1;
}

int WItemBaseObject::close( WNav *wnav, double x, double y)
{
  double	node_x, node_y;

  if ( cdh_ObjidIsNull( objid))
    return 1;

  if ( brow_IsOpen( node))
  {
    // Close
    brow_GetNodePosition( node, &node_x, &node_y);
    brow_SetNodraw( wnav->brow->ctx);
    brow_CloseNode( wnav->brow->ctx, node);
    if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & wnav_mOpen_Children)
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_map);
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( wnav->brow->ctx);
    brow_Redraw( wnav->brow->ctx, node_y);
  }
  return 1;
}


WItemMenu::WItemMenu( WNav *wnav, char *item_name, 
	brow_tNode dest, flow_eDest dest_code, wnav_sMenu **item_child_list,
	int item_is_root) :
	WItem( pwr_cNObjid, item_is_root), child_list(item_child_list)
{
  type = wnav_eItemType_Menu;
  strcpy( name, item_name);
  if ( !is_root)
  {
    brow_CreateNode( wnav->brow->ctx, name, wnav->brow->nc_object,
		dest, dest_code, (void *)this, 1, &node);

    // Set pixmap
    if ( *child_list)
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_map);
    else
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_leaf);
    // Set object name annotation
    brow_SetAnnotation( node, 0, name, strlen(name));
  }
}

int WItemMenu::open_children( WNav *wnav, double x, double y)
{
  int		action_open;

  if ( !is_root)
  {
    if ( !brow_IsOpen( node))
      action_open = 1;
    else 
      action_open = 0;
  }
  if ( action_open || is_root)
  {
    // Display childlist
    WItem 		*item;
    wnav_sMenu		*menu;
    
    menu = *child_list;
    while ( menu)
    {
      switch ( menu->item_type)
      {
        case wnav_eItemType_Menu:
          item = (WItem *) new WItemMenu( wnav, menu->title, node, 
		flow_eDest_IntoLast, &menu->child_list,
		0);
          break;
        case wnav_eItemType_Command:
          item = (WItem *) new WItemCommand( wnav, menu->title, node, 
		flow_eDest_IntoLast, menu->command, 0,
		wnav->brow->pixmap_graph);
          break;
        default:
          ;
      }
      menu = menu->next;
      if ( !is_root)
      {
        brow_SetOpen( node, wnav_mOpen_Children);
        brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_openmap);
      }
    }
  }
  else
    close( wnav, x, y);
  return 1;
}

int WItemMenu::close( WNav *wnav, double x, double y)
{
  double	node_x, node_y;

  if ( brow_IsOpen( node))
  {
    // Close
    brow_GetNodePosition( node, &node_x, &node_y);
    brow_SetNodraw( wnav->brow->ctx);
    brow_CloseNode( wnav->brow->ctx, node);
    if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & wnav_mOpen_Children)
      brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_map);
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( wnav->brow->ctx);
    brow_Redraw( wnav->brow->ctx, node_y);
  }
  return 1;
}

WItemCommand::WItemCommand( WNav *wnav, char *item_name, 
	brow_tNode dest, flow_eDest dest_code, char *item_command,
	int item_is_root, flow_sAnnotPixmap *pixmap) :
	WItem( pwr_cNObjid, item_is_root)
{
  type = wnav_eItemType_Command;

  strcpy( command, item_command);
  strcpy( name, item_name);
  if ( !is_root)
  {
    brow_CreateNode( wnav->brow->ctx, "command_item", wnav->brow->nc_object,
		dest, dest_code, (void *)this, 1, &node);

    // Set pixmap
    brow_SetAnnotPixmap( node, 0, pixmap);
    // Set object name annotation
    brow_SetAnnotation( node, 0, name, strlen(name));
  }
}

int WItemCommand::open_children( WNav *wnav, double x, double y)
{

  wnav->command( command);
  return 1;
}


WItemLocal::WItemLocal( WNav *wnav, char *item_name, char *attr, 
	int attr_type, int attr_size, double attr_min_limit, 
	double attr_max_limit,
	void *attr_value_p, brow_tNode dest, flow_eDest dest_code) :
	WItem( pwr_cNObjid, 0), value_p(attr_value_p), first_scan(1), 
	type_id(attr_type), size(attr_size), 
	min_limit(attr_min_limit), max_limit(attr_max_limit)
{

  type = wnav_eItemType_Local;

  strcpy( name, item_name);
  memset( old_value, 0, sizeof(old_value));

//  sts = wnav->ldb.add( attr, attr_type, value_p);
//  if (EVEN(sts)) return;

  brow_CreateNode( wnav->brow->ctx, item_name, wnav->brow->nc_attr, 
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_attr);

  brow_SetAnnotation( node, 0, item_name, strlen(item_name));
  brow_SetTraceAttr( node, attr, "", flow_eTraceType_User);
}

WItemText::WItemText( WNav *wnav, char *item_name, char *text,
	brow_tNode dest, flow_eDest dest_code) :
	WItem( pwr_cNObjid, 0)
{
  type = wnav_eItemType_Text;
  strcpy( name, item_name);
  brow_CreateNode( wnav->brow->ctx, "text", wnav->brow->nc_object,
		dest, dest_code, (void *)this, 1, &node);
  brow_SetAnnotation( node, 0, text, strlen(text));
}

WItemHeader::WItemHeader( WNav *wnav, char *item_name, char *title,
	brow_tNode dest, flow_eDest dest_code) :
	WItem( pwr_cNObjid, 0)
{
  type = wnav_eItemType_Header;
  strcpy( name, item_name);
  brow_CreateNode( wnav->brow->ctx, "header", wnav->brow->nc_header,
		dest, dest_code, (void *)this, 1, &node);
  brow_SetAnnotation( node, 0, title, strlen(title));
}

WItemHeaderLarge::WItemHeaderLarge( WNav *wnav, char *item_name, char *title,
	brow_tNode dest, flow_eDest dest_code) :
	WItem( pwr_cNObjid, 0)
{
  type = wnav_eItemType_HeaderLarge;
  strcpy( name, item_name);
  brow_CreateNode( wnav->brow->ctx, "header", wnav->brow->nc_headerlarge,
		dest, dest_code, (void *)this, 1, &node);
  brow_SetAnnotation( node, 0, title, strlen(title));
}

WItemVolume::WItemVolume( WNav *wnav, pwr_tVolumeId item_volid, 
	brow_tNode dest, flow_eDest dest_code) :
	WItem( pwr_cNObjid, 0), volid(item_volid)
{
  int sts;
  char	name[120];
  char	classname[40];
  pwr_tClassId classid;
  int	size;

  type = wnav_eItemType_Volume;

  sts = ldh_VolumeIdToName( wnav->wbctx, volid, name, sizeof(name),
	    &size);

  brow_CreateNode( wnav->brow->ctx, name, wnav->brow->nc_object, 
		dest, dest_code, (void *) this, 1, &node);

  // Set pixmap
  brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_map);

  // Set object name annotation
  brow_SetAnnotation( node, 0, name, strlen(name));

  // Set class annotation
  sts = ldh_GetVolumeClass( wnav->wbctx, volid, &classid);
  switch( classid) {
    case pwr_eClass_ClassVolume: strcpy( classname, "ClassVolume"); break;
    case pwr_eClass_WorkBenchVolume: strcpy( classname, "WorkBenchVolume"); break;
    case pwr_eClass_RootVolume: strcpy( classname, "RootVolume"); break;
    case pwr_eClass_SubVolume: strcpy( classname, "SubVolume"); break;
    case pwr_eClass_SharedVolume: strcpy( classname, "SharedVolume"); break;
    case pwr_eClass_DynamicVolume: strcpy( classname, "DynamicVolume"); break;
    case pwr_eClass_DirectoryVolume: strcpy( classname, "DirectoryVolume"); break;
  }
  brow_SetAnnotation( node, 1, classname, strlen(classname));
}

int WItemVolume::close( WNav *wnav, double x, double y)
{
  return 1;
}

int WItemVolume::open_children( WNav *wnav, double x, double y)
{
  int		sts;

  if ( wnav->ldhses)
  {
    wnav->message('E', "Other volume is already open");
    return 0;
  }

  sts = (wnav->attach_volume_cb)( wnav->parent_ctx, volid, 1);
  if ( EVEN(sts)) return sts;

  return 1;
}

WItemObjectName::WItemObjectName(
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, 
	brow_tNode dest, flow_eDest dest_code) :
	WItem( item_objid, 0), brow(item_brow), ldhses(item_ldhses)
{
  int sts;
  int	size;
  char	segname[120];
  ldh_sSessInfo info;

  type = wnav_eItemType_ObjectName;

  sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Object, 
	segname, sizeof(segname), &size);
  brow_CreateNode( brow->ctx, segname, brow->nc_attr, 
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, brow->pixmap_objname);

  // Set name
  brow_SetAnnotation( node, 0, "ObjectName", strlen("ObjectName"));
  brow_SetAnnotation( node, 1, segname, strlen(segname));

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite)
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);
}

int WItemObjectName::update()
{
  char	segname[120];
  int   sts;
  int	size;

  sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Object, 
	segname, sizeof(segname), &size);
  if ( EVEN(sts)) return sts;
  brow_SetAnnotation( node, 1, segname, strlen(segname));

  return WNAV__SUCCESS;
}

int WItemObjectName::get_value( char **value)
{
  char	*segname;
  int size = 120;
  int sts;

  segname = (char *) malloc( size);
  sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Object, 
	segname, size, &size);
  if ( EVEN(sts))
  {
    free( segname);
    return sts;
  }
  *value = segname;
  return WNAV__SUCCESS;
}

WItemFile::WItemFile( WNav *wnav, char *item_name, char *text, 
	char *item_file_name, item_eFileType item_filetype,
	brow_tNode dest, flow_eDest dest_code) :
	WItem( pwr_cNObjid, 0), file_type(item_filetype)
{
  FILE *file;
  char script_descr[200];

  type = wnav_eItemType_File;
  strcpy( name, item_name);
  strcpy( file_name, item_file_name);

  if ( file_type == item_eFileType_Script)
  {
    // Read first line to find a description
    file = fopen( file_name, "r");
    if ( file)
    {
      char *s;
      char line[200];
      if ( fgets( line, sizeof(line), file)) {
        if ((s = strstr( line, wnav_cScriptDescKey)))
          strcpy( script_descr, s + strlen(wnav_cScriptDescKey) + 1);
        else if ((s = strstr( line, wnav_cScriptInvisKey))) {
          delete this;
	  return;
        }
      }
      fclose(file);
    }
  }      


  brow_CreateNode( wnav->brow->ctx, name, wnav->brow->nc_object,
		dest, dest_code, (void *)this, 1, &node);
  if ( file_type == item_eFileType_Script)
  {
    brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_script);
    if ( strcmp( script_descr, "") != 0)
      brow_SetAnnotation( node, 1, script_descr, 
		strlen(script_descr));
  }      
  else if ( file_type == item_eFileType_Graph)
    brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_graph);
  else
    brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_file);
  brow_SetAnnotation( node, 0, text, strlen(text));
}

int WItemFile::open_children( WNav *wnav, double x, double y)
{
  int sts;

  switch ( file_type)
  {
    case item_eFileType_Script:
    {
      char cmd[120];
      strcpy( cmd, "@");
      strcat( cmd, file_name);
      sts = wnav->command( cmd);
      break;
    }
    default:
      wnav->message('I', "Unknown filetype");
  }
  return 1;
}

// NEW !!

WItemBaseAttr::WItemBaseAttr(
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	char *attr_name, int attr_type_id, int attr_size, int attr_flags,
	char *attr_body) :
	WItem(item_objid, 0), brow(item_brow), ldhses(item_ldhses),
	type_id(attr_type_id), size(attr_size), flags(attr_flags)
{
  strcpy( attr, attr_name);
  strcpy( body, attr_body);
  
  ldh_GetObjectClass( ldhses, objid, &classid);
}

int WItemBaseAttr::get_value( char **value)
{
  int psize;

  return ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)value, &psize);
}

WItemAttr::WItemAttr( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, int attr_size, int attr_flags,
	char *attr_body, int fullname) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body)
{
  char	obj_name[120];
  char	annot[120];
  int	sts;
  int	psize;
  ldh_sSessInfo info;
  brow_tNodeClass nc;

  type = wnav_eItemType_Attr;

  strcpy( name, attr_name);
  strcpy( body, attr_body);
  strcpy( annot, attr_name);

  if ( fullname)
    nc = brow->nc_attr_full;
  else
    nc = brow->nc_attr;

  switch( type_id)
  {
    case pwr_eType_Objid:
      brow_CreateNode( brow->ctx, attr_name, nc, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_ref);
      break;
    case pwr_eType_Enum:
      brow_CreateNode( brow->ctx, attr_name, nc, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attrenum);
      break;
    case pwr_eType_Mask:
      brow_CreateNode( brow->ctx, attr_name, nc, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attrmask);
      break;
    case pwr_eType_Text:
      if ( fullname)
        nc = brow->nc_attr_multiline_full;
      else
        nc = brow->nc_attr_multiline;
      brow_CreateNode( brow->ctx, attr_name, nc, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
      break;
    default:
      brow_CreateNode( brow->ctx, attr_name, nc, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
  }
  sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Hierarchy, obj_name, 
		sizeof(obj_name), &psize);
  if ( fullname)
    sprintf( annot, "%s.%s", obj_name, attr_name); 

  brow_SetAnnotation( node, 0, annot, strlen(annot));

//  brow_SetTraceAttr( node, obj_name, attr_name, flow_eTraceType_User);

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemAttr::open_children( double x, double y)
{
  double	node_x, node_y;
  int		sts;

  switch( type_id)
  {
    case pwr_eType_Enum:
    case pwr_eType_Mask:
      break;
    default:
      return WNAV__NOCHILDREN;
  }

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node))
  {
    // Close
    brow_SetNodraw( brow->ctx);
    brow_CloseNode( brow->ctx, node);
    if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & wnav_mOpen_Children)
    {
      switch( type_id)
      {
        case pwr_eType_Enum:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attrenum);
          break;
        case pwr_eType_Mask:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attrmask);
          break;
        default:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
      }
    }
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  else // if ( parent && !noedit)
  {
    types_sEnumElement	*elem_p;

    if ( type_id == pwr_eType_Enum)
    {
      sts = types_find_enum( classid, name, &elem_p);
      if ( EVEN(sts)) return WNAV__NOCHILDREN;

      // Create some children
      brow_SetNodraw( brow->ctx);

      for ( ; elem_p->name[0] != 0; elem_p++)
      {
        new WItemEnum( brow, ldhses, objid, elem_p->name, attr, type_id,
		size, flags, body, elem_p->num, 0, 0,
		node, flow_eDest_IntoLast);
      }
    }
    else
    {
      sts = types_find_mask( classid, name, &elem_p);
      if ( EVEN(sts)) return WNAV__NOCHILDREN;

      // Create some children
      brow_SetNodraw( brow->ctx);

      for ( ; elem_p->name[0] != 0; elem_p++)
      {
        new WItemMask( brow, ldhses, objid, elem_p->name, attr, type_id,
		size, flags, body, (unsigned int) elem_p->num, 0, 0, 
		node, flow_eDest_IntoLast);
      }
    }

    brow_SetOpen( node, wnav_mOpen_Children);
    brow_SetAnnotPixmap( node, 0, brow->pixmap_openmap);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  return 1;
}

int WItemAttr::close( double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node))
  {
    // Close
    brow_SetNodraw( brow->ctx);
    brow_CloseNode( brow->ctx, node);
    if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & wnav_mOpen_Children)
    {
      switch( type_id)
      {
        case pwr_eType_Enum:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attrenum);
          break;
        case pwr_eType_Mask:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attrmask);
          break;
        default:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
      }
    }
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  return 1;
}

int WItemAttr::update()
{
  int 	sts;
  int	psize;
  void	*value;
  char	*buff;
  int	len;
  char	buf[80];

  // Get the attribute value
  sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize); 
  if ( EVEN(sts)) return sts;

  switch( type_id)
  {
    case pwr_eType_Enum:
      sts = types_translate_enum( classid, attr, * (unsigned int *)value, buf);
      if ( EVEN(sts))
        wnav_attrvalue_to_string( ldhses, type_id, value, &buff, &len);
      else
      {
        buff = buf;        
        len = strlen(buf);
      }
      break;
    default:
      wnav_attrvalue_to_string( ldhses, type_id, value, &buff, &len);
  }

  brow_SetAnnotation( node, 1, buff, len);
  free( (char *)value);
  return WNAV__SUCCESS;
}


WItemAttrInput::WItemAttrInput(
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, int attr_size, int attr_flags,
	char *attr_body, int attr_input_num) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body)
{
  char	obj_name[120];
  char	annot[120];
  int	sts;
  int	psize;
  ldh_sSessInfo info;

  type = wnav_eItemType_AttrInput;

  strcpy( name, attr_name);
  strcpy( body, attr_body);
  strcpy( annot, attr_name);
  mask = 1 << attr_input_num;

  brow_CreateNode( brow->ctx, attr_name, brow->nc_attr_input, 
		dest, dest_code, (void *) this, 1, &node);

  switch( type_id)
  {
    case pwr_eType_Objid:
      brow_SetAnnotPixmap( node, 0, brow->pixmap_ref);
      break;
    default:
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
  }
  brow_SetAnnotation( node, 0, annot, strlen(annot));
  sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Hierarchy, obj_name, 
		sizeof(obj_name), &psize);
//  brow_SetTraceAttr( node, obj_name, attr_name, flow_eTraceType_User);

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemAttrInput::update()
{
  int 	sts;
  int	psize;
  void	*value;
  char	*buff;
  int	len;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;

  // Get the attribute value
  sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize); 
  if ( EVEN(sts)) return sts;

  wnav_attrvalue_to_string( ldhses, type_id, value, &buff, &len);
  brow_SetAnnotation( node, 1, buff, len);
  free( (char *)value);

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( plcnode->mask[0] & mask)
    brow_SetRadiobutton( node, 0, 1);
  else
    brow_SetRadiobutton( node, 0, 0);

  // Invert mask
  if ( plcnode->mask[2] & mask)
    brow_SetRadiobutton( node, 1, 1);
  else
    brow_SetRadiobutton( node, 1, 0);

  free((char *)plcnode);

  return WNAV__SUCCESS;
}

int WItemAttrInput::set_mask( int radio_button, int value)
{
  int 	sts;
  int	psize;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;
  int		ldh_cb_used = brow->ldh_cb_used;

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( radio_button == 0)
  {
    if ( value == 1)
      plcnode->mask[0] |= mask;
    else
      plcnode->mask[0] &= ~mask;
  }
  else
  {
    if ( value == 1)
      plcnode->mask[2] |= mask;
    else
      plcnode->mask[2] &= ~mask;
  }

  sts = ldh_SetObjectBuffer( ldhses, objid, "DevBody", "PlcNode",	
	(char *)plcnode);
  if( EVEN(sts)) return sts;

  if ( !ldh_cb_used)
  {
    if ( plcnode->mask[0] & mask)
      brow_SetRadiobutton( node, 0, 1);
    else
      brow_SetRadiobutton( node, 0, 0);

  // Invert mask
    if ( plcnode->mask[2] & mask)
      brow_SetRadiobutton( node, 1, 1);
    else
      brow_SetRadiobutton( node, 1, 0);
  }

  free((char *)plcnode);
  return WNAV__SUCCESS;
}

WItemAttrInputF::WItemAttrInputF( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, int attr_size, int attr_flags,
	char *attr_body, int attr_input_num) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body)
{
  char	obj_name[120];
  char	annot[120];
  int	sts;
  int	psize;
  ldh_sSessInfo info;

  type = wnav_eItemType_AttrInputF;

  strcpy( name, attr_name);
  strcpy( body, attr_body);
  strcpy( annot, attr_name);
  mask = 1 << attr_input_num;

  brow_CreateNode( brow->ctx, attr_name, brow->nc_attr_output, 
		dest, dest_code, (void *) this, 1, &node);

  switch( type_id)
  {
    case pwr_eType_Objid:
      brow_SetAnnotPixmap( node, 0, brow->pixmap_ref);
      break;
    default:
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
  }
  brow_SetAnnotation( node, 0, annot, strlen(annot));
  sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Hierarchy, obj_name, 
		sizeof(obj_name), &psize);
//  brow_SetTraceAttr( node, obj_name, attr_name, flow_eTraceType_User);

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemAttrInputF::update()
{
  int 	sts;
  int	psize;
  void	*value;
  char	*buff;
  int	len;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;

  // Get the attribute value
  sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize); 
  if ( EVEN(sts)) return sts;

  wnav_attrvalue_to_string( ldhses, type_id, value, &buff, &len);
  brow_SetAnnotation( node, 1, buff, len);
  free( (char *)value);

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( plcnode->mask[0] & mask)
    brow_SetRadiobutton( node, 0, 1);
  else
    brow_SetRadiobutton( node, 0, 0);

  free((char *)plcnode);

  return WNAV__SUCCESS;
}

int WItemAttrInputF::set_mask( int radio_button, int value)
{
  int 	sts;
  int	psize;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;
  int		ldh_cb_used = brow->ldh_cb_used;

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( radio_button == 0)
  {
    if ( value == 1)
      plcnode->mask[0] |= mask;
    else
      plcnode->mask[0] &= ~mask;
  }

  sts = ldh_SetObjectBuffer( ldhses, objid, "DevBody", "PlcNode",	
	(char *)plcnode);
  if( EVEN(sts)) return sts;

  if ( !ldh_cb_used)
  {
    if ( plcnode->mask[0] & mask)
      brow_SetRadiobutton( node, 0, 1);
    else
      brow_SetRadiobutton( node, 0, 0);
  }
  free((char *)plcnode);
  return WNAV__SUCCESS;
}

WItemAttrInputInv::WItemAttrInputInv( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, int attr_size, int attr_flags,
	char *attr_body, int attr_input_num) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body)
{
  char	obj_name[120];
  char	annot[120];
  int	sts;
  int	psize;
  ldh_sSessInfo info;

  type = wnav_eItemType_AttrInputInv;

  strcpy( name, attr_name);
  strcpy( body, attr_body);
  strcpy( annot, attr_name);
  mask = 1 << attr_input_num;

  brow_CreateNode( brow->ctx, attr_name, brow->nc_attr_inputinv, 
		dest, dest_code, (void *) this, 1, &node);

  switch( type_id)
  {
    case pwr_eType_Objid:
      brow_SetAnnotPixmap( node, 0, brow->pixmap_ref);
      break;
    default:
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
  }
  brow_SetAnnotation( node, 0, annot, strlen(annot));
  sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Hierarchy, obj_name, 
		sizeof(obj_name), &psize);
//  brow_SetTraceAttr( node, obj_name, attr_name, flow_eTraceType_User);

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemAttrInputInv::update()
{
  int 	sts;
  int	psize;
  void	*value;
  char	*buff;
  int	len;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;

  // Get the attribute value
  sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize); 
  if ( EVEN(sts)) return sts;

  wnav_attrvalue_to_string( ldhses, type_id, value, &buff, &len);
  brow_SetAnnotation( node, 1, buff, len);
  free( (char *)value);

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( plcnode->mask[2] & mask)
    brow_SetRadiobutton( node, 0, 1);
  else
    brow_SetRadiobutton( node, 0, 0);

  free((char *)plcnode);

  return WNAV__SUCCESS;
}

int WItemAttrInputInv::set_mask( int radio_button, int value)
{
  int 	sts;
  int	psize;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;
  int		ldh_cb_used = brow->ldh_cb_used;

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( radio_button == 0)
  {
    if ( value == 1)
      plcnode->mask[2] |= mask;
    else
      plcnode->mask[2] &= ~mask;
  }

  sts = ldh_SetObjectBuffer( ldhses, objid, "DevBody", "PlcNode",	
	(char *)plcnode);
  if( EVEN(sts)) return sts;

  if ( !ldh_cb_used)
  {
    if ( plcnode->mask[2] & mask)
      brow_SetRadiobutton( node, 0, 1);
    else
      brow_SetRadiobutton( node, 0, 0);
  }
  free((char *)plcnode);
  return WNAV__SUCCESS;
}

WItemAttrOutput::WItemAttrOutput( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, int attr_size, int attr_flags,
	char *attr_body, int attr_output_num) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body)
{
  char	obj_name[120];
  char	annot[120];
  int	sts;
  int	psize;
  ldh_sSessInfo info;

  type = wnav_eItemType_AttrOutput;

  strcpy( name, attr_name);
  strcpy( body, attr_body);
  strcpy( annot, attr_name);
  mask = 1 << attr_output_num;

  brow_CreateNode( brow->ctx, attr_name, brow->nc_attr_output, 
		dest, dest_code, (void *) this, 1, &node);

  switch( type_id)
  {
    case pwr_eType_Objid:
      brow_SetAnnotPixmap( node, 0, brow->pixmap_ref);
      break;
    default:
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
  }
  brow_SetAnnotation( node, 0, annot, strlen(annot));
  sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Hierarchy, obj_name, 
		sizeof(obj_name), &psize);
//  brow_SetTraceAttr( node, obj_name, attr_name, flow_eTraceType_User);

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemAttrOutput::update()
{
  int 	sts;
  int	psize;
  void	*value;
  char	*buff;
  int	len;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;

  // Get the attribute value
  sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize); 
  if ( EVEN(sts)) return sts;

  wnav_attrvalue_to_string( ldhses, type_id, value, &buff, &len);
  brow_SetAnnotation( node, 1, buff, len);
  free( (char *)value);

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( plcnode->mask[1] & mask)
    brow_SetRadiobutton( node, 0, 1);
  else
    brow_SetRadiobutton( node, 0, 0);

  free((char *)plcnode);

  return WNAV__SUCCESS;
}

int WItemAttrOutput::set_mask( int radio_button, int value)
{
  int 	sts;
  int	psize;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;
  int		ldh_cb_used = brow->ldh_cb_used;

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( radio_button == 0)
  {
    if ( value == 1)
      plcnode->mask[1] |= mask;
    else
      plcnode->mask[1] &= ~mask;
  }

  sts = ldh_SetObjectBuffer( ldhses, objid, "DevBody", "PlcNode",	
	(char *)plcnode);
  if( EVEN(sts)) return sts;

  if ( !ldh_cb_used)
  {
    if ( plcnode->mask[1] & mask)
      brow_SetRadiobutton( node, 0, 1);
    else
      brow_SetRadiobutton( node, 0, 0);

    free((char *)plcnode);
  }

  return WNAV__SUCCESS;
}

WItemAttrArray::WItemAttrArray( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_elements, int attr_type_id,
	int attr_size, int attr_flags, char *attr_body, int fullname) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body),
	elements(attr_elements)
{
  ldh_sSessInfo info;
  char annot[120];
  int psize;
  int sts;

  type = wnav_eItemType_AttrArray;

  strcpy( name, attr_name);
  strcpy( body, attr_body);
  brow_CreateNode( brow->ctx, attr_name, brow->nc_object, 
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, brow->pixmap_attrarray);

  if ( fullname) {
    sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Hierarchy, annot, 
		sizeof(annot), &psize);
    strcat( annot, ".");
    strcat( annot, attr_name);
  }
  else
    strcpy( annot, attr_name);

  brow_SetAnnotation( node, 0, annot, strlen(annot));

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);
}

int WItemAttrArray::open_attributes( double x, double y)
{
  double	node_x, node_y;
  int		i;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
  {
    // Attributes is open, close
    brow_SetNodraw( brow->ctx);
    brow_CloseNode( brow->ctx, node);
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  else 
  {
    // Create some elements
    brow_SetNodraw( brow->ctx);

    for ( i = 0; i < elements; i++)
    {
      new WItemAttrArrayElem( brow, ldhses, objid, node, 
		flow_eDest_IntoLast, name, i, type_id, 
		size / elements, i * size / elements, flags, body);
    }

    brow_SetOpen( node, wnav_mOpen_Attributes);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  return 1;
}

int WItemAttrArray::close( double x, double y)
{
  double	node_x, node_y;

  if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
  {
    // Attributes is open, close
    brow_GetNodePosition( node, &node_x, &node_y);
    brow_SetNodraw( brow->ctx);
    brow_CloseNode( brow->ctx, node);
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  return 1;
}

WItemAttrArrayOutput::WItemAttrArrayOutput( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_elements, int attr_type_id,
	int attr_size, int attr_flags, char *attr_body, int attr_output_num) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body),
	elements(attr_elements)
{
  ldh_sSessInfo info;

  type = wnav_eItemType_AttrArrayOutput;

  strcpy( name, attr_name);
  strcpy( body, attr_body);
  mask = 1 << attr_output_num;

  brow_CreateNode( brow->ctx, attr_name, brow->nc_attr_output,
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, brow->pixmap_attrarray);
  brow_SetAnnotation( node, 0, attr_name, strlen(attr_name));

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemAttrArrayOutput::update()
{
  int 	sts;
  int	psize;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( plcnode->mask[1] & mask)
    brow_SetRadiobutton( node, 0, 1);
  else
    brow_SetRadiobutton( node, 0, 0);

  free((char *)plcnode);

  return WNAV__SUCCESS;
}

int WItemAttrArrayOutput::set_mask( int radio_button, int value)
{
  int 	sts;
  int	psize;
  pwr_eClass	eclass;
  pwr_sPlcNode	*plcnode;
  int		ldh_cb_used = brow->ldh_cb_used;

  sts = ldh_GetObjectBuffer( ldhses, objid, "DevBody", "PlcNode", &eclass,	
	(char **)&plcnode, &psize);
  if( EVEN(sts)) return sts;

  // Used mask
  if ( radio_button == 0)
  {
    if ( value == 1)
      plcnode->mask[1] |= mask;
    else
      plcnode->mask[1] &= ~mask;
  }

  sts = ldh_SetObjectBuffer( ldhses, objid, "DevBody", "PlcNode",	
	(char *)plcnode);
  if( EVEN(sts)) return sts;

  if ( !ldh_cb_used)
  {
    if ( plcnode->mask[1] & mask)
      brow_SetRadiobutton( node, 0, 1);
    else
      brow_SetRadiobutton( node, 0, 0);

    free((char *)plcnode);
  }

  return WNAV__SUCCESS;
}

int WItemAttrArrayOutput::open_attributes( double x, double y)
{
  double	node_x, node_y;
  int		i;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
  {
    // Attributes is open, close
    brow_SetNodraw( brow->ctx);
    brow_CloseNode( brow->ctx, node);
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  else 
  {
    // Create some elements
    brow_SetNodraw( brow->ctx);

    for ( i = 0; i < elements; i++)
    {
      new WItemAttrArrayElem( brow, ldhses, objid, node, 
		flow_eDest_IntoLast, name, i, type_id, 
		size / elements, i * size / elements, flags, body);
    }

    brow_SetOpen( node, wnav_mOpen_Attributes);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  return 1;
}

int WItemAttrArrayOutput::close( double x, double y)
{
  double	node_x, node_y;

  if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
  {
    // Attributes is open, close
    brow_GetNodePosition( node, &node_x, &node_y);
    brow_SetNodraw( brow->ctx);
    brow_CloseNode( brow->ctx, node);
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  return 1;
}

WItemAttrArrayElem::WItemAttrArrayElem( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_element, int attr_type_id,
	int attr_size, int attr_offset, int attr_flags, char *attr_body) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body),
	element(attr_element), offset(attr_offset)
{
  char	annot[120];
  ldh_sSessInfo info;

  type = wnav_eItemType_AttrArrayElem;

  sprintf( name, "%s[%d]", attr_name, element);
  strcpy( annot, name);
  
  switch( type_id)
  {
    case pwr_eType_Objid:
      brow_CreateNode( brow->ctx, attr_name, brow->nc_attr, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_ref);
      break;
    case pwr_eType_Enum:
      brow_CreateNode( brow->ctx, attr_name, brow->nc_attr, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attrenum);
      break;
    case pwr_eType_Mask:
      brow_CreateNode( brow->ctx, attr_name, brow->nc_attr, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attrmask);
      break;
    case pwr_eType_Text:
      brow_CreateNode( brow->ctx, attr_name, 
		brow->nc_attr_multiline, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
      break;
    default:
      brow_CreateNode( brow->ctx, attr_name, brow->nc_attr, 
		dest, dest_code, (void *) this, 1, &node);
      brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
  }
  brow_SetAnnotation( node, 0, annot, strlen(annot));

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemAttrArrayElem::get_value( char **value)
{
  int psize;
  char *attr_value;
  int sts;


  sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, &attr_value, &psize);
  if ( EVEN(sts)) return sts;

  *value = (char *) malloc( size);
  memcpy( *value, attr_value + offset, size);
  free( attr_value);

  return WNAV__SUCCESS;
}

int WItemAttrArrayElem::open_children( double x, double y)
{
  double	node_x, node_y;
  int		sts;

  switch( type_id)
  {
    case pwr_eType_Enum:
    case pwr_eType_Mask:
      break;
    default:
      return WNAV__NOCHILDREN;
  }

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node))
  {
    // Close
    brow_SetNodraw( brow->ctx);
    brow_CloseNode( brow->ctx, node);
    if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & wnav_mOpen_Children)
    {
      switch( type_id)
      {
        case pwr_eType_Enum:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attrenum);
          break;
        case pwr_eType_Mask:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attrmask);
          break;
        default:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
      }
    }
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  else // if ( parent && !noedit)
  {
    types_sEnumElement	*elem_p;

    if ( type_id == pwr_eType_Enum)
    {
      sts = types_find_enum( classid, attr, &elem_p);
      if ( EVEN(sts)) return WNAV__NOCHILDREN;

      // Create some children
      brow_SetNodraw( brow->ctx);

      for ( ; elem_p->name[0] != 0; elem_p++)
      {
        new WItemEnum( brow, ldhses, objid, elem_p->name, attr, type_id,
		size, flags, body, elem_p->num, 1, element,
		node, flow_eDest_IntoLast);
      }
    }
    else
    {
      sts = types_find_mask( classid, attr, &elem_p);
      if ( EVEN(sts)) return WNAV__NOCHILDREN;

      // Create some children
      brow_SetNodraw( brow->ctx);

      for ( ; elem_p->name[0] != 0; elem_p++)
      {
        new WItemMask( brow, ldhses, objid, elem_p->name, attr, type_id,
		size, flags, body, (unsigned int) elem_p->num, 1, element,
		node, flow_eDest_IntoLast);
      }
    }

    brow_SetOpen( node, wnav_mOpen_Children);
    brow_SetAnnotPixmap( node, 0, brow->pixmap_openmap);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  return 1;
}

int WItemAttrArrayElem::close( double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node))
  {
    // Close
    brow_SetNodraw( brow->ctx);
    brow_CloseNode( brow->ctx, node);
    if ( brow_IsOpen( node) & wnav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & wnav_mOpen_Children)
    {
      switch( type_id)
      {
        case pwr_eType_Enum:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attrenum);
          break;
        case pwr_eType_Mask:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attrmask);
          break;
        default:
          brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
      }
    }
    brow_ResetOpen( node, wnav_mOpen_All);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, node_y);
  }
  return 1;
}

int WItemAttrArrayElem::update()
{
  int 	sts;
  int	psize;
  void	*value;
  char	*buff;
  int	len;
  char	buf[80];

  // Get the attribute value
  sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize); 
  if ( EVEN(sts)) return sts;

  switch( type_id)
  {
    case pwr_eType_Enum:
      sts = types_translate_enum( classid, attr, 
	* (unsigned int *)((char *) value + size * element), buf);
      if ( EVEN(sts))
        wnav_attrvalue_to_string( ldhses, type_id, 
		(char *) value + size * element, &buff, &len);
      else
      {
        buff = buf;        
        len = strlen(buf);
      }
      break;
    default:
      wnav_attrvalue_to_string( ldhses, type_id, 
		(char *) value + size * element, &buff, &len);
  }

  brow_SetAnnotation( node, 1, buff, len);
  free( (char *)value);
  return WNAV__SUCCESS;
}

WItemEnum::WItemEnum( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, 
	char *attr_enum_name, char *attr_name, 
	int attr_type_id, int attr_size, int attr_flags, char *attr_body,
	unsigned int item_num, int item_is_element, int item_element,
	brow_tNode dest, flow_eDest dest_code) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body),
	num(item_num), is_element(item_is_element), element(item_element)
{
  ldh_sSessInfo info;

  type = wnav_eItemType_Enum;
  sprintf( name, "%s%u", attr_name, num);
  strcpy( enum_name, attr_enum_name);

  brow_CreateNode( brow->ctx, enum_name, brow->nc_enum, 
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
  brow_SetAnnotation( node, 0, enum_name, strlen(enum_name));

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemEnum::update()
{
  int 	sts;
  int	psize;
  unsigned int	*value;
  char	*buf;

  // Get the attribute value
  if ( !is_element)
  {
    sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize);
    if ( EVEN(sts)) return sts;
  }
  else
  {
    sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, &buf, &psize);
    if ( EVEN(sts)) return sts;
    value = (unsigned int *)(buf + size * element);
  }


  if ( *value == num)
    brow_SetRadiobutton( node, 0, 1);
  else
    brow_SetRadiobutton( node, 0, 0);
  if ( !is_element)
    free( (char *)value);
  else
    free( buf);
  return WNAV__SUCCESS;
}

int WItemEnum::set()
{
  int 	sts;
  WItemAttr *item;
  WItemEnum *sibling_item;
  brow_tNode parent, sibling;
  char	*buf;
  int	psize;
  int		ldh_cb_used = brow->ldh_cb_used;

  // Set the attribute value
  if ( !is_element)
  {
    sts = ldh_SetObjectPar( ldhses, objid, body,
		attr, (char *) &num, sizeof(num));
    if ( EVEN(sts)) return sts;
  }
  else
  {
    sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, &buf, &psize);
    if ( EVEN(sts)) return sts;
    * (unsigned int *)(buf + size * element) = num;

    sts = ldh_SetObjectPar( ldhses, objid, body,
		attr, buf, psize);
    if ( EVEN(sts)) return sts;

    // Warning!! the item will be deleted here by the ldh backcall
    free( buf);
  }

  if ( !ldh_cb_used)
  {
    brow_SetRadiobutton( node, 0, 1);

    // Update parent
    sts = brow_GetParent( brow->ctx, node, &parent);
    if ( EVEN(sts)) return sts;
    brow_GetUserData( parent, (void **)&item);
    if ( !is_element)
      item->update();
    else
      ((WItemAttrArrayElem *)item)->update();
  
    // Update all siblings
    sts = brow_GetChild( brow->ctx, parent, &sibling);
    while( ODD(sts))
    {
      if ( sibling != node)
      {
        brow_GetUserData( sibling, (void **)&sibling_item);
        sibling_item->update();
      }
      sts = brow_GetNextSibling( brow->ctx, sibling, &sibling);
    }
  }
  return WNAV__SUCCESS;
}

WItemMask::WItemMask( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, 
	char *attr_mask_name, char *attr_name, 
	int attr_type_id, int attr_size, int attr_flags, char *attr_body,
	unsigned int item_mask, int item_is_element, int item_element,
	brow_tNode dest, flow_eDest dest_code) :
	WItemBaseAttr( item_brow, item_ldhses, item_objid, attr_name,
	attr_type_id, attr_size, attr_flags, attr_body),
	mask(item_mask), is_element(item_is_element), element(item_element)
{
  ldh_sSessInfo info;

  type = wnav_eItemType_Mask;

  sprintf( name, "%s%u", attr_name, mask);
  strcpy( mask_name, attr_mask_name);

  brow_CreateNode( brow->ctx, mask_name, brow->nc_enum, 
		dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, brow->pixmap_attr);
  brow_SetAnnotation( node, 0, mask_name, strlen(mask_name));

  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite &&
       !(flags & PWR_MASK_NOEDIT || flags & PWR_MASK_STATE))
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);

  update();
}

int WItemMask::update()
{
  int 	sts;
  int	psize;
  unsigned int	*value;
  char	*buf;

  // Get the attribute value
  if ( !is_element)
  {
    sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize);
    if ( EVEN(sts)) return sts;
  }
  else
  {
    sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, &buf, &psize);
    if ( EVEN(sts)) return sts;
    value = (unsigned int *)(buf + size * element);
  }

  if ( *value & mask)
    brow_SetRadiobutton( node, 0, 1);
  else
    brow_SetRadiobutton( node, 0, 0);
  if ( !is_element)
    free( (char *)value);
  else
    free( buf);
  return WNAV__SUCCESS;
}

int WItemMask::set( int set_value)
{
  int 	sts;
  int	psize;
  unsigned int	*value;
  WItemAttr *item;
  brow_tNode parent;
  char	*buf;
  int	is_elem = is_element;
  int		ldh_cb_used = brow->ldh_cb_used;

  // Get the attribute value
  if ( !is_elem)
  {
    sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, (char **)&value, &psize);
    if ( EVEN(sts)) return sts;

    if ( set_value)
      *value |= mask;
    else
      *value &= ~mask;
    
    sts = ldh_SetObjectPar( ldhses, objid, body,
		attr, (char *)value, psize);
    if ( EVEN(sts)) return sts;
  }
  else
  {
    sts = ldh_GetObjectPar( ldhses, objid, body,
		attr, &buf, &psize);
    if ( EVEN(sts)) return sts;
    value = (unsigned int *)(buf + size * element);
    if ( set_value)
      *value |= mask;
    else
      *value &= ~mask;

    sts = ldh_SetObjectPar( ldhses, objid, body,
		attr, buf, psize);
    if ( EVEN(sts)) return sts;

    // Warning!! the item will be deleted here by the ldh backcall
  }

  if ( !ldh_cb_used)
  {
    if ( *value & mask)
      brow_SetRadiobutton( node, 0, 1);
    else
      brow_SetRadiobutton( node, 0, 0);
  }

  if ( !is_elem)
    free( (char *)value);
  else
    free( buf);

  if ( !ldh_cb_used)
  {
    // Update parent
    sts = brow_GetParent( brow->ctx, node, &parent);
    if ( EVEN(sts)) return sts;
    brow_GetUserData( parent, (void **)&item);
    if ( !is_element)
      item->update();
    else
      ((WItemAttrArrayElem *)item)->update();
  }
  
  return WNAV__SUCCESS;
}

WItemCrossref::WItemCrossref( WNav *wnav, char *item_ref_name, 
	char *item_ref_class, int item_write, brow_tNode dest, 
	flow_eDest dest_code) :
	WItem( pwr_cNObjid, 0), write(item_write)
{
  int sts;
  char window_name[120];
  char *s;

  type = wnav_eItemType_Crossref;

  strcpy( name, item_ref_name);
  strcpy( ref_class, item_ref_class);

  // Get window objid
  strcpy( window_name, name);
  s = strrchr( window_name, '-');
  if ( !s)
    return;

  strcpy( ref_name, s + 1);
  *s = 0;
  sts = ldh_NameToObjid( wnav->ldhses, &objid, window_name);
  if ( EVEN(sts)) return;

  brow_CreateNode( wnav->brow->ctx, "crr", wnav->brow->nc_object, 
		dest, dest_code, (void *) this, 1, &node);

  if ( write == 1)
    brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_crrwrite);
  else if ( write == 2)
  {
    brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_crrwrite);
    brow_SetAnnotPixmap( node, 1, wnav->brow->pixmap_crrread);
  }
  else
    brow_SetAnnotPixmap( node, 0, wnav->brow->pixmap_crrread);

  brow_SetAnnotation( node, 0, name, strlen(name));

  // Set ref_class annotation
  brow_SetAnnotation( node, 1, ref_class, strlen(ref_class));
}

WItemDocBlock::WItemDocBlock(
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, char *item_block, int item_size,
	brow_tNode dest, flow_eDest dest_code) :
        WItem( item_objid, 0), brow(item_brow), ldhses(item_ldhses)
{
  int	size = item_size;
  ldh_sSessInfo info;

  type = wnav_eItemType_DocBlock;

  brow_CreateNode( brow->ctx, "Documentation",
		brow->nc_attr_multiline, 
		dest, dest_code, (void *) this, 1, &node);
  brow_SetAnnotPixmap( node, 0, brow->pixmap_docblock);

  // Set name
  brow_SetAnnotation( node, 0, "Documentation", strlen("Documentation"));
  if ( item_block) {
    brow_SetAnnotation( node, 1, item_block, size);
    free( item_block);
  }
  
  // Examine access
  ldh_GetSessionInfo( ldhses, &info);
  if ( info.Access == ldh_eAccess_ReadWrite)
    brow_SetAnnotPixmap( node, 1, brow->pixmap_morehelp);
}

int WItemDocBlock::update()
{
  char  *block;
  int   sts;
  int	size;

  sts = ldh_GetDocBlock( ldhses, objid, &block, &size);
  if ( EVEN(sts)) return sts;

  if ( block) {
    brow_SetAnnotation( node, 1, block, size);
    free( block);
  }

  return WNAV__SUCCESS;
}

int WItemDocBlock::get_value( char **value)
{
  int size;

  return ldh_GetDocBlock( ldhses, objid, value, &size);
}

