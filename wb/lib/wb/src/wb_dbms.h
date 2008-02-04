/* 
 * Proview   $Id: wb_dbms.h,v 1.2 2008-02-04 13:34:49 claes Exp $
 * Copyright (C) 2007 SSAB Oxelösund AB.
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

#ifndef wb_dbms_h
#define wb_dbms_h

#if defined PWRE_CONF_MYSQL

#include <assert.h>
#include "pwr.h"
#include <mysql/mysql.h>
#include "wb_import.h"
#include <exception>
#include <string>
#include <errno.h>

class wb_name;

typedef struct {
  pwr_tOid        oid;        /**< object identifier */
  pwr_tCid        cid;        /**< class identifier */
  pwr_tOid        poid;       /**< object identifier of parent */
  pwr_tObjName    name;       /**< name of object */
  pwr_tObjName    normname;   /**< normalized object name. */
  pwr_tTime       time;       /**< time of last change in object header */
  pwr_tOid        boid;       /**< object before this object. */
  pwr_tOid        aoid;       /**< object after this object. */
  pwr_tOid        foid;       /**< first child object. */
  pwr_tOid        loid;       /**< last child object. */

  pwr_mClassDef   flags;
  struct {
    pwr_tTime       time;
    size_t          size;
  } body[2];    /**< bodies */
} dbms_sObject;

typedef struct 
{
  MYSQL_BIND *bind;
  my_bool    *is_null;
  size_t     *length;
} dbms_sBind;

class wb_dbms;
class wb_dbms_txn;
class wb_dbms_ohead;
class wb_destination;
class wb_dbms_table;
class wb_dbms_env;

class wb_dbms_txn
{
public:
  wb_dbms_env *m_env;
  
	wb_dbms_txn() {;}
	wb_dbms_txn(wb_dbms_env *env) : m_env(env) {;}
	wb_dbms_txn(const wb_dbms_txn &);
	void operator = (const wb_dbms_txn &);
  virtual ~	wb_dbms_txn() {;}

	int abort();
	int commit();
	int subAbort();
  int subBegin();
	int subCommit() { return 0;}
  void set_env(wb_dbms_env *env) {m_env = env;}
  
  
};

class wb_dbms_env
{
public:
  MYSQL *m_con;

  char *m_fileName;
  
  char *m_host;
  char *m_user;
  char *m_passwd;
  char *m_dbName;
  unsigned int m_port;
  char *m_socket;
  bool m_exists;
  

  /* Create a new file system environment:

     wb_dbms_env *env = new wb_dbms_env();
     env->create(fileName, host, user, passwd, dbName, port, socket);
     MYSQL *con = env->createDb();

     or:

     wb_dbms_env *env = new wb_dbms_env(fileName, host, user, passwd, dbName, port, socket);
     env->create();
     MYSQL *con = env->createDb();

  */
	wb_dbms_env();
  
  /* Open an existing file system environment:

     wb_dbms_env *env = new wb_dbms_env(fileName);
     env->open();
     MYSQL *con = env->openDb();

     or:

     wb_dbms_env *env = new wb_dbms_env();
     env->open(fileName);
     MYSQL *con = env->openDb();

  */
	wb_dbms_env(const char *fileName);

  /* Environment without file system environment:
     wb_dbms_env *env = new wb_dbms_env(host, user, passwd, dbName, port, socket);
     env->open();
     MYSQL *con = env->openDb();

     or:
     wb_dbms_env *env = new wb_dbms_env();
     env->open(host, user, passwd, dbName, port, socket);
     MYSQL *con = env->openDb();
  */
  wb_dbms_env(const char *host, const char *user, const char *passwd,
                const char *dbName, unsigned int port, const char *socket);

  /* Stop copying. */
	wb_dbms_env(const wb_dbms_env &);
	void operator = (const wb_dbms_env &);

  virtual ~	wb_dbms_env() { close();}

  int create(const char *fileName, const char *host, const char *user, const char *passwd,
             const char *dbName, unsigned int port, const char *socket);  
  
  int open(void);
  int open(const char *fileName);
  int open(const char *host, const char *user, const char *passwd,
             const char *dbName, unsigned int port, const char *socket);

  MYSQL *createDb(void);
  MYSQL *openDb(void);

  bool exists() { return m_exists;}
    
  int close(void);

  void fileName(const char *fileName);
  void host(const char *host);
  void user(const char *user);
  void passwd(const char *passwd);
  void dbName(const char *dbName);
  void port(unsigned int port);  
  void socket(const char *socket);
  
  char *dbName(void);
  inline char *fileName(void) { return m_fileName;}
  inline char *host(void) { return m_host;}
  inline char *user(void) { return m_user;}
  inline char *passwd(void) { return m_passwd;}
  inline unsigned int port(void) { return m_port;}
  inline char *socket(void) { return m_socket;}  
  

  inline MYSQL *con(void) {
    return m_con;
  }
  
	int txn_begin(wb_dbms_txn *pid, wb_dbms_txn **tid);
private:
  int create();
};

class wb_dbms_qe 
{
public:

  char *m_data;
  int m_off;
  size_t m_size;
  size_t m_bSize;
  my_bool m_isNull;
  
  wb_dbms_qe() :  m_data(0), m_off(0), m_size(0), m_bSize(0), m_isNull(0) {;}
  wb_dbms_qe(void *data, size_t size) : m_data((char *)data), m_off(0), m_size(size), m_bSize(size), m_isNull(0) {;}
  ~wb_dbms_qe() {;}

	wb_dbms_qe(const wb_dbms_qe &);
	void operator = (const wb_dbms_qe &);

  void data(void *data) { m_data = (char *)data; }
	char *data() const { return m_data; }
  void size(size_t size) { m_size = size; }
	size_t size() const { return m_size; }
  void offset(size_t off) { m_off = off; }
  size_t offset() const { return m_off; }
  void bSize(size_t bSize) { m_bSize = bSize; }
  size_t bSize() const { return m_bSize; }

  size_t *a_size() { return &m_size; }
  my_bool *a_isNull() { return &m_isNull; }
  
};

class wb_dbms_query
{
public:
  wb_dbms *m_db;
  bool m_prepared;
  
  MYSQL_STMT *m_stmt;

  dbms_sBind m_result;
  dbms_sBind m_param;
  const char *m_query;
  
  int m_nResult;
  int m_nParam;

  wb_dbms_query() : m_db(0), m_prepared(false), m_stmt(0) {;}
  virtual ~wb_dbms_query() {;}

  virtual int    bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) = 0;
  virtual int execute(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) = 0;

  int prepare(const char *query, int nResult, int nParam);
  void bindParam(int index, enum enum_field_types type);
  void bindParam(int index,                             char *var);
  void bindParam(int index, enum enum_field_types type, char *var);
  void bindParam(int index,                             char *var, size_t size);
  void bindParam(int index, enum enum_field_types type, char *var, size_t size);
  void bindParam(int index, enum enum_field_types type, char *var, size_t size, size_t bsize);
  void bindParam(int index, enum enum_field_types type, wb_dbms_qe *par);
  void bindResult(int index, enum enum_field_types type);
  void bindResult(int index,                             char *var);
  void bindResult(int index, enum enum_field_types type, char *var);
  void bindResult(int index,                             char *var, size_t size);
  void bindResult(int index, enum enum_field_types type, char *var, size_t size);
  void bindResult(int index, enum enum_field_types type, char *var, size_t size, size_t bsize);
  void bindResult(int index, enum enum_field_types type, wb_dbms_qe *par);
  int  bindQuery(void);
  void error(const char *method, const char* function);
  void error(int rc, const char *method, const char* function);
  
};

class wb_dbms_cursor
{
public:
  wb_dbms_query *m_query;
  
  wb_dbms_cursor(wb_dbms_query *query) : m_query(query) {;}
  ~wb_dbms_cursor() {;}

  inline int get()
  {
    return mysql_stmt_fetch(m_query->m_stmt);
  }
  inline int count() { return 0;}
  inline int close()
  {
    return mysql_stmt_free_result(m_query->m_stmt);
  }
};

class wb_dbms_table
{
public:
  wb_dbms *m_db;

  wb_dbms_query *m_q_get;
  wb_dbms_query *m_q_succ;
  wb_dbms_query *m_q_pred;
  wb_dbms_query *m_q_ins;
  wb_dbms_query *m_q_upd;
  wb_dbms_query *m_q_del;
  wb_dbms_query *m_q_cursor;

  
  wb_dbms_table(wb_dbms *db) : m_db(db) {;}
	virtual ~wb_dbms_table() {;}
  
  int create(const char *query);
  int close();
  
	int open(wb_dbms_txn *txn, const char *name, const char *subname, int dbtype, int mode);
  int cursor(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data, wb_dbms_cursor **cp);

  inline void queryGet(wb_dbms_query *query) {
    m_q_get = query;
  }
  inline void querySucc(wb_dbms_query *query) {
    m_q_succ = query;
  }
  inline void queryPred(wb_dbms_query *query) {
    m_q_pred = query;
  }
  inline void queryIns(wb_dbms_query *query) {
    m_q_ins = query;
  }
  inline void queryUpd(wb_dbms_query *query) {
    m_q_upd = query;
  }
  inline void queryDel(wb_dbms_query *query) {
    m_q_del = query;
  }
  inline void queryCursor(wb_dbms_query *query) {
    m_q_cursor = query;
  }
  
  inline int get(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    return m_q_get->execute(txn, key, data);
  }
  inline int succ(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    return m_q_succ->execute(txn, key, data);
  }
  inline int pred(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    return m_q_pred->execute(txn, key, data);
  }
  inline int ins(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    return m_q_ins->execute(txn, key, data);
  }
  inline int upd(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    return m_q_upd->execute(txn, key, data);
  }
  inline int del(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    return m_q_del->execute(txn, key, data);
  }
	/*virtual*/ void err(int, const char *, ...);
};
  
  
 
class wb_dbms : public wb_import
{
public:
  pwr_tVid m_vid;
  pwr_tCid m_cid;
  wb_dbms_env *m_env;
  MYSQL *m_con;
  wb_dbms_txn *m_txn;
  pwr_tObjName m_volumeName;

  wb_dbms_table *m_t_ohead;
  wb_dbms_table *m_t_rbody;
  wb_dbms_table *m_t_dbody;
  wb_dbms_table *m_t_class;
  wb_dbms_table *m_t_name;
  wb_dbms_table *m_t_info;

  char m_buf[65532];
private:
  int open();
  int createDb();
public:

  wb_dbms();
  wb_dbms(pwr_tVid vid);
  ~wb_dbms() {;}

  int initTables();

  inline MYSQL *con() {
    return m_con;
  }

  inline wb_dbms_table *tOhead() {
    return m_t_ohead;
  }
  inline wb_dbms_table *tRbody() {
    return m_t_rbody;
  } 
  inline wb_dbms_table *tDbody() {
    return m_t_dbody;
  } 
  inline wb_dbms_table *tClass() {
    return m_t_class;
  } 
  inline wb_dbms_table *tName() {
    return m_t_name;
  } 
  inline wb_dbms_table *tInfo() {
    return m_t_info;
  } 

  pwr_tCid cid() { return m_cid;}
  pwr_tVid vid() { return m_vid;}
  //pwr_tTime time() { return m_volume.time;}
  const char *volumeName() { return m_volumeName;}

  pwr_tOid new_oid(wb_dbms_txn *txn);
  pwr_tOid new_oid(wb_dbms_txn *txn, pwr_tOid oid);


  void open(const char *fileName);
  void open(wb_dbms_env *env);

  void copy(wb_export &e, const char *fileName);
  void copy(wb_export &e, wb_dbms_env *env);
  void copy(wb_export &e, pwr_tVid vid, pwr_tCid cid, const char *volumeName, const char *fileName);
  void copy(wb_export &e, pwr_tVid vid, pwr_tCid cid, const char *volumeName, wb_dbms_env *env);

  void create(pwr_tVid vid, pwr_tCid cid, const char *volumeName, const char *fileName);
  void create(pwr_tVid vid, pwr_tCid cid, const char *volumeName, wb_dbms_env *env);

  int close();

  int del_family(wb_dbms_txn *txn, wb_dbms_cursor *cp, pwr_tOid poid);

  wb_dbms_txn *begin(wb_dbms_txn *txn);
  wb_dbms_txn *subBegin(wb_dbms_txn *txn);

  bool commit(pwr_tStatus *sts);

  bool abort(pwr_tStatus *sts);

  //void adopt(wb_dbms_txn *txn, wb_dbms_ohead &o, wb_destination &dest);

  //void unadopt(wb_dbms_txn *txn, wb_dbms_ohead &o);

  bool deleteFamily(pwr_tStatus *sts, wb_dbms_ohead *o);

  //bool deleteOset(pwr_tStatus *sts, wb_oset *o);

  bool importVolume(wb_export &e);

  bool importHead(pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
                  pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid, pwr_tOid loid,
                  const char *name, const char *normname, pwr_mClassDef flags,
                  pwr_tTime ohTime, pwr_tTime rbTime, pwr_tTime dbTime,
                  size_t rbSize, size_t dbSize);

  bool importRbody(pwr_tOid oid, size_t size, void *body);

  bool importDbody(pwr_tOid oid, size_t size, void *body);

  bool importDocBlock(pwr_tOid oid, size_t size, char *block) { return true;}

  bool importMeta(dbs_sMenv *mep);

  void checkClassList(pwr_tOid oid, pwr_tCid cid, bool update);

};

class wb_dbms_info
{
public:
  wb_dbms *m_db;

  struct
  {
    pwr_tVid vid;
    pwr_tCid cid;
    pwr_tTime time;
    pwr_tObjName name;
  } m_volume;


  wb_dbms_qe m_key;
  wb_dbms_qe m_data;

  wb_dbms_info(wb_dbms *db);
  ~wb_dbms_info() {;}

  void ins(wb_dbms_txn *txn);
  void upd(wb_dbms_txn *txn);
  void get(wb_dbms_txn *txn);

  pwr_tCid cid() { return m_volume.cid;}
  pwr_tVid vid() { return m_volume.vid;}
  pwr_tTime time() { return m_volume.time;}
  char *name() { return m_volume.name;}

  void cid(pwr_tCid cid) { m_volume.cid = cid;}
  void vid(pwr_tVid vid) { m_volume.vid = vid;}
  void time(pwr_tTime time) { m_volume.time = time;}
  void name(char const *name) { strcpy(m_volume.name, name);}

};

class wb_dbms_ohead
{
public:
  dbms_sObject m_o;
  pwr_tOid m_oid;

  wb_dbms *m_db;
  wb_dbms_qe m_key;
  wb_dbms_qe m_data;

  wb_dbms_cursor *m_dbc;

  wb_dbms_ohead();
  wb_dbms_ohead(wb_dbms *db);
  wb_dbms_ohead(wb_dbms *db, pwr_tOid oid);
  wb_dbms_ohead(wb_dbms *db, wb_dbms_txn *txn, pwr_tOid oid);
  wb_dbms_ohead(wb_dbms *db, pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
              pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid, pwr_tOid loid,
              const char *name, const char *normname, pwr_mClassDef flags,
              pwr_tTime ohTime, pwr_tTime rbTime, pwr_tTime dbTime,
              size_t rbSize, size_t dbSize);

  wb_dbms_ohead &get(wb_dbms_txn *txn);
  wb_dbms_ohead &get(wb_dbms_txn *txn, pwr_tOid oid);

  void setDb(wb_dbms *db) { m_db = db;}

  int ins(wb_dbms_txn *txn);
  int upd(wb_dbms_txn *txn);
  int del(wb_dbms_txn *txn);

  pwr_tOid oid() { return m_o.oid;}
  pwr_tVid vid() { return m_o.oid.vid;}
  pwr_tOix oix() { return m_o.oid.oix;}
  pwr_tCid cid() { return m_o.cid;}
  pwr_tOid poid() { return m_o.poid;}
  pwr_tOid foid() { return m_o.foid;}
  pwr_tOid loid() { return m_o.loid;}
  pwr_tOid boid() { return m_o.boid;}
  pwr_tOid aoid() { return m_o.aoid;}
  pwr_tTime ohTime() { return m_o.time;}

  const char *name() { return m_o.name;}

  const char *normname() {return m_o.normname;}

  pwr_mClassDef flags() { return m_o.flags;}

  size_t rbSize() { return m_o.body[0].size;}
  size_t dbSize() { return m_o.body[1].size;}
  pwr_tTime rbTime() { return m_o.body[0].time;}
  pwr_tTime dbTime() { return m_o.body[1].time;}

  void name(wb_name &name);
  void name(pwr_tOid &oid);

  void oid(pwr_tOid oid)  { m_o.oid = m_oid = oid;}

  void cid(pwr_tCid cid)  { m_o.cid = cid;}
  void poid(pwr_tOid oid) { m_o.poid = oid;}
  void foid(pwr_tOid oid) { m_o.foid = oid;}
  void loid(pwr_tOid oid) { m_o.loid = oid;}
  void boid(pwr_tOid oid) { m_o.boid = oid;}
  void aoid(pwr_tOid oid) { m_o.aoid = oid;}
  void flags(pwr_mClassDef flags) { m_o.flags = flags;}

  void rbSize(size_t size) { m_o.body[0].size = size;}
  void dbSize(size_t size) { m_o.body[1].size = size;}
  void ohTime(pwr_tTime &time) { m_o.time = time;}
  void rbTime(pwr_tTime &time) { m_o.body[0].time = time;}
  void dbTime(pwr_tTime &time) { m_o.body[1].time = time;}

  void clear();

  void iter(void (*func)(pwr_tOid oid, dbms_sObject *op)); // select oix, o from ohead
  void iter(wb_import &i);                               // select oix, o from ohead
};

typedef struct
{
  pwr_tOid     poid;
  pwr_tObjName normname;
} dbms_sNameKey;

typedef struct
{
  pwr_tOid oid;
} dbms_sNameData;

class wb_dbms_name
{
public:

  dbms_sNameKey  m_k;
  dbms_sNameData m_d;

  wb_dbms *m_db;
  wb_dbms_qe m_key;
  wb_dbms_qe m_data;
  wb_dbms_cursor *m_dbc;

  wb_dbms_name(wb_dbms *db, wb_dbms_txn *txn);
  wb_dbms_name(wb_dbms *db, wb_dbms_ohead &o);
  wb_dbms_name(wb_dbms *db, pwr_tOid poid, const char *name);
  wb_dbms_name(wb_dbms *db, pwr_tOid oid, pwr_tOid poid, const char *name);
  wb_dbms_name(wb_dbms *db, wb_dbms_txn *txn, pwr_tOid poid, wb_name &name);

  int get(wb_dbms_txn *txn);
  int ins(wb_dbms_txn *txn);
  int upd(wb_dbms_txn *txn);

  int del(wb_dbms_txn *txn);

  void name(wb_name &name);
  void iter(void (*print)(pwr_tOid poid, pwr_tObjName name, pwr_tOid oid));

  pwr_tOid oid() { return m_d.oid;}
};

class wb_dbms_class
{
public:
  struct
  {
    pwr_tOix oix;
    pwr_tCid cid;
  } m_k;

  wb_dbms *m_db;

  wb_dbms_qe m_key;
  wb_dbms_cursor *m_dbc;

  wb_dbms_class(wb_dbms *db);
  wb_dbms_class(wb_dbms *db, pwr_tCid cid);
  wb_dbms_class(wb_dbms *db, pwr_tCid cid, pwr_tOid oid);
  wb_dbms_class(wb_dbms *db, wb_dbms_ohead &o);
  wb_dbms_class(wb_dbms *db, wb_dbms_txn *txn, pwr_tCid cid);
  ~wb_dbms_class();

  bool succ(pwr_tOid oid);
  bool succClass(pwr_tCid cid);
  bool pred(pwr_tOid oid);
  int ins(wb_dbms_txn *txn);
  int upd(wb_dbms_txn *txn);
  int del(wb_dbms_txn *txn);

  pwr_tCid cid() { return m_k.cid;}
  pwr_tOid oid() {
    pwr_tOid oid;
    oid.oix = m_k.oix;
    oid.vid = m_db->vid();
    return oid;
  }

  void iter(void (*func)(pwr_tOid oid, pwr_tCid cid));
};

class wb_dbms_class_iterator
{
  struct
  {
    pwr_tOix oix;
    pwr_tCid cid;
  } m_k;

  wb_dbms *m_db;
  wb_dbms_qe m_key;
  bool m_atEnd;
  int m_rc;


public:
  wb_dbms_class_iterator(wb_dbms *db);
  wb_dbms_class_iterator(wb_dbms *db, pwr_tCid cid);
  wb_dbms_class_iterator(wb_dbms *db, pwr_tCid cid, pwr_tOid oid);
  ~wb_dbms_class_iterator();

    
  inline bool atEnd() {return m_atEnd;}
  bool first();
  bool succObject();
  bool succClass();
  bool succClass(pwr_tCid cid);

  inline pwr_tOid oid() {
    pwr_tOid oid;
    oid.vid = m_db->vid();
    oid.oix = m_k.oix;
    return oid;
  }
  inline pwr_tCid cid() {return m_k.cid;}
  inline void oid(pwr_tOid oid) {m_k.oix = oid.oix;}
  inline void cid(pwr_tCid cid) {m_k.cid = cid;}
};

class wb_dbms_dbody
{
public:
  wb_dbms *m_db;

  pwr_tOid m_oid;
  size_t m_size;
  void *m_p;  

  wb_dbms_qe m_key;
  wb_dbms_qe m_data;
  wb_dbms_cursor *m_dbc;

  wb_dbms_dbody(wb_dbms *db);
  wb_dbms_dbody(wb_dbms *db, pwr_tOid oid);
  wb_dbms_dbody(wb_dbms *db, pwr_tOid oid, size_t size, void *p);

  void oid(pwr_tOid oid) {m_oid = oid;}

  int get(wb_dbms_txn *txn, size_t offset, size_t size, void *p);
  int ins(wb_dbms_txn *txn);
  int ins(wb_dbms_txn *txn, size_t offset, size_t size, void *p);
  int upd(wb_dbms_txn *txn);
  int upd(wb_dbms_txn *txn, size_t offset, size_t size, void *p);
  int del(wb_dbms_txn *txn);

  void iter(void (*print)(pwr_tOid oid, size_t size));
  void iter(wb_import &i);
};

class wb_dbms_rbody
{
public:
  wb_dbms *m_db;

  pwr_tOid m_oid;
  size_t m_size;
  void *m_p;

  wb_dbms_qe m_key;
  wb_dbms_qe m_data;
  wb_dbms_cursor *m_dbc;

  wb_dbms_rbody(wb_dbms *db);
  wb_dbms_rbody(wb_dbms *db, pwr_tOid oid);
  wb_dbms_rbody(wb_dbms *db, pwr_tOid oid, size_t size, void *p);

  void oid(pwr_tOid oid) {m_oid = oid;}

  int get(wb_dbms_txn *txn, size_t offset, size_t size, void *p);
  int ins(wb_dbms_txn *txn);
  int ins(wb_dbms_txn *txn, size_t offset, size_t size, void *p);
  int upd(wb_dbms_txn *txn);
  int upd(wb_dbms_txn *txn, size_t offset, size_t size, void *p);
  int del(wb_dbms_txn *txn);

  void iter(void (*print)(pwr_tOid oid, size_t size));
  void iter(wb_import &i);
};



class wb_dbms_get_query : public wb_dbms_query
{
public:
  wb_dbms_get_query() {;}
  virtual ~wb_dbms_get_query() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) = 0;

  virtual int execute(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    int rc = 0;
    static const char *method = "execute";
  
    bind(txn, key, data);
  
    rc = mysql_stmt_execute(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_execute");

    rc = mysql_stmt_store_result(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_store_result");

    if (mysql_stmt_num_rows(m_stmt) == 0)
      return 1;
  
    rc = mysql_stmt_fetch(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_fetch");
  
    mysql_stmt_free_result(m_stmt);
  
    return 0;
  }
};

class wb_dbms_iter_query : public wb_dbms_query
{
public:
  wb_dbms_iter_query() {;}
  virtual ~wb_dbms_iter_query() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) = 0;

  virtual int execute(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    int rc = 0;
    static const char *method = "execute";
  
    bind(txn, key, data);
  
    rc = mysql_stmt_execute(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_execute");

    rc = mysql_stmt_store_result(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_store_result");

    if (mysql_stmt_num_rows(m_stmt) == 0)
      return 1;
  
    rc = mysql_stmt_fetch(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_fetch");
  
    mysql_stmt_free_result(m_stmt);
  
    if (m_result.is_null[0])
      return 1;
    
    return 0;
  }
};

class wb_dbms_ins_query : public wb_dbms_query
{
public:
  wb_dbms_ins_query() {;}
  virtual ~wb_dbms_ins_query() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) = 0;

  virtual int execute(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    int rc = 0;
    static const char *method = "execute";
  
    bind(txn, key, data);
  
    rc = mysql_stmt_execute(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_execute");

    if (mysql_stmt_affected_rows(m_stmt) == 1)
      return 0;
  
    return 1;
  }
};

class wb_dbms_upd_query : public wb_dbms_query
{
public:
  wb_dbms_upd_query() {;}
  virtual ~wb_dbms_upd_query() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) = 0;

  virtual int execute(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    int rc = 0;
    static const char *method = "execute";
  
    bind(txn, key, data);
  
    rc = mysql_stmt_execute(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_execute");

    if (mysql_stmt_affected_rows(m_stmt) == 1)
      return 0;
  
    return 1;
  }
};

class wb_dbms_del_query : public wb_dbms_query
{
public:
  wb_dbms_del_query() {;}
  virtual ~wb_dbms_del_query() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) = 0;

  virtual int execute(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    int rc = 0;
    static const char *method = "execute";
  
    bind(txn, key, data);
  
    rc = mysql_stmt_execute(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_execute");

    if (mysql_stmt_affected_rows(m_stmt) == 1)
      return 0;
  
    return 1;
  }
};

class wb_dbms_cursor_query : public wb_dbms_query
{
public:
  wb_dbms_cursor_query() {;}
  virtual ~wb_dbms_cursor_query() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) = 0;

  virtual int execute(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    int rc = 0;
    static const char *method = "execute";
  
    bind(txn, key, data);
  
    rc = mysql_stmt_execute(m_stmt);
    if (rc) error(rc, method, "mysql_stmt_execute");

    return 0;
  }
};

class wb_dbms_get_info : public wb_dbms_get_query
{
public:
  wb_dbms_get_info(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_get_info() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select volume from info where id = ?", 1, 1); 

    bindResult(0, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
    bindParam(0,  MYSQL_TYPE_LONG,      (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_ins_info : public wb_dbms_ins_query
{
public:
  wb_dbms_ins_info(wb_dbms *db) { m_db = db;}
  ~wb_dbms_ins_info() {;}
  
  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("insert into info (id, volume) values (?, ?)", 0, 2);

    bindParam(0, MYSQL_TYPE_LONG,      (char *) key->data(),  key->size());
    bindParam(1, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_upd_info : public wb_dbms_upd_query
{
public:

  wb_dbms_upd_info(wb_dbms *db) { m_db = db;}
  ~wb_dbms_upd_info() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("update info set volume = ? where id = ?", 0, 2);

    bindParam(0, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
    bindParam(1, MYSQL_TYPE_LONG,      (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_del_info : public wb_dbms_del_query
{
public:

  wb_dbms_del_info(wb_dbms *db) { m_db = db;}
  ~wb_dbms_del_info() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("delete from inf where id = ?", 0, 1);

    bindParam(0, MYSQL_TYPE_LONG, (char *)key->data(), key->size());
    bindQuery();

    return 0;
  }
};
  
class wb_dbms_get_ohead : public wb_dbms_get_query
{
public:
  wb_dbms_get_ohead(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_get_ohead() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select head from ohead where oid = ?", 1, 1); 

    bindResult(0, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
    bindParam(0,  MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_ins_ohead : public wb_dbms_ins_query
{
public:
  wb_dbms_ins_ohead(wb_dbms *db) { m_db = db;}
  ~wb_dbms_ins_ohead() {;}
  
  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("insert into ohead (oid, head) values (?, ?)", 0, 2);

    bindParam(0, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindParam(1, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_upd_ohead : public wb_dbms_upd_query
{
public:

  wb_dbms_upd_ohead(wb_dbms *db) { m_db = db;}
  ~wb_dbms_upd_ohead() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("update ohead set head = ? where oid = ?", 0, 2);

    bindParam(0, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
    bindParam(1, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_del_ohead : public wb_dbms_del_query
{
public:

  wb_dbms_del_ohead(wb_dbms *db) { m_db = db;}
  ~wb_dbms_del_ohead() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("delete from ohead where oid = ?", 0, 1);

    bindParam(0, MYSQL_TYPE_LONGLONG, (char *)key->data(), key->size());
    bindQuery();

    return 0;
  }
};  

class wb_dbms_cursor_ohead : public wb_dbms_cursor_query
{
public:
  wb_dbms_cursor_ohead(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_cursor_ohead() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select oid, head from ohead", 2, 0); 

    bindResult(0, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindResult(1, MYSQL_TYPE_LONG_BLOB, data);
    bindQuery();

    return 0;
  }
};

class wb_dbms_get_dbody : public wb_dbms_get_query
{
public:
  wb_dbms_get_dbody(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_get_dbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select body from dbody where oid = ?", 1, 1); 

    bindResult(0, MYSQL_TYPE_LONG_BLOB, data);
    bindParam(0,  MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_ins_dbody : public wb_dbms_ins_query
{
public:
  wb_dbms_ins_dbody(wb_dbms *db) { m_db = db;}
  ~wb_dbms_ins_dbody() {;}
  
  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("insert into dbody (oid, body) values (?, ?)", 0, 2);

    bindParam(0, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindParam(1, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_upd_dbody : public wb_dbms_upd_query
{
public:

  wb_dbms_upd_dbody(wb_dbms *db) { m_db = db;}
  ~wb_dbms_upd_dbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("update dbody set body = ? where oid = ?", 0, 2);

    bindParam(0, MYSQL_TYPE_LONG_BLOB, data);
    bindParam(1, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

#if 0
class wb_dbms_pupd_dbody : public wb_dbms_upd_query
{
public:

  wb_dbms_pupd_dbody(wb_dbms *db) { m_db = db;}
  ~wb_dbms_pupd_dbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    //prepare("update dbody set body = concat(substring(body, 1, ?), ?, substring(body, ?, 500000)) where oid = ?", 0, 4);

/*
   update rbody set body = concat(substring(body, 1, 50), repeat('x', 10), substring(body, 61, 68)) where oid = 853238203023360;

 */
    int size_1 = 10;//data->offset();
    int size_2 = 20;//data->offset() + data->size() + 1;
    pwr_tOid oid;
    oid.vid = 198660;
    oid.oix = 76202;
    //char a[] = "9";
    //char b[] = "344";
    
    
    //bindParam(0, MYSQL_TYPE_LONG,      (char *)         &size_1);
    //bindParam(0, MYSQL_TYPE_STRING,      a, strlen(a));
    bindParam(0, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
//    bindParam(2, MYSQL_TYPE_LONG,      (char *)         &size_2, sizeof(size_2));
    //bindParam(2, MYSQL_TYPE_LONG,      (char *)         &size_2);
    //bindParam(2, MYSQL_TYPE_STRING,      b, strlen(b));
    bindParam(1, MYSQL_TYPE_LONGLONG,  (char *)            &oid);
    //bindParam(3, MYSQL_TYPE_LONGLONG,  (char *) key->data(), key->size());
    bindQuery();

    return 0;
  }
};
#endif

class wb_dbms_del_dbody : public wb_dbms_del_query
{
public:

  wb_dbms_del_dbody(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_del_dbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("delete from dbody where oid = ?", 0, 1);

    bindParam(0, MYSQL_TYPE_LONGLONG, (char *)key->data(), key->size());
    bindQuery();

    return 0;
  }
};
  
class wb_dbms_cursor_dbody : public wb_dbms_cursor_query
{
public:
  wb_dbms_cursor_dbody(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_cursor_dbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select oid, body from dbody", 2, 0); 

    bindResult(0, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindResult(1, MYSQL_TYPE_LONG_BLOB, data);
    bindQuery();

    return 0;
  }
};

class wb_dbms_get_rbody : public wb_dbms_get_query
{
public:
  wb_dbms_get_rbody(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_get_rbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select body from rbody where oid = ?", 1, 1); 

    bindResult(0, MYSQL_TYPE_LONG_BLOB, data);
    bindParam(0,   MYSQL_TYPE_LONGLONG, (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_ins_rbody : public wb_dbms_ins_query
{
public:
  wb_dbms_ins_rbody(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_ins_rbody() {;}
  
  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("insert into rbody (oid, body) values (?, ?)", 0, 2);

    bindParam(0, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindParam(1, MYSQL_TYPE_LONG_BLOB, (char *)data->data(), data->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_upd_rbody : public wb_dbms_upd_query
{
public:

  wb_dbms_upd_rbody(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_upd_rbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("update rbody set body = ? where oid = ?", 0, 2);

    bindParam(0, MYSQL_TYPE_LONG_BLOB, data);
    bindParam(1, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_del_rbody : public wb_dbms_del_query
{
public:

  wb_dbms_del_rbody(wb_dbms *db) { m_db = db;}
  ~wb_dbms_del_rbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("delete from rbody where oid = ?", 0, 1);

    bindParam(0, MYSQL_TYPE_LONGLONG, (char *)key->data(), key->size());
    bindQuery();
    
    return 0;
  }
};  

class wb_dbms_cursor_rbody : public wb_dbms_cursor_query
{
public:
  wb_dbms_cursor_rbody(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_cursor_rbody() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select oid, body from rbody", 2, 0); 

    bindResult(0, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindResult(1, MYSQL_TYPE_LONG_BLOB, data);
    bindQuery();

    return 0;
  }
};

class wb_dbms_get_class : public wb_dbms_get_query
{
public:
  wb_dbms_get_class(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_get_class() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select cidoix from class where cidoix = ?", 1, 1);

    bindResult(0, MYSQL_TYPE_LONGLONG,   (char *) key->data(),  key->size());
    bindParam(0,  MYSQL_TYPE_LONGLONG,   (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_succ_class : public wb_dbms_iter_query
{
public:
  wb_dbms_succ_class(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_succ_class() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select min(cidoix) from class where cidoix > ?", 1, 1);

    bindResult(0, MYSQL_TYPE_LONGLONG, (char *)key->data(), key->size());
    bindParam(0,  MYSQL_TYPE_LONGLONG, (char *)key->data(), key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_pred_class : public wb_dbms_iter_query
{
public:
  wb_dbms_pred_class(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_pred_class() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select max(cidoix) from class where cidoix < ?", 1, 1);

    bindResult(0, MYSQL_TYPE_LONGLONG, (char *)key->data(), key->size());
    bindParam(0,  MYSQL_TYPE_LONGLONG, (char *)key->data(), key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_ins_class : public wb_dbms_ins_query
{
public:
  wb_dbms_ins_class(wb_dbms *db) { m_db = db;}
  ~wb_dbms_ins_class() {;}
  
  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("insert into class (cidoix) values (?)", 0, 1);

    bindParam(0, MYSQL_TYPE_LONGLONG,  (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_del_class : public wb_dbms_del_query
{
public:

  wb_dbms_del_class(wb_dbms *db) { m_db = db;}
  ~wb_dbms_del_class() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("delete from class where cidoix = ?", 0, 1);

    bindParam(0, MYSQL_TYPE_LONGLONG, (char *)key->data(), key->size());
    bindQuery();

    return 0;
  }
};
  
class wb_dbms_cursor_class : public wb_dbms_cursor_query
{
public:
  wb_dbms_cursor_class(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_cursor_class() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select cidoix from class", 1, 0);

    bindResult(0, MYSQL_TYPE_LONGLONG,   (char *) key->data(),  key->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_get_name : public wb_dbms_get_query
{
public:
  wb_dbms_get_name(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_get_name() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select oid from name where poid = ? and normname = ?", 1, 2); 

    dbms_sNameKey *nk = (dbms_sNameKey *)key->data();
  
    bindResult(0, MYSQL_TYPE_LONGLONG,  (char *)data->data(),  data->size());
    bindParam(0,  MYSQL_TYPE_LONGLONG,  (char *)&nk->poid,     sizeof(nk->poid));
    bindParam(1,  MYSQL_TYPE_TINY_BLOB, (char *)&nk->normname, sizeof(nk->normname));
    bindQuery();

    return 0;
  }
};

class wb_dbms_ins_name : public wb_dbms_ins_query
{
public:
  wb_dbms_ins_name(wb_dbms *db) { m_db = db;}
  ~wb_dbms_ins_name() {;}
  
  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("insert into name (poid, normname, oid) values (?, ?, ?)", 0, 3);

    dbms_sNameKey *nk = (dbms_sNameKey *)key->data();
  
    bindParam(0, MYSQL_TYPE_LONGLONG,  (char *)&nk->poid,     sizeof(nk->poid));
    bindParam(1, MYSQL_TYPE_TINY_BLOB, (char *)&nk->normname, sizeof(nk->normname));
    bindParam(2, MYSQL_TYPE_LONGLONG,  (char *)data->data(),  data->size());
    bindQuery();

    return 0;
  }
};

class wb_dbms_del_name : public wb_dbms_del_query
{
public:

  wb_dbms_del_name(wb_dbms *db) { m_db = db;}
  ~wb_dbms_del_name() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("delete from name where poid = ? and normname = ?", 0, 2);

    dbms_sNameKey *nk = (dbms_sNameKey *)key->data();
  
    bindParam(0, MYSQL_TYPE_LONGLONG,  (char *)&nk->poid,     sizeof(nk->poid));
    bindParam(1, MYSQL_TYPE_TINY_BLOB, (char *)&nk->normname, sizeof(nk->normname));
    bindQuery();

    return 0;
  }
};
  
class wb_dbms_cursor_name : public wb_dbms_cursor_query
{
public:
  wb_dbms_cursor_name(wb_dbms *db) { m_db = db;}
  virtual ~wb_dbms_cursor_name() {;}

  virtual int bind(wb_dbms_txn *txn, wb_dbms_qe *key, wb_dbms_qe *data) {
    prepare("select poid, normname, oid from name", 3, 0); 

    dbms_sNameKey *nk = (dbms_sNameKey *)key->data();
  
    bindResult(0, MYSQL_TYPE_LONGLONG,  (char *)&nk->poid,     sizeof(nk->poid));
    bindResult(1, MYSQL_TYPE_TINY_BLOB, (char *)&nk->normname, sizeof(nk->normname));
    bindResult(2, MYSQL_TYPE_LONGLONG,  (char *)data->data(),  data->size());
    bindQuery();

    return 0;
  }
};


using namespace std;

class wb_dbms_error
{
  string m_error_str;
  string m_sql_error;
  int m_errno;
  wb_dbms *m_db;

public:

  wb_dbms_error(wb_dbms *db, string str);
  string what() const;
};
#endif
#endif

  
