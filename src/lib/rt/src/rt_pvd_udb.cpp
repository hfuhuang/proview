/* 
 * Proview   $Id: rt_pvd_udb.cpp,v 1.2 2008-06-24 07:15:31 claes Exp $
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
#include "rt_pvd_udb.h"
#include "wb_ldh.h"
#include "wb_ldh_msg.h"

extern "C" {
#include "co_cdh.h"
#include "co_dcli.h"
}

void rt_pvd_udb::save( pwr_tStatus *sts)
{
  gu->clear();

  for ( int i = 1; i < (int)m_list.size(); i++) {
    if ( m_list[i].flags & procom_obj_mFlags_Deleted)
      continue;

    switch ( m_list[i].cid) {
    case pwr_cClass_SystemGroupReg: {
      pwr_sClass_SystemGroupReg *body = (pwr_sClass_SystemGroupReg *)m_list[i].body;

      *sts = gu->add_system( groupname(longname( m_list[i].oix)), body->Attributes, body->Description, m_list[i].oix-1);
      if ( EVEN(*sts)) return;
      break;
    }
    case pwr_cClass_UserReg: {
      pwr_sClass_UserReg *body = (pwr_sClass_UserReg *)m_list[i].body;
      char gname[120];
      char *s;

      strcpy( gname, longname( m_list[i].oix)); 
      if ( (s = strrchr( gname, '-')))
	*s = 0;
	   
      *sts = gu->add_user( groupname(gname), m_list[i].name, body->Password, 
			   body->Privileges, body->FullName, body->Description, body->Email,
			   body->Phone, body->Sms, m_list[i].oix-1);
      if ( EVEN(*sts)) return;
      break;
    }
    default: ;
    }
  }
  *sts = gu->save();
}

char *rt_pvd_udb::groupname( char *name)
{
  static char str[200];
  char *s, *t;

  for ( s = name, t = str; *s; s++,t++) {
    if ( *s == '-')
      *t = '.';
    else
      *t = *s;
  }
  *t = 0;
  return str;
}

void rt_pvd_udb::load( pwr_tStatus *rsts)
{
  char		filename[256];
  int		sts;

  if ( gu)
    gu->clear();
  else
    gu = new GeUser();
  sts = dcli_get_defaultfilename( user_cFilename, filename, "");
  gu->load( filename);
 
  // Create Root object

  procom_obj rootitem;
  if ( m_env == pvd_eEnv_Wb) {
    strcpy( rootitem.name, "UserDatabase");
    rootitem.cid = pwr_eClass_PlantHier;
    rootitem.oix = 0; 
  }
  else {
    strcpy( rootitem.name, "VolUserDatabase");
    rootitem.cid = pwr_eClass_ExternVolume;
    rootitem.oix = 0; 
    rootitem.body_size = sizeof(pwr_sExternVolume);
    rootitem.body = calloc( 1, rootitem.body_size);
  }
  m_list.push_back(rootitem);
  menu_stack[menu_cnt] = rootitem.oix;
  menu_cnt++;

  SystemList *systemgroup = gu->root_system();
  while ( systemgroup) {
    load_systemgroup( systemgroup);

    systemgroup = systemgroup->next_system();
  }

  if ( m_env == pvd_eEnv_Rt) {
    // Convert to Rt style
    for ( int i = 1; i < (int)m_list.size(); i++) {
      if ( m_list[i].bwsoix == 0)
	m_list[i].bwsoix = m_list[m_list[i].fthoix].lchoix;
      if ( m_list[i].fwsoix == 0)
	m_list[i].fwsoix = m_list[m_list[i].fthoix].fchoix;
    }  
  }
  
}

void rt_pvd_udb::load_systemgroup( SystemList *systemgroup)
{
  procom_obj item;
  pwr_sClass_SystemGroupReg *body;
  char sname[120];
  char *s;

  body = (pwr_sClass_SystemGroupReg *) calloc( 1, sizeof(pwr_sClass_SystemGroupReg));
  item.body = body;
  gu->get_system_name( systemgroup, sname);
  if (( s = strrchr( sname, '.')))
    strcpy( item.name, s+1);
  else
    strcpy( item.name, sname);

  gu->get_system_data( sname, &body->Attributes, &item.oix, body->Description);
  item.oix++;

  if ( next_oix <= item.oix)
    next_oix = item.oix + 1;
  item.cid = pwr_cClass_SystemGroupReg;
  item.fthoix = menu_stack[menu_cnt - 1];
  item.bwsoix = m_list[item.fthoix].lchoix;
  if ( item.bwsoix)
    m_list[item.bwsoix].fwsoix = item.oix;
  if ( m_list[item.fthoix].fchoix == 0)
    m_list[item.fthoix].fchoix = item.oix;
  m_list[item.fthoix].lchoix = item.oix;

  item.body_size = sizeof(pwr_sClass_SystemGroupReg);

  menu_stack[menu_cnt] = item.oix;
  menu_cnt++;
  if ( m_list.size() <= item.oix + 1)
    m_list.resize( item.oix + 1);
  m_list[item.oix] = item;

  UserList *user = systemgroup->first_user();
  while ( user) {
    load_user( user, systemgroup);

    user = user->next_user();
  }
  SystemList *sg = systemgroup->first_system();
  while ( sg) {
    load_systemgroup( sg);

    sg = sg->next_system();
  }
  menu_cnt--;
}

void rt_pvd_udb::load_user( UserList *user, SystemList *sg)
{
  procom_obj item;
  pwr_sClass_UserReg *body;

  body = (pwr_sClass_UserReg *) calloc( 1, sizeof(pwr_sClass_UserReg));
  user->get_data( body->Password, &body->Privileges, &item.oix, body->FullName, body->Description, 
		  body->Email, body->Phone, body->Sms);
  item.oix++;

  if ( next_oix <= item.oix)
    next_oix = item.oix + 1;

  item.cid = pwr_cClass_UserReg;
  item.fthoix = menu_stack[menu_cnt - 1];
  item.bwsoix = m_list[item.fthoix].lchoix;
  if ( item.bwsoix)
    m_list[item.bwsoix].fwsoix = item.oix;
  if ( m_list[item.fthoix].fchoix == 0)
    m_list[item.fthoix].fchoix = item.oix;
  m_list[item.fthoix].lchoix = item.oix;

  item.body_size = sizeof(pwr_sClass_UserReg);
  item.body = body;
  strcpy( item.name, user->get_name());

  if ( m_list.size() <= item.oix + 1)
    m_list.resize( item.oix + 1);
  m_list[item.oix] = item;
  // m_list.push_back( item);
}

void rt_pvd_udb::writeAttribute( co_procom *pcom, pwr_tOix oix, unsigned int offset,
				 unsigned int size, char *buffer)
{
  if ( oix >= m_list.size() || oix <= 0) {
    pcom->provideStatus( LDH__NOSUCHOBJ);
    return;
  }

  if ( offset + size > m_list[oix].body_size) {
    pcom->provideStatus( LDH__NOSUCHATTR);
    return;
  }

  // Crypt password
  if ( m_list[oix].cid == pwr_cClass_UserReg &&
       (int)offset == (char *)((pwr_sClass_UserReg *)m_list[oix].body)->Password - (char *)m_list[oix].body) {
    pwr_tString40 pw;

    strncpy( pw, UserList::pwcrypt( buffer), sizeof(pw));
    memcpy( (void *)((unsigned long)m_list[oix].body + (unsigned long)offset), pw, size);
  }
  else
    memcpy( (void *)((unsigned long)m_list[oix].body + (unsigned long)offset), buffer, size);

  pcom->provideStatus( 1);
}





