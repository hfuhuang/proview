/* co_dcli_file.c -- File handling in dcli

   PROVIEW/R  
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */


# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

#include "co_cdh.h"
#include "co_dcli_msg.h"
#include "co_dcli.h"


typedef enum {
  dcli_eTrans_EnvVar,
  dcli_eTrans_Dir,
  dcli_eTrans_File
} dcli_eTrans;


static char dcli_default_directory[200] = "";

void 	dcli_set_default_directory( char *dir)
{
  strcpy( dcli_default_directory, dir);
}

int	dcli_get_defaultfilename(
			char	*inname,
			char	*outname,
			char	*ext)
{
  char	filename[80];
  char 	*s, *s2;

#if defined(OS_VMS)
  if ( strchr( inname, '<') || strchr( inname, '[') || 
       strchr( inname, ':'))
    strcpy( outname, inname);
  else
  {
    strcpy( filename, dcli_default_directory);
    strcat( filename, inname);
    strcpy( outname, filename);
  }
#elif defined(OS_LYNX) || defined(OS_LINUX)
  if ( strchr( inname, '/'))
    strcpy( outname, inname);
  else if ( ( s = strchr( inname, ':')))
  {
    /* Replace VMS disp to env variable */
    strcpy( filename, "$");
    strncat( filename, inname, s - inname);
    filename[ s - inname + 1] = 0;
    strcat( filename, "/");
    strcat( filename, s + 1);
    dcli_replace_env( filename, outname);
  }
  else
  {
    if ( strcmp( dcli_default_directory, "") == 0)
    {
      char cwd[200];

      if ( getcwd( cwd, sizeof(cwd)) != NULL)
      {
        strcpy( filename, cwd);
        strcat( filename, "/");
      }
      else
        strcpy( filename, "");
      strcat( filename, inname);
      dcli_replace_env( filename, outname);
    }
    else
    {
      strcpy( filename, dcli_default_directory);
      if ( (filename[strlen(filename)-1] != '/') &&
         (inname[0] != '/'))
        strcat( filename, "/");
      strcat( filename, inname);
      dcli_replace_env( filename, outname);
    }
  }
#endif

  /* Look for extention in filename */
  if ( ext != NULL)
  {
    s = strrchr( inname, ':');
    if ( s == 0)
      s = inname;
  
    s2 = strrchr( s, '>');
    if ( s2 == 0)
    {
      s2 = strrchr( s, ']');
      if ( s2 == 0)
        s2 = s;
    }

    s = strrchr( s2, '.');
    if ( s == 0)
    {
      /* No extention found, add extention */
      strcat( outname, ext);
    }
  }

#if defined(OS_LYNX) || defined(OS_LINUX)
  cdh_ToLower( outname, outname);
#endif
  return DCLI__SUCCESS;
}

#if defined(OS_LYNX) || defined(OS_LINUX)

/*************************************************************************
*
* Name:		dcli_replace_env()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Replace env variables in UNIX filenames.
*	A string that starts with $ and ends with . or / will replaced 
*	if then string is found by getenv. Only lower case variables 
*	will be detected.
*
**************************************************************************/
int     dcli_replace_env( char *str, char *newstr)
{
  char    *s;
  char    *t;
  char    *u;
  int     symbolmode;
  int     size;
  char    *value;
  char    symbol[80];
  char    lower_symbol[80];
  char    new[160];

  symbolmode = 0;
  s = str;
  t = new;

  while ( *s != 0)
  {
    if (*s == '$')
    {
      symbolmode = 1;
      u = s + 1;
      *t = *s;
      t++;
    }
    else if (symbolmode && (*s == '/' || *s == '.'))
    {
      /* End of potential symbol */
      size = (int) s - (int) u;
      strncpy( symbol, u, size);
      symbol[size] = 0;
      if ( strcmp( symbol, "HOME") == 0)
	strcpy( lower_symbol, symbol);
      else
	cdh_ToLower( lower_symbol, symbol);
      if ( (value = getenv( lower_symbol)) == NULL)
      {
        /* It was no symbol */
        *t = *s;
        t++;
      }
      else
      {
        /* Symbol found */
        t -= strlen(symbol) + 1;
        strcpy( t, value);
        t += strlen(value);
        *t = *s;
        t++;
      }
      symbolmode = 0;
    }
    else
    {
      *t = *s;
      t++;
    }
    s++;
  }

  if ( symbolmode)
  {
    /* End of potential symbol */
    size = (int) s - (int) u;
    strncpy( symbol, u, size);
    symbol[size] = 0;
    cdh_ToLower( lower_symbol, symbol);
    if ( (value = getenv( lower_symbol)) == NULL)
    {
      /* It was no symbol */
      *t = 0;
    }
    else
    {
      /* Symbol found */
      t -= strlen(symbol) + 1;
      strcpy( t, value);
      t += strlen(value);
      *t = 0;
    }
  }
  else
    *t = 0;

  strcpy( newstr, new);
  return 1;
}
#endif

/*************************************************************************
*
* Name:		dcli_fgetname()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get filename for a filedescriptor.
*	This function is not implementet on all os, therefor a defaultname
*	should be supplied which is returned for this os.
*
**************************************************************************/
char	*dcli_fgetname( FILE *fp, char *name, char *def_name)
{
#if defined(OS_VMS) || defined(OS_ELN)
	return fgetname( fp, name);
#else
	dcli_translate_filename( name, def_name);
	return name;
#endif	
}


/*************************************************************************
*
* Name:		dcli_translate_filename()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Translate VMS or unix filename to the current platform.
*
**************************************************************************/

int dcli_translate_filename( char *out, const char *in)
{
  char out_name[200];
  const char *s;
  char *t;
  int i;
  int state;

#if defined OS_VMS
  if ( strchr( in, '/') != 0)
  {
    // Convert from unix to VMS
    for ( i = 0, s = in, t = out_name; *s; i++, s++)
    {
      if ( i == 0)
      {
        if ( *s == '$') 
          state = dcli_eTrans_EnvVar;
        else if ( *s == '/')
        {
          if ( strchr( s+1, '/') == 0)
            state = dcli_eTrans_File;
          else
          {
            state = dcli_eTrans_Dir;
            *t++ = '[';
          }
        }
        else
        {
          if ( strchr( s, '/') == 0)
          {
            state = dcli_eTrans_File;
            *t++ = *s;
          }
          else
          {
            state = dcli_eTrans_Dir;
            *t++ = '[';
            *t++ = '.';
            *t++ = *s;
          }
        }
        continue;
      }
      switch ( state)
      {
        case dcli_eTrans_EnvVar:
          if ( *s == '/')
          {
            if ( strchr( s+1, '/') != 0)
            {
              *t++ = ':';
              *t++ = '[';
              state = dcli_eTrans_Dir;
            }
            else
            {
              state = dcli_eTrans_File;
              *t++ = ':';
            }
          }
          else
            *t++ = *s;
          break;
        case dcli_eTrans_Dir:
          if ( *s == '/')
          {
            if ( strchr( s+1, '/') == 0)
            {
              *t++ = ']';
              state = dcli_eTrans_File;
            }
            else
              *t++ = '.';
          }
          else
            *t++ = *s;
          break;
        case dcli_eTrans_File:
          *t++ = *s;
          break;
      }
    }
    *t = 0;
    strcpy( out, out_name);
  }
  else
  {
    // Already VMS syntax
    strcpy( out_name, in);
    strcpy( out, out_name);
  }
  return DCLI__SUCCESS;
#elif defined OS_ELN
  if ( (s = strrchr( in, ']')) != 0 ||
       (s = strrchr( in, '>')) != 0 ||
       (s = strrchr( in, ':')) != 0 ||
       (s = strrchr( in, '/')) != 0)
  {
    strcpy( out_name, "[sys0.sysexe]");
    strcat( out_name, s+1);
    strcpy( out, out_name);
  }
  else
  {
    strcpy( out_name, "[sys0.sysexe]");
    strcat( out_name, in);
    strcpy( out, out_name);
  }
  return DCLI__SUCCESS;
#else
  int sts;

  if ( strchr( in, ':') != 0 ||
       strchr( in, '[') != 0 ||
       strchr( in, '<') != 0)
  {
    // Convert from VMS to unix
    for ( i = 0, s = in, t = out_name; *s; i++, s++)
    {
      if ( i == 0)
      {
        if ( strchr( in, ':') != 0) 
        {
          state = dcli_eTrans_EnvVar;
          *t++ = '$';
          *t++ = *s;
        }
        else if ( *s == '[' ||
                  *s == '<')
        {
          if ( *(s+1) == '.')
            s++;
          else
            *t++ = '/';
          state = dcli_eTrans_Dir;
        }
        else
        {
          state = dcli_eTrans_File;
          *t++ = *s;
        }
        continue;
      }
      switch ( state)
      {
        case dcli_eTrans_EnvVar:
          if ( *s == ':')
          {
            *t++ = '/';
            if ( *(s+1) == '[' ||
                 *(s+1) == '<')
            {
              state = dcli_eTrans_Dir;
              s++;
            }
            else
              state = dcli_eTrans_File;
          }
          else
            *t++ = *s;
          break;
        case dcli_eTrans_Dir:
          if ( *s == '.')
          {
            *t++ = '/';
          }
          else if ( *s == ']' ||
                    *s == '>')
          {
            *t++ = '/';
            state = dcli_eTrans_File;
          }
          else
            *t++ = *s;
          break;
        case dcli_eTrans_File:
          *t++ = *s;
          break;
      }
    }
    *t = 0;
    sts = dcli_replace_env( out_name, out);
    return sts;
  }
  else
  {
    // Already unix syntax
    strcpy( out_name, in);
    sts = dcli_replace_env( out_name, out);
    return sts;
  }
#endif
}




