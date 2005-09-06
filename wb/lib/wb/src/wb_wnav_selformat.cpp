/* 
 * Proview   $Id: wb_wnav_selformat.cpp,v 1.4 2005-09-06 10:43:32 claes Exp $
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



#include <string.h>
#include "wb_wnav_selformat.h"

typedef	struct {char	    *TypeStr;
		pwr_eType   Type;
		pwr_tInt16  Size;
		} wnav_sTypeStr;

static int wnav_type_to_string( 
    pwr_eType type, 
    char *type_buf, 
    int  *size)
{
  int i;

  static const    wnav_sTypeStr	type_table[] = {
    {"Boolean",	pwr_eType_Boolean,  sizeof(pwr_tBoolean)},
    {"Float32",	pwr_eType_Float32,  sizeof(pwr_tFloat32)},
    {"Float64",	pwr_eType_Float64,  sizeof(pwr_tFloat64)},
    {"Char",	pwr_eType_Char,	    sizeof(pwr_tChar)},
    {"Int8",	pwr_eType_Int8,	    sizeof(pwr_tInt8)},
    {"Int16",	pwr_eType_Int16,    sizeof(pwr_tInt16)},
    {"Int32",	pwr_eType_Int32,    sizeof(pwr_tInt32)},
    {"UInt8",	pwr_eType_UInt8,    sizeof(pwr_tUInt8)},
    {"UInt16",	pwr_eType_UInt16,   sizeof(pwr_tUInt16)},
    {"UInt32",	pwr_eType_UInt32,   sizeof(pwr_tUInt32)},
    {"Objid",	pwr_eType_Objid,    sizeof(pwr_tObjid)},
    {"Time",	pwr_eType_Time,     sizeof(pwr_tTime)},
    {"DeltaTime", pwr_eType_DeltaTime, sizeof(pwr_tDeltaTime)},
    {"AttrRef", pwr_eType_AttrRef,  sizeof(pwr_sAttrRef)}
    };

  for (i = 0; i < int(sizeof(type_table)/sizeof(type_table[0])); i++)
  {
    if ( type_table[i].Type == type)
    {
      strcpy(type_buf, type_table[i].TypeStr);
      if (size)
        *size = type_table[i].Size;
      return 1;
    }
  }
    
  if (type == pwr_eType_String)
  {
    strcpy(type_buf, "String");
    if (size)
      *size = 1; /* This is not the real size */
    return 1;
  }
  return 0;
}

pwr_tBoolean wnav_format_selection( ldh_tSesContext ldhses, pwr_sAttrRef attrref, 
				   pwr_tBoolean is_class, pwr_tBoolean is_attr,
				   int select_syntax, int select_volume, 
				   int select_attr, int select_type, char *buff)
{
  pwr_sAttrRef aref;
  int	ret_len, size, sts;
  char name[256];
  char attr_name[128], type_buff[80];
  pwr_tClassId	classid, body_class;    
  pwr_sGraphPlcNode *graph_body;
  char    *p1, *p2;
  char    hyphen[8], dot[8], colon[8];
  char	*name_ptr;
  pwr_tObjid object = attrref.Objid;
  char	*name_p;

  if ( select_syntax == wnav_eSelectionMode_Extern && !select_attr) {
    sts = ldh_ObjidToName( ldhses, object, ldh_eName_Objid, 
					   name, sizeof(name), &ret_len); 
    if (EVEN(sts)) return FALSE;

    strcpy( buff, name);
    return TRUE;
  }
  else if ( select_syntax == wnav_eSelectionMode_Extern && select_attr) {
    sts = ldh_ObjidToName( ldhses, object, ldh_eName_Default,
					   name, sizeof(name), &ret_len); 
    if (EVEN(sts)) return FALSE;
  }
  else if ( select_volume) {
    sts = ldh_AttrRefToName(ldhses, &attrref, 
			    cdh_mName_volume | cdh_mName_object | cdh_mName_attribute, 
			    &name_p, &ret_len); 
    if (EVEN(sts)) return FALSE;
    strcpy( name, name_p);
  }
  else {
    sts = ldh_AttrRefToName(ldhses, &attrref, 
			    cdh_mName_path | cdh_mName_object | cdh_mName_attribute, 
			    &name_p, &ret_len); 
    if (EVEN(sts)) return FALSE;
    strcpy( name, name_p);
  }

  strcpy( buff, name);

  strcpy(hyphen, "-");
  strcpy(dot, ".");
  strcpy(colon, ":");

#if 0
  if (select_syntax == wnav_eSelectionMode_GMS) {
    strcpy(hyphen, "\\-");
    strcpy(dot, "\\.");
    strcpy(colon, "\\:");
  }

  for (i = 0, j = 0; i < ret_len + 1; i++) {
    if (name[i] == '-') {
      strcpy( &buff[j], hyphen);
      j += strlen(hyphen);
    }
    else if (name[i] == ':') {
      strcpy( &buff[j], colon);
      j += strlen(colon);
    }
    else {
      buff[j] = name[i];
      j++;
    }
  }
#endif

  // Fetch and add attribute name if necessary

  aref = attrref;
  if (select_attr && !is_class) {
    sts = ldh_GetAttrRefTid( ldhses, &attrref, &classid);
    if ( EVEN(sts)) return FALSE;

    if ( !is_attr || (cdh_tidIsCid(classid) && !attrref.Flags.b.Array)) {
      // Get the debugparameter if there is one, else add ActualValue  

      sts = ldh_GetClassBody(ldhses, classid, 
	    "GraphPlcNode", &body_class, (char **)&graph_body, &size);
      if ( ODD(sts))
        strcpy(attr_name, graph_body->debugpar);
      else
        strcpy(attr_name, "ActualValue");

      strcat(name, ".");
      strcat(name, attr_name);

      // Check if attribute exists
      sts = ldh_NameToAttrRef(ldhses, name, &aref);
      if (ODD(sts)) {
	sts = ldh_GetAttrRefTid( ldhses, &aref, &classid);
	if ( EVEN(sts)) return FALSE;

        if (select_syntax == wnav_eSelectionMode_Extern && select_attr) {
           sts = ldh_AttrRefToName(ldhses, &aref, 
				ldh_eName_ArefExport,
				&name_ptr, &ret_len); 
           if (EVEN(sts)) return FALSE;
	   strcpy( buff, name_ptr);
           return TRUE;
        }
      }
      strcat(buff, dot);
      strcat(buff, attr_name);
    }


    if ( select_type && !cdh_tidIsCid(classid)) {
      ldh_sAttrRefInfo info;
      int idx;
 
      ldh_GetAttrRefInfo( ldhses, &aref, &info);
      if ( ODD(sts) && 
	   wnav_type_to_string( info.type, type_buff, NULL)) {
        char num[8];
	bool hasIndex = false;

	
        if ( buff[strlen(buff)-1] == ']' && 
	     (p2 = strrchr(buff, '[')) &&
	     sscanf( p2+1, "%d", &idx) == 1) {
	  hasIndex = true;
          *p2 = 0;

	  // Get attrref info for array
	  sts = ldh_NameToAttrRef(ldhses, buff, &aref);
	  if (ODD(sts))
	    sts = ldh_GetAttrRefInfo( ldhses, &aref, &info);
	}
        if ( info.type == pwr_eType_String) {
          sprintf(num, "%d", info.size/info.nElement);  
          strcat(type_buff, num);
        }
        strcat(buff, "##");
        strcat(buff, type_buff);

        // Check if array
        if ( hasIndex) {
          sprintf(&buff[strlen(buff)], "#%d[%d]", info.nElement, idx); 
        }
	else if ( info.nElement > 1) {
          sprintf(&buff[strlen(buff)], "#%d", info.nElement); 
	}
      }
    }
  }
  if ( select_syntax == wnav_eSelectionMode_Extern && select_attr) {
    sts = ldh_NameToAttrRef(ldhses, buff, &aref);
    if (EVEN(sts)) return FALSE;
    sts = ldh_AttrRefToName( ldhses, &aref, 
			ldh_eName_ArefExport, &p1, &size);
    if (EVEN(sts)) return FALSE;
    strcpy( buff, p1);
  }

  return TRUE;
}




