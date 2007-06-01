/* 
 * Proview   $Id: opc_provider.cpp,v 1.16 2007-06-01 12:52:40 claes Exp $
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

#include <vector.h>
#include <string.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <iconv.h>
#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "wb_vext.h"
#include "rt_procom.h"
#include "opc_provider.h"
#include "wb_ldh.h"
#include "wb_ldh_msg.h"
#include "rt_gdh_msg.h"
#include "co_cdh.h"
#include "co_cnv.h"
#include "co_dcli.h"
#include "pwr_opcclasses.h"
#include "opc_utl.h"
#include "opc_soap_H.h"
#include "Service.nsmap"

#define START_OIX 1000
#define procom_obj_mFlags_Analog (1 << 31)

typedef map<pwr_tUInt32, opcprv_sub>::iterator sublist_iterator;

static pwr_tVid opc_vid;
static char opc_vname[32];
static char opc_endpoint[256];

// Wb only
void opc_provider::object( co_procom *pcom)
{
  if ( m_list.size() <= 1 || m_list[0]->po.fchoix == 0) {
    pcom->provideObject( LDH__NOSUCHOBJ,0,0,0,0,0,0,0,"","");
    return;
  }
  objectOid( pcom, m_list[0]->po.fchoix);
}

void opc_provider::fault()
{
  int code;
  const char *c, **d;
  d = soap_faultcode( &soap);
  if (!*d)
    soap_set_fault( &soap);
  c = *d;

  if ( !opc_string_to_resultcode( (char *)c, &code))
    code = pwr_eOpc_ResultCodeEnum_Unknown;

  server_state->LastError = code;
  server_state->ErrorRequestCnt = server_state->RequestCnt;

  if ( 1)
    soap_print_fault( &soap, stderr);
}

void opc_provider::insert_object( pwr_tOix fth, pwr_tOix bws, s0__BrowseElement *element,
				  int first, int last, int load_children, std::string *path)
{
  opcprv_obj *o = new opcprv_obj();
  xsd__anyType *valp;
  int opctype;
	
  strcpy( o->po.name, name_to_objectname( (char *) element->Name->c_str()));
  if ( element->ItemPath)
    path = element->ItemPath;
  if ( path) {
    strcpy( o->item_name, path->c_str());
    strcat( o->item_name, element->ItemName->c_str());
  }
  else
    strcpy( o->item_name, element->ItemName->c_str());

  o->po.oix = next_oix++;
  o->po.fthoix = fth;
  if ( !element->IsItem) {
    o->po.cid = pwr_cClass_Opc_Hier;
    o->po.body_size = sizeof(pwr_sClass_Opc_Hier);
    o->po.body = calloc( 1, o->po.body_size);
    if ( opc_get_property( element->Properties, opc_mProperty_Description, &valp))
      strncpy( ((pwr_sClass_Opc_Hier *)o->po.body)->Description, 
	       ((xsd__string *)valp)->__item.c_str(), 
	       sizeof(((pwr_sClass_Opc_Hier *)o->po.body)->Description));
  }
  else {
    if ( opc_get_property( element->Properties, opc_mProperty_DataType, &valp)) {
      if ( !opc_string_to_opctype( ((xsd__string *)valp)->__item.c_str(), &opctype))
	opctype = opc_eDataType_;
      
      switch ( opctype) {
      case opc_eDataType_string:
	o->po.cid = pwr_cClass_Opc_String;
	o->po.body_size = sizeof(pwr_sClass_Opc_String);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_String;
	o->size = sizeof( pwr_tString80);
	break;
      case opc_eDataType_boolean:
	o->po.cid = pwr_cClass_Opc_Boolean;
	o->po.body_size = sizeof(pwr_sClass_Opc_Boolean);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Boolean;
	o->size = sizeof( pwr_tBoolean);
	break;
      case opc_eDataType_float:
	o->po.cid = pwr_cClass_Opc_Float;
	o->po.body_size = sizeof(pwr_sClass_Opc_Float);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Float32;
	o->size = sizeof( pwr_tFloat32);
	((pwr_sClass_Opc_Float *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_double:
	o->po.cid = pwr_cClass_Opc_Double;
	o->po.body_size = sizeof(pwr_sClass_Opc_Double);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Float64;
	o->size = sizeof( pwr_tFloat64);
	((pwr_sClass_Opc_Double *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_decimal:
	o->po.cid = pwr_cClass_Opc_Decimal;
	o->po.body_size = sizeof(pwr_sClass_Opc_Decimal);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Float32;
	o->size = sizeof( pwr_tFloat32);
	((pwr_sClass_Opc_Decimal *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_long:
	o->po.cid = pwr_cClass_Opc_Long;
	o->po.body_size = sizeof(pwr_sClass_Opc_Long);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Int64;
	o->size = sizeof( pwr_tInt64);
	((pwr_sClass_Opc_Long *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_int:
	o->po.cid = pwr_cClass_Opc_Int;
	o->po.body_size = sizeof(pwr_sClass_Opc_Int);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Int32;
	o->size = sizeof( pwr_tInt32);
	((pwr_sClass_Opc_Int *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_short:
	o->po.cid = pwr_cClass_Opc_Short;
	o->po.body_size = sizeof(pwr_sClass_Opc_Short);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Int16;
	o->size = sizeof( pwr_tInt16);
	((pwr_sClass_Opc_Short *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_byte:
	o->po.cid = pwr_cClass_Opc_Byte;
	o->po.body_size = sizeof(pwr_sClass_Opc_Byte);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Int8;
	o->size = sizeof( pwr_tInt8);
	((pwr_sClass_Opc_Byte *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_unsignedLong:
	o->po.cid = pwr_cClass_Opc_UnsignedLong;
	o->po.body_size = sizeof(pwr_sClass_Opc_UnsignedLong);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_UInt64;
	o->size = sizeof( pwr_tUInt64);
	((pwr_sClass_Opc_UnsignedLong *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_unsignedInt:
	o->po.cid = pwr_cClass_Opc_UnsignedInt;
	o->po.body_size = sizeof(pwr_sClass_Opc_UnsignedInt);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_UInt32;
	o->size = sizeof( pwr_tUInt32);
	((pwr_sClass_Opc_UnsignedInt *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_unsignedShort:
	o->po.cid = pwr_cClass_Opc_UnsignedShort;
	o->po.body_size = sizeof(pwr_sClass_Opc_UnsignedShort);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_UInt16;
	o->size = sizeof( pwr_tUInt16);
	((pwr_sClass_Opc_UnsignedShort *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_unsignedByte:
	o->po.cid = pwr_cClass_Opc_UnsignedByte;
	o->po.body_size = sizeof(pwr_sClass_Opc_UnsignedByte);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_UInt8;
	o->size = sizeof( pwr_tUInt8);
	((pwr_sClass_Opc_UnsignedByte *)o->po.body)->HighEU = 100;
	break;
      case opc_eDataType_base64Binary:
	o->po.cid = pwr_cClass_Opc_Base64Binary;
	o->po.body_size = sizeof(pwr_sClass_Opc_Base64Binary);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_UInt64;
	o->size = sizeof( pwr_tUInt64);
	break;
      case opc_eDataType_dateTime:
	o->po.cid = pwr_cClass_Opc_DateTime;
	o->po.body_size = sizeof(pwr_sClass_Opc_DateTime);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Time;
	o->size = sizeof( pwr_tTime);
	break;
      case opc_eDataType_time:
	o->po.cid = pwr_cClass_Opc_Time;
	o->po.body_size = sizeof(pwr_sClass_Opc_Time);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Time;
	o->size = sizeof( pwr_tTime);
	break;
      case opc_eDataType_date:
	o->po.cid = pwr_cClass_Opc_Date;
	o->po.body_size = sizeof(pwr_sClass_Opc_Date);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_Time;
	o->size = sizeof( pwr_tTime);
	break;
      case opc_eDataType_duration:
	o->po.cid = pwr_cClass_Opc_Duration;
	o->po.body_size = sizeof(pwr_sClass_Opc_Duration);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_DeltaTime;
	o->size = sizeof( pwr_tDeltaTime);
	break;
      case opc_eDataType_QName:
	o->po.cid = pwr_cClass_Opc_QName;
	o->po.body_size = sizeof(pwr_sClass_Opc_QName);
	o->po.body = calloc( 1, o->po.body_size);
	o->type = pwr_eType_String;
	o->size = sizeof( pwr_tString80);
      default:
	o->po.cid = pwr_cClass_Opc_Hier;
	o->po.body_size = sizeof(pwr_sClass_Opc_Hier);
	o->po.body = calloc( 1, o->po.body_size);
      }

      if ( opc_get_property( element->Properties, opc_mProperty_Description, &valp)) {
	pwr_tString80 desc;
	strncpy( desc, ((xsd__string *)valp)->__item.c_str(), sizeof(desc));

	switch ( o->po.cid) {
	case pwr_cClass_Opc_Float:
	  strncpy( ((pwr_sClass_Opc_Float *)o->po.body)->Description, desc, 
		   sizeof(((pwr_sClass_Opc_Float *)o->po.body)->Description));
	  break;
	case pwr_cClass_Opc_Double:
	  strncpy( ((pwr_sClass_Opc_Float *)o->po.body)->Description, desc, 
		   sizeof(((pwr_sClass_Opc_Double *)o->po.body)->Description));
	  break;
	case pwr_cClass_Opc_Int:
	  strncpy( ((pwr_sClass_Opc_Int *)o->po.body)->Description, desc, 
		   sizeof(((pwr_sClass_Opc_Int *)o->po.body)->Description));
	  break;
	case pwr_cClass_Opc_Short:
	  strncpy( ((pwr_sClass_Opc_Short *)o->po.body)->Description, desc, 
		   sizeof(((pwr_sClass_Opc_Short *)o->po.body)->Description));
	  break;
	case pwr_cClass_Opc_Byte:
	  strncpy( ((pwr_sClass_Opc_Byte *)o->po.body)->Description, desc, 
		   sizeof(((pwr_sClass_Opc_Byte *)o->po.body)->Description));
	  break;
	case pwr_cClass_Opc_UnsignedInt:
	  strncpy( ((pwr_sClass_Opc_UnsignedInt *)o->po.body)->Description, desc, 
		   sizeof(((pwr_sClass_Opc_UnsignedInt *)o->po.body)->Description));
	  break;
	case pwr_cClass_Opc_UnsignedShort:
	  strncpy( ((pwr_sClass_Opc_UnsignedShort *)o->po.body)->Description, desc, 
		   sizeof(((pwr_sClass_Opc_UnsignedShort *)o->po.body)->Description));
	  break;
	case pwr_cClass_Opc_UnsignedByte:
	  strncpy( ((pwr_sClass_Opc_UnsignedByte *)o->po.body)->Description, desc, 
		   sizeof(((pwr_sClass_Opc_UnsignedByte *)o->po.body)->Description));
	  break;
	}
      }
    }
    else {
      o->po.cid = pwr_cClass_Opc_Hier;
      o->po.body_size = sizeof(pwr_sClass_Opc_Hier);
      o->po.body = calloc( 1, o->po.body_size);
      if ( opc_get_property( element->Properties, opc_mProperty_Description, &valp)) {
	strncpy( ((pwr_sClass_Opc_Hier *)o->po.body)->Description, 
		 ((xsd__string *)valp)->__item.c_str(),
		 sizeof(((pwr_sClass_Opc_Hier *)o->po.body)->Description));
      }
    }
  }
  if ( first)
    m_list[fth]->po.fchoix = o->po.oix;
  else {
    o->po.bwsoix = bws;
    m_list[bws]->po.fwsoix = o->po.oix;
  }
  if ( last) {
    m_list[fth]->po.lchoix = o->po.oix;
    if ( !first) {
      o->po.fwsoix = m_list[fth]->po.fchoix;
      m_list[o->po.fwsoix]->po.bwsoix = o->po.oix;
    }
    else {
      // Single child
      o->po.fwsoix = o->po.oix;
      o->po.bwsoix = o->po.oix;
    }
  }
  if ( element->HasChildren && load_children)
    o->po.flags |= procom_obj_mFlags_Loaded;
  else if ( !element->HasChildren)
    o->po.flags |= procom_obj_mFlags_Loaded;

  m_list.push_back( o);

  if ( opc_get_property( element->Properties, opc_mProperty_EuType, &valp)) {
    if ( ((xsd__string *)valp)->__item == "analog")
      m_list[o->po.oix]->po.flags |= procom_obj_mFlags_Analog;
  }
  if ( m_list[o->po.oix]->po.flags & procom_obj_mFlags_Analog) {
    // Get analog properties
    _s0__GetProperties get_properties;
    _s0__GetPropertiesResponse properties_response;
    s0__ItemIdentifier id;      
    pwr_tFloat32 high_eu = 0;
    pwr_tFloat32 low_eu = 0;
    pwr_tString16 engineering_units = "";

    get_properties.ReturnPropertyValues = (bool *) malloc( sizeof(bool));
    *get_properties.ReturnPropertyValues = true;
    id.ItemName = new std::string( o->item_name);
    
    get_properties.ItemIDs.push_back( &id);

    opc_mask_to_propertynames( get_properties.PropertyNames, 
			       opc_mProperty_HighEU | opc_mProperty_LowEU |
			       opc_mProperty_EngineeringUnits);
    
    if ( soap_call___s0__GetProperties( &soap, opc_endpoint, NULL, &get_properties, 
					 &properties_response) == SOAP_OK) {
      server_state->RequestCnt++;
      if ( properties_response.PropertyLists.size() > 0 &&
	   properties_response.PropertyLists[0]->Properties.size() > 0) {
	  	
	if ( opc_get_property( properties_response.PropertyLists[0]->Properties, 
			       opc_mProperty_HighEU, &valp)) {
	  high_eu = ((xsd__double *)valp)->__item;
	}
	if ( opc_get_property( properties_response.PropertyLists[0]->Properties, 
			       opc_mProperty_LowEU, &valp)) {
	  low_eu = ((xsd__double *)valp)->__item;
	}
	if ( opc_get_property( properties_response.PropertyLists[0]->Properties, 
			       opc_mProperty_EngineeringUnits, &valp)) {
	  strncpy( engineering_units, ((xsd__string *)valp)->__item.c_str(), 
				       sizeof(engineering_units));
	}
	
	void *body = m_list[o->po.oix]->po.body;
	switch ( o->po.cid) {
	case pwr_cClass_Opc_Float:
	  ((pwr_sClass_Opc_Float *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_Float *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_Float *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_Double:
	  ((pwr_sClass_Opc_Double *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_Double *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_Double *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_Decimal:
	  ((pwr_sClass_Opc_Decimal *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_Decimal *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_Decimal *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_Long:
	  ((pwr_sClass_Opc_Long *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_Long *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_Long *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_Int:
	  ((pwr_sClass_Opc_Int *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_Int *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_Int *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_Short:
	  ((pwr_sClass_Opc_Short *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_Short *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_Short *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_Byte:
	  ((pwr_sClass_Opc_Byte *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_Byte *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_Byte *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_UnsignedLong:
	  ((pwr_sClass_Opc_UnsignedLong *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_UnsignedLong *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_UnsignedLong *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_UnsignedInt:
	  ((pwr_sClass_Opc_UnsignedInt *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_UnsignedInt *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_UnsignedInt *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_UnsignedShort:
	  ((pwr_sClass_Opc_UnsignedShort *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_UnsignedShort *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_UnsignedShort *)body)->EngineeringUnits, engineering_units);
	  break;
	case pwr_cClass_Opc_UnsignedByte:
	  ((pwr_sClass_Opc_UnsignedByte *)body)->HighEU = high_eu;
	  ((pwr_sClass_Opc_UnsignedByte *)body)->LowEU = low_eu;
	  strcpy( ((pwr_sClass_Opc_UnsignedByte *)body)->EngineeringUnits, engineering_units);
	  break;
	default: ;
	}
      }
    }
    else {
      // Error returned from soap
      server_state->RequestCnt++;
      fault();
    }

    free( (char *)get_properties.ReturnPropertyValues);
    delete id.ItemName;
  }


  if ( load_children) {
    _s0__Browse browse;
    _s0__BrowseResponse browse_response;

    browse.ItemName = new std::string( cnv_iso8859_to_utf8( o->item_name, 
				       strlen(o->item_name)+1));
    opc_mask_to_propertynames( browse.PropertyNames, 
			       opc_mProperty_DataType | opc_mProperty_Description |
			       opc_mProperty_EuType);

    if ( soap_call___s0__Browse( &soap, opc_endpoint, NULL, &browse, &browse_response) ==
	 SOAP_OK) {
      pwr_tOix next_bws;
      pwr_tOix bws = 0;

      server_state->RequestCnt++;
      for ( int i = 0; i < (int)browse_response.Elements.size(); i++) {
	next_bws = next_oix;
	insert_object( o->po.oix, bws, browse_response.Elements[i],
		       i == 0, i == (int)browse_response.Elements.size() - 1, 0, 0);
	bws = next_bws;
      }
    }
    else {
      // Error returned from soap
      server_state->RequestCnt++;
      fault();
    }
    delete browse.ItemName;
  }
}

void opc_provider::init()
{
  // Insert volume object
  opcprv_obj *vo = new opcprv_obj();

  vo->po.cid = pwr_eClass_ExternVolume;
  strcpy( vo->po.name, opc_vname);
  vo->po.body_size = sizeof(pwr_sExternVolume);
  vo->po.body = calloc( 1, vo->po.body_size);
  vo->po.oix = 0;
  vo->po.flags |= procom_obj_mFlags_Loaded;
  m_list.push_back( vo);
  
  // Insert ServerState object
  opcprv_obj *so = new opcprv_obj();
  so->po.cid = pwr_cClass_Opc_ServerState;
  strcpy( so->po.name, "OpcServerState");
  so->po.body_size = sizeof(pwr_sClass_Opc_ServerState);
  so->po.body = calloc( 1, so->po.body_size);
  so->po.oix = next_oix++;
  so->po.fthoix = 0;
  so->po.bwsoix = so->po.oix;
  so->po.fwsoix = so->po.oix;
  m_list[0]->po.fchoix = so->po.oix;
  m_list[0]->po.lchoix = so->po.oix;
  so->po.flags |= procom_obj_mFlags_Loaded;
  m_list.push_back( so);
  server_state = (pwr_sClass_Opc_ServerState *)m_list[1]->po.body;
}

void opc_provider::objectOid( co_procom *pcom, pwr_tOix oix)
{
  if ( m_list.size() == 2) {
    // Load Rootlist
    _s0__Browse browse;
    _s0__BrowseResponse browse_response;

    opc_mask_to_propertynames( browse.PropertyNames, 
			       opc_mProperty_DataType | opc_mProperty_Description |
			       opc_mProperty_EuType);
    if ( soap_call___s0__Browse( &soap, opc_endpoint, NULL, &browse, &browse_response) ==
	 SOAP_OK) {
      server_state->RequestCnt++;
      pwr_tOix bws = m_list[1]->po.oix;
      pwr_tOix next_bws;
      for ( int i = 0; i < (int)browse_response.Elements.size(); i++) {
	next_bws = next_oix;
	insert_object( oix, bws, browse_response.Elements[i],
		       0, i == (int)browse_response.Elements.size() - 1, 1, 0);
	bws = next_bws;
      }
    }
    else {
      // Error returned from soap
      server_state->RequestCnt++;
      fault();
    }
  }
  else if ( oix < m_list.size()) {
    if ( !(m_list[oix]->po.flags & procom_obj_mFlags_Loaded)) {
      _s0__Browse browse;
      _s0__BrowseResponse browse_response;

      browse.ItemName = new std::string( cnv_iso8859_to_utf8( m_list[oix]->item_name, 
					 strlen(m_list[oix]->item_name)+1));
      opc_mask_to_propertynames( browse.PropertyNames, 
				 opc_mProperty_DataType | opc_mProperty_Description |
				 opc_mProperty_EuType);
      
      if ( soap_call___s0__Browse( &soap, opc_endpoint, NULL, &browse, &browse_response) ==
	   SOAP_OK) {
	pwr_tOix next_bws;
	pwr_tOix bws = 0;

	server_state->RequestCnt++;
	if ( browse_response.Errors.size() > 0) {
	  errlog( browse.ItemName, browse_response.Errors);
	}
	for ( int i = 0; i < (int)browse_response.Elements.size(); i++) {
	  next_bws = next_oix;
	  insert_object( oix, bws, browse_response.Elements[i],
			 i == 0, i == (int)browse_response.Elements.size() - 1, 0, 0);
	  bws = next_bws;
	}
	m_list[oix]->po.flags |= procom_obj_mFlags_Loaded;
      }
      else {
	// Error returned from soap
	server_state->RequestCnt++;
	fault();
      }
      delete browse.ItemName;
    }
  }

  if ( oix >= m_list.size() || oix < 0) {
    pcom->provideStatus( GDH__NOSUCHOBJ);
    return;
  }

#if 0
  for ( int i = 0; i < (int)m_list.size(); i++) {
    printf( "oix %2d bws %2d fws %2d fth %2d fch %2d lch %2d flags %lu %s\n", 
	    m_list[i]->po.oix, m_list[i]->po.bwsoix, m_list[i]->po.fwsoix, m_list[i]->po.fthoix, 
	    m_list[i]->po.fchoix, m_list[i]->po.lchoix, m_list[i]->po.flags, m_list[i]->po.name);
  }
#endif
  vector<procom_obj>olist;

  if ( oix == 0)
    olist.push_back( m_list[0]->po);
  else {
    int plist_cnt = 0;
    pwr_tOix p, c;
    pwr_tOix plist[100];
    
    // Get parents
    for ( p = m_list[oix]->po.fthoix; 
	  ; 
	  p = m_list[p]->po.fthoix) {
      plist[plist_cnt++] = p;
      if ( m_list[p]->po.fthoix == 0)
	break;
    }

    // Add parents
    for ( int i = plist_cnt - 1; i >= 0; i--)
      olist.push_back( m_list[plist[i]]->po);

    // Add siblings
    for ( c = m_list[plist[0]]->po.fchoix; ; c = m_list[c]->po.fwsoix) {
      if  ( m_list[c]->po.flags & procom_obj_mFlags_Loaded)
	olist.push_back( m_list[c]->po);

      if ( m_list[c]->po.fwsoix == m_list[plist[0]]->po.fchoix)
	break;
    }
  }
#if 0
  for ( int i = 0; i < (int) m_list.size(); i++) {
    if  ( m_list[i]->po.flags & procom_obj_mFlags_Loaded)
      olist.push_back( m_list[i]->po);
  }
#endif
  printf( "*********************************************\n");
  for ( int i = 0; i < (int)olist.size(); i++) {
    printf( "oix %2d bws %2d fws %2d fth %2d fch %2d lch %2d flags %lu %s\n", 
	    olist[i].oix, olist[i].bwsoix, olist[i].fwsoix, olist[i].fthoix, 
	    olist[i].fchoix, olist[i].lchoix, olist[i].flags, olist[i].name);
  }
  pcom->provideObjects( GDH__SUCCESS, olist);
}

void opc_provider::objectName( co_procom *pcom, char *name, pwr_tOix poix)
{
  pwr_tStatus sts = GDH__SUCCESS;
  cdh_sParseName pn;
  pwr_tOix oix, coix;

  if ( poix) {
    if ( poix >= m_list.size()) {
      pcom->provideStatus( GDH__NOSUCHOBJ);
      return;
    }
  }

  cdh_ParseName( &sts, &pn, pwr_cNOid, name, 0);
  if ( poix)
    oix = poix;
  else
    oix = 0;

  for ( int i = 0; i < (int)pn.nObject; i++) {
    bool found = false;

    if ( !(m_list[oix]->po.flags & procom_obj_mFlags_Loaded)) {
      _s0__Browse browse;
      _s0__BrowseResponse browse_response;

      browse.ItemName = new std::string( cnv_iso8859_to_utf8( m_list[oix]->item_name, 
					 strlen(m_list[oix]->item_name)+1));
      opc_mask_to_propertynames( browse.PropertyNames, 
				 opc_mProperty_DataType | opc_mProperty_Description |
				 opc_mProperty_EuType);
      
      if ( soap_call___s0__Browse( &soap, opc_endpoint, NULL, &browse, &browse_response) ==
	   SOAP_OK) {
	pwr_tOix next_bws;
	pwr_tOix bws = 0;

	server_state->RequestCnt++;
	if ( browse_response.Errors.size() > 0) {
	  errlog( browse.ItemName, browse_response.Errors);
	}
	for ( int i = 0; i < (int)browse_response.Elements.size(); i++) {
	  next_bws = next_oix;
	  insert_object( oix, bws, browse_response.Elements[i],
			 i == 0, i == (int)browse_response.Elements.size() - 1, 0, 0);
	  bws = next_bws;
	}
	m_list[oix]->po.flags |= procom_obj_mFlags_Loaded;
      }
      else {
	// Error returned from soap
	server_state->RequestCnt++;
	fault();
	sts = GDH__NOSUCHOBJ;
	break;
      }
      delete browse.ItemName;
    }

    for ( coix = m_list[oix]->po.fchoix; ;
	  coix = m_list[coix]->po.fwsoix) {
      if ( cdh_NoCaseStrcmp( m_list[coix]->po.name, pn.object[i].name.norm) == 0) {
	oix = coix;
	found = true;
	break;
      }
      if ( m_list[coix]->po.fwsoix == m_list[oix]->po.fchoix)
	// Last child
	break;
    }
    if ( !found)
      sts = GDH__NOSUCHOBJ;
  }

  if ( ODD(sts)) {
    objectOid( pcom, oix);
  }
  else {    
    if ( m_env == pvd_eEnv_Wb)
      pcom->provideObject( 0,0,0,0,0,0,0,0,"","");
    else
      pcom->provideStatus( sts);
  }    
}

// Wb only
void opc_provider::objectBody( co_procom *pcom, pwr_tOix oix)
{
}

// Wb only
void opc_provider::createObject( co_procom *pcom, pwr_tOix destoix, int desttype,
		     pwr_tCid cid, char *name)
{
}

// Wb only
void opc_provider::moveObject( co_procom *pcom, pwr_tOix oix, pwr_tOix destoix, 
				 int desttype)
{
}

// Wb only
void opc_provider::deleteObject( co_procom *pcom, pwr_tOix oix)
{
}

// Wb only
void opc_provider::copyObject( co_procom *pcom, pwr_tOix oix, pwr_tOix destoix, int desttype,
				 char *name)
{
}

// Wb only
void opc_provider::deleteFamily( co_procom *pcom, pwr_tOix oix)
{
}

// Wb only
void opc_provider::renameObject( co_procom *pcom, pwr_tOix oix, char *name)
{
}

void opc_provider::writeAttribute( co_procom *pcom, pwr_tOix oix, unsigned int offset,
		       unsigned int size, char *buffer)
{
  if ( oix >= m_list.size() || oix <= 0) {
    pcom->provideStatus( LDH__NOSUCHOBJ);
    return;
  }

  if ( offset + size > m_list[oix]->po.body_size) {
    pcom->provideStatus( LDH__NOSUCHATTR);
    return;
  }

  memcpy( (void *)((unsigned long)m_list[oix]->po.body + (unsigned long)offset), buffer, size);

  switch ( m_list[oix]->po.cid) {
  case pwr_cClass_Opc_String:
  case pwr_cClass_Opc_Boolean:
  case pwr_cClass_Opc_Float:
  case pwr_cClass_Opc_Double:
  case pwr_cClass_Opc_Decimal:
  case pwr_cClass_Opc_Long:
  case pwr_cClass_Opc_Int:
  case pwr_cClass_Opc_Short:
  case pwr_cClass_Opc_Byte:
  case pwr_cClass_Opc_UnsignedLong:
  case pwr_cClass_Opc_UnsignedInt:
  case pwr_cClass_Opc_UnsignedShort:
  case pwr_cClass_Opc_UnsignedByte:
  case pwr_cClass_Opc_Base64Binary:
  case pwr_cClass_Opc_Time:
  case pwr_cClass_Opc_Date:
  case pwr_cClass_Opc_DateTime:
  case pwr_cClass_Opc_Duration:
  case pwr_cClass_Opc_QName: {

    // Value has the same offset for all opctype classes
    if ( offset == (unsigned int) ((char *) &((pwr_sClass_Opc_String *)m_list[oix]->po.body)->Value -
		    (char *)m_list[oix]->po.body)) {      
      _s0__Write write;
      _s0__WriteResponse write_response;
      char opc_buffer[2000];
      int opc_type;

      s0__ItemValue *item = new s0__ItemValue();
      item->ItemName = new std::string( cnv_iso8859_to_utf8( m_list[oix]->item_name,
							     strlen(m_list[oix]->item_name)+1));
      opc_pwrtype_to_opctype( m_list[oix]->type, &opc_type);
      opc_convert_pwrtype_to_opctype( buffer, opc_buffer, sizeof(opc_buffer), opc_type, 
				      m_list[oix]->type);
      item->Value = opc_opctype_to_value( &soap, opc_buffer, sizeof(opc_buffer), opc_type);
      write.ItemList = new s0__WriteRequestItemList;
      write.ItemList->Items.push_back( item);

      if ( soap_call___s0__Write( &soap, opc_endpoint, NULL, &write, &write_response) ==
	   SOAP_OK) {
	// Check item errors
	server_state->RequestCnt++;
      }
      else {
	// Error returned from soap
	server_state->RequestCnt++;
	fault();
      }
      delete write.ItemList;
      delete item->Value;
      delete item;
    }
    break;
  }
  }


  pcom->provideStatus( 1);
}

// Rt only
void opc_provider::readAttribute( co_procom *pcom, pwr_tOix oix, unsigned int offset,
		       unsigned int size)
{
  if ( oix >= m_list.size() || oix <= 0) {
    pcom->provideStatus( GDH__NOSUCHOBJ);
    return;
  }

  if ( offset + size > m_list[oix]->po.body_size) {
    pcom->provideStatus( GDH__NOSUCHOBJ);
    return;
  }

  switch ( m_list[oix]->po.cid) {
  case pwr_cClass_Opc_String:
  case pwr_cClass_Opc_Boolean:
  case pwr_cClass_Opc_Float:
  case pwr_cClass_Opc_Double:
  case pwr_cClass_Opc_Decimal:
  case pwr_cClass_Opc_Long:
  case pwr_cClass_Opc_Short:
  case pwr_cClass_Opc_Byte:
  case pwr_cClass_Opc_UnsignedLong:
  case pwr_cClass_Opc_UnsignedInt:
  case pwr_cClass_Opc_UnsignedShort:
  case pwr_cClass_Opc_UnsignedByte:
  case pwr_cClass_Opc_Base64Binary:
  case pwr_cClass_Opc_Time:
  case pwr_cClass_Opc_Date:
  case pwr_cClass_Opc_Duration:
  case pwr_cClass_Opc_QName:
    // Value has the same offset for all opctype classes
    if ( offset == (unsigned int) ((char *) &((pwr_sClass_Opc_String *)m_list[oix]->po.body)->Value -
		    (char *)m_list[oix]->po.body)) {      
      _s0__Read read;
      _s0__ReadResponse read_response;

      s0__ReadRequestItem *item = new s0__ReadRequestItem();
      item->ItemName = new std::string( cnv_iso8859_to_utf8( m_list[oix]->item_name,
							     strlen(m_list[oix]->item_name)+1));
      read.ItemList = new s0__ReadRequestItemList;
      read.ItemList->Items.push_back( item);

      if ( soap_call___s0__Read( &soap, opc_endpoint, NULL, &read, &read_response) ==
	   SOAP_OK) {
	server_state->RequestCnt++;
	if ( read_response.RItemList && read_response.RItemList->Items.size() > 0) {
	  opc_convert_opctype_to_pwrtype( &((pwr_sClass_Opc_String *)m_list[oix]->po.body)->Value,
					  m_list[oix]->size, read_response.RItemList->Items[0]->Value,
					  (pwr_eType) m_list[oix]->type);
	}
      }
      else {
	// Error returned from soap
	server_state->RequestCnt++;
	fault();
      }
      delete read.ItemList;
      delete item->ItemName;
      delete item;
    }
    break;
  }

  void *p = (void *)((unsigned long)m_list[oix]->po.body + (unsigned long)offset);
  pcom->provideAttr( GDH__SUCCESS, oix, size, p);
}

// Rt only
void opc_provider::subAssociateBuffer( co_procom *pcom, void **buff, int oix, int offset, 
				      int size, pwr_tSubid sid) 
{
  if ( oix >= (int)m_list.size()) {
    *buff = 0;
    return;
  }

  *buff = (char *)m_list[oix]->po.body + offset;

  switch ( m_list[oix]->po.cid) {
  case pwr_cClass_Opc_String:
  case pwr_cClass_Opc_Boolean:
  case pwr_cClass_Opc_Float:
  case pwr_cClass_Opc_Double:
  case pwr_cClass_Opc_Decimal:
  case pwr_cClass_Opc_Long:
  case pwr_cClass_Opc_Int:
  case pwr_cClass_Opc_Short:
  case pwr_cClass_Opc_Byte:
  case pwr_cClass_Opc_UnsignedLong:
  case pwr_cClass_Opc_UnsignedInt:
  case pwr_cClass_Opc_UnsignedShort:
  case pwr_cClass_Opc_UnsignedByte:
  case pwr_cClass_Opc_Base64Binary:
  case pwr_cClass_Opc_DateTime:
  case pwr_cClass_Opc_Time:
  case pwr_cClass_Opc_Date:
  case pwr_cClass_Opc_Duration:
  case pwr_cClass_Opc_QName:
    if ( *buff == ((pwr_sClass_Opc_String *)m_list[oix]->po.body)->Value) {
      // Add opc subscription
      _s0__Subscribe subscribe;
      _s0__SubscribeResponse subscribe_response;
      char handle[20];

      subscribe.Options = new s0__RequestOptions();
      subscribe.Options->ReturnItemTime = (bool *) malloc( sizeof(bool));
      *subscribe.Options->ReturnItemTime = true;

      subscribe.ItemList = new s0__SubscribeRequestItemList();
      s0__SubscribeRequestItem *ritem = new s0__SubscribeRequestItem();
      ritem->ItemName = new std::string( cnv_iso8859_to_utf8( m_list[oix]->item_name,
							      strlen(m_list[oix]->item_name)+1));
      sprintf( handle, "%d", oix);
      ritem->ClientItemHandle = new std::string( handle);
      ritem->RequestedSamplingRate = (int *) malloc( sizeof(int));
      *ritem->RequestedSamplingRate = 1000;

      subscribe.ItemList->Items.push_back( ritem);

      if ( soap_call___s0__Subscribe( &soap, opc_endpoint, NULL, &subscribe, &subscribe_response) ==
	 SOAP_OK) {
	opcprv_sub sub;

	server_state->RequestCnt++;
	sub.handle = *subscribe_response.ServerSubHandle;
	sub.oix = oix;
	m_sublist[sid.rix] = sub;

	if ( subscribe_response.RItemList && subscribe_response.RItemList->Items.size()) {
	  for ( int i = 0; i < (int)subscribe_response.RItemList->Items.size(); i++) {
	    // subscribe_response.RItemList->Items[i]->ItemValue...
	  }
	}
      }
      else {
	// Error returned from soap
	server_state->RequestCnt++;
	fault();
      }
      free( (char *) subscribe.Options->ReturnItemTime);
      delete subscribe.ItemList;
      delete ritem->ItemName;
      free( (char *)ritem->RequestedSamplingRate);
      delete ritem;
    }
    break;
  default: ;
  }
}

// Rt only
void opc_provider::subDisassociateBuffer( co_procom *pcom, pwr_tSubid sid) 
{
  sublist_iterator it = m_sublist.find( sid.rix);
  if ( it != m_sublist.end()) {
    // Cancel subscription
    _s0__SubscriptionCancel subcancel;
    _s0__SubscriptionCancelResponse subcancel_response;

    subcancel.ServerSubHandle = new std::string(it->second.handle);

    if ( soap_call___s0__SubscriptionCancel( &soap, opc_endpoint, NULL, &subcancel, &subcancel_response) ==
	 SOAP_OK) {
      server_state->RequestCnt++;
    }
    else {
      // Error returned from soap
      server_state->RequestCnt++;
      fault();
    }

    m_sublist.erase( it);
    delete subcancel.ServerSubHandle;
  }
}

// Rt only
void opc_provider::cyclic( co_procom *pcom)
{
  int size = 0;
  bool reconnect = false;
  _s0__SubscriptionPolledRefresh subpoll;
  _s0__SubscriptionPolledRefreshResponse subpoll_response;
  
  
  for ( sublist_iterator it = m_sublist.begin(); it != m_sublist.end(); it++) {
    subpoll.ServerSubHandles.push_back( it->second.handle);
    size++;
  }

  if ( size) {
    if ( soap_call___s0__SubscriptionPolledRefresh( &soap, opc_endpoint, NULL, &subpoll, &subpoll_response) ==
	 SOAP_OK) {
      server_state->RequestCnt++;
      if ( (int) subpoll_response.RItemList.size() != size) {
	return;
      }

      int idx = 0;
      for ( sublist_iterator it = m_sublist.begin(); it != m_sublist.end(); it++) {
	if ( subpoll_response.RItemList[idx]->Items.size()) {
	  if ( subpoll_response.RItemList[idx]->Items[0]->ResultID) {
	    int code;
	    if ( opc_string_to_resultcode( (char *)subpoll_response.RItemList[idx]->Items[0]->ResultID->c_str(),
					   &code)) {
	      if ( code == opc_eResultCode_E_NOSUBSCRIPTION) {
		// Subscription removed, add new subscription
		reconnect = true;
		break;
	      }
	      else {
		server_state->LastError = code;
		server_state->ErrorRequestCnt = server_state->RequestCnt;
	      }
	    }
	  }
	  else {
	    pwr_tOix oix = it->second.oix;
	    opc_convert_opctype_to_pwrtype( (void *) ((pwr_sClass_Opc_String *)m_list[oix]->po.body)->Value,
					    m_list[oix]->size,
					    subpoll_response.RItemList[idx]->Items[0]->Value,
					    (pwr_eType) m_list[oix]->type);
	  }
	}
	idx++;
      }
      
      if ( reconnect) {
	int idx = 0;
	map<pwr_tUInt32, opcprv_sub> sublist_add;
	map<pwr_tUInt32, opcprv_sub> sublist_erase;

	for ( sublist_iterator it = m_sublist.begin(); it != m_sublist.end(); it++) {
	  if ( subpoll_response.RItemList[idx]->Items.size()) {
	    if ( subpoll_response.RItemList[idx]->Items[0]->ResultID) {
	      int code;
	      if ( opc_string_to_resultcode( (char *)subpoll_response.RItemList[idx]->Items[0]->ResultID->c_str(),
					     &code)) {
		if ( code == opc_eResultCode_E_NOSUBSCRIPTION) {
		  // Subscription removed, add new subscription
		  server_state->LastError = code;
		  server_state->ErrorRequestCnt = server_state->RequestCnt;
		  
		  _s0__Subscribe subscribe;
		  _s0__SubscribeResponse subscribe_response;
		  pwr_tOix oix = it->second.oix;
		  pwr_tUInt32 rix = it->first;
		  
		  subscribe.Options = new s0__RequestOptions();
		  subscribe.Options->ReturnItemTime = (bool *) malloc( sizeof(bool));
		  *subscribe.Options->ReturnItemTime = true;
		  
		  subscribe.ItemList = new s0__SubscribeRequestItemList();
		  s0__SubscribeRequestItem *ritem = new s0__SubscribeRequestItem();
		  ritem->ItemName = new std::string( cnv_iso8859_to_utf8( m_list[oix]->item_name,
									  strlen(m_list[oix]->item_name)+1));
		  ritem->ClientItemHandle = new std::string( it->second.handle);
		  ritem->RequestedSamplingRate = (int *) malloc( sizeof(int));
		  *ritem->RequestedSamplingRate = 1000;
		  subscribe.ItemList->Items.push_back( ritem);
		  
		  // Remove old subscription
		  //m_sublist.erase( it);
		  sublist_erase[it->first] = it->second;

		  printf( "Reconnect: %s\n", m_list[oix]->item_name); 
		  
		  if ( soap_call___s0__Subscribe( &soap, opc_endpoint, NULL, &subscribe, &subscribe_response) ==
		       SOAP_OK) {
		    opcprv_sub sub;
		    
		    // Insert new subscription with new handle
		    sub.handle = *subscribe_response.ServerSubHandle;
		    sub.oix = oix;
		    //m_sublist[rix] = sub;
		    sublist_add[rix] = sub;

		    server_state->RequestCnt++;
		    
		    if ( subscribe_response.RItemList && subscribe_response.RItemList->Items.size()) {
		      for ( int i = 0; i < (int)subscribe_response.RItemList->Items.size(); i++) {
			// subscribe_response.RItemList->Items[i]->ItemValue...
		      }
		    }
		  }
		  else {
		    // Error returned from soap
		    server_state->RequestCnt++;
		    fault();
		  }
		  free( (char *)subscribe.Options->ReturnItemTime);
		  delete subscribe.Options;
		  delete subscribe.ItemList;
		  delete ritem->ItemName;
		  delete ritem->ClientItemHandle;
		  free( (char *)ritem->RequestedSamplingRate);
		  delete ritem;
		}
	      }
	    }
	  }
	}
	idx++;

	// Remove old subscriptions to m_sublist
	for ( sublist_iterator it = sublist_erase.begin(); it != sublist_erase.end(); it++) {
	  m_sublist.erase(it->first);
	}
	// Add new subscriptions to m_sublist
	for ( sublist_iterator it = sublist_add.begin(); it != sublist_add.end(); it++) {
	  m_sublist[it->first] = it->second;
	}
      }
    }
    else {
      // Error returned from soap
      server_state->RequestCnt++;
      fault();
    }
  }
}


// Wb only
void opc_provider::commit( co_procom *pcom)
{
  pwr_tStatus sts;

  save( &sts);
  pcom->provideStatus( sts);
}

// Wb only
void opc_provider::abort( co_procom *pcom)
{
  pwr_tStatus sts;

  m_list.clear();
  next_oix = 1;
  load( &sts);
  pcom->provideStatus( sts);
}

void opc_provider::delete_tree( pwr_tOix oix)
{
  m_list[oix]->po.flags = procom_obj_mFlags_Deleted;

  for ( pwr_tOix ix = m_list[oix]->po.fchoix;
	ix;
	ix = m_list[ix]->po.fwsoix)
    delete_tree( ix);
}

char *opc_provider::longname( pwr_tOix oix)
{
  if ( m_list[oix]->po.fthoix == 0)
    strcpy( m_list[oix]->po.lname, m_list[oix]->po.name);
  else {
    strcpy( m_list[oix]->po.lname, longname( m_list[oix]->po.fthoix));
    strcat( m_list[oix]->po.lname, "-");
    strcat( m_list[oix]->po.lname, m_list[oix]->po.name);
  }
  return m_list[oix]->po.lname;
}

bool opc_provider::find( pwr_tOix fthoix, char *name, pwr_tOix *oix)
{

  for ( int i = 0; i < (int) m_list.size(); i++) {
    if  ( !m_list[i]->po.flags & procom_obj_mFlags_Deleted) {
      if ( m_list[i]->po.fthoix == fthoix && 
	   cdh_NoCaseStrcmp( name, m_list[i]->po.name) == 0) {
	*oix = m_list[i]->po.oix;
	return true;
      }
    }
  }
  return false;
}

void opc_provider::get_server_state()
{
  _s0__GetStatus get_status;
  _s0__GetStatusResponse get_status_response;
  get_status.ClientRequestHandle = new std::string("Opc Client");

  if ( soap_call___s0__GetStatus( &soap, opc_endpoint, NULL, &get_status, &get_status_response) ==
       SOAP_OK) {
    server_state->RequestCnt++;
    if ( get_status_response.Status->VendorInfo)
      strcpy( server_state->VendorInfo, get_status_response.Status->VendorInfo->c_str());
    if ( get_status_response.Status->ProductVersion)
      strcpy( server_state->ProductVersion, get_status_response.Status->ProductVersion->c_str());
    opc_time_OPCAsciiToA( (char *)get_status_response.Status->StartTime.c_str(), 
			  &server_state->StartTime);
    server_state->ServerState = get_status_response.GetStatusResult->ServerState;
  }
  else {
    server_state->RequestCnt++;
    server_state->ServerState = s0__serverState__commFault;
    fault();
  }
  delete get_status.ClientRequestHandle;
}

//
// Create a valid object name from an item name
//
char *opc_provider::name_to_objectname( char *name)
{
  static char n[32];
  char *s, *t;

  for ( s = name, t = n; *s; s++) {
    if ( t - n >= (int)sizeof(n) - 1)
      break;

    if ( *s == '[' || *s == ']' || *s == '-' || *s == '/' || *s == '.')
      *t = '$';
    else
      *t = *s;
    t++;
  }
  *t = 0;
  return n;
}

void opc_provider::errlog( std::string* item, std::vector<s0__OPCError *>& errvect)
{
  for ( int i = 0; i < (int) errvect.size(); i++)
    printf( "OPC Error: %s  %s\n", item->c_str(), errvect[i]->ID.c_str());
}


void usage()
{
  cout << "remote_pvd_pwrcli   Proview provider client" << endl << endl <<
    "Arguments: " << endl <<
    "  1   Opc server URL" << endl <<
    "  2   Extern volume id" << endl <<
    "  3   Extern volume name" << endl <<
    "  4   Server identity (optional, default 200)" << endl;
}

int main(int argc, char *argv[])
{
  pwr_tStatus sts;
  char server_url[256];
  char extern_vid[40];
  char extern_volume_name[40];
  int server_id;
  
  /* Read arguments */
  if ( argc < 4) {
    usage();
    exit(0);
  }
  strcpy( server_url, argv[1]);
  strcpy( extern_vid, argv[2]);
  strcpy( extern_volume_name, argv[3]);

  if ( argc >= 5) {
    sts = sscanf( argv[4], "%d", &server_id);
    if ( sts != 1) {
      usage();
      exit(0);
    } 
  }
  else
    server_id = 200;

  strcpy( opc_endpoint, server_url);
  cdh_StringToVolumeId( extern_vid, &opc_vid);
  strcpy( opc_vname, extern_volume_name);

  opc_provider provider( pvd_eEnv_Rt);
  rt_procom procom( &provider,
		    errh_eNAnix, 	// Application index
		    "opc_provider", // Process name
		    server_id,	       	// Sid
		    opc_vid, 		// Vid
		    opc_vname,  	// Volume name
		    0);			// Global
  
  procom.init();

  // provider.nodeUp();
  soap_init( &provider.soap);

  provider.init();
  strcpy( provider.server_state->Server, server_url);
  provider.get_server_state();

  procom.mainLoop();
}

