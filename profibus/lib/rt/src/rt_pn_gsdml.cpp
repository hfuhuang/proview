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
 */

/* rt_pn_gsdml.cpp -- Parse gsdml file */

#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "co_dcli.h"
#include "rt_pn_gsdml.h"
#include "rt_pb_msg.h"
#include "pwr_baseclasses.h"

static char noref[] = "Unresolved reference";

typedef enum {
  gsdml_eState_Init 		= 1 << 0,
  gsdml_eState_Tag		= 1 << 1,
  gsdml_eState_MetaTag		= 1 << 2,
  gsdml_eState_TagNameFound    	= 1 << 3,
  gsdml_eState_TagName		= 1 << 4,
  gsdml_eState_Attribute	= 1 << 5,
  gsdml_eState_AttributeName	= 1 << 6,
  gsdml_eState_AttributeNameFound = 1 << 7,
  gsdml_eState_AttributeValue	= 1 << 8,
  gsdml_eState_AttributeValueFound = 1 << 9,
  gsdml_eState_TagValue		= 1 << 10,
  gsdml_eState_TagValueFound   	= 1 << 11,
  gsdml_eState_EndTag 		= 1 << 12
} gsdml_eState;

typedef struct {
  char name[80];
  gsdml_eTag id;
  gsdml_eType type;
  unsigned int size;
  unsigned int offset;
  int has_data;
} gsdml_sTag;

typedef struct {
  char name[80];
  gsdml_eTag id;
  gsdml_eTag pid;
  gsdml_eType type;
  unsigned int size;
  unsigned int offset;
  int ignore;
  char default_value[40];
} gsdml_sAttribute;

static gsdml_sTag taglist[] = {
  { "xml", 		gsdml_eTag_xml, 		gsdml_eType_, 0, 0, 1},
  { "ISO15745Profile", 	gsdml_eTag_ISO15745Profile, 	gsdml_eType_, 0, 0, 0},
  { "ProfileHeader", 	gsdml_eTag_ProfileHeader, 	gsdml_eType_, 0, 0, 1},
  { "ProfileIdentification", gsdml_eTag_ProfileIdentification, gsdml_eType_String, sizeof(gsdml_tString80), 	offsetof(gsdml_sProfileHeader,ProfileIdentification), 0},
  { "ProfileRevision", 	gsdml_eTag_ProfileRevision, 	gsdml_eType_String, 	sizeof(gsdml_tString80), 	offsetof(gsdml_sProfileHeader,ProfileRevision), 0},
  { "ProfileName", 	gsdml_eTag_ProfileName, 	gsdml_eType_String, 	sizeof(gsdml_tString80), 	offsetof(gsdml_sProfileHeader,ProfileName), 0},
  { "ProfileSource", 	gsdml_eTag_ProfileSource, 	gsdml_eType_String, 	sizeof(gsdml_tString80), 	offsetof(gsdml_sProfileHeader,ProfileSource), 0},
  { "ProfileClassID", 	gsdml_eTag_ProfileClassID, 	gsdml_eType_String, 	sizeof(gsdml_tString80), 	offsetof(gsdml_sProfileHeader,ProfileClassID), 0},
  { "ISO15745Reference", gsdml_eTag_ISO15745Reference, 	gsdml_eType_, 0, 0, 0},
  { "ISO15745Part", 	gsdml_eTag_ISO15745Part, 	gsdml_eType_Integer, 	sizeof(gsdml_tInteger), 	offsetof(gsdml_sProfileHeader,ISO15745Part), 0},
  { "ISO15745Edition", 	gsdml_eTag_ISO15745Edition, 	gsdml_eType_Integer, 	sizeof(gsdml_tInteger), 	offsetof(gsdml_sProfileHeader,ISO15745Edition), 0},
  { "ProfileTechnology", gsdml_eTag_ProfileTechnology, 	gsdml_eType_String, 	sizeof(gsdml_tString80), 	offsetof(gsdml_sProfileHeader,ProfileTechnology), 0},
  { "ProfileBody", 	gsdml_eTag_ProfileBody, 	gsdml_eType_, 0, 0, 0},
  { "DeviceIdentity", 	gsdml_eTag_DeviceIdentity, 	gsdml_eType_, 0, 0, 1},
  { "InfoText", 	gsdml_eTag_InfoText, 		gsdml_eType_, 0, 0, 0},
  { "VendorName", 	gsdml_eTag_VendorName, 		gsdml_eType_, 0, 0, 0},
  { "DeviceFunction", 	gsdml_eTag_DeviceFunction, 	gsdml_eType_, 0, 0, 1},
  { "Family", 		gsdml_eTag_Family, 		gsdml_eType_, 0, 0, 0},
  { "ApplicationProcess", gsdml_eTag_ApplicationProcess, gsdml_eType_, 0, 0, 1},
  { "DeviceAccessPointList", gsdml_eTag_DeviceAccessPointList, gsdml_eType_, 0, 0, 1},
  { "DeviceAccessPointItem", gsdml_eTag_DeviceAccessPointItem, gsdml_eType_, 0, 0, 1},
  { "ModuleInfo", gsdml_eTag_ModuleInfo, gsdml_eType_, 0, 0, 1},
  { "Name", gsdml_eTag_Name, gsdml_eType_, 0, 0, 0},
  { "OrderNumber", gsdml_eTag_OrderNumber, gsdml_eType_, 0, 0, 0},
  { "HardwareRelease", gsdml_eTag_HardwareRelease, gsdml_eType_, 0, 0, 0},
  { "SoftwareRelease", gsdml_eTag_SoftwareRelease, gsdml_eType_, 0, 0, 0},
  { "SubslotList", gsdml_eTag_SubslotList, gsdml_eType_, 0, 0, 1},
  { "SubslotItem", gsdml_eTag_SubslotItem, gsdml_eType_, 0, 0, 1},
  { "IOConfigData", gsdml_eTag_IOConfigData, gsdml_eType_, 0, 0, 1},
  { "UseableModules", gsdml_eTag_UseableModules, gsdml_eType_, 0, 0, 1},
  { "ModuleItemRef", gsdml_eTag_ModuleItemRef, gsdml_eType_, 0, 0, 1},
  { "VirtualSubmoduleList", gsdml_eTag_VirtualSubmoduleList, gsdml_eType_, 0, 0, 1},
  { "VirtualSubmoduleItem", gsdml_eTag_VirtualSubmoduleItem, gsdml_eType_, 0, 0, 1},
  { "IOData", gsdml_eTag_IOData, gsdml_eType_, 0, 0, 1},
  { "Input", gsdml_eTag_Input, gsdml_eType_, 0, 0, 1},
  { "Output", gsdml_eTag_Output, gsdml_eType_, 0, 0, 1},
  { "DataItem", gsdml_eTag_DataItem, gsdml_eType_, 0, 0, 1},
  { "BitDataItem", gsdml_eTag_BitDataItem, gsdml_eType_, 0, 0, 1},
  { "RecordDataList", gsdml_eTag_RecordDataList, gsdml_eType_, 0, 0, 1},
  { "ParameterRecordDataItem", gsdml_eTag_ParameterRecordDataItem, gsdml_eType_, 0, 0, 1},
  { "Const", gsdml_eTag_Const, gsdml_eType_, 0, 0, 1},
  { "Ref", gsdml_eTag_Ref, gsdml_eType_, 0, 0, 1},
  { "F_ParameterRecordDataItem", gsdml_eTag_F_ParameterRecordDataItem, gsdml_eType_, 0, 0, 1},
  { "F_Check_iPar", gsdml_eTag_F_Check_iPar, gsdml_eType_, 0, 0, 0},
  { "F_SIL", gsdml_eTag_F_SIL, gsdml_eType_, 0, 0, 0},
  { "F_CRC_Length", gsdml_eTag_F_CRC_Length, gsdml_eType_, 0, 0, 0},
  { "F_Block_ID", gsdml_eTag_F_Block_ID, gsdml_eType_, 0, 0, 0},
  { "F_Par_Version", gsdml_eTag_F_Par_Version, gsdml_eType_, 0, 0, 0},
  { "F_Source_Add", gsdml_eTag_F_Source_Add, gsdml_eType_, 0, 0, 0},
  { "F_Dest_Add", gsdml_eTag_F_Dest_Add, gsdml_eType_, 0, 0, 0},
  { "F_WD_Time", gsdml_eTag_F_WD_Time, gsdml_eType_, 0, 0, 0},
  { "F_Par_CRC", gsdml_eTag_F_Par_CRC, gsdml_eType_, 0, 0, 0},
  { "F_iPar_CRC", gsdml_eTag_F_iPar_CRC, gsdml_eType_, 0, 0, 0},
  { "Graphics", gsdml_eTag_Graphics, gsdml_eType_, 0, 0, 1},
  { "GraphicItemRef", gsdml_eTag_GraphicItemRef, gsdml_eType_, 0, 0, 1},
  { "IsochroneMode", gsdml_eTag_IsochroneMode, gsdml_eType_, 0, 0, 1},
  { "SystemDefinedSubmoduleList", gsdml_eTag_SystemDefinedSubmoduleList, gsdml_eType_, 0, 0, 1},
  { "InterfaceSubmoduleItem", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_, 0, 0, 1},
  { "General", gsdml_eTag_General, gsdml_eType_, 0, 0, 1},
  { "DCP_FlashOnceSignalUnit", gsdml_eTag_DCP_FlashOnceSignalUnit, gsdml_eType_, 0, 0, 1},
  { "RT_Class3Properties", gsdml_eTag_RT_Class3Properties, gsdml_eType_, 0, 0, 1},
  { "SynchronisationMode", gsdml_eTag_SynchronisationMode, gsdml_eType_, 0, 0, 1},
  { "ApplicationRelations", gsdml_eTag_ApplicationRelations, gsdml_eType_, 0, 0, 1},
  { "TimingProperties", gsdml_eTag_TimingProperties, gsdml_eType_, 0, 0, 1},
  { "RT_Class3TimingProperties", gsdml_eTag_RT_Class3TimingProperties, gsdml_eType_, 0, 0, 1},
  { "MediaRedundancy", gsdml_eTag_MediaRedundancy, gsdml_eType_, 0, 0, 1},
  { "PortSubmoduleItem", gsdml_eTag_PortSubmoduleItem, gsdml_eType_, 0, 0, 1},
  { "UseableSubmodules", gsdml_eTag_UseableSubmodules, gsdml_eType_, 0, 0, 1},
  { "SubmoduleItemRef", gsdml_eTag_SubmoduleItemRef, gsdml_eType_, 0, 0, 1},
  { "SlotList", gsdml_eTag_SlotList, gsdml_eType_, 0, 0, 1},
  { "SlotItem", gsdml_eTag_SlotItem, gsdml_eType_, 0, 0, 1},
  { "SlotGroups", gsdml_eTag_SlotGroups, gsdml_eType_, 0, 0, 1},
  { "SlotGroup", gsdml_eTag_SlotGroup, gsdml_eType_, 0, 0, 1},
  { "ModuleList", gsdml_eTag_ModuleList, gsdml_eType_, 0, 0, 1},
  { "ModuleItem", gsdml_eTag_ModuleItem, gsdml_eType_, 0, 0, 1},
  { "SubmoduleList", gsdml_eTag_SubmoduleList, gsdml_eType_, 0, 0, 1},
  { "ValueList", gsdml_eTag_ValueList, gsdml_eType_, 0, 0, 1},
  { "ValueItem", gsdml_eTag_ValueItem, gsdml_eType_, 0, 0, 1},
  { "Help", gsdml_eTag_Help, gsdml_eType_, 0, 0, 0},
  { "Assignments", gsdml_eTag_Assignments, gsdml_eType_, 0, 0, 1},
  { "Assign", gsdml_eTag_Assign, gsdml_eType_, 0, 0, 1},
  { "ChannelDiagList", gsdml_eTag_ChannelDiagList, gsdml_eType_, 0, 0, 1},
  { "ChannelDiagItem", gsdml_eTag_ChannelDiagItem, gsdml_eType_, 0, 0, 1},
  { "ExtChannelDiagList", gsdml_eTag_ExtChannelDiagList, gsdml_eType_, 0, 0, 1},
  { "ExtChannelDiagItem", gsdml_eTag_ExtChannelDiagItem, gsdml_eType_, 0, 0, 1},
  { "ExtChannelAddValue", gsdml_eTag_ExtChannelAddValue, gsdml_eType_, 0, 0, 1},
  { "ProfileChannelDiagItem", gsdml_eTag_ChannelDiagItem, gsdml_eType_, 0, 0, 1},
  { "ProfileExtChannelDiagList", gsdml_eTag_ExtChannelDiagList, gsdml_eType_, 0, 0, 1},
  { "ProfileExtChannelDiagItem", gsdml_eTag_ExtChannelDiagItem, gsdml_eType_, 0, 0, 1},
  { "UnitDiagTypeList", gsdml_eTag_UnitDiagTypeList, gsdml_eType_, 0, 0, 1},
  { "UnitDiagTypeItem", gsdml_eTag_UnitDiagTypeItem, gsdml_eType_, 0, 0, 1},
  { "ProfileUnitDiagTypeItem", gsdml_eTag_UnitDiagTypeItem, gsdml_eType_, 0, 0, 1},
  { "GraphicsList", gsdml_eTag_GraphicsList, gsdml_eType_, 0, 0, 1},
  { "GraphicItem", gsdml_eTag_GraphicItem, gsdml_eType_, 0, 0, 1},
  { "Embedded", gsdml_eTag_Embedded, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sGraphicItem,Embedded), 0},
  { "CategoryList", gsdml_eTag_CategoryList, gsdml_eType_, 0, 0, 1},
  { "CategoryItem", gsdml_eTag_CategoryItem, gsdml_eType_, 0, 0, 1},
  { "ExternalTextList", gsdml_eTag_ExternalTextList, gsdml_eType_, 0, 0, 1},
  { "PrimaryLanguage", gsdml_eTag_PrimaryLanguage, gsdml_eType_, 0, 0, 1},
  { "Language", gsdml_eTag_Language, gsdml_eType_, 0, 0, 1},
  { "Text", gsdml_eTag_Text, gsdml_eType_, 0, 0, 1},
  { "", gsdml_eTag_, gsdml_eType_, 0}};

static gsdml_sAttribute attrlist[] = {
  { "version", gsdml_eTag_xml, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sXml,Version), 0, ""},
  { "encoding", gsdml_eTag_xml, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sXml,Encoding), 0, ""},
  { "xmlns", gsdml_eTag_, gsdml_eTag_, gsdml_eType_String, 0, 0, 1, ""},
  { "xmlns:base", gsdml_eTag_ISO15745Profile, gsdml_eTag_, gsdml_eType_String, 0, 0, 1, ""},
  { "xmlns:ds", gsdml_eTag_ISO15745Profile, gsdml_eTag_, gsdml_eType_String, 0, 0, 1, ""},
  { "xmlns:xsi", gsdml_eTag_ISO15745Profile, gsdml_eTag_, gsdml_eType_String, 0, 0, 1, ""},
  { "xsi:schemaLocation", gsdml_eTag_ISO15745Profile, gsdml_eTag_, gsdml_eType_String, 0, 0, 1, ""},
  //
  // DeviceIdentity
  //
  { "VendorID", gsdml_eTag_DeviceIdentity, gsdml_eTag_, gsdml_eType_Unsigned16hex, sizeof(gsdml_tUnsigned16hex), offsetof(gsdml_sDeviceIdentity,VendorID), 0, ""},
  { "DeviceID", gsdml_eTag_DeviceIdentity, gsdml_eTag_, gsdml_eType_Unsigned16hex, sizeof(gsdml_tUnsigned16hex), offsetof(gsdml_sDeviceIdentity,DeviceID), 0, ""},
  { "TextId", gsdml_eTag_InfoText, gsdml_eTag_DeviceIdentity, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sDeviceIdentity,InfoText), 0, ""},
  { "Value", gsdml_eTag_VendorName, gsdml_eTag_DeviceIdentity, gsdml_eType_Token, sizeof(gsdml_tToken), offsetof(gsdml_sDeviceIdentity,VendorName), 0, ""},
  //
  // DeviceFunction
  //
  { "MainFamily", gsdml_eTag_Family, gsdml_eTag_DeviceFunction, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sDeviceFunction,MainFamily), 0, ""},
  { "ProductFamily", gsdml_eTag_Family, gsdml_eTag_DeviceFunction, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sDeviceFunction,ProductFamily), 0, ""},
  //
  // DeviceAccessPointItem
  //
  { "PhysicalSlots", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sDeviceAccessPointItem,PhysicalSlots), 0, ""},
  { "ID", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Id, sizeof(gsdml_tId), offsetof(gsdml_sDeviceAccessPointItem,ID), 0, ""},
  { "ModuleIdentNumber", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Unsigned32hex, sizeof(gsdml_tUnsigned32hex), offsetof(gsdml_sDeviceAccessPointItem,ModuleIdentNumber), 0, ""},
  { "MinDeviceInterval", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sDeviceAccessPointItem,MinDeviceInterval), 0, ""},
  { "ImplementationType", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_NormalizedString, sizeof(gsdml_tNormalizedString), offsetof(gsdml_sDeviceAccessPointItem,ImplementationType), 0, ""},
  { "DNS_CompatibleName", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sDeviceAccessPointItem,DNS_CompatibleName), 0, ""},
  { "ExtendedAddressAssignmentSupported", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sDeviceAccessPointItem,ExtendedAddressAssignmentSupported), 0, "false"},
  { "AddressAssignment", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sDeviceAccessPointItem,AddressAssignment), 0, "DCP"},
  { "AllowedInSlots", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sDeviceAccessPointItem,AllowedInSlots), 0, ""},
  { "FixedInSlots", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sDeviceAccessPointItem,FixedInSlots), 0, ""},
  { "ObjectUUID_LocalIndex", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sDeviceAccessPointItem,ObjectUUID_LocalIndex), 0, ""},
  { "RequiredSchemaVersion", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sDeviceAccessPointItem,RequiredSchemaVersion), 0, "V1.0"},
  { "MultipleWriteSupported", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sDeviceAccessPointItem,MultipleWriteSupported), 0, "false"},
  { "IOXS_Required", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sDeviceAccessPointItem,IOXS_Required), 0, "true"},
  { "PhysicalSubslots", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sDeviceAccessPointItem,PhysicalSubslots), 0, ""},
  { "RemoteApplicationTimeout", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sDeviceAccessPointItem,RemoteApplicationTimeout), 0, "300"},
  { "MaxSupportedRecordSize", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sDeviceAccessPointItem,MaxSupportedRecordSize), 0, "4068"},
  { "PowerOnToCommReady", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sDeviceAccessPointItem,PowerOnToCommReady), 0, "0"},
  { "ParameterizationSpeedupSupported", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sDeviceAccessPointItem,ParameterizationSpeedSupported), 0, "false"},
  { "NameOfStationNotTransferable", gsdml_eTag_DeviceAccessPointItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sDeviceAccessPointItem,NameOfStationNotTransferable), 0, "false"},
  //
  // ModuleInfo
  //
  { "CategoryRef", gsdml_eTag_ModuleInfo, gsdml_eTag_, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sModuleInfo,CategoryRef), 0, ""},
  { "SubCategory1Ref", gsdml_eTag_ModuleInfo, gsdml_eTag_, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sModuleInfo,SubCategory1Ref), 0, ""},
  { "TextId", gsdml_eTag_Name, gsdml_eTag_ModuleInfo, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sModuleInfo,Name), 0, ""},
  { "TextId", gsdml_eTag_InfoText, gsdml_eTag_ModuleInfo, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sModuleInfo,InfoText), 0, ""},
  { "Value", gsdml_eTag_VendorName, gsdml_eTag_ModuleInfo, gsdml_eType_Token, sizeof(gsdml_tToken), offsetof(gsdml_sModuleInfo,VendorName), 0, ""},
  { "Value", gsdml_eTag_OrderNumber, gsdml_eTag_ModuleInfo, gsdml_eType_Token, sizeof(gsdml_tToken), offsetof(gsdml_sModuleInfo,OrderNumber), 0, ""},
  { "Value", gsdml_eTag_HardwareRelease, gsdml_eTag_ModuleInfo, gsdml_eType_Token, sizeof(gsdml_tToken), offsetof(gsdml_sModuleInfo,HardwareRelease), 0, ""},
  { "Value", gsdml_eTag_SoftwareRelease, gsdml_eTag_ModuleInfo, gsdml_eType_Token, sizeof(gsdml_tToken), offsetof(gsdml_sModuleInfo,SoftwareRelease), 0, ""},
  { "MainFamily", gsdml_eTag_Family, gsdml_eTag_ModuleInfo, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sModuleInfo,MainFamily), 0, ""},
  { "ProductFamily", gsdml_eTag_Family, gsdml_eTag_ModuleInfo, gsdml_eType_String, sizeof(gsdml_tString80), offsetof(gsdml_sModuleInfo,ProductFamily), 0, ""},
  // 
  // SubslotItem
  //
  { "SubslotNumber", gsdml_eTag_SubslotItem, gsdml_eTag_SubslotList, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sSubslotItem,SubslotNumber), 0, ""},
  { "TextId", gsdml_eTag_SubslotItem, gsdml_eTag_SubslotList, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sSubslotItem,TextId), 0, ""},
  //
  // IOConfigData
  //
  { "MaxInputLength", gsdml_eTag_IOConfigData, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sIOConfigData,MaxInputLength), 0, ""},
  { "MaxOutputLength", gsdml_eTag_IOConfigData, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sIOConfigData,MaxOutputLength), 0, ""},
  { "MaxDataLength", gsdml_eTag_IOConfigData, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sIOConfigData,MaxDataLength), 0, ""},
  //
  // ModuleItemRef
  //
  { "ModuleItemTarget", gsdml_eTag_ModuleItemRef, gsdml_eTag_, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sModuleItemRef,ModuleItemTarget), 0, ""},
  { "AllowedInSlots", gsdml_eTag_ModuleItemRef, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sModuleItemRef,AllowedInSlots), 0, ""},
  { "UsedInSlots", gsdml_eTag_ModuleItemRef, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sModuleItemRef,UsedInSlots), 0, ""},
  { "FixedInSlots", gsdml_eTag_ModuleItemRef, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sModuleItemRef,FixedInSlots), 0, ""},
  //
  // VirtualSubmoduleItem
  //
  { "ID", gsdml_eTag_VirtualSubmoduleItem, gsdml_eTag_, gsdml_eType_Id, sizeof(gsdml_tId), offsetof(gsdml_sVirtualSubmoduleItem,ID), 0, ""},
  { "SubmoduleIdentNumber", gsdml_eTag_VirtualSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned32hex, sizeof(gsdml_tUnsigned32hex), offsetof(gsdml_sVirtualSubmoduleItem,SubmoduleIdentNumber), 0, ""},
  { "API", gsdml_eTag_VirtualSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sVirtualSubmoduleItem,API), 0, "0"},
  { "FixedInSubslots", gsdml_eTag_VirtualSubmoduleItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sVirtualSubmoduleItem,FixedInSubslots), 0, "1"},
  { "PROFIsafeSupported", gsdml_eTag_VirtualSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sVirtualSubmoduleItem,PROFIsafeSupported), 0, "false"},
  { "Writeable_IM_Records", gsdml_eTag_VirtualSubmoduleItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sVirtualSubmoduleItem,Writeable_IM_Records), 0, "0"},
  { "Max_iParameterSize", gsdml_eTag_VirtualSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sVirtualSubmoduleItem,Max_iParameterSize), 0, "0"},
  { "SubsysModuleDirIndex", gsdml_eTag_VirtualSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sVirtualSubmoduleItem,SubsysModuleDirIndex), 0, ""},
  //
  // IOData
  //
  { "IOPS_Length", gsdml_eTag_IOData, gsdml_eTag_, gsdml_eType_Unsigned8, sizeof(gsdml_tUnsigned8), offsetof(gsdml_sIOData,IOPS_Length), 0, ""},
  { "IOCS_Length", gsdml_eTag_IOData, gsdml_eTag_, gsdml_eType_Unsigned8, sizeof(gsdml_tUnsigned8), offsetof(gsdml_sIOData,IOCS_Length), 0, ""},
  { "F_IO_StructureDescVersion", gsdml_eTag_IOData, gsdml_eTag_, gsdml_eType_Unsigned8, sizeof(gsdml_tUnsigned8), offsetof(gsdml_sIOData,F_IO_StructureDescVersion), 0, ""},
  { "F_IO_StructureDescCRC", gsdml_eTag_IOData, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sIOData,F_IO_StructureDescCRC), 0, ""},
  //
  // Input
  //
  { "Consistency", gsdml_eTag_Input, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sInput,Consistency), 0, "Item consistency"},
  //
  // Output
  //
  { "Consistency", gsdml_eTag_Output, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sOutput,Consistency), 0, "Item consistency"},
  //
  // ExtChannelAddValue-DataItem
  //
  { "Id", gsdml_eTag_DataItem, gsdml_eTag_ExtChannelAddValue, gsdml_eType_Unsigned8, sizeof(gsdml_tUnsigned8), offsetof(gsdml_sExtChannelAddValue_DataItem,Id), 0, ""},
  { "DataType", gsdml_eTag_DataItem, gsdml_eTag_ExtChannelAddValue, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sExtChannelAddValue_DataItem,DataType), 0, ""},
  { "Length", gsdml_eTag_DataItem, gsdml_eTag_ExtChannelAddValue, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sExtChannelAddValue_DataItem,Length), 0, ""},
  //
  // DataItem
  //
  { "DataType", gsdml_eTag_DataItem, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sDataItem,DataType), 0, ""},
  { "Length", gsdml_eTag_DataItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sDataItem,Length), 0, ""},
  { "UseAsBits", gsdml_eTag_DataItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sDataItem,UseAsBits), 0, "false"},
  { "TextId", gsdml_eTag_DataItem, gsdml_eTag_, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sDataItem,TextId), 0, ""},
  //
  // BitDataItem
  //
  { "BitOffset", gsdml_eTag_BitDataItem, gsdml_eTag_, gsdml_eType_Unsigned8, sizeof(gsdml_tUnsigned8), offsetof(gsdml_sBitDataItem,BitOffset), 0, ""},
  { "TextId", gsdml_eTag_BitDataItem, gsdml_eTag_, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sBitDataItem,TextId), 0, ""},
  //
  // ParameterRecordDataItem
  //
  { "Index", gsdml_eTag_ParameterRecordDataItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sParameterRecordDataItem,Index), 0, ""},
  { "Length", gsdml_eTag_ParameterRecordDataItem, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sParameterRecordDataItem,Length), 0, ""},
  { "TransferSequence", gsdml_eTag_ParameterRecordDataItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sParameterRecordDataItem,TransferSequence), 0, "0"},
  { "TextId", gsdml_eTag_Name, gsdml_eTag_ParameterRecordDataItem, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sParameterRecordDataItem,Name), 0, ""},
  //
  // Const
  //
  { "ByteOffset", gsdml_eTag_Const, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sConst,ByteOffset), 0, ""},
  { "Data", gsdml_eTag_Const, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString1024), offsetof(gsdml_sConst,Data), 0, ""},
  //
  // Ref
  //
  { "ValueItemTarget", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sRef,ValueItemTarget), 0, ""},
  { "ByteOffset", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sRef,ByteOffset), 0, ""},
  { "BitOffset", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_Integer, sizeof(gsdml_tInteger), offsetof(gsdml_sRef,BitOffset), 0, "0"},
  { "BitLength", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_Integer, sizeof(gsdml_tInteger), offsetof(gsdml_sRef,BitLength), 0, "1"},
  { "DataType", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sRef,DataType), 0, ""},
  { "DefaultValue", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString), offsetof(gsdml_sRef,DefaultValue), 0, ""},
  { "AllowedValues", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_SignedOrFloatValueList, sizeof(gsdml_tSignedOrFloatValueList), offsetof(gsdml_sRef,AllowedValues), 0, ""},
  { "Changeable", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sRef,Changeable), 0, "true"},
  { "Visible", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sRef,Visible), 0, "true"},
  { "TextId", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sRef,TextId), 0, ""},
  { "Length", gsdml_eTag_Ref, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sRef,Length), 0, ""},
  //
  // F_ParameterRecordDataItem
  //
  { "F_ParamDescCRC", gsdml_eTag_F_ParameterRecordDataItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sF_ParameterRecordDataItem,F_ParamDescCRC), 0, ""},
  { "Index", gsdml_eTag_F_ParameterRecordDataItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sF_ParameterRecordDataItem,Index), 0, ""},
  { "TransferSequence", gsdml_eTag_F_ParameterRecordDataItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sF_ParameterRecordDataItem,TransferSequence), 0, ""},
  //
  // F_Check_iPar
  //
  { "DefaultValue", gsdml_eTag_F_Check_iPar, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sF_ParameterRecordDataItem,F_Check_iPar_DefaultValue), 0, "NoCheck"},
  { "AllowedValues", gsdml_eTag_F_Check_iPar, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString), offsetof(gsdml_sF_ParameterRecordDataItem,F_Check_iPar_AllowedValues), 0, "Check NoCheck"},
  { "Visible", gsdml_eTag_F_Check_iPar, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Check_iPar_Visible), 0, "false"},
  { "Changeable", gsdml_eTag_F_Check_iPar, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Check_iPar_Changeable), 0, "false"},
  //
  // F_SIL
  //
  { "DefaultValue", gsdml_eTag_F_SIL, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sF_ParameterRecordDataItem,F_SIL_DefaultValue), 0, "SIL3"},
  { "AllowedValues", gsdml_eTag_F_SIL, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString), offsetof(gsdml_sF_ParameterRecordDataItem,F_SIL_AllowedValues), 0, "SIL1 SIL2 SIL3 NoSIL"},
  { "Visible", gsdml_eTag_F_SIL, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_SIL_Visible), 0, "true"},
  { "Changeable", gsdml_eTag_F_SIL, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_SIL_Changeable), 0, "true"},
  //
  // F_CRC_Length
  //
  { "DefaultValue", gsdml_eTag_F_CRC_Length, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sF_ParameterRecordDataItem,F_CRC_Length_DefaultValue), 0, "3-Byte-CRC"},
  { "AllowedValues", gsdml_eTag_F_CRC_Length, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString), offsetof(gsdml_sF_ParameterRecordDataItem,F_CRC_Length_AllowedValues), 0, "3-Byte-CRC"},
  { "Visible", gsdml_eTag_F_CRC_Length, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_CRC_Length_Visible), 0, "false"},
  { "Changeable", gsdml_eTag_F_CRC_Length, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_CRC_Length_Changeable), 0, "false"},
  //
  // F_Block_ID
  //
  { "DefaultValue", gsdml_eTag_F_Block_ID, gsdml_eTag_, gsdml_eType_Integer, sizeof(gsdml_tInteger), offsetof(gsdml_sF_ParameterRecordDataItem,F_Block_ID_DefaultValue), 0, "0"},
  { "AllowedValues", gsdml_eTag_F_Block_ID, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sF_ParameterRecordDataItem,F_Block_ID_AllowedValues), 0, "0..7"},
  { "Visible", gsdml_eTag_F_Block_ID, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Block_ID_Visible), 0, "true"},
  { "Changeable", gsdml_eTag_F_Block_ID, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Block_ID_Changeable), 0, "false"},
  //
  // F_Par_Version
  //
  { "DefaultValue", gsdml_eTag_F_Par_Version, gsdml_eTag_, gsdml_eType_Integer, sizeof(gsdml_tInteger), offsetof(gsdml_sF_ParameterRecordDataItem,F_Par_Version_DefaultValue), 0, "1"},
  { "AllowedValues", gsdml_eTag_F_Par_Version, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sF_ParameterRecordDataItem,F_Par_Version_AllowedValues), 0, "1"},
  { "Visible", gsdml_eTag_F_Par_Version, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Par_Version_Visible), 0, "true"},
  { "Changeable", gsdml_eTag_F_Par_Version, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Par_Version_Changeable), 0, "false"},
  //
  // F_Source_Add
  //
  { "DefaultValue", gsdml_eTag_F_Source_Add, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sF_ParameterRecordDataItem,F_Source_Add_DefaultValue), 0, "1"},
  { "AllowedValues", gsdml_eTag_F_Source_Add, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sF_ParameterRecordDataItem,F_Source_Add_AllowedValues), 0, "1..65534"},
  { "Visible", gsdml_eTag_F_Source_Add, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Source_Add_Visible), 0, "true"},
  { "Changeable", gsdml_eTag_F_Source_Add, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Source_Add_Changeable), 0, "false"},
  //
  // F_Dest_Add
  //
  { "DefaultValue", gsdml_eTag_F_Dest_Add, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sF_ParameterRecordDataItem,F_Dest_Add_DefaultValue), 0, "1"},
  { "AllowedValues", gsdml_eTag_F_Dest_Add, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sF_ParameterRecordDataItem,F_Dest_Add_AllowedValues), 0, "0..65534"},
  { "Visible", gsdml_eTag_F_Dest_Add, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Dest_Add_Visible), 0, "true"},
  { "Changeable", gsdml_eTag_F_Dest_Add, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Dest_Add_Changeable), 0, "true"},
  //
  // F_WD_Time
  //
  { "DefaultValue", gsdml_eTag_F_WD_Time, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sF_ParameterRecordDataItem,F_WD_Time_DefaultValue), 0, "150"},
  { "AllowedValues", gsdml_eTag_F_WD_Time, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sF_ParameterRecordDataItem,F_WD_Time_AllowedValues), 0, "1..65535"},
  { "Visible", gsdml_eTag_F_WD_Time, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_WD_Time_Visible), 0, "true"},
  { "Changeable", gsdml_eTag_F_WD_Time, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_WD_Time_Changeable), 0, "true"},
  //
  // F_Par_CRC
  //
  { "DefaultValue", gsdml_eTag_F_Par_CRC, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sF_ParameterRecordDataItem,F_Par_CRC_DefaultValue), 0, "53356"},
  { "AllowedValues", gsdml_eTag_F_Par_CRC, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sF_ParameterRecordDataItem,F_Par_CRC_AllowedValues), 0, "1..65535"},
  { "Visible", gsdml_eTag_F_Par_CRC, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Par_CRC_Visible), 0, "true"},
  { "Changeable", gsdml_eTag_F_Par_CRC, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_Par_CRC_Changeable), 0, "true"},
  //
  // F_iPar_CRC
  //
  { "DefaultValue", gsdml_eTag_F_iPar_CRC, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sF_ParameterRecordDataItem,F_iPar_CRC_DefaultValue), 0, "0"},
  { "AllowedValues", gsdml_eTag_F_iPar_CRC, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sF_ParameterRecordDataItem,F_iPar_CRC_AllowedValues), 0, "0..4294967295"},
  { "Visible", gsdml_eTag_F_iPar_CRC, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_iPar_CRC_Visible), 0, "true"},
  { "Changeable", gsdml_eTag_F_iPar_CRC, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sF_ParameterRecordDataItem,F_iPar_CRC_Changeable), 0, "true"},
  //
  // GraphicItemRef
  //
  { "Type", gsdml_eTag_GraphicItemRef, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sGraphicItemRef,Type), 0, ""},
  { "GraphicItemTarget", gsdml_eTag_GraphicItemRef, gsdml_eTag_, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sGraphicItemRef,GraphicItemTarget), 0, ""},
  //
  // IsochroneMode
  //
  { "T_DC_Base", gsdml_eTag_IsochroneMode, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sIsochroneMode,T_DC_Base), 0, ""},
  { "T_DC_Min", gsdml_eTag_IsochroneMode, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sIsochroneMode,T_DC_Min), 0, ""},
  { "T_DC_Max", gsdml_eTag_IsochroneMode, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sIsochroneMode,T_DC_Max), 0, ""},
  { "T_IO_Base", gsdml_eTag_IsochroneMode, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sIsochroneMode,T_IO_Base), 0, ""},
  { "T_IO_InputMin", gsdml_eTag_IsochroneMode, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sIsochroneMode,T_IO_InputMin), 0, ""},
  { "T_IO_OutputMin", gsdml_eTag_IsochroneMode, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sIsochroneMode,T_IO_OutputMin), 0, ""},
  { "IsochroneModeRequired", gsdml_eTag_IsochroneMode, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sIsochroneMode,IsochroneModeRequired), 0, "false"},
  //
  // InterfaceSubmoduleItem
  //
  { "SubslotNumber", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sInterfaceSubmoduleItem,SubslotNumber), 0, "32768"},
  { "TextId", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sInterfaceSubmoduleItem,TextId), 0, ""},
  { "SupportedRT_Class", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sInterfaceSubmoduleItem,SupportedRT_Class), 0, "Class1"},
  { "SupportedRT_Classes", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sInterfaceSubmoduleItem,SupportedRT_Classes), 0, "RT_CLASS_1"},
  { "IsochroneModeSupported", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sInterfaceSubmoduleItem,IsochroneModeSupported), 0, "false"},
  { "IsochroneModeInRT_Classes", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sInterfaceSubmoduleItem,IsochroneModeInRT_Classes), 0, ""},
  { "SubmoduleIdentNumber", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned32hex, sizeof(gsdml_tUnsigned32hex), offsetof(gsdml_sInterfaceSubmoduleItem,SubmoduleIdentNumber), 0, ""},
  { "SupportedProtocols", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sInterfaceSubmoduleItem,SupportedProtocols), 0, ""},
  { "SupportedMibs", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sInterfaceSubmoduleItem,SupportedMibs), 0, ""},
  { "NetworkComponentDiagnosisSupported", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sInterfaceSubmoduleItem,NetworkComponentDiagnosisSupported), 0, "false"},
  { "DCP_HelloSupported", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sInterfaceSubmoduleItem,DCP_HelloSupported), 0, "false"},
  { "PTP_BoundarySupported", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sInterfaceSubmoduleItem,PTP_BoundarySupported), 0, "false"},
  { "DCP_BoundarySupported", gsdml_eTag_InterfaceSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sInterfaceSubmoduleItem,DCP_BoundarySupported), 0, "false"},
  //
  // DCP_FlashOnceSignalUnit
  //
  { "TextId", gsdml_eTag_DCP_FlashOnceSignalUnit, gsdml_eTag_, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sDCP_FlashOnceSignalUnit,TextId), 0, ""},
  //
  // RT_Class3Properties
  //
  { "MaxBridgeDelay", gsdml_eTag_RT_Class3Properties, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sRT_Class3Properties,MaxBridgeDelay), 0, ""},
  { "MaxNumberIR_FrameData", gsdml_eTag_RT_Class3Properties, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sRT_Class3Properties,MaxNumberIR_FrameData), 0, ""},
  //
  // SynchronisationMode
  //
  { "SupportedRole", gsdml_eTag_SynchronisationMode, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sSynchronisationMode,SupportedRole), 0, "SyncSlave"},
  { "MaxLocalJitter", gsdml_eTag_SynchronisationMode, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sSynchronisationMode,MaxLocalJitter), 0, ""},
  { "T_PLL_MAX", gsdml_eTag_SynchronisationMode, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sSynchronisationMode,T_PLL_MAX), 0, "1000"},
  { "SupportedSyncProtocols", gsdml_eTag_SynchronisationMode, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sSynchronisationMode,SupportedSyncProtocols), 0, ""},
  //
  // ApplicationRelations (InterfaceSubmoduleItem)
  //
  { "NumberOfAdditionalInputCR", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sInterfaceSubmoduleItem_ApplicationRelations,NumberOfAdditionalInputCR), 0, "0"},
  { "NumberOfAdditionalOutputCR", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sInterfaceSubmoduleItem_ApplicationRelations,NumberOfAdditionalOutputCR), 0, "0"},
  { "NumberOfAdditionalMulticastProviderCR", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sInterfaceSubmoduleItem_ApplicationRelations,NumberOfAdditionalMulticastProviderCR), 0, "0"},
  { "NumberOfMulticastConsumerCR", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sInterfaceSubmoduleItem_ApplicationRelations,NumberOfMulticastConsumerCR), 0, "0"},
  { "PullModuleAlarmSupported", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sInterfaceSubmoduleItem_ApplicationRelations,PullModuleAlarmSupported), 0, "false"},
  // These should not exist in InterfaceSubmodulItem ...
  { "AR_BlockVersion", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Unsigned16, 0, 0, 1, ""},
  { "IOCR_BlockVersion", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Unsigned16, 0, 0, 1, ""},
  { "AlarmCR_BlockVersion", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Unsigned16, 0, 0, 1, ""},
  { "SubmoduleDataBlockVersion", gsdml_eTag_ApplicationRelations, gsdml_eTag_InterfaceSubmoduleItem, gsdml_eType_Unsigned16, 0, 0, 1, ""},
  //
  // TimingProperties
  //
  { "SendClock", gsdml_eTag_TimingProperties, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sTimingProperties,SendClock), 0, "32"},
  { "ReductionRatio", gsdml_eTag_TimingProperties, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sTimingProperties,ReductionRatio), 0, "1 2 4 8 16 32 64 128 256 512"},
  { "ReductionRatioPow2", gsdml_eTag_TimingProperties, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sTimingProperties,ReductionRatioPow2), 0, "0"},
  { "ReductionRationNonPow2", gsdml_eTag_TimingProperties, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sTimingProperties,ReductionRatioNonPow2), 0, "0"},
  //
  // RT_Class3TimingProperties
  //
  { "SendClock", gsdml_eTag_RT_Class3TimingProperties, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sRT_Class3TimingProperties,SendClock), 0, "32"},
  { "ReductionRatio", gsdml_eTag_RT_Class3TimingProperties, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sRT_Class3TimingProperties,ReductionRatio), 0, "1 2 4 8 16"},
  { "ReductionRatioPow2", gsdml_eTag_RT_Class3TimingProperties, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sRT_Class3TimingProperties,ReductionRatioPow2), 0, "0"},
  { "ReductionRatioNonPow2", gsdml_eTag_RT_Class3TimingProperties, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sRT_Class3TimingProperties,ReductionRatioNonPow2), 0, "0"},
  //
  // MediaRedundancy
  //
  { "RT_MediaRedundancySupported", gsdml_eTag_MediaRedundancy, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sMediaRedundancy,RT_MediaRedundancySupported), 0, "true"},
  { "SupportedRole", gsdml_eTag_MediaRedundancy, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sMediaRedundancy,SupportedRole), 0, "Client"},
  //
  // PortSubmoduleItem
  //
  { "SubslotNumber", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sPortSubmoduleItem,SubslotNumber), 0, ""},
  { "TextId", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sPortSubmoduleItem,TextId), 0, ""},
  { "MAUType", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sPortSubmoduleItem,MAUType), 0, "100BASETXFD"},
  { "MAUTypes", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sPortSubmoduleItem,MAUTypes), 0, "16"},
  { "FiberOpticTypes", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sPortSubmoduleItem,FiberOpticTypes), 0, ""},
  { "MaxPortTxDelay", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sPortSubmoduleItem,MaxPortTxDelay), 0, ""},
  { "MaxPortRxDelay", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sPortSubmoduleItem,MaxPortRxDelay), 0, ""},
  { "SubmoduleIdentNumber", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Unsigned32hex, sizeof(gsdml_tUnsigned32hex), offsetof(gsdml_sPortSubmoduleItem,SubmoduleIdentNumber), 0, ""},
  { "PortDeactivationSupported", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sPortSubmoduleItem,PortDeactivationSupported), 0, "false"},
  { "LinkStateDiagnosisCapability", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sPortSubmoduleItem,LinkStateDiagnosisCapability), 0, ""},
  { "PowerBudgetControlSupported", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sPortSubmoduleItem,PowerBudgetControlSupported), 0, "false"},
  { "SupportsRingportConfig", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sPortSubmoduleItem,SupportsRingportConfig), 0, "false"},
  { "IsDefaultRingport", gsdml_eTag_PortSubmoduleItem, gsdml_eTag_, gsdml_eType_Boolean, sizeof(gsdml_tBoolean), offsetof(gsdml_sPortSubmoduleItem,IsDefaultRingport), 0, "false"},
  //
  // DeviceAccessPointItem-ApplicationRelations
  //
  { "AR_BlockVersion", gsdml_eTag_ApplicationRelations, gsdml_eTag_DeviceAccessPointItem, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sDeviceAccessPointItem_ApplicationRelations,AR_BlockVersion), 0, ""},
  { "IOCR_BlockVersion", gsdml_eTag_ApplicationRelations, gsdml_eTag_DeviceAccessPointItem, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sDeviceAccessPointItem_ApplicationRelations,IOCR_BlockVersion), 0, ""},
  { "AlarmCR_BlockVersion", gsdml_eTag_ApplicationRelations, gsdml_eTag_DeviceAccessPointItem, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sDeviceAccessPointItem_ApplicationRelations,AlarmCR_BlockVersion), 0, ""},
  { "SubmoduleDataBlockVersion", gsdml_eTag_ApplicationRelations, gsdml_eTag_DeviceAccessPointItem, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sDeviceAccessPointItem_ApplicationRelations,SubmoduleDataBlockVersion), 0, ""},
  //
  // SubmoduleItemRef
  //
  { "SubmoduleItemTarget", gsdml_eTag_SubmoduleItemRef, gsdml_eTag_, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sSubmoduleItemRef,SubmoduleItemTarget), 0, ""},
  { "AllowedInSubslots", gsdml_eTag_SubmoduleItemRef, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sSubmoduleItemRef,AllowedInSubslots), 0, ""},
  { "UsedInSubslots", gsdml_eTag_SubmoduleItemRef, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sSubmoduleItemRef,UsedInSubslots), 0, ""},
  { "FixedInSubslots", gsdml_eTag_SubmoduleItemRef, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sSubmoduleItemRef,FixedInSubslots), 0, ""},
  //
  // SlotGroup
  //
  { "SlotList", gsdml_eTag_SlotGroup, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sSlotGroup,SlotList), 0, ""},
  { "TextId", gsdml_eTag_Name, gsdml_eTag_SlotGroup, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sSlotGroup,Name), 0, ""},
  { "TextId", gsdml_eTag_InfoText, gsdml_eTag_SlotGroup, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sSlotGroup,InfoText), 0, ""},
  //
  // ModuleItem
  //
  { "ID", gsdml_eTag_ModuleItem, gsdml_eTag_, gsdml_eType_Id, sizeof(gsdml_tId), offsetof(gsdml_sModuleItem,ID), 0, ""},
  { "ModuleIdentNumber", gsdml_eTag_ModuleItem, gsdml_eTag_, gsdml_eType_Unsigned32hex, sizeof(gsdml_tUnsigned32hex), offsetof(gsdml_sModuleItem,ModuleIdentNumber), 0, ""},
  { "RequiredSchemaVersion", gsdml_eTag_ModuleItem, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString), offsetof(gsdml_sModuleItem,RequiredSchemaVersion), 0, "V1.0"},
  { "PhysicalSubslots", gsdml_eTag_ModuleItem, gsdml_eTag_, gsdml_eType_ValueList, sizeof(gsdml_tValueList), offsetof(gsdml_sModuleItem,PhysicalSubslots), 0, ""},
  //
  // ValueItem
  //
  { "ID", gsdml_eTag_ValueItem, gsdml_eTag_, gsdml_eType_Id, sizeof(gsdml_tId), offsetof(gsdml_sValueItem,ID), 0, ""},
  { "TextId", gsdml_eTag_Help, gsdml_eTag_ValueItem, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sValueItem,Help), 0, ""},
  //
  // Assign
  //
  { "Content", gsdml_eTag_Assign, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString), offsetof(gsdml_sAssign,Content), 0, ""},
  { "TextId", gsdml_eTag_Assign, gsdml_eTag_, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sAssign,TextId), 0, ""},
  //
  // ChannelDiagItem
  //
  { "ErrorType", gsdml_eTag_ChannelDiagItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sChannelDiagItem,ErrorType), 0, ""},
  { "MaintenanceAlarmState", gsdml_eTag_ChannelDiagItem, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sChannelDiagItem,ErrorType), 0, ""},
  { "API", gsdml_eTag_ChannelDiagItem, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sChannelDiagItem,API), 0, ""},
  { "TextId", gsdml_eTag_Name, gsdml_eTag_ChannelDiagItem, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sChannelDiagItem,Name), 0, ""},
  { "TextId", gsdml_eTag_Help, gsdml_eTag_ChannelDiagItem, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sChannelDiagItem,Help), 0, ""},
  //
  // ExtChannelDiagItem
  //
  { "ErrorType", gsdml_eTag_ExtChannelDiagItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sExtChannelDiagItem,ErrorType), 0, ""},
  { "MaintenanceAlarmState", gsdml_eTag_ChannelDiagItem, gsdml_eTag_, gsdml_eType_TokenList, sizeof(gsdml_tTokenList), offsetof(gsdml_sExtChannelDiagItem,ErrorType), 0, ""},
  { "API", gsdml_eTag_ExtChannelDiagItem, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sExtChannelDiagItem,API), 0, ""},
  { "TextId", gsdml_eTag_Name, gsdml_eTag_ExtChannelDiagItem, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sExtChannelDiagItem,Name), 0, ""},
  { "TextId", gsdml_eTag_Help, gsdml_eTag_ExtChannelDiagItem, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sExtChannelDiagItem,Help), 0, ""},
  //
  // UnitDiagTypeItem
  //
  { "UserStructureIdentifier", gsdml_eTag_UnitDiagTypeItem, gsdml_eTag_, gsdml_eType_Unsigned16, sizeof(gsdml_tUnsigned16), offsetof(gsdml_sUnitDiagTypeItem,UserStructureIdentifier), 0, ""},
  { "API", gsdml_eTag_UnitDiagTypeItem, gsdml_eTag_, gsdml_eType_Unsigned32, sizeof(gsdml_tUnsigned32), offsetof(gsdml_sUnitDiagTypeItem,API), 0, ""},
  //
  // GraphicItem
  //
  { "ID", gsdml_eTag_GraphicItem, gsdml_eTag_, gsdml_eType_Id, sizeof(gsdml_tId), offsetof(gsdml_sGraphicItem,ID), 0, ""},
  { "GraphicFile", gsdml_eTag_GraphicItem, gsdml_eTag_, gsdml_eType_String, sizeof(gsdml_tString), offsetof(gsdml_sGraphicItem,GraphicFile), 0, ""},
  //
  // CategoryItem
  //
  { "ID", gsdml_eTag_CategoryItem, gsdml_eTag_, gsdml_eType_Id, sizeof(gsdml_tId), offsetof(gsdml_sCategoryItem,ID), 0, ""},
  { "TextId", gsdml_eTag_CategoryItem, gsdml_eTag_, gsdml_eType_RefIdT, sizeof(gsdml_tRefIdT), offsetof(gsdml_sCategoryItem,TextId), 0, ""},
  { "TextId", gsdml_eTag_InfoText, gsdml_eTag_CategoryItem, gsdml_eType_RefId, sizeof(gsdml_tRefId), offsetof(gsdml_sCategoryItem,InfoText), 0, ""},
  //
  // Text
  //
  { "TextId", gsdml_eTag_Text, gsdml_eTag_, gsdml_eType_IdT, sizeof(gsdml_tIdT), offsetof(gsdml_sText,TextId), 0, ""},
  { "Value", gsdml_eTag_Text, gsdml_eTag_, gsdml_eType_AllocatedString, 0, offsetof(gsdml_sText,Value), 0, ""},
  //
  // Language
  //
  { "xml:lang", gsdml_eTag_Language, gsdml_eTag_, gsdml_eType_Enum, sizeof(gsdml_tEnum), offsetof(gsdml_sLanguage,xml_lang), 0, ""},
  //
  { "", gsdml_eTag_, gsdml_eTag_, gsdml_eType_, 0, 0, 0, ""}};

pn_gsdml::pn_gsdml() : logglevel(0), first_token(true), state( gsdml_eState_Init), line_cnt(1), 
		       char_cnt(0), in_comment(false), c(0), c_f(0), c_ff(0), c_sts(0), c_f_sts(0), 
		       c_ff_sts((void *)1), current_tag_idx(0), current_attribute_name_idx(0),
		       current_attribute_value_idx(0), current_tag_value_idx(0), suppress_msg(0),
		       tag_stack_cnt(0), object_stack_cnt(0), current_body(0), current_body_size(0), 
		       byte_order(0), module_classlist(0), xml(0), ProfileHeader(0), 
		       DeviceIdentity(0), DeviceFunction(0), ApplicationProcess(0)
{
  strcpy( gsdmlfile, "");
}

pn_gsdml::~pn_gsdml()
{
  if ( xml)
    delete xml;
  if ( ProfileHeader)
    delete ProfileHeader;
  if ( DeviceIdentity)
    delete DeviceIdentity;
  if ( DeviceFunction)
    delete DeviceFunction;
  if ( ApplicationProcess)
    delete ApplicationProcess;
}

void pn_gsdml::set_language( const char *lang)
{
  strncpy( current_lang, lang, sizeof(current_lang));
}

void *pn_gsdml::next_token()
{
  char t;
  void *sts;

  if ( first_token) {
    first_token = false;

    c_sts = fp.get(c);
    if ( !c_sts) return c_sts;
    
    c_f_sts = fp.get(c_f);
    if ( c_f_sts)
      c_ff_sts = fp.get(c_ff);
  }
  else {
        
    if ( c_ff_sts)
      sts = fp.get(t);

    c = c_f;
    c_sts = c_f_sts;
    c_f = c_ff;
    c_f_sts = c_ff_sts;
    if ( c_ff_sts)
      c_ff_sts = sts;
    if ( sts)
      c_ff = t;
  }
  if ( c_sts) {
    if ( c == '\n') {
      line_cnt++;
      char_cnt = 1;
    }
    else
      char_cnt++;
  }
  return c_sts;
}

void pn_gsdml::error_message_line( const char *msg)
{
  printf( "** GSDML-parser error, %s, line %d, %d\n", msg, line_cnt, char_cnt);
}

void pn_gsdml::error_message( const char *format, const char *value)
{
  printf( "** GSDML-parser error, ");
  printf( format, value);
  printf( "\n");
}

bool pn_gsdml::is_space( const char c)
{
  return c == ' ' || c == '	' || c == '\n' || c == '\r';
}

int pn_gsdml::read( const char *filename)
{
  pwr_tFileName fname;

  strncpy( gsdmlfile, filename, sizeof(gsdmlfile));

  dcli_translate_filename( fname, filename);
  fp.open( fname);
  if ( !fp)
    return PB__GSDMLFILE;

  while ( next_token()) {
    if ( in_comment) {
      if ( c == '-' && c_f == '-' && c_ff == '>') {
	// End of comment
	in_comment = false;
	next_token();
	next_token();
	continue;
      }
      else
	continue;
    }
    //
    //  State MetaTag
    //
    if ( state & gsdml_eState_MetaTag) {
      if ( !(state & gsdml_eState_TagNameFound)) {
	if ( is_space(c))
	  continue;
	else {
	  // Start of tag name
	  state |= gsdml_eState_TagName;
	  state |= gsdml_eState_TagNameFound;
	  current_tag_idx = 0;
	  current_tag[current_tag_idx++] = c;
	}
      }
      else {
	if ( state & gsdml_eState_TagName) {
	  if ( is_space(c)) {
	    // End of tag name
	    current_tag[current_tag_idx] = 0;	
	    state &= ~gsdml_eState_TagName;
	    metatag( current_tag);	
	  }
	  else if ( c == '?' && c_f == '>') {
	    // End of meta tag
	    next_token();
	    current_tag[current_tag_idx] = 0;	
	    if ( state & gsdml_eState_AttributeName || 
		 state & gsdml_eState_AttributeValue)
	      error_message_line( "Syntax error");
	    state &= ~gsdml_eState_TagName;
	    state &= ~gsdml_eState_TagNameFound;
	    state &= ~gsdml_eState_MetaTag;
	    metatag( current_tag);	
	  }
	  else {
	    // Next tag character
	    if ( current_tag_idx >= sizeof(current_tag) - 1)
	      error_message_line( "Buffer to small");
	    else
	      current_tag[current_tag_idx++] = c;
	  }
	}
	else {
	  if ( is_space(c) && !(state & gsdml_eState_AttributeValue))
	    continue;
	  else if ( c == '?' && c_f == '>') {
	    // End of meta tag
	    next_token();
	    if ( state & gsdml_eState_AttributeName || 
		 state & gsdml_eState_AttributeValue)
	      error_message_line( "Syntax error");
	    state &= ~gsdml_eState_AttributeNameFound;
	    state &= ~gsdml_eState_AttributeValueFound;
	    state &= ~gsdml_eState_TagNameFound;
	    state &= ~gsdml_eState_MetaTag;
	    metatag_end( current_tag);
	  }
	  else {
	    if ( !(state & gsdml_eState_AttributeNameFound)) {
	      // Start of attribute name
	      state |= gsdml_eState_Attribute;
	      state |= gsdml_eState_AttributeName;
	      state |= gsdml_eState_AttributeNameFound;
	      current_attribute_name_idx = 0;
	      current_attribute_name[current_attribute_name_idx++] = c;
	    }
	    else if ( state & gsdml_eState_AttributeName) {
	      if ( is_space(c)) {
		// End of attribute name
		current_attribute_name[current_attribute_name_idx] = 0;
		state &= ~gsdml_eState_AttributeName;
	      }
	      else if ( c == '=') {
		// End of attribute name
		current_attribute_name[current_attribute_name_idx] = 0;
		state &= ~gsdml_eState_AttributeName;
	      }
	      else {
		// Next char of attribute name
		if ( current_attribute_name_idx >= sizeof(current_attribute_name) - 1)
		  error_message_line( "Buffer to small");
		else
		  current_attribute_name[current_attribute_name_idx++] = c;
	      }
	    }
	    else if ( state & gsdml_eState_AttributeNameFound) {
	      if ( !(state & gsdml_eState_AttributeValueFound)) {
		if ( c == '\"') {
		  // Start of attribute value
		  state |= gsdml_eState_AttributeValue;
		  state |= gsdml_eState_AttributeValueFound;
		  current_attribute_value_idx = 0;
		}
		else
		  continue;
	      }
	      else if ( state & gsdml_eState_AttributeValue) {
		if ( c == '\\' && c_f == '\"') {
		  // '"' sign in string
		  if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1)
		    error_message_line( "Buffer to small");
		  else
		    current_attribute_value[current_attribute_value_idx++] = c_f;		 
		  next_token();
		}
		else if ( c == '\"') {
		  // End of attribute value
		  current_attribute_value[current_attribute_value_idx] = 0;
		  state &= ~gsdml_eState_AttributeValue;
		  state &= ~gsdml_eState_Attribute;
		  state &= ~gsdml_eState_AttributeNameFound;
		  state &= ~gsdml_eState_AttributeValueFound;
		  tag_attribute( current_attribute_name, current_attribute_value);
		}
		else {
		  // Next char in attribute value
		  if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1)
		    error_message_line( "Buffer to small");
		  else
		    current_attribute_value[current_attribute_value_idx++] = c;
		}
	      }
      	    }
	  }
	}
      }
    }
    //
    //  State Tag
    //
    else if ( state & gsdml_eState_Tag) {
      if ( !(state & gsdml_eState_TagNameFound)) {
	if ( is_space(c))
	  continue;
	else {
	  // Start of tag name
	  state |= gsdml_eState_TagName;
	  state |= gsdml_eState_TagNameFound;
	  current_tag_idx = 0;
	  current_tag[current_tag_idx++] = c;
	}
      }
      else {
	if ( state & gsdml_eState_TagName) {
	  if ( is_space(c)) {
	    // End of tag name
	    current_tag[current_tag_idx] = 0;	
	    state &= ~gsdml_eState_TagName;
	    tag( current_tag);
	  }
	  else if ( c == '/' && c_f == '>') {
	    // End of tag
	    next_token();
	    current_tag[current_tag_idx] = 0;
	    tag( current_tag);
	    tag_end( current_tag);
	    if ( state & gsdml_eState_AttributeName || 
		 state & gsdml_eState_AttributeValue)
	      error_message_line( "Syntax error");
	    state &= ~gsdml_eState_TagName;
	    state &= ~gsdml_eState_TagNameFound;
	    state &= ~gsdml_eState_Tag;
	  }
	  else if ( c == '>') {
	    // End of tag
	    current_tag[current_tag_idx] = 0;	
	    if ( state & gsdml_eState_AttributeName || 
		 state & gsdml_eState_AttributeValue)
	      error_message_line( "Syntax error");
	    state &= ~gsdml_eState_TagName;
	    state &= ~gsdml_eState_TagNameFound;
	    state &= ~gsdml_eState_Tag;
	    state |= gsdml_eState_TagValue;
	    current_tag_value_idx = 0;
	    tag( current_tag);
	  }
	  else {
	    // Next tag character
	    if ( current_tag_idx >= sizeof(current_tag) - 1)
	      error_message_line( "Buffer to small");
	    else
	      current_tag[current_tag_idx++] = c;
	  }
	}
	else {
	  if ( is_space(c) && !(state & gsdml_eState_AttributeValue))
	    continue;
	  else if ( c == '/' && c_f == '>') {
	    // End of tag
	    next_token();
	    if ( state & gsdml_eState_AttributeName)
	      error_message_line( "Syntax error");
	    else if ( state & gsdml_eState_AttributeValue) {
	      // Next char in attribute value
	      if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1)
		error_message_line( "Buffer to small");
	      else
		current_attribute_value[current_attribute_value_idx++] = c;
	      continue;
	    }
	    tag_end( current_tag);
	    state &= ~gsdml_eState_AttributeNameFound;
	    state &= ~gsdml_eState_AttributeValueFound;
	    state &= ~gsdml_eState_TagNameFound;
	    state &= ~gsdml_eState_Tag;	    
	    suppress_msg = 0;
	  }
	  else if ( c == '>') {
	    // End of tag
	    if ( state & gsdml_eState_AttributeName)
	      error_message_line( "Syntax error");
	    else if ( state & gsdml_eState_AttributeValue) {
	      // Next char in attribute value
	      if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1) {
		if ( !suppress_msg) {
		  error_message_line( "Buffer size to small, value cut off");
		  suppress_msg = 1;
		}
	      }
	      else
		current_attribute_value[current_attribute_value_idx++] = c;
	      continue;
	    }
	    state &= ~gsdml_eState_AttributeNameFound;
	    state &= ~gsdml_eState_AttributeValueFound;
	    state &= ~gsdml_eState_TagNameFound;
	    state &= ~gsdml_eState_Tag;
	    suppress_msg = 0;
	  }
	  else {
	    if ( !(state & gsdml_eState_AttributeNameFound)) {
	      // Start of attribute name
	      state |= gsdml_eState_Attribute;
	      state |= gsdml_eState_AttributeName;
	      state |= gsdml_eState_AttributeNameFound;
	      current_attribute_name_idx = 0;
	      current_attribute_name[current_attribute_name_idx++] = c;
	    }
	    else if ( state & gsdml_eState_AttributeName) {
	      if ( is_space(c)) {
		// End of attribute name
		current_attribute_name[current_attribute_name_idx] = 0;
		state &= ~gsdml_eState_AttributeName;
	      }
	      else if ( c == '=') {
		// End of attribute name
		current_attribute_name[current_attribute_name_idx] = 0;
		state &= ~gsdml_eState_AttributeName;
	      }
	      else {
		// Next char of attribute name
		if ( current_attribute_name_idx >= sizeof(current_attribute_name) - 1)
		  error_message_line( "Buffer size to small");
		else
		  current_attribute_name[current_attribute_name_idx++] = c;
	      }
	    }
	    else if ( state & gsdml_eState_AttributeNameFound) {
	      if ( !(state & gsdml_eState_AttributeValueFound)) {
		if ( c == '\"') {
		  // Start of attribute value
		  state |= gsdml_eState_AttributeValue;
		  state |= gsdml_eState_AttributeValueFound;
		  current_attribute_value_idx = 0;
		}
		else
		  continue;
	      }
	      else if ( state & gsdml_eState_AttributeValue) {
		if ( c == '\\' && c_f == '\"') {
		  // '"' sign in string
		  if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1) {
		    if ( !suppress_msg) {
		      error_message_line( "Buffer size to small, value cut off");
		      suppress_msg = 1;
		    }
		  }
		  else
		    current_attribute_value[current_attribute_value_idx++] = c_f;		 
		  next_token();
		}
		else if ( c == '\"') {
		  // End of attribute value
		  current_attribute_value[current_attribute_value_idx] = 0;
		  state &= ~gsdml_eState_AttributeValue;
		  state &= ~gsdml_eState_Attribute;
		  state &= ~gsdml_eState_AttributeNameFound;
		  state &= ~gsdml_eState_AttributeValueFound;
		  suppress_msg = 0;
		  tag_attribute( current_attribute_name, current_attribute_value);
		}
		else {
		  // Next char in attribute value
		  if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1) {
		    if ( !suppress_msg) {
		      error_message_line( "Buffer size to small, value cut off");
		      suppress_msg = 1;
		    }
		  }
		  else
		    current_attribute_value[current_attribute_value_idx++] = c;
		}
	      }
      	    }
	  }
	}
      }
    }
    //
    //  State TagValue
    //
    else if ( state & gsdml_eState_TagValue) {
      if ( c == '<' && c_f == '!' && c_ff == '-' && !(state & gsdml_eState_AttributeValue)) {
	in_comment = true;
	next_token();
	next_token();
      }
      else if ( c == '<' && c_f == '/') {
	// Start of EndTag
	next_token();
	current_tag_value[current_tag_value_idx] = 0;
	if ( state & gsdml_eState_TagValueFound) {
	  tag_value( current_tag_value);
	  state &= ~gsdml_eState_TagValueFound;
	}
	state &= ~gsdml_eState_TagValue;
	state |= gsdml_eState_EndTag;
	current_tag_idx = 0;
      }
      else if ( c == '<' && !(state & gsdml_eState_TagValueFound)) {
	// New tag, no value found
	state &= ~gsdml_eState_TagValue;
	state |= gsdml_eState_Tag;
	current_tag_idx = 0;
      }
      else {
	if ( !is_space(c))
	  state |= gsdml_eState_TagValueFound;
	if ( current_tag_value_idx >= sizeof(current_tag_value) - 1)
	  error_message_line( "Buffer to small");
	else
	  current_tag_value[current_tag_value_idx++] = c;
      }
    }
    //
    //  State EndTag
    //
    else if ( state & gsdml_eState_EndTag) {
      if ( !(state & gsdml_eState_TagNameFound)) {
	if ( is_space(c))
	  continue;
	else {
	  // Start of tag name
	  state |= gsdml_eState_TagName;
	  state |= gsdml_eState_TagNameFound;
	  current_tag_idx = 0;
	  current_tag[current_tag_idx++] = c;
	}
      }
      else {
	if ( state & gsdml_eState_TagName) {
	  if ( is_space(c)) {
	    // End of tag name
	    current_tag[current_tag_idx] = 0;	
	    state &= ~gsdml_eState_TagName;
	    tag_end( current_tag);
	  }
	  else if ( c == '>') {
	    // End of tag
	    current_tag[current_tag_idx] = 0;	
	    state &= ~gsdml_eState_TagName;
	    state &= ~gsdml_eState_TagNameFound;
	    state &= ~gsdml_eState_EndTag;
	    tag_end( current_tag);
	  }
	  else {
	    // Next tag character
	    if ( current_tag_idx >= sizeof(current_tag) - 1)
	      error_message_line( "Buffer to small");
	    else
	      current_tag[current_tag_idx++] = c;
	  }
	}
	else {
	  if ( is_space(c))
	    continue;
	  else if ( c == '>') {
	    // End of tag
	    state &= ~gsdml_eState_TagNameFound;
	    state &= ~gsdml_eState_EndTag;
	  }
	}
      }
    }
    else if ( state & gsdml_eState_Init) {
      if ( c == '<' && c_f == '?') {
	state |= gsdml_eState_MetaTag;
	current_tag_idx = 0;
	next_token();
      }
      else if ( c == '<' && c_f == '!' && c_ff == '-') {
	in_comment = true;
	next_token();
	next_token();
      }
      else if ( c == '<' && c_f == '/') {
	state |= gsdml_eState_EndTag;
	current_tag_idx = 0;
	next_token();
      }
      else if ( c == '<') {
	state |= gsdml_eState_Tag;
	current_tag_idx = 0;
      }
    }
  }

  fp.close();

  if ( logglevel & 1)
    gsdml_print();
  return PB__SUCCESS;
}

int pn_gsdml::tag( const char *name)
{
  int idx;
  int sts;

  if ( logglevel & 2)
    printf( "Tag: %s\n", name);

  sts = find_tag( name, &idx);
  if ( !sts) {
    error_message_line( "Unknown tag");
    return 0;
  }

  tag_stack_push( taglist[idx].id);
  if ( taglist[idx].has_data) {
    void *o;

    o = object_factory( taglist[idx].id);
    object_stack_push( o, taglist[idx].id);

  }
  return 1;
}

int pn_gsdml::metatag( const char *name)
{
  return tag( name);
}

int pn_gsdml::tag_end( const char *name)
{
  int idx;
  int sts;

  if ( logglevel & 2)
    printf( "EndTag: %s\n", name);

  sts = find_tag( name, &idx);
  if ( !sts) {
    error_message_line( "Unknown tag");
    return 0;
  }

  if ( taglist[idx].has_data) {
    object_stack_pull( taglist[idx].id);
  }

  tag_stack_pull( taglist[idx].id);
  return 1;
}

int pn_gsdml::metatag_end( const char *name)
{
  if ( logglevel & 2)
    printf( "EndMetaTag: %s\n", name);
  return 1;
}

int pn_gsdml::tag_value( const char *value)
{
  int idx;
  int sts;

  if ( logglevel & 2)
    printf( "TagValue: %s\n", value);

  sts = find_tag( get_tag_stack(), &idx);
  if ( !sts) {
    error_message_line( "Undefined tag");
    return 0;
  }

  if ( taglist[idx].offset + taglist[idx].size > current_body_size) {
    error_message_line( "Data offset error");
    return 0;
  }

  string_to_value( taglist[idx].type, taglist[idx].size, value, (char *)current_body + 
		   taglist[idx].offset);
  return 1;
}

int pn_gsdml::tag_attribute( const char *name, const char *value)
{
  int idx;
  int sts;

  if ( logglevel & 2)
    printf( "Attr: %s = \"%s\"\n", name, value);

  sts = find_tag_attribute( name, get_tag_stack(), get_tag_stack(1), &idx);
  if ( !sts) {
    error_message_line( "Undefined tag attribute");
    return 0;
  }

  if ( attrlist[idx].ignore)
    return 1;

  if ( attrlist[idx].offset + attrlist[idx].size > current_body_size) {
    error_message_line( "Data offset error");
    return 0;
  }

  string_to_value( attrlist[idx].type, attrlist[idx].size, value, (char *)current_body + 
		   attrlist[idx].offset);
  return 1;
}

int pn_gsdml::find_tag( const char *name, int *idx)
{
  for ( unsigned int i = 0; i < sizeof(taglist)/sizeof(taglist[0]); i++) {
    if ( strcmp( taglist[i].name, name) == 0) {
      *idx = i;
      return 1;
    }
  }
  return 0;
}

int pn_gsdml::find_tag( gsdml_eTag id, int *idx)
{
  for ( unsigned int i = 0; i < sizeof(taglist)/sizeof(taglist[0]); i++) {
    if ( taglist[i].id == id) {
      *idx = i;
      return 1;
    }
  }
  return 0;
}

int pn_gsdml::find_tag_attribute( const char *name, gsdml_eTag id, gsdml_eTag pid, int *idx)
{
  for ( unsigned int i = 0; i < sizeof(attrlist)/sizeof(attrlist[0]); i++) {
    if ( strcmp( attrlist[i].name, name) == 0 && 
	 (attrlist[i].id == gsdml_eTag_ || attrlist[i].id == id) &&
	 (attrlist[i].pid == gsdml_eTag_ || pid == attrlist[i].pid)) {
      *idx = i;
      return 1;
    }
  }
  return 0;
}

int pn_gsdml::tag_stack_push( gsdml_eTag id)
{
  if ( tag_stack_cnt >= sizeof(tag_stack)/sizeof(tag_stack[0]) - 1) {
    error_message_line( "Tag stack overflow");
    return 0;
  }
  tag_stack[tag_stack_cnt++] = id;
  return 1;
}

int pn_gsdml::tag_stack_pull( gsdml_eTag id)
{
  if ( tag_stack_cnt < 1 || tag_stack[tag_stack_cnt - 1] != id) {
    error_message_line( "Tag stack missmatch");
    return 0;
  }
  tag_stack_cnt--;
  return 1;
}

int pn_gsdml::object_stack_push( void *o, gsdml_eTag id)
{
  if ( object_stack_cnt >= sizeof(object_stack)/sizeof(object_stack[0]) - 1) {
    error_message_line( "Object stack overflow");
    return 0;
  }
  object_stack_id[object_stack_cnt] = id;
  object_stack[object_stack_cnt++] = o;
  return 1;
}

int pn_gsdml::object_stack_pull( gsdml_eTag id)
{
  if ( id != object_stack_id[object_stack_cnt-1] ||
       object_stack_cnt == 0) {
    error_message_line( "Object stack pull mismatch");
    return 0;
  }

  object_stack_cnt--;
  return 1;
}


int pn_gsdml::string_to_value( gsdml_eType type, unsigned int size, const char *str, void *buf)
{
  switch ( type) {
  case gsdml_eType_Integer:
    sscanf( str, "%d", (int *)buf);
    break;
  case gsdml_eType_Boolean:
    if ( strcmp( str, "true") == 0)
      *(gsdml_tBoolean *)buf = 1;
    else
      *(gsdml_tBoolean *)buf = 0;
    break;
  case gsdml_eType_Unsigned8:
    sscanf( str, "%hhu", (unsigned char *)buf);
    break;
  case gsdml_eType_Unsigned16:
    sscanf( str, "%hu", (unsigned short *)buf);
    break;
  case gsdml_eType_Unsigned32:
    sscanf( str, "%u", (unsigned int *)buf);
    break;
  case gsdml_eType_Unsigned16hex:
    sscanf( str, "%hx", (unsigned short *)buf);
    break;
  case gsdml_eType_Unsigned32hex:
    sscanf( str, "%x", (unsigned int *)buf);
    break;
  case gsdml_eType_String:
  case gsdml_eType_NormalizedString:
  case gsdml_eType_Token:
  case gsdml_eType_TokenList:
  case gsdml_eType_ValueList:
  case gsdml_eType_SignedOrFloatValueList:
  case gsdml_eType_Id:
  case gsdml_eType_IdT:
  case gsdml_eType_Enum:
    if ( strlen(str) >= size)
      error_message_line( "Attribute size to small, value cut off");
    strncpy( (char *)buf, str, size);
    break;
  case gsdml_eType_RefId:
    if ( strlen(str) >= size)
      error_message_line( "Attribute size to small, value cut off");
    strncpy( ((gsdml_tRefId *)buf)->ref, str, size);
    break;
  case gsdml_eType_RefIdT:
    if ( strlen(str) >= size)
      error_message_line( "Attribute size to small, value cut off");
    strncpy( ((gsdml_tRefIdT *)buf)->ref, str, size);
    break;
  case gsdml_eType_AllocatedString: 
    *(gsdml_tAllocatedString *) buf = (char *)malloc( strlen(str) + 1);
    strcpy( *(char **)buf, str);
    break;
  case gsdml_eType_:
  case gsdml_eType__:
    break;
  }
  return 1;
}

int pn_gsdml::string_to_value_datatype( char *str, gsdml_eValueDataType *type)
{
  if ( strcmp( str, "Bit") == 0)
    *type = gsdml_eValueDataType_Bit;
  else if ( strcmp( str, "BitArea") == 0)
    *type = gsdml_eValueDataType_BitArea;
  else if ( strcmp( str, "Integer8") == 0)
    *type = gsdml_eValueDataType_Integer8;
  else if ( strcmp( str, "Integer16") == 0)
    *type = gsdml_eValueDataType_Integer16;
  else if ( strcmp( str, "Integer32") == 0)
    *type = gsdml_eValueDataType_Integer32;
  else if ( strcmp( str, "Integer64") == 0)
    *type = gsdml_eValueDataType_Integer64;
  else if ( strcmp( str, "Unsigned8") == 0)
    *type = gsdml_eValueDataType_Unsigned8;
  else if ( strcmp( str, "Unsigned16") == 0)
    *type = gsdml_eValueDataType_Unsigned16;
  else if ( strcmp( str, "Unsigned32") == 0)
    *type = gsdml_eValueDataType_Unsigned32;
  else if ( strcmp( str, "Unsigned64") == 0)
    *type = gsdml_eValueDataType_Unsigned64;
  else if ( strcmp( str, "Float32") == 0)
    *type = gsdml_eValueDataType_Float32;
  else if ( strcmp( str, "Float64") == 0)
    *type = gsdml_eValueDataType_Float64;
  else if ( strcmp( str, "Date") == 0)
    *type = gsdml_eValueDataType_Date;
  else if ( strcmp( str, "TimeOfDay with date indication") == 0)
    *type = gsdml_eValueDataType_TimeOfDayWithDate;
  else if ( strcmp( str, "TimeOfDay without date indication") == 0)
    *type = gsdml_eValueDataType_TimeOfDayWithoutDate;
  else if ( strcmp( str, "TimeDifference with date indication") == 0)
    *type = gsdml_eValueDataType_TimeDiffWithDate;
  else if ( strcmp( str, "TimeDifference without date indication") == 0)
    *type = gsdml_eValueDataType_TimeDiffWithoutDate;
  else if ( strcmp( str, "NetworkTime") == 0)
    *type = gsdml_eValueDataType_NetworkTime;
  else if ( strcmp( str, "NetworkTimeDifference") == 0)
    *type = gsdml_eValueDataType_NetworkTimeDiff;
  else if ( strcmp( str, "VisibleString") == 0)
    *type = gsdml_eValueDataType_VisibleString;
  else if ( strcmp( str, "OctetString") == 0)
    *type = gsdml_eValueDataType_OctetString;
  else {
    *type = gsdml_eValueDataType_;
    return 0;
  }
  return 1;
}

int pn_gsdml::datavalue_to_string( gsdml_eValueDataType datatype, void *value,
				   unsigned int size, char *str, unsigned int strsize)
{
  switch ( datatype) {
  case gsdml_eValueDataType_Integer8:
    snprintf( str, strsize, "%hhd", *(char *)value);
    break;
  case gsdml_eValueDataType_Unsigned8:
    snprintf( str, strsize, "%hhu", *(unsigned char *)value);
    break;
  case gsdml_eValueDataType_Integer16: {
    short v;
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( &v, value, sizeof(v));
    else {
      unsigned char b[2];
      b[1] = *(unsigned char *)value;
      b[0] = *(((unsigned char *)value)+1);
      memcpy( &v, b, sizeof(v));
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( &v, value, sizeof(v));
    else {
      unsigned char b[2];
      b[1] = *(unsigned char *)value;
      b[0] = *(((unsigned char *)value)+1);
      memcpy( &v, b, sizeof(v));
    }
#endif
    snprintf( str, strsize, "%hd", v);
    break;
  }
  case gsdml_eValueDataType_Unsigned16: {
    unsigned short v;
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( &v, value, sizeof(v));
    else {
      unsigned char b[2];
      b[1] = *(unsigned char *)value;
      b[0] = *(((unsigned char *)value)+1);
      memcpy( &v, b, sizeof(v));
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( &v, value, sizeof(v));
    else {
      unsigned char b[2];
      b[1] = *(unsigned char *)value;
      b[0] = *(((unsigned char *)value)+1);
      memcpy( &v, b, sizeof(v));
    }
#endif
    snprintf( str, strsize, "%hu", v);
    break;
  }
  case gsdml_eValueDataType_Integer32: {
    int v;
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( &v, value, sizeof(v));
    else {
      unsigned char b[4];
      b[3] = *(unsigned char *)value;
      b[2] = *(((unsigned char *)value)+1);
      b[1] = *(((unsigned char *)value)+2);
      b[0] = *(((unsigned char *)value)+3);
      memcpy( &v, b, sizeof(v));
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( &v, &data[item[i].ref->Reference_Offset], sizeof(v));
    else {
      unsigned char b[4];
      b[3] = *(unsigned char *)value;
      b[2] = *(((unsigned char *)value)+1);
      b[1] = *(((unsigned char *)value)+2);
      b[0] = *(((unsigned char *)value)+3);
      memcpy( &v, b, sizeof(v));
    }
#endif
    snprintf( str, strsize, "%d", v);
    break;
  }
  case gsdml_eValueDataType_Unsigned32: {
    unsigned int v;
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( &v, value, sizeof(v));
    else {
      unsigned char b[4];
      b[3] = *(unsigned char *)value;
      b[2] = *(((unsigned char *)value)+1);
      b[1] = *(((unsigned char *)value)+2);
      b[0] = *(((unsigned char *)value)+3);
      memcpy( &v, b, sizeof(v));
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( &v, &data[item[i].ref->Reference_Offset], sizeof(v));
    else {
      unsigned char b[4];
      b[3] = *(unsigned char *)value;
      b[2] = *(((unsigned char *)value)+1);
      b[1] = *(((unsigned char *)value)+2);
      b[0] = *(((unsigned char *)value)+3);
      memcpy( &v, b, sizeof(v));
    }
#endif
    snprintf( str, strsize, "%u", v);
    break;
  }
  case gsdml_eValueDataType_Float32:
    snprintf( str, strsize, "%g", *(float *)value);
    break;
  case gsdml_eValueDataType_Integer64: {
    long long v;
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( &v, value, sizeof(v));
    else {
      unsigned char b[8];
      b[7] = *(unsigned char *)value;
      b[6] = *(((unsigned char *)value)+1);
      b[5] = *(((unsigned char *)value)+2);
      b[4] = *(((unsigned char *)value)+3);
      b[3] = *(((unsigned char *)value)+4);
      b[2] = *(((unsigned char *)value)+5);
      b[1] = *(((unsigned char *)value)+6);
      b[0] = *(((unsigned char *)value)+7);
      memcpy( &v, b, sizeof(v));
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( &v, &data[item[i].ref->Reference_Offset], sizeof(v));
    else {
      unsigned char b[8];
      b[7] = *(unsigned char *)value;
      b[6] = *(((unsigned char *)value)+1);
      b[5] = *(((unsigned char *)value)+2);
      b[4] = *(((unsigned char *)value)+3);
      b[3] = *(((unsigned char *)value)+4);
      b[2] = *(((unsigned char *)value)+5);
      b[1] = *(((unsigned char *)value)+6);
      b[0] = *(((unsigned char *)value)+7);
      memcpy( &v, b, sizeof(v));
    }
#endif
    snprintf( str, strsize, "%lld", v);
    break;
  }
  case gsdml_eValueDataType_Unsigned64: {
    unsigned long long v;
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( &v, value, sizeof(v));
    else {
      unsigned char b[8];
      b[7] = *(unsigned char *)value;
      b[6] = *(((unsigned char *)value)+1);
      b[5] = *(((unsigned char *)value)+2);
      b[4] = *(((unsigned char *)value)+3);
      b[3] = *(((unsigned char *)value)+4);
      b[2] = *(((unsigned char *)value)+5);
      b[1] = *(((unsigned char *)value)+6);
      b[0] = *(((unsigned char *)value)+7);
      memcpy( &v, b, sizeof(v));
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( &v, &data[item[i].ref->Reference_Offset], sizeof(v));
    else {
      unsigned char b[8];
      b[7] = *(unsigned char *)value;
      b[6] = *(((unsigned char *)value)+1);
      b[5] = *(((unsigned char *)value)+2);
      b[4] = *(((unsigned char *)value)+3);
      b[3] = *(((unsigned char *)value)+4);
      b[2] = *(((unsigned char *)value)+5);
      b[1] = *(((unsigned char *)value)+6);
      b[0] = *(((unsigned char *)value)+7);
      memcpy( &v, b, sizeof(v));
    }
#endif
    snprintf( str, strsize, "%llu", v);
    break;
  }
  case gsdml_eValueDataType_Float64:
    snprintf( str, strsize, "%lg", *(double *)value);
    break;
  case gsdml_eValueDataType_VisibleString:
    strncpy( str, (char *)value, strsize);
    break;
  case gsdml_eValueDataType_OctetString: {
    unsigned int len = 0;
    for ( unsigned int i = 0; i < size; i++) {
      if ( i == size - 1) {
	if ( len + 4 >= strsize)
	  break;
	len += sprintf( &str[i*5], "0x%02hhx", *(((unsigned char *)value)+i));
      }
      else {
	if ( len + 5 >= strsize)
	  break;
	len += sprintf( &str[i*5], "0x%02hhx,", *(((unsigned char *)value)+i));		 
      }
    }
    break;
  }
  case gsdml_eValueDataType_Date:
  case gsdml_eValueDataType_TimeOfDayWithDate:
  case gsdml_eValueDataType_TimeOfDayWithoutDate:
  case gsdml_eValueDataType_TimeDiffWithDate:
  case gsdml_eValueDataType_TimeDiffWithoutDate:
  case gsdml_eValueDataType_NetworkTime:
  case gsdml_eValueDataType_NetworkTimeDiff:
    sprintf( str, "Not implemented data type");
    return 0;
  default:
    return 0;
  }
  return 1;
}

int pn_gsdml::string_to_datavalue( gsdml_eValueDataType datatype, void *value,
				   unsigned int size, char *str)
{
  switch ( datatype) {
  case gsdml_eValueDataType_Integer8:
    if ( sscanf( str, "%hhd", (char *)value) != 1)
      return PB__SYNTAX;
    break;
  case gsdml_eValueDataType_Unsigned8:
    if ( sscanf( str, "%hhu", (unsigned char *)value) != 1)
      return PB__SYNTAX;
    break;
  case gsdml_eValueDataType_Integer16: {
    short v;
    if ( sscanf( str, "%hd", &v) != 1)
      return PB__SYNTAX;

#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[2];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[1];
      *(((unsigned char *)value)+1) = b[0];
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[2];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[1];
      *(((unsigned char *)value)+1) = b[0];
    }
#endif
    break;
  }
  case gsdml_eValueDataType_Unsigned16: {
    unsigned short v;
    if ( sscanf( str, "%hu", &v) != 1)
      return PB__SYNTAX;

#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[2];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[1];
      *(((unsigned char *)value)+1) = b[0];
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[2];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[1];
      *(((unsigned char *)value)+1) = b[0];
    }
#endif
    break;
  }
  case gsdml_eValueDataType_Integer32: {
    int v;
    if ( sscanf( str, "%d", &v) != 1)
      return PB__SYNTAX;

#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[4];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[3];
      *(((unsigned char *)value)+1) = b[2];
      *(((unsigned char *)value)+2) = b[1];
      *(((unsigned char *)value)+3) = b[0];
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[4];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[3];
      *(((unsigned char *)value)+1) = b[2];
      *(((unsigned char *)value)+2) = b[1];
      *(((unsigned char *)value)+3) = b[0];
    }
#endif
    break;
  }
  case gsdml_eValueDataType_Unsigned32: {
    unsigned int v;
    if ( sscanf( str, "%u", &v) != 1)
      return PB__SYNTAX;

#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[4];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[3];
      *(((unsigned char *)value)+1) = b[2];
      *(((unsigned char *)value)+2) = b[1];
      *(((unsigned char *)value)+3) = b[0];
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[4];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[3];
      *(((unsigned char *)value)+1) = b[2];
      *(((unsigned char *)value)+2) = b[1];
      *(((unsigned char *)value)+3) = b[0];
    }
#endif
    break;
  }
  case gsdml_eValueDataType_Float32:
    if ( sscanf( str, "%g", (float *)value) != 1)
      return PB__SYNTAX;
    break;
  case gsdml_eValueDataType_Integer64: {
    long long v;
    if ( sscanf( str, "%lld", &v) != 1)
      return PB__SYNTAX;

#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[8];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[7];
      *(((unsigned char *)value)+1) = b[6];
      *(((unsigned char *)value)+2) = b[5];
      *(((unsigned char *)value)+3) = b[4];
      *(((unsigned char *)value)+4) = b[3];
      *(((unsigned char *)value)+5) = b[2];
      *(((unsigned char *)value)+6) = b[1];
      *(((unsigned char *)value)+7) = b[0];
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[8];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[7];
      *(((unsigned char *)value)+1) = b[6];
      *(((unsigned char *)value)+2) = b[5];
      *(((unsigned char *)value)+3) = b[4];
      *(((unsigned char *)value)+4) = b[3];
      *(((unsigned char *)value)+5) = b[2];
      *(((unsigned char *)value)+6) = b[1];
      *(((unsigned char *)value)+7) = b[0];
    }
#endif
    break;
  }
  case gsdml_eValueDataType_Unsigned64: {
    unsigned long long v;
    if ( sscanf( str, "%llu", &v) != 1)
      return PB__SYNTAX;

#if (pwr_dHost_byteOrder == pwr_dLittleEndian)
    if ( byte_order == pwr_eByteOrderingEnum_LittleEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[8];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[7];
      *(((unsigned char *)value)+1) = b[6];
      *(((unsigned char *)value)+2) = b[5];
      *(((unsigned char *)value)+3) = b[4];
      *(((unsigned char *)value)+4) = b[3];
      *(((unsigned char *)value)+5) = b[2];
      *(((unsigned char *)value)+6) = b[1];
      *(((unsigned char *)value)+7) = b[0];
    }
#elif (pwr_dHost_byteOrder == pwr_dBigEndian)
    if ( byte_order == pwr_eByteOrderingEnum_BigEndian)
      memcpy( value, &v, sizeof(v));
    else {
      unsigned char b[8];
      memcpy( b, &v, sizeof(b));
      *(unsigned char *)value = b[7];
      *(((unsigned char *)value)+1) = b[6];
      *(((unsigned char *)value)+2) = b[5];
      *(((unsigned char *)value)+3) = b[4];
      *(((unsigned char *)value)+4) = b[3];
      *(((unsigned char *)value)+5) = b[2];
      *(((unsigned char *)value)+6) = b[1];
      *(((unsigned char *)value)+7) = b[0];
    }
#endif
    break;
  }
  case gsdml_eValueDataType_Float64:
    if ( sscanf( str, "%lg", (double *)value) != 1)
      return PB__SYNTAX;
    break;
  case gsdml_eValueDataType_VisibleString:
    strncpy( (char *)value, str, size);
    break;
  case gsdml_eValueDataType_OctetString: {
    unsigned int len;
    for ( unsigned int i = 0; i < size; i++) {
      len = sscanf( &str[i*5], "0x%2hhx", (unsigned char *)((unsigned char *)value)[i]);
      if ( len != 1)
	break;
    }
    break;
  }
  case gsdml_eValueDataType_Date:
  case gsdml_eValueDataType_TimeOfDayWithDate:
  case gsdml_eValueDataType_TimeOfDayWithoutDate:
  case gsdml_eValueDataType_TimeDiffWithDate:
  case gsdml_eValueDataType_TimeDiffWithoutDate:
  case gsdml_eValueDataType_NetworkTime:
  case gsdml_eValueDataType_NetworkTimeDiff:
    return PB__NYI;
  default:
    return PB__NYI;
  }
  return 1;
}

int pn_gsdml::set_default_values( gsdml_eTag id, void *data, unsigned int size)
{
  for ( unsigned int i = 0; i < sizeof(attrlist)/sizeof(attrlist[0]); i++) {
    if ( attrlist[i].id == id && strcmp( attrlist[i].default_value, "") != 0) {
      if ( attrlist[i].offset + attrlist[i].size > size)
	continue;
	
      string_to_value( attrlist[i].type, attrlist[i].size, attrlist[i].default_value, (char *)data + attrlist[i].offset);
    }
  }
  return 1;
}

gsdml_eTag pn_gsdml::get_tag_stack() 
{ 
  if ( tag_stack_cnt) 
    return tag_stack[tag_stack_cnt-1];
  else 
    return gsdml_eTag_;
}

gsdml_eTag pn_gsdml::get_tag_stack( int p) 
{ 
  if ( p <= (int)tag_stack_cnt - 1) 
    return tag_stack[tag_stack_cnt- 1 - p];
  else 
    return gsdml_eTag_;
}

void *pn_gsdml::get_object_stack( gsdml_eTag id) 
{ 
  if ( object_stack_cnt) {
    if ( object_stack_id[object_stack_cnt-1] == id)
      return object_stack[object_stack_cnt-1];
  }
  return 0;
}

void *pn_gsdml::get_object_stack( int p, gsdml_eTag id) 
{ 
  if ( p <= (int)object_stack_cnt - 1) {
    if ( object_stack_id[object_stack_cnt-1-p] == id)
      return object_stack[object_stack_cnt- 1 - p];
  }
  return 0;
}

void *pn_gsdml::object_factory( gsdml_eTag id)
{
  void *ro = 0;

  switch ( id) {
  case gsdml_eTag_xml: 
    xml = new gsdml_Xml( this);
    current_body = &xml->Body;
    current_body_size = sizeof( gsdml_sXml);
    ro = xml;
    break;
  case gsdml_eTag_ProfileHeader: 
    ProfileHeader = new gsdml_ProfileHeader( this);
    current_body = &ProfileHeader->Body;
    current_body_size = sizeof( gsdml_sProfileHeader);
    ro = ProfileHeader;
    break;
  case gsdml_eTag_DeviceIdentity: 
    DeviceIdentity = new gsdml_DeviceIdentity( this);
    current_body = &DeviceIdentity->Body;
    current_body_size = sizeof( gsdml_sDeviceIdentity);
    ro = DeviceIdentity;
    break;
  case gsdml_eTag_DeviceFunction: 
    DeviceFunction = new gsdml_DeviceFunction( this);
    current_body = &DeviceFunction->Body;
    current_body_size = sizeof( gsdml_sDeviceFunction);
    ro = DeviceFunction;
    break;
  case gsdml_eTag_ApplicationProcess: 
    ApplicationProcess = new gsdml_ApplicationProcess( this);
    ro = ApplicationProcess;
    break;
  case gsdml_eTag_DeviceAccessPointList: {
    gsdml_DeviceAccessPointList *o = new gsdml_DeviceAccessPointList( this);
    gsdml_ApplicationProcess *p = (gsdml_ApplicationProcess *) get_object_stack( gsdml_eTag_ApplicationProcess);
    if ( !p) {
      error_message_line( "Misplaced DeviceAccessPointList");
      return 0;
    }
    p->DeviceAccessPointList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_DeviceAccessPointItem: {
    gsdml_DeviceAccessPointItem *o = new gsdml_DeviceAccessPointItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sDeviceAccessPointItem);
    set_default_values( gsdml_eTag_DeviceAccessPointItem, current_body, current_body_size);

    gsdml_DeviceAccessPointList *p = (gsdml_DeviceAccessPointList *)get_object_stack( gsdml_eTag_DeviceAccessPointList);
    if ( !p) {
      error_message_line( "Misplaced DeviceAccessPointItem");
      return 0;
    }

    p->DeviceAccessPointItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_ModuleList: {
    gsdml_ModuleList *o = new gsdml_ModuleList( this);
    gsdml_ApplicationProcess *p = (gsdml_ApplicationProcess *) get_object_stack( gsdml_eTag_ApplicationProcess);
    if ( !p) {
      error_message_line( "Misplaced ModuleList");
      return 0;
    }
    p->ModuleList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_ModuleItem: {
    gsdml_ModuleItem *o = new gsdml_ModuleItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sModuleItem);
    set_default_values( gsdml_eTag_ModuleItem, current_body, current_body_size);

    gsdml_ModuleList *p = (gsdml_ModuleList *)get_object_stack( gsdml_eTag_ModuleList);
    if ( !p) {
      error_message_line( "Misplaced ModuleItem");
      return 0;
    }

    p->ModuleItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_ModuleInfo: {
    gsdml_ModuleInfo *o = new gsdml_ModuleInfo( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sModuleInfo);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_DeviceAccessPointItem)))
      ((gsdml_DeviceAccessPointItem *)p)->ModuleInfo = o;
    else if ( (p = get_object_stack( gsdml_eTag_VirtualSubmoduleItem)))
      ((gsdml_VirtualSubmoduleItem *)p)->ModuleInfo = o;
    else if ( (p = get_object_stack( gsdml_eTag_ModuleItem)))
      ((gsdml_ModuleItem *)p)->ModuleInfo = o;
    else {
      error_message_line( "Misplaced ModuleInfo");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_SubslotList: {
    gsdml_SubslotList *o = new gsdml_SubslotList( this);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_DeviceAccessPointItem)))
      ((gsdml_DeviceAccessPointItem *)p)->SubslotList = o;
    else if ( (p = get_object_stack( gsdml_eTag_ModuleItem)))
      ((gsdml_ModuleItem *)p)->SubslotList = o;
    else {
      error_message_line( "Misplaced SubslotList");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_SubslotItem: {
    gsdml_SubslotItem *o = new gsdml_SubslotItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sSubslotItem);

    gsdml_SubslotList *p = (gsdml_SubslotList *)get_object_stack( gsdml_eTag_SubslotList);
    if ( !p) {
      error_message_line( "Misplaced SubslotItem");
      return 0;
    }
    p->SubslotItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_IOConfigData: {
    gsdml_IOConfigData *o = new gsdml_IOConfigData( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sIOConfigData);

    gsdml_DeviceAccessPointItem *p = (gsdml_DeviceAccessPointItem *)get_object_stack( gsdml_eTag_DeviceAccessPointItem);
    if ( !p) {
      error_message_line( "Misplaced IOConfigData");
      return 0;
    }

    p->IOConfigData = o;
    ro = o;
    break;
  }
  case gsdml_eTag_UseableModules: {
    gsdml_UseableModules *o = new gsdml_UseableModules( this);

    gsdml_DeviceAccessPointItem *p = (gsdml_DeviceAccessPointItem *)get_object_stack( gsdml_eTag_DeviceAccessPointItem);
    if ( !p) {
      error_message_line( "Misplaced IOConfigData");
      return 0;
    }

    p->UseableModules = o;
    ro = o;
    break;
  }
  case gsdml_eTag_ModuleItemRef: {
    gsdml_ModuleItemRef *o = new gsdml_ModuleItemRef( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sModuleItemRef);

    gsdml_UseableModules *p = (gsdml_UseableModules *)get_object_stack( gsdml_eTag_UseableModules);
    if ( !p) {
      error_message_line( "Misplaced ModuleItemRef");
      return 0;
    }
    p->ModuleItemRef.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_VirtualSubmoduleList: {
    gsdml_VirtualSubmoduleList *o = new gsdml_VirtualSubmoduleList( this);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_DeviceAccessPointItem)))
      ((gsdml_DeviceAccessPointItem *)p)->VirtualSubmoduleList = o;
    else if ( (p = get_object_stack( gsdml_eTag_ModuleItem)))
      ((gsdml_ModuleItem *)p)->VirtualSubmoduleList = o;
    else {
      error_message_line( "Misplaced VirtualSubmoduleList");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_VirtualSubmoduleItem: {
    gsdml_VirtualSubmoduleItem *o = new gsdml_VirtualSubmoduleItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sVirtualSubmoduleItem);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_VirtualSubmoduleList)))
      ((gsdml_VirtualSubmoduleList *)p)->VirtualSubmoduleItem.push_back( o);
    else if ( (p = get_object_stack( gsdml_eTag_SubmoduleList)))
      ((gsdml_SubmoduleList *)p)->SubmoduleItem.push_back( o);
    else {
      error_message_line( "Misplaced VirtualSubmoduleItem");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_IOData: {
    gsdml_IOData *o = new gsdml_IOData( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sIOData);

    gsdml_VirtualSubmoduleItem *p = (gsdml_VirtualSubmoduleItem *)get_object_stack( gsdml_eTag_VirtualSubmoduleItem);
    if ( !p) {
      error_message_line( "Misplaced IOData");
      return 0;
    }

    p->IOData = o;
    ro = o;
    break;
  }
  case gsdml_eTag_Input: {
    gsdml_Input *o = new gsdml_Input( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sInput);

    gsdml_IOData *p = (gsdml_IOData *)get_object_stack( gsdml_eTag_IOData);
    if ( !p) {
      error_message_line( "Misplaced Input");
      return 0;
    }

    p->Input = o;
    ro = o;
    break;
  }
  case gsdml_eTag_Output: {
    gsdml_Output *o = new gsdml_Output( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sOutput);

    gsdml_IOData *p = (gsdml_IOData *)get_object_stack( gsdml_eTag_IOData);
    if ( !p) {
      error_message_line( "Misplaced Output");
      return 0;
    }

    p->Output = o;
    ro = o;
    break;
  }
  case gsdml_eTag_DataItem: {
    gsdml_eTag id = get_tag_stack( 1);
    if ( id == gsdml_eTag_Input || id == gsdml_eTag_Output) {

      gsdml_DataItem *o = new gsdml_DataItem( this);
      current_body = &o->Body;
      current_body_size = sizeof( gsdml_sDataItem);
      
      gsdml_Input *p = (gsdml_Input *)get_object_stack( gsdml_eTag_Input);
      if ( p)
	p->DataItem.push_back( o);
      else {
	gsdml_Output *p = (gsdml_Output *)get_object_stack( gsdml_eTag_Output);
	if ( p)
	  p->DataItem.push_back( o);
	else {
	  error_message_line( "Misplaced DataItem");
	  return 0;
	}
      }
      ro = o;
    }
    else if ( id == gsdml_eTag_ExtChannelAddValue) {

      gsdml_ExtChannelAddValue_DataItem *o = new gsdml_ExtChannelAddValue_DataItem( this);
      current_body = &o->Body;
      current_body_size = sizeof( gsdml_sExtChannelAddValue_DataItem);
      
      gsdml_ExtChannelAddValue *p = (gsdml_ExtChannelAddValue *)get_object_stack( gsdml_eTag_ExtChannelAddValue);
      if ( !p) {
	error_message_line( "Misplaced DataItem");
	return 0;
      }

      p->DataItem.push_back( o);

      ro = o;
    }
    break;
  }
  case gsdml_eTag_BitDataItem: {
    gsdml_BitDataItem *o = new gsdml_BitDataItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sBitDataItem);

    gsdml_DataItem *p = (gsdml_DataItem *)get_object_stack( gsdml_eTag_DataItem);
    if ( !p) {
      error_message_line( "Misplaced BitDataItem");
      return 0;
    }

    p->BitDataItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_Const: {
    gsdml_Const *o = new gsdml_Const( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sConst);

    gsdml_ParameterRecordDataItem *p = (gsdml_ParameterRecordDataItem *)get_object_stack( gsdml_eTag_ParameterRecordDataItem);
    if ( !p) {
      error_message_line( "Misplaced Const");
      return 0;
    }

    p->Const.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_Ref: {
    gsdml_Ref *o = new gsdml_Ref( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sRef);
    set_default_values( gsdml_eTag_Ref, current_body, current_body_size);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_ParameterRecordDataItem)))
      ((gsdml_ParameterRecordDataItem *)p)->Ref.push_back( o);
    else if ( (p = get_object_stack( gsdml_eTag_UnitDiagTypeItem)))
      ((gsdml_UnitDiagTypeItem *)p)->Ref.push_back( o);
    else {
      error_message_line( "Misplaced Ref");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_RecordDataList: {
    gsdml_RecordDataList *o = new gsdml_RecordDataList( this);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_VirtualSubmoduleItem)))
      ((gsdml_VirtualSubmoduleItem *)p)->RecordDataList = o;
    else if ( (p = get_object_stack( gsdml_eTag_InterfaceSubmoduleItem)))
      ((gsdml_InterfaceSubmoduleItem *)p)->RecordDataList = o;
    else {
      error_message_line( "Misplaced RecordDataList");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_ParameterRecordDataItem: {
    gsdml_ParameterRecordDataItem *o = new gsdml_ParameterRecordDataItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sParameterRecordDataItem);

    gsdml_RecordDataList *p = (gsdml_RecordDataList *)get_object_stack( gsdml_eTag_RecordDataList);
    if ( !p) {
      error_message_line( "Misplaced ParameterRecordItem");
      return 0;
    }

    p->ParameterRecordDataItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_F_ParameterRecordDataItem: {
    gsdml_F_ParameterRecordDataItem *o = new gsdml_F_ParameterRecordDataItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sF_ParameterRecordDataItem);

    gsdml_RecordDataList *p = (gsdml_RecordDataList *)get_object_stack( gsdml_eTag_RecordDataList);
    if ( !p) {
      error_message_line( "Misplaced F_ParameterRecordItem");
      return 0;
    }

    p->F_ParameterRecordDataItem = o;
    ro = o;
    break;
  }
  case gsdml_eTag_Graphics: {
    gsdml_Graphics *o = new gsdml_Graphics( this);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_VirtualSubmoduleItem)))
      ((gsdml_VirtualSubmoduleItem *)p)->Graphics = o;
    else if ( (p = get_object_stack( gsdml_eTag_DeviceAccessPointItem)))
      ((gsdml_DeviceAccessPointItem *)p)->Graphics = o;
    else if ( (p = get_object_stack( gsdml_eTag_ModuleItem)))
      ((gsdml_ModuleItem *)p)->Graphics = o;
    else {
      error_message_line( "Misplaced Graphics");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_GraphicItemRef: {
    gsdml_GraphicItemRef *o = new gsdml_GraphicItemRef( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sGraphicItemRef);

    gsdml_Graphics *p = (gsdml_Graphics *)get_object_stack( gsdml_eTag_Graphics);
    if ( !p) {
      error_message_line( "Misplaced GraphicItemRef");
      return 0;
    }

    p->GraphicItemRef.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_IsochroneMode: {
    gsdml_IsochroneMode *o = new gsdml_IsochroneMode( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sIsochroneMode);

    gsdml_VirtualSubmoduleItem *p = (gsdml_VirtualSubmoduleItem *)get_object_stack( gsdml_eTag_VirtualSubmoduleItem);
    if ( !p) {
      error_message_line( "Misplaced IsochroneMode");
      return 0;
    }

    p->IsochroneMode = o;
    ro = o;
    break;
  }
  case gsdml_eTag_SystemDefinedSubmoduleList: {
    gsdml_SystemDefinedSubmoduleList *o = new gsdml_SystemDefinedSubmoduleList( this);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_DeviceAccessPointItem)))
      ((gsdml_DeviceAccessPointItem *)p)->SystemDefinedSubmoduleList = o;
    else if ( (p = get_object_stack( gsdml_eTag_ModuleItem)))
      ((gsdml_ModuleItem *)p)->SystemDefinedSubmoduleList = o;
    else {
      error_message_line( "Misplaced SystemDefinedSubmoduleList");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_InterfaceSubmoduleItem: {
    gsdml_InterfaceSubmoduleItem *o = new gsdml_InterfaceSubmoduleItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sInterfaceSubmoduleItem);
    set_default_values( gsdml_eTag_InterfaceSubmoduleItem, current_body, current_body_size);

    gsdml_SystemDefinedSubmoduleList *p = (gsdml_SystemDefinedSubmoduleList *)get_object_stack( gsdml_eTag_SystemDefinedSubmoduleList);
    if ( !p) {
      error_message_line( "Misplaced InterfaceSubmoduleItem");
      return 0;
    }

    p->InterfaceSubmoduleItem = o;
    ro = o;
    break;
  }
  case gsdml_eTag_General: {
    gsdml_General *o = new gsdml_General( this);

    gsdml_InterfaceSubmoduleItem *p = (gsdml_InterfaceSubmoduleItem *)get_object_stack( gsdml_eTag_InterfaceSubmoduleItem);
    if ( !p) {
      error_message_line( "Misplaced General");
      return 0;
    }

    p->General = o;
    ro = o;
    break;
  }
  case gsdml_eTag_DCP_FlashOnceSignalUnit: {
    gsdml_DCP_FlashOnceSignalUnit *o = new gsdml_DCP_FlashOnceSignalUnit( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sDCP_FlashOnceSignalUnit);

    gsdml_General *p = (gsdml_General *)get_object_stack( gsdml_eTag_General);
    if ( !p) {
      error_message_line( "Misplaced DCP_FlashOnceSignalUnit");
      return 0;
    }

    p->DCP_FlashOnceSignalUnit = o;
    ro = o;
    break;
  }
  case gsdml_eTag_RT_Class3Properties: {
    gsdml_RT_Class3Properties *o = new gsdml_RT_Class3Properties( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sRT_Class3Properties);

    gsdml_InterfaceSubmoduleItem *p = (gsdml_InterfaceSubmoduleItem *)get_object_stack( gsdml_eTag_InterfaceSubmoduleItem);
    if ( !p) {
      error_message_line( "Misplaced RT_Class3Properties");
      return 0;
    }

    p->RT_Class3Properties = o;
    ro = o;
    break;
  }
  case gsdml_eTag_SynchronisationMode: {
    gsdml_SynchronisationMode *o = new gsdml_SynchronisationMode( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sSynchronisationMode);

    gsdml_InterfaceSubmoduleItem *p = (gsdml_InterfaceSubmoduleItem *)get_object_stack( gsdml_eTag_InterfaceSubmoduleItem);
    if ( !p) {
      error_message_line( "Misplaced SynchronisationMode");
      return 0;
    }

    p->SynchronisationMode = o;
    ro = o;
    break;
  }
  case gsdml_eTag_ApplicationRelations: {
    gsdml_eTag id = get_tag_stack( 1);
    if ( id == gsdml_eTag_InterfaceSubmoduleItem) {
      gsdml_InterfaceSubmoduleItem_ApplicationRelations *o = new gsdml_InterfaceSubmoduleItem_ApplicationRelations( this);
      current_body = &o->Body;
      current_body_size = sizeof( gsdml_sInterfaceSubmoduleItem_ApplicationRelations);
      
      gsdml_InterfaceSubmoduleItem *p = (gsdml_InterfaceSubmoduleItem *)get_object_stack( gsdml_eTag_InterfaceSubmoduleItem);
      if ( !p) {
	error_message_line( "Misplaced ApplicationRelations");
	return 0;
      }

      p->ApplicationRelations = o;
      ro = o;
    }
    else if ( id == gsdml_eTag_DeviceAccessPointItem) {
      gsdml_DeviceAccessPointItem_ApplicationRelations *o = new gsdml_DeviceAccessPointItem_ApplicationRelations( this);
      current_body = &o->Body;
      current_body_size = sizeof( gsdml_sDeviceAccessPointItem_ApplicationRelations);
      
      gsdml_DeviceAccessPointItem *p = (gsdml_DeviceAccessPointItem *)get_object_stack( gsdml_eTag_DeviceAccessPointItem);
      if ( !p) {
	error_message_line( "Misplaced ApplicationRelations");
	return 0;
      }

      p->ApplicationRelations = o;
      ro = o;
    }
    break;
  }
  case gsdml_eTag_MediaRedundancy: {
    gsdml_MediaRedundancy *o = new gsdml_MediaRedundancy( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sMediaRedundancy);

    gsdml_InterfaceSubmoduleItem *p = (gsdml_InterfaceSubmoduleItem *)get_object_stack( gsdml_eTag_InterfaceSubmoduleItem);
    if ( !p) {
      error_message_line( "Misplaced MediaRedundancy");
      return 0;
    }

    p->MediaRedundancy = o;
    ro = o;
    break;
  }
  case gsdml_eTag_TimingProperties: {
    gsdml_TimingProperties *o = new gsdml_TimingProperties( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sTimingProperties);

    gsdml_eTag id = get_tag_stack( 2);
    if ( id == gsdml_eTag_InterfaceSubmoduleItem) {
      gsdml_InterfaceSubmoduleItem_ApplicationRelations *p = (gsdml_InterfaceSubmoduleItem_ApplicationRelations *)get_object_stack( gsdml_eTag_ApplicationRelations);
      if ( !p) {
	error_message_line( "Misplaced TimingProperties");
	return 0;
      }

      p->TimingProperties = o;
    }
    else if ( id == gsdml_eTag_InterfaceSubmoduleItem) {
      gsdml_DeviceAccessPointItem_ApplicationRelations *p = (gsdml_DeviceAccessPointItem_ApplicationRelations *)get_object_stack( gsdml_eTag_ApplicationRelations);
      if ( !p) {
	error_message_line( "Misplaced TimingProperties");
	return 0;
      }

      p->TimingProperties = o;
    }
    ro = o;
    break;
  }
  case gsdml_eTag_RT_Class3TimingProperties: {
    gsdml_RT_Class3TimingProperties *o = new gsdml_RT_Class3TimingProperties( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sRT_Class3TimingProperties);

    gsdml_InterfaceSubmoduleItem_ApplicationRelations *p = (gsdml_InterfaceSubmoduleItem_ApplicationRelations *)get_object_stack( gsdml_eTag_ApplicationRelations);
    if ( !p) {
      error_message_line( "Misplaced RT_Class3TimingProperties");
      return 0;
    }
    
    p->RT_Class3TimingProperties = o;
    ro = o;
    break;
  }
  case gsdml_eTag_PortSubmoduleItem: {
    gsdml_PortSubmoduleItem *o = new gsdml_PortSubmoduleItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sPortSubmoduleItem);
    set_default_values( gsdml_eTag_PortSubmoduleItem, current_body, current_body_size);

    gsdml_SystemDefinedSubmoduleList *p = (gsdml_SystemDefinedSubmoduleList *)get_object_stack( gsdml_eTag_SystemDefinedSubmoduleList);
    if ( !p) {
      error_message_line( "Misplaced PortSubmoduleItem");
      return 0;
    }

    p->PortSubmoduleItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_UseableSubmodules: {
    gsdml_UseableSubmodules *o = new gsdml_UseableSubmodules( this);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_DeviceAccessPointItem)))
      ((gsdml_DeviceAccessPointItem *)p)->UseableSubmodules = o;
    else if ( (p = get_object_stack( gsdml_eTag_ModuleItem)))
      ((gsdml_ModuleItem *)p)->UseableSubmodules = o;
    else {
      error_message_line( "Misplaced UseableSubmodules");
      return 0;
    }

    ro = o;
    break;
  }
  case gsdml_eTag_SubmoduleItemRef: {
    gsdml_SubmoduleItemRef *o = new gsdml_SubmoduleItemRef( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sSubmoduleItemRef);

    gsdml_UseableSubmodules *p = (gsdml_UseableSubmodules *)get_object_stack( gsdml_eTag_UseableSubmodules);
    if ( !p) {
      error_message_line( "Misplaced SubmoduleItemRef");
      return 0;
    }

    p->SubmoduleItemRef.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_SlotList: {
    gsdml_SlotList *o = new gsdml_SlotList( this);

    gsdml_DeviceAccessPointItem *p = (gsdml_DeviceAccessPointItem *)get_object_stack( gsdml_eTag_DeviceAccessPointItem);
    if ( !p) {
      error_message_line( "Misplaced SlotList");
      return 0;
    }

    p->SlotList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_SlotItem: {
    gsdml_SlotItem *o = new gsdml_SlotItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sSlotItem);

    gsdml_SlotList *p = (gsdml_SlotList *)get_object_stack( gsdml_eTag_SlotList);
    if ( !p) {
      error_message_line( "Misplaced SlotItem");
      return 0;
    }

    p->SlotItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_SlotGroups: {
    gsdml_SlotGroups *o = new gsdml_SlotGroups( this);

    gsdml_DeviceAccessPointItem *p = (gsdml_DeviceAccessPointItem *)get_object_stack( gsdml_eTag_DeviceAccessPointItem);
    if ( !p) {
      error_message_line( "Misplaced SlotGroups");
      return 0;
    }

    p->SlotGroups = o;
    ro = o;
    break;
  }
  case gsdml_eTag_SlotGroup: {
    gsdml_SlotGroup *o = new gsdml_SlotGroup( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sSlotGroup);

    gsdml_SlotGroups *p = (gsdml_SlotGroups *)get_object_stack( gsdml_eTag_SlotGroups);
    if ( !p) {
      error_message_line( "Misplaced SlotGroup");
      return 0;
    }

    p->SlotGroup.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_ValueList: {
    gsdml_ValueList *o = new gsdml_ValueList( this);

    gsdml_ApplicationProcess *p = (gsdml_ApplicationProcess *)get_object_stack( gsdml_eTag_ApplicationProcess);
    if ( !p) {
      error_message_line( "Misplaced ValueList");
      return 0;
    }

    p->ValueList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_ValueItem: {
    gsdml_ValueItem *o = new gsdml_ValueItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sValueItem);

    gsdml_ValueList *p = (gsdml_ValueList *)get_object_stack( gsdml_eTag_ValueList);
    if ( !p) {
      error_message_line( "Misplaced ValueItem");
      return 0;
    }

    p->ValueItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_Assignments: {
    gsdml_Assignments *o = new gsdml_Assignments( this);

    gsdml_ValueItem *p = (gsdml_ValueItem *)get_object_stack( gsdml_eTag_ValueItem);
    if ( !p) {
      error_message_line( "Misplaced Assignments");
      return 0;
    }

    p->Assignments = o;
    ro = o;
    break;
  }
  case gsdml_eTag_Assign: {
    gsdml_Assign *o = new gsdml_Assign( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sAssign);

    gsdml_Assignments *p = (gsdml_Assignments *)get_object_stack( gsdml_eTag_Assignments);
    if ( !p) {
      error_message_line( "Misplaced Assign");
      return 0;
    }

    p->Assign.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_ChannelDiagList: {
    gsdml_ChannelDiagList *o = new gsdml_ChannelDiagList( this);

    gsdml_ApplicationProcess *p = (gsdml_ApplicationProcess *)get_object_stack( gsdml_eTag_ApplicationProcess);
    if ( !p) {
      error_message_line( "Misplaced ChannelDiagList");
      return 0;
    }

    p->ChannelDiagList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_ChannelDiagItem: {
    gsdml_ChannelDiagItem *o = new gsdml_ChannelDiagItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sChannelDiagItem);

    gsdml_ChannelDiagList *p = (gsdml_ChannelDiagList *)get_object_stack( gsdml_eTag_ChannelDiagList);
    if ( !p) {
      error_message_line( "Misplaced ChannelDiagItem");
      return 0;
    }

    p->ChannelDiagItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_ExtChannelDiagList: {
    gsdml_ExtChannelDiagList *o = new gsdml_ExtChannelDiagList( this);

    gsdml_ChannelDiagItem *p = (gsdml_ChannelDiagItem *)get_object_stack( gsdml_eTag_ChannelDiagItem);
    if ( !p) {
      error_message_line( "Misplaced ExtChannelDiagList");
      return 0;
    }

    p->ExtChannelDiagList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_ExtChannelDiagItem: {
    gsdml_ExtChannelDiagItem *o = new gsdml_ExtChannelDiagItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sExtChannelDiagItem);

    gsdml_ExtChannelDiagList *p = (gsdml_ExtChannelDiagList *)get_object_stack( gsdml_eTag_ExtChannelDiagList);
    if ( !p) {
      error_message_line( "Misplaced ExtChannelDiagItem");
      return 0;
    }

    p->ExtChannelDiagItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_ExtChannelAddValue: {
    gsdml_ExtChannelAddValue *o = new gsdml_ExtChannelAddValue( this);

    gsdml_ExtChannelDiagItem *p = (gsdml_ExtChannelDiagItem *)get_object_stack( gsdml_eTag_ExtChannelDiagItem);
    if ( !p) {
      error_message_line( "Misplaced ExtChannelAddValue");
      return 0;
    }

    p->ExtChannelAddValue.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_UnitDiagTypeList: {
    gsdml_UnitDiagTypeList *o = new gsdml_UnitDiagTypeList( this);

    gsdml_ApplicationProcess *p = (gsdml_ApplicationProcess *)get_object_stack( gsdml_eTag_ApplicationProcess);
    if ( !p) {
      error_message_line( "Misplaced UnitDiagTypeList");
      return 0;
    }

    p->UnitDiagTypeList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_UnitDiagTypeItem: {
    gsdml_UnitDiagTypeItem *o = new gsdml_UnitDiagTypeItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sUnitDiagTypeItem);

    gsdml_UnitDiagTypeList *p = (gsdml_UnitDiagTypeList *)get_object_stack( gsdml_eTag_UnitDiagTypeList);
    if ( !p) {
      error_message_line( "Misplaced UnitDiagTypeItem");
      return 0;
    }

    p->UnitDiagTypeItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_GraphicsList: {
    gsdml_GraphicsList *o = new gsdml_GraphicsList( this);

    gsdml_ApplicationProcess *p = (gsdml_ApplicationProcess *)get_object_stack( gsdml_eTag_ApplicationProcess);
    if ( !p) {
      error_message_line( "Misplaced GraphicsList");
      return 0;
    }

    p->GraphicsList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_GraphicItem: {
    gsdml_GraphicItem *o = new gsdml_GraphicItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sGraphicItem);

    gsdml_GraphicsList *p = (gsdml_GraphicsList *)get_object_stack( gsdml_eTag_GraphicsList);
    if ( !p) {
      error_message_line( "Misplaced GraphicItem");
      return 0;
    }

    p->GraphicItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_CategoryList: {
    gsdml_CategoryList *o = new gsdml_CategoryList( this);

    gsdml_ApplicationProcess *p = (gsdml_ApplicationProcess *)get_object_stack( gsdml_eTag_ApplicationProcess);
    if ( !p) {
      error_message_line( "Misplaced CategoryList");
      return 0;
    }

    p->CategoryList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_CategoryItem: {
    gsdml_CategoryItem *o = new gsdml_CategoryItem( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sCategoryItem);

    gsdml_CategoryList *p = (gsdml_CategoryList *)get_object_stack( gsdml_eTag_CategoryList);
    if ( !p) {
      error_message_line( "Misplaced CategoryItem");
      return 0;
    }

    p->CategoryItem.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_ExternalTextList: {
    gsdml_ExternalTextList *o = new gsdml_ExternalTextList( this);

    gsdml_ApplicationProcess *p = (gsdml_ApplicationProcess *)get_object_stack( gsdml_eTag_ApplicationProcess);
    if ( !p) {
      error_message_line( "Misplaced ExternalTextList");
      return 0;
    }

    p->ExternalTextList = o;
    ro = o;
    break;
  }
  case gsdml_eTag_PrimaryLanguage: {
    gsdml_PrimaryLanguage *o = new gsdml_PrimaryLanguage( this);

    gsdml_ExternalTextList *p = (gsdml_ExternalTextList *)get_object_stack( gsdml_eTag_ExternalTextList);
    if ( !p) {
      error_message_line( "Misplaced PrimaryLanguage");
      return 0;
    }

    p->PrimaryLanguage = o;
    ro = o;
    break;
  }
  case gsdml_eTag_Language: {
    gsdml_Language *o = new gsdml_Language( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sLanguage);

    gsdml_ExternalTextList *p = (gsdml_ExternalTextList *)get_object_stack( gsdml_eTag_ExternalTextList);
    if ( !p) {
      error_message_line( "Misplaced Language");
      return 0;
    }

    p->Language.push_back( o);
    ro = o;
    break;
  }
  case gsdml_eTag_Text: {
    gsdml_Text *o = new gsdml_Text( this);
    current_body = &o->Body;
    current_body_size = sizeof( gsdml_sText);

    void *p;
    if ( (p = get_object_stack( gsdml_eTag_PrimaryLanguage)))
      ((gsdml_PrimaryLanguage *)p)->Text.push_back( o);
    else if ( (p = get_object_stack( gsdml_eTag_Language)))
      ((gsdml_Language *)p)->Text.push_back( o);
    else {
      error_message_line( "Misplaced Text");
      return 0;
    }

    ro = o;
    break;
  }
  default: ;
  }
  return ro;
}

void *pn_gsdml::find_value_ref( char *ref)
{
  if ( !ApplicationProcess || !ApplicationProcess->ValueList)
    return 0;
  for ( unsigned int i = 0; i < ApplicationProcess->ValueList->ValueItem.size(); i++) {
    if ( strcmp( ApplicationProcess->ValueList->ValueItem[i]->Body.ID, ref) == 0)
      return (void *)ApplicationProcess->ValueList->ValueItem[i];
  }
  return 0;
}

void *pn_gsdml::find_module_ref( char *ref)
{
  if ( !ApplicationProcess || !ApplicationProcess->ModuleList)
    return 0;
  for ( unsigned int i = 0; i < ApplicationProcess->ModuleList->ModuleItem.size(); i++) {
    if ( strcmp( ApplicationProcess->ModuleList->ModuleItem[i]->Body.ID, ref) == 0)
      return (void *)ApplicationProcess->ModuleList->ModuleItem[i];
  }
  return 0;
}

void *pn_gsdml::find_submodule_ref( char *ref)
{
  if ( !ApplicationProcess || !ApplicationProcess->SubmoduleList)
    return 0;
  for ( unsigned int i = 0; i < ApplicationProcess->SubmoduleList->SubmoduleItem.size(); i++) {
    if ( strcmp( ApplicationProcess->SubmoduleList->SubmoduleItem[i]->Body.ID, ref) == 0)
      return (void *)ApplicationProcess->SubmoduleList->SubmoduleItem[i];
  }
  return 0;
}

void *pn_gsdml::find_category_ref( char *ref)
{
  if ( !ApplicationProcess || !ApplicationProcess->CategoryList)
    return 0;
  for ( unsigned int i = 0; i < ApplicationProcess->CategoryList->CategoryItem.size(); i++) {
    if ( strcmp( ApplicationProcess->CategoryList->CategoryItem[i]->Body.ID, ref) == 0)
      return (void *)ApplicationProcess->CategoryList->CategoryItem[i];
  }
  return 0;
}

void *pn_gsdml::find_graphic_ref( char *ref)
{
  if ( !ApplicationProcess || !ApplicationProcess->GraphicsList)
    return 0;
  for ( unsigned int i = 0; i < ApplicationProcess->GraphicsList->GraphicItem.size(); i++) {
    if ( strcmp( ApplicationProcess->GraphicsList->GraphicItem[i]->Body.ID, ref) == 0)
      return (void *)ApplicationProcess->GraphicsList->GraphicItem[i];
  }
  return 0;
}

void *pn_gsdml::find_text_ref( char *ref)
{
  if ( !ApplicationProcess || !ApplicationProcess->ExternalTextList)
    return noref;

  int lang_idx = -1;
  if ( strcmp( current_lang, "") != 0) {
    for ( unsigned int i = 0; i < ApplicationProcess->ExternalTextList->Language.size(); i++) {
      if ( strcmp( ApplicationProcess->ExternalTextList->Language[i]->Body.xml_lang, current_lang) == 0)
	lang_idx = i;
    }
  }
  if ( lang_idx != -1) {
    for ( unsigned int i = 0; i < ApplicationProcess->ExternalTextList->Language[lang_idx]->Text.size(); i++) {
      if ( strcmp( ref, ApplicationProcess->ExternalTextList->Language[lang_idx]->Text[i]->Body.TextId) == 0)
	return (void *) ApplicationProcess->ExternalTextList->Language[lang_idx]->Text[i]->Body.Value;
    }
  }

  if ( !ApplicationProcess->ExternalTextList->PrimaryLanguage)
    return noref;

  for ( unsigned int i = 0; i < ApplicationProcess->ExternalTextList->PrimaryLanguage->Text.size(); i++) {
    if ( strcmp( ref, ApplicationProcess->ExternalTextList->PrimaryLanguage->Text[i]->Body.TextId) == 0)
      return (void *) ApplicationProcess->ExternalTextList->PrimaryLanguage->Text[i]->Body.Value;
  }

  return noref;
}


int pn_gsdml::data_to_ostring( unsigned char *data, int size, char *str, int strsize)
{
  int len = 0;
  for ( int i = 0; i < size; i++) {
    if ( i == size - 1) {
      if ( len + 5 >= strsize)
	return 0;
      len += sprintf( &str[i*5], "0x%02hhx", data[i]);
    }
    else {
      if ( len + 4 >= strsize)
	return 0;
      len += sprintf( &str[i*5], "0x%02hhx,", data[i]);
    }
  }
  return 1;
}

int pn_gsdml::ostring_to_data( unsigned char **data, const char *str, int size, int *rsize)
{
  char valstr[40];
  int valcnt;
  unsigned int val;
  const char *s, *t;
  int sts;

  *data = (unsigned char *) calloc( 1, size);
  t = str;
  valcnt = 0;
  for ( s = str;; s++) {
    if ( valcnt > size) {
      printf( "** Size error");
      break;
    }
    if ( *s == ',' || *s == 0) {
      strncpy( valstr, t, s - t);
      valstr[s - t] = 0;
      dcli_remove_blank( valstr, valstr);
      if ( valstr[0] == '0' && valstr[1] == 'x')
	sts = sscanf( &valstr[2], "%x", &val);
      else
	sts = sscanf( valstr, "%d", &val);
      *(*data + valcnt++) = (unsigned char) val;
      if ( sts != 1)
	printf( "** GSDML-parser error, Syntax error in octet string, %s\n", str);
 
      t = s+1;
    }
    if ( *s == 0)
      break;
  }
  if ( rsize)
    *rsize = valcnt;
  return 1;
}

int pn_gsdml::set_par_record_default( unsigned char *data, int size, 
				      gsdml_ParameterRecordDataItem *par_record)
{
  gsdml_eValueDataType type;
  int sts;
  int datasize;

  for ( unsigned int i = 0; i < par_record->Ref.size(); i++) {
    if ( strcmp( par_record->Ref[i]->Body.DefaultValue, "") == 0)
      continue;

    sts = string_to_value_datatype( par_record->Ref[i]->Body.DataType, &type);
    if ( EVEN(sts)) continue;

    switch ( type) {
    case gsdml_eValueDataType_Integer8:
    case gsdml_eValueDataType_Unsigned8: 
    case gsdml_eValueDataType_Bit:
    case gsdml_eValueDataType_BitArea:
      datasize = 1;
      break;
    case gsdml_eValueDataType_Integer16:
    case gsdml_eValueDataType_Unsigned16:
      datasize = 2;
      break;
    case gsdml_eValueDataType_Integer32:
    case gsdml_eValueDataType_Unsigned32:
    case gsdml_eValueDataType_Float32:
      datasize = 4;
      break;
    case gsdml_eValueDataType_Integer64:
    case gsdml_eValueDataType_Unsigned64:
    case gsdml_eValueDataType_Float64:
      datasize = 8;
      break;
    case gsdml_eValueDataType_OctetString:
    case gsdml_eValueDataType_VisibleString:
      datasize = par_record->Ref[i]->Body.Length;
      break;
    default:
      datasize = 0;
    }

    switch ( type) {
    case gsdml_eValueDataType_Bit: {
      if ( datasize + par_record->Ref[i]->Body.ByteOffset > (unsigned int)size) {
	printf( "GSDML-Parser error, Default value exceeds data size");
	return 0;
      }

      unsigned char mask = 1 << par_record->Ref[i]->Body.BitOffset;
      if ( strcmp( par_record->Ref[i]->Body.DefaultValue, "0") == 0)
	*(data + par_record->Ref[i]->Body.ByteOffset) &= ~mask;
      else if ( strcmp( par_record->Ref[i]->Body.DefaultValue, "1") == 0)
	*(data + par_record->Ref[i]->Body.ByteOffset) |= mask;
      break;
    }
    case gsdml_eValueDataType_BitArea: {
      unsigned short mask = 0;
      unsigned short value;

      if ( datasize + par_record->Ref[i]->Body.ByteOffset > (unsigned int)size) {
	printf( "GSDML-Parser error, Default value exceeds data size");
	return 0;
      }

      for ( int j = 0; j < par_record->Ref[i]->Body.BitLength; j++)
	mask |= (mask << 1) | 1; 
      mask <<= par_record->Ref[i]->Body.BitOffset;
      
      sts = sscanf( par_record->Ref[i]->Body.DefaultValue, "%hu", &value);
      if ( sts != 1)
	break;

      value <<= par_record->Ref[i]->Body.BitOffset;

      *(data + par_record->Ref[i]->Body.ByteOffset) &= ~mask;
      *(data + par_record->Ref[i]->Body.ByteOffset) |= value;
      break;
    }
    default:
      if ( datasize + par_record->Ref[i]->Body.ByteOffset > (unsigned int)size) {
	printf( "GSDML-Parser error, Default value exceeds data size");
	return 0;
      }

      string_to_datavalue( type, data + par_record->Ref[i]->Body.ByteOffset,
			   par_record->Ref[i]->Body.Length, par_record->Ref[i]->Body.DefaultValue);
    }
  }
  return 1;
}

void gsdml_ProfileHeader::print( int ind) {
  char is[] = "                    ";
  is[ind] = 0;

  printf("%s<ProfileHeader\n"
	 "%s  ProfileIdentification=\"%s\"\n"
	 "%s  ProfileRevision=\"%s\"\n"
	 "%s  ProfileName=\"%s\"\n"
	 "%s  ProfileSource=\"%s\"\n"
	 "%s  ProfileClassID=\"%s\"\n"
	 "%s  ISO15745Part=\"%d\"\n"
	 "%s  ISO15745Edition=\"%d\"\n"
	 "%s  Profiletechnology=\"%s\"/>\n",
	 is,
	 is, Body.ProfileIdentification,
	 is, Body.ProfileRevision,
	 is, Body.ProfileName,
	 is, Body.ProfileSource,
	 is, Body.ProfileClassID,
	 is, Body.ISO15745Part,
	 is, Body.ISO15745Edition,
	 is, Body.ProfileTechnology);
}

void gsdml_DeviceIdentity::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf("%s<DeviceIdentity\n"
	 "%s  VendorID=\"%d\"\n"
	 "%s  DeviceID=\"%d\"\n"
	 "%s  InfoText=\"%s\"\n"
	 "%s  VendorName=\"%s\"/>\n",
	 is,
	 is, Body.VendorID,
	 is, Body.DeviceID,
	 is, Body.InfoText.ref,
	 is, Body.VendorName);
}

void gsdml_DeviceFunction::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf("%s<DeviceFunction:\n"
	 "%s  MainFamily=\"%s\"\n"
	 "%s  ProductFamily=\"%s\"/>\n",
	 is, 
	 is, Body.MainFamily,
	 is, Body.ProductFamily);
}

void gsdml_ApplicationProcess::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ApplicationProcess>\n", is);
  if ( DeviceAccessPointList)
    DeviceAccessPointList->print( ind + 2);
  if ( ModuleList)
    ModuleList->print( ind + 2);
  if ( ValueList)
    ValueList->print( ind + 2);
  if ( ChannelDiagList)
    ChannelDiagList->print( ind + 2);
  if ( UnitDiagTypeList)
    UnitDiagTypeList->print( ind + 2);
  if ( GraphicsList)
    GraphicsList->print( ind + 2);
  if ( CategoryList)
    CategoryList->print( ind + 2);
  if ( ExternalTextList)
    ExternalTextList->print( ind + 2);
  
  printf( "%s</ApplicationProcess>\n", is);
}

void gsdml_DeviceAccessPointList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<DeviceAccessPointList>\n", is);
  for ( unsigned int i = 0; i < DeviceAccessPointItem.size(); i++) {
    DeviceAccessPointItem[i]->print( ind + 2);
  }
  printf( "%s</DeviceAccessPointList>\n", is);
}

void gsdml_DeviceAccessPointItem::print( int ind)
{  
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<DeviceAccessPointItem\n"
	  "%s  ID=\"%s\"\n"
	  "%s  PhysicalSlots=\"%s\"\n"
	  "%s  ModuleIdentNumber=\"%u\"\n"
	  "%s  MinDeviceInterval=\"%hu\"\n"
	  "%s  ImplementationType=\"%s\"\n"
	  "%s  DNS_CompatibleName=\"%s\"\n"
	  "%s  ExtendedAddressAssignmentSupported=\"%d\"\n"
	  "%s  AddressAssignment=\"%s\"\n"
	  "%s  AllowedInSlots=\"%s\"\n"
	  "%s  FixedInSlots=\"%s\"\n"
	  "%s  ObjectUUID_LocalIndex=\"%hu\"\n"
	  "%s  RequiredSchemaVersion=\"%s\"\n"
	  "%s  MultipleWriteSupported=\"%d\"\n"
	  "%s  OIXS_Required=\"%d\"\n"
	  "%s  PhysicalSubslots=\"%s\"\n"
	  "%s  RemoteApplicationTimeout=\"%hu\"\n"
	  "%s  MaxSupportedRecordSize=\"%u\"\n"
	  "%s  PowerOnToCommReady=\"%u\"\n"
	  "%s  ParameterizationSpeedSupported=\"%d\"\n"
	  "%s  NameOfStationNotTransferable=\"%d\"/>\n",
	  is, 
	  is, Body.ID,
	  is, Body.PhysicalSlots.str,
	  is, Body.ModuleIdentNumber,
	  is, Body.MinDeviceInterval,
	  is, Body.ImplementationType,
	  is, Body.DNS_CompatibleName,
	  is, Body.ExtendedAddressAssignmentSupported,
	  is, Body.AddressAssignment,
	  is, Body.AllowedInSlots.str,
	  is, Body.FixedInSlots.str,
	  is, Body.ObjectUUID_LocalIndex,
	  is, Body.RequiredSchemaVersion,
	  is, Body.MultipleWriteSupported,
	  is, Body.IOXS_Required,
	  is, Body.PhysicalSubslots.str,
	  is, Body.RemoteApplicationTimeout,
	  is, Body.MaxSupportedRecordSize,
	  is, Body.PowerOnToCommReady,
	  is, Body.ParameterizationSpeedSupported,
	  is, Body.NameOfStationNotTransferable);

  if ( ModuleInfo)
    ModuleInfo->print( ind + 2);
  if ( SubslotList)
    SubslotList->print( ind + 2);
  if ( IOConfigData)
    IOConfigData->print( ind + 2);
  if ( UseableModules)
    UseableModules->print( ind + 2);
  if ( VirtualSubmoduleList)
    VirtualSubmoduleList->print( ind + 2);
  if ( SystemDefinedSubmoduleList)
    SystemDefinedSubmoduleList->print( ind + 2);
  if ( Graphics)
    Graphics->print( ind + 2);
  if ( ApplicationRelations)
    ApplicationRelations->print( ind + 2);
  if ( UseableSubmodules)
    UseableSubmodules->print( ind + 2);
  if ( SlotList)
    SlotList->print( ind + 2);
  if ( SlotGroups)
    SlotGroups->print( ind + 2);
  printf( "%s</DeviceAccessPointItem>\n", is);
}  
  
void gsdml_ModuleInfo::build()
{
  if ( strcmp( Body.Name.ref, "") != 0) {
    Body.Name.p = gsdml->find_text_ref( Body.Name.ref);
    if ( Body.Name.p == noref)
            gsdml->error_message("Name not found: \"%s\"", Body.Name.ref);
  }  
  if ( strcmp( Body.InfoText.ref, "") != 0) {
    Body.InfoText.p = gsdml->find_text_ref( Body.InfoText.ref);
    if ( Body.InfoText.p == noref)
      gsdml->error_message("InfoText not found: \"%s\"", Body.InfoText.ref);
  }  
  if ( strcmp( Body.CategoryRef.ref, "") != 0) {
    Body.CategoryRef.p = gsdml->find_category_ref( Body.CategoryRef.ref);
    if ( !Body.CategoryRef.p)
      gsdml->error_message("CategoryRef not found: \"%s\"", Body.CategoryRef.ref);
  }  
  if ( strcmp( Body.SubCategory1Ref.ref, "") != 0) {
    Body.SubCategory1Ref.p = gsdml->find_category_ref( Body.SubCategory1Ref.ref);
    if ( !Body.SubCategory1Ref.p)
      gsdml->error_message("SubCategory1Ref not found: \"%s\"", Body.SubCategory1Ref.ref);
  }  
}

void gsdml_ModuleInfo::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf ( "%s<ModuleInfo \n"  
	   "%s  CategoryRef=\"%s\"\n" 
	   "%s  SubCategory1Ref=\"%s\"\n" 
	   "%s  Name=\"%s\"\n" 
	   "%s  InfoText=\"%s\"\n" 
	   "%s  VendorName=\"%s\"\n" 
	   "%s  OrderNumber=\"%s\"\n" 
	   "%s  HardwareRelease=\"%s\"\n" 
	   "%s  SoftwareRelease=\"%s\"\n" 
	   "%s  MainFamily=\"%s\"\n" 
	   "%s  ProductFamily=\"%s\"/>\n", 
	   is,
 	   is, Body.CategoryRef.ref,
	   is, Body.SubCategory1Ref.ref,
	   is, Body.Name.ref,
	   is, Body.InfoText.ref,
	   is, Body.VendorName,
	   is, Body.OrderNumber,
	   is, Body.HardwareRelease,
	   is, Body.SoftwareRelease,
	   is, Body.MainFamily,
	   is, Body.ProductFamily);
}

void gsdml_SubslotList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SubslotList> \n", is);
  for ( unsigned int k = 0; k < SubslotItem.size(); k++)
    SubslotItem[k]->print( ind + 2);
  printf( "%s</SubslotList> \n", is);
}

void gsdml_SubslotItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SubslotItem\n"
	  "%s  TextId=\"%s\"\n"
	  "%s  SubslotNumber=\"%d\"/>\n",
	  is,
	  is, Body.TextId.ref,
	  is, Body.SubslotNumber);
}

void gsdml_IOConfigData::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf("%s<IOConfigData\n"
	 "%s  MaxInputLength=\"%d\"\n"
	 "%s  MaxOutputLength=\"%d\"\n"
	 "%s  MaxDataLength=\"%d\"/>\n",
	 is,
	 is, Body.MaxInputLength,
	 is, Body.MaxOutputLength,
	 is, Body.MaxDataLength);
}

void gsdml_UseableModules::build()
{
  for ( unsigned int i = 0; i < ModuleItemRef.size(); i++)
    ModuleItemRef[i]->build();
}

void gsdml_UseableModules::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<UseableModules>\n", is);
  for ( unsigned int k = 0; k < ModuleItemRef.size(); k++)
    ModuleItemRef[k]->print( ind + 2);
  printf( "%s</UseableModules>\n", is);
}

void gsdml_ModuleItemRef::build()
{
  Body.ModuleItemTarget.p = gsdml->find_module_ref( Body.ModuleItemTarget.ref);
  if ( !Body.ModuleItemTarget.p)
    gsdml->error_message("ModuleItemTarget not found: \"%s\"", Body.ModuleItemTarget.ref);

  if ( strcmp( Body.AllowedInSlots.str, "") != 0)
    Body.AllowedInSlots.list = new gsdml_Valuelist( Body.AllowedInSlots.str);
  if ( strcmp( Body.FixedInSlots.str, "") != 0)
    Body.FixedInSlots.list = new gsdml_Valuelist( Body.FixedInSlots.str);
  if ( strcmp( Body.UsedInSlots.str, "") != 0)
    Body.UsedInSlots.list = new gsdml_Valuelist( Body.UsedInSlots.str);
}

gsdml_ModuleItemRef::~gsdml_ModuleItemRef()
{
  if ( Body.AllowedInSlots.list)
    delete Body.AllowedInSlots.list;
  if ( Body.FixedInSlots.list)
    delete Body.FixedInSlots.list;
  if ( Body.UsedInSlots.list)
    delete Body.UsedInSlots.list;
}

void gsdml_ModuleItemRef::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ModuleItemRef \n"
	  "%s  ModuleItemTarget=\"%s\"\n"
	  "%s  AllowedInSlots=\"%s\"\n"
	  "%s  UsedInSlots=\"%s\"\n"
	  "%s  FixedInSlots=\"%s\"/>\n",
	  is,
	  is, Body.ModuleItemTarget.ref,
	  is, Body.AllowedInSlots.str,
	  is, Body.UsedInSlots.str,
	  is, Body.FixedInSlots.str);
}

void gsdml_BitDataItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<BitDataItem \n"
	  "%s  BitOffset=\"%d\"\n"
	  "%s  TextId=\"%s\"/>\n",
	  is,
	  is, Body.BitOffset,
	  is, Body.TextId.ref);
}

void gsdml_DataItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<DataItem \n"
	  "%s  DataType=\"%s\"\n"
	  "%s  Length=\"%d\"\n"
	  "%s  UseAsBits=\"%d\"\n"
	  "%s  TextId=\"%s\">\n",
	  is,
	  is, Body.DataType,
	  is, Body.Length,
	  is, Body.UseAsBits,
	  is, Body.TextId.ref);
  for ( unsigned int k = 0; k < BitDataItem.size(); k++)
    BitDataItem[k]->print( ind + 2);
  printf( "%s</DataItem>\n", is);
}

void gsdml_Input::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Input \n"
	  "%s  Consistency=\"%s\">\n",
	  is,
	  is, Body.Consistency);
  for ( unsigned int k = 0; k < DataItem.size(); k++)
    DataItem[k]->print( ind + 2);
  printf( "%s</Input>\n", is);
}

void gsdml_Output::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Output \n"
	  "%s  Consistency=\"%s\">\n",
	  is,
	  is, Body.Consistency);
  for ( unsigned int k = 0; k < DataItem.size(); k++)
    DataItem[k]->print( ind + 2);
  printf( "%s</Output>\n", is);
}

void gsdml_IOData::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<IOData \n"
	  "%s  IOPS_Length=\"%hhu\"\n"
	  "%s  IOCS_Length=\"%hhu\"\n"
	  "%s  F_IO_StructureDescVersion=\"%hhu\"\n"
	  "%s  F_IO_StructureDescCRC=\"%u\">\n",
	  is,
	  is, Body.IOPS_Length,
	  is, Body.IOCS_Length,
	  is, Body.F_IO_StructureDescVersion,
	  is, Body.F_IO_StructureDescCRC);
  if ( Input)
    Input->print( ind + 2);
  if ( Output)
    Output->print( ind + 2);
  printf( "%s</IOData>\n", is);
}

void gsdml_Ref::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Ref \n"
	  "%s  ValueItemTarget=\"%s\"\n"
	  "%s  ByteOffset=\"%u\"\n"
	  "%s  BitOffset=\"%d\"\n"
	  "%s  BitLength=\"%d\"\n"
	  "%s  DataType=\"%s\"\n"
	  "%s  DefaultValue=\"%s\"\n"
	  "%s  AllowedValues=\"%s\"\n"
	  "%s  Changeable=\"%d\"\n"
	  "%s  Visible=\"%d\"\n"
	  "%s  TextId=\"%s\"\n"
	  "%s  Length=\"%hu\"/>\n",
	  is,
	  is, Body.ValueItemTarget.ref,
	  is, Body.ByteOffset,
	  is, Body.BitOffset,
	  is, Body.BitLength,
	  is, Body.DataType,
	  is, Body.DefaultValue,
	  is, Body.AllowedValues,
	  is, Body.Changeable,
	  is, Body.Visible,
	  is, Body.TextId.ref,
	  is, Body.Length);
}

void gsdml_Const::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Const \n"
	  "%s  ByteOffset=\"%u\"\n"
	  "%s  Data=\"%s\"/>\n",
	  is,
	  is, Body.ByteOffset,
	  is, Body.Data);
}

void gsdml_ParameterRecordDataItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ParameterRecordDataItem \n"
	  "%s  Index=\"%hu\"\n"
	  "%s  Length=\"%u\"\n"
	  "%s  TransferSequence=\"%hu\"\n"
	  "%s  Name=\"%s\">\n",
	  is,
	  is, Body.Index,
	  is, Body.Length,
	  is, Body.TransferSequence,
	  is, Body.Name.ref);

  for ( unsigned int k = 0; k < Const.size(); k++)
    Const[k]->print( ind + 2);
  for ( unsigned int k = 0; k < Ref.size(); k++)
    Ref[k]->print( ind + 2);
  printf( "%s</ParameterRecordDataItem>\n", is);
}

void gsdml_F_ParameterRecordDataItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<F_ParameterRecordDataItem \n"
	  "%s  F_ParamDescCRC=\"%hu\"\n"
	  "%s  Index=\"%hu\"\n"
	  "%s  TransferSequence=\"%hu\"\n"
	  "%s  F_Check_iPar_DefaultValue=\"%s\"\n"
	  "%s  F_Check_iPar_AllowedValues=\"%s\"\n"
	  "%s  F_Check_iPar_Visible=\"%d\"\n"
	  "%s  F_Check_iPar_Changeable=\"%d\"\n"
	  "%s  F_SIL_DefaultValue=\"%s\"\n"
	  "%s  F_SIL_AllowedValues=\"%s\"\n"
	  "%s  F_SIL_Visible=\"%d\"\n"
	  "%s  F_SIL_Changeable=\"%d\"\n"
	  "%s  F_CRC_Length_DefaultValue=\"%s\"\n"
	  "%s  F_CRC_Length_AllowedValues=\"%s\"\n"
	  "%s  F_CRC_Length_Visible=\"%d\"\n"
	  "%s  F_CRC_Length_Changeable=\"%d\"\n"
	  "%s  F_Block_ID_DefaultValue=\"%d\"\n"
	  "%s  F_Block_ID_AllowedValues=\"%s\"\n"
	  "%s  F_Block_ID_Visible=\"%d\"\n"
	  "%s  F_Block_ID_Changeable=\"%d\"\n"
	  "%s  F_Par_Version_DefaultValue=\"%d\"\n"
	  "%s  F_Par_Version_AllowedValues=\"%s\"\n"
	  "%s  F_Par_Version_Visible=\"%d\"\n"
	  "%s  F_Par_Version_Changeable=\"%d\"\n"
	  "%s  F_Source_Add_DefaultValue=\"%hu\"\n"
	  "%s  F_Source_Add_AllowedValues=\"%s\"\n"
	  "%s  F_Source_Add_Visible=\"%d\"\n"
	  "%s  F_Source_Add_Changeable=\"%d\"\n"
	  "%s  F_Dest_Add_DefaultValue=\"%hu\"\n"
	  "%s  F_Dest_Add_AllowedValues=\"%s\"\n"
	  "%s  F_Dest_Add_Visible=\"%d\"\n"
	  "%s  F_Dest_Add_Changeable=\"%d\"\n"
	  "%s  F_WD_Time_DefaultValue=\"%hu\"\n"
	  "%s  F_WD_Time_AllowedValues=\"%s\"\n"
	  "%s  F_WD_Time_Visible=\"%d\"\n"
	  "%s  F_WD_Time_Changeable=\"%d\"\n"
	  "%s  F_Par_CRC_DefaultValue=\"%hu\"\n"
	  "%s  F_Par_CRC_AllowedValues=\"%s\"\n"
	  "%s  F_Par_CRC_Visible=\"%d\"\n"
	  "%s  F_Par_CRC_Changeable=\"%d\"\n"
	  "%s  F_iPar_CRC_DefaultValue=\"%u\"\n"
	  "%s  F_iPar_CRC_AllowedValues=\"%s\"\n"
	  "%s  F_iPar_CRC_Visible=\"%d\"\n"
	  "%s  F_iPar_CRC_Changeable=\"%d\"/>\n",
	  is,
	  is, Body.F_ParamDescCRC,
	  is, Body.Index,
	  is, Body.TransferSequence,
	  is, Body.F_Check_iPar_DefaultValue,
	  is, Body.F_Check_iPar_AllowedValues,
	  is, Body.F_Check_iPar_Visible,
	  is, Body.F_Check_iPar_Changeable,
	  is, Body.F_SIL_DefaultValue,
	  is, Body.F_SIL_AllowedValues,
	  is, Body.F_SIL_Visible,
	  is, Body.F_SIL_Changeable,
	  is, Body.F_CRC_Length_DefaultValue,
	  is, Body.F_CRC_Length_AllowedValues,
	  is, Body.F_CRC_Length_Visible,
	  is, Body.F_CRC_Length_Changeable,
	  is, Body.F_Block_ID_DefaultValue,
	  is, Body.F_Block_ID_AllowedValues.str,
	  is, Body.F_Block_ID_Visible,
	  is, Body.F_Block_ID_Changeable,
	  is, Body.F_Par_Version_DefaultValue,
	  is, Body.F_Par_Version_AllowedValues.str,
	  is, Body.F_Par_Version_Visible,
	  is, Body.F_Par_Version_Changeable,
	  is, Body.F_Source_Add_DefaultValue,
	  is, Body.F_Source_Add_AllowedValues.str,
	  is, Body.F_Source_Add_Visible,
	  is, Body.F_Source_Add_Changeable,
	  is, Body.F_Dest_Add_DefaultValue,
	  is, Body.F_Dest_Add_AllowedValues.str,
	  is, Body.F_Dest_Add_Visible,
	  is, Body.F_Dest_Add_Changeable,
	  is, Body.F_WD_Time_DefaultValue,
	  is, Body.F_WD_Time_AllowedValues.str,
	  is, Body.F_WD_Time_Visible,
	  is, Body.F_WD_Time_Changeable,
	  is, Body.F_Par_CRC_DefaultValue,
	  is, Body.F_Par_CRC_AllowedValues.str,
	  is, Body.F_Par_CRC_Visible,
	  is, Body.F_Par_CRC_Changeable,
	  is, Body.F_iPar_CRC_DefaultValue,
	  is, Body.F_iPar_CRC_AllowedValues.str,
	  is, Body.F_iPar_CRC_Visible,
	  is, Body.F_iPar_CRC_Changeable);

}

void gsdml_RecordDataList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<RecordDataList> \n", is);

  for ( unsigned int k = 0; k < ParameterRecordDataItem.size(); k++)
    ParameterRecordDataItem[k]->print( ind + 2);
  if ( F_ParameterRecordDataItem)
    F_ParameterRecordDataItem->print( ind + 2);
  printf( "%s</RecordDataList>\n", is);
}

void gsdml_GraphicItemRef::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<GraphicItemRef \n"
	  "%s  Type=\"%s\"\n"
	  "%s  GraphicItemTarget=\"%s\"/>\n",
	  is,
	  is, Body.Type,
	  is, Body.GraphicItemTarget.ref);
}

void gsdml_Graphics::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Graphics> \n", is);

  for ( unsigned int k = 0; k < GraphicItemRef.size(); k++)
    GraphicItemRef[k]->print( ind + 2);
  printf( "%s</Graphics>\n", is);
}

void gsdml_IsochroneMode::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<IsochroneMode \n"
	  "%s  T_DC_Base=\"%hu\"\n"
	  "%s  T_DC_Min=\"%hu\"\n"
	  "%s  T_DC_Max=\"%hu\"\n"
	  "%s  T_IO_Base=\"%hu\"\n"
	  "%s  T_IO_InputMin=\"%hu\"\n"
	  "%s  T_IO_OutputMin=\"%hu\"\n"
	  "%s  IsochroneModeRequired=\"%d\"/>\n",
	  is,
	  is, Body.T_DC_Base,
	  is, Body.T_DC_Min,
	  is, Body.T_DC_Max,
	  is, Body.T_IO_Base,
	  is, Body.T_IO_InputMin,
	  is, Body.T_IO_OutputMin,
	  is, Body.IsochroneModeRequired);
}

void gsdml_VirtualSubmoduleItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<VirtualSubmoduleItem \n"
	  "%s  ID=\"%s\"\n"
	  "%s  SubmoduleItentNumber=\"%u\"\n"
	  "%s  API=\"%u\"\n"
	  "%s  FixedInSubslots=\"%s\"\n"
	  "%s  PROFISafeSupported=\"%u\"\n"
	  "%s  Writeable_IM_Records=\"%s\"\n"
	  "%s  Max_iParameterSize=\"%u\"\n"
	  "%s  SubsysModuleDirIndex=\"%hu\">\n",
	  is,
	  is, Body.ID,
	  is, Body.SubmoduleIdentNumber,
	  is, Body.API,
	  is, Body.FixedInSubslots.str,
	  is, Body.PROFIsafeSupported,
	  is, Body.Writeable_IM_Records.str,
	  is, Body.Max_iParameterSize,
	  is, Body.SubsysModuleDirIndex);

  if ( IOData)
    IOData->print( ind + 2);
  if ( RecordDataList)
    RecordDataList->print( ind + 2);
  if ( ModuleInfo)
    ModuleInfo->print( ind + 2);
  if ( Graphics)
    Graphics->print( ind + 2);
  if ( IsochroneMode)
    IsochroneMode->print( ind + 2);

  printf( "%s<VirtualSubmoduleItem>\n", is);
}

void gsdml_VirtualSubmoduleList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<VirtualSubmoduleList> \n", is);

  for ( unsigned int k = 0; k < VirtualSubmoduleItem.size(); k++)
    VirtualSubmoduleItem[k]->print( ind + 2);
  printf( "%s</VirtualSubmoduleList>\n", is);
}

void gsdml_DCP_FlashOnceSignalUnit::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<DCP_FlashOnceSignalUnit \n"
	  "%s  TextId=\"%s\"/>\n",
	  is,
	  is, Body.TextId.ref);
}

void gsdml_General::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<General>\n", is);
  if ( DCP_FlashOnceSignalUnit)
    DCP_FlashOnceSignalUnit->print( ind + 2);
  printf( "%s</General>\n", is);
}

void gsdml_RT_Class3Properties::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<RT_Class3Properties \n"
	  "%s  MaxBridgeDelay=\"%hu\"\n"
	  "%s  MaxNumberIR_FrameData=\"%hu\"/>\n",
	  is,
	  is, Body.MaxBridgeDelay,
	  is, Body.MaxNumberIR_FrameData);
}

void gsdml_SynchronisationMode::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SynchronisationMode \n"
	  "%s  SupportedRole=\"%s\"\n"
	  "%s  MaxLocalJitter=\"%hu\"\n"
	  "%s  T_PLL_MAX=\"%hu\"\n"
	  "%s  SupportedSyncProtocols=\"%s\"/>\n",
	  is,
	  is, Body.SupportedRole,
	  is, Body.MaxLocalJitter,
	  is, Body.T_PLL_MAX,
	  is, Body.SupportedSyncProtocols);
}

void gsdml_TimingProperties::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<TimingProperties \n"
	  "%s  SendClock=\"%s\"\n"
	  "%s  ReductionRatio=\"%s\"\n"
	  "%s  ReductionRatioPow2=\"%s\"\n"
	  "%s  ReductionRatioNonPow2=\"%s\"/>\n",
	  is,
	  is, Body.SendClock.str,
	  is, Body.ReductionRatio.str,
	  is, Body.ReductionRatioPow2.str,
	  is, Body.ReductionRatioNonPow2.str);
}

void gsdml_RT_Class3TimingProperties::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<RT_Class3TimingProperties \n"
	  "%s  SendClock=\"%s\"\n"
	  "%s  ReductionRatio=\"%s\"\n"
	  "%s  ReductionRatioPow2=\"%s\"\n"
	  "%s  ReductionRatioNonPow2=\"%s\"/>\n",
	  is,
	  is, Body.SendClock.str,
	  is, Body.ReductionRatio.str,
	  is, Body.ReductionRatioPow2.str,
	  is, Body.ReductionRatioNonPow2.str);
}

void gsdml_InterfaceSubmoduleItem_ApplicationRelations::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ApplicationRelations \n"
	  "%s  NumberOfAdditionalInputCR=\"%hu\"\n"
	  "%s  NumberOfAdditionalOutputCR=\"%hu\"\n"
	  "%s  NumberOfAdditionalMulticastProviderCR=\"%hu\"\n"
	  "%s  NumberOfMulticastConsumerCR=\"%hu\"\n"
	  "%s  PullModuleAlarmSupported=\"%hu\">\n",
	  is,
	  is, Body.NumberOfAdditionalInputCR,
	  is, Body.NumberOfAdditionalOutputCR,
	  is, Body.NumberOfAdditionalMulticastProviderCR,
	  is, Body.NumberOfMulticastConsumerCR,
	  is, Body.PullModuleAlarmSupported);

  if ( TimingProperties)
    TimingProperties->print( ind + 2);
  if ( RT_Class3TimingProperties)
    RT_Class3TimingProperties->print( ind + 2);

  printf( "%s</ApplicationRelations>\n", is);
}

void gsdml_MediaRedundancy::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<MediaRedundancy \n"
	  "%s  RT_MediaRedundancySupported=\"%u\"\n"
	  "%s  SupportedRole=\"%s\"/>\n",
	  is,
	  is, Body.RT_MediaRedundancySupported,
	  is, Body.SupportedRole);
}

void gsdml_InterfaceSubmoduleItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<InterfaceSubmoduleItem> \n"
	  "%s  SubslotNumber=\"%hu\"\n"
	  "%s  TextId=\"%s\"\n"
	  "%s  SupportedRt_Class=\"%s\"\n"
	  "%s  SupportedRT_Classes=\"%s\"\n"
	  "%s  IsochroneModeSupported=\"%u\"\n"
	  "%s  IsochroneModeInRT_Classes=\"%s\"\n"
	  "%s  SubmoduleIdentNumber=\"%u\"\n"
	  "%s  SupportedProtocols=\"%s\"\n"
	  "%s  SupportedMibs=\"%s\"\n"
	  "%s  NetworkComponentDiagnosisSupported=\"%u\"\n"
	  "%s  DCP_HelloSupported=\"%u\"\n"
	  "%s  PTP_BoundarySupported=\"%u\"\n"
	  "%s  DCP_BoundarySupported=\"%u\">\n",
	  is,
	  is, Body.SubslotNumber,
	  is, Body.TextId.ref,
	  is, Body.SupportedRT_Class,
	  is, Body.SupportedRT_Classes,
	  is, Body.IsochroneModeSupported,
	  is, Body.IsochroneModeInRT_Classes,
	  is, Body.SubmoduleIdentNumber,
	  is, Body.SupportedProtocols,
	  is, Body.SupportedMibs,
	  is, Body.NetworkComponentDiagnosisSupported,
	  is, Body.DCP_HelloSupported,
	  is, Body.PTP_BoundarySupported,
	  is, Body.DCP_BoundarySupported);

  if ( General)
    General->print( ind + 2);
  if ( RecordDataList)
    RecordDataList->print( ind + 2);
  if ( RT_Class3Properties)
    RT_Class3Properties->print( ind + 2);
  if ( SynchronisationMode)
    SynchronisationMode->print( ind + 2);
  if ( ApplicationRelations)
    ApplicationRelations->print( ind + 2);
  if ( MediaRedundancy)
    MediaRedundancy->print( ind + 2);

  printf( "%s</InterfaceSubmoduleItem>\n", is);
}

void gsdml_PortSubmoduleItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<PortSubmoduleItem> \n"
	  "%s  SubslotNumber=\"%hu\"\n"
	  "%s  TextId=\"%s\"\n"
	  "%s  MAUType=\"%s\"\n"
	  "%s  MAUTypes=\"%s\"\n"
	  "%s  FiberOpticTypes=\"%s\"\n"
	  "%s  MaxPortTxDelay=\"%hu\"\n"
	  "%s  MaxPortRxDelay=\"%hu\"\n"
	  "%s  SubmoduleIdentNumber=\"%u\"\n"
	  "%s  PortDeactivationSupported=\"%u\"\n"
	  "%s  LinkStateDianosisCapability=\"%s\"\n"
	  "%s  PowerBudgetControlSupported=\"%u\"\n"
	  "%s  SupportsRingportConfig=\"%u\"\n"
	  "%s  IsDefauleRingport=\"%u\">\n",
	  is,
	  is, Body.SubslotNumber,
	  is, Body.TextId.ref,
	  is, Body.MAUType,
	  is, Body.MAUTypes.str,
	  is, Body.FiberOpticTypes.str,
	  is, Body.MaxPortTxDelay,
	  is, Body.MaxPortRxDelay,
	  is, Body.SubmoduleIdentNumber,
	  is, Body.PortDeactivationSupported,
	  is, Body.LinkStateDiagnosisCapability,
	  is, Body.PowerBudgetControlSupported,
	  is, Body.SupportsRingportConfig,
	  is, Body.IsDefaultRingport);

  if ( RecordDataList)
    RecordDataList->print( ind + 2);

  printf( "%s</PortSubmoduleItem>\n", is);
}

void gsdml_SystemDefinedSubmoduleList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SystemDefinedSubmoduleList> \n", is);

  if ( InterfaceSubmoduleItem)
    InterfaceSubmoduleItem->print( ind + 2);
  for ( unsigned int i = 0; i < PortSubmoduleItem.size(); i++)
    PortSubmoduleItem[i]->print( ind + 2);

  printf( "%s</SystemDefinedSubmoduleList>\n", is);
}

void gsdml_DeviceAccessPointItem_ApplicationRelations::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ApplicationRelations \n"
	  "%s  AR_BlockVersion=\"%hu\"\n"
	  "%s  IOCR_BlockVersion=\"%hu\"\n"
	  "%s  AlarmCR_BlockVersion=\"%hu\"\n"
	  "%s  SubmoduleDataBlockVersion=\"%hu\">\n",
	  is,
	  is, Body.AR_BlockVersion,
	  is, Body.IOCR_BlockVersion,
	  is, Body.AlarmCR_BlockVersion,
	  is, Body.SubmoduleDataBlockVersion);

  if ( TimingProperties)
    TimingProperties->print( ind + 2);

  printf( "%s</ApplicationRelations>\n", is);
}

void gsdml_SubmoduleItemRef::build()
{
  Body.SubmoduleItemTarget.p = gsdml->find_submodule_ref( Body.SubmoduleItemTarget.ref);
  if ( !Body.SubmoduleItemTarget.p)
    gsdml->error_message("SubmoduleItemTarget not found: \"%s\"", Body.SubmoduleItemTarget.ref);
}

void gsdml_SubmoduleItemRef::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SubmoduleItemRef \n"
	  "%s  SubmoduleItemTarget=\"%s\"\n"
	  "%s  AllowedInSubslots=\"%s\"\n"
	  "%s  UsedInSubslots=\"%s\"\n"
	  "%s  FixedInSubslots=\"%s\"/>\n",
	  is,
	  is, Body.SubmoduleItemTarget.ref,
	  is, Body.AllowedInSubslots.str,
	  is, Body.UsedInSubslots.str,
	  is, Body.FixedInSubslots.str);
}

void gsdml_UseableSubmodules::build()
{
  for ( unsigned int i = 0; i < SubmoduleItemRef.size(); i++)
    SubmoduleItemRef[i]->build();
}

void gsdml_UseableSubmodules::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<UseableSubmodules>\n", is);
  for ( unsigned int k = 0; k < SubmoduleItemRef.size(); k++)
    SubmoduleItemRef[k]->print( ind + 2);
  printf( "%s</UseableSubmodules>\n", is);
}

void gsdml_SlotItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SlotItem \n"
	  "%s  SlotNumber=\"%hu\"\n"
	  "%s  TextId=\"%s\"/>\n",
	  is,
	  is, Body.SlotNumber,
	  is, Body.TextId.ref);
}

void gsdml_SlotList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SlotList>\n", is);
  for ( unsigned int k = 0; k < SlotItem.size(); k++)
    SlotItem[k]->print( ind + 2);
  printf( "%s</SlotList>\n", is);
}

void gsdml_SlotGroup::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SlotGroup \n"
	  "%s  SlotList=\"%s\"\n"
	  "%s  Name=\"%s\"\n"
	  "%s  InfoText=\"%s\"/>\n",
	  is,
	  is, Body.SlotList.str,
	  is, Body.Name.ref,
	  is, Body.InfoText.ref);
}

void gsdml_SlotGroups::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SlotGroups>\n", is);
  for ( unsigned int k = 0; k < SlotGroup.size(); k++)
    SlotGroup[k]->print( ind + 2);
  printf( "%s</SlotGroups>\n", is);
}

void gsdml_ModuleList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ModuleList>\n", is);
  for ( unsigned int k = 0; k < ModuleItem.size(); k++)
    ModuleItem[k]->print( ind + 2);
  printf( "%s</ModuleList>\n", is);
}

void gsdml_ModuleItem::print( int ind)
{  
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ModuleItem\n"
	  "%s  ID=\"%s\"\n"
	  "%s  ModuleIdentNumber=\"%u\"\n"
	  "%s  RequiredSchemaVersion=\"%s\"\n"
	  "%s  PhysicalSubslots=\"%s\"/>\n",
	  is, 
	  is, Body.ID,
	  is, Body.ModuleIdentNumber,
	  is, Body.RequiredSchemaVersion,
	  is, Body.PhysicalSubslots.str);

  if ( ModuleInfo)
    ModuleInfo->print( ind + 2);
  if ( SubslotList)
    SubslotList->print( ind + 2);
  if ( VirtualSubmoduleList)
    VirtualSubmoduleList->print( ind + 2);
  if ( SystemDefinedSubmoduleList)
    SystemDefinedSubmoduleList->print( ind + 2);
  if ( UseableSubmodules)
    UseableSubmodules->print( ind + 2);
  if ( Graphics)
    Graphics->print( ind + 2);
  printf( "%s</ModuleItem>\n", is);
}  
  
void gsdml_SubmoduleList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<SubmoduleList> \n", is);

  for ( unsigned int k = 0; k < SubmoduleItem.size(); k++)
    SubmoduleItem[k]->print( ind + 2);
  printf( "%s</SubmoduleList>\n", is);
}

void gsdml_ValueList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ValueList> \n", is);

  for ( unsigned int k = 0; k < ValueItem.size(); k++)
    ValueItem[k]->print( ind + 2);
  printf( "%s</ValueList>\n", is);
}

void gsdml_ValueItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ValueItem\n"
	  "%s  ID=\"%s\"\n"
	  "%s  Help=\"%s\">\n",
	  is, 
	  is, Body.ID,
	  is, Body.Help.ref);

  if ( Assignments)
    Assignments->print( ind + 2);
  printf( "%s</ValueItem>\n", is);
}

void gsdml_Assignments::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Assignments> \n", is);

  for ( unsigned int k = 0; k < Assign.size(); k++)
    Assign[k]->print( ind + 2);
  printf( "%s</Assignments>\n", is);
}

void gsdml_Assign::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Assign\n"
	  "%s  Content=\"%s\"\n"
	  "%s  TextId=\"%s\"/>\n",
	  is, 
	  is, Body.Content,
	  is, Body.TextId.ref);
}

void gsdml_ExtChannelAddValue_DataItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<DataItem\n"
	  "%s  Id=\"%hhu\"\n"
	  "%s  DataType=\"%s\"\n"
	  "%s  Length=\"%hu\"/>\n",
	  is, 
	  is, Body.Id,
	  is, Body.DataType,
	  is, Body.Length);
}

void gsdml_ExtChannelAddValue::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ExtChannelAddValue> \n", is);

  for ( unsigned int k = 0; k < DataItem.size(); k++)
    DataItem[k]->print( ind + 2);
  printf( "%s</ExtChannelAddValue>\n", is);
}

void gsdml_ExtChannelDiagItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ExtChannelDiagItem\n"
	  "%s  ErrorType=\"%hu\"\n"
	  "%s  MaintenanceAlarmState=\"%s\"\n"
	  "%s  API=\"%u\"\n"
	  "%s  Name=\"%s\"\n"
	  "%s  Help=\"%s\">\n",
	  is, 
	  is, Body.ErrorType,
	  is, Body.MaintenanceAlarmState,
	  is, Body.API,
	  is, Body.Name.ref,
	  is, Body.Help.ref);

  for ( unsigned int k = 0; k < ExtChannelAddValue.size(); k++)
    ExtChannelAddValue[k]->print( ind + 2);
  printf( "%s</ExtChannelDiagItem>\n", is);
}

void gsdml_ExtChannelDiagList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ExtChannelDiagList>\n", is);

  for ( unsigned int k = 0; k < ExtChannelDiagItem.size(); k++)
    ExtChannelDiagItem[k]->print( ind + 2);
  printf( "%s</ExtChannelDiagList>\n", is);
}

void gsdml_ChannelDiagItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ChannelDiagItem\n"
	  "%s  ErrorType=\"%hu\"\n"
	  "%s  MaintenanceAlarmTate=\"%s\"\n"
	  "%s  API=\"%u\"\n"
	  "%s  Name=\"%s\"\n"
	  "%s  Help=\"%s\">\n",
	  is, 
	  is, Body.ErrorType,
	  is, Body.MaintenanceAlarmState,
	  is, Body.API,
	  is, Body.Name.ref,
	  is, Body.Help.ref);

  if ( ExtChannelDiagList)
    ExtChannelDiagList->print( ind + 2);
  printf( "%s</ChannelDiagItem>\n", is);
}

void gsdml_ChannelDiagList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ChannelDiagList>\n", is);

  for ( unsigned int k = 0; k < ChannelDiagItem.size(); k++)
    ChannelDiagItem[k]->print( ind + 2);
  printf( "%s</ChannelDiagList>\n", is);
}

void gsdml_UnitDiagTypeItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<UnitDiagTypeItem\n"
	  "%s  UserStructureIdentifier=\"%hu\"\n"
	  "%s  API=\"%u\">\n",
	  is, 
	  is, Body.UserStructureIdentifier,
	  is, Body.API);

  for ( unsigned int i = 0; i < Ref.size(); i++)
    Ref[i]->print( ind + 2);
  printf( "%s</UnitDiagTypeItem>\n", is);
}

void gsdml_UnitDiagTypeList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<UnitDiagTypeList>\n", is);

  for ( unsigned int k = 0; k < UnitDiagTypeItem.size(); k++)
    UnitDiagTypeItem[k]->print( ind + 2);
  printf( "%s</UnitDiagTypeList>\n", is);
}

void gsdml_GraphicItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<GraphicItem\n"
	  "%s  ID=\"%s\"\n"
	  "%s  GraphicFile=\"%s\"\n"
	  "%s  Embedded=\"%s\">\n",
	  is, 
	  is, Body.ID,
	  is, Body.GraphicFile,
	  is, Body.Embedded);
}

void gsdml_GraphicsList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<GraphicsList>\n", is);

  for ( unsigned int k = 0; k < GraphicItem.size(); k++)
    GraphicItem[k]->print( ind + 2);
  printf( "%s</GraphicsList>\n", is);
}

void gsdml_CategoryItem::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<CategoryItem\n"
	  "%s  ID=\"%s\"\n"
	  "%s  TextId=\"%s\"\n"
	  "%s  InfoText=\"%s\">\n",
	  is, 
	  is, Body.ID,
	  is, Body.TextId.ref,
	  is, Body.InfoText.ref);
}

void gsdml_CategoryList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<CategoryList>\n", is);

  for ( unsigned int k = 0; k < CategoryItem.size(); k++)
    CategoryItem[k]->print( ind + 2);
  printf( "%s</CategoryList>\n", is);
}

void gsdml_Text::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Text\n"
	  "%s  TextId=\"%s\"\n"
	  "%s  Value=\"%s\">\n",
	  is, 
	  is, Body.TextId,
	  is, Body.Value ? Body.Value : "");
}

void gsdml_PrimaryLanguage::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<PrimaryLanguage>\n", is);

  for ( unsigned int k = 0; k < Text.size(); k++)
    Text[k]->print( ind + 2);
  printf( "%s</PrimaryLanguage>\n", is);
}

void gsdml_Language::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<Language\n"
	  "%s  xml:lang=\"%s\">\n",
	  is, 
	  is, Body.xml_lang);

  for ( unsigned int k = 0; k < Text.size(); k++)
    Text[k]->print( ind + 2);
  printf( "%s</Language>\n", is);
}

void gsdml_ExternalTextList::print( int ind)
{
  char is[] = "                    ";
  is[ind] = 0;

  printf( "%s<ExternalTextList>\n", is);

  if ( PrimaryLanguage)
    PrimaryLanguage->print( ind + 2);
  for ( unsigned int k = 0; k < Language.size(); k++)
    Language[k]->print( ind + 2);
  printf( "%s</ExternalTextList>\n", is);
}

void pn_gsdml::build()
{
  if ( DeviceIdentity)
    DeviceIdentity->build();
  if ( ApplicationProcess)
    ApplicationProcess->build();
}

void pn_gsdml::gsdml_print()
{
  if ( ProfileHeader)
    ProfileHeader->print(0);
  if ( DeviceIdentity)
    DeviceIdentity->print(0);
  if ( DeviceFunction)
    DeviceFunction->print(0);
  if ( ApplicationProcess)
    ApplicationProcess->print(0);
}

void gsdml_SubslotList::build()
{
  for ( unsigned int i = 0; i < SubslotItem.size(); i++)
    if ( strcmp( SubslotItem[i]->Body.TextId.ref, "") != 0) {
      SubslotItem[i]->Body.TextId.p = gsdml->find_text_ref( SubslotItem[i]->Body.TextId.ref);
    if ( SubslotItem[i]->Body.TextId.p == noref)
      gsdml->error_message("TextId not found: \"%s\"", SubslotItem[i]->Body.TextId.ref);
  }  
}

gsdml_SubslotList::~gsdml_SubslotList()
{
  for ( unsigned int i = 0; i < SubslotItem.size(); i++)
    delete SubslotItem[i];
}

gsdml_UseableModules::~gsdml_UseableModules()
{
  for ( unsigned int i = 0; i < ModuleItemRef.size(); i++)
    delete ModuleItemRef[i];
}

void gsdml_BitDataItem::build()
{
  if ( strcmp( Body.TextId.ref, "") != 0) {
    Body.TextId.p = gsdml->find_text_ref( Body.TextId.ref);
    if ( Body.TextId.p == noref)
      gsdml->error_message("TextId not found: \"%s\"", Body.TextId.ref);
  }  
}

void gsdml_DataItem::build()
{
  if ( strcmp( Body.TextId.ref, "") != 0) {
    Body.TextId.p = gsdml->find_text_ref( Body.TextId.ref);
    if ( Body.TextId.p == noref)
      gsdml->error_message("TextId not found: \"%s\"", Body.TextId.ref);
  }  

  for ( unsigned int i = 0; i < BitDataItem.size(); i++)
    BitDataItem[i]->build();
}

gsdml_DataItem::~gsdml_DataItem()
{
  for ( unsigned int i = 0; i < BitDataItem.size(); i++)
    delete BitDataItem[i];
}

void gsdml_Input::build()
{
  for ( unsigned int i = 0; i < DataItem.size(); i++)
    DataItem[i]->build();
}

gsdml_Input::~gsdml_Input()
{
  for ( unsigned int i = 0; i < DataItem.size(); i++)
    delete DataItem[i];
}

void gsdml_Output::build()
{
  for ( unsigned int i = 0; i < DataItem.size(); i++)
    DataItem[i]->build();
}

gsdml_Output::~gsdml_Output()
{
  for ( unsigned int i = 0; i < DataItem.size(); i++)
    delete DataItem[i];
}

void gsdml_IOData::build()
{
  if ( Input)
    Input->build();
  if ( Output)
    Output->build();
}

gsdml_IOData::~gsdml_IOData()
{
  if ( Input)
    delete Input;
  if ( Output)
    delete Output;
}

void gsdml_ParameterRecordDataItem::build()
{
  Body.Name.p = gsdml->find_text_ref( Body.Name.ref);
  if ( Body.Name.p == noref)
    gsdml->error_message("Name not found: \"%s\"", Body.Name.ref);

  for ( unsigned int i = 0; i < Ref.size(); i++) {
    Ref[i]->Body.TextId.p = gsdml->find_text_ref( Ref[i]->Body.TextId.ref);
    if ( Ref[i]->Body.TextId.p == noref)
      gsdml->error_message("TextId not found: \"%s\"", Ref[i]->Body.TextId.ref);
    if ( strcmp( Ref[i]->Body.ValueItemTarget.ref, "") != 0) {
      Ref[i]->Body.ValueItemTarget.p = gsdml->find_value_ref( Ref[i]->Body.ValueItemTarget.ref);
      if ( !Ref[i]->Body.ValueItemTarget.p)
	gsdml->error_message("ValueItemTarget not found: \"%s\"", Ref[i]->Body.ValueItemTarget.ref);
    }
  }
}

gsdml_ParameterRecordDataItem::~gsdml_ParameterRecordDataItem()
{
  for ( unsigned int i = 0; i < Const.size(); i++)
    delete Const[i];
  for ( unsigned int i = 0; i < Ref.size(); i++)
    delete Ref[i];
}

void gsdml_RecordDataList::build()
{
  for ( unsigned int i = 0; i < ParameterRecordDataItem.size(); i++)
    ParameterRecordDataItem[i]->build();
}

gsdml_RecordDataList::~gsdml_RecordDataList()
{
  for ( unsigned int i = 0; i < ParameterRecordDataItem.size(); i++)
    delete ParameterRecordDataItem[i];
  if ( F_ParameterRecordDataItem)
    delete F_ParameterRecordDataItem;
}

void gsdml_Graphics::build()
{
  for ( unsigned int i = 0; i < GraphicItemRef.size(); i++) {
    GraphicItemRef[i]->Body.GraphicItemTarget.p = gsdml->find_graphic_ref( GraphicItemRef[i]->Body.GraphicItemTarget.ref);
    if ( !GraphicItemRef[i]->Body.GraphicItemTarget.p)
      gsdml->error_message("GraphicItemTarget not found: \"%s\"", GraphicItemRef[i]->Body.GraphicItemTarget.ref);
  }
}

gsdml_Graphics::~gsdml_Graphics()
{
  for ( unsigned int i = 0; i < GraphicItemRef.size(); i++)
    delete GraphicItemRef[i];
}

void gsdml_VirtualSubmoduleItem::build()
{
  if ( IOData)
    IOData->build();
  if ( RecordDataList)
    RecordDataList->build();
  if ( ModuleInfo)
    ModuleInfo->build();
  if ( Graphics)
    Graphics->build();      
}

gsdml_VirtualSubmoduleItem::~gsdml_VirtualSubmoduleItem()
{
  if ( IOData)
    delete IOData;
  if ( RecordDataList)
    delete RecordDataList;
  if ( ModuleInfo)
    delete ModuleInfo;
  if ( Graphics)
    delete Graphics;
  if ( IsochroneMode)
    delete IsochroneMode;
}

void gsdml_VirtualSubmoduleList::build()
{
  for ( unsigned int i = 0; i < VirtualSubmoduleItem.size(); i++)
    VirtualSubmoduleItem[i]->build();
}

gsdml_VirtualSubmoduleList::~gsdml_VirtualSubmoduleList()
{
  for ( unsigned int i = 0; i < VirtualSubmoduleItem.size(); i++)
    delete VirtualSubmoduleItem[i];
}

gsdml_General::~gsdml_General()
{
  if ( DCP_FlashOnceSignalUnit)
    delete DCP_FlashOnceSignalUnit;
}

gsdml_InterfaceSubmoduleItem_ApplicationRelations::~gsdml_InterfaceSubmoduleItem_ApplicationRelations()
{
  if ( TimingProperties)
    delete TimingProperties;
  if ( RT_Class3TimingProperties)
    delete RT_Class3TimingProperties;
}

void gsdml_InterfaceSubmoduleItem::build()
{
  Body.TextId.p = gsdml->find_text_ref( Body.TextId.ref);
  if ( Body.TextId.p == noref)
    gsdml->error_message("TextId not found: \"%s\"", Body.TextId.ref);

  if ( RecordDataList)
    RecordDataList->build();
}

gsdml_InterfaceSubmoduleItem::~gsdml_InterfaceSubmoduleItem()
{
  if ( General)
    delete General;
  if ( RecordDataList)
    delete RecordDataList;
  if ( RT_Class3Properties)
    delete RT_Class3Properties;
  if ( SynchronisationMode)
    delete SynchronisationMode;
  if ( ApplicationRelations)
    delete ApplicationRelations;
  if ( MediaRedundancy)
    delete MediaRedundancy;
}

void gsdml_PortSubmoduleItem::build()
{
  Body.TextId.p = gsdml->find_text_ref( Body.TextId.ref);
  if ( Body.TextId.p == noref)
    gsdml->error_message("TextId not found: \"%s\"", Body.TextId.ref);

  if ( RecordDataList)
    RecordDataList->build();
}

gsdml_PortSubmoduleItem::~gsdml_PortSubmoduleItem()
{
  if ( RecordDataList)
    delete RecordDataList;
}

void gsdml_SystemDefinedSubmoduleList::build()
{
  if ( InterfaceSubmoduleItem)
    InterfaceSubmoduleItem->build();
  for ( unsigned int i = 0; i < PortSubmoduleItem.size(); i++)
    PortSubmoduleItem[i]->build();
}

gsdml_SystemDefinedSubmoduleList::~gsdml_SystemDefinedSubmoduleList()
{
  for ( unsigned int i = 0; i < PortSubmoduleItem.size(); i++)
    delete PortSubmoduleItem[i];
}

gsdml_DeviceAccessPointItem_ApplicationRelations::~gsdml_DeviceAccessPointItem_ApplicationRelations()
{
  if ( TimingProperties)
    delete TimingProperties;
}

gsdml_UseableSubmodules::~gsdml_UseableSubmodules()
{
  for ( unsigned int i = 0; i < SubmoduleItemRef.size(); i++)
    delete SubmoduleItemRef[i];
}

void gsdml_SlotList::build()
{
  for ( unsigned int i = 0; i < SlotItem.size(); i++) {
    if ( strcmp( SlotItem[i]->Body.TextId.ref, "") != 0) {
      SlotItem[i]->Body.TextId.p = gsdml->find_text_ref( SlotItem[i]->Body.TextId.ref);
      if ( SlotItem[i]->Body.TextId.p == noref) 
	gsdml->error_message("TextId not found: \"%s\"", SlotItem[i]->Body.TextId.ref);
    }
  }  
}

gsdml_SlotList::~gsdml_SlotList()
{
  for ( unsigned int i = 0; i < SlotItem.size(); i++)
    delete SlotItem[i];
}

gsdml_SlotGroups::~gsdml_SlotGroups()
{
  for ( unsigned int i = 0; i < SlotGroup.size(); i++)
    delete SlotGroup[i];
}

void gsdml_DeviceAccessPointItem::build()
{
  if ( ModuleInfo)
    ModuleInfo->build();
  if ( SubslotList)
    SubslotList->build();
  if ( VirtualSubmoduleList)
    VirtualSubmoduleList->build();
  if ( SystemDefinedSubmoduleList)
    SystemDefinedSubmoduleList->build();
  if ( Graphics)
    Graphics->build();      
  if ( UseableModules)
    UseableModules->build();
  if ( UseableSubmodules)
    UseableSubmodules->build();
  if ( SlotList)
    SlotList->build();
  Body.PhysicalSlots.list = new gsdml_Valuelist( Body.PhysicalSlots.str);
}

gsdml_DeviceAccessPointItem::~gsdml_DeviceAccessPointItem()
{
  if ( ModuleInfo)
    delete ModuleInfo;
  if ( SubslotList)
    delete SubslotList;
  if ( IOConfigData)
    delete IOConfigData;
  if ( UseableModules)
    delete UseableModules;
  if ( VirtualSubmoduleList)
    delete VirtualSubmoduleList;
  if ( Graphics)
    delete Graphics;
  if ( ApplicationRelations)
    delete ApplicationRelations;
  if ( UseableSubmodules)
    delete UseableSubmodules;
  if ( SlotList)
    delete SlotList;
  if ( SlotGroups)
    delete SlotGroups;
  delete Body.PhysicalSlots.list;
}

void gsdml_DeviceAccessPointList::build()
{
  for ( unsigned int i = 0; i < DeviceAccessPointItem.size(); i++)
    DeviceAccessPointItem[i]->build();
}

gsdml_DeviceAccessPointList::~gsdml_DeviceAccessPointList()
{
  for ( unsigned int i = 0; i < DeviceAccessPointItem.size(); i++)
    delete DeviceAccessPointItem[i];
}

void gsdml_ModuleItem::build()
{
  if ( ModuleInfo)
    ModuleInfo->build();
  if ( SubslotList)
    SubslotList->build();
  if ( VirtualSubmoduleList)
    VirtualSubmoduleList->build();
  if ( UseableSubmodules)
    UseableSubmodules->build();
  if ( Graphics)
    Graphics->build();
}

gsdml_ModuleItem::~gsdml_ModuleItem()
{
  if ( ModuleInfo)
    delete ModuleInfo;
  if ( SubslotList)
    delete SubslotList;
  if ( VirtualSubmoduleList)
    delete VirtualSubmoduleList;
  if ( SystemDefinedSubmoduleList)
    delete SystemDefinedSubmoduleList;
  if ( UseableSubmodules)
    delete UseableSubmodules;
  if ( Graphics)
    delete Graphics;
}

void gsdml_ModuleList::build()
{
  for ( unsigned int i = 0; i < ModuleItem.size(); i++)
    ModuleItem[i]->build();
}

gsdml_ModuleList::~gsdml_ModuleList()
{
  for ( unsigned int i = 0; i < ModuleItem.size(); i++)
    delete ModuleItem[i];
}

gsdml_SubmoduleList::~gsdml_SubmoduleList()
{
  for ( unsigned int i = 0; i < SubmoduleItem.size(); i++)
    delete SubmoduleItem[i];
}

void gsdml_Assign::build()
{
  if ( strcmp( Body.TextId.ref, "") != 0) {
      Body.TextId.p = gsdml->find_text_ref( Body.TextId.ref);
      if ( Body.TextId.p == noref)
	gsdml->error_message("TextId not found: \"%s\"", Body.TextId.ref);
  }
}

gsdml_Assignments::~gsdml_Assignments()
{
  for ( unsigned int i = 0; i < Assign.size(); i++)
    delete Assign[i];
}

void gsdml_ValueItem::build()
{
  if ( Assignments) {
    for ( unsigned int i = 0; i < Assignments->Assign.size(); i++)
      Assignments->Assign[i]->build();
  }
}

gsdml_ValueItem::~gsdml_ValueItem()
{
  if ( Assignments)
    delete Assignments;
}

void gsdml_ValueList::build()
{
  for ( unsigned int i = 0; i < ValueItem.size(); i++)
    ValueItem[i]->build();
}

gsdml_ValueList::~gsdml_ValueList()
{
  for ( unsigned int i = 0; i < ValueItem.size(); i++)
    delete ValueItem[i];
}

gsdml_ExtChannelAddValue::~gsdml_ExtChannelAddValue()
{
  for ( unsigned int i = 0; i < DataItem.size(); i++)
    delete DataItem[i];
}

gsdml_ExtChannelDiagItem::~gsdml_ExtChannelDiagItem()
{
  for ( unsigned int i = 0; i < ExtChannelAddValue.size(); i++)
    delete ExtChannelAddValue[i];
}

gsdml_ExtChannelDiagList::~gsdml_ExtChannelDiagList()
{
  for ( unsigned int i = 0; i < ExtChannelDiagItem.size(); i++)
    delete ExtChannelDiagItem[i];
}

gsdml_ChannelDiagItem::~gsdml_ChannelDiagItem()
{
  if ( ExtChannelDiagList)
    delete ExtChannelDiagList;
}

void gsdml_ChannelDiagItem::build()
{
  if ( strcmp( Body.Name.ref, "") != 0) {
    Body.Name.p = gsdml->find_text_ref( Body.Name.ref);
    if ( Body.Name.p == noref) 
      gsdml->error_message("Name not found: \"%s\"", Body.Name.ref);
  }
  if ( strcmp( Body.Help.ref, "") != 0) {
    Body.Help.p = gsdml->find_text_ref( Body.Help.ref);
    if ( Body.Help.p == noref) 
      gsdml->error_message("Help not found: \"%s\"", Body.Help.ref);
  }
}

gsdml_ChannelDiagList::~gsdml_ChannelDiagList()
{
  for ( unsigned int i = 0; i < ChannelDiagItem.size(); i++)
    delete ChannelDiagItem[i];
}

void gsdml_ChannelDiagList::build()
{
  for ( unsigned int i = 0; i < ChannelDiagItem.size(); i++)
    ChannelDiagItem[i]->build();
}

gsdml_UnitDiagTypeItem::~gsdml_UnitDiagTypeItem()
{
  for ( unsigned int i = 0; i < Ref.size(); i++)
    delete Ref[i];
}

gsdml_UnitDiagTypeList::~gsdml_UnitDiagTypeList()
{
  for ( unsigned int i = 0; i < UnitDiagTypeItem.size(); i++)
    delete UnitDiagTypeItem[i];
}

gsdml_GraphicsList::~gsdml_GraphicsList()
{
  for ( unsigned int i = 0; i < GraphicItem.size(); i++)
    delete GraphicItem[i];
}

gsdml_CategoryList::~gsdml_CategoryList()
{
  for ( unsigned int i = 0; i < CategoryItem.size(); i++)
    delete CategoryItem[i];
}

gsdml_Text::~gsdml_Text()
{
  if ( Body.Value)
    free( Body.Value);
}

gsdml_PrimaryLanguage::~gsdml_PrimaryLanguage()
{
  for ( unsigned int i = 0; i < Text.size(); i++)
    delete Text[i];
}

gsdml_Language::~gsdml_Language()
{
  for ( unsigned int i = 0; i < Text.size(); i++)
    delete Text[i];
}

gsdml_ExternalTextList::~gsdml_ExternalTextList()
{
  if ( PrimaryLanguage)
    delete PrimaryLanguage;
  for ( unsigned int i = 0; i < Language.size(); i++)
    delete Language[i];
}

void gsdml_ApplicationProcess::build()
{
  if ( DeviceAccessPointList)
    DeviceAccessPointList->build();
  if ( ModuleList)
    ModuleList->build();
  if ( ValueList)
    ValueList->build();
  if ( ChannelDiagList)
    ChannelDiagList->build();
}

gsdml_ApplicationProcess::~gsdml_ApplicationProcess()
{
  if ( DeviceAccessPointList)
    delete DeviceAccessPointList;
  if ( SubmoduleList)
    delete SubmoduleList;
  if ( ValueList)
    delete ValueList;
  if ( ChannelDiagList)
    delete ChannelDiagList;
  if ( UnitDiagTypeList)
    delete UnitDiagTypeList;
  if ( GraphicsList)
    delete GraphicsList;
  if ( CategoryList)
    delete CategoryList;
  if ( ExternalTextList)
    delete ExternalTextList;
}

void gsdml_DeviceIdentity::build()
{
  if ( strcmp( Body.InfoText.ref, "") != 0) {
      Body.InfoText.p = gsdml->find_text_ref( Body.InfoText.ref);
      if ( Body.InfoText.p == noref) 
	gsdml->error_message("InfoText not found: \"%s\"", Body.InfoText.ref);
  }
}

//
// Unsigned ValueList
//

unsigned int gsdml_ValuelistIterator::begin()
{
  initiated = true;
  if ( valuelist->value.size() == 0)
    current_value = VL_END;
  else {
    current_value = valuelist->value[0].value1;
    current_idx = 0;
  }
  return current_value;
}

unsigned int gsdml_ValuelistIterator::next()
{
  if ( !initiated)
    return VL_END;
  if ( !valuelist->value[current_idx].is_range ||
       valuelist->value[current_idx].value2 == current_value) {
    // Next index
    current_idx++;
    if ( current_idx == valuelist->value.size()) {
      initiated = false;
      return VL_END;
    }
    current_value = valuelist->value[current_idx].value1;
  }
  else {
    // Next in current index
    current_value++;
  }
  return current_value;
}

typedef enum {
  gsdml_eValuelistState_Init,
  gsdml_eValuelistState_Value1,
  gsdml_eValuelistState_Value2,
} gsdml_eValuelistState;

gsdml_Valuelist::gsdml_Valuelist( char *str) : status(PB__SUCCESS)
{
  if ( !parse( str)) {
    status = PB__SYNTAX;
    printf( "GSDML-Parser, syntax error in ValueList, \"%s\"\n", str);
    return;
  }
  sort();
}

void gsdml_Valuelist::sort()
{
  if ( value.size() == 0)
    return;

  for ( unsigned int i = value.size() - 1; i > 0; i--) {
    for ( unsigned int j = 0; j < i; j++) {
      if ( value[i].value1 < value[j].value1) {
	gsdml_ValuelistValue tmp = value[i];
	value[i] = value[j];
	value[j] = tmp;
      }
    }
  }
}

int gsdml_Valuelist::parse( char *str)
{
  unsigned int v1, v2;
  char vstr[40];
  char *s1;
  char *s;
  unsigned int state = gsdml_eValuelistState_Init;

  for ( s = str; *s; s++) {
    switch ( state) { 
    case gsdml_eValuelistState_Init:
      if ( isspace(*s))
	continue;
      if ( isdigit(*s)) {
	s1 = s;
	state = gsdml_eValuelistState_Value1;
      }
      else
	return 0;
      break;
    case gsdml_eValuelistState_Value1: {
      if ( isdigit(*s))
	continue;
      if ( isspace(*s)) {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%u", &v1) != 1)
	  return 0;
	gsdml_ValuelistValue vlv( v1);
	value.push_back( vlv);
	state = gsdml_eValuelistState_Init;
      }
      else if ( *s == '.' && *(s+1) == '.') {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%u", &v1) != 1)
	  return 0;
	state = gsdml_eValuelistState_Value2;
	s1 = s+2;
	s++;
      }
      break;
    }
    case gsdml_eValuelistState_Value2: {
      if ( isdigit(*s))
	continue;
      if ( isspace(*s)) {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%u", &v2) != 1)
	  return 0;
	gsdml_ValuelistValue vlv( v1, v2);
	value.push_back( vlv);
	state = gsdml_eValuelistState_Init;
      }
      break;
    }
    }
  }

  switch ( state) { 
  case gsdml_eValuelistState_Value1: {
    strncpy( vstr, s1, s - s1);
    vstr[s - s1] = 0;
    if ( sscanf( vstr, "%u", &v1) != 1)
      return 0;
    gsdml_ValuelistValue vlv( v1);
    value.push_back(vlv);
    state = gsdml_eValuelistState_Init;
    break;
  }
  case gsdml_eValuelistState_Value2: {
    strncpy( vstr, s1, s - s1);
    vstr[s - s1] = 0;
    if ( sscanf( vstr, "%u", &v2) != 1)
      return 0;
    gsdml_ValuelistValue vlv( v1, v2);
    value.push_back( vlv);
    state = gsdml_eValuelistState_Init;
    break;
  }
  default: ;
  }  
  return 1;
}

bool gsdml_Valuelist::in_list( unsigned int val)
{
  for ( unsigned int i = 0; i < value.size(); i++) {
    if ( value[i].is_range && val >= value[i].value1 && val <= value[i].value2)
      return true;
    else if ( !value[i].is_range && value[i].value1 == val)
      return true;
  }
  return false;
}

int gsdml_SValuelistIterator::begin()
{
  initiated = true;
  if ( valuelist->value.size() == 0)
    current_value = VL_END;
  else {
    current_value = valuelist->value[0].value1;
    current_idx = 0;
  }
  return current_value;
}

int gsdml_SValuelistIterator::next()
{
  if ( !initiated)
    return VL_END;
  if ( !valuelist->value[current_idx].is_range ||
       valuelist->value[current_idx].value2 == current_value) {
    // Next index
    current_idx++;
    if ( current_idx == valuelist->value.size()) {
      initiated = false;
      return VL_END;
    }
    current_value = valuelist->value[current_idx].value1;
  }
  else {
    // Next in current index
    current_value++;
  }
  return current_value;
}

//
// Signed ValueList
//

gsdml_SValuelist::gsdml_SValuelist( char *str) : status(PB__SUCCESS)
{
  if ( !parse( str)) {
    status = PB__SYNTAX;
    printf( "GSDML-Parser, syntax error in ValueList, \"%s\"\n", str);
    return;
  }
  sort();
}

void gsdml_SValuelist::sort()
{
  if ( value.size() == 0)
    return;

  for ( unsigned int i = value.size() - 1; i > 0; i--) {
    for ( unsigned int j = 0; j < i; j++) {
      if ( value[i].value1 < value[j].value1) {
	gsdml_SValuelistValue tmp = value[i];
	value[i] = value[j];
	value[j] = tmp;
      }
    }
  }
}

int gsdml_SValuelist::parse( char *str)
{
  int v1, v2;
  char vstr[40];
  char *s1;
  char *s;
  unsigned int state = gsdml_eValuelistState_Init;

  for ( s = str; *s; s++) {
    switch ( state) { 
    case gsdml_eValuelistState_Init:
      if ( isspace(*s))
	continue;
      if ( isdigit(*s) || *s ==  '-') {
	s1 = s;
	state = gsdml_eValuelistState_Value1;
      }
      else
	return 0;
      break;
    case gsdml_eValuelistState_Value1: {
      if ( isdigit(*s))
	continue;
      if ( isspace(*s)) {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%d", &v1) != 1)
	  return 0;
	gsdml_SValuelistValue vlv( v1);
	value.push_back( vlv);
	state = gsdml_eValuelistState_Init;
      }
      else if ( *s == '.' && *(s+1) == '.') {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%d", &v1) != 1)
	  return 0;
	state = gsdml_eValuelistState_Value2;
	s1 = s+2;
	s++;
      }
      break;
    }
    case gsdml_eValuelistState_Value2: {
      if ( isdigit(*s) || *s == '-')
	continue;
      if ( isspace(*s)) {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%d", &v2) != 1)
	  return 0;
	gsdml_SValuelistValue vlv( v1, v2);
	value.push_back( vlv);
	state = gsdml_eValuelistState_Init;
      }
      break;
    }
    }
  }

  switch ( state) { 
  case gsdml_eValuelistState_Value1: {
    strncpy( vstr, s1, s - s1);
    vstr[s - s1] = 0;
    if ( sscanf( vstr, "%d", &v1) != 1)
      return 0;
    gsdml_SValuelistValue vlv( v1);
    value.push_back(vlv);
    state = gsdml_eValuelistState_Init;
    break;
  }
  case gsdml_eValuelistState_Value2: {
    strncpy( vstr, s1, s - s1);
    vstr[s - s1] = 0;
    if ( sscanf( vstr, "%d", &v2) != 1)
      return 0;
    gsdml_SValuelistValue vlv( v1, v2);
    value.push_back( vlv);
    state = gsdml_eValuelistState_Init;
    break;
  }
  default: ;
  }  
  return 1;
}

bool gsdml_SValuelist::in_list( int val)
{
  for ( unsigned int i = 0; i < value.size(); i++) {
    if ( value[i].is_range && val >= value[i].value1 && val <= value[i].value2)
      return true;
    else if ( !value[i].is_range && value[i].value1 == val)
      return true;
  }
  return false;
}

//
// Float ValueList
//

gsdml_FValuelist::gsdml_FValuelist( char *str) : status(PB__SUCCESS)
{
  if ( !parse( str)) {
    status = PB__SYNTAX;
    printf( "GSDML-Parser, syntax error in ValueList, \"%s\"\n", str);
    return;
  }
  sort();
}

void gsdml_FValuelist::sort()
{
  if ( value.size() == 0)
    return;

  for ( unsigned int i = value.size() - 1; i > 0; i--) {
    for ( unsigned int j = 0; j < i; j++) {
      if ( value[i].value1 < value[j].value1) {
	gsdml_FValuelistValue tmp = value[i];
	value[i] = value[j];
	value[j] = tmp;
      }
    }
  }
}

int gsdml_FValuelist::parse( char *str)
{
  double v1, v2;
  char vstr[40];
  char *s1;
  char *s;
  unsigned int state = gsdml_eValuelistState_Init;

  for ( s = str; *s; s++) {
    switch ( state) { 
    case gsdml_eValuelistState_Init:
      if ( isspace(*s))
	continue;
      if ( isdigit(*s) || *s ==  '-') {
	s1 = s;
	state = gsdml_eValuelistState_Value1;
      }
      else
	return 0;
      break;
    case gsdml_eValuelistState_Value1: {
      if ( isdigit(*s) || *s == 'e' || *s == 'E' || *s == '-' || 
	   (*s == '.' && *(s+1) != '.'))
	continue;
      if ( isspace(*s)) {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%lg", &v1) != 1)
	  return 0;
	gsdml_FValuelistValue vlv( v1);
	value.push_back( vlv);
	state = gsdml_eValuelistState_Init;
      }
      else if ( *s == '.' && *(s+1) == '.') {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%lg", &v1) != 1)
	  return 0;
	state = gsdml_eValuelistState_Value2;
	s1 = s+2;
	s++;
      }
      break;
    }
    case gsdml_eValuelistState_Value2: {
      if ( isdigit(*s) || *s == 'e' || *s == 'E' || *s == '-' || 
	   (*s == '.' && *(s+1) != '.'))
	continue;
      if ( isspace(*s)) {
	strncpy( vstr, s1, s - s1);
	vstr[s - s1] = 0;
	if ( sscanf( vstr, "%lg", &v2) != 1)
	  return 0;
	gsdml_FValuelistValue vlv( v1, v2);
	value.push_back( vlv);
	state = gsdml_eValuelistState_Init;
      }
      break;
    }
    }
  }

  switch ( state) { 
  case gsdml_eValuelistState_Value1: {
    strncpy( vstr, s1, s - s1);
    vstr[s - s1] = 0;
    if ( sscanf( vstr, "%lg", &v1) != 1)
      return 0;
    gsdml_FValuelistValue vlv( v1);
    value.push_back(vlv);
    state = gsdml_eValuelistState_Init;
    break;
  }
  case gsdml_eValuelistState_Value2: {
    strncpy( vstr, s1, s - s1);
    vstr[s - s1] = 0;
    if ( sscanf( vstr, "%lg", &v2) != 1)
      return 0;
    gsdml_FValuelistValue vlv( v1, v2);
    value.push_back( vlv);
    state = gsdml_eValuelistState_Init;
    break;
  }
  default: ;
  }  
  return 1;
}

bool gsdml_FValuelist::in_list( double val)
{
  for ( unsigned int i = 0; i < value.size(); i++) {
    if ( value[i].is_range && val >= value[i].value1 - 100 * FLT_EPSILON && 
	 val <= value[i].value2 + 100 * FLT_EPSILON)
      return true;
    else if ( !value[i].is_range && value[i].value1 == val)
      return true;
  }
  return false;
}


#if 0
int main()
{
  pn_gsdml *gsdml = new pn_gsdml();

  // gsdml.read( "/home/claes/gsdml/GSDML-V1.0-OMRON-GRT1-PNT-20080327.xml");
  gsdml->read( "/home/claes/gsdml/t.xml");
  gsdml->build();

  delete gsdml;
}
#endif
