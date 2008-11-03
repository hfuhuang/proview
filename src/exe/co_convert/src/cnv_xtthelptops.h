/* 
 * Proview   $Id: cnv_xtthelptops.h,v 1.5 2008-11-03 09:50:24 claes Exp $
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

#ifndef cnv_xtthelptops_h
#define cnv_xtthelptops_h

using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include "pwr.h"
#include "cnv_xtthelpto.h"
#include "cnv_style.h"
#include "cnv_content.h"
#include "cnv_tops.h"

class CnvCtx;

typedef enum {
  ps_mStatus_xtthelp 	= 1 << 0,
  ps_mStatus_paragraph 	= 1 << 1,
  ps_mStatus_table 	= 1 << 2,
  ps_mStatus_topic 	= 1 << 3
} ps_mStatus;

typedef enum {
  ps_mOption_printDisable	= 1 << 0
} ps_mOption;

class CnvXtthelpToPs : public CnvXtthelpTo {
 public:
  CnvXtthelpToPs( CnvCtx *cnv_ctx) :
    ctx(cnv_ctx), base_ci(0), first_topic(1), user_style(0), status(0),
    conf_pass(false), option(0)
    { 
      strcpy( current_subject, "");
    }
  virtual ~CnvXtthelpToPs();

  Cnv_eXtthelpToType type() { return Cnv_eXtthelpToType_Ps;}
  void *insert( navh_eItemType item_type, const char *text1,
		const char *text2, const char *text3, const char *link, 
		const char *link_bookmark, const char *file_name,
		navh_eHelpFile file_type, int help_index, 
		const char *bookmark);
  void subject_to_fname( char *fname, const char *subject, int path);
  bool confpass() { return true;}
  void set_confpass( bool conf) { 
    tops.set_confpass( conf);
    conf_pass = conf;
    if ( !conf) {
      // Reset
      base_ci = 0;
      user_style = 0;
      first_topic = 1;
      status = 0;    
    }
  }

  CnvCtx *ctx;
  CnvToPs tops;
  int base_ci;
  int first_topic;
  int first_chaptertopic;
  int user_style;
  unsigned int status;
  bool conf_pass;
  char current_subject[80];
  unsigned int option;
};

#endif









