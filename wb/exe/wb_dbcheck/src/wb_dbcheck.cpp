
#include "pwr.h"
#include "wb_erep.h"
#include "wb_env.h"
#include "wb_vrepdbs.h"
#include "wb_vrep.h"
#include "wb_dbs.h"
#include "wb_db.h"
#include "wb_vrepwbl.h"
#include "wb_vrepdbs.h"
#include "wb_orepdbs.h"
#include "wb_orepwbl.h"
#include "wb_volume.h"
#include "wb_session.h"
#include "wb_error.h"
#include "co_dbs.h"
#include "co_time.h"
#include "co_tree.h"

static int count_name = 0;
static int count_class = 0;
static int count_ohead = 0;
static int count_rbody = 0;
static int count_dbody = 0;

static void check(wb_db *dp);
static void printname(pwr_tOid poid, pwr_tObjName name, pwr_tOid oid);
static void printclass(pwr_tOid oid, pwr_tCid cid);
static void printohead(pwr_tOid oid, db_sObject *op);
static void printrbody(pwr_tOid oid);
static void printdbody(pwr_tOid oid);


typedef union {
  struct {
    pwr_tBit    inOhead       : 1;
    pwr_tBit    inName        : 1;
    pwr_tBit    inClass       : 1;
    pwr_tBit    inRbody       : 1;
    pwr_tBit    inDbody       : 1;
    
    pwr_tBit    pOk           : 1;
    pwr_tBit    bOk           : 1;
    pwr_tBit    aOk           : 1;
    pwr_tBit    fOk           : 1;
    pwr_tBit    lOk           : 1;
  } b;
  
  pwr_tBitMask m;

#define mOentry_inOhead		      0x001
#define mOentry_inName		      0x002
#define mOentry_inClass		      0x004
#define mOentry_inRbody		      0x008
#define mOentry_inDbody		      0x010
#define mOentry_pOk		          0x020
#define mOentry_bOk		          0x040
#define mOentry_aOk		          0x080
#define mOentry_fOk		          0x100
#define mOentry_lOk		          0x200

#define mOentry_all             0x3ff
} mOentry;

  struct sOentry;
  struct sNentry;
  struct sCentry;

  struct sOentry {
    tree_sNode       node;
    db_sObject       o;
    mOentry          flags;

    sOentry         *poep;
    sOentry         *boep;
    sOentry         *aoep;
    sOentry         *foep;
    sOentry         *loep;
  };

static tree_sTable  *oid_th;

int main( int argc, char *argv[])
{
  pwr_tStatus sts;
  
  if (argc <= 1) exit(0);

  printf("open database> %s\n", argv[1]);
  
  oid_th = tree_CreateTable(&sts, sizeof(pwr_tOid), offsetof(sOentry, o.oid), sizeof(sOentry), 1000, tree_Comp_oid);

  try {
    
    wb_db *db = new wb_db();
    db->open(argv[1]);

    check(db);
  
    db->close();
  }
  catch (DbException &e) {
    printf("Exception: %s\n", e.what());
  }
  
  for (sOentry *oep = (sOentry *)tree_Minimum(&sts, oid_th); oep != NULL; oep = (sOentry *)tree_Successor(&sts, oid_th, oep)) {
    if (!oep->flags.b.inOhead) printf("[%d] not in ohead\n", oep->o.oid.oix);
    if (!oep->flags.b.inName) printf("[%d] not in name\n", oep->o.oid.oix);
    if (!oep->flags.b.inClass) printf("[%d] not in class\n", oep->o.oid.oix);
    if (!oep->flags.b.inRbody && oep->o.body[0].size != 0) printf("[%d] not in rbody\n", oep->o.oid.oix);
    if (!oep->flags.b.inDbody && oep->o.body[1].size != 0) printf("[%d] not in dbody\n", oep->o.oid.oix);
    if (oep->flags.b.inRbody && oep->o.body[0].size == 0) printf("[%d] in rbody\n", oep->o.oid.oix);
    if (oep->flags.b.inDbody && oep->o.body[1].size == 0) printf("[%d] in dbody\n", oep->o.oid.oix);
    if (!oep->flags.b.pOk) {
#if 1
      if (oep->poep != NULL && oep->poep->flags.b.inOhead) {
        oep->flags.b.pOk = 1;
      } else {
        printf("[%d] pNok %s\n", oep->o.oid.oix, oep->o.name);
      }
#else
      printf("[%d] pNok %s\n", oep->o.oid.oix, oep->o.name);
#endif
    }
    if (!oep->flags.b.bOk) printf("[%d] bNok\n", oep->o.oid.oix);
    if (!oep->flags.b.aOk) printf("[%d] aNok\n", oep->o.oid.oix);
    if (!oep->flags.b.fOk) printf("[%d] fNok\n", oep->o.oid.oix);
    if (!oep->flags.b.lOk) printf("[%d] lNok\n", oep->o.oid.oix);
  }
  
}

static void
check(wb_db *db)
{
  printf("check()\n");
  wb_db_ohead *op = new wb_db_ohead(db);
  
  op->iter(printohead);
  
  wb_db_name *np = new wb_db_name(db, NULL);
  
  np->iter(printname);

  wb_db_class *cp = new wb_db_class(db);
  
  cp->iter(printclass);

  wb_db_rbody *rp = new wb_db_rbody(db);

  rp->iter(printrbody);
  
  wb_db_dbody *dp = new wb_db_dbody(db);
  
  dp->iter(printdbody);

  printf("Ohead: %d\n", count_ohead);
  printf("Name.: %d\n", count_name);
  printf("Class: %d\n", count_class);
  printf("Rbody: %d\n", count_rbody);
  printf("Dbody: %d\n", count_dbody);

}

static void
printname(pwr_tOid poid, pwr_tObjName name, pwr_tOid oid)
{
  pwr_tStatus sts;
  sOentry     *oep;
  count_name++;
    
  printf("N [%10.10d.%10.10d] %d:%s\n", oid.vid, oid.oix, strlen(name), name);

  oep = (sOentry *)tree_Find(&sts, oid_th, &oid);
  if (EVEN(sts)) {
    printf("Name: object not found %d.%d, %s\n", oid.vid, oid.oix, name);
  } else {
    oep->flags.b.inName = 1;
    if (strcmp(name, oep->o.normname) != 0) {
      printf("name: \"%s\" [%d] not same as: \"%s\"\n", oep->o.name, oep->o.oid.oix, name);
    }
#if 0
    if (memcmp(name, oep->o.normname, sizeof(name)) != 0) {
      printf("name memcmp: \"%s\" [%d] not same as: \"%s\"\n", oep->o.name, oep->o.oid.oix, name);
    }
#endif
    if (cdh_ObjidIsNotEqual(poid, oep->o.poid)) {
      printf("name: \"%s\" [%d] poid: [%d] is not same as: [%d]n", oep->o.name, oep->o.oid.oix, oep->o.poid.oix, poid.oix);
    }    
  }
}

static void
printclass(pwr_tOid oid, pwr_tCid cid)
{
  pwr_tStatus sts;
  sOentry     *oep;
  count_class++;

  printf("C [%10.10d.%10.10d] <%d>\n", oid.vid, oid.oix, cid);
  
  oep = (sOentry *)tree_Find(&sts, oid_th, &oid);
  if (EVEN(sts)) {
    printf("Class: object not found %d.%d, %d\n", oid.vid, oid.oix, cid);
  } else {
    oep->flags.b.inClass = 1;
  }

}

static void
printohead(pwr_tOid oid, db_sObject *op)
{
  count_ohead++;

  sOentry     *oep;
  pwr_tStatus  sts;
    
  if (cdh_ObjidIsNull(oid))
    printf("** Error: object is null!\n");

  if (cdh_ObjidIsNotEqual(oid, op->oid))
    printf("Ohead: oid not equal %s oid: %d.%d != %d.%d\n", op->name, oid.vid, oid.oix, op->oid.vid, op->oid.oix);
  
//  printf("O  [%10.10d.%10.10d]   [%10.10d.%10.10d] P [%10.10d.%10.10d] %d:%s\n B [%10.10d.%10.10d] A [%10.10d.%10.10d] F [%10.10d.%10.10d] L [%10.10d.%10.10d] \n", oid.vid, oid.oix, op->oid.vid, op->oid.oix, op->poid.vid, op->poid.oix, strlen(op->name), op->name, op->boid.vid, op->boid.oix, op->aoid.vid, op->aoid.oix, op->foid.vid, op->foid.oix, op->loid.vid, op->loid.oix);
  printf("P [%6.6d] B [%6.6d] O [%6.6d] A [%6.6d] F [%6.6d] L [%6.6d]\n", op->poid.oix, op->boid.oix, op->oid.oix, op->aoid.oix, op->foid.oix, op->loid.oix);

  oep = (sOentry *)tree_Insert(&sts, oid_th, &oid);
  if (sts == TREE__INSERTED) {
    oep->flags.b.inOhead = 1;
    oep->o = *op;
  } else if (!oep->flags.b.inOhead) {
    oep->flags.b.inOhead = 1;
    oep->o = *op;
  }

  if (cdh_ObjidIsNotNull(op->poid)) {
    oep->poep = (sOentry *)tree_Insert(&sts, oid_th, &op->poid);
    if (sts == TREE__FOUND && oep->poep->flags.b.inOhead) {
      oep->flags.b.pOk = 1;
      if (oep == oep->poep->foep) {
        oep->poep->flags.b.fOk = 1;
      }
      if (oep == oep->poep->loep) {
        oep->poep->flags.b.lOk = 1;
      }
    } else {
      
    }
  } else {
    oep->flags.b.pOk = 1;
    printf("This should be the volume object: %s, [%d.%d]\n", oep->o.name, oep->o.oid.vid, oep->o.oid.oix);
  }
  
  
  if (cdh_ObjidIsNotNull(op->boid)) {
    oep->boep = (sOentry *)tree_Insert(&sts, oid_th, &op->boid);
    if (sts == TREE__FOUND && oep->boep->flags.b.inOhead) {
      oep->flags.b.bOk = 1;
      if (oep->boep->flags.b.aOk) {
        printf("Flags: %s [%d] b-object [%d] allready connected to a-object\n", oep->o.name, oep->o.oid.oix, oep->boep->o.oid.oix);
      }
      if (oep->boep->aoep != NULL && oep->boep->aoep != oep) {
        printf("aoep: %s [%d] b-object [%d] allready connected to a-object [%d]\n", oep->o.name, oep->o.oid.oix, oep->boep->o.oid.oix, oep->boep->aoep->o.oid.oix);
      } else {
        oep->boep->aoep = oep;
        oep->boep->flags.b.aOk = 1;
      }
    }
    
  } else {
    oep->flags.b.bOk = 1;
    // todo! check I am first child
  }
  
  
  if (cdh_ObjidIsNotNull(op->aoid)) {
    oep->aoep = (sOentry *)tree_Insert(&sts, oid_th, &op->aoid);
    if (sts == TREE__FOUND && oep->aoep->flags.b.inOhead) {
      oep->flags.b.aOk = 1;
      if (oep->aoep->flags.b.bOk) {
        printf("Flags: %s [%d] a-object [%d] allready connected to b-object\n", oep->o.name, oep->o.oid.oix, oep->aoep->o.oid.oix);
      }
      if (oep->aoep->boep != NULL && oep->aoep->boep != oep) {
        printf("boep: %s [%d] a-object [%d] allready connected to b-object [%d]\n", oep->o.name, oep->o.oid.oix, oep->aoep->o.oid.oix, oep->aoep->boep->o.oid.oix);
      } else {
        oep->aoep->boep = oep;
        oep->aoep->flags.b.bOk = 1;
      }
    }
  } else {
    oep->flags.b.aOk = 1;
    // todo! check I am last child
  }
  
  
  if (cdh_ObjidIsNotNull(op->foid)) {
    oep->foep = (sOentry *)tree_Insert(&sts, oid_th, &op->foid);
    if (sts == TREE__FOUND && oep->foep->flags.b.inOhead) {
      oep->flags.b.fOk = 1;
      if (oep->foep->flags.b.pOk) {
        printf("Flags: %s [%d] f-object [%d] allready connected to p-object\n", oep->o.name, oep->o.oid.oix, oep->foep->o.oid.oix);
      }
      if (oep->foep->poep != NULL && oep->foep->poep != oep) {
        printf("foep: %s [%d] f-object [%d] allready connected to p-object [%d]\n", oep->o.name, oep->o.oid.oix, oep->foep->o.oid.oix, oep->foep->poep->o.oid.oix);
      } else {
        oep->foep->poep = oep;
        oep->foep->flags.b.pOk = 1;
      }
    }
  } else {
    oep->flags.b.fOk = 1;
  }
  
  
  if (cdh_ObjidIsNotNull(op->loid)) {    
    oep->loep = (sOentry *)tree_Insert(&sts, oid_th, &op->loid);
    if (sts == TREE__FOUND && oep->loep->flags.b.inOhead) {
      oep->flags.b.lOk = 1;
      if (oep->loep->flags.b.pOk) {
        if (oep->foep == oep->loep) {
          if (cdh_ObjidIsNotEqual(oep->foep->o.oid, oep->loep->o.oid))
            printf("Object: %s [%d] has only one child %d == %d\n", oep->o.name, oep->o.oid.oix, oep->foep->o.oid.oix, oep->loep->o.oid.oix);
        } else {
          printf("Flags: %s [%d] l-object [%d] allready connected to p-object\n", oep->o.name, oep->o.oid.oix, oep->loep->o.oid.oix);
        }
      }
      if (oep->loep->poep != NULL && oep->loep->poep != oep) {
        printf("loep: %s [%d] l-object [%d] allready connected to p-object [%d]\n", oep->o.name, oep->o.oid.oix, oep->loep->o.oid.oix, oep->loep->poep->o.oid.oix);
      } else {
        oep->loep->poep = oep;
        oep->loep->flags.b.pOk = 1;
      }
    }
  } else {
    oep->flags.b.lOk = 1;
  }
}

static void
printrbody(pwr_tOid oid)
{
  sOentry     *oep;
  pwr_tStatus  sts;

  count_rbody++;

  oep = (sOentry *)tree_Find(&sts, oid_th, &oid);
  if (EVEN(sts)) {
    printf("Rbody: object not found %d.%d\n", oid.vid, oid.oix);
  } else {
    oep->flags.b.inRbody = 1;
  }
  
}

static void
printdbody(pwr_tOid oid)
{
  sOentry     *oep;
  pwr_tStatus  sts;

  count_dbody++;

  oep = (sOentry *)tree_Find(&sts, oid_th, &oid);
  if (EVEN(sts)) {
    printf("dbody: object not found %d.%d\n", oid.vid, oid.oix);
  } else {
    oep->flags.b.inDbody = 1;
  }

}
