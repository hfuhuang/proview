/* 
 * Proview   $Id: wb_c_dstrend.cpp,v 1.2 2008-01-17 14:20:48 claes Exp $
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

/* wb_c_dstrend.c -- work bench methods of the DsTrend class. */

#include <string.h>
#include "wb_pwrs.h"
#include "wb_ldh_msg.h"
#include "wb_pwrb_msg.h"
#include "pwr_baseclasses.h"
#include "wb_ldh.h"
#include "wb_session.h"
#include "wb_wsx.h"
#include "wb_pwrb_msg.h"

//
//  PostCreate
//
static pwr_tStatus PostCreate (
  ldh_tSesContext Session,
  pwr_tObjid	  Object,
  pwr_tObjid	  Father,
  pwr_tClassId	  Class
) {
  pwr_tStatus sts;
  int size;
  pwr_tAName Name;
  pwr_sAttrRef Attribute;
  
  /*
    If father of DsFast has an "ActualValue" attribute, then make this DsFast
    refer to this attribute.
  */

  sts = ldh_ObjidToName(Session, Father, ldh_eName_Hierarchy, Name,
    sizeof(Name), &size);
  if (EVEN(sts)) return PWRB__SUCCESS;

  strcat(Name, ".ActualValue");

  sts = ldh_NameToAttrRef(Session, Name, &Attribute);
  if (EVEN(sts)) {
    memset(&Attribute, 0, sizeof(Attribute));
  }

  sts = ldh_SetObjectPar(Session, Object, "RtBody", "DataName", (char *)&Attribute,
    sizeof(Attribute));
  if (EVEN(sts)) return PWRB__SUCCESS;

  return PWRB__SUCCESS;
}

//
//  PostMove
//
static pwr_tStatus PostMove (
  ldh_tSesContext Session,
  pwr_tObjid	  Object,
  pwr_tObjid	  Father,
  pwr_tClassId	  Class
) {
  pwr_tStatus sts;
  int size;
  pwr_tAName Name;
  pwr_sAttrRef Attribute;
  
  /*
    If father of DsTrend has an "ActualValue" attribute, then make this ASup
    refer to this attribute.
  */

  sts = ldh_ObjidToName(Session, Father, ldh_eName_Hierarchy, Name,
    sizeof(Name), &size);
  if (EVEN(sts)) return PWRB__SUCCESS;

  strcat(Name, ".ActualValue");

  sts = ldh_NameToAttrRef(Session, Name, &Attribute);
  if (EVEN(sts)) return PWRB__SUCCESS;

  sts = ldh_SetObjectPar(Session, Object, "RtBody", "DataName", (char *)&Attribute,
    sizeof(Attribute));
  if (EVEN(sts)) return PWRB__SUCCESS;

  return PWRB__SUCCESS;
}

//
//  Syntax check.
//
static pwr_tStatus SyntaxCheck (
  ldh_tSesContext Session,
  pwr_tAttrRef Object,	      /* current object */
  int *ErrorCount,	      /* accumulated error count */
  int *WarningCount	      /* accumulated waring count */
) {

  // Check DataName
  wb_session *sp = (wb_session *)Session;
  pwr_tAttrRef dataname_aref;

  wb_attribute a = sp->attribute( &Object);
  if ( !a) return a.sts();

  wb_attribute dataname_a( a, 0, "DataName");
  if (!dataname_a) return dataname_a.sts();
    
  dataname_a.value( &dataname_aref);
  if ( !dataname_a) return dataname_a.sts();

  wb_attribute data_a = sp->attribute( &dataname_aref);
  if ( !data_a) {
    wsx_error_msg_str( Session, "Bad DataName reference", Object, 'E', ErrorCount, WarningCount);
    return PWRB__SUCCESS;
  }
  
  // Check DataName type
  switch ( data_a.tid()) {
  case	pwr_eType_Boolean:
  case	pwr_eType_Float32:
  case	pwr_eType_Float64:
  case	pwr_eType_Int8:
  case	pwr_eType_Int16:
  case	pwr_eType_Int32:
  case	pwr_eType_UInt8:
  case	pwr_eType_UInt16:
  case	pwr_eType_UInt32:
    break;
  default:
    wsx_error_msg_str( Session, "DataName type not supported", Object, 'E', ErrorCount, WarningCount);
  }
  return PWRB__SUCCESS;
}


//  Every method to be exported to the workbench should be registred here.

pwr_dExport pwr_BindMethods(DsTrend) = {
  pwr_BindMethod(PostCreate),
  pwr_BindMethod(PostMove),
  pwr_BindMethod(SyntaxCheck),
  pwr_NullMethod
};
