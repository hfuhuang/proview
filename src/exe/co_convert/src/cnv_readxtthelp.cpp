/* 
 * Proview   $Id: cnv_readxtthelp.cpp,v 1.3 2008-10-31 12:51:30 claes Exp $
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

/* cnv_readxtthelp.cpp --
   Convert xtt help file to html. */

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
#include "cnv_readxtthelp.h"
#include "cnv_xtthelpto.h"

using namespace std;

#define CNV_TAB 18

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

CnvReadXtthelp::CnvReadXtthelp( char *x_name, char *x_directory,
				CnvXtthelpTo *to) :
  xtthelpto(to)
{
  char *s;

  strcpy( filename, x_name);
  if ( (s = strrchr(x_name,'/')))
    strcpy( name, s+1);
  else
    strcpy( name, x_name);
  if ((s = strrchr( name, '.')))
    *s = 0;
  strcpy( directory, x_directory);

}

void *help_insert_cb( void *ctx, navh_eItemType item_type, const char *text1,
		      const char *text2, const char *text3, const char *link, 
		      const char *link_bookmark, const char *file_name,
		      navh_eHelpFile file_type, int help_index, 
		      const char *bookmark)
{
  CnvReadXtthelp *xh = (CnvReadXtthelp *)ctx;

  return xh->xtthelpto->insert( item_type, text1, text2, text3, link, 
		     link_bookmark, file_name, file_type, help_index, 
		     bookmark);
}

int CnvReadXtthelp::read_xtthelp()
{
  int sts;
  void *bookmark_node;

  if ( xtthelpto->confpass()) {
    xtthelpto->set_confpass( true);
    NavHelp *navhelp = new NavHelp( (void *)this, "$pwr_exe/wtt_help.dat",
				  "$pwrp_exe/xtt_help.dat");
    navhelp->insert_cb = help_insert_cb;
    navhelp->set_propagate(0);  			// Don't print include files
    
    sts = navhelp->help( NULL, "", navh_eHelpFile_Other, 
			 filename, &bookmark_node);
    if ( EVEN(sts)) return sts;
    delete navhelp;
    xtthelpto->set_confpass( false);
  }
  NavHelp *navhelp = new NavHelp( (void *)this, "$pwr_exe/wtt_help.dat",
				  "$pwrp_exe/xtt_help.dat");
  navhelp->insert_cb = help_insert_cb;
  navhelp->set_propagate(0);  			// Don't print include files
  sts = navhelp->help( NULL, "", navh_eHelpFile_Other, 
		       filename, &bookmark_node);
  if ( EVEN(sts)) return sts;
  delete navhelp;

  return 1;
}




