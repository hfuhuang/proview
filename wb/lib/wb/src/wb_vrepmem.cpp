/* 
 * Proview   $Id: wb_vrepmem.cpp,v 1.28 2007-11-06 13:26:53 claes Exp $
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

#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "wb_vrepmem.h"
#include "wb_orepmem.h"
#include "wb_ldh_msg.h"
#include "wb_dbs.h"
#include "wb_bdrep.h"
#include "wb_cdrep.h"
#include "wb_attribute.h"
#include "wb_erep.h"
#include "wb_merep.h"
#include "wb_tdrep.h"
#include "wb_ldh_msg.h"
#include "wb_vrepwbl.h"
#include "wb_vrepref.h"
#include "wb_print_wbl.h"
#include "wb_volume.h"
#include "wb_palfile.h"
#include "pwr_baseclasses.h"
#include "co_msgwindow.h"

extern "C" {
#include "co_dcli.h"
}

wb_vrepmem::wb_vrepmem( wb_erep *erep, pwr_tVid vid) :
  wb_vrep(vid), m_erep(erep), m_merep(erep->merep()), root_object(0), volume_object(0),
  m_nextOix(0), m_source_vid(0), m_classeditor(false), m_ignore(false)
{
  strcpy( m_filename, "");

#if 0
  pwr_tStatus sts;

  // Create the volume object
  wb_cdrep *cdrep = m_merep->cdrep( &sts, pwr_eClass_VolatileVolume);
  wb_cdef cdef = wb_cdef( cdrep);
  wb_destination d = wb_destination( pwr_cNObjid, ldh_eDest_IntoFirst);
  wb_name n = wb_name("Tmp");
  
  wb_orep *o = createObject( &sts, cdef, d, n);
  delete o;
#endif
  m_vid = vid;
  m_cid = pwr_eClass_VolatileVolume;
}

wb_vrepmem::~wb_vrepmem()
{
  m_erep->resetRefMerep();

  clear();
}

void wb_vrepmem::loadWbl( char *filename, pwr_tStatus *sts, bool reload)
{
  if ( !reload && m_erep->refMerepOccupied()) {
    *sts = LDH__OTHERSESS;
    return;
  }
  wb_vrepwbl *vrep = new wb_vrepwbl( m_erep);
  *sts = vrep->load( filename);
  if ( vrep->vid() == 0) {
    delete vrep;
    return;
  }

  // Insert vrepwbl in local merep to interprete the Template objects
  if ( m_merep == m_erep->merep())
    m_merep = new wb_merep( *m_erep->merep(), this);

  wb_mvrep *mvrep = m_merep->volume( sts, vrep->vid());
  if ( ODD(*sts))
    m_merep->removeDbs( sts, mvrep);

  m_merep->addDbs( sts, (wb_mvrep *)vrep);

  // Change merep in ref volumes
  m_erep->setRefMerep( m_merep);

  vrep->ref();
  m_vid = vrep->vid();
  strcpy( m_filename, filename);
  name( vrep->name());

  importVolume( *vrep);
  vrep->unref();

  m_classeditor = true;
  *sts = LDH__SUCCESS;
}

int wb_vrepmem::nextOix()
{
  m_nextOix++;

  // This oix might be occupied
  while ( findObject( m_nextOix))
    m_nextOix++;

  if ( m_classeditor && volume_object)
    ((pwr_sClassVolume *)volume_object->rbody)->NextOix = m_nextOix;
  
  return m_nextOix;
}

wb_orep *wb_vrepmem::object(pwr_tStatus *sts, pwr_tOid oid)
{
  if ( oid.vid != m_vid) {
    *sts = LDH__BADOBJID;
    return 0;
  }
  
  if ( oid.oix == 0 && !volume_object) {
    // Volume object is not created yet...
    createVolumeObject( volume_name);
  }


  mem_object *n = findObject( oid.oix);
  if ( !n) {
    *sts = LDH__NOSUCHOBJ;
    return 0;
  }
  *sts = LDH__SUCCESS;
  return new wb_orepmem( this, n);
}

wb_orep *wb_vrepmem::object(pwr_tStatus *sts, pwr_tCid cid)
{
  if ( root_object) {
    mem_object *n;
    if ( root_object->m_cid == cid)
      n = root_object;
    else {
      pwr_tOix oix = 0;
      n = root_object->next( cid, &oix);
    }
    if ( n) {
      *sts = LDH__SUCCESS;
      return new wb_orepmem( this, n);
    }
  }
  *sts = LDH__NOSUCHOBJ;
  return 0;
}

wb_orep *wb_vrepmem::object(pwr_tStatus *sts, wb_name &name)
{
  mem_object *n = find( name.name());
  if ( !n) {
    *sts = LDH__NOSUCHOBJ;
    return 0;
  }
  *sts = LDH__SUCCESS;
  return new wb_orepmem( this, n);
}

wb_vrep *wb_vrepmem::next()
{
  pwr_tStatus sts;

  return m_erep->nextVolume( &sts, vid());
}

void wb_vrepmem::info()
{

  cout << "Volume : " << volume_name << " " << volume_class << " " << m_vid << endl;

}

bool
wb_vrepmem::createSnapshot(const char *fileName, const pwr_tTime *time)
{
  try {
    wb_dbs dbs(this);
        
    if ( fileName)
      dbs.setFileName( fileName);
    if ( time)
      dbs.setTime( *time);

    dbs.importVolume(*this);

    return true;
  } catch (wb_error& e) {
    return false;
  }
}

bool
wb_vrepmem::exportVolume(wb_import &i)
{
  try {
    i.importVolume(*this);

    return true;
  } catch (wb_error& e) {
    return false;
  }
}

bool wb_vrepmem::exportHead(wb_import &i)
{
  if ( root_object)
    return root_object->exportHead(i);
  else
    return false;
}

bool wb_vrepmem::exportDbody(wb_import &i)
{
  if ( root_object)
    return root_object->exportDbody(i);
  else
    return false;
  
}

bool wb_vrepmem::exportRbody(wb_import &i)
{
  if ( root_object)
    return root_object->exportRbody(i);
  else
    return false;
}

bool wb_vrepmem::exportDocBlock(wb_import &i)
{
  return false;
}

bool wb_vrepmem::exportMeta(wb_import &i)
{
  return false;
}

mem_object *wb_vrepmem::findObject( pwr_tOix oix)
{
  iterator_oix_list it = m_oix_list.find( oix);
  if ( it == m_oix_list.end())
    return 0;
  return it->second;
}

int wb_vrepmem::nameToOid( const char *name, pwr_tOid *oid)
{
  if ( strncmp( name, "_O", 2) == 0) {
    cdh_StringToObjid( name, oid);
    return 1;
  }

  mem_object *n = find( name);
  if ( n) {
    *oid = n->m_oid;
    return 1;
  }

  // Search in other volume
  pwr_tStatus sts;

  wb_name na(name);
  
  wb_orep *orep = m_erep->object( &sts, na);
  if ( EVEN(sts))
    return 0;
  else {
    *oid = orep->oid();
    // Delete
    orep->ref();
    orep->unref();

    return 1;
  }
}

bool wb_vrepmem::registerObject( pwr_tOix oix, mem_object *node)
{
  pair<pwr_tOix, mem_object *>p(oix,node);
  pair<map<pwr_tOix,mem_object *>::iterator,bool>result = m_oix_list.insert(p);

  return result.second;
}

bool wb_vrepmem::unregisterObject( pwr_tOix oix)
{
  iterator_oix_list it = m_oix_list.find( oix);
  if ( it == m_oix_list.end())
    return false;
  m_oix_list.erase( it);
  return true;
}

void wb_vrepmem::registerVolume( const char *name, pwr_tCid cid, pwr_tVid vid, mem_object *node)
{
  m_vid = vid;
  strcpy( volume_name, name);
  strcpy( m_name, name);
  m_cid = cid;
  root_object = node;
}


mem_object *wb_vrepmem::find( const char *name)
{
  wb_name oname = wb_name(name);

  if ( oname.evenSts() || (oname.hasVolume() && !oname.volumeIsEqual(this->name())))
    return 0;

  if ( root_object)
    return root_object->find( &oname, 0);
  return 0;
}

void wb_vrepmem::unref()
{
  if (--m_nRef == 0)
    delete this;
}

wb_vrep *wb_vrepmem::ref()
{
  m_nRef++;
  return this;
}

wb_orep *wb_vrepmem::object(pwr_tStatus *sts)
{
  wb_orepmem *orep = 0;

  if ( root_object) {
    orep = new wb_orepmem( (wb_vrepmem *)this, root_object);
    *sts = LDH__SUCCESS;
  }
  else
    *sts = LDH__NOSUCHOBJ;

  return orep;
}

wb_orep *wb_vrepmem::ancestor(pwr_tStatus *sts, const wb_orep *o)
{
  wb_orepmem *orep = 0;

  mem_object *n = ((wb_orepmem *)o)->memobject();
  while ( n->fth)
    ;

  orep = new wb_orepmem( (wb_vrepmem *)this, n);

  *sts = LDH__SUCCESS;
  return orep;
}

wb_orep *wb_vrepmem::parent(pwr_tStatus *sts, const wb_orep *o)
{
  wb_orepmem *orep = 0;
        
  if ( ((wb_orepmem *)o)->memobject()->fth) {
    orep = new wb_orepmem( (wb_vrepmem *)this, ((wb_orepmem *)o)->memobject()->fth);
    *sts = LDH__SUCCESS;
  }
  else
    *sts = LDH__NO_PARENT;
    
  return orep;
}

wb_orep *wb_vrepmem::after(pwr_tStatus *sts, const wb_orep *o)
{
  wb_orepmem *orep = 0;
    
  if ( ((wb_orepmem *)o)->memobject()->fws) {
    orep = new wb_orepmem( (wb_vrepmem *)this, ((wb_orepmem *)o)->memobject()->fws);
    *sts = LDH__SUCCESS;
  }
  else
    *sts = LDH__NO_SIBLING;
  return orep;
}

wb_orep *wb_vrepmem::before(pwr_tStatus *sts, const wb_orep *o)
{
  wb_orepmem *orep = 0;
    
  if ( ((wb_orepmem *)o)->memobject()->bws) {
    orep = new wb_orepmem( (wb_vrepmem *)this, ((wb_orepmem *)o)->memobject()->bws);
    *sts = LDH__SUCCESS;
  }
  else
    *sts = LDH__NO_SIBLING;
    
  return orep;
}

wb_orep *wb_vrepmem::first(pwr_tStatus *sts, const wb_orep *o)
{
  wb_orepmem *orep = 0;
    
  if ( ((wb_orepmem *)o)->memobject()->fch) {
    orep = new wb_orepmem( (wb_vrepmem *)this, ((wb_orepmem *)o)->memobject()->fch);
    *sts = LDH__SUCCESS;
  }
  else
    *sts = LDH__NO_CHILD;
    
  return orep;
}

wb_orep *wb_vrepmem::child(pwr_tStatus *sts, const wb_orep *o, wb_name &name)
{
  wb_orepmem *orep = 0;
    
  mem_object *m = ((wb_orepmem *)o)->memobject()->fch;
  while ( m) {
    if ( name.segmentIsEqual( m->name())) {
      orep = new wb_orepmem( (wb_vrepmem *)this, m);
      *sts = LDH__SUCCESS;
      break;
    }
    m = m->fws;
  }
  if ( !orep)
    *sts = LDH__NO_CHILD;
    
  return orep;
}

wb_orep *wb_vrepmem::last(pwr_tStatus *sts, const wb_orep *o)
{
  wb_orepmem *orep = 0;

  mem_object *n = ((wb_orepmem *)o)->memobject()->get_lch();
    
  if ( n) {
    orep = new wb_orepmem( (wb_vrepmem *)this, n);
    *sts = LDH__SUCCESS;
  }
  else
    *sts = LDH__NO_CHILD;
    
  return orep;
}

wb_orep *wb_vrepmem::next(pwr_tStatus *sts, const wb_orep *o)
{
  mem_object *mem = findObject( o->oid().oix);
  if ( mem) {
    pwr_tOix oix = mem->m_oid.oix;
    mem_object *next = root_object->next( mem->m_cid, &oix);
    if ( next) {
      wb_orepmem *orep = new wb_orepmem( this, next);
      return orep;
    }
  }
  *sts = LDH__NOSUCHOBJ;
  return 0;
}

wb_orep *wb_vrepmem::previous(pwr_tStatus *sts, const wb_orep *o)
{
  return 0;
}

void wb_vrepmem::objectName(const wb_orep *o, char *str)
{
  *str = 0;
        
  // Count ancestors
  int cnt = 0;
  mem_object *n = ((wb_orepmem *)o)->memobject();
  while ( n) {
    cnt++;
    n = n->fth;
  }

  mem_object **vect = (mem_object **) calloc( cnt, sizeof(vect));

  n = ((wb_orepmem *)o)->memobject();
  for ( int i = 0; i < cnt; i++) {
    vect[i] = n;
    n = n->fth;
  }

  strcat( str, name());
  strcat( str, ":");
  for ( int i = cnt - 1; i >= 0; i--) {
    strcat( str, vect[i]->name());
    if ( i != 0)
      strcat( str, "-");
  }
  free( vect);
}

bool wb_vrepmem::writeAttribute(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p)
{
  pwr_tTime time;

  *sts = LDH__SUCCESS;
  clock_gettime(CLOCK_REALTIME, &time);

    
  mem_object *n = ((wb_orepmem *) o)->memobject();

  switch ( bix) {
  case pwr_eBix_rt:
    if ( n->rbody_size == 0) {
      *sts = LDH__NOSUCHBODY;
      return false;
    }
    memcpy( (char *)n->rbody + offset, p, MIN(n->rbody_size - offset, size));
    n->m_rbtime = time;
    return true;
  case pwr_eBix_dev:
    if ( n->dbody_size == 0) {
      *sts = LDH__NOSUCHBODY;
      return false;
    }
    memcpy( (char *)n->dbody + offset, p, MIN(n->dbody_size - offset, size));
    n->m_dbtime = time;
    return true;
  default:
    *sts = LDH__NOSUCHBODY;
    return false;
  }
}

void *wb_vrepmem::readAttribute(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p)
{
  *sts = LDH__SUCCESS;
    
  mem_object *n = ((wb_orepmem *) o)->memobject();

  switch ( bix) {
  case pwr_eBix_rt:
    if ( n->rbody_size == 0) {
      *sts = LDH__NOSUCHBODY;
      return 0;
    }
    if ( p) {
      memcpy( p, (char *)n->rbody + offset, MIN(n->rbody_size - offset, size));
      return p;
    }
    return (void *)((char *)n->rbody + offset);
  case pwr_eBix_dev:
    if ( n->dbody_size == 0) {
      *sts = LDH__NOSUCHBODY;
      return 0;
    }
    if ( p) {
      memcpy( p, (char *)n->dbody + offset, MIN(n->dbody_size - offset, size));
      return p;
    }
    return (void *)((char *)n->dbody + offset);
  default:
    *sts = LDH__NOSUCHBODY;
    return 0;
  }
}

bool wb_vrepmem::writeBody(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, void *p)
{
  pwr_tTime time;

  *sts = LDH__SUCCESS;
  clock_gettime(CLOCK_REALTIME, &time);
    
  mem_object *n = ((wb_orepmem *) o)->memobject();

  switch ( bix) {
  case pwr_eBix_rt:
    if ( n->rbody_size == 0) {
      *sts = LDH__NOSUCHBODY;
      return false;
    }

    memcpy( n->rbody, p, n->rbody_size);
    n->m_rbtime = time;
    return true;
  case pwr_eBix_dev:
    if ( n->dbody_size == 0) {
      *sts = LDH__NOSUCHBODY;
      return false;
    }
    memcpy( n->dbody, p, n->dbody_size);
    n->m_dbtime = time;
    return true;
  default:
    *sts = LDH__NOSUCHBODY;
    return false;
  }
}

void *wb_vrepmem::readBody(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, void *p)
{
  *sts = LDH__SUCCESS;
    
  mem_object *n = ((wb_orepmem *) o)->memobject();

  switch ( bix) {
  case pwr_eBix_rt:
    if ( n->rbody_size == 0) {
      *sts = LDH__NOSUCHBODY;
      return 0;
    }
    if ( p) {
      memcpy( p, n->rbody, n->rbody_size);
      return p;
    }
    return n->rbody;
  case pwr_eBix_dev:
    if ( n->dbody_size == 0) {
      *sts = LDH__NOSUCHBODY;
      return 0;
    }
    if ( p) {
      memcpy( p, n->dbody, n->dbody_size);
      return p;
    }
    return n->dbody;
  default:
    *sts = LDH__NOSUCHBODY;
    return 0;
  }
}


mem_object *mem_object::find( wb_name *oname, int level)
{
  if ( oname->segmentIsEqual( m_name, level)) {
    if ( !oname->hasSegment(level+1))
      return this;
    else if ( fch)
      return fch->find( oname, level+1);
    else
      return 0;
  }
  else if ( fws)
    return fws->find( oname, level);
  else
    return 0;
  return 0;
}

void wb_vrepmem::clear()
{
  // Remove all mem objects
  if ( root_object)
    freeObject( root_object);
  root_object = 0;

  if ( volume_object)
    freeObject( volume_object);
  volume_object = 0;
}

void wb_vrepmem::freeObject( mem_object *memo)
{
  // Free all children and siblings
  if ( memo->fch)
    freeObject( memo->fch);
  if ( memo->fws)
    freeObject( memo->fws);

  unregisterObject( memo->m_oid.oix);
  delete memo;
}

wb_orep *wb_vrepmem::createObject(pwr_tStatus *sts, wb_cdef cdef, wb_destination &d, wb_name &name)
{
  mem_object *dest;
  ldh_eDest code = d.code();
  char name_str[32];
  pwr_tOix oix;
  pwr_tTime time;

  clock_gettime(CLOCK_REALTIME, &time);

  if ( d.oid().oix == 0) {
    dest = root_object;
    if ( code == ldh_eDest_After)
      code = ldh_eDest_IntoLast;
    if ( code == ldh_eDest_Before)
      code = ldh_eDest_IntoFirst;
  }
  else {
    dest = findObject( d.oid().oix);
    if ( !dest) {
      *sts = LDH__BADDEST;
      return 0;
    }
    // Check that name is unic
    if ( name && !nameCheck( dest, name.segment(), code)) {
      *sts = LDH__NAMALREXI;
      return 0;
    }
  }

  if ( !m_classeditor) {
    oix = nextOix();
    if ( name.evenSts())
      sprintf( name_str, "O%u", oix);
    else
      strcpy( name_str, name.object());
  }
  else {
    if ( !classeditorCheck( code, dest, cdef.cid(), &oix, name_str, sts, false))
      return 0;

    if ( name.oddSts())
      strcpy( name_str, name.object());
  }


  mem_object *memo = new mem_object();
  strcpy( memo->m_name, name_str);
  memo->m_oid.oix = oix;
  memo->m_oid.vid = m_vid;
  memo->m_cid = cdef.cid();
  memo->m_flags = cdef.flags();
  memo->m_ohtime = time;
  memo->m_created = 1;
  memo->rbody_size = cdef.size( pwr_eBix_rt);
  if ( memo->rbody_size) {
    memo->m_rbtime = time;
    memo->rbody = malloc( memo->rbody_size);
    cdef.templateBody( sts, pwr_eBix_rt, memo->rbody, memo->m_oid);
    if ( EVEN(*sts)) return 0;
  }
  memo->dbody_size = cdef.size( pwr_eBix_dev);
  if ( memo->dbody_size) {
    memo->m_dbtime = time;
    memo->dbody = malloc( memo->dbody_size);
    cdef.templateBody( sts, pwr_eBix_dev, memo->dbody, memo->m_oid);
    if ( EVEN(*sts)) return 0;
  }

  if ( !root_object) {
    // This is the root object
    root_object = memo;
  }
  else {
    switch ( code) {
    case ldh_eDest_IntoFirst:
      memo->fws = dest->fch;
      if ( dest->fch)
	dest->fch->bws = memo;
      dest->fch = memo;
      memo->fth = dest;
      break;
    case ldh_eDest_IntoLast:
      {
	mem_object *lch = dest->get_lch();
	if ( lch)
	  lch->fws = memo;
	else
	  dest->fch = memo;
	memo->bws = lch;
	memo->fth = dest;
	break;
      }
    case ldh_eDest_After:
      memo->fws = dest->fws;
      memo->bws = dest;
      if ( dest->fws)
	dest->fws->bws = memo;
      dest->fws = memo;
      memo->fth = dest->fth;
      break;
    case ldh_eDest_Before:
      memo->bws = dest->bws;
      memo->fws = dest;
      memo->fth = dest->fth;
      if ( dest->bws)
	dest->bws->fws = memo;
      else if ( dest->fth)
	dest->fth->fch = memo;
      dest->bws = memo;
      if ( dest == root_object)
	root_object = memo;
      break;
    default:
      *sts = LDH__NODEST;
      delete memo;
      return 0;
    }
  }
  registerObject( memo->m_oid.oix, memo);

  wb_orepmem *o = new wb_orepmem( this, memo);
  return o;
}

bool wb_vrepmem::createVolumeObject( char *name)
{
  pwr_tStatus sts;

  wb_cdrep *cdrep = m_merep->cdrep( &sts, m_cid);
  wb_cdef cdef = wb_cdef( cdrep);

  mem_object *memo = new mem_object();
  strcpy( memo->m_name, name);
  memo->m_oid.vid = m_vid;
  memo->m_oid.oix = 0;
  memo->m_cid = m_cid;
  memo->m_flags = cdef.flags();
  memo->rbody_size = cdef.size( pwr_eBix_rt);
  if ( memo->rbody_size) {
    memo->rbody = malloc( memo->rbody_size);
    cdef.templateBody( &sts, pwr_eBix_rt, memo->rbody, memo->m_oid);
    if ( EVEN(sts)) return false;
  }
  memo->dbody_size = cdef.size( pwr_eBix_dev);
  if ( memo->dbody_size) {
    memo->dbody = malloc( memo->dbody_size);
    cdef.templateBody( &sts, pwr_eBix_dev, memo->dbody, memo->m_oid);
    if ( EVEN(sts)) return false;
  }

  volume_object = memo;
  strcpy( volume_name, name);
  registerObject( memo->m_oid.oix, memo);
  return true;
}


wb_orep *wb_vrepmem::copyObject(pwr_tStatus *sts, const wb_orep *orep, wb_destination &d, wb_name &name)
{
  mem_object *dest;
  ldh_eDest code = d.code();
  char name_str[32];
  pwr_tTime time;

  clock_gettime(CLOCK_REALTIME, &time);

  if ( cdh_ObjidIsNull( d.oid())) {
    dest = root_object;
    if ( code == ldh_eDest_After)
      code = ldh_eDest_IntoLast;
    if ( code == ldh_eDest_Before)
      code = ldh_eDest_IntoFirst;
  }
  else {
    dest = findObject( d.oid().oix);
    if ( !dest) {
      *sts = LDH__BADDEST;
      return 0;
    }
  }

  pwr_tOix oix;
  if ( !m_classeditor) {
    oix = nextOix();
    if ( name.evenSts())
      strcpy( name_str, orep->name());
    else
      strcpy( name_str, name.object());
  }
  else {
    if ( strcmp( orep->name(), "Template") == 0) {
      *sts = LDH__CLASSMISPLACED;
      return 0;
    }
    if ( !classeditorCheck( code, dest, orep->cid(), &oix, name_str, sts, false))
      return 0;

    if ( name.oddSts())
      strcpy( name_str, name.object());
  }

  mem_object *memo = new mem_object();
  strcpy( memo->m_name, name_str);
  memo->m_oid.oix = oix;
  memo->m_oid.vid = m_vid;
  memo->m_cid = orep->cid();
  memo->m_ohtime = time;
  memo->m_created = 1;

  wb_attribute rbody;
  rbody = wb_attribute( LDH__SUCCESS, (wb_orep *)orep, "RtBody");
  if ( !rbody)
    rbody = wb_attribute( LDH__SUCCESS, (wb_orep *)orep, "SysBody");
  if ( rbody) {
    memo->m_rbtime = time;
    memo->rbody_size = rbody.size();
    memo->rbody = malloc( memo->rbody_size);
    rbody.value( memo->rbody);
  }

  wb_attribute dbody;
  dbody = wb_attribute( LDH__SUCCESS, (wb_orep *)orep, "DevBody");
  if ( dbody) {
    memo->m_dbtime = time;
    memo->dbody_size = dbody.size();
    memo->dbody = malloc( memo->dbody_size);
    dbody.value( memo->dbody);
  }

  if ( !root_object) {
    // This is the root object
    root_object = memo;
  }
  else {
    switch ( code) {
    case ldh_eDest_IntoFirst:
      memo->fws = dest->fch;
      if ( dest->fch)
	dest->fch->bws = memo;
      dest->fch = memo;
      memo->fth = dest;
      break;
    case ldh_eDest_IntoLast:
      {
	mem_object *lch = dest->get_lch();
	if ( lch)
	  lch->fws = memo;
	else
	  dest->fch = memo;
	memo->bws = lch;
	memo->fth = dest;
	break;
      }
    case ldh_eDest_After:
      memo->fws = dest->fws;
      memo->bws = dest;
      if ( dest->fws)
	dest->fws->bws = memo;
      dest->fws = memo;
      memo->fth = dest->fth;
      break;
    case ldh_eDest_Before:
      memo->bws = dest->bws;
      memo->fws = dest;
      memo->fth = dest->fth;
      if ( dest->bws)
	dest->bws->fws = memo;
      else if ( dest->fth)
	dest->fth->fch = memo;
      dest->bws = memo;
      if ( dest == root_object)
	root_object = memo;
      break;
    default:
      *sts = LDH__NODEST;
      delete memo;
      return 0;
    }
  }
  if ( !nameCheck( memo)) {
    char str[80];
    sprintf( str, "O%u_%s", memo->m_oid.oix, memo->m_name);
    strncpy( memo->m_name, str, sizeof( memo->m_name));
    memo->m_name[sizeof(memo->m_name)-1] = 0;
  }

  registerObject( memo->m_oid.oix, memo);

  wb_orepmem *o = new wb_orepmem( this, memo);
  return o;
}

bool wb_vrepmem::moveObject(pwr_tStatus *sts, wb_orep *orep, wb_destination &d)
{
  mem_object *dest;
  ldh_eDest code = d.code();
  pwr_tTime time;

  clock_gettime(CLOCK_REALTIME, &time);

  if ( cdh_ObjidIsEqual( d.oid(), orep->oid()))
    return false;

  if ( cdh_ObjidIsNull( d.oid())) {
    dest = root_object;
    if ( code == ldh_eDest_After)
      code = ldh_eDest_IntoLast;
    if ( code == ldh_eDest_Before)
      code = ldh_eDest_IntoFirst;
  }
  else {
    dest = findObject( d.oid().oix);
    if ( !dest) {
      *sts = LDH__BADDEST;
      return false;
    }
  }

  mem_object *memo = ((wb_orepmem *)orep)->memobject();

  if ( m_classeditor && !classeditorCheckMove( memo, code, dest, sts))
    return false;


  // Remove from current position
  if ( memo == root_object) {
    if ( !memo->fws)
      return LDH__NODEST;
    root_object = memo->fws;
  }
  if ( !memo->bws && memo->fth)
    memo->fth->fch = memo->fws;
  else if ( memo->bws)
    memo->bws->fws = memo->fws;
  if ( memo->fws)
    memo->fws->bws = memo->bws;
  memo->fth = memo->bws = memo->fws = 0;

  // Insert in new position
  switch ( code) {
    case ldh_eDest_IntoFirst:
      memo->fws = dest->fch;
      if ( dest->fch)
	dest->fch->bws = memo;
      dest->fch = memo;
      memo->fth = dest;
      break;
    case ldh_eDest_IntoLast:
      {
	mem_object *lch = dest->get_lch();
	if ( lch)
	  lch->fws = memo;
	else
	  dest->fch = memo;
	memo->bws = lch;
	memo->fth = dest;
	break;
      }
    case ldh_eDest_After:
      memo->fws = dest->fws;
      memo->bws = dest;
      if ( dest->fws)
	dest->fws->bws = memo;
      dest->fws = memo;
      memo->fth = dest->fth;
      break;
    case ldh_eDest_Before:
      if ( dest == root_object)
	root_object = memo;
      memo->bws = dest->bws;
      memo->fws = dest;
      memo->fth = dest->fth;
      if ( dest->bws)
	dest->bws->fws = memo;
      else if ( dest->fth)
	dest->fth->fch = memo;
      dest->bws = memo;
      break;
    default:
      *sts = LDH__NODEST;
      delete memo;
      return false;
  }

  memo->m_ohtime = time;

  return true;
}

bool wb_vrepmem::deleteObject(pwr_tStatus *sts, wb_orep *orep)
{
  mem_object *memo = ((wb_orepmem *)orep)->memobject();
  if ( !memo) return false;

  if ( memo->fch) {
    *sts = LDH__HAS_CHILD;
    return false;
  }

  if ( m_classeditor)
    classeditorDeleteObject( memo);

  if ( memo == root_object)
    root_object = memo->fws;
     
  if ( !memo->bws && memo->fth)
    memo->fth->fch = memo->fws;
  if ( memo->bws)
    memo->bws->fws = memo->fws;
  if ( memo->fws)
    memo->fws->bws = memo->bws;

  unregisterObject( memo->m_oid.oix);
  delete memo;

  return true;
}

bool wb_vrepmem::deleteFamily(pwr_tStatus *sts, wb_orep *orep)
{
  mem_object *memo = ((wb_orepmem *)orep)->memobject();
  if ( !memo) return false;

  if ( memo == root_object)
    root_object = memo->fws;

  if ( !memo->bws && memo->fth)
    memo->fth->fch = memo->fws;
  if ( memo->bws)
    memo->bws->fws = memo->fws;
  if ( memo->fws)
    memo->fws->bws = memo->bws;

  deleteChildren( memo);

  unregisterObject( memo->m_oid.oix);
  delete memo;

  return true;
}

void wb_vrepmem::deleteChildren( mem_object *memo) 
{
  mem_object *o;
  mem_object *ch = memo->fch;
  while ( ch) {
    o = ch;
    ch = ch->fws;
    deleteChildren( o);
    unregisterObject( o->m_oid.oix);
    delete o;
  }
}

bool wb_vrepmem::renameObject(pwr_tStatus *sts, wb_orep *orep, wb_name &name) 
{
  pwr_tTime time;

  clock_gettime(CLOCK_REALTIME, &time);

  mem_object *memo = ((wb_orepmem *)orep)->memobject();
  if ( !memo) return false;

  char old_name[80];
  strcpy( old_name, memo->m_name);

  if ( m_classeditor && name.segmentIsEqual("Template")) {
    // Name "Template" is reserved
    *sts = LDH__BADNAME;
    return false;
  }

  if ( strlen( name.segment()) >= sizeof( memo->m_name)) {
    *sts = LDH__BADNAME;
    return false;
  }

  strcpy( memo->m_name, name.segment());
  if ( !nameCheck( memo)) {
    // Change back
    strcpy( memo->m_name, old_name);
    return LDH__NAMALREXI;
  }

  if ( m_classeditor)
    classeditorRenameObject( memo, old_name, name);

  memo->m_ohtime = time;

  *sts = LDH__SUCCESS;
  return true;
}

//
// Update oid and attrref attributes, reset extern references
//
bool wb_vrepmem::importTree( bool keepref)
{
  if ( !root_object)
    return true;

  pwr_tStatus sts;

  wb_orep *o = object( &sts);
  if ( EVEN(sts)) return false;

  while ( ODD(sts)) {
    o->ref();
    updateObject( o, keepref);

    wb_orep *prev = o;
    o = o->after( &sts);
    prev->unref();
  }
  return true;
}

bool wb_vrepmem::updateSubClass( wb_adrep *subattr, char *body, bool keepref)
{
  pwr_tStatus sts;
  pwr_tCid cid = subattr->subClass();
  wb_cdrep *cdrep = m_merep->cdrep( &sts, cid);
  wb_bdrep *bdrep = cdrep->bdrep( &sts, pwr_eBix_rt);
  if ( EVEN(sts)) return false;

  int subattr_elements = subattr->isArray() ? subattr->nElement() : 1;

  for ( int i = 0; i < subattr_elements; i++) {
    wb_adrep *adrep = bdrep->adrep( &sts);
    while ( ODD(sts)) {
      int elements = adrep->isArray() ? adrep->nElement() : 1;
      if ( adrep->isClass()) {
	updateSubClass( adrep, body + i * subattr->size() / subattr_elements + adrep->offset(),
			keepref);
      }
      else {
	switch ( adrep->type()) {
	case pwr_eType_Objid: {
	  pwr_tOid *oidp = (pwr_tOid *)(body + i * subattr->size() / subattr_elements + 
					adrep->offset());
	  for ( int j = 0; j < elements; j++) {
	    if ( oidp->vid == m_source_vid && findObject( oidp->oix))
	      oidp->vid = m_vid;
	    else if ( !keepref && 
		      !(oidp->vid == ldh_cPlcMainVolume || 
			oidp->vid == ldh_cPlcFoVolume || 
			oidp->vid == ldh_cIoConnectVolume))
	      *oidp = pwr_cNOid;
	    oidp++;
	  }
	  break;
	}
	case pwr_eType_AttrRef: {
	  pwr_sAttrRef *arp = (pwr_sAttrRef *)(body + i * subattr->size() / subattr_elements + 
					adrep->offset());
	  for ( int j = 0; j < elements; j++) {
	    if ( arp->Objid.vid == m_source_vid && findObject( arp->Objid.oix))
	      arp->Objid.vid = m_vid;
	    else if ( !keepref && 
		      !(arp->Objid.vid == ldh_cPlcMainVolume || 
			arp->Objid.vid == ldh_cPlcFoVolume || 
			arp->Objid.vid == ldh_cIoConnectVolume))
	      arp->Objid = pwr_cNOid;
	    arp++;
	  }
	  break;
	}
	case pwr_eType_DataRef: {
	  pwr_tDataRef *drp = (pwr_tDataRef *)(body + i * subattr->size() / subattr_elements + 
					adrep->offset());
	  for ( int j = 0; j < elements; j++) {
	    if ( drp->Aref.Objid.vid == m_source_vid && findObject( drp->Aref.Objid.oix))
	      drp->Aref.Objid.vid = m_vid;
	    else if ( !keepref && 
		      !(drp->Aref.Objid.vid == ldh_cPlcMainVolume || 
			drp->Aref.Objid.vid == ldh_cPlcFoVolume || 
			drp->Aref.Objid.vid == ldh_cIoConnectVolume))
	      drp->Aref.Objid = pwr_cNOid;
	    drp++;
	  }
	  break;
	}
	default:
	  ;
	}
      }
      wb_adrep *prev = adrep;
      adrep = adrep->next( &sts);
      delete prev;
    }
  }
  delete bdrep;
  delete cdrep;

  return true;
}

bool wb_vrepmem::updateObject( wb_orep *o, bool keepref)
{
  pwr_tStatus sts;
  wb_cdrep *cdrep = m_merep->cdrep( &sts, o->cid());
  if ( EVEN(sts)) return false;
  pwr_mClassDef flags = cdrep->flags();

  for ( int i = 0; i < 2; i++) {
    pwr_eBix bix = i ? pwr_eBix_rt : pwr_eBix_dev;

    wb_bdrep *bdrep = cdrep->bdrep( &sts, bix);
    if ( EVEN(sts)) continue;

    char *body = (char *) readBody( &sts, o, bix, 0);

    wb_adrep *adrep = bdrep->adrep( &sts);
    while ( ODD(sts)) {
      int elements = adrep->isArray() ? adrep->nElement() : 1;
      if ( adrep->isClass()) {
	updateSubClass( adrep, body + adrep->offset(), keepref);
      }
      else {
	switch ( adrep->type()) {
	case pwr_eType_Objid: {
	  pwr_tOid *oidp = (pwr_tOid *)(body + adrep->offset());
	  for ( int j = 0; j < elements; j++) {
	    if ( oidp->vid == m_source_vid && findObject( oidp->oix))
	      // Intern reference
	      oidp->vid = m_vid;
	    else if ( !keepref &&
		      !(oidp->vid == ldh_cPlcMainVolume || 
			oidp->vid == ldh_cPlcFoVolume || 
			oidp->vid == ldh_cIoConnectVolume))
	      *oidp = pwr_cNOid;
	    oidp++;
	  }
	  break;
	}
	case pwr_eType_AttrRef: {
	  pwr_sAttrRef *arp = (pwr_sAttrRef *)(body + adrep->offset());
	  for ( int j = 0; j < elements; j++) {
	    if ( arp->Objid.vid == m_source_vid && findObject( arp->Objid.oix))
	      arp->Objid.vid = m_vid;
	    else if ( !keepref && 
		      !(arp->Objid.vid == ldh_cPlcMainVolume || 
			arp->Objid.vid == ldh_cPlcFoVolume || 
			arp->Objid.vid == ldh_cIoConnectVolume))
	      arp->Objid = pwr_cNOid;
	    arp++;
	  }
	  break;
	}
	case pwr_eType_DataRef: {
	  pwr_tDataRef *drp = (pwr_tDataRef *)(body + adrep->offset());
	  for ( int j = 0; j < elements; j++) {
	    if ( drp->Aref.Objid.vid == m_source_vid && findObject( drp->Aref.Objid.oix))
	      drp->Aref.Objid.vid = m_vid;
	    else if ( !keepref && 
		      !(drp->Aref.Objid.vid == ldh_cPlcMainVolume || 
			drp->Aref.Objid.vid == ldh_cPlcFoVolume || 
			drp->Aref.Objid.vid == ldh_cIoConnectVolume))
	      drp->Aref.Objid = pwr_cNOid;
	    drp++;
	  }
	  break;
	}
	default:
	  ;
	}
      }
      wb_adrep *prev = adrep;
      adrep = adrep->next( &sts);
      delete prev;
    }
    delete bdrep;
  }

  delete cdrep;

  wb_orep *child = o->first( &sts);
  while ( ODD(sts)) {
    child->ref();
    updateObject( child, keepref);

    wb_orep *prev = child;
    child = child->after( &sts);
    prev->unref();
  }
  return true;
}

bool wb_vrepmem::importTreeObject(wb_merep *merep, pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
				  pwr_tOid boid, const char *name, pwr_mClassDef flags,
				  size_t rbSize, size_t dbSize, void *rbody, void *dbody)
{
  pwr_tStatus sts;
  mem_object *memo = new mem_object();
  strcpy( memo->m_name, name);
  memo->m_oid.oix = oid.oix;
  memo->m_oid.vid = m_vid;
  memo->m_cid = cid;
  memo->m_flags = flags;

  bool class_error = false;
  bool convert = false;
  if ( merep && merep != m_merep) {
    // Check if class version differs
    wb_cdrep *cdrep_import = m_merep->cdrep( &sts, cid);
    if ( EVEN(sts)) {
      if ( m_ignore) {
	memo->m_cid = pwr_eClass_ClassLost;
	class_error = true;
      }
      else
	throw wb_error (sts);
    }

    wb_cdrep *cdrep_export = merep->cdrep( &sts, memo->m_cid);
    if ( EVEN(sts)) {
      if ( m_ignore) {
	memo->m_cid = pwr_eClass_ClassLost;
	class_error = true;
      }
      else
	throw wb_error (sts);
    }

    if ( !class_error &&
	 cdrep_import->ohTime().tv_sec != cdrep_export->ohTime().tv_sec) {
      convert = true;

      cdrep_import->convertObject( merep, rbody, dbody, 
				   &memo->rbody_size, &memo->dbody_size,
				   &memo->rbody, &memo->dbody);
    }
    delete cdrep_import;
    delete cdrep_export;
  }
  if ( !convert) {
    memo->rbody_size = rbSize;
    if ( memo->rbody_size) {
      memo->rbody = malloc( memo->rbody_size);
      memcpy( memo->rbody, rbody, memo->rbody_size);
    }
    memo->dbody_size = dbSize;
    if ( memo->dbody_size) {
      memo->dbody = malloc( memo->dbody_size);
      memcpy( memo->dbody, dbody, memo->dbody_size);
    }
  }
  if ( cdh_ObjidIsNull( poid) && cdh_ObjidIsNull( boid)) {
    // This is a top object
    if ( !root_object) {
      root_object = memo;
      m_source_vid = oid.vid;
    }
    else {
      // Insert as last sibling to rootobject
      mem_object *next = root_object;
      while( next->fws)
	next = next->fws;

      next->fws = memo;
      memo->bws = next;

      if ( !nameCheck( memo)) {
	char str[80];
	sprintf( str, "O%u_%s", memo->m_oid.oix, memo->m_name);
	strncpy( memo->m_name, str, sizeof( memo->m_name));
	memo->m_name[sizeof(memo->m_name)-1] = 0;
      }
    }
  }
  else if ( cdh_ObjidIsNotNull( boid)) {
    // Insert as next sibling to boid
    mem_object *bws = findObject( boid.oix);
    if ( !bws) {
      delete memo;
      throw wb_error(LDH__MEMINCON);
    }
    memo->bws = bws;
    memo->fws = bws->fws;
    memo->fth = bws->fth;
    if ( bws->fws)
      bws->fws->bws = memo;
    bws->fws = memo;
  }
  else {
    // Insert as first child to poid
    mem_object *fth = findObject( poid.oix);
    if ( !fth) {
      delete memo;
      throw wb_error(LDH__MEMINCON);
    }
    memo->fth = fth;
    memo->fws = fth->fch;
    if ( fth->fch)
      fth->fch->bws = memo;
    fth->fch = memo;
  }
  registerObject( memo->m_oid.oix, memo);

  return true;
}

bool wb_vrepmem::importPasteObject(pwr_tOid destination, ldh_eDest destcode, 
				   bool keepoid, pwr_tOid oid, 
				   pwr_tCid cid, pwr_tOid poid,
				   pwr_tOid boid, const char *name, pwr_mClassDef flags,
				   size_t rbSize, size_t dbSize, void *rbody, void *dbody,
				   pwr_tOid *roid)
{
  pwr_tStatus sts;
  pwr_tTime time;

  clock_gettime(CLOCK_REALTIME, &time);

  mem_object *memo = new mem_object();
  strcpy( memo->m_name, name);

  if ( keepoid) {
    mem_object *o = findObject( oid.oix);
    if ( !o)
      memo->m_oid.oix = oid.oix;
    else
      memo->m_oid.oix = nextOix();
  }      
  else {
    if ( !m_classeditor) {
      memo->m_oid.oix = nextOix();
    }
    else {
      mem_object *pmemo;

      if ( cdh_ObjidIsNull( poid) && cdh_ObjidIsNull( boid)) {
	// Root object
	mem_object *dest = findObject( destination.oix);
	if ( !dest)
	  throw wb_error(LDH__BADDEST);

	switch ( destcode) {
	case ldh_eDest_After:
	  pmemo = dest->fth;
	  break;
	case ldh_eDest_IntoFirst:
	  pmemo = dest;
	  break;
	default:
	  throw wb_error(LDH__NYI);
	}
      }
      else if ( cdh_ObjidIsNull( poid)) {
	
	pwr_tOix boix = importTranslate( boid.oix);
	pmemo = findObject( boix);
	if ( pmemo)
	  pmemo = pmemo->fth;
      }
      else {
	pwr_tOix poix = importTranslate( poid.oix);
	pmemo = findObject( poix);
	if ( strcmp( name, "Template") == 0 && 
	     pmemo->m_cid == pwr_eClass_ClassDef &&
	     !cdh_ObjidIsNull( boid)) {
	  // Unable to paste a tempate object correctly, remove it
	  delete memo;

	  // Link any forward sibling to backward sibling
	  pwr_tOix boix = importTranslate( boid.oix);
	  importTranslationTableInsert( oid.oix, boix);
	  return true;
	}
      }
      memo->fth = pmemo;
      if ( !classeditorCheck( ldh_eDest_IntoLast, pmemo, cid, &memo->m_oid.oix, 
			      memo->m_name, &sts, true))
	return 0;
    }
  }

  if ( cdh_ObjidIsNotNull( boid)) {
    boid.oix = importTranslate( boid.oix);
    if ( !boid.oix) throw wb_error(LDH__PASTEINCON);
  }
  if ( cdh_ObjidIsNotNull( poid)) {
    poid.oix = importTranslate( poid.oix);
    if ( !poid.oix) throw wb_error(LDH__PASTEINCON);
  }

  memo->m_oid.vid = m_vid;
  memo->m_cid = cid;
  memo->m_flags = flags;
  memo->m_ohtime = time;
  memo->rbody_size = rbSize;
  if ( memo->rbody_size) {
    memo->m_rbtime = time;
    memo->rbody = malloc( memo->rbody_size);
    memcpy( memo->rbody, rbody, memo->rbody_size);
  }
  memo->dbody_size = dbSize;
  if ( memo->dbody_size) {
    memo->m_dbtime = time;
    memo->dbody = malloc( memo->dbody_size);
    memcpy( memo->dbody, dbody, memo->dbody_size);
  }

  if ( cdh_ObjidIsNull( poid) && cdh_ObjidIsNull( boid)) {
    // This is the top object
    importTranslationTableClear();
    importSetSourceVid( oid.vid);

    if ( !root_object) {
      if ( cdh_ObjidIsNull( destination))
        root_object = memo;
      else
	throw wb_error(LDH__PASTEINCON);
    }
    else {
      // Insert as next sibling or first child to destination object
      mem_object *dest = findObject( destination.oix);
      if ( !dest)
	throw wb_error(LDH__BADDEST);

      switch ( destcode) {
      case ldh_eDest_After:
	memo->bws = dest;
	memo->fws = dest->fws;
	memo->fth = dest->fth;
	if ( dest->fws)
	  dest->fws->bws = memo;
	dest->fws = memo;
	break;
      case ldh_eDest_IntoFirst:
	memo->fth = dest;
	memo->fws = dest->fch;
	if ( memo->fch)
	  memo->fch->bws = memo;
	dest->fch = memo;
	break;
      default:
	throw wb_error(LDH__NYI);
      }
    }
  }
  else if ( cdh_ObjidIsNotNull( boid)) {
    // Insert as next sibling to boid
    mem_object *bws = findObject( boid.oix);
    if ( !bws) {
      delete memo;
      throw wb_error(LDH__PASTEINCON);
    }
    memo->bws = bws;
    memo->fws = bws->fws;
    memo->fth = bws->fth;
    if ( bws->fws)
      bws->fws->bws = memo;
    bws->fws = memo;
  }
  else {
    // Insert as first child to poid
    mem_object *fth = findObject( poid.oix);
    if ( !fth) {
      delete memo;
      throw wb_error(LDH__PASTEINCON);
    }
    memo->fth = fth;
    memo->fws = fth->fch;
    if ( fth->fch)
      fth->fch->bws = memo;
    fth->fch = memo;
  }

  if ( !nameCheck( memo)) {
    char str[80];
    sprintf( str, "O%u_%s", memo->m_oid.oix, memo->m_name);
    strncpy( memo->m_name, str, sizeof( memo->m_name));
    memo->m_name[sizeof(memo->m_name)-1] = 0;
  }

  registerObject( memo->m_oid.oix, memo);
  importTranslationTableInsert( oid.oix, memo->m_oid.oix);

  *roid = memo->m_oid;
  return true;
}

bool wb_vrepmem::importPaste()
{
  importUpdateTree( this);
  importTranslationTableClear();
  return true;
}

bool wb_vrepmem::exportTree(wb_treeimport &i, pwr_tOid oid)
{
  mem_object *memo = findObject( oid.oix);
  if ( !memo)
    throw wb_error(LDH__NOSUCHOBJ);

  memo->exportTree( i, true);
  return true;
}

bool wb_vrepmem::exportPaste(wb_treeimport &i, pwr_tOid destination, ldh_eDest destcode,
			     bool keepoid, pwr_tOid **rootlist)
{
  // Count number of topobjects
  int top_cnt = 0;
  for ( mem_object *top = root_object; top; top = top->fws)
    top_cnt++;
  
  *rootlist = (pwr_tOid *) calloc( top_cnt + 1, sizeof(pwr_tOid));

  if ( root_object) {
    root_object->exportPaste( i, destination, true, destcode, keepoid, *rootlist);
    i.importPaste();
  }
  return true;
}

//
// Check that the name of an object is unic within a sibling group
//
bool wb_vrepmem::nameCheck( mem_object *memo)
{
  // Get first
  mem_object *o = memo;
  while ( o->bws)
    o = o->bws;

  while ( o) {
    if ( o != memo && cdh_NoCaseStrcmp( memo->name(), o->name()) == 0)
      return false;
    o = o->fws;
  }
  return true;
}

bool wb_vrepmem::nameCheck( mem_object *dest, char *name, ldh_eDest code)
{
  mem_object *o;
  switch ( code) {
  case ldh_eDest_After:
  case ldh_eDest_Before:
    o = dest;
    while ( o->bws)
      o = o->bws;
    break;
  case ldh_eDest_IntoFirst:
  case ldh_eDest_IntoLast:
    if ( dest)
      o = dest->fch;
    else
      o = root_object;
    break;
  default:
    return false;
  }
  while ( o) {
    if ( cdh_NoCaseStrcmp( name, o->name()) == 0)
      return false;
    o = o->fws;
  }
  return true;
}

bool wb_vrepmem::importVolume( wb_export &e)
{

  e.exportHead( *this);
  e.exportDbody( *this);
  e.exportRbody( *this);
  e.exportDocBlock( *this);

  // Build volume
  if ( !volume_object)
    return false;

  if ( volume_object->fchoid.oix != 0)
    root_object = findObject( volume_object->fchoid.oix);
  if ( root_object)
    importBuildObject( root_object);

  switch( cid()) {
  case pwr_eClass_ClassVolume: {
    // If classvolume, insert itself into its merep
#if 0
    m_merep = new wb_merep( *m_erep->merep(), this);
    wb_mvrep *mvrep = m_merep->volume( &sts, vid());
    if ( ODD(sts))
      m_merep->removeDbs( &sts, mvrep);
    m_merep->addDbs( &sts, (wb_mvrep *)this);
    m_nRef--;
#endif
    m_nextOix = ((pwr_sClassVolume *)volume_object->rbody)->NextOix;
    break;
  }
  case pwr_eClass_RootVolume:
    m_nextOix = ((pwr_sRootVolume *)volume_object->rbody)->NextOix;
    break;
  case pwr_eClass_SubVolume:
    m_nextOix = ((pwr_sSubVolume *)volume_object->rbody)->NextOix;
    break;
  case pwr_eClass_DirectoryVolume:
    m_nextOix = ((pwr_sDirectoryVolume *)volume_object->rbody)->NextOix;
    break;
  case pwr_eClass_WorkBenchVolume:
    m_nextOix = ((pwr_sWorkBenchVolume *)volume_object->rbody)->NextOix;
    break;
  default:;
  }
  return true;
}

bool wb_vrepmem::importBuildObject( mem_object *memo)
{
  if ( memo->fthoid.oix)
    memo->fth = findObject( memo->fthoid.oix);
  if ( memo->fchoid.oix)
    memo->fch = findObject( memo->fchoid.oix);
  if ( memo->bwsoid.oix)
    memo->bws = findObject( memo->bwsoid.oix);
  if ( memo->fwsoid.oix)
    memo->fws = findObject( memo->fwsoid.oix);

  if ( memo->fch)
    importBuildObject( memo->fch);
  if ( memo->fws)
    importBuildObject( memo->fws);

  return true;
}

bool wb_vrepmem::importHead(pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
                        pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid, pwr_tOid loid,
                        const char *name, const char *normname, pwr_mClassDef flags,
                        pwr_tTime ohTime, pwr_tTime rbTime, pwr_tTime dbTime,
                        size_t rbSize, size_t dbSize)
{
    
  if (cdh_ObjidIsNull(oid))
    printf("** Error: object is null!\n");

  mem_object *memo = new mem_object();
  strcpy( memo->m_name, name);
  memo->m_oid = oid;
  memo->m_cid = cid;
  memo->m_flags = flags;
  memo->fthoid = poid;
  memo->bwsoid = boid;
  memo->fwsoid = aoid;
  memo->fchoid = foid;
  memo->m_ohtime = ohTime;
  memo->m_rbtime = rbTime;
  memo->m_dbtime = dbTime;

  if (oid.oix == pwr_cNOix) {
    // this is the volume object
    volume_object = memo;
    strcpy( volume_name, name);
    m_cid = cid;
  }
  registerObject( memo->m_oid.oix, memo);

  return true;
}

bool wb_vrepmem::importDbody(pwr_tOid oid, size_t size, void *body)
{

  mem_object *memo = findObject( oid.oix);
  if ( !memo)
    return false;

  memo->dbody_size = size;
  if ( memo->dbody_size) {
    memo->dbody = malloc( memo->dbody_size);
    memcpy( memo->dbody, body, size);
  }
  return true;
}

bool wb_vrepmem::importRbody(pwr_tOid oid, size_t size, void *body)
{

  mem_object *memo = findObject( oid.oix);
  if ( !memo)
    return false;

  memo->rbody_size = size;
  if ( memo->rbody_size) {
    memo->rbody = malloc( memo->rbody_size);
    memcpy( memo->rbody, body, size);
  }
  return true;
}


bool wb_vrepmem::importDocBlock(pwr_tOid oid, size_t size, char *block)
{

  mem_object *memo = findObject( oid.oix);
  if ( !memo)
    return false;

  memo->docblock_size = size;
  if ( memo->docblock_size) {
    memo->docblock = (char *) malloc( memo->docblock_size);
    memcpy( memo->docblock, block, size);
  }
  return true;
}


bool wb_vrepmem::commit(pwr_tStatus *sts) 
{
  pwr_tCmd cmd;

  if ( m_classeditor) {
    classeditorCommit();
  }

  // Play safe and save the previous file...
  sprintf( cmd, "pwrp_env.sh save file %s", m_filename);
  system( cmd);

  ofstream fp( m_filename);
  if ( !fp) {
    *sts = LDH__FILEOPEN;
    return false;
  }

  try {
    wb_volume vol(this);

    wb_print_wbl wprint( fp);
    wprint.printVolume( vol);
    if ( wprint.getErrCnt() != 0) {
      char str[400];
      sprintf( str, "Errors when saving volume: %d error%s found", wprint.getErrCnt(),
	       (wprint.getErrCnt() == 1) ? "" : "s");
      MsgWindow::message( 'E', str);
    }
  }
  catch ( wb_error& e) {
    *sts = e.sts();
    return false;
  }

  printPaletteFile();

  // Reload to get new template objects
  clear();
  loadWbl( m_filename, sts, true);

  return true;
}

bool wb_vrepmem::abort(pwr_tStatus *sts) 
{
  // Reload
  if ( m_classeditor) {
    clear();
    loadWbl( m_filename, sts, true);
  }

  return true;
}


void wb_vrepmem::classeditorRenameObject( mem_object *memo, char *oldname, 
					  wb_name &name)
{
  pwr_tStatus sts;

  switch ( memo->m_cid) {
  case pwr_eClass_ClassDef: {
    pwr_tCid cid = cdh_ClassObjidToId( memo->m_oid);
    
    try {
      wb_cdrep *cdrep = m_merep->cdrep( &sts, cid);
      if ( cdrep) {
	cdrep->renameClass( &sts, name);
	delete cdrep;
      }
    } catch ( wb_error &e) {
    }
    break;
  }
  case pwr_eClass_Param:
  case pwr_eClass_Intern:
  case pwr_eClass_Input:
  case pwr_eClass_Output:
  case pwr_eClass_ObjXRef:
  case pwr_eClass_AttrXRef:
  case pwr_eClass_Buffer: {
    if ( !memo->fth || !memo->fth->fth)
      break;
    pwr_tCid cid = cdh_ClassObjidToId( memo->fth->fth->m_oid);
    
    try {
      wb_cdrep *cdrep = m_merep->cdrep( &sts, cid);
      if ( !cdrep)
	break;

      wb_bdrep *bdrep = cdrep->bdrep( &sts, memo->fth->m_name);
      delete cdrep;
      if ( !bdrep)
	break;

      wb_adrep *adrep = bdrep->adrep( &sts, oldname);
      delete bdrep;
      if ( !adrep)
	break;
      
      adrep->renameAttribute( &sts, name);
      delete adrep;
    } catch ( wb_error &e) {
    }
    break;
  }
  case pwr_eClass_TypeDef: {
    pwr_tTid tid = cdh_TypeObjidToId( memo->m_oid);
    
    try {
      wb_tdrep *tdrep = m_merep->tdrep( &sts, tid);
      if ( tdrep) {
	tdrep->renameType( &sts, name);
	delete tdrep;
      }
    } catch ( wb_error &e) {
    }
    break;    
  }
  default: ;
  }
}

void wb_vrepmem::classeditorDeleteObject( mem_object *memo)
{

  switch ( memo->m_cid) {
  case pwr_eClass_ObjBodyDef:
  case pwr_eClass_Param:
  case pwr_eClass_Intern:
  case pwr_eClass_Input:
  case pwr_eClass_Output:
  case pwr_eClass_ObjXRef:
  case pwr_eClass_AttrXRef:
  case pwr_eClass_Buffer: {
    // Change ohtime in parent to get new class version
    if ( !memo->fth)
      break;

    pwr_tTime time;

    clock_gettime(CLOCK_REALTIME, &time);
    memo->fth->m_ohtime = time;
    break;
  }
  default: ;
  }
}

bool wb_vrepmem::classeditorCheck( ldh_eDest dest_code, mem_object *dest, pwr_tCid cid,
				   pwr_tOix *oix, char *name, pwr_tStatus *sts, 
				   bool import_paste)
{
  mem_object *fth;

  // Get father
  switch ( dest_code) {
  case ldh_eDest_After:
  case ldh_eDest_Before:
    fth = dest->fth;
    break;
  case ldh_eDest_IntoFirst:
  case ldh_eDest_IntoLast:
    fth = dest;
    break;
  default: 
    return false;
  }

  if ( fth) {
    switch ( fth->m_cid) {
    case pwr_eClass_ObjBodyDef:
      switch ( cid) {
      case pwr_eClass_Param:
      case pwr_eClass_Intern:
      case pwr_eClass_Input:
      case pwr_eClass_Output:
      case pwr_eClass_ObjXRef:
      case pwr_eClass_AttrXRef:
      case pwr_eClass_Buffer:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    case pwr_eClass_ClassDef:
      switch ( cid) {
      case pwr_eClass_ObjBodyDef:
      case pwr_eClass_GraphPlcNode:
      case pwr_eClass_GraphPlcConnection:
      case pwr_eClass_GraphPlcWindow:
      case pwr_eClass_GraphPlcProgram:
      case pwr_eClass_RtMenu:
      case pwr_eClass_Menu:
      case pwr_eClass_RtMethod:
      case pwr_eClass_Method:
      case pwr_eClass_DbCallBack:
      case pwr_cClass_PlcTemplate:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    case pwr_eClass_ClassHier:
      switch ( cid) {
      case pwr_eClass_ClassDef:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    case pwr_eClass_TypeHier:
      switch ( cid) {
      case pwr_eClass_Type:
      case pwr_eClass_TypeDef:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    case pwr_eClass_TypeDef:
      switch ( cid) {
      case pwr_eClass_Bit:
      case pwr_eClass_Value:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    default: ;
    }
  }

  switch ( cid) {
  case pwr_eClass_ClassHier: {
    // Top object, named Class
    if ( fth) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }
    if ( !import_paste) {
      strcpy( name, "Class");
      if ( !nameCheck( dest, name, dest_code)) {
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
    }
    *oix = nextOix();
    break;
  }
  case pwr_eClass_TypeHier: {
    // Top object, named Type
    if ( fth) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }
    if ( !import_paste) {
      strcpy( name, "Type");
      if ( !nameCheck( dest, name, dest_code)) {
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
    }
    *oix = nextOix();
    break;
  }
  case pwr_eClass_ClassDef:
    // Child to ClassHier, oix from cix
    if ( !fth || fth->m_cid != pwr_eClass_ClassHier) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }

    // Get next cix
    if ( volume_object && volume_object->rbody) {
      if ( ((pwr_sClassVolume *)volume_object->rbody)->NextCix == 0)
	((pwr_sClassVolume *)volume_object->rbody)->NextCix++; 
      while ( 1) {
	*oix = cdh_cixToOix( ((pwr_sClassVolume *)volume_object->rbody)->NextCix++, 0, 0);
	if ( !findObject( *oix))
	  break;
      }
    }
    if ( !import_paste) 
      sprintf( name, "O%u", *oix);
    break;

  case pwr_eClass_TypeDef:
    // Child to TypeHier, oix from tix
    if ( !fth || fth->m_cid != pwr_eClass_TypeHier) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }

    // Get next tix
    if ( volume_object && volume_object->rbody) {
      if ( ((pwr_sClassVolume *)volume_object->rbody)->NextTix[0] == 0)
	((pwr_sClassVolume *)volume_object->rbody)->NextTix[0]++; 
      while ( 1) {
	*oix = cdh_tixToOix( 0, ((pwr_sClassVolume *)volume_object->rbody)->NextTix[0]++);
	if ( !findObject( *oix))
	  break;
      }
    }
    if ( !import_paste)
      sprintf( name, "O%u", *oix);
    break;

  case pwr_eClass_ObjBodyDef: {
    // Child to ClassDef, oix from bix, named RtBody or DevBody
    if ( !fth || fth->m_cid != pwr_eClass_ClassDef) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }

    if ( !import_paste) {
      bool rtbody_found = false;
      bool devbody_found = false;
      for ( mem_object *memo = fth->fch; memo; memo = memo->fws) {
	if ( memo->m_cid == pwr_eClass_ObjBodyDef) {
	  if ( cdh_oixToBix( memo->m_oid.oix) == pwr_eBix_rt)
	    rtbody_found = true;
	  else if ( cdh_oixToBix( memo->m_oid.oix) == pwr_eBix_dev)
	    devbody_found = true;
	}
      }
      if ( !rtbody_found) {
	*oix = cdh_cixToOix( cdh_oixToCix(fth->m_oid.oix), pwr_eBix_rt, 0);
	if ( findObject( *oix)) {
	  *sts = LDH__CLASSMISPLACED;
	  return false;
	}
	strcpy( name, "RtBody");
      }
      else if ( !devbody_found) {
	*oix = cdh_cixToOix( cdh_oixToCix(fth->m_oid.oix), pwr_eBix_dev, 0);
	if ( findObject( *oix)) {
	  *sts = LDH__CLASSMISPLACED;
	  return false;
	}
	strcpy( name, "DevBody");
      }
      else {
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
    }
    else {
      // Use the name to choose oix
      if ( strcmp( name, "DevBody") == 0)
	*oix = cdh_cixToOix( cdh_oixToCix(fth->m_oid.oix), pwr_eBix_dev, 0);
      else
	*oix = cdh_cixToOix( cdh_oixToCix(fth->m_oid.oix), pwr_eBix_rt, 0);
      if ( findObject( *oix)) {
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
    }
    break;
  }
  case pwr_eClass_Param:
  case pwr_eClass_Input:
  case pwr_eClass_Output:
  case pwr_eClass_Intern:
  case pwr_eClass_ObjXRef:
  case pwr_eClass_AttrXRef:
  case pwr_eClass_Buffer: {
    // Child to ObjBodyDef, oix from aix
    if ( !fth || fth->m_cid != pwr_eClass_ObjBodyDef) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }

    if ( ((pwr_sObjBodyDef *)fth->rbody)->NextAix == 0)
      ((pwr_sObjBodyDef *)fth->rbody)->NextAix++;

    while( 1) {
      *oix = cdh_cixToOix( cdh_oixToCix(fth->fth->m_oid.oix), cdh_oixToBix(fth->m_oid.oix),
			 ((pwr_sObjBodyDef *)fth->rbody)->NextAix++);
      if ( !findObject( *oix))
	break;
    }
    if ( !import_paste)
      sprintf( name, "O%u", *oix);
    break;
  }
  case pwr_cClass_PlcTemplate: {
    // Child to ClassDef, named Code
    if ( !fth || fth->m_cid != pwr_eClass_ClassDef) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }

    if ( !import_paste) {
      strcpy( name, "Code");
      if ( !nameCheck( dest, name, dest_code)) {
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
    }
    *oix = nextOix();
    break;
  }
  case pwr_eClass_GraphPlcNode: {
    // Child to ClassDef, named GraphPlcNode
    if ( !fth || fth->m_cid != pwr_eClass_ClassDef) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }

    if ( !import_paste) {
      strcpy( name, "GraphPlcNode");
      if ( !nameCheck( dest, name, dest_code)) {
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
    }
    *oix = nextOix();
    break;
  }
  default: {
    *oix = nextOix();
    if ( !import_paste)
      sprintf( name, "O%u", *oix);
  }
  }
  *sts = LDH__SUCCESS;
  return true;
}

bool wb_vrepmem::classeditorCheckMove( mem_object *memo, ldh_eDest dest_code, 
				       mem_object *dest, pwr_tStatus *sts)
{
  mem_object *fth;

  // Get father
  switch ( dest_code) {
  case ldh_eDest_After:
  case ldh_eDest_Before:
    fth = dest->fth;
    break;
  case ldh_eDest_IntoFirst:
  case ldh_eDest_IntoLast:
    fth = dest;
    break;
  default: 
    return false;
  }

  if ( fth) {
    switch ( fth->m_cid) {
    case pwr_eClass_ObjBodyDef:
      switch ( memo->m_cid) {
      case pwr_eClass_Param:
      case pwr_eClass_Intern:
      case pwr_eClass_Input:
      case pwr_eClass_Output:
      case pwr_eClass_ObjXRef:
      case pwr_eClass_AttrXRef:
      case pwr_eClass_Buffer:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    case pwr_eClass_ClassDef:
      switch ( memo->m_cid) {
      case pwr_eClass_ObjBodyDef:
      case pwr_eClass_GraphPlcNode:
      case pwr_eClass_GraphPlcConnection:
      case pwr_eClass_GraphPlcWindow:
      case pwr_eClass_GraphPlcProgram:
      case pwr_eClass_RtMenu:
      case pwr_eClass_Menu:
      case pwr_eClass_RtMethod:
      case pwr_eClass_Method:
      case pwr_eClass_DbCallBack:
      case pwr_cClass_PlcTemplate:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    case pwr_eClass_ClassHier:
      switch ( memo->m_cid) {
      case pwr_eClass_ClassDef:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    case pwr_eClass_TypeHier:
      switch ( memo->m_cid) {
      case pwr_eClass_Type:
      case pwr_eClass_TypeDef:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    case pwr_eClass_TypeDef:
      switch ( memo->m_cid) {
      case pwr_eClass_Bit:
      case pwr_eClass_Value:
	break;
      default:
	*sts = LDH__CLASSMISPLACED;
	return false;
      }
      break;
    default: ;
    }
  }

  switch ( memo->m_cid) {
  case pwr_eClass_ClassHier:
  case pwr_eClass_TypeHier: {
    // Top object
    if ( fth) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }
  }
  case pwr_eClass_ClassDef:
  case pwr_eClass_TypeDef: {
    if ( fth != memo->fth) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }
    break;
  }
  case pwr_eClass_ObjBodyDef:
  case pwr_eClass_Param:
  case pwr_eClass_Input:
  case pwr_eClass_Output:
  case pwr_eClass_Intern:
  case pwr_eClass_ObjXRef:
  case pwr_eClass_AttrXRef:
  case pwr_eClass_Buffer:
  case pwr_cClass_PlcTemplate:
  case pwr_eClass_GraphPlcNode: {
    if ( !fth || fth != memo->fth) {
      *sts = LDH__CLASSMISPLACED;
      return false;
    }
    break;
  }
  default: ;
  }
  *sts = LDH__SUCCESS;
  return true;
}

void wb_vrepmem::classeditorCommit()
{
  // Set 'newattribute' bit in flag in $Attribute objects to indicate that 
  // template object should be updated with defaultvalues.
  if ( !root_object)
    return;

  for ( mem_object *o1 = root_object->fch; o1; o1 = o1->fws) {
    for ( mem_object *o2 = o1->fch; o2; o2 = o2->fws) {
      for ( mem_object *o3 = o2->fch; o3; o3 = o3->fws) {
	switch ( o3->m_cid) {
	case pwr_eClass_Param:
	  if ( o3->m_created)
	    ((pwr_sParam *)o3->rbody)->Info.Flags |= PWR_MASK_NEWATTRIBUTE;
	  else
	    ((pwr_sParam *)o3->rbody)->Info.Flags &= ~PWR_MASK_NEWATTRIBUTE;
	  break;
	case pwr_eClass_Intern:
	  if ( o3->m_created)
	    ((pwr_sIntern *)o3->rbody)->Info.Flags |= PWR_MASK_NEWATTRIBUTE;
	  else
	    ((pwr_sIntern *)o3->rbody)->Info.Flags &= ~PWR_MASK_NEWATTRIBUTE;
	  break;
	default: ;
	}
      }
    }
  }
}

void wb_vrepmem::printPaletteFile()
{
  // Print new palette file
  pwr_tStatus psts;
  int menu_found = 0;
  int allclasses_found = 0;
  int palette_found = 0;
  PalFileMenu *menu = PalFile::config_tree_build( 0, pal_cLocalPaletteFile, 
					     pal_eNameType_All, "", 0);
  PalFileMenu *mp, *mp2, *mp3, *mp4;
  mem_object *memch, *memcd, *memgn;

  // Add menu "NavigatorPalette-AllClasses-'volumename' if not found
  for ( mp = menu; mp; mp = mp->next) {
    if ( mp->item_type == pal_eMenuType_Palette &&
	 cdh_NoCaseStrcmp( mp->title, "NavigatorPalette") == 0) {
      for ( mp2 = mp->child_list; mp2; mp2 = mp2->next) {
	if ( mp2->item_type == pal_eMenuType_Menu &&
	     cdh_NoCaseStrcmp( mp2->title, "AllClasses") == 0) {
	  for ( mp3 = mp2->child_list; mp3; mp3 = mp3->next) {
	    if ( mp3->item_type == pal_eMenuType_ClassVolume &&
		 cdh_NoCaseStrcmp( mp3->title, volume_name) == 0) {
	      menu_found = 1;
	      break;
	    }
	  }
	  allclasses_found = 1;
	  break;
	}
      }
      palette_found = 1;
      break;
    }
  }

  if ( !palette_found) {
    // Create palette
    mp = new PalFileMenu( "NavigatorPalette", pal_eMenuType_Palette, 0);
    mp->next = menu;
    menu = mp;
  }
  if ( !allclasses_found) {
    // Create volume menu
    mp2 = new PalFileMenu( "AllClasses", pal_eMenuType_Menu, mp);
    mp2->next = mp->child_list;
    mp->child_list = mp2;
  }
  if ( !menu_found) {
    // Create volume menu
    mp3 = new PalFileMenu( volume_name, pal_eMenuType_ClassVolume, mp2);
    mp3->next = mp2->child_list;
    mp2->child_list = mp3;
  }


  // Replace menu "PlcEditorPalette-'volumename'-* with function object classes
  menu_found = 0;
  palette_found = 0;
  for ( mp = menu; mp; mp = mp->next) {
    if ( mp->item_type == pal_eMenuType_Palette &&
	 cdh_NoCaseStrcmp( mp->title, "PlcEditorPalette") == 0) {
      for ( mp2 = mp->child_list; mp2; mp2 = mp2->next) {
	if ( mp2->item_type == pal_eMenuType_Menu &&
	     cdh_NoCaseStrcmp( mp2->title, volume_name) == 0) {
	  // Remove
	  PalFile::config_tree_free( mp2->child_list);
	  mp2->child_list = 0;
	  menu_found = 1;
	  break;
	}
      }
      palette_found = 1;
      break;
    }
  }

  if ( !palette_found) {
    // Create palette
    mp = new PalFileMenu( "PlcEditorPalette", pal_eMenuType_Palette, 0);
    mp->next = menu;
    menu = mp;
  }
  if ( !menu_found) {
    // Create volume menu
    mp2 = new PalFileMenu( volume_name, pal_eMenuType_Menu, mp);
    mp2->next = mp->child_list;
    mp->child_list = mp2;
  }

  for ( memch = root_object; memch; memch = memch->fws) {
    if ( memch->m_cid == pwr_eClass_ClassHier) {
      for ( memcd = memch->fch; memcd; memcd = memcd->fws) {
	for ( memgn = memcd->fch; memgn; memgn = memgn->fws) {
	  if ( memgn->m_cid == pwr_eClass_GraphPlcNode) {
	    // Add to menu
	    mp4 = mp3;
	    mp3 = new PalFileMenu( memcd->m_name, pal_eMenuType_Class, mp2);
	    if ( !mp2->child_list)
	      mp2->child_list = mp3;
	    else
	      mp4->next = mp3;
	    break;
	  }
	}
      }
      break;
    }
  }

  PalFile::config_tree_print( pal_cLocalPaletteFile, menu, &psts);
}

