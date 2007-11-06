/* 
 * Proview   $Id: wb_nav.cpp,v 1.18 2007-11-06 13:28:09 claes Exp $
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

/* wb_nav.cpp -- Display plant and node hiererachy or plc-editor */

#if defined OS_VMS && defined __ALPHA
# pragma message disable (NOSIMPINT,EXTROUENCUNNOBJ)
#endif

#if defined OS_VMS && !defined __ALPHA
# pragma message disable (LONGEXTERN)
#endif

#include <stdio.h>
#include <stdlib.h>

#include "wb_ldh.h"
#include "wb_ldh_msg.h"
#include "co_cdh.h"
#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "wb_nav.h"
#include "wb_wnav_msg.h"

#include "xnav_bitmap_leaf8.h"
#include "xnav_bitmap_leaf10.h"
#include "xnav_bitmap_leaf12.h"
#include "xnav_bitmap_leaf14.h"
#include "xnav_bitmap_leaf16.h"
#include "xnav_bitmap_leaf18.h"
#include "xnav_bitmap_leaf20.h"
#include "xnav_bitmap_leaf24.h"
#include "xnav_bitmap_map8.h"
#include "xnav_bitmap_map10.h"
#include "xnav_bitmap_map12.h"
#include "xnav_bitmap_map14.h"
#include "xnav_bitmap_map16.h"
#include "xnav_bitmap_map18.h"
#include "xnav_bitmap_map20.h"
#include "xnav_bitmap_map24.h"
#include "xnav_bitmap_openmap8.h"
#include "xnav_bitmap_openmap10.h"
#include "xnav_bitmap_openmap12.h"
#include "xnav_bitmap_openmap14.h"
#include "xnav_bitmap_openmap16.h"
#include "xnav_bitmap_openmap18.h"
#include "xnav_bitmap_openmap20.h"
#include "xnav_bitmap_openmap24.h"
#include "xnav_bitmap_ref8.h"
#include "xnav_bitmap_ref10.h"
#include "xnav_bitmap_ref12.h"
#include "xnav_bitmap_ref14.h"
#include "xnav_bitmap_ref16.h"
#include "xnav_bitmap_ref18.h"
#include "xnav_bitmap_ref20.h"
#include "xnav_bitmap_ref24.h"
#include "xnav_bitmap_crossref8.h"
#include "xnav_bitmap_crossref10.h"
#include "xnav_bitmap_crossref12.h"
#include "xnav_bitmap_crossref14.h"
#include "xnav_bitmap_crossref16.h"
#include "xnav_bitmap_crossref18.h"
#include "xnav_bitmap_crossref20.h"
#include "xnav_bitmap_crossref24.h"
#include "xnav_bitmap_openattr12.h"
#include "xnav_bitmap_attr12.h"
#include "xnav_bitmap_attrarra12.h"
#include "xnav_bitmap_attrarel12.h"
#include "xnav_bitmap_object12.h"

typedef enum {
  nav_mOpen_All		= ~0,
  nav_mOpen_Children	= 1 << 0,
  nav_mOpen_Attributes	= 1 << 1
} nav_mOpen;

//
// Definition of item classes
// The following classes are defined:
//    Item		superclass.
//    ItemObject	Object that is not of class Group, GroupRef or 
//			$ClassHier. Doesn't display it's children.
//

typedef enum {
  nav_eItemType_Object,
  nav_eItemType_Attr,
  nav_eItemType_AttrArray,
  nav_eItemType_AttrArrayElem,
  nav_eItemType_AttrObject
} nav_eItemType;

class Item {
public:
  Item( pwr_tObjid item_objid, int item_is_root) :
    type( nav_eItemType_Object), objid(item_objid), is_root(item_is_root), 
    node(NULL) {}
  int open_attributes( Nav *nav, double x, double y) { return 1;}
  int open_children( Nav *nav, double x, double y) { return 1;}
  int close( Nav *nav, double x, double y) { return 1;}
  nav_eItemType		type;
  pwr_tObjid		objid;
  int			is_root;
  brow_tNode		node;
  char	 		name[80];
};


class ItemObject : public Item {
public:
  ItemObject( Nav *nav, pwr_tObjid item_objid, 
	      brow_tNode dest, flow_eDest dest_code, int item_is_root);
  int open_children( Nav *nav, double x, double y);
  int open_attributes( Nav *nav, double x, double y);
  int close( Nav *nav, double x, double y);
};

class ItemAttr : public Item {
public:
  int	body;
  int attr_idx;
  int type_id;
  pwr_tOName aname;
  ItemAttr( Nav *nav, pwr_tObjid item_objid,
	    brow_tNode dest, flow_eDest dest_code, int attr_body,
	    int idx, char *attr_name, char *attr_aname, int attr_type_id, 
	    int item_is_root);
};

class ItemAttrArray : public Item {
public:
  int	body;
  int attr_idx;
  int elements;
  int type_id;
  pwr_tOName aname;
  int flags;
  int size;
  ItemAttrArray( Nav *nav, pwr_tObjid item_objid,
		 brow_tNode dest, flow_eDest dest_code, int attr_body,
		 int idx, char *attr_name, char *attr_aname, int attr_elements, 
		 int attr_type_id, int attr_flags, int attr_size,
		 int item_is_root);
  int     open_children( Nav *nav, double x, double y);
  int     open_attributes( Nav *nav, double x, double y);
  int	    close( Nav *nav, double x, double y);
};

class ItemAttrArrayElem : public Item {
public:
  int	body;
  int attr_idx;
  int element;
  int type_id;
  pwr_tOName aname;
  int flags;
  int size;
  ItemAttrArrayElem( Nav *nav, pwr_tObjid item_objid,
		     brow_tNode dest, flow_eDest dest_code, int attr_body,
		     int idx, char *attr_name, char *attr_aname, int attr_element, 
		     int attr_type_id, int attr_flags, int attr_size, int item_is_root);
};

class ItemAttrObject : public Item {
public:
  int type_id;
  bool is_elem;
  int idx;
  int element;
  int flags;
  int size;
  int body;
  pwr_tOName aname;
  ItemAttrObject( Nav *nav, pwr_tObjid item_objid,
		  brow_tNode dest, flow_eDest dest_code,
		  char *attr_name, char *attr_aname, int attr_type_id, int attr_size,
		  bool attr_is_elem, int attr_idx, int attr_flags, int attr_body,
		  int item_is_root);
  int open_attributes( Nav *nav, double x, double y);
  int	    close( Nav *nav, double x, double y);
};

// Prototypes of local functions
static int nav_create_object_item( Nav *nav, pwr_tObjid objid, 
				   brow_tNode dest, flow_eDest dest_code, Item **item,
				   int is_root);

//
// Member functions for Item classes
//
ItemObject::ItemObject( Nav *nav, pwr_tObjid item_objid, 
			brow_tNode dest, flow_eDest dest_code, int item_is_root) :
  Item( item_objid, item_is_root)
{
  int sts;
  char name[80];
  int size;
  pwr_tObjid child;
  pwr_tClassId classid;
  char	*descr;
  ldh_tSession ldhses;

  type = nav_eItemType_Object;
  if ( !is_root) {
    sts = ldh_ObjidToName( nav->ldhses, objid, ldh_eName_Object, 
			   name, sizeof(name), &size);

    brow_CreateNode( nav->brow_ctx, name, nav->nc_object, 
		     dest, dest_code, NULL, 1, &node);

    // Set pixmap
    sts = ldh_GetChildMnt( nav->ldhses, objid, &child);
    if( ODD(sts))
      brow_SetAnnotPixmap( node, 0, nav->pixmap_map);
    else
      brow_SetAnnotPixmap( node, 0, nav->pixmap_leaf);
    // Set object name annotation
    brow_SetAnnotation( node, 0, name, strlen(name));

    // Set class annotation
    sts = ldh_GetObjectClass( nav->ldhses, objid, &classid);

    // Objects in mounted volumes has to use its own metavolumes.
    if ( ldh_ExternObject( nav->ldhses, objid))
      ldh_OpenMntSession( nav->ldhses, objid, &ldhses);
    else
      ldhses = nav->ldhses;

    sts = ldh_ObjidToName( ldhses, cdh_ClassIdToObjid( classid),
			   ldh_eName_Object, name, sizeof(name), &size);

    if ( ldhses != nav->ldhses)
      ldh_CloseSession( ldhses);

    brow_SetAnnotation( node, 1, name, strlen(name));
    brow_SetUserData( node, (void *)this);

    // Set description annotation
    if ( nav->show_descrip) {
      sts = ldh_GetObjectPar( nav->ldhses, objid, "RtBody", "Description",
		&descr, &size);
      if ( EVEN(sts))
        sts = ldh_GetObjectPar( nav->ldhses, objid, "DevBody", "Description",
		&descr, &size);
      if ( EVEN(sts))
        sts = ldh_GetObjectPar( nav->ldhses, objid, "SysBody", "Description",
		&descr, &size);
      if ( ODD(sts)) {
        brow_SetAnnotation( node, 2, descr, strlen(descr));
        free( descr);
      }
    }
  }
}

int ItemObject::close( Nav *nav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node)) {
    // Close
    brow_SetNodraw( nav->brow_ctx);
    brow_CloseNode( nav->brow_ctx, node);
    if ( brow_IsOpen( node) & nav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & nav_mOpen_Children)
      brow_SetAnnotPixmap( node, 0, nav->pixmap_map);
    brow_ResetOpen( node, nav_mOpen_All);
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  return 1;
}

int ItemObject::open_children( Nav *nav, double x, double y)
{
  double	node_x, node_y;
  pwr_tObjid	child;
  int		child_exist;
  int		sts;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node)) {
    // Close
    brow_SetNodraw( nav->brow_ctx);
    brow_CloseNode( nav->brow_ctx, node);
    if ( brow_IsOpen( node) & nav_mOpen_Attributes)
      brow_RemoveAnnotPixmap( node, 1);
    if ( brow_IsOpen( node) & nav_mOpen_Children)
      brow_SetAnnotPixmap( node, 0, nav->pixmap_map);
    brow_ResetOpen( node, nav_mOpen_All);
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  else {
    Item *item;

    // Create some children
    brow_SetNodraw( nav->brow_ctx);

    child_exist = 0;
    sts = ldh_GetChildMnt( nav->ldhses, objid, &child);
    while ( ODD(sts)) {
      child_exist = 1;
      sts = nav_create_object_item( nav, child, node, flow_eDest_IntoLast,
				    &item, 0);
      sts = ldh_GetNextSibling( nav->ldhses, child, &child);
    }

    if ( child_exist && !is_root) {
      brow_SetOpen( node, nav_mOpen_Children);
      brow_SetAnnotPixmap( node, 0, nav->pixmap_openmap);
    }
    brow_ResetNodraw( nav->brow_ctx);
    if ( child_exist)
      brow_Redraw( nav->brow_ctx, node_y);
    else
      return WNAV__NOCHILD;
  }
  return 1;
}

int ItemObject::open_attributes( Nav *nav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & nav_mOpen_Attributes) {
    // Attributes is open, close
    brow_SetNodraw( nav->brow_ctx);
    brow_CloseNode( nav->brow_ctx, node);
    brow_ResetOpen( node, nav_mOpen_Attributes);
    brow_RemoveAnnotPixmap( node, 1);
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  else {
    Item *item;
    int		attr_exist;
    int		sts;
    int		i, j;
    ldh_sParDef *bodydef;
    int 	rows;
    pwr_tClassId classid;
    char	body[20];
    char parname[40];
    ldh_tSession ldhses;
    pwr_tAttrRef aref = cdh_ObjidToAref( objid);

    if ( brow_IsOpen( node) & nav_mOpen_Children) {
      // Close children first
      brow_SetNodraw( nav->brow_ctx);
      brow_CloseNode( nav->brow_ctx, node);
      brow_ResetOpen( node, nav_mOpen_Children);
      brow_SetAnnotPixmap( node, 0, nav->pixmap_map);
      brow_ResetNodraw( nav->brow_ctx);
      brow_Redraw( nav->brow_ctx, node_y);
    }

    // Create some attributes
    brow_SetNodraw( nav->brow_ctx);

    // Objects in mounted volumes has to use its own metavolumes.
    if ( ldh_ExternObject( nav->ldhses, objid))
      ldh_OpenMntSession( nav->ldhses, objid, &ldhses);
    else
      ldhses = nav->ldhses;

    sts = ldh_GetObjectClass( ldhses, objid, &classid);
    if ( EVEN(sts)) return sts;

    attr_exist = 0;
    for ( j = 0; j < 3; j++) {
      if ( j == 0)
	strcpy( body, "DevBody");
      else if ( j == 1)
	strcpy( body, "RtBody");
      else
	strcpy( body, "SysBody");

      sts = ldh_GetObjectBodyDef( ldhses, classid, body, 1, 
				  &bodydef, &rows);
      if ( EVEN(sts) ) continue;

      for ( i = 0; i < rows; i++) {
	if ( aref.Flags.b.Object)
	  strcpy( parname, bodydef[i].ParName);
	else {
	  strcpy( parname, name);
	  strcat( parname, ".");
	  strcat( parname, bodydef[i].ParName);
	}

	if ( bodydef[i].Par->Param.Info.Flags & PWR_MASK_INVISIBLE ||
	     bodydef[i].Par->Param.Info.Flags & PWR_MASK_RTVIRTUAL)
	  continue;

	if ( bodydef[i].Par->Output.Info.Flags & PWR_MASK_DISABLEATTR && 
	     i > 0) {
	  pwr_tDisableAttr disabled;
	  pwr_sAttrRef aar;
	  pwr_sAttrRef ar = cdh_ObjidToAref( objid);
	 
	  sts = ldh_ArefANameToAref( ldhses, &ar, parname, &aar);
	  if ( EVEN(sts)) return sts;

	  sts = ldh_AttributeDisabled( ldhses, &aar, &disabled);
	  if ( EVEN(sts)) return sts;
	
	  if ( disabled)
	    continue;
	}

	if ( bodydef[i].Par->Param.Info.Flags & PWR_MASK_ARRAY) {
	  attr_exist = 1;
	  item = (Item *) new ItemAttrArray( nav, objid, node, 
					     flow_eDest_IntoLast,
					     j, i, bodydef[i].ParName, 0,
					     bodydef[i].Par->Param.Info.Elements, 
					     bodydef[i].Par->Param.Info.Type, 
					     bodydef[i].Par->Param.Info.Flags, 
					     bodydef[i].Par->Param.Info.Size, 
					     is_root);
	}
	else if ( bodydef[i].Par->Param.Info.Flags & PWR_MASK_CLASS) {
	  attr_exist = 1;
	  item = (Item *) new ItemAttrObject( nav, objid, node, 
					      flow_eDest_IntoLast, 
					      bodydef[i].ParName, 0,
					      bodydef[i].Par->Param.Info.Type, 
					      bodydef[i].Par->Param.Info.Size, false, 0,
					      bodydef[i].Par->Param.Info.Flags, j,
					      is_root);
	}
	else {
	  attr_exist = 1;
	  item = (Item *) new ItemAttr( nav, objid, node, 
					flow_eDest_IntoLast, j, i, bodydef[i].ParName, 0,
					bodydef[i].Par->Param.Info.Type, is_root);
	} 
      }
      free( (char *)bodydef);
    }

    if ( ldhses != nav->ldhses)
      ldh_CloseSession( ldhses);

    if ( attr_exist && !is_root) {
      brow_SetOpen( node, nav_mOpen_Attributes);
      brow_SetAnnotPixmap( node, 1, nav->pixmap_openattr);
    }
    brow_ResetNodraw( nav->brow_ctx);
    if ( attr_exist)
      brow_Redraw( nav->brow_ctx, node_y);
  }
  return 1;
}

ItemAttrObject::ItemAttrObject( Nav *nav,
				pwr_tObjid item_objid,
				brow_tNode dest, flow_eDest dest_code,
				char *attr_name, char *attr_aname, int attr_type_id,
				int attr_size, bool attr_is_elem, int attr_idx,
				int attr_flags, int attr_body, int item_is_root) :
  Item( item_objid, item_is_root), type_id(attr_type_id),
  is_elem(attr_is_elem), idx(attr_idx), flags( attr_flags), body(attr_body)
{
  pwr_tOName annot;
  int psize;
  int sts;
  char *s;
  ldh_tSession ldhses;

  type = nav_eItemType_AttrObject;

  if ( !is_elem)
    strcpy( name, attr_name);
  else
    sprintf( name, "%s[%d]", attr_name, idx);

  if ( attr_aname && strcmp( attr_aname, "") != 0) {
    strcpy( aname, attr_aname);
    if ( !is_elem) {
      strcat( aname, ".");
      strcat( aname, name);
    }
    else
      sprintf( &aname[strlen(aname)], "[%d]", idx);
  }
  else
    strcpy( aname, name);

  brow_CreateNode( nav->brow_ctx, attr_name, nav->nc_object, 
		dest, dest_code, this, 1, &node);

  // Objects in mounted volumes has to use its own metavolumes.
  if ( ldh_ExternObject( nav->ldhses, objid))
    ldh_OpenMntSession( nav->ldhses, objid, &ldhses);
  else
    ldhses = nav->ldhses;

  brow_SetAnnotPixmap( node, 0, nav->pixmap_attrobject);

  if ( flags & PWR_MASK_CASTATTR) {
    // Replace tid from class definition to tid from actual attribute, TODO...
  }

  s = strrchr( name, '.');
  if ( s)
    strcpy( annot, s+1);
  else
    strcpy( annot, name);

  brow_SetAnnotation( node, 0, annot, strlen(annot));

  // Set class annotation
  sts = ldh_ObjidToName( ldhses, cdh_ClassIdToObjid( type_id), ldh_eName_Object,
			 annot, sizeof(annot), &psize);
  if ( ODD(sts))
    brow_SetAnnotation( node, 1, annot, strlen(annot));

  if ( ldhses != nav->ldhses)
    ldh_CloseSession( ldhses);

}

int ItemAttrObject::open_attributes( Nav *nav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & nav_mOpen_Attributes) {
    // Attributes is open, close
    brow_SetNodraw( nav->brow_ctx);
    brow_CloseNode( nav->brow_ctx, node);
    brow_ResetOpen( node, nav_mOpen_Attributes);
    brow_RemoveAnnotPixmap( node, 1);
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  else {
    Item *item;
    int		attr_exist;
    int		sts;
    int		i, j;
    ldh_sParDef *bodydef;
    int 	rows;
    pwr_tClassId classid;
    char	body[20];
    char parname[40];
    ldh_tSession ldhses;
    pwr_tAttrRef aref = cdh_ObjidToAref( objid);

    if ( brow_IsOpen( node) & nav_mOpen_Children) {
      // Close children first
      brow_SetNodraw( nav->brow_ctx);
      brow_CloseNode( nav->brow_ctx, node);
      brow_ResetOpen( node, nav_mOpen_Children);
      brow_SetAnnotPixmap( node, 0, nav->pixmap_map);
      brow_ResetNodraw( nav->brow_ctx);
      brow_Redraw( nav->brow_ctx, node_y);
    }

    // Create some attributes
    brow_SetNodraw( nav->brow_ctx);

    // Objects in mounted volumes has to use its own metavolumes.
    if ( ldh_ExternObject( nav->ldhses, objid))
      ldh_OpenMntSession( nav->ldhses, objid, &ldhses);
    else
      ldhses = nav->ldhses;

    classid = type_id;

    attr_exist = 0;
    for ( j = 0; j < 3; j++) {
      if ( j == 0)
	strcpy( body, "DevBody");
      else if ( j == 1)
	strcpy( body, "RtBody");
      else
	strcpy( body, "SysBody");

      sts = ldh_GetObjectBodyDef( ldhses, classid, body, 1, 
				  &bodydef, &rows);
      if ( EVEN(sts) ) continue;

      for ( i = 0; i < rows; i++) {
	strcpy( parname, name);
	strcat( parname, ".");
	strcat( parname, bodydef[i].ParName);

	if ( bodydef[i].Par->Param.Info.Flags & PWR_MASK_INVISIBLE ||
	     bodydef[i].Par->Param.Info.Flags & PWR_MASK_RTVIRTUAL)
	  continue;

	if ( bodydef[i].Par->Output.Info.Flags & PWR_MASK_DISABLEATTR && 
	     i > 0) {
	  pwr_tDisableAttr disabled;
	  pwr_sAttrRef aar;
	  pwr_sAttrRef ar = cdh_ObjidToAref( objid);
	 
	  sts = ldh_ArefANameToAref( ldhses, &ar, parname, &aar);
	  if ( EVEN(sts)) return sts;

	  sts = ldh_AttributeDisabled( ldhses, &aar, &disabled);
	  if ( EVEN(sts)) return sts;
	
	  if ( disabled)
	    continue;
	}

	if ( bodydef[i].Par->Param.Info.Flags & PWR_MASK_ARRAY) {
	  attr_exist = 1;
	  item = (Item *) new ItemAttrArray( nav, objid, node, 
					     flow_eDest_IntoLast,
					     j, i, bodydef[i].ParName, aname,
					     bodydef[i].Par->Param.Info.Elements, 
					     bodydef[i].Par->Param.Info.Type, 
					     bodydef[i].Par->Param.Info.Flags, 
					     bodydef[i].Par->Param.Info.Size, 
					     is_root);
	}
	else if ( bodydef[i].Par->Param.Info.Flags & PWR_MASK_CLASS) {
	  attr_exist = 1;
	  item = (Item *) new ItemAttrObject( nav, objid, node, 
					      flow_eDest_IntoLast, 
					      bodydef[i].ParName, aname,
					      bodydef[i].Par->Param.Info.Type, 
					      bodydef[i].Par->Param.Info.Size, false, 0,
					      bodydef[i].Par->Param.Info.Flags, j,
					      is_root);
	}
	else {
	  attr_exist = 1;
	  item = (Item *) new ItemAttr( nav, objid, node, 
					flow_eDest_IntoLast, j, i, bodydef[i].ParName,
					aname, bodydef[i].Par->Param.Info.Type, is_root);
	} 
      }
      free( (char *)bodydef);
    }

    if ( ldhses != nav->ldhses)
      ldh_CloseSession( ldhses);

    if ( attr_exist && !is_root) {
      brow_SetOpen( node, nav_mOpen_Attributes);
      brow_SetAnnotPixmap( node, 1, nav->pixmap_openattr);
    }
    brow_ResetNodraw( nav->brow_ctx);
    if ( attr_exist)
      brow_Redraw( nav->brow_ctx, node_y);
  }
  return 1;
}

int ItemAttrObject::close( Nav *nav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & nav_mOpen_Attributes) {
    // Attributes is open, close
    brow_SetNodraw( nav->brow_ctx);
    brow_CloseNode( nav->brow_ctx, node);
    brow_RemoveAnnotPixmap( node, 1);
    brow_ResetOpen( node, nav_mOpen_All);
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  return 1;
}


int ItemAttrArray::close( Nav *nav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & nav_mOpen_Attributes) {
    // Attributes is open, close
    brow_SetNodraw( nav->brow_ctx);
    brow_CloseNode( nav->brow_ctx, node);
    brow_ResetOpen( node, nav_mOpen_All);
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  return 1;
}


int ItemAttrArray::open_children( Nav *nav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & nav_mOpen_Attributes) {
    // Attributes is open, close
    brow_SetNodraw( nav->brow_ctx);
    brow_CloseNode( nav->brow_ctx, node);
    brow_ResetOpen( node, nav_mOpen_All);
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  return 1;
}

int ItemAttrArray::open_attributes( Nav *nav, double x, double y)
{
  double	node_x, node_y;
  int		i;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & nav_mOpen_Attributes) {
    // Attributes is open, close
    brow_SetNodraw( nav->brow_ctx);
    brow_CloseNode( nav->brow_ctx, node);
    brow_ResetOpen( node, nav_mOpen_All);
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  else {
    Item *item;

    // Create some elements
    brow_SetNodraw( nav->brow_ctx);

    for ( i = 0; i < elements; i++) {
      if ( flags & PWR_MASK_CLASS)
	item = (Item *) new ItemAttrObject( nav, objid, node, 
					    flow_eDest_IntoLast, name, aname, type_id, 
					    size/elements,
					    1, i, flags, body, is_root);
      else
	item = (Item *) new ItemAttrArrayElem( nav, objid, node, 
					       flow_eDest_IntoLast, body, attr_idx, name, 
					       aname, i, type_id, flags, size/elements, is_root);

    }

    if ( !is_root) {
      brow_SetOpen( node, nav_mOpen_Attributes);
    }
    brow_ResetNodraw( nav->brow_ctx);
    brow_Redraw( nav->brow_ctx, node_y);
  }
  return 1;
}

ItemAttr::ItemAttr( Nav *nav, pwr_tObjid item_objid,
		    brow_tNode dest, flow_eDest dest_code, int attr_body,
		    int idx, char *attr_name, char *attr_aname, int attr_type_id, 
		    int item_is_root) :
  Item( item_objid, item_is_root), body(attr_body),
  attr_idx(idx), type_id(attr_type_id)
{
  char type_id_name[80];
  pwr_tOName aname;

  type = nav_eItemType_Attr;

  strcpy( name, attr_name);
  if ( attr_aname && strcmp( attr_aname, "") != 0) {
    strcpy( aname, attr_aname);
    strcat( aname, ".");
    strcat( aname, name);
  }
  else
    strcpy( aname, name);

  if ( !is_root) {
    brow_CreateNode( nav->brow_ctx, attr_name, nav->nc_object, 
		     dest, dest_code, NULL, 1, &node);

    nav->type_id_to_name( type_id, type_id_name);
    switch( type_id) {
    case pwr_eType_Objid:
      brow_SetAnnotPixmap( node, 0, nav->pixmap_ref);
      break;
    default:
      brow_SetAnnotPixmap( node, 0, nav->pixmap_attr);
    }

    cdh_SuppressSuper( aname, attr_name);
    brow_SetAnnotation( node, 0, aname, strlen(aname));
    brow_SetAnnotation( node, 1, type_id_name, strlen(type_id_name));
    brow_SetUserData( node, (void *)this);
  }
}

ItemAttrArray::ItemAttrArray( Nav *nav, pwr_tObjid item_objid,
			      brow_tNode dest, flow_eDest dest_code, int attr_body,
			      int idx, char *attr_name, char *attr_aname, 
			      int attr_elements, int attr_type_id, int attr_flags, 
			      int attr_size, int item_is_root) :
  Item( item_objid, item_is_root), body(attr_body),
  attr_idx(idx), elements(attr_elements), type_id(attr_type_id), flags(attr_flags), size(attr_size)
{
  char type_id_name[80];

  type = nav_eItemType_AttrArray;

  strcpy( name, attr_name);

  if ( attr_aname && strcmp( attr_aname, "") != 0) {
    strcpy( aname, attr_aname);
    strcat( aname, ".");
    strcat( aname, name);
  }
  else
    strcpy( aname, name);

  if ( !is_root) {
    brow_CreateNode( nav->brow_ctx, attr_name, nav->nc_object, 
		     dest, dest_code, NULL, 1, &node);

    nav->type_id_to_name( type_id, type_id_name);
    brow_SetAnnotPixmap( node, 0, nav->pixmap_attrarray);
    brow_SetAnnotation( node, 0, attr_name, strlen(attr_name));
    brow_SetAnnotation( node, 1, type_id_name, strlen(type_id_name));
    brow_SetUserData( node, (void *)this);
  }
}

ItemAttrArrayElem::ItemAttrArrayElem( Nav *nav, pwr_tObjid item_objid,
				      brow_tNode dest, flow_eDest dest_code, int attr_body,
				      int idx, char *attr_name, char *attr_aname, 
				      int attr_element, int attr_type_id, int attr_flags,
				      int attr_size, int item_is_root) :
  Item( item_objid, item_is_root), body(attr_body),
  attr_idx(idx), element(attr_element), type_id(attr_type_id), flags(attr_flags), size(attr_size)
{
  char type_id_name[80];

  type = nav_eItemType_AttrArrayElem;

  sprintf( name, "%s[%d]", attr_name, element);
  
  if ( attr_aname && strcmp( attr_aname, "") != 0) {
    sprintf( aname, "%s[%d]", attr_aname, element);
  }
  else
    strcpy( aname, name);

  if ( !is_root) {
    brow_CreateNode( nav->brow_ctx, name, nav->nc_object, 
		     dest, dest_code, NULL, 1, &node);

    nav->type_id_to_name( type_id, type_id_name);
    brow_SetAnnotPixmap( node, 0, nav->pixmap_attrarrayelem);
    brow_SetAnnotation( node, 0, name, strlen(name));
    brow_SetAnnotation( node, 1, type_id_name, strlen(type_id_name));
    brow_SetUserData( node, (void *)this);
  }
}

//
// Convert type_id to name
//
void  Nav::type_id_to_name( int type_id, char *type_id_name)
{
  switch( type_id) {
  case pwr_eType_Boolean: strcpy( type_id_name, "Boolean"); break;
  case pwr_eType_Float32: strcpy( type_id_name, "Float32"); break;
  case pwr_eType_Float64: strcpy( type_id_name, "Float64"); break;
  case pwr_eType_Char: strcpy( type_id_name, "Char"); break;
  case pwr_eType_Int8: strcpy( type_id_name, "Int8"); break;
  case pwr_eType_Int16: strcpy( type_id_name, "Int16"); break;
  case pwr_eType_Int32: strcpy( type_id_name, "Int32"); break;
  case pwr_eType_UInt8: strcpy( type_id_name, "UInt8"); break;
  case pwr_eType_UInt16: strcpy( type_id_name, "UInt16"); break;
  case pwr_eType_UInt32: strcpy( type_id_name, "UInt32"); break;
  case pwr_eType_Objid: strcpy( type_id_name, "Objid"); break;
  case pwr_eType_Buffer: strcpy( type_id_name, "Buffer"); break;
  case pwr_eType_String: strcpy( type_id_name, "String"); break;
  case pwr_eType_Enum: strcpy( type_id_name, "Enum"); break;
  case pwr_eType_Struct: strcpy( type_id_name, "Struct"); break;
  case pwr_eType_Mask: strcpy( type_id_name, "Mask"); break;
  case pwr_eType_Array: strcpy( type_id_name, "Array"); break;
  case pwr_eType_Time: strcpy( type_id_name, "Time"); break;
  case pwr_eType_Text: strcpy( type_id_name, "Text"); break;
  case pwr_eType_AttrRef: strcpy( type_id_name, "AttrRef"); break;
  case pwr_eType_UInt64: strcpy( type_id_name, "UInt64"); break;
  case pwr_eType_Int64: strcpy( type_id_name, "Int64"); break;
  case pwr_eType_ClassId: strcpy( type_id_name, "ClassId"); break;
  case pwr_eType_TypeId: strcpy( type_id_name, "TypeId"); break;
  case pwr_eType_VolumeId: strcpy( type_id_name, "VolumeId"); break;
  case pwr_eType_ObjectIx: strcpy( type_id_name, "ObjectIx"); break;
  case pwr_eType_RefId: strcpy( type_id_name, "RefId"); break;
  case pwr_eType_DeltaTime: strcpy( type_id_name, "DeltaTime"); break;
  default: strcpy( type_id_name, "");
  }
}

//
//  Free pixmaps
//
void Nav::free_pixmaps()
{
  brow_FreeAnnotPixmap( brow_ctx, pixmap_leaf);
  brow_FreeAnnotPixmap( brow_ctx, pixmap_map);
  brow_FreeAnnotPixmap( brow_ctx, pixmap_openmap);
  brow_FreeAnnotPixmap( brow_ctx, pixmap_ref);
  brow_FreeAnnotPixmap( brow_ctx, pixmap_crossref);
  brow_FreeAnnotPixmap( brow_ctx, pixmap_openattr);
  brow_FreeAnnotPixmap( brow_ctx, pixmap_attr);
  brow_FreeAnnotPixmap( brow_ctx, pixmap_attrarray);
  brow_FreeAnnotPixmap( brow_ctx, pixmap_attrarrayelem);
}

//
//  Create pixmaps for leaf, closed map and open map
//
void Nav::allocate_pixmaps()
{
  flow_sPixmapData pixmap_data;
  int i;
	
  i = 0;
  pixmap_data[i].width =xnav_bitmap_leaf8_width;
  pixmap_data[i].height =xnav_bitmap_leaf8_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf8_bits;
  pixmap_data[i].width =xnav_bitmap_leaf10_width;
  pixmap_data[i].height =xnav_bitmap_leaf10_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf10_bits;
  pixmap_data[i].width =xnav_bitmap_leaf12_width;
  pixmap_data[i].height =xnav_bitmap_leaf12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf12_bits;
  pixmap_data[i].width =xnav_bitmap_leaf14_width;
  pixmap_data[i].height =xnav_bitmap_leaf14_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf14_bits;
  pixmap_data[i].width =xnav_bitmap_leaf16_width;
  pixmap_data[i].height =xnav_bitmap_leaf16_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf16_bits;
  pixmap_data[i].width =xnav_bitmap_leaf18_width;
  pixmap_data[i].height =xnav_bitmap_leaf18_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf18_bits;
  pixmap_data[i].width =xnav_bitmap_leaf20_width;
  pixmap_data[i].height =xnav_bitmap_leaf20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf20_bits;
  pixmap_data[i].width =xnav_bitmap_leaf20_width;
  pixmap_data[i].height =xnav_bitmap_leaf20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf20_bits;
  pixmap_data[i].width =xnav_bitmap_leaf24_width;
  pixmap_data[i].height =xnav_bitmap_leaf24_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf24_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, &pixmap_leaf);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_map8_width;
  pixmap_data[i].height =xnav_bitmap_map8_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map8_bits;
  pixmap_data[i].width =xnav_bitmap_map10_width;
  pixmap_data[i].height =xnav_bitmap_map10_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map10_bits;
  pixmap_data[i].width =xnav_bitmap_map12_width;
  pixmap_data[i].height =xnav_bitmap_map12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map12_bits;
  pixmap_data[i].width =xnav_bitmap_map14_width;
  pixmap_data[i].height =xnav_bitmap_map14_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map14_bits;
  pixmap_data[i].width =xnav_bitmap_map16_width;
  pixmap_data[i].height =xnav_bitmap_map16_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map16_bits;
  pixmap_data[i].width =xnav_bitmap_map18_width;
  pixmap_data[i].height =xnav_bitmap_map18_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map18_bits;
  pixmap_data[i].width =xnav_bitmap_map20_width;
  pixmap_data[i].height =xnav_bitmap_map20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map20_bits;
  pixmap_data[i].width =xnav_bitmap_map20_width;
  pixmap_data[i].height =xnav_bitmap_map20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map20_bits;
  pixmap_data[i].width =xnav_bitmap_map24_width;
  pixmap_data[i].height =xnav_bitmap_map24_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_map24_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, &pixmap_map);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_openmap8_width;
  pixmap_data[i].height =xnav_bitmap_openmap8_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap8_bits;
  pixmap_data[i].width =xnav_bitmap_openmap10_width;
  pixmap_data[i].height =xnav_bitmap_openmap10_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap10_bits;
  pixmap_data[i].width =xnav_bitmap_openmap12_width;
  pixmap_data[i].height =xnav_bitmap_openmap12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap12_bits;
  pixmap_data[i].width =xnav_bitmap_openmap14_width;
  pixmap_data[i].height =xnav_bitmap_openmap14_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap14_bits;
  pixmap_data[i].width =xnav_bitmap_openmap16_width;
  pixmap_data[i].height =xnav_bitmap_openmap16_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap16_bits;
  pixmap_data[i].width =xnav_bitmap_openmap18_width;
  pixmap_data[i].height =xnav_bitmap_openmap18_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap18_bits;
  pixmap_data[i].width =xnav_bitmap_openmap20_width;
  pixmap_data[i].height =xnav_bitmap_openmap20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap20_bits;
  pixmap_data[i].width =xnav_bitmap_openmap20_width;
  pixmap_data[i].height =xnav_bitmap_openmap20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap20_bits;
  pixmap_data[i].width =xnav_bitmap_openmap24_width;
  pixmap_data[i].height =xnav_bitmap_openmap24_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap24_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, &pixmap_openmap);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_ref8_width;
  pixmap_data[i].height =xnav_bitmap_ref8_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref8_bits;
  pixmap_data[i].width =xnav_bitmap_ref10_width;
  pixmap_data[i].height =xnav_bitmap_ref10_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref10_bits;
  pixmap_data[i].width =xnav_bitmap_ref12_width;
  pixmap_data[i].height =xnav_bitmap_ref12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref12_bits;
  pixmap_data[i].width =xnav_bitmap_ref14_width;
  pixmap_data[i].height =xnav_bitmap_ref14_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref14_bits;
  pixmap_data[i].width =xnav_bitmap_ref16_width;
  pixmap_data[i].height =xnav_bitmap_ref16_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref16_bits;
  pixmap_data[i].width =xnav_bitmap_ref18_width;
  pixmap_data[i].height =xnav_bitmap_ref18_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref18_bits;
  pixmap_data[i].width =xnav_bitmap_ref20_width;
  pixmap_data[i].height =xnav_bitmap_ref20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref20_bits;
  pixmap_data[i].width =xnav_bitmap_ref20_width;
  pixmap_data[i].height =xnav_bitmap_ref20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref20_bits;
  pixmap_data[i].width =xnav_bitmap_ref24_width;
  pixmap_data[i].height =xnav_bitmap_ref24_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_ref24_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, &pixmap_ref);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_crossref8_width;
  pixmap_data[i].height =xnav_bitmap_crossref8_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref8_bits;
  pixmap_data[i].width =xnav_bitmap_crossref10_width;
  pixmap_data[i].height =xnav_bitmap_crossref10_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref10_bits;
  pixmap_data[i].width =xnav_bitmap_crossref12_width;
  pixmap_data[i].height =xnav_bitmap_crossref12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref12_bits;
  pixmap_data[i].width =xnav_bitmap_crossref14_width;
  pixmap_data[i].height =xnav_bitmap_crossref14_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref14_bits;
  pixmap_data[i].width =xnav_bitmap_crossref16_width;
  pixmap_data[i].height =xnav_bitmap_crossref16_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref16_bits;
  pixmap_data[i].width =xnav_bitmap_crossref18_width;
  pixmap_data[i].height =xnav_bitmap_crossref18_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref18_bits;
  pixmap_data[i].width =xnav_bitmap_crossref20_width;
  pixmap_data[i].height =xnav_bitmap_crossref20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref20_bits;
  pixmap_data[i].width =xnav_bitmap_crossref20_width;
  pixmap_data[i].height =xnav_bitmap_crossref20_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref20_bits;
  pixmap_data[i].width =xnav_bitmap_crossref24_width;
  pixmap_data[i].height =xnav_bitmap_crossref24_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref24_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, &pixmap_crossref);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
  pixmap_data[i].width =xnav_bitmap_openattr12_width;
  pixmap_data[i].height =xnav_bitmap_openattr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, 
			 &pixmap_openattr);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
  pixmap_data[i].width =xnav_bitmap_attr12_width;
  pixmap_data[i].height =xnav_bitmap_attr12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, 
			 &pixmap_attr);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, 
			 &pixmap_attrarray);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, 
			 &pixmap_attrarrayelem);

  i = 0;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
  pixmap_data[i].width =xnav_bitmap_object12_width;
  pixmap_data[i].height =xnav_bitmap_object12_height;
  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;

  brow_AllocAnnotPixmap( brow_ctx, &pixmap_data, 
			 &pixmap_attrobject);

}


//
// Callbacks from brow
//
int Nav::brow_cb( FlowCtx *ctx, flow_tEvent event)
{
  Nav    	*nav;
  Item 		*item;

  brow_GetCtxUserData( (BrowCtx *)ctx, (void **) &nav);
  switch ( event->event) {
  case flow_eEvent_SelectClear:
    brow_ResetSelectInverse( nav->brow_ctx);
    break;
  case flow_eEvent_ObjectDeleted:
    brow_GetUserData( event->object.object, (void **)&item);
    delete item;
    break;
  case flow_eEvent_Key_PageDown: {
    brow_Page( nav->brow_ctx, 0.8);
    break;
  }
  case flow_eEvent_Key_PageUp: {
    brow_Page( nav->brow_ctx, -0.8);
    break;
  }
  case flow_eEvent_ScrollDown: {
    brow_Page( nav->brow_ctx, 0.1);
    break;
  }
  case flow_eEvent_ScrollUp: {
    brow_Page( nav->brow_ctx, -0.1);
    break;
  }
  case flow_eEvent_Key_Up: {
    brow_tNode	*node_list;
    int		node_count;
    brow_tObject	object;
    int		sts;

    brow_GetSelectedNodes( nav->brow_ctx, &node_list, &node_count);
    if ( !node_count) {
      if ( nav->last_selected && nav->object_exist( nav->last_selected))
	object = nav->last_selected;
      else {
	sts = brow_GetLastVisible( nav->brow_ctx, &object);
	if ( EVEN(sts)) return 1;
      }
    }
    else {
      if ( !brow_IsVisible( nav->brow_ctx, node_list[0], flow_eVisible_Partial)) {
	sts = brow_GetLastVisible( nav->brow_ctx, &object);
	if ( EVEN(sts)) return 1;
      }
      else {
	sts = brow_GetPrevious( nav->brow_ctx, node_list[0], &object);
	if ( EVEN(sts)) {
	  if ( node_count)
	    free( node_list);
	  return 1;
	}
      }
    }
    brow_SelectClear( nav->brow_ctx);
    brow_SetInverse( object, 1);
    brow_SelectInsert( nav->brow_ctx, object);
    if ( !brow_IsVisible( nav->brow_ctx, object, flow_eVisible_Full))
      brow_CenterObject( nav->brow_ctx, object, 0.25);
    if ( node_count)
      free( node_list);
    nav->last_selected = object;
    break;
  }
  case flow_eEvent_Key_Down:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( nav->brow_ctx, &node_list, &node_count);
      if ( !node_count) {
	if ( nav->last_selected && nav->object_exist( nav->last_selected))
	  object = nav->last_selected;
	else {
	  sts = brow_GetFirstVisible( nav->brow_ctx, &object);
	  if ( EVEN(sts)) return 1;
	}
      }
      else {
	if ( !brow_IsVisible( nav->brow_ctx, node_list[0], flow_eVisible_Partial)) {
	  sts = brow_GetFirstVisible( nav->brow_ctx, &object);
	  if ( EVEN(sts)) return 1;
	}
	else {
	  sts = brow_GetNext( nav->brow_ctx, node_list[0], &object);
	  if ( EVEN(sts)) {
	    if ( node_count)
	      free( node_list);
	    return 1;
	  }
	}
      }
      brow_SelectClear( nav->brow_ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( nav->brow_ctx, object);
      if ( !brow_IsVisible( nav->brow_ctx, object, flow_eVisible_Full))
	brow_CenterObject( nav->brow_ctx, object, 0.75);
      if ( node_count)
	free( node_list);
      nav->last_selected = object;
      break;
    }
  case flow_eEvent_Key_Right: {
    brow_tNode	*node_list;
    int		node_count;
    int 	sts;

    brow_GetSelectedNodes( nav->brow_ctx, &node_list, &node_count);
    if ( !node_count)
      break;
    brow_GetUserData( node_list[0], (void **)&item);
    free( node_list);
    switch( item->type) {
    case nav_eItemType_Object: 
      sts =((ItemObject *)item)->open_children( nav, 0, 0);
      if ( sts == WNAV__NOCHILD)
	((ItemObject *)item)->open_attributes( nav, 0, 0);
      break;
    case nav_eItemType_AttrArray: 
      ((ItemAttrArray *)item)->open_attributes( nav, 0, 0);
      break;
    case nav_eItemType_AttrObject: 
      ((ItemAttrObject *)item)->open_attributes( nav, 0, 0);
      break;
    default:
      ;
    }
    break;
  }
  case flow_eEvent_Key_ShiftRight: {
    brow_tNode	*node_list;
    int		node_count;

    brow_GetSelectedNodes( nav->brow_ctx, &node_list, &node_count);
    if ( !node_count)
      break;
    brow_GetUserData( node_list[0], (void **)&item);
    free( node_list);
    switch( item->type) {
    case nav_eItemType_Object: 
      ((ItemObject *)item)->open_attributes( nav, 0, 0);
      break;
    case nav_eItemType_AttrObject: 
      ((ItemAttrObject *)item)->open_attributes( nav, 0, 0);
      break;
    case nav_eItemType_AttrArray: 
      ((ItemAttrArray *)item)->open_attributes( nav, 0, 0);
      break;
    default:
      ;
    }
    break;
  }
  case flow_eEvent_Key_Left: {
    brow_tNode	*node_list;
    int		node_count;
    brow_tObject	object;
    int		sts;

    brow_GetSelectedNodes( nav->brow_ctx, &node_list, &node_count);
    if ( !node_count)
      return 1;

    if ( brow_IsOpen( node_list[0]))
      // Close this node
      object = node_list[0];
    else {
      // Close parent
      sts = brow_GetParent( nav->brow_ctx, node_list[0], &object);
      if ( EVEN(sts)) {
	free( node_list);
	return 1;
      }
    }
    brow_GetUserData( object, (void **)&item);
    switch( item->type) {
    case nav_eItemType_Object: 
      ((ItemObject *)item)->close( nav, 0, 0);
      break;
    case nav_eItemType_AttrObject: 
      ((ItemAttrObject *)item)->close( nav, 0, 0);
      break;
    case nav_eItemType_AttrArray: 
      ((ItemAttrArray *)item)->close( nav, 0, 0);
      break;
    default:
      ;
    }
    brow_SelectClear( nav->brow_ctx);
    brow_SetInverse( object, 1);
    brow_SelectInsert( nav->brow_ctx, object);
    if ( !brow_IsVisible( nav->brow_ctx, object, flow_eVisible_Full))
      brow_CenterObject( nav->brow_ctx, object, 0.25);
    free( node_list);
    nav->last_selected = object;
    break;
  }
  case flow_eEvent_Key_Tab: {
    if ( nav->traverse_focus_cb)
      (nav->traverse_focus_cb)( nav->parent_ctx, nav);
    break;
  }
  case flow_eEvent_MB1DoubleClick:
    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      brow_GetUserData( event->object.object, (void **)&item);
      switch( item->type) {
      case nav_eItemType_Object: 
	((ItemObject *)item)->open_children( nav,
					     event->object.x, event->object.y);
	break;
      case nav_eItemType_AttrArray: 
	((ItemAttrArray *)item)->open_children( nav,
						event->object.x, event->object.y);
	break;
      case nav_eItemType_AttrObject: 
	((ItemAttrObject *)item)->open_attributes( nav,
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
  case flow_eEvent_MB1DoubleClickShift:
    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      brow_GetUserData( event->object.object, (void **)&item);
      switch( item->type) {
      case nav_eItemType_Object: 
	((ItemObject *)item)->open_attributes( nav,
					       event->object.x, event->object.y);
	break;
      case nav_eItemType_AttrObject: 
	((ItemAttrObject *)item)->open_attributes( nav,
						  event->object.x, event->object.y);
	break;
      case nav_eItemType_AttrArray: 
	((ItemAttrArray *)item)->open_attributes( nav,
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
  case flow_eEvent_MB1Click: {
    // Select
    double ll_x, ll_y, ur_x, ur_y;
    int		sts;

    if ( nav->set_focus_cb)
      (nav->set_focus_cb)( nav->parent_ctx, nav);

    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      brow_MeasureNode( event->object.object, &ll_x, &ll_y,
			&ur_x, &ur_y);
      if ( event->object.x < ll_x + 1.0) {
	// Simulate doubleclick
	flow_tEvent doubleclick_event;

	doubleclick_event = (flow_tEvent) calloc( 1, sizeof(*doubleclick_event));
	memcpy( doubleclick_event, event, sizeof(*doubleclick_event));
	doubleclick_event->event = flow_eEvent_MB1DoubleClick;
	sts = brow_cb( ctx, doubleclick_event);
	free( (char *) doubleclick_event);
	return sts;
      }

      if ( brow_FindSelectedObject( nav->brow_ctx, event->object.object)) {
	brow_SelectClear( nav->brow_ctx);
      }
      else {
	brow_SelectClear( nav->brow_ctx);
	brow_SetInverse( event->object.object, 1);
	brow_SelectInsert( nav->brow_ctx, event->object.object);
      }
      nav->set_selection_owner();
      break;
    default:
      brow_SelectClear( nav->brow_ctx);
    }
    break;
  }
  case flow_eEvent_MB1ClickShift: {
    // Select
    double ll_x, ll_y, ur_x, ur_y;
    int		sts;

    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      brow_MeasureNode( event->object.object, &ll_x, &ll_y,
			&ur_x, &ur_y);
      if ( event->object.x < ll_x + 1.0) {
	// Simulate doubleclick
	flow_tEvent doubleclick_event;

	doubleclick_event = (flow_tEvent) calloc( 1, sizeof(*doubleclick_event));
	memcpy( doubleclick_event, event, sizeof(*doubleclick_event));
	doubleclick_event->event = flow_eEvent_MB1DoubleClickShift;
	sts = brow_cb( ctx, doubleclick_event);
	free( (char *) doubleclick_event);
	return sts;
      }
      break;
    default:
      ;
    }
    break;
  }
  case flow_eEvent_Map: {
    nav->displayed = 1;
    break;
  }
  default:
    ;
  }
  return 1;
}

//
// Create nodeclasses
//
void Nav::create_nodeclasses()
{
  allocate_pixmaps();

  // Create common-class

  brow_CreateNodeClass( brow_ctx, "NavigatorDefault", 
			flow_eNodeGroup_Common, &nc_object);
  brow_AddFrame( nc_object, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_object, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_object, 1, 1.1, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_object, 2, 0.6, 0,
		 flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		 0);
  brow_AddAnnot( nc_object, 5, 0.6, 1,
		 flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		 1);
  brow_AddAnnot( nc_object, 9, 0.6, 2,
		 flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		 1);
}

//
// Backcall routine called at creation of the brow widget
// Enable event, create nodeclasses and insert the root objects.
//
int Nav::init_brow_cb( FlowCtx *fctx, void *client_data)
{
  Nav *nav = (Nav *) client_data;
  BrowCtx *ctx = (BrowCtx *)fctx;
  brow_sAttributes brow_attr;
  unsigned long mask;

  nav->brow_ctx = ctx;
  mask = 0;

  mask |= brow_eAttr_indentation;
  brow_attr.indentation = 0.5;
  mask |= brow_eAttr_annotation_space;
  brow_attr.annotation_space = 0.5;
  brow_SetAttributes( nav->brow_ctx, &brow_attr, mask); 
  brow_SetCtxUserData( nav->brow_ctx, nav);

  brow_EnableEvent( ctx, flow_eEvent_MB1DoubleClickShift, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB1ClickShift, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Up, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Down, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Left, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Right, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_ShiftRight, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Tab, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ObjectDeleted, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Map, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PageUp, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PageDown, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ScrollUp, flow_eEventType_CallBack, 
		    brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ScrollDown, flow_eEventType_CallBack, 
		    brow_cb);

  nav->create_nodeclasses();

#if 0
  // Create the root item
  sts = nav_create_object_item( nav, nav->root_objid, NULL, flow_eDest_After,
				(Item **)&nav->root_item, 1);
  if (EVEN(sts)) return sts;
#endif

  if ( !nav->menu) {
    nav->menu = PalFile::config_tree_build( nav->ldhses, pal_cPaletteFile,
					    pal_eNameType_TopObjects, nav->root_name, NULL); 
    if ( !nav->menu) {
      printf( 
	     "** Nav: topobjects entry \"%s\" not found in configuration file\n",
	     nav->root_name);
      return 0;
    }
    PalFile::config_tree_build( nav->ldhses, pal_cLocalPaletteFile,
				pal_eNameType_TopObjects, nav->root_name, nav->menu); 
  }

  nav->open_top();
  return 1;
}


//
// Create a navigator item. The class of item depends of the class
// of the object.
//
static int nav_create_object_item( Nav *nav, pwr_tObjid objid, 
				   brow_tNode dest, flow_eDest dest_code, Item **item,
				   int is_root)
{
  *item = (Item *) new ItemObject( nav, objid, dest, dest_code, is_root);
  return 1;
}

//
// Create the navigator widget
//
Nav::Nav(
	 void *nav_parent_ctx,
	 char *nav_name,
	 ldh_tSesContext nav_ldhses,
	 char *nav_root_name,
	 pwr_tStatus *status
	 ) :
  parent_ctx(nav_parent_ctx), ldhses(nav_ldhses),
  root_item(0), last_selected(0), get_plant_select_cb(0), set_focus_cb(0),
  traverse_focus_cb(0), displayed(0), menu(0), selection_owner(0), show_descrip(1)
{
  strcpy( name, nav_name);
  strcpy( root_name, nav_root_name);
  wbctx = ldh_SessionToWB( ldhses);
  *status = 1;
}

//
//  Delete a nav context
//
Nav::~Nav()
{
}

//
//  Zoom
//
void Nav::zoom( double zoom_factor)
{
  brow_Zoom( brow_ctx, zoom_factor);
}

//
//  Return to base zoom factor
//
void Nav::unzoom()
{
  brow_UnZoom( brow_ctx);
}

//
//  Return associated class of selected object
//
int Nav::get_select( pwr_sAttrRef *attrref, int *is_attr)
{
  brow_tNode	*node_list;
  int		node_count;
  Item		*item;
  pwr_tOName   	attr_str;
  int		sts, size;
  
  brow_GetSelectedNodes( brow_ctx, &node_list, &node_count);
  if ( !node_count)
    return 0;

  brow_GetUserData( node_list[0], (void **)&item);
  free( node_list);

  sts = ldh_ObjidToName( ldhses, item->objid, 
			 ldh_eName_VolPath,
			 attr_str, sizeof(attr_str), &size);
  if ( EVEN(sts)) return sts;

  memset( attrref, 0, sizeof(*attrref));
  switch( item->type) {
  case nav_eItemType_Attr:
    strcat( attr_str, ".");
    strcat( attr_str, ((ItemAttr *)item)->aname);
    sts = ldh_NameToAttrRef( ldhses, attr_str, attrref);
    if ( EVEN(sts)) return sts;
    *is_attr = 1;
    break;
  case nav_eItemType_AttrArray:
    strcat( attr_str, ".");
    strcat( attr_str, ((ItemAttrArray *)item)->aname);
    sts = ldh_NameToAttrRef( ldhses, attr_str, attrref);
    if ( EVEN(sts)) return sts;
    *is_attr = 1;
    break;
  case nav_eItemType_AttrArrayElem:
    strcat( attr_str, ".");
    strcat( attr_str, ((ItemAttrArrayElem *)item)->aname);
    sts = ldh_NameToAttrRef( ldhses, attr_str, attrref);
    if ( EVEN(sts)) return sts;
    *is_attr = 1;
    break;
  default:
    sts = ldh_NameToAttrRef( ldhses, attr_str, attrref);
    *is_attr = 0;
    if ( EVEN(sts)) return sts;
  }
  return 1;
}

int Nav::object_exist( brow_tObject object)
{
  brow_tObject 	*object_list;
  int		object_cnt;
  int		i;

  brow_GetObjectList( brow_ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    if ( object_list[i] == object)
      return 1;
  }
  return 0;
}

int Nav::open_top()
{
  int 		class_cnt;
  int		i, sts;
  pwr_tClassId 	valid_class[100];
  pwr_tObjid 	root;
  pwr_tClassId	classid;
  Item 		*item;
  PalFileMenu   *menu_p;

  if (!menu)
    return 0;

  // Get the toplevel objects
  class_cnt = 0;
  for ( menu_p = menu->child_list; menu_p; menu_p = menu_p->next)  {
    sts = ldh_ClassNameToId( ldhses, &valid_class[class_cnt],
			     menu_p->title);
    if ( ODD(sts))
      class_cnt++;
  }
  if ( !class_cnt)
    return sts;

  sts = ldh_GetRootList( ldhses, &root);
  if ( sts == LDH__NOSUCHOBJ)
    return  1;
  if ( EVEN(sts)) return sts;

  //  Loop through all root objects and see if they are valid at toplevel
  while ( ODD(sts)) {
    sts = ldh_GetObjectClass( ldhses, root, &classid);
    if (ODD(sts)) {
      for (i = 0; i < class_cnt; i ++)  {
	if (classid == valid_class[i]) {
	  sts = nav_create_object_item( this, root, NULL,
					flow_eDest_IntoLast, &item, 0);
	  if ( EVEN(sts)) return sts;
	}
      }
    }
    sts = ldh_GetNextSibling( ldhses, root, &root);
  }
  return 1;
}

