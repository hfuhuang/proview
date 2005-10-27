/** 
 * Proview   $Id: co_dcli.c,v 1.6 2005-10-27 08:47:10 claes Exp $
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

/* co_dcli.c 
   Command line interpreter. */

#if defined(OS_ELN)
# include stdarg
# include stdio
# include stdlib
# include signal
# include string
# include float
# include math
#else
# include <stdarg.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <float.h>
# include <math.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "co_time.h"
#include "co_cdh.h"
#include "co_ccm.h"
#include "co_dcli.h"
#include "co_dcli_msg.h"


/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define r_toupper(c) (((c) >= 'a' && (c) <= 'z') ? (c) & 0xDF : (c))
#define r_tolower(c) (((c) >= 'A' && (c) <= 'Z') ? (c) | 0x20 : (c))

#define DCLI_SUBST_SLASH 3
#define DCLI_SUBST_EQUAL 4

typedef	struct	{
	char	key[DCLI_SYM_KEY_SIZE];
	char	value[DCLI_SYM_VALUE_SIZE];
	} dcli_t_symboltable;

#define	DCLI_SYMBOLTABLE_SIZE 500
#define CMD_BUFF_SIZE (DCLI_SYMBOLTABLE_SIZE * 160)

static char		dcli_qual_str[10][2][400];
static dcli_t_symboltable dcli_symboltable[DCLI_SYMBOLTABLE_SIZE];
static int		dcli_symboltable_count;

static int	dcli_symbol_to_int( int	*int_value, char *value);
static int	dcli_symbol_to_float( float *int_value, char *value);
#if 0
static int	dcli_get_attr_value(
			char		*attrname,
			char		*value);
#endif
static void dcli_message( char s, char *text);

/*************************************************************************
*
* Name:		dcli_parse()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*string		I	string to be parsed.
* char		*parse_char	I	parse charachter(s).
* char		*inc_parse_char	I	parse charachter(s) that will be
*					included in the parsed string.
* char		*outstr		O	parsed strings.
* int		max_rows	I	maximum number of chars in a parsed
*					string.
* int 		max_cols	I	maximum number of parsed elements.
*
* Description:
*	Parses a string.
*
**************************************************************************/

int	dcli_parse( 	char	*string,
			char	*parse_char,
			char	*inc_parse_char,
			char	*outstr,
			int	max_rows,
			int 	max_cols,
			int	keep_quota)
{
	int	row;
	int	col;
	char	*char_ptr;	
	char	*inc_char_ptr;	
	int	parsechar_found;
	int	inc_parsechar_found;
	int	next_token;
	int	char_found;
	int	one_token = 0;
	unsigned char	prev_char;
	int	nullstr;

	prev_char = 0;
	row = 0;
	col = 0;
	char_found = 0;
	next_token = 0;
	nullstr = 0;
	while ( *string != '\0')	
	{
	  char_ptr = parse_char;
	  inc_char_ptr = inc_parse_char;
	  parsechar_found = 0;
	  inc_parsechar_found = 0;
	  if ( *string == '"' && prev_char != '\\')
	  {
	    one_token = !one_token;
	    prev_char = (unsigned char) *string;
 	    if ( !one_token && col == 0)
 	      nullstr = 1;
	    else
	      nullstr = 0;
	    if ( !keep_quota)
	    {
	      string++; 
	      continue;
	    }
	  }
	  else if ( *string == '"' && prev_char == '\\')
	    col--;
	  if ( !one_token)
	  {
	    while ( *char_ptr != '\0')
	    {
	      /* Check if this is a parse charachter */
	      if ( *string == *char_ptr)
	      {
	        parsechar_found = 1;
	        /* Next token */
	        if ( col > 0 || nullstr)	
	        {
	          *(outstr + row * max_cols + col) = '\0';
	          row++;
	          if ( row >= max_rows )
	            return row;
	          col = 0;
	          next_token = 0;
	        }
	        break;
	      }
	      char_ptr++;	    
	    }
	    while ( *inc_char_ptr != '\0')
	    {
	      /* Check if this is a parse charachter */
	      if ( *string == *inc_char_ptr)
	      {
	        parsechar_found = 1;
	        inc_parsechar_found = 1;
	        /* Next token */
	        if ( col > 0 || nullstr)	
	        {
	          *(outstr + row * max_cols + col) = '\0';
	          row++;
	          if ( row >= max_rows )
	            return row;
	          col = 0;
	          next_token = 0;
	        }
	        break;
	      }
	      inc_char_ptr++;	    
	    }
	  }
	  if ( !parsechar_found && !next_token)
	  {
	    char_found++;
	    *(outstr + row * max_cols + col) = *string;
	    col++;
	  }
	  if ( inc_parsechar_found)
	  {
	    *(outstr + row * max_cols + col) = *inc_char_ptr;
	    col++;
	  }
	  prev_char = (unsigned char) *string;
	  string++; 
	  if ( col >= (max_cols - 1))
	    next_token = 1;	    
	  nullstr = 0;
	}
	*(outstr + row * max_cols + col) = '\0';
	row++;

	if ( char_found == 0)
	  return 0;

	return row;
}

/*************************************************************************
*
* Name:		dcli_cli()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* dcli_tCmdTable *command_table	I	command table.
* char		*string		I	command string.
* unsigned long	userdata	I	argument passed to the command
*					function.
*
* Description:
*	This function identifies a command described int the command table
*	in the command string and calls the command function for this
*	command with the userdata as argument.
*	Qualifiers and parameters are stored and can be fetched by
*	dcli_get_qualifier.
*
**************************************************************************/

int	dcli_cli( 	dcli_tCmdTable	*command_table,
			char		*string,
			void		*userdata1,
			void		*userdata2)
{
	char	out_str[20][DCLI_QUAL_SIZE];
	char	value_str[10][DCLI_QUAL_SIZE];
	int	nr, i, j, valuenr;
	int	hitnr, sts;
	char	command[DCLI_CMD_SIZE];
	int	(*func) ();
	dcli_tCmdTable	*comtbl_ptr;
	dcli_tCmdTable	*current_comtbl;
	int		arg_count;
	char	qual[80];
	char	*s, *t;
	int	is_arg;
	int	quota_mode;

	/* Fill spaces around '=' with '=' , this is a trick to avoid
	   parsing of the qualifier and value in the first step */
	s = string;
	quota_mode = 0;
	while ( *s != 0)
	{
	  if ( *s == '"')
	    quota_mode = !quota_mode;
	  if ( quota_mode)
	  {
	    /* Replace / and = to avoid parsing */
	    if ( *s == '/')
	      *s = DCLI_SUBST_SLASH;
	    if ( *s == '=')
	      *s = DCLI_SUBST_EQUAL;
	    s++;
	    continue;
	  }
	  if ( *s == '=')
	  {
	    t = s - 1;
	    while ( (*t == ' ') && (*t != 0))
	    {
	      *t = '=';	
	      t--;
	    }
	    t = s + 1; 
	    while ( (*t == ' ') && (*t != 0))
	    {
	      *t = '=';	
	      t++;
	    }
	    s = t - 1;	   
	  }
	  s++;
	}


	/* Parse the command string */
	nr = dcli_parse( string, " ", "/", (char *)out_str, 
		sizeof( out_str) / sizeof( out_str[0]), sizeof( out_str[0]), 0);

	if ( nr == 0)
	  return DCLI__NOCOMMAND;

	/* Find the command in the command table */
	comtbl_ptr = command_table;
	hitnr = 0;
	while ( comtbl_ptr->command[0] != '\0')
	{
	  strcpy( command, comtbl_ptr->command);
	  if ( strcmp( out_str[0], command) == 0)
	  {
	    /* Perfect hit */
	    func = comtbl_ptr->func;
	    hitnr = 1;
	    current_comtbl = comtbl_ptr;
	    break;
	  }
	  else
	  {
	    command[ strlen( out_str[0])] = '\0';
	    if ( strcmp( out_str[0], command) == 0)
	    {
	      /* Hit */
	      func = comtbl_ptr->func;
	      hitnr++;
	      current_comtbl = comtbl_ptr;
	    }
	  }
	  comtbl_ptr++;
	}	   

	if ( hitnr > 1)
	{
	  /* Command not unic */
	  return DCLI__COM_AMBIG;
	}
	else if ( hitnr < 1)
	{
	  /* Command not defined */
	  return DCLI__COM_NODEF;
	}

	/* Identify the qualifiers */
	arg_count = 0;
	for ( i = 1; i < nr; i++)
	{
	  valuenr = dcli_parse( (char *)out_str[i], "=", "",
		(char *) value_str, sizeof( value_str)/sizeof( value_str[0]), 
		sizeof( value_str[0]), 1);
	  if ( valuenr > 1)
	  {
	    strcpy( dcli_qual_str[i-1][1], value_str[1]);
	    for ( j = 2; j < valuenr; j++)
	    {
	      strcat( dcli_qual_str[i-1][1], "=");	  
	      strcat( dcli_qual_str[i-1][1], value_str[j]);	  
	    }
	  }
	  else
	    strcpy( dcli_qual_str[i-1][1], "");	  

	  /* Check if this qualifier is ok */
	  j = 0;
	  hitnr = 0;
	  is_arg = 0;
	  if ( value_str[0][0] == 0 )
	    /* Null string sent as an argument */
	    is_arg = 1;
	  else
          {
	    while ( current_comtbl->qualifier[j][0] != 0)
	    {	  
	      strcpy( qual, current_comtbl->qualifier[j]);
	      qual[ strlen( value_str[0])] = '\0';
	      if ( strcmp( qual, value_str[0]) == 0)
	      {
	        /* Hit */
	        strcpy( dcli_qual_str[i-1][0], current_comtbl->qualifier[j]);
	        hitnr++;
	      }
	      j++;
	    }
	  }
	  if ( hitnr == 0 || is_arg)
	  {
	    /* This might be a argument, look for a argument */
	    if ( strncmp( current_comtbl->qualifier[arg_count], 
			"dcli_arg", 7) == 0)
	    {	  
	      sprintf( dcli_qual_str[i-1][0], "dcli_arg%d", arg_count+1);	
	      strcpy( dcli_qual_str[i-1][1], value_str[0]); 
	      arg_count++;
	    }
	    else
	    {
	      /* qualifier not found */
	      return DCLI__QUAL_NODEF;
	    }
	  }
	  else if ( hitnr > 1)
	  {
	    /* qualifier not unic */
	    return DCLI__QUAL_AMBIG;
	  }
	  /* Place back the / and = within quotes */
	  for ( s = dcli_qual_str[i-1][1]; *s != 0; s++)
	  {
	    if ( *s == DCLI_SUBST_SLASH)
	      *s = '/';
	    if ( *s == DCLI_SUBST_EQUAL)
	      *s = '=';
	  }
	}
	strcpy( dcli_qual_str[nr-1][0], "");

	sts = (func) (userdata1, userdata2);
	return sts;
}

/*************************************************************************
*
* Name:		dcli_get_qualifier()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*qualifier	I	
* char		*value		O	
*
* Description:
*	Returns the value of a specified qualifier.
*	If no value is found a NULL string is returned.
*	If the qualifier is not found DCLI__QUAL_NOFOUND is returned as
*	statuscode.
*
**************************************************************************/

int	dcli_get_qualifier( 	char	*qualifier,
				char	*value,
				size_t	size)
{
	int	i, found;

	i = 0;	
	found = 0;
	while ( dcli_qual_str[i][0][0] != '\0')
	{
	  if ( strcmp( qualifier, (char *) dcli_qual_str[i]) == 0)
	  {
	    /* Hit */
            if ( value) {
	      if ( strlen( dcli_qual_str[i][1]) > size - 1)
		return DCLI__TOOLONG;
	      strcpy( value, dcli_qual_str[i][1]);
	    }
	    found = 1;
	  }
	  i++;
	}	   
	if ( !found)
	{
	  /* qualifier is not found */
          if ( value)
	    *value = 0;
	  return DCLI__QUAL_NOFOUND;
	}
	  
	return DCLI__SUCCESS;
}


/*************************************************************************
*
* Name:		dcli_store_symbols()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Store all symbols in a rtt commandfile.
*
**************************************************************************/
int	dcli_store_symbols( char	*filename)
{
	char	filename_str[80];
	FILE	*file;
	char	message[120];
	char	*s;
	int	i;

	strcpy( filename_str, filename);

	/* Open the file */
	file = fopen( filename_str, "w");
	if ( file == 0)
	  return DCLI__NOFILE;

	for ( i = 0; i < dcli_symboltable_count; i++)
	{
	  fprintf( file, "define %s \"", dcli_symboltable[i].key);
	  for ( s = dcli_symboltable[i].value; *s; s++)
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
/*	dcli_fgetname( file, filename_str, filename_str); */
	fclose( file);
	sprintf( message, "Symbols stored in %s", filename_str);
	dcli_message('I', message);
	return DCLI__SUCCESS;	
}

/*************************************************************************
*
* Name:		dcli_show_symbols()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Show all symbols in a view-window.
*
**************************************************************************/
#if 0
int	dcli_show_symbols( menu_ctx	ctx)
{
	char 	*buff;
	int	i;
	int	sts;

	if ( dcli_symboltable_count == 0)
	{
	  dcli_message('I',"No symbol is defined");
	  return DCLI__NOSYMDEFINED;
	}
	buff = calloc( 1, CMD_BUFF_SIZE);
	sprintf( buff, " %-20.20s  %s\n\n", "Symbol", "Value");
	for ( i = 0; i < dcli_symboltable_count; i++)
	{
	  sprintf( buff+strlen(buff), " %-20.20s  %s\n", 
		dcli_symboltable[i].key, dcli_symboltable[i].value);
	}
	sts = dcli_view( ctx, 0, buff, "Rtt Symbols", 
			DCLI_VIEWTYPE_BUF);
	free( buff);
	return sts;
}
#endif
/*************************************************************************
*
* Name:		dcli_replace_symbol()
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
int	dcli_replace_symbol( char *command, char *newcommand, int newsize)
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
	char	new[1000];
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
	      cdh_ToUpper( upper_symbol, symbol);
	      sts = dcli_get_symbol( upper_symbol, value);
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

        if ( strlen(new) >= newsize)
          return DCLI__STRLONG;
	strcpy( newcommand, new);
	return DCLI__SUCCESS;
}

/*************************************************************************
*
* Name:		dcli_get_symbol()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get a symbol from the symbol table.
*
**************************************************************************/
int	dcli_get_symbol( char *key, char *value)
{
	int	i;

	for ( i = 0; i < dcli_symboltable_count; i++)
	{
	  if ( !strcmp( dcli_symboltable[i].key, key))
	  {
	    strcpy( value, dcli_symboltable[i].value);
	    return DCLI__SUCCESS;
	  }
	}
	return DCLI__NOSYMBOL;
}

int	dcli_get_symbol_by_index( int index, char *key, char *value)
{
	if ( index >= dcli_symboltable_count)
	  return DCLI__NOSYMBOL;

	strcpy( key, dcli_symboltable[index].key);
	strcpy( value, dcli_symboltable[index].value);
	return DCLI__SUCCESS;
}

/*************************************************************************
*
* Name:		dcli_get_cmd_symbol()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get a symbol from the symbol table. Abbrevations is allowed.
*
**************************************************************************/
int	dcli_get_symbol_cmd( char *key, char *value)
{
	int	i;
	int	found;

	found = 0;
	for ( i = 0; i < dcli_symboltable_count; i++)
	{
	  if ( !strcmp( dcli_symboltable[i].key, key))
	  {
	    /* Perfect match */
	    strcpy( value, dcli_symboltable[i].value);
	    return DCLI__SUCCESS;
	  }
	  else if ( !strncmp( dcli_symboltable[i].key, key, strlen(key)))
	  {
	    if ( found)
	      return DCLI__SYMBOL_AMBIG;
	    strcpy( value, dcli_symboltable[i].value);
	    found = 1;
	  }
	}
	if ( found)
	  return DCLI__SUCCESS;

	return DCLI__NOSYMBOL;
}

/*************************************************************************
*
* Name:		dcli_define_symbol()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Define a symbol in the symbol table.
*
**************************************************************************/
int	dcli_define_symbol( char *key, char *arg1, char *arg2, char *arg3)
{
	int	i;
	char	value[DCLI_SYM_VALUE_SIZE];
	int	int_value;
	float	float_value;
	int	int_value1;
	float	float_value1;
	int	int_value2;
	float	float_value2;
	int	sts;

	if ( dcli_symboltable_count >= DCLI_SYMBOLTABLE_SIZE)
	{
	  return DCLI__SYMTBLFULL;
	}
	if ( strlen( key) > sizeof(dcli_symboltable[0].key) - 1)
	{
	  return DCLI__SYMNAMETOLONG;
	}

	/* Check if arg1 is an operator */
	if ( !strcmp( arg1, "++"))
	{
	  /* Get the old value */
	  sts = dcli_get_symbol( key, value);
	  if ( EVEN(sts))
	  {
	    return DCLI__SYMNOTFOUND;
	  }

	  /* convert to integer or float */
	  if ( ODD( dcli_symbol_to_int( &int_value, value)))
	  {
	    int_value++;
	    sprintf( value, "%d", int_value);
	  }
	  else if ( ODD( dcli_symbol_to_float( &float_value, value)))
	  {
	    float_value++;
	    sprintf( value, "%f", float_value);
	  }
	  else
	  {
	    return DCLI__SYMTYPE;
	  }
	}
	else if ( !strcmp( arg1, "--"))
	{
	  /* Get the old value */
	  sts = dcli_get_symbol( key, value);
	  if ( EVEN(sts))
	  {
	    return DCLI__SYMNOTFOUND;
	  }

	  /* convert to integer or float */
	  if ( ODD( dcli_symbol_to_int( &int_value, value)))
	  {
	    int_value--;
	    sprintf( value, "%d", int_value);
	  }
	  else if ( ODD( dcli_symbol_to_float( &float_value, value)))
	  {
	    float_value--;
	    sprintf( value, "%f", float_value);
	  }
	  else
	  {
	    return DCLI__SYMTYPE;
	  }
	}
	else if ( arg2 == 0)
	{
	  /* arg1 is the value */
	  cdh_StrncpyCutOff( value, arg1, sizeof(value), 0);
	}
	else
	{
	  /* arg2 is the operator */
	  if ( arg3 == 0)
	  {
	    return DCLI__SYNTAX;
	  }
	  if ( !strcmp( arg2, "+"))
	  {
	    if ( ODD( dcli_symbol_to_int( &int_value1, arg1)) &&
	         ODD( dcli_symbol_to_int( &int_value2, arg3)))
	    {
	      int_value = int_value1 + int_value2;
	      sprintf( value, "%d", int_value);
	    }
	    else if ( ODD( dcli_symbol_to_float( &float_value1, arg1)) &&
	              ODD( dcli_symbol_to_float( &float_value2, arg3)))
	    {
	      float_value = float_value1 + float_value2;
	      sprintf( value, "%f", float_value);
	    }
	    else
	    {
	      return DCLI__SYMTYPE;
	    }
	  }
	  else if ( !strcmp( arg2, "-"))
	  {
	    if ( ODD( dcli_symbol_to_int( &int_value1, arg1)) &&
	         ODD( dcli_symbol_to_int( &int_value2, arg3)))
	    {
	      int_value = int_value1 - int_value2;
	      sprintf( value, "%d", int_value);
	    }
	    else if ( ODD( dcli_symbol_to_float( &float_value1, arg1)) &&
	              ODD( dcli_symbol_to_float( &float_value2, arg3)))
	    {
	      float_value = float_value1 - float_value2;
	      sprintf( value, "%f", float_value);
	    }
	    else
	    {
	      return DCLI__SYMTYPE;
	    }
	  }
	  else if ( !strcmp( arg2, "*"))
	  {
	    if ( ODD( dcli_symbol_to_int( &int_value1, arg1)) &&
	         ODD( dcli_symbol_to_int( &int_value2, arg3)))
	    {
	      int_value = int_value1 * int_value2;
	      sprintf( value, "%d", int_value);
	    }
	    else if ( ODD( dcli_symbol_to_float( &float_value1, arg1)) &&
	              ODD( dcli_symbol_to_float( &float_value2, arg3)))
	    {
	      float_value = float_value1 * float_value2;
	      sprintf( value, "%f", float_value);
	    }
	    else
	    {
	      return DCLI__SYMTYPE;
	    }
	  }
	  else if ( !strcmp( arg2, "/"))
	  {
	    if ( ODD( dcli_symbol_to_int( &int_value1, arg1)) &&
	         ODD( dcli_symbol_to_int( &int_value2, arg3)))
	    {
	      int_value = int_value1 / int_value2;
	      sprintf( value, "%d", int_value);
	    }
	    else if ( ODD( dcli_symbol_to_float( &float_value1, arg1)) &&
	              ODD( dcli_symbol_to_float( &float_value2, arg3)))
	    {
	      float_value = float_value1 / float_value2;
	      sprintf( value, "%f", float_value);
	    }
	    else
	    {
	      return DCLI__SYMTYPE;
	    }
	  }
	}

	if ( strlen( value) > sizeof(dcli_symboltable[0].value) - 1)
	{
	  return DCLI__SYMVALTOLONG;
	}

	cdh_ToUpper( value, value);
	/* Look if the symbol exists */

	for ( i = 0; i < dcli_symboltable_count; i++)
  	{
	  if ( !strcmp( dcli_symboltable[i].key, key))
	  {
	    strcpy( dcli_symboltable[i].value, value);
	    return DCLI__SUCCESS;
	  }
	}

	strcpy( dcli_symboltable[i].key, key);
	strcpy( dcli_symboltable[i].value, value);
	dcli_symboltable_count++;


	return DCLI__SYMDEFINED;
}


static int	dcli_symbol_to_int( int	*int_value, char *value)
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

static int	dcli_symbol_to_float( float *float_value, char *value)
{
	int	nr;

	nr = sscanf( value, "%f", float_value);
	if ( nr != 1)
	  return 0;
	return 1;
}


/*************************************************************************
*
* Name:		dcli_command_toupper()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*str		I	input string.
* char		*upper_str	I	string converted to upper case.
*
* Description:
*	Converts a string to upper case.
*	Text between two '"' will not be affekted.
*
**************************************************************************/

int dcli_toupper( 	char	*str_upper,
			char	*str)
{
	char	namechar;
	char	*u;
	char	*t; 
	int	convert;
	unsigned char prev_char;

	/* Convert to upper case */
	u = str;
	t = str_upper;
	prev_char = 0;
	convert = 1;
	while ( *u != '\0')
	{
	  namechar = *(u++);
	  if ( namechar == '"' && prev_char != '\\')
	    convert = !convert;
	  if (convert)
	  {
	    *t = (char) r_toupper( namechar);
	    if (*t == '�') *t = (char) '�';
	    else if (*t == '�') *t = (char) '�';
	    else if (*t == '�') *t = (char) '�';
	  }
	  else
	    *t = namechar;
	  prev_char = (unsigned char) *t;
	  t++;
	}
	*t = '\0';

	return DCLI__SUCCESS;
}

static void dcli_message( char s, char *text)
{
  printf( "%%DCLI-%c-MSG, %s\n", s, text);
}

int	dcli_trim( char *out_str, char *in_str)
{
  return dcli_remove_blank( out_str, in_str);
}

int	dcli_remove_blank( char *out_str, char *in_str)
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
	return 1;
}


char	*dcli_pwr_dir( char *dir)
{
	static char pwr_dir[120];
	char 	*s;

#if defined(OS_VMS)
	strcpy( pwr_dir, dir);
	strcat( pwr_dir, ":");
#elif defined(OS_LYNX) || defined(OS_LINUX)
	if ( (s = getenv( dir)) == NULL)
	  strcpy( pwr_dir, "");
        else
	{
	  strcpy( pwr_dir, s);
	  strcat( pwr_dir, "/");
	}
#endif
	return pwr_dir;
}

/*************************************************************************
*
* Name:		dcli_wildcard()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*wildname	I	wildcard name.
* char		*name		I	object name.
*
* Description:
*	Checks if the object name can be described by the
*	wildcard string.
*	Returns 0 if ok, else 1.
*
**************************************************************************/

int dcli_wildcard(	char	*wildname,
			char	*name)
{
	int	len;
	char	*s;
	char	*t;
	char	*u;
	char	checkstr[400];
	char	upper_name[400];

	/* Convert to upper case */
	cdh_ToUpper( upper_name, name);

	t = wildname;
	u = upper_name;
	s = strchr( t, '*');
	if ( s == 0)
	{
	  if (strcmp( t, u) == 0)
	    return 0;
	  else 
	    return 1;
	}

	len = s - t;
	if ( len > 0)
	{
	  strncpy ( checkstr, t, len);
	  checkstr[len] = '\0';
	  if ( strncmp ( checkstr, u, len) != 0)
	    return 1;
	  u += len;
	}	  
	t += len + 1;
	s = strchr( t, '*');

	while ( s != 0)
	{
	  len = s - t;
	  if ( len > 0)
	  {
	    strncpy ( checkstr, t, len);
	    checkstr[len] = '\0';
	    u = strstr( u, checkstr);
	    if ( u == 0)
	      return 1;
	    u += len;
	  }	  
	  t += len + 1;
	  s = strchr( t, '*');
	}
	strcpy ( checkstr, t);
	u = u + strlen(u) - strlen (checkstr);
	if ( strcmp ( checkstr, u ) != 0)
	      return 1;

	return 0;
}

/*************************************************************************
*
* Name:		dcli_read_line()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Read a line from a file.
*
**************************************************************************/

int dcli_read_line( char *line, int maxsize, FILE *file)
{ 
	char	*s;

	if (fgets( line, maxsize, file) == NULL)
	  return 0;
	line[maxsize-1] = 0;
	s = strchr( line, 10);
	if ( s != 0)
	  *s = 0;

	return 1;
}
