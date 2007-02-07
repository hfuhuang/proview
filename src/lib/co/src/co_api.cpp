/** 
 * Proview   $Id: co_api.cpp,v 1.11 2007-02-07 15:46:22 claes Exp $
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

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"

#include "co_msgwindow.h"
#include "co_xhelp.h"
#include "co_nav_crr.h"

extern "C" {
#include "co_api.h"
#include "co_dcli.h"
}

#include "co_user.h"
#include "co_lng.h"

//
// c-api to co_user
//

int user_CheckUser( char *systemgroup, char *user, char *password, 
	unsigned int *priv)
{
  GeUser *gu;
  int sts;
  char filename[120];

  gu = new GeUser();
  sts = dcli_get_defaultfilename( user_cFilename, filename, "");
  sts = gu->load( filename);
  if ( ODD(sts))
    sts = gu->get_user( systemgroup, user, password, priv);
  delete gu;
  return sts;
}

int user_CheckSystemGroup( char *systemgroup)
{
  GeUser *gu;
  int sts;
  unsigned int attributes;
  char filename[120];

  gu = new GeUser();
  sts = dcli_get_defaultfilename( user_cFilename, filename, "");
  sts = gu->load( filename);
  if ( ODD(sts))
    sts = gu->get_system_data( systemgroup, &attributes);
  delete gu;
  return sts;
}

int user_GetUserPriv( char *systemgroup, char *user, unsigned int *priv)
{
  GeUser *gu;
  int sts;
  char filename[120];

  gu = new GeUser();
  sts = dcli_get_defaultfilename( user_cFilename, filename, "");
  sts = gu->load( filename);
  if ( ODD(sts))
    sts = gu->get_user_priv( systemgroup, user, priv);
  delete gu;
  return sts;
}

void user_PrivToString( unsigned int priv, char *str, int size)
{
  GeUser::priv_to_string( priv, str, size);
}

void user_RtPrivToString( unsigned int priv, char *str, int size)
{
  GeUser::rt_priv_to_string( priv, str, size);
}

void user_DevPrivToString( unsigned int priv, char *str, int size)
{
  GeUser::dev_priv_to_string( priv, str, size);
}

//
//  c api to co_lng
//

void lng_get_uid( char *in, char *out)
{
  Lng::get_uid( in, out);
}

char *lng_translate( char *str)
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
// c api to co_msgwindow
//
void msgw_message( int severity, char *text, msgw_ePop pop)
{
  MsgWindow::message( severity, text, pop);
}

void msgw_message_sts( pwr_tStatus sts, char *text1, char *text2)
{
  MsgWindow::message( co_error(sts), text1, text2);
}

void msgw_message_object( pwr_tStatus sts, char *text1, char *text2, pwr_tOid oid)
{
  MsgWindow::message( co_error(sts), text1, text2, oid);
}

void msgw_message_plcobject( pwr_tStatus sts, char *text1, char *text2, pwr_tOid oid)
{
  MsgWindow::message( co_error(sts), text1, text2, oid, true);
}

void msgw_set_nodraw()
{
  MsgWindow::dset_nodraw();
}

void msgw_reset_nodraw()
{
  MsgWindow::dreset_nodraw();
}

//
// c api to co_xhelp
//

int xhelp_help( char *key, char *help_bookmark, navh_eHelpFile file_type,
		char *file_name, int strict)
{
  return CoXHelp::dhelp( key, help_bookmark, file_type, file_name, strict != 0);
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




