/* 
 * Proview   $Id: wb_c_nodeconfig.cpp,v 1.1 2007-01-04 07:29:03 claes Exp $
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

/* wb_c_nodeconfig.c -- work bench methods of the NodeConfig class. */

#include <string.h>
#include "co_dcli.h"
#include "wb_pwrs.h"
#include "wb_pwrs_msg.h"
#include "wb_ldh.h"
#include "wb_session.h"
#include "wb_wsx.h"
#include "wb_pwrb_msg.h"
#include "pwr_baseclasses.h"

//
//  PostCreate method
//
static pwr_tStatus PostCreate (
  ldh_tSesContext   Session,
  pwr_tObjid	    Object,
  pwr_tObjid	    Father,
  pwr_tClassId	    Class
) {
  pwr_tObjid oid, poid;
  pwr_tClassId cid;
  pwr_tStatus sts;

  sts = ldh_ClassNameToId(Session, &cid, "RootVolumeLoad");
  sts = ldh_CreateObject(Session, &oid, "O1", cid, Object, ldh_eDest_IntoLast);

  sts = ldh_ClassNameToId(Session, &cid, "Distribute");
  sts = ldh_CreateObject(Session, &poid, "Distribute", cid, Object, ldh_eDest_IntoLast);

  return PWRS__SUCCESS;
}

//
//  Syntax check method
//
static pwr_tStatus SyntaxCheck (
  ldh_tSesContext Session,
  pwr_tAttrRef Object,	      /* current object */
  int *ErrorCount,	      /* accumulated error count */
  int *WarningCount	      /* accumulated waring count */
) {
  wb_session *sp = (wb_session *)Session;
  pwr_tString80 str;

  wb_object o = sp->object( Object.Objid);
  if ( !o) return o.sts();

  wb_object p = o.parent();
  if ( !p || p.cid() != pwr_cClass_BusConfig)
    wsx_error_msg_str( Session, "Bad parent", Object, 'E', ErrorCount, WarningCount);

  if ( Object.Objid.vid != ldh_cDirectoryVolume)
    wsx_error_msg_str( Session, "Not a DirectoryVolume", Object, 'E', ErrorCount, WarningCount);

  // Check NodeName
  wb_attribute a = sp->attribute( Object.Objid, "RtBody", "NodeName");
  if (!a) return a.sts();
    
  a.value( &str);
  if ( !a) return a.sts();

  dcli_trim( str, str);
  if ( strcmp( str, "") == 0)
    wsx_error_msg_str( Session, "NodeName is missing", Object, 'E', ErrorCount, WarningCount);

  // Check OperatingSystem
  a = sp->attribute( Object.Objid, "RtBody", "OperatingSystem");
  if (!a) return a.sts();
    
  pwr_tEnum opsys;
  a.value( &opsys);
  if ( !a) return a.sts();

  if ( opsys <= pwr_mOpSys__ || opsys >= pwr_mOpSys_)
    wsx_error_msg_str( Session, "Invalid OperatingSystem", Object, 'E', ErrorCount, WarningCount);

  // Check Address
  a = sp->attribute( Object.Objid, "RtBody", "Address");
  if (!a) return a.sts();
    
  a.value( &str);
  if ( !a) return a.sts();

  unsigned char adr1, adr2, adr3, adr4;
  int num;
  num = sscanf( str, "%hhu.%hhu.%hhu.%hhu", &adr1, &adr2, &adr3, &adr4);
  if ( num != 4)
    wsx_error_msg_str( Session, "Syntax error in Address", Object, 'E', ErrorCount, WarningCount);
  else if ( adr1 == 0 && adr2 == 0 && adr3 == 0 && adr4 == 0)
    wsx_error_msg_str( Session, "Address is zero", Object, 'E', ErrorCount, WarningCount);

  // Look for RootVolumeLoad child
  bool found = 0;
  for ( wb_object child = o.first(); child; child = child.after()) {
    if ( child.cid() == pwr_cClass_RootVolumeLoad) {
      found = 1;
      break;
    }
  }
  if ( !found)
    wsx_error_msg_str( Session, "RootVolumeLoad object is missing", Object, 'E', ErrorCount, WarningCount);

  return PWRB__SUCCESS;
}

pwr_dExport pwr_BindMethods(NodeConfig) = {
  pwr_BindMethod(PostCreate),
  pwr_BindMethod(SyntaxCheck),
  pwr_NullMethod
};

