/* rt_dvol.c -- Dynamic volumes

   PROVIEW/R
   Copyright (C) 1996-98 by Mandator AB.

   .  */

#if defined(OS_ELN)
# include $vaxelnc
# include stsdef
#else 
# include <stdio.h>
#endif

#include "pwr.h"
#include "rt_gdh_msg.h"
#include "rt_hash_msg.h"
#include "pwr_class.h"
#include "rt_errh.h"
#include "rt_gdb.h"
#include "rt_vol.h"
#include "rt_dvol.h"
#include "rt_cvols.h"
#include "rt_net.h"
#include "rt_sub.h"


/* Adopt an object.  */

static gdb_sObject *
adoptObject (
  pwr_tStatus		*sts,
  gdb_sObject		*op,
  gdb_sObject		*pop,
  cvol_sNotify		*nmp
)
{

  gdb_AssumeLocked;

  vol_InsertSiblist(sts, op, pop);

  /* Connect to parent.  */

  op->g.f.poid = pop->g.oid;

  pwr_Assert(!op->l.flags.b.inFamilyTab);
  hash_Insert(sts, gdbroot->family_ht, op);
  op->l.flags.b.inFamilyTab = 1;

  if (nmp != NULL) {
    switch (nmp->subtype) {
    case net_eMsg_createObject:
      {
	net_sCreateObject *cp = &nmp->msg.c;

	cp->sib.flink    = op->g.sib.flink;
	cp->sib.newblink = op->g.oid.oix;

	cp->sib.blink    = op->g.sib.blink;
	cp->sib.newflink = op->g.oid.oix;

	cp->par.oid  = pop->g.oid;
	cp->par.soid = pop->g.soid;

	break;
      }
    case net_eMsg_moveObject:
      {
	net_sMoveObject *mp = &nmp->msg.m;

	mp->sib = op->g.sib;

	mp->nsib.flink    = op->g.sib.flink;
	mp->nsib.newblink = op->g.oid.oix;

	mp->nsib.blink    = op->g.sib.blink;
	mp->nsib.newflink = op->g.oid.oix;

	mp->npar.oid  = pop->g.oid;
	mp->npar.soid = pop->g.soid;
	break;

      }
    default:
      errh_Bugcheck(2, "");
    }
  }

#if 0 /* !!! To do!!!*/
  malvl = MAX(op->g.alvl, op->g.malvl);
  mblvl = MAX(op->g.blvl, op->g.mblvl);
  if (malvl > pop->g.malvl || mblvl > op->g.mblvl) {
    op->o.ablvl_idx = ++gdbroot->db->ablvl_idx;
    vol_PropagateAlarmLevel(sts, op, 0, malvl, NO);
    vol_PropagateBlockLevel(sts, op, 0, mblvl, NO);
  }
#endif

  return op;
}

/* Unadopt an object.  */

static gdb_sObject *
unadoptObject (
  pwr_tStatus		*sts,
  gdb_sObject		*op,
  gdb_sObject		*pop,
  cvol_sNotify		*nmp
)
{

  gdb_AssumeLocked;

  if (nmp != NULL) {
    switch (nmp->subtype) {
    case net_eMsg_deleteObject:
      {
	net_sDeleteObject *dp = &nmp->msg.d;

	dp->sib.flink	 = op->g.sib.flink;
	dp->sib.newblink = op->g.sib.blink;
	dp->sib.blink    = op->g.sib.blink;
	dp->sib.newflink = op->g.sib.flink;

	break;
      }
    case net_eMsg_moveObject:
      {
	net_sMoveObject *mp = &nmp->msg.m;

	mp->osib.flink    = op->g.sib.flink;
	mp->osib.newblink = op->g.sib.blink;
	mp->osib.blink    = op->g.sib.blink;
	mp->osib.newflink = op->g.sib.flink;

	break;
      }
    default:
      errh_Bugcheck(2, "");
    }
  }

  vol_RemoveSiblist(sts, op, pop);

  if (nmp != NULL) {
    /* Copying the parent must be deferred until after the unadopt,
       since parent might be affected by that... */

    switch (nmp->subtype) {
    case net_eMsg_deleteObject:
      {
	net_sDeleteObject *dp = &nmp->msg.d;

	dp->par.oid = pop->g.oid;
	dp->par.soid = pop->g.soid;

	break;
      }
    case net_eMsg_moveObject:
      {
	net_sMoveObject *mp = &nmp->msg.m;

	mp->opar.oid  = pop->g.oid;
	mp->opar.soid = pop->g.soid;

	break;
      }
    default:
      errh_Bugcheck(2, "");
    }
  }

#if 0
  op->g.f.poid = pwr_cNObjid;
  op->l.por = pool_cNRef;
#endif

  pwr_Assert(op->l.flags.b.inFamilyTab);
  hash_Remove(sts, gdbroot->family_ht, op);
  op->l.flags.b.inFamilyTab = 0;

#if 0 /* !!! To do!! */
  malvl = MAX(op->g.alvl, op->g.malvl);
  mblvl = MAX(op->g.blvl, op->g.mblvl);
  if (malvl == pop->g.malvl || mblvl == pop->g.mblvl) {
    op->o.ablvl_idx = ++ghdi->db->ablvl_idx;
    vol_PropagateAlarmLevel(sts, op, malvl, 0, NO);
    vol_PropagateBlockLevel(sts, op, mblvl, 0, NO);
  }
#endif

  return op;
}

/* Create an initialize a new local object. Space
   is allocated for the object which must not
   exist. All reachable nodes are notified about this new object.  */

gdb_sObject *
dvol_CreateObject (
  pwr_tStatus		*sts,
  cdh_sParseName	*pn,
  pwr_tClassId		cid,
  pwr_tUInt32		size,
  pwr_tObjid		oid		/* Requested objid, */
)
{
  static cvol_sNotify	cm;		/* Cannot be on the stack for VAXELN */
  pwr_tObjid		poid;		/* Objid of parent (or pwr_cNObjid) */
  pwr_tInt32		size2;		/* Size of the new object */
  gdb_sClass		*cp;
  gdb_sObject		*tmp_op;
  gdb_sObject		*op;
  gdb_sObject		*pop;
  gdb_sVolume		*vp;
  
  gdb_AssumeLocked;

  if (pn->flags.b.idString) pwr_Return(NULL, sts, GDH__NOSUCHOBJ);

  /* Verify class.  */

  cp = hash_Search(sts, gdbroot->cid_ht, &cid);
  if (cp == NULL) return NULL;

  size2 = MAX(size, cp->size);

  /* Check parent.  */

  pop = vol_NameToParentObject(sts, pn, gdb_mLo_dynamic, vol_mTrans_all);
  if (pop == NULL) {
    if (pn->nObject > 1)
      pwr_Return(NULL, sts , GDH__BADPARENT);

    poid = pwr_cNObjid;

    if (cdh_ObjidIsNull(oid))
      poid.vid = gdbroot->db->vid;
    else
      poid.vid = oid.vid;

    pop = vol_OidToObject(sts, poid, gdb_mLo_dynamic, vol_mTrans_all, cvol_eHint_none);
    if (pop == NULL) return NULL;
  } else {
    poid = pop->g.oid;
    if (cdh_ObjidIsNotNull(oid) && oid.vid != pop->g.oid.vid)
      pwr_Return(NULL, sts, GDH__BADPARENT);
  }

  vp = pool_Address(sts, gdbroot->pool, pop->l.vr);
  if (vp == NULL)
    pwr_Return(NULL, sts, GDH__BADPARENT);

  if (!vp->l.flags.b.dynamic)
    pwr_Return(NULL, sts, GDH__STATICVOL);

  /* Make sure that the name is unique.  */

  pn->object[pn->nObject-1].poid = poid;
  if (hash_Search(sts, gdbroot->family_ht, &pn->object[pn->nObject-1]) != NULL)
    pwr_Return(NULL, sts, GDH__DUPLNAME);

  if (cdh_ObjidIsNull(oid))
    oid = vol_Oid(sts, vp, cid);

  if (hash_Search(sts, gdbroot->oid_ht, &oid) != NULL)
    return NULL;

  tmp_op = op = gdb_AddObject(sts, pn->object[pn->nObject-1].name.orig, oid, cid, size2, poid, net_mGo__, pwr_cNObjid);
  if (op == NULL) return NULL;

  do {
    /* We have alloced an object header.
       Free it if something happens.  */

    op = vol_LinkObject(sts, vp, op, vol_mLink_create);
    if (op == NULL) break;

    cvols_InitNotify(op, &cm, net_eMsg_createObject);

    op = adoptObject(sts, op, pop, &cm);
    if (op == NULL) break;

    cvols_Notify(&cm);

    return op;
  } while (0);

  vol_UnlinkObject(sts, vp, op, vol_mLink_delete);
  return NULL;  
}

/* Delete a local object.

   Object header and the associated body are removed.

   This call is possible only if the object is
   in a local volume, i.e a volume owned by the local node.
   The volume must be of class $DynamicVolume or $SystemVolume.
   The object must not be parent to any other object.

   All reachable nodes, who have mounted the volume in question,
   are notified about the removal of this object.  */

pwr_tBoolean
dvol_DeleteObject (
  pwr_tStatus		*sts,
  pwr_tObjid		oid
)
{
  static cvol_sNotify	dm;		/* Cannot be on the stack for VAXELN */

  gdb_sObject		*op;
  gdb_sObject		*p_op;
  gdb_sVolume		*vp;

  gdb_AssumeLocked;

  if (oid.oix == pwr_cNObjectIx)
    pwr_Return(FALSE, sts, GDH__VOLDELETE);

  op = vol_OidToObject(sts, oid, gdb_mLo_dynamic, vol_mTrans_none, cvol_eHint_none);
  if (op == NULL) return FALSE;

  if (op->g.flags.b.isParent && cdh_ObjidIsNotNull(op->g.soid))
    pwr_Return(FALSE, sts, GDH__CHILDREN);

  p_op	= pool_Address(NULL, gdbroot->pool, op->l.por);
  vp	= pool_Address(NULL, gdbroot->pool, op->l.vr);

  cvols_InitNotify(op, &dm, net_eMsg_deleteObject);

  unadoptObject(sts, op, p_op, &dm);

  vol_UnlinkObject(sts, vp, op, vol_mLink_delete);

  cvols_Notify(&dm);

  pwr_Return(TRUE, sts, GDH__SUCCESS);
}

/* Move a local object.

   This call is possible only if the object is
   in a local volume, i.e a volume owned by the local node.
   The volume must be of class $DynamicVolume or $SystemVolume.

   All reachable nodes, who have mounted the volume in question,
   are notified about the move of this object.  */

gdb_sObject *
dvol_MoveObject (
  pwr_tStatus		*sts,
  pwr_tObjid		oid,
  pwr_tObjid		poid
)
{
  static cvol_sNotify	mm;		/* Cannot be on the stack for VAXELN */

  gdb_sObject		*op;
  gdb_sObject		*new_pop;
  gdb_sObject		*old_pop;
  gdb_sObject		*tmp_op;
  
  if (oid.oix == pwr_cNObjectIx)	/* This volume object cannot be moved.  */
    pwr_Return(NULL, sts, GDH__NOTMOVABLE);

  if (cdh_ObjidIsNull(poid))
    poid.vid = oid.vid;

  if (oid.vid != poid.vid)
    pwr_Return(NULL, sts, GDH__BADPARENT);

  if (cdh_ObjidIsEqual(oid, poid))	/* An object can not be parent to itself.  */
    pwr_Return(NULL, sts, GDH__CHILDSELF);

  gdb_AssumeLocked;

  op = hash_Search(sts, gdbroot->oid_ht, &oid);
  if (op == NULL) pwr_Return(NULL, sts, GDH__NOSUCHOBJ);

  new_pop = hash_Search(sts, gdbroot->oid_ht, &poid);
  if (new_pop == NULL) pwr_Return(NULL, sts, GDH__NOSUCHOBJ);

  old_pop = pool_Address(sts, gdbroot->pool, op->l.por);
  if (old_pop == NULL) pwr_Return(NULL, sts, GDH__WEIRD);

  if (new_pop == old_pop)	/* The same parent, consider it done!  */
    pwr_Return(op, sts, GDH__SUCCESS);


  for (		/* Detect potential hierarchy loops.  */
    tmp_op = old_pop;
    tmp_op != NULL && tmp_op->g.oid.oix != pwr_cNObjectIx;
    tmp_op = pool_Address(sts, gdbroot->pool, tmp_op->l.por)
  ) {
    if (tmp_op == op)		/* Loop detected! */
      pwr_Return(NULL, sts, GDH__CHILDSELF);
  }

  if (tmp_op == NULL)
    pwr_Return(NULL, sts, GDH__WEIRD);

  /* Check potential name conflicts.  */

  tmp_op = vol_FamilyToObject(sts, op->g.f.name.orig, poid);
  if (tmp_op != NULL)
    pwr_Return(NULL, sts, GDH__DUPLNAME);

  cvols_InitNotify(op, &mm, net_eMsg_deleteObject);

  unadoptObject(NULL, op, old_pop, &mm);  

  op->g.f.poid = poid;

  adoptObject(NULL, op, new_pop, &mm);

  cvols_Notify(&mm);

  pwr_Return(op, sts, GDH__SUCCESS);
}

/* Rename a local object.

   This call is possible only if the object is
   in a local volume, i.e a volume owned by the local node.
   The volume must be of class $DynamicVolume or $SystemVolume.

   All reachable nodes, who have mounted the volume in question,
   are notified about the renaming of this object.  */

gdb_sObject *
dvol_RenameObject (
  pwr_tStatus		*sts,
  pwr_tObjid		oid,
  cdh_sParseName	*pn
)
{
  static cvol_sNotify	rm;		/* Cannot be on the stack for VAXELN */

  gdb_sObject		*op;
  gdb_sObject		*tmp_op;

  gdb_AssumeLocked;

  op = hash_Search(sts, gdbroot->oid_ht, &oid);
  if (op == NULL) return NULL;

  if (!op->l.flags.b.dynamic)
    pwr_Return(NULL, sts, GDH__STATICVOL);

  tmp_op = vol_FamilyToObject(sts, pn->object[0].name.orig, op->g.f.poid);
  if (tmp_op != NULL && tmp_op != op)
    pwr_Return(NULL, sts, GDH__DUPLNAME);

  hash_Remove(sts, gdbroot->family_ht, op);

  op->g.f.name = pn->object[0].name;

  hash_Insert(sts, gdbroot->family_ht, op);

  cvols_InitNotify(op, &rm, net_eMsg_renameObject);

  rm.msg.r.f.name = op->g.f.name;

  cvols_Notify(&rm);

  pwr_Return(op, sts, GDH__SUCCESS);
}
