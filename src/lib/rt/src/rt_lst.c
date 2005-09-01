/* 
 * Proview   $Id: rt_lst.c,v 1.2 2005-09-01 14:57:55 claes Exp $
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

#include "pwr.h"
#include "rt_errh.h"
#include "rt_sect.h"
#include "rt_lst.h"
#include "rt_sync.h"


static pwr_tBoolean
check	(
  lst_sEntry		*link
)
{

  if (link == NULL || link->blink == NULL || link->flink == NULL)
    return NO;

  return YES;
}

static pwr_tBoolean
checkInit (
  lst_sEntry		*link
)
{

  if ((link->flink == NULL || link->blink == NULL) && link->flink != link->blink)
    return NO;

  if (link->flink == NULL)
    link->flink = link->blink = link;

  return YES;
}

/* Check if a queue has one and only one member.  */

pwr_tBoolean
lst_HasOne (
  thread_sMutex		*mp,
  lst_sEntry		*link
)
{
  pwr_tBoolean		has_one;

  if (mp != NULL) sync_MutexLock(mp);

  has_one = link != NULL && link != link->flink && link->flink == link->blink;

  if (mp != NULL) sync_MutexUnlock(mp);

  return has_one;
}

/* Initialize a queue link.  */

lst_sEntry *
lst_Init (
  thread_sMutex		*mp,
  lst_sEntry		*link,
  void			*item
)
{

  if (mp != NULL) sync_MutexLock(mp);

  if (link != NULL) {
    link->flink = link->blink = link;
    link->item = item;
  }
  if (mp != NULL) sync_MutexUnlock(mp);

  return link;
}

/* Insert 'link' as predecessor to 'succ'.  */

lst_sEntry *
lst_InsertPred (
  thread_sMutex		*mp,
  lst_sEntry		*succ,		/* Insert before this element */
  lst_sEntry		*link,		/* Link to insert */
  void			*item		/* Item to insert */
)
{
  lst_sEntry		*pred;

  if (mp != NULL) sync_MutexLock(mp);

  pwr_Assert(checkInit(link));

  pred = succ->blink;
  
  pwr_Assert(check(succ));
  pwr_Assert(check(pred));
  pwr_Assert(pred->flink == succ);

  link->flink = succ;
  link->blink = succ->blink;
  succ->blink = pred->flink = link;
  if (item != NULL)
    link->item = item;

  if (mp != NULL) sync_MutexUnlock(mp);

  return pred;  
}

/* Insert 'link' as successor to 'pred'.  */

lst_sEntry *
lst_InsertSucc (
  thread_sMutex		*mp,
  lst_sEntry		*pred,		/* Insert after this element */
  lst_sEntry		*link,		/* link to insert */
  void			*item		/* Item to insert */
)
{
  lst_sEntry		*succ;

  if (mp != NULL) sync_MutexLock(mp);

  pwr_Assert(checkInit(link));

  succ = pred->flink;
  
  pwr_Assert(check(succ));
  pwr_Assert(check(pred));
  pwr_Assert(succ->blink == pred);

  link->blink = pred;
  link->flink = pred->flink;
  pred->flink = succ->blink = link;
  if (item != NULL)
    link->item = item;

  if (mp != NULL) sync_MutexUnlock(mp);

  return succ;  
}

/* Check if a queue header is empty.  */

pwr_tBoolean
lst_IsEmpty (
  thread_sMutex		*mp,
  lst_sEntry		*link
)
{
  pwr_tBoolean		is_empty;

  if (mp != NULL) sync_MutexLock(mp);

  is_empty = link != NULL && link->flink == link && link->blink == link;

  if (mp != NULL) sync_MutexUnlock(mp);

  return is_empty;
}

/* Check if a d-link is linked.  */

pwr_tBoolean
lst_IsLinked (
  thread_sMutex		*mp,
  lst_sEntry		*link
)
{
  pwr_tBoolean		is_linked;
  lst_sEntry		*pred;
  lst_sEntry		*succ;

  if (mp != NULL) sync_MutexLock(mp);

  pwr_Assert(checkInit(link));

  pred = link->blink;
  succ = link->flink;
  
  pwr_Assert(check(succ));
  pwr_Assert(check(pred));
  pwr_Assert(pred->flink == link);
  pwr_Assert(succ->blink == link);

  is_linked = link->flink != link;

  if (mp != NULL) sync_MutexUnlock(mp);

  return is_linked;
}

/* Check if a queue link is initiated.  */

pwr_tBoolean
lst_IsNull (
  thread_sMutex		*mp,
  lst_sEntry		*link
)
{
  pwr_tBoolean		is_null;

  if (mp != NULL) sync_MutexLock(mp);

  pwr_Assert((link->blink == NULL && link->flink == NULL) ||
	     (link->blink != NULL && link->flink != NULL));

  is_null = (link->blink == NULL) && (link->flink == NULL);

  if (mp != NULL) sync_MutexUnlock(mp);

  return is_null;
}

/* Return successor to 'link'. */

pwr_tBoolean
lst_IsSucc (
  thread_sMutex		*mp,
  lst_sEntry		*pred,
  lst_sEntry		*link
)
{
  lst_sEntry		*succ;
  pwr_tBoolean		is_succ;

  if (mp != NULL) sync_MutexLock(mp);

  pwr_Assert(check(pred));
  pwr_Assert(check(link));


  succ = link->flink;
  
  if (succ != NULL) {
    pwr_Assert(check(succ));
    pwr_Assert(succ->blink == link);
    pwr_Assert(link->flink == succ);
  }

  is_succ = pred->flink == link;

  if (mp != NULL) sync_MutexUnlock(mp);

  return is_succ;  
}

/* Move the elements in 'old' list to 'new' list.
   Return ?  */

lst_sEntry *
lst_Move (
  thread_sMutex		*mp,
  lst_sEntry		*old,		/* Old queue header */
  lst_sEntry		*new		/* New queue header */
)
{
  lst_sEntry		*pred;
  lst_sEntry		*succ;

  pwr_Assert(checkInit(new));

  pred = old->blink;
  succ = old->flink;
  
  pwr_Assert(check(succ));
  pwr_Assert(check(pred));
  pwr_Assert(pred->flink == old);
  pwr_Assert(succ->blink == old);

  if (old->flink != old) {
    new->flink = succ;
    new->blink = pred;
    succ->blink = pred->flink = new;
    old->flink = old->blink = old;
  }

  return new;  
}

/* Return predecessor to 'link'.  */

void *
lst_Pred (
  thread_sMutex		*mp,
  lst_sEntry		*link,
  lst_sEntry		**lp
)
{
  lst_sEntry		*pred;
  void			*item = NULL;

  if (mp != NULL) sync_MutexLock(mp);

  pwr_Assert(check(link));

  pred = link->blink;
  
  if (pred != NULL) {
    pwr_Assert(check(pred));
    pwr_Assert(pred->flink == link);
  }

  if (lp != NULL)
    *lp = pred;

  item = pred->item;  

  if (mp != NULL) sync_MutexUnlock(mp);

  return item;
}

/* Remove a link from a queue.  */

void *
lst_Remove (
  thread_sMutex		*mp,
  lst_sEntry		*link
)
{
  lst_sEntry		*succ;
  lst_sEntry		*pred;
  void			*item = NULL;

  if (mp != NULL) sync_MutexLock(mp);

  do {	/* Locking scope */

    pwr_Assert(check(link));

    if (link->flink == link || link->blink == link) {
      pwr_Assert(link->flink == link->blink);
      break;
    }

    succ = link->flink;
    pred = link->blink;

    pwr_Assert(succ->blink == link);
    pwr_Assert(pred->flink == link);

    succ->blink = link->blink;
    pred->flink = link->flink;
    link->flink = link;
    link->blink = link;

    item = link->item;

  } while (0);

  if (mp != NULL) sync_MutexUnlock(mp);

  return item;

}

/* Remove predecessor of an link, from a queue.
   If link is queue header this call will
   remove the first link in the queue.

   Return address of removed predecessor. */

void *
lst_RemovePred (
  thread_sMutex		*mp,
  lst_sEntry		*link,
  lst_sEntry		**lp
)
{
  lst_sEntry		*pred;
  lst_sEntry		*pred_pred;
  void			*item = NULL;

  if (mp != NULL) sync_MutexLock(mp);

  do {	/* Locking scope */

    pwr_Assert(check(link));

    if (link->flink == link || link->blink == link) {
      pwr_Assert(link->flink == link->blink);
      if (lp != NULL)
	*lp = NULL;
	break;
    }

    pred = link->blink;

    pwr_Assert(check(pred));
    pwr_Assert(pred->flink == link);

    pred_pred = pred->blink;

    pwr_Assert(check(pred_pred));
    pwr_Assert(pred_pred->flink == pred);

    pred_pred->flink = link;
    link->blink = pred_pred;
    pred->flink = pred->blink = pred;

    if (lp != NULL)
      *lp = pred;

    item = pred->item;

  } while (0);

  if (mp != NULL) sync_MutexUnlock(mp);

  return item;
}

/* Remove successor of an link, from a list.
   If link is queue header this call will
   remove the first link in the queue.

   Return address of removed successor. */

void *
lst_RemoveSucc (
  thread_sMutex		*mp,
  lst_sEntry		*link,
  lst_sEntry		**lp
)
{
  lst_sEntry		*succ;
  lst_sEntry		*succ_succ;
  void			*item = NULL;

  if (mp != NULL) sync_MutexLock(mp);

  do {	/* Locking scope */

    pwr_Assert(check(link));

    if (link->flink == link || link->blink == link) {
      pwr_Assert(link->flink == link->blink);
      if (lp != NULL)
	*lp = NULL;
      break;
    }

    succ = link->flink;

    pwr_Assert(check(succ));
    pwr_Assert(succ->blink == link);

    succ_succ = succ->flink;

    pwr_Assert(check(succ_succ));
    pwr_Assert(succ_succ->blink == succ);

    succ_succ->blink = link;
    link->flink = succ_succ;
    succ->flink = succ->blink = succ;

    if (lp != NULL)
      *lp = succ;
    item = succ->item;

  } while (0);

  if (mp != NULL) sync_MutexUnlock(mp);

  return item;
}

/* Return successor to 'link'. */

void *
lst_Succ (
  thread_sMutex		*mp,
  lst_sEntry		*link,
  lst_sEntry		**lp
)
{
  lst_sEntry		*succ;
  void			*item = NULL;

  if (mp != NULL) sync_MutexLock(mp);

  pwr_Assert(check(link));

  succ = link->flink;
  
  if (succ != NULL) {
    pwr_Assert(check(succ));
    pwr_Assert(succ->blink == link);
  }

  if (lp != NULL)
    *lp = succ;

  item = succ->item;  

  if (mp != NULL) sync_MutexUnlock(mp);

  return item;
}
