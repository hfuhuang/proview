/* 
 * Proview   $Id: co_tree.c,v 1.2 2005-09-01 14:58:00 claes Exp $
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

/* co_tree.c -- Test program  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pwr.h"
#include "co_tree.h"

typedef struct {
  tree_sNode	node;
  int		key;
  int		count;
} sNode;

main ()
{
  tree_sTable	*tp;
  sNode		*np;
  int		key;
  int		i;
  char		s[256];
  char		*cp;
  char		c;

  tp = tree_CreateTable(sizeof(int), offsetof(sNode, key), sizeof(sNode), 100, tree_eComp_int32, NULL);

  for (i = 0; i < 1000; i += 10) {
    tree_Insert(tp, &i);
  }

  for ( ;;) {
    printf("Command: ");
    cp = gets(s);
    c = s[0];
    if (cp == NULL || c == '\0') {
      printf("\nGoodbye\n");
      return;
    }
    switch (c) {
    case 'i':
    case 'I':
      printf("Insert, Key: ");
      gets(s);
      key = atoi(s);
      if (tree_Find(tp, &key) == NULL) {
	tree_Insert(tp, &key);
      } else
	printf("\nKey allready exists!\n");
      break;
    case 'd':
    case 'D':
      printf("Delete, Key: ");
      gets(s);
      key = atoi(s);
      if ((np = tree_Find(tp, &key)) != NULL) {
	tree_Remove(tp, &key);
      } else
	printf("\nKey does not exist!\n");
      break;
    case 'f':
    case 'F':
      printf("Find, Key: ");
      gets(s);
      key = atoi(s);
      if ((np = tree_Find(tp, &key)) == NULL) {
	printf("\nKey does not exist!\n");
      } else
	printf("\nKey exists! %d\n", np->key);
      break;
    case 's':
    case 'S':
      printf("Find successor, Key: ");
      gets(s);
      key = atoi(s);
      if ((np = tree_FindSuccessor(tp, &key)) == NULL) {
	printf("\nKey does not exist!\n");
      } else
	printf("\nKey exists! %d\n", np->key);
      break;
    case 'p':
    case 'P':
      printf("Find predecessor, Key: ");
      gets(s);
      key = atoi(s);
      if ((np = tree_FindPredecessor(tp, &key)) == NULL) {
	printf("\nKey does not exist!\n");
      } else
	printf("\nKey exists! %d\n", np->key);
      break;
#if 0
    case 't':
    case 'T':
      printf("Start: ");
      gets(s);
      start = atoi(s);
      printf("Stop: ");
      gets(s);
      stop = atoi(s);
      printf("Order: ");
      gets(s);
      c = s[0];
      switch (c) {
      case 's':
      case 'S':
	printf("\navl-tree\n");
	i = start;
	j = stop;
	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	cpu = cputim;
	page = pageflts;

	for (; i <= j; i++) {
	  if (TreeSearch(tp,i) == tp->Null) {
	    np = TreeAlloc(tp, i);
	    TreeInsert(tp, np);
	  }
	}

	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	printf("Cputim: %d, Pageflts: %d\n", cputim - cpu, pageflts - page);

	i = start;
	j = stop;
	printf("\nlib$tree\n");
	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	cpu = cputim;
	page = pageflts;

	for (; i <= j; i++) {
	  sts = lib$lookup_tree(&ltp, i, Compare, &lnp);
	  if (!(sts & 1)) {
	    lib$insert_tree(&ltp, i, &0, Compare, Alloc, &lnp, 0);
	  }
	}

	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	printf("Cputim: %d, Pageflts: %d\n", cputim - cpu, pageflts - page);
	break;
      case 'd':
      case 'D':
	i = start;
	j = stop;
	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	cpu = cputim;
	page = pageflts;

	for (; i <= j; i++) {
	  if ((np = TreeSearch(tp,i)) != tp->Null) {
	    TreeDelete(tp, np);
	  } else {
	    printf("Could not find %d\n", i);
	  }
	}

	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	printf("Cputim: %d, Pageflts: %d\n", cputim - cpu, pageflts - page);
	break;
      case 'f':
      case 'F':
	printf("\navl-tree\n");
	i = start;
	j = stop;
	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	cpu = cputim;
	page = pageflts;

	for (; i <= j; i++) {
	  if ((np = TreeSearch(tp,i)) != tp->Null) {
	  } else {
	    printf("Could not find %d\n", i);
	  }
	}

	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	printf("Cputim: %d, Pageflts: %d\n", cputim - cpu, pageflts - page);

	i = start;
	j = stop;
	printf("\nlib$tree\n");
	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	cpu = cputim;
	page = pageflts;

	for (; i <= j; i++) {
	  sts = lib$lookup_tree(&ltp, i, Compare, &lnp);
	}

	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	printf("Cputim: %d, Pageflts: %d\n", cputim - cpu, pageflts - page);
	break;
      case 'b':
      case 'B':
	i = start;
	j = stop;
	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	cpu = cputim;
	page = pageflts;

	for (; i <= j; j--) {
	  if (TreeSearch(tp,j) == tp->Null) {
	    np = TreeAlloc(tp, j);
	    TreeInsert(tp, np);
	  }
	}

	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	printf("Cputim: %d, Pageflts: %d\n", cputim - cpu, pageflts - page);
	break;
      case 'r':
      case 'R':
	i = start;
	j = stop;
	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	cpu = cputim;
	page = pageflts;

	for (; i <= j; i++) {
	  k = 65535 & rand();
	  if (TreeSearch(tp,k) == tp->Null) {
	    np = TreeAlloc(tp, k);
	    TreeInsert(tp, np);
	  }
	}

	sts = sys$getjpiw(0, &pid, 0,item_list, 0, 0, 0);
	printf("Cputim: %d, Pageflts: %d\n", cputim - cpu, pageflts - page);
	break;
      default:
	printf("Illegal order!\n");
	break;
      }
      break;
    case 'p':
    case 'P':
      tp->ErrorCount = 0;
      tp->HZCount = 0;
      tp->HNCount = 0;
      tp->HPCount = 0;
      maxlevel = 0;
      count = 0;
      hight = 0;
      TreePrint(tp, tp->Root, NULL, NULL,0);
      TreeCheck (tp, tp->Root, &count, &maxlevel, &hight, 0);
      printf("Hight: %d\n", hight);
  #if 0
      TreePrintInorder(tp, tp->Root, 0);
  #endif
      sp = TreeMinimum(tp, tp->Root);
      ep = TreeMaximum(tp, tp->Root);
      op = sp;
      for (np = TreeSuccessor(tp, op); op != ep; np = TreeSuccessor(tp, np)) {
  #if 0
	printf("Key: %d\n", op->Key);
  #endif
	if (op->Key >= np->Key)
	  tp->ErrorCount++;
	op = np;
      }
      printf("Hight.......: %d\n", hight);
      printf("Insert......: %d\n", tp->Insert);
      printf("Search......: %d\n", tp->Search);
      printf("Delete......: %d\n", tp->Delete);
      printf("NodeCount...: %d\n", tp->NodeCount);
      printf("FreeCount...: %d\n", tp->FreeCount);
      printf("MaxDepth....: %d\n", tp->MaxDepth);
      printf("HZCount.....: %d\n", tp->HZCount);
      printf("HNCount.....: %d\n", tp->HNCount);
      printf("HPCount.....: %d\n", tp->HPCount);
      printf("LLCount.....: %d\n", tp->LLCount);
      printf("LRCount.....: %d\n", tp->LRCount);
      printf("RLCount.....: %d\n", tp->RLCount);
      printf("RRCount.....: %d\n", tp->RRCount);
      printf("AllocCount..: %d\n", tp->AllocCount);
      printf("MallocCount.: %d\n", tp->MallocCount);
      printf("ErrorCount..: %d\n", tp->ErrorCount);
      count = maxlevel = 0;
      Print(ltp, &count, &maxlevel, 0);
      printf("\nlib$tree\n");
      printf("Count.......: %d\n", count);
      printf("MaxDepth....: %d\n", maxlevel);
      break;
    case 'l':
    case 'L':
      TreePrintInorder(tp, tp->Root, 0);
      break;
#endif
    case 'q':
    case 'Q':
      printf("\nGoodbye\n");
      return;
      break;
    default:
      printf("Illegal command!\n");

      break;
    }
  }
}

