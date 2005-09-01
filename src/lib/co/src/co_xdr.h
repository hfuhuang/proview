/** 
 * Proview   $Id: co_xdr.h,v 1.3 2005-09-01 14:57:52 claes Exp $
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

#ifndef co_xdr_h
#define co_xdr_h

#ifdef __cplusplus
extern "C" {
#endif

/* co_xdr.h -- xdr routines
   This module contains the XDR-routines for 
   PROVIEW's data types */

#include <rpc/rpc.h>

#ifndef pwr_h
# include "pwr.h"
#endif


#ifndef co_cdh_h
# include "co_cdh.h"
#endif

#ifndef co_platform_h
# include "co_platform.h"
#endif


#define xdr_pwr_tFloat32 xdr_float
#define xdr_pwr_tFloat64 xdr_double

pwr_tBoolean co_IsXdrNeeded(co_sPlatform *plat1, co_sPlatform *plat2);



bool_t
xdr_cdh_sFamily(XDR *xdrs, cdh_sFamily *objp);

bool_t
xdr_cdh_sObjName(XDR *xdrs, cdh_sObjName *objp);

bool_t 
xdr_co_sPlatform(XDR *xdrs, co_sPlatform *objp);

bool_t
xdr_pwr_eType(XDR *xdrs, pwr_eType *objp);

bool_t
xdr_pwr_mAdef(XDR *xdrs, pwr_mAdef *objp);

bool_t
xdr_pwr_sClassDef(XDR *xdrs, pwr_sClassDef *objp);

bool_t
xdr_pwr_sObjBodyDef(XDR *xdrs, pwr_sObjBodyDef *objp);

bool_t
xdr_pwr_sParInfo(XDR *xdrs, pwr_sParInfo *objp);

bool_t
xdr_pwr_sParam(XDR *xdrs, pwr_sParam *objp);

bool_t
xdr_pwr_tAix(XDR *xdrs, pwr_tAix *objp);

bool_t
xdr_pwr_tBit(XDR *xdrs, pwr_tBit *objp);

bool_t
xdr_pwr_tBitMask(XDR *xdrs, pwr_tBitMask *objp);

bool_t
xdr_pwr_tBoolean(XDR *xdrs, pwr_tBoolean *objp);

bool_t
xdr_pwr_tGeneration(XDR *xdrs, pwr_tGeneration *objp);

bool_t
xdr_pwr_tChar(XDR *xdrs, pwr_tChar *objp);

bool_t
xdr_pwr_tInt8(XDR *xdrs, pwr_tInt8 *objp);

bool_t
xdr_pwr_tInt16(XDR *xdrs, pwr_tInt16 *objp);

bool_t
xdr_pwr_tInt32(XDR *xdrs, pwr_tInt32 *objp);

bool_t
xdr_pwr_tInt64(XDR *xdrs, pwr_tInt64 *objp);

bool_t
xdr_pwr_tUInt8(XDR *xdrs, pwr_tUInt8 *objp);

bool_t
xdr_pwr_tUInt16(XDR *xdrs, pwr_tUInt16 *objp);

bool_t
xdr_pwr_tUInt32(XDR *xdrs, pwr_tUInt32 *objp);

bool_t
xdr_pwr_tUInt64(XDR *xdrs, pwr_tUInt64 *objp);

bool_t
xdr_pwr_tStatus(XDR *xdrs, pwr_tStatus *objp);

bool_t
xdr_pwr_tVolumeId(XDR *xdrs, pwr_tVolumeId *objp);

bool_t
xdr_pwr_tObjectIx(XDR *xdrs, pwr_tObjectIx *objp);

bool_t
xdr_pwr_tObjid(XDR *xdrs, pwr_tObjid *objp);

bool_t
xdr_pwr_tClassId(XDR *xdrs, pwr_tClassId *objp);

bool_t
xdr_pwr_tTypeId(XDR *xdrs, pwr_tTypeId *objp);

bool_t
xdr_pwr_tVersion(XDR *xdrs, pwr_tVersion *objp);

bool_t
xdr_pwr_tPwrVersion(XDR *xdrs, pwr_tPwrVersion *objp);

bool_t
xdr_pwr_tProjVersion(XDR *xdrs, pwr_tProjVersion *objp);

bool_t
xdr_pwr_tUserId(XDR *xdrs, pwr_tUserId *objp);

bool_t
xdr_pwr_tDbId(XDR *xdrs, pwr_tDbId *objp);

bool_t
xdr_pwr_tNodeId(XDR *xdrs, pwr_tNodeId *objp);

bool_t
xdr_pwr_tNodeIndex(XDR *xdrs, pwr_tNodeIndex *objp);

bool_t
xdr_pwr_tRefId(XDR *xdrs, pwr_tRefId *objp);

bool_t
xdr_pwr_tDlid(XDR *xdrs, pwr_tDlid *objp);

bool_t
xdr_pwr_tSubid(XDR *xdrs, pwr_tSubid *objp);

bool_t
xdr_pwr_tTime(XDR *xdrs, pwr_tTime *objp);

bool_t
xdr_pwr_tDeltaTime(XDR *xdrs, pwr_tDeltaTime *objp);

bool_t
xdr_pwr_tObjName(XDR *xdrs, pwr_tObjName *objp);

bool_t
xdr_pwr_tPgmName(XDR *xdrs, pwr_tPgmName *objp);

bool_t
xdr_pwr_tXRef(XDR *xdrs, pwr_tXRef *objp);

bool_t
xdr_pwr_tGraphName(XDR *xdrs, pwr_tGraphName *objp);

bool_t
xdr_pwr_tStructName(XDR *xdrs, pwr_tStructName *objp);

bool_t
xdr_pwr_tAttrName(XDR *xdrs, pwr_tAttrName *objp);

bool_t
xdr_pwr_tPathName(XDR *xdrs, pwr_tPathName *objp);

bool_t
xdr_pwr_tFullName(XDR *xdrs, pwr_tFullName *objp);

bool_t
xdr_pwr_tString256(XDR *xdrs, pwr_tString256 *objp);

bool_t
xdr_pwr_tString132(XDR *xdrs, pwr_tString132 *objp);

bool_t
xdr_pwr_tString80(XDR *xdrs, pwr_tString80 *objp);

bool_t
xdr_pwr_tString40(XDR *xdrs, pwr_tString40 *objp);

bool_t
xdr_pwr_tString32(XDR *xdrs, pwr_tString32 *objp);

bool_t
xdr_pwr_tString16(XDR *xdrs, pwr_tString16 *objp);

bool_t
xdr_pwr_tString8(XDR *xdrs, pwr_tString8 *objp);

#ifdef __cplusplus
bool_t
xdr_pwr_tString(XDR *xdrs, char *objp);
#else
bool_t
xdr_pwr_tString(XDR *xdrs, pwr_tString *objp);
#endif

bool_t
xdr_pwr_mAttrRef(XDR *xdrs, pwr_mAttrRef *objp);

bool_t 
xdr_pwr_sAttrRef(XDR *xdrs, pwr_sAttrRef *objp);

#ifdef __cplusplus
}
#endif

#endif
