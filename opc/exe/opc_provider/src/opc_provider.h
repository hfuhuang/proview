/* 
 * Proview   $Id: opc_provider.h,v 1.3 2007-03-08 07:26:29 claes Exp $
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

#ifndef opc_provider_h
#define opc_provider_h

#include "co_provider.h"
#include "co_procom.h"
#include "pwr_opcclasses.h"
#include "opc_soap_H.h"


class opc_provider : public co_provider {
public:
  opc_provider( pvd_eEnv env = pvd_eEnv_Wb) : co_provider(env), root(0), next_oix(1) {
    memset( &server_state, 0, sizeof(server_state));
  }
  virtual void object( co_procom *pcom);
  virtual void objectOid( co_procom *pcom, pwr_tOix oix);
  virtual void objectName( co_procom *pcom, char *name);
  virtual void objectBody( co_procom *pcom, pwr_tOix oix);
  virtual void createObject( co_procom *pcom, pwr_tOix destoix, int desttype,
		     pwr_tCid cid, char *name);
  virtual void moveObject( co_procom *pcom, pwr_tOix oix, pwr_tOix destoix, int desttype);
  virtual void copyObject( co_procom *pcom, pwr_tOix oix, pwr_tOix destoix, int desttype, 
		   char *name);
  virtual void deleteObject( co_procom *pcom, pwr_tOix oix);
  virtual void deleteFamily( co_procom *pcom, pwr_tOix oix);
  virtual void renameObject( co_procom *pcom, pwr_tOix oix, char *name);
  virtual void writeAttribute( co_procom *pcom, pwr_tOix oix, unsigned int offset,
		       unsigned int size, char *buffer);
  virtual void readAttribute( co_procom *pcom, pwr_tOix oix, unsigned int offset,
			      unsigned int size);
  virtual void subAssociateBuffer( co_procom *pcom, void **buff, int oix, int offset, 
				   int size, pwr_tSubid sid);
  virtual void commit( co_procom *pcom);
  virtual void abort( co_procom *pcom);

  virtual char *longname( pwr_tOix oix);
  virtual void delete_tree( pwr_tOix oix);

  virtual void save( pwr_tStatus *sts) {}

  virtual void load( pwr_tStatus *rsts) {}

  virtual bool find( pwr_tOix fthoix, char *name, pwr_tOix *oix);
  
  void insert_object( pwr_tOix fth, pwr_tOix bws, ns1__BrowseElement *element,
		      int first, int last, int load_children);

  void get_server_state();

  vector<procom_obj> m_list;
  pwr_tOix root;
  pwr_tOix next_oix;
  struct soap soap;
  pwr_sClass_Opc_ServerState server_state;
};

#endif