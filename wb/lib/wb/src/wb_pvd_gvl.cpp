/* 
 * Proview   $Id: wb_pvd_gvl.cpp,v 1.1 2005-09-20 13:14:28 claes Exp $
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

#include <vector.h>
#include <string.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "wb_vext.h"
#include "wb_pvd_gvl.h"
#include "wb_ldh.h"
#include "wb_ldh_msg.h"

extern "C" {
#include "co_cdh.h"
#include "co_dcli.h"
}

void wb_pvd_gvl::save( pwr_tStatus *sts)
{
  ofstream of;
  pwr_tFileName fname;
  
  *sts = LDH__SUCCESS;
  dcli_translate_filename( fname, "$pwra_db/pwr_volumelist.tst");
  of.open( fname);
  if ( !of) {
    *sts = LDH__FILEOPEN;
    return;
  }

  for ( int oix = m_list[0].fchoix; oix; oix = m_list[oix].fwsoix)
    save_item( oix, of);

  of.close();
}

void wb_pvd_gvl::save_item( pwr_tOix oix, ofstream& of)
{
  switch ( m_list[oix].cid) {
  case pwr_eClass_PlantHier: {
    pwr_sPlantHier *body = (pwr_sPlantHier *)m_list[oix].body;
    of << "!**Menu " << m_list[oix].name << " { // "  << body->Description << endl;

    for ( int ix = m_list[oix].fchoix; ix; ix = m_list[ix].fwsoix)
      save_item( ix, of);
    of << "!**}" << endl;
    break;
  }
  case pwr_cClass_VolumeReg: {
    pwr_sClass_VolumeReg *body = (pwr_sClass_VolumeReg *)m_list[oix].body;

    of << "	" << m_list[oix].name << "	" << 
      cdh_VolumeIdToString( 0, body->VolumeId, 0, 0) << "	" << body->Project << 
      endl;

    for ( int ix = m_list[oix].fchoix; ix; ix = m_list[ix].fwsoix)
      save_item( ix, of);
    break;
  }
  default: ;
  }
}

void wb_pvd_gvl::load( pwr_tStatus *rsts)
{
  char line[200];
  char line_item[6][80];
  int num;
  ifstream is;
  pwr_tFileName fname;
  int line_cnt = 0;
  pwr_tStatus sts;
  int menu_stack[100];
  int menu_cnt = 0;
  char description[80];

  *rsts = LDH__SUCCESS;

  dcli_translate_filename( fname, "$pwra_db/pwr_volumelist.tst");
  is.open( fname);
  if ( !is) {
    *rsts = LDH__NEWFILE;
    return;
  }

  // Create Root object
  pitem rootitem;
  strcpy( rootitem.name, "GlobalVolumeList");
  rootitem.cid = pwr_eClass_PlantHier;
  rootitem.oix = 0; 
  m_list.push_back(rootitem);
  menu_stack[menu_cnt] = rootitem.oix;
  menu_cnt++;


  while ( is.getline( line, sizeof(line))) {
    line_cnt++;
    if ( line[0] == '!') {
      if ( strncmp( line, "!**Menu", 7) == 0) {
	// Add PlantHier
	char *s = strstr( line, "// ");
	if ( s) {
	  strncpy( description, s+3, sizeof(description));
	  description[sizeof(description)-1] = 0;
	}
	else
	  strcpy( description, "");

	num = dcli_parse( line, " 	", "", (char *)line_item,
		     sizeof(line_item)/sizeof(line_item[0]),
		     sizeof(line_item[0]), 0);
	if ( num < 3) {
	  cout << "Syntax error " << fname << " row " << line_cnt << endl;
	  continue;
	}

	pitem plantitem;
	strcpy( plantitem.name, line_item[1]);
	plantitem.cid = pwr_eClass_PlantHier;
	plantitem.oix = next_oix++;
	plantitem.fthoix = menu_stack[menu_cnt - 1];
	plantitem.bwsoix = m_list[plantitem.fthoix].lchoix;
	plantitem.fwsoix = 0;
	plantitem.body_size = sizeof(pwr_sPlantHier);
	pwr_sPlantHier *plantbody = (pwr_sPlantHier *) calloc( 1, plantitem.body_size); 
	plantitem.body = plantbody;
	strcpy( plantbody->Description, description);

	m_list.push_back(plantitem);

	if ( plantitem.bwsoix != 0)
	  m_list[plantitem.bwsoix].fwsoix = plantitem.oix;
	m_list[plantitem.fthoix].lchoix = plantitem.oix;
	if ( m_list[plantitem.fthoix].fchoix == 0)
	  m_list[plantitem.fthoix].fchoix = plantitem.oix;

	menu_stack[menu_cnt] = plantitem.oix;
	menu_cnt++;
      }
      else if ( strncmp( line, "!**}", 4) == 0) {
	if ( menu_cnt == 0) {
	  cout << "Syntax error " << fname << " row " << line_cnt << endl;
	  continue;
	}
	menu_cnt--;
      }
      continue;
    }
    dcli_trim( line, line);

    num = dcli_parse( line, " 	", "", (char *)line_item,
		     sizeof(line_item)/sizeof(line_item[0]),
		     sizeof(line_item[0]), 0);
    if ( num != 3) {
      cout << "Syntax error " << fname << " row " << line_cnt << endl;
      continue;
    }

    pitem volitem;
    strcpy( volitem.name, line_item[0]);
    
    volitem.body_size = sizeof(pwr_sClass_VolumeReg);
    pwr_sClass_VolumeReg *volbody = 
      (pwr_sClass_VolumeReg *) calloc( 1, volitem.body_size);

    volitem.body = volbody;
    strcpy( volbody->Project, line_item[2]);
    sts = cdh_StringToVolumeId( line_item[1], &volbody->VolumeId);
    if ( EVEN(sts)) {
      cout << "Syntax error " << fname << " row " << line_cnt << endl;
      continue;
    }

    volitem.cid = pwr_cClass_VolumeReg;
    volitem.oix = next_oix++;
    volitem.fthoix = menu_stack[menu_cnt - 1];
    volitem.bwsoix = m_list[volitem.fthoix].lchoix;
    volitem.fwsoix = 0;
    strcpy( volbody->Description, line_item[1]);
    for ( int i = strlen(volbody->Description); i < 18; i++)
      strcat( volbody->Description, " ");
    strcat( volbody->Description, line_item[2]);

    m_list.push_back(volitem);

    if ( volitem.bwsoix != 0)
      m_list[volitem.bwsoix].fwsoix = volitem.oix;
    m_list[volitem.fthoix].lchoix = volitem.oix;
    if ( m_list[volitem.fthoix].fchoix == 0)
      m_list[volitem.fthoix].fchoix = volitem.oix;

  }
}


