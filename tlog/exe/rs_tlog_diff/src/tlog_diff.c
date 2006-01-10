/* 
 * Proview   $Id: tlog_diff.c,v 1.1 2006-01-10 14:38:36 claes Exp $
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

/*_Include files_________________________________________________________*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <descrip.h>
#include <processes.h>
#include <starlet.h>
#include <libdef.h>
#include <libdtdef.h>
#include <lib$routines.h>

#include "pwr.h"
#include "pwr_class.h"
#include "wb_dir.h"
#include "co_time.h"
#include "rs_tlog_msg.h"
#include "tlog_diff.h"

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define	TLOG_LINEALLOC	50
#define TLOG_TIME_EPSILON 0.05

#define TLOG_LINE_TEXT		56
#define TLOG_LINE_ATTRIBUTE	15
#define TLOG_LINE_TYPE		12


enum tlog_ee_logtype
{
	tlog_logtype_DtLogUp,
	tlog_logtype_DtLogDown,
	tlog_logtype_AtLogUp,
	tlog_logtype_AtLogDown,
	tlog_logtype_Info,
	tlog_logtype_AlarmA,
	tlog_logtype_AlarmB,
	tlog_logtype_AlarmC,
	tlog_logtype_AlarmD,
	tlog_logtype_Modify,
	tlog_logtype_UnModify,
	tlog_logtype_Close,
	tlog_logtype_CAtLog,
	tlog_logtype_CDtLog,
	tlog_logtype_COtLog,
	tlog_logtype_ExecOn,
	tlog_logtype_ExecOff
};
typedef enum tlog_ee_logtype tlog_e_logtype;

typedef	struct {
	pwr_tString132	line;
	pwr_tDeltaTime	time;
	tlog_e_logtype	logtype;
	float		value;
	int		line_nr;
	int		cmp_index;
	int		written;
		} tlog_t_linelist;

typedef struct {
	tlog_t_linelist	*new_list;
	tlog_t_linelist	*old_list;
	int	new_list_count;
	int	old_list_count;
	int	current_new;
	int	current_old;
	float	max_difftime;
	FILE	*outfile;
	int	diff_found;
	int	parallell;
	int	attribute;
	int	text;
	int	ttext;
	int	noorder;
	int	exact;
	char	new_filename[80];
	char	old_filename[80];
	} *diff_ctx;

static int	announce = 0;

static int	tlog_get_defaultfilename( 	char *inname, 
						char *outname, 
						char *ext,
						char *disk);
static int	tlog_get_modulename( 		char *inname, 
						char *outname);
static int	tlog_checktime( pwr_tDeltaTime	*time_new,
				pwr_tDeltaTime	*time_old,
				float		maxdiff);
static int	tlog_print_line( diff_ctx ctx, tlog_t_linelist *list, 
		int index, int old);
static int	tlog_print_linepar( diff_ctx ctx, tlog_t_linelist *newlist, 
		tlog_t_linelist	*oldlist,
		int 		newindex, 
		int 		oldindex);
static int	tlog_lists_cmp_line( diff_ctx ctx);
static int	tlog_line_compare( tlog_t_linelist *newlist_ptr, 
		tlog_t_linelist *oldlist_ptr);
static void	tlog_lists_cmp_printparseline( diff_ctx ctx);
static int	tlog_lists_cmp_printnew( diff_ctx ctx, int index, int *lines);
static int	tlog_lists_cmp_printold( diff_ctx ctx, int index, int *lines);
static int	tlog_lists_cmp_printboth( diff_ctx ctx, int newindex, int oldindex,
		int *newlines, int *oldlines);
static int	tlog_lists_cmp( 	diff_ctx	ctx);
static int tlog_read_line(	char	*line,
				int	maxsize,
				FILE	*file);
static pwr_tStatus	tlog_line_add(	pwr_tString132	*line,
				tlog_t_linelist	**list,
				int		*list_count,
				int		*alloc, 
				int		line_nr);
static pwr_tStatus	tlog_insert_file( 	char 		*filename,
					tlog_t_linelist **list,
					int		*list_count,
					char		*full_filename);
static pwr_tStatus tlog_qual_to_time( 	char 		*in_str, 
					pwr_tTime 	*time);

/*************************************************************************
*
* Name:		tlog_get_defaultfilename()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

static int	tlog_get_defaultfilename( 	char *inname, 
						char *outname, 
						char *ext,
						char *disk)
{
	char	*s;
	char	*s2;
	char	filename[80];

	/* Add default disk if no disk is suplied */
	if ( !(strchr( inname, ':') || strchr(inname, '<') || 
		strchr(inname, '[')))
	{
	  strcpy( filename, disk);
	  strcat( filename, inname);
	  strcpy( outname, filename);
	}
	else
	  strcpy( outname, inname);
	
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

	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_get_modulename()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/

static int	tlog_get_modulename( 		char *inname, 
						char *outname)
{
	char	*s;
	char	*s2;
	char	filename[80];

	s2 = 0;
	if ( s = strchr( inname, ':'))
	  s2 = s;
	if ( s = strchr( inname, ']'))
	  s2 = s;
	if ( s = strchr( inname, '>'))
	  s2 = s;
	if ( s2 )
	  strcpy( outname, s2 + 1);
	else
	  strcpy( outname, inname);
	
	if ( s = strrchr( outname, '.'))
	  *s = 0;

	return TLOG__SUCCESS;
}


/*************************************************************************
*
* Name:		tlog_checktime
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	
*
**************************************************************************/

static int	tlog_checktime( pwr_tDeltaTime	*time_new,
				pwr_tDeltaTime	*time_old,
				float		maxdiff)
{
	pwr_tDeltaTime	tim_maxdiff;
	pwr_tDeltaTime	tim_limlow;
	pwr_tDeltaTime	tim_limhigh;
	int		sts;


	time_FloatToD( &tim_maxdiff, maxdiff);
	time_Dadd( &tim_limhigh, time_new, &tim_maxdiff);
	time_Dsub( &tim_limlow, time_new, &tim_maxdiff);
	if ( time_Dcomp( time_old, &tim_limlow) == -1)
	  return TLOG__TIME_LT;
	if ( time_Dcomp( time_old, &tim_limhigh) == 1)
	  return TLOG__TIME_GT;

	return TLOG__TIME_EQ;

/*************
	tim_maxdiff.high = -1;
	tim_maxdiff.low = -maxdiff * 10000000;
	if ( maxdiff == 0)
	  tim_maxdiff.low = -1;
	  

	sts = lib$add_times( time_new, &tim_maxdiff, &tim_limhigh);

	sts = lib$sub_times( time_new, &tim_maxdiff, &tim_limlow);
	if ( sts == LIB$_NEGTIM)
	{
	  tim_limlow.high = -1;
	  tim_limlow.low = -1;
	}
	sts = lib$sub_times( time_old, &tim_limlow, &testtime);
	if ( sts == LIB$_NEGTIM)
	  return TLOG__TIME_LT;

	sts = lib$sub_times( &tim_limhigh, time_old, &testtime);
	if ( sts == LIB$_NEGTIM)
	  return TLOG__TIME_GT;

	return TLOG__TIME_EQ;
************/
}


/*************************************************************************
*
* Name:		tlog_print_line
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	
*
**************************************************************************/

static int	tlog_print_line( diff_ctx ctx, tlog_t_linelist *list, 
		int index, int old)
{
	pwr_tString132	str;

	list += index;
	if ( ctx->parallell)
	{
	  if ( ctx->attribute)
	  {
	    sprintf( str, "%d ", list->line_nr);
	    strncat( str, &list->line[TLOG_LINE_ATTRIBUTE], 
		TLOG_LINE_TEXT - TLOG_LINE_ATTRIBUTE);
	    str[TLOG_LINE_TEXT] = 0;
	  }
	  else if ( ctx->text)
	  {
	    sprintf( str, "%d ", list->line_nr);
	    strcat( str, &list->line[TLOG_LINE_TEXT]);
	  }
	  else if ( ctx->ttext)
	  {
	    sprintf( str, "%d %*.*s", list->line_nr, 
		TLOG_LINE_ATTRIBUTE, TLOG_LINE_ATTRIBUTE, list->line);
	    strcat( str, &list->line[TLOG_LINE_TEXT]);
	  }
	  else
	    sprintf( str, "%d %s", list->line_nr, list->line);
	  if ( old)
	  {
	    if ( ctx->outfile)
	      fprintf( ctx->outfile,
"					| %-38.38s\n", 
			str);	
	    else
	      printf( 
"					| %-38.38s\n", 
			str);
	  }
	  else
	  {
	    if ( ctx->outfile)
	      fprintf( ctx->outfile," %-38.38s |\n", str);	
	    else
	      printf( " %-38.38s |\n", str);	
	  }
	}
	else
	{
	  if ( ctx->outfile)
	    fprintf( ctx->outfile, "%d %s\n", list->line_nr, list->line);
	  else
	    printf( "%d %s\n", list->line_nr, list->line);
	}
	return TLOG__SUCCESS;
}

static int	tlog_print_linepar( diff_ctx ctx, tlog_t_linelist *newlist, 
		tlog_t_linelist	*oldlist,
		int 		newindex, 
		int 		oldindex)
{
	pwr_tString132	strnew;	
	pwr_tString132	strold;	

	newlist += newindex;
	oldlist += oldindex;
	if ( ctx->attribute)
	{
	  sprintf( strnew, "%d ", newlist->line_nr);
	  strncat( strnew, &newlist->line[TLOG_LINE_ATTRIBUTE], 
		TLOG_LINE_TEXT - TLOG_LINE_ATTRIBUTE);
	  strnew[TLOG_LINE_TEXT] = 0;
	  sprintf( strold, "%d ", oldlist->line_nr);
	  strncat( strold, &oldlist->line[TLOG_LINE_ATTRIBUTE], 
		TLOG_LINE_TEXT - TLOG_LINE_ATTRIBUTE);
	  strold[TLOG_LINE_TEXT] = 0;
	}
	else if ( ctx->text)
	{
	  sprintf( strnew, "%d ", newlist->line_nr);
	  strcat( strnew, &newlist->line[TLOG_LINE_TEXT]);
	  sprintf( strold, "%d ", oldlist->line_nr);
	  strcat( strold, &oldlist->line[TLOG_LINE_TEXT]);
	}
	else if ( ctx->ttext)
	{
	  sprintf( strnew, "%d %*.*s", newlist->line_nr, 
		TLOG_LINE_ATTRIBUTE, TLOG_LINE_ATTRIBUTE, newlist->line);
	  strcat( strnew, &newlist->line[TLOG_LINE_TEXT]);
	  sprintf( strold, "%d %*.*s", oldlist->line_nr, 
		TLOG_LINE_ATTRIBUTE, TLOG_LINE_ATTRIBUTE, oldlist->line);
	  strcat( strold, &oldlist->line[TLOG_LINE_TEXT]);
	}
	else
	{
	  sprintf( strnew, "%d %s", newlist->line_nr, newlist->line);
	  sprintf( strold, "%d %s", oldlist->line_nr, oldlist->line);
	}
	if ( ctx->outfile)
	  fprintf( ctx->outfile," %-38.38s | %-38.38s\n", strnew, strold); 
	else
	  printf( " %-38.38s | %-38.38s\n", strnew, strold); 

	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_lists_cmp_line
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Compare two lines
*
**************************************************************************/

static int	tlog_lists_cmp_line( diff_ctx ctx)
{
	tlog_t_linelist	*newlist_ptr;
	tlog_t_linelist	*oldlist_ptr;
	int	i;
	int	sts;
	int	found;

	newlist_ptr = ctx->new_list + ctx->current_new;

	/* Start with current old and search forward */
	found = 0;
	for ( i = ctx->current_old; i < ctx->old_list_count; i++)
	{
	  oldlist_ptr = ctx->old_list + i;
	  if ( oldlist_ptr->cmp_index == -1)
	  {
	    sts = tlog_checktime( &newlist_ptr->time, &oldlist_ptr->time,
		ctx->max_difftime);
	    if ( sts == TLOG__TIME_GT) break;
	    if ( sts == TLOG__TIME_EQ)
	    {
	      sts = tlog_line_compare( newlist_ptr, oldlist_ptr);
	      if ( ODD(sts))
	      {
	        found = 1;
	        break;
	      }
	    }
	  }
	}
	if ( !found && ctx->noorder)
	{
	  /* Search backwards */
	  for ( i = ctx->current_old - 1; i >= 0; i--)
	  {
	    oldlist_ptr = ctx->old_list + i;
	    if ( oldlist_ptr->cmp_index == -1)
	    {
	      sts = tlog_checktime( &newlist_ptr->time, &oldlist_ptr->time,
		ctx->max_difftime);
	      if ( sts == TLOG__TIME_LT) break;
	      if ( sts == TLOG__TIME_EQ)
	      {
	        sts = tlog_line_compare( newlist_ptr, oldlist_ptr);
	        if ( ODD(sts))
	        {
	          found = 1;
	          break;
	        }
	      }
	    }
	  }
	}
	else if ( !found && !ctx->exact)
	{
	  /* Search backwards as long as the time is the same*/
	  for ( i = ctx->current_old - 1; i >= 0; i--)
	  {
	    oldlist_ptr = ctx->old_list + i;
	    if ( oldlist_ptr->cmp_index == -1)
	    {
	      sts = tlog_checktime( &newlist_ptr->time, &oldlist_ptr->time,
		TLOG_TIME_EPSILON);
	      if ( sts == TLOG__TIME_LT) break;
	      sts = tlog_line_compare( newlist_ptr, oldlist_ptr);
	      if ( ODD(sts))
	      {
	        found = 1;
	        break;
	      }
	    }
	  }
	}
	if ( !found)
	  return TLOG__SUCCESS;
	
	/* Insert the index of the matching line */
	newlist_ptr->cmp_index = i;
	oldlist_ptr->cmp_index = ctx->current_new;
	
	ctx->current_old = i + 1;
	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_line_compare
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Compare two lines
*
**************************************************************************/

static int	tlog_line_compare( tlog_t_linelist *newlist_ptr, 
		tlog_t_linelist *oldlist_ptr)
{
	/* Compare the text 	*/
	if ( strcmp( &newlist_ptr->line[TLOG_LINE_TEXT], 
		&oldlist_ptr->line[TLOG_LINE_TEXT]))
	  return TLOG__DIFFTEXT;

	if ( strncmp( &newlist_ptr->line[TLOG_LINE_ATTRIBUTE], 
		&oldlist_ptr->line[TLOG_LINE_ATTRIBUTE],
		TLOG_LINE_TEXT - TLOG_LINE_ATTRIBUTE))
	  return TLOG__DIFFATTR;
	
	

	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_lists_cmp_linelist
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Print a lines in newlist.
*
**************************************************************************/

static void	tlog_lists_cmp_printparseline( diff_ctx ctx)
{
	if ( ctx->parallell)
	{
	  if ( ctx->outfile)
	    fprintf( ctx->outfile, 
"----------------------------------------|---------------------------------------\n");
	  else
	    printf( 
"----------------------------------------|---------------------------------------\n");
	}
	else
	{
	  if ( ctx->outfile)
	    fprintf( ctx->outfile, 
		"*********************************************************\n");
	  else
	    printf( 
		"*********************************************************\n");
	}
}

static int	tlog_lists_cmp_printnew( diff_ctx ctx, int index, int *lines)
{
	tlog_t_linelist	*newlist_ptr;
	int	i;

	tlog_lists_cmp_printparseline( ctx);
	if ( !ctx->parallell)
	{
	  if ( ctx->outfile)
	    fprintf( ctx->outfile, "--New----------------------------------------\n");
	  else
	    printf("--New----------------------------------------\n");
	}
	*lines = 0;
	for ( i = index; i < ctx->new_list_count; i++)
	{ 
	  newlist_ptr = ctx->new_list + i;
	  if ( newlist_ptr->cmp_index != -1) 
	    break;
	  tlog_print_line( ctx, ctx->new_list, i, 0);
	  (*lines)++;
	}
	ctx->diff_found++;
	return TLOG__SUCCESS;
}

static int	tlog_lists_cmp_printold( diff_ctx ctx, int index, int *lines)
{
	tlog_t_linelist	*oldlist_ptr;
	int	i;

	tlog_lists_cmp_printparseline( ctx);
	if ( !ctx->parallell)
	{
	  if ( ctx->outfile)
	    fprintf( ctx->outfile, "--Old----------------------------------------\n");
	  else
	    printf("--Old----------------------------------------\n");
	}
	*lines = 0;
	for ( i = index; i < ctx->old_list_count; i++)
	{ 
	  oldlist_ptr = ctx->old_list + i;
	  if ( oldlist_ptr->cmp_index != -1) 
	    break;
	  tlog_print_line( ctx, ctx->old_list, i, 1);
	  (*lines)++;
	}
	ctx->diff_found++;
	return TLOG__SUCCESS;
}

static int	tlog_lists_cmp_printboth( diff_ctx ctx, int newindex, int oldindex,
		int *newlines, int *oldlines)
{
	tlog_t_linelist	*newlist_ptr;
	tlog_t_linelist	*oldlist_ptr;
	int	i;
	int	print_new;
	int	print_old;

	if ( !ctx->parallell)
	{
	  tlog_lists_cmp_printparseline( ctx);
	  if ( ctx->outfile)
	    fprintf( ctx->outfile, "--New----------------------------------------\n");
	  else
	    printf("--New----------------------------------------\n");
	  *newlines = 0;
	  for ( i = newindex; i < ctx->new_list_count; i++)
	  { 
	    newlist_ptr = ctx->new_list + i;
	    if ( newlist_ptr->cmp_index != -1) 
	      break;
	    tlog_print_line( ctx, ctx->new_list, i, 0);
	    (*newlines)++;
	  }

	  if ( ctx->outfile)
	    fprintf( ctx->outfile, "--Old----------------------------------------\n");
	  else
	    printf("--Old----------------------------------------\n");
	  *oldlines = 0;
	  for ( i = oldindex; i < ctx->old_list_count; i++)
	  { 
	    oldlist_ptr = ctx->old_list + i;
	    if ( oldlist_ptr->cmp_index != -1) 
	      break;
	    tlog_print_line( ctx, ctx->old_list, i, 1);
	    (*oldlines)++;
	  }
	  ctx->diff_found++;
	}
	else
	{
	  *newlines = 0;
	  *oldlines = 0;
	  tlog_lists_cmp_printparseline( ctx);
	  print_old = 1;
	  print_new = 1;
	  for ( i = 0; ; i++)
	  { 
	    oldlist_ptr = ctx->old_list + oldindex + i;
	    newlist_ptr = ctx->new_list + newindex + i;
	    if ( print_old && oldindex + i < ctx->old_list_count &&
	         oldlist_ptr->cmp_index == -1)
	      print_old = 1;
	    else
	      print_old = 0;
	    if ( print_new && newindex + i < ctx->new_list_count &&
	         newlist_ptr->cmp_index == -1)
	      print_new = 1;
	    else
	      print_new = 0;
	    if ( print_new && print_old)
	    {
	      tlog_print_linepar( ctx, ctx->new_list, ctx->old_list, 
			newindex + i, oldindex + i);
	      (*oldlines)++;
	      (*newlines)++;
	    }
	    else if ( print_new)
	    {
	      tlog_print_line( ctx, ctx->new_list, newindex + i, 0);
	      (*newlines)++;
	    }
	    else if ( print_old)
	    {
	      tlog_print_line( ctx, ctx->old_list, oldindex + i, 1);
	      (*oldlines)++;
	    }
	    else
	      break;
	  }
	  ctx->diff_found++;
	}
	return TLOG__SUCCESS;
}


/*************************************************************************
*
* Name:		tlog_lists_cmp
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Compare to lists.
*
**************************************************************************/

static int	tlog_lists_cmp( 	diff_ctx	ctx)
{
	int	sts;
	int	i, j;
	tlog_t_linelist	*newlist_ptr;
	tlog_t_linelist	*oldlist_ptr;
	char		filename[80];
	int		lines, newlines, oldlines;
	int		next_new;

	for ( i = 0; i < ctx->new_list_count; i++)
	{
	  newlist_ptr = ctx->new_list + i;
	  newlist_ptr->cmp_index = -1;
	}
	for ( i = 0; i < ctx->old_list_count; i++)
	{
	  oldlist_ptr = ctx->old_list + i;
	  oldlist_ptr->cmp_index = -1;
	}

	for ( i = 0; i < ctx->new_list_count; i++)
	{
	  sts = tlog_lists_cmp_line( ctx);
	  ctx->current_new++;
	}

	if ( announce)
	{
	  printf( "Newlist\n");
	  for ( i = 0; i < ctx->new_list_count; i++)
	  {
	    newlist_ptr = ctx->new_list + i;
	    printf( "%40s %d\n", newlist_ptr->line, newlist_ptr->cmp_index);
	  }
	  printf( "Oldlist\n");
	  for ( i = 0; i < ctx->old_list_count; i++)
	  {
	    oldlist_ptr = ctx->old_list + i;
	    printf( "%40s %d\n", oldlist_ptr->line, oldlist_ptr->cmp_index);
	  }
	}

	/* Start Output */
	ctx->current_old = 0;
	for ( i = 0; i < ctx->new_list_count; i++)
	{
	  newlist_ptr = ctx->new_list + i;
	  if ( newlist_ptr->cmp_index == -1)
	  {
	    next_new = 0;
	    while( !next_new)
	    {
	      /* Check if anything in oldlist should be written */
	      for ( j = ctx->current_old; j < ctx->old_list_count; j++)
	      {
	        oldlist_ptr = ctx->old_list + j;
	        if ( oldlist_ptr->cmp_index == -1)
	        {
	          if ( j == 0 && i == 0)
	          {
	            next_new = 1;
	  	    tlog_lists_cmp_printboth( ctx, i, j, &newlines, &oldlines);
		    i += newlines - 1;
	            j += oldlines;
		    ctx->current_old = j;
	          }
	          else if ( i == 0)
	          {
	            /* Print newlist */
		    tlog_lists_cmp_printnew( ctx, i, &lines);
		    i += lines - 1;
	            next_new = 1;
	          }
	          else if ( j == 0)
	          {
	            /* Print oldlist */
		    tlog_lists_cmp_printold( ctx, j, &lines);
		    j += lines;
		    ctx->current_old = j;
	          }
	          else
	          {
	            /* Compare the index of the previous old */
	            oldlist_ptr--;
		    if ( oldlist_ptr->cmp_index < i - 1)
	            {
	              /* Print old */
		      tlog_lists_cmp_printold( ctx, j, &lines);
		      j += lines;
		      ctx->current_old = j;
	              next_new = 0;
	            }
	            else if ( oldlist_ptr->cmp_index > i - 1)
		    {
	              /* Print new */
	  	      tlog_lists_cmp_printnew( ctx, i, &lines);
		      i += lines - 1;
	              next_new = 1;
	            }
	            else
	            {
 	              /* Equal, print both */
	              next_new = 1;
	  	      tlog_lists_cmp_printboth( ctx, i, j, &newlines, &oldlines);
		      i += newlines - 1;
		      j += oldlines;
		      ctx->current_old = j;
	            }
	          }
	          break;
	        }
	      }

	      if ( j == ctx->old_list_count)
	      {
	  	tlog_lists_cmp_printnew( ctx, i, &lines);
		i += lines - 1;
	        next_new = 1;
	      }
	    }
	  }
	}

	for ( j = ctx->current_old; j < ctx->old_list_count; j++)
	{
	  oldlist_ptr = ctx->old_list + j;
	  if ( oldlist_ptr->cmp_index == -1)
	  {
	    if ( j == 0)
	    {
	      /* Print oldlist */
	      tlog_lists_cmp_printold( ctx, j, &lines);
	      ctx->current_old += lines;
	      j += lines;
	    }
	    else
	    {
	      /* Print old */
	      tlog_lists_cmp_printold( ctx, j, &lines);
	      ctx->current_old = j + lines;
	      j += lines;
	      next_new = 0;
	    }
	  }
	}

	if ( ctx->diff_found)
	{
	  tlog_lists_cmp_printparseline( ctx);
	  if ( ctx->outfile)
	    fprintf( ctx->outfile, "%d differences found\n", ctx->diff_found); 
	  else
	    printf( "%d differences found\n", ctx->diff_found); 
	}
	else
	{
	  if ( ctx->outfile)
	    fprintf( ctx->outfile, "No differences found\n", ctx->diff_found); 
	  else
	    printf( "No differences found\n", ctx->diff_found); 
	}
	return TLOG__SUCCESS;
}


/*************************************************************************
*
* Name:		tlog_read_line()
*
* Type		void
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Read a line in a file.
*
**************************************************************************/

static int tlog_read_line(	char	*line,
				int	maxsize,
				FILE	*file)
{ 
	char	*s;

	if (fgets( line, maxsize, file) == NULL)
	  return 0;
	s = strchr( line, 10);
	if ( s != 0)
	  *s = 0;

	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_line_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Allocate memory and insert a new line in the list.
*
**************************************************************************/

static pwr_tStatus	tlog_line_add(	pwr_tString132	*line,
				tlog_t_linelist	**list,
				int		*list_count,
				int		*alloc, 
				int		line_nr)
{
	tlog_t_linelist		*list_ptr;
	tlog_t_linelist		*new_list;
	char			time_str[20];
	char			type_str[10];
	int			sts;

	if ( *list_count == 0)
	{
	  *list = calloc( TLOG_LINEALLOC , sizeof(tlog_t_linelist));
	  if ( *list == 0)
	    return TLOG__NOMEMORY;
	  *alloc = TLOG_LINEALLOC;
	}
	else if ( *alloc <= *list_count)
	{
	  new_list = calloc( *alloc + TLOG_LINEALLOC, 
			sizeof(tlog_t_linelist));
	  if ( new_list == 0)
	    return TLOG__NOMEMORY;
	  memcpy( new_list, *list,
			*list_count * sizeof(tlog_t_linelist));
	  free( *list);
	  *list = new_list;
	  (*alloc) += TLOG_LINEALLOC;
	}
	list_ptr = *list + *list_count;

	/* Convert the time */
	strncpy( time_str, (char *) line, 11);
	time_str[11] = 0;
	sts = time_AsciiToD( time_str, &list_ptr->time);

	/* Get the type */
	strncpy( type_str, (char *)line + TLOG_LINE_TYPE, 2);
	type_str[2] = 0;
	if ( !strcmp( type_str, "Du"))
	  list_ptr->logtype = tlog_logtype_DtLogUp;
	else if ( !strcmp( type_str, "Dd"))
	  list_ptr->logtype = tlog_logtype_DtLogDown;
	else if ( !strcmp( type_str, "Du"))
	  list_ptr->logtype = tlog_logtype_DtLogUp;
	else if ( !strcmp( type_str, "Au"))
	  list_ptr->logtype = tlog_logtype_AtLogUp;
	else if ( !strcmp( type_str, "Ad"))
	  list_ptr->logtype = tlog_logtype_AtLogDown;
	else if ( !strcmp( type_str, "Mi"))
	  list_ptr->logtype = tlog_logtype_Info;
	else if ( !strcmp( type_str, "Ma"))
	  list_ptr->logtype = tlog_logtype_AlarmA;
	else if ( !strcmp( type_str, "Mb"))
	  list_ptr->logtype = tlog_logtype_AlarmB;
	else if ( !strcmp( type_str, "Mc"))
	  list_ptr->logtype = tlog_logtype_AlarmC;
	else if ( !strcmp( type_str, "Md"))
	  list_ptr->logtype = tlog_logtype_AlarmD;
	else if ( !strcmp( type_str, "Sl"))
	  list_ptr->logtype = tlog_logtype_Modify;
	else if ( !strcmp( type_str, "Fc"))
	  list_ptr->logtype = tlog_logtype_Close;
	else if ( !strcmp( type_str, "Ca"))
	  list_ptr->logtype = tlog_logtype_CAtLog;
	else if ( !strcmp( type_str, "Cd"))
	  list_ptr->logtype = tlog_logtype_CDtLog;
	else if ( !strcmp( type_str, "Co"))
	  list_ptr->logtype = tlog_logtype_COtLog;
	else if ( !strcmp( type_str, "Ex"))
	  list_ptr->logtype = tlog_logtype_ExecOn;

	list_ptr->line_nr = line_nr;
	strcpy( list_ptr->line, (char *) line);
	(*list_count)++;
	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_insert_file()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Read a file and insert it into a list.
*
**************************************************************************/

static pwr_tStatus	tlog_insert_file( 	char 		*filename,
					tlog_t_linelist **list,
					int		*list_count,
					char		*full_filename)
{
	int		sts;
	int		alloc = 0;
	FILE		*infile;
	pwr_tString132	line;
	int		line_nr;

	/* Open the infile */
	infile = fopen( filename, "r");
	if ( !infile )
	  return TLOG__FILEOPEN;

	line_nr = 0;
	while( 1)
	{
	  /* Read next line */
	  sts = tlog_read_line( line, sizeof(line), infile);
	  if ( EVEN(sts))
	     break;
	  line_nr++;

	  /* The two first line are comments 	*/
	  if ( line_nr > 2)
	  {
	    sts = tlog_line_add( (pwr_tString132 *) line, list, list_count, 
			&alloc, line_nr);
	    if ( EVEN(sts)) return sts;
	  }
	}
	fgetname( infile, full_filename);
	fclose( infile);
	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_qual_to_time()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Convert a commandline input time to pwr-time.
*
**************************************************************************/

static pwr_tStatus tlog_qual_to_time( 	char 		*in_str, 
					pwr_tTime 	*time)
{
	pwr_tStatus	sts;
	char		*s;
	pwr_tDeltaTime	one_day_time;
	pwr_tTime	current_time;
	char			str[64];
	char			timstr[64];


	if ( !strcmp( in_str, "") ||
	     !strncmp( in_str, "TODAY", strlen(in_str)))
	{
	  clock_gettime( CLOCK_REALTIME, &current_time);
	  time_AtoAscii( &current_time, time_eFormat_DateAndTime,
			timstr, sizeof(timstr));
	  timstr[12] = 0;
	  strcat( timstr, " 00:00:00.00");
	  sts = time_AsciiToA( timstr, time);
	}
	else if ( !strncmp( in_str, "YESTERDAY", strlen(in_str)))
	{
	  clock_gettime( CLOCK_REALTIME, &current_time);
	  time_AtoAscii( &current_time, time_eFormat_DateAndTime,
			timstr, sizeof(timstr));
	  timstr[12] = 0;
	  strcat( timstr, " 00:00:00.00");
	  sts = time_AsciiToA( timstr, &current_time);
	  strcpy( timstr, "1 00:00:00");
	  sts = time_AsciiToD( timstr, &one_day_time);
	  time_Dneg( &one_day_time, &one_day_time);
	  time_Aadd( time, &current_time, &one_day_time);
	}
	else
	{
	  strcpy( str, in_str);
	  if ( s = strchr( str, '-'))
	  {
	    /* Date is supplied, replace ':' to space */
	    if ( s = strchr( str, ':'))
	      *s = ' ';
	    strcpy( timstr, str);
	  }
	  else
	  {
	    /* No date is supplied, add current date as default */
	    clock_gettime( CLOCK_REALTIME, &current_time);
	    time_AtoAscii( &current_time, time_eFormat_DateAndTime,
			timstr, sizeof(timstr));
	    timstr[12] = 0;
 	    strcat( timstr, " ");
 	    strcat( timstr, str);
	  }
	  sts = time_AsciiToA( timstr, time);
	  if (EVEN(sts)) return sts;
	}
	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_diff()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Examine the differens between the last and an old tlogfile.
*
**************************************************************************/

pwr_tStatus	tlog_diff( 	char *filestr, 
				char *output, 
				char *timestr, 
				int parallell, 
				int attribute, 
				int text, 
				int ttext,
				int noorder, 
				int exact,
				char *since_str,
				char *before_str
)
{
	char		saved_filename[80];
	char		filename[80];
	FILE		*outfile;
	int		sts;
	tlog_t_linelist	*new_list;
	tlog_t_linelist	*old_list;
	int		new_list_count;
	int		old_list_count;
	float		time;
	int		nr;
	diff_ctx	ctx;
	char		modulestr[80];
	char		cmd[100];
	char		new_filename[80];
	char		old_filename[80];
	char		wild_filename[80];
	char		dev[80];
	char		dir[80];
	char		file[80];
	char		type[80];
	int		version;
	int		first;
	pwr_tTime	file_time;
	int		file_size;
	int		file_version;
	pwr_tTime	before_time;
	pwr_tTime	since_time;
	int		diff_count;
	unsigned long	search_ctx;

	tlog_get_defaultfilename( filestr, wild_filename, ".tlog", "pwrp_tlog:");

	diff_count = 0;
	first = 1;
	for(;;)
	{
	  if ( first)
	  {
	    /* Get the first file */
	    search_ctx = 0;
	    sts = dir_search_file( &search_ctx, wild_filename, filename);
	    if (EVEN(sts)) return sts;

	    /* At least one file is found */
	    if ( output != NULL)
	    {
	      /* Open the output file */
	      outfile = fopen( output, "w");
	      if ( !outfile )
	        return TLOG__FILEOPEN;
	    }
	    else
	      outfile = 0;

	    if ( timestr != NULL)
	    {
	      /* Convert to float */
	      nr = sscanf( timestr, "%f", &time);
	      if ( nr != 1)
	        return TLOG__TIMESYNTAX;
 	    }
	    else
	      time = 0.5;

	    if ( before_str != NULL)
	    {
	      sts = tlog_qual_to_time( before_str, &before_time);
	      if (EVEN(sts)) return TLOG__BEFOREQUAL;
	    }

	    if ( since_str != NULL)
	    {
	      sts = tlog_qual_to_time( since_str, &since_time);
	      if (EVEN(sts)) return TLOG__SINCEQUAL;
	    }

	    first = 0;
	  }
	  else
	  {
	    /* Get the next file */
	    sts = dir_search_file( &search_ctx, wild_filename, filename);
	    if (EVEN(sts)) break;

/*	    fprintf( outfile, ""); */
	  }

	  sts = dir_get_fileinfo( filename, (pwr_tTime *) &file_time, &file_size,
			&file_version, NULL);
	  if (EVEN(sts)) return sts;

	  if ( since_str != NULL)
	  {
	    if ( time_Acomp( &file_time, &since_time) < 0)
	      continue;
	  }
	  if ( before_str != NULL)
	  {
	    if ( time_Acomp( &file_time, &before_time) > 0)
	      continue;
	  }

	  sts = dir_parse_filename( filename, dev, dir, file, type, 
		&version);
	  strcpy( saved_filename, "pwrp_stlog:");
	  strcat( saved_filename, file);
	  strcat( saved_filename, type);

	  /* Read the new file 	*/
	  new_list_count = 0;
	  sts = tlog_insert_file( filename, &new_list, &new_list_count,
		new_filename);
	  if ( EVEN(sts))
	  {
	    if ( outfile)
	    {
	      fprintf( outfile, "\n\n\nNew file: %s\n", new_filename);
	      fprintf( outfile, "%%TLOG-E-NEWOPEN, Unable to open new file\n");
	    }
	    else
	    {
	      printf( "\n\n\nNew file: %s\n", new_filename);
	      printf( "%%TLOG-E-NEWOPEN, Unable to open new file\n");
	    }
	    continue;
	  }
	  if ( outfile)
	    fprintf( outfile, "\n\n\nNew file: %s\n", new_filename);
	  else
	    printf( "\n\n\nNew file: %s\n", new_filename);

	  /* Read the old file 	*/
	  old_list_count = 0;
	  sts = tlog_insert_file( saved_filename, &old_list, &old_list_count,
		old_filename);
	  if ( EVEN(sts)) 
	  {
	    if ( outfile)
	    {
	      fprintf( outfile, "Old file: %s\n", old_filename);
	      fprintf( outfile, "%%TLOG-E-OLDOPEN, Unable to open old file\n");
	    }
	    else
	    {
	      printf( "Old file: %s\n", old_filename);
	      printf( "%%TLOG-E-OLDOPEN, Unable to open old file\n");
	    }
	    free( new_list);
	    continue;
	  }
	  if ( outfile)
	    fprintf( outfile, "Old file: %s\n", old_filename);
	  else
	    printf( "Old file: %s\n", old_filename);

	  ctx = calloc( 1, sizeof( *ctx));

	  ctx->max_difftime = time;
	  ctx->new_list = new_list;
	  ctx->old_list = old_list;
	  ctx->old_list_count = old_list_count;
	  ctx->new_list_count = new_list_count;
	  ctx->outfile = outfile;
	  ctx->parallell = parallell;
	  ctx->attribute = attribute;
	  ctx->text = text;
	  ctx->ttext = ttext;
	  ctx->noorder = noorder;
	  ctx->exact = exact;
	  strcpy( ctx->new_filename, new_filename);
	  strcpy( ctx->old_filename, old_filename);

	  sts = tlog_lists_cmp( ctx);
	  if ( EVEN(sts)) return sts;

	  diff_count++;
	  free( new_list);
	  free( old_list);
	  free( ctx);
	}
	if ( outfile)
	  fclose( outfile);

        sts = dir_search_file_end( &search_ctx);

	if ( diff_count == 0)
	  if ( outfile)
	    fprintf( outfile, "No files found\n");
	  else
	    printf( "No files found\n");

	return TLOG__SUCCESS;
}



/*************************************************************************
*
* Name:		tlog_save
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Save the tlog file in library
*
**************************************************************************/

int	tlog_save( char *filestr)
{
	char		cmd[100];
	char		filename[80];
	int		sts;
	char		saved_filename[80];
	char		wild_filename[80];
	char		dev[80];
	char		dir[80];
	char		file[80];
	char		type[80];
	int		version;
	int		first;
	char		*s;
	unsigned long	search_ctx;

	tlog_get_defaultfilename( filestr, wild_filename, ".tlog", "pwrp_tlog:");

	first = 1;
	for(;;)
	{
	  if ( first)
	  {
	    /* Get the first file */
	    search_ctx = 0;
	    sts = dir_search_file( &search_ctx, wild_filename, filename);
	    if (EVEN(sts)) return sts;

	    first = 0;
	  }
	  else
	  {
	    /* Get the next file */
	    sts = dir_search_file( &search_ctx, wild_filename, filename);
	    if (EVEN(sts)) break;
	  }

	  sts = dir_parse_filename( filename, dev, dir, file, type, 
		&version);
	  strcpy( saved_filename, "pwrp_stlog:");
	  strcat( saved_filename, file);
	  strcat( saved_filename, type);

	  if ( s = strrchr( filename, ';'))
	    *s = 0;
	  sprintf( cmd, "copy/log %s %s", filename, saved_filename);
	  sts = system( cmd);
	  if (EVEN(sts)) return sts;
	}
	dir_search_file_end( &search_ctx);
	return TLOG__SUCCESS;
}

#ifdef TLOG_TEST
/*************************************************************************
*
* Name:		main()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Main program of rs_tlog_diff.
*
**************************************************************************/

main(int argc, char *argv[])
{
	char	filename[80];
	char	outputname[80];
	char	*output_ptr;
	char	timestr[80];
	char	*timestr_ptr;
	int	sts;
	int	parallell = 1;
	int	attribute = 0;
	int	text = 0;
	int	ttext = 0;
	int	order = 0;

	/* Filename in first argument */
	if ( argc >= 2 )
	{
	  strcpy( filename, argv[1]);
	  if ( argc >= 3 )
	  {
	    strcpy( timestr, argv[2]);
	    timestr_ptr = &timestr;
	    if ( argc >= 4 )
	    {
	      strcpy( outputname, argv[3]);
	      output_ptr = &outputname;
	    }
	    else
	      output_ptr = NULL;
	  }
	  else
	    timestr_ptr = NULL;
	}
	else
	  printf( "Usage: 'filename' 'output'\n");

	sts = tlog_diff( filename, output_ptr, timestr_ptr, parallell,
		attribute, text, ttext, order);
	exit(sts);

}
#endif
