/* rt_ptree.c -- <short description>

   PROVIEW/R
   Copyright (C) 1996, 2002 by Cell Network AB.

   <Description>.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pwr.h"
#include "rt_ptree.h"


static pool_tRef allocNode( ptree_sTable *tp,
			    void *key);

static void ptreePrintInorder(ptree_sTable *tp, 
			      pool_tRef nr, 
			      void (*printNode)(ptree_sNode *, pool_tRef));

static void deleteTree(pwr_tStatus *sts, ptree_sTable *tp);

static void emptyTree(pwr_tStatus *sts, ptree_sTable *tp);

static void freeNode( ptree_sTable *tp, 
		      pool_tRef nr);

static pool_tRef findNearNode( ptree_sTable *tp, 
			       void *key);

static pool_tRef findNode( ptree_sTable *tp, 
			   void *key);

static pool_tRef minimumNode( ptree_sTable *tp, 
			      pool_tRef nr);

static pool_tRef maximumNode( ptree_sTable *tp, 
			      pool_tRef nr);

static pool_tRef successorNode( ptree_sTable *tp, 
				 pool_tRef nr);

static pool_tRef predecessorNode( ptree_sTable *tp,
				  pool_tRef nr);

static pool_tRef deleteNode( ptree_sTable *tp, 
			     pool_tRef zr);

static pool_tRef insertNode( ptree_sTable *tp, 
			     pool_tRef zr);

static void ptreePrint( ptree_sTable *tp,
			pool_tRef    nr,
			pool_tRef    or,
			pool_tRef    oor,
			int          level,
			void         (*printNode)(ptree_sNode *, int));

static void ptreeCheck( ptree_sTable *tp, 
		        pool_tRef nr, int *count, int *maxlevel,
			int *hight, int level, 
			char *(*printKey)(ptree_sNode *));



static pool_tRef allocNode( ptree_sTable *tp,
			    void *key)
{
  pool_tRef    nr;
  ptree_sNode  *np;
  pool_tRef 	 fr;
  ptree_sNode	 *fp;
  pool_tRef	or;
  ptree_sNode  *op;
  pool_tRef	 ar;
  ptree_sAlloc *ap;
  int          i;
  pwr_tStatus sts;
  ptree_sGtable *gttp = tp->g; 

  nr = gttp->free;

  if (nr == pool_cNRef) {
    gttp->nMalloc++;
    ar = pool_RefAlloc( &sts, tp->php, gttp->allocCount * gttp->recordSize +
			sizeof(ptree_sAlloc));
    ap = (ptree_sAlloc *) pool_Address( &sts, tp->php, ar);
    ap->next = gttp->firstAlloc;
    gttp->firstAlloc = ar;
    nr = fr = (pool_tRef)( ((ptree_sAlloc *) ar) + 1);
    np = fp = (ptree_sNode *)( ap + 1);
    if (fr == pool_cNRef)
      return fr;
    fr = gttp->recordSize + fr;
    fp = (ptree_sNode *) (gttp->recordSize + (char *) fp);
    gttp->free = fr;
    gttp->nFree += gttp->allocCount;
    for (i = 1, op = fp, or = fr; i < gttp->allocCount - 1; i++) {
      fp = (ptree_sNode *) (gttp->recordSize + (char *) fp);
      fr = gttp->recordSize + fr;
      op->right = fr;
      op = fp;
    }
  } 
  else {
    np = pool_Address( &sts, tp->php, gttp->free);
    gttp->free = np->right;
  }
  gttp->nAlloc++;
  gttp->nFree--;
  np->left = np->right = np->parent = gttp->null;
  if (key != NULL)
    memcpy(gttp->keyOffset + (char *) np,  key, gttp->keySize);
  return nr;
}


static void ptreePrintInorder( ptree_sTable *tp, 
			       pool_tRef nr, 
			       void (*printNode)(ptree_sNode *, pool_tRef))
{
  pwr_tStatus sts;

  if ( nr == tp->g->null)
    return;

  ptree_sNode *np = pool_Address( &sts, tp->php, nr);
  if (np->right != tp->g->null)
    ptreePrintInorder(tp, np->right, printNode);

  printNode(np, nr);

  if (np->left != tp->g->null)
    ptreePrintInorder(tp, np->left, printNode);
}  

ptree_sTable *ptree_Create( pwr_tStatus  *sts,
			    pool_sHead   *php,
			    ptree_sTable *ttp,
			    ptree_sGtable *gttp,
			    int (*compareFunc) (ptree_sTable *tp, ptree_sNode *x, ptree_sNode *y))
{
  ttp->php = php;
  ttp->g = gttp;
  ttp->nullp = pool_Address( sts, php, gttp->null);
  ttp->keyp = pool_Address( sts, php, gttp->key);
  ttp->compareFunc = compareFunc;

  *sts = TREE__SUCCESS;
  return ttp;
}

void ptree_Init( pool_sHead   *php,
		 ptree_sGtable *gttp,
		 size_t       keySize,
		 ptrdiff_t    keyOffset,
		 size_t       recordSize,
		 unsigned int allocCount)
{
  pwr_tStatus sts;
  ptree_sNode *nullp;
  ptree_sTable t;

  gttp->keySize     = keySize;
  gttp->keyOffset   = keyOffset;
  gttp->recordSize  = recordSize;
  gttp->allocCount  = allocCount;

  t.g = gttp;
  t.php = php;
  gttp->null        = allocNode( &t, NULL);
  gttp->key         = allocNode( &t, NULL);
  gttp->root        = gttp->null;
      
  nullp = pool_Address( &sts, php, gttp->null);
  nullp->bal        = 0;
  nullp->left       = gttp->null;
  nullp->right      = gttp->null;
  nullp->parent     = gttp->null;
}

static void deleteTree( pwr_tStatus *sts, ptree_sTable *tp)
{
  ptree_sAlloc *ap;
  pool_tRef ar;
  pool_tRef far;
  
  if (tp == NULL) return;

  for ( ar = tp->g->firstAlloc; ar != pool_cNRef;) {
    far = ar;
    ap = pool_Address( sts, tp->php, ar);
    ar = ap->next;
    pool_FreeReference( sts, tp->php, far);
  }
}

static void emptyTree( pwr_tStatus *sts, ptree_sTable *tp)
{
  pool_tRef nr;
  pool_tRef mr;
  
  if ( tp == NULL) return;

  for ( mr = minimumNode( tp, tp->g->root); mr != pool_cNRef;) {
    if ( mr == tp->g->null) return;
    nr = successorNode( tp, mr);
    mr = deleteNode( tp, mr);
    freeNode( tp, mr);
    mr = nr;
  }
}

static void freeNode( ptree_sTable *tp, 
		      pool_tRef nr)
{
  pwr_tStatus sts;
  ptree_sNode *np = pool_Address( &sts, tp->php, nr);

  memset( np, 0 , tp->g->recordSize);
  np->right = tp->g->free;
  tp->g->free = nr;
  tp->g->nFree++;
  return;
}  


static pool_tRef findNearNode( ptree_sTable *tp, 
			       void *key)
{
  ptree_sNode *np;
  pool_tRef   nr;
  int         comp;
  pwr_tStatus sts;

  tp->g->nSearch++;

  /* use key node for comparisons */
  memcpy(tp->g->keyOffset + (char *) tp->keyp, key, tp->g->keySize);

  for (nr = tp->last = tp->g->root, np = pool_Address(&sts, tp->php, nr); 
       nr != tp->g->null; 
       tp->lastComp = comp, np = pool_Address(&sts, tp->php, nr)) {
    tp->last = nr;
    comp = tp->compareFunc(tp, tp->keyp, np);
    if (comp == 0)
      break;
    else if (comp < 0)
      nr = np->left;
    else
      nr = np->right;
  }
  
  return nr;
}



static pool_tRef findNode( ptree_sTable *tp, 
			   void *key)
{
  ptree_sNode *np;
  pool_tRef   nr;
  int         comp;
  pwr_tStatus sts;

  tp->g->nSearch++;

  /* use sentinel */
  memcpy( tp->g->keyOffset + (char *) tp->nullp,  key, tp->g->keySize);

  /* use key node for comparisons */
  memcpy(tp->g->keyOffset + (char *) tp->keyp,  key, tp->g->keySize);

  for (nr = tp->g->root, np = (ptree_sNode *)pool_Address( &sts, tp->php, nr);;
       np = pool_Address(&sts, tp->php, nr)) {
    comp = tp->compareFunc(tp, tp->keyp, np);
    if (comp == 0)
      break;
    else if (comp < 0)
      nr = np->left;
    else
      nr = np->right;
  }
  
  return nr;
}


static pool_tRef minimumNode( ptree_sTable *tp, 
			      pool_tRef nr)
{
  pwr_tStatus sts;
  ptree_sNode *np = pool_Address(&sts, tp->php, nr);

  for (; np->left != tp->g->null; 
       nr = np->left, np = pool_Address(&sts, tp->php, nr))
    ;
  return nr;
}


static pool_tRef maximumNode( ptree_sTable *tp, 
			      pool_tRef nr)
{
  pwr_tStatus sts;
  ptree_sNode *np = pool_Address(&sts, tp->php, nr);

  for (; np->right != tp->g->null; 
       nr = np->right, np = pool_Address(&sts, tp->php, nr))
    ;
  return nr;
}



static pool_tRef successorNode( ptree_sTable *tp, 
				 pool_tRef nr)
{
  pwr_tStatus sts;
  ptree_sNode *p;
  pool_tRef   pr;
  ptree_sNode *np = pool_Address(&sts, tp->php, nr);
  
  if ( np->right != tp->g->null)
    return minimumNode( tp, np->right);

  for (pr = np->parent, p = pool_Address(&sts, tp->php, pr); 
       pr != tp->g->null && nr == p->right; 
       np = p, nr = pr, pr = p->parent, p = pool_Address(&sts, tp->php, pr))
    ;
  return pr;
}


static pool_tRef predecessorNode( ptree_sTable *tp,
				  pool_tRef nr)
{
  pwr_tStatus sts;
  ptree_sNode *p;
  pool_tRef   pr;
  ptree_sNode *np = pool_Address(&sts, tp->php, nr);
  
  if ( np->left != tp->g->null)
    return maximumNode( tp, np->left);

  for ( pr = np->parent, p = pool_Address(&sts, tp->php, pr); 
	pr != tp->g->null && nr == p->left; 
	np = p, nr = pr, pr = p->parent, p = pool_Address(&sts, tp->php, pr))
    ;
  return pr;
}



static pool_tRef deleteNode( ptree_sTable *tp, 
			     pool_tRef zr)
{
  ptree_sNode *p;
  pool_tRef   pr;
  ptree_sNode *x;
  pool_tRef   xr;
  ptree_sNode *y;
  pool_tRef   yr;
  ptree_sNode *p1;
  pool_tRef   p1r;
  ptree_sNode *p2;
  pool_tRef   p2r;
  int        h;
  int        b1;
  int        b2;
  ptree_sNode *left, *right, *parent;
  pwr_tStatus sts;
  ptree_sNode *z = pool_Address(&sts, tp->php, zr);
  
  tp->g->nDelete++;
  tp->g->nNode--;

  if (z->left == tp->g->null || z->right == tp->g->null) {
    y = z;
    yr = zr;
  }
  else {
    yr = successorNode(tp, zr);
    y = pool_Address(&sts, tp->php, yr);
  }

  if (y->left != tp->g->null)
    xr = y->left;
  else
    xr = y->right;

  x = pool_Address(&sts, tp->php, xr);
  x->parent = y->parent;
  parent = pool_Address(&sts, tp->php, y->parent);
  if (y->parent == tp->g->null) {
    tp->g->root = xr;
    h = 0;
  } else if (yr == parent->left) {
    parent->left = xr;
    h = 1; /* left branch has shrunk */
  } else {
    parent->right = xr;
    h = -1;  /* right branch has shrunk */
  }
  
  if (y->parent == zr) {
    p = y;
    pr = yr;
  }
  else {
    pr = y->parent;
    p = parent;
  }
  
  if (z != y) { /* Replace z with y */
    y->bal = z->bal;
    y->parent = z->parent;
    parent = pool_Address(&sts, tp->php, z->parent);
    if (z->parent == tp->g->null) {
      tp->g->root = yr;
    } else if (zr == parent->left) {
      parent->left = yr;
    } else {
      parent->right = yr;
    }
    y->left = z->left;
    if (z->left != tp->g->null) {
      left = pool_Address(&sts, tp->php, z->left);
      left->parent = yr;
    }
    y->right = z->right;
    if (z->right != tp->g->null) {
      right = pool_Address(&sts, tp->php, z->right);
      right->parent = yr;
    }
  }

  for (; pr != tp->g->null && h != 0; 
       pr = p->parent, p = pool_Address(&sts, tp->php, pr)) {
    if (h == -1) { /* right branch has shrunk */
      switch (p->bal) {
      case 1:
	p->bal = 0;
	break;
      case 0:
	p->bal = -1;
	h = 0;
	break;
      case -1:
	p1r = p->left;
	p1 = pool_Address(&sts, tp->php, p1r);
	b1 = p1->bal;
	if (b1 < 1) { /* single LL rotation */
	  tp->g->nLL++;
	  p->left = p1->right;
	  right = pool_Address(&sts, tp->php, p1->right);
	  right->parent = pr;
	  p1->right = pr;
	  if (b1 == 0) {
	    p->bal = -1;
	    p1->bal = 1;
	    h = 0;
	  } else {
	    p->bal = p1->bal = 0;
	  }
	  p1->parent = p->parent;
	  parent = pool_Address(&sts, tp->php, p->parent);
	  if (p->parent == tp->g->null)
	    tp->g->root = p1r;
	  else if (pr == parent->left)
	    parent->left = p1r;
	  else
	    parent->right = p1r;
	  p->parent = p1r;
	  p = p1;
	  pr = p1r;
	} else { /* double LR rotation */
	  tp->g->nLR++;
	  p2r = p1->right;
	  p2 = pool_Address(&sts, tp->php, p2r);
	  b2 = p2->bal;
	  p1->right = p2->left;
	  left = pool_Address(&sts, tp->php, p2->left);
	  left->parent = p1r;
	  p2->left = p1r;
	  p->left = p2->right;
	  right = pool_Address(&sts, tp->php, p2->right);
	  right->parent = pr;
	  p2->right = pr;
	  p2->parent = p->parent;
	  parent = pool_Address(&sts, tp->php, p->parent);
	  if (p->parent == tp->g->null)
	    tp->g->root = p2r;
	  else if (pr == parent->left)
	    parent->left = p2r;
	  else
	    parent->right = p2r;
	  p->parent = p1->parent = p2r;
	  if (b2 == -1)
	    p->bal = 1;
	  else
	    p->bal = 0;
	  if (b2 == 1)
	    p1->bal = -1;
	  else
	    p1->bal = 0;
	  p2->bal = 0;
	  p = p2;
	  pr = p2r;
	}
	break;
      }
    } else { /* left branch has grown */
      switch (p->bal) {
      case -1:
	p->bal = 0;
	break;
      case 0:
	p->bal = 1;
	h = 0;
	break;
      case 1:
	p1r = p->right;
	p1 = pool_Address(&sts, tp->php, p1r);
	b1 = p1->bal;
	if (b1 > -1) { /* single RR rotation */
	  tp->g->nRR++;
	  p->right = p1->left;
	  left = pool_Address(&sts, tp->php, p1->left);
	  left->parent = pr;
	  p1->left = pr;
	  if (b1 == 0) {
	    p->bal = 1;
	    p1->bal = -1;
	    h = 0;
	  } else {
	    p->bal = p1->bal = 0;
	  }
	  p1->parent = p->parent;
	  parent = pool_Address(&sts, tp->php, p->parent);
	  if (p->parent == tp->g->null)
	    tp->g->root = p1r;
	  else if (pr == parent->left)
	    parent->left = p1r;
	  else
	    parent->right = p1r;
	  p->parent = p1r;
	  p = p1;
	  pr = p1r;
	} else { /* double RL rotation */
	  tp->g->nRL++;
	  p2r = p1->left;
	  p2 = pool_Address(&sts, tp->php, p2r);
	  b2 = p2->bal;
	  p1->left = p2->right;
	  right = pool_Address(&sts, tp->php, p2->right);
	  right->parent = p1r;
	  p2->right = p1r;
	  p->right = p2->left;
	  left = pool_Address(&sts, tp->php, p2->left);
	  left->parent = pr;
	  p2->left = pr;
	  p2->parent = p->parent;
	  parent = pool_Address(&sts, tp->php, p->parent);
	  if (p->parent == tp->g->null)
	    tp->g->root = p2r;
	  else if (pr == parent->left)
	    parent->left = p2r;
	  else
	    parent->right = p2r;
	  p->parent = p1->parent = p2r;
	  if (b2 == 1)
	    p->bal = -1;
	  else
	    p->bal = 0;
	  if (b2 == -1)
	    p1->bal = 1;
	  else
	    p1->bal = 0;
	  p2->bal = 0;
	  p = p2;
	  pr = p2r;
	}
	break;
      }
    }
    parent =  pool_Address(&sts, tp->php, p->parent);
    if (h!= 0 && p->parent != tp->g->null && pr == parent->left)
      h = 1; /* left branch has shrunk */
    else if (h!= 0 && p->parent != tp->g->null && pr == parent->right)
      h = -1;
  }    
  
  return zr;
}



static pool_tRef insertNode( ptree_sTable *tp, 
			     pool_tRef zr)
{
  pwr_tStatus sts;
  ptree_sNode *p;
  pool_tRef   pr;
  ptree_sNode *p1;
  pool_tRef   p1r;
  ptree_sNode *p2;
  pool_tRef   p2r;
  ptree_sNode *y;
  pool_tRef   yr;
  ptree_sNode *x;
  pool_tRef  xr;
  int        comp;
  int        h;
  ptree_sNode *left, *right, *parent;
  ptree_sNode *z = pool_Address( &sts, tp->php, zr);
  
  for ( yr = tp->g->null, y = tp->nullp, 
	  xr = tp->g->root, x = pool_Address( &sts, tp->php, xr); 
	xr != tp->g->null;
	x = pool_Address( &sts, tp->php, xr)) {
    y = x;
    yr = xr;
    comp = tp->compareFunc(tp, z, x);
    if (comp == 0)
      return xr;  /* already exists */
    if (comp < 0) {
      xr = x->left;
    } else {
      xr = x->right;
    }
  }
  z->parent = yr;
  if (yr == tp->g->null) {
    tp->g->root = zr;
  } else if (comp < 0) {
    y->left = zr;
  } else {
    y->right = zr;
  }

  z->bal = 0;

  tp->g->nNode++;
  tp->g->nInsert++;


  for (h = 1, y = z, yr = zr, pr = z->parent, p = pool_Address(&sts, tp->php, pr); 
       pr != tp->g->null && h != 0;
       y = p, yr = pr, pr = p->parent, p = pool_Address(&sts, tp->php, pr)) {
    if (yr == p->left) { /* left branch has grown */
      switch (p->bal) {
      case 1:
	p->bal = 0;
	h = 0;
	break;
      case 0:
	p->bal = -1;
	break;
      case -1:
	p1r = p->left;
	p1 = pool_Address(&sts, tp->php, p1r);
	if (p1->bal == -1) { /* single LL rotation */
	  tp->g->nLL++;
	  p->left = p1->right;
	  right = pool_Address(&sts, tp->php, p1->right);
	  right->parent = pr;
	  p1->right = pr;
	  p1->parent = p->parent;
	  parent = pool_Address(&sts, tp->php, p->parent);
	  if (p->parent == tp->g->null)
	    tp->g->root = p1r;
	  else if (pr == parent->left)
	    parent->left = p1r;
	  else
	    parent->right = p1r;
	  p->parent = p1r;
	  p->bal = p1->bal = 0;
	  p = p1;
	  pr = p1r;
	  h = 0;
	} else { /* double LR rotation */
	  tp->g->nLR++;
	  p2r = p1->right;
	  p2 = pool_Address(&sts, tp->php, p2r);
	  p1->right = p2->left;
	  left = pool_Address(&sts, tp->php, p2->left);
	  left->parent = p1r;
	  p2->left = p1r;
	  p->left = p2->right;
	  right = pool_Address(&sts, tp->php, p2->right);
	  right->parent = pr;
	  p2->right = pr;
	  p2->parent = p->parent;
	  parent = pool_Address(&sts, tp->php, p->parent);
	  if (p->parent == tp->g->null)
	    tp->g->root = p2r;
	  else if (pr == parent->left)
	    parent->left = p2r;
	  else
	    parent->right = p2r;
	  p->parent = p1->parent = p2r;
	  if (p2->bal == -1)
	    p->bal = 1;
	  else
	    p->bal = 0;
	  if (p2->bal == 1)
	    p1->bal = -1;
	  else
	    p1->bal = 0;
	  p2->bal = 0;
	  p = p2;
	  pr = p2r;
	  h = 0;
	}
	break;
      }
    } else { /* right branch has grown */
      switch (p->bal) {
      case -1:
	p->bal = 0;
	h = 0;
	break;
      case 0:
	p->bal = 1;
	break;
      case 1:
	p1r = p->right;
	p1 = pool_Address(&sts, tp->php, p1r);
	if (p1->bal == 1) { /* single RR rotation */
	  tp->g->nRR++;
	  p->right = p1->left;
	  left = pool_Address(&sts, tp->php, p1->left);
	  left->parent = pr;
	  p1->left = pr;
	  p1->parent = p->parent;
	  parent = pool_Address(&sts, tp->php, p->parent);
	  if (p->parent == tp->g->null)
	    tp->g->root = p1r;
	  else if (pr == parent->left)
	    parent->left = p1r;
	  else
	    parent->right = p1r;
	  p->parent = p1r;
	  p->bal = p1->bal = 0;
	  p = p1;
	  pr = p1r;
	  h = 0;
	} else { /* double RL rotation */
	  tp->g->nRL++;
	  p2r = p1->left;
	  p2 = pool_Address(&sts, tp->php, p2r);
	  p1->left = p2->right;
	  right = pool_Address(&sts, tp->php, p2->right);
	  right->parent = p1r;
	  p2->right = p1r;
	  p->right = p2->left;
	  left = pool_Address(&sts, tp->php, p2->left);
	  left->parent = pr;
	  p2->left = pr;
	  p2->parent = p->parent;
	  parent = pool_Address(&sts, tp->php, p->parent);
	  if (p->parent == tp->g->null)
	    tp->g->root = p2r;
	  else if (pr == parent->left)
	    parent->left = p2r;
	  else
	    parent->right = p2r;
	  p->parent = p1->parent = p2r;
	  if (p2->bal == 1)
	    p->bal = -1;
	  else
	    p->bal = 0;
	  if (p2->bal == -1)
	    p1->bal = 1;
	  else
	    p1->bal = 0;
	  p2->bal = 0;
	  p = p2;
	  pr = p2r;
	  h = 0;
	}
	break;
      }
    }
  }    

  return zr;
}


static void ptreePrint( ptree_sTable *tp,
			pool_tRef    nr,
			pool_tRef    or,
			pool_tRef    oor,
			int          level,
			void         (*printNode)(ptree_sNode *, int))
{
  pwr_tStatus sts;
  ptree_sNode *np;

  if ( nr == tp->g->null)
    return;

  if (level > tp->g->maxDepth)
    tp->g->maxDepth = level;

  np = pool_Address( &sts, tp->php, nr);
  ptreePrint( tp, np->left, nr, np->parent, level+1, printNode);  

  printNode(np, level);

  switch (np->bal) {
  case 1:
    tp->g->nHP++;
    break;
  case 0:
    tp->g->nHZ++;
    break;
  case -1:
    tp->g->nHN++;
    break;
  }

  ptreePrint(tp, np->right, nr, np->parent, level+1, printNode);
} 


static void ptreeCheck( ptree_sTable *tp, 
		        pool_tRef nr, int *count, int *maxlevel,
			int *hight, int level, 
			char *(*printKey)(ptree_sNode *))
{
  int comp;
  int hleft;
  int hright;
  ptree_sNode *np, *left, *right;
  pwr_tStatus sts;

  if (nr == tp->g->null) {
    *hight = 0;
    return;
  }

  if (level > *maxlevel)
    *maxlevel = level;

  np = pool_Address(&sts, tp->php, nr);
  ptreeCheck(tp, np->left, count, maxlevel, &hleft, level+1, printKey);  
  if (np->left != tp->g->null) {
    left = pool_Address(&sts, tp->php, np->left);
    if (left->parent != nr) {
      printf("leftLinkerror: Node key: %s not linked to parent key: %s\n",
	     printKey(left), printKey(np));
    }
    comp = tp->compareFunc(tp, np, left);
    if (comp < 1) {
      printf("leftLink sort error: Node key: %s not less than key: %s\n",
	     printKey(left), printKey(np));
    }
  }

  (*count)++;
  ptreeCheck(tp, np->right, count, maxlevel, &hright, level+1, printKey); 
  if (np->right != tp->g->null) {
    right = pool_Address(&sts, tp->php, np->right);
    if (right->parent != nr) {
      printf("rightLinkerror: Node key: %s not linked to parent key: %s\n",
	     printKey(right), printKey(np));
    }
    comp = tp->compareFunc(tp, np, right);
    if (comp > -1) {
      printf("rightLink sort error: Node key: %s not greater than key: %s\n",
	     printKey(right), printKey(np));
    }
  }

  if ((hright - hleft) != np->bal) {
    printf("balerror key: %s, level: %d, hr: %d, hl: %d, bal: %d\n",
	   printKey(np), level, hright, hleft, np->bal);
  }
  *hight = (hright > hleft ? hright : hleft) + 1;
}

void ptree_DeleteTable(pwr_tStatus *sts, ptree_sTable *tp)
{
  deleteTree( sts, tp);
}


void ptree_EmptyTable(pwr_tStatus *sts, ptree_sTable *tp)
{
  emptyTree( sts, tp);
}

pool_tRef  ptree_Insert( pwr_tStatus *sts, 
			 ptree_sTable *tp, 
			 void *key)
{
  pool_tRef nr;
  pool_tRef or;

  nr = findNode(tp, key);
  if (nr != tp->g->null)
    pwr_Return( nr, sts, TREE__FOUND);

  nr = allocNode( tp, key);

  if (nr == pool_cNRef) 
    pwr_Return( pool_cNRef, sts, TREE__ERROR);
  or = insertNode( tp, nr);

  if (nr == or) {
    pwr_Return( nr, sts, TREE__INSERTED);
  } else {
    freeNode( tp, nr);
    pwr_Return( pool_cNRef, sts, TREE__ERROR);
  }
}

void *ptree_Find( pwr_tStatus *sts, ptree_sTable *tp, void *key)
{
  ptree_sNode *np;
  pool_tRef nr;

  nr = findNode(tp, key);
  if (nr == tp->g->null)
    pwr_Return(NULL, sts, TREE__NOTFOUND);

  np = pool_Address( sts, tp->php, nr);
  pwr_Return((void *)np, sts, TREE__FOUND);
}


void *ptree_FindPredecessor( pwr_tStatus *sts, ptree_sTable *tp, void *key)
{
    ptree_sNode *np;
    pool_tRef nr;

    nr = findNearNode(tp, key);
    if (tp->last == tp->g->null)
      pwr_Return(NULL, sts, TREE__NOTFOUND);
  
    if (nr != tp->g->null)
      nr = predecessorNode(tp, nr);
    else if (tp->lastComp < 0)
      nr = predecessorNode(tp, tp->last);
    else
      nr = tp->last;

    if (nr == tp->g->null)
      pwr_Return(NULL, sts, TREE__NOTFOUND);

    np = pool_Address(sts, tp->php, nr);
    pwr_Return((void *)np, sts, TREE__FOUND);
}

void *ptree_FindSuccessor(pwr_tStatus *sts, ptree_sTable *tp, void *key)
{
  ptree_sNode *np;
  pool_tRef nr;

  nr = findNearNode(tp, key);
  if ( tp->last == tp->g->null)
    pwr_Return(NULL, sts, TREE__NOTFOUND);
  
  if ( nr != tp->g->null)
    nr = successorNode(tp, nr);
  else if ( tp->lastComp < 0)
    nr = tp->last;
  else
    nr = successorNode(tp, tp->last);

  if ( nr == tp->g->null)
    pwr_Return(NULL, sts, TREE__NOTFOUND);

  np = pool_Address(sts, tp->php, nr);
  pwr_Return((void *)np, sts, TREE__FOUND);
}

int ptree_TableIsEmpty(pwr_tStatus *sts, ptree_sTable *tp)
{
  return tp->g->nNode == 0;
}

void *ptree_Maximum(pwr_tStatus *sts, ptree_sTable *tp)
{
  ptree_sNode *np;
  pool_tRef nr;

  nr = maximumNode(tp, tp->g->root);
  if (nr == tp->g->null)
    pwr_Return(NULL, sts, TREE__NOTFOUND);

  np = pool_Address(sts, tp->php, nr);
  pwr_Return((void *)np, sts, TREE__FOUND);
}

void *ptree_Minimum(pwr_tStatus *sts, ptree_sTable*tp)
{
  ptree_sNode *np;
  pool_tRef nr;

  nr = minimumNode(tp, tp->g->root);
  if ( nr == tp->g->null)
    pwr_Return(NULL, sts, TREE__NOTFOUND);

  np = pool_Address(sts, tp->php, nr);
  pwr_Return((void *)np, sts, TREE__FOUND);
}

void *
ptree_Predecessor(pwr_tStatus *sts, ptree_sTable *tp, void *np)
{
  pool_tRef nr = pool_Reference( sts, tp->php, np);

  nr = predecessorNode(tp, nr);

  if (nr == tp->g->null)
    pwr_Return(NULL, sts, TREE__NOTFOUND);

  np = pool_Address( sts, tp->php, nr);
  pwr_Return(np, sts, TREE__FOUND);
}

void
ptree_PrintTable(pwr_tStatus *sts,
                ptree_sTable *tp,
                void (*printNode)(ptree_sNode *, pool_tRef),
                char *(*printKey)(ptree_sNode *))
{
    int count = 0;
    int maxlevel = 0;
    int hight = 0;

#if 1
    ptreePrintInorder(tp, tp->g->root, printNode);
#endif

    ptreeCheck(tp, tp->g->root, &count, &maxlevel, &hight, 0, printKey);

    printf("count: %d, maxlevel: %d, hight: %d\n", count, maxlevel, hight);
    printf("nNode.......: %d\n", tp->g->nNode);
    printf("nError......: %d\n", tp->g->nError);
    printf("nHZ.........: %d\n", tp->g->nHZ);
    printf("nHN.........: %d\n", tp->g->nHN);
    printf("nHP.........: %d\n", tp->g->nHP);
    printf("maxDepth....: %d\n", tp->g->maxDepth);
    printf("nLL.........: %d\n", tp->g->nLL);
    printf("nLR.........: %d\n", tp->g->nLR);
    printf("nRL.........: %d\n", tp->g->nRL);
    printf("nRR.........: %d\n", tp->g->nRR);
    printf("nMalloc.....: %d\n", tp->g->nMalloc);
    printf("nAlloc......: %d\n", tp->g->nAlloc);
    printf("nFree.......: %d\n", tp->g->nFree);
    printf("nInsert.....: %d\n", tp->g->nInsert);
    printf("nDelete.....: %d\n", tp->g->nDelete);
    printf("nSearch.....: %d\n", tp->g->nSearch);
}


void ptree_Remove(pwr_tStatus *sts, ptree_sTable *tp, void *key)
{
  pool_tRef nr;

  nr = findNode( tp, key);
  if (nr != tp->g->null) {
    nr = deleteNode(tp, nr);
    freeNode( tp, nr);
  }
  return;
}

void *ptree_Successor(pwr_tStatus *sts, ptree_sTable *tp, void *np)
{
  pool_tRef nr = pool_Reference( sts, tp->php, np);

  nr = successorNode( tp, nr);
  if (nr == tp->g->null)
    return NULL;

  np = pool_Address( sts, tp->php, nr);
  return np;
}

int
ptree_Comp_cid( ptree_sTable *tp, ptree_sNode  *x, ptree_sNode  *y)
{
    pwr_tCid *xKey = (pwr_tCid *) (tp->g->keyOffset + (char *) x);
    pwr_tCid *yKey = (pwr_tCid *) (tp->g->keyOffset + (char *) y);

    if (*xKey < *yKey)
        return -1;
    else if (*xKey == *yKey)
        return 0;
    else
        return 1;
}

int
ptree_Comp_nid( ptree_sTable *tp, ptree_sNode  *x, ptree_sNode  *y)
{
    pwr_tNid *xKey = (pwr_tNid *) (tp->g->keyOffset + (char *) x);
    pwr_tNid *yKey = (pwr_tNid *) (tp->g->keyOffset + (char *) y);

    if (*xKey < *yKey)
        return -1;
    else if (*xKey == *yKey)
        return 0;
    else
        return 1;
}

int
ptree_Comp_vid( ptree_sTable *tp, ptree_sNode  *x, ptree_sNode  *y)
{
    pwr_tVid *xKey = (pwr_tVid *) (tp->g->keyOffset + (char *) x);
    pwr_tVid *yKey = (pwr_tVid *) (tp->g->keyOffset + (char *) y);

    if (*xKey < *yKey)
        return -1;
    else if (*xKey == *yKey)
        return 0;
    else
        return 1;
}

int
ptree_Comp_tid( ptree_sTable *tp, ptree_sNode  *x, ptree_sNode  *y)
{
    pwr_tTid *xKey = (pwr_tTid *) (tp->g->keyOffset + (char *) x);
    pwr_tTid *yKey = (pwr_tTid *) (tp->g->keyOffset + (char *) y);

    if (*xKey < *yKey)
        return -1;
    else if (*xKey == *yKey)
        return 0;
    else
        return 1;
}

int
ptree_Comp_oix( ptree_sTable *tp, ptree_sNode  *x, ptree_sNode  *y)
{
    pwr_tOix *xKey = (pwr_tOix *) (tp->g->keyOffset + (char *) x);
    pwr_tOix *yKey = (pwr_tOix *) (tp->g->keyOffset + (char *) y);

    if (*xKey < *yKey)
        return -1;
    else if (*xKey == *yKey)
        return 0;
    else
        return 1;
}

int
ptree_Comp_uint32( ptree_sTable *tp, ptree_sNode  *x, ptree_sNode  *y)
{
    pwr_tUInt32 *xKey = (pwr_tUInt32 *) (tp->g->keyOffset + (char *) x);
    pwr_tUInt32 *yKey = (pwr_tUInt32 *) (tp->g->keyOffset + (char *) y);

    if (*xKey < *yKey)
        return -1;
    else if (*xKey == *yKey)
        return 0;
    else
        return 1;
}

int
ptree_Comp_oid( ptree_sTable *tp, ptree_sNode *x, ptree_sNode *y)
{
    pwr_tOid *xKey = (pwr_tOid *) (tp->g->keyOffset + (char *) x);
    pwr_tOid *yKey = (pwr_tOid *) (tp->g->keyOffset + (char *) y);

    if (xKey->vid == yKey->vid) {
        if (xKey->oix == yKey->oix)
            return 0;
        else if (xKey->oix < yKey->oix)
            return -1;
        else
            return 1;
    } else if (xKey->vid < yKey->vid)
        return -1;
    else
        return 1;
}


int
ptree_Comp_time( ptree_sTable *tp, ptree_sNode *x, ptree_sNode *y)
{
    pwr_tTime *xKey = (pwr_tTime *) (tp->g->keyOffset + (char *) x);
    pwr_tTime *yKey = (pwr_tTime *) (tp->g->keyOffset + (char *) y);

    if (xKey->tv_sec == yKey->tv_sec) {
        if (xKey->tv_nsec == yKey->tv_nsec)
            return 0;
        else if ((int)xKey->tv_nsec < (int)yKey->tv_nsec)
            return -1;
        else
            return 1;
    } else if ((int)xKey->tv_sec < (int)yKey->tv_sec)
        return -1;
    else
        return 1;
}

int
ptree_Comp_memcmp( ptree_sTable *tp, ptree_sNode *x, ptree_sNode *y)
{
    void *xKey = (void *) (tp->g->keyOffset + (char *) x);
    void *yKey = (void *) (tp->g->keyOffset + (char *) y);

    return memcmp(xKey, yKey, tp->g->keySize);
}


int
ptree_Comp_strncmp( ptree_sTable *tp, ptree_sNode *x, ptree_sNode *y)
{
    char *xKey = (char *) (tp->g->keyOffset + (char *) x);
    char *yKey = (char *) (tp->g->keyOffset + (char *) y);

    return strncmp(xKey, yKey, tp->g->keySize);
}


int
ptree_Comp_int32( ptree_sTable *tp, ptree_sNode *x, ptree_sNode *y)
{
    pwr_tInt32 *xKey = (pwr_tInt32 *) (tp->g->keyOffset + (char *) x);
    pwr_tInt32 *yKey = (pwr_tInt32 *) (tp->g->keyOffset + (char *) y);

    if (*xKey < *yKey)
        return -1;
    else if (*xKey == *yKey)
        return 0;
    else
        return 1;
}
