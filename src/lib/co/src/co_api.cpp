/** 
 * Proview   $Id: co_api.cpp,v 1.13 2008-10-31 12:51:30 claes Exp $
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

#if defined OS_VMS && defined __ALPHA
# pragma message disable (NOSIMPINT,EXTROUENCUNNOBJ)
#endif

#if defined OS_VMS && !defined __ALPHA
# pragma message disable (LONGEXTERN)
#endif

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "pwr.h"
}

#include "co_nav_crr.h"

extern "C" {
#include "co_api.h"
#include "co_dcli.h"
}

#include "co_lng.h"

//
//  c api to co_lng
//

void lng_get_uid( char *in, char *out)
{
  Lng::get_uid( in, out);
}

char *lng_translate( const char *str)
{
  return Lng::translate( str);
}

void lng_set( char *str)
{
  Lng::set( str);
}

char *lng_get_language_str()
{
  return Lng::get_language_str();
}

//
//  c api to co_crr
//

int crr_signal( void *parent_ctx, char *signalname,
	     void (*insert_cb)( void *, void *, navc_eItemType, char *, char *, int),
	     int (*name_to_objid_cb)( void *, char *, pwr_tObjid *),
	     int (*get_volume_cb)( void *, pwr_tVolumeId *))
{
  int sts;
  NavCrr *navcrr = new NavCrr( parent_ctx, 0);
  navcrr->insert_cb = insert_cb;
  navcrr->name_to_objid_cb = name_to_objid_cb;
  navcrr->get_volume_cb = get_volume_cb;
  sts = navcrr->crr_signal( 0, signalname);

  delete navcrr;
  return sts;
}

int crr_object( void *parent_ctx, char *objectname,
	     void (*insert_cb)( void *, void *, navc_eItemType, char *, char *, int),
	     int (*name_to_objid_cb)( void *, char *, pwr_tObjid *),
	     int (*get_volume_cb)( void *, pwr_tVolumeId *))
{
  int sts;
  NavCrr *navcrr = new NavCrr( parent_ctx, 0);
  navcrr->insert_cb = insert_cb;
  navcrr->name_to_objid_cb = name_to_objid_cb;
  navcrr->get_volume_cb = get_volume_cb;

  sts = navcrr->crr_object( 0, objectname);

  delete navcrr;
  return sts;
}
