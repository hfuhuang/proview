#ifndef wb_db_h
#define wb_db_h

#include "pwr.h"
#include "db_cxx.h"
#include "wb_import.h"

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
    pwr_tTime      time;
    pwr_tUInt32    size;
  } body[2];    /**< bodies */
} db_sObject;

class wb_db_txn;
class wb_db_ohead;
class wb_destination;

class wb_db : public wb_import
{
public:
  pwr_tVid m_vid;

  DbEnv *m_env;
  Db *m_ohead;
  Db *m_obody;
  Db *m_class;
  Db *m_name;
  Db *m_info;

  wb_db_txn *m_txn;    
    
    
    
  //wb_db_ohead m_ohead;
    

public:
    
  wb_db();
  //~wb_db();

  pwr_tOid new_oid(wb_db_txn *txn);

  void close();
  void open(char *name);
  
  int del_family(wb_db_txn *txn, Dbc *cp, pwr_tOid poid);
  
  wb_db_txn *begin(wb_db_txn *txn);

  bool commit(wb_db_txn *txn);
  
  bool abort(wb_db_txn *txn);
  
  void adopt(wb_db_txn *txn, wb_db_ohead &o, wb_destination &dest);
  
  void unadopt(wb_db_txn *txn, wb_db_ohead &o);
  
  bool deleteFamily(pwr_tStatus *sts, wb_db_ohead *o);
  
  //bool deleteOset(pwr_tStatus *sts, wb_oset *o);
  
  bool importVolume(wb_export *e);
  
  bool importHead(pwr_tOid oid, pwr_tCid cid, pwr_tOid poid, pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid,
                  pwr_tOid loid, pwr_tObjName name, pwr_tObjName normname, pwr_tTime ohTime,
                  pwr_tTime rbTime, pwr_tTime dbTime, size_t rbSize, size_t dbSize);

  bool importRbody(pwr_tOid oid, size_t size, void *body);
  
  bool importDbody(pwr_tOid oid, size_t size, void *body);
  
  bool importMeta(const char *fileName);
  

};

class wb_db_ohead
{
public:
  db_sObject m_o;

  wb_db *m_db;
  Dbt m_key;
  Dbt m_data;

  wb_db_ohead();
  wb_db_ohead(wb_db *db);
  wb_db_ohead(wb_db *db, pwr_tOid oid);
  wb_db_ohead(wb_db *db, wb_db_txn *txn, pwr_tOid oid);
  wb_db_ohead(wb_db *db, pwr_tOid oid, pwr_tCid cid,
              pwr_tOid poid, pwr_tOid boid, pwr_tOid aoid, pwr_tOid foid, pwr_tOid loid,
              pwr_tObjName name, pwr_tObjName normname,
              pwr_tTime ohTime, pwr_tTime rbTime, pwr_tTime dbTime,
              size_t rbSize, size_t dbSize);

  wb_db_ohead &get(wb_db_txn *txn);
  wb_db_ohead &get(pwr_tOid oid);
  wb_db_ohead &get(wb_db_txn *txn, pwr_tOid oid);

  void put(wb_db_txn *txn);
  void del(wb_db_txn *txn);
        
  pwr_tOid oid() { return m_o.oid;}
  pwr_tCid cid() { return m_o.cid;}
  pwr_tOid poid() { return m_o.poid;}
  pwr_tOid foid() { return m_o.foid;}
  pwr_tOid loid() { return m_o.loid;}
  pwr_tOid boid() { return m_o.boid;}
  pwr_tOid aoid() { return m_o.aoid;}

  char *name();
  void name(wb_name &name);
        
  void poid(pwr_tOid oid) { m_o.poid = oid;}
  void foid(pwr_tOid oid) { m_o.foid = oid;}
  void loid(pwr_tOid oid) { m_o.loid = oid;}
  void boid(pwr_tOid oid) { m_o.boid = oid;}
  void aoid(pwr_tOid oid) { m_o.aoid = oid;}

  void clear();
};
    
class wb_db_name
{
public:
  struct
  {
    pwr_tOid     poid;
    pwr_tObjName normname;
  } m_k;
  struct
  {
    pwr_tOid      oid;
    //pwr_tCid      cid;   // saved here to optimize tree traversal
    //pwr_mClassDef flags; // saved here to optimize tree traversal
  } m_d;
        
  wb_db *m_db;
  Dbt m_key;
  Dbt m_data;
        
  wb_db_name(wb_db *db, wb_db_txn *txn);
  wb_db_name(wb_db *db, wb_db_ohead &o);
  wb_db_name(wb_db *db, pwr_tOid, char *name);
  wb_db_name(wb_db *db, pwr_tOid poid, const char *name);
  wb_db_name(wb_db *db, wb_db_txn *txn, pwr_tOid poid, wb_name name);
        
  void get(wb_db_txn *txn);
  void put(wb_db_txn *txn);
  void del(wb_db_txn *txn);
        
  void name(wb_name &name);
        
  pwr_tOid oid() { return m_d.oid;}
};

class wb_db_class
{
public:
  struct 
  {
    pwr_tCid cid;
    pwr_tOid oid;
  } m_k;

  wb_db *m_db;
  
  Dbt m_key;
  Dbc *m_dbc;
        
  wb_db_class(wb_db *db, pwr_tCid cid);
  wb_db_class(wb_db *db, pwr_tCid cid, pwr_tOid oid);
  wb_db_class(wb_db *db, wb_db_ohead &o);
  wb_db_class(wb_db *db, wb_db_txn *txn, pwr_tCid cid);
  ~wb_db_class();

  bool succ(pwr_tOid oid);
  bool pred(pwr_tOid oid);
  void put(wb_db_txn *txn);
  void del(wb_db_txn *txn);
        
  pwr_tCid cid() { return m_k.cid;}
  pwr_tOid oid() { return m_k.oid;}
        
                
};

class wb_db_body
{
public:
  wb_db_body(wb_db *db, pwr_tOid oid, size_t size, void *p);
  
  void put(wb_db_txn *txn);
};

class wb_db_txn : public DbTxn
{

};


#endif
