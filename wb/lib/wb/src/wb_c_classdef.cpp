/* 
 * Proview   $Id: wb_c_classdef.cpp,v 1.2 2007-04-26 12:36:53 claes Exp $
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

/* wb_c_classdef.c -- work bench methods of the ClassDef class. */

#undef Status
#include <string.h>
#include "wb_pwrs.h"
#include "wb_pwrs_msg.h"
#include "wb_ldh.h"
#include "pwr_baseclasses.h"
#include "wb_wnav.h"

static pwr_tStatus AnteCreate (
  ldh_tSesContext   Session,
  pwr_tObjid	    Father,
  pwr_tClassId	    Class
) {
  pwr_tCid cid;
  pwr_tStatus sts;

  if ( cdh_ObjidIsNull( Father))
    return PWRS__POSCLASSDEF;

  // Check that the father is of class ClassHier
  sts = ldh_GetObjectClass( Session, Father, &cid);
  if ( EVEN(sts) || cid != pwr_eClass_ClassHier)
    return PWRS__POSCLASSDEF;
  return PWRS__SUCCESS;
}

static pwr_tStatus OpenObjectGraph (
  ldh_sMenuCall *ip
)
{
  int 		sts;
  char		name[32];
  int		size;
 
  sts = ldh_ObjidToName( ip->PointedSession, ip->Pointed.Objid, ldh_eName_Object, 
	name, sizeof(name), &size);
  if ( EVEN(sts)) return sts;

  cdh_ToLower( name, name);
  ip->wnav->ge_new( name);
  return PWRS__SUCCESS;
}

static pwr_tStatus ConfigFc (
  ldh_sMenuCall *ip
)
{
  int 		sts;
  pwr_tOName   	name;
  pwr_tOName   	pname;
  pwr_tObjName  oname;
  int		size;
  pwr_tCid	cid;
  pwr_tTid	tid;
  unsigned int  flags;
  pwr_tInt32	int_val;
  pwr_tObjid	oid;
  pwr_tObjid	coid;
  int plcconnect = 0;
  int plctemplate = 0;
  char 		str[80];
  pwr_sMenuButton   mb;
  
  sts = ldh_GetChild(ip->PointedSession, ip->Pointed.Objid, &oid);
  if ( ODD(sts)) {
    ip->wnav->wow->DisplayError( "Error", 
		      "ClassDef is already configured\n  Object has child");
    return 0;
  }

  /* If argument is PlcConnect, configure a connected function object
     i.e. an $Intern named PlcConnnect, a PlcConnect MenuRef, and
     connectmethod 10 */
  sts = ldh_ReadObjectBody(ip->PointedSession,
			   ip->ItemList[ip->ChosenItem].MenuObject,
			   "SysBody", &mb, sizeof(pwr_sMenuButton));

  if ( strcmp(mb.MethodArguments[0], "PlcConnect") == 0) {
    plcconnect = 1;
    plctemplate = 1;
  }
  else if ( strcmp(mb.MethodArguments[0], "CCode") == 0) {
    plcconnect = 0;
    plctemplate = 0;
  }
  else if ( strcmp(mb.MethodArguments[0], "PlcConnectCCode") == 0) {
    plcconnect = 1;
    plctemplate = 0;
  }
  else {
    plcconnect = 0;
    plctemplate = 1;
  }

  sts = ldh_ObjidToName( ip->PointedSession, ip->Pointed.Objid, ldh_eName_Hierarchy, 
			 pname, sizeof(pname), &size);
  if ( EVEN(sts)) return sts;

  sts = ldh_ObjidToName( ip->PointedSession, ip->Pointed.Objid, ldh_eName_Object, 
			 oname, sizeof(oname), &size);
  if ( EVEN(sts)) return sts;

  sts = ldh_CreateObject( ip->PointedSession, &oid, "RtBody", pwr_eClass_ObjBodyDef,
			    ip->Pointed.Objid, ldh_eDest_IntoFirst);

  if ( plcconnect) {
    // Create PlcConnect attribute
    strcpy( name, pname);
    strcat( name, "-");
    strcat( name, "RtBody");
    sts = ldh_NameToObjid( ip->PointedSession, &oid, name);
    if ( EVEN(sts)) return sts;

    sts = ldh_CreateObject( ip->PointedSession, &coid, "PlcConnect", pwr_eClass_Intern,
			    oid, ldh_eDest_IntoLast);
    if ( ODD(sts)) {
      tid = pwr_eType_AttrRef;

      sts = ldh_SetObjectPar(ip->PointedSession, coid, "SysBody", "TypeRef", (char *)&tid,
			     sizeof(tid));
      if ( EVEN(sts)) return sts;
    }
    if ( !plctemplate) {
      // Create PlcConnectP attribute
      sts = ldh_CreateObject( ip->PointedSession, &coid, "PlcConnectP", pwr_eClass_Intern,
			    oid, ldh_eDest_IntoLast);
      if ( ODD(sts)) {
	tid = pwr_eType_Char;

	sts = ldh_SetObjectPar(ip->PointedSession, coid, "SysBody", "TypeRef", (char *)&tid,
			       sizeof(tid));
	if ( EVEN(sts)) return sts;

	flags = PWR_MASK_INVISIBLE | PWR_MASK_POINTER | PWR_MASK_PRIVATE;

	sts = ldh_SetObjectPar(ip->PointedSession, coid, "SysBody", "Flags", (char *)&flags,
			       sizeof(flags));
	if ( EVEN(sts)) return sts;
      }
    }
  }

  sts = ldh_CreateObject( ip->PointedSession, &oid, "DevBody", pwr_eClass_ObjBodyDef,
			    ip->Pointed.Objid, ldh_eDest_IntoLast);
  if ( EVEN(sts)) {
    // The object already exist
  }

  strcpy( name, pname);
  strcat( name, "-");
  strcat( name, "DevBody");
  sts = ldh_NameToObjid( ip->PointedSession, &oid, name);
  if ( EVEN(sts)) return sts;

  sts = ldh_CreateObject( ip->PointedSession, &oid, "PlcNode", pwr_eClass_Buffer,
			    oid, ldh_eDest_IntoLast);
  if ( ODD(sts)) {
    cid = pwr_eClass_PlcNode;

    sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "Class", (char *)&cid,
			   sizeof(cid));
    if ( EVEN(sts)) return sts;
  }

  sts = ldh_CreateObject( ip->PointedSession, &oid, "GraphPlcNode",
			  pwr_eClass_GraphPlcNode,
			  ip->Pointed.Objid, ldh_eDest_IntoLast);
  if ( ODD(sts)) {
    if ( plctemplate) {
      cid = pwr_cClass_windowplc;
      int_val = 1;

      sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "subwindows", 
			   (char *)&int_val, sizeof(int_val));
      if ( EVEN(sts)) return sts;

      sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "subwindow_class[0]", 
			     (char *)&cid, sizeof(cid));
      if ( EVEN(sts)) return sts;
    }

    int_val = 1;
    sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "segname_annotation",
			   (char *)&int_val, sizeof(int_val));
    if ( EVEN(sts)) return sts;

    if ( plctemplate)
      int_val = 58;
    else if ( plcconnect)
      int_val = 35;
    else
      int_val = 4;

    sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "compmethod",
			   (char *)&int_val, sizeof(int_val));
    if ( EVEN(sts)) return sts;

    if ( plcconnect) {
      int_val = 10;
      sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "connectmethod",
			     (char *)&int_val, sizeof(int_val));
      if ( EVEN(sts)) return sts;
    }

    int_val = 2;
    sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "executeordermethod",
			   (char *)&int_val, sizeof(int_val));
    if ( EVEN(sts)) return sts;

    oname[15] = 0;
    sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "objname",
			   oname, sizeof(pwr_tString16));
    if ( EVEN(sts)) return sts;

    sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "graphname",
			   oname, sizeof(pwr_tString16));
    if ( EVEN(sts)) return sts;
  }

  if ( plcconnect) {
    sts = ldh_CreateObject( ip->PointedSession, &oid, "RtXtt",
			    pwr_eClass_RtMenu,
			    ip->Pointed.Objid, ldh_eDest_IntoLast);
    strcpy( name, pname);
    strcat( name, "-");
    strcat( name, "RtXtt");
    sts = ldh_NameToObjid( ip->PointedSession, &oid, name);
    if ( EVEN(sts)) return sts;

    sts = ldh_CreateObject( ip->PointedSession, &oid, "PlcConnect", pwr_eClass_MenuRef,
			    oid, ldh_eDest_IntoLast);
    if ( ODD(sts)) {
      strcpy( str, "PlcConnect");
      
      sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "ButtonName", 
			     str, sizeof(pwr_tString40));
      if ( EVEN(sts)) return sts;
      
      sts = ldh_SetObjectPar(ip->PointedSession, oid, "SysBody", "RefAttribute", 
			     str, sizeof(pwr_tString40));
      if ( EVEN(sts)) return sts;
    }
  }

  if ( plctemplate) {
    sts = ldh_CreateObject( ip->PointedSession, &oid, "Code",
			    pwr_cClass_PlcTemplate,
			    ip->Pointed.Objid, ldh_eDest_IntoLast);
  }
  return PWRS__SUCCESS;
}

pwr_dExport pwr_BindMethods($ClassDef) = {
  pwr_BindMethod(AnteCreate),
  pwr_BindMethod(OpenObjectGraph),
  pwr_BindMethod(ConfigFc),
  pwr_NullMethod
};





