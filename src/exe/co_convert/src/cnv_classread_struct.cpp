

#include <iostream.h>
#include <fstream.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern "C" {
#include "pwr.h"
#include "co_dcli.h"
#include "co_cdh.h"
}
#include "cnv_classread.h"

typedef struct {
	char		name[40];
	unsigned int	id;
	} struct_tCixArray;

int ClassRead::struct_init()
{
  char struct_filename[200];
  char fname[200];
  int sts;

  struct_get_filename( struct_filename);
  strcpy( fname, dir);
  strcat( fname, struct_filename);
  fp_struct.open( fname);

  if ( strcmp( low(volume_name), "pwrb") == 0)
    sprintf( fname, "pwr_%sclasses_h", "base");
  else if ( strcmp( low(volume_name), "pwrs") == 0)
    sprintf( fname, "pwr_%sclasses_h", "system");
  else
    sprintf( fname, "pwr_%sclasses_h", low(volume_name));

  sts = struct_volname_to_id();
  if ( sts == 0)
  {
    // printf( "** Can't get volume id\n");
    // exit(0);
  }

  fp_struct <<
"/*	Generated by co_convert 		*/" << endl <<
"#ifndef " << fname << endl <<
"#define " << fname << endl <<
endl <<
"#ifndef pwr_class_h" << endl <<
"#include \"pwr_class.h\"" << endl <<
"#endif" << endl <<
endl <<
endl;

  return 1;
}

int ClassRead::struct_close()
{

  fp_struct <<
"#endif" << endl;

  fp_struct.close();

  return 1;
}

int ClassRead::struct_class()
{

  // Open class struct file
  char struct_filename[200];
  char fname[200];
  char volume_name_low[40];

  strcpy( volume_name_low, low(volume_name));

  struct_get_filename( struct_filename);
  sprintf( fname, "%s%s_c_%s.h", dir, volume_name_low, low(class_name));
  if ( !common_structfile_only) {
    cstruc = new CnvFile();
    cstruc->f.open( fname);
  }
  struct_class_open = 1;
  sprintf( fname, "%s_c_%s_h", volume_name_low, low(class_name));

  if ( !common_structfile_only)
    cstruc->f <<
"/*	Generated by co_convert 		*/" << endl << endl <<
"#ifndef " << fname << endl <<
"#define " << fname << endl <<
endl <<
"#ifndef pwr_class_h" << endl <<
"#include \"pwr_class.h\"" << endl <<
"#endif" << endl <<
endl <<
endl;

  // Add into index file
  struct_cclass_written = 0;
  struct_filler_cnt = 0;

  return 1;
}

int ClassRead::struct_body()
{
  int sts;
  char cclass_name[80];
  unsigned int cix;
  pwr_tClassId cid;
  char struct_name[80];

  attr_count = 0;

  if ( strcmp( low( body_name), "devbody") == 0)
    strcpy( struct_name, "pwr_sdClass_");
  else
    strcpy( struct_name, "pwr_sClass_");
  if ( strcmp( body_structname, "") == 0)
    strcat( struct_name, class_name);
  else
    strcat( struct_name, body_structname);

  // For systemclasses classindex is defined as a pwr_eCix...
  if ( strncmp( class_id, "pwr_eCix_", strlen("pwr_eCix_")) == 0)
  {
    sts = struct_cixstr_to_classid( class_id, &cid);
    if ( EVEN(sts))
    {
      printf("Error, unknown classid %s", class_id);
      return sts;
    }
  }
  else
  {
    // Classindex is a number
    sts = sscanf( class_id, "%d", &cix);
    if ( sts != 1)
    {
      printf("Error, unknown classid %s", class_id);
      return 0;
    }
    struct_cix_to_classid( cix, &cid);
  }
  if ( strcmp( body_structname, "") == 0)
    strcpy( cclass_name, class_name);
  else
    strcpy( cclass_name, body_structname);

  if ( !struct_cclass_written)
  {
    fp_struct <<
endl <<
"#ifndef pwr_cClass_" << cclass_name << endl <<
"#define pwr_cClass_" << cclass_name << " " << cid << endl <<
endl;
    if ( !common_structfile_only)
      cstruc->f <<
endl <<
"#ifndef pwr_cClass_" << cclass_name << endl <<
"#define pwr_cClass_" << cclass_name << " " << cid << endl <<
endl;
    struct_cclass_written = 1;
  }

  if ( strcmp( low( body_name), "rtbody") == 0 &&
       class_devonly)
  { 
    fp_struct <<
"/*  Class: " << class_name << endl <<
"    Body:  " << body_name << endl <<
"    Body is virtual" << endl <<
"*/" << endl << endl;
     return 1;
  }

  fp_struct <<
"/** Class: " << class_name << endl <<
"    Body:  " << body_name << endl <<
"    @Aref " << class_name << " " << struct_name << endl <<
"*/" << endl;
  if ( !common_structfile_only)
    cstruc->f <<
"/*  Body:  " << body_name << "  */" << endl;

  fp_struct << 
endl <<
"typedef struct {" << endl;
  if ( !common_structfile_only)
    cstruc->f << 
endl <<
"typedef struct {" << endl;

  return 1;
}

int ClassRead::struct_body_close()
{
  char struct_name[80];

  if ( strcmp( low( body_name), "rtbody") == 0 &&
       class_devonly)
    return 1;

  if ( attr_count == 0)
  {
    // Write a dummy element...
    fp_struct <<
"  int dummy;" << endl;
    if ( !common_structfile_only)
      cstruc->f <<
"  int dummy;" << endl;
  }

  if ( strcmp( low( body_name), "devbody") == 0)
    strcpy( struct_name, "pwr_sdClass_");
  else
    strcpy( struct_name, "pwr_sClass_");
  if ( strcmp( body_structname, "") == 0)
    strcat( struct_name, class_name);
  else
    strcat( struct_name, body_structname);

  fp_struct <<
"} " << struct_name << ";"  << endl;
  if ( !common_structfile_only)
    cstruc->f <<
"} " << struct_name << ";"  << endl;

  fp_struct << 
endl;
  if ( !common_structfile_only)
    cstruc->f << 
endl;

  return 1;
}

int ClassRead::struct_class_close()
{
  if ( !struct_cclass_written)
  {
    fp_struct <<
endl <<
"#ifndef pwr_cClass_" << class_name << endl <<
"#define pwr_cClass_" << class_name << " " << class_id << endl <<
endl;
    if ( !common_structfile_only)
      cstruc->f <<
endl <<
"#ifndef pwr_cClass_" << class_name << endl <<
"#define pwr_cClass_" << class_name << " " << class_id << endl <<
endl;

    struct_cclass_written = 1;
  }

  // endif pwr_cClass...
  if ( struct_class_open)
    fp_struct <<
"#endif" << endl <<
endl;

  if ( !common_structfile_only && struct_class_open)
    cstruc->f <<
"#endif" << endl <<
endl;

  // Close class structfile
  if ( !common_structfile_only && struct_class_open)
    cstruc->f <<
"#endif" << endl <<
endl;
  if ( !common_structfile_only && struct_class_open) {
    cstruc->f.close();
    delete cstruc;
    struct_class_open = 0;
  }
  return 1;
}

int ClassRead::struct_attribute()
{
  int i;
  int fill;
  int size;
  int elements;
  char type_name[80];
  char pgmname[80];
  int sts;

  if ( strcmp( low( body_name), "rtbody") == 0 &&
       class_devonly)
    return 1;

//  if ( attr_rtvirtual)
//    return 1;

  if ( strcmp( attr_pgmname, "") == 0)
    strcpy( pgmname, attr_name);
  else
    strcpy( pgmname, attr_pgmname);

  if ( strncmp( low(attr_typeref), "pwr_etype_", strlen("pwr_etype_")) == 0)
    strcpy( attr_typeref, &attr_typeref[strlen("pwr_etype_")]);

  if ( strcmp( low(attr_type), "buffer") == 0)
  {
    strcpy( type_name, "pwr_s");
    if ( attr_typeref[11] == '$')
      strcat( type_name, &attr_typeref[12]);
    else
      strcat( type_name, &attr_typeref[11]);
  }
  else if ( strcmp( low(attr_typeref), "attrref") == 0)
  {
    strcpy( type_name, "pwr_s");
    strcat( type_name, attr_typeref);
  }
  else
  {
    strcpy( type_name, "pwr_t");
    strcat( type_name, attr_typeref);
  }
  // For backward compatibility...
  if ( strcmp( type_name, "pwr_tObjId") == 0)
    strcpy( type_name, "pwr_tObjid");

  // Check type for baseclasses
  if ( strcmp( low(volume_name), "pwrb") == 0)
  {
    sts = struct_check_typename( type_name); 
    if ( EVEN(sts))
    {
      printf("Error, unknown attribute type '%s'", type_name);
      return sts;
    }
  }

  if ( strcmp( low(attr_type), "input") == 0)
  {
    if ( attr_array && attr_pointer)
    {
      fp_struct <<
"  " << type_name;
      if ( !common_structfile_only)
        cstruc->f <<
"  " << type_name;
      for ( i = 0; i < int(36 - strlen(type_name)); i++)
      {
        fp_struct << ' ';
        if ( !common_structfile_only)
          cstruc->f << ' ';
      }
      fp_struct << "**" << pgmname << "P[" << attr_elements << "];" << endl;
      if ( !common_structfile_only)
        cstruc->f << "**" << pgmname << "P[" << attr_elements << "];" << endl;
    }
    else if ( attr_array)
    {
      fp_struct <<
"  " << type_name;
      if ( !common_structfile_only)
        cstruc->f <<
"  " << type_name;
      for ( i = 0; i < int(36 - strlen(type_name)); i++)
      {
        fp_struct << ' ';
        if ( !common_structfile_only)
          cstruc->f << ' ';
      }
      fp_struct << "*" << pgmname << "P[" << attr_elements << "];" << endl;
      if ( !common_structfile_only)
        cstruc->f << "*" << pgmname << "P[" << attr_elements << "];" << endl;
    }
    else if ( attr_pointer)
    {
      fp_struct <<
"  " << type_name;
      if ( !common_structfile_only)
        cstruc->f <<
"  " << type_name;
      for ( i = 0; i < int(36 - strlen(type_name)); i++)
      {
        fp_struct << ' ';
        if ( !common_structfile_only)
          cstruc->f << ' ';
      }
      fp_struct << "**" << pgmname << "P;" << endl;
      if ( !common_structfile_only)
        cstruc->f << "**" << pgmname << "P;" << endl;
    }
    else
    {
      fp_struct <<
"  " << type_name;
      if ( !common_structfile_only)
        cstruc->f <<
"  " << type_name;
      for ( i = 0; i < int(36 - strlen(type_name)); i++)
      {
        fp_struct << ' ';
        if ( !common_structfile_only)
          cstruc->f << ' ';
      }
      fp_struct << "*" << pgmname << "P;" << endl;
      if ( !common_structfile_only)
        cstruc->f << "*" << pgmname << "P;" << endl;
    }
  }

  if ( attr_array && attr_pointer)
  {
    fp_struct <<
"  " << type_name;
    if ( !common_structfile_only)
      cstruc->f <<
"  " << type_name;
    for ( i = 0; i < int(36 - strlen(type_name)); i++)
    {
      fp_struct << ' ';
      if ( !common_structfile_only)
        cstruc->f << ' ';
    }
    fp_struct << "*" << pgmname << "[" << attr_elements << "];" << endl;
    if ( !common_structfile_only)
      cstruc->f << "*" << pgmname << "[" << attr_elements << "];" << endl;
  }
  else if ( attr_array)
  {
    fp_struct <<
"  " << type_name;
    if ( !common_structfile_only)
      cstruc->f <<
"  " << type_name;
    for ( i = 0; i < int(36 - strlen(type_name)); i++)
    {
      fp_struct << ' ';
      if ( !common_structfile_only)
        cstruc->f << ' ';
    }
    fp_struct << pgmname << "[" << attr_elements << "];" << endl;
    if ( !common_structfile_only)
      cstruc->f << pgmname << "[" << attr_elements << "];" << endl;
  }
  else if ( attr_pointer)
  {
    fp_struct <<
"  " << type_name;
    if ( !common_structfile_only)
      cstruc->f <<
"  " << type_name;
    for ( i = 0; i < int(36 - strlen(type_name)); i++)
    {
      fp_struct << ' ';
      if ( !common_structfile_only)
        cstruc->f << ' ';
    }
    fp_struct << "*" << pgmname << ";" << endl;
    if ( !common_structfile_only)
      cstruc->f << "*" << pgmname << ";" << endl;
  }
  else
  {
    fp_struct <<
"  " << type_name;
    if ( !common_structfile_only)
      cstruc->f <<
"  " << type_name;
    for ( i = 0; i < int(36 - strlen(type_name)); i++)
    {
      fp_struct << ' ';
      if ( !common_structfile_only)
        cstruc->f << ' ';
    }
    fp_struct << pgmname << ";" << endl;
    if ( !common_structfile_only)
      cstruc->f << pgmname << ";" << endl;
  }

  if ( attr_pointer)
    size = 4;
  else if ( strcmp( low(attr_typeref), "uint8") == 0 ||
       strcmp( low(attr_typeref), "int8") == 0 ||
       strcmp( low(attr_typeref), "char") == 0)
    size = 1;
  else if ( strcmp( low(attr_typeref), "uint16") == 0 ||
            strcmp( low(attr_typeref), "int16") == 0)
    size = 2;
  else
    size = 4;

  if ( attr_elem == 0)
    elements = 1;
  else
    elements = attr_elem;

  if ( size < 4)
  {
    fill = 4 - ((elements * size) % 4);
    if ( fill == 4)
      fill = 0;
  }
  else
    fill = 0;

  if ( fill)
  {
    fp_struct <<
"  " << "char";
    if ( !common_structfile_only)
      cstruc->f <<
"  " << "char";
    for ( i = 0; i < int(36 - strlen("char")); i++)
    {
      fp_struct << ' ';
      if ( !common_structfile_only)
        cstruc->f << ' ';
    }
    if ( fill == 1)
    {
      fp_struct << "filler_" << struct_filler_cnt << ";" << endl;
      if ( !common_structfile_only)
        cstruc->f << "filler_" << struct_filler_cnt << ";" << endl;
    }
    else  
    {
      fp_struct << "filler_" << struct_filler_cnt << "[" << fill << "];" << endl;
      if ( !common_structfile_only)
        cstruc->f << "filler_" << struct_filler_cnt << "[" << fill << "];" << endl;
    }
    struct_filler_cnt++;
  }
  attr_count++;
  return 1;
}

int ClassRead::struct_typedef()
{
  if ( strcmp( typedef_typeref, "String") == 0)
    fp_struct << 
"typedef char pwr_t" << typedef_name << "[" << typedef_elements << "];" << endl << endl;
  else
    fp_struct <<
"typedef pwr_t" << typedef_typeref << " pwr_t" << typedef_name << "[" << typedef_elements << "];" << endl << endl;


  return 1;
}

int ClassRead::struct_volname_to_id()
{
  FILE *fp;
  char fname[200];
  char line[400];
  char	line_part[4][80];
  int nr;
  int sts;
  unsigned int vid_0, vid_1, vid_2, vid_3;

  strcpy( struct_volid, "");

  strcpy( fname, source_dir);
  strcat( fname, low(volume_name));
  strcat( fname, "_v.wb_load");

  fp = fopen( fname, "r");
  if ( !fp) {
    strcpy( fname, source_dir);
    strcat( fname, "usertypes.wb_load");
    fp = fopen( fname, "r");
    if ( !fp) {
      strcpy( fname, source_dir);
      strcat( fname, "userclasses.wb_load");
      fp = fopen( fname, "r");
      if ( !fp) {
	printf( "** Unable to open volume wb_load file\n");
	return 0;
      }
    }
  }

  while( 1)
  {
    sts = read_line( line, sizeof(line), fp);
    if ( !sts)
      break;
    else
    {
      remove_spaces( line, line);
      if ( strcmp( line, "") == 0)
        continue;

      if ( line[0] == '!')
        continue;

      nr = dcli_parse( line, " 	=", "", (char *)line_part,
                	sizeof( line_part) / sizeof( line_part[0]), 
			sizeof( line_part[0]), 0);
      if ( strcmp( low( line_part[0]), "volume") == 0)
      {
        if ( nr > 3)
        {
          strcpy( struct_volid, line_part[3]);
          break;
        }
      }
    }  
  }
  fclose( fp);

  if ( strcmp( struct_volid, "") == 0)
    return 0;

  if (sscanf( struct_volid, "%d.%d.%d.%d", &vid_3, &vid_2, &vid_1, &vid_0) != 4)
    return 0;

  struct_vid_0 = vid_0;
  struct_vid_1 = vid_1;

  return 1;
}

void ClassRead::struct_cix_to_classid( unsigned int cix, pwr_tClassId *cid)
{
  cdh_uTypeId		lcid;

  lcid.pwr = pwr_cNClassId;
  lcid.c.vid_1 = struct_vid_1;
  lcid.c.vid_0 = struct_vid_0;
  lcid.c.cix = cix;

  *cid = lcid.pwr;
}

int ClassRead::struct_cixstr_to_classid( char *cix_str, pwr_tClassId *cid)
{
  int found;
  struct_tCixArray	*cix_p;
  struct_tCixArray cix_array[] = {
	{ "pwr_eCix_ClassDef",		pwr_eCix_ClassDef},
	{ "pwr_eCix_Type",		pwr_eCix_Type},
	{ "pwr_eCix_TypeDef",		pwr_eCix_TypeDef},
	{ "pwr_eCix_ObjBodyDef",	pwr_eCix_ObjBodyDef},
	{ "pwr_eCix_Param",		pwr_eCix_Param},
	{ "pwr_eCix_Input",		pwr_eCix_Input},
	{ "pwr_eCix_Output",		pwr_eCix_Output},
	{ "pwr_eCix_Intern",		pwr_eCix_Intern},
	{ "pwr_eCix_Buffer",		pwr_eCix_Buffer},
	{ "pwr_eCix_ObjXRef",		pwr_eCix_ObjXRef},
	{ "pwr_eCix_Layout",		pwr_eCix_Layout},
	{ "pwr_eCix_Group",		pwr_eCix_Group},
	{ "pwr_eCix_GroupRef",		pwr_eCix_GroupRef},
	{ "pwr_eCix_TypeHier",		pwr_eCix_TypeHier},
	{ "pwr_eCix_ClassHier",		pwr_eCix_ClassHier},
	{ "pwr_eCix_ModHier",		pwr_eCix_ModHier},
	{ "pwr_eCix_PlantHier",		pwr_eCix_PlantHier},
	{ "pwr_eCix_PlcProgram",	pwr_eCix_PlcProgram},
	{ "pwr_eCix_PlcWindow",		pwr_eCix_PlcWindow},
	{ "pwr_eCix_PlcNode",		pwr_eCix_PlcNode},
	{ "pwr_eCix_PlcConnection",	pwr_eCix_PlcConnection},
	{ "pwr_eCix_Point",		pwr_eCix_Point},
	{ "pwr_eCix_GraphPlcProgram",	pwr_eCix_GraphPlcProgram},
	{ "pwr_eCix_GraphPlcWindow",	pwr_eCix_GraphPlcWindow},
	{ "pwr_eCix_GraphPlcNode",	pwr_eCix_GraphPlcNode},
	{ "pwr_eCix_GraphPlcConnection", pwr_eCix_GraphPlcConnection},
	{ "pwr_eCix_PlcPgm",		pwr_eCix_PlcPgm},
	{ "pwr_eCix_Hierarchy",		pwr_eCix_Hierarchy},
	{ "pwr_eCix_NodeHier",		pwr_eCix_NodeHier},
	{ "pwr_eCix_PgmDef",		pwr_eCix_PgmDef},
	{ "pwr_eCix_Node",		pwr_eCix_Node},
	{ "pwr_eCix_Appl",		pwr_eCix_Appl},
	{ "pwr_eCix_System",		pwr_eCix_System},
	{ "pwr_eCix_LibHier",		pwr_eCix_LibHier},
	{ "pwr_eCix_DocHier",		pwr_eCix_DocHier},
	{ "pwr_eCix_AttrXRef",		pwr_eCix_AttrXRef},
	{ "pwr_eCix_Menu",		pwr_eCix_Menu},
	{ "pwr_eCix_MenuSeparator", 	pwr_eCix_MenuSeparator},
	{ "pwr_eCix_MenuCascade",	pwr_eCix_MenuCascade},
	{ "pwr_eCix_MenuButton",	pwr_eCix_MenuButton},
	{ "pwr_eCix_Object",		pwr_eCix_Object},
	{ "pwr_eCix_DbCallBack",	pwr_eCix_DbCallBack},
	{ "pwr_eCix_Alias",		pwr_eCix_Alias},
	{ "pwr_eCix_RootVolume",	pwr_eCix_RootVolume},
	{ "pwr_eCix_SubVolume",		pwr_eCix_SubVolume},
	{ "pwr_eCix_SharedVolume",	pwr_eCix_SharedVolume},
	{ "pwr_eCix_DynamicVolume",	pwr_eCix_DynamicVolume},
	{ "pwr_eCix_SystemVolume",	pwr_eCix_SystemVolume},
	{ "pwr_eCix_ClassVolume",	pwr_eCix_ClassVolume},
	{ "pwr_eCix_WorkBenchVolume",	pwr_eCix_WorkBenchVolume},
	{ "pwr_eCix_DirectoryVolume",	pwr_eCix_DirectoryVolume},
	{ "pwr_eCix_CreateVolume",	pwr_eCix_CreateVolume},
	{ "pwr_eCix_MountVolume",	pwr_eCix_MountVolume},
	{ "pwr_eCix_MountObject",	pwr_eCix_MountObject},
	{ "pwr_eCix_RtMenu",		pwr_eCix_RtMenu},
	{ "pwr_eCix_VolatileVolume",		pwr_eCix_VolatileVolume},
	{ "", 0}};

  found = 0;
  for ( cix_p = cix_array; strcmp( cix_p->name, "") != 0; cix_p++)
  {
    if ( strcmp( cix_str, cix_p->name) == 0)
    {
      found = 1;
      break;
    }
  }
  if ( !found)
    return 0;

  struct_cix_to_classid( cix_p->id, cid);
  return 1;
}

void ClassRead::struct_get_filename( char *struct_file)
{
  strcpy( struct_file, "pwr_");

  if ( strcmp( low(volume_name), "pwrb") == 0)
    strcat( struct_file, "base");
  else if ( strcmp( low(volume_name), "pwrs") == 0)
    // To separate from pwr_systemclasses.h 
    strcat( struct_file, "system");
  else
    strcat( struct_file, low(volume_name));

  strcat( struct_file, "classes.h");
}

int ClassRead::struct_check_typename( char *type_name)
{
  char		*name;
  char		valid_names[][40] = {
	"pwr_tAddress",
	"pwr_tBit",
	"pwr_tBitMask",
	"pwr_tBoolean",
	"pwr_tFloat32",
	"pwr_tFloat64",
	"pwr_tGeneration",
	"pwr_tChar",
	"pwr_tString",
	"pwr_tText",
	"pwr_tInt8",
	"pwr_tInt16",
	"pwr_tInt32",
	"pwr_tStatus",
	"pwr_tInt64",
	"pwr_tUInt64",
	"pwr_tUInt8",
	"pwr_tUInt16",
	"pwr_tUInt32",
	"pwr_tVolumeId",
	"pwr_tObjectIx",
	"pwr_tMask",
	"pwr_tEnum",
	"pwr_tClassId",
	"pwr_tTypeId",
	"pwr_tVersion",
	"pwr_tPwrVersion",
	"pwr_tProjVersion",
	"pwr_tUserId",
	"pwr_tDbId",
	"pwr_tNodeId",
	"pwr_tNodeIndex",
	"pwr_tDlid",
	"pwr_tSubid",
	"pwr_tTime",
	"pwr_tDeltaTime",
	"pwr_tRefId",
	"pwr_tObjid",
	"pwr_sAttrRef",
	"pwr_sPlcNode",
	"pwr_sPlcConnection",
	"pwr_sPlcWindow",
	"pwr_sPlcProgram",
	"pwr_tString256",
	"pwr_tString132",
	"pwr_tString80",
	"pwr_tString40",
	"pwr_tString32",
	"pwr_tString16",
	"pwr_tString8",
	"pwr_tString1",
	"pwr_tText1024",
	"pwr_tURL",
	""};

  for ( name = valid_names[0]; strcmp(name,"") != 0; 
		name += sizeof(valid_names[0]))
  {
    if ( strcmp( name, type_name) == 0)
      return 1;
  }
  return 0;
}
