/* 
 * Proview   $Id: wb_c_backup.cpp,v 1.2 2007-04-26 12:37:46 claes Exp $
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

/* wb_c_backup.c -- work bench methods of the Backup class. */

#include <string.h>
#undef Status
#include "pwr.h"
#include "wb_pwrs.h"
#include "wb_ldh_msg.h"
#include "wb_pwrb_msg.h"
#include "pwr_baseclasses.h"
#include "wb_ldh.h"
#include "wb_wsx.h"
#include "wb_wsx_msg.h"
#include "wb_session.h"

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
    If father of this object has an "ActualValue" attribute, then make this object
    refer to that attribute.
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
    If father of ASup has an "ActualValue" attribute, then make this ASup
    refer to this attribute.
  */

  sts = ldh_ObjidToName(Session, Father, ldh_eName_Hierarchy, Name,
    sizeof(Name), &size);
  if (EVEN(sts)) return PWRB__SUCCESS;

  strcat(Name, ".ActualValue");

  sts = ldh_NameToAttrRef(Session, Name, &Attribute);
  if (EVEN(sts)) return PWRB__SUCCESS;

  sts = ldh_SetObjectPar(Session, Object, "RtBody", "DataName", Name,
    strlen(Name) + 1);
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
  char *s;

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
  
  // Backup on whole signal objects is not allowed
  switch ( data_a.tid()) {
  case pwr_cClass_Do:
  case pwr_cClass_Dv:
  case pwr_cClass_Ao:
  case pwr_cClass_Av:
  case pwr_cClass_Io:
  case pwr_cClass_Iv:
  case pwr_cClass_Po:
  case pwr_cClass_Co:
  case pwr_cClass_Di:
  case pwr_cClass_Ai:
  case pwr_cClass_Ii:
    wsx_error_msg( Session, WSX__BCKINVALID, Object, ErrorCount, WarningCount);
    return PWRB__SUCCESS;
  default: ;
  }

  // If DataName is a signal, the attribute has to be ActualValue
  pwr_tAName aname;

  strncpy( aname, data_a.longName().c_str(), sizeof(aname));
  if (( s = strchr( aname, '.'))) {
    *s = 0;

    wb_attribute data_aobject = sp->attribute(aname);
    if ( !data_aobject) return data_aobject.sts();

    switch ( data_aobject.cid()) {
    case pwr_cClass_Do:
    case pwr_cClass_Dv:
    case pwr_cClass_Ao:
    case pwr_cClass_Av:
    case pwr_cClass_Io:
    case pwr_cClass_Iv:
    case pwr_cClass_Po:
    case pwr_cClass_Co: {
      if ( strcmp( s+1, "ActualValue") != 0)
	wsx_error_msg( Session, WSX__BCKINVALID, Object, ErrorCount, WarningCount);
      break;
    }
    case pwr_cClass_Di:
    case pwr_cClass_Ai:
    case pwr_cClass_Ii: {
      wsx_error_msg( Session, WSX__BCKINVALID, Object, ErrorCount, WarningCount);
      break;
    }
    default: ;
    }
  }

  return PWRB__SUCCESS;
}


//  Every method to be exported to the workbench should be registred here.

pwr_dExport pwr_BindMethods(Backup) = {
  pwr_BindMethod(PostCreate),
  pwr_BindMethod(PostMove),
  pwr_BindMethod(SyntaxCheck),
  pwr_NullMethod
};
