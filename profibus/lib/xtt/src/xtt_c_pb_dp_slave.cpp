/* 
 * Proview   $Id: xtt_c_pb_dp_slave.cpp,v 1.6 2008-10-31 12:51:30 claes Exp $
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

/* xtt_c_pb_db_slave.cpp -- xtt methods for Pb_DP_Slave. */

#include "pwr_baseclasses.h"
#include "pwr_profibusclasses.h"
#include "flow_std.h"

#include "co_cdh.h"
#include "xtt_menu.h"
#include "xtt_xnav.h"
#include "rt_xnav_msg.h"
#include "pwr_privilege.h"

#include "rt_pb_gsd.h"
#include "rt_pb_gsd_attr.h"
#include "xtt_c_pb_dp_slave.h"
#include "rt_pb_msg.h"

int xtt_pb_dp_slave_help_cb( void *sctx, const char *text)
{
  pwr_tCmd cmd;
  xtt_slave_sCtx *ctx = (xtt_slave_sCtx *)sctx;

  strcpy( cmd, "help ");
  strcat( cmd, text);
  return ((XNav *)ctx->editor_ctx)->command( cmd);
}

void xtt_pb_dp_slave_close_cb( void *sctx)
{
  xtt_slave_sCtx *ctx = (xtt_slave_sCtx *)sctx;
  delete ctx->attr;
  delete ctx->gsd;
  free( (char *)ctx);
}

int xtt_pb_dp_slave_save_cb( void *sctx)
{
  return 1;
}

static pwr_tStatus xtt_pb_dp_slave_load_modules( xtt_slave_sCtx *ctx)
{
  pwr_tOid oid;
  pwr_tCid cid;
  int found;
  pwr_tObjName name;
  pwr_tString40 module_name;
  int sts;
  pwr_tAttrRef maref, aaref;

  for ( sts = gdh_GetChild( ctx->aref.Objid, &oid);
	ODD(sts);
	sts = gdh_GetNextSibling( oid, &oid)) {

    // Check that this is a module
    sts = gdh_GetObjectClass( oid, &cid);
    if ( EVEN(sts)) return sts;

    found = 0;
    for ( int i = 0; ; i++) {
      if ( ctx->gsd->module_classlist[i].cid == 0)
	break;
      if ( ctx->gsd->module_classlist[i].cid == cid) {
	found = 1;
	break;
      }
    }
    if ( !found)
      // This is not a known module object
      continue;

    // Get name
    sts = gdh_ObjidToName( oid, name, sizeof(name), cdh_mName_object);
    if ( EVEN(sts)) return sts;

    maref = cdh_ObjidToAref( oid);

    // Get ModuleName attribute
    sts = gdh_ArefANameToAref( &maref, "ModuleName", &aaref);
    if ( EVEN(sts)) return sts;
      
    sts = gdh_GetObjectInfoAttrref( &aaref, module_name, sizeof(module_name));
    if ( EVEN(sts)) return sts;

    ctx->gsd->add_module_conf( cid, oid, name, module_name);
  }

  // Set address
  pwr_tUInt16 address;
  
  sts = gdh_ArefANameToAref( &ctx->aref, "SlaveAddress", &aaref);
  if ( EVEN(sts)) return sts;

  sts = gdh_GetObjectInfoAttrref( &aaref, &address, sizeof(address));
  if ( EVEN(sts)) return sts;

  ctx->gsd->address = address;

  // Set byte order
  pwr_tByteOrderingEnum byte_order;
  
  sts = gdh_ArefANameToAref( &ctx->aref, "ByteOrdering", &aaref);
  if ( EVEN(sts)) return sts;

  sts = gdh_GetObjectInfoAttrref( &aaref, &byte_order, sizeof(byte_order));
  if ( EVEN(sts)) return sts;

  ctx->gsd->byte_order = byte_order;

  // Set Ext_User_Prm_Data
  pwr_tUInt8 prm_user_data[256];
  pwr_tUInt16 prm_user_data_len;
  int len;
  
  sts = gdh_ArefANameToAref( &ctx->aref, "PrmUserData", &aaref);
  if ( EVEN(sts)) return sts;

  sts = gdh_GetObjectInfoAttrref( &aaref, prm_user_data, sizeof(prm_user_data));
  if ( EVEN(sts)) return sts;

  sts = gdh_ArefANameToAref( &ctx->aref, "PrmUserDataLen", &aaref);
  if ( EVEN(sts)) return sts;

  sts = gdh_GetObjectInfoAttrref( &aaref, &prm_user_data_len, sizeof(prm_user_data_len));
  if ( EVEN(sts)) return sts;

  len = prm_user_data_len;
  if ( len != 0) {
    sts = ctx->gsd->unpack_ext_user_prm_data( (char *)prm_user_data, len);
    if ( EVEN(sts)) return sts;
  }
  return 1;
}


pwr_tStatus xtt_pb_dp_slave_create_ctx( pwr_tAttrRef aref, void *editor_ctx, 
					xtt_slave_sCtx **ctxp)
{
  pwr_tAName name;
  pwr_tString80 gsdfile;
  int sts;
  pwr_tFileName fname;
  pwr_tCid cid;
  pwr_tOid oid;
  int found;
  int mc_cnt;
  int module_cnt;
  pwr_tAttrRef aaref;

  sts = gdh_ObjidToName( aref.Objid, 
			name, sizeof(name), cdh_mName_volumeStrict);
  if ( EVEN(sts)) return sts;

  sts = gdh_ArefANameToAref( &aref, "GSDFile", &aaref);
  if ( EVEN(sts)) return sts;

  sts = gdh_GetObjectInfoAttrref( &aaref, gsdfile, sizeof(gsdfile));
  if ( EVEN(sts)) return sts;
  if ( strcmp( gsdfile, "") == 0) {
    return PB__GSDATTR;
  }

  xtt_slave_sCtx *ctx = (xtt_slave_sCtx *) calloc( 1, sizeof(xtt_slave_sCtx));
  ctx->aref = aref;

  // Count modules
  module_cnt = 0;
  for ( sts = gdh_GetChild( aref.Objid, &oid);
	ODD(sts);
	sts = gdh_GetNextSibling( oid, &oid)) {
    module_cnt++;
  }

  ctx->mc = (gsd_sModuleClass *) calloc( module_cnt + 2, sizeof(gsd_sModuleClass));
  mc_cnt = 0;
  ctx->editor_ctx = editor_ctx;
  
  ctx->mc[0].cid = pwr_cClass_Pb_Module;
  sts = gdh_ObjidToName( cdh_ClassIdToObjid(ctx->mc[0].cid),  
			 ctx->mc[0].name, sizeof(ctx->mc[0].name), cdh_mName_object);
  if ( EVEN(sts)) return sts;
  mc_cnt++;

  for ( sts = gdh_GetChild( aref.Objid, &oid);
	ODD(sts);
	sts = gdh_GetNextSibling( oid, &oid)) {
    sts = gdh_GetObjectClass( oid, &cid);
    if ( EVEN(sts)) return sts;

    found = 0;
    for ( int i = 0; i < mc_cnt; i++) {
      if ( ctx->mc[i].cid == cid) {
	found = 1;
	break;
      }
    }
    if ( found)
      continue;

    ctx->mc[mc_cnt].cid = cid;
    sts = gdh_ObjidToName( cdh_ClassIdToObjid(cid),
			   ctx->mc[mc_cnt].name, sizeof(ctx->mc[0].name), cdh_mName_object);
    if ( EVEN(sts)) return sts;
    mc_cnt++;
  }

  if ( strchr( gsdfile, '/') == 0) {
    strcpy( fname, "$pwrp_exe/");
    strcat( fname, gsdfile);
  }
  else
    strcpy( fname, gsdfile);
    
  ctx->gsd = new pb_gsd();
  sts = ctx->gsd->read( fname);
  if ( EVEN(sts))
    return sts;
    
  ctx->gsd->set_classes( ctx->mc);

  sts = xtt_pb_dp_slave_load_modules( ctx);
  if ( EVEN(sts)) return sts;

  *ctxp = ctx;
  return 1;
}





