/** 
 * Proview   $Id: co_dcli_struct.c,v 1.3 2007-04-25 13:39:21 claes Exp $
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

/* co_dcli_struct.c 
   This module contains routines for reading an interpreting struct-files. */

/*_Include files_________________________________________________________*/

#if defined(OS_ELN)
# include stdio
# include stdarg
# include stdlib
# include chfdef
# include signal
# include string
# include float
#else
# include <stdio.h>
# include <stdarg.h>
# include <stdlib.h>
# include <string.h>
# include <float.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "co_cdh.h"
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

#define READSTRUCT_CALLER_ROOT		0
#define READSTRUCT_CALLER_UNDEFSEARCH	1
#define READSTRUCT_CALLER_INCLUDE	2

#define READSTRUCT_DEFINESIZE		40

typedef struct s_define {
	char	define[READSTRUCT_DEFINESIZE];
	int	value;
	struct s_define *next;
	} t_define;

typedef struct {
	int		lines;
	dcli_sStructElement *element_list;
	char		filename[120];
	FILE		*file;
	int		parlevel;
	int		struct_proc;
	int		file_level;
	t_define	*definelist;
	} *t_filectx;

typedef struct {
	char	root_filename[120];
	int	error_line;
	char	error_file[120];
	} *t_ctx;

static char dcli_message[180];
static int dcli_message_set = 0;

/*___Local function prototypes_______________________________________________*/

static int find_define( 
		t_filectx	filectx,
		char		*str,
		int		*value);
static int add_element( 	t_ctx		ctx,
				t_filectx	filectx,
				char 		*line,
				dcli_sStructElement **element_list);
static int	find_struct( t_ctx	ctx,
			char *filename,
			char *struct_name, 
			dcli_sStructElement **e_list, 
			int caller);
static int	process_struct( 	t_ctx		ctx,
					t_filectx	filectx,
					char		*struct_line,
					dcli_sStructElement *struct_element);

/*___Local functions_________________________________________________________*/

/*************************************************************************
*
* Name:		process_struct()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Analyses an element in a struct of the type struct.
*
**************************************************************************/

static int	process_struct( 	t_ctx		ctx,
					t_filectx	filectx,
					char		*struct_line,
					dcli_sStructElement *struct_element)
{
	char	line[200];
	int	sts;
	int	struct_found;
	int	name_found;
	int	start_found;
	int	typedef_line;
	int	begin_line;
	int	begin_typedef;
	int	end_of_typedef;
	int	start_parlevel;
	char	*s1, *s2;
	int	elnumcount;
	int	elnum[5];
	char	*s, *t;
	char	*begin_addr, *end_addr;
	char	name[40];
	int	i, j, k;
	dcli_sStructElement *element_p;
	dcli_sStructElement *e_p;
	dcli_sStructElement *element_ptr;
	int	first;
	int	nr;
	int	count;
	char	tmp[40];
	int	highest_struct_proc;

	if ( filectx->struct_proc == 0)
	{
	  highest_struct_proc = 1;
	  filectx->struct_proc = 1;
	}
	else
	  highest_struct_proc = 0;

	strcpy( line, struct_line);
	typedef_line = filectx->lines;
	start_parlevel = filectx->parlevel;
	end_of_typedef = 0;
	typedef_line = 0;
	struct_found = 0;
	first = 1;
	name_found = 0;
	begin_line = filectx->lines;
	begin_typedef = filectx->lines;
	begin_addr = strstr(line, "struct") + strlen("struct");

	while( 1)
	{

	  /* Search for end of struct */
	  for ( s = line; *s != 0; s++)
	  {
	    if ( *s == '{')
	    {
	      if ( first && !highest_struct_proc)
	        /* parlevel is already increased by calling function */
	        start_parlevel--;
	      else
	        filectx->parlevel++;
	    }
	    else if ( *s == '}')
	    {
	      if ( first && !highest_struct_proc)
	        /* parlevel is already decreased by calling function */
	        start_parlevel++;
	      else
	        filectx->parlevel--;
	      if ( filectx->parlevel == start_parlevel)
	      {
	        begin_line = filectx->lines;
	        begin_addr = s;
	      }
	    }
	    else if ( *s == ';' && filectx->parlevel == start_parlevel)
	    {
	      end_of_typedef = 1;
	      end_addr = s;
	    }

	    if ( end_of_typedef)
	    {
	      if ( begin_line != filectx->lines)
	      {
	        strcpy( ctx->error_file, filectx->filename);
	        ctx->error_line = filectx->lines;
	        return DCLI__ENDSTRUCTERR;
	      }
	      /* The name of the typdef should be found between
	         begin_addr and end_addr */
	      *end_addr = 0;
	      begin_addr++;
	      start_found = 0;
	      t = name;
	      for ( s = begin_addr; *s; s++)	        
	      {
	        if ( *s == ' ' || *s == 9)
	        {
	          if ( !start_found)
	            continue;
	          else
	            break;
	        }
	        else
	        {
	          name_found = 1;
	          *t = *s;
	          t++;
	        }
	      }
	      *t = 0;
	    }
	    if ( name_found)
	      break;
	  }
	  if ( name_found)
	  {
	    /* Add structname to all elements, and if an array of structs */
	    /* copy the elementlist */
	    
	    s1 = name;
	    i = 0;
	    elnumcount = 0;
	    while ( (s2 = strchr( s1, '[')) != 0)
	    {
	      /* Extract number of elements */
	      *s2 = 0;
	      s2++;
	      if ( (s1 = strchr( s2, ']')) == 0)
	      {
	        strcpy( ctx->error_file, filectx->filename);
	        ctx->error_line = filectx->lines;
	        return DCLI__STRUCTSYNTAX;
	      }
	      *s1 = 0;
	      s1++;
	      nr = sscanf( s2, "%d", &elnum[i]);
	      if ( nr != 1)
	      {
	        sts = find_define( filectx, s2, &elnum[i]);
	        if ( EVEN(sts))
	        {
	          strcpy( ctx->error_file, filectx->filename);
	          ctx->error_line = filectx->lines;
	          return DCLI__STRUCTSYNTAX;
	        }
	      }
	      i++;
	      elnumcount++;
	    }
	    count = 0;
	    for ( element_p = struct_element->next; element_p; element_p = element_p->next)
	      count++;
	    strcpy( struct_element->name, name);
	    if ( elnumcount == 0)
	    {
	      for ( element_p = struct_element->next; element_p; element_p = element_p->next)
	      {
	        strcpy( tmp, element_p->name);
	        strcpy( element_p->name, name);
	        strcat( element_p->name, ".");
	        strcat( element_p->name, tmp);
	      }
	    }
	    else if ( elnumcount == 1)
	    {
	      for ( i = 1; i < elnum[0]; i++)
	      {
	        element_p = struct_element->next;
	        for ( j = 0; j < count; j++)
		{
	          
	          element_ptr = calloc( 1, sizeof( *element_ptr));
	          memcpy( element_ptr, element_p, sizeof(*element_p));
	          for ( e_p = filectx->element_list; e_p->next; e_p = e_p->next) ;
	          e_p->next= element_ptr;
	          element_ptr->prev = e_p;
	          element_ptr->next = 0;

	          element_p = element_p->next;
	        }
	      }
	      element_p = struct_element->next;
	      for ( i = 0; i < elnum[0]; i++)
	      {
	        for ( j = 0; j < count; j++)
		{
	          strcpy( tmp, element_p->name);
	          sprintf( element_p->name, "%s[%d].%s", name, i, tmp);
	          element_p = element_p->next;
	        }
	      }
	    }
	    else if ( elnumcount == 2)
	    {
	      for ( i = 0; i < elnum[0]; i++)
	      {
	        for ( k = 0; k < elnum[1]; k++)
	        {
	          if ( i == 0 && k == 0)
	            continue;
	          element_p = struct_element->next;
	          for ( j = 0; j < count; j++)
		  {
	          
	            element_ptr = calloc( 1, sizeof( *element_ptr));
	            memcpy( element_ptr, element_p, sizeof(*element_p));
	            for ( e_p = filectx->element_list; e_p->next; e_p = e_p->next) ;
	            e_p->next= element_ptr;
	            element_ptr->prev = e_p;
	            element_ptr->next = 0;

	            element_p = element_p->next;
	          }
	        }
	      }

	      element_p = struct_element->next;
	      for ( i = 0; i < elnum[0]; i++)
	      {
	        for ( k = 0; k < elnum[1]; k++)
	        {
	          for ( j = 0; j < count; j++)
		  {
	            strcpy( tmp, element_p->name);
	            sprintf( element_p->name, "%s[%d][%d].%s", name, i, k, tmp);
	            element_p = element_p->next;
	          }
	        }
	      }
	    }
	    else
	    {
	      strcpy( ctx->error_file, filectx->filename);
	      ctx->error_line = filectx->lines;
	      return DCLI__ARRAYDIM;
	    }
	    break;
	  }
	  if ( !first)
	  {
	    sts = add_element( ctx, filectx, line, &filectx->element_list);
	    if ( EVEN(sts)) return sts;
	  }
	  else
	    first = 0;

	  if ( dcli_read_line( line, sizeof(line), filectx->file) == 0)
	    return DCLI__EOF;
	  filectx->lines++;
	}

	if ( highest_struct_proc)
	  filectx->struct_proc = 0;

	return DCLI__SUCCESS;
}

static void free_filectx( t_filectx	filectx)
{
	t_define	*define_ptr, *next_ptr;

	for ( define_ptr = filectx->definelist; define_ptr; define_ptr = next_ptr)
	{
	  next_ptr = define_ptr->next;
	  free( define_ptr);
	}
	free( filectx);
}

static int find_define( 
		t_filectx	filectx,
		char		*str,
		int		*value)
{
	char		def[READSTRUCT_DEFINESIZE];
	t_define	*define_ptr;

	/* Remove tabs and spaces */
	dcli_remove_blank( def, str);
	for ( define_ptr = filectx->definelist; define_ptr; define_ptr = define_ptr->next)
	{
	  if ( strcmp( define_ptr->define, def) == 0)
	  {
	    *value = define_ptr->value;
	    return DCLI__SUCCESS;
	  }
	}
	return 0;
}

/*************************************************************************
*
* Name:		add_element()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Detect the type and size of an element in a struct.
*	Create an entry in the description list for the element.
*
**************************************************************************/

static int add_element( 	t_ctx		ctx,
				t_filectx	filectx,
				char 		*line,
				dcli_sStructElement **element_list)
{
	int	sts;
	char	line_elem[4][80];
	int	nr;
	int	unsign;
	int	i, j;
	int	type;
	int	size;
	char	*s1, *s2;
	int	elnumcount;
	int	elnum[5];
	dcli_sStructElement *element_ptr;
	dcli_sStructElement *element_p;
	int	undefined;
	int	struct_begin;
	unsigned int mask;

	memset( line_elem, 0, sizeof(line_elem));
	nr = dcli_parse( line, " 	;", "",
		(char *) line_elem, sizeof( line_elem)/sizeof( line_elem[0]), 
		sizeof( line_elem[0]), 1);
	if ( strcmp( line_elem[nr-1], "") == 0)
	  nr--;

	struct_begin = 0;
	undefined = 0;
	unsign = 0;
	mask = 0;
	if ( strcmp( line_elem[0], "unsigned") == 0)
	{
	  unsign = 1;
	  for ( i = 1; i < (int)(sizeof( line_elem)/sizeof( line_elem[0])); i++)
	    strcpy( line_elem[i-1], line_elem[i]);
	  nr--;
	}
	if ( nr == 0)
	  return DCLI__SUCCESS;



	undefined = 0;
	if ( strncmp( line_elem[0], "/*", 2) == 0)
	{
	  /* Assume the whole line is a comment */
	  return DCLI__SUCCESS;
	}
	else if ( strcmp( line_elem[0], "struct") == 0)
	{
	  struct_begin = 1;
	  type = 0;
	  size = 0;
	}
	else if ( strcmp( line_elem[0], "long") == 0)
	{
	  if ( strcmp( line_elem[1], "int") == 0)
	  {
	    for ( i = 1; i < (int)(sizeof( line_elem)/sizeof( line_elem[0])); i++)
	      strcpy( line_elem[i-1], line_elem[i]);
	    nr--;
	  }
	  if ( unsign)
	    type = pwr_eType_UInt32;
	  else
	    type = pwr_eType_Int32;
	  size = sizeof(pwr_tInt32);
	}
	else if ( strcmp( line_elem[0], "short") == 0)
	{
	  if ( strcmp( line_elem[1], "int") == 0)
	  {
	    for ( i = sizeof( line_elem)/sizeof( line_elem[0]) - 1; i > 0; i-- )
	      strcpy( line_elem[i-1], line_elem[i]);
	    nr--;
	  }
	  if ( unsign)
	    type = pwr_eType_UInt16;
	  else
	    type = pwr_eType_Int16;
	  size = sizeof(pwr_tInt16);
	}
	else if ( strcmp( line_elem[0], "char") == 0)
	{
	  if ( unsign)
	    type = pwr_eType_UInt8;
	  else
	    type = pwr_eType_Int8;
	  size = sizeof(pwr_tInt8);
	}
	else if ( strcmp( line_elem[0], "int") == 0)
	{
	  if ( unsign)
	    type = pwr_eType_UInt32;
	  else
	    type = pwr_eType_Int32;
	  size = sizeof(pwr_tInt32);
	}
	else if ( strcmp( line_elem[0], "float") == 0)
	{
	  type = pwr_eType_Float32;
	  size = sizeof(pwr_tFloat32);
	}
	else if ( strcmp( line_elem[0], "double") == 0)
	{
	  type = pwr_eType_Float64;
	  size = sizeof(pwr_tFloat64);
	}
	else if ( strcmp( line_elem[0], "pwr_tBoolean") == 0)
	{
	  type = pwr_eType_Boolean;
	  size = sizeof(pwr_tBoolean);
	}
	else if ( strcmp( line_elem[0], "pwr_tFloat32") == 0)
	{
	  type = pwr_eType_Float32;
	  size = sizeof(pwr_tFloat32);
	}
	else if ( strcmp( line_elem[0], "pwr_tFloat64") == 0)
	{
	  type = pwr_eType_Float64;
	  size = sizeof(pwr_tFloat64);
	}
	else if ( strcmp( line_elem[0], "pwr_tChar") == 0)
	{
	  type = pwr_eType_Char;
	  size = sizeof(pwr_tChar);
	}
	else if ( strcmp( line_elem[0], "pwr_tInt8") == 0)
	{
	  type = pwr_eType_Int8;
	  size = sizeof(pwr_tInt8);
	}
	else if ( strcmp( line_elem[0], "pwr_tInt16") == 0)
	{
	  type = pwr_eType_Int16;
	  size = sizeof(pwr_tInt16);
	}
	else if ( strcmp( line_elem[0], "pwr_tInt32") == 0)
	{
	  type = pwr_eType_Int32;
	  size = sizeof(pwr_tInt32);
	}
	else if ( strcmp( line_elem[0], "pwr_tInt64") == 0)
	{
	  type = pwr_eType_Int64;
	  size = sizeof(pwr_tInt64);
	}
	else if ( strcmp( line_elem[0], "pwr_tUInt8") == 0)
	{
	  type = pwr_eType_UInt8;
	  size = sizeof(pwr_tUInt8);
	}
	else if ( strcmp( line_elem[0], "pwr_tUInt16") == 0)
	{
	  type = pwr_eType_UInt16;
	  size = sizeof(pwr_tUInt16);
	}
	else if ( strcmp( line_elem[0], "pwr_tUInt32") == 0)
	{
	  type = pwr_eType_UInt32;
	  size = sizeof(pwr_tUInt32);
	}
	else if ( strcmp( line_elem[0], "pwr_tUInt64") == 0)
	{
	  type = pwr_eType_UInt64;
	  size = sizeof(pwr_tUInt64);
	}
	else if ( strcmp( line_elem[0], "pwr_tObjDId") == 0)
	{
	  type = pwr_eType_ObjDId;
	  size = sizeof(pwr_tObjDId);
	}
	else if ( strcmp( line_elem[0], "pwr_tObjid") == 0)
	{
	  type = pwr_eType_Objid;
	  size = sizeof(pwr_tObjid);
	}
	else if ( strcmp( line_elem[0], "pwr_tString256") == 0)
	{
	  type = pwr_eType_String;
	  size = sizeof(pwr_tString256);
	}
	else if ( strcmp( line_elem[0], "pwr_tString132") == 0)
	{
	  type = pwr_eType_String;
	  size = sizeof(pwr_tString132);
	}
	else if ( strcmp( line_elem[0], "pwr_tString80") == 0)
	{
	  type = pwr_eType_String;
	  size = sizeof(pwr_tString80);
	}
	else if ( strcmp( line_elem[0], "pwr_tString40") == 0)
	{
	  type = pwr_eType_String;
	  size = sizeof(pwr_tString40);
	}
	else if ( strcmp( line_elem[0], "pwr_tString32") == 0)
	{
	  type = pwr_eType_String;
	  size = sizeof(pwr_tString32);
	}
	else if ( strcmp( line_elem[0], "pwr_tString16") == 0)
	{
	  type = pwr_eType_String;
	  size = sizeof(pwr_tString16);
	}
	else if ( strcmp( line_elem[0], "pwr_tString8") == 0)
	{
	  type = pwr_eType_String;
	  size = sizeof(pwr_tString8);
	}
	else if ( strcmp( line_elem[0], "pwr_tString1") == 0)
	{
	  type = pwr_eType_String;
	  size = sizeof(pwr_tString1);
	}
	else if ( strcmp( line_elem[0], "pwr_tText1024") == 0)
	{
	  type = pwr_eType_Text;
	  size = sizeof(pwr_tText1024);
	}
	else if ( strcmp( line_elem[0], "pwr_tTime") == 0)
	{
	  type = pwr_eType_Time;
	  size = sizeof(pwr_tTime);
	}
	else if ( strcmp( line_elem[0], "pwr_tDeltaTime") == 0)
	{
	  type = pwr_eType_DeltaTime;
	  size = sizeof(pwr_tDeltaTime);
	}
	else if ( strcmp( line_elem[0], "pwr_sAttrRef") == 0)
	{
	  type = pwr_eType_AttrRef;
	  size = sizeof(pwr_sAttrRef);
	}
	else if ( strcmp( line_elem[0], "pwr_tUInt64") == 0)
	{
	  type = pwr_eType_UInt64;
	  size = sizeof(pwr_tUInt64);
	}
	else if ( strcmp( line_elem[0], "pwr_tInt64") == 0)
	{
	  type = pwr_eType_Int64;
	  size = sizeof(pwr_tInt64);
	}
	else if ( strcmp( line_elem[0], "pwr_tClassId") == 0)
	{
	  type = pwr_eType_ClassId;
	  size = sizeof(pwr_tClassId);
	}
	else if ( strcmp( line_elem[0], "pwr_tTypeId") == 0)
	{
	  type = pwr_eType_TypeId;
	  size = sizeof(pwr_tTypeId);
	}
	else if ( strcmp( line_elem[0], "pwr_tVolumeId") == 0)
	{
	  type = pwr_eType_VolumeId;
	  size = sizeof(pwr_tVolumeId);
	}
	else if ( strcmp( line_elem[0], "pwr_tObjectIx") == 0)
	{
	  type = pwr_eType_ObjectIx;
	  size = sizeof(pwr_tObjectIx);
	}
	else if ( strcmp( line_elem[0], "pwr_tRefId") == 0)
	{
	  type = pwr_eType_RefId;
	  size = sizeof(pwr_tRefId);
	}
	else
	{
	  undefined = 1;
	  type = 0;
	  size = 0;
	}

	/* Second arg is name */
	if ( nr < 2)
	  return DCLI__SUCCESS;

	if ( line_elem[1][0] == '*')
	{
	  mask = PWR_MASK_POINTER;
	  size = sizeof(pwr_tAddress);
	}

	elnumcount = 0;
	if ( !undefined)
	{
	  s1 = line_elem[1];	
	  i = 0;
	  while ( (s2 = strchr( s1, '[')) != 0)
	  {
	    /* Extract number of elements */
	    *s2 = 0;
	    s2++;
	    if ( (s1 = strchr( s2, ']')) == 0)
	    {
	      strcpy( ctx->error_file, filectx->filename);
	      ctx->error_line = filectx->lines;
	      return DCLI__STRUCTSYNTAX;
	    }
	    *s1 = 0;
	    s1++;
	    nr = sscanf( s2, "%d", &elnum[i]);
	    if ( nr != 1)
	    {
	      sts = find_define( filectx, s2, &elnum[i]);
	      if ( EVEN(sts))
	      {
	        strcpy( ctx->error_file, filectx->filename);
	        ctx->error_line = filectx->lines;
	        return DCLI__STRUCTSYNTAX;
	      }
	    }
	    i++;
	    elnumcount++;
	  }
	}
	if ( elnumcount == 0)
	{	
	  /* No elements found */
	  element_ptr = calloc( 1, sizeof( *element_ptr));
	  strcpy( element_ptr->name, line_elem[1]);
	  element_ptr->type = type;
	  element_ptr->size = size;
          strcpy( element_ptr->filename, filectx->filename);
          element_ptr->line_nr = filectx->lines;
	  element_ptr->mask = mask;
	    element_ptr->struct_begin = struct_begin;
	  element_ptr->undefined = undefined;
	  if ( undefined)
	    strcpy( element_ptr->typestr, line_elem[0]);
	  /* Insert last in list */
	  if ( *element_list == 0)
	    *element_list = element_ptr;
	  else
	  {
	    for ( element_p = *element_list; element_p->next; element_p = element_p->next) ;
	    element_p->next= element_ptr;
	    element_ptr->prev = element_p;
	  }
	}
	else if ( elnumcount == 1)
	{
	  for ( i = 0; i < elnum[0]; i++)
	  {
	    element_ptr = calloc( 1, sizeof( *element_ptr));
	    sprintf( element_ptr->name, "%s[%d]", line_elem[1], i);
	    element_ptr->type = type;
	    element_ptr->size = size;
	    element_ptr->mask = mask;
            strcpy( element_ptr->filename, filectx->filename);
            element_ptr->line_nr = filectx->lines;
	    element_ptr->undefined = undefined;
	    element_ptr->struct_begin = struct_begin;
	    /* Insert last in list */
	    if ( *element_list == 0)
	      *element_list = element_ptr;
	    else
	    {
	      for ( element_p = *element_list; element_p->next; element_p = element_p->next) ;
	      element_p->next= element_ptr;
	      element_ptr->prev = element_p;
	    }
	  }
	}
	else if ( elnumcount == 2)
	{
	  for ( i = 0; i < elnum[0]; i++)
	  {
	    for ( j = 0; j < elnum[1]; j++)
	    {
	      element_ptr = calloc( 1, sizeof( *element_ptr));
	      sprintf( element_ptr->name, "%s[%d][%d]", line_elem[1], i, j);
	      element_ptr->type = type;
	      element_ptr->size = size;
	      element_ptr->mask = mask;
              strcpy( element_ptr->filename, filectx->filename);
              element_ptr->line_nr = filectx->lines;
	      element_ptr->undefined = undefined;
	      element_ptr->struct_begin = struct_begin;
	      /* Insert last in list */
	      if ( *element_list == 0)
	        *element_list = element_ptr;
	      else
	      {
	        for ( element_p = *element_list; element_p->next; element_p = element_p->next) ;
	        element_p->next= element_ptr;
	        element_ptr->prev = element_p;
	      }
	    }
	  }
	}
	else
	{
	  strcpy( ctx->error_file, filectx->filename);
	  ctx->error_line = filectx->lines;
	  return DCLI__ARRAYDIM;
	}

	if ( struct_begin)
	{
	  sts = process_struct( ctx, filectx, line, element_ptr);
	  if (EVEN(sts)) return sts;
	}

	return DCLI__SUCCESS;
}

/*************************************************************************
*
* Name:		find_struct()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Opens an includfile and looks for a typedef of the specified struct.
*	Returns a list of the elements of in the struct.
*
**************************************************************************/

static int	find_struct( 	t_ctx	ctx,	
				char *filename,
				char *struct_name, 
				dcli_sStructElement **e_list, 
				int caller)
{
	FILE 	*file;
	char	line[200];
	int	size;
	int	sts;
	int	struct_found;
	int	name_found;
	int	start_found;
	int	typedef_line;
	int	begin_line;
	int	begin_typedef, end_typedef;
	int	end_of_typedef;
	int	parlevel;
	char	*s, *t;
	char	*begin_addr, *end_addr;
	char	name[40];
	int	i, j, k;
	dcli_sStructElement *element_p, *e_p, *e_ptr, *element_ptr;
	dcli_sStructElement *element_list;
	dcli_sStructElement *struct_element;
	dcli_sStructElement *next_ptr;
	t_filectx	filectx;
	char	*s1, *s2;
	char	tmp[40];
	int	nr;
	int	count;
	int	elnumcount;
	int	elnum[5];
	char	includename[80];
	char	normfilename[120];
	char	fname[120];
	int	return_sts;
	t_define	*define_ptr;
	char	define_elem[2][READSTRUCT_DEFINESIZE];
	int	define_num;

	if ( caller == READSTRUCT_CALLER_ROOT)
	{
	  /* Not case sensitive */
	  cdh_ToUpper( struct_name, struct_name);

	  /* Create an universal context */
	  ctx = calloc( 1, sizeof(*ctx));
	  strcpy( ctx->root_filename, filename);
	}
	  
/*	printf( "Processing file %s\n", filename);*/

#if defined (OS_ELN)
	/* Remove any logical device */
	if ( strstr( filename, "::") == 0)
	{
	  if ( (s = strchr( filename, ':')) != 0)
	  {
	    s++;
	    strcpy( fname, s);
	  }
	}
#else
        /* If no path is given, set pwrp_inc a default */
        if ( strchr( filename, ':') == 0 &&
	     strchr( filename, '<') == 0 &&
	     strchr( filename, '[') == 0 &&
	     strchr( filename, '/') == 0)
	{
	  strcpy( fname, "pwrp_inc:");
	  strcat( fname, filename);
	  dcli_translate_filename( fname, fname);
	}
	else
	  dcli_translate_filename( fname, filename);
#endif
	dcli_get_defaultfilename( fname, normfilename, ".h");
	file = fopen( normfilename, "r");
	if ( file == NULL)
	{
	  if ( caller != READSTRUCT_CALLER_INCLUDE)
	  {
	    return_sts = DCLI__NOFILE;
	    goto readstruct_error_return;
	  }
	  else
	    /* All includefiles might not be present in the runtime environment */
	    return DCLI__STRUCTNOTFOUND;
	}


	filectx = calloc( 1, sizeof(*filectx));
	filectx->file = file;
	strcpy( filectx->filename, normfilename);

	filectx->lines = 0;
	size = sizeof(line);
	typedef_line = 0;
	struct_found = 0;
	while( 1)
	{

	  if ( dcli_read_line( line, size, file) == 0)
	  {
	    free_filectx( filectx);
	    fclose( file);
	    return DCLI__EOF;
	  }
	  filectx->lines++;

	  /* If we are looking for undefined typedefs, open the included files also */
	  if ( caller == READSTRUCT_CALLER_UNDEFSEARCH &&
	       (s = strstr( line, "#include")) != 0)
	  {
	    if ( (s = strchr(line, '"')) != 0)
	    {
	      s++;
	      strcpy( includename, s);
	      if ( (s = strchr( includename, '"')) != 0)
	      {
	        *s = 0;
	        sts = find_struct( ctx, includename, struct_name, e_list,
			 READSTRUCT_CALLER_INCLUDE);
	        if ( ODD(sts))
	        {
	          free_filectx( filectx);
	          fclose( file);
	          return sts;
	        }
	      }
	    }
	  }
	  

	  if ( (s = strstr( line, "typedef")) != 0)
	  {
	    /* Store the line nr */
	    typedef_line = filectx->lines;
	    parlevel = 0;
	    end_of_typedef = 0;
	    name_found = 0;
	    begin_line = filectx->lines;
	    begin_typedef = filectx->lines;
	    begin_addr = s + strlen("typedef");
	  }
	  if ( typedef_line)
	  {
	    /* Search for end of typedef */
	    for ( s = line; *s != 0; s++)
	    {
	      if ( *s == '{')
	        parlevel++;
	      else if ( *s == '}')
	      {
	        parlevel--;
	        if ( parlevel == 0)
	        {
	          begin_line = filectx->lines;
	          begin_addr = s;
	        }
	      }
	      else if ( *s == ';' && parlevel == 0)
	      {
	        end_of_typedef = 1;
	        end_addr = s;
	      }

	      if ( end_of_typedef)
	      {
	        if ( begin_line != filectx->lines)
	        {
	          strcpy( ctx->error_file, filectx->filename);
	          ctx->error_line = filectx->lines;
	          free_filectx( filectx);
	          fclose( file);
	          return_sts = DCLI__ENDSTRUCTERR;
	          goto readstruct_error_return;
	        }

	        /* The name of the typdef should be found between
	           begin_addr and end_addr */
	        *end_addr = 0;
	        begin_addr++;
	        start_found = 0;
	        t = name;
	        for ( s = begin_addr; *s; s++)	        
	        {
	          if ( *s == ' ' || *s == 9)
	          {
	            if ( !start_found)
	              continue;
	            else
	              break;
	          }
	          else
	          {
	            name_found = 1;
	            *t = *s;
	            t++;
	          }
	        }
	        *t = 0;
	      }
	      if ( name_found)
	        break;
	    }
	    if ( name_found)
	    {
	      if ( caller == READSTRUCT_CALLER_ROOT)
	        /* Not case sensitive */
	        cdh_ToUpper( tmp, name);
	      else
	        strcpy( tmp,name);
	      if ( strcmp( struct_name, tmp) == 0)
	      {
	        struct_found = 1;
	        end_typedef = filectx->lines;
	      }
	      else
	        /* Look for next struct */
	        typedef_line = 0;
	    }
	  }
	  if ( struct_found)
	    break;
	}

	fclose( file);

	if ( !struct_found)
	{
	  free_filectx( filectx);
	  return_sts = DCLI__STRUCTNOTFOUND;
	  goto readstruct_error_return;	  
	}

	file = fopen( normfilename, "r");
	if ( file == NULL)
	{
	  free_filectx( filectx);
	  return DCLI__NOFILE;
	}
	filectx->file = file;

	filectx->lines = 0;
	for ( i = 0; i < begin_typedef; i++)
	{
	  if ( dcli_read_line( line, size, file) == 0)
	  {
	    free_filectx( filectx);
	    fclose( file);
	    return DCLI__EOF;
	  }
	  filectx->lines++;

	  /* Store all defines */
	  if ((s = strstr( line, "#define")) != 0)
	  {
	    nr = dcli_parse( s+strlen("#define"), " 	", "",
		(char *) define_elem, sizeof( define_elem)/sizeof(define_elem[0]), 
		sizeof(define_elem[0]), 1);
	    if ( nr > 1)
	    {
	      define_elem[0][READSTRUCT_DEFINESIZE-1] = 0;
	      define_elem[1][READSTRUCT_DEFINESIZE-1] = 0;
	      nr = sscanf( define_elem[1], "%d", &define_num);
	      if ( nr == 1)
	      {
	        define_ptr = calloc( 1, sizeof( t_define));
	        strcpy( define_ptr->define, define_elem[0]);
	        define_ptr->value = define_num;
	        define_ptr->next = filectx->definelist;
	        filectx->definelist = define_ptr;
	      }
	    }
	  }
	}

	for ( filectx->lines = begin_typedef+1; filectx->lines < end_typedef;)
	{
	  if ( dcli_read_line( line, size, file) == 0)
	  {
	    free_filectx( filectx);
	    fclose( file);
	    return DCLI__EOF;
	  }
	  filectx->lines++;

	  /* Parse the line */
	  sts = add_element( ctx, filectx, line, &filectx->element_list);
	  if (EVEN(sts))
	  {
	    free_filectx( filectx);
	    fclose( file);
	    return_sts = sts;
	    goto readstruct_error_return;	  
	  }
	}

	fclose( file);


	/* Find the undefined */
	for ( e_ptr = filectx->element_list; e_ptr; e_ptr = e_ptr->next)
	{
	  if ( e_ptr->undefined)
	  {
	    element_list = 0;
	    sts = find_struct( ctx, filename, e_ptr->typestr, &element_list, 
			READSTRUCT_CALLER_UNDEFSEARCH);
	    if ( EVEN(sts)) 
	    {
	      return_sts = DCLI__TYPEUNDEF;
	      goto readstruct_error_return;
	    }
	    e_ptr->undefined = 0;
	    e_ptr->struct_begin = 1;
	
	    /* Add the name and insert the list */

	    s1 = e_ptr->name;
	    elnumcount = 0;
	    struct_element = element_list;
	    i = 0;
	    while ( (s2 = strchr( s1, '[')) != 0)
	    {
	      /* Extract number of elements */
	      *s2 = 0;
	      s2++;
	      if ( (s1 = strchr( s2, ']')) == 0)
	      {
	        strcpy( ctx->error_file, filectx->filename);
	        ctx->error_line = filectx->lines;
	        return_sts = DCLI__ELEMSYNTAX;
	        goto readstruct_error_return;
	      }
	      *s1 = 0;
	      s1++;
	      nr = sscanf( s2, "%d", &elnum[i]);
	      if ( nr != 1)
	      {
	        sts = find_define( filectx, s2, &elnum[i]);
	        if ( EVEN(sts))
	        {
	          strcpy( ctx->error_file, filectx->filename);
	          ctx->error_line = filectx->lines;
	          return_sts = DCLI__ELEMSYNTAX;
	          goto readstruct_error_return;
	        }
	      }
	      i++;
	      elnumcount++;
	    }
	    count = 0;
	    for ( element_p = struct_element; element_p; element_p = element_p->next)
	      count++;
	    if ( elnumcount == 0)
	    {
	      for ( element_p = struct_element; element_p; element_p = element_p->next)
	      {
	        strcpy( tmp, element_p->name);
	        strcpy( element_p->name, e_ptr->name);
	        strcat( element_p->name, ".");
	        strcat( element_p->name, tmp);
	      }
	    }
	    else if ( elnumcount == 1)
	    {
	      for ( i = 1; i < elnum[0]; i++)
	      {
	        element_p = struct_element;
	        for ( j = 0; j < count; j++)
		{
	          
	          element_ptr = calloc( 1, sizeof( *element_ptr));
	          memcpy( element_ptr, element_p, sizeof(*element_p));
	          for ( e_p = element_list; e_p->next; e_p = e_p->next) ;
	          e_p->next= element_ptr;
	          element_ptr->prev = e_p;
	          element_ptr->next = 0;

	          element_p = element_p->next;
	        }
	      }
	      element_p = struct_element;
	      for ( i = 0; i < elnum[0]; i++)
	      {
	        for ( j = 0; j < count; j++)
		{
	          strcpy( tmp, element_p->name);
	          sprintf( element_p->name, "%s[%d].%s", e_ptr->name, i, tmp);
	          element_p = element_p->next;
	        }
	      }
	    }
	    else if ( elnumcount == 2)
	    {
	      for ( i = 0; i < elnum[0]; i++)
	      {
	        for ( k = 0; k < elnum[1]; k++)
	        {
	          if ( i == 0 && k == 0)
	            continue;
	          element_p = struct_element;
	          for ( j = 0; j < count; j++)
		  {
	          
	            element_ptr = calloc( 1, sizeof( *element_ptr));
	            memcpy( element_ptr, element_p, sizeof(*element_p));
	            for ( e_p = element_list; e_p->next; e_p = e_p->next) ;
	            e_p->next= element_ptr;
	            element_ptr->prev = e_p;
	            element_ptr->next = 0;

	            element_p = element_p->next;
	          }
	        }
	      }

	      element_p = struct_element;
	      for ( i = 0; i < elnum[0]; i++)
	      {
	        for ( k = 0; k < elnum[1]; k++)
	        {
	          for ( j = 0; j < count; j++)
		  {
	            strcpy( tmp, element_p->name);
	            sprintf( element_p->name, "%s[%d][%d].%s", e_ptr->name, i, k,tmp);
	            element_p = element_p->next;
	          }
	        }
	      }
	    }
	    else
	    {
	      strcpy( ctx->error_file, filectx->filename);
	      ctx->error_line = filectx->lines;
	      return_sts = DCLI__ARRAYDIM;
	      goto readstruct_error_return;
	    }

	    next_ptr = e_ptr->next;
	    e_ptr->next = element_list;
	    element_list->prev = e_ptr;
	    for ( e_p = element_list; e_p->next; e_p = e_p->next) ;
	    e_p->next = next_ptr;
	    if ( next_ptr)
	      next_ptr->prev = e_p;

	  }
	}


	if ( e_list)
	  *e_list = filectx->element_list;
	free_filectx( filectx);

	if ( caller == READSTRUCT_CALLER_ROOT)
	  free( ctx);

	return DCLI__SUCCESS;

readstruct_error_return:
	if ( caller != READSTRUCT_CALLER_ROOT)
	  return return_sts;

	/* Send an error message */
	if ( return_sts == DCLI__NOFILE)
	  sprintf( dcli_message, "Unable to open file %s", normfilename);
	else if ( return_sts == DCLI__STRUCTNOTFOUND)
	  sprintf( dcli_message, "Struct not found, file: %s", normfilename);
	else if ( return_sts == DCLI__EOF)
	  sprintf( dcli_message, "Unexpected end of file, %s", normfilename);
	else if ( return_sts == DCLI__ENDSTRUCTERR)
	  sprintf( dcli_message, 
		   "Syntax error at end of struct, line: %d,  file: %s",
		   ctx->error_line, ctx->error_file);
	else if ( return_sts == DCLI__ELEMSYNTAX)
	  sprintf( dcli_message, "Syntax error in element, line: %d, file: %s",
			ctx->error_line, ctx->error_file);
	else if ( return_sts == DCLI__ARRAYDIM)
	  sprintf( dcli_message, 
		   "Error in array dimension, line: %d, file: %s",
		   ctx->error_line, ctx->error_file);
	else if ( return_sts == DCLI__TYPEUNDEF)
	  sprintf( dcli_message,
		   "Unknown type %s, line: %d, file: %s",
		   e_ptr->typestr, e_ptr->line_nr, e_ptr->filename);
	else
	  sprintf( dcli_message, "Unknown error message");
        dcli_message_set = 1;
        printf( "%s\n", dcli_message);
	free( ctx);
	return return_sts;
}

/*___Global functions_______________________________________________________*/

/*************************************************************************
*
* Name:		dcli_readstruct_find()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Find the struct in an include-file and return a description of
*	the elements of the struct. The descriptions is a linked list
*	witch should be freed by the dcli_readstruct_free function.
*
**************************************************************************/

int	dcli_readstruct_find( 	char *filename, 
				char *struct_name, 
				dcli_sStructElement **e_list)
{
	int	sts;

	*e_list = 0;
        dcli_message_set = 0;
	sts = find_struct( NULL, filename, struct_name, e_list, 
			   READSTRUCT_CALLER_ROOT);
        if ( sts == DCLI__EOF && !dcli_message_set) {
          sprintf( dcli_message, "Struct %s not found", struct_name);
          dcli_message_set = 1;
          return sts;
        }
        else if ( EVEN(sts) && *e_list == 0 && !dcli_message_set) {
          sprintf( dcli_message, "Struct %s not found", struct_name);
          dcli_message_set = 1;
          return DCLI__STRUCTNOTFOUND;
        }
        else if ( EVEN(sts))
          return sts;

	return DCLI__SUCCESS;
}

int dcli_readstruct_get_message( char **message)
{
  if ( !dcli_message_set)
    return 0;
  *message = dcli_message;
  return 1;
}

/*************************************************************************
*
* Name:		dcli_readstruct_free()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Free an elementlist.
*
**************************************************************************/

void	dcli_readstruct_free(	dcli_sStructElement *e_list)
{
	dcli_sStructElement *element_p;
	dcli_sStructElement *next_ptr;

	for ( element_p = e_list; element_p->next; element_p = next_ptr)
	{
	  next_ptr = element_p->next;
	  free( element_p);
        }
}

#if 0
main()
{
	char	filename[120] = "read_struct.h";
	char	struct_name[40] = "MyStruct";
	int	sts;
	dcli_sStructElement	*e_list, *element_p;

	sts = dcli_readstruct_find( filename, struct_name, &e_list);

	for ( element_p = e_list; element_p; element_p = element_p->next)
	{
	  printf( "%s	type: %d	size: %d	struct: %d	undef: %d\n", 
		element_p->name, element_p->type, element_p->size, element_p->struct_begin,
		element_p->undefined);
	}

}
#endif
