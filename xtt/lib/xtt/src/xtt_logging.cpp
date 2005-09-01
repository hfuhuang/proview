/* 
 * Proview   $Id: xtt_logging.cpp,v 1.2 2005-09-01 14:57:48 claes Exp $
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

/* xtt_logging.c 

   This module contains routines for handling of logging in xtt. */

/*_Include files_________________________________________________________*/

#include "flow_std.h"

#if defined(OS_VMS)
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <descrip.h>
# include <chfdef.h>
# include <stdarg.h>
# include <processes.h>
# include <lib$routines.h>
# include <libdef.h>
# include <libdtdef.h>
# include <starlet.h>
#else
# include <stdarg.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <pthread.h>
#endif
#if defined OS_LINUX
# include <time.h>
#endif

extern "C" {
#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "co_dcli.h"
}

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget.h"
#include "rt_xtt.h"
#include "xtt_xnav.h"
#include "xtt_logging.h"
#include "rt_xnav_msg.h"

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

static void	*xtt_logproc( void *arg);



XttLogging::XttLogging() :
  xnav(0), index(0), active(0), intern(0), stop_logg(0), 
  logg_type(xtt_LoggType_Cont), logg_priority(0), condition_ptr(0),
  logg_time(200), logg_file(0), line_size(10000), parameter_count(0), 
  print_shortname(0), buffer_size(100), buffer_count(0), buffer_ptr(0)
{
  for ( int i = 0; i < RTT_LOGG_MAXPAR; i++) {
    parameterstr[i][0] = 0;
    shortname[i][0] = 0;
  }
  memset( parameter_type, 0, sizeof(parameter_type));
  memset( parameter_size, 0, sizeof(parameter_size));
  memset( old_value, 0, sizeof(old_value));

  strcpy( logg_filename, "rtt_logging.rtt_log");
}

void XttLogging::init( int logg_index, void *logg_xnav)
{
  index = logg_index;
  xnav = logg_xnav;
}

XttLogging::~XttLogging()
{
  if ( buffer_ptr)
    free ( buffer_ptr);
}

/*************************************************************************
*
* Name:		logging_set()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Modify parameter in the logging table.
*
**************************************************************************/

int XttLogging::logging_set(
			int		a_logg_time,
			char		*filename,
			char		*parameter,
			char		*condition,
			int		a_logg_type,
			int		insert,
			int		a_buffer_size,
			int		stop,
			int		priority,
			int		create,
			int		a_line_size,
			int		shortname)
{
	int		i, sts;
	int		found, par_index;
	char		buffer[8];
	pwr_sParInfo	parinfo;
	char		msg[80];
	int		type_error;
	
	if ( active )	
	{
	  message('E',"Unable to modify entry, entry is started");
	  return XNAV__HOLDCOMMAND;
	} 

	if ( a_buffer_size != 0 || buffer_ptr == 0)
	{
          if ( a_buffer_size != 0)
	    buffer_size = a_buffer_size;
	
	  /* Reallocate the buffer to store logging info in */
	  if ( buffer_ptr != 0)
	    free( buffer_ptr);
	  buffer_ptr = (char *) calloc( 1, buffer_size * 512);
	  if (buffer_ptr == 0)
	  {
	    message('E', "Buffer is to large");
	    /* set default buffer */
	    buffer_size = RTT_BUFFER_DEFSIZE;
	
	    /* Reallocate the buffer to store logging info in */
	    buffer_ptr = (char *) calloc( 1, buffer_size * 512);
	    if (buffer_ptr == 0)
	      exit( XNAV__NOMEMORY);
	  }
	}

	/* Insert in the entry */
	if ( filename != NULL)
	{
	  dcli_get_defaultfilename( filename, logg_filename, 
		".rtt_log");
	}
	if ( parameter != NULL)
	{
	  /* Get a free parameter index */
	  found = 0;
	  for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	  {
	    if ( parameterstr[i][0] == 0)
	    {
	      found = 1;
	      par_index = i;
	      break;
	    }
	  }
	  if ( !found) 
	  {
	    message('E', "Max number of parameters exceeded");
	    return XNAV__HOLDCOMMAND;
	  }

	  /* Check that parameter exists */
	  sts = gdh_GetObjectInfo ( parameter, &buffer, sizeof(buffer));
	  if (EVEN(sts))
	  {
	    message('E',"Parameter doesn't exist");
	    return XNAV__HOLDCOMMAND;
	  }

	  sts = get_parinfo( parameter, &parinfo);
	  if (EVEN(sts))
	  {
	    message('E',"Parameter doesn't exist");
	    return XNAV__HOLDCOMMAND;
	  }
	  strcpy ( parameterstr[ par_index], parameter); 
	  parameter_type[ par_index] = parinfo.Type; 	  
	  parameter_size[ par_index] = parinfo.Size/parinfo.Elements;
	}

	if ( condition != NULL)
	{
	  /* Check that parameter exists */
	  sts = gdh_GetObjectInfo ( condition, &buffer, sizeof(buffer)); 
	  if (EVEN(sts))
	  {
	    message('E',"Condition doesn't exist");
	    return XNAV__HOLDCOMMAND;
	  }
	  strcpy ( conditionstr, condition);
	}
	if ( a_logg_time != 0)
	  logg_time = a_logg_time;

	if ( a_logg_type != 0)
	  logg_type = a_logg_type;

	if ( priority < -1 || priority > 32)
	{
	    message('E',"Priority out of range");
	    return XNAV__HOLDCOMMAND;
	}
	else if ( priority != -1)
	  logg_priority = priority;

	if ( a_line_size != 0)
	  line_size = a_line_size;

	if ( shortname != -1)
	  print_shortname = shortname;

	if ( stop != -1)
	  intern = stop;

	if ( insert)
	{
          pwr_sAttrRef *alist, *ap;
          int *is_attrp, *is_attr;
          pwr_tTypeId attr_type;
          unsigned int attr_size, attr_offset, attr_dimension;
          char name[120];

	  /* Insert from collection picture */
          sts = ((XNav *)xnav)->get_all_collect_objects( &alist, &is_attr); 
          if ( EVEN(sts)) {
            message('E', "Nothing to insert");
	    return XNAV__HOLDCOMMAND;
          }

	  /* Clear all parameters */
	  memset( &(parameterstr), 0, sizeof( parameterstr));

	  i = 0;
	  type_error = 0;

          ap = alist;
          is_attrp = is_attr;
          while( cdh_ObjidIsNotNull( ap->Objid))
          {
            if ( *is_attrp)
            {
	      if ( i >= RTT_LOGG_MAXPAR )
	      {
	        message('E', "Max number of parameters exceeded");
	        break;
	      }

              sts = gdh_AttrrefToName( ap, name, sizeof(name), 
				       cdh_mNName);
              if ( EVEN(sts)) return sts;

              sts = gdh_GetAttributeCharacteristics( name, &attr_type, 
                       &attr_size, &attr_offset, &attr_dimension);
              if ( EVEN(sts)) {
                sts = gdh_AttrrefToName( ap, name, sizeof(name), 
				       cdh_mName_volumeStrict);
                if ( EVEN(sts)) return sts;

                sts = gdh_GetAttributeCharacteristics( name, &attr_type, 
                       &attr_size, &attr_offset, &attr_dimension);
                if ( EVEN(sts)) return sts;
              }

	      strcpy(  parameterstr[i], name);
	      parameter_type[i] = attr_type; 
	      switch( parameter_type[i])
	      {
	        case pwr_eType_Float32:
	        case pwr_eType_Float64:
	        case pwr_eType_UInt8:
	        case pwr_eType_Boolean:
	        case pwr_eType_Char:
	        case pwr_eType_Int8:
	        case pwr_eType_Int16:
	        case pwr_eType_UInt16:
	        case pwr_eType_Int32:
	        case pwr_eType_UInt32:
	        case pwr_eType_Objid:
	        case pwr_eType_AttrRef:
	        case pwr_eType_Time:
	          break;
	        default:
	          sprintf( msg, "Error in parameter nr %d: type is not supported", i+1);
	          message('E', msg);
	          type_error = 1;
	      }
	      i++;
            }
            ap++;
            is_attrp++;
          }
          free( alist);
          free( is_attr);

	  if ( i < RTT_LOGG_MAXPAR && !type_error)
	    message('I', "Parameters copied");
	}

	/* Count the parameters */
	parameter_count = 0;
	for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	{
	  if ( parameterstr[i][0] != 0)
	    parameter_count++;
	}

	return XNAV__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_logging_show()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Show the entry.
*
**************************************************************************/

int XttLogging::show()
{
	char		*buff;
	int		buff_cnt;

	buff = (char *) calloc( 1, 120000);
	if ( buff == 0)
	  return XNAV__NOMEMORY;
	buff_cnt = 0;
	show_entry( buff, &buff_cnt);
        printf( "%s", buff);	
	free( buff);

	return 1;
}

int XttLogging::show_entry(
			char		*buff,
			int		*buff_cnt)
{
	int		i;
	int		par_cnt;

	(*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Entry:     %d", index + 1);

	if ( active)
	  (*buff_cnt) += sprintf( buff + *buff_cnt, "       ACTIVE\n");
	else
	  (*buff_cnt) += sprintf( buff + *buff_cnt, "       NOT ACTIVE\n");

	if ( logg_type == xtt_LoggType_Mod)                      
	  (*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Type:      Event\n");
	else if ( logg_type == xtt_LoggType_Cont)
	  (*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Type:      Cont\n");

	(*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Time:      %d ms\n", logg_time);
	(*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Buffer:    %d pages\n", buffer_size);
	(*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Priority:  %d\n", logg_priority);
	(*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Line size: %d\n", line_size);
	if ( intern) 
	  (*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Stop when buffer is full\n");
	(*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Filename:  %s\n", logg_filename);
	(*buff_cnt) += sprintf( buff + *buff_cnt, 
		"   Number of parameters: %d\n", parameter_count);
	par_cnt = 0;
	for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	{
	  if ( parameterstr[i][0] != 0)
	  {
	    par_cnt++;
	    (*buff_cnt) += sprintf( buff + *buff_cnt, "Parameter%d :	%s\n", 
		par_cnt, parameterstr[i]);
	  }
	}
	(*buff_cnt) += sprintf( buff + *buff_cnt, "Condition:	%s\n", conditionstr);
	(*buff_cnt) += sprintf( buff + *buff_cnt, "\n");

	return XNAV__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_logging_store_entry()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Store one entry.
*
**************************************************************************/

int XttLogging::store(
			char		*filename)
{
	int		i;
	char		filename_str[120];
	FILE		*outfile;
	char		msg[120];
	int		found_parameter;

	found_parameter = 0;
	for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	{
	  if ( parameterstr[i][0] != 0)
	    found_parameter++;
	}
	if ( !found_parameter )
	{
	  message('E', "No parameters found in Logging entry");
	  return XNAV__SUCCESS;
	}

	dcli_get_defaultfilename( filename, filename_str, ".rtt_com");

	outfile = fopen( filename_str, "w");
	if ( outfile == 0)
	{
	  message('E', "Unable to open file");
	  return XNAV__HOLDCOMMAND;
	}

	fprintf( outfile, "logging set/create/entry=current/file=\"%s\"\n",
		logg_filename);
	if ( logg_time != 0)
	  fprintf( outfile, "logging set/entry=current/time=%d\n",
		logg_time);
	fprintf( outfile, "logging set/entry=current/buffer=%d\n", 
		buffer_size);
	fprintf( outfile, "logging set/entry=current/line_size=%d\n", 
		line_size);
	fprintf( outfile, "logging set/entry=current/priority=%d\n", 
		logg_priority);
	if ( print_shortname)
	  fprintf( outfile, "logging set/entry=current/shortname\n");
	else
	  fprintf( outfile, "logging set/entry=current/noshortname\n");

	if ( logg_type == xtt_LoggType_Mod)
	  fprintf( outfile, "logging set/entry=current/type=event\n");
	else if ( logg_type == xtt_LoggType_Cont)
	  fprintf( outfile, "logging set/entry=current/type=cont\n");

	for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	{
	  if ( parameterstr[i][0] != 0)
	    fprintf( outfile, "logging set/entry=current/parameter=\"%s\"\n",
		parameterstr[i]);
	}
	if ( conditionstr[0] != 0)
	  fprintf( outfile, "logging set/entry=current/condition=\"%s\"\n", 
		conditionstr);

	if ( intern )
	  fprintf( outfile, "logging set/entry=current/stop\n");
	else
	  fprintf( outfile, "logging set/entry=current/nostop\n");

	dcli_fgetname( outfile, filename_str, filename_str);
	fclose( outfile);

	sprintf( msg, "%s created", filename_str);
	message('I', msg);
	return XNAV__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_logging_start()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Start the entry.
*
**************************************************************************/

int XttLogging::start()
{
	int		sts;
	int		i, found;
	char		msg[256];
	
	if ( active )	
	{
	  message('E',"Entry is already started");
	  return XNAV__HOLDCOMMAND;
	} 

	/* Get the parameters */
	found = 0;
	for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	{
	  if ( parameterstr[i][0] != 0)
	  {
	    found = 1;

	    sts = gdh_RefObjectInfo(
		parameterstr[i],	
		(pwr_tAddress *) &parameter_ptr[i],
		&(parameter_subid[i]),
		parameter_size[i]);
	    if ( EVEN(sts)) 
	    {
	      message('E', "Parameter not found");
	      return XNAV__HOLDCOMMAND;
	    }
	  }
	}
	if ( !found)
	{
	  message('E', "Parameter is missing");
	  return XNAV__HOLDCOMMAND;
	}

	/* Get the condition */
	if ( conditionstr[0] != 0)
	{
	  sts = gdh_RefObjectInfo(
		conditionstr,	
		(pwr_tAddress *) &condition_ptr,
		&(condition_subid), 1);
	  if ( EVEN(sts)) 
	  {
	    message('E', "Condition parameter not found");
	    return XNAV__HOLDCOMMAND;
	  }
	}
	else
	{
	  condition_ptr = 0;
	}
	
	/* Open the file */
	if ( logg_filename[0] != 0)
	{
	  logg_file = fopen( logg_filename, "w");
	  if ( logg_file == 0)
	  {
	    message('E', "Unable to open file");
	    return XNAV__HOLDCOMMAND;
	  }
	}
	else
	{
	  message('E', "File is missing");	
	  return XNAV__HOLDCOMMAND;
	}

	/* Check time */
	if ( logg_time == 0)
	{
	  message('E', "Time is missing");
	  return XNAV__HOLDCOMMAND;
	}

	/* Clear buffer */
	buffer_count = 0;
	*(buffer_ptr) = 0;
	
	active = 1;
	stop_logg = 0;

	/* Create a subprocess */
#if defined(OS_VMS) || defined(OS_LINUX) || defined(OS_LYNX) && !defined(PWR_LYNX_30)
	sts = pthread_create (
		&thread,
		NULL,			 /* attr */
		xtt_logproc,		/* start_routine */
		(void *)this);			/* arg */
        if ( sts != 0) return sts;
#elif defined(OS_LYNX)
	sts = pthread_create (
		&thread,
		pthread_attr_default,		/* attr */
		xtt_logproc,		/* start_routine */
		(void *)this);			/* arg */
        if ( sts != 0) return sts;
#endif

	strcpy( msg, "Logg start ");
	dcli_fgetname( logg_file, msg + strlen(msg),
		logg_filename);
	message('I', msg);
	return XNAV__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_logging_stop()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Stop the entry.
*
**************************************************************************/

int XttLogging::stop()
{
	if ( !active )	
	{
	  message('E',"Entry is already stopped");
	  return XNAV__HOLDCOMMAND;
	} 

	entry_stop();

	message('I', "Logging stopped");
	return XNAV__SUCCESS;
}

/*************************************************************************
*
* Name:		entry_stop()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Stop the entry.
*
**************************************************************************/

int XttLogging::entry_stop()
{
	int		i;
	int		sts;
	
	/* This will stop the subprocess */
	active = 0;
	stop_logg = 1;

	/* Unref from gdh */
	for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	{
	  if ( parameterstr[i][0] != 0)
	  {
	    sts = gdh_UnrefObjectInfo ( parameter_subid[i]);
	  }
	}
	if ( condition_ptr != 0)
	  sts = gdh_UnrefObjectInfo ( condition_subid);

	return XNAV__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_logging_delete()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Delete the entry.
*
**************************************************************************/

int XttLogging::remove( char		*parameter)
{
	int		i;
	int		found;
	
	/* Remove this parameter */
	found = 0;
	for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	{
	  if ( strcmp( parameterstr[i], parameter) == 0)
	  {
	    /* Parmeter is found, remove it */
	    parameterstr[i][0] = 0;
	    message('I', "Parameter removed");
	    found = 1;
	    break;
	  }
	}
        if ( !found)
	{
	  message('E',"Parameter not found");
	  return XNAV__HOLDCOMMAND;
	}

        return XNAV__SUCCESS;
}

/*************************************************************************
*
* Name:		xtt_logproc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Logging subprocess.
*
**************************************************************************/

static void	*xtt_logproc( void *arg)
{
	int		sts;
	int		i, j, k;
	int		char_cnt;
	pwr_tTime	time;
	pwr_tDeltaTime	timediff;
	char		time_str[80];
	float		time_float;
	char		*value_ptr;
	char		*old_value_ptr;
	int		first_scan;
	pwr_tObjid		objid;
	pwr_sAttrRef		*attrref;
	char			hiername[120];
	char			timstr[64];
	char			parname[40];
	char			*s;
	pwr_tTime	nextime;
	pwr_tTime	restime;
	pwr_tDeltaTime	deltatime;
	pwr_tDeltaTime	wait_time;
#ifdef OS_VMS
 	pwr_tVaxTime 	vmstime;
#endif
	XttLogging *logg = (XttLogging *) arg;


	char_cnt = 0;
	first_scan = 1;

        clock_gettime( CLOCK_REALTIME, &nextime);

	logg->starttime = nextime;

	/* Print starttime and logged parameters on the file */
 	time_AtoAscii( &logg->starttime, time_eFormat_DateAndTime, 
			time_str, sizeof(time_str));

	switch ( logg->logg_type)
	{
	  case xtt_LoggType_Cont:
	    if ( logg->logg_file)
	      fprintf( logg->logg_file, "\"\"");
	    /* Find a unic shortname for each parameter */
	    for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	    {
	      if ( logg->print_shortname)
	      {
	        if ( logg->parameterstr[i][0] != 0)
	        {
		  /* Print only last segment and not ActualValue */
	          xnav_cut_segments( logg->shortname[i], logg->parameterstr[i], 1);
	          if ( (s = strchr( logg->shortname[i], '.')) != 0)
                  {
	            cdh_ToUpper( parname, s+1);
	            if ( strcmp( parname, "ACTUALVALUE") == 0)
                      *s = 0;
                  }
	          /* Check that this name is unic */
	          for ( j = 0; j < RTT_LOGG_MAXPAR; j++)
	          {
	            if ( j != i &&
	                 !strcmp( logg->shortname[i], logg->shortname[j]))
	            {
	              for ( k = 2; k < 7; k++)
	              {                  
	                /* Increase number of segments */
	                xnav_cut_segments( logg->shortname[i], logg->parameterstr[i], k);
	                if ( (s = strchr( logg->shortname[i], '.')) != 0)
                        {
	                  cdh_ToUpper( parname, s+1);
	                  if ( strcmp( parname, "ACTUALVALUE") == 0)
                            *s = 0;
                        }
	                xnav_cut_segments( logg->shortname[j], logg->parameterstr[j], k);
	                if ( (s = strchr( logg->shortname[j], '.')) != 0)
                        {
	                  cdh_ToUpper( parname, s+1);
	                  if ( strcmp( parname, "ACTUALVALUE") == 0)
                            *s = 0;
                        }
	                if ( strcmp( logg->shortname[i], logg->shortname[j]))
	                  break;
	              }
	            }
	          }
	        }
	      }
	      else
	        strcpy( logg->shortname[i], logg->parameterstr[i]);
	    }
	    for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	    {
	      if ( logg->parameterstr[i][0] != 0)
	      {
	        if ( logg->logg_file)
	        {
	          char_cnt += fprintf( logg->logg_file, "	%s",
			logg->shortname[i]);
	          if (char_cnt + 120 > logg->line_size)
	          {
	            fprintf( logg->logg_file, "\n");
	            char_cnt = 0;    
	          }
	        }
	      }
	    }
	    if ( logg->logg_file)
	    {
	      fprintf( logg->logg_file, "\n");
	      char_cnt = 0;    
	    }
	    break;

	  case xtt_LoggType_Mod:
	    if ( logg->logg_file)
	      fprintf( logg->logg_file, "RTT LOGGING STARTED AT %s\n", 
		time_str);
	    for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	    {
	      if ( logg->parameterstr[i][0] != 0)
	      {
	        if ( logg->logg_file)
	          fprintf( logg->logg_file, "Parameter: %s\n",
		logg->parameterstr[i]);
	      }
	    }
	    break;
	}

	time_MsToD( &deltatime, logg->logg_time);

	if ( logg->logg_priority != 0)
	  sts = logg->set_prio( logg->logg_priority);

	for (;;)
	{

	  /* Calculation of starttime for next loop */
	  time_Aadd( &restime, &nextime, &deltatime);
	  nextime = restime;

	  if ( logg->condition_ptr != 0)
	  {
	    if ( logg->active && !logg->stop_logg)
	    {
	      if ( !*(logg->condition_ptr))
	      {
	        /*  Don't log, wait until next scan */
#ifdef OS_VMS
	        time_PwrToVms( &nextime, &vmstime);
	        sys$clref( logg->event_flag);
	        sys$setimr( logg->event_flag, &vmstime, 0, 0, 0);
	        sys$waitfr( logg->event_flag);
#endif
#if defined OS_LYNX || defined OS_LINUX
	        clock_gettime( CLOCK_REALTIME, &time);
	        time_Adiff( &wait_time, &nextime, &time);
                nanosleep( (struct timespec *) &wait_time, NULL);
#endif
	        continue;
	      }
	    }
	  }

	  clock_gettime( CLOCK_REALTIME, &time);
	  switch ( logg->logg_type)
	  {
	    case xtt_LoggType_Cont:
	      /* Convert time to seconds since start */
	      time_Adiff( &timediff, &time, &logg->starttime);
	      time_float = time_DToFloat( NULL, &timediff);
	
	      /* fix */
	      if ( first_scan)
	        time_float = 0.;
	      /* Print time and the value of the parameter on the file */
	      char_cnt += logg->log_print( "%11.3f", time_float);
	      for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	      {
	        if ( logg->parameterstr[i][0] != 0)
	        {
	          value_ptr = logg->parameter_ptr[i];
	          switch ( logg->parameter_type[i])
	          {
	            case pwr_eType_Float32:
	              char_cnt += logg->log_print( "	%f", *(pwr_tFloat32 *)value_ptr);
	              break;

	            case pwr_eType_Float64:
	              char_cnt += logg->log_print( "	%f", *(pwr_tFloat64 *)value_ptr);
	              break;

	            case pwr_eType_UInt8:
	              char_cnt += logg->log_print( "	%d", 
					*(pwr_tUInt8 *)value_ptr);
	              break;
	            case pwr_eType_Boolean:
	              char_cnt += logg->log_print( "	%d", 
					*(pwr_tBoolean *)value_ptr);
	              break;
	            case pwr_eType_Char:
	              char_cnt += logg->log_print( "	%c", *(pwr_tChar *)value_ptr);
	              break;
	            case pwr_eType_Int8:
	              char_cnt += logg->log_print( "	%d", *(pwr_tInt8 *)value_ptr);
	              break;
	            case pwr_eType_Int16:
	              char_cnt += logg->log_print( "	%d", *(pwr_tInt16 *)value_ptr);
	              break;
	            case pwr_eType_UInt16:
	              char_cnt += logg->log_print( "	%d", *(pwr_tUInt16 *)value_ptr);
	              break;
	            case pwr_eType_Int32:
	              char_cnt += logg->log_print( "	%d", *(pwr_tInt32 *)value_ptr);
	              break;
	            case pwr_eType_UInt32:
	              char_cnt += logg->log_print( "	%d", *(pwr_tUInt32 *)value_ptr);
	              break;
	            case pwr_eType_Objid:
	              objid = *(pwr_tObjid *)value_ptr;
	              if ( !objid.oix)
	                sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
				 cdh_mName_volumeStrict);
	              else
	                sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
				cdh_mNName);
	              if (EVEN(sts)) strcpy( hiername, "** Unknown objid");
	              char_cnt += logg->log_print( "	%s", hiername);
	              break;
	            case pwr_eType_AttrRef:
	              attrref = (pwr_sAttrRef *) value_ptr;
	              sts = gdh_AttrrefToName ( attrref, hiername, sizeof(hiername), cdh_mNName);
	              if ( EVEN(sts)) strcpy( hiername, "** Unknown attrref");
	              char_cnt += logg->log_print( "	%s", hiername);
	              break;
	            case pwr_eType_Time:
	              sts = time_AtoAscii( (pwr_tTime *) value_ptr, 
			    time_eFormat_DateAndTime, timstr, sizeof(timstr));
	              if ( EVEN(sts))
	                strcpy( timstr, "Undefined time");
	              char_cnt += logg->log_print( "	%s", timstr);
	              break;
	            case pwr_eType_DeltaTime:
	              sts = time_DtoAscii( (pwr_tDeltaTime *) value_ptr, 
			    	1, timstr, sizeof(timstr));
	              if ( EVEN(sts))
	                strcpy( timstr, "Undefined time");
	              char_cnt += logg->log_print( "	%s", timstr);
	              break;
	            default:
	              char_cnt += logg->log_print( "	%s", "Type error");
	          }
	          if (char_cnt + 10 > logg->line_size)
	          {
	            logg->log_print( "\n");
	            char_cnt = 0;    
	          }
	        }
	      }
	      logg->log_print( "\n");
	      char_cnt = 0;    
	   
	      break;

	    case xtt_LoggType_Mod:
	      /* Write only if value is changed */
	      for ( i = 0; i < RTT_LOGG_MAXPAR; i++)
	      {
	        if ( logg->parameterstr[i][0] != 0)
	        {
	          value_ptr = logg->parameter_ptr[i];
	          old_value_ptr = (char *) &logg->old_value[i];
	          switch ( logg->parameter_type[i])
	          {
	            case pwr_eType_Float32:
	              if ( (*(pwr_tFloat32 *)value_ptr != 
				*(pwr_tFloat32 *)old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s", 
				&(logg->parameterstr[i]));
	                logg->log_print( "	%f\n", *(pwr_tFloat32 *)value_ptr);
	                *(pwr_tFloat32 *)old_value_ptr = *(pwr_tFloat32 *)value_ptr;
	              }
	              break;

	            case pwr_eType_Float64:
	              if ( (*(pwr_tFloat64 *)value_ptr != 
				*(pwr_tFloat64 *)old_value_ptr) || first_scan)
	              {
	                logg->log_print( "	%s", 
				&(logg->parameterstr[i]));
	                logg->log_print( "	%f\n", *(pwr_tFloat64 *)value_ptr);
	                  *(pwr_tFloat64 *)old_value_ptr = *(pwr_tFloat64 *)value_ptr;
	              }
	              break;

	            case pwr_eType_Boolean:
	              if ( (*(pwr_tBoolean *)value_ptr != 
				*(pwr_tBoolean *)old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%d\n", *(pwr_tBoolean *)value_ptr);
	                *(pwr_tBoolean *)old_value_ptr = 
				*(pwr_tBoolean *)value_ptr;
	              }
	              break;
	            case pwr_eType_Char:
	              if ( (*(pwr_tChar *)value_ptr != 
				*(pwr_tChar *)old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%c\n", *(pwr_tChar *)value_ptr);
	                *(pwr_tChar *)old_value_ptr = 
				*(pwr_tChar *)value_ptr;
	              }
	              break;
	            case pwr_eType_UInt8:
	              if ( (*(pwr_tUInt8 *)value_ptr != 
				*(pwr_tUInt8 *)old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%d\n", *(pwr_tUInt8 *)value_ptr);
	                *(pwr_tUInt8 *)old_value_ptr = 
				*(pwr_tUInt8 *)value_ptr;
	              }
	              break;
	            case pwr_eType_Int8:
	              if ( (*(pwr_tInt8 *)value_ptr != 
				*(pwr_tInt8 *) old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%d\n", *value_ptr);
	                *(pwr_tInt8 *)old_value_ptr = *(pwr_tInt8 *)value_ptr;
	              }
	              break;
	            case pwr_eType_UInt16:
	              if ( (*(pwr_tUInt16 *)value_ptr != 
				*(pwr_tUInt16 *)old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%d\n", *(pwr_tUInt16 *)value_ptr);
	                *(pwr_tUInt16 *)old_value_ptr = 
				*(pwr_tUInt16 *)value_ptr;
	              }
	              break;
	            case pwr_eType_Int16:
	              if ( (*(pwr_tInt16 *)value_ptr != 
				*(pwr_tInt16 *)old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%d\n", *(pwr_tInt16 *)value_ptr);
	                *(pwr_tInt16 *)old_value_ptr = *(pwr_tInt16 *)value_ptr;
	              }
	              break;
	            case pwr_eType_UInt32:
	              if ( (*(pwr_tUInt32 *)value_ptr != 
				*(pwr_tUInt32 *)old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%d\n", *(pwr_tUInt32 *)value_ptr);
	                *(pwr_tUInt32 *)old_value_ptr = 
				*(pwr_tUInt32 *)value_ptr;
	              }
	              break;
	            case pwr_eType_Int32:
	              if ( (*(pwr_tInt32 *)value_ptr != 
				*(pwr_tInt32 *)old_value_ptr) || first_scan)
	              {
	                /* Value is changed, print */
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%d\n", *(pwr_tInt32 *)value_ptr);
	                *(pwr_tInt32 *)old_value_ptr = 
				*(pwr_tInt32 *)value_ptr;
	              }
	              break;
	            case pwr_eType_Objid:
	              if ( memcmp( value_ptr, old_value_ptr, 
				sizeof( pwr_tObjid)) != 0 || first_scan)
	              {
	                /* Value is changed, print */
	                objid = *(pwr_tObjid *)value_ptr;
	                if ( !objid.oix)
	                  sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
				 cdh_mName_volumeStrict);
	                else
	                  sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), 
				cdh_mNName);
	                if (EVEN(sts)) strcpy( hiername, "** Unknown objid");
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%s\n", hiername);
	                memcpy( old_value_ptr, value_ptr, sizeof(pwr_tObjid));
	              }
	              break;
	            case pwr_eType_AttrRef:
	              attrref = (pwr_sAttrRef *) value_ptr;
	              /* There is only space for the objid of the attrref in oldvalue .. */
	              if ( memcmp( (char *) &attrref->Objid, old_value_ptr,
				sizeof( pwr_tObjid)) != 0 || first_scan)
	              {
	                /* At least the objid is changed */
	                sts = gdh_AttrrefToName ( attrref, hiername, sizeof(hiername), cdh_mNName);
	                if (EVEN(sts)) strcpy( hiername, "** Unknown attrref");
	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",  
				&(logg->parameterstr[i]));
	                logg->log_print( "	%s\n", hiername);
	                memcpy( old_value_ptr, (char *) &attrref->Objid, sizeof(pwr_tObjid));
	              }
	              break;
	            case pwr_eType_Time:
	              if ( memcmp( value_ptr, old_value_ptr, 
				sizeof( pwr_tTime)) != 0 || first_scan)
	              {
	                /* Value is changed, print */
	                sts = time_AtoAscii( (pwr_tTime *) value_ptr, 
		           time_eFormat_DateAndTime, timstr, sizeof(timstr));
	                if ( EVEN(sts))
	                  strcpy( timstr, "Undefined time");

	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",
				&(logg->parameterstr[i]));
	                logg->log_print( "	%s\n", timstr);
	                memcpy( old_value_ptr, value_ptr, sizeof(pwr_tTime));
	              }
	              break;
	            case pwr_eType_DeltaTime:
	              if ( memcmp( value_ptr, old_value_ptr, 
				sizeof( pwr_tTime)) != 0 || first_scan)
	              {
	                /* Value is changed, print */
	                sts = time_DtoAscii( (pwr_tDeltaTime *) value_ptr, 
		           1, timstr, sizeof(timstr));
	                if ( EVEN(sts))
	                  strcpy( timstr, "Undefined time");

	                time_AtoAscii( &time, time_eFormat_DateAndTime, 
				time_str, sizeof(time_str));
	                logg->log_print( "%s", &time_str);
	                logg->log_print( "	%s",
				&(logg->parameterstr[i]));
	                logg->log_print( "	%s\n", timstr);
	                memcpy( old_value_ptr, value_ptr, sizeof(pwr_tDeltaTime));
	              }
	              break;
	          }
	        }
	      }
	      
	      break;
	  }
	  first_scan = 0;

	  /*  Wait "cytime" ms */

	  if ( !logg->active || logg->stop_logg)
	  {
	    logg->entry_stop();
	    logg->print_buffer();
	    if ( logg->logg_file)
	      fclose( logg->logg_file);
	    logg->logg_file = 0;

	    if ( logg->logg_priority != 0)
	      sts = logg->set_default_prio();

	    pthread_exit( (void *) 1);
	  }
          clock_gettime( CLOCK_REALTIME, &time);
	  while( time_Acomp( &time, &nextime) > 0)
	  {
	    /* To late for next lap, skip it */
	    time_Aadd( &restime, &nextime, &deltatime);
	    nextime = restime;
	  }

#ifdef OS_VMS
	  time_PwrToVms( &nextime, &vmstime);
	  sys$clref( logg->event_flag);
	  sys$setimr( logg->event_flag, &vmstime, 0, 0, 0);
	  sys$waitfr( logg->event_flag);
#endif
#if defined OS_LYNX || defined OS_LINUX
	  time_Adiff( &wait_time, &nextime, &time);
          nanosleep( (struct timespec *) &wait_time, NULL);
#endif

	}

	// pthread_exit(0);

	// return NULL;
}


/*************************************************************************
*
* Name:		get_parinfo()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get parameter info.
*
**************************************************************************/

int  XttLogging::get_parinfo(
			char		*parameter_name,
			pwr_sParInfo	*parinfo)
{
	char		hiername[80];
	char		parname[80];
	char		name_array[2][80];
	int		nr;
	int		sts;
	pwr_tObjid	objid;
	pwr_tObjid	parameter;
	pwr_tClassId	classid;
	char		objname[80];
	char		classname[80];

	/* Get object name */
	/* Parse the parameter name into a object and a parameter name */
	nr = dcli_parse( parameter_name, ".", "",
		(char *)name_array, 
		sizeof( name_array)/sizeof( name_array[0]), 
		sizeof( name_array[0]), 0);
	if ( nr < 2)
	  return XNAV__OBJECTNOTFOUND;

	strcpy( objname, name_array[0]);
	strcpy( parname, name_array[1]);

	/* Get objid */
	sts = gdh_NameToObjid ( objname, &objid);
	if ( EVEN(sts)) return sts;

	/* Get class name */	
	sts = gdh_GetObjectClass ( objid, &classid);
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( cdh_ClassIdToObjid(classid), classname, 
            sizeof(classname), cdh_mName_object);
	if ( EVEN(sts)) return sts;

	/* Get objid of rtbody */
	sts = gdh_ObjidToName ( cdh_ClassIdToObjid(classid), hiername, 
			sizeof(hiername), cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;
	strcat( hiername, "-RtBody");
	sts = gdh_NameToObjid ( hiername, &parameter);
	if ( EVEN(sts)) 
	{
	  /* Try with sysbody */
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(classid), hiername,
                        sizeof(hiername), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) return sts;
	  strcat( hiername, "-SysBody");
	  sts = gdh_NameToObjid ( hiername, &parameter);
	  if ( EVEN(sts)) 
	  {
	    message('E', "Unable to open object");	  
	    return 0;
	  }
	}

	strcat( hiername, "-");
	strcat( hiername, parname);

	sts = gdh_GetObjectInfo ( hiername, parinfo, sizeof( *parinfo)); 
	if (EVEN(sts)) return sts;

	return XNAV__SUCCESS;
}

/*************************************************************************
*
* Name:		log_print()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Print. Equivalent to fprintf but the character string is put
*	in a buffer and printed when the buffer size is exceeded or 
*	when r_print_buffer is called.
*	The max size of the character string is 200.
*
**************************************************************************/

int XttLogging::log_print( 
		char		*format,
		... )
{
	char	buff[200];
	int	sts;
	char	*s;
	va_list ap;

	va_start( ap, format);
	sts = vsprintf( buff, format, ap);
	va_end( ap);

	s = buffer_ptr + buffer_count;
	strcpy( s, buff);
	buffer_count += strlen( buff);

	if ( buffer_count > ( buffer_size * 512 -
		(int)sizeof( buff)))
	{
	  if ( intern)
	  {
	    stop_logg = 1;
	    return 1;
	  }
	  print_buffer();
	}
	return sts;
}

/*************************************************************************
*
* Name:		print_buffer()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

int XttLogging::print_buffer()
{

        if ( logg_priority != 0)
	  set_default_prio();

	if ( logg_file)
	  fwrite( buffer_ptr, 1, buffer_count, 
		logg_file);

	if ( logg_priority != 0)
	  set_prio( logg_priority);

	buffer_count = 0;
	*(buffer_ptr) = 0;

	return XNAV__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_logging_close_files()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Close all open files.
*	This file is called at execute termintation to close the files.
*
**************************************************************************/
int XttLogging::close_files()
{
	if ( active)
	{
	  if ( logg_file)
	  {
	    print_buffer();
	    fclose( logg_file);
	  }
	}
	return XNAV__SUCCESS;
}

void XttLogging::message( char severity, char *msg)
{
  ((XNav *)xnav)->message( severity, msg);
}
