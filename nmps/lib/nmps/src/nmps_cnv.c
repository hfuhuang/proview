/* 
 * Proview   $Id: nmps_cnv.c,v 1.1 2006-01-12 05:57:43 claes Exp $
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pwr.h"
#include "pwr_class.h"
#include "co_cdh.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "pwr_nmpsclasses.h"
#include "nmps_cnv.h"
#include "rs_cnv_msg.h"
#include "co_dcli.h"

#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))

#define CNV_CONVDEFTYPE_OBJ 	0
#define CNV_CONVDEFTYPE_FILE 	1
#define CNV_CONVDEFTYPE_CLASS 	2

#define CNV_CONVDEF_SIZE 200

static int	cnv_remove_blank( char *out_str, char *in_str);
static int	cnv_string_to_partype( 	char 		*string, 
					int		args,
					char		*third_arg,
					cnv_eParType 	*type,
					int		*size,	
					char		*format);
static int	cnv_convtable_add( 	cnv_eParType	to_type,
					cnv_eParType	from_type,
					int	to_offset,
					int	from_offset,
					int	to_size,
					int	from_size,
					char	*to_format,
					char	*from_format,
					cnv_t_conv_table *conv_table,
					int	*conv_table_count);
static int	cnv_convtable_compress(	cnv_t_conv_table *conv_table,
					int		*conv_table_count);
static int cnv_read_line(	char	*line,
				int	maxsize,
				FILE	*file);
static int	cnv_ReadConvTable( 	char			*filename,
					pwr_sClass_ConvDef 	**convdef);
static int	cnv_GetConvTableFromClass( 
					pwr_tObjid		class,
					pwr_sClass_ConvDef 	**convdef);

int	cnv_CreateConvTable(
		pwr_sClass_ConvDef	*to_convdef,
		pwr_sClass_ConvDef	*from_convdef,
		cnv_t_conv_table	*conv_table,
		int			*conv_table_count)
{
	char	out_str[5][80];
	int	nr;
	int	i, j;
	cnv_eParType	to_type[CNV_CONVDEF_SIZE];
	int		to_offset[CNV_CONVDEF_SIZE];
	int		to_size[CNV_CONVDEF_SIZE];
	pwr_tString80	to_param[CNV_CONVDEF_SIZE];
	pwr_tString80	to_format[CNV_CONVDEF_SIZE];
	cnv_eParType	from_type[CNV_CONVDEF_SIZE];
	int		from_offset[CNV_CONVDEF_SIZE];
	int		from_size[CNV_CONVDEF_SIZE];
	pwr_tString80	from_param[CNV_CONVDEF_SIZE];
	pwr_tString80	from_format[CNV_CONVDEF_SIZE];
	int		from_convdef_count;
	int		to_convdef_count;
	char		type_str[80];
	int		sts;

	/* Search trough parameternames in the to convdef
	   and match them with the from convdef */
	from_convdef_count = 0;
	for ( i = 0; i < CNV_CONVDEF_SIZE; i++)
	{
	  cnv_remove_blank( from_convdef->Param[i], from_convdef->Param[i]);
	  if ( from_convdef->Param[i][0] == 0 )
	    break;
	  if ( from_convdef->Param[i][0] == '!')
	    continue;

	  nr = dcli_parse( from_convdef->Param[i], " 	", "", (char *)out_str,
		sizeof( out_str) / sizeof( out_str[0]), sizeof( out_str[0]), 0);
	  if ( nr != 2 && nr != 3)
	  {
	    errh_CErrLog( CNV__SYNTAX, errh_ErrArgAF( from_convdef->Param[i]), NULL);
	    return CNV__SYNTAX;
	  }

	  /* First arg is type */
	  cdh_ToUpper( type_str, out_str[0]);
	  /* Second arg i parameter name */
	  cdh_ToUpper( from_param[ from_convdef_count], out_str[1]);
	  strcpy( from_format[ from_convdef_count], "");
	  sts = cnv_string_to_partype( type_str, nr, out_str[2],
				&from_type[ from_convdef_count],
				&from_size[ from_convdef_count],
				from_format[ from_convdef_count]);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( sts, errh_ErrArgAF( to_convdef->Param[i]), NULL);
	    return sts;
	  }
	  if ( from_convdef_count == 0)
	    from_offset[ from_convdef_count] = 0;
	  else
	    from_offset[ from_convdef_count] = 
		from_offset[ from_convdef_count - 1] + 
		from_size[ from_convdef_count - 1];
	  from_convdef_count++;
	}

	to_convdef_count = 0;
	for ( i = 0; i < CNV_CONVDEF_SIZE; i++)
	{
	  cnv_remove_blank( to_convdef->Param[i], to_convdef->Param[i]);
	  if ( to_convdef->Param[i][0] == 0 )
	    break;
	  if ( to_convdef->Param[i][0] == '!')
	    continue;

	  nr = dcli_parse( to_convdef->Param[i], " 	", "", (char *)out_str,
		sizeof( out_str) / sizeof( out_str[0]), sizeof( out_str[0]), 0);
	  if ( nr != 2 && nr != 3)
	  {
	    errh_CErrLog( CNV__SYNTAX, errh_ErrArgAF( to_convdef->Param[i]), NULL);
	    return CNV__SYNTAX;
	  }

	  cdh_ToUpper( type_str, out_str[0]);
	  strcpy( to_format[ from_convdef_count], "");
	  sts = cnv_string_to_partype( type_str, nr, out_str[2],
				&to_type[ to_convdef_count], 
				&to_size[ to_convdef_count],
				to_format[ from_convdef_count]);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( sts, errh_ErrArgAF( to_convdef->Param[i]), NULL);
	    return sts;
	  }
	  cdh_ToUpper( to_param[ to_convdef_count], out_str[1]);
	  if ( to_convdef_count == 0)
	    to_offset[ to_convdef_count] = 0;
	  else
	    to_offset[ to_convdef_count] =
		to_offset[ to_convdef_count - 1] +
		to_size[ to_convdef_count - 1];
	  to_convdef_count++;
	}
	
	for ( i = 0; i < to_convdef_count; i++)
	{
	  /* Find a match i from_convdef */
	  for ( j = 0; j < from_convdef_count; j++)
	  {
	    if ( !strcmp( to_param[i], from_param[j]))
	    {
	      /* Hit */
	      sts = cnv_convtable_add( to_type[i], from_type[j],
				 to_offset[i], from_offset[j],
				 to_size[i], from_size[j],
				 to_format[i], from_format[j],
				 conv_table, conv_table_count);
	      if ( EVEN(sts))
	      {
	        errh_CErrLog( sts, errh_ErrArgAF( to_param[i]), NULL);
	        return sts;
	      }
	    }
	  }
	}

	sts = cnv_convtable_compress(	conv_table,
					conv_table_count);
	return CNV__SUCCESS;
}

static int	cnv_convtable_add( 	cnv_eParType	to_type,
					cnv_eParType	from_type,
					int	to_offset,
					int	from_offset,
					int	to_size,
					int	from_size,
					char	*to_format,
					char	*from_format,
					cnv_t_conv_table *conv_table,
					int	*conv_table_count)
{
	cnv_t_conv_item *conv_table_ptr;

	conv_table_ptr = (cnv_t_conv_item *)conv_table + *conv_table_count;

	if ( to_type == from_type)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = to_size;
	  conv_table_ptr->type = cnv_eConvType_memcpy;
	}
	else if ( from_type == cnv_eParType_Char &&
		  to_type == cnv_eParType_String)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = to_size;
	  conv_table_ptr->type = cnv_eConvType_CharToString;
	}
	else if ( from_type == cnv_eParType_Int16 &&
		  to_type == cnv_eParType_Int32)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = 0;
	  conv_table_ptr->type = cnv_eConvType_Int16ToInt32;
	}
	else if ( from_type == cnv_eParType_Int32 &&
		  to_type == cnv_eParType_Float32)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = 0;
	  conv_table_ptr->type = cnv_eConvType_Int32ToFloat32;
	}
	else if ( from_type == cnv_eParType_Int32 &&
		  to_type == cnv_eParType_Int16)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = 0;
	  conv_table_ptr->type = cnv_eConvType_Int32ToInt16;
	}
	else if ( from_type == cnv_eParType_Int32 &&
		  to_type == cnv_eParType_String)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = to_size;
	  conv_table_ptr->type = cnv_eConvType_Int32ToString;
	}
	else if ( from_type == cnv_eParType_Int32 &&
		  to_type == cnv_eParType_Ascii)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = to_size;
	  conv_table_ptr->type = cnv_eConvType_Int32ToAscii;
	}
	else if ( from_type == cnv_eParType_Float32 &&
		  to_type == cnv_eParType_Int32)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = 0;
	  conv_table_ptr->type = cnv_eConvType_Float32ToInt32;
	}
	else if ( from_type == cnv_eParType_Float32 &&
		  to_type == cnv_eParType_String)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = to_size;
	  conv_table_ptr->type = cnv_eConvType_Float32ToString;
	  strcpy( conv_table_ptr->format, from_format);
	}
	else if ( from_type == cnv_eParType_Float32 &&
		  to_type == cnv_eParType_Ascii)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = to_size;
	  conv_table_ptr->type = cnv_eConvType_Float32ToAscii;
	  strcpy( conv_table_ptr->format, from_format);
	}
	else if ( from_type == cnv_eParType_String &&
		  to_type == cnv_eParType_Int32)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = from_size;
	  conv_table_ptr->type = cnv_eConvType_StringToInt32;
	}
	else if ( from_type == cnv_eParType_String &&
		  to_type == cnv_eParType_Float32)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = from_size;
	  conv_table_ptr->type = cnv_eConvType_StringToFloat32;
	}
	else if ( from_type == cnv_eParType_Ascii &&
		  to_type == cnv_eParType_Int32)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = from_size;
	  conv_table_ptr->type = cnv_eConvType_AsciiToInt32;
	}
	else if ( from_type == cnv_eParType_Ascii &&
		  to_type == cnv_eParType_Float32)
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = from_size;
	  conv_table_ptr->type = cnv_eConvType_AsciiToFloat32;
	}
	else if ( (from_type == cnv_eParType_Time && 
		   to_type == cnv_eParType_Binary && 
		   to_size == sizeof(pwr_tTime)) ||
		  (from_type == cnv_eParType_Binary && 
		   to_type == cnv_eParType_Time && 
		   from_size == sizeof(pwr_tTime)))
	{
	  conv_table_ptr->from = from_offset;
	  conv_table_ptr->to = to_offset;
	  conv_table_ptr->size = to_size;
	  conv_table_ptr->type = cnv_eConvType_memcpy;
	}
	else
	  return CNV__CONVTYPE;

	(*conv_table_count)++;

	return CNV__SUCCESS;
}

int	cnv_ConvertData( 
			cnv_t_conv_table *conv_table,
			int		conv_table_count,
			char		*from_data,
			char		*to_data)
{
	cnv_t_conv_item *conv_item;
	int		i;
	char		tmpstr[1000];

	conv_item = (cnv_t_conv_item *) conv_table;
	for ( i = 0; i < conv_table_count; i++)
	{
	  switch( conv_item->type)
	  {
	    case cnv_eConvType_memcpy:
	      memcpy( to_data + conv_item->to, from_data + conv_item->from,
			conv_item->size);
	      break;
	    case cnv_eConvType_CharToString:
	      memcpy( to_data + conv_item->to, from_data + conv_item->from, 1);
	      memset( to_data + conv_item->to + 1, 0, 1);
	      break;
	    case cnv_eConvType_Int16ToInt32:
	      * (pwr_tInt32 *) (to_data + conv_item->to) =
		* (pwr_tInt16 *) (from_data + conv_item->from);
	      break;
	    case cnv_eConvType_Int32ToFloat32:
	      * (pwr_tFloat32 *) (to_data + conv_item->to) =
		* (pwr_tInt32 *) (from_data + conv_item->from);
	      break;
	    case cnv_eConvType_Int32ToInt16:
	      * (pwr_tInt16 *) (to_data + conv_item->to) =
		* (pwr_tInt32 *) (from_data + conv_item->from);
	      break;
	    case cnv_eConvType_Int32ToString:
	      sprintf( to_data + conv_item->to, "%d",
		* (pwr_tInt32 *) (from_data + conv_item->from));
	      break;
	    case cnv_eConvType_Int32ToAscii:
	      sprintf( tmpstr, "%*d",
	        conv_item->size,
		* (pwr_tInt32 *) (from_data + conv_item->from));
	      memcpy( to_data + conv_item->to, tmpstr, conv_item->size);
	      break;
	    case cnv_eConvType_Float32ToInt32:
	      * (pwr_tInt32 *) (to_data + conv_item->to) =
		* (pwr_tFloat32 *) (from_data + conv_item->from);
	      break;
	    case cnv_eConvType_Float32ToString:
	      sprintf( to_data + conv_item->to, conv_item->format, 
		* (pwr_tFloat32 *) (from_data + conv_item->from));
	      break;
	    case cnv_eConvType_Float32ToAscii:
	      if ( conv_item->format[0] != 0)
	        sprintf( tmpstr, conv_item->format,
		  * (pwr_tFloat32 *) (from_data + conv_item->from));
	      else
	        sprintf( tmpstr, "%*f",
	          conv_item->size,
		  * (pwr_tFloat32 *) (from_data + conv_item->from));
	      memcpy( to_data + conv_item->to, tmpstr, conv_item->size);
	      break;
	    case cnv_eConvType_StringToInt32:
	      sscanf( from_data + conv_item->from, "%d",
		(pwr_tInt32 *) (to_data + conv_item->to));
	      break;
	    case cnv_eConvType_StringToFloat32:
	      sscanf( from_data + conv_item->from, "%f",
		(pwr_tFloat32 *) (to_data + conv_item->to));
	      break;
	    case cnv_eConvType_AsciiToInt32:
	      strncpy( tmpstr, from_data + conv_item->from,
			conv_item->size);
	      tmpstr[ conv_item->size] = 0;
	      sscanf( tmpstr, "%d",
		(pwr_tInt32 *) (to_data + conv_item->to));
	      break;
	    case cnv_eConvType_AsciiToFloat32:
	      strncpy( tmpstr, from_data + conv_item->from, 
			conv_item->size);
	      tmpstr[ conv_item->size] = 0;
	      sscanf( tmpstr, "%f",
		  (pwr_tFloat32 *) (to_data + conv_item->to));
	      break;
	  }
	  conv_item++;
	}
	return CNV__SUCCESS;
}

static int	cnv_remove_blank( char *out_str, char *in_str)
{
	char *s;

	s = in_str;
	/* Find first not blank */
	while ( *s)
	{
	  if ( !(*s == 9 || *s == 32)) 
	    break;
	  s++;
	}
	strcpy( out_str, s);
	/* Remove at end */
	s = out_str + strlen(out_str);
	s--;
	while( s >= out_str)
	{
	  if ( !(*s == 9 || *s == 32)) 
	    break;
	  s--;
	}
	s++;
	*s = 0;
	return CNV__SUCCESS;
}

static int	cnv_string_to_partype( 	char 		*string, 
					int		args,
					char		*third_arg,
					cnv_eParType 	*type,
					int		*size,	
					char		*format)
{
	int	nr;

	if ( !strcmp( string, "CHAR"))
	{
	  *type = cnv_eParType_Char;
	  *size = 1;
	}
	else if ( !strcmp( string, "BOOLEAN"))
	{
	  *type = cnv_eParType_Boolean;
	  *size = 4;
	}
	else if ( !strcmp( string, "INT8"))
	{
	  *type = cnv_eParType_Int8;
	  *size = 1;
	}
	else if ( !strcmp( string, "UINT8"))
	{
	  *type = cnv_eParType_UInt8;
	  *size = 1;
	}
	else if ( !strcmp( string, "INT16"))
	{
	  *type = cnv_eParType_Int16;
	  *size = 2;
	}
	else if ( !strcmp( string, "UINT16"))
	{
	  *type = cnv_eParType_UInt16;
	  *size = 2;
	}
	else if ( !strcmp( string, "INT32"))
	{
	  *type = cnv_eParType_Int32;
	  *size = 4;
	}
	else if ( !strcmp( string, "UINT32"))
	{
	  *type = cnv_eParType_UInt32;
	  *size = 4;
	}
	else if ( !strcmp( string, "FLOAT32"))
	{
	  /* Format may be given as an argument */
	  if ( args == 3)
	    strcpy( format, third_arg);
	  *type = cnv_eParType_Float32;
	  *size = 4;
	}
	else if ( !strcmp( string, "FLOAT64"))
	{
	  *type = cnv_eParType_Float64;
	  *size = 8;
	}
	else if ( !strcmp( string, "STRING"))
	{
	  /* Size should be given as an argument */
	  /* Third arg is size */
	  if ( args != 3)
	    return CNV__SYNTAX;

	  nr = sscanf( third_arg, "%d", size);
	  if ( nr != 1)
	    return CNV__SYNTAX;
	  *type = cnv_eParType_String;
	}
	else if ( !strcmp( string, "ASCII"))
	{
	  /* Size should be given as an argument */
	  /* Third arg is size */
	  if ( args != 3)
	    return CNV__SYNTAX;

	  nr = sscanf( third_arg, "%d", size);
	  if ( nr != 1)
	    return CNV__SYNTAX;
	  *type = cnv_eParType_Ascii;
	}
	else if ( !strcmp( string, "BINARY"))
	{
	  /* Size should be given as an argument */
	  if ( args != 3)
	    return CNV__SYNTAX;

	  nr = sscanf( third_arg, "%d", size);
	  if ( nr != 1)
	    return CNV__SYNTAX;
	  *type = cnv_eParType_Binary;
	}
	else if ( !strcmp( string, "UNKNOWN"))
	{
	  /* Size should be given as an argument */
	  if ( args != 3)
	    return CNV__SYNTAX;

	  nr = sscanf( third_arg, "%d", size);
	  if ( nr != 1)
	    return CNV__SYNTAX;
	  *type = cnv_eParType_Unknown;
	}
	else if ( !strcmp( string, "TIME"))
	{
	  *type = cnv_eParType_Time;
	  *size = 8;
	}
	else
	  return CNV__NOPARTYPE;

	return CNV__SUCCESS;
}

static int	cnv_convtable_compress(	cnv_t_conv_table *conv_table,
					int		*conv_table_count)
{
	cnv_t_conv_item *conv_table_ptr;
	cnv_t_conv_item previous;
	cnv_t_conv_item *current_ptr;
	int		new_count;
	int		i;
	int		compress;

	new_count = 1;
	current_ptr = (cnv_t_conv_item *) conv_table;
	conv_table_ptr = (cnv_t_conv_item *) conv_table + 1;
	memcpy( &previous, conv_table, sizeof(previous));
	for ( i = 1; i < *conv_table_count; i++)
	{
	  compress = 0;
	  if ( current_ptr->type == cnv_eConvType_memcpy &&
	       conv_table_ptr->type == cnv_eConvType_memcpy  &&
	       conv_table_ptr->to ==
			(previous.to + previous.size) &&
	       conv_table_ptr->from ==
			(previous.from + previous.size))
	  {
	    compress = 1;
	  }

	  if ( compress)
	  {
	    /* Compress these */
	    current_ptr->size += conv_table_ptr->size;
	  }
	  else
	  {
	    current_ptr++;
	    memcpy( current_ptr, conv_table_ptr, sizeof(*current_ptr));
	    new_count++;
	  }

	  memcpy( &previous, conv_table_ptr, sizeof(previous));
	  conv_table_ptr++;
	}
	*conv_table_count = new_count;

	return CNV__SUCCESS;	
}

static int	cnv_GetConvTableFromClass( 
					pwr_tObjid		class,
					pwr_sClass_ConvDef 	**convdef) 
{
	pwr_tClassId 	class_class;
	pwr_tObjid 	objid;
	pwr_tObjid 	rtbody_objid;
	char		name[80];
	pwr_sParInfo	parinfo;
	char		*attrname;
	int		add_size;
	int		size;
	char		type[32];
	int		i, j;
	pwr_tStatus	sts;
	int		param_size;
	

	/* Is this a ClassDef object ? */
	sts = gdh_GetObjectClass ( class, &class_class);
	if ( EVEN(sts)) return sts;

	if ( class_class != pwr_eClass_ClassDef)
	  return CNV__CLASS;

	*convdef = calloc( 1, sizeof( **convdef));
	if ( *convdef == 0)
	  return CNV__NOMEMORY;

	/* Get the attributes under RtBody */
	sts = gdh_ObjidToName ( class, name, sizeof(name), 
		cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	strcat( name, "-RtBody");
	sts = gdh_NameToObjid ( name, &rtbody_objid);
	if ( EVEN(sts)) return CNV__CLASS;

	param_size = sizeof((*convdef)->Param)/sizeof((*convdef)->Param[0]);
	i = 0;
	sts = gdh_GetChild( rtbody_objid, &objid);
	while (ODD(sts))
	{
	  if ( i >= param_size)
	    break;

	  sts = gdh_ObjidToName ( objid, name, sizeof(name), 
		cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;

	  attrname = strrchr( name, '-');
	  if ( attrname == 0)
	    attrname = name;
	  else
	    attrname++;

	  sts = gdh_GetObjectInfo ( name, &parinfo, sizeof(parinfo));
	  if ( EVEN(sts)) return sts;

	  add_size = 0;
	  switch( parinfo.Type)
	  {
	    case pwr_eType_Boolean:
	      strcpy( type, "Boolean");
	      break;
	    case pwr_eType_Float32:
	      strcpy( type, "Float32");
	      break;
	    case pwr_eType_Float64:
	      strcpy( type, "Float64");
	      break;
	    case pwr_eType_Char:
	      strcpy( type, "Char");
	      break;
	    case pwr_eType_Int8:
	      strcpy( type, "Int8");
	      break;
	    case pwr_eType_Int16:
	      strcpy( type, "Int16");
	      break;
	    case pwr_eType_Int32:
	      strcpy( type, "Int32");
	      break;
	    case pwr_eType_UInt8:
	      strcpy( type, "UInt8");
	      break;
	    case pwr_eType_UInt16:
	      strcpy( type, "UInt16");
	      break;
	    case pwr_eType_UInt32:
	      strcpy( type, "UInt32");
	      break;
	    case pwr_eType_String:
	      strcpy( type, "String");
	      add_size = 1;
	      break;
	    case pwr_eType_Text:
	      strcpy( type, "String");
	      add_size = 1;
	      break;
	    case pwr_eType_Time:
	    case pwr_eType_DeltaTime:
	      strcpy( type, "Time");
	      break;
	    default:
	      strcpy( type, "Binary");
	      add_size = 1;
	      break;
	  }
	  if (!( parinfo.Flags & PWR_MASK_ARRAY ))
	  {
	    strcpy( (*convdef)->Param[i], type);
	    strcat( (*convdef)->Param[i], " ");
	    strcat( (*convdef)->Param[i], attrname);
	    if ( add_size)
	    {
	      size = parinfo.Size;
	      sprintf( (*convdef)->Param[i]+strlen((*convdef)->Param[i]),
			" %d", size);
	    }
	    i++;
	  }
	  else
	  {
	    for( j = 0; j < (int) parinfo.Elements; j++)
	    {
	      if ( i >= param_size)
		break;
	      strcpy( (*convdef)->Param[i], type);
	      strcat( (*convdef)->Param[i], " ");
	      strcat( (*convdef)->Param[i], attrname);
	      sprintf( (*convdef)->Param[i]+strlen((*convdef)->Param[i]),
			"[%d]", j);
	      if ( add_size)
	      {
	        size = parinfo.Size/parinfo.Elements;
	        sprintf( (*convdef)->Param[i]+strlen((*convdef)->Param[i]),
			" %d", size);
	      }
	      i++;
	    }
	  }
	  sts = gdh_GetNextSibling ( objid, &objid);
	}

	return CNV__SUCCESS;
}

static int	cnv_ReadConvTable( 	char			*filename,
					pwr_sClass_ConvDef 	**convdef) 
{
	FILE		*infile;
	char		line[80];
	int		i;
	int		sts;

	infile = fopen( filename, "r");
	if ( !infile)
	{
	  errh_CErrLog( CNV__NOFILE, errh_ErrArgAF( filename), NULL);
	  return CNV__NOFILE;
	}

	*convdef = calloc( 1, sizeof( **convdef));
	if ( *convdef == 0)
	  return CNV__NOMEMORY;

	i = 0;
	while( ODD( sts = cnv_read_line( line, sizeof(line), infile)))
	{
	  cnv_remove_blank( line, line);
	  if ( line[0] == '!')
	    continue;
	  if ( i >= CNV_CONVDEF_SIZE) 
	    return CNV__FILESIZE;

	  strcpy( (*convdef)->Param[i], line);
	  i++;
	}
	fclose( infile);
	return CNV__SUCCESS;
}

/*************************************************************************
*
* Name:		dtt_read_line()
*
* Type		void
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Read a line for a file.
*
**************************************************************************/

static int cnv_read_line(	char	*line,
				int	maxsize,
				FILE	*file)
{ 
	char	*s;

	if (fgets( line, maxsize, file) == NULL)
	  return 0;
	s = strchr( line, 10);
	if ( s != 0)
	  *s = 0;

	return CNV__SUCCESS;
}

int cvn_ConvInit( 
	pwr_tInt32	ToConvdefType,
	pwr_tObjid	ToConvdef,
	char		*ToConvdefFile,
	pwr_tInt32	FromConvdefType,
	pwr_tObjid	FromConvdef,
	char		*FromConvdefFile,
	pwr_tInt32	*ConvTableSize,
	pwr_tInt32	AllocConvTable,
	char		**ConvTable)
{
	cnv_t_conv_table	*conv_table;
	int			conv_table_count;
	int			i;
	int			sts;
	int			log = 1;
	pwr_sClass_ConvDef 	*to_convdef_ptr;
	pwr_sClass_ConvDef 	*from_convdef_ptr;
	
	if ( ToConvdefType == CNV_CONVDEFTYPE_OBJ)
	{
	  /* Get pointer to ConvDef object */
	  sts = gdh_ObjidToPointer ( ToConvdef, (pwr_tAddress *) &to_convdef_ptr);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( CNV__CONVDEFOBJECT, errh_ErrArgMsg(sts), NULL);
	    *ConvTableSize = 0;
	    return CNV__CONVDEFOBJECT;
	  }
	}
	else if ( ToConvdefType == CNV_CONVDEFTYPE_FILE)
	{
	  sts = cnv_ReadConvTable( ToConvdefFile, &to_convdef_ptr);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( CNV__CONVDEFFILE, errh_ErrArgMsg(sts), NULL);
	    *ConvTableSize = 0;
	    return CNV__CONVDEFFILE;
	  }
	}
	else if ( ToConvdefType == CNV_CONVDEFTYPE_CLASS)
	{
	  sts = cnv_GetConvTableFromClass( ToConvdef, &to_convdef_ptr);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( CNV__CONVDEFCLASS, errh_ErrArgMsg(sts), NULL);
	    *ConvTableSize = 0;
	    return CNV__CONVDEFCLASS;
	  }
	}
	else
	{
	  errh_CErrLog( CNV__CONVDEFTYPE, NULL);
	  *ConvTableSize = 0;
	  return CNV__CONVDEFTYPE;
	}

	if ( FromConvdefType == CNV_CONVDEFTYPE_OBJ)
	{
	  /* Get pointer to ConvDef object */
	  sts = gdh_ObjidToPointer ( FromConvdef, 
			(pwr_tAddress *) &from_convdef_ptr);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( CNV__CONVDEFOBJECT, errh_ErrArgMsg(sts), NULL);
	    *ConvTableSize = 0;
	    return CNV__CONVDEFOBJECT;
	  }
	}
	else if ( FromConvdefType == CNV_CONVDEFTYPE_FILE)
	{
	  sts = cnv_ReadConvTable( FromConvdefFile, &from_convdef_ptr);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( CNV__CONVDEFFILE, errh_ErrArgMsg(sts), NULL);
	    *ConvTableSize = 0;
	    return CNV__CONVDEFFILE;
	  }
	}
	else if ( FromConvdefType == CNV_CONVDEFTYPE_CLASS)
	{
	  sts = cnv_GetConvTableFromClass( FromConvdef, &from_convdef_ptr);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( CNV__CONVDEFCLASS, errh_ErrArgMsg(sts), NULL);
	    *ConvTableSize = 0;
	    return CNV__CONVDEFCLASS;
	  }
	}
	else
	{
	  errh_CErrLog( CNV__CONVDEFTYPE, NULL);
	  *ConvTableSize = 0;
	  return CNV__CONVDEFTYPE;
	}

	conv_table_count = 0;
	if ( AllocConvTable)
	{
	  /* Allocate space for the conversion table */
	  conv_table = calloc( 1, sizeof( *conv_table));
	  if ( !conv_table)
	  {
	    errh_CErrLog( CNV__NOMEMORY, NULL);
	    *ConvTableSize = 0;
	    return CNV__NOMEMORY;
	  }
	}
	else
	  /* The user has already allocated space */
	  conv_table = (cnv_t_conv_table *) ConvTable;

	sts = cnv_CreateConvTable( to_convdef_ptr, from_convdef_ptr,
		conv_table, &conv_table_count);
	if ( EVEN(sts))
	{
	  *ConvTableSize = 0;
	  return sts;
	}

	if ( AllocConvTable)
	  /* Return the address of the conversion table */
	  *ConvTable = (char *) conv_table;

	*ConvTableSize = conv_table_count;

	if ( log)
	{
	  printf( "nr	from	to	size	type\n");
	  for ( i = 0; i < conv_table_count; i++)
	  {
	    printf( "%d	%d	%d	%d	%d\n", i,
		(* conv_table)[i].from,
		(* conv_table)[i].to,
		(* conv_table)[i].size,
		(* conv_table)[i].type);
	  }
	}
	return CNV__SUCCESS;
}

