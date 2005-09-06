/* 
 * Proview   $Id: wb_dir.c,v 1.3 2005-09-06 10:43:31 claes Exp $
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

/* wb_dir.c -- Directory information
   This module returns information about directory and files.  */

#ifdef OS_VMS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <starlet.h>
#include <rms.h>
#include <descrip.h>
#include <libdef.h>
#include <libdtdef.h>
#include <lib$routines.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "pwr.h"
#include "co_time.h"
#include "co_dcli.h"
#include "wb_foe_macros.h"
#include "wb_dir.h"

/* Global functions_____________________________________________________*/

#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)


/*************************************************************************
*
* Name:		dir_parse_filename
*
* Type		int
*
* Description:
*	Parses a filename in device, directory, namn, type and version.
*
* Parameters
*	filename	*char	I  Filenamn that will be parsed.
*	dev		*char	O  device.
*	dir		*char	O  directory.
*	file		*char	O  filenamn.
*	type		*char	O  type.
*	version		*int	O  version.
*
**************************************************************************/

pwr_tStatus dir_parse_filename( char	*filename,
				char	*dev,
				char	*dir,
				char	*file,
				char	*type,
				int	*version)
{
	char	*s, *t;
	char	ldev[200];
	char	ldir[200];
	char	lfile[200];
	char	ltype[80];
	char	lversion[80];
	int	sts;

	if ( (s = strstr( filename, "::")))
	  s += 2;
	else
	  s = filename;
	strcpy( ldev, s);

	/* Device */
	if ( (s = strchr( ldev, ':')))
	  s++;
	else
	  s = ldev;
	strcpy( ldir, s);
	*s = 0;

	/* Directory */
	if ( (s = strchr( ldir, '>')))
	  s++;
	else
	{
	  if ( (s = strchr( ldir, ']')))
	    s++;
	  else
	    s = ldir;
	}
	strcpy( lfile, s);
	*s = 0;

	/* File */
	if ( (s = strchr( lfile, '.')) == 0)
	  s = lfile;
	strcpy( ltype, s);
	*s = 0;

	/* Type */
	if ( (s = strchr( ltype, ';')))
	  t = s++;
	else
	{
	  if (( s = strchr( ltype + 1, '.')))
	    t = s++;
	  else
	  {
	    s = ltype;
	    t = s + strlen(s);
	  }
	}
	strcpy( lversion, t);
	*t = 0;

	if ( strcmp( lversion, "") == 0)
	{
	  *version = 0;
	}
	else
	{
	  sts = sscanf( lversion, "%d", version);
	  if ( sts != 1)
	    *version = 0;
	}

	strcpy( dev, ldev);
	strcpy( dir, ldir);
	strcpy( file, lfile);
	strcpy( type, ltype);
	return 1;
}  


/****************************************************************************
* Name:		dir_get_fileinfo ()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
* 	Send back creation time and size of a given file 
*
**************************************************************************/
pwr_tStatus dir_get_fileinfo( 	char 	*file_name,
                         	pwr_tTime *time2_ptr,
			 	int 	*size_p,
				int 	*vmsvers_p,
				char	*found_file) 
{
#ifdef OS_VMS
  static struct NAM 	nam1;
  static struct FAB 	fab1;
  static struct	XABDAT	dat1; 
  static pwr_tTime    	zero2 = {0 ,0};
  char			result_name[NAM$C_MAXRSS];
  int 			sys1_sts , sys2_sts; 

  /* Initialize RMS user structures for the checking file. */
  fab1  	 = cc$rms_fab;
  fab1.fab$l_xab = (char *) &dat1;
  fab1.fab$l_nam = &nam1;
  
  dat1  	 = cc$rms_xabdat;
  dat1.xab$l_nxt = 0;

  fab1.fab$l_fna = file_name;
  fab1.fab$b_fns = strlen (file_name);

  nam1  	 = cc$rms_nam;
  nam1.nam$l_rsa = result_name;
  nam1.nam$b_rss = sizeof(result_name);

  sys1_sts= sys$open (&fab1);

  if ((sys1_sts & 1) == 0) 
  { 
    time_VmsToPwr((pwr_tVaxTime *)&dat1.xab$q_cdt, time2_ptr);
    return (sys1_sts); 
  }
  else
  {
    if ( size_p != NULL) 
        *size_p = fab1.fab$l_alq; 

    if ( vmsvers_p != NULL) 
    {
      nam1.nam$l_ver[nam1.nam$b_ver] = '\0'; 
      nam1.nam$l_ver++;
        
      sscanf ( nam1.nam$l_ver , "%d" , vmsvers_p );
    }
    time_VmsToPwr((pwr_tVaxTime *)&dat1.xab$q_cdt, time2_ptr);
    result_name[nam1.nam$b_rsl] = 0;
    if ( found_file != NULL)
      strcpy( found_file, result_name);
    sys2_sts = sys$close (&fab1);
  }

  sys2_sts = sys$close (&fab1);
#else
  struct stat info;
  int	sts;
  char  fname[200];

  dcli_translate_filename( fname, file_name);
  sts = stat( fname, &info);
  if ( sts == -1)
    return 0;

  if ( time2_ptr) {
    time2_ptr->tv_sec = info.st_ctime;
    time2_ptr->tv_nsec = 0;
  }
  if ( size_p)
    *size_p = (int) info.st_size;
  if ( vmsvers_p)
    *vmsvers_p = 0;  
  if ( found_file != NULL)
    strcpy( found_file, file_name);
  
#endif
  return 1; 
}  


/*************************************************************************
*
* Name:		dir_TimeString
*
* Type		*char
*
* Description: 
*		Converts a VMS-time to a string.
*
* Parameters
*
**************************************************************************/

char	*dir_TimeString( 		pwr_tTime	*time,
					char		*timestr)
{
	static char			timstr[32];

	time_AtoAscii(time, time_eFormat_DateAndTime, timstr, sizeof(timstr));         
	if ( timestr != NULL)
	  strcpy( timestr, &timstr[5]);

	return timstr;
}


/*************************************************************************
*
* Name:		dir_CopyFile
*
* Type		pwr_tStatus
*
* Description: 
*		Copy a file.
*
* Parameters
*
**************************************************************************/
pwr_tStatus dir_CopyFile( 
	char	*from,
	char	*to
)
{
	int		sts;
	static char	cmd[100];
#ifdef OS_VMS
	$DESCRIPTOR(cmd_desc,cmd);
	sprintf( cmd, "copy/nolog %s %s", from, to);
#elif OS_LINUX
	sprintf( cmd, "cp %s %s", from, to);
#endif
	sts = system( cmd);

	return sts;
}


/*************************************************************************
*
* Name:		dir_PurgeFile
*
* Type		pwr_tStatus
*
* Description: 
*		Purge a file.
*
* Parameters
*
**************************************************************************/
pwr_tStatus dir_PurgeFile( 
	char	*filename,
	int	keep
)
{
#ifdef OS_VMS
	int		sts;
	static char	cmd[100];
	$DESCRIPTOR(cmd_desc,cmd);
	sprintf( cmd, "purge/nolog/keep=%d %s", keep, filename);
	sts = system( cmd);
	return sts;
#else
	return 1;
#endif
}


/*************************************************************************
*
* Name:		dir_DeleteFile
*
* Type		pwr_tStatus
*
* Description: 
*		Delete a file.
*
* Parameters
*
**************************************************************************/
pwr_tStatus dir_DeleteFile( 
	char	*filename
)
{
	int		sts;
	static char	cmd[100];
#ifdef OS_VMS
	$DESCRIPTOR(cmd_desc,cmd);
	sprintf( cmd, "delete_/nolog %s", filename);
#elif OS_LINUX
	sprintf( cmd, "rm %s", filename);
#endif	
	sts = system( cmd);

	return sts;
}



/*************************************************************************
*
* Name:		dir_DefineLogical
*
* Type		pwr_tStatus
*
* Description: 
*		Define a logical name.
*
* Parameters
*
**************************************************************************/
pwr_tStatus dir_DefineLogical( 
	char	*name,
	char	*value,
	char	*table
)
{
#ifdef OS_VMS
	pwr_tStatus sts;
	struct dsc$descriptor_s name_desc = 
			{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
	struct dsc$descriptor_s table_desc = 
			{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
	struct dsc$descriptor_s value_desc = 
			{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};

	name_desc.dsc$w_length = strlen( name);
	name_desc.dsc$a_pointer = name;
	table_desc.dsc$w_length = strlen( table);
	table_desc.dsc$a_pointer = table;
	value_desc.dsc$w_length = strlen( value);
	value_desc.dsc$a_pointer = value;

	sts = lib$set_logical( &name_desc, &value_desc, &table_desc,
		NULL,NULL);
	return sts;
#else
	printf("DefineLogical: NYI\n");
	return 0;
#endif
}
/*************************************************************************
*
* Name:		dir_DeassignLogical
*
* Type		pwr_tStatus
*
* Description: 
*		Deassign a logical name.
*
* Parameters
*
**************************************************************************/
pwr_tStatus dir_DeassignLogical( 
	char	*name,
	char	*table
)
{
#ifdef OS_VMS
	pwr_tStatus sts;
	struct dsc$descriptor_s name_desc = 
			{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
	struct dsc$descriptor_s table_desc = 
			{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};

	name_desc.dsc$w_length = strlen( name);
	name_desc.dsc$a_pointer = name;
	table_desc.dsc$w_length = strlen( table);
	table_desc.dsc$a_pointer = table;

	sts = lib$delete_logical( &name_desc, &table_desc);
	return sts;
#else
	printf("DeassignLogical: NYI\n");
	return 0;
#endif
}


#ifdef RMS_TEST
main()
{
	int	sts;
	long	date2[2];
	char	wild_filename[80] = "t*.t";
	char	filename[80];
	char	dev[80];
	char	dir[80];
	char	file[80];
	char	type[80];
	int	version;

	sts = dir_search_file( wild_filename, filename, 1, 0);
	if (EVEN(sts)) printf("No files\n");
	printf("%s\n", filename);

	sts = dir_parse_filename( filename, dev, dir, file, type, &version);

	while (ODD(sts))
	{
	  sts = dir_search_file( wild_filename, filename, 0, 0); 
	  if (ODD(sts)) printf("%s\n", filename);
	}
}
#endif
