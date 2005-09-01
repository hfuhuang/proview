/* 
 * Proview   $Id: rt_pool.h,v 1.2 2005-09-01 14:57:56 claes Exp $
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

#ifndef	rt_pool_h
#define	rt_pool_h

#if defined(OS_ELN)
# include stddef
#else
# include <stddef.h>
#endif
#include "pwr.h"
#include "rt_sect.h"

/* This definition of the pool_tRef allows us to have 256 pool segments
   and a maximum segment size of (1<<24) which is 16 Mb. Hence, the max
   size of any pool is 256*16 Mbyte = 4 Gbyte!  */

#define	pool_cSegBits	8			/* Bits in segidx field */
#define	pool_cOffsBits	24			/* Bits in offset field */

#define	pool_cOffsAlign	3			/* Entry alignment */
#define pool_cOffsGranul	0		/* Entry granularity */
#define	pool_cSegs	(1<<pool_cSegBits)	/* Max number of segments in one directory.  */

#define	pool_cMaxSize	(1<<(pool_cOffsBits + pool_cOffsGranul))
#define	pool_cMaxOffs	(pool_cMaxSize-1)
						/* max size of a segment */

#define	pool_cLists	4			/* Max number of lookaside lists.  */

/*  A NULL reference can be 0, since the base offset in a segment
    always contains the pool_sEntry for the first entry. This means
    that the lowest possible valid offset in any segment (as the
    user sees it) is 1!  */

typedef ptrdiff_t	pool_tRef;
#define	pool_cNRef	(pool_tRef)0

typedef struct {
  pool_tRef		self;		/* Pool reference to this virtual address */
  pool_tRef		flink;		/* Forward link */
  pool_tRef		blink;		/* Backward link */
} pool_sQlink;

#define pool_QisUnlinked(a) ((a)->self == (a)->flink && (a)->self == (a)->blink)

/* Internal interpretation of a pool_tRef */
typedef union {
  struct { pwr_Endian_4 (
    pwr_Bits( offs,	pool_cOffsBits),	/* offset in section, in quadwords */
    pwr_Bits( seg,	pool_cSegBits),,	/*  */
  ) } b;
  pool_tRef		m;
} pool_uRefBits;

typedef void *pool_tHead;

typedef	struct {
  char			filler[1<<pool_cOffsGranul];
} pool_sData;

typedef pwr_tInt32	pool_tOffset;		/* Segment offset, in pool_sData units */
#define pool_cNOffset	(pool_tOffset)-1	/* where -1=>NULL */


/* Global data structures */

typedef struct {
  pool_tOffset		next;		/* Next entry if free list, or mark */
  pwr_tUInt32		size;		/* Size of entry, in pool_sData units */
} pool_sEntry;

typedef enum {
  pool_eSegType__ = 0,
  pool_eSegType_dynamic,
  pool_eSegType_lookaside,
  pool_eSegType_named,
  pool_eSegType_
} pool_eSegType;

typedef struct {
  pool_sEntry		freeroot;	/* Root of free list */
  pwr_tUInt32		size;		/* Segment size in pool_sData! units */
  pwr_tUInt32		generation;	/* Segment generation #, 0=>not created */
  pwr_tUInt32		alloccnt;	/* # of allocated pieces of memory in seg */
  pwr_tUInt32		fragcnt;	/* # of free fragments in segment */
  pwr_tUInt32		fragmax;	/* Largest fragment in pool_sEntry units */
  pwr_tUInt32		fragmaxcnt;	/* # of fragmax fragments */
  pwr_tUInt32		fragsize;	/* Total size of fragments, pool_sEntry units */
  pwr_tUInt32		seg;		/* Segment index */
  pool_eSegType		type;
  char			name[16];	/* Name of named segment.  */
  pwr_tUInt32		la_size;	/* Lookaside size */
  pwr_tUInt32		la_idx;		/* Segment lookaside index.   */
  pwr_tUInt32		next_la_seg;	/* Segment index */
} pool_sGsegment;

typedef struct {
  pwr_tUInt32		size;		/* Size of entry in lookaside list.  */
  pwr_tUInt32		seg;		/* Segment index */
  pool_tOffset		next;		/* Next entry if free list, or mark */
} pool_sList;

typedef struct {
  pwr_tUInt32		created;
  pwr_tUInt32		generation;	/* Next generation number */
  pwr_tUInt32		initsize;	/* Segment 0 size in pool_sEntry units */
  pwr_tUInt32		extendsize;	/* Segment 1..n size in pool_sEntry units */
  char			name[16];	/* Name (prefix) */
  pwr_tUInt32		la_idx;		/* Next free lookaside index.  */
  pool_sList		la[pool_cLists]; /* Lookaside lists.  */
  pool_sGsegment	seg[pool_cSegs]; /* Entry for each segment */
} pool_sGhead;

/* Local data structures */

typedef struct {
  sect_sHead		sect;		/* Section header for this pool segment.  */
  pool_sEntry		*base;		/* Segmant base address.  */
  pool_sGsegment	*gpsp;		/* */
  pwr_tUInt32		generation;	/* Segment generation #, 0=>not mapped */
  pwr_tUInt32		seg;		/* Segment index */
} pool_sSegment;

typedef struct {
  sect_sHead		sect;		/* Section header for this pool.  */
  pwr_tUInt32		created;
  pool_sGhead		*gphp;		/* Address of the global pool header */
  pool_sSegment		seg[pool_cSegs]; /* Entry for each segment */
} pool_sHead;

#define pool_Qitem(a, b, c) ((b *)((char *)a - offsetof(b, c)))
#define pool_Qlink(a, b, c) ((pool_sQlink *)((char *)a + offsetof(b, c)))

void		*pool_Address (pwr_tStatus*, pool_sHead*, pool_tRef);
void		*pool_Alloc (pwr_tStatus*, pool_sHead*, pwr_tUInt32);
pwr_tBoolean	pool_AllocLookasideSegment (pwr_tStatus*, pool_sHead*, pwr_tUInt32, pwr_tUInt32);
void		*pool_AllocNamedSegment (pwr_tStatus*, pool_sHead*, pwr_tUInt32, char*);
pool_sHead	*pool_Create (pwr_tStatus*, pool_sHead*, char*, pwr_tUInt32, pwr_tUInt32);
void		pool_Dump (pwr_tStatus*, pool_sHead*);
pwr_tBoolean	pool_Free (pwr_tStatus*, pool_sHead*, void*);
pwr_tBoolean	pool_FreeReference (pwr_tStatus*, pool_sHead*, pool_tRef);
pool_tRef	pool_InPool (pwr_tStatus*, pool_sHead*, void*, pwr_tUInt32);
pool_tRef	pool_ItemReference (pwr_tStatus*, pool_sHead*, void*);
pool_sQlink	*pool_Qalloc (pwr_tStatus*, pool_sHead*);
pwr_tBoolean	pool_QhasOne (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pool_tRef	pool_Qinit (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pool_sQlink	*pool_QinsertPred (pwr_tStatus*, pool_sHead*, pool_sQlink*, pool_sQlink*);
pool_sQlink	*pool_QinsertSucc (pwr_tStatus*, pool_sHead*, pool_sQlink*, pool_sQlink*);
pwr_tBoolean	pool_QisEmpty (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pwr_tBoolean	pool_QisLinked (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pwr_tBoolean	pool_QisNull (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pool_sQlink	*pool_Qmove (pwr_tStatus*, pool_sHead*, pool_sQlink*, pool_sQlink*);
pool_sQlink	*pool_Qpred (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pool_sQlink	*pool_Qremove (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pool_sQlink	*pool_QremovePred (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pool_sQlink	*pool_QremoveSucc (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pool_sQlink	*pool_Qsucc (pwr_tStatus*, pool_sHead*, pool_sQlink*);
pool_tRef	pool_RefAlloc (pwr_tStatus*, pool_sHead*, pwr_tUInt32);
pool_tRef	pool_Reference (pwr_tStatus*, pool_sHead*, void*);
#endif
