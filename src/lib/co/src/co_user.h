/** 
 * Proview   $Id: co_user.h,v 1.7 2008-10-31 12:51:30 claes Exp $
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

#ifndef co_user_h
#define co_user_h

/* ge_user.h -- User authorization */

#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

#include "pwr.h"

#if defined __cplusplus
extern "C" {
#endif

#define user_cFilename 	"pwra_db:pwr_user2.dat"
#define user_cVersion 	"V2.0.0"
#define user_cNId       ((pwr_tOix)(~0))
#define FSPACE 		" "

typedef enum {
  user_eData_GeUser			= 1,
  user_eData_System			= 2,
  user_eData_User			= 3,
  user_eData_End		       	= 99,
  user_eData_GeUserVersion		= 100,
  user_eData_GeNextId			= 101,
  user_eData_SystemName			= 200,
  user_eData_SystemLevel	       	= 201,
  user_eData_SystemAttributes		= 202,
  user_eData_SystemId			= 203,
  user_eData_SystemDescription 		= 204,
  user_eData_UserName			= 300,
  user_eData_UserPassword	       	= 301,
  user_eData_UserPrivilege		= 302,
  user_eData_UserId			= 303,
  user_eData_UserDescription   		= 304,
  user_eData_UserFullName   		= 305,
  user_eData_UserPhone			= 306,
  user_eData_UserEmail			= 307,
  user_eData_UserSms			= 308
} user_eData;


typedef enum {
  user_mSystemAttr_UserInherit = 1
} user_mSystemAttr;

class SystemName {
 public:
  SystemName( const char *system_name) : segments(0)
    {
      strcpy( pathname, system_name);
    };
  char	pathname[80];
  char	segname[10][40];
  int	segments;
  
  int 	parse();
  SystemName *parent();
  char	*name() { return pathname; };
  char	*segment( int idx) { return segname[idx]; };
};


class UserList;
class GeUser;

class SystemList {

  friend class GeUser;
  friend class UserList;

 public:
  SystemList( pwr_tOix sl_id, const char *sl_name, int sl_level, unsigned int sl_attributes, char *sl_descr) :
    level(sl_level), attributes( sl_attributes), id(sl_id),
    next(0), childlist(0), userlist(0) 
    {
      strcpy( name, sl_name);
      if ( sl_descr)	
	strncpy( description, sl_descr, sizeof(description));
      else
	strcpy( description, "");
    };
  

 private:
  pwr_tObjName 		name;
  int			level;
  pwr_tMask		attributes;
  pwr_tOix     		id;
  pwr_tString80		description;
  SystemList		*next;
  SystemList		*childlist;
  UserList		*userlist;

 public:
  int 		load( ifstream& fp);
  void		save( ofstream& fp);
  void 		print( int brief);
  void 		print_all( int brief);
  void		*find_user( char *name);
  int		add_user( pwr_tOix ident, char *user, char *password, pwr_tMask priv, char *fullname, char *descr, char *phone, char *email, char *sms);
  int		add_system( pwr_tOix ident, SystemName *name, pwr_tMask attributes, char *descr);
  int		load_system( ifstream& fp);
  int 		load_user( ifstream& fp);
  SystemList	*find_system( SystemName *name);
  int		remove_user( char *user);
  int		remove_system( SystemList *sys);
  void		modify( pwr_tMask attributes, char *description);
  void		get_data( pwr_tMask *attributes, pwr_tOix *id, char *description);
  SystemList	*first_system() { return childlist;}
  SystemList	*next_system() { return next;}	
  UserList	*first_user() { return userlist;}
  char		*get_name() { return name;}
  unsigned long	get_attributes() { return attributes;}
  ~SystemList();
};
  
class UserList {

  friend class SystemList;
  friend class GeUser;

 public:
  UserList( pwr_tOix ul_id, const char *ul_name, const char *ul_password, pwr_tMask ul_priv, 
	    const char *ul_fullname, const char *ul_description, const char *ul_email, 
	    const char *ul_phone, const char *ul_sms) :
    priv(ul_priv), id(ul_id), next(NULL)
    {
      strcpy( name, ul_name);
      strcpy( password, ul_password);
      if ( ul_fullname)
	strncpy( fullname, ul_fullname, sizeof(fullname));
      else
	strcpy( fullname, "");
      if ( ul_description)
	strncpy( description, ul_description, sizeof(description));
      else
	strcpy( description, "");
      if ( ul_email)
	strncpy( email, ul_email, sizeof(email));
      else
	strcpy( email, "");
      if ( ul_phone)
	strncpy( phone, ul_phone, sizeof(phone));
      else
	strcpy( phone, "");
      if ( ul_sms)
	strncpy( sms, ul_sms, sizeof(sms));
      else
	strcpy( sms, "");
    };
  

 private:
  pwr_tObjName 		name;
  char			password[40];
  pwr_tMask		priv;
  pwr_tOix     		id;
  pwr_tString80		fullname;
  pwr_tString80		description;
  pwr_tString80		email;
  pwr_tString40		phone;
  pwr_tString40		sms;
  UserList		*next;

  unsigned long	icrypt( unsigned long i);
  unsigned long	idecrypt( unsigned long i);

 public:
  int 		load( ifstream& fp);
  void 		save( ofstream& fp);
  void 		print( int brief);
  void 		print_all( int brief);
  void		modify( char *password, pwr_tMask priv, char *fullname, char *description,
			char *email, char *phone, char *sms);
  int		check_password( char *password);
  void	 	get_data( char *password, pwr_tMask *priv, pwr_tOix *id, char *fullname, char *description, 
			  char *email, char *phone, char *sms);
  UserList	*next_user() { return next;}
  char		*get_name() { return name;}
  unsigned long	get_priv() { return priv;}		
  static char     *pwcrypt( const char *str);
};

class GeUser {

  friend class SystemList;
  friend class UserList;

 public:
  GeUser() : root(NULL) { strcpy( version, ""); strcpy( fname, "");}
  ~GeUser();

 private:
  SystemList   	*root;
  SystemList   	*last_system;
  char		version[20];
  char		fname[256];
  pwr_tOix      next_id;

  bool 		get_system_name_child( SystemList *s, SystemList *system, char *name);
  SystemList   	*get_system_child( SystemList *system, UserList *user);
    

 public:
  int 		load( char *filename);
  void 		clear();
  int 		save() { return save( fname);}
  int 		save( char *filename);
  int 		load_system( ifstream& fp);
  void 		print( int brief);
  void 		print_all( int brief);
  SystemList   	*find_system( SystemName *name);
  int	       	add_system( char *name, unsigned int attributes, char *description, pwr_tOix id = user_cNId);
  int	       	add_user( char *system, char *user, char *password, 
			  unsigned int priv, char *fullname, char *description, char *email, char *phone, 
			  char *sms, pwr_tOix id = user_cNId);
  int	       	remove_user( char *system, char *user);
  int	       	modify_user( char *system, char *user, char *password,
			     unsigned int priv, char *fullname, char *description, char *email, char *phone, char *sms);
  int	       	get_user_data( char *system, char *user, char *password,
			       pwr_tMask *priv, pwr_tOix *id, char *fullname, char *description, char *email,
			       char *phone, char *sms);
  int	       	modify_system( char *system, unsigned int attributes, char *description);
  int	       	get_system_data( const char *system, pwr_tMask *attributes, pwr_tOix *id, char *description);
  int	       	remove_system( char *name);
  int 		get_user( const char *system, const char *user, const char *password,
			  unsigned int *priv);
  int 		get_user_priv( const char *system, const char *user,
			       unsigned int *priv);
  char 		*get_status( int sts);
  SystemList   	*root_system() { return root;}
  SystemList   	*get_system( UserList *user);
  bool		get_system_name( SystemList *system, char *name);
  pwr_tOix     	get_next_id() { return next_id++;}
  static void  	priv_to_string( unsigned int priv, char *str, int size);
  static void  	rt_priv_to_string( unsigned int priv, char *str, int size);
  static void  	dev_priv_to_string( unsigned int priv, char *str, int size);
  
};

#if defined __cplusplus
}
#endif
#endif








