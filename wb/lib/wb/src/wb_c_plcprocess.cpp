/* 
 * Proview   $Id: wb_c_plcprocess.cpp,v 1.2 2007-01-30 07:13:02 claes Exp $
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

/* wb_c_plcprocess.c -- work bench methods of the PlcProcess class. */

#include "wb_pwrs.h"
#include "wb_pwrb_msg.h"
#include "wb_ldh.h"
#include "wb_build.h"

static pwr_tStatus PostCreate (
  ldh_tSesContext   Session,
  pwr_tOid	    Object,
  pwr_tOid	    Father,
  pwr_tCid	    Class
) {
  pwr_tOid oid;
  pwr_tCid cid;
  pwr_tStatus sts;
  pwr_tFloat32 scan_time = 0.1;

  sts = ldh_ClassNameToId(Session, &cid, "PlcThread");
  if ( EVEN(sts)) return sts;

  sts = ldh_CreateObject(Session, &oid, "100ms", cid, Object, ldh_eDest_IntoLast); 
  if ( EVEN(sts)) return sts;

  sts = ldh_SetObjectPar(Session, oid, "RtBody", "ScanTime", (char *)&scan_time,
			 sizeof(scan_time));
  if ( EVEN(sts)) return sts;

  return PWRB__SUCCESS;
}

static pwr_tStatus Build (
  ldh_sMenuCall *ip
)
{
  wb_build build( *(wb_session *)ip->PointedSession, ip->wnav);

  build.opt = ip->wnav->gbl.build;
  build.application( ip->Pointed.Objid);

  if ( build.sts() == PWRB__NOBUILT)
    ip->wnav->message( 'I', "Nothing to build");
  return build.sts();
}

pwr_dExport pwr_BindMethods(PlcProcess) = {
  pwr_BindMethod(PostCreate),
  pwr_BindMethod(Build),
  pwr_NullMethod
};







