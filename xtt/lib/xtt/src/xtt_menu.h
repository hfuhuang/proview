/* 
 * Proview   $Id: xtt_menu.h,v 1.9 2006-01-25 14:18:45 claes Exp $
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

#ifndef xtt_menu_h
#define xtt_menu_h

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef pwr_class_h
#include "pwr_class.h"
#endif

#ifndef PWR_BASECLASSES_H
#include "pwr_baseclasses.h"
#endif


typedef struct xmenu_s_MenuItem	xmenu_sMenuItem;
typedef struct xmenu_s_MenuCall	xmenu_sMenuCall;
typedef union xmenu_m_MenuFlags	xmenu_mMenuFlags;

typedef enum {
  xmenu_mUtility_XNav      = 1 << 0,
  xmenu_mUtility_Trace     = 1 << 1,
  xmenu_mUtility_Simulate  = 1 << 2,
  xmenu_mUtility_EventList = 1 << 3,
  xmenu_mUtility_EventLog  = 1 << 4,
  xmenu_mUtility_Ge        = 1 << 5,
  xmenu_mUtility_AttrEditor= 1 << 6
} xmenu_mUtility;

typedef enum {
  xmenu_eItemType_Object 	= 1,
  xmenu_eItemType_AttrObject 	= 2,
  xmenu_eItemType_Attribute 	= 3,
  xmenu_eItemType_Crossref 	= 4
} xmenu_eItemType;

typedef enum {
  xmenu_eMenuItem__ = 0,
  xmenu_eMenuItem_Button,
  xmenu_eMenuItem_Cascade,
  xmenu_eMenuItem_Separator,
  xmenu_eMenuItem_Ref,
  xmenu_eMenuItem_
} xmenu_eMenuItem;

typedef enum {
  xmenu_eMenuSet__ = 0,
  xmenu_eMenuSet_Attribute,
  xmenu_eMenuSet_Class,
  xmenu_eMenuSet_Many,
  xmenu_eMenuSet_None,
  xmenu_eMenuSet_Object,
  xmenu_eMenuSet_
} xmenu_eMenuSet;

union xmenu_m_MenuFlags {
  struct {
    pwr_tBit Sensitive : 1;
  } f;

  pwr_tBitMask	m;

#define xmenu_mMenuFlags_Sensitive  1
};

struct xmenu_s_MenuItem {
  pwr_tString80		Name;
  pwr_tUInt32		Level;
  xmenu_eMenuItem	Item;
  xmenu_mMenuFlags	Flags;
  pwr_tString80		Method;
  pwr_tString80		Filter;
  pwr_sAttrRef		CurrentObject;
  pwr_tObjid		MenuObject;
  pwr_tString40         MethodArguments[5];
  pwr_tString40         FilterArguments[5];
};

struct xmenu_s_MenuCall {
  void			*EditorContext;
  void			*WindowContext;
  xmenu_eMenuSet	PointedSet;
  pwr_sAttrRef		Pointed;
  xmenu_eMenuSet	SelectedSet;
  pwr_sAttrRef		*Selected;
  pwr_tUInt32		SelectCount;
  xmenu_sMenuItem	*ItemList;
  pwr_tUInt32		ItemCount;
  pwr_tUInt32		ChosenItem;
  xmenu_mUtility        Caller;
  xmenu_eItemType       ItemType;
  unsigned long         Priv;
  char                  Arg[80];
};



typedef struct {
  pwr_tObjName MethodName;
  pwr_tStatus (*Method)( xmenu_sMenuCall *);
} xtt_sMethodBinding;

typedef struct {
  pwr_tObjName ClassName;
  xtt_sMethodBinding (*Methods)[];
} xtt_sClassBinding;

/* Base methods */
#if defined (OS_LYNX)  || defined (OS_LINUX)
# define pwr_BindXttMethods(Class) xtt_sMethodBinding pwr_g ## Class ## _XttMethods[]
# define pwr_BindXttClasses(Type) xtt_sClassBinding pwr_g ## Type ## _XttClassMethods[]
# if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
# define pwr_BindXttClass(Class) {#Class, (xtt_sMethodBinding (*)[0])pwr_g ## Class ## _XttMethods}
# else
# define pwr_BindXttClass(Class) {#Class, (xtt_sMethodBinding (*)[])pwr_g ## Class ## _XttMethods}
# endif
# define pwr_BindXttMethod(Method) {#Method, (pwr_tStatus (*)( xmenu_sMenuCall *))Method}
#elif defined (OS_VMS)
# define pwr_BindXttMethods(Class) xtt_sMethodBinding pwr_g## Class ##_XttMethods[]
# define pwr_BindXttClasses(Type) xtt_sClassBinding pwr_g## Type ##_XttClassMethods[]
# define pwr_BindXttClass(Class) {#Class, (xtt_sMethodBinding (*)[])pwr_g## Class ##_XttMethods}
# define pwr_BindXttMethod(Method) {#Method, (pwr_tStatus (*)( xmenu_sMenuCall *))Method}
#else
# define pwr_BindXttMethods(Class) xtt_sMethodBinding pwr_g/**/Class/**/_XttMethods[]
# define pwr_BindXttClasses(Type) xtt_sClassBinding pwr_g/**/Type/**/_XttClassMethods[]
# define pwr_BindXttClass(Class) {"Class", pwr_g/**/Class/**/_XttMethods}
# define pwr_BindXttMethod(Method) {"Method", (pwr_tStatus (*)())Method}
#endif

#define pwr_NullMethod {"", NULL}

#define pwr_NullClass {"", NULL}


#endif










