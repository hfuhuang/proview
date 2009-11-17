/* 
 * Proview   $Id$
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

/* wb_c_pndevice.c -- work bench methods of the PnDevice class. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <map>

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "pwr_profibusclasses.h"
#include "wb_env.h"
#include "flow.h"
#include "flow_ctx.h"
#include "flow_api.h"
#include "flow_browctx.h"
#include "flow_browapi.h"

#include "co_msg.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "cow_wow.h"
#include "cow_msgwindow.h"
#include "cow_xhelp.h"
#include "rt_pn_gsdml.h"
#include "rt_pn_gsdml_attr.h"
#include "rt_pn_gsdml_attrnav.h"
#include "wb_c_pndevice.h"

#include "wb_pwrs.h"
#include "wb_ldh_msg.h"
#include "wb_ldh.h"
#include "wb_pwrb_msg.h"
#include "rt_pb_msg.h"
#include "wb_wnav.h"
#include "wb_wsx.h"

using namespace std;

class ChanItem {
 public:
  ChanItem() : subslot_number(0), representation(0), number(0), use_as_bit(0), cid(0) {}

  unsigned int subslot_number;
  unsigned int representation;
  unsigned int number;
  unsigned int use_as_bit;
  pwr_tCid cid;
  char description[80];
};

static int pndevice_add_channels( device_sCtx *ctx, gsdml_VirtualSubmoduleItem *vi, int subslot_number,
				  vector<ChanItem>& input_vect, vector<ChanItem>& output_vect);
static int pndevice_check_io( device_sCtx *ctx, gsdml_VirtualSubmoduleList *vsl, 
			      vector<ChanItem>& input_vect, vector<ChanItem>& output_vect);

/*----------------------------------------------------------------------------*\
  Configure the slave from gsd file.
\*----------------------------------------------------------------------------*/

static void get_subcid( ldh_tSession ldhses, pwr_tCid cid, vector<pwr_tCid>& v)
{
  pwr_tCid subcid;
  pwr_tStatus sts;

  for ( sts = ldh_GetSubClass( ldhses, cid, pwr_cNCid, &subcid);
	ODD(sts);
	sts = ldh_GetSubClass( ldhses, cid, subcid, &subcid)) {
    v.push_back( subcid);
    get_subcid( ldhses, subcid, v);
  }
}

int pndevice_help_cb( void *sctx, const char *text)
{
  pwr_tCmd cmd;
  device_sCtx *ctx = (device_sCtx *)sctx;

  strcpy( cmd, "help ");
  strcat( cmd, text);
  return ((WNav *)ctx->editor_ctx)->command( cmd);
}

void pndevice_close_cb( void *sctx)
{
  device_sCtx *ctx = (device_sCtx *)sctx;
  delete ctx->attr;
  delete ctx->gsdml;
  free( (char *)ctx);
}

int pndevice_save_cb( void *sctx)
{
  device_sCtx *ctx = (device_sCtx *)sctx;
  pwr_tStatus sts;
  pwr_tOName name;
  int size;
  pwr_tOid oid;
  
  // Syntax check
  if ( ctx->attr->attrnav->device_num == 0) {
    MsgWindow::message( 'E', "Device type not selected");
    return PB__SYNTAX;
  }

  for ( unsigned int i = 1; i < ctx->attr->attrnav->dev_data.slot_data.size(); i++) {
    if ( ctx->attr->attrnav->dev_data.slot_data[i]->module_enum_number == 0 &&
	 ctx->attr->attrnav->dev_data.slot_data[i]->module_class != 0) {
      // Module class selected but not module type
      char msg[20];

      sprintf( msg, "Slot %d", i);
      MsgWindow::message( 'E', "Module type not selected, ", msg);
    }
    if ( ctx->attr->attrnav->dev_data.slot_data[i]->module_class == 0 &&
	 ctx->attr->attrnav->dev_data.slot_data[i]->module_enum_number != 0) {
      // Module type selected but not module class
      char msg[20];

      sprintf( msg, "Slot %d", i);
      MsgWindow::message( 'E', "Module class not selected, ", msg);
    }
  }

  // Save configuration
  ((WNav *)ctx->editor_ctx)->set_nodraw();

  sts = ldh_ObjidToName(ctx->ldhses, ctx->aref.Objid, 
			ldh_eName_Hierarchy, name, sizeof(name), &size);
  if ( EVEN(sts)) goto return_now;

  // Do a temporary rename all module object to avoid name collisions
  for ( sts = ldh_GetChild( ctx->ldhses, ctx->aref.Objid, &oid);
	ODD(sts);
	sts = ldh_GetNextSibling( ctx->ldhses, oid, &oid)) {
    sts = ldh_ObjidToName( ctx->ldhses,  oid, cdh_mName_object, name,
			   sizeof(name), &size);
    if ( EVEN(sts)) goto return_now;

    strcat( name, "__tmp");
    sts = ldh_ChangeObjectName( ctx->ldhses, oid, name);
    if ( EVEN(sts)) goto return_now;
  }

  for ( unsigned int i = 1; i < ctx->attr->attrnav->dev_data.slot_data.size(); i++) {
    GsdmlSlotData *slot = ctx->attr->attrnav->dev_data.slot_data[i];
    pwr_tCid cid;
    pwr_tOid prev;
    pwr_tObjName mname;

    int found = 0;
    sprintf( mname, "M%d", slot->slot_idx);

    if ( cdh_ObjidIsNotNull( slot->module_oid)) {

      // Find module object
      sts = ldh_GetObjectClass( ctx->ldhses, slot->module_oid, &cid);
      if ( ODD(sts)) {
	if ( cid == slot->module_class) {
	  // Module object found and class is ok
	  found = 1;

	  // Check if name is changed
	  sts = ldh_ObjidToName( ctx->ldhses, slot->module_oid, cdh_mName_object, name,
				 sizeof(name), &size);
	  if ( EVEN(sts)) goto return_now;

	  if ( strcmp( name, mname) != 0) {
	    // Change name
	    sts = ldh_ChangeObjectName( ctx->ldhses, slot->module_oid, mname);
	    if ( EVEN(sts)) goto return_now;
	  }	  
	  
	  // Check that sibling position is right
	  sts = ldh_GetPreviousSibling( ctx->ldhses, slot->module_oid, &prev);
	  if ( i == 0) {
	    // Should be first sibling
	    if ( ODD(sts)) {
	      // Move to first sibling
	      sts = ldh_MoveObject( ctx->ldhses, slot->module_oid, ctx->aref.Objid, ldh_eDest_IntoFirst);
	    }
	  }
	  else {
	    if ( (ODD(sts) && 
		  cdh_ObjidIsNotEqual( ctx->attr->attrnav->dev_data.slot_data[i-1]->module_oid, prev)) ||
		 EVEN(sts)) {
	      // Move to next to i-1
	      sts = ldh_MoveObject( ctx->ldhses, slot->module_oid, 
				    ctx->attr->attrnav->dev_data.slot_data[i-1]->module_oid, 
				    ldh_eDest_After);
	    }
	  }
	}
	else {
	  // New class, delete current object
	  sts = ldh_DeleteObjectTree( ctx->ldhses, slot->module_oid, 0);
	}
      }
    }
    if ( !found && slot->module_class != pwr_cNCid) {
      // Create a new module object
      if ( i == 1)
	sts = ldh_CreateObject( ctx->ldhses, &slot->module_oid, mname, slot->module_class,
				ctx->aref.Objid, ldh_eDest_IntoFirst);
      else
	sts = ldh_CreateObject( ctx->ldhses, &slot->module_oid, mname, slot->module_class,
				ctx->attr->attrnav->dev_data.slot_data[i-1]->module_oid, 
				ldh_eDest_After);
      if ( EVEN(sts)) {
	printf( "Error creating module object, %d\n", sts);
	sts = 0;
	goto return_now;
      }
    }
  }

  // Remove modules that wasn't configured any more
  pwr_tOid moid[100];
  int mcnt;
  int found;
  mcnt = 0;
  for ( sts = ldh_GetChild( ctx->ldhses, ctx->aref.Objid, &oid);
	ODD(sts);
	sts = ldh_GetNextSibling( ctx->ldhses, oid, &oid)) {
    found = 0;
    for ( unsigned int i = 0; i < ctx->attr->attrnav->dev_data.slot_data.size(); i++) {
      if ( cdh_ObjidIsEqual( ctx->attr->attrnav->dev_data.slot_data[i]->module_oid, oid)) {
	found = 1;
	break;
      }
    }
    if ( !found) {
      moid[mcnt++] = oid;
      if ( mcnt > (int) (sizeof(moid)/sizeof(moid[0])))
	break;	 
    }
  }

  for ( int i = 0; i < mcnt; i++)
    sts = ldh_DeleteObjectTree( ctx->ldhses, moid[i], 0);

  for ( unsigned int i = 0; i < ctx->attr->attrnav->dev_data.slot_data.size(); i++) {
    GsdmlSlotData *slot = ctx->attr->attrnav->dev_data.slot_data[i];
    
    if ( i == 0) {
      vector<ChanItem> input_vect;
      vector<ChanItem> output_vect;

      sts = pndevice_check_io( ctx, ctx->attr->attrnav->device_item->VirtualSubmoduleList,
			       input_vect, output_vect);
      if ( sts == PB__CREATECHAN) {
	char msg[20];
	sprintf( msg, "Slot %d", i);
	MsgWindow::message( 'W', "Unexpected datatype, channel not created, ", msg);
      }
    }
    else {
      if ( slot->module_class == pwr_cClass_PnModule) {
	vector<ChanItem> input_vect;
	vector<ChanItem> output_vect;
	gsdml_UseableModules *um = ctx->gsdml->ApplicationProcess->DeviceAccessPointList->
	  DeviceAccessPointItem[ctx->attr->attrnav->device_num-1]->UseableModules;

	if ( !um)
	  continue;
	gsdml_ModuleItem *mi = (gsdml_ModuleItem *)um->
	  ModuleItemRef[slot->module_enum_number-1]->Body.ModuleItemTarget.p;

	sts = pndevice_check_io( ctx, mi->VirtualSubmoduleList, input_vect, output_vect);
	if ( sts == PB__CREATECHAN) {
	  char msg[20];
	  sprintf( msg, "Slot %d", i);
	  MsgWindow::message( 'W', "Unexpected datatype, channel not created, ", msg);
	}

	// Create the channels
	if ( EVEN(ldh_GetChild( ctx->ldhses, slot->module_oid, &oid))) {
	  unsigned int chan_cnt = 0;
	  for ( unsigned int j = 0; j < input_vect.size(); j++) {
	    char name[80];
	    sprintf( name, "Ch%02u", chan_cnt++);
	    sts = ldh_CreateObject( ctx->ldhses, &oid, name, input_vect[j].cid,
				    slot->module_oid, ldh_eDest_IntoLast);
	    if ( EVEN(sts)) goto return_now;

	    pwr_tAttrRef aaref;
	    pwr_tAttrRef chanaref = cdh_ObjidToAref( oid);

	    // Set Representation
	    pwr_tEnum representation = input_vect[j].representation;
	    sts = ldh_ArefANameToAref( ctx->ldhses, &chanaref, "Representation", &aaref);
	    if ( EVEN(sts)) goto return_now;
	    
	    sts = ldh_WriteAttribute( ctx->ldhses, &aaref, &representation, sizeof(representation));
	    if ( EVEN(sts)) goto return_now;

	    // Set Number
	    pwr_tUInt16 number = input_vect[j].number;
	    sts = ldh_ArefANameToAref( ctx->ldhses, &chanaref, "Number", &aaref);
	    if ( EVEN(sts)) goto return_now;
	    
	    sts = ldh_WriteAttribute( ctx->ldhses, &aaref, &number, sizeof(number));
	    if ( EVEN(sts)) goto return_now;

	    // Set Description
	    pwr_tString80 description;
	    strncpy( description, input_vect[j].description, sizeof(description));
	    sts = ldh_ArefANameToAref( ctx->ldhses, &chanaref, "Description", &aaref);
	    if ( EVEN(sts)) goto return_now;
	    
	    sts = ldh_WriteAttribute( ctx->ldhses, &aaref, description, sizeof(description));
	    if ( EVEN(sts)) goto return_now;
	  }
	  for ( unsigned int j = 0; j < output_vect.size(); j++) {
	    char name[80];
	    sprintf( name, "Ch%02u", chan_cnt++);
	    sts = ldh_CreateObject( ctx->ldhses, &oid, name, output_vect[j].cid,
				    slot->module_oid, ldh_eDest_IntoLast);
	    if ( EVEN(sts)) goto return_now;

	    pwr_tAttrRef aaref;
	    pwr_tAttrRef chanaref = cdh_ObjidToAref( oid);

	    // Set Representation
	    pwr_tEnum representation = output_vect[j].representation;
	    sts = ldh_ArefANameToAref( ctx->ldhses, &chanaref, "Representation", &aaref);
	    if ( EVEN(sts)) goto return_now;
	    
	    sts = ldh_WriteAttribute( ctx->ldhses, &aaref, &representation, sizeof(representation));
	    if ( EVEN(sts)) goto return_now;

	    // Set Number
	    pwr_tUInt16 number = output_vect[j].number;
	    sts = ldh_ArefANameToAref( ctx->ldhses, &chanaref, "Number", &aaref);
	    if ( EVEN(sts)) goto return_now;
	    
	    sts = ldh_WriteAttribute( ctx->ldhses, &aaref, &number, sizeof(number));
	    if ( EVEN(sts)) goto return_now;

	    // Set Description
	    pwr_tString80 description;
	    strncpy( description, output_vect[j].description, sizeof(description));
	    sts = ldh_ArefANameToAref( ctx->ldhses, &chanaref, "Description", &aaref);
	    if ( EVEN(sts)) goto return_now;
	    
	    sts = ldh_WriteAttribute( ctx->ldhses, &aaref, description, sizeof(description));
	    if ( EVEN(sts)) goto return_now;
	  }
	}
      }
      else {
	// Remove existing channels
	vector<pwr_tOid> chanvect;
	pwr_tCid cid;
      
	for ( sts = ldh_GetChild( ctx->ldhses, slot->module_oid, &oid);
	      ODD(sts);
	      sts = ldh_GetNextSibling( ctx->ldhses, oid, &oid)) {
	  sts = ldh_GetObjectClass( ctx->ldhses, oid, &cid);
	  if ( EVEN(sts)) goto return_now;
	
	  switch ( cid) {
	  case pwr_cClass_ChanDi:
	  case pwr_cClass_ChanDo:
	  case pwr_cClass_ChanAi:
	  case pwr_cClass_ChanAo:
	  case pwr_cClass_ChanIi:
	  case pwr_cClass_ChanIo:
	    chanvect.push_back( oid);
	    break;
	  default: ;
	  }
	}
	for ( unsigned int i = 0; i < chanvect.size(); i++) {
	  sts = ldh_DeleteObject( ctx->ldhses, chanvect[i]);
	  if ( EVEN(sts)) goto return_now;
	}
      }
    }
  }
  sts = PWRB__SUCCESS;

 return_now:
  ((WNav *)ctx->editor_ctx)->reset_nodraw();
  return sts;
}
  
static int pndevice_check_io( device_sCtx *ctx, gsdml_VirtualSubmoduleList *vsl, 
			      vector<ChanItem>& input_vect, vector<ChanItem>& output_vect)
{
  int sts;

  if ( vsl) {
    unsigned int subslot_number = 0;
	
    for ( unsigned int i = 0; i < vsl->VirtualSubmoduleItem.size(); i++) {
      if ( strcmp( vsl->VirtualSubmoduleItem[i]->Body.FixedInSubslots.str, "") == 0) {
	// FixedInSubslots not supplied, default subslot number is 1 
	
	if ( vsl->VirtualSubmoduleItem.size() == 1)
	  subslot_number = 1;
	else
	  subslot_number++;
	
	sts = pndevice_add_channels( ctx, vsl->VirtualSubmoduleItem[i], subslot_number,
				     input_vect, output_vect);
	if ( EVEN(sts)) return sts;
      }
      else {
	// FixedInSubslots supplied, create channels for all fixed subslots
	
	gsdml_Valuelist *vl = new gsdml_Valuelist(  vsl->
			      VirtualSubmoduleItem[i]->Body.FixedInSubslots.str);
	gsdml_ValuelistIterator iter( vl);
	
	for ( unsigned int j = iter.begin(); j != iter.end(); j = iter.next()) {
	  subslot_number = j;
	  
	  sts = pndevice_add_channels( ctx, vsl->VirtualSubmoduleItem[i], subslot_number,
				       input_vect, output_vect);
	  if (EVEN(sts)) { 
	    delete vl; 
	    return sts;
	  }
	}
	delete vl;
      }
    }
  }
  return PB__SUCCESS;
}
    
static int pndevice_add_channels( device_sCtx *ctx, gsdml_VirtualSubmoduleItem *vi, int subslot_number,
				  vector<ChanItem>& input_vect, vector<ChanItem>& output_vect)
{

  // Find input data
  if ( vi->IOData && vi->IOData->Input) {
    for ( unsigned int i = 0; 
	  i < vi->IOData->Input->DataItem.size(); 
	  i++) {
      gsdml_DataItem *di = vi->IOData->Input->DataItem[i];
      gsdml_eValueDataType datatype;
      
      ctx->attr->attrnav->gsdml->string_to_value_datatype( di->Body.DataType, &datatype);
      
      if ( !di->Body.UseAsBits) {
	unsigned int representation;
	int invalid_type = 0;

	switch ( datatype) {
	case gsdml_eValueDataType_Integer8:	
	  representation = pwr_eDataRepEnum_Int8;
	  break;
	case gsdml_eValueDataType_Unsigned8:
	  representation = pwr_eDataRepEnum_UInt8;
	  break;
	case gsdml_eValueDataType_Integer16:
	  representation = pwr_eDataRepEnum_Int16;
	  break;
	case gsdml_eValueDataType_Unsigned16:
	  representation = pwr_eDataRepEnum_UInt16;
	  break;
	case gsdml_eValueDataType_Integer32:
	  representation = pwr_eDataRepEnum_Int32;
	  break;
	case gsdml_eValueDataType_Unsigned32:
	  representation = pwr_eDataRepEnum_UInt32;
	  break;
	case gsdml_eValueDataType_Integer64:
	  representation = pwr_eDataRepEnum_Int64;
	  break;
	case gsdml_eValueDataType_Unsigned64:
	  representation = pwr_eDataRepEnum_UInt64;
	  break;
	case gsdml_eValueDataType_Float32:
	  representation = pwr_eDataRepEnum_Float32;
	  break;
	case gsdml_eValueDataType_Float64:
	  representation = pwr_eDataRepEnum_Float64;
	  break;
	default:
	  invalid_type = 1;
	}

	if ( invalid_type)
	  return PB__CREATECHAN;

	ChanItem ci;
	ci.subslot_number = subslot_number;
	ci.number = 0;
	ci.representation = representation;
	ci.use_as_bit = 0;	
	ci.cid = pwr_cClass_ChanAi;
	strncpy( ci.description, (char *)di->Body.TextId.p, sizeof(ci.description));
	ci.description[sizeof(ci.description)-1] = 0;
	
	input_vect.push_back( ci);
      }
      else {
	// Use as bits
	unsigned int bits;
	unsigned int representation;

	switch ( datatype) {
	case gsdml_eValueDataType_Integer8:
	case gsdml_eValueDataType_Unsigned8:
	  representation = pwr_eDataRepEnum_Bit8;
	  bits = 8;
	  break;
	case gsdml_eValueDataType_Integer16:
	case gsdml_eValueDataType_Unsigned16:
	  representation = pwr_eDataRepEnum_Bit16;
	  bits = 16;
	  break;
	case gsdml_eValueDataType_Integer32:
	case gsdml_eValueDataType_Unsigned32:
	  representation = pwr_eDataRepEnum_Bit32;
	  bits = 32;
	  break;
	case gsdml_eValueDataType_Integer64:
	case gsdml_eValueDataType_Unsigned64:
	  representation = pwr_eDataRepEnum_Bit64;
	  bits = 64;
	  break;
	default:
	  bits = 0;
	}
	if ( di->BitDataItem.size() == 0) {
	  // Add all bits
	  for ( unsigned int j = 0; j < bits; j++) {
	    // Add Channel
	    ChanItem ci;
	    ci.subslot_number = subslot_number;
	    ci.number = j;
	    ci.representation = representation;
	    ci.use_as_bit = 1;
	    ci.cid = pwr_cClass_ChanDi;
	    strncpy( ci.description, (char *)di->Body.TextId.p, sizeof(ci.description));
	    ci.description[sizeof(ci.description)-2] = 0;			   
	    
	    input_vect.push_back( ci);
	  }
	}
	else {
	  for ( unsigned int j = 0; j < di->BitDataItem.size(); j++) {
	    // Add channel
	    ChanItem ci;
	    ci.subslot_number = subslot_number;
	    ci.number = di->BitDataItem[j]->Body.BitOffset;
	    ci.representation = representation;
	    ci.use_as_bit = 1;
	    ci.cid = pwr_cClass_ChanDi;
	    strncpy( ci.description, (char *)di->BitDataItem[j]->Body.TextId.p, sizeof(ci.description));
	    ci.description[sizeof(ci.description)-2] = 0;			   

	    input_vect.push_back( ci);
	  }
	}
      }
    }
  }

  // Find output data
  if ( vi->IOData && vi->IOData->Output) {
    for ( unsigned int i = 0; 
	  i < vi->IOData->Output->DataItem.size(); 
	  i++) {
      gsdml_DataItem *di = vi->IOData->Output->DataItem[i];
      gsdml_eValueDataType datatype;
      
      ctx->attr->attrnav->gsdml->string_to_value_datatype( di->Body.DataType, &datatype);
      
      if ( !di->Body.UseAsBits) {
	unsigned int representation;
	int invalid_type = 0;

	switch ( datatype) {
	case gsdml_eValueDataType_Integer8:	
	  representation = pwr_eDataRepEnum_Int8;
	  break;
	case gsdml_eValueDataType_Unsigned8:
	  representation = pwr_eDataRepEnum_UInt8;
	  break;
	case gsdml_eValueDataType_Integer16:
	  representation = pwr_eDataRepEnum_Int16;
	  break;
	case gsdml_eValueDataType_Unsigned16:
	  representation = pwr_eDataRepEnum_UInt16;
	  break;
	case gsdml_eValueDataType_Integer32:
	  representation = pwr_eDataRepEnum_Int32;
	  break;
	case gsdml_eValueDataType_Unsigned32:
	  representation = pwr_eDataRepEnum_UInt32;
	  break;
	case gsdml_eValueDataType_Integer64:
	  representation = pwr_eDataRepEnum_Int64;
	  break;
	case gsdml_eValueDataType_Unsigned64:
	  representation = pwr_eDataRepEnum_UInt64;
	  break;
	case gsdml_eValueDataType_Float32:
	  representation = pwr_eDataRepEnum_Float32;
	  break;
	case gsdml_eValueDataType_Float64:
	  representation = pwr_eDataRepEnum_Float64;
	  break;
	default:
	  invalid_type = 1;
	}

	if ( invalid_type) {
	  printf("GSDML-Error, Invalid type, unable to create channel\n");
	  return 0;
	}

	ChanItem ci;
	ci.subslot_number = subslot_number;
	ci.number = 0;
	ci.representation = representation;
	ci.use_as_bit = 0;	
	ci.cid = pwr_cClass_ChanAo;
	strncpy( ci.description, (char *)di->Body.TextId.p, sizeof(ci.description));
	ci.description[sizeof(ci.description)-2] = 0;			   

	output_vect.push_back( ci);
      }
      else {
	// Use as bits
	unsigned int bits;
	unsigned int representation;

	switch ( datatype) {
	case gsdml_eValueDataType_Integer8:
	case gsdml_eValueDataType_Unsigned8:
	  representation = pwr_eDataRepEnum_Bit8;
	  bits = 8;
	  break;
	case gsdml_eValueDataType_Integer16:
	case gsdml_eValueDataType_Unsigned16:
	  representation = pwr_eDataRepEnum_Bit16;
	  bits = 16;
	  break;
	case gsdml_eValueDataType_Integer32:
	case gsdml_eValueDataType_Unsigned32:
	  representation = pwr_eDataRepEnum_Bit32;
	  bits = 32;
	  break;
	case gsdml_eValueDataType_Integer64:
	case gsdml_eValueDataType_Unsigned64:
	  representation = pwr_eDataRepEnum_Bit64;
	  bits = 64;
	  break;
	default:
	  bits = 0;
	}
	if ( di->BitDataItem.size() == 0) {
	  // Add all bits
	  for ( unsigned int j = 0; j < bits; j++) {
	    // Add Channel
	    ChanItem ci;
	    ci.subslot_number = subslot_number;
	    ci.number = j;
	    ci.representation = representation;
	    ci.use_as_bit = 1;
	    ci.cid = pwr_cClass_ChanDo;
	    strncpy( ci.description, (char *)di->Body.TextId.p, sizeof(ci.description));
	    ci.description[sizeof(ci.description)-2] = 0;			   
	    
	    output_vect.push_back( ci);
	  }
	}
	else {
	  for ( unsigned int j = 0; j < di->BitDataItem.size(); j++) {
	    // Add channel
	    ChanItem ci;
	    ci.subslot_number = subslot_number;
	    ci.number = di->BitDataItem[j]->Body.BitOffset;
	    ci.representation = representation;
	    ci.use_as_bit = 1;
	    ci.cid = pwr_cClass_ChanDo;
	    strncpy( ci.description, (char *)di->BitDataItem[j]->Body.TextId.p, sizeof(ci.description));
	    ci.description[sizeof(ci.description)-2] = 0;			   

	    output_vect.push_back( ci);
	  }
	}
      }
    }
  }
  return PB__SUCCESS;
}

pwr_tStatus pndevice_create_ctx( ldh_tSession ldhses, pwr_tAttrRef aref, 
				    void *editor_ctx, device_sCtx **ctxp)
{
  pwr_tOName name;
  char *gsdmlfile;
  int size;
  int sts;
  pwr_tFileName fname;
  ldh_sSessInfo Info;
  vector<pwr_tCid> mcv;

  sts = ldh_GetSessionInfo( ldhses, &Info);


  sts = ldh_ObjidToName( ldhses, aref.Objid, 
			 ldh_eName_Hierarchy, name, sizeof(name), &size);
  if ( EVEN(sts)) return sts;

  sts = ldh_GetObjectPar( ldhses, aref.Objid, "RtBody",
			  "GSDMLfile", &gsdmlfile, &size);
  if ( EVEN(sts)) return sts;
  if ( strcmp( gsdmlfile, "") == 0) {
    free( gsdmlfile);  
    return PB__GSDATTR;
  }

  device_sCtx *ctx = (device_sCtx *) calloc( 1, sizeof(device_sCtx));
  ctx->ldhses = ldhses;
  ctx->aref = aref;
  ctx->editor_ctx = editor_ctx;
  ctx->edit_mode = (ODD(sts) && Info.Access == ldh_eAccess_ReadWrite) &&
    ldh_LocalObject( ldhses, aref.Objid);

  get_subcid( ctx->ldhses, pwr_cClass_PnModule, mcv);
  ctx->mc = (gsdml_sModuleClass *) calloc( mcv.size() + 2, sizeof(gsdml_sModuleClass));

  ctx->mc[0].cid = pwr_cClass_PnModule;
  sts = ldh_ObjidToName( ctx->ldhses, cdh_ClassIdToObjid(ctx->mc[0].cid), cdh_mName_object, 
			 ctx->mc[0].name, sizeof(ctx->mc[0].name), &size);
  if ( EVEN(sts)) return sts;

  for ( int i = 1; i <= (int) mcv.size(); i++) {
    ctx->mc[i].cid = mcv[i-1];
    sts = ldh_ObjidToName( ctx->ldhses, cdh_ClassIdToObjid(ctx->mc[i].cid), cdh_mName_object, 
			   ctx->mc[i].name, sizeof(ctx->mc[0].name), &size);
    if ( EVEN(sts)) return sts;
  }

  if ( strchr( gsdmlfile, '/') == 0) {
    strcpy( fname, "$pwrp_exe/");
    strcat( fname, gsdmlfile);
  }
  else
    strcpy( fname, gsdmlfile);
  free( gsdmlfile);
    
  ctx->gsdml = new pn_gsdml();
  sts = ctx->gsdml->read( fname);
  if ( EVEN(sts))
    return sts;
  ctx->gsdml->build();
  ctx->gsdml->set_classes( ctx->mc);

  *ctxp = ctx;
  return 1;
}

pwr_tStatus pndevice_init( device_sCtx *ctx)
{
  pwr_tObjName module_name;
  pwr_tOid module_oid;
  int corrupt = 0;
  unsigned int idx;
  pwr_tStatus sts;
  int size;
  
  // Identify module objects
  for ( sts = ldh_GetChild( ctx->ldhses, ctx->aref.Objid, &module_oid);
	ODD(sts);
	sts = ldh_GetNextSibling( ctx->ldhses, module_oid, &module_oid)) {
    sts = ldh_ObjidToName( ctx->ldhses,  module_oid, cdh_mName_object, module_name,
			   sizeof(module_name), &size);
    if ( EVEN(sts)) return sts;

    if ( !(sscanf(  module_name, "M%d", &idx) == 1)) {
      corrupt = 1;
      continue;
    }
    if ( idx >= ctx->attr->attrnav->dev_data.slot_data.size()) {
      corrupt = 1;
      continue;
    }
    ctx->attr->attrnav->dev_data.slot_data[idx]->module_oid = module_oid;
  }
  if ( corrupt) {
    ctx->attr->wow->DisplayError( "Configuration corrupt", 
		      "Configuration of module objects doesn't match device configuration");
  }
  return 1;
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
  return wsx_CheckIoDevice( Session, Object, ErrorCount, WarningCount, wsx_mCardOption_None);
}

//
//  Every method to be exported to the workbench should be registred here.
//

pwr_dExport pwr_BindMethods(PnDevice) = {
  pwr_BindMethod(SyntaxCheck),
  pwr_NullMethod
};


