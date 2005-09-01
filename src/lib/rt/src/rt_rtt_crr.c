/* 
 * Proview   $Id: rt_rtt_crr.c,v 1.3 2005-09-01 14:57:56 claes Exp $
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

/* rt_rtt_crr.c 
   This module contains routines for displaying crossreferences in rtt. */

/*_Include files_________________________________________________________*/

#if defined(OS_ELN)
# include $vaxelnc
# include stdio
# include ctype
# include string
# include chfdef
# include stdlib
#else
# include <stdio.h>
# include <ctype.h>
# include <string.h>
# include <stdlib.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "rt_gdh.h"
#include "rt_rtt.h"
#include "rt_rtt_msg.h"
#include "rt_rtt_global.h"
#include "rt_rtt_functions.h"
#include "rt_gdh_msg.h"

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define CRR_BUFF_SIZE 10000

static int one = 1;

/*** Local funktions ***************************************************/

static int	rtt_get_signal_line(
			FILE	*file,
			char	*line,
			int	size,
			int	*spaces,
			char	*text,
			int	*lines);
static int	rtt_remove_spaces(
			char	*in,
			char	*out);
static char	*rtt_VolumeIdToStr( pwr_tVolumeId volumeid);


/*************************************************************************
*
* Name:		rtt_get_signal_line()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Reads a line i a proview signal or plcmodule list.
*	Returns the read line (line), number of spaces to the first charachter,
*	the number of lines read( empty lines are not returned), 
*	and the first word (text).
*	Returns RTT__EOF if end of file.
*
**************************************************************************/

static int	rtt_get_signal_line(
			FILE	*file,
			char	*line,
			int	size,
			int	*spaces,
			char	*text,
			int	*lines)
{
	char    *s;
	char	*f;

	*lines = 0;
	while( 1)
	{
	  if ( rtt_read_line( line, size, file) == 0)
	    return RTT__EOF;
	  (*lines)++;
	
	  /* Get number of spaces before text */
	  *spaces = 0;
	  for ( s = line; !((*s == 0) || ((*s != ' ') && (*s != 9))); s++)
	  {
	    (*spaces)++;
	    if ( *s == 9)
	      (*spaces) += 7;
	  }
	  strcpy( text, s);

	  /* Get end of text */
	  for ( f = text; !((*f == 0) || (*f == ' ') || (*f == 9)); f++);
	  *f = 0;

	  /* Check the text, if no text read next line */
	  if ( text[0] == 0) continue;
	  if ( text[0] == '-')
	  {
	    if ( *spaces > 30)
	    {
	      /* Next line is a header */
	      if ( rtt_read_line( line, size, file) == 0)
	        return RTT__EOF;
	      (*lines)++;
	    }
	    continue;
	  }
	  break;
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_remove_spaces()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Removes spaces and tabs at the begining and at the end of a string.
*
**************************************************************************/

static int	rtt_remove_spaces(
			char	*in,
			char	*out)
{
	char    *s;

	for ( s = in; !((*s == 0) || ((*s != ' ') && (*s != 9))); s++);

	strcpy( out, s);
        
        if ( strlen(s) != 0)
        {
	  for ( s += strlen(s) - 1; 
                !((s == in) || ((*s != ' ') && (*s != 9))); s--) ;
	  s++;
	  *s = 0;
        }

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_crossref_signal()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Prints the cross references to a signal.
*
**************************************************************************/

int	rtt_crossref_signal(
			unsigned long	ctx,
			pwr_tObjid	objid,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	pwr_tOName	hiername;
	int		sts;

	sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	if ( EVEN(sts)) 
	{
	  rtt_message('E', "Cross reference not defined");
	  return RTT__NOPICTURE;
	}

	sts = rtt_crr_signal( NULL, hiername);
	return sts;
}

/*************************************************************************
*
* Name:		rtt_crossref_signal()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Prints the cross references to a signal.
*
**************************************************************************/

int	rtt_crossref_channel(
			unsigned long	ctx,
			pwr_tObjid	objid,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	char 		hiername[120];
	int		sts;
	pwr_tObjid	signal_objid;

	sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	if ( EVEN(sts)) 
	{
	  rtt_message('E', "Cross reference not defined");
	  return RTT__NOPICTURE;
	}
	strcat( hiername, ".SigChanCon");
	sts = gdh_GetObjectInfo ( hiername, (pwr_tAddress) &signal_objid,
		sizeof(signal_objid));
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( signal_objid, hiername, sizeof(hiername), cdh_mNName);
	if ( EVEN(sts)) 
	{
	  rtt_message('E', "Cross reference not defined");
	  return RTT__NOPICTURE;
	}

	sts = rtt_crr_signal( NULL, hiername);
	return sts;
}

/*************************************************************************
*
* Name:		rtt_crr_signal()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Prints the cross references to a signal.
*
**************************************************************************/

int	rtt_crr_signal(
			char	*filename,
			char	*signalname)
{
	pwr_tFileName	default_filename;
	FILE		*file;
	char		line[200];
	int		hierarchy_spaces;
	char		hierarchy[80];
	int		object_spaces;
	char		object[80];
	pwr_tOName     	objname;
	char		show_objname[80];
	int		spaces;
	int		first;
	int		sts;	
	char		*s;
	int		wildcard;
	int		signalcount = 0;
	pwr_tFileName  	filestr;
	int		lines;
	char		*buff;
	pwr_tVolumeId	volid;
	pwr_tObjid	objid;
	int		buffcnt;

	rtt_toupper( signalname, signalname);

	/* Check if wildcard */
	s = strchr( signalname, '*');
	if ( s == 0)
	  wildcard = 0;
	else
	  wildcard = 1;

	/* Open file */
	if ( filename == NULL)
	{
	  /* Open file, first get the volume id */
	  if ( !wildcard)
	  {
	    sts = gdh_NameToObjid ( signalname, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	    volid = objid.vid;
	  }
	  else
	  {
	    sts = gdh_GetNodeObject( 0, &objid);
	    if ( EVEN(sts)) return sts;
	    volid = objid.vid;
	  }

	  sprintf( default_filename, "%srtt_crr_%s.dat", 
		rtt_pwr_dir("pwrp_load"), rtt_VolumeIdToStr( volid));
	  rtt_get_defaultfilename( default_filename, filestr, NULL);
	  file = fopen( filestr, "r");
	}
	else
	{
	  rtt_get_defaultfilename( filename, filestr, ".lis");
	  file = fopen( filestr, "r");
	}
	  
	if ( file == 0)
	{
	  rtt_message('E', "Unable to open file");
	  return RTT__NOPICTURE;
	}
	
	buff = calloc( 1, CRR_BUFF_SIZE);
	if ( !buff)
	{
	  rtt_message('E',"Unable to allocate memory");
	  fclose(file);
	  return RTT__NOPICTURE;
	}

	/* First line is a header, skip it */
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;

	/* Get the hierarchy */
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&spaces, object, &lines);
	if ( EVEN(sts)) goto finish;
	hierarchy_spaces = spaces;


	first = 1;
	while ( 1)
	{
	  while ( spaces != hierarchy_spaces)
	  {
	    sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	    if ( EVEN(sts)) goto finish;
	  }
	  strcpy( hierarchy, object);

	  /* Next line is an object */
	  sts = rtt_get_signal_line( file, line, sizeof( line), 
				&spaces, object, &lines);
	  if ( EVEN(sts)) goto finish;
	  if ( first)
	  {
	    object_spaces = spaces;
	    first = 0;
	  }

	  while ( spaces == object_spaces)
	  {
	    /* Put object and hierarchy together and check if this is 
			the object */
	    strcpy( objname, hierarchy);
	    strcat( objname, "-");
	    strcat( objname, object);
	    strcpy( show_objname, objname);
	    rtt_toupper( objname, objname);

	    sts = rtt_wildcard( signalname, objname);
	    if ( !sts )
	    {
	      /* Hit, print this object */
	      if ( signalcount == 0)
	      {
	        rtt_clear_screen();
	        buffcnt = sprintf( buff, "Crossreferens list   %s\n\n", show_objname);
	      }
	      signalcount++;

	      sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	      if ( EVEN(sts)) goto finish;
	      while( spaces > object_spaces)
	      {
	        rtt_remove_spaces( line, line);

	        if ( buffcnt > CRR_BUFF_SIZE - 100)
	        {
	          buffcnt += sprintf( buff+buffcnt, 
	            "RTT-E-QUOTAEXC, Crossref quota exceeded");
	          goto finish;
	        }
	        if ( line[0] == '#')
	          buffcnt += sprintf( buff+buffcnt, " %s\n", line);
	        else
	          buffcnt += sprintf( buff+buffcnt, "   %s\n", line);

	        sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	        if ( EVEN(sts)) goto finish;
	      }
	      if ( !wildcard)
	        goto finish;
	    }
	    else
	    {
	      sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	      if ( EVEN(sts)) goto finish;
	      while( spaces > object_spaces)
	      {
	        sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	        if ( EVEN(sts)) goto finish;
	      }
	    }
	  }
	}

finish:
	fclose( file);

	if ( signalcount > 0)
	{
	  sts = rtt_view( 0, 0, buff, "Crossreference list", 
			RTT_VIEWTYPE_BUF);
	  return sts;
	}
	else
	{
	  rtt_message('E', "Object not found");	
	  return RTT__NOPICTURE;
	} 
	free( buff);
	return RTT__SUCCESS;

}

/*************************************************************************
*
* Name:		rtt_crr_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Prints the cross references to a object that is not a signal.
*
**************************************************************************/

int	rtt_crr_object(
			char	*filename,
			char	*objectname)
{
	pwr_tFileName	default_filename;
	FILE		*file;
	char		line[200];
	int		hierarchy_spaces;
	char		hierarchy[80];
	int		object_spaces;
	char		object[80];
	pwr_tOName     	objname;
	char		show_objname[80];
	int		spaces;
	int		first;
	int		sts;	
	char		*s;
	int		wildcard;
	int		signalcount = 0;
	pwr_tFileName  	filestr;
	int		lines;
	char		*buff;
	pwr_tVolumeId	volid;
	pwr_tObjid	objid;
	int		buffcnt;

	rtt_toupper( objectname, objectname);

	/* Check if wildcard */
	s = strchr( objectname, '*');
	if ( s == 0)
	  wildcard = 0;
	else
	  wildcard = 1;

	/* Open file */
	if ( filename == NULL)
	{
	  /* Open file, first get the volume id */
	  if ( !wildcard)
	  {
	    sts = gdh_NameToObjid ( objectname, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Object not found");
	      return RTT__NOPICTURE;
	    }
	    volid = objid.vid;
	  }
	  else
	  {
	    sts = gdh_GetNodeObject( 0, &objid);
	    if ( EVEN(sts)) return sts;
	    volid = objid.vid;
	  }

	  sprintf( default_filename, "%srtt_crro_%s.dat", 
		rtt_pwr_dir("pwrp_load"), rtt_VolumeIdToStr( volid));
	  rtt_get_defaultfilename( default_filename, filestr, NULL);
	  file = fopen( filestr, "r");
	}
	else
	{
	  rtt_get_defaultfilename( filename, filestr, ".lis");
	  file = fopen( filestr, "r");
	}
	  
	if ( file == 0)
	{
	  rtt_message('E', "Unable to open file");
	  return RTT__NOPICTURE;
	}
	
	buff = calloc( 1, CRR_BUFF_SIZE);
	if ( !buff)
	{
	  rtt_message('E',"Unable to allocate memory");
	  fclose(file);
	  return RTT__NOPICTURE;
	}

	/* First line is a header, skip it */
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;

	/* Get the hierarchy */
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&spaces, object, &lines);
	if ( EVEN(sts)) goto finish;
	object_spaces = spaces;


	first = 1;
	while ( 1)
	{
	  while ( spaces != object_spaces)
	  {
	    sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	    if ( EVEN(sts)) goto finish;
	  }
	  strcpy( objname, object);

	  strcpy( show_objname, objname);
	  rtt_toupper( objname, objname);

	  sts = rtt_wildcard( objectname, objname);
	  if ( !sts )
	  {
	    /* Hit, print this object */
	    if ( signalcount == 0)
	    {
	      rtt_clear_screen();
	        buffcnt = sprintf( buff, "Crossreferens list   %s\n\n", show_objname);
	    }
	    signalcount++;

	    sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	    if ( EVEN(sts)) goto finish;
	    while( spaces > object_spaces)
	    {
	      rtt_remove_spaces( line, line);

	      if ( buffcnt > CRR_BUFF_SIZE - 100)
	      {
	        buffcnt += sprintf( buff+buffcnt, 
	            "RTT-E-QUOTAEXC, Crossref quota exceeded");
	        goto finish;
	      }

	      if ( line[0] == '#' || line[0] == '&')
	        buffcnt += sprintf( buff+buffcnt, " %s\n", line);
	      else
	        buffcnt += sprintf( buff+buffcnt, "   %s\n", line);

	      sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	      if ( EVEN(sts)) goto finish;
	    }
	    if ( !wildcard)
	      goto finish;
	  }
	  else
	  {
	    sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	    if ( EVEN(sts)) goto finish;
	  }
	}

finish:
	fclose( file);

	if ( signalcount > 0)
	{
	  sts = rtt_view( 0, 0, buff, "Crossreference list", 
			RTT_VIEWTYPE_BUF);
	  return sts;
	}
	else
	{
	  rtt_message('E', "Object not found");	
	  return RTT__NOPICTURE;
	} 
	free( buff);
	return RTT__SUCCESS;

}

/*************************************************************************
*
* Name:		rtt_crr_code()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Prints the cross references to a fuction or string
*		in arithm objects.
*
**************************************************************************/

int	rtt_crr_code(
			char	*filename,
			char	*str,
			int	brief,
			int	func,
			int	case_sensitive)
{
	pwr_tFileName	default_filename;
	FILE		*file;
	char		line[200];
	char		tst_line[200];
	int		hierarchy_spaces;
	char		hierarchy[80];
	int		object_spaces;
	char		object[80];
	pwr_tOName	objname;
	int		spaces;
	int		first;
	int		sts;	
	char		*s;
	int		signalcount = 0;
	char		filestr[80];
	int		lines;
	char		*buff;
	pwr_tVolumeId	volid;
	pwr_tObjid	objid;
	int		buffcnt;
	int		len;
	int		i;
	int		objname_written;
	int		hit;
	char		*tst_char;

	/* Open file */
	if ( filename == NULL)
	{
	  sts = gdh_GetNodeObject( 0, &objid);
	  if ( EVEN(sts)) return sts;
	  volid = objid.vid;

	  sprintf( default_filename, "%srtt_crrc_%s.dat", 
		rtt_pwr_dir("pwrp_load"), rtt_VolumeIdToStr( volid));
	  rtt_get_defaultfilename( default_filename, filestr, NULL);
	  file = fopen( filestr, "r");
	}
	else
	{
	  rtt_get_defaultfilename( filename, filestr, ".lis");
	  file = fopen( filestr, "r");
	}
	  
	if ( file == 0)
	{
	  rtt_message('E', "Unable to open file");
	  return RTT__NOPICTURE;
	}
	
	/* Case sensitive if any lowercase */
	if ( !case_sensitive)
	  for ( s = str; *s != 0; s++)
	  {
	    if ( !isupper(*s))
	      case_sensitive = 1;
	  }

	buff = calloc( 1, CRR_BUFF_SIZE);
	if ( !buff)
	{
	  rtt_message('E',"Unable to allocate memory");
	  fclose(file);
	  return RTT__NOPICTURE;
	}

	/* First line is a header, skip it */
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&hierarchy_spaces, hierarchy, &lines);
	if ( EVEN(sts)) goto finish;

	/* Get the hierarchy */
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&spaces, object, &lines);
	if ( EVEN(sts)) goto finish;
	object_spaces = spaces;


	first = 1;
	while ( 1)
	{
	  while ( strncmp( line, "_Obj_ ", 6) != 0)
	  {
	    sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	    if ( EVEN(sts)) goto finish;
	  }
	  strcpy( objname, &line[6]);
	  for ( s = objname; !(*s == 32 || *s == 9 || *s == 0); s++);
	  *s = 0;

	  sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	  objname_written = 0;
	  while ( strncmp( line, "_Obj_ ", 6) != 0)
	  {
	    if ( !case_sensitive)
	      rtt_toupper( tst_line, line);
	    else
	      strcpy( tst_line, line);

	    hit = 0;
	    if ( !func)
	    {
	      if ( strstr( tst_line, str) != 0)
	        hit = 1;
	    }
	    else
	    {
	      if ( (s = strstr( tst_line, str)) != 0)
	      {
	        hit = 1;
	        /* Check char after */
	        tst_char = s + strlen(str);
	        if ( isalpha( *tst_char) || isdigit( *tst_char) ||
		     *tst_char == '_')
	          hit = 0;
	        /* Check char before */
	        if ( s != tst_line)
	        {
	          tst_char = s - 1;
	          if ( isalpha( *tst_char) || isdigit( *tst_char) ||
		       *tst_char == '_')
	            hit = 0;
	        }
	      }
	    }
	    if ( hit)
	    {
	      /* Hit, print this object */
	      if ( signalcount == 0)
	      {
	        rtt_clear_screen();
	        if ( func)
	          buffcnt = sprintf( buff, "Crossreferens list Function  \"%s\"\n\n", str);
	        else
	          buffcnt = sprintf( buff, "Crossreferens list String    \"%s\"\n\n", str);
	      }
	      signalcount++;

	      if ( buffcnt > CRR_BUFF_SIZE - 100)
	      {
	        buffcnt += sprintf( buff+buffcnt, 
	            "RTT-E-QUOTAEXC, Crossref quota exceeded");
	        goto finish;
	      }
	      if ( !objname_written)
	      {
	        len = sprintf( buff+buffcnt, " %s", objname);
	        objname_written = 1;
	      }
	      else
	        len = 0;
	      for ( i = len; i < 45; i++)
	      {
	        strcat( buff+buffcnt+len, " ");
	        len++;
	      }
	      buffcnt += len;

	      buffcnt += sprintf( buff+buffcnt, " \"%s\"\n", line);

	      if ( brief)
	      {
	        while ( strncmp( line, "_Obj_ ", 6) != 0)
	        {	      
	          sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	          if ( EVEN(sts)) goto finish;
	        }
	      }
	      else
	      {
	        sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	        if ( EVEN(sts)) goto finish;
	      }
	    }
	    else
	    {
	      sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	      if ( EVEN(sts)) goto finish;
	    }
	  }
	}

finish:
	fclose( file);

	if ( signalcount > 0)
	{
	  sts = rtt_view( 0, 0, buff, "Crossreference list", 
			RTT_VIEWTYPE_BUF);
	  return sts;
	}
	else
	{
	  if ( func)
	    rtt_message('E', "String not found");	
	  else
	    rtt_message('E', "Function not found");	
	  return RTT__NOPICTURE;
	} 
	free( buff);
	return RTT__SUCCESS;

}


/*************************************************************************
*
* Name:		rtt_show_signals()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*wildname	I	wildcard name.
* char		*name		I	object name.
*
* Description:
*
**************************************************************************/

int	rtt_show_signals(
			menu_ctx	parent_ctx,
			char		*filename,
			char		*windowname,
			int		debug)
{
	pwr_tFileName	default_filename;
	FILE		*file;
	char		line[200];
	int		window_spaces;
	char		window[80];
	char		object[80];
	pwr_tOName     	objname;
	int		spaces = 30;
	int		sts;	
	char		*s;
	int		wildcard;
	char		title[120];
	char		classname[32];
	int		j;
	int		index = 0;
	pwr_tObjid	objid;
	pwr_tObjid	childobjid;
	rtt_t_menu	*menulist = 0;
	char		filestr[80];
	int		lines;
	int		window_found;
	pwr_tVolumeId	volid;

	window_found = 0;
	rtt_toupper( windowname, windowname);

	/* Check if wildcard */
	s = strchr( windowname, '*');
	if ( s == 0)
	  wildcard = 0;
	else
	  wildcard = 1;

	/* Open file */
	if ( filename == NULL)
	{
	  /* Open file, first get the volume id */
	  if ( !wildcard)
	  {
	    sts = gdh_NameToObjid ( windowname, &objid);
	    if ( EVEN(sts))
	    {
	      rtt_message('E', "Window not found");
	      return RTT__NOPICTURE;
	    }
	    volid = objid.vid;
	  }
	  else
	  {
	    sts = gdh_GetNodeObject( 0, &objid);
	    if ( EVEN(sts)) return sts;
	    volid = objid.vid;
	  }

	  sprintf( default_filename, "%srtt_plc_%s.dat", 
		rtt_pwr_dir("pwrp_load"), rtt_VolumeIdToStr( volid));
	  rtt_get_defaultfilename( default_filename, filestr, NULL);
	  file = fopen( filestr, "r");
	}
	else
	{
	  rtt_get_defaultfilename( filename, filestr, ".lis");
	  file = fopen( filestr, "r");
	}
	  
	if ( file == 0)
	{
	  rtt_message('E', "Unable to open file");
	  return RTT__NOPICTURE;
	}
	
	/* First line is a header, skip it */
	sts = rtt_get_signal_line( file, line, sizeof( line), 
				&window_spaces, object, &lines);
	if ( EVEN(sts)) return RTT__NOFILE;

	/* Get the hierarchy */
	while ( spaces >= 30)
	{
	  /* This is still the header */
	  sts = rtt_get_signal_line( file, line, sizeof( line), 
				&spaces, object, &lines);
	  if ( EVEN(sts)) return RTT__NOFILE;
	}
	window_spaces = spaces;

	while ( 1)
	{
	  while ( spaces != window_spaces)
	  {
	    sts = rtt_get_signal_line( file, line, sizeof( line), 
				&spaces, object, &lines);
	    if ( EVEN(sts)) goto finish;
	  }
	  strcpy( window, object);
	  rtt_toupper( window, window);

	  sts = rtt_wildcard( windowname, window);
	  if ( !sts )
	  {
	    window_found = 1;
	    /* Hit, print the window */
	    /* Get objid for the object */
	    sts = gdh_NameToObjid ( window, &objid);
	    if ( EVEN(sts))
	    {
	      sts = rtt_get_signal_line( file, line, sizeof( line), 
				&spaces, object, &lines);
	      if ( EVEN(sts)) goto finish;
	      continue;
	    }

	    /* Get the object name */
	    sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mNName);
	    if ( EVEN(sts)) return sts;

	    /* Get class name */	
	    sts = rtt_objidtoclassname( objid, classname);
	    if (EVEN(sts)) return sts;

	    /* Add class name to objname in title */
	    strcpy( title, objname);
	    for ( j = strlen( title); j < 45; j++)
	      strcat( title, " ");
	    strcat( title, " ");
	    strcat( title, classname);

	    /* Mark if the object has children */
	    sts = gdh_GetChild( objid, &childobjid);
	    if ( ODD(sts))
	      strcat( title, " *");

	    if ( !debug)
	    {
	      sts = rtt_menu_list_add( &menulist, index, 0, title,
			&rtt_hierarchy_child,
			&rtt_object_parameters, 
			&rtt_crossref_signal,
			objid,0,0,0,0);
	      if ( EVEN(sts)) return sts;
	      index++;
	    }

	    /* Find the signal list */
	    sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	    if ( EVEN(sts)) goto finish;
	    while( spaces > window_spaces)
	    {
	      if ( strcmp( object, "Signals") == 0)
	      {
	        /* This is the signals */
	        sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	        if ( EVEN(sts)) goto finish;
		lines = 1;
	        while( (spaces > window_spaces) && (lines == 1))
	        {
	          /* Insert the object in menulist */

	          /* Get objid for the object */
	          sts = gdh_NameToObjid ( object, &objid);
	          if ( EVEN(sts))
	          {
	            /* End of this list, read next line and continue */
	            sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	            if ( EVEN(sts)) goto finish;
	            continue;
	          }
	          /* Get the object name */
	          sts = gdh_ObjidToName ( objid, objname, sizeof(objname), cdh_mNName);
	          if ( EVEN(sts)) return sts;

	          /* Get class name */	
	          sts = rtt_objidtoclassname( objid, classname);
	          if (EVEN(sts)) return sts;

	          /* Add class name to objname in title */
	          strcpy( title, "    ");
	          strcat( title, objname);
	          for ( j = strlen( title); j < 45; j++)
	            strcat( title, " ");
	          strcat( title, " ");
	          strcat( title, classname);

	          if ( !debug)
	          {
	            sts = rtt_menu_list_add( &menulist, index, 0, title,
			&rtt_hierarchy_child,
			&rtt_object_parameters, 
			&rtt_crossref_signal,
			objid,0,0,0,0);
		    if ( EVEN(sts)) return sts;
	            index++;
	          }
	          else
	          {
	            sts = rtt_debug_object_add( objid, 
			(rtt_t_menu_upd **) &menulist, &index, &one, 0, 0);
		    if ( EVEN(sts)) return sts;
	          }

	          sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	          if ( EVEN(sts)) goto finish;
	          /* Check if end of signals !! */

	        }
	      }
	      else
	      {
	        sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	        if ( EVEN(sts)) goto finish;
	      }
	    }
	    if ( !wildcard) goto finish;
	  }
	  else
	  {
	    if ( window_found)
	      goto finish;

	    sts = rtt_get_signal_line( file, line, sizeof( line),
				&spaces, object, &lines);
	    if ( EVEN(sts)) goto finish;
	  }
	}

finish:
	fclose( file);

	if ( menulist != 0)
	{
	  strcpy( title, "LIST OF SIGNALS");
/*	  sts = rtt_menu_bubblesort( menulist);*/
	  if ( !debug)
	    sts = rtt_menu_new( parent_ctx, pwr_cNObjid, &menulist, title, 0, 
		RTT_MENUTYPE_DYN);
	  else
	    sts = rtt_menu_upd_new( parent_ctx, pwr_cNObjid, 
		(rtt_t_menu_upd **) &menulist, title, 0, RTT_MENUTYPE_DYN);
	  if ( sts == RTT__FASTBACK) return sts;
	  else if ( sts == RTT__BACKTOCOLLECT) return sts;
	  else if (EVEN(sts)) return sts;
	}
	else
	{
	  rtt_message('E', "No windows found");
	  return RTT__NOPICTURE;
	}

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_VolumeIdToStr()
*
* Type		* char
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Converts an VolumeId to a string.
*	The returned string is static and must be used befor next call
*	of the function.
*
**************************************************************************/

static char	*rtt_VolumeIdToStr( pwr_tVolumeId volumeid)
{
	static char	str[80];
	unsigned char	volid[4];

	memcpy( &volid, &volumeid, sizeof(volid));
#if defined OS_ELN
	sprintf( str, "%03.3u_%03.3u_%03.3u_%03.3u",
		volid[3], volid[2], volid[1], volid[0]);
#else
	sprintf( str, "%3.3u_%3.3u_%3.3u_%3.3u",
		volid[3], volid[2], volid[1], volid[0]);
#endif
	return str;
}

/*
main()
{
	char	*filename = '\0';
	char	signalname[80] = "SH-FSMK-markon_f_dv";
	char	windowname[80] = "SH-FSMK-GEM-A_Larm-W";
	int 	sts;

	sts = rtt_crr( filename, signalname);
	sts = rtt_show_signals( filename, windowname);
}
*/


