/* 
 * Proview   $Id: co_login.cpp,v 1.4 2008-10-31 12:51:30 claes Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "co_login_msg.h"
#include "co_login.h"
#include "co_user.h"
#include "co_api_user.h"
#include "co_dcli.h"

char		CoLogin::m_username[40] = "";
char	        CoLogin::m_password[40] = "";
char	        CoLogin::m_ucpassword[40] = "";
char		CoLogin::m_group[40] = "";
pwr_mPrv	CoLogin::m_priv = (pwr_mPrv)0;
login_mAttr     CoLogin::m_attribute = (login_mAttr)0;

void CoLogin::activate_ok()
{
  int	sts;

  message( "");
  sts = get_values();
  if ( ODD(sts)) {
    if ( bc_success)
      ( bc_success) ( parent_ctx);
    delete this;
    return;
  }
  else {
    message( (char *)"User not authorized");
    printf( "User not authorized\n");
    strcpy( (char *) &password, "");
  }
}

void CoLogin::activate_cancel()
{
  if ( bc_cancel)
    (bc_cancel) ( parent_ctx);

  delete this;
}

//
// Constructor
//
CoLogin::CoLogin( void		*wl_parent_ctx,
		const char     	*wl_name,
		const char     	*wl_groupname,
		void		(* wl_bc_success)( void *),
		void		(* wl_bc_cancel)( void *),
		pwr_tStatus   	*status) :
  parent_ctx(wl_parent_ctx), bc_success(wl_bc_success), bc_cancel(wl_bc_cancel)  
{
  strcpy( name, wl_name);
  strcpy( groupname, wl_groupname);
  *status = 1;
}

//
// Destructor
//
CoLogin::~CoLogin()
{
}

//
//	Check username and password and insert login infomation.
//
pwr_tStatus CoLogin::user_check( const char *groupname, const char *username, const char *password)
{
  pwr_tStatus		sts;
  unsigned int		priv;
  unsigned long		attr = 0;
  char			cpassword[40];
  
  strcpy( cpassword, UserList::pwcrypt( password));
  sts = user_CheckUser( groupname, username, cpassword, &priv);
  if ( EVEN(sts)) return sts;

  insert_login_info( groupname, cpassword, username, priv, attr);
  strcpy( m_ucpassword, password);
  return LOGIN__SUCCESS;
}

//
//	Inserts login info in global priv struct.
//
pwr_tStatus CoLogin::insert_login_info( const char *groupname, const char *password, const char *username, 
				       unsigned long priv, unsigned long attr)
{
  strcpy( m_username, username);
  strcpy( m_ucpassword, password);
  strcpy( m_group, groupname);
  m_priv = (pwr_mPrv)priv;
  m_attribute = (login_mAttr)attr;

  return LOGIN__SUCCESS;
}

pwr_tStatus CoLogin::get_login_info( char *groupname, char *password, char *username, 
				     unsigned long *priv, unsigned long *attr)
{
  if ( username)
    strcpy( username, m_username);
  if ( password)
    strcpy( password, m_password);
  if ( groupname)
    strcpy( groupname, m_group);
  if ( priv)
    *priv = (unsigned long)m_priv;
  if ( attr)
    *attr = (unsigned long)m_attribute;

  return LOGIN__SUCCESS;
}

