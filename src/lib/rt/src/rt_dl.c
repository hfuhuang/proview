/* rt_dl.c -- direct linkage

   PROVIEW/R
   Copyright (C) 1997-98 by Mandator AB.

   This module contains routines to handle direct linkage.
   Those routines can be called from the GDH API.  */

#if defined(OS_ELN)
#  include $vaxelnc
#else
#  include <stdio.h>
#  include <string.h>
#endif

#include "pwr.h"
#include "rt_gdh_msg.h"
#include "rt_hash_msg.h"
#include "rt_errh.h"
#include "rt_gdb.h"
#include "rt_hash.h"
#include "rt_pool.h"
#include "rt_sub.h"
#include "rt_dl.h"
#include "rt_mvol.h"


void
dl_Cancel (
  pwr_tStatus		*sts,
  pwr_tDlid		dlid
)
{
  dl_sLink		*dp;
  gdb_sObject		*op;
  cdh_uRefId		rid;

  gdb_AssumeLocked;
  
  rid.pwr = dlid;
  if (rid.r.vid_3 != cdh_eVid3_dlid) {
    pwr_Status(sts, GDH__DLID);
    return;
  }

  dp = hash_Search(sts, gdbroot->subc_ht, &dlid);
  if (dp == NULL) {
    pwr_Status(sts, GDH__DLID);
    return;
  }

  dp = hash_Remove(sts, gdbroot->subc_ht, dp);
  if (dp == NULL) errh_Bugcheck(GDH__WEIRD, "hash_Remove");

  pool_Qremove(NULL, gdbroot->pool, &dp->dl_ll);
  gdbroot->db->dl_lc--;

  op = pool_Address(NULL, gdbroot->pool, dp->or);
  if (op == NULL) errh_Bugcheck(GDH__WEIRD, "direct link inconsitency");

  gdb_UnlockObject(sts, op);

  pool_Free(NULL, gdbroot->pool, dp);

  return;
}

/* Cancel all direct links for this user.
   Database must be locked by caller.  */

void
dl_CancelUser (
  pid_t			user
)
{
  pool_sQlink		*dl;
  dl_sLink		*dp;

  gdb_AssumeLocked;

  for (
    dl = pool_Qsucc(NULL, gdbroot->pool, &gdbroot->db->dl_lh);
    dl != &gdbroot->db->dl_lh;
  ) {
    dp = pool_Qitem(dl, dl_sLink, dl_ll);
    dl = pool_Qsucc(NULL, gdbroot->pool, dl);

    if (dp->user == user)
      dl_Cancel(NULL, dp->dlid);
  }
}

/* Create a direct link.
   Database must be locked by caller.  */

dl_sLink *
dl_Create (
  pwr_tStatus		*sts,
  mvol_sAttribute	*ap,
  pwr_sAttrRef		*arp
)
{
  pwr_tStatus		lsts;
  dl_sLink		*dp;
  dl_sLink		*rdp;

  gdb_AssumeLocked;

  pwr_Assert(ap != NULL);
  pwr_Assert(ap->op != NULL);
  pwr_Assert(arp != NULL);

  dp = pool_Alloc(NULL, gdbroot->pool, sizeof(dl_sLink));

  do {
    gdbroot->db->dlid.rix++;
    dp->dlid = gdbroot->db->dlid;

    rdp = hash_Insert(&lsts, gdbroot->subc_ht, dp);
    if (rdp != dp && lsts != HASH__DUPLICATE)
      errh_Bugcheck(lsts, "hash_Insert");
  } while (rdp != dp);

  dp->user = gdbroot->my_pid;
  dp->aref = *arp;
  dp->or   = pool_ItemReference(NULL, gdbroot->pool, ap->op);


  /* Lock the object */

  gdb_LockObject(sts, ap->op);

  /* Insert in list of direct links */

  pool_QinsertSucc(NULL, gdbroot->pool, &dp->dl_ll, &gdbroot->db->dl_lh);
  gdbroot->db->dl_lc++;

  return dp;
}
