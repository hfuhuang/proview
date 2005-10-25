/* 
 * Proview   $Id: wb_wnav.cpp,v 1.27 2005-10-25 15:28:11 claes Exp $
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

/* wb_wnav.cpp -- Display plant and node hiererachy */

#include "flow_std.h"

#include <stdio.h>
#include <stdlib.h>
#if defined(OS_VMS)
#include <descrip.h>
#endif

extern "C" {
#include "pwr_privilege.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_msg.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "wb_wnav_msg.h"
#include "wb_ldh.h"
#include "wb_login.h"
#include "wb_wccm.h"
}

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget.h"

#include "wb_wnav.h"
#include "wb_wnav_item.h"

extern "C" {
#include "flow_x.h"
#include "wb_wtt_api.h"
#include "co_api.h"
}

#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))

static char null_str[] = "";

static void wnav_trace_scan( WNav *wnav);
static int wnav_trace_scan_bc( brow_tObject object, void *p);
static int wnav_trace_connect_bc( brow_tObject object, char *name, char *attr, 
	flow_eTraceType type, void **p);
static int wnav_trace_disconnect_bc( brow_tObject object);
static int wnav_init_brow_cb( BrowCtx *ctx, void *client_data);
static int wnav_init_brow_base_cb( FlowCtx *fctx, void *client_data);
static void wnav_sel_lose_cb( Widget w, Atom *selection);
static Boolean wnav_sel_convert_cb(
    Widget  w,
    Atom    *selection,
    Atom    *target,
    Atom    *type_return,
    XtPointer	*value_return,
    unsigned long   *length_return,
    int	    *format_return);
static void wnav_reset_avoid_deadlock( WNav *wnav);
static void wnav_set_avoid_deadlock( WNav *wnav, int time);


//
// Convert attribute string to value
//
int  wnav_attr_string_to_value( ldh_tSesContext ldhses, int type_id, char *value_str, 
	void *buffer_ptr, int buff_size, int attr_size)
{
  int		sts;

  switch ( type_id )
  {
    case pwr_eType_Boolean:
    {
      if ( sscanf( value_str, "%d", (pwr_tBoolean *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      if ( *(pwr_tBoolean *)buffer_ptr > 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_Float32:
    {
      if ( sscanf( value_str, "%f", (float *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_Float64:
    {
      pwr_tFloat32 f;
      pwr_tFloat64 d;
      if ( sscanf( value_str, "%f", &f) != 1)
        return WNAV__INPUT_SYNTAX;
      d = f;
      memcpy( buffer_ptr, (char *) &d, sizeof(d));

      break;
    }
    case pwr_eType_Char:
    {
      pwr_tChar c;
      if ( strcmp( value_str, "") == 0) {
        c = '\0';
	memcpy( buffer_ptr, &c, sizeof(c));
      }
      else if ( sscanf( value_str, "%c", (char *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_Int8:
    {
      pwr_tInt8 	i8;
      pwr_tInt16	i16;
      if ( sscanf( value_str, "%hd", &i16) != 1)
        return WNAV__INPUT_SYNTAX;
      i8 = i16;
      memcpy( buffer_ptr, (char *)&i8, sizeof(i8));
      break;
    }
    case pwr_eType_Int16:
    {
      if ( sscanf( value_str, "%hd", (short *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_Int32:
    {
      if ( sscanf( value_str, "%d", (int *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_UInt8:
    {
      pwr_tUInt8 	i8;
      pwr_tUInt16	i16;
      if ( sscanf( value_str, "%hu", &i16) != 1)
        return WNAV__INPUT_SYNTAX;
      i8 = i16;
      memcpy( buffer_ptr, (char *)&i8, sizeof(i8));
      break;
    }
    case pwr_eType_UInt16:
    {
      if ( sscanf( value_str, "%hu", (unsigned short *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_UInt32:
    case pwr_eType_DisableAttr:
    {
      if ( sscanf( value_str, "%lu", (unsigned long *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_Enum:
    {
      if ( sscanf( value_str, "%lu", (unsigned long *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_Mask:
    {
      if ( sscanf( value_str, "%lu", (unsigned long *)buffer_ptr) != 1)
        return WNAV__INPUT_SYNTAX;
      break;
    }
    case pwr_eType_Text:
    case pwr_eType_String:
    case pwr_eType_ProString:
    {
      if ( (int) strlen( value_str) >= attr_size)
        return WNAV__STRINGTOLONG;
      strncpy( (char *)buffer_ptr, value_str, min(attr_size, buff_size));
      break;
    }
    case pwr_eType_Objid:
    {
      pwr_tObjid	objid;

      if ( strcmp( value_str, "") == 0)
	objid = pwr_cNObjid;
      else {
	sts = ldh_NameToObjid ( ldhses, &objid, value_str);
	if (EVEN(sts)) return WNAV__OBJNOTFOUND;
      }
      memcpy( buffer_ptr, &objid, sizeof(objid));
      break;
    }
    case pwr_eType_ClassId:
    {
      pwr_tClassId	classid;
      pwr_tObjid	objid;

      sts = ldh_NameToObjid( ldhses, &objid, value_str);
      if (EVEN(sts)) return WNAV__OBJNOTFOUND;
  	classid = cdh_ClassObjidToId( objid);
      memcpy( buffer_ptr, (char *) &classid, sizeof(classid));
      break;
    }
    case pwr_eType_TypeId:
    case pwr_eType_CastId:
    {
      pwr_tTypeId	val_typeid;
      pwr_tObjid	objid;

      sts = ldh_NameToObjid( ldhses, &objid, value_str);
      if (EVEN(sts)) return WNAV__OBJNOTFOUND;
  	val_typeid = cdh_TypeObjidToId( objid);
      memcpy( buffer_ptr, (char *) &val_typeid, sizeof(val_typeid));
      break;
    }
    case pwr_eType_ObjectIx:
    {
      pwr_tObjectIx	objectix;

      sts = cdh_StringToObjectIx( value_str, &objectix); 
      if (EVEN(sts)) return WNAV__OBJNOTFOUND;
  	memcpy( buffer_ptr, (char *) &objectix, sizeof(objectix));
      break;
    }
    case pwr_eType_VolumeId:
    {
      pwr_tVolumeId	volumeid;

      sts = cdh_StringToVolumeId( value_str, &volumeid); 
      if (EVEN(sts)) return WNAV__OBJNOTFOUND;
  	memcpy( buffer_ptr, (char *) &volumeid, sizeof(volumeid));
      break;
    }
    case pwr_eType_RefId:
    {
      pwr_tRefId	subid;

      sts = cdh_StringToSubid( value_str, &subid);
      if (EVEN(sts)) return WNAV__OBJNOTFOUND;
  	memcpy( buffer_ptr, (char *) &subid, sizeof(subid));
      break;
    }
    case pwr_eType_AttrRef:
    {
      pwr_sAttrRef	attrref;

      sts = ldh_NameToAttrRef( ldhses, value_str, &attrref);
      if (EVEN(sts)) return WNAV__OBJNOTFOUND;
  	memcpy( buffer_ptr, &attrref, sizeof(attrref));
      break;
    }
    case pwr_eType_DataRef:
    {
      pwr_tDataRef	dataref;

      dataref.Ptr = 0;
      sts = ldh_NameToAttrRef( ldhses, value_str, &dataref.Aref);
      if (EVEN(sts)) return WNAV__OBJNOTFOUND;
  	memcpy( buffer_ptr, &dataref, sizeof(dataref));
      break;
    }
    case pwr_eType_Time:
    {
      pwr_tTime	time;

      sts = time_AsciiToA( value_str, &time);
      if (EVEN(sts)) return WNAV__INPUT_SYNTAX;
  	memcpy( buffer_ptr, (char *) &time, sizeof(time));
      break;
    }
    case pwr_eType_DeltaTime:
    {
      pwr_tDeltaTime deltatime;

      sts = time_AsciiToD( value_str, &deltatime);
      if (EVEN(sts)) return WNAV__INPUT_SYNTAX;
  	memcpy( buffer_ptr, (char *) &deltatime, sizeof(deltatime));
      break;
    }
  }
  return 1;
}

//
// Convert attribute value to string
//
void  wnav_attrvalue_to_string( ldh_tSesContext ldhses, int type_id, void *value_ptr,
	char **buff, int *len)
{
  pwr_tObjid		objid;
  pwr_sAttrRef		*attrref;
  int			sts;
  static char		str[2048];

  switch ( type_id )
  {
    case pwr_eType_Boolean:
    {
      *len = sprintf( str, "%d", *(pwr_tBoolean *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_Float32:
    {
      *len = sprintf( str, "%f", *(float *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_Float64:
    {
      *len = sprintf( str, "%f", *(double *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_Char:
    {
      *len = sprintf( str, "%c", *(char *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_Int8:
    {
      *len = sprintf( str, "%d", *(char *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_Int16:
    {
      *len = sprintf( str, "%hd", *(short *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_Int32:
    {
      *len = sprintf( str, "%d", *(int *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_UInt8:
    {
      *len = sprintf( str, "%d", *(unsigned char *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_UInt16:
    {
      *len = sprintf( str, "%hd", *(unsigned short *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_UInt32:
    case pwr_eType_DisableAttr:
    {
      *len = sprintf( str, "%d", *(unsigned int *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_Enum:
    {
      *len = sprintf( str, "%d", *(unsigned int *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_Mask:
    {
      *len = sprintf( str, "%d", *(unsigned int *)value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_String:
    case pwr_eType_Text:
    {
      *len = strlen((char *) value_ptr);
      strcpy( str, (char *) value_ptr);
      *buff = str;
      break;
    }
    case pwr_eType_ProString:
    {
      *len = strlen((char *) value_ptr);
      strcpy( str, "");
      for ( int i = 0; i < *len; i++)
	strcat( str, "*");
      *buff = str;
      break;
    }
    case pwr_eType_Objid:
    {
      ldh_sVolumeInfo info;
      ldh_GetVolumeInfo( ldh_SessionToVol( ldhses), &info);

      objid = *(pwr_tObjid *)value_ptr;
      if ( objid.vid == info.Volume)
	sts = ldh_ObjidToName( ldhses, objid, ldh_eName_Hierarchy, 
			       str, sizeof(str), len);
      else
	sts = ldh_ObjidToName( ldhses, objid, ldh_eName_VolPath, 
			       str, sizeof(str), len);
      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
      }
      *buff = str;
      break;
    }
    case pwr_eType_AttrRef:
    {
      char *name_p;
      ldh_sVolumeInfo info;
      ldh_GetVolumeInfo( ldh_SessionToVol( ldhses), &info);

      attrref = (pwr_sAttrRef *) value_ptr;
      if ( attrref->Objid.vid == info.Volume)
	sts = ldh_AttrRefToName( ldhses, attrref, ldh_eName_Aref, &name_p, len);
      else
	sts = ldh_AttrRefToName( ldhses, attrref, ldh_eName_ArefVol, &name_p, len);

      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
	*buff = str;
        break;
      }
      strcpy( str, name_p);
      *buff = str;
      break;
    }
    case pwr_eType_DataRef:
    {
      char *name_p;
      pwr_tDataRef *dataref;
      ldh_sVolumeInfo info;
      ldh_GetVolumeInfo( ldh_SessionToVol( ldhses), &info);

      dataref = (pwr_tDataRef *) value_ptr;
      if ( dataref->Aref.Objid.vid == info.Volume)
	sts = ldh_AttrRefToName( ldhses, &dataref->Aref, ldh_eName_Aref, &name_p, len);
      else
	sts = ldh_AttrRefToName( ldhses, &dataref->Aref, ldh_eName_ArefVol, &name_p, len);

      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
	*buff = str;
        break;
      }
      strcpy( str, name_p);
      *buff = str;
      break;
    }
    case pwr_eType_Time:
    {
      sts = time_AtoAscii( (pwr_tTime *) value_ptr, time_eFormat_DateAndTime, 
		str, sizeof(str));
      if ( EVEN(sts))
        strcpy( str, "-");
      *len = strlen(str);
      *buff = str;
      break;
    }
    case pwr_eType_DeltaTime:
    {
      sts = time_DtoAscii( (pwr_tDeltaTime *) value_ptr, 1, 
		str, sizeof(str));
      if ( EVEN(sts))
        strcpy( str, "Undefined time");
      *len = strlen( str);
      *buff = str;
      break;
    }
    case pwr_eType_ObjectIx:
    {
      *len = sprintf( str, "%s", cdh_ObjectIxToString( NULL, 
		*(pwr_tObjectIx *) value_ptr, 1));
      *buff = str;
      break;
    }
    case pwr_eType_ClassId:
    {
      objid = cdh_ClassIdToObjid( *(pwr_tClassId *) value_ptr);
      sts = ldh_ObjidToName( ldhses, objid, ldh_eName_VolPath,
		str, sizeof(str), len);
      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
      }
      *buff = str;
      break;
    }
    case pwr_eType_TypeId:
    case pwr_eType_CastId:
    {
      objid = cdh_TypeIdToObjid( *(pwr_tTypeId *) value_ptr);
      sts = ldh_ObjidToName( ldhses, objid, ldh_eName_VolPath,
		str, sizeof(str), len);
      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
      }
      *buff = str;
      break;
    }
    case pwr_eType_VolumeId:
    {
      *len = sprintf( str, "%s", cdh_VolumeIdToString( NULL, 
		*(pwr_tVolumeId *) value_ptr, 1, 0));
      *buff = str;
      break;
    }
    case pwr_eType_RefId:
    {
      *len = sprintf( str, "%s", cdh_SubidToString( NULL, 
		*(pwr_tSubid *) value_ptr, 1));
      *buff = str;
      break;
    }
    default:
      strcpy( str, "");
      *buff = str;
      *len = 0;
  }
}

void WNav::message( char sev, char *text)
{
  if ( message_cb && !scriptmode)
    (message_cb)( parent_ctx, sev, text);
  else
  {
    if ( strcmp( text, "") == 0)
      return;
    if ( sev != ' ')
      printf( "%%WNAV-%c-MSG, %s\n", sev, text);
    else
      printf( "%s\n", text);
  }
}


//
//  Pop wnav window
//

void WNav::pop()
{
  Widget parent, top;

  parent = XtParent( parent_wid);
  while( parent)
  {
    top = parent;
    parent = XtParent( parent);
  }
  flow_UnmapWidget( top);
  flow_MapWidget( top);
}

//
//  Show crossreferences
//

//
// Create a navigator item. The class of item depends of the class
// of the object.
//
int WNav::create_object_item( pwr_tObjid objid, 
	brow_tNode dest, flow_eDest dest_code, void **item,
	int is_root)
{
  int sts;
  pwr_tClassId classid;

  sts = ldh_GetObjectClass( ldhses, objid, &classid);
  if ( EVEN(sts)) return sts;

  switch( classid)
  {
    default:
      *item = (void *) new WItemObject( this, objid, dest, dest_code, is_root);
      break;
  }
  return 1;
}

//
// Create the navigator widget
//
WNav::WNav(
	void *xn_parent_ctx,
	Widget	xn_parent_wid,
	char *xn_name,
	char *xn_layout,
	Widget *w,
        ldh_tSesContext	xn_ldhses,
	wnav_sStartMenu *root_menu,
	wnav_eWindowType xn_type,
	pwr_tStatus *status) :

	parent_ctx(xn_parent_ctx), parent_wid(xn_parent_wid),
	window_type(xn_type), ldhses(xn_ldhses), wbctx(0),
	brow(0), brow_cnt(0), trace_started(0),
	message_cb(NULL), close_cb(NULL), map_cb(NULL), change_value_cb(NULL),
	ccm_func_registred(0),
	menu_tree(NULL), closing_down(0),
	base_priv(pwr_mPrv_System), priv(pwr_mPrv_System), editmode(0),
	layout_objid(pwr_cNObjid), search_last(pwr_cNObjid), search_compiled(0),
	search_type(wnav_eSearchType_No), selection_owner(0), last_selected(0),
	displayed(0), scriptmode(0), dialog_width(0), dialog_height(0),
	dialog_x(0), dialog_y(0), menu(0), avoid_deadlock(0)
{
  Arg 		args[5];
  int		i;

  strcpy( name, xn_name);

  strcpy( user, login_prv.username);
  strcpy( base_user, user);
  priv = login_prv.priv;
  base_priv = priv;

  if ( window_type == wnav_eWindowType_No)
  {
    editmode = 1;
    return;
  }

  if ( xn_layout)
    strcpy( layout, xn_layout);
  else
    strcpy( layout, "");

  form_widget = ScrolledBrowCreate( parent_wid, name, NULL, 0, 
	wnav_init_brow_base_cb, this, (Widget *)&brow_widget);
  XtManageChild( form_widget);

  i = 0;
  XtSetArg(args[i], XmNborderWidth, 1);i++;
  XtSetValues( form_widget, args,i);

  // Create the root item
  *w = form_widget;

  wow_GetAtoms( form_widget, &graph_atom, &objid_atom, &attrref_atom);
  gbl.load_config( this);

  if ( root_menu && !ldhses)
    menu_tree_build( root_menu);

  *status = 1;
}

//
//  Delete a nav context
//
WNav::~WNav()
{
  closing_down = 1;

  if ( trace_started)
    XtRemoveTimeOut( trace_timerid);
  if ( avoid_deadlock)
    XtRemoveTimeOut( deadlock_timerid);

  menu_tree_free();
  PalFile::config_tree_free( menu);
  for ( int i = 1; i < brow_cnt; i++)
  {
    brow_DeleteSecondaryCtx( brow_stack[brow_cnt]->ctx);
    brow_stack[brow_cnt]->free_pixmaps();
    delete brow_stack[i];
  }
  delete brow;
  XtDestroyWidget( form_widget);
}

//
//  Print
//
void WNav::print( char *filename)
{
  brow_Print( brow->ctx, filename);
}

//
//  Get current zoom factor
//
void WNav::get_zoom( double *zoom_factor)
{
  brow_GetZoom( brow->ctx, zoom_factor);
}

//
//  Zoom
//
void WNav::zoom( double zoom_factor)
{
  brow_Zoom( brow->ctx, zoom_factor);
}

//
//  Return to base zoom factor
//
void WNav::unzoom()
{
  brow_UnZoom( brow->ctx);
}

void WNav::set_inputfocus( int focus)
{
  Arg 		args[2];
  Pixel 	bg, fg;

  if ( !displayed)
    return;

  XtVaGetValues( form_widget, XmNbackground, &bg, XmNforeground, &fg, NULL);
  if ( !focus)
  {
    XtSetArg(args[0], XmNborderColor, bg);
    XtSetValues( form_widget, args, 1);
  }
  else
  {
    if ( flow_IsViewable( brow_widget)) {
      XtCallAcceptFocus( brow_widget, CurrentTime);
      XtSetArg(args[0], XmNborderColor, fg);
      XtSetValues( form_widget, args, 1);
    }
  }
}

//
// Set attribute value
//
int WNav::set_attr_value( brow_tObject node, pwr_tObjid objid, char *value_str)
{
  WItem		*base_item;
  int		sts;
  char 		buff[200];
  int		size;

  // Check that object still exist
  if ( !object_exist( node))
    return WNAV__DISAPPEARD;

  brow_GetUserData( node, (void **)&base_item);

  // Check that objid is still the same
  if ( ! cdh_ObjidIsEqual( objid, base_item->objid))
    return WNAV__DISAPPEARD;

  switch( base_item->type)
  {
    case wnav_eItemType_Attr:
    case wnav_eItemType_AttrInput:
    case wnav_eItemType_AttrInputInv:
    case wnav_eItemType_AttrInputF:
    case wnav_eItemType_AttrOutput:
    {
      WItemBaseAttr *item = (WItemBaseAttr *)base_item;

      // Check that objid is still the same
      if ( ! cdh_ObjidIsEqual( objid, base_item->objid))
        return WNAV__DISAPPEARD;

      sts =  wnav_attr_string_to_value( ldhses, item->type_id, 
	value_str, buff, sizeof(buff), item->size);
      if ( EVEN(sts))
        message( 'E', "Input syntax error");
      else
        sts = ldh_SetObjectPar( ldhses, item->objid, item->body,
		item->attr, buff, item->size);
      return sts;
    }
    case wnav_eItemType_AttrArrayElem:
    {
      char *value;

      WItemAttrArrayElem *item = (WItemAttrArrayElem *)base_item;

      // Check that objid is still the same
      if ( ! cdh_ObjidIsEqual( objid, base_item->objid))
        return WNAV__DISAPPEARD;

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
        free( (char *)value);
      }
      return sts;
    }
    case wnav_eItemType_ObjectName:
    {
      WItemObjectName *item = (WItemObjectName *)base_item;

      // Check that objid is still the same
      if ( ! cdh_ObjidIsEqual( objid, base_item->objid))
        return WNAV__DISAPPEARD;

      sts = ldh_ChangeObjectName( ldhses, item->objid, value_str);
      return sts;
    }
    case wnav_eItemType_Local:
    {
      WItemLocal	*item;

      item = (WItemLocal *)base_item;

      sts =  wnav_attr_string_to_value( ldhses, item->type_id, 
	value_str, buff, sizeof(buff), item->size);
      if ( EVEN(sts)) return sts;

      memcpy( item->value_p, buff, item->size);
      break;
    }
    default:
      ;
  }
  return 1;
}

//
// Check if an item is valid for change
//
int WNav::check_attr_value( brow_tObject node, int *multiline,
	char **init_value, int *size)
{
  WItem		*base_item;
  int		sts;
  char          *p;
  int           len;

  // Check that object still exist
  if ( !object_exist( node))
    return WNAV__DISAPPEARD;

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

      if ( !editmode)
        return WNAV__NOEDIT;
      if ( item->flags & PWR_MASK_NOEDIT && !gbl.bypass)
        return WNAV__FLAGNOEDIT;
      if ( item->flags & PWR_MASK_STATE && !gbl.bypass)
        return WNAV__FLAGSTATE;

      sts = item->get_value( (char **)&p);
      wnav_attrvalue_to_string( ldhses, item->type_id, p, init_value, 
				  &len);
      free( p);
      *size = item->size;
      if ( item->type_id == pwr_eType_Text)
        *multiline = 1;
      else
        *multiline = 0;

      break;
    }
    case wnav_eItemType_AttrArrayElem:
    {
      WItemAttrArrayElem *item = (WItemAttrArrayElem *)base_item;

      if ( !editmode)
        return WNAV__NOEDIT;
      if ( item->flags & PWR_MASK_NOEDIT && !gbl.bypass)
        return WNAV__FLAGNOEDIT;
      if ( item->flags & PWR_MASK_STATE && !gbl.bypass)
        return WNAV__FLAGSTATE;

      sts = item->get_value( (char **)&p);
      wnav_attrvalue_to_string( ldhses, item->type_id, p, init_value, 
				  &len);
      free( p);
      *size = item->size;
      if ( item->type_id == pwr_eType_Text)
        *multiline = 1;
      else
        *multiline = 0;

      break;
    }
    case wnav_eItemType_ObjectName:
    {
      static char name[32];
      char *p;

      if ( !editmode)
        return WNAV__NOEDIT;
      sts = ((WItemObjectName *)base_item)->get_value( &p);
      if ( ODD(sts)) {
        strcpy( name, p);
        free( p);
      }
      else
        strcpy( name, "");
      *init_value = name;
      *multiline = 0;
      *size = sizeof( name);
      break;
    }
    case wnav_eItemType_Local:
    {
      WItemLocal *item = (WItemLocal *)base_item;

      *multiline = 0;
      *size = item->size;
      break;
    }
    default:
      ;
  }
  return WNAV__SUCCESS;
}

//
// Set objectname
//
int WNav::set_object_name( brow_tObject node, pwr_tObjid objid, char *value_str)
{
  WItem		*base_item;
  int		sts;

  // Check that object still exist
  if ( !object_exist( node))
    return WNAV__DISAPPEARD;

  brow_GetUserData( node, (void **)&base_item);

  // Check that objid is still the same
  if ( ! cdh_ObjidIsEqual( objid, base_item->objid))
    return WNAV__DISAPPEARD;

  switch( base_item->type)
  {
    case wnav_eItemType_Object:
    {
      sts = ldh_ChangeObjectName( ldhses, base_item->objid, value_str);
      return sts;
    }
    default:
      ;
  }
  return 1;
}

//
// Check if an item is valid for change
//
int WNav::check_object_name( brow_tObject node)
{
  WItem		*base_item;

  // Check that object still exist
  if ( !object_exist( node))
    return WNAV__DISAPPEARD;

  brow_GetUserData( node, (void **)&base_item);

  switch( base_item->type)
  {
    case wnav_eItemType_Object:
    {
      if ( !editmode)
        return WNAV__NOEDIT;
      break;
    }
    default:
      return WNAV__NONAME;
  }
  return WNAV__SUCCESS;
}


//
//  Return attrref of selected object
//  List is terminated with a null attrref
//
int WNav::get_select( pwr_sAttrRef **attrref, int **is_attr, int *cnt)
{
  brow_tNode	*node_list;
  int		node_count;
  WItem		*item;
  pwr_tAName   	attr_str;
  int		sts;
  pwr_sAttrRef	*ap;
  int		size;
  int		i;
  int           *is_attr_p;
  
  if ( window_type == wnav_eWindowType_No)
    return WNAV__CMDMODE;

  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
  {
    *cnt = 0;
    return WNAV__NOSELECT;
  }

  *attrref = (pwr_sAttrRef *) calloc( node_count + 1, sizeof(pwr_sAttrRef));
  *is_attr = (int *) calloc( node_count + 1, sizeof(int));
  *cnt = node_count;

  ap = *attrref;
  is_attr_p = *is_attr;
  for ( i = 0; i < node_count; i++)
  {
    brow_GetUserData( node_list[i], (void **)&item);

    if ( cdh_ObjidIsNull(item->objid))
    {
      (*cnt)--;
      if ( *cnt == 0)
        free( (char *) *attrref);
      continue;
    }
    sts = ldh_ObjidToName( ldhses, item->objid, ldh_eName_Hierarchy,
	    	attr_str, sizeof(attr_str), &size);
    if ( EVEN(sts)) return sts;

    switch( item->type)
    {
      case wnav_eItemType_Attr:
      case wnav_eItemType_AttrInput:
      case wnav_eItemType_AttrInputF:
      case wnav_eItemType_AttrInputInv:
      case wnav_eItemType_AttrOutput:
      case wnav_eItemType_AttrArray:
      case wnav_eItemType_AttrArrayOutput:
      case wnav_eItemType_AttrArrayElem:
      case wnav_eItemType_AttrObject:
        strcat( attr_str, ".");
        strcat( attr_str, item->name);
        sts = ldh_NameToAttrRef( ldhses, attr_str, ap);
        if ( EVEN(sts))
        {
          // ldh_NameToAttrRef doesn't handler objects with no RtBody...
          ap->Objid = item->objid;
        }
        *is_attr_p = 1;
        break;
      default:
        sts = ldh_NameToAttrRef( ldhses, attr_str, ap);
        if ( EVEN(sts)) {
          // ldh_NameToAttrRef doesn't handle objects with no RtBody...
          *ap = cdh_ObjidToAref( item->objid);
        }
        *is_attr_p = 0;
    }
    ap++;
    is_attr_p++;
  }
  free( node_list);
  return WNAV__SUCCESS;
}

//
//  Return selected objects
//  The list should be freed with free()
//
int WNav::get_selected_nodes( brow_tObject **sellist, int *sel_cnt)
{
  brow_GetSelectedNodes( brow->ctx, sellist, sel_cnt);
  if ( !(*sel_cnt))
    return WNAV__NOSELECT;
  return WNAV__SUCCESS;
}


//
//  Open plc for selected object
//
int WNav::open_plc()
{
  brow_tNode	*node_list;
  int		node_count;
  WItem		*item;
  pwr_tClassId	classid;
  int		sts;
  void		*foectx;
  
  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);
  if ( !node_count)
    return 0;

  brow_GetUserData( node_list[0], (void **)&item);
  free( node_list);

  switch( item->type)
  {
    case wnav_eItemType_Object:

      sts = ldh_GetObjectClass( ldhses, item->objid, &classid);
      if ( EVEN(sts)) return sts;

      if ( classid == pwr_cClass_plc)
        sts = wnav_foe_new( (void *)parent_ctx, parent_wid, "PlcProgram1",
		item->objid, wbctx, ldhses,
		&foectx, 1, ldh_eAccess_ReadOnly, pwr_cNOid);
      break;
    default:
     sts = 0;
  }
  return sts;
}

//
//  Open plc for specified object
//
int WNav::open_plc( pwr_tOid oid)
{
  pwr_tCid	cid;
  int		sts = WNAV__SUCCESS;
  void		*foectx;
  pwr_tOid      plc;
  bool		found;
  
  plc = oid;
  while ( ODD(sts)) {
    sts = ldh_GetObjectClass( ldhses, plc, &cid);
    if ( EVEN(sts)) return sts;

    if ( cid == pwr_cClass_plc) {
      found = true;
      break;
    }
    sts = ldh_GetParent( ldhses, plc, &plc);
  }
  if ( !found)
    return WNAV__NOPLC;

  sts = wnav_foe_new( (void *)parent_ctx, parent_wid, "PlcProgram1",
		      plc, wbctx, ldhses,
		      &foectx, 1, ldh_eAccess_ReadOnly, oid);
  return sts;
}


//
// Convert type_id to name
//
#if 0
static void  wnav_type_id_to_name( int type_id, char *type_id_name)
{
  switch( type_id)
  {
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
    case pwr_eType_DataRef: strcpy( type_id_name, "DataRef"); break;
    default: strcpy( type_id_name, "");
  }
}
#endif

//
// Callbacks from brow
//
static int wnav_brow_cb( FlowCtx *ctx, flow_tEvent event)
{
  WNav		*wnav;
  WItem 		*item;
  int		sts;

  brow_GetCtxUserData( (BrowCtx *)ctx, (void **) &wnav);
  if ( wnav->closing_down)
    return 1;

  if ( event->event != flow_eEvent_ObjectDeleted)
    wnav->message( ' ', null_str);

  switch ( event->event)
  {
    case flow_eEvent_Key_Up:
    case flow_eEvent_Key_ShiftUp:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object, current;
      int		sts;

      brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
	current = 0;
        if ( wnav->last_selected && wnav->object_exist( wnav->last_selected))
          object = wnav->last_selected;
        else
        {
          sts = brow_GetLast( wnav->brow->ctx, &object);
          if ( EVEN(sts)) return 1;
        }
      }
      else
      {
	if ( node_count == 1)
	  current = node_list[0];
	else {
	  bool found = false;
          for ( int i = 0; i < node_count; i++) {
	    if ( node_list[i] == wnav->last_selected)
	      found = true;
	  }
	  if ( found)
	    current = wnav->last_selected;
	  else
	    current = node_list[0];
	}
	sts = brow_GetPrevious( wnav->brow->ctx, current, &object);
        if ( EVEN(sts)) {
	  current = 0;
          sts = brow_GetLast( wnav->brow->ctx, &object);
          if ( EVEN(sts))
	  {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }

      if ( event->event ==  flow_eEvent_Key_ShiftUp) {
	bool found = false;
	for ( int i = 0; i < node_count; i++) {
	  if ( node_list[i] == object)
	    found = true;
	}
	if ( found) {
	  // Previous object is already selected, unselect current
	  if ( current) {
	    brow_SetInverse( current, 0);
	    brow_SelectRemove( wnav->brow->ctx, current);
	  }
	}
	else {
	  brow_SetInverse( object, 1);
	  brow_SelectInsert( wnav->brow->ctx, object);
	}
      }
      else {
        brow_SelectClear( wnav->brow->ctx);
        brow_SetInverse( object, 1);
        brow_SelectInsert( wnav->brow->ctx, object);
      }
      if ( !brow_IsVisible( wnav->brow->ctx, object, flow_eVisible_Full))
        brow_CenterObject( wnav->brow->ctx, object, 0.25);
      if ( node_count)
        free( node_list);
      wnav->last_selected = object;
      break;
    }
    case flow_eEvent_Key_Down:
    case flow_eEvent_Key_ShiftDown:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object, current;
      int		sts;

      brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
	current = 0;
        if ( wnav->last_selected && wnav->object_exist( wnav->last_selected))
          object = wnav->last_selected;
        else
        {
          sts = brow_GetFirst( wnav->brow->ctx, &object);
          if ( EVEN(sts)) return 1;
        }
      }
      else
      {
	if ( node_count == 1)
	  current = node_list[0];
	else {
	  bool found = false;
          for ( int i = 0; i < node_count; i++) {
	    if ( node_list[i] == wnav->last_selected)
	      found = true;
	  }
	  if ( found)
	    current = wnav->last_selected;
	  else
	    current = node_list[0];
	}
	sts = brow_GetNext( wnav->brow->ctx, current, &object);
        if ( EVEN(sts)) {
	  current = 0;
          sts = brow_GetFirst( wnav->brow->ctx, &object);
          if ( EVEN(sts))
	  {
            if ( node_count)
	      free( node_list);
            return 1;
 	  }
        }
      }
      if ( event->event ==  flow_eEvent_Key_ShiftDown) {
	bool found = false;
	for ( int i = 0; i < node_count; i++) {
	  if ( node_list[i] == object)
	    found = true;
	}
	if ( found) {
	  // Previous object is already selected, unselect current
	  if ( current) {
	    brow_SetInverse( current, 0);
	    brow_SelectRemove( wnav->brow->ctx, current);
	  }
	}
	else {
	  brow_SetInverse( object, 1);
	  brow_SelectInsert( wnav->brow->ctx, object);
	}
      }
      else {
        brow_SelectClear( wnav->brow->ctx);
	brow_SetInverse( object, 1);
	brow_SelectInsert( wnav->brow->ctx, object);
      }
      if ( !brow_IsVisible( wnav->brow->ctx, object, flow_eVisible_Full))
        brow_CenterObject( wnav->brow->ctx, object, 0.75);
      if ( node_count)
        free( node_list);
      wnav->last_selected = object;
      break;
    }
    case flow_eEvent_Key_PF1:
    case flow_eEvent_Key_ShiftRight:
    {
      brow_tNode	*node_list;
      int		node_count;

      brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        break;
      brow_GetUserData( node_list[0], (void **)&item);
      free( node_list);
      switch( item->type)
      {
        case wnav_eItemType_Object: 
	  ((WItemBaseObject *)item)->open_attributes( wnav, 0, 0);
          break;
        case wnav_eItemType_AttrArray: 
	  ((WItemAttrArray *)item)->open_attributes( 0, 0);
          break;
        case wnav_eItemType_AttrArrayOutput: 
	  ((WItemAttrArrayOutput *)item)->open_attributes( 0, 0);
          break;
        case wnav_eItemType_AttrObject: 
	  ((WItemAttrObject *)item)->open_attributes( 0, 0);
          break;
        default:
          ;
      }
      break;
    }
    case flow_eEvent_Key_Right:
    {
      brow_tNode	*node_list;
      int		node_count;

      brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
        break;
      brow_GetUserData( node_list[0], (void **)&item);
      switch( item->type)
      {
        case wnav_eItemType_Object: 
	  sts = ((WItemBaseObject *)item)->open_children( wnav, 0, 0);
          if ( sts == WNAV__NOCHILD && wnav->gbl.advanced_user)
            // Open attributes instead
	    ((WItemBaseObject *)item)->open_attributes( wnav, 0, 0);
          break;
        case wnav_eItemType_Menu: 
	  ((WItemMenu *)item)->open_children( wnav, 0, 0);
          break;
        case wnav_eItemType_Command: 
	  ((WItemCommand *)item)->open_children( wnav, 0, 0);
          break;
        case wnav_eItemType_Volume: 
	  ((WItemVolume *)item)->open_children( wnav, 0, 0);
          break;
        case wnav_eItemType_Attr:
	  sts = ((WItemAttr *)item)->open_children( 0, 0);
	  if ( ODD(sts))
            break;

          if ( wnav->gbl.advanced_user && wnav->change_value_cb)
            (wnav->change_value_cb)( wnav->parent_ctx);
          break;
        case wnav_eItemType_AttrInput:
        case wnav_eItemType_AttrInputF:
        case wnav_eItemType_AttrInputInv:
        case wnav_eItemType_AttrOutput:
          if ( wnav->gbl.advanced_user && wnav->change_value_cb)
            (wnav->change_value_cb)( wnav->parent_ctx);
          break;
        case wnav_eItemType_AttrArrayElem:
	  sts = ((WItemAttrArrayElem *)item)->open_children( 0, 0);
	  if ( ODD(sts))
            break;

          if ( wnav->gbl.advanced_user && wnav->change_value_cb)
            (wnav->change_value_cb)( wnav->parent_ctx);
          break;
        case wnav_eItemType_ObjectName:
        case wnav_eItemType_Local:
          if ( wnav->gbl.advanced_user && wnav->change_value_cb)
            (wnav->change_value_cb)( wnav->parent_ctx);
          break;
        case wnav_eItemType_AttrArray: 
	  ((WItemAttrArray *)item)->open_attributes( 0, 0);
          break;
        case wnav_eItemType_AttrArrayOutput: 
	  ((WItemAttrArrayOutput *)item)->open_attributes( 0, 0);
          break;
        case wnav_eItemType_AttrObject: 
	  ((WItemAttrObject *)item)->open_attributes( 0, 0);
          break;
        case wnav_eItemType_File: 
	  ((WItemFile *)item)->open_children( wnav, 0, 0);
          break;
        case wnav_eItemType_Enum: 
        {
          int value;

	  if ( !wnav->gbl.advanced_user)
            break;
          brow_GetRadiobutton( node_list[0], 0, &value);
          if ( !value)
	    ((WItemEnum *)item)->set();
          break;
        }
        case wnav_eItemType_Mask: 
        {
          int value;

	  if ( !wnav->gbl.advanced_user)
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
    case flow_eEvent_Key_PF3:
    {
      break;
    }
    case flow_eEvent_Key_PF4:
    case flow_eEvent_Key_Left:
    {
      brow_tNode	*node_list;
      int		node_count;
      brow_tObject	object;
      int		sts;

      brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
      if ( !node_count)
      {
	wnav->brow_push();
        return 1;
      }

      if ( brow_IsOpen( node_list[0]))
        // Close this node
        object = node_list[0];
      else
      {
        // Close parent
        sts = brow_GetParent( wnav->brow->ctx, node_list[0], &object);
        if ( EVEN(sts))
        {
          free( node_list);
	  wnav->brow_push();
          return 1;
        }
      }
      brow_GetUserData( object, (void **)&item);
      switch( item->type)
      {
        case wnav_eItemType_Object: 
	  ((WItemBaseObject *)item)->close( wnav, 0, 0);
          break;
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
        case wnav_eItemType_AttrObject: 
	  ((WItemAttrObject *)item)->close( 0, 0);
          break;
        case wnav_eItemType_Menu: 
	  ((WItemMenu *)item)->close( wnav, 0, 0);
          break;
        default:
          ;
      }
      brow_SelectClear( wnav->brow->ctx);
      brow_SetInverse( object, 1);
      brow_SelectInsert( wnav->brow->ctx, object);
      if ( !brow_IsVisible( wnav->brow->ctx, object, flow_eVisible_Full))
        brow_CenterObject( wnav->brow->ctx, object, 0.25);
      free( node_list);
      wnav->last_selected = object;
      break;
    }
    case flow_eEvent_Key_Tab:
    {
      if ( wnav->traverse_focus_cb)
        (wnav->traverse_focus_cb)( wnav->parent_ctx, wnav);
      break;
    }
    case flow_eEvent_SelectClear:
      brow_ResetSelectInverse( wnav->brow->ctx);
      break;
    case flow_eEvent_ObjectDeleted:
      brow_GetUserData( event->object.object, (void **)&item);
      delete item;
      break;
    case flow_eEvent_MB1DoubleClick:
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          switch( item->type)
          {
            case wnav_eItemType_Object: 
	      ((WItemObject *)item)->open_children( wnav,
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_Menu: 
	      ((WItemMenu *)item)->open_children( wnav,
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_Command: 
	      ((WItemCommand *)item)->open_children( wnav,
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_Volume: 
              ((WItemVolume *)item)->open_children( wnav,
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_File: 
              ((WItemFile *)item)->open_children( wnav,
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
            case wnav_eItemType_AttrObject: 
	      ((WItemAttrObject *)item)->open_attributes(
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_Attr: 
	      ((WItemAttr *)item)->open_children( 
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
    case flow_eEvent_MB1DoubleClickShift:
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          switch( item->type)
          {
            case wnav_eItemType_Object: 
	      ((WItemObject *)item)->open_attributes( wnav,
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
            case wnav_eItemType_Attr: 
	      ((WItemAttr *)item)->open_children( 
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_AttrArrayElem: 
	      ((WItemAttrArrayElem *)item)->open_children( 
			event->object.x, event->object.y);
              break;
            case wnav_eItemType_AttrObject: 
	      ((WItemAttrObject *)item)->open_children( 
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
    case flow_eEvent_MB1DoubleClickCtrl:
      switch ( event->object.object_type) {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          switch( item->type) {
	  case wnav_eItemType_Attr: 
	  case wnav_eItemType_AttrArrayElem: {
	    WItemBaseAttr 	*item_attr = (WItemBaseAttr *)item;
	    pwr_sAttrRef      	*sel_list;
	    int               	*sel_is_attr;
	    int		      	sel_cnt = 0;
	    pwr_tOName 		str;
	    int 		size;

	    if ( item_attr->type_id == pwr_eType_Objid) {

	      if ( wnav->get_global_select_cb)
		sts = (wnav->get_global_select_cb)( wnav->parent_ctx, &sel_list, 
					      &sel_is_attr, &sel_cnt);
	      if ( sel_cnt > 1) {
		wnav->message( 'E', "Select one object"); 
		break;
	      }
	      else if ( sel_cnt) {
		sts = ldh_ObjidToName( wnav->ldhses, sel_list[0].Objid, ldh_eName_VolPath, str, 
				       sizeof(str), &size);
		if ( EVEN(sts)) return sts;
	      }
	      else {
		sts = wow_GetSelection( wnav->form_widget, str, sizeof(str), wnav->objid_atom);
		if ( EVEN(sts))
		  sts = wow_GetSelection( wnav->form_widget, str, sizeof(str), XA_STRING);
	      }
	      if ( ODD(sts))
		wnav->set_attr_value( item_attr->node, item_attr->objid, str);
	      break;
	    }
	  }
	  default:
	    ;
          }
          break;
      default:
	;
      }
      break;
    case flow_eEvent_MB1Click:
    {
      // Select
      double ll_x, ll_y, ur_x, ur_y;
      int		sts;

      if ( wnav->set_focus_cb)
        (wnav->set_focus_cb)( wnav->parent_ctx, wnav);

      wnav->set_selection_owner();

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
            sts = wnav_brow_cb( ctx, doubleclick_event);
            free( (char *) doubleclick_event);
            return sts;
          }

          if ( brow_FindSelectedObject( wnav->brow->ctx, event->object.object))
          {
            brow_SelectClear( wnav->brow->ctx);
          }
          else
          {
            brow_SelectClear( wnav->brow->ctx);
            brow_SetInverse( event->object.object, 1);
            brow_SelectInsert( wnav->brow->ctx, event->object.object);
          }
          break;
        default:
          brow_SelectClear( wnav->brow->ctx);
      }
      wnav->last_selected = event->object.object;
      break;
    }
    case flow_eEvent_MB1ClickShift:
    {
      // Add elect
      double ll_x, ll_y, ur_x, ur_y;
      int		sts;
      if ( wnav->set_focus_cb)
        (wnav->set_focus_cb)( wnav->parent_ctx, wnav);

      wnav->set_selection_owner();

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
            doubleclick_event->event = flow_eEvent_MB1DoubleClickShift;
            sts = wnav_brow_cb( ctx, doubleclick_event);
            free( (char *) doubleclick_event);
            return sts;
          }

          if ( brow_FindSelectedObject( wnav->brow->ctx, event->object.object))
          {
            brow_SetInverse( event->object.object, 0);
            brow_SelectRemove( wnav->brow->ctx, event->object.object);
          }
          else
          {
            brow_SetInverse( event->object.object, 1);
            brow_SelectInsert( wnav->brow->ctx, event->object.object);
          }
          break;
        default:
          ;
      }
      break;
    }
    case flow_eEvent_MB1Press:
      // Select region

      if ( wnav->set_focus_cb)
        (wnav->set_focus_cb)( wnav->parent_ctx, wnav);
      wnav->set_selection_owner();

      brow_SetSelectInverse( wnav->brow->ctx);
      break;
    case flow_eEvent_MB1PressShift:
      // Add select region

      if ( wnav->set_focus_cb)
        (wnav->set_focus_cb)( wnav->parent_ctx, wnav);
      wnav->set_selection_owner();

      brow_SetSelectInverse( wnav->brow->ctx);
      break;
    case flow_eEvent_MB3Down:
    {
      brow_SetClickSensitivity( wnav->brow->ctx, flow_mSensitivity_MB3Press);
      break;
    }
    case flow_eEvent_MB3Press:
    {
      // Popup menu
      pwr_sAttrRef aref;
      Arg arg[4];
      short x, y;

      if ( wnav->avoid_deadlock)
        break;

      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          aref = item->aref();
          break;
        default:
          aref = pwr_cNAttrRef;
      }

      XtSetArg( arg[0], XmNx, &x);
      XtSetArg( arg[1], XmNy, &y);
      XtGetValues( XtParent(wnav->brow_widget), arg, 2);

      (wnav->create_popup_menu_cb)( wnav->parent_ctx, aref,
		event->any.x_pixel + x, event->any.y_pixel + y);
      wnav_set_avoid_deadlock( wnav, 2000);

      break;
    }
    case flow_eEvent_MB2Click:
    {
      // Create object or move object
      double ll_x, ll_y, ur_x, ur_y;
      ldh_eDest	destcode;
      pwr_tClassId classid;
      int sts;
      pwr_tObjid objid;
      pwr_sAttrRef 	*sel_list;
      int               *sel_is_attr;
      int		sel_cnt;

      if ( wnav->get_global_select_cb)
        (wnav->get_global_select_cb)( wnav->parent_ctx, &sel_list, 
				      &sel_is_attr, &sel_cnt);
      else
        break;

      if ( !sel_cnt)
      {
        // Create object
        sts = (wnav->get_palette_select_cb)( wnav->parent_ctx, &classid);
        if ( EVEN(sts))
        {
          wnav->message('E', "Select a class");
          break;
        }
        switch ( event->object.object_type)
        {
          case flow_eObjectType_Node:
            brow_GetUserData( event->object.object, (void **)&item);
            switch( item->type)
            {
              case wnav_eItemType_Object: 
                brow_MeasureNode( event->object.object, &ll_x, &ll_y,
			&ur_x, &ur_y);
	        if ( event->object.x < ll_x + 1.0)
	          destcode = ldh_eDest_IntoFirst;
	        else
	          destcode = ldh_eDest_After;
                sts = ldh_CreateObject( wnav->ldhses, 
			&objid,
			0, classid, ((WItemObject *)item)->objid, 
			destcode);
		if (EVEN(sts))
		  wnav->message(' ', wnav_get_message(sts));
                break;
	      default:
                ;
            }
            break;
          default:
           // Create toplevel object
           sts = ldh_CreateObject( wnav->ldhses, &objid,
		0, classid, pwr_cNObjid, ldh_eDest_IntoLast);
	   if (EVEN(sts))
	     wnav->message(' ', wnav_get_message(sts));
        }
      }
      else
      {
        // Move object

        if ( event->object.object_type == flow_eObjectType_Node)
        {
          brow_MeasureNode( event->object.object, &ll_x, &ll_y,
		&ur_x, &ur_y);
	  if ( event->object.x < ll_x + 1.0)
	    destcode = ldh_eDest_IntoFirst;
          else
            destcode = ldh_eDest_After;

          brow_GetUserData( event->object.object, (void **)&item);
          if ( item->type == wnav_eItemType_Object)
          {
            sts = ldh_MoveObject( wnav->ldhses, sel_list->Objid, 
		item->objid, destcode);
            if ( EVEN(sts)) {
	      wnav->message(' ', wnav_get_message(sts));
	      return sts;
	    }
            // Unselect moved object
            if ( wnav->global_unselect_objid_cb)
              (wnav->global_unselect_objid_cb)( wnav->parent_ctx, sel_list->Objid);
          }
        }
        free( (char *)sel_list);
        free( (char *)sel_is_attr);
      }
      break;
    }
    case flow_eEvent_Radiobutton:
    {
      if ( !wnav->editmode)
      {
        wnav->message('E', "Not in edit mode");
        break;
      }
      switch ( event->object.object_type)
      {
        case flow_eObjectType_Node:
          brow_GetUserData( event->object.object, (void **)&item);
          switch( item->type)
          {
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
      wnav->displayed = 1;
      break;
    }
    default:
      ;
  }
  return 1;
}

static void wnav_trace_scan( WNav *wnav)
{
  int time = 1000;

  if ( wnav->trace_started)
  {
    brow_TraceScan( wnav->brow->ctx);

    wnav->trace_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext(wnav->brow_widget) , time,
	(XtTimerCallbackProc)wnav_trace_scan, wnav);
  }
}

void WNav::force_trace_scan()
{
  if ( trace_started)
    brow_TraceScan( brow->ctx);
}

static int wnav_trace_scan_bc( brow_tObject object, void *p)
{
  WItem		*base_item;
  char		*buf;
  int		len;
  WNav		*wnav;

  brow_GetUserData( object, (void **)&base_item);

  switch( base_item->type)
  {
    case wnav_eItemType_Local:
    {
      WItemLocal	*item;

      item = (WItemLocal *)base_item;
      if ( !item->first_scan)
      {
        if ( item->size > (int) sizeof(item->old_value) && 
	     item->type_id == pwr_eType_String &&
	     strlen((char *)p) < sizeof(item->old_value) && 
	     strcmp( (char *)p, item->old_value) == 0)
          // No change since last time
          return 1;
        else if ( memcmp( item->old_value, p, item->size) == 0)
          // No change since last time
          return 1;
      }
      else
        item->first_scan = 0;

      brow_GetCtxUserData( brow_GetCtx( object), (void **) &wnav);

      wnav_attrvalue_to_string( wnav->ldhses, item->type_id, p, &buf, &len);
      brow_SetAnnotation( object, 1, buf, len);
      memcpy( item->old_value, p, min(item->size, (int) sizeof(item->old_value)));
      break;
    }
    default:
      ;
  }
  return 1;
}

static int wnav_trace_connect_bc( brow_tObject object, char *name, char *attr, 
	flow_eTraceType type, void **p)
{
  WItem 		*base_item;

  if ( strcmp(name,"") == 0)
    return 1;

  brow_GetUserData( object, (void **)&base_item);
  switch( base_item->type)
  {
    case wnav_eItemType_Local:
    {
      WItemLocal	*item;

      item = (WItemLocal *) base_item;
      *p = item->value_p;
      break;
    }
    default:
      ;
  }      
  return 1;
}

static int wnav_trace_disconnect_bc( brow_tObject object)
{
  return 1;
}


int WNav::unselect_objid( pwr_tObjid objid)
{
  WItem 	*item;
  int	sts;

  sts = find( objid, (void **) &item);
  if ( ODD(sts))
  {
    brow_SetInverse( item->node, 0);
    brow_SelectRemove( brow->ctx, item->node);
  }
  return sts;
}

int WNav::display_object( pwr_tObjid objid)
{
#define PARENTLIST_SIZE 40
  pwr_tObjid 	parent_list[PARENTLIST_SIZE];
  int		parent_cnt = 0;
  int		i;
  WItemObject	*item;
  int		sts;

  if ( brow->type != wnav_eBrowType_Volume)
    return 0;

  sts = ldh_GetParent( ldhses, objid, &parent_list[parent_cnt]);
  while( ODD(sts))
  {
    if ( parent_cnt == PARENTLIST_SIZE)
      return 0;
    parent_cnt++;
    sts = ldh_GetParent( ldhses, parent_list[parent_cnt-1], &parent_list[parent_cnt]);
  }

  brow_SetNodraw( brow->ctx);
  brow_DeleteAll( brow->ctx);
  
  // Restore the rootlist
  get_rootlist();

  for ( i = parent_cnt; i > 0; i--)
  {
    sts = find( parent_list[i - 1], (void **) &item);
    if ( EVEN(sts)) 
    {
      brow_ResetNodraw( brow->ctx);
      brow_Redraw( brow->ctx, 0);
      return WNAV__OBJECTNOTFOUND;
    }
    item->open_children( this, 0, 0);
  }
  sts = find( objid, (void **) &item);
  if ( EVEN(sts))
  {
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, 0);
    return WNAV__OBJECTNOTFOUND;
  }
  brow_SetInverse( item->node, 1);
  brow_SelectInsert( brow->ctx, item->node);

  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);

  brow_CenterObject( brow->ctx, item->node, 0.80);
  return 1;
}

int WNav::object_exist( brow_tObject object)
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

int WNav::is_empty()
{
  brow_tObject 	*object_list;
  int		object_cnt;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  return (object_cnt == 0);
}

int WNav::node_to_objid( brow_tNode node, pwr_tObjid *objid)
{
  WItem	 	*item;

  brow_GetUserData( node, (void **)&item);
  switch( item->type)
  {
    case wnav_eItemType_Object:
    case wnav_eItemType_Attr:
    case wnav_eItemType_AttrInput:
    case wnav_eItemType_AttrInputF:
    case wnav_eItemType_AttrInputInv:
    case wnav_eItemType_AttrOutput:
    case wnav_eItemType_AttrArray:
    case wnav_eItemType_AttrArrayOutput:
    case wnav_eItemType_AttrArrayElem:
    case wnav_eItemType_Enum:
    case wnav_eItemType_Mask:
    case wnav_eItemType_ObjectName:
      if ( cdh_ObjidIsNull( item->objid))
        return 0;
      *objid = item->objid;
      return 1;
    default:
      *objid = pwr_cNObjid;
  }
  return 0;
}

int WNav::find( pwr_tObjid objid, void **item)
{
  brow_tObject 	*object_list;
  int		object_cnt;
  WItem	 	*object_item;
  int		i;

  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    brow_GetUserData( object_list[i], (void **)&object_item);
    if ( cdh_ObjidIsEqual( object_item->objid, objid))
    {
      *item = (void *) object_item;
      return WNAV__SUCCESS;
    }
  }
  return WNAV__OBJNOTFOUND;
}

int	WNav::setup()
{
  brow_pop( wnav_eBrowType_Setup);
  brow_SetNodraw( brow->ctx);
  new WItemHeader( this, "Title", "Setup",  NULL, flow_eDest_IntoLast);

  new WItemLocal( this, "DefaultDirectory", "setup_defaultdirectory", 
	pwr_eType_String, sizeof( gbl.default_directory), 0, 0,
	(void *) gbl.default_directory, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "SymbolFilename", "setup_symbolfilename", 
	pwr_eType_String, sizeof(gbl.symbolfilename), 0, 0,
	(void *) gbl.symbolfilename, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "Verify", "setup_verify", 
	pwr_eType_Int32, sizeof( gbl.verify), 0, 1,
	(void *) &gbl.verify, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "AdvancedUser", "setup_advanceduser", 
	pwr_eType_Int32, sizeof( gbl.advanced_user), 0, 1,
	(void *) &gbl.advanced_user, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "AllToplevel", "setup_alltoplevel", 
	pwr_eType_Int32, sizeof( gbl.all_toplevel), 0, 1,
	(void *) &gbl.all_toplevel, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "Bypass", "setup_bypass", 
	pwr_eType_Int32, sizeof( gbl.bypass), 0, 1,
	(void *) &gbl.bypass, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "ShowClass", "setup_show_class", 
	pwr_eType_Int32, sizeof( gbl.show_class), 0, 1,
	(void *) &gbl.show_class, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "ShowAlias", "setup_show_alias", 
	pwr_eType_Int32, sizeof( gbl.show_alias), 0, 1,
	(void *) &gbl.show_alias, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "ShowDescription", "setup_show_descrip", 
	pwr_eType_Int32, sizeof( gbl.show_descrip), 0, 1,
	(void *) &gbl.show_descrip, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "ShowAttrRef", "setup_show_attrref", 
	pwr_eType_Int32, sizeof( gbl.show_attrref), 0, 1,
	(void *) &gbl.show_attrref, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "ShowAttrXRef", "setup_show_attrxref", 
	pwr_eType_Int32, sizeof( gbl.show_attrxref), 0, 1,
	(void *) &gbl.show_attrxref, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "ShowObjectRef", "setup_show_objref", 
	pwr_eType_Int32, sizeof( gbl.show_objref), 0, 1,
	(void *) &gbl.show_objref, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "ShowObjXRef", "setup_show_objxref", 
	pwr_eType_Int32, sizeof( gbl.show_objxref), 0, 1,
	(void *) &gbl.show_objxref, NULL, flow_eDest_IntoLast);
  new WItemLocal( this, "ShowTrueDb", "setup_show_truedb", 
	pwr_eType_Int32, sizeof( gbl.show_truedb), 0, 1,
	(void *) &gbl.show_truedb, NULL, flow_eDest_IntoLast);

  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
  force_trace_scan();
  return WNAV__SUCCESS;
}

int	WNavGbl::symbolfile_exec( void *wnav)
{
  pwr_tCmd cmd;

  if ( strcmp( symbolfilename, "") != 0)
  {
    strcpy( cmd, "@");
    dcli_translate_filename( &cmd[1], symbolfilename);
  }
  else
  {
    strcpy( cmd, "@");
    dcli_translate_filename( &cmd[1], wnav_cSymbolFile);
  }
  ((WNav *)wnav)->command( cmd);

  return WNAV__SUCCESS;
}

void WNav::set_options( int sh_class, int sh_alias, int sh_descrip, 
	int sh_objref, int sh_objxref, int sh_attrref, int sh_attrxref)
{
  gbl.show_class = sh_class;
  gbl.show_alias = sh_alias;
  gbl.show_descrip = sh_descrip;
  gbl.show_objref = sh_objref;
  gbl.show_objxref = sh_objxref;
  gbl.show_attrref = sh_attrref;
  gbl.show_attrxref = sh_attrxref;
  ldh_refresh( pwr_cNObjid);
}

void WNav::get_options( int *sh_class, int *sh_alias, int *sh_descrip, 
	int *sh_objref, int *sh_objxref, int *sh_attrref, int *sh_attrxref)
{
  *sh_class =  gbl.show_class;
  *sh_alias =  gbl.show_alias;
  *sh_descrip =  gbl.show_descrip;
  *sh_objref =  gbl.show_objref;
  *sh_objxref =  gbl.show_objxref;
  *sh_attrref =  gbl.show_attrref;
  *sh_attrxref =  gbl.show_attrxref;
}

int WNav::save_settnings( ofstream& fp)
{
  if ( window_type == wnav_eWindowType_W1)
    fp << "if ( IsW1())" << endl;
  else if ( window_type == wnav_eWindowType_W2)
    fp << "if ( IsW2())" << endl;

  if ( gbl.advanced_user)
    fp << "  set advanceduser /local" << endl;
  else
    fp << "  set noadvanceduser /local" << endl;

  if ( gbl.all_toplevel)
    fp << "  set alltoplevel /local" << endl;
  else
    fp << "  set noalltoplevel /local" << endl;
 
  if ( gbl.show_class)
    fp << "  set showclass /local" << endl;
  else
    fp << "  set noshowclass /local" << endl;
 
  if ( gbl.show_alias)
    fp << "  set showalias /local" << endl;
  else
    fp << "  set noshowalias /local" << endl;
 
  if ( gbl.show_descrip)
    fp << "  set show description/local" << endl;
  else
    fp << "  set noshowdescription /local" << endl;
 
  if ( gbl.show_objref)
    fp << "  set showobjref /local" << endl;
  else
    fp << "  set noshowobjref /local" << endl;
 
  if ( gbl.show_objxref)
    fp << "  set showobjxref /local" << endl;
  else
    fp << "  set noshowobjxref /local" << endl;
 
  if ( gbl.show_attrref)
    fp << "  set showattrref /local" << endl;
  else
    fp << "  set noshowattrref /local" << endl;
 
  if ( gbl.show_attrxref)
    fp << "  set showattrxref /local" << endl;
  else
    fp << "  set noshowattrxref /local" << endl;
 
  if ( strcmp( gbl.symbolfilename, "") != 0)
    fp << "  set symbolfile /local \"" << gbl.symbolfilename << "\"" << endl;

  if ( window_type == wnav_eWindowType_W1)
    fp << "endif" << endl;
  else if ( window_type == wnav_eWindowType_W2)
    fp << "endif" << endl;
  return 1;
}

int	WNavGbl::load_config( void *wnav)
{
  return WNAV__SUCCESS;
}

int WNav::brow_pop( wnav_eBrowType type)
{
  BrowCtx *secondary_ctx;

  if ( brow_cnt >= WNAV_BROW_MAX)
    return 0;
  brow_CreateSecondaryCtx( brow->ctx, &secondary_ctx,
        wnav_init_brow_cb, (void *)this, flow_eCtxType_Brow);

  brow_ChangeCtx( brow_widget, brow->ctx, brow_stack[brow_cnt]->ctx);
  brow_stack[brow_cnt]->type = type;
  *brow = *brow_stack[brow_cnt];
  brow_cnt++;
  last_selected = 0;
  return 1;
}

int WNav::brow_push()
{
  brow_tNode	*node_list;
  int		node_count;
  WItem		*item;
  int		sts;

  if ( brow_cnt == 1)
     return 0;

  if ( brow->type == wnav_eBrowType_Volume)
  {
    // Detach the volume first
    sts = (detach_volume_cb)( parent_ctx);
    return 1;
  }

  // Call close memberfunction for selected object
  brow_GetSelectedNodes( brow_stack[brow_cnt-2]->ctx, &node_list, &node_count);
  if ( node_count == 1)
  {
    brow_GetUserData( node_list[0], (void **)&item);
    free( node_list);

    switch( item->type)
    {
      case wnav_eItemType_Volume:
	sts = ((WItemVolume *)item)->close( this, 0, 0);
        return 1;
      default:
        ;
    }
  }

  brow_cnt--;
  brow_ChangeCtx( brow_widget, brow_stack[brow_cnt]->ctx, 
		brow_stack[brow_cnt-1]->ctx);
  *brow = *brow_stack[brow_cnt-1];
  brow_DeleteSecondaryCtx( brow_stack[brow_cnt]->ctx);
  brow_stack[brow_cnt]->free_pixmaps();
  delete brow_stack[brow_cnt];

  if ( brow->type == wnav_eBrowType_Volume)
    ldh_refresh( pwr_cNObjid);

  last_selected = 0;

  return 1;
}

// 
// Push for a volume
//
int WNav::brow_push_volume()
{
  if ( brow_cnt == 1)
     return 0;

  brow_cnt--;
  brow_ChangeCtx( brow_widget, brow_stack[brow_cnt]->ctx, 
		brow_stack[brow_cnt-1]->ctx);
  *brow = *brow_stack[brow_cnt-1];

  brow_DeleteSecondaryCtx( brow_stack[brow_cnt]->ctx);
  brow_stack[brow_cnt]->free_pixmaps();
  delete brow_stack[brow_cnt];

  if ( brow->type == wnav_eBrowType_Volume)
    ldh_refresh( pwr_cNObjid);

  last_selected = 0;
  return 1;
}

int WNav::brow_push_all()
{
  while( brow_push())
    ;
  return 1;
}

int WNav::brow_push_to_volume()
{
  if ( brow->type == wnav_eBrowType_Volume)
    return 1;
  while( brow_push() && brow->type != wnav_eBrowType_Volume)
    ;
  return 1;
}

void WNav::menu_tree_build( wnav_sStartMenu *root)
{
  menu_tree = menu_tree_build_children( root, NULL);
}

wnav_sMenu *WNav::menu_tree_build_children( wnav_sStartMenu *first_child,
	wnav_sMenu *parent)
{
  wnav_sStartMenu 	*start_menu_p;
  wnav_sMenu		*menu_p, *prev;
  wnav_sMenu		*return_menu = NULL;
  int			first = 1;

  if ( !first_child)
    return NULL;

  start_menu_p = first_child;
  while ( strcmp( start_menu_p->title, ""))
  {
    switch ( start_menu_p->item_type)
    {
      case wnav_eItemType_Menu:
        menu_p = (wnav_sMenu *) calloc( 1, sizeof(wnav_sMenu));        
        menu_p->parent = parent;
        menu_p->item_type = start_menu_p->item_type;
        strcpy( menu_p->title, start_menu_p->title);
        menu_p->child_list = menu_tree_build_children( 
		(wnav_sStartMenu *)start_menu_p->action, menu_p);
        if ( first)
        {
          return_menu = menu_p;
          first = 0;
        }
        else
          prev->next = menu_p;
        prev = menu_p;
        break;
      case wnav_eItemType_Command:
        menu_p = (wnav_sMenu *) calloc( 1, sizeof(wnav_sMenu));        
        menu_p->parent = parent;
        menu_p->item_type = start_menu_p->item_type;
        strcpy( menu_p->title, start_menu_p->title);
        strcpy( menu_p->command, (char *)start_menu_p->action);
        if ( first)
        {
          return_menu = menu_p;
          first = 0;
        }
        else
          prev->next = menu_p;
        prev = menu_p;
        break;
      default:
        ;
    }
    start_menu_p++;
  }
  return return_menu;
}

void WNav::menu_tree_free()
{
  menu_tree_free_children( menu_tree);
}

void WNav::menu_tree_free_children( wnav_sMenu *first_child)
{
  wnav_sMenu *menu_p, *next;

  menu_p = next = first_child;  
  while( next)
  {
    menu_p = next;
    next = menu_p->next;
    menu_tree_free_children( menu_p->child_list);

    free( (char *) menu_p);
  }
}

int WNav::menu_tree_delete( char *name)
{
  int		sts;
  wnav_sMenu 	*delete_item, *mp;

  sts = menu_tree_search( name, &delete_item);
  if ( EVEN(sts)) return sts;

  if ( !delete_item->parent)
  {
    if ( menu_tree == delete_item)
      menu_tree = delete_item->next;
    else
    {
      for ( mp = menu_tree; mp->next != delete_item; mp = mp->next)
        ;
      mp->next = delete_item->next;
    }

    // Reconfigure the root menu
    brow_push_all();
    brow_SetNodraw( brow->ctx);
    brow_DeleteAll( brow->ctx);
    ((WItemMenu *)root_item)->open_children( this, 0, 0);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, 0);
  }
  else
  {
    if ( delete_item->parent->child_list == delete_item)
      delete_item->parent->child_list = delete_item->next;
    else
    {
      for ( mp = delete_item->parent->child_list; mp->next != delete_item; mp = mp->next)
        ;
      mp->next = delete_item->next;
    }
  }
  free( (char *)delete_item);

  return WNAV__SUCCESS;
}

int WNav::menu_tree_insert( char *title, int item_type, char *command,
	char *destination, int dest_code, wnav_sMenu **menu_item)
{
  wnav_sMenu 	*dest_item;
  wnav_sMenu 	*menu_p, *child_p, *mp;
  int		sts;
  int		first_child = 0;

  if ( destination)
  {
    sts = menu_tree_search( destination, &dest_item);
    if ( EVEN(sts)) return sts;
  }

  switch ( item_type)
  {
    case wnav_eItemType_Menu:
      menu_p = (wnav_sMenu *) calloc( 1, sizeof(wnav_sMenu));        
      menu_p->item_type = item_type;
      strcpy( menu_p->title, title);
      break;
    case wnav_eItemType_Command:
      menu_p = (wnav_sMenu *) calloc( 1, sizeof(wnav_sMenu));        
      menu_p->item_type = item_type;
      strcpy( menu_p->title, title);
      strcpy( menu_p->command, command);
      break;
      default:
        ;
  }

  if ( !destination)
  {
    // Insert first
    menu_p->next = menu_tree;
    menu_tree = menu_p;
  }
  else
  {
    switch( dest_code)
    {
      case wnav_eDestCode_After:
        menu_p->next = dest_item->next;
        menu_p->parent = dest_item->parent;
        dest_item->next = menu_p;
        break;
      case wnav_eDestCode_Before:
        if ( !dest_item->parent)
        {
          if ( dest_item == menu_tree)
          {
            menu_p->next = dest_item;
            menu_tree = menu_p;
          }
          else
          {
            for ( mp = menu_tree; mp->next != dest_item; mp = mp->next)
              ;
            menu_p->next = mp->next;
            mp->next = menu_p;
          }
        }
        else
        {
          if ( dest_item == dest_item->parent->child_list)
          {
            menu_p->next = dest_item;
            dest_item->parent->child_list = menu_p;
          }
          else
          {
            for ( mp = dest_item->parent->child_list; mp->next != dest_item; mp = mp->next)
              ;
            menu_p->next = mp->next;
            mp->next = menu_p;
          }
        }
        menu_p->parent = dest_item->parent;
        break;
      case wnav_eDestCode_FirstChild:
        if ( !dest_item->child_list)
          first_child = 1;
        menu_p->next = dest_item->child_list;
        menu_p->parent = dest_item;
        dest_item->child_list = menu_p;
        break;
      case wnav_eDestCode_LastChild:
        if ( !dest_item->child_list)
        {
          first_child = 1;
          dest_item->child_list = menu_p;
        }
        else
        {
          for( child_p = dest_item->child_list; child_p->next; 
		child_p = child_p->next)
            ;
          child_p->next = menu_p;
        }
        menu_p->parent = dest_item;
        break;
    }
  }

  if ( menu_p->parent == NULL ||
       (menu_p->parent->parent == NULL && first_child))
  {
    // Reconfigure the root menu
    brow_push_all();
    brow_SetNodraw( brow->ctx);
    brow_DeleteAll( brow->ctx);
    ((WItemMenu *)root_item)->open_children( this, 0, 0);
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, 0);
  }

  *menu_item = menu_p;
  return 1;
}

int WNav::menu_tree_search( char *name, wnav_sMenu **menu_item)
{
  char upname[80];
  cdh_ToUpper( upname, name);
  return menu_tree_search_children( upname, menu_tree, menu_item);
}

int WNav::menu_tree_search_children( char *name, wnav_sMenu *child_list,
		wnav_sMenu **menu_item)
{
  wnav_sMenu *menu_p;
  char *s;
  char *next_search_name;
  char search_name[80];
  char up_title[80];
  int final_search;

  if ( !child_list)
    return WNAV__NOTFOUND;

  strcpy( search_name, name);
  s = strchr( search_name, '-');
  if ( s == 0)
  {
    final_search = 1;
  }
  else
  {
    final_search = 0;
    next_search_name = s + 1;
    *s = 0;
  }

  menu_p = child_list;
  while( menu_p)
  {
    cdh_ToUpper( up_title, menu_p->title);
    if ( strcmp( up_title, search_name) == 0)
    {
      if ( final_search)
      {
        *menu_item = menu_p;
        return 1;
      }
      else
      {
        return menu_tree_search_children( next_search_name, 
		menu_p->child_list, menu_item);
      }
    }
    menu_p = menu_p->next;
  }
  return WNAV__NOTFOUND;
}

int WNav::volume_attached( ldh_tWBContext wbcontext, ldh_tSesContext ldhsession, int pop)
{
  ldhses = ldhsession;
  wbctx = wbcontext;
  wccm_store_ldhses( (void *)this, ldhses);
  if ( window_type != wnav_eWindowType_No)
    show_volume( pop);
  return 1;
}

int WNav::volume_detached()
{
  if ( window_type != wnav_eWindowType_No)
  {
    brow_push_to_volume();
    brow_push_volume();
  }
  ldhses = 0;
  wccm_store_ldhses( (void *)this, 0);
  return 1;
}

void WNav::ldh_refresh( pwr_tObjid new_open)
{
  pwr_tObjid 	open_objid[100];
  int		open_cnt;
  brow_tObject 	*object_list;
  int		object_cnt;
  WItem	 	*object_item;
  int		i, j;
  int		open_type[100];
  char		open_attr[100][80];
  int		open;
  int		found;
  brow_tNode	*node_list;
  int		sel_node_count;
  WItem		*item_sel;
  pwr_tObjid	*sel_objid;
  int		*sel_type;
  char		*sel_attr;

  if ( brow->type != wnav_eBrowType_Volume)
    return;

  // Store all open objects

  open_cnt = 0;
  brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
  for ( i = 0; i < object_cnt; i++)
  {
    if ( (open = brow_IsOpen( object_list[i])))
    {
      brow_GetUserData( object_list[i], (void **)&object_item);
      open_objid[open_cnt] = object_item->objid;
      open_type[open_cnt] = open;
      switch( object_item->type)
      {
        case wnav_eItemType_AttrArray:
          strcpy( open_attr[open_cnt], object_item->name);
          break;
        case wnav_eItemType_AttrArrayOutput:
          strcpy( open_attr[open_cnt], object_item->name);
          break;
        case wnav_eItemType_Attr:
          strcpy( open_attr[open_cnt], object_item->name);
          break;
        case wnav_eItemType_AttrArrayElem:
          strcpy( open_attr[open_cnt], object_item->name);
          break;
        case wnav_eItemType_AttrObject:
          strcpy( open_attr[open_cnt], object_item->name);
          break;
        default:
          ;
      }
      open_type[open_cnt] = open;
      open_cnt++;
      if ( open_cnt == 100)
        break;
    }
  }

  // Store selected object

  brow_GetSelectedNodes( brow->ctx, &node_list, &sel_node_count);
  if ( sel_node_count > 0)
  {
    sel_objid = (pwr_tObjid *)calloc( sel_node_count, sizeof( pwr_tObjid));
    sel_type = (int *)calloc( sel_node_count, sizeof( int));
    sel_attr = (char *)calloc( sel_node_count, 80);

    for ( i = 0; i < sel_node_count; i++)
    {
      brow_GetUserData( node_list[i], (void **)&item_sel);
      sel_objid[i] = item_sel->objid;
      sel_type[i] = item_sel->type;
      switch( item_sel->type)
      {
        case wnav_eItemType_Attr:
        case wnav_eItemType_AttrInput:
        case wnav_eItemType_AttrInputF:
        case wnav_eItemType_AttrInputInv:
        case wnav_eItemType_AttrOutput:
        case wnav_eItemType_AttrArray:
        case wnav_eItemType_AttrObject:
        case wnav_eItemType_AttrArrayOutput:
        case wnav_eItemType_AttrArrayElem:
        case wnav_eItemType_Enum:
        case wnav_eItemType_Mask:
          strcpy( &sel_attr[i * 80], item_sel->name);
          break;
        default:
          ;
      }
    }
    free( node_list);
  }

  brow_SetNodraw( brow->ctx);
  brow_DeleteAll( brow->ctx);
  
  // Restore the rootlist
  get_rootlist();

  if ( cdh_ObjidIsNotNull( new_open))
  {
    // Open parent to a created object
    found = 0;
    for ( i  = 0; i < open_cnt; i++)
    {
      if ( cdh_ObjidIsEqual( new_open, open_objid[i]))
      {
        open_type[i] = wnav_mOpen_Children;
        found = 1;
        break;
      }
    }
    if ( !found && open_cnt < 100)
    {
      open_objid[open_cnt] = new_open;
      open_type[open_cnt] = wnav_mOpen_Children;
      open_cnt++;
    }
  }

  // Open all previously open objects
  for ( i = 0; i < open_cnt; i++)
  {
    brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
    found = 0;
    for ( j = object_cnt - 1; j >= 0; j--)
    {
      brow_GetUserData( object_list[j], (void **)&object_item);
      switch( object_item->type)
      {
        case wnav_eItemType_Object:
          if ( cdh_ObjidIsEqual( open_objid[i], object_item->objid))
          {
            if ( open_type[i] & wnav_mOpen_Children)
              ((WItemObject *)object_item)->open_children( this, 0, 0);
            else if ( open_type[i] & wnav_mOpen_Attributes)
              ((WItemObject *)object_item)->open_attributes( this, 0, 0);
            found = 1;
          }
          break;
        case wnav_eItemType_AttrArray:
          if ( cdh_ObjidIsEqual( open_objid[i], object_item->objid) &&
	       strcmp( object_item->name, open_attr[i]) == 0)
          {
            if ( open_type[i] & wnav_mOpen_Attributes)
              ((WItemAttrArray *)object_item)->open_attributes( 0, 0);
            found = 1;
          }
          break;
        case wnav_eItemType_AttrArrayOutput:
          if ( cdh_ObjidIsEqual( open_objid[i], object_item->objid) &&
	       strcmp( object_item->name, open_attr[i]) == 0)
          {
            if ( open_type[i] & wnav_mOpen_Attributes)
              ((WItemAttrArrayOutput *)object_item)->open_attributes( 0, 0);
            found = 1;
          }
          break;
        case wnav_eItemType_Attr:
          if ( cdh_ObjidIsEqual( open_objid[i], object_item->objid) &&
	       strcmp( object_item->name, open_attr[i]) == 0)
          {
            if ( open_type[i] & wnav_mOpen_Children)
              ((WItemAttr *)object_item)->open_children( 0, 0);
            found = 1;
          }
          break;
        case wnav_eItemType_AttrArrayElem:
          if ( cdh_ObjidIsEqual( open_objid[i], object_item->objid) &&
	       strcmp( object_item->name, open_attr[i]) == 0)
          {
            if ( open_type[i] & wnav_mOpen_Children)
              ((WItemAttrArrayElem *)object_item)->open_children( 0, 0);
            found = 1;
          }
          break;
        case wnav_eItemType_AttrObject:
          if ( cdh_ObjidIsEqual( open_objid[i], object_item->objid) &&
	       strcmp( object_item->name, open_attr[i]) == 0)
          {
            if ( open_type[i] & wnav_mOpen_Attributes)
              ((WItemAttrObject *)object_item)->open_attributes( 0, 0);
            found = 1;
          }
          break;
        default:
          ;
      }
      if ( found)
        break;
    }
  }

  // Select previously selected
  if ( sel_node_count > 0)
  {
    brow_GetObjectList( brow->ctx, &object_list, &object_cnt);
    for ( i = 0; i < sel_node_count; i++)
    {
      for ( j = object_cnt - 1; j >= 0; j--)
      {
        brow_GetUserData( object_list[j], (void **)&object_item);
        found = 0;
        if ( cdh_ObjidIsEqual( sel_objid[i], object_item->objid) &&
	     sel_type[i] == object_item->type)
        {
          switch( object_item->type)
          {
            case wnav_eItemType_Attr:
            case wnav_eItemType_AttrInput:
            case wnav_eItemType_AttrInputF:
            case wnav_eItemType_AttrInputInv:
            case wnav_eItemType_AttrOutput:
            case wnav_eItemType_AttrArray:
            case wnav_eItemType_AttrObject:
            case wnav_eItemType_AttrArrayOutput:
            case wnav_eItemType_AttrArrayElem:
            case wnav_eItemType_Enum:
            case wnav_eItemType_Mask:
              if ( strcmp( &sel_attr[i*80], object_item->name) == 0)
                found = 1;
              break;
            default:
              found = 1;
          }
        }
        if ( found)
        {
          brow_SetInverse( object_item->node, 1);
          brow_SelectInsert( brow->ctx, object_item->node);
          break;
        }
      }
    }
    free( (char *)sel_objid);
    free( (char *)sel_type);
    free( (char *)sel_attr);
  }

  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
}

void WNav::ldh_event( ldh_sEvent *e)
{
  WItem	*item;
  ldh_sEvent *event = e;

  if ( e->nep)
    // Multiple events
    brow_SetNodraw( brow->ctx);

  while ( event) {
    switch (event->Event) 
      {
      case ldh_eEvent_ObjectCopied:
      case ldh_eEvent_ObjectCreated:
	if ( cdh_ObjidIsNull(event->NewParent) ||
	     find( event->NewParent, (void **) &item))
	  ldh_refresh( event->NewParent);
	break;

      case ldh_eEvent_ObjectDeleted:
	if ( cdh_ObjidIsNull(event->NewParent) ||
	     find( event->OldParent, (void **) &item))
	  ldh_refresh( event->NewParent);
	break;

      case ldh_eEvent_ObjectMoved:
	if ( cdh_ObjidIsNull(event->NewParent) ||
	     cdh_ObjidIsNull(event->OldParent) ||
	     find( event->NewParent, (void **) &item) ||
	     find( event->OldParent, (void **) &item))
	  ldh_refresh( event->NewParent);
	break;

      case ldh_eEvent_AttributeModified:
      case ldh_eEvent_ObjectRenamed:
      case ldh_eEvent_BodyModified:
	if ( find( event->Object, (void **) &item))
	  ldh_refresh( pwr_cNObjid);
	break;

      case ldh_eEvent_ObjectTreeCopied:
	if ( find( event->Object, (void **) &item))
	  ldh_refresh( event->Object);
	break;

      case ldh_eEvent_SessionReverted:
	ldh_refresh( pwr_cNObjid);
	break;

      default:
	break;
      }
    event = event->nep;
  }
  if ( e->nep) {
    brow_ResetNodraw( brow->ctx);
    brow_Redraw( brow->ctx, 0);
  }
}

void WNav::refresh()
{
  brow_SelectClear( brow->ctx);
  ldh_refresh( pwr_cNObjid);
}

void WNav::select_object( brow_tObject object)
{
  if ( set_focus_cb)
    (set_focus_cb)( parent_ctx, this);

  set_selection_owner();

  brow_SelectClear( brow->ctx);
  brow_SetInverse( object, 1);
  brow_SelectInsert( brow->ctx, object);
}

void WNav::collapse()
{
  brow_SelectClear( brow->ctx);

  if ( brow->type != wnav_eBrowType_Volume)
    return;

  brow_SetNodraw( brow->ctx);
  brow_DeleteAll( brow->ctx);
  
  // Restore the rootlist
  get_rootlist();

  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
}

void WNav::enable_events( WNavBrow *brow)
{
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1DoubleClickShift, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1DoubleClickCtrl, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1ClickShift, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB2Click, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_ObjectDeleted, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Up, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Down, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_ShiftUp, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_ShiftDown, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_PF1, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_PF2, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_PF3, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_PF4, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Return, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Right, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Left, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_PageUp, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_PageDown, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_ShiftRight, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Key_Tab, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1Press, flow_eEventType_RegionSelect, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB1PressShift, flow_eEventType_RegionAddSelect, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB3Press, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_MB3Down, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Map, flow_eEventType_CallBack, 
	wnav_brow_cb);
  brow_EnableEvent( brow->ctx, flow_eEvent_Radiobutton, flow_eEventType_CallBack, 
	wnav_brow_cb);
}

void WNav::set_selection_owner()
{
  int sts;

  sts = XtOwnSelection( brow_widget, XA_PRIMARY, 
	XtLastTimestampProcessed(flow_Display(brow_widget)),  
	wnav_sel_convert_cb, wnav_sel_lose_cb , NULL);
  if ( !sts)
  {
     message('E', "Failed attempting to become primary selection owner");
     brow_SelectClear( brow->ctx);
     return;
  }	
  selection_owner = 1;
}

void WNav::set_editmode( int value) 
{
  editmode = value;
  if ( brow)
    ldh_refresh(pwr_cNObjid);
}

static void  wnav_reset_avoid_deadlock( WNav *wnav)
{
  wnav->avoid_deadlock = 0;
}

static void  wnav_set_avoid_deadlock( WNav *wnav, int time)
{
  wnav->avoid_deadlock = 1;
  wnav->deadlock_timerid = XtAppAddTimeOut(
	XtWidgetToApplicationContext( wnav->brow_widget), time,
	(XtTimerCallbackProc)wnav_reset_avoid_deadlock, wnav);
}

static Boolean wnav_sel_convert_cb(
    Widget  w,
    Atom    *selection,
    Atom    *target,
    Atom    *type_return,
    XtPointer	*value_return,
    unsigned long   *length_return,
    int	    *format_return)
{
  WNav		*wnav;
  brow_tCtx	browctx;
  int 		sts;
  int		size;
  WItem		*item;
  pwr_tAName   	attr_str;
  pwr_sAttrRef attrref;
  char          name[200];

  BrowCtxFromWidget( w, (void **) &browctx);
  brow_GetCtxUserData( browctx, (void **) &wnav);

  if (*target == XA_TARGETS(flow_Display(wnav->brow_widget))) {
    Atom *targetP;
    Atom *std_targets;
    unsigned long std_length;
    XSelectionRequestEvent *req = XtGetSelectionRequest( w, *selection, 
       (XtRequestId)NULL);

    XmuConvertStandardSelection( w, req->time, selection, target, type_return,
		(caddr_t *)&std_targets, &std_length, format_return);

    *value_return = XtMalloc( sizeof(Atom) * (std_length + 2));
    targetP = *(Atom **) value_return;
    *targetP++ = XA_STRING;
    *targetP++ = XA_TEXT(flow_Display(wnav->brow_widget));
    *length_return = std_length + (targetP - (*(Atom **) value_return));
    bcopy((char *)std_targets, (char *)targetP, sizeof(Atom) * std_length);
    XtFree( (char *)std_targets);
    *type_return = XA_ATOM;
    *format_return = 32;
    return True;
  }

  if (*target == XA_STRING ||
      *target == XA_TEXT(flow_Display(wnav->brow_widget)) ||
      *target == XA_COMPOUND_TEXT(flow_Display(wnav->brow_widget)) ||
      *target == wnav->graph_atom ||
      *target == wnav->objid_atom ||
      *target == wnav->attrref_atom)
  {
    brow_tNode	*node_list;
    int		node_count;
    wnav_eSelectionFormat format;
  
    if ( *target == wnav->graph_atom)
      format = wnav_eSelectionFormat_Graph;
    else if ( *target == wnav->objid_atom)
      format = wnav_eSelectionFormat_Objid;
    else if ( *target == wnav->attrref_atom)
      format = wnav_eSelectionFormat_Attrref;
    else
      format = wnav_eSelectionFormat_User;
	

    brow_GetSelectedNodes( wnav->brow->ctx, &node_list, &node_count);
    if ( !node_count)
      return FALSE;

    brow_GetUserData( node_list[0], (void **)&item);

    switch( item->type) {
    case wnav_eItemType_Attr:
    case wnav_eItemType_AttrInput:
    case wnav_eItemType_AttrInputInv:
    case wnav_eItemType_AttrInputF:
    case wnav_eItemType_AttrOutput:
    case wnav_eItemType_AttrArray:
    case wnav_eItemType_AttrArrayOutput:
    case wnav_eItemType_AttrArrayElem:
    case wnav_eItemType_AttrObject: {
      WItemBaseAttr *aitem = (WItemBaseAttr *)item;

      sts = ldh_ObjidToName( wnav->ldhses, item->objid, ldh_eName_Hierarchy,
	    	attr_str, sizeof(attr_str), &size);
      if ( EVEN(sts)) return FALSE;

      strcat( attr_str, ".");
      strcat( attr_str, aitem->name);
      sts = ldh_NameToAttrRef( wnav->ldhses, attr_str, &attrref);
      if ( EVEN(sts)) return FALSE;
      sts = (wnav->format_selection_cb)( wnav->parent_ctx, attrref, 
					 value_return, length_return, 0, 1, format);
      if ( !sts) return FALSE;
//        sts = ldh_AttrRefToName( wnav->ldhses, &attrref, ldh_eName_Aref, 
//		&name_p, &size);
//        if ( EVEN(sts)) return FALSE;
//        strcpy( name, name_p);
      break;
    }
    case wnav_eItemType_Object:
      memset( &attrref, 0, sizeof(attrref));
      attrref.Objid = item->objid;
      sts = (wnav->format_selection_cb)( wnav->parent_ctx, attrref, 
					 value_return, length_return, 0, 0, format);
      if ( !sts) return FALSE;
        break;
    default:
      brow_GetAnnotation( node_list[0], 0, name, sizeof(name));
      *value_return = XtNewString(name);      
      *length_return = strlen(name) + 1;
    }
    free( node_list);

    if ( *target == XA_COMPOUND_TEXT(flow_Display(wnav->brow_widget)) ||
 	 *target == wnav->graph_atom ||
 	 *target == wnav->objid_atom ||
	 *target == wnav->attrref_atom)
      *type_return = *target;
    else
      *type_return = XA_STRING;
    *format_return = 8;

    return TRUE;
  }
  return FALSE;
}

static void wnav_sel_lose_cb( Widget w, Atom *selection)
{
  WNav		*wnav;
  brow_tCtx	browctx;

  BrowCtxFromWidget( w, (void **) &browctx);
  brow_GetCtxUserData( browctx, (void **) &wnav);

  brow_SelectClear( wnav->brow->ctx);
  wnav->selection_owner = 0;
}

//
// Backcall routine called at creation of the brow widget
// Enable event, create nodeclasses and insert the root objects.
//
static int wnav_init_brow_base_cb( FlowCtx *fctx, void *client_data)
{
  WNav *wnav = (WNav *) client_data;
  BrowCtx *ctx = (BrowCtx *)fctx;
  int		sts;
  pwr_tCmd     	cmd;

  wnav->brow = new WNavBrow( ctx, (void *)wnav);
  wnav->brow_stack[0] = new WNavBrow( ctx, (void *)wnav);
  wnav->brow_cnt++;

  wnav->brow->brow_setup();
  wnav->brow->create_nodeclasses();
  if ( wnav->ldhses)
    wnav->brow->type = wnav_eBrowType_Volume;

  memcpy( wnav->brow_stack[0], wnav->brow, sizeof( *wnav->brow));
  wnav->enable_events( wnav->brow);

  if ( ! wnav->ldhses)
  {
    // Create the root item
    wnav->root_item = new WItemMenu( wnav, "Root", 
	NULL, flow_eDest_After, &wnav->menu_tree, 1);

    // Open the root item
    ((WItemMenu *)wnav->root_item)->open_children( wnav, 0, 0);
  }
  sts = brow_TraceInit( ctx, wnav_trace_connect_bc, 
		wnav_trace_disconnect_bc, wnav_trace_scan_bc);
  wnav->trace_started = 1;
  wnav_trace_scan( wnav);

  // Execute the init file
  if ( wnav->script_filename_cb) {
    strcpy( cmd, "@");
    strcat( cmd, (wnav->script_filename_cb)( wnav->parent_ctx));
    ((WNav *)wnav)->command( cmd);

    // Execute the symbolfile
    if ( wnav->window_type == wnav_eWindowType_W1)
      wnav->gbl.symbolfile_exec( wnav);
  }
  return 1;
}

static int wnav_init_brow_cb( BrowCtx *ctx, void *client_data)
{
  WNav *wnav = (WNav *) client_data;

  wnav->brow_stack[wnav->brow_cnt] = new WNavBrow( ctx, (void *)wnav);

  wnav->brow_stack[wnav->brow_cnt]->brow_setup();
  wnav->brow_stack[wnav->brow_cnt]->create_nodeclasses();
  wnav->enable_events( wnav->brow_stack[wnav->brow_cnt]);

  return 1;
}


ApplListElem::ApplListElem( applist_eType al_type, void *al_ctx, 
	pwr_tObjid al_objid, char *al_name):
	type(al_type), ctx(al_ctx), objid(al_objid), next(NULL)
{
  strcpy( name, al_name);
}

void ApplList::insert( applist_eType type, void *ctx, 
	pwr_tObjid objid, char *name)
{
  ApplListElem *elem = new ApplListElem( type, ctx, objid, name);
  elem->next = root;
  root = elem;
}

void ApplList::remove( void *ctx)
{
  ApplListElem *elem;
  ApplListElem *prev;

  for ( elem = root; elem; elem = elem->next)
  {
    if ( elem->ctx == ctx)
    {
      if ( elem == root)
        root = elem->next;
      else
        prev->next = elem->next;    
      delete elem;
      return;
    }
    prev = elem;
  }
}

int ApplList::find( applist_eType type, pwr_tObjid objid, void **ctx)
{
  ApplListElem *elem;

  for ( elem = root; elem; elem = elem->next)
  {
    if ( elem->type == type && cdh_ObjidIsEqual( elem->objid, objid))
    {
      *ctx = elem->ctx;
      return 1;
    }
  }
  return 0;
}

int ApplList::find( applist_eType type, char *name, void **ctx)
{
  ApplListElem *elem;

  for ( elem = root; elem; elem = elem->next)
  {
    if ( elem->type == type && strcmp( name, elem->name) == 0)
    {
      *ctx = elem->ctx;
      return 1;
    }
  }
  return 0;
}

char *wnav_get_message( int sts)
{
  static char msg[256];

  return msg_GetMsg( sts, msg, sizeof(msg));
}

