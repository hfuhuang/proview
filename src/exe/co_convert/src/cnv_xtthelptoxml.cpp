/* 
 * Proview   $Id: cnv_xtthelptoxml.cpp,v 1.2 2005-09-01 14:57:47 claes Exp $
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

/* cnv_xtthelptoxml.cpp --
   Convert xtt help file to xml. */

/*_Include files_________________________________________________________*/

#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
#include "co_cdh.h"
#include "co_dcli.h"
}

#include "co_nav_help.h"
#include "cnv_ctx.h"
#include "cnv_readxtthelp.h"
#include "cnv_xtthelptoxml.h"

#define CNV_TAB 18

#define xml_cHead "<?xml version=\"1.0\" ?>"
#define xml_cCss  "<?xml-stylesheet type=\"text/css\" href=\"pxtthelp.css\" ?>"

typedef enum {
  xml_eTag_paragraph   	= 0,
  xml_eTag_bold		= 1,
  xml_eTag_link		= 2,
  xml_eTag_weblink	= 3,
  xml_eTag_h1		= 4,
  xml_eTag_h2		= 5,
  xml_eTag_h3		= 6,
  xml_eTag_topic	= 7,
  xml_eTag_image	= 8,
  xml_eTag_table	= 9,
  xml_eTag_t1		= 10,
  xml_eTag_t2		= 11,
  xml_eTag_t3		= 12,
  xml_eTag_xtthelp     	= 13,
  xml_eTag_break     	= 14,
  xml_eTag_hline     	= 15
} xml_eTag;

typedef struct {
  char start[40];
  char end[40];
} xml_sTag;

static xml_sTag tags[] = {
  {"<p>"	, "</p>"},
  {"<b>"	, "</b>"},
  {"<a>"       	, "</a>"},
  {"<a"       	, "</a>"},
  {"<h1>"	, "</h1>"},
  {"<h2>"	, "</h2>"},
  {"<h3>"	, "</h3>"},
  {"<topic>"	, "</topic>"},
  {"<img "	, " />"},
  {"<table>"	, "</table>"},
  {"<t1>"	, "</t1>"},
  {"<t2>"	, "</t2>"},
  {"<t3>"	, "</t3>"},
  {"<xtthelp>"	, "</xtthelp>"},
  {"<br/>"	, ""},
  {"<hr/>"	, ""}
};

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

void CnvXtthelpToXml::cnv_text( char *to, char *from)
{
  if ( !from) {
    strcpy( to, "");
    return;
  }

  char *t = to;
  char *s = from;

  for ( ; *s; s++) {
    switch ( *s) {
    case '<':
      *t++ = 'l';
      *t++ = 't';
      *t++ = ';';
      break;
    case '>':
      *t++ = 'g';
      *t++ = 't';
      *t++ = ';';
      break;
    case '&':
      *t++ = 'a';
      *t++ = 'm';
      *t++ = 'p';
      *t++ = ';';
      break;
    case '\'':
      *t++ = 'a';
      *t++ = 'p';
      *t++ = 'o';
      *t++ = 's';
      *t++ = ';';
      break;
    case '\"':
      *t++ = 'q';
      *t++ = 'u';
      *t++ = 'o';
      *t++ = 't';
      *t++ = ';';
      break;
    default:
      *t++ = *s;
    }
  }
  *t = 0;
}

void CnvXtthelpToXml::subject_to_fname( char *fname, char *subject, int path)
{
  char *s, *t;

  if ( path) {
    strcpy( fname, ctx->dir);
    strcat( fname, ctx->rx->name);
  }
  else
    strcpy( fname, ctx->rx->name);
  if ( !ctx->common_structfile_only) {
    strcat( fname, "_");
    t = fname + strlen(fname);
    for ( s = subject; *s; s++,t++) {
      if ( *s == ' ' || *s == '(' || *s == ')')
	*t = '_';
      else
	*t = *s;
    }
    *t = 0;
  }
  strcat( fname, ".xml");
  cdh_ToLower( fname, fname);
}

CnvXtthelpToXml::~CnvXtthelpToXml()
{
  if ( ctx->common_structfile_only && !first_topic) {
    if ( status & xml_mStatus_xtthelp) {
      fp <<
	tags[xml_eTag_xtthelp].end << endl;
      status &= ~xml_mStatus_xtthelp;
    }
    fp.close();
  }
}

void *CnvXtthelpToXml::insert( navh_eItemType item_type, char *t1,
			       char *t2, char *t3, char *link, 
			       char *link_bookmark, char *file_name,
			       navh_eHelpFile file_type, int help_index, 
			       char *bookmark)
{
  int i;
  static int in_table = 0;
  char text1[1000];
  char text2[1000];
  char text3[1000];

  cnv_text( text1, t1);
  cnv_text( text2, t2);
  cnv_text( text3, t3);

  if ( (t2 && strcmp(text2, "") != 0) || 
       (t3 && strcmp(text3, "") != 0) ) {
    if ( !status & xml_mStatus_table) {
      fp << 
tags[xml_eTag_table].start << endl;
      status |= xml_mStatus_table;
    }
  }
  else {
    if ( status & xml_mStatus_table) {
      // Close table (keep if empty line) 
      if ( !( t1 && strcmp( text1, "") == 0 && 
	      (item_type == navh_eItemType_Help || 
	       item_type == navh_eItemType_HelpCode || 
	       item_type == navh_eItemType_HelpBold))) {
        fp << 
tags[xml_eTag_table].end << endl;
        status &= ~xml_mStatus_table;
      }
    }
  }
  switch ( item_type) {
    case navh_eItemType_Topic:
    {
      pwr_tFileName fname;

      if ( first_topic || !ctx->common_structfile_only) {
	subject_to_fname( fname, text1, 1);
	fp.open( fname);
	fp << 
xml_cHead << endl << 
xml_cCss << endl <<
tags[xml_eTag_xtthelp].start << endl;

	status |= xml_mStatus_xtthelp;
	first_topic = 0;
      }
      fp <<
tags[xml_eTag_topic].start << endl;
      status |= xml_mStatus_topic;
      return NULL;
    } 
    case navh_eItemType_EndTopic:
    {
      if ( status & xml_mStatus_table) {
	fp <<
tags[xml_eTag_table].end << endl;
	status &= ~xml_mStatus_table;
      }
      if ( status & xml_mStatus_paragraph) {
	fp <<
tags[xml_eTag_paragraph].end << endl;
	status &= ~xml_mStatus_paragraph;
      }
      if ( status & xml_mStatus_topic) {
	fp <<
tags[xml_eTag_topic].end << endl;
	status &= ~xml_mStatus_topic;
      }
      if ( !ctx->common_structfile_only) {
	if ( status & xml_mStatus_xtthelp) {
	  fp <<
	    tags[xml_eTag_xtthelp].end << endl;
	  status &= ~xml_mStatus_xtthelp;
	}
	fp.close();
      }
      return NULL;
    }
    case navh_eItemType_Help:
    case navh_eItemType_HelpCode:
    {      

      if ( strcmp( link, "") != 0) {
        pwr_tFileName fname;

	if ( strncmp( link, "$web:", 5) == 0) {
	  if ( strncmp( &link[5], "$pwrp_web/", 10) == 0)
	    strcpy( fname, &link[15]);
	  else
	    strcpy( fname, &link[5]);
	} 
        else if ( (strstr( link, ".htm") != 0) || 
		  (strstr( link, ".pdf") != 0)) {
          strcpy( fname, link);
        }
        else {
          subject_to_fname( fname, link, 0);
        
          if ( strcmp( link_bookmark, "") != 0) {
	    strcat( fname, "#");
	    strcat( fname, link_bookmark);
          }
        }
        fp << tags[xml_eTag_link].start << " href=\"" <<  fname << "\">";
      }
      else if ( bookmark) {
        fp << tags[xml_eTag_link].start << " name=\"" << bookmark << "\">";
      }

      if ( ! in_table) {
        fp << text1;
        if ( strcmp( link, "") != 0 || bookmark)
          fp << tags[xml_eTag_break].start << tags[xml_eTag_link].end << endl;
        else
          fp << tags[xml_eTag_break].start << endl;
      }
      else {
	fp << "<TR><TD>" << text1;
        if ( strcmp( text2, "") != 0 || strcmp( text3, "") != 0) {
          for ( i = 0; i < (int)(CNV_TAB - strlen(text1)); i++)
            fp << "&nbsp;";
          fp << "&nbsp;&nbsp;</TD><TD>" << text2;
          if ( strcmp( text3, "") != 0) {
            for ( i = 0; i < (int)(CNV_TAB - strlen(text2)); i++)
              fp << "&nbsp;";
            fp << "&nbsp;&nbsp;</TD><TD>" << text3;
          }
        }
        fp << "</TD></TR>";
        if ( strcmp( link, "") != 0 || bookmark)
          fp << tags[xml_eTag_link].end << endl;
        else
          fp << endl;
      }
      return NULL;
    }
    case navh_eItemType_HelpBold:
    {
      pwr_tFileName fname;
      if ( strcmp( link, "") != 0) {
	if ( strncmp( link, "$web:", 5) == 0) {
	  if ( strncmp( &link[5], "$pwrp_web/", 10) == 0)
	    strcpy( fname, &link[15]);
	  else
	    strcpy( fname, &link[5]);
	} 
        else if ( (strstr( link, ".htm") != 0) || 
		  (strstr( link, ".pdf") != 0)) {
          strcpy( fname, link);
        }
        else {
          subject_to_fname( fname, link, 0);
          if ( strcmp( link_bookmark, "") != 0) {
	    strcat( fname, "#");
	    strcat( fname, link_bookmark);
          }
        }
        if ( !in_table)
          fp << tags[xml_eTag_link].start << " href=\"" <<  fname << "\">";
      }
      else if ( bookmark) {
	if ( !in_table)
          fp << tags[xml_eTag_link].start <<" name=\"" << bookmark << "\">";
      }

      if ( ! in_table) {
        fp << tags[xml_eTag_bold].start << text1 << tags[xml_eTag_bold].end; 
        if ( strcmp( link, "") != 0 || bookmark)
          fp<< tags[xml_eTag_break].start << tags[xml_eTag_link].end << endl;
        else
          fp<< tags[xml_eTag_break].start << endl;
      }
      else {
	fp << "<TR><TD><B>";
        if ( strcmp( link, "") != 0)
          fp << "<A HREF=\"" <<  fname << "\">";
        else if ( bookmark != 0)
          fp << "<A NAME=\"" <<  bookmark << "\">";
        fp << text1;
        if ( strcmp( link, "") != 0 || bookmark)
	  fp << "</A>";
        if ( strcmp( text2, "") != 0 || strcmp( text3, "") != 0) {
          for ( i = 0; i < (int)(CNV_TAB - strlen(text1)); i++)
            fp << "&nbsp;";
          fp << "&nbsp;&nbsp;</B></TD><TD><B>" << text2;
          if ( strcmp( text3, "") != 0) {
            for ( i = 0; i < (int)(CNV_TAB - strlen(text2)); i++)
              fp << "&nbsp;";
            fp << "&nbsp;&nbsp;</B></TD><TD><B>" << text3;
          }
        }
        fp << "</B></TD></TR>";
        if ( strcmp( link, "") != 0 || bookmark)
          fp << "</A>" << endl;
        else
          fp << endl;
      }
      return NULL;
    }
    case navh_eItemType_HelpHeader:
    {
      fp << tags[xml_eTag_h1].start << text1 << tags[xml_eTag_h1].end <<
	tags[xml_eTag_break].start << endl;
      return NULL;
    }
    case navh_eItemType_Header:
    {      
      fp << tags[xml_eTag_h3].start << text1 << tags[xml_eTag_h3].end <<
	tags[xml_eTag_break].start << endl;
      return NULL;
    }
    case navh_eItemType_HeaderLarge:
    {      
      fp << tags[xml_eTag_h2].start << text1 << tags[xml_eTag_h2].end <<
	tags[xml_eTag_break].start << endl;
      return NULL;
    }
    case navh_eItemType_HorizontalLine:
    {      
      fp << tags[xml_eTag_hline].start << endl;
      return NULL;
    }
    case navh_eItemType_Image:
    {      
      fp << tags[xml_eTag_image].start <<
	" src=\"" << text1 << "\"" << tags[xml_eTag_image].end << endl <<
      tags[xml_eTag_break].start << endl;
      return NULL;
    }
    default:
      return 0;
  }
}







