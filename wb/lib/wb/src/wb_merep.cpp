/* 
 * Proview   $Id: wb_merep.cpp,v 1.37 2006-07-27 10:14:01 claes Exp $
 * Copyright (C) 2005 SSAB Oxelösund AB.
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
#include <stdlib.h>

#include "wb_merep.h"
#include "wb_cdrep.h"
#include "wb_bdrep.h"
#include "wb_erep.h"
#include "wb_tdrep.h"
#include "wb_vrepdb.h"
#include "wb_attrname.h"
#include "wb_ldh_msg.h"
#include "co_time.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include "co_msgwindow.h"

static int compCatt( tree_sTable *tp, tree_sNode *x, tree_sNode *y);

wb_merep::wb_merep( const wb_merep& x, wb_vrep *vrep) :
  m_mvrepdbs(x.m_mvrepdbs), m_erep(x.m_erep), m_vrep(vrep), m_catt_tt(0)
{
  for ( mvrep_iterator it = m_mvrepdbs.begin(); it != m_mvrepdbs.end(); it++)
    it->second->ref();
}

wb_merep::~wb_merep()
{
  pwr_tStatus sts;

  for ( mvrep_iterator it = m_mvrepdbs.begin(); it != m_mvrepdbs.end(); it++) {
    if ( it->second != m_vrep)
      it->second->unref();
  }

  if ( m_catt_tt)
    tree_DeleteTable( &sts, m_catt_tt);
}

wb_merep::wb_merep(const char *dirName, wb_erep *erep, wb_vrep *vrep) :
  m_erep(erep), m_vrep(vrep), m_catt_tt(0)
{
  DIR *dirp;
  struct dirent *dp;
  char *pos;
  int ipos;
  int ilen;
  char fileName[255];

  dirp = opendir(dirName);

  while (dirp) {
    errno = 0;
    if ((dp = readdir(dirp)) != NULL) {
      if ((pos = strstr(dp->d_name, ".dbs")) != 0) {
        ipos = (int)(pos - dp->d_name);
        ilen = strlen(dp->d_name);
        if ((ilen - ipos) == (sizeof(".dbs") - 1)) {
          dbs_sEnv env;
          dbs_sVolume volume;
          pwr_tStatus sts;
          char vname[32];

          sprintf(fileName, "%s/%s", dirName, dp->d_name);
          dbs_sEnv *ep = dbs_Open(&sts, &env, fileName);
          dbs_sVolume *vp = dbs_Volume(&sts, &volume, ep);
          cdh_ToLower(vname, vp->name);
          *rindex(dp->d_name, '.') = '\0';
          cdh_ToLower(dp->d_name, dp->d_name);
          dbs_Close(&sts, ep);
          if (strcmp(dp->d_name, vname) == 0) {
            try {
              wb_vrepdbs *vrep = new wb_vrepdbs(erep, this, fileName, vp->name, vp->vid, vp->cid);
	      vrep->load();
              addDbs(&sts, (wb_mvrep *)vrep);
              char buff[256];
              sprintf(buff, "Local class volume \"%s\" loaded from \"%s\", in data base %s", vp->name, fileName, dirName);
              MsgWindow::message( 'O', buff);
            }
            catch (wb_error& e) {
              MsgWindow::message( 'E', "Unable to open local class volume", fileName, e.what().c_str());
            }

          }
        }
      }
    } else {
      if (errno == 0) {
        closedir(dirp);
        break;
      }
      closedir(dirp);
      break;
    }
  }
}

// Get first volume
wb_mvrep *wb_merep::volume( pwr_tStatus *sts)
{
  mvrep_iterator it = m_mvrepdbs.begin();
  if ( it == m_mvrepdbs.end()) {
    *sts = LDH__NOSUCHVOL;
    return 0;
  }
  *sts = LDH__SUCCESS;
  return it->second;
}

wb_mvrep *wb_merep::volume(pwr_tStatus *sts, pwr_tVid vid)
{
  mvrep_iterator it = m_mvrepdbs.find( vid);
  if ( it == m_mvrepdbs.end()) {
    *sts = LDH__NOSUCHVOL;
    return 0;
  }
  *sts = LDH__SUCCESS;
  return it->second;
}

wb_mvrep *wb_merep::volume(pwr_tStatus *sts, const char *name)
{
  mvrep_iterator it;
  for ( it = m_mvrepdbs.begin(); it != m_mvrepdbs.end(); it++) {
    if ( cdh_NoCaseStrcmp( it->second->name(), name) == 0) {
      *sts = LDH__SUCCESS;
      return it->second;
    }
  }
  *sts = LDH__NOSUCHVOL;
  return 0;
}

void wb_merep::copyFiles(const char *dirName)
{
  mvrep_iterator it;
  for ( it = m_mvrepdbs.begin(); it != m_mvrepdbs.end(); it++) {
    wb_vrepdbs *dp = (wb_vrepdbs *)it->second;
    char cmd[512];
    sprintf(cmd, "cp %s %s", dp->fileName(), dirName);
    system(cmd);
  }
}

void wb_merep::copyFiles(const char *dirName, wb_merep *merep)
{
  mvrep_iterator it;

  for (it = merep->m_mvrepdbs.begin(); it != merep->m_mvrepdbs.end(); it++) {
    wb_vrepdbs *e_dp = (wb_vrepdbs *)it->second;

    mvrep_iterator i_it = m_mvrepdbs.find(e_dp->vid());

    if (i_it == m_mvrepdbs.end()) {
      char cmd[512];
      sprintf(cmd, "cp %s %s", e_dp->fileName(), dirName);
    
      system(cmd);
      char buff[256];
      char e_timbuf[32];
      time_AtoAscii(&e_dp->dbsenv()->vp->time, time_eFormat_NumDateAndTime, e_timbuf, sizeof(e_timbuf));
      sprintf(buff, "Global class volume \"%s\" [%s](%s), was copied to data base \"%s\"",
              e_dp->dbsenv()->vp->name, e_timbuf, e_dp->fileName(), dirName);
      MsgWindow::message('I', buff);
      
      continue;
    } else {
      wb_vrepdbs *i_dp = (wb_vrepdbs *)i_it->second;

      if (time_Acomp(&i_dp->dbsenv()->vp->time, &e_dp->dbsenv()->vp->time) == 0)
        continue;

      char cmd[512];
      sprintf(cmd, "cp %s %s", e_dp->fileName(), dirName);
    
      system(cmd);
      char buff[256];
      char e_timbuf[32];
      char i_timbuf[32];
      
      time_AtoAscii(&i_dp->dbsenv()->vp->time, time_eFormat_NumDateAndTime, i_timbuf, sizeof(i_timbuf));
      time_AtoAscii(&e_dp->dbsenv()->vp->time, time_eFormat_NumDateAndTime, e_timbuf, sizeof(e_timbuf));

      sprintf(buff, "Local class volume \"%s\" [%s], in data base \"%s\", was updated [%s]",
              i_dp->dbsenv()->vp->name, i_timbuf, dirName, e_timbuf);
      MsgWindow::message('I', buff);
    }
  }
}

bool wb_merep::compareMeta(const char *dbName, wb_merep *merep)
{
  mvrep_iterator it;
  int need_update = 0;

  for (it = m_mvrepdbs.begin(); it != m_mvrepdbs.end(); it++) {
    wb_vrepdbs *i_dp = (wb_vrepdbs *)it->second;
    char i_timbuf[32];
    char e_timbuf[32];
    char buff[256];

    mvrep_iterator e_it = merep->m_mvrepdbs.find(i_dp->vid());

    if (e_it == merep->m_mvrepdbs.end()) {
      time_AtoAscii(&i_dp->dbsenv()->vp->time, time_eFormat_NumDateAndTime, i_timbuf, sizeof(i_timbuf));
      sprintf(buff, "Local class volume \"%s\" [%s] (%s), in data base \"%s\", does not exist in global scope",
              i_dp->dbsenv()->vp->name, i_timbuf, i_dp->fileName(), dbName);
      MsgWindow::message('W', buff);
      continue;
    }

    wb_vrepdbs *e_dp = (wb_vrepdbs *)e_it->second;

    if (time_Acomp(&i_dp->dbsenv()->vp->time, &e_dp->dbsenv()->vp->time) == 0)
      continue;

    time_AtoAscii(&i_dp->dbsenv()->vp->time, time_eFormat_NumDateAndTime, i_timbuf, sizeof(i_timbuf));
    time_AtoAscii(&e_dp->dbsenv()->vp->time, time_eFormat_NumDateAndTime, e_timbuf, sizeof(e_timbuf));

    sprintf(buff, "Local class volume \"%s\" [%s] (%s), in data base \"%s\", can be updated [%s]",
            i_dp->dbsenv()->vp->name, i_timbuf, i_dp->fileName(), dbName, e_timbuf);
    MsgWindow::message('W', buff, msgw_ePop_No);
    need_update++;
  }
  
  for (it = merep->m_mvrepdbs.begin(); it != merep->m_mvrepdbs.end(); it++) {
    wb_vrepdbs *e_dp = (wb_vrepdbs *)it->second;
    mvrep_iterator i_it = m_mvrepdbs.find(e_dp->vid());

    if (i_it == m_mvrepdbs.end()) {
      char buff[256];
      char e_timbuf[32];
      time_AtoAscii(&e_dp->dbsenv()->vp->time, time_eFormat_NumDateAndTime, e_timbuf, sizeof(e_timbuf));
      sprintf(buff, "Global class volume \"%s\" [%s], does not exist in data base \"%s\"", e_dp->dbsenv()->vp->name, e_timbuf, dbName);
      MsgWindow::message('W', buff);
    }
  }
  if ( need_update)
    MsgWindow::message( 'W', "Classvolumes need update, execute 'Function->UpdateClasses'");
  
  return true;
}

wb_orep *wb_merep::object(pwr_tStatus *sts, pwr_tOid oid)
{
  wb_vrep *vrep = volume( sts, oid.vid);
  if ( EVEN(*sts)) return 0;

  return vrep->object( sts, oid);
}

void wb_merep::addDbs( pwr_tStatus *sts, wb_mvrep *mvrep)
{
  mvrep_iterator it = m_mvrepdbs.find( mvrep->vid());
  if ( it == m_mvrepdbs.end()) {
    // Look for vrep in erep list... TODO

    m_mvrepdbs[mvrep->vid()] = mvrep;
    if ( mvrep != m_vrep)
      mvrep->ref();
    *sts = LDH__SUCCESS;
  }
  else {
    *sts = LDH__VOLIDALREXI;
  }

}

void wb_merep::removeDbs(pwr_tStatus *sts, wb_mvrep *mvrep)
{
  mvrep_iterator it = m_mvrepdbs.find( mvrep->vid());
  if ( it == m_mvrepdbs.end()) {
    *sts = LDH__NOSUCHVOL;
    return;
  }
  if ( it->second != m_vrep)
    it->second->unref();
  m_mvrepdbs.erase( it);
  *sts = LDH__SUCCESS;
}

wb_cdrep *wb_merep::cdrep( pwr_tStatus *sts, const wb_orep& o)
{

  pwr_tVid vid = cdh_CidToVid(o.cid());
  mvrep_iterator it =  m_mvrepdbs.find( vid);
  if ( it == m_mvrepdbs.end()) {
    *sts = LDH__NOSUCHVOL;
    return 0;
  }
  *sts = LDH__SUCCESS;
  return it->second->cdrep( o);
}

wb_cdrep *wb_merep::cdrep( pwr_tStatus *sts, pwr_tCid cid)
{
  mvrep_iterator it =  m_mvrepdbs.find( cdh_CidToVid( cid));
  if ( it == m_mvrepdbs.end()) {
    *sts = LDH__NOSUCHVOL;
    return 0;
  }
  try {
    *sts = LDH__SUCCESS;
    return it->second->cdrep( cid);
  }
  catch ( wb_error& e) {
    *sts = e.sts();
    return 0;
  }
}

wb_cdrep *wb_merep::cdrep( pwr_tStatus *sts, wb_name name)
{
  wb_cdrep *cdrep;

  if ( name.hasVolume()) {
    wb_mvrep *mvrep = volume( sts, name.volume());
    if ( EVEN( *sts)) return 0;
    try {
      cdrep = new wb_cdrep( mvrep, name);
      *sts = LDH__SUCCESS;
      return cdrep;
    }
    catch ( wb_error& e) {
      *sts = e.sts();
      return 0;
    }
  }
  else {
    for ( mvrep_iterator it = m_mvrepdbs.begin(); it != m_mvrepdbs.end(); it++) {
      try {
        cdrep = new wb_cdrep( it->second, name);
        *sts = LDH__SUCCESS;
        return cdrep;
      }
      catch ( wb_error& e) {
        // Not found in this volume, try next
        continue;
      }
    }
  }
  // Not found
  *sts = LDH__NOCLASS;
  return 0;
}

wb_tdrep *wb_merep::tdrep( pwr_tStatus *sts, const wb_adrep& a)
{
  if ( m_erep && a.vrep() != m_vrep)
    // Fetch from other meta environment
    return m_erep->tdrep( sts, a);

  map<pwr_tVid, wb_mvrep*>::iterator it =  m_mvrepdbs.find( cdh_TidToVid(a.type()));
  if ( it == m_mvrepdbs.end()) {
    *sts = LDH__NOSUCHVOL;
    return 0;
  }
  *sts = LDH__SUCCESS;
  return it->second->tdrep( a);
}

wb_tdrep *wb_merep::tdrep( pwr_tStatus *sts, pwr_tTid tid)
{
  map<pwr_tVid, wb_mvrep*>::iterator it =  m_mvrepdbs.find( cdh_TidToVid( tid));
  if ( it == m_mvrepdbs.end()) {
    *sts = LDH__NOSUCHVOL;
    return 0;
  }
  *sts = LDH__SUCCESS;
  return it->second->tdrep( tid);
}

wb_tdrep *wb_merep::tdrep( pwr_tStatus *sts, wb_name name)
{
  wb_tdrep *tdrep;

  if ( name.hasVolume()) {
    wb_mvrep *mvrep = volume( sts, name.volume());
    if ( EVEN( *sts)) return 0;

    try {
      tdrep = new wb_tdrep( mvrep, name);
      *sts = LDH__SUCCESS;
      return tdrep;
    }
    catch ( wb_error& e) {
      *sts = e.sts();
      return 0;
    }
  }
  else {
    for ( mvrep_iterator it = m_mvrepdbs.begin(); it != m_mvrepdbs.end(); it++) {
      try {
        tdrep = new wb_tdrep( it->second, name);
        *sts = LDH__SUCCESS;
        return tdrep;
      }
      catch ( wb_error& e) {
        // Not found in this volume, try next
      }
    }
  }

  // Not found
  *sts = LDH__NOTYPE;
  return 0;
}

int wb_merep::getAttrInfoRec( wb_attrname *attr, pwr_eBix bix, pwr_tCid cid, size_t *size,
                              size_t *offset, pwr_tTid *tid, int *elements,
                              pwr_eType *type, int *flags, int level)
{
  pwr_tStatus sts;

  if ( level > 0)
    bix = pwr_eBix_rt;

  wb_cdrep *cd = cdrep( &sts, cid);
  if ( EVEN(sts)) return 0;

  wb_adrep *adrep;
  wb_bdrep *bd = 0;
  if ( bix == 0) {
    // Search in all bodies
    adrep = cd->adrep( &sts, attr->attributesAllTrue(level));
    if ( EVEN(sts)) { delete cd; return 0;}
  }
  else {
    // Search in specified body
    bd = cd->bdrep( &sts, bix);
    if ( EVEN(sts)) { delete cd; return 0;}

    adrep = bd->adrep( &sts, attr->attributesAllTrue(level));
    if ( EVEN(sts)) { delete cd; delete bd; return 0;}
  }

  if ( attr->hasAttrIndex(attr->attributes()-1)) {
    *size = adrep->size() / adrep->nElement();
    *offset += adrep->offset() + attr->attrIndex(attr->attributes()-1) * *size;
    *elements = 1;
  }
  else {
    *offset += adrep->offset();
    *size = adrep->size();
    *elements = adrep->nElement();
  }
  *tid = adrep->tid();
  *type = adrep->type();
  *flags = adrep->flags();

  delete cd;
  if ( bd) delete bd;
  delete adrep;
  return 1;
}

void wb_merep::classDependency( pwr_tStatus *sts, pwr_tCid cid,
                                pwr_tCid **lst, pwr_sAttrRef **arlst, int *cnt)
{
  *lst = 0;
  *arlst = 0;
  *cnt = 0;

  wb_cdrep *cd = cdrep( sts, cid);
  if ( !cd) return;

  wb_bdrep *bd = cd->bdrep( sts, pwr_eBix_rt);
  if ( !bd) {
    delete cd;
    return;
  }
  if ( bd->nAttribute() == 0) {
    delete cd;
    delete bd;
    return;
  }

  *lst = (pwr_tCid *) calloc( bd->nAttribute(), sizeof(pwr_tCid));
  *arlst = (pwr_sAttrRef *) calloc( bd->nAttribute(), sizeof(pwr_sAttrRef));

  *cnt = 0;
  wb_adrep *ad, *oad;
  for ( ad = bd->adrep( sts); ad;) {
    if ( cdh_tidIsCid( ad->tid())) {
      (*lst)[*cnt] = ad->tid();
      (*arlst)[*cnt] = ad->aref();
      (*cnt)++;
    }
    oad = ad;
    ad = ad->next( sts);
    delete oad;
  }
  delete cd;
  delete bd;
  *sts = LDH__SUCCESS;
}

void wb_merep::classVersion( pwr_tStatus *sts, pwr_tCid cid, pwr_tTime *time)
{

  wb_cdrep *cd = cdrep( sts, cid);
  if ( !cd) return;

  if ( cd->vtype() == ldh_eVolRep_Dbs) {
    // ohTime contains class version for a class in vrepdbs
    *time = cd->ohTime();
    *sts = LDH__SUCCESS;
    return;
  }

  wb_bdrep *bd = cd->bdrep( sts, pwr_eBix_rt);
  if ( !bd) {
    delete cd;
    return;
  }
  if ( bd->nAttribute() == 0) {
    delete cd;
    delete bd;
    return;
  }

  *time = cd->structModTime();

  wb_adrep *ad, *oad;
  for ( ad = bd->adrep( sts); ad;) {
    if ( cdh_tidIsCid( ad->tid())) {
      pwr_tTime t;
      classVersion( sts, ad->tid(), &t);
      if ( EVEN(*sts)) return;

      if ( time_Acomp( time, &t) == -1)
        *time = t;
    }
    oad = ad;
    ad = ad->next( sts);
    delete oad;
  }
  delete cd;
  delete bd;
  *sts = LDH__SUCCESS;
}

void wb_merep::insertCattObject( pwr_tStatus *sts, pwr_tCid cid,
                                 wb_adrep *adp, int offset)
{
  merep_sClassAttrKey   key;
  merep_sClassAttr  *item;
  int     j;

  wb_cdrep *cd = cdrep( sts, adp->tid());
  if ( EVEN(*sts)) throw wb_error(*sts);

  // Find a tree node with free offsets
  key.subCid = adp->tid();
  key.hostCid = cid;
  key.idx = 0;
  item = (merep_sClassAttr *) tree_Find( sts, m_catt_tt, &key);

  while ( ODD(*sts) && item->numOffset == merep_cCattOffsetSize) {
    key.idx++;
    item = (merep_sClassAttr *) tree_Find( sts, m_catt_tt, &key);
  }
  if ( !adp->flags() & PWR_MASK_ARRAY) {
    if ( ODD(*sts)) {
      // Insert in found item
      item->offset[item->numOffset] = offset + adp->offset();
      item->flags[item->numOffset++] = adp->flags();
    }
    else {
      // Insert a new item
      item = (merep_sClassAttr *) tree_Insert( sts, m_catt_tt, &key);
      item->offset[item->numOffset] = offset + adp->offset();
      item->flags[item->numOffset++] = adp->flags();
    }

    // Look for class attributes in this class
    wb_bdrep *bd = cd->bdrep( sts, pwr_eBix_rt);
    if ( EVEN(*sts)) {
      delete cd;
      *sts = LDH__SUCCESS;
      return;
    }

    wb_adrep *ad, *adnext;
    for ( ad = bd->adrep( sts);
          ODD(*sts);
          adnext = ad->next( sts), delete ad, ad = adnext) {
      if ( ad->flags() & PWR_MASK_CLASS && cdh_tidIsCid( ad->tid())) {
        insertCattObject( sts, cid, ad, offset + ad->offset());
        if ( EVEN(*sts)) return;
      }
    }
    delete bd;
  }
  else {
    // Insert all offsets in the array
    for ( j = 0; j < adp->nElement(); j++) {
      if ( ODD(*sts) && item->numOffset < merep_cCattOffsetSize) {
        // Insert in current item
	item->offset[item->numOffset] = offset + adp->offset() +
          j * adp->size() / adp->nElement();
	item->flags[item->numOffset++] = adp->flags();
      }
      else {
        // Insert a new item
        if ( ODD(*sts))
          key.idx++;
        item = (merep_sClassAttr *) tree_Insert( sts, m_catt_tt, &key);
	item->offset[item->numOffset] = offset + adp->offset() +
          j * adp->size() / adp->nElement();
	item->flags[item->numOffset++] = adp->flags();
      }

      // Look for class attributes in this class
      wb_bdrep *bd = cd->bdrep( sts, pwr_eBix_rt);
      if ( EVEN(*sts)) {
        delete cd;
        *sts = LDH__SUCCESS;
        return;
      }

      wb_adrep *ad, *adnext;
      for ( ad = bd->adrep( sts);
            ODD(*sts);
            adnext = ad->next( sts), delete ad, ad = adnext) {
        if ( ad->flags() & PWR_MASK_CLASS && cdh_tidIsCid( ad->tid())) {
          insertCattObject( sts, cid, ad, offset + adp->offset() +
                            j * adp->size() / adp->nElement());
          if ( EVEN(*sts)) return;
        }
      }
      delete bd;
    }
  }
  delete cd;
  *sts = LDH__SUCCESS;
}

tree_sTable *wb_merep::buildCatt( pwr_tStatus *sts)
{
  if ( m_catt_tt) {
    // Already built
    *sts = LDH__SUCCESS;
    return m_catt_tt;
  }

  m_catt_tt = tree_CreateTable( sts, sizeof(merep_sClassAttrKey),
                                offsetof(merep_sClassAttr, key),
                                sizeof(merep_sClassAttr), 100, compCatt);

  // Loop through all $ClassDef objects
  for ( mvrep_iterator it = m_mvrepdbs.begin();
        it != m_mvrepdbs.end();
        it++) {
    wb_vrepdbs *vrep = (wb_vrepdbs *)it->second;
    wb_orep *o, *onext;
    wb_adrep *ad, *adnext;
    pwr_tCid cid;

    for ( o = vrep->object( sts, pwr_eClass_ClassDef);
          ODD(*sts);
          onext = o->next( sts), o->unref(), o = onext) {
      o->ref();

      cid = cdh_ClassObjidToId( o->oid());
      wb_cdrep *cd = cdrep( sts, cid);
      if ( EVEN(*sts)) throw wb_error(*sts);

      wb_bdrep *bd = cd->bdrep( sts, pwr_eBix_rt);
      if ( EVEN(*sts)) {
        delete cd;
        continue;
      }

      for ( ad = bd->adrep( sts);
            ODD(*sts);
            adnext = ad->next( sts), delete ad, ad = adnext) {
        if ( ad->flags() & PWR_MASK_CLASS && cdh_tidIsCid( ad->tid())) {
          insertCattObject( sts, cid, ad, 0);
          if ( EVEN(*sts)) throw wb_error(*sts);
        }
      }
      delete bd;
      delete cd;
    }
  }

#if 0
  merep_sClassAttrKey key;
  key.subCid = 0;
  key.hostCid = 0;
  key.idx = 0;
  merep_sClassAttr *item;
  for ( item = (merep_sClassAttr*) tree_FindSuccessor( sts, m_catt_tt, &key);
        item != 0;
        item = (merep_sClassAttr*) tree_FindSuccessor( sts, m_catt_tt, &item->key)) {
    wb_cdrep *cd1 = cdrep( sts, item->key.subCid);
    wb_cdrep *cd2 = cdrep( sts, item->key.hostCid);
    printf( "%-20s %-20s %2d offs ", cd1->name(), cd2->name(),
            item->key.idx);
    for ( int i = 0; i < item->numOffset; i++)
      printf( "%d ", item->offset[i]);
    printf( "\n");
    delete cd1;
    delete cd2;
  }
#endif

  *sts = LDH__SUCCESS;
  return m_catt_tt;
}

// Compare two keys in class attribute binary tree

static int compCatt( tree_sTable *tp, tree_sNode *x, tree_sNode *y)
{
  merep_sClassAttrKey *xKey = (merep_sClassAttrKey *) (tp->keyOffset + (char *) x);
  merep_sClassAttrKey *yKey = (merep_sClassAttrKey *) (tp->keyOffset + (char *) y);

  if ( xKey->subCid == yKey->subCid) {
    if ( xKey->hostCid == yKey->hostCid) {
      if ( xKey->idx == yKey->idx)
        return 0;
      else if ( xKey->idx < yKey->idx)
        return -1;
      else
        return 1;
    }
    else if ( xKey->hostCid < yKey->hostCid)
      return -1;
    else
      return 1;
  }
  else if ( xKey->subCid < yKey->subCid)
    return -1;
  else
    return 1;
}


void wb_merep::subClass( pwr_tCid supercid, pwr_tCid subcid, pwr_tCid *nextsubcid,
                         pwr_tStatus *sts)
{
  bool prev_found = false;

  // Loop through all $ClassDef objects
  for ( mvrep_iterator it = m_mvrepdbs.begin();
        it != m_mvrepdbs.end();
        it++) {
    wb_vrepdbs *vrep = (wb_vrepdbs *)it->second;
    wb_orep *o, *onext;
    wb_adrep *ad;
    pwr_tCid cid;

    for ( o = vrep->object( sts, pwr_eClass_ClassDef);
          ODD(*sts);
          onext = o->next( sts), o->unref(), o = onext) {
      o->ref();

      cid = cdh_ClassObjidToId( o->oid());
      wb_cdrep *cd = cdrep( sts, cid);
      if ( EVEN(*sts)) throw wb_error(*sts);

      wb_bdrep *bd = cd->bdrep( sts, pwr_eBix_rt);
      if ( EVEN(*sts)) {
        delete cd;
        continue;
      }

      ad = bd->adrep( sts);
      if ( ODD(*sts) && ad->flags() & PWR_MASK_SUPERCLASS && ad->tid() == supercid) {
        if ( subcid == pwr_cNCid || prev_found) {
          *nextsubcid = cid;
          delete bd;
          delete cd;
          return;
        }
        else if ( subcid == cid)
          prev_found = true;
      }
      delete bd;
      delete cd;
    }
  }
  *sts = LDH__NONEXTCLASS;
}

wb_mvrep *wb_merep::nextVolume(pwr_tStatus *sts, pwr_tVid vid)
{
  // Search in dbs
  mvrep_iterator it = m_mvrepdbs.find(vid);
  if ( it != m_mvrepdbs.end()) {
    it++;
    if ( it != m_mvrepdbs.end()) {
      *sts = LDH__SUCCESS;
      return it->second;
    }
  }

  *sts = LDH__NOSUCHVOL;
  return 0;
}
