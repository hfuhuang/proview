

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
  pwr_sAttrRef attr_ref;
  int	ret_len, i, j, size, sts;
  char name[256];
  char attr_name[128], type_buff[80];
  pwr_tClassId	classid, body_class;    
  pwr_sGraphPlcNode *graph_body;
  ldh_sParDef	    attr_def;
  char    *p1, *p2;
  char    hyphen[8], dot[8], colon[8];
  char	*name_ptr;
  pwr_tObjid object = attrref.Objid;
  char	*s;

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
    sts = ldh_ObjidToName(ldhses, object, ldh_eName_VolPath, 
					   name, sizeof(name), &ret_len); 
    if (EVEN(sts)) return FALSE;
  }
  else {
    sts = ldh_ObjidToName(ldhses, object, ldh_eName_Hierarchy, 
					   name, sizeof(name), &ret_len); 
    if (EVEN(sts)) return FALSE;
  }

  if (select_syntax == wnav_eSelectionMode_GMS) {
    strcpy(hyphen, "\\-");
    strcpy(dot, "\\.");
    strcpy(colon, "\\:");
  }
  else {
    strcpy(hyphen, "-");
    strcpy(dot, ".");
    strcpy(colon, ":");
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


  // Fetch and add attribute name if necessary
  if (select_attr && !is_class) {
    sts = ldh_GetObjectClass(ldhses, object, &classid);
    if ( EVEN(sts)) return FALSE;

    if ( ! is_attr) {
      // Get the debugparameter if there is one, else add ActualValue  

      sts = ldh_GetClassBody(ldhses, classid, 
	    "GraphPlcNode", &body_class, (char **)&graph_body, &size);
      if ( ODD(sts) )
        strcpy(attr_name, graph_body->debugpar);
      else
        strcpy(attr_name, "ActualValue");

      strcat(name, ".");
      strcat(name, attr_name);

      // Check if attribute exists
      sts = ldh_NameToAttrRef(ldhses, name, &attr_ref);
      if (ODD(sts)) {
        if (select_syntax == wnav_eSelectionMode_Extern && select_attr) {
           sts = ldh_AttrRefToName(ldhses, &attr_ref, 
				ldh_eName_ArefExport,
				&name_ptr, &ret_len); 
           if (EVEN(sts)) return FALSE;
	   strcpy( buff, name_ptr);
           return TRUE;
        }
      }
    }
    else {
      sts = ldh_AttrRefToName(ldhses, &attrref, 
				ldh_eName_Aref,
        			&name_ptr, &ret_len); 
      if ( EVEN(sts)) return FALSE;
      if ( (s = strrchr( name_ptr, '.')))
	strcpy( attr_name, s + 1);
      else
	strcpy( attr_name, "");
    }
    strcat(buff, dot);
    strcat(buff, attr_name);

    if (select_type) {
      // If attribute is an array element 
      // Get attribute definition for the array.

      if ( (p1 = strstr(attr_name, "[")))
        *p1 = '\0';
		
      sts = ldh_GetAttrDef(ldhses, classid, "RtBody", 
						     attr_name, &attr_def);
      if (EVEN(sts))
        sts = ldh_GetAttrDef(ldhses, classid, "SysBody", 
						     attr_name, &attr_def);
      if ( ODD(sts) && 
             wnav_type_to_string( attr_def.Par->Input.Info.Type, type_buff, NULL)) {
        char num[8];

        if ( (p2 = strstr(buff, "[")))	
          *p2 = '\0';

        if (attr_def.Par->Input.Info.Type == pwr_eType_String) {
          sprintf(num, "%d", attr_def.Par->Input.Info.Size/attr_def.Par->Input.Info.Elements);  
          strcat(type_buff, num);
        }
        strcat(buff, "##");
        strcat(buff, type_buff);

        // Check if array
        if (p1) {
          sprintf(&buff[strlen(buff)], "#%d", 
		       attr_def.Par->Input.Info.Elements); 
          *p1 = '[';
          strcat(buff, p1);
        }
	else if ( attr_def.Par->Input.Info.Elements > 1) {
          sprintf(&buff[strlen(buff)], "#%d", 
		       attr_def.Par->Input.Info.Elements); 
	}
      }
    }
  }
  if ( select_syntax == wnav_eSelectionMode_Extern && select_attr) {
    sts = ldh_NameToAttrRef(ldhses, buff, &attr_ref);
    if (EVEN(sts)) return FALSE;
    sts = ldh_AttrRefToName( ldhses, &attr_ref, 
			ldh_eName_ArefExport, &p1, &size);
    if (EVEN(sts)) return FALSE;
    strcpy( buff, p1);
  }

  return TRUE;
}




