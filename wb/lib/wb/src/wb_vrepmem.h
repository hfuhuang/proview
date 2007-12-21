/* 
 * Proview   $Id: wb_vrepmem.h,v 1.24 2007-12-21 13:18:01 claes Exp $
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

#ifndef wb_vrepmem_h
#define wb_vrepmem_h

#include <string>

#include "wb_vrep.h"
#include "wb_erep.h"
#include "wb_attrname.h"
#include "wb_treeimport.h"
#include "co_dbs.h"
#include "wb_import.h"
#include "wb_recix.h"

class wb_vrepmem;

class mem_object
{
  friend class wb_vrepmem;

 public:
  mem_object() : rbody_size(0), dbody_size(0), docblock_size(0), rbody(0), dbody(0),
    docblock(0), m_cid(0), m_tid(0), fth(0), bws(0), fws(0), fch(0),
    is_built(0), m_created(0)
    { 
      strcpy( m_name, "");
      strcpy( cname, ""); 
      m_flags.m = 0;
      m_oid.oix = 0;
      m_oid.vid = 0;
      m_ohtime.tv_sec = 0;
      m_ohtime.tv_nsec = 0;
      m_rbtime.tv_sec = 0;
      m_rbtime.tv_nsec = 0;
      m_dbtime.tv_sec = 0;
      m_dbtime.tv_nsec = 0;
    }
  ~mem_object() {
    if ( rbody_size) free( rbody);
    if ( dbody_size) free( dbody);
    if ( docblock_size) free( docblock);
  }    
  char *name() { return m_name; }
  char *longName() {
    static pwr_tOName n;
    pwr_tOName tmp;
    strcpy( n, m_name);
    for ( mem_object *f = fth; f; f = f->fth) {
      strcpy( tmp, n);
      strcpy( n, f->m_name);
      strcat( n, "-");
      strcat( n, tmp);
    }
    return n;
  }
  mem_object *get_lch() { 
    mem_object *c = fch;
    if ( c)
      while ( c->fws) 
	c = c->fws;
    return c;
  }

  bool exportHead(wb_import &i) { return false;}
  bool exportDbody(wb_import &i) { return false;}
  bool exportRbody(wb_import &i) { return false;}
  bool exportDocBlock(wb_import &i) { return false;}
  bool exportTree( wb_treeimport &i, bool isRoot) {
    pwr_tOid fthoid = (fth && !isRoot) ? fth->m_oid : pwr_cNOid;
    pwr_tOid bwsoid = (bws && !isRoot) ? bws->m_oid : pwr_cNOid;

    i.importTreeObject( 0, m_oid, m_cid, fthoid, bwsoid, name(), m_flags, 
			rbody_size, dbody_size, rbody, dbody);
  
    if ( fch)
      fch->exportTree( i, false);

    if ( !isRoot && fws)
      fws->exportTree( i, false);

    return true;
  }
  bool exportPaste( wb_treeimport &i, pwr_tOid destination, bool isRoot, 
		    ldh_eDest destcode, bool keepoid, wb_recix *recix, pwr_tOid *rootlist) {
    pwr_tOid fthoid = (fth && !isRoot) ? fth->m_oid : pwr_cNOid;
    pwr_tOid bwsoid = (bws && !isRoot) ? bws->m_oid : pwr_cNOid;
    pwr_tOid oid;
    pwr_tOid woid;    

    if ( recix) {
      pwr_tOix ix;

      if ( recix->get( longName(), &ix)) {
	woid.oix = ix;
	woid.vid = m_oid.vid;
      }
      else
	woid = pwr_cNOid;
    }
    i.importPasteObject( destination, destcode, keepoid, m_oid, m_cid, fthoid, bwsoid, 
			 name(), m_flags, rbody_size, dbody_size, rbody, dbody, woid, &oid);

    if ( rootlist)
      *rootlist++ = oid;
  
    if ( fch)
      fch->exportPaste( i, destination, false, destcode, keepoid, recix, 0);

    if ( fws)
      fws->exportPaste( i, destination, false, destcode, keepoid, recix, rootlist);

    return true;
  }

  mem_object *next( pwr_tCid cid, pwr_tOix *oix) {
    // search is turned on when oix = 0
    if ( *oix == m_oid.oix)
      *oix = 0;
    mem_object *n;
    if ( fch) {
      if ( !*oix && fch->m_cid == cid)
	return fch;
      else {
	n = fch->next( cid, oix);
	if ( n)
	  return n;
      }
    }
    if ( fws) {
      if ( !*oix && fws->m_cid == cid)
	return fws;
      else {
	n = fws->next( cid, oix);
	if ( n)
	  return n;
      }
    }
    return 0;
  }

  mem_object *find( wb_name *oname, int level);
  bool docBlock( char **block, int *size) const {
    switch ( m_cid) {
    case pwr_eClass_ClassDef:
    case pwr_eClass_Param:
    case pwr_eClass_Intern:
    case pwr_eClass_Input:
    case pwr_eClass_Output:
    case pwr_eClass_ObjXRef:
    case pwr_eClass_AttrXRef:
      break;
    default:
      return false;
    }
    if ( docblock) {
      *block = (char *) malloc( docblock_size);
      memcpy( *block, docblock, docblock_size);
      *size = docblock_size;
    }
    else {
      // Return nullstring
      *block = (char *) calloc( 1, 1);
      *size = 1;
    }
    return true;
  }
  bool docBlock( char *block) {
    docblock_size = strlen(block) + 1;
    docblock = (char *) malloc( docblock_size);
    memcpy( docblock, block, docblock_size);
    return true;
  }

  
  size_t rbody_size;
  size_t dbody_size;
  size_t docblock_size;
  void *rbody;
  void *dbody;
  char *docblock;
  pwr_tCid m_cid;
  pwr_tTid m_tid;
  pwr_tOid m_oid;
  char cname[32];
  char m_name[32];
  pwr_mClassDef m_flags;
  mem_object *fth;
  mem_object *bws;
  mem_object *fws;
  mem_object *fch;
  pwr_tOid fthoid;
  pwr_tOid bwsoid;
  pwr_tOid fwsoid;
  pwr_tOid fchoid;
  int is_built;
  int m_created;
  pwr_tTime m_ohtime;
  pwr_tTime m_rbtime;
  pwr_tTime m_dbtime;
};

class wb_orepmem;

class wb_vrepmem : public wb_vrep, public wb_import
{
  wb_erep *m_erep;
  wb_merep *m_merep;
  unsigned int m_nRef;
  mem_object *root_object;
  mem_object *volume_object;
  int m_nextOix;
  pwr_tVid m_source_vid;
  char m_filename[200];
  bool m_classeditor;
  bool m_ignore;

  map<pwr_tOix, mem_object *> m_oix_list;

  typedef map<pwr_tOix, mem_object *>::iterator iterator_oix_list;

public:
  wb_vrepmem( wb_erep *erep) : 
    m_erep(erep), m_merep(erep->merep()), m_nRef(0), root_object(0), 
    volume_object(0), m_nextOix(0) {}

  wb_vrepmem( wb_erep *erep, pwr_tVid vid);
  ~wb_vrepmem();

  virtual ldh_eVolRep type() const { return ldh_eVolRep_Mem;}
  pwr_tVid vid() const { return m_vid;}
  pwr_tCid cid() const { return m_cid;}

  wb_vrep *next();

  virtual bool createSnapshot(const char *fileName, const pwr_tTime *time);
  virtual pwr_tStatus updateMeta() {return 0;}

  char volume_class[32];
  char volume_name[32];

  virtual void unref();
  virtual wb_vrep *ref();

  wb_erep *erep() {return m_erep;}
  wb_merep *merep() const { return m_merep;}

  int nextOix();
  mem_object *findObject( pwr_tOix oix);
  mem_object *find( const char *name);
  int nameToOid( const char *name, pwr_tOid *oid);
  bool registerObject( pwr_tOix oix, mem_object *node);
  bool unregisterObject( pwr_tOix oix);
  void registerVolume( const char *name, pwr_tCid cid, pwr_tVid vid, mem_object *node);
  void info();
  bool createVolumeObject( char *name);

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
    
  virtual pwr_tTime ohTime(pwr_tStatus *sts, const wb_orep *o) { return o->ohTime();}
  virtual pwr_tTime rbTime(pwr_tStatus *sts, const wb_orep *o) { return o->rbTime();}
  virtual pwr_tTime dbTime(pwr_tStatus *sts, const wb_orep *o) { return o->dbTime();}
  virtual pwr_mClassDef flags(pwr_tStatus *sts, const wb_orep *o) { pwr_mClassDef f; f.m = 0; return f;}    
    
  virtual bool isOffspringOf(pwr_tStatus *sts, const wb_orep *child, const wb_orep *parent) { return false;}
    

  wb_orep *object(pwr_tStatus *sts);
  wb_orep *object(pwr_tStatus *sts, pwr_tOid oid);
  wb_orep *object(pwr_tStatus *sts, pwr_tCid cid);
  wb_orep *object(pwr_tStatus *sts, wb_name &name);
  wb_orep *object(pwr_tStatus *sts, const wb_orep *parent, wb_name &name) {return 0;}

  wb_orep *createObject(pwr_tStatus *sts, wb_cdef cdef, wb_destination &d, wb_name &name,
			pwr_tOix oix = 0);

  wb_orep *copyObject(pwr_tStatus *sts, const wb_orep *orep, wb_destination &d, wb_name &name,
		      pwr_tOix oix = 0);
  bool copyOset(pwr_tStatus *sts, wb_oset *oset, wb_destination &d) {return false;}

  bool moveObject(pwr_tStatus *sts, wb_orep *orep, wb_destination &d);

  bool deleteObject(pwr_tStatus *sts, wb_orep *orep);
  bool deleteFamily(pwr_tStatus *sts, wb_orep *orep);
  bool deleteOset(pwr_tStatus *sts, wb_oset *oset) {return false;}

  bool renameObject(pwr_tStatus *sts, wb_orep *orep, wb_name &name);


  bool commit(pwr_tStatus *sts);
  bool abort(pwr_tStatus *sts);

  virtual bool writeAttribute(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p);

  virtual void *readAttribute(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p);

  virtual void *readBody(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, void *p);

  virtual bool writeBody(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, void *p);


  wb_orep *ancestor(pwr_tStatus *sts, const wb_orep *o);

  wb_orep *parent(pwr_tStatus *sts, const wb_orep *o);

  wb_orep *after(pwr_tStatus *sts, const wb_orep *o);

  wb_orep *before(pwr_tStatus *sts, const wb_orep *o);

  wb_orep *first(pwr_tStatus *sts, const wb_orep *o);

  wb_orep *child(pwr_tStatus *sts, const wb_orep *o, wb_name &name);

  wb_orep *last(pwr_tStatus *sts, const wb_orep *o);

  wb_orep *next(pwr_tStatus *sts, const wb_orep *o);

  wb_orep *previous(pwr_tStatus *sts, const wb_orep *o);

  wb_srep *newSession() {return 0;}

  bool isLocal(const wb_orep *o) {return o->oid().vid == vid();}

  void objectName(const wb_orep *o, char *str);

  virtual bool exportVolume(wb_import &i);
  virtual bool exportHead(wb_import &i);
  virtual bool exportRbody(wb_import &i);
  virtual bool exportDbody(wb_import &i);
  virtual bool exportDocBlock(wb_import &i);
  virtual bool exportMeta(wb_import &i);
  virtual bool exportTree(wb_treeimport &i, pwr_tOid oid);
  bool exportPaste(wb_treeimport &i, pwr_tOid destination, ldh_eDest destcode, bool keepoid,
		   wb_recix *recix, pwr_tOid **rootlist);
  virtual bool importTreeObject(wb_merep *merep, pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
				pwr_tOid boid, const char *name, pwr_mClassDef flags,
				size_t rbSize, size_t dbSize, void *rbody, void *dbody);
  virtual bool importTree( bool keepref);
  virtual bool importPasteObject(pwr_tOid destination, ldh_eDest destcode,
				 bool keepoid, pwr_tOid oid, 
				 pwr_tCid cid, pwr_tOid poid,
				 pwr_tOid boid, const char *name, pwr_mClassDef flags,
				 size_t rbSize, size_t dbSize, void *rbody, void *dbody,
				 pwr_tOid woid, pwr_tOid *roid);
  virtual bool importPaste();
  virtual void importIgnoreErrors() { m_ignore = true;}
  bool updateObject( wb_orep *o, bool keepref);
  bool updateSubClass( wb_adrep *subattr, char *body, bool keepref);
  virtual bool accessSupported( ldh_eAccess access) { return true;}
  virtual const char *fileName() { return "";}

  virtual bool importVolume(wb_export &e);    
  virtual bool importHead(pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
                          pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid, pwr_tOid loid,
                          const char *name, const char *normname, pwr_mClassDef flags,
                          pwr_tTime ohTime, pwr_tTime rbTime, pwr_tTime dbTime,
                          size_t rbSize, size_t dbSize);
  virtual bool importRbody(pwr_tOid oid, size_t size, void *body);    
  virtual bool importDbody(pwr_tOid oid, size_t size, void *body);
  virtual bool importDocBlock(pwr_tOid oid, size_t size, char *block);
  virtual bool importMeta(dbs_sMenv *mep) { return true;}
  bool importBuildObject( mem_object *memo);
  void loadWbl( char *filename, pwr_tStatus *sts, bool reload = false);
  void freeObject( mem_object *mem);
  void clear();
  bool classeditorCheck( ldh_eDest dest_code, mem_object *dest, pwr_tCid cid,
			 pwr_tOix *oix, char *name, pwr_tStatus *sts, bool import_paste);
  bool classeditorCheckMove( mem_object *memo, ldh_eDest dest_code, 
			     mem_object *dest, pwr_tStatus *sts);
  void classeditorCommit();
  void classeditorRenameObject( mem_object *memo, char *oldname, 
				wb_name &name);
  void classeditorDeleteObject( mem_object *memo);

 private:
  bool nameCheck( mem_object *memo);
  bool nameCheck( mem_object *parent, char *name, ldh_eDest code);
  void deleteChildren( mem_object *memo);
  void printPaletteFile();

};

#endif









