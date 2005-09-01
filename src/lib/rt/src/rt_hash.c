/* 
 * Proview   $Id: rt_hash.c,v 1.2 2005-09-01 14:57:55 claes Exp $
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
 */

/* rt_hash.c 
   Hash table routines. Dynamic, using chaining.
   Requires a pool package where all structures are allocated.  */

#ifdef	OS_ELN
# include $vaxelnc
# include math
#else
# include <stdio.h>
# include <string.h>
# include <math.h>
#endif

#include "rt_hash_msg.h"
#include "pwr.h"
#include "co_cdh.h"
#include "rt_errh.h"
#include "rt_hash.h"

static pool_sQlink *
findEntry (
  pwr_tStatus		*sts,
  pool_sQlink		**bp,		/* Return pointer to bucket.  */
  hash_sTable		*htp,
  const void		*keyp
);

static int
nextPrime (
  int		n
);


/*  Return a slot pointer in the hash table.
    THIS ROUTINE IS INTERNAL TO THIS PACKAGE! The hash table has to be
    locked/unlocked by the caller.  */

static pool_sQlink *
findEntry (
  pwr_tStatus		*sts,
  pool_sQlink		**bp,		/* Return pointer to bucket.  */
  hash_sTable		*htp,
  const void		*keyp
)
{
  pwr_tUInt32		key;		/* key transformation value */
  pwr_tBoolean		found;
  pwr_tUInt32		depth;
  hash_sGtable		*ghtp;
  pool_sQlink		*lh;
  pool_sQlink		*il;
  char			*ip;

  ghtp = htp->ghtp;
  
  switch (ghtp->key_type) {
  case hash_eKey_oid:
    {
      pwr_tObjid *valp = (pwr_tObjid *) keyp;

      key = (valp->oix + ((valp->vid * 127) ^ (valp->vid * 131))) % ghtp->size;
    }
    break;
  case hash_eKey_int32:
    {
      pwr_tUInt32 val = *(pwr_tUInt32 *) keyp;
      key = ((val * 127) ^ (val * 131)) % ghtp->size;
    }
    break;
  case hash_eKey_family:
    {
      pwr_tUInt32 val = ((cdh_sFamily *) keyp)->name.pack.key;
      pwr_tObjid *valp = &(((cdh_sFamily *) keyp)->poid);
      key = (((val * 127) ^ (val * 131)) ^
	     ((valp->oix * 127) ^ (valp->oix * 131)) ^
	     ((valp->vid * 127) ^ (valp->vid * 131))) % ghtp->size;
    }
    break;
  case hash_eKey_objName:
    {
      pwr_tUInt32 val = ((cdh_sObjName *) keyp)->pack.key;
      key = ((val * 127) ^ (val * 131)) % ghtp->size;
    }
    break;
  case hash_eKey_memcmp:
    {
      const char *s = keyp;
      pwr_tUInt32 val = 0;
      int i = ghtp->key_size;

      for (s = keyp; i > 0; s++, i--)
	val += val*3 + *s;
      key = val % ghtp->size;
    }
    break;
  case hash_eKey_strncmp:
    {
      const char *s = keyp;
      pwr_tUInt32 val = 0;
      int i = ghtp->key_size;

      for (s = keyp; *s != '\0' && i > 0; s++, i--)
	val += val*3 + *s;
      key = val % ghtp->size;
    }
    break;
  case hash_eKey_uint16:
    {
      pwr_tUInt32 val = *(pwr_tUInt16 *) keyp;
      key = ((val * 127) ^ (val * 131)) % ghtp->size;
    }
    break;
  case hash_eKey_user:
    if (htp->xform_f != NULL)
      key = htp->xform_f(keyp, ghtp->size);
    else
      errh_Bugcheck(HASH__BUGCHECK, "");
    break;
  default:
    errh_Bugcheck(HASH__BUGCHECK, "");
  }

  ghtp->xforms++;

  /* Walk the chain starting in this pht entry */

  found = NO;
  lh = htp->tp + key; /* NOTE: Pointer arithmetic */
  depth = 0;
  for (il = pool_Qsucc(sts, htp->php, lh); il != lh; il =  pool_Qsucc(sts, htp->php, il)) {
    ip = (char *) il - ghtp->link_offset;
    depth++;
    ghtp->comps++;

    switch (ghtp->key_type) {
    case hash_eKey_oid:
      {
	pwr_tObjid *xkey = (pwr_tObjid *) (ip + ghtp->key_offset);
	pwr_tObjid *ykey = (pwr_tObjid *) keyp;
	found = (xkey->oix == ykey->oix && xkey->vid == ykey->vid);
      }
      break;
    case hash_eKey_int32:
      {
	pwr_tUInt32 *xkey = (pwr_tUInt32 *) (ip + ghtp->key_offset);
	pwr_tUInt32 *ykey = (pwr_tUInt32 *) keyp;
	found = *xkey == *ykey;
      }
      break;
    case hash_eKey_family:
      {
	cdh_sFamily *xkey = (cdh_sFamily *) (ip + ghtp->key_offset);
	cdh_sFamily *ykey = (cdh_sFamily *) keyp;
	found = (xkey->name.pack.key == ykey->name.pack.key &&
	  xkey->poid.oix == ykey->poid.oix && xkey->poid.vid == ykey->poid.vid &&
	  strcmp(xkey->name.norm, ykey->name.norm) == 0);
      }
      break;
    case hash_eKey_objName:
      {
	cdh_sObjName *xkey = (cdh_sObjName *) (ip + ghtp->key_offset);
	cdh_sObjName *ykey = (cdh_sObjName *) keyp;
	found = (xkey->pack.key == ykey->pack.key && strcmp(xkey->norm, ykey->norm) == 0);
      }
      break;
    case hash_eKey_memcmp:
      found = memcmp(ip + ghtp->key_offset, keyp, ghtp->key_size) == 0;
      break;
    case hash_eKey_strncmp:
      found = strncmp(ip + ghtp->key_offset, keyp, ghtp->key_size) == 0;
      break;
    case hash_eKey_uint16:
      {
	pwr_tUInt16 *xkey = (pwr_tUInt16 *) (ip + ghtp->key_offset);
	pwr_tUInt16 *ykey = (pwr_tUInt16 *) keyp;
	found = *xkey == *ykey;
      }
    case hash_eKey_user:
      if (htp->comp_f != NULL)
	found = htp->comp_f(keyp, ip);
      else
	errh_Bugcheck(HASH__BUGCHECK, "");
      break;
    default:
      errh_Bugcheck(HASH__BUGCHECK, "");
    }
    if (found) break;
  }

  if (depth > ghtp->max_depth) ghtp->max_depth = depth;

  if (bp != NULL) *bp = (void *)(lh);

  if (!found) {
    ghtp->no_finds++;
    pwr_Return(NULL, sts, HASH__NOTFOUND);
  }

  ghtp->finds++;
  pwr_Return(il, sts, HASH__SUCCESS);
}

static int
nextPrime (
  int		n
)
{
  int		i;
  int		p = n;
  int		sqrt_p;

  if ((n & 1) == 0) p++;
  while (1) {
    sqrt_p = sqrt(p);
    for(i=3; i <= sqrt_p; i+=2) if ((p % i) == 0) break;
    if (i > sqrt_p) break;
    p += 2;
  }

  return p;
}

/* Find a matching table entry to a given key.  */

void *
hash_Search (
  pwr_tStatus		*sts,
  hash_sTable		*htp,
  const void		*key
)
{
  pool_sQlink		*il;

  htp->ghtp->searchs++;

  il = findEntry(sts, NULL, htp, key);
  if (il == NULL) return NULL;

  pwr_Return((char *)il - htp->ghtp->link_offset, sts, HASH__SUCCESS);
}

/*  Insert an item with key into a hash table.  */

void *
hash_Insert (
  pwr_tStatus		*sts,
  hash_sTable		*htp,
  void			*ip		/* Address of item to be inserted.  */
)
{
  hash_sGtable		*ghtp;
  pool_sQlink		*il;
  pool_sQlink		*bl;		/* Address of bucket link.  */

  ghtp = htp->ghtp;
  ghtp->inserts++;

  il = findEntry(sts, &bl, htp, (char* )ip + ghtp->key_offset);

  if (il != NULL) {
    pwr_Return(NULL, sts, HASH__DUPLICATE);
  } else {
    if (bl->self == bl->flink) {	/* This is an empty bucket.  */
      ghtp->used_buckets++;
      if (ghtp->used_buckets > ghtp->max_used_buckets)
	ghtp->max_used_buckets = ghtp->used_buckets;
    }

    il = (pool_sQlink *)((char *)ip + ghtp->link_offset);
    il = pool_QinsertSucc(sts, htp->php, il, bl);
    if (il == NULL) return NULL;
    ghtp->entries++;
    if (ghtp->entries > ghtp->max_entries)
      ghtp->max_entries = ghtp->entries;
  }

  pwr_Return(ip, sts, HASH__SUCCESS);
}


/*  Remove an entry from a hash table.  */

void *
hash_Remove (
  pwr_tStatus		*sts,
  hash_sTable		*htp,
  void			*ip		/* Address of item to be removed.  */
)
{
  hash_sGtable		*ghtp;
  pool_sQlink		*il;
  pool_sQlink		*bl;


  ghtp = htp->ghtp;
  ghtp->removes++;

  il = findEntry(sts, &bl, htp, (char *)ip + ghtp->key_offset);
  if (il == NULL) return NULL;

  il = pool_Qremove(sts, htp->php, il);
  if (il == NULL)
    return NULL;

  ghtp->entries--;
  if (bl->flink == bl->self)
    ghtp->used_buckets--;

  pwr_Return(ip, sts, HASH__SUCCESS);
}

/* Create a hash table.
   Map the hash data structures, and initialize then
   if the structures are created.
   References are set to free (i.e. zeroed), the mutex is created
   and the statistics are cleared.

   The return value is a pointer to the root of the data structures for this
   table, or NONE if unsuccessful.  */

hash_sTable *
hash_Create (
  pwr_tStatus		*sts,
  pool_sHead		*php,
  hash_sTable		*htp,
  hash_sGtable		*ghtp,
  pwr_tBoolean		(*comp_f) (const void *, void *),	/* Key comparison routine */
  pwr_tUInt32		(*xform_f) (const void *, size_t)	/* Key transformation routine */
)
{
  pwr_tUInt32		i;
  pool_sQlink		*lh;

  memset(htp, 0, sizeof(*htp));

  htp->php = php;
  ghtp->inits++;

  if (!ghtp->flags.b.created) {

    ghtp->size = nextPrime(ghtp->size);

    ghtp->table = pool_RefAlloc(sts, php, sizeof(hash_sEntry) * ghtp->size);
    if (ghtp->table == pool_cNRef) return NULL;
    htp->tp = pool_Address(sts, php, ghtp->table);
    if (htp->tp == NULL) return NULL;
    for (i = 0, lh = htp->tp; i < ghtp->size; i++, lh++)
      pool_Qinit(sts, php, lh);

    ghtp->flags.b.created = 1;
  } else {
    htp->tp = pool_Address(sts, php, ghtp->table);
  }

  htp->ghtp = ghtp;
  htp->comp_f = comp_f;
  htp->xform_f = xform_f;

  pwr_Return(htp, sts, HASH__SUCCESS);
}

/* Initiate a hash table structure.  */

void
hash_Init (
  hash_sGtable		*p,
  size_t		size,
  size_t		key_size,
  size_t		record_size,
  ptrdiff_t		key_offset,
  ptrdiff_t		link_offset,
  hash_eKey		key_type
)
{
  
  p->size	  = size;
  p->key_size	  = key_size;
  p->record_size  = record_size;
  p->key_offset	  = key_offset;
  p->link_offset  = link_offset;
  p->key_type	  = key_type;
}

/*  Print information about a hash table.  */

void
hash_Print (
  pwr_tStatus		*sts,
  hash_sTable		*htp
)
{
  hash_sGtable		*ghtp;


  ghtp = htp->ghtp;

  printf("  size............: %d\n", ghtp->size);
  printf("  key_size........: %d\n", ghtp->key_size);
  printf("  record_size.....: %d\n", ghtp->record_size);
  printf("  key_offset......: %d\n", (int)ghtp->key_offset);
  printf("  link_offset.....: %d\n", (int)ghtp->link_offset);
  printf("  key_type........: %d\n", ghtp->key_type);
  printf("  xforms..........: %d\n", ghtp->xforms);
  printf("  comps...........: %d\n", ghtp->comps);
  printf("  max_depth.......: %d\n", ghtp->max_depth);
  printf("  mean_depth......: %f\n", (float)ghtp->comps/(float)ghtp->xforms);
  printf("  inserts.........: %d\n", ghtp->inserts);
  printf("  removes.........: %d\n", ghtp->removes);
  printf("  searchs.........: %d\n", ghtp->searchs);
  printf("  used_buckets....: %d\n", ghtp->used_buckets);
  printf("  max_used_buckets: %d\n", ghtp->max_used_buckets);
  printf("  entries.........: %d\n", ghtp->entries);
  printf("  max_entries.....: %d\n", ghtp->max_entries);
  printf("  finds...........: %d\n", ghtp->finds);
  printf("  no_finds........: %d\n", ghtp->no_finds);
  printf("  inits...........: %d\n", ghtp->inits);

  pwr_Status(sts, HASH__SUCCESS);
}
