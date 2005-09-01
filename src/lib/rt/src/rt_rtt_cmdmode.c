/* 
 * Proview   $Id: rt_rtt_cmdmode.c,v 1.2 2005-09-01 14:57:56 claes Exp $
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

/* rt_rtt_cmdmode.c
   This module contains routines for handling of commandfiles in rtt. */

/*_Include files_________________________________________________________*/



#if defined(OS_ELN)
# include $vaxelnc
# include stdio
# include string
# include stdlib
# include float
# include math
# include ctype
# include chfdef
# include starlet
# include descrip
#else
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <float.h>
# include <math.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "co_time.h"
#include "co_ccm.h"
#include "rt_gdh.h"
#include "rt_rtt.h"
#include "rt_rtt_global.h"
#include "rt_rtt_msg.h"
#include "rt_rtt_functions.h"
#include "dtt_rttsys_functions.h"
#include "rt_gdh_msg.h"


/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif


#define IF_NOGDH_RETURN \
if ( !rtt_gdh_started)\
{\
rtt_message('E',"Rtt is not connected to nethandler");\
return RTT__NOPICTURE;\
}

typedef	struct	{
	char	key[80];
	char	value[80];
	} rtt_t_symboltable;

typedef	struct	{
	char	p1[80];
	char	p2[80];
	char	p3[80];
	char	p4[80];
	char	p5[80];
	char	p6[80];
	char	p7[80];
	char	p8[80];
	} rtt_t_cmdargtable;

#define	RTT_COMMANDLEVEL_MAX 10
#define	RTT_SYMBOLTABLE_SIZE 100
#define CMD_BUFF_SIZE (RTT_SYMBOLTABLE_SIZE * 160)

static rtt_t_cmdargtable rtt_cmdargtable[RTT_COMMANDLEVEL_MAX];
static rtt_t_symboltable rtt_symboltable[RTT_SYMBOLTABLE_SIZE];
static int		rtt_symboltable_count;

static int	rtt_symbol_to_int( int	*int_value, char *value);
static int	rtt_symbol_to_float( float *int_value, char *value);
static int	rtt_get_attr_value(
			char		*attrname,
			char		*value);

/*************************************************************************
*
* Name:		rtt_store_symbols()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Store all symbols in a rtt commandfile.
*
**************************************************************************/
int	rtt_store_symbols( char	*filename)
{
	char	filename_str[80];
	FILE	*file;
	char	message[120];
	char	*s;
	int	i;

	rtt_get_defaultfilename( filename, filename_str, ".rtt_com");

	/* Open the file */
	file = fopen( filename_str, "w");
	if ( file == 0)
	  return RTT__NOFILE;

	for ( i = 0; i < rtt_symboltable_count; i++)
	{
	  fprintf( file, "define %s \"", rtt_symboltable[i].key);
	  for ( s = rtt_symboltable[i].value; *s; s++)
	  {
	    if ( *s == '"')
	    {
	      fputc( '\\', file);
	      fputc( *s, file);
	    }
	    else
	      fputc( *s, file);
	  }
	  fprintf( file, "\"\n");
	}
	rtt_fgetname( file, filename_str, filename_str);
	fclose( file);
	sprintf( message, "Symbols stored in %s", filename_str);
	rtt_message('I', message);
	return RTT__NOPICTURE;	
}

/*************************************************************************
*
* Name:		rtt_show_symbols()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Show all symbols in a view-window.
*
**************************************************************************/
int	rtt_show_symbols( menu_ctx	ctx)
{
	char 	*buff;
	int	i;
	int	sts;

	if ( rtt_symboltable_count == 0)
	{
	  rtt_message('I',"No symbol is defined");
	  return RTT__NOPICTURE;
	}
	buff = calloc( 1, CMD_BUFF_SIZE);
	sprintf( buff, " %-20.20s  %s\n\n", "Symbol", "Value");
	for ( i = 0; i < rtt_symboltable_count; i++)
	{
	  sprintf( buff+strlen(buff), " %-20.20s  %s\n", 
		rtt_symboltable[i].key, rtt_symboltable[i].value);
	}
	sts = rtt_view( ctx, 0, buff, "Rtt Symbols", 
			RTT_VIEWTYPE_BUF);
	free( buff);
	return sts;
}
/*************************************************************************
*
* Name:		rtt_replace_symbol()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Replace a symbol in the symbol table.
*	A string surrounde by ' will replaced if then string is found
*	to be a symbol. If the symbol is surrounded by #' the symbol
*	will not be replaced.
*
**************************************************************************/
int	rtt_replace_symbol( char *command, char *newcommand)
{
	char	*s;
	char	*t;
	char	*u;
	int	symbolmode;
	int	size;
	char	value[80];
	char	symbol[80];
	char	upper_symbol[80];
	int	sts;
	char	new[160];
	int	ignore_symbolmode;

	symbolmode = 0;
	ignore_symbolmode = 0;
	s = command;
	t = new;

	while ( *s != 0)
	{
	  if ( (unsigned char) *s == '#')
	  {	
	    if ( ignore_symbolmode)
	    {
	      strcpy( t, "#");
	      t++;
	    }
	    ignore_symbolmode = 1;
	  }
	  else if (*s == '\'')
	  {
	    if ( symbolmode)
	    {
	      /* End of potential symbol */
	      size = (int) s - (int) u;
	      strncpy( symbol, u, size);
	      symbol[size] = 0;
	      rtt_toupper( upper_symbol, symbol);
	      sts = rtt_get_symbol( upper_symbol, value);
	      if ( EVEN(sts))
	      {
	        /* It was no symbol */
	        strcpy( t, "'");
	        t++;
	        strcat( t, symbol);
	        t += strlen(symbol);
	        strcat( t, "'");
	        t++;
	      }
	      else
	      {
	        /* Symbol found */
	        strcpy( t, value);
	        t += strlen(value);
	      }
	      symbolmode = 0;
	    }
	    else
	    {
	      if ( ignore_symbolmode)
	      {
	        strcpy( t, "'");
	        t++;
	      }
	      else
	      {
	        symbolmode = 1;
	        u = s + 1; 
	      }
	    }
	    ignore_symbolmode = 0;
	  }
	  else
	  {
	    if ( !symbolmode)
	    {
	      if ( ignore_symbolmode)
	      {
	        strcpy( t, "#");
	        t++;
	      }
	      *t = *s;
	      t++;
	    }
	    ignore_symbolmode = 0;
	  }
	  s++;
	}
	if ( ignore_symbolmode)
	{
	  strcpy( t, "#");
	}
	else if ( symbolmode)
	{
	  strcpy( t, u);
	}
	else
	  *t = 0;

	strcpy( newcommand, new);
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_symbol()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get a symbol from the symbol table.
*
**************************************************************************/
int	rtt_get_symbol( char *key, char *value)
{
	int	i;
	int	sts;

	if ( *key == '#')
	{
	  /* This is an attribute reference */
	  IF_NOGDH_RETURN;
	  sts = rtt_get_attr_value( &key[1], value);
	  if (EVEN(sts)) 
	  {
	    rtt_message('E', "Object not found");
	    return 0;
	  }
	  return RTT__SUCCESS;
	}
	/* Check if system symbol */
	if ( !strcmp( "RTT_PLATFORM", key))
	{
	  strcpy( value, rtt_platform);
	  return RTT__SUCCESS;	  
	}
	if ( !strcmp( "RTT_HW", key))
	{
	  strcpy( value, rtt_hw);
	  return RTT__SUCCESS;	  
	}
	if ( !strcmp( "RTT_OS", key))
	{
	  strcpy( value, rtt_os);
	  return RTT__SUCCESS;	  
	}
	if ( !strcmp( "RTT_NODE", key))
	{
	  strcpy( value, rtt_node);
	  return RTT__SUCCESS;	  
	}
	if ( !strcmp( "RTT_SYS", key))
	{
	  strcpy( value, rtt_sys);
	  return RTT__SUCCESS;	  
	}
	if ( !strcmp( "RTT_IDENT", key))
	{
	  strcpy( value, rtt_ident);
	  return RTT__SUCCESS;	  
	}
	if ( !strcmp( "RTT_TIME", key))
	{
	  rtt_update_time ();
	  strcpy( value, rtt_time);
	  return RTT__SUCCESS;	  
	}

	for ( i = 0; i < rtt_symboltable_count; i++)
	{
	  if ( !strcmp( rtt_symboltable[i].key, key))
	  {
	    strcpy( value, rtt_symboltable[i].value);
	    return RTT__SUCCESS;
	  }
	}
	return RTT__NOSYMBOL;
}

/*************************************************************************
*
* Name:		rtt_get_cmd_symbol()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get a symbol from the symbol table. Abbrevations is allowed.
*
**************************************************************************/
int	rtt_get_symbol_cmd( char *key, char *value)
{
	int	i;
	int	found;

	found = 0;
	for ( i = 0; i < rtt_symboltable_count; i++)
	{
	  if ( !strcmp( rtt_symboltable[i].key, key))
	  {
	    /* Perfect match */
	    strcpy( value, rtt_symboltable[i].value);
	    return RTT__SUCCESS;
	  }
	  else if ( !strncmp( rtt_symboltable[i].key, key, strlen(key)))
	  {
	    if ( found)
	      return RTT__SYMBOL_AMBIG;
	    strcpy( value, rtt_symboltable[i].value);
	    found = 1;
	  }
	}
	if ( found)
	  return RTT__SUCCESS;

	return RTT__NOSYMBOL;
}

/*************************************************************************
*
* Name:		rtt_define_symbol()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Define a symbol in the symbol table.
*
**************************************************************************/
int	rtt_define_symbol( char *key, char *arg1, char *arg2, char *arg3)
{
	int	i;
	char	value[80];
	int	int_value;
	float	float_value;
	int	int_value1;
	float	float_value1;
	int	int_value2;
	float	float_value2;
	int	sts;

	if ( rtt_symboltable_count >= RTT_SYMBOLTABLE_SIZE)
	{
	  rtt_message('E', "Symboltable is full");
	  return RTT__NOPICTURE;
	}
	if ( strlen( key) > sizeof(rtt_symboltable[0].key) - 1)
	{
	  rtt_message('E', "Symbol name to long");
	  return RTT__HOLDCOMMAND;
	}

	/* Check if arg1 is an operator */
	if ( !strcmp( arg1, "++"))
	{
	  /* Get the old value */
	  sts = rtt_get_symbol( key, value);
	  if ( EVEN(sts))
	  {
	    rtt_message('E',"Symbol not found");
	    return RTT__HOLDCOMMAND;
	  }

	  /* convert to integer or float */
	  if ( ODD( rtt_symbol_to_int( &int_value, value)))
	  {
	    int_value++;
	    sprintf( value, "%d", int_value);
	  }
	  else if ( ODD( rtt_symbol_to_float( &float_value, value)))
	  {
	    float_value++;
	    sprintf( value, "%f", float_value);
	  }
	  else
	  {
	    rtt_message('E',"Error in symbol type");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( !strcmp( arg1, "--"))
	{
	  /* Get the old value */
	  sts = rtt_get_symbol( key, value);
	  if ( EVEN(sts))
	  {
	    rtt_message('E',"Symbol not found");
	    return RTT__HOLDCOMMAND;
	  }

	  /* convert to integer or float */
	  if ( ODD( rtt_symbol_to_int( &int_value, value)))
	  {
	    int_value--;
	    sprintf( value, "%d", int_value);
	  }
	  else if ( ODD( rtt_symbol_to_float( &float_value, value)))
	  {
	    float_value--;
	    sprintf( value, "%f", float_value);
	  }
	  else
	  {
	    rtt_message('E',"Error in symbol type");
	    return RTT__HOLDCOMMAND;
	  }
	}
	else if ( arg2 == 0)
	{
	  /* arg1 is the value */
	  strcpy( value, arg1);
	}
	else
	{
	  /* arg2 is the operator */
	  if ( arg3 == 0)
	  {
	    rtt_message('E', "Syntax error");
	    return RTT__HOLDCOMMAND;
	  }
	  if ( !strcmp( arg2, "+"))
	  {
	    if ( ODD( rtt_symbol_to_int( &int_value1, arg1)) &&
	         ODD( rtt_symbol_to_int( &int_value2, arg3)))
	    {
	      int_value = int_value1 + int_value2;
	      sprintf( value, "%d", int_value);
	    }
	    else if ( ODD( rtt_symbol_to_float( &float_value1, arg1)) &&
	              ODD( rtt_symbol_to_float( &float_value2, arg3)))
	    {
	      float_value = float_value1 + float_value2;
	      sprintf( value, "%f", float_value);
	    }
	    else
	    {
	      rtt_message('E',"Error in symbol type");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else if ( !strcmp( arg2, "-"))
	  {
	    if ( ODD( rtt_symbol_to_int( &int_value1, arg1)) &&
	         ODD( rtt_symbol_to_int( &int_value2, arg3)))
	    {
	      int_value = int_value1 - int_value2;
	      sprintf( value, "%d", int_value);
	    }
	    else if ( ODD( rtt_symbol_to_float( &float_value1, arg1)) &&
	              ODD( rtt_symbol_to_float( &float_value2, arg3)))
	    {
	      float_value = float_value1 - float_value2;
	      sprintf( value, "%f", float_value);
	    }
	    else
	    {
	      rtt_message('E',"Error in symbol type");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else if ( !strcmp( arg2, "*"))
	  {
	    if ( ODD( rtt_symbol_to_int( &int_value1, arg1)) &&
	         ODD( rtt_symbol_to_int( &int_value2, arg3)))
	    {
	      int_value = int_value1 * int_value2;
	      sprintf( value, "%d", int_value);
	    }
	    else if ( ODD( rtt_symbol_to_float( &float_value1, arg1)) &&
	              ODD( rtt_symbol_to_float( &float_value2, arg3)))
	    {
	      float_value = float_value1 * float_value2;
	      sprintf( value, "%f", float_value);
	    }
	    else
	    {
	      rtt_message('E',"Error in symbol type");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	  else if ( !strcmp( arg2, "/"))
	  {
	    if ( ODD( rtt_symbol_to_int( &int_value1, arg1)) &&
	         ODD( rtt_symbol_to_int( &int_value2, arg3)))
	    {
	      int_value = int_value1 / int_value2;
	      sprintf( value, "%d", int_value);
	    }
	    else if ( ODD( rtt_symbol_to_float( &float_value1, arg1)) &&
	              ODD( rtt_symbol_to_float( &float_value2, arg3)))
	    {
	      float_value = float_value1 / float_value2;
	      sprintf( value, "%f", float_value);
	    }
	    else
	    {
	      rtt_message('E',"Error in symbol type");
	      return RTT__HOLDCOMMAND;
	    }
	  }
	}

	if ( strlen( value) > sizeof(rtt_symboltable[0].value) - 1)
	{
	  rtt_message('E', "Symbol value to long");
	  return RTT__HOLDCOMMAND;
	}

	rtt_toupper( value, value);
	/* Look if the symbol exists */
	if ( !strcmp(key,"P1"))
	  strcpy( rtt_cmdargtable[rtt_command_level-1].p1, value);
	else if ( !strcmp(key,"P2"))
	  strcpy( rtt_cmdargtable[rtt_command_level-1].p2, value);
	else if ( !strcmp(key,"P3"))
	  strcpy( rtt_cmdargtable[rtt_command_level-1].p3, value);
	else if ( !strcmp(key,"P4"))
	  strcpy( rtt_cmdargtable[rtt_command_level-1].p4, value);
	else if ( !strcmp(key,"P5"))
	  strcpy( rtt_cmdargtable[rtt_command_level-1].p5, value);
	else if ( !strcmp(key,"P6"))
	  strcpy( rtt_cmdargtable[rtt_command_level-1].p6, value);
	else if ( !strcmp(key,"P7"))
	  strcpy( rtt_cmdargtable[rtt_command_level-1].p7, value);
	else if ( !strcmp(key,"P8"))
	  strcpy( rtt_cmdargtable[rtt_command_level-1].p8, value);
	else
	{
	  for ( i = 0; i < rtt_symboltable_count; i++)
  	  {
	    if ( !strcmp( rtt_symboltable[i].key, key))
	    {
	      strcpy( rtt_symboltable[i].value, value);
	      return RTT__NOPICTURE;
	    }
	  }

	  strcpy( rtt_symboltable[i].key, key);
	  strcpy( rtt_symboltable[i].value, value);
	  rtt_symboltable_count++;
	}
	rtt_message('I',"Symbol defined");

	return RTT__NOPICTURE;
}


static int	rtt_symbol_to_int( int	*int_value, char *value)
{
	int	nr;

	/* Check if this is a float */
	if ( strchr( value, '.'))
	  return 0;
	if ( strchr( value, 'E'))
	  return 0;
	if ( strchr( value, 'e'))
	  return 0;
	nr = sscanf( value, "%d", int_value);
	if ( nr != 1)
	  return 0;
	return 1;
}

static int	rtt_symbol_to_float( float *float_value, char *value)
{
	int	nr;

	nr = sscanf( value, "%f", float_value);
	if ( nr != 1)
	  return 0;
	return 1;
}

/*************************************************************************
*
* Name:		rtt_attribute_func()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Backcall function for ccm.
*
**************************************************************************/
int rtt_attribute_func ( 
  char		*name,
  int		*return_decl,
  float		*return_float,
  int		*return_int,
  char		*return_string
)
{
	int		sts;
	char		hier_name[80];
	char		object_par[80];
	char		*object_element;
	int		element;
	char		elementstr[20];
	char		*s;
	char		*t;
	int		len;
	int		decl;
	int		int_val;
	float		float_val;
	char		string_val[80];
	pwr_sAttrRef	attrref;
	pwr_tTypeId	attrtype;
	unsigned int	attrsize, attroffs, attrelem;

	sts = gdh_NameToAttrref( pwr_cNObjid, name, &attrref);
	if ( EVEN(sts)) return sts;

	/* Get type of attribute */
	sts = gdh_GetAttributeCharacteristics ( name,
		&attrtype, &attrsize, &attroffs, &attrelem);
	if ( EVEN(sts)) return sts;

	/* Get the pointer to attribute */
	sts = gdh_GetObjectInfo ( name, &object_par, sizeof(object_par));
	if ( EVEN(sts)) return sts;

	/* Check index in parameter */
	s = strchr( name, '[');
	if ( s == 0)
	  element = 0;
	else
	{
	  t = strchr( name, ']');
	  if ( t == 0)
	    return RTT__ELEMSYNTAX;
	  else
	  {
	    len = t - s - 1;
	    strncpy( elementstr, s + 1, len);
	    elementstr[ len ] = 0;
	    sscanf( elementstr, "%d", &element);
	    *s = '\0';
	    if ( (element < 0) || (element > 100) )
	      return RTT__ELEMSYNTAX;
	  }
	}
	
	object_element = object_par + element * attrsize / attrelem;

        switch ( attrtype )
        {
          case pwr_eType_Boolean:
          {
	    int_val = *(pwr_tBoolean *)object_element;
	    decl = CCM_DECL_INT;
            break;
          }
          case pwr_eType_Float32:
          {
	    float_val = *(pwr_tFloat32 *)object_element;
	    decl = CCM_DECL_FLOAT;
            break;
          }
          case pwr_eType_Float64:
          {
	    float_val = *(pwr_tFloat64 *)object_element;
	    decl = CCM_DECL_FLOAT;
            break;
          }
          case pwr_eType_Char:
          {
	    int_val = *(pwr_tChar *)object_element;
	    decl = CCM_DECL_INT;
            break;
          }
          case pwr_eType_Int8:
          {
	    int_val = *(pwr_tInt8 *)object_element;
	    decl = CCM_DECL_INT;
            break;
          }
          case pwr_eType_Int16:
          {
	    int_val = *(pwr_tInt16 *)object_element;
	    decl = CCM_DECL_INT;
            break;
          }
          case pwr_eType_Int32:
          {
	    int_val = *(pwr_tInt32 *)object_element;
	    decl = CCM_DECL_INT;
            break;
          }
          case pwr_eType_UInt8:
          {
	    int_val = *(pwr_tUInt8 *)object_element;
	    decl = CCM_DECL_INT;
            break;
          }
          case pwr_eType_UInt16:
          {
	    int_val = *(pwr_tUInt16 *)object_element;
	    decl = CCM_DECL_INT;
            break;
          }
          case pwr_eType_UInt32:
	  case pwr_eType_ClassId:
	  case pwr_eType_TypeId:
	  case pwr_eType_VolumeId:
	  case pwr_eType_ObjectIx:
          {
	    int_val = *(pwr_tUInt32 *)object_element;
	    decl = CCM_DECL_INT;
            break;
          }
          case pwr_eType_String:
          {
	    strncpy( string_val, object_element, sizeof( string_val));
	    string_val[sizeof(string_val)-1] = 0;
	    decl = CCM_DECL_STRING;
            break;
          }
          case pwr_eType_Text:
          {
	    strncpy( string_val, object_element, sizeof( string_val));
	    string_val[sizeof(string_val)-1] = 0;
	    decl = CCM_DECL_STRING;
            break;
          }
          case pwr_eType_Objid:
          {
            /* Get the object name from ldh */
            sts = gdh_ObjidToName( *(pwr_tObjid *)object_element, 
		hier_name, sizeof( hier_name), cdh_mName_volumeStrict);
            if ( EVEN(sts))
	      strcpy( string_val, "Undefined Object");
	    else
	      strncpy( string_val, object_element, sizeof( string_val));
	    string_val[sizeof(string_val)-1] = 0;
	    decl = CCM_DECL_STRING;
            break;
          }
	  case pwr_eType_AttrRef:
          {
            /* Get the object name from ldh */
            sts = gdh_AttrrefToName( (pwr_sAttrRef *)object_element,
		hier_name, sizeof(hier_name), cdh_mName_volumeStrict);
 	    if ( EVEN(sts))
	      strcpy( string_val, "Undefined attribute");
	    else
	      strncpy( string_val, hier_name, sizeof( string_val));
	    string_val[sizeof(string_val)-1] = 0;
	    decl = CCM_DECL_STRING;
            break;
          }
          case pwr_eType_Time:
          {
	    /* Convert time to ascii */	
	    sts = time_AtoAscii((pwr_tTime *)object_element, 
                      time_eFormat_DateAndTime, 
                      string_val, sizeof(string_val));
	    string_val[20] = 0;
	    decl = CCM_DECL_STRING;
	    break;
          }
        }

	*return_decl = decl;
	if ( decl == CCM_DECL_INT)
	  *return_int = int_val;
	else if ( decl == CCM_DECL_FLOAT)
	  *return_float = float_val;
	else if ( decl == CCM_DECL_STRING)
	  strcpy( return_string, string_val);

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_get_attr_value()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

static int	rtt_get_attr_value(
			char		*attrname,
			char		*value)
{
	int	sts;
	int	return_int;
	float	return_float;
	char	return_string[80];
	int	decl;

	sts = rtt_attribute_func( attrname, &decl, &return_float, &return_int,
		return_string);
	if (EVEN(sts)) return sts;

	switch ( decl)
	{
	  case CCM_DECL_INT:
	    sprintf( value, "%d", return_int);
	    break;

	  case CCM_DECL_FLOAT:
	    /* If value is close to integer, round it */
	    if ( fabs( return_float - floor( return_float)) <= FLT_EPSILON)
	    {
	      return_int = return_float + FLT_EPSILON;
	      sprintf( value, "%d", return_int);
	    }
	    else if ( fabs( return_float - floor( return_float) + 1) <= FLT_EPSILON)
	    {
	      return_int = return_float + FLT_EPSILON;
	      sprintf( value, "%d", return_int);
	    }
	    else
	      sprintf( value, "%f", return_float);
	    break;
	  case pwr_eType_String:
	    sprintf( value, "%s", return_string);
	    break;
	}
	return RTT__SUCCESS;
}

