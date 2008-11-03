/* 
 * Proview   $Id: cnv_xtthelptops.cpp,v 1.5 2008-11-03 09:50:24 claes Exp $
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

/* cnv_xtthelptops.cpp --
   Convert xtt help file to postscript. */

/*_Include files_________________________________________________________*/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
#include "co_cdh.h"
#include "co_dcli.h"
}

#include "co_nav_help.h"
#include "co_lng.h"
#include "cnv_ctx.h"
#include "cnv_readxtthelp.h"
#include "cnv_xtthelptops.h"
#include "cnv_image.h"

#define ps_cCellSize 110
/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

void CnvXtthelpToPs::subject_to_fname( char *fname, const char *subject, int path)
{

  if ( path) {
    strcpy( fname, ctx->dir);
    strcat( fname, ctx->rx->name);
  }
  else
    strcpy( fname, ctx->rx->name);
  strcat( fname, ".ps");
  cdh_ToLower( fname, fname);
}

CnvXtthelpToPs::~CnvXtthelpToPs()
{
  if ( !first_topic) {
    if ( status & ps_mStatus_xtthelp) {
      status &= ~ps_mStatus_xtthelp;
    }
  }
  tops.close();
}

void *CnvXtthelpToPs::insert( navh_eItemType item_type, const char *text1,
			      const char *text2, const char *text3, const char *link, 
			      const char *link_bookmark, const char *file_name,
			      navh_eHelpFile file_type, int help_index, 
			      const char *bookmark)
{
  if ( option & ps_mOption_printDisable &&
       item_type != navh_eItemType_Option)
    return NULL;


  if ( (text2 && strcmp(text2, "") != 0) || 
       (text3 && strcmp(text3, "") != 0) ) {
    if ( !(status & ps_mStatus_table))
      status |= ps_mStatus_table;
  }
  else {
    if ( status & ps_mStatus_table) {
      // Close table (keep if empty line) 
      if ( !( text1 && strcmp( text1, "") == 0 && 
	      (item_type == navh_eItemType_Help || 
	       item_type == navh_eItemType_HelpCode || 
	       item_type == navh_eItemType_HelpBold)))
        status &= ~ps_mStatus_table;
    }
  }
  switch ( item_type) {
    case navh_eItemType_DocTitlePage:
    case navh_eItemType_DocInfoPage:
    case navh_eItemType_Topic:
    {
      strcpy( current_subject, text1);

      if ( item_type == navh_eItemType_DocTitlePage) {
	tops.set_cf( ps_eFile_Info);
	tops.set_ci( ps_eId_TitlePage);
      }
      else if ( item_type == navh_eItemType_DocInfoPage) {
	tops.set_cf( ps_eFile_Info);
	tops.set_ci( ps_eId_InfoPage);
	tops.print_pagebreak( 0);
      }
      else {
	if ( tops.ci == ps_eId_Chapter) {
	  if ( !first_chaptertopic)
	    tops.set_ci( ps_eId_TopicL1);
	  else
	    first_chaptertopic = 0;
	}
	if ( tops.cf == ps_eFile_Info)
	  tops.y = ps_cPageHeight - ps_cTopMargin;
	tops.set_cf( ps_eFile_Body);
      }

      if ( first_topic) {
	pwr_tFileName fname;

	subject_to_fname( fname, text1, 1);
	tops.set_filename( ps_eFile_Info, fname);
	tops.set_filename( ps_eFile_Body, ps_cTmpFile);
	tops.open();

	status |= ps_mStatus_xtthelp;
	first_topic = 0;
      }
      status |= ps_mStatus_topic;
      return NULL;
    } 
    case navh_eItemType_EndTopic: {
      if ( status & ps_mStatus_table)
	status &= ~ps_mStatus_table;
      if ( status & ps_mStatus_paragraph)
	status &= ~ps_mStatus_paragraph;
      if ( status & ps_mStatus_topic)
	status &= ~ps_mStatus_topic;
      if ( user_style) {
	user_style = 0;
	tops.set_ci( base_ci);
      }
      return NULL;
    }
    case navh_eItemType_Style: {
      if ( cdh_NoCaseStrcmp( text1, "function") == 0) {
	base_ci = tops.ci;
	tops.set_ci( ps_eId_Function);
	user_style = 1;
      }
      return NULL;
    }
    case navh_eItemType_EndChapter: {
      if ( status & ps_mStatus_table)
	status &= ~ps_mStatus_table;
      if ( status & ps_mStatus_paragraph)
	status &= ~ps_mStatus_paragraph;
      if ( status & ps_mStatus_topic)
	status &= ~ps_mStatus_topic;

      tops.set_ci( ps_eId_TopicL1);
      user_style = 0;
      return NULL;
    }
    case navh_eItemType_Chapter: {
      if ( status & ps_mStatus_table)
	status &= ~ps_mStatus_table;
      if ( status & ps_mStatus_paragraph)
	status &= ~ps_mStatus_paragraph;
      if ( status & ps_mStatus_topic)
	status &= ~ps_mStatus_topic;

      tops.set_ci( ps_eId_Chapter);
      first_chaptertopic = 1;
      user_style = 0;

      tops.reset_headernumbers( 1);
      return NULL;
    }
    case navh_eItemType_HeaderLevel: {
      if ( user_style) {
	user_style = 0;
	tops.set_ci( base_ci);
      }
      tops.incr_headerlevel();
      return NULL;
    }
    case navh_eItemType_EndHeaderLevel: {
      if ( user_style) {
	user_style = 0;
	tops.set_ci( base_ci);
      }
      tops.decr_headerlevel();
      return NULL;
    }
    case navh_eItemType_PageBreak: {
      tops.print_pagebreak(1);
      return NULL;
    }
    case navh_eItemType_Help:
    case navh_eItemType_HelpCode:
    case navh_eItemType_HelpBold:
    {      
      int printmode;
      CnvStyle *hstyle;

      if ( item_type == navh_eItemType_Help)
	hstyle = &tops.style[tops.ci].text;
      else if ( item_type == navh_eItemType_HelpBold)
	hstyle = &tops.style[tops.ci].boldtext;
      else if ( item_type == navh_eItemType_HelpCode)
	hstyle = &tops.style[tops.ci].code;

      if ( strcmp( link, "") != 0)
	printmode = ps_mPrintMode_Start;
      else
	printmode = ps_mPrintMode_Pos;
      if ( !(status & ps_mStatus_table)) {
	tops.x = ps_cLeftMargin;
	tops.print_text( text1, *hstyle, printmode);
      }
      else {
	tops.x = ps_cLeftMargin;
	tops.print_text( text1, *hstyle);
        if ( text2 && strcmp( text2, "") != 0) {
	  tops.x = ps_cLeftMargin + ps_cCellSize;
	  tops.print_text( text2, *hstyle, ps_mPrintMode_KeepY | ps_mPrintMode_FixX);
	}
	if ( text3 && strcmp( text3, "") != 0) {
	  tops.x = ps_cLeftMargin + 2 * ps_cCellSize;
	  tops.print_text( text3, *hstyle, ps_mPrintMode_KeepY | ps_mPrintMode_FixX);
	}
      }
      if ( strcmp( link, "") != 0 && !conf_pass) {
        pwr_tFileName fname;
	char str[200];
	int page;

	if ( strncmp( link, "$web:", 5) == 0) {
	  if ( strncmp( &link[5], "$pwrp_web/", 10) == 0)
	    strcpy( fname, &link[15]);
	  else
	    strcpy( fname, &link[5]);
	  sprintf( str, " (%s %s)", Lng::translate("See"), fname);
	  if ( !(status & ps_mStatus_table))
	    tops.print_text( str, tops.style[tops.ci].link, 
			     ps_mPrintMode_End | ps_mPrintMode_FixX);	  
	  else {
	    tops.x = ps_cLeftMargin + 3 * ps_cCellSize;
	    if ( ps_cLeftMargin + 2 * ps_cCellSize + strlen(text3) * hstyle->font_size * 0.65 > tops.x)
	      tops.x = ps_cLeftMargin + 2 * ps_cCellSize + strlen(text3) * hstyle->font_size * 0.65;
	    tops.print_text( str, tops.style[tops.ci].link, 
			     ps_mPrintMode_End | ps_mPrintMode_FixX);	  
	  }
	} 
        else if ( (strstr( link, ".htm") != 0) || 
		  (strstr( link, ".pdf") != 0)) {
          strcpy( fname, link);
	  sprintf( str, " (%s %s)", Lng::translate("See"), fname);
	  if ( !(status & ps_mStatus_table))
	    tops.print_text( str, tops.style[tops.ci].link, 
			     ps_mPrintMode_End | ps_mPrintMode_FixX);	  
	  else {
	    tops.x = ps_cLeftMargin + 3 * ps_cCellSize;
	    if ( ps_cLeftMargin + 2 * ps_cCellSize + strlen(text3) * hstyle->font_size * 0.65 > tops.x)
	      tops.x = ps_cLeftMargin + 2 * ps_cCellSize + strlen(text3) * hstyle->font_size * 0.65;
	    tops.print_text( str, tops.style[tops.ci].link, 
			     ps_mPrintMode_End | ps_mPrintMode_FixX);	  
	  }
        }
        else {
	  char text[80];
	  int sts = tops.content.find_link( link, text, &page);
	  if ( ODD(sts)) {
 	    sprintf( str, " (%s %s ", Lng::translate("See"), text);
	    sprintf( &str[strlen(str)], "%s %d)", Lng::translate("page"), page);
	    if ( !(status & ps_mStatus_table))
	      tops.print_text( str, tops.style[tops.ci].link, 
			       ps_mPrintMode_End | ps_mPrintMode_FixX);	  
	    else {
#if 0
	      if ( !(text3 && strcmp(text3, "") != 0)) {
		tops.x = ps_cLeftMargin + 2 * ps_cCellSize;
		if ( ps_cLeftMargin + ps_cCellSize + strlen(text2) * hstyle->font_size * 0.5 > tops.x)
		  tops.x = ps_cLeftMargin + ps_cCellSize + strlen(text2) * hstyle->font_size * 0.5;
	      }
	      else {
		tops.x = ps_cLeftMargin + 3 * ps_cCellSize;
		if ( ps_cLeftMargin + ps_cCellSize + strlen(text3) * hstyle->font_size * 0.5 > tops.x)
		  tops.x = ps_cLeftMargin + 2 * ps_cCellSize + strlen(text3) * hstyle->font_size * 0.5;
	      }
	      tops.print_text( str, tops.style[tops.ci].link, 
			       ps_mPrintMode_KeepY | ps_mPrintMode_FixX);
#endif
	    }
	  }
        }
      }
      else if ( bookmark) {
        // fp[cf] << tags[ps_eTag_link].start << " name=\"" << bookmark << "\">";
      }

      return NULL;
    }
    case navh_eItemType_HelpHeader:
    {
      int hlevel;

      if ( !user_style)
	hlevel = tops.ci - (int) ps_eId_Chapter;
      else
	hlevel = base_ci - (int) ps_eId_Chapter;
      tops.print_h1( text1, hlevel, current_subject);
      return NULL;
    }
    case navh_eItemType_Header:
    {      
      tops.print_h3( text1);
      return NULL;
    }
    case navh_eItemType_HeaderLarge:
    {      
      tops.print_h2( text1);
      return NULL;
    }
    case navh_eItemType_HorizontalLine:
    {      
      tops.print_horizontal_line();
      return NULL;
    }
    case navh_eItemType_Image:
    {      
      int sts = tops.print_image( text1);
      if ( EVEN(sts))
	printf( "Image: %s not found\n", text1);
      return NULL;
    }
    case navh_eItemType_Option:
    {      
      if ( strcmp( text1, "printdisable") == 0)
	option |= ps_mOption_printDisable;
      else if ( strcmp( text1, "printenable") == 0)
	option &= ~ps_mOption_printDisable;
      return NULL;
    }
    default:
      return 0;
  }
  return 0;
}







