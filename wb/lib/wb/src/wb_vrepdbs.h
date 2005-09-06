/** 
 * Proview   $Id: wb_vrepdbs.h,v 1.32 2005-09-06 08:02:04 claes Exp $
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

#ifndef wb_vrepdbs_h
#define wb_vrepdbs_h

#include "co_dbs.h"
#include "wb_vrep.h"
#include "wb_cdef.h"

class wb_merep;
class wb_orepdbs;

class wb_vrepdbs : public wb_vrep
{
public:
  wb_erep *m_erep;
  wb_merep *m_merep;
  unsigned int m_nRef;

  char m_fileName[200];
  bool m_isDbsenvLoaded;
    
  dbs_sMenv *m_dbsmep;
  dbs_sVenv *m_dbsvep;
  bool m_duplicate;
    
  wb_vrepdbs(wb_erep *erep, const char *fileName);
  wb_vrepdbs(wb_erep *erep, wb_merep * merep, const char *fileName, dbs_sMenv *mep, dbs_sVenv *vep);

  dbs_sVenv *dbsenv();
  bool load();
  void objectName(const wb_orep *o, char *str);
    
    
  virtual void unref();
  virtual wb_vrep *ref();

  virtual ldh_eVolRep type() const { return ldh_eVolRep_Dbs;}
  virtual wb_erep *erep() ;

  virtual pwr_tOid oid(pwr_tStatus *sts, const wb_orep *o);
    
  virtual pwr_tVid vid(pwr_tStatus *sts, const wb_orep *o);
    
  virtual pwr_tOix oix(pwr_tStatus *sts, const wb_orep *o);
    

  virtual pwr_tCid cid(pwr_tStatus *sts, const wb_orep *o);
    
  virtual pwr_tOid poid(pwr_tStatus *sts, const wb_orep *o);
  virtual pwr_tOid foid(pwr_tStatus *sts, const wb_orep *o);
  virtual pwr_tOid loid(pwr_tStatus *sts, const wb_orep *o);
  virtual pwr_tOid boid(pwr_tStatus *sts, const wb_orep *o);
  virtual pwr_tOid aoid(pwr_tStatus *sts, const wb_orep *o);
    
  virtual const char * objectName(pwr_tStatus *sts, const wb_orep *o);
    
  virtual wb_name longName(pwr_tStatus *sts, const wb_orep *o);
    
  virtual pwr_tTime ohTime(pwr_tStatus *sts, const wb_orep *o);
  virtual pwr_tTime rbTime(pwr_tStatus *sts, const wb_orep *o);
  virtual pwr_tTime dbTime(pwr_tStatus *sts, const wb_orep *o);
  virtual pwr_mClassDef flags(pwr_tStatus *sts, const wb_orep *o);
    
  virtual bool isOffspringOf(pwr_tStatus *sts, const wb_orep *child, const wb_orep *parent) { return false;}
    

  virtual wb_orep *object(pwr_tStatus *sts, pwr_tOid oid);
  virtual wb_orep *object(pwr_tStatus *sts, pwr_tCid cid);
  virtual wb_orep *object(pwr_tStatus *sts, wb_name &name);
  virtual wb_orep *object(pwr_tStatus *sts, const wb_orep *parent, wb_name &name);

  virtual wb_orep *createObject(pwr_tStatus *sts, wb_cdef cdef, wb_destination &d, wb_name &name);

  virtual wb_orep *copyObject(pwr_tStatus *sts, const wb_orep *orep, wb_destination &d, wb_name &name);
  virtual bool copyOset(pwr_tStatus *sts, wb_oset *oset, wb_destination &d);

  virtual bool moveObject(pwr_tStatus *sts, wb_orep *orep, wb_destination &d);

  virtual bool deleteObject(pwr_tStatus *sts, wb_orep *orep);
  virtual bool deleteFamily(pwr_tStatus *sts, wb_orep *orep);
  virtual bool deleteOset(pwr_tStatus *sts, wb_oset *oset);

  virtual bool renameObject(pwr_tStatus *sts, wb_orep *orep, wb_name &name);


  virtual bool commit(pwr_tStatus *sts);
  virtual bool abort(pwr_tStatus *sts);

  virtual bool writeAttribute(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p);

  virtual void *readAttribute(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p);

  virtual void *readBody(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, void *p);

  virtual bool writeBody(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, void *p);

  virtual wb_orep *ancestor(pwr_tStatus *sts, const wb_orep *o);

  virtual wb_orep *parent(pwr_tStatus *sts, const wb_orep *o);

  virtual wb_orep *after(pwr_tStatus *sts, const wb_orep *o);

  virtual wb_orep *before(pwr_tStatus *sts, const wb_orep *o);

  virtual wb_orep *first(pwr_tStatus *sts, const wb_orep *o);

  virtual wb_orep *child(pwr_tStatus *sts, const wb_orep *o, wb_name &name);

  virtual wb_orep *last(pwr_tStatus *sts, const wb_orep *o);

  virtual wb_orep *next(pwr_tStatus *sts, const wb_orep *o);

  virtual wb_orep *previous(pwr_tStatus *sts, const wb_orep *o);

  virtual wb_srep *newSession();

  virtual pwr_tVid vid() const;

  wb_orepdbs *new_wb_orepdbs(size_t size);
  void delete_wb_orepdbs(void *p);



  virtual wb_vrep *next ();
  virtual wb_orep *object (pwr_tStatus *);
  virtual bool isLocal (const wb_orep *);
  virtual pwr_tCid cid () const;
  virtual wb_merep *merep () const;
  virtual bool createSnapshot (const char *);

  virtual bool isCommonMeta() const { return false;}
  virtual bool isMeta() const { return (cid() == pwr_eClass_ClassVolume);}


  virtual bool exportVolume(wb_import &e);
  virtual bool exportHead(wb_import &e);
  virtual bool exportRbody(wb_import &e);
  virtual bool exportDbody(wb_import &e);
  virtual bool exportDocBlock(wb_import &e);
  virtual bool exportMeta(wb_import &e);
  virtual bool exportTree(wb_treeimport &i, pwr_tOid oid);
  bool wb_vrepdbs::exportTreeObject(wb_treeimport &i, dbs_sObject *op, bool isRoot);
  virtual bool importTree(bool keepref) { return false;}
  virtual bool importTreeObject(wb_merep *merep, pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
				pwr_tOid boid, const char *name, pwr_mClassDef flags,
				size_t rbSize, size_t dbSize, void *rbody, void *dbody)
    { return false;}
  virtual bool importPaste() { return false;}
  virtual bool importPasteObject(pwr_tOid destination, ldh_eDest destcode, 
				 bool keepoid, pwr_tOid oid, 
				 pwr_tCid cid, pwr_tOid poid,
				 pwr_tOid boid, const char *name, pwr_mClassDef flags,
				 size_t rbSize, size_t dbSize, void *rbody, void *dbody,
				 pwr_tOid *roid)
    { return false;}
  virtual void importIgnoreErrors() {}
  virtual bool accessSupported( ldh_eAccess access) { return access == ldh_eAccess_ReadOnly; }
  virtual bool duplicateDb() const { return m_duplicate;}
  virtual void setDuplicateDb( bool duplicate) { m_duplicate = duplicate;}
  virtual const char *fileName() { return m_fileName;}
};


#endif
