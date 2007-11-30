/* 
 * Proview   $Id: xtt_menu.cpp,v 1.15 2007-11-30 08:19:15 claes Exp $
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


#include "flow_std.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "co_cdh.h"
#include "co_ccm.h"
#include "co_time.h"
#include "co_dcli.h"
#include "pwr_baseclasses.h"
#include "co_ccm_msg.h"
#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"

#include "co_lng.h"
#include "co_dcli_msg.h"
#include "rt_xnav_msg.h"
#include "xtt_xnav.h"
#include "xtt_menu.h"
#include "co_api.h"

pwr_dImport pwr_BindXttClasses(Base);

xmenu_sMenuCall *XNav::mcp = 0;
static xmenu_sMenuItem xmenu_lMenuItem[100];

int XNav::getAllMenuItems( xmenu_sMenuCall	*ip,
			   xmenu_sMenuItem	**Item,
			   pwr_tObjid		objid,
			   pwr_tUInt32		Level,
			   int			*nItems,
			   int			AddSeparator,
			   pwr_sAttrRef		*CurrentObject)
{
  int                   sts;
  pwr_tCid	        classid;
  pwr_tObjid            child;
  pwr_sMenuButton	*mbp;
  pwr_sMenuCascade	*mcp;
  pwr_sMenuRef		*mrp;
  pwr_tStatus           (*filter)( xmenu_sMenuCall *);
  int                   sensitive;
  int                   i;


  Level++;
  memset(*Item, 0, sizeof(**Item));

  if(AddSeparator) {
    (*Item)->Level = Level;
    (*Item)->Item = xmenu_eMenuItem_Separator;
    (*Item)->MenuObject = pwr_cNObjid;
    (*Item)++;
    (*nItems)++;
  } 
  else {
    sts = gdh_GetObjectClass( objid, &classid);
    if ( EVEN(sts)) return sts;

    if ( classid == pwr_eClass_MenuButton) {
      sts = gdh_ObjidToPointer( objid, (void **) &mbp);
      if ( EVEN(sts)) return sts;

      
      ip->ChosenItem = *nItems;
      for ( i = 0; i < 5; i++) {
        strcpy( (*Item)->FilterArguments[i], mbp->FilterArguments[i]);
      }

      // Call any filter method
      (*Item)->CurrentObject = *CurrentObject;
      sensitive = 1;
      if ( strcmp( mbp->FilterName, "") != 0) {
        sts = GetMethod( mbp->FilterName, &filter);
        if ( ODD(sts)) {
          sts = (filter) ( ip);
	  if ( sts == XNAV__INSENSITIVE) 
            sensitive = 0;
        }
      }
      else
        sts = XNAV__SUCCESS;

      if ( ODD(sts)) {
        (*Item)->Level = Level;
        (*Item)->Item = xmenu_eMenuItem_Button;
        (*Item)->MenuObject = objid;

        (*Item)->Flags.f.Sensitive = sensitive;
        if (strcmp( mbp->MethodName, "")  == 0)
          (*Item)->Flags.f.Sensitive = 0;

        strcpy((*Item)->Name, mbp->ButtonName);
        strcpy( (*Item)->Method, mbp->MethodName);
        strcpy( (*Item)->Filter, mbp->FilterName);
        for ( i = 0; i < 5; i++) {
          strcpy( (*Item)->MethodArguments[i], mbp->MethodArguments[i]);
        }
        (*Item)++;
        (*nItems)++;
      }
    } 
    else if ( classid == pwr_eClass_MenuSeparator) {
      (*Item)->Level = Level;
      (*Item)->Item = xmenu_eMenuItem_Separator;
      (*Item)->MenuObject = objid;
      (*Item)++;
      (*nItems)++;
    } 
    else if ( classid == pwr_eClass_MenuCascade) {
      sts = gdh_ObjidToPointer( objid, (void **) &mcp);
      if ( EVEN(sts)) return sts;

      // Call any filter method
      (*Item)->CurrentObject = *CurrentObject;
      if ( strcmp( mcp->FilterName, "") != 0) {
        sts = GetMethod( mcp->FilterName, &filter);
        if ( ODD(sts)) {
          sts = (filter) ( ip);
        }
      }
      else
        sts = XNAV__SUCCESS;

      if ( ODD(sts)) {
        (*Item)->Level = Level;
        (*Item)->Item = xmenu_eMenuItem_Cascade;
        (*Item)->Flags.f.Sensitive = 1;
        strcpy((*Item)->Name, mcp->ButtonName);
        (*Item)->MenuObject = objid;
        (*Item)++;
        (*nItems)++;
     
        sts = gdh_GetChild( objid, &child);
        while( ODD(sts)) {
          sts = getAllMenuItems(ip, Item, child, Level, nItems, 0, CurrentObject);
          if ( EVEN(sts)) return sts;
          sts = gdh_GetNextSibling( child, &child);
        }
      }
    }
    else if ( classid == pwr_eClass_MenuRef && 
	      cdh_ObjidIsNull( CurrentObject->Objid)) {
      pwr_tAName aname;
      pwr_sAttrRef currentar;
      pwr_tCid current_cid;
      pwr_tTid a_tid;
      pwr_tUInt32 a_size, a_offs, a_elem;

      sts = gdh_ObjidToPointer( objid, (void **) &mrp);
      if ( EVEN(sts)) return sts;

      // Call any filter method
      (*Item)->CurrentObject = *CurrentObject;
      if ( strcmp( mrp->FilterName, "") != 0) {
        sts = GetMethod( mrp->FilterName, &filter);
        if ( ODD(sts)) {
          sts = (filter) ( ip);
        }
      }
      else
        sts = XNAV__SUCCESS;

      if ( ODD(sts)) {
	sts = gdh_AttrrefToName( &ip->Pointed, aname, sizeof(aname), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( aname, ".");
	strcat( aname, mrp->RefAttribute);

	sts = gdh_GetAttributeCharacteristics( aname, &a_tid, &a_size,
					       &a_offs, &a_elem);
	if ( ODD(sts)) {
	  switch ( a_tid) {
	  case pwr_eType_AttrRef:
	    sts = gdh_GetObjectInfo( aname, &currentar, sizeof(currentar));
	    break;
	  case pwr_eType_Objid: {
	    pwr_tOid oid;

	    currentar = pwr_cNAttrRef;
	    sts = gdh_GetObjectInfo( aname, &oid, sizeof(oid));
	    currentar = cdh_ObjidToAref( oid);
	    break;
	  }
	  default:
	    sts = 0;
	  }
	}
	if ( ODD(sts) && cdh_ObjidIsNotNull( currentar.Objid)) {
	  (*Item)->Level = Level;
	  (*Item)->Item = xmenu_eMenuItem_Ref;
	  (*Item)->Flags.f.Sensitive = 1;
	  strcpy((*Item)->Name, mrp->ButtonName);
	  (*Item)->MenuObject = objid;
	  (*Item)++;
	  (*nItems)++;

	  // Create a label with current object name
	  sts = gdh_AttrrefToName( &currentar, aname, sizeof(aname), 
				   cdh_mNName);
	  if ( ODD(sts)) {
	    (*Item)->Level = Level;
	    (*Item)->Item = xmenu_eMenuItem_Button;
	    (*Item)->MenuObject = pwr_cNObjid;
	    strncpy((*Item)->Name, aname, sizeof((*Item)->Name));
	    (*Item)->Name[sizeof((*Item)->Name)-1] = 0;
	    (*Item)->MenuObject = pwr_cNObjid;
	    (*Item)->CurrentObject = currentar;
	    (*Item)->Flags.f.Sensitive = 1;
	    strcpy( (*Item)->Method, "$Object-OpenObject");
	    (*Item)++;
	    (*nItems)++;
	  }

	  sts = gdh_GetAttrRefTid( &currentar, &current_cid);
	  if ( EVEN(sts)) return sts;

	  sts = GetObjectMenu(ip, current_cid, Item, Level, nItems, 0, &currentar);
	  if ( EVEN(sts)) return sts;
        }
      }
    }
  }
  return XNAV__SUCCESS;
}


int XNav::GetMenu( xmenu_sMenuCall *ip)
{
  xmenu_sMenuItem	*Item = (xmenu_sMenuItem *) &xmenu_lMenuItem;
  int			nItems = 0;
  pwr_tCid		classid;
  pwr_sAttrRef		current = pwr_cNAttrRef;
  pwr_tStatus		sts;

  if ( cdh_ObjidIsNotNull( ip->Selected[0].Objid)) {
    sts = gdh_GetAttrRefTid ( &ip->Selected[0], &classid);
    if ( EVEN(sts)) return sts;

    GetObjectMenu( ip, classid, &Item, 0, &nItems, 0, &current);
  }
  Item->Level = 0;
  ip->ItemCount = nItems - 1;

  return XNAV__SUCCESS;
}

int XNav::GetObjectMenu( xmenu_sMenuCall	*ip,
			 pwr_tCid		classid,
			 xmenu_sMenuItem	**Item,
			 pwr_tUInt32		Level,
			 int			*nItems,
			 int			AddSeparator,
			 pwr_sAttrRef		*CurrentObject)
{
  int                   sts;
  pwr_tObjid            child;
  pwr_tObjid		menu_objid;
  char  		menu[80];
  pwr_tOName  		classname;
  pwr_sAttrRef		currentar = pwr_cNAttrRef;
  pwr_tCid		supercid;


  if ( cdh_ObjidIsNotNull( CurrentObject->Objid))
    currentar = *CurrentObject;

    if ( ip->ItemType == xmenu_eItemType_Object) {
      // Popup-menu for an object
      if ( !cdh_tidIsCid( classid))
	return XNAV__SUCCESS;


      // Get the RtXtt common menu-objects
      strcpy( menu, "pwrs:Class-$Object-RtXtt");
      sts = gdh_NameToObjid( menu, &menu_objid);
      if ( ODD(sts)) {
        sts = gdh_GetChild( menu_objid, &child);
        while( ODD(sts)) {
          sts = getAllMenuItems( ip, Item, child, Level, nItems, 0, &currentar);
          if ( EVEN(sts)) return sts;
          sts = gdh_GetNextSibling( child, &child);
        }
      }

      // Get the RtXtt menu-objects for superclasses
      // TODO shadow overlayed methods...
      sts = gdh_GetSuperClass( classid, &supercid, CurrentObject->Objid);
      while ( ODD(sts)) {
	sts = gdh_ObjidToName( cdh_ClassIdToObjid( supercid), classname, 
			       sizeof(classname), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	sprintf( menu, "%s-RtXtt", classname);
	sts = gdh_NameToObjid( menu, &menu_objid);
	if ( ODD(sts)) {
	  sts = gdh_GetChild( menu_objid, &child);
	  while( ODD(sts)) {
	    sts = getAllMenuItems( ip, Item, child, 0, nItems, 0, &currentar);
	    if ( EVEN(sts)) return sts;
	    sts = gdh_GetNextSibling( child, &child);
	  }
	}
	sts = gdh_GetSuperClass( supercid, &supercid, CurrentObject->Objid);
      }

      // Get the RtXtt menu-objects for this class, or for superclasses
      sts = gdh_ObjidToName( cdh_ClassIdToObjid( classid), classname, 
		     sizeof(classname), cdh_mName_volumeStrict);
      if ( EVEN(sts)) return sts;

      sprintf( menu, "%s-RtXtt", classname);
      sts = gdh_NameToObjid( menu, &menu_objid);
      if ( ODD(sts)) {
        sts = gdh_GetChild( menu_objid, &child);
        while( ODD(sts)) {
          sts = getAllMenuItems( ip, Item, child, 0, nItems, 0, &currentar);
          if ( EVEN(sts)) return sts;
          sts = gdh_GetNextSibling( child, &child);
        }
      }
    }
    else if ( ip->ItemType == xmenu_eItemType_AttrObject) {   
      // Find attribute object methods...
      if ( !cdh_tidIsCid( classid))
	return XNAV__SUCCESS;

      // Get the RtXtt common menu-objects
      strcpy( menu, "pwrs:Class-$Object-RtXttAttrObject");
      sts = gdh_NameToObjid( menu, &menu_objid);
      if ( ODD(sts)) {
        sts = gdh_GetChild( menu_objid, &child);
        while( ODD(sts)) {
          sts = getAllMenuItems( ip, Item, child, 0, nItems, 0, &currentar);
          if ( EVEN(sts)) return sts;
          sts = gdh_GetNextSibling( child, &child);
        }
      }

      // Get the RtXtt menu-objects for this class
      sts = gdh_ObjidToName( cdh_ClassIdToObjid( classid), classname, 
		     sizeof(classname), cdh_mName_volumeStrict);
      if ( EVEN(sts)) return sts;

      sprintf( menu, "%s-RtXtt", classname);
      sts = gdh_NameToObjid( menu, &menu_objid);
      if ( ODD(sts)) {
        sts = gdh_GetChild( menu_objid, &child);
        while( ODD(sts)) {
          sts = getAllMenuItems( ip, Item, child, 0, nItems, 0, &currentar);
          if ( EVEN(sts)) return sts;
          sts = gdh_GetNextSibling( child, &child);
        }
      }
    }
    else if ( ip->ItemType == xmenu_eItemType_Attribute) {   
      // Find attribute methods...
      // Get the RtXttAttribute common menu-objects
      strcpy( menu, "pwrs:Class-$Object-RtXttAttribute");
      sts = gdh_NameToObjid( menu, &menu_objid);
      if ( ODD(sts)) {
        sts = gdh_GetChild( menu_objid, &child);
        while( ODD(sts)) {
          sts = getAllMenuItems( ip, Item, child, 0, nItems, 0, &currentar);
          if ( EVEN(sts)) return sts;
          sts = gdh_GetNextSibling( child, &child);
        }
      }
    }
    else if ( ip->ItemType == xmenu_eItemType_Crossref) {   
      // Find attribute methods...
      // Get the RtXttCrossref common menu-objects
      strcpy( menu, "pwrs:Class-$Object-RtXttCrossref");
      sts = gdh_NameToObjid( menu, &menu_objid);
      if ( ODD(sts)) {
        sts = gdh_GetChild( menu_objid, &child);
        while( ODD(sts)) {
          sts = getAllMenuItems( ip, Item, child, 0, nItems, 0, &currentar);
          if ( EVEN(sts)) return sts;
          sts = gdh_GetNextSibling( child, &child);
        }
      }
    }

  return XNAV__SUCCESS;
}

int XNav::GetMethod( char *name, 
		     pwr_tStatus (**method)( xmenu_sMenuCall *))
{
  int i, j;
  char cname[80];
  char mname[80];
  char *s;

  strcpy( cname, name);
  if ( (s = strchr( cname, '-'))) {
    *s = 0;
    strcpy( mname, s+1);
  }
  else
    strcpy( mname, "");

  for ( i = 0;; i++) {
    if (pwr_gBase_XttClassMethods[i].ClassName[0] == '\0') break; {
      if ( strcmp(pwr_gBase_XttClassMethods[i].ClassName, cname) == 0) {
        for (j = 0;; j++) {
          if ((*pwr_gBase_XttClassMethods[i].Methods)[j].MethodName[0] == '\0')
	    break;
          if ( strcmp( (*pwr_gBase_XttClassMethods[i].Methods)[j].MethodName,
		       mname) == 0) {
	    *method = (*pwr_gBase_XttClassMethods[i].Methods)[j].Method;
	    return XNAV__SUCCESS;
          }
        }
	break;
      }
    }
  }
  return XNAV__NOMETHOD;
}

int XNav::CallMenuMethod( xmenu_sMenuCall *ip, int idx)
{
  pwr_tStatus sts;
  pwr_tStatus (*method)( xmenu_sMenuCall *);

  sts = GetMethod( ip->ItemList[idx].Method, &method);
  if ( ODD(sts))
    sts = (method) ( ip);

  return sts;
}

int XNav::CheckMenuMethodFilter( xmenu_sMenuCall *ip, int idx)
{
  pwr_tStatus sts;
  pwr_tStatus (*filter)( xmenu_sMenuCall *);

  sts = GetMethod( ip->ItemList[idx].Filter, &filter);
  if ( ODD(sts))
    sts = (filter) ( ip);

  return sts;
}


int XNav::call_method( char *method, char *filter,
		       pwr_sAttrRef attrref, 
		       xmenu_eItemType item_type, 
		       xmenu_mUtility caller,
		       unsigned int priv, char *arg)
{
  pwr_tStatus 	sts;
  int		sel_cnt;
  pwr_tStatus (*method_func)( xmenu_sMenuCall *);
  pwr_tStatus (*filter_func)( xmenu_sMenuCall *);

  if (mcp == NULL)
    mcp = (xmenu_sMenuCall *)calloc(1, sizeof(xmenu_sMenuCall));

  mcp->EditorContext = (void *)this;
  mcp->PointedSet = xmenu_eMenuSet_Object;

  mcp->Pointed = attrref;
  mcp->Caller = caller;
  mcp->ItemType = item_type;
  mcp->Priv = priv;
  mcp->SelectedSet = xmenu_eMenuSet_None;
  mcp->SelectedSet = xmenu_eMenuSet_Object;
  if ( arg)
    strcpy( mcp->Arg, arg);

  sel_cnt = 1;
  if ( mcp->Selected)
    free( mcp->Selected);
  mcp->Selected = (pwr_sAttrRef *) calloc( sel_cnt + 1, sizeof (pwr_sAttrRef));
  mcp->Selected[0] = attrref;
  mcp->Selected[sel_cnt].Objid = pwr_cNObjid;
  mcp->SelectCount = sel_cnt;

  if ( filter && strcmp( filter, "") != 0) {
    sts = GetMethod( filter, &filter_func);
    if ( EVEN(sts)) return sts;

    sts = (filter_func) ( mcp);
    if ( EVEN(sts)) return sts;
  }

  sts = GetMethod( method, &method_func);
  if ( EVEN(sts)) return sts;

  sts = (method_func) ( mcp);
  if ( EVEN(sts)) return sts;

  free( mcp->Selected);
  mcp->Selected = 0;

  return XNAV__SUCCESS;
}

int XNav::call_object_method( pwr_sAttrRef attrref, 
			      xmenu_eItemType item_type, 
			      xmenu_mUtility caller, 
			      unsigned int priv, char *method_name)
{
  pwr_tStatus 	sts;
  int 		i;
  int		sel_cnt;
  int		idx;

  if (mcp == NULL)
    mcp = (xmenu_sMenuCall *)calloc(1, sizeof(xmenu_sMenuCall));

  mcp->ItemList = xmenu_lMenuItem;
  mcp->EditorContext = (void *)this;
  mcp->PointedSet = xmenu_eMenuSet_Object;

  mcp->Pointed = attrref;
  mcp->Caller = caller;
  mcp->ItemType = item_type;
  mcp->Priv = priv;
  mcp->SelectedSet = xmenu_eMenuSet_None;
  mcp->SelectedSet = xmenu_eMenuSet_Object;

  sel_cnt = 1;
  if ( mcp->Selected)
    free( mcp->Selected);
  mcp->Selected = (pwr_sAttrRef *) calloc( sel_cnt + 1, sizeof (pwr_sAttrRef));
  mcp->Selected[0] = attrref;
  mcp->Selected[sel_cnt].Objid = pwr_cNObjid;
  mcp->SelectCount = sel_cnt;

  sts = GetMenu( mcp);  
  if (EVEN(sts) || mcp->ItemList[0].Level == 0) {
    return 0;
  }

  // Find index of method
  idx = -1;
  for ( i = 0; i <= (int) mcp->ItemCount; i++) {
    if ( cdh_ObjidIsNull( mcp->ItemList[i].CurrentObject.Objid) &&
	 mcp->ItemList[i].Flags.f.Sensitive &&
	 cdh_NoCaseStrcmp( mcp->ItemList[i].Name, method_name) == 0) {
      idx = i;
      break;
    }
  }
  if ( idx == -1)
    return 0;

  mcp->ChosenItem = idx;
  sts = CallMenuMethod( mcp, mcp->ChosenItem);
  return sts;
}

int XNav::check_object_methodfilter( pwr_sAttrRef attrref, 
				     xmenu_eItemType item_type, 
				     xmenu_mUtility caller, 
				     unsigned int priv, char *method_name)
{
  pwr_tStatus 	sts;
  int 		i;
  int		sel_cnt;
  int		idx;

  if (mcp == NULL)
    mcp = (xmenu_sMenuCall *)calloc(1, sizeof(xmenu_sMenuCall));

  mcp->ItemList = xmenu_lMenuItem;
  mcp->EditorContext = (void *)this;
  mcp->PointedSet = xmenu_eMenuSet_Object;

  mcp->Pointed = attrref;
  mcp->Caller = caller;
  mcp->ItemType = item_type;
  mcp->Priv = priv;
  mcp->SelectedSet = xmenu_eMenuSet_None;
  mcp->SelectedSet = xmenu_eMenuSet_Object;

  sel_cnt = 1;
  if ( mcp->Selected)
    free( mcp->Selected);
  mcp->Selected = (pwr_sAttrRef *) calloc( sel_cnt + 1, sizeof (pwr_sAttrRef));
  mcp->Selected[0] = attrref;
  mcp->Selected[sel_cnt].Objid = pwr_cNObjid;
  mcp->SelectCount = sel_cnt;

  sts = GetMenu( mcp);  
  if (EVEN(sts) || mcp->ItemList[0].Level == 0) {
    return 0;
  }

  // Find index of method
  idx = -1;
  for ( i = 0; i <= (int) mcp->ItemCount; i++) {
    if ( cdh_ObjidIsNull( mcp->ItemList[i].CurrentObject.Objid) &&
	 mcp->ItemList[i].Flags.f.Sensitive &&
	 cdh_NoCaseStrcmp( mcp->ItemList[i].Name, method_name) == 0) {
      idx = i;
      break;
    }
  }
  if ( idx == -1)
    return 0;

  mcp->ChosenItem = idx;
  sts = CheckMenuMethodFilter( mcp, mcp->ChosenItem);
  return sts;
}

void XNav::get_popup_menu_items( pwr_sAttrRef attrref,
				 xmenu_eItemType item_type, 
				 xmenu_mUtility caller, 
				 unsigned int priv, char *arg)
{
  pwr_tStatus 	sts;
  int		sel_cnt;

  if (mcp == NULL)
    mcp = (xmenu_sMenuCall *)calloc(1, sizeof(xmenu_sMenuCall));

  mcp->ItemList = xmenu_lMenuItem;
  mcp->EditorContext = (void *)this;
  mcp->PointedSet = xmenu_eMenuSet_Object;

  mcp->Pointed = attrref;
  mcp->Caller = caller;
  mcp->ItemType = item_type;
  mcp->Priv = priv;
  mcp->SelectedSet = xmenu_eMenuSet_None;
  mcp->SelectedSet = xmenu_eMenuSet_Object;
  if ( arg)
    strcpy( mcp->Arg, arg);

  sel_cnt = 1;
  if ( mcp->Selected)
    free( mcp->Selected);
  mcp->Selected = (pwr_sAttrRef *) calloc( sel_cnt + 1, sizeof (pwr_sAttrRef));
  mcp->Selected[0] = attrref;
  mcp->Selected[sel_cnt].Objid = pwr_cNObjid;
  mcp->SelectCount = sel_cnt;

  sts = GetMenu( mcp);  
  if (EVEN(sts) || mcp->ItemList[0].Level == 0) {
    return;
  }
}



