/* 
 * Proview   $Id: wb_orepdbms.cpp,v 1.1 2007-10-18 09:11:01 claes Exp $
 * Copyright (C) 2007 SSAB Oxel�sund AB.
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

#if defined PWRE_CONF_MYSQL

#include "wb_orepdbms.h"
#include "wb_vrepdbms.h"


wb_orepdbms::wb_orepdbms() :
  m_o(0), m_oid(pwr_cNOid)
{
}

wb_orepdbms::wb_orepdbms(pwr_tOid oid) :
  m_o(0), m_oid(oid)
{
}

wb_orepdbms::wb_orepdbms(dbms_sObject *o) :
  m_o(o), m_oid(o->oid)
{
}

wb_orepdbms::~wb_orepdbms()
{
}

void *
wb_orepdbms::operator new(size_t size, wb_vrepdbms *v)
{
  return (void *)v->new_wb_orepdbms(size);
}

#if 1
void
wb_orepdbms::operator delete(void *p)
{
  wb_orepdbms *o = (wb_orepdbms *)p;
  
  ((wb_vrepdbms *)o->m_vrep)->delete_wb_orepdbms(p);
}
#endif

//
//  Operations declared in wb_orep
//

pwr_tOid wb_orepdbms::oid() const
{    
  return m_oid;
}

pwr_tVid wb_orepdbms::cid() const
{
  pwr_tStatus sts;
  return m_vrep->cid(&sts, (wb_orep*)this);
}

pwr_tVid wb_orepdbms::vid() const
{
  pwr_tStatus sts;
  return m_vrep->vid(&sts, (wb_orep*)this);
}

pwr_tOix wb_orepdbms::oix() const
{
  pwr_tStatus sts;
  return m_vrep->oix(&sts, (wb_orep*)this);
}

pwr_tOid wb_orepdbms::poid() const
{
  pwr_tStatus sts;
  return m_vrep->poid(&sts, (wb_orep*)this);
}

pwr_tOid wb_orepdbms::foid() const
{
  pwr_tStatus sts;
  return m_vrep->foid(&sts, (wb_orep*)this);
}

pwr_tOid wb_orepdbms::loid() const
{
  pwr_tStatus sts;
  return m_vrep->loid(&sts, (wb_orep*)this);
}

pwr_tOid wb_orepdbms::boid() const
{
  pwr_tStatus sts;
  return m_vrep->boid(&sts, (wb_orep*)this);
}

pwr_tOid wb_orepdbms::aoid() const
{
  pwr_tStatus sts;
  return m_vrep->aoid(&sts, (wb_orep*)this);
}

wb_name wb_orepdbms::longName()
{
  pwr_tStatus sts;
  
  return m_vrep->longName(&sts, (wb_orep*)this);
}

const char * wb_orepdbms::name() const
{
  pwr_tStatus sts;
  return m_vrep->objectName(&sts, (wb_orep*)this);
}

pwr_tTime wb_orepdbms::ohTime() const
{
  pwr_tStatus sts;
   
  return m_vrep->ohTime(&sts, (wb_orep*)this);
}

pwr_tTime wb_orepdbms::rbTime() const
{
  pwr_tStatus sts;
   
  return m_vrep->rbTime(&sts, (wb_orep*)this);
}

pwr_tTime wb_orepdbms::dbTime() const
{
  pwr_tStatus sts;
   
  return m_vrep->dbTime(&sts, (wb_orep*)this);
}

pwr_mClassDef wb_orepdbms::flags() const
{
  pwr_tStatus sts;
    
  return m_vrep->flags(&sts, (wb_orep*)this);
}

bool wb_orepdbms::isOffspringOf(const wb_orep *o) const
{
  pwr_tStatus sts;
  return m_vrep->isOffspringOf(&sts, (wb_orep*)this, o);
}

wb_orep *wb_orepdbms::ancestor(pwr_tStatus *sts)
{
  return m_vrep->ancestor(sts, (wb_orep*)this);
}

wb_orep *wb_orepdbms::parent(pwr_tStatus *sts)
{
  return m_vrep->parent(sts, (wb_orep*)this);
}

wb_orep *wb_orepdbms::after(pwr_tStatus *sts)
{
  return m_vrep->after(sts, (wb_orep*)this);
}


wb_orep *wb_orepdbms::before(pwr_tStatus *sts)
{
  return m_vrep->before(sts, (wb_orep*)this);
}

wb_orep* wb_orepdbms::first(pwr_tStatus *sts)
{
  return m_vrep->first(sts, (wb_orep*)this);
}

wb_orep *wb_orepdbms::child(pwr_tStatus *sts, wb_name &name)
{
  return m_vrep->child(sts, (wb_orep*)this, name);
}

wb_orep *wb_orepdbms::last(pwr_tStatus *sts)
{
  return m_vrep->last(sts, (wb_orep*)this);
}

wb_orep *wb_orepdbms::next(pwr_tStatus *sts)
{
  return m_vrep->next(sts, (wb_orep*)this);
}

wb_orep *wb_orepdbms::previous(pwr_tStatus *sts)
{
  return m_vrep->previous(sts, (wb_orep*)this);
}

wb_adrep *wb_orepdbms::attribute(pwr_tStatus*, const char *aname)
{
  return 0;
}

wb_adrep *wb_orepdbms::attribute(pwr_tStatus*)
{
  return 0;
}

#endif
