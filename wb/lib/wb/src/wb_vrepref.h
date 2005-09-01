/** 
 * Proview   $Id: wb_vrepref.h,v 1.3 2005-09-01 14:57:59 claes Exp $
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

#ifndef wb_vrepref_h
#define wb_vrepref_h

#include <string>

#include "wb_vrep.h"
#include "wb_erep.h"
#include "wb_attrname.h"

class wb_orepref;

class wb_vrepref : public wb_vrep
{

  wb_erep *m_erep;
  wb_merep *m_merep;
  unsigned int m_nRef;

  friend class wb_orep;
public:
  wb_vrepref( wb_erep *erep, pwr_tVid vid) : wb_vrep(vid), m_erep(erep), 
    m_merep(erep->merep()), m_nRef(0) {
    m_cid = pwr_eClass_VolatileVolume;
    switch( m_vid) {
    case ldh_cPlcConnectVolume:
      strcpy( m_name, "$PlcConnect");
      break;
    case ldh_cPlcHostVolume:
      strcpy( m_name, "$PlcHost");
      break;
    case ldh_cIoConnectVolume:
      strcpy( m_name, "$IoConnect");
      break;
    }
  }

  ~wb_vrepref() {};

  virtual ldh_eVolRep type() const { return ldh_eVolRep_Ref;}
  pwr_tVid vid() const { return m_vid;}
  pwr_tCid cid() const { return m_cid;}

  void setMerep( wb_merep *merep) { m_merep = merep;}
  wb_vrep *next() {
    pwr_tStatus sts;
    return m_erep->nextVolume( &sts, vid());
  }

  virtual void unref() {
    if (--m_nRef == 0)
      delete this;
  }
  virtual wb_vrep *ref() {
    m_nRef++;
    return this;
  }

  wb_erep *erep() {return m_erep;}
  wb_merep *merep() const { return m_merep;}
  virtual bool createSnapshot(const char *fileName) { return false;}
  virtual pwr_tOid oid(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNOid;}    
  virtual pwr_tVid vid(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNVid;}    
  virtual pwr_tOix oix(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNOix;}
  virtual pwr_tCid cid(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNCid;}    
  virtual pwr_tOid poid(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNOid;}
  virtual pwr_tOid foid(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNOid;}
  virtual pwr_tOid loid(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNOid;}
  virtual pwr_tOid boid(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNOid;}
  virtual pwr_tOid aoid(pwr_tStatus *sts, const wb_orep *o) { return pwr_cNOid;}    
  virtual const char * objectName(pwr_tStatus *sts, const wb_orep *o) { return "";}    
  virtual wb_name longName(pwr_tStatus *sts, const wb_orep *o) { return wb_name();}    
  virtual pwr_tTime ohTime(pwr_tStatus *sts, const wb_orep *o) { pwr_tTime t = {0, 0}; return t;}
  virtual pwr_mClassDef flags(pwr_tStatus *sts, const wb_orep *o) { pwr_mClassDef f; f.m = 0; return f;}
  virtual void objectName(const wb_orep *o, char *str);
  virtual bool isOffspringOf(pwr_tStatus *sts, const wb_orep *child, const wb_orep *parent) { return false;}
    

  wb_orep *object(pwr_tStatus *sts) { *sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *object(pwr_tStatus *sts, pwr_tOid oid);
  wb_orep *object(pwr_tStatus *sts, pwr_tCid cid) { *sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *object(pwr_tStatus *sts, wb_name &name);
  wb_orep *object(pwr_tStatus *sts, const wb_orep *parent, wb_name &name) { *sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *createObject(pwr_tStatus *sts, wb_cdef cdef, wb_destination &d, wb_name &name) {return 0;}
  wb_orep *copyObject(pwr_tStatus *sts, const wb_orep *orep, wb_destination &d, wb_name &name) {return 0;}
  bool copyOset(pwr_tStatus *sts, wb_oset *oset, wb_destination &d) {return false;}
  bool moveObject(pwr_tStatus *sts, wb_orep *orep, wb_destination &d) {return false;}

  bool deleteObject(pwr_tStatus *sts, wb_orep *orep) {return false;}
  bool deleteFamily(pwr_tStatus *sts, wb_orep *orep) {return false;}
  bool deleteOset(pwr_tStatus *sts, wb_oset *oset) {return false;}
  bool renameObject(pwr_tStatus *sts, wb_orep *orep, wb_name &name) {return false;}
  bool commit(pwr_tStatus *sts) {return false;}
  bool abort(pwr_tStatus *sts) {return false;}
  virtual bool writeAttribute(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p) {return false;}

  virtual void *readAttribute(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p) { return 0;}
  virtual void *readBody(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, void *p) {return 0;}
  virtual bool writeBody(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, void *p) {return false;}
  wb_orep *ancestor(pwr_tStatus *sts, const wb_orep *o) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *parent(pwr_tStatus *sts, const wb_orep *o) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *after(pwr_tStatus *sts, const wb_orep *o) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *before(pwr_tStatus *sts, const wb_orep *o) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *first(pwr_tStatus *sts, const wb_orep *o) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *child(pwr_tStatus *sts, const wb_orep *o, wb_name &name) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *last(pwr_tStatus *sts, const wb_orep *o) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *next(pwr_tStatus *sts, const wb_orep *o) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_orep *previous(pwr_tStatus *sts, const wb_orep *o) {*sts = LDH__NOSUCHOBJ; return 0;}
  wb_srep *newSession() {return 0;}
  bool isLocal(const wb_orep *o) {return o->oid().vid == vid();}

  virtual bool accessSupported( ldh_eAccess access) { return false;}
  virtual const char *fileName() { return "";}

  virtual bool exportVolume(wb_import &i) {return false;}
  virtual bool exportHead(wb_import &i) {return false;}
  virtual bool exportRbody(wb_import &i) {return false;}
  virtual bool exportDbody(wb_import &i) {return false;}
  virtual bool exportDocBlock(wb_import &i) {return false;}
  virtual bool exportMeta(wb_import &i) {return false;}
  virtual bool exportTree(wb_treeimport &i, pwr_tOid oid) {return false;}
  bool exportPaste(wb_treeimport &i, pwr_tOid destination, ldh_eDest destcode, bool keepoid,
		   pwr_tOid **rootlist) {return false;}
  virtual bool importTreeObject(wb_merep *merep, pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
				pwr_tOid boid, const char *name, pwr_mClassDef flags,
				size_t rbSize, size_t dbSize, void *rbody, void *dbody) {return false;}
  virtual bool importTree( bool keepref) {return false;}
  virtual bool importPasteObject(pwr_tOid destination, ldh_eDest destcode,
				 bool keepoid, pwr_tOid oid, 
				 pwr_tCid cid, pwr_tOid poid,
				 pwr_tOid boid, const char *name, pwr_mClassDef flags,
				 size_t rbSize, size_t dbSize, void *rbody, void *dbody,
				 pwr_tOid *roid) {return false;}
  virtual bool importPaste() {return false;}
  virtual void importIgnoreErrors() {}
  bool updateObject( wb_orep *o, bool keepref) {return false;}
  bool updateSubClass( wb_adrep *subattr, char *body, bool keepref) {return false;}
  // virtual bool importVolume(wb_export &e) {return false;}
  // virtual bool importHead(pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
  //                        pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid, pwr_tOid loid,
  //                        const char *name, const char *normname, pwr_mClassDef flags,
  //                        pwr_tTime ohTime, pwr_tTime rbTime, pwr_tTime dbTime,
  //                        size_t rbSize, size_t dbSize) {return false;}
  // virtual bool importRbody(pwr_tOid oid, size_t size, void *body) {return false;}
  // virtual bool importDbody(pwr_tOid oid, size_t size, void *body) {return false;}
  // virtual bool importDocBlock(pwr_tOid oid, size_t size, char *block) {return false;}
  // virtual bool importMeta(dbs_sMenv *mep) { return false;}
};

#endif
