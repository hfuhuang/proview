#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "pwr.h"
#include "pwr_class.h"
#include "co_dcli.h"
#include "db_cxx.h"
#include "wb_ldh.h"
#include "wb_destination.h"
#include "wb_db.h"
#include "wb_name.h"
#include "wb_export.h"


static void printstat(DbEnv *ep, char *s);


wb_db_info::wb_db_info(wb_db *db) :
  m_db(db), m_data(&m_volume, sizeof(m_volume))
{
}

void wb_db_info::get(wb_db_txn *txn)
{
  int index = 0;
  int ret;
  
  m_key.set_data(&index);
  m_key.set_size(sizeof(index));
  m_data.set_ulen(sizeof(m_volume));
  m_data.set_flags(DB_DBT_USERMEM);
  
  ret = m_db->m_t_info->get(txn, &m_key, &m_data, 0);
  printf("info get: %d\n", ret);
}

void wb_db_info::put(wb_db_txn *txn)
{
  int index = 0;
  int ret;
  
  m_key.set_data(&index);
  m_key.set_size(sizeof(index));
  
  ret = m_db->m_t_info->put(txn, &m_key, &m_data, 0);
  printf("info put: %d\n", ret);
}

wb_db_class::wb_db_class(wb_db *db) :
  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(0, 0), m_dbc(0)
{
}

wb_db_class::wb_db_class(wb_db *db, wb_db_txn *txn, pwr_tCid cid) :
  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(0, 0), m_dbc(0)
{
  m_k.oid = pwr_cNOid;
  m_k.cid = cid;    
}

wb_db_class::wb_db_class(wb_db *db, pwr_tCid cid, pwr_tOid oid) :
  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(0, 0), m_dbc(0)
{
  m_k.oid = oid;
  m_k.cid = cid;
}

wb_db_class::wb_db_class(wb_db *db, wb_db_ohead &o) :
  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(0, 0), m_dbc(0)
{
  m_k.oid = o.oid();
  m_k.cid = o.cid();
}

bool wb_db_class::succ(pwr_tOid oid)
{
  m_k.oid = oid;
  m_k.oid.oix += 1;
  
  if (m_dbc)
    m_dbc->close();
#if 0
  m_db->m_t_class->cursor(txn, &m_dbc, 0);
  int ret = m_dbc->get(&m_key, &m_data, DB_SET_RANGE);
  m_dbc->close();
  return ret == 0;
#endif
  return false;
}

bool wb_db_class::pred(pwr_tOid oid)
{
  m_k.oid = oid;
    
  int ret = m_dbc->get(&m_key, &m_data, DB_SET_RANGE);
  if (ret != 0)
    return false;
  ret = m_dbc->get(&m_key, &m_data, DB_PREV);
  return ret == 0;
}

int wb_db_class::put(wb_db_txn *txn)
{
  return m_db->m_t_class->put(txn, &m_key, &m_data, DB_NOOVERWRITE);
}

int wb_db_class::del(wb_db_txn *txn)
{
  return m_db->m_t_class->del(txn, &m_key, 0);
}

void wb_db_class::iter(void (*print)(pwr_tOid oid, pwr_tCid cid))
{
  int rc = 0;  

  m_db->m_t_class->cursor(m_db->m_txn, &m_dbc, 0);

	/* Initialize the key/data pair so the flags aren't set. */
	memset(&m_k, 0, sizeof(m_k));

  printf("sizeof(m_k): %d\n", sizeof(m_k));
  
  m_key.set_data(&m_k);
  m_key.set_ulen(sizeof(m_k));
  m_key.set_flags(DB_DBT_USERMEM);
  
	/* Walk through the database and print out the key/data pairs. */
  //int rc = m_dbc->get(&m_key, &m_data, DB_FIRST);

  while ((rc = m_dbc->get(&m_key, &m_data, DB_NEXT)) == 0) {
    //printf("k: %d, d: %d\n", (int)m_key.get_size(), (int)m_data.get_size());
    //volatile int a = m_key.get_size();
    //a = m_data.get_size();
    
    print(m_k.oid, m_k.cid);
    
  }
  
  m_dbc->close();
}

wb_db_class::~wb_db_class()
{
  if (m_dbc)
    m_dbc->close();
}

wb_db_name::wb_db_name(wb_db *db, wb_db_txn *txn) :
  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(&m_d, sizeof(m_d))
{
  memset(&m_k, 0, sizeof(m_k));
  m_k.poid = pwr_cNOid;
}

wb_db_name::wb_db_name(wb_db *db, wb_db_ohead &o) :
  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(&m_d, sizeof(m_d))
{
  memset(&m_k, 0, sizeof(m_k));
  m_k.poid = o.poid();
  strcpy(m_k.normname, o.normname());
  m_d.oid = o.oid();
}

wb_db_name::wb_db_name(wb_db *db, pwr_tOid oid, pwr_tOid poid, const char *name) :
  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(&m_d, sizeof(m_d))
{
  wb_name n(name);

  memset(&m_k, 0, sizeof(m_k));
  
  m_k.poid = poid;
  strcpy(m_k.normname, n.normName(cdh_mName_object));
  m_d.oid = oid;
}

wb_db_name::wb_db_name(wb_db *db, wb_db_txn *txn, pwr_tOid poid, wb_name &name) :
  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(&m_d, sizeof(m_d))
{
  memset(&m_k, 0, sizeof(m_k));
  m_k.poid = poid;
  strcpy(m_k.normname, name.normName(cdh_mName_object));
  m_d.oid = pwr_cNOid;
}

//wb_db_name::wb_db_name(wb_db *db, pwr_tOid, char *name) :
//  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(&m_d, sizeof(m_d))
//{
//}

//wb_db_name::wb_db_name(wb_db *db, pwr_tOid poid, const char *name) :
//  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(&m_d, sizeof(m_d))
//{
//  m_k.poid = poid();
//  strcpy(m_k.normname, name);
//  m_d.oid = ?;
//}

//wb_db_name::wb_db_name(wb_db *db, wb_db_txn *txn, pwr_tOid poid, wb_name name) :
//  m_db(db), m_key(&m_k, sizeof(m_k)), m_data(&m_d, sizeof(m_d))
//{
//}


int wb_db_name::put(wb_db_txn *txn)
{
  return m_db->m_t_name->put(txn, &m_key, &m_data, DB_NOOVERWRITE);
}

int wb_db_name::del(wb_db_txn *txn)
{
  return m_db->m_t_name->del(txn, &m_key, 0);
}

void wb_db_name::name(wb_name &name)
{
  memset(m_k.normname, 0, sizeof(m_k.normname));
  strcpy(m_k.normname, name.normName(cdh_mName_object));
}

void wb_db_name::iter(void (*print)(pwr_tOid poid, pwr_tObjName name, pwr_tOid oid))
{
  int rc = 0;  

  m_db->m_t_name->cursor(m_db->m_txn, &m_dbc, 0);

	/* Initialize the key/data pair so the flags aren't set. */
	memset(&m_k, 0, sizeof(m_k));
	memset(&m_d, 0, sizeof(m_d));

  printf("sizeof(m_k): %d\n", sizeof(m_k));
  printf("sizeof(m_d): %d\n", sizeof(m_d));
  
  m_key.set_data(&m_k);
  m_key.set_ulen(sizeof(m_k));
  m_key.set_flags(DB_DBT_USERMEM);

  m_data.set_data(&m_d);
  m_data.set_ulen(sizeof(m_d));
  m_data.set_flags(DB_DBT_USERMEM);
  
  
	/* Walk through the database and print out the key/data pairs. */
  //int rc = m_dbc->get(&m_key, &m_data, DB_FIRST);

  while ((rc = m_dbc->get(&m_key, &m_data, DB_NEXT)) == 0) {
    //printf("k: %d, d: %d\n", (int)m_key.get_size(), (int)m_data.get_size());
    //volatile int a = m_key.get_size();
    //a = m_data.get_size();
    
    print(m_k.poid, m_k.normname, m_d.oid);
    
  }
  
  m_dbc->close();
}


wb_db_ohead::wb_db_ohead() :
  m_db(0), m_key(&m_oid, sizeof(m_oid)), m_data(&m_o, sizeof(m_o))
{
  memset(&m_o, 0, sizeof(m_o));
}

wb_db_ohead::wb_db_ohead(wb_db *db) :
  m_db(db), m_key(&m_oid, sizeof(m_oid)), m_data(&m_o, sizeof(m_o))
{
  memset(&m_o, 0, sizeof(m_o));
}

wb_db_ohead::wb_db_ohead(wb_db *db, pwr_tOid oid) :
  m_db(db), m_key(&m_oid, sizeof(m_oid)), m_data(&m_o, sizeof(m_o))
{
  memset(&m_o, 0, sizeof(m_o));
  m_oid = m_o.oid = oid;
}

wb_db_ohead::wb_db_ohead(wb_db *db, wb_db_txn *txn, pwr_tOid oid) :
  m_db(db), m_key(&m_oid, sizeof(m_oid)), m_data(&m_o, sizeof(m_o))
{
  memset(&m_o, 0, sizeof(m_o));
  m_oid = m_o.oid = oid;
  get(txn);
}

wb_db_ohead::wb_db_ohead(wb_db *db, pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
                         pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid, pwr_tOid loid,
                         const char *name, const char *normname, pwr_mClassDef flags,
                         pwr_tTime ohTime, pwr_tTime rbTime, pwr_tTime dbTime,
                         size_t rbSize, size_t dbSize) :
  m_db(db), m_key(&m_oid, sizeof(m_oid)), m_data(&m_o, sizeof(m_o))
{
  
  memset(&m_o, 0, sizeof(m_o));
  m_oid = m_o.oid = oid;
  m_o.cid = cid;
  m_o.poid = poid;
  strcpy(m_o.name, name);
  strcpy(m_o.normname, normname);
  m_o.time = ohTime;
  m_o.boid = boid;
  m_o.aoid = aoid;
  m_o.foid = foid;
  m_o.loid =loid;
    
  m_o.flags = flags;

  m_o.body[0].time = rbTime;
  m_o.body[0].size = rbSize;
  m_o.body[1].time = dbTime;
  m_o.body[1].size = dbSize;
}


wb_db_ohead &wb_db_ohead::get(wb_db_txn *txn)
{
  int rc = 0;
  m_data.set_ulen(sizeof(m_o));
  m_data.set_flags(DB_DBT_USERMEM);

  rc = m_db->m_t_ohead->get(txn, &m_key, &m_data, 0);
  if (rc)
    printf("wb_db_ohead::get(txn), get, rc %d\n", rc);
  return *this;
}

int wb_db_ohead::put(wb_db_txn *txn)
{
  return m_db->m_t_ohead->put(txn, &m_key, &m_data, 0);
}

wb_db_ohead &wb_db_ohead::get(wb_db_txn *txn, pwr_tOid oid)
{
  int rc = 0;
  m_oid = oid;
  m_data.set_ulen(sizeof(m_o));
  m_data.set_flags(DB_DBT_USERMEM);

  m_db->m_t_ohead->get(txn, &m_key, &m_data, 0);
  if (rc)
    printf("wb_db_ohead::get(txn, oid = %d.%d), get, rc %d\n", oid.vid, oid.oix, rc);
  //pwr_Assert(oid.oix == m_o.oid.oix);
  if (oid.oix != m_o.oid.oix)
    printf("oid.oix (%d.%d) != m_o.oid.oix (%d.%d), %s\n", oid.vid, oid.oix, m_o.oid.vid, m_o.oid.oix, m_o.name);
  return *this;
}

int wb_db_ohead::del(wb_db_txn *txn)
{
  return m_db->m_t_ohead->del(txn, &m_key, 0);
}

void wb_db_ohead::name(wb_name &name)
{
  memset(m_o.name, 0, sizeof(m_o.name));
  memset(m_o.normname, 0, sizeof(m_o.normname));
  strcpy(m_o.name, name.name(cdh_mName_object));
  strcpy(m_o.normname, name.normName(cdh_mName_object));
}

void wb_db_ohead::name(pwr_tOid &oid)
{
  memset(m_o.name, 0, sizeof(m_o.name));
  memset(m_o.normname, 0, sizeof(m_o.normname));
  sprintf(m_o.name, "O%d", oid.oix);
  strcpy(m_o.normname, m_o.name);
}

void wb_db_ohead::clear()
{
  memset(&m_o, 0, sizeof(m_o));
}

void wb_db_ohead::iter(void (*print)(pwr_tOid oid, db_sObject *op))
{
  int rc = 0;  

  m_db->m_t_ohead->cursor(m_db->m_txn, &m_dbc, 0);

	/* Initialize the key/data pair so the flags aren't set. */
	memset(&m_oid, 0, sizeof(m_oid));
	memset(&m_o, 0, sizeof(m_o));

  printf("sizeof(m_oid): %d\n", sizeof(m_oid));
  printf("sizeof(m_o): %d\n", sizeof(m_o));
  
  m_key.set_data(&m_oid);
  m_key.set_ulen(sizeof(m_oid));
  m_key.set_flags(DB_DBT_USERMEM);

  m_data.set_data(&m_o);
  m_data.set_ulen(sizeof(m_o));
  m_data.set_flags(DB_DBT_USERMEM);
  
  
	/* Walk through the database and print out the key/data pairs. */
  //int rc = m_dbc->get(&m_key, &m_data, DB_FIRST);

  while ((rc = m_dbc->get(&m_key, &m_data, DB_NEXT)) == 0) {
    //printf("k: %d, d: %d\n", (int)m_key.get_size(), (int)m_data.get_size());
    //volatile int a = m_key.get_size();
    //a = m_data.get_size();
    
    print(m_oid, &m_o);
    
  }
  
  m_dbc->close();
}

wb_db_rbody::wb_db_rbody(wb_db *db, pwr_tOid oid, size_t size, void *p) :
  m_db(db), m_oid(oid), m_size(size), m_p(p), m_key(&m_oid, sizeof(m_oid)), m_data(p, size)
{
}

wb_db_rbody::wb_db_rbody(wb_db *db) :
  m_db(db), m_oid(pwr_cNOid), m_size(0), m_p(0), m_key(&m_oid, sizeof(m_oid)), m_data(0, 0)
{
}

wb_db_rbody::wb_db_rbody(wb_db *db, pwr_tOid oid) :
  m_db(db), m_oid(oid), m_size(0), m_p(0), m_key(&m_oid, sizeof(m_oid)), m_data(0, 0)
{
}

int wb_db_rbody::put(wb_db_txn *txn)
{
  return m_db->m_t_rbody->put(txn, &m_key, &m_data, 0);
}

int wb_db_rbody::put(wb_db_txn *txn, size_t offset, size_t size, void *p)
{
  m_data.set_doff(offset);
  m_data.set_dlen(size);
  m_data.set_data(p);
  m_data.set_ulen(size);
  m_data.set_size(size);
  m_data.set_flags(DB_DBT_PARTIAL);
  
  return m_db->m_t_rbody->put(txn, &m_key, &m_data, 0);
}

int wb_db_rbody::get(wb_db_txn *txn, size_t offset, size_t size, void *p)
{
  m_data.set_doff(offset);
  m_data.set_dlen(size);
  m_data.set_data(p);
  m_data.set_ulen(size);
  m_data.set_flags(DB_DBT_USERMEM|DB_DBT_PARTIAL);
  
  return m_db->m_t_rbody->get(txn, &m_key, &m_data, 0);
}

int wb_db_rbody::del(wb_db_txn *txn)
{
  return m_db->m_t_rbody->del(txn, &m_key, 0);
}

void wb_db_rbody::iter(void (*print)(pwr_tOid oid))
{
  int rc = 0;
  //static char b[1];
  

  m_db->m_t_rbody->cursor(m_db->m_txn, &m_dbc, 0);

	/* Initialize the key/data pair so the flags aren't set. */
	memset(&m_oid, 0, sizeof(m_oid));
  m_key.set_data(&m_oid);
  m_key.set_ulen(sizeof(m_oid));
  //m_key.set_dlen(sizeof(m_oid));
  //m_key.set_size(sizeof(m_oid));
  m_key.set_flags(DB_DBT_USERMEM);
  //m_data.set_data(b);
  //m_data.set_ulen(0);
  //m_data.set_dlen(sizeof(b));
  //m_data.set_size(sizeof(b));
  //m_data.set_doff(0);
  m_data.set_flags(DB_DBT_USERMEM|DB_DBT_PARTIAL);
  
	/* Walk through the database and print out the key/data pairs. */
  //int rc = m_dbc->get(&m_key, &m_data, DB_FIRST);

  while (1) {
    try {
      rc = m_dbc->get(&m_key, &m_data, DB_NEXT);
    }
    catch (DbException &e) {
      printf("Exc: %s\n", e.what());
    }
  
    if (rc == DB_NOTFOUND)
      break;
    
    print(m_oid);
  }
  
  m_dbc->close();
}

wb_db_dbody::wb_db_dbody(wb_db *db, pwr_tOid oid, size_t size, void *p) :
  m_db(db), m_oid(oid), m_size(size), m_p(p), m_key(&m_oid, sizeof(m_oid)), m_data(p, size)
{
}

wb_db_dbody::wb_db_dbody(wb_db *db) :
  m_db(db), m_oid(pwr_cNOid), m_size(0), m_p(0), m_key(&m_oid, sizeof(m_oid)), m_data(0, 0)
{
}

wb_db_dbody::wb_db_dbody(wb_db *db, pwr_tOid oid) :
  m_db(db), m_oid(oid), m_size(0), m_p(0), m_key(&m_oid, sizeof(m_oid)), m_data(0, 0)
{
}

int wb_db_dbody::put(wb_db_txn *txn)
{
  return m_db->m_t_dbody->put(txn, &m_key, &m_data, 0);
}

int wb_db_dbody::put(wb_db_txn *txn, size_t offset, size_t size, void *p)
{
  m_data.set_doff(offset);
  m_data.set_dlen(size);
  m_data.set_data(p);
  m_data.set_ulen(size);
  m_data.set_size(size);
  m_data.set_flags(DB_DBT_PARTIAL);
  
  return m_db->m_t_dbody->put(txn, &m_key, &m_data, 0);
}

int wb_db_dbody::get(wb_db_txn *txn, size_t offset, size_t size, void *p)
{
  m_data.set_doff(offset);
  m_data.set_dlen(size);
  m_data.set_data(p);
  m_data.set_ulen(size);
  m_data.set_flags(DB_DBT_USERMEM|DB_DBT_PARTIAL);
  
  return m_db->m_t_dbody->get(txn, &m_key, &m_data, 0);
}

int wb_db_dbody::del(wb_db_txn *txn)
{
  return m_db->m_t_dbody->del(txn, &m_key, 0);
}

void wb_db_dbody::iter(void (*print)(pwr_tOid oid))
{
  int rc = 0;
  //static char b[65000];
  
  m_db->m_t_dbody->cursor(m_db->m_txn, &m_dbc, 0);

	/* Initialize the key/data pair so the flags aren't set. */
	memset(&m_oid, 0, sizeof(m_oid));
  m_key.set_data(&m_oid);
  m_key.set_ulen(sizeof(m_oid));
  m_key.set_flags(DB_DBT_USERMEM);
  //m_data.set_data(b);
  //m_data.set_ulen(sizeof(b));
  //m_data.set_dlen(sizeof(b));
  //m_data.set_size(sizeof(b));
  m_data.set_flags(DB_DBT_USERMEM|DB_DBT_PARTIAL);

  //m_data.set_flags(DB_DBT_USERMEM);
  
	/* Walk through the database and print out the key/data pairs. */
  //int rc = m_dbc->get(&m_key, &m_data, DB_FIRST);

  while (1) {

    try {
      rc = m_dbc->get(&m_key, &m_data, DB_NEXT);
    }
    catch (DbException &e) {
      printf("Exc: %s\n", e.what());
    }
  
    if (rc == DB_NOTFOUND)
      break;
    
    print(m_oid);
 }
  
  m_dbc->close();
}

wb_db::wb_db()
{
}

wb_db::wb_db(pwr_tVid vid) :
  m_vid(vid)
{
}

void wb_db::close()
{
  m_t_ohead->close(0);
  m_t_rbody->close(0);
  m_t_dbody->close(0);
  m_t_class->close(0);
  m_t_name->close(0);
  m_t_info->close(0);
  
  printstat(m_env, "before abort");
  int rc =  m_txn->abort();
  printf("int rc =  m_txn->abort(): %d\n", rc);
  printstat(m_env, "before m_env->close(0)");
  m_env->close(0);
}

void wb_db::create(pwr_tVid vid, pwr_tCid cid, const char *volumeName, const char *fileName)
{
  m_vid = vid;
  m_cid = cid;
  strcpy(m_volumeName, volumeName);
  dcli_translate_filename(m_fileName, fileName);
  
  openDb(false);
}

wb_db_txn *wb_db::begin(wb_db_txn *txn)
{
  wb_db_txn *new_txn = 0;
  
  m_env->txn_begin((DbTxn *)txn, (DbTxn **)&new_txn, 0);
  
  return new_txn;
}

void  wb_db::open(const char *fileName)
{
  dcli_translate_filename(m_fileName, fileName);
  openDb(true);
  
  m_env->txn_begin(0, (DbTxn **)&m_txn, 0);
  //m_txn = 0;
  
  wb_db_info i(this);
  i.get(m_txn);
  m_vid = i.vid();
  m_cid = i.cid();
  strcpy(m_volumeName, i.name());
}

static void printstat(DbEnv *ep, char *s)
{
  DB_LOCK_STAT *lp;

  return;
  
  printf("DbEnv loc statistics, %s:\n", s);
  
  ep->lock_stat(&lp, 0);
  printf("  lastid.......: %d\n", lp->st_id);
  printf("  nmodes.......: %d\n", lp->st_nmodes);
  printf("  maxlocks:....: %d\n", lp->st_maxlocks);
  printf("  maxlockers...: %d\n", lp->st_maxlockers);
  printf("  maxobjects...: %d\n", lp->st_maxobjects);
  printf("  nlocks.......: %d\n", lp->st_nlocks);
  printf("  maxnlocks....: %d\n", lp->st_maxnlocks);
  printf("  nlockers.....: %d\n", lp->st_nlockers);
  printf("  maxnlockers..: %d\n", lp->st_maxnlockers);
  printf("  nobjects.....: %d\n", lp->st_nobjects);
  printf("  maxnobjects..: %d\n", lp->st_maxnobjects);
  printf("  nrequests....: %d\n", lp->st_nrequests);
  printf("  nreleases....: %d\n", lp->st_nreleases);
  printf("  nnowaits.....: %d\n", lp->st_nnowaits);
  printf("  nconflicts...: %d\n", lp->st_nconflicts);
  printf("  ndeadlocks...: %d\n", lp->st_ndeadlocks);
  printf("  regsize......: %d\n", lp->st_regsize);
  printf("  region_wait..: %d\n", lp->st_region_wait);
  printf("  region_nowait: %d\n", lp->st_region_nowait);
  printf("\n");
}

void wb_db::openDb(bool useTxn)
{
  struct stat sb;
  int rc;
  
  /* Create the directory, read/write/access owner only. */
  if (stat(m_fileName, &sb) != 0) {
    if (mkdir(m_fileName, S_IRWXU) != 0) {
      fprintf(stderr, "txnapp: mkdir: %s, %s\n", m_fileName, strerror(errno));
      //exit(1);
    }
        
  }

  m_env = new DbEnv(0/*DB_CXX_NO_EXCEPTIONS*/);
  printf("%s\n", m_env->version(0, 0, 0));
  m_env->set_errpfx("PWR db");
	m_env->set_cachesize(0, 256 * 1024 * 1024, 0);
  rc = m_env->set_lg_bsize(1024*1024*2);
  rc = m_env->set_lg_max(1024*1024*8*2);
  rc = m_env->set_lk_max_locks(500000);
  rc = m_env->set_lk_max_objects(20000);

  if (useTxn) {
    m_env->open(m_fileName,
                DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER,
                S_IRUSR | S_IWUSR);
  } else {
    m_env->open(m_fileName,
                DB_CREATE | DB_INIT_MPOOL | DB_PRIVATE,
                S_IRUSR | S_IWUSR);
  }  

  printstat(m_env, "after open env");
  
  m_t_ohead = new Db(m_env, 0);
  m_t_rbody = new Db(m_env, 0);
  m_t_dbody = new Db(m_env, 0);
  m_t_class = new Db(m_env, 0);
  m_t_name  = new Db(m_env, 0);
  m_t_info  = new Db(m_env, 0);
    
  m_t_ohead->open(NULL, "ohead", NULL, DB_BTREE, DB_CREATE, S_IRUSR | S_IWUSR);
  m_t_rbody->open(NULL, "rbody", NULL, DB_BTREE, DB_CREATE, S_IRUSR | S_IWUSR);
  m_t_dbody->open(NULL, "dbody", NULL, DB_BTREE, DB_CREATE, S_IRUSR | S_IWUSR);
//  m_t_dbody->open(NULL, "dbody", NULL, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, S_IRUSR | S_IWUSR);
  m_t_class->open(NULL, "class", NULL, DB_BTREE, DB_CREATE, S_IRUSR | S_IWUSR);
  m_t_name->open(NULL, "name", NULL, DB_BTREE, DB_CREATE, S_IRUSR | S_IWUSR);
  m_t_info->open(NULL, "info", NULL, DB_BTREE, DB_CREATE, S_IRUSR | S_IWUSR);
  printstat(m_env, "after open databases");
    
}


pwr_tOid wb_db::new_oid(wb_db_txn *txn)
{
  int rc = 0;
  pwr_tOid oid = pwr_cNOid;
  oid.vid = m_vid;
  wb_db_rbody b(this, oid);

  rc = b.get(txn, offsetof(pwr_sRootVolume, NextOix), sizeof(pwr_tOix), &oid.oix);
  if (rc)
    printf("wb_db::new_oid, b.get, rc %d\n", rc);
  oid.oix++;
  rc = b.put(txn, offsetof(pwr_sRootVolume, NextOix), sizeof(pwr_tOix), &oid.oix);
  if (rc)
    printf("wb_db::new_oid, b.put, rc %d\n", rc);
  
  return oid;
}

int wb_db::del_family(wb_db_txn *txn, Dbc *cp, pwr_tOid poid)
{
  int ret = 0;
#if 0
  dbName name;
  name.poid = poid;
  name.normname = "";
  pwr_tOid oid = 0;
    
  FamilyKey  nk(po, );
  FamiltData nd(&oid, sizeof(oid));    
  Dbc *ncp;
  cp->dup(*ncp, 0);

  while ((ret = cp->get(&nk, &nd, DB_NEXT)) != DB_NOTFOUND) {
    del_family(txn, ncp, oid);
    cp->del(0);
    oh_k ok(nd);
    oh_d od();
    m_db_ohead->get(txn, &ok, &od, 0);
    wb_DbClistK ck(od);
    m_db_class->del(txn, &ck, 0);
    wb_DbBodyK bk(od);
    m_db_obody->del(txn, &bk, 0);
    m_db_ohead->del(txn, &ok, 0);
  }
  ncp->close();

  ret = m_db_name->del(txn, &key, 0);
#endif
  return ret;
}

//
// Save all changes done in the current transaction.
//
bool wb_db::commit(pwr_tStatus *sts)
{
  int rc = 0;
  
  printstat(m_env, "before commit");
  rc = m_txn->commit(0);
  if (rc)
    printf("wb_db::commit, rc %d\n", rc);
  rc = m_env->txn_checkpoint(0, 0, 0);
  if (rc)
    printf("wb_db::commit, CHECK, rc %d\n", rc);
  m_env->txn_begin(0, (DbTxn **)&m_txn, 0);
  *sts = rc;
  printstat(m_env, "after commit");
  return true;
}

//
// Abort the current transactionm, the database is restored to
// the state it had before the current transaction started.
//
bool wb_db::abort(pwr_tStatus *sts)
{
  printstat(m_env, "before abort");
  *sts = m_txn->abort();
  m_env->txn_begin(0, (DbTxn **)&m_txn, 0);
  printstat(m_env, "after abort");
  return true;
}

bool wb_db::deleteFamily(pwr_tStatus *sts, wb_db_ohead *o)
{
  DbTxn *txn = 0;    
    
  m_env->txn_begin(m_txn, &txn, 0);
        
  try {
    //unadopt(txn, wb_Position(o));
    //del_ohead(txn, o);
    //del_clist(txn, o);
    //del_body(txn, o);

    //txn->commit(0);
    //o->mark(is_deleted);
        
  }
  catch (DbException &e) {
    txn->abort();
  }
    
  return true;
}

#if 0
bool wb_db::deleteOset(pwr_tStatus *sts, wb_oset *o)
{
  DbTxn *txn = 0;
    
  m_env->txn_begin(m_txn, &txn, 0);
        
  try {
    //del_family(txn, o);
    //unadopt(txn, wb_Position(o));
    //del_ohead(txn, o);
    //del_clist(txn, o);
    //del_name(txn, o);
    //del_body(txn, o);

    txn->commit(0);
  }
  catch (DbException &e) {
    txn->abort();
  }

  return true;
}
#endif


bool wb_db::importVolume(wb_export &e)
{
  try {
    //m_env->txn_begin(0, (DbTxn **)&m_txn, 0);
//    m_txn = 0;
    
  printstat(m_env, "importVolume");
    e.exportHead(*this);
  printstat(m_env, "after head");
    e.exportRbody(*this);
  printstat(m_env, "after rbody");
    e.exportDbody(*this);
  printstat(m_env, "after dbody");
    e.exportMeta(*this);
  printstat(m_env, "after meta");
    
  //txn->commit(0);
  printstat(m_env, "after commit");
  //m_env->txn_checkpoint(0, 0, 0);
  printstat(m_env, "after checkpoint");
    return true;
  }
  catch (DbException &e) {
  printstat(m_env, "after exception");
  //txn->abort();
    printf("exeption: %s\n", e.what());
  printstat(m_env, "after abort");
    return false;
  }
}


bool wb_db::importHead(pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
                       pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid, pwr_tOid loid,
                       const char *name, const char *normname, pwr_mClassDef flags,
                       pwr_tTime ohTime, pwr_tTime rbTime, pwr_tTime dbTime,
                       size_t rbSize, size_t dbSize)
{
  wb_db_ohead o(this, oid, cid, poid, boid, aoid, foid, loid, name, normname, flags, ohTime, rbTime, dbTime, rbSize, dbSize);
  o.put(m_txn);
  //printf("head put: %d.%d %s\n", oid.vid, oid.oix, name);
  wb_db_name n(this, oid, poid, normname);
  n.put(m_txn);
  wb_db_class c(this, cid, oid);
  c.put(m_txn);
  if (oid.oix == pwr_cNOix) { // This is the volume object
    wb_db_info i(this);
    i.cid(cid);
    i.vid(oid.vid);
    i.name(name);
    i.put(m_txn);
  }
  
  return true;
}


bool wb_db::importRbody(pwr_tOid oid, size_t size, void *body)
{
  wb_db_rbody rb(this, oid, size, body);
  //printf("rbody size: %d.%d %d\n", oid.vid, oid.oix, size);
  rb.put(m_txn);
  return true;
}

bool wb_db::importDbody(pwr_tOid oid, size_t size, void *body)
{
  wb_db_dbody db(this, oid, size, body);
  //printf("dbody size: %d.%d %d\n", oid.vid, oid.oix, size);
  db.put(m_txn);
  return true;
}

bool  wb_db::importMeta(dbs_sEnv *ep)
{
  return true;
}
