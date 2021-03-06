/** 
 * Proview   $Id: co_regex.h,v 1.3 2006-05-21 22:30:49 lw Exp $
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

#ifndef co_regex_h
#define co_regex_h

#include <stddef.h>

#if defined __cplusplus
extern "C" {
#endif
 
typedef struct regex {
  char *buffer;
  long allocated;
  long used;
  char *fastmap;
  char *translate;
  char fastmap_accurate;
  char can_be_null;
} regex_t;


typedef struct regmatch {
  int rm_so;
  int rm_eo;
  char *sp;
  char *ep;
} regmatch_t;

/* Compilation flags  */

# define REG_EXTENDED	0x0001	/* Use Extended Regular Expressions.	    */
# define REG_NOSUB	0x0002	/* Report only success/fail in regexec()    */
# define REG_ICASE	0x0004	/* Ignore case in match.		    */
# define REG_NEWLINE	0x0008	/* Eliminate any special significance of    */
				/* <newline>; treat it as a regular char.   */
# define REG_DELIM	0x0010	/* implementation-private		    */
# define REG_DEBUG	0x0020	/* implementation-private		    */
# define REG_ANCHOR	0x0040	/* implementation-private (for fgrep -x)    */
# define REG_WORDS	0x0080	/* implementation-private		    */
# define REG_MUST	0x0100	/* implementation-private		    */

/* Execution flags  */

# define REG_NOTBOL	0x0200	/* The first character of the string is not */
				/* the beginning of the line.		    */
# define REG_NOTEOL	0x0400	/* The last character of the string is not  */
				/* the end of the line.			    */
# define REG_NOOPT	0x0800	/* implementation-private		    */

/* regerror() flags  */

# define REG_MALLOC	0x80	/* Dynamically allocate storage for message */
				/* text.				    */

/* Return values  */

#define REG_OK		0 /* "success" */
#define REG_NOMATCH	1 /* "failed to match" */
#define REG_ECOLLATE	2 /* "invalid collation element" */
#define REG_EESCAPE	3 /* "trailing \ in pattern" */
#define REG_ENEWLINE	4 /* "newline found before end of pattern" */
#define REG_ENSUB	5 /* "more than 9 \( \) pairs" */
#define REG_ESUBREG	6 /* "number in \[0-9] invalid" */
#define REG_EBRACK	7 /* "[ ] inbalance or syntax error" */
#define REG_EPAREN	8 /* "( ) inbalance" */
#define REG_EBRACE	9 /* "{ } inbalance" */
#define REG_ERANGE	10 /* "invalid endpoint in range" */
#define REG_ESPACE	11 /* "out of memory" */
#define REG_BADRPT	12 /* "invalid  repetition" */
#define REG_ECTYPE	13 /* "invalid character class type" */
#define REG_BADPAT	14 /* "syntax error" */
#define REG_BADBR	15 /* "contents of { } invalid" */
#define REG_EFATAL	16 /* "internal error" */
#define REG__LAST	17 /* "unknown regex error" */

/* Functions  */

int regcomp(regex_t *preg, char *pattern, int cflags);

int regexec(regex_t *preg, char *string, size_t nmatch,
  regmatch_t pmatch[], int eflags);

void regfree(regex_t *preg);

char *regerror(int errcode);

#if defined __cplusplus
}
#endif

# endif
