/* 
 * Proview   $Id: wb_vrepdbs.cpp,v 1.46 2005-10-25 15:27:47 claes Exp $
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

#include "wb_vrepdbs.h"
#include "wb_orepdbs.h"
#include "wb_merep.h"
#include "wb_erep.h"
#include "wb_import.h"
#include "wb_treeimport.h"

void wb_vrepdbs::unref()
{
  // printf("wb_vrepdbs::unref\n");
  
  if (--m_nRef == 0) {
    pwr_tStatus sts;
    printf("wb_vrepdbs::unref::dbs_Unmap\n");
    
    dbs_Unmap(&sts, m_dbsmep);
    delete this;
  }
}

wb_vrep *wb_vrepdbs::ref()
{
  m_nRef++;
  return this;
}

wb_vrepdbs::wb_vrepdbs(wb_erep *erep, const char *fileName) : m_erep(erep), m_nRef(0), m_duplicate(false)
{
  // printf("wb_vrepdbs(erep, fileName):%s\n", fileName);
  strcpy(m_fileName, fileName);
  m_isDbsenvLoaded = false;
  if (false && isCommonMeta())
    m_merep = m_erep->merep();
  else
    m_merep = new wb_merep(m_erep, (wb_mvrep *)this);
}

wb_vrepdbs::wb_vrepdbs(wb_erep *erep, wb_merep * merep, const char *fileName, dbs_sMenv *mep, dbs_sVenv *vep) : m_erep(erep), m_merep(merep), m_nRef(0), m_dbsmep(mep), m_dbsvep(vep), m_duplicate(false)
{
  // printf("wb_vrepdbs(erep, fileName, mep, vep):%d,%s\n", vep->vp->vid, fileName);
  strcpy(m_fileName, fileName);
  strcpy(m_name, m_dbsvep->vp->name);
  m_vid = m_dbsvep->vp->vid;
  m_cid = m_dbsvep->vp->cid;
  // printf("m_name: %s, m_vid: %d, m_cid: %d\n", m_name, m_vid, m_cid);
  m_isDbsenvLoaded = true;
}

dbs_sVenv *wb_vrepdbs::dbsenv()
{ 
  pwr_tStatus sts;

  if (!m_isDbsenvLoaded) {
    m_dbsmep = dbs_Map(&sts, m_fileName);
    if (!m_dbsmep) {
      throw wb_error(sts);
    }

    m_isDbsenvLoaded = true;

    m_dbsvep = dbs_Vmap(&sts, 0, m_dbsmep);

    strcpy(m_name, m_dbsvep->vp->name);
    m_vid = m_dbsvep->vp->vid;
    m_cid = m_dbsvep->vp->cid;
    // printf("m_name: %s, m_vid: %d, m_cid: %d\n", m_name, m_vid, m_cid);

    for (int i = 0; i < dbs_nVolRef(&sts, m_dbsmep); i++) {
      dbs_sVenv *vep = dbs_Vmap(&sts, i + 1, m_dbsmep);
      wb_vrepdbs *vp = new wb_vrepdbs(m_erep, m_merep, m_fileName, m_dbsmep, vep);
      // printf("before addDbs, i:%d, name: %s, vid: %d\n", i, vep->vp->name, vep->vp->vid);
      m_merep->addDbs(&sts, (wb_mvrep *)vp);
    }
  } else {
    if (strstr(m_fileName, "x86")) {
      //printf("%s::%s\n", m_dbsvep->vp->name, m_fileName);
    }
  }
 
  return m_dbsvep;
}

bool wb_vrepdbs::load()
{
  pwr_tStatus sts;
  bool rsts = (dbsenv() != 0);

  if (isMeta())
    m_merep->addDbs(&sts, (wb_mvrep *)this);

  return rsts;
}

pwr_tOid wb_vrepdbs::oid(pwr_tStatus *sts, const wb_orep *o)
{
  return o->oid();
}

pwr_tVid wb_vrepdbs::vid(pwr_tStatus *sts, const wb_orep *o)
{
  return o->vid();
}

pwr_tOix wb_vrepdbs::oix(pwr_tStatus *sts, const wb_orep *o)
{
  return o->oix();
}

pwr_tCid wb_vrepdbs::cid(pwr_tStatus *sts, const wb_orep *o)
{
  return o->cid();
}

pwr_tOid wb_vrepdbs::poid(pwr_tStatus *sts, const wb_orep *o)
{
  return o->foid();
}

pwr_tOid wb_vrepdbs::foid(pwr_tStatus *sts, const wb_orep *o)
{
  return o->foid();
}

pwr_tOid wb_vrepdbs::loid(pwr_tStatus *sts, const wb_orep *o)
{
  return o->loid();
}

pwr_tOid wb_vrepdbs::boid(pwr_tStatus *sts, const wb_orep *o)
{
  return o->boid();
}

pwr_tOid wb_vrepdbs::aoid(pwr_tStatus *sts, const wb_orep *o)
{
  return o->aoid();
}

const char *wb_vrepdbs::objectName(pwr_tStatus *sts, const wb_orep *o)
{
  return o->name();
}

wb_name wb_vrepdbs::longName(pwr_tStatus *sts, const wb_orep *o)
{
  return wb_name();
}

pwr_tTime wb_vrepdbs::ohTime(pwr_tStatus *sts, const wb_orep *o)
{
  return o->ohTime();
}

pwr_tTime wb_vrepdbs::rbTime(pwr_tStatus *sts, const wb_orep *o)
{
  return o->rbTime();
}

pwr_tTime wb_vrepdbs::dbTime(pwr_tStatus *sts, const wb_orep *o)
{
  return o->dbTime();
}

pwr_mClassDef wb_vrepdbs::flags(pwr_tStatus *sts, const wb_orep *o)
{
  return o->flags();
}    

wb_orep *wb_vrepdbs::object(pwr_tStatus *sts, pwr_tOid oid)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_OidToObject(sts, dbsenv(), oid);
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::object(pwr_tStatus *sts, wb_name &name)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_VolumeObject(sts, dbsenv());
    
  for (int i = 0; op && name.hasSegment(i); i++) {
    op = dbs_Child(sts, dbsenv(), op, name.normSegment(i));
  }
    
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;
    return 0;
  }    
    
  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::object(pwr_tStatus *sts, const wb_orep *parent, wb_name &name)
{
  *sts = LDH__SUCCESS;
    
  dbs_sObject *op = dbs_Child(sts, dbsenv(), ((wb_orepdbs *)parent)->o(), name.normName(cdh_mName_object));
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;
    return 0;
  }    
    
  return new (this) wb_orepdbs(op);
}


wb_orep *wb_vrepdbs::createObject(pwr_tStatus *sts, wb_cdef cdef, wb_destination &d, wb_name &name)
{
  *sts = LDH__NYI;
  return 0;
}


wb_orep *wb_vrepdbs::copyObject(pwr_tStatus *sts, const wb_orep *orep, wb_destination &d, wb_name &name)
{
  *sts = LDH__NYI;
  return 0;
}

bool wb_vrepdbs::copyOset(pwr_tStatus *sts, wb_oset *oset, wb_destination &d)
{
  *sts = LDH__NYI;
  return false;
}


bool wb_vrepdbs::moveObject(pwr_tStatus *sts, wb_orep *orep, wb_destination &d)
{
  *sts = LDH__NYI;
  return false;
}


bool wb_vrepdbs::deleteObject(pwr_tStatus *sts, wb_orep *orep)
{
  *sts = LDH__NYI;
  return false;
}

bool wb_vrepdbs::deleteFamily(pwr_tStatus *sts, wb_orep *orep)
{
  *sts = LDH__NYI;
  return false;
}

bool wb_vrepdbs::deleteOset(pwr_tStatus *sts, wb_oset *oset)
{
  *sts = LDH__NYI;
  return false;
}

bool wb_vrepdbs::renameObject(pwr_tStatus *sts, wb_orep *orep, wb_name &name)
{
  *sts = LDH__NYI;
  return false;
}

bool wb_vrepdbs::commit(pwr_tStatus *sts)
{
  *sts = LDH__SUCCESS;
  return true;
}

bool wb_vrepdbs::abort(pwr_tStatus *sts)
{
  *sts = LDH__SUCCESS;
  return true;
}

bool wb_vrepdbs::writeAttribute(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p)
{
  *sts = LDH__NYI;
  return false;
}

void *wb_vrepdbs::readAttribute(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p)
{// note! must be compensated for offset !
  *sts = LDH__SUCCESS;

  dbs_sObject *op = ((wb_orepdbs *)o)->o();
  void *bp = dbs_Body(sts, dbsenv(), op, bix);
    
  if (bp == 0) {
    *sts = LDH__NOSUCHBODY;
    return 0;
  }
    
  if (p) {
    switch (bix) {
    case pwr_eBix_rt:
      memcpy(p, (char *)bp + offset, MIN(op->rbody.size - offset, size));
      break;
    case pwr_eBix_dev:
      memcpy(p, (char *)bp + offset, MIN(op->dbody.size - offset, size));
      break;
    default:
      *sts = LDH__NOSUCHBODY;
      break;
    }
    return p;
  }
        
  return (void *)((char *)bp + offset);
}

void *wb_vrepdbs::readBody(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, void *p)
{
  *sts = LDH__SUCCESS;
    
  dbs_sObject *op = ((wb_orepdbs *)o)->o();
  void *bp = dbs_Body(sts, dbsenv(), op, bix);
    
  if (bp == 0) {
    *sts = LDH__NOSUCHBODY;
    return 0;
  }
    
  if (p) {
    switch (bix) {
    case pwr_eBix_rt:
      memcpy(p, bp, op->rbody.size);
      break;
    case pwr_eBix_dev:
      memcpy(p, bp, op->dbody.size);
      break;
    default:
      *sts = LDH__NOSUCHBODY;
      break;
    }
    return p;
  }
        
  return bp;
}

bool wb_vrepdbs::writeBody(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, void *p)
{
  *sts = LDH__NYI;
  return false;
}

wb_orep *wb_vrepdbs::ancestor(pwr_tStatus *sts, const wb_orep *o)
{    
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_Ancestor(sts, dbsenv(), ((wb_orepdbs *)o)->o());
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::parent(pwr_tStatus *sts, const wb_orep *o)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_Parent(sts, dbsenv(), ((wb_orepdbs *)o)->o());
  if (op == 0 || op->oid.oix == 0) {
    *sts = LDH__NOSUCHOBJ;
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::after(pwr_tStatus *sts, const wb_orep *o)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_After(sts, dbsenv(), ((wb_orepdbs *)o)->o());
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::before(pwr_tStatus *sts, const wb_orep *o)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_Before(sts, dbsenv(), ((wb_orepdbs *)o)->o());
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::first(pwr_tStatus *sts, const wb_orep *o)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_First(sts, dbsenv(), ((wb_orepdbs *)o)->o());
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::child(pwr_tStatus *sts, const wb_orep *o, wb_name &name)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_Child(sts, dbsenv(), ((wb_orepdbs *)o)->o(), name.normObject());
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::last(pwr_tStatus *sts, const wb_orep *o)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_Last(sts, dbsenv(), ((wb_orepdbs *)o)->o());
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::object(pwr_tStatus *sts, pwr_tCid cid)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_ClassToObject(sts, dbsenv(), cid);
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::next(pwr_tStatus *sts, const wb_orep *o)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_Next(sts, dbsenv(), ((wb_orepdbs *)o)->o());
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_orep *wb_vrepdbs::previous(pwr_tStatus *sts, const wb_orep *o)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_Previous(sts, dbsenv(), ((wb_orepdbs *)o)->o());
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;        
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

wb_srep *wb_vrepdbs::newSession()
{
//    *sts = LDH__SUCCESS;

  return (wb_srep*)0;
}

wb_orep *wb_vrepdbs::object(pwr_tStatus *sts)
{
  *sts = LDH__SUCCESS;

  dbs_sObject *op = dbs_Object(sts, dbsenv());    
  if (op == 0) {
    *sts = LDH__NOSUCHOBJ;
    return 0;
  }

  return new (this) wb_orepdbs(op);
}

bool wb_vrepdbs::isLocal(const wb_orep *)
{
  //*sts = LDH__NYI;
  return false;
}

pwr_tCid wb_vrepdbs::cid() const
{
  return m_dbsvep->vp->cid;;
}

pwr_tVid wb_vrepdbs::vid() const
{
  return m_vid;
}

wb_erep *wb_vrepdbs::erep()
{
  return m_erep;
}

wb_vrep *wb_vrepdbs::next ()
{
  pwr_tStatus sts;

  return m_erep->nextVolume( &sts, vid());
}

wb_merep *wb_vrepdbs::merep() const
{
  return m_merep;
}

bool wb_vrepdbs::createSnapshot(const char *)
{
  return false;
}

wb_orepdbs *wb_vrepdbs::new_wb_orepdbs(size_t size)
{
  wb_orepdbs *o = (wb_orepdbs *) calloc(1, size);
  o->m_vrep = this;
  return o;
}

void wb_vrepdbs::delete_wb_orepdbs(void *p)
{
  free(p);
}

void wb_vrepdbs::objectName(const wb_orep *o, char *str)
{
  pwr_tStatus sts;
    
  *str = 0;

  dbs_ObjectToName(&sts, dbsenv(), ((wb_orepdbs *)o)->o(), str);
}

bool wb_vrepdbs::exportVolume(wb_import &i)
{
  return i.importVolume(*this);
}

bool wb_vrepdbs::exportHead(wb_import &i)
{
  dbs_sObject *op = 0;
  pwr_tStatus sts;
  
  while ((op = dbs_NextHead(&sts, dbsenv(), op))) {
    i.importHead(op->oid, op->cid, op->poid, op->boid, op->aoid, op->foid, op->loid, op->name, op->normname,
                 op->ohFlags, op->time, op->rbody.time, op->dbody.time, op->rbody.size, op->dbody.size);
  }

  return true;
}

bool wb_vrepdbs::exportRbody(wb_import &i)
{
  dbs_sBody *bp = 0;
  pwr_tStatus sts;
  
  while ((bp = dbs_NextRbody(&sts, dbsenv(), bp))) {
    i.importRbody(bp->oid, bp->size, (void*)(bp + 1));
  }

  return true;
}

bool wb_vrepdbs::exportDbody(wb_import &i)
{
  dbs_sBody *bp = 0;
  pwr_tStatus sts;
  
  while ((bp = dbs_NextDbody(&sts, dbsenv(), bp))) {
    i.importDbody(bp->oid, bp->size, (void*)(bp + 1));
  }

  return true;
}

bool wb_vrepdbs::exportDocBlock(wb_import &i)
{
  
  return false;
}

bool wb_vrepdbs::exportMeta(wb_import &i)
{
  i.importMeta(dbsenv()->mp);

  return false;
}

bool wb_vrepdbs::exportTree(wb_treeimport &i, pwr_tOid oid)
{
  pwr_tStatus sts;

  dbs_sObject *op = dbs_OidToObject( &sts, dbsenv(), oid);
  if (op == 0)
    throw wb_error(LDH__NOSUCHOBJ);

  exportTreeObject( i, op, true);
  return true;
}

bool wb_vrepdbs::exportTreeObject(wb_treeimport &i, dbs_sObject *op, bool isRoot)
{
  pwr_tStatus sts;
  dbs_sObject *before = dbs_Before(&sts, dbsenv(), op);
  dbs_sObject *parent = dbs_Parent(&sts, dbsenv(), op);
  dbs_sObject *first = dbs_First(&sts, dbsenv(), op);
  dbs_sObject *after = dbs_After(&sts, dbsenv(), op);
  pwr_tOid parentoid = pwr_cNOid;
  pwr_tOid beforeoid = pwr_cNOid;
  void *rbody = 0;
  void *dbody = 0;

  if ( parent && !isRoot)
    parentoid = parent->oid;
  if ( before && !isRoot)
    beforeoid = before->oid;
  if ( op->rbody.size)
    rbody = dbs_Body(&sts, dbsenv(), op, pwr_eBix_rt);
  if ( op->dbody.size)
    dbody = dbs_Body(&sts, dbsenv(), op, pwr_eBix_dev);

  i.importTreeObject( m_merep, op->oid, op->cid, parentoid, beforeoid, op->name, op->ohFlags,
		      op->rbody.size, op->dbody.size, rbody, dbody);

  if ( first)
    exportTreeObject( i, first, false);

  if ( !isRoot && after)
    exportTreeObject( i, after, false);

  return true;
}






