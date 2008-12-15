/* 
 * Proview   $Id: rt_ndc.c,v 1.9 2008-09-23 07:25:17 claes Exp $
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

/* rt_ndc.c -- Network data conversion.  */

#ifdef	OS_ELN
# include $vaxelnc
#else
# include <stdio.h>
# include <string.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "rt_gdh_msg.h"
#include "rt_errh.h"
#include "rt_pool.h"
#include "rt_vol.h"
#include "rt_net.h"
#include "rt_cvol.h"
#include "rt_cvolc.h"
#include "rt_ndc.h"
#include "rt_ndc_msg.h"
#include "rt_cmvolc.h"


/* Vax f-float on a little endian machine.


   3             2 2             1 1 1           0 0 0           0
   1             4 3             6 5 4           8 7 6           0
  +---------------+---------------+-+-------------+-+-------------+
  |       fraction 15 <- 0        |s|  exponent   : | f 22 <- 16  |
  +---------------+---------------+-+-------------+-+-------------+
   1             0 0             0   0           0 0 2           1
   5             8 7             0   7           1 0 2           6


*/

union vax_f_le {
  unsigned int	i;
  struct {
    unsigned int    f22_16	:  7;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
    unsigned int    f15_0	: 16;
  } b;
};

/* Vax f-float on a big endian machine.



  +-+-------------+   +-+-------------+   +---------------+   +---------------+
  : | f 22 <- 16  |   |s|  exponent   :   : 15 <- 0       |   |      fraction :
  +-+-------------+   +-+-------------+   +---------------+   +---------------+
   0 2           1       0           0     0             0     1             0
   0 2           6       7           1     7             0     5             8

   0             0 0             1 1 1           2 2             3
   0             7 8             5 6 7           3 4             1
  +---------------+---------------+-+-------------+-+-------------+
  |       fraction 0 -> 15        |s|  exponent   : | f 16 -> 22  |
  +---------------+---------------+-+-------------+-+-------------+
   0             0 0             1   0             0 1           2
   0             7 8             5   0             7 6           2

*/

union vax_f_be {
  unsigned int	i;
  struct {
    unsigned int    f0_15	: 16;
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f16_22	:  7;
  } b;
};

/* IEEE single on a little endian machine.

   3             2 2             1 1             0 0             0
   1             4 3             6 5             8 7             0
  +-+-------------+-+-------------+---------------+---------------+
  |s|  exponent   : |        fraction 22 <- 0     :               |
  +-+-------------+-+-------------+---------------+---------------+
     0             0 2           1 1                             0
     7             0 2           6 5                             0

*/

union i3e_s_le {
  unsigned int	i;
  struct {
    unsigned int    f22_0	: 23;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
  } b;
  struct {
    unsigned int    f15_0	: 16;
    unsigned int    f22_16	:  7;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
  } v;
};

/* IEEE single on a big endian machine.

   0             0 0             1 1             2 2             3
   0             7 8             5 6             3 4             1
  +-+-------------+-+-------------+---------------+---------------+
  |s|  exponent   : |        fraction 0 -> 22     :               |
  +-+-------------+-+-------------+---------------+---------------+
     0           0 0 0           0 0                             2
     0           6 7 0           6 7                             2

*/

union i3e_s_be {
  unsigned int	i;
  struct {
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f0_22	: 23;
  } b;
  struct {
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f0_6	:  7;
    unsigned int    f7_22	: 16;
  } v;
};

#define VAX_F_BIAS    0x81
#define I3E_S_BIAS    0x7f
#define VAX_D_BIAS    0x81
#define VAX_G_BIAS    0x401
#define I3E_D_BIAS    0x3ff

#define IBYTE0(i) ((i >> 0x18) & 0x000000ff) 
#define IBYTE1(i) ((i >> 0x08) & 0x0000ff00) 
#define IBYTE2(i) ((i << 0x08) & 0x00ff0000) 
#define IBYTE3(i) ((i << 0x18) & 0xff000000) 

#define ENDIAN_SWAP_INT(t, s)\
  {int i = *(int *)s; *(int *)t = (IBYTE0(i) | IBYTE1(i) | IBYTE2(i) | IBYTE3(i));}

#define SBYTE0(s) ((s >> 0x08) & 0x00ff) 
#define SBYTE1(s) ((s << 0x08) & 0xff00) 

#define ENDIAN_SWAP_SHORT(t, s)\
  {short int i = *(short *)s; *(short *)t = (SBYTE0(i) | SBYTE1(i));}

#if 0
# define ENDIAN_SWAP_BOOL(t, s) (*(int *)t = (0 != *(int *)s))
#else
# define ENDIAN_SWAP_BOOL(t, s) ENDIAN_SWAP_INT(t, s)
#endif

#define touchObject(op)  if (op != NULL && op->l.flags.b.isCached) cvolc_TouchObject(op)


#ifndef __powerpc__
/* .  */

static pwr_tBoolean
decode_aref (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{

  for (; count > 0 && *size >= sizeof(pwr_sAttrRef); count--) {
    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);
    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);

    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);

    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);

    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);

    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);

    *size -= sizeof(pwr_sAttrRef);
  }

  return TRUE;
}    

/* .  */

static pwr_tBoolean
decode_bool (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{

  for (; count > 0 && *size >= sizeof(int); count--) {
    ENDIAN_SWAP_BOOL(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);
    *size -= sizeof(int);
  }

  return TRUE;
}    
#endif


/* .  */

static pwr_tBoolean
decode_copy (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{

  if (tp != sp)
    memcpy(tp, sp, MIN(asize, *size));
  *size = (*size >= asize ? *size - asize : 0);
  
  return TRUE;
}    

/* .  */

static pwr_tBoolean
decode_null (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	size
)
{

  return FALSE;
}    

#if defined(OS_VMS) || defined(OS_ELN)
/* .  */

static pwr_tBoolean
encode_sfloat (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{
  union vax_f_le	*vp;
  union i3e_s_le	i3e;

  for (; count > 0 && *size >= sizeof(float); count--) {
    vp = ((union vax_f_le *)sp);
    
    if (vp->b.f22_16 == 0x7f && vp->b.exp == 0xff && vp->b.f15_0 == 0xffff) {  /* High value.  */
      i3e.i = 0, i3e.v.exp = 0xff;
    } else if (vp->b.f22_16 == 0 && vp->b.exp == 0 && vp->b.f15_0 == 0) {  /* Low value.  */
      i3e.i = 0;
    } else {
      i3e.v.exp	    = vp->b.exp - VAX_F_BIAS + I3E_S_BIAS;
      i3e.v.f15_0   = vp->b.f15_0;
      i3e.v.f22_16  = vp->b.f22_16;
    }

    i3e.b.sign = vp->b.sign;

    ENDIAN_SWAP_INT(tp, &i3e.i);
    tp += sizeof(float);
    sp += sizeof(float);
    *size -= sizeof(float);
  }

  return TRUE;
}    

#elif (defined(OS_LYNX) || defined(OS_LINUX)) && defined(HW_X86)

static pwr_tBoolean
encode_sfloat (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{

  for (; count > 0 && *size >= sizeof(float); count--) {
    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(float);
    sp += sizeof(float);
    *size -= sizeof(float);
  }

  return TRUE;
}
#endif

#if defined(OS_VMS) || defined(OS_ELN)
/* .  */

static pwr_tBoolean
decode_sfloat (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{
  union vax_f_le	*vp;
  union i3e_s_le	i3e;

  for (; count > 0 && *size >= sizeof(float); count--) {
    vp = ((union vax_f_le *)tp);
    ENDIAN_SWAP_INT(&i3e.i, sp);
    
    /* PPC some times sets the sign bit for zero (-0), which isn't valid
     * for f-float. ML 971025
     */ 
    if (i3e.i == 0x80000000)
      i3e.i = 0; /* Clear sign bit */

    if (i3e.b.f22_0 == 0x0 && i3e.b.exp == 0xff) {  /* High value.  */
      vp->b.f22_16 = 0x7f;
      vp->b.exp	   = 0xff;
      vp->b.f15_0  = 0xffff;
    } else if (i3e.b.f22_0 == 0x0 && i3e.b.exp == 0x00) {  /* Low value.  */
      vp->i = 0;
    } else {
      vp->b.exp	   = i3e.v.exp - I3E_S_BIAS + VAX_F_BIAS;
      vp->b.f22_16 = i3e.v.f22_16;
      vp->b.f15_0  = i3e.v.f15_0;
    }

    vp->b.sign = i3e.b.sign;

    tp += sizeof(float);
    sp += sizeof(float);
    *size -= sizeof(float);
  }

  return TRUE;
}    

#elif (defined(OS_LYNX) || defined(OS_LINUX)) && defined(HW_X86)

static pwr_tBoolean
decode_sfloat(
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{
  for (; count > 0 && *size >= sizeof(float); count--) {
    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(float);
    sp += sizeof(float);
    *size -= sizeof(float);
  }

  return TRUE;
}
#endif

#ifndef __powerpc__
/* .  */

static pwr_tBoolean
decode_int (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{

  for (; count > 0 && *size >= sizeof(int); count--) {
    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);
    *size -= sizeof(int);
  }

  return TRUE;
}    
#endif

#ifndef __powerpc__
/* .  */

static pwr_tBoolean
decode_2_int (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{

  for (count *= 2; count > 0 && *size >= sizeof(int); count--) {
    ENDIAN_SWAP_INT(tp, sp);
    tp += sizeof(int);
    sp += sizeof(int);
    *size -= sizeof(int);
  }

  return TRUE;
}    
#endif

#ifndef __powerpc__
/* Convert a short integer.

  NOTA BENE   A short occupies 4 bytes in the Proview rtdb
              and thus this routine is not currently used.  */

static pwr_tBoolean
decode_short (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{

  for (; count > 0 && *size >= sizeof(short); count--) {
    ENDIAN_SWAP_SHORT(tp, sp);
    tp += sizeof(short);
    sp += sizeof(short);
    *size -= sizeof(short);
  }

  return TRUE;
}    
#endif

#ifndef __powerpc__
/* Convert a tiny integer (8 bits).

  NOTA BENE   A tiny occupies 4 bytes in the Proview rtdb
              and thus this routine is not currently used.  */

static pwr_tBoolean
decode_tiny (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{

  for (; count > 0 && *size >= sizeof(char); count--) {
    *tp++ = *sp++;
    (*size)--;
  }

  return TRUE;
}    
#endif

#ifndef __powerpc__
static pwr_tBoolean (*decode[pwr_eTix_])() = {
  decode_null,		/* pwr_eTix__		*/
  decode_bool,		/* pwr_eTix_Boolean	*/
  decode_sfloat,	/* pwr_eTix_Float32	*/
  decode_copy,		/* pwr_eTix_Float64	*/
  decode_tiny,		/* pwr_eTix_Char	*/
  decode_tiny,		/* pwr_eTix_Int8	*/
  decode_short,		/* pwr_eTix_Int16	*/
  decode_int,		/* pwr_eTix_Int32	*/
  decode_tiny,		/* pwr_eTix_UInt8	*/
  decode_short,		/* pwr_eTix_UInt16	*/
  decode_int,		/* pwr_eTix_UInt32	*/
  decode_2_int,		/* pwr_eTix_Objid	*/
  decode_copy,		/* pwr_eTix_Buffer	*/
  decode_copy,		/* pwr_eTix_String	*/
  decode_int,		/* pwr_eTix_Enum	*/
  decode_null,		/* pwr_eTix_Struct	*/
  decode_int,		/* pwr_eTix_Mask	*/
  decode_null,		/* pwr_eTix_Array	*/
  decode_2_int,		/* pwr_eTix_Time	*/
  decode_copy,		/* pwr_eTix_Text	*/
  decode_aref,		/* pwr_eTix_AttrRef	*/
  decode_2_int,		/* pwr_eTix_UInt64	*/
  decode_2_int,		/* pwr_eTix_Int64	*/
  decode_int,		/* pwr_eTix_ClassId	*/
  decode_int,		/* pwr_eTix_TypeId	*/
  decode_int,		/* pwr_eTix_VolumeId	*/
  decode_int,		/* pwr_eTix_ObjectIx	*/
  decode_2_int,		/* pwr_eTix_RefId	*/
  decode_2_int,		/* pwr_eTix_DeltaTime	*/
};
static pwr_tBoolean (*encode[pwr_eTix_])() = {
  decode_null,		/* pwr_eTix__		*/
  decode_bool,		/* pwr_eTix_Boolean	*/
  encode_sfloat,	/* pwr_eTix_Float32	*/
  decode_null,		/* pwr_eTix_Float64	*/
  decode_tiny,		/* pwr_eTix_Char	*/
  decode_tiny,		/* pwr_eTix_Int8	*/
  decode_short,		/* pwr_eTix_Int16	*/
  decode_int,		/* pwr_eTix_Int32	*/
  decode_tiny,		/* pwr_eTix_UInt8	*/
  decode_short,		/* pwr_eTix_UInt16	*/
  decode_int,		/* pwr_eTix_UInt32	*/
  decode_2_int,		/* pwr_eTix_Objid	*/
  decode_copy,		/* pwr_eTix_Buffer	*/
  decode_copy,		/* pwr_eTix_String	*/
  decode_int,		/* pwr_eTix_Enum	*/
  decode_null,		/* pwr_eTix_Struct	*/
  decode_int,		/* pwr_eTix_Mask	*/
  decode_null,		/* pwr_eTix_Array	*/
  decode_2_int,		/* pwr_eTix_Time	*/
  decode_copy,		/* pwr_eTix_Text	*/
  decode_aref,		/* pwr_eTix_AttrRef	*/
  decode_2_int,		/* pwr_eTix_UInt64	*/
  decode_2_int,		/* pwr_eTix_Int64	*/
  decode_int,		/* pwr_eTix_ClassId	*/
  decode_int,		/* pwr_eTix_TypeId	*/
  decode_int,		/* pwr_eTix_VolumeId	*/
  decode_int,		/* pwr_eTix_ObjectIx	*/
  decode_2_int,		/* pwr_eTix_RefId	*/
  decode_2_int,		/* pwr_eTix_DeltaTime	*/
};
#else
static pwr_tBoolean (*decode[pwr_eTix_])() = {
  decode_null,		/* pwr_eTix__		*/
  decode_copy,		/* pwr_eTix_Boolean	*/
  decode_copy,		/* pwr_eTix_Float32	*/
  decode_copy,		/* pwr_eTix_Float64	*/
  decode_copy,		/* pwr_eTix_Char	*/
  decode_copy,		/* pwr_eTix_Int8	*/
  decode_copy,		/* pwr_eTix_Int16	*/
  decode_copy,		/* pwr_eTix_Int32	*/
  decode_copy,		/* pwr_eTix_UInt8	*/
  decode_copy,		/* pwr_eTix_UInt16	*/
  decode_copy,		/* pwr_eTix_UInt32	*/
  decode_copy,		/* pwr_eTix_Objid	*/
  decode_copy,		/* pwr_eTix_Buffer	*/
  decode_copy,		/* pwr_eTix_String	*/
  decode_copy,		/* pwr_eTix_Enum	*/
  decode_null,		/* pwr_eTix_Struct	*/
  decode_copy,		/* pwr_eTix_Mask	*/
  decode_null,		/* pwr_eTix_Array	*/
  decode_copy,		/* pwr_eTix_Time	*/
  decode_copy,		/* pwr_eTix_Text	*/
  decode_copy,		/* pwr_eTix_AttrRef	*/
  decode_copy,		/* pwr_eTix_UInt64	*/
  decode_copy,		/* pwr_eTix_Int64	*/
  decode_copy,		/* pwr_eTix_ClassId	*/
  decode_copy,		/* pwr_eTix_TypeId	*/
  decode_copy,		/* pwr_eTix_VolumeId	*/
  decode_copy,		/* pwr_eTix_ObjectIx	*/
  decode_copy,		/* pwr_eTix_RefId	*/
  decode_copy,		/* pwr_eTix_DeltaTime	*/
};
static pwr_tBoolean (*encode[pwr_eTix_])() = {
  decode_null,		/* pwr_eTix__		*/
  decode_copy,		/* pwr_eTix_Boolean	*/
  decode_copy,		/* pwr_eTix_Float32	*/
  decode_null,		/* pwr_eTix_Float64	*/
  decode_copy,		/* pwr_eTix_Char	*/
  decode_copy,		/* pwr_eTix_Int8	*/
  decode_copy,		/* pwr_eTix_Int16	*/
  decode_copy,		/* pwr_eTix_Int32	*/
  decode_copy,		/* pwr_eTix_UInt8	*/
  decode_copy,		/* pwr_eTix_UInt16	*/
  decode_copy,		/* pwr_eTix_UInt32	*/
  decode_copy,		/* pwr_eTix_Objid	*/
  decode_copy,		/* pwr_eTix_Buffer	*/
  decode_copy,		/* pwr_eTix_String	*/
  decode_copy,		/* pwr_eTix_Enum	*/
  decode_null,		/* pwr_eTix_Struct	*/
  decode_copy,		/* pwr_eTix_Mask	*/
  decode_null,		/* pwr_eTix_Array	*/
  decode_copy,		/* pwr_eTix_Time	*/
  decode_copy,		/* pwr_eTix_Text	*/
  decode_copy,		/* pwr_eTix_AttrRef	*/
  decode_copy,		/* pwr_eTix_UInt64	*/
  decode_copy,		/* pwr_eTix_Int64	*/
  decode_copy,		/* pwr_eTix_ClassId	*/
  decode_copy,		/* pwr_eTix_TypeId	*/
  decode_copy,		/* pwr_eTix_VolumeId	*/
  decode_copy,		/* pwr_eTix_ObjectIx	*/
  decode_copy,		/* pwr_eTix_RefId	*/
  decode_copy,		/* pwr_eTix_DeltaTime	*/
};
#endif

/**
 * @note There is no support for double. 
 *       if we want to implement it we have too look at the OS to decide what 
 *       format to use. Because for backward compatibility is the float format set 
 *       to vaxF for VAX to AXP and not vaxD and vaxG.
 */

pwr_tBoolean
ndc_ConvertData (
  pwr_tStatus		*sts,
  const gdb_sNode	*np,
  gdb_sClass		*cp, 
  const pwr_sAttrRef	*arp,
  void			*tp,	/* Address of target.  */
  const void		*sp,	/* Address of source.  */
  pwr_tUInt32		*size,	/* Size of source.  */
  ndc_eOp		op, 
  pwr_tUInt32           offset,
  pwr_tUInt32           offs
)
{
  int			i;
  int			base;
  gdb_sAttribute	*ap;
  pwr_tUInt32           aoffs;
  pwr_tUInt32           count;

 /* The new way, convert if different co_mFormat
  * The old way, always convert if different OS 
  */

  if ((np->netver >= net_cFirstCclassVersion && np->fm.m == gdbroot->my_node->fm.m) 
      || (np->netver < net_cFirstCclassVersion && 
          np->os == gdbroot->my_node->os && np->fm.b.bo == gdbroot->my_node->fm.b.bo)) {
      if (tp != sp)
        memcpy(tp, sp, *size);
      return TRUE;
  }
  
  /* Find attribute.  */

  for (i = 0, ap = cp->attr; i < cp->acount; i++, ap++)
    if (offset <= ap->moffset)
      break;

  if (i >= cp->acount) pwr_Return(NO, sts, NDC__OFFSET);

  if (offset == 0)
    base = 0;
  else
    base = ap->offs;

  switch (op) {
  case ndc_eOp_encode:
    for (; i < cp->acount && *size > 0; i++, ap++) {
      if (ap->flags.b.isclass) {
        gdb_sClass               *lcp;
	  
        lcp = hash_Search(sts, gdbroot->cid_ht, &ap->tid);

	/* Single attribute or array */
	/* Loop n:o element */

	aoffs = 0; /* Attribute offset - source */
	
	for (count = ap->elem; count > 0 && *size > 0; count--) {
          ndc_ConvertData(sts, np, lcp, arp, tp, sp, size, op, (offset - ap->offs) % (ap->size / ap->elem), ap->offs - base + aoffs + offs);
	  aoffs += ap->size / ap->elem;  
	}
      } else {
        if(!encode[pwr_Tix(ap->type)](ap->elem, ap->size, (char *)tp + (ap->offs - base + offs),
	  (char *)sp + (ap->offs - base + offs), size))
	  pwr_Return(NO, sts, NDC__CONVERT);
      }
    }
    break;
  case ndc_eOp_decode:
    for (; i < cp->acount && *size > 0; i++, ap++) {
      if (ap->flags.b.isclass) {
        gdb_sClass               *lcp;
	  
        lcp = hash_Search(sts, gdbroot->cid_ht, &ap->tid);

	/* Single attribute or array */
	/* Loop n:o element */

	aoffs = 0; /* Attribute offset - source */
	
	for (count = ap->elem; count > 0 && *size > 0; count--) {
          ndc_ConvertData(sts, np, lcp, arp, tp, sp, size, op, (offset - ap->offs) % (ap->size / ap->elem), ap->offs - base + aoffs + offs);
	  aoffs += ap->size / ap->elem;  
	}
      } else {
        if(!decode[pwr_Tix(ap->type)](ap->elem, ap->size, (char *)tp + (ap->offs - base + offs),
	  (char *)sp + (ap->offs - base + offs), size))
 	  pwr_Return(NO, sts, NDC__CONVERT);
      }
    }
    break;
  default:
    {
      char ebuf[80];
      sprintf(ebuf, "unknown op: %d", op);
      errh_Bugcheck(NDC__OP, ebuf);
    }
    break;
  }

  pwr_Return(YES, sts, NDC__SUCCESS);
}

/**
 * Converts native data that has a different class version.
 * 
 */
pwr_tBoolean
ndc_ConvertNativeToRemoteData (
  pwr_tStatus		*sts,
  const gdb_sCclass	*ccp,	/**< Cached class */
  pwr_tUInt32		ridx,	/**< Attribute index in ccp */
  const mvol_sAttribute *nap,   /**< Native attribute */
  const pwr_sAttrRef	*rarp,  /**< Remote attribute reference */
  const pwr_sAttrRef	*narp,  /**< Native attribute reference */
  void			*tp,	/**< Address of target.  */
  const void		*sp,	/**< Address of source.  */
  pwr_tUInt32		*size,	/**< Size of target buffer.  */
  pwr_tUInt32           offset,
  pwr_tUInt32           toffs,
  pwr_tUInt32           soffs,
  pwr_tNodeId           nid
)
{
  const net_sCattribute	*cap;
  conv_eIdx		cidx;
  int			i,j;
  int 			tasize,sasize;
  pwr_mAdef		adef;
  int			tcount;
  int			scount;
  int                   zsize;
  pwr_tUInt32           asoffs;
  pwr_tUInt32           atoffs;
  const gdb_sClass	*cp;
  const gdb_sAttribute	*ap;
  
  gdb_AssumeUnlocked;

  if (offset == 0) { /* from start */
    
    cp = hash_Search(sts, gdbroot->cid_ht, &ccp->key.cid);
    if (cp == NULL) 
      errh_Bugcheck(GDH__WEIRD, "can't find native class");
    

    for (i = 0, cap = ccp->attr; i < ccp->acount && *size > 0; i++, cap++) {
      for (j = 0, ap = cp->attr; j < cp->acount; j++, ap++) {
        if (ap->aix == cap->aix) {
	  if (ap->flags.b.isclass) {
            gdb_sCcVolKey             ccvKey;
            gdb_sCclassKey            ccKey;
            gdb_sCclass               *lccp;
            gdb_sCclassVolume         *ccvp;
	  
            ccvKey.nid = nid;
            ccvKey.vid = ap->tid >> 16;
	  
            ccvp = hash_Search(sts, gdbroot->ccvol_ht, &ccvKey);
	  
            ccKey.cid = ap->tid;
            ccKey.ccvoltime = ccvp->time;

            lccp = hash_Search(sts, gdbroot->cclass_ht, &ccKey);
            if (lccp == NULL) errh_Bugcheck(GDH__WEIRD, "can't get class");
	    	      
	    atoffs = 0; /* Attribute offset - target */
	    asoffs = 0; /* Attribute offset - source */
	    for (scount = ap->elem, tcount = cap->elem; tcount > 0 && *size > 0; tcount--, scount--) {
	      if (scount > 0) {
                ndc_ConvertNativeToRemoteData(sts, lccp, ridx, nap, rarp, narp, tp, sp, size, 0,
                    toffs + atoffs, soffs + asoffs, nid);
	        asoffs += ap->size / ap->elem;  
	      } else {
	        memset(tp + toffs + atoffs, 0, cap->size / cap->elem);
	        *size -= cap->size / cap->elem;
	      }
	      atoffs += cap->size / cap->elem;
	    }
	  } else {
            tasize = cap->size / cap->elem;
            sasize = ap->size / ap->elem;

            cidx = conv_GetIdx(ap->type, cap->type);
            if (cidx == conv_eIdx_invalid)
              cidx = conv_eIdx_zero; /* Zero the attribute */

            /* Prevent conversion of pointers in objects. 
             * If we are unlucky we can get a floating point exception.
             */
            adef.m = ap->flags.m;
            adef.b.privatepointer = 1; 
          

            if (!conv_Fctn[cidx](cap->elem, tasize, (char *)tp + cap->offs + toffs, (int *) size,
	         ap->elem, sasize, (const char *)sp + ap->offs + soffs, adef)) {
              pwr_Return(NO, sts, NDC__CONVERT);
            }
	  }
          break;
        }
      }

      if (j >= cp->acount) {/* the remote attribute doesn't exist locally */
        zsize = MIN(*size, cap->size);
	memset((char *) tp + (cap->offs + toffs), 0, zsize);
	*size -= zsize;
      }
    }
    
    
  } else { /* single attribute */

    /* Find attribute.  */

    cp = hash_Search(sts, gdbroot->cid_ht, &ccp->key.cid);
    if (cp == NULL) 
      return FALSE;

    for (i = 0, cap = ccp->attr; i < ccp->acount; i++, cap++)
      if (offset <= cap->moffset)
        break;

    if (i >= ccp->acount) pwr_Return(NO, sts, NDC__OFFSET);

    for (j = 0, ap = cp->attr; j < cp->acount; j++, ap++) {
      if (ap->aix == cap->aix) {
        if (ap->flags.b.isclass) {
          gdb_sCcVolKey             ccvKey;
          gdb_sCclassKey            ccKey;
          gdb_sCclass               *lccp;
          gdb_sCclassVolume         *ccvp;
	  
          ccvKey.nid = nid;
          ccvKey.vid = ap->tid >> 16;
	  
          ccvp = hash_Search(sts, gdbroot->ccvol_ht, &ccvKey);
	  
          ccKey.cid = ap->tid;
          ccKey.ccvoltime = ccvp->time;

          lccp = hash_Search(sts, gdbroot->cclass_ht, &ccKey);
          if (lccp == NULL) errh_Bugcheck(GDH__WEIRD, "can't get class");

          atoffs = 0; /* Attribute offset - target */
          asoffs = 0; /* Attribute offset - source */
          for (tcount = cap->elem, scount = ap->elem; tcount > 0 && *size > 0; tcount--, scount--) {
	    if (scount > 0) {
              ndc_ConvertRemoteToNativeData(sts, lccp, ridx, nap, rarp, narp, tp, sp, size, (offset - cap->offs) % (cap->size / cap->elem),
                    toffs + atoffs, soffs + asoffs, nid);
	      asoffs += ap->size / ap->elem;
	    } else {
	      memset(tp + toffs + atoffs, 0, cap->size / cap->elem);
	      *size -= cap->size / cap->elem;
	    }
	    atoffs += cap->size / cap->elem;
	  }
	} else {

          pwr_Assert(nap->adef != NULL);
    
          sasize = nap->size / nap->elem;
          tasize = cap->size / cap->elem;

          cidx = conv_GetIdx(ap->type, cap->type);
          if (cidx == conv_eIdx_invalid) {
            pwr_Return(NO, sts, NDC__NOCONV);    
          } 

          /* Prevent conversion of pointers if it's not a single pointer. 
           * If we are unlucky we can get a floating point exception.
          */
          adef.m = nap->adef->Info.Flags;
          if (adef.b.array && *size > cap->size/cap->elem)
            adef.b.privatepointer = 1; 

          if (!conv_Fctn[cidx](cap->elem, tasize, tp, (int *) size, 
                       nap->elem, sasize, sp, adef)) {
            pwr_Return(NO, sts, NDC__CONVERT);
          }
	}
      }
    }
    if (j >= cp->acount) {/* the remote attribute doesn't exist locally */
      memset((char *) tp, 0, *size);
      *size = 0;
    }
  }

  pwr_Return(YES, sts, NDC__SUCCESS);

}


/**
 * Encodes/decodes remote data by means of the cached class.
 */
pwr_tBoolean
ndc_ConvertRemoteData (
  pwr_tStatus		*sts,
  const gdb_sNode	*np,
  const gdb_sCclass	*ccp,
  const pwr_sAttrRef	*arp,
  void			*tp,	/* Address of target.  */
  const void		*sp,	/* Address of source.  */
  pwr_tUInt32		*size,	/* Size of source.  */
  ndc_eOp		op, 
  pwr_tUInt32           offset,
  pwr_tUInt32           offs
)
{
  int			i;
  int			base;
  const net_sCattribute	*cap;
  pwr_tUInt32           aoffs;
  pwr_tUInt32           count;


  if (np->fm.m == gdbroot->my_node->fm.m) {
    if (tp != sp)
      memcpy(tp, sp, *size);
    return TRUE;
  }

  /* Find attribute.  */

  for (i = 0, cap = ccp->attr; i < ccp->acount; i++, cap++)
    if (offset <= cap->offs + cap->size - 1) /* maximum attribute offset */
      break;

  if (i >= ccp->acount) pwr_Return(NO, sts, NDC__OFFSET);

  if (offset == 0)
    base = 0;
  else
    base = cap->offs;

  switch (op) {
  case ndc_eOp_encode:
    for (; i < ccp->acount && *size > 0; i++, cap++) {
      if (cap->flags.b.isclass) {
        gdb_sCcVolKey             ccvKey;
        gdb_sCclassKey            ccKey;
        gdb_sCclass               *lccp;
        gdb_sCclassVolume         *ccvp;
	  
        ccvKey.nid = np->nid;
        ccvKey.vid = cap->type >> 16;
	ccvp = hash_Search(sts, gdbroot->ccvol_ht, &ccvKey);
	  
        ccKey.cid = cap->type;
        ccKey.ccvoltime = ccvp->time;
        lccp = hash_Search(sts, gdbroot->cclass_ht, &ccKey);

	/* Single attribute or array */
	/* Loop n:o element */

	aoffs = 0; /* Attribute offset - source */
	
	for (count = cap->elem; count > 0 && *size > 0; count--) {
          ndc_ConvertRemoteData(sts, np, lccp, arp, tp, sp, size, op, (offset - cap->offs) % (cap->size / cap->elem), cap->offs - base + aoffs + offs);
	  aoffs += cap->size / cap->elem;  
	}
      } else {
      
      if(!encode[pwr_Tix(cap->type)](cap->elem, cap->size, (char *)tp + (cap->offs - base + offs),
	(char *)sp + (cap->offs - base + offs), size))
	pwr_Return(NO, sts, NDC__CONVERT);
      }
    }
    break;
  case ndc_eOp_decode:
    for (; i < ccp->acount && *size > 0; i++, cap++) {
      if (cap->flags.b.isclass) {
        gdb_sCcVolKey             ccvKey;
        gdb_sCclassKey            ccKey;
        gdb_sCclass               *lccp;
        gdb_sCclassVolume         *ccvp;
	  
        ccvKey.nid = np->nid;
        ccvKey.vid = cap->type >> 16;
	ccvp = hash_Search(sts, gdbroot->ccvol_ht, &ccvKey);
	  
        ccKey.cid = cap->type;
        ccKey.ccvoltime = ccvp->time;
        lccp = hash_Search(sts, gdbroot->cclass_ht, &ccKey);

	/* Single attribute or array */
	/* Loop n:o element */

	aoffs = 0; /* Attribute offset - source */
	
	for (count = cap->elem; count > 0 && *size > 0; count--) {
          ndc_ConvertRemoteData(sts, np, lccp, arp, tp, sp, size, op, (offset - cap->offs) % (cap->size / cap->elem), cap->offs - base + aoffs + offs);
	  aoffs += cap->size / cap->elem;  
	}
      } else {
        if(!decode[pwr_Tix(cap->type)](cap->elem, cap->size, (char *)tp + (cap->offs - base + offs),
	  (char *)sp + (cap->offs - base + offs), size))
	  pwr_Return(NO, sts, NDC__CONVERT);
      }
    }
    break;
  default:
    {
      char ebuf[80];
      sprintf(ebuf, "unknown op: %d", op);
      errh_Bugcheck(NDC__OP, ebuf);
    }
    break;
  }

  pwr_Return(YES, sts, NDC__SUCCESS);
}

/**
 * Converts remote data that has a different class version.
 * The data has already been converted to native data format
 */
pwr_tBoolean
ndc_ConvertRemoteToNativeData (
  pwr_tStatus		*sts,
  const gdb_sCclass	*ccp,	/**< Cached class */
  pwr_tUInt32		ridx,	/**< Attribute index in ccp */
  const mvol_sAttribute *nap,   /**< Native attribute */
  const pwr_sAttrRef	*rarp,  /**< Remote attribute reference */
  const pwr_sAttrRef	*narp,  /**< Native attribute reference */
  void			*tp,	/**< Address of target.  */
  const void		*sp,	/**< Address of source.  */
  pwr_tUInt32		*size,	/**< Size of target buffer.  */
  pwr_tUInt32           offset,
  pwr_tUInt32           toffs,
  pwr_tUInt32           soffs,
  pwr_tNodeId           nid
)
{
  const net_sCattribute	*cap;
  conv_eIdx		cidx;
  int			i,j;
  int 			tasize,sasize;
  pwr_mAdef		adef;
  int			tcount;
  int			scount;
  int                   zsize;
  pwr_tUInt32           asoffs;
  pwr_tUInt32           atoffs;
  const gdb_sAttribute	*ap;
  const gdb_sClass      *cp;

  gdb_AssumeUnlocked;

  if (offset == 0) { /* from start */
    

//    pwr_Assert(narp->Offset == 0);

    cp = hash_Search(sts, gdbroot->cid_ht, &ccp->key.cid);
    if (cp == NULL) 
      return FALSE;

    ap = cp->attr;

    for (i = 0; i < cp->acount && *size > 0; i++, ap++) {
      for (j = 0, cap = ccp->attr; j < ccp->acount; j++, cap++) {
        if (ap->aix == cap->aix) {
	
	  if (ap->flags.b.isclass) {
            gdb_sCcVolKey             ccvKey;
            gdb_sCclassKey            ccKey;
            gdb_sCclass               *lccp;
            gdb_sCclassVolume         *ccvp;
	  
            ccvKey.nid = nid;
            ccvKey.vid = ap->tid >> 16;
	  
            ccvp = hash_Search(sts, gdbroot->ccvol_ht, &ccvKey);
	  
            ccKey.cid = ap->tid;
            ccKey.ccvoltime = ccvp->time;

            lccp = hash_Search(sts, gdbroot->cclass_ht, &ccKey);
            if (lccp == NULL) errh_Bugcheck(GDH__WEIRD, "can't get class");

	    atoffs = 0; /* Attribute offset - target */
	    asoffs = 0; /* Attribute offset - source */
	    for (tcount = ap->elem, scount = cap->elem; tcount > 0 && *size > 0; tcount--, scount--) {
	      if (scount > 0) {
                ndc_ConvertRemoteToNativeData(sts, lccp, ridx, nap, rarp, narp, tp, sp, size, 0,
                    toffs + atoffs, soffs + asoffs, nid);
	        asoffs += cap->size / cap->elem;  
	      } else {
	        memset(tp + toffs + atoffs, 0, ap->size / ap->elem);
	        *size -= ap->size / ap->elem;
	      }
	      atoffs += ap->size / ap->elem;
	    }
	  } else {	  
            tasize = ap->size / ap->elem;
            sasize = cap->size / cap->elem;

            cidx = conv_GetIdx(cap->type, ap->type);
            if (cidx == conv_eIdx_invalid)
              cidx = conv_eIdx_zero; /* Zero the attribute */

            /* Prevent conversion of pointers in objects. 
             * If we are unlucky we can get a floating point exception.
             */
            adef.m = cap->flags.m;
            adef.b.privatepointer = 1;          

            if (!conv_Fctn[cidx](ap->elem, tasize, tp + ap->offs + toffs, (int *) size, cap->elem, sasize, sp + cap->offs + soffs, adef)) {
              pwr_Return(NO, sts, NDC__CONVERT);
            }
	  }
	  break;
        }
      }

      if (j >= ccp->acount) {/* the native attribute doesn't exist remotely */
        zsize = MIN(*size, ap->size);
	memset((char *) tp + (ap->offs + toffs), 0, zsize);
	*size -= zsize;
      }
    }
        
    
  } else { /* single attribute */
  
    /* Find attribute.  */

    cp = hash_Search(sts, gdbroot->cid_ht, &ccp->key.cid);
    if (cp == NULL) 
      return FALSE;

    for (i = 0, ap = cp->attr; i < cp->acount; i++, ap++)
      if (offset <= ap->moffset)
        break;

    if (i >= cp->acount) pwr_Return(NO, sts, NDC__OFFSET);

    for (j = 0, cap = ccp->attr; j < ccp->acount; j++, cap++) {
      if (ap->aix == cap->aix) {
        if (ap->flags.b.isclass) {
          gdb_sCcVolKey             ccvKey;
          gdb_sCclassKey            ccKey;
          gdb_sCclass               *lccp;
          gdb_sCclassVolume         *ccvp;
	  
          ccvKey.nid = nid;
          ccvKey.vid = ap->tid >> 16;
	  
          ccvp = hash_Search(sts, gdbroot->ccvol_ht, &ccvKey);
	  
          ccKey.cid = ap->tid;
          ccKey.ccvoltime = ccvp->time;

          lccp = hash_Search(sts, gdbroot->cclass_ht, &ccKey);
          if (lccp == NULL) errh_Bugcheck(GDH__WEIRD, "can't get class");

          atoffs = 0; /* Attribute offset - target */
          asoffs = 0; /* Attribute offset - source */
          for (tcount = ap->elem, scount = cap->elem; tcount > 0 && *size > 0; tcount--, scount--) {
	    if (scount > 0) {
              ndc_ConvertRemoteToNativeData(sts, lccp, ridx, nap, rarp, narp, tp, sp, size, (offset - ap->offs) % (ap->size / ap->elem),
                    toffs + atoffs, soffs + asoffs, nid);
	      asoffs += cap->size / cap->elem;
	    } else {
	      memset(tp + toffs + atoffs, 0, ap->size / ap->elem);
	        *size -= ap->size / ap->elem;
	    }
	    atoffs += ap->size / ap->elem;
	  }
	} else {
          pwr_Assert(nap->adef != NULL);
    
          tasize = nap->size / nap->elem;
          sasize = cap->size / cap->elem;
    
          cidx = conv_GetIdx(cap->type, ap->type);
          if (cidx == conv_eIdx_invalid) {
            pwr_Return(NO, sts, NDC__NOCONV);    
	  }

          /* Prevent conversion of pointers if it's not a single attribute. 
           * If we are unlucky we can get a floating point exception.
           */
          *size = MIN(*size, nap->size);
          adef = cap->flags;
          if (adef.b.array && *size > nap->size/nap->elem)
            adef.b.privatepointer = 1;

          if (!conv_Fctn[cidx](nap->elem, tasize, tp, (int *) size, 
                       cap->elem, sasize, sp, adef)) {
            pwr_Return(NO, sts, NDC__CONVERT);
          }
	}
      }
    } 
    if (j >= ccp->acount) {/* the native attribute doesn't exist remotely */
      memset((char *) tp, 0, *size);
      *size = 0;
    }
  }

  pwr_Return(YES, sts, NDC__SUCCESS);
}

/**
 * Converts remote data that has a different class version.
 * The data has already been converted to native data format
 */
pwr_tBoolean
ndc_ConvertRemoteToNativeTable (
  pwr_tStatus			*sts,
  const gdb_sCclass		*ccp,	/**< Cached class */
  const ndc_sRemoteToNative	*tbl,
  const pwr_sAttrRef		*rarp,  /**< Remote attribute reference */
  const pwr_sAttrRef		*narp,  /**< Native attribute reference */
  void				*tp,	/**< Address of target.  */
  const void			*sp,	/**< Address of source.  */
  pwr_tUInt32			*size,	/**< Size of target buffer.  */
  pwr_tUInt32                   offset,
  pwr_tUInt32                   toffs,
  pwr_tUInt32                   soffs,
  pwr_tBoolean                  *first,
  pwr_tNodeId                   nid
)
{
  const gdb_sClass	*cp;
  const gdb_sAttribute	*ap;
  const net_sCattribute	*cap;
  int			i;
  int			base;
  int 			zsize;
  int 			idx;
  int  			relem;
  int			tcount;
  int			scount;
  pwr_tUInt32           asoffs;
  pwr_tUInt32           atoffs;
  pwr_tUInt32		roffs;
  pwr_tUInt32		raidx;
  conv_eIdx		cidx;
  pwr_mAdef		adef;
  
  

  gdb_AssumeLocked;

  cp = hash_Search(sts, gdbroot->cid_ht, &ccp->key.cid);
  if (cp == NULL) 
    return FALSE;

  /* Find attribute.  */

  for (i = 0, ap = cp->attr; i < cp->acount; i++, ap++)
    if (offset <= ap->moffset)
      break;

  if (i >= cp->acount) pwr_Return(NO, sts, NDC__OFFSET);

  if (offset == 0)
    base = 0;
  else
    base = ap->offs;


  for (; i < cp->acount && *size > 0; i++, ap++) {
    cidx = tbl[i].cidx;
    raidx =  tbl[i].raidx;
    pwr_Assert(raidx < ccp->acount || raidx == ULONG_MAX);    

    if ( raidx == ULONG_MAX || cidx == conv_eIdx_invalid) {
      /* Attribute doesn't exist on remote node or there is no valid conversion
       * Zero the local attribute 
       */
      zsize = MIN(*size, ap->size);
      memset((char *)tp + (ap->offs - base + toffs), 0, zsize);
      *size -= zsize;
      if (*first)
        *first = 0;
    } 
    else if (ap->flags.b.isclass) {
      gdb_sCcVolKey             ccvKey;
      gdb_sCclassKey            ccKey;
      gdb_sCclass               *lccp;
      gdb_sCclassVolume         *ccvp;
      ndc_sRemoteToNative	*ltbl;
	  
      ccvKey.nid = nid;
      ccvKey.vid = ap->tid >> 16;
	  
      ccvp = hash_Search(sts, gdbroot->ccvol_ht, &ccvKey);
	  
      ccKey.cid = ap->tid;
      ccKey.ccvoltime = ccvp->time;

      lccp = hash_Search(sts, gdbroot->cclass_ht, &ccKey);
             
      if (!lccp) {
        /* If class iis not found something is very weird, it shouldn't happen 
         * anyway if it happens zero local attribute and contnue with next
         */
        zsize = MIN(*size, ap->size);
        memset((char *)tp + (ap->offs - base + toffs), 0, zsize);
        *size -= zsize;
        if (*first)
          *first = 0;
	continue;
      }

      if (!lccp->flags.b.rnConv) {
        const gdb_sClass      *lcp = hash_Search(sts, gdbroot->cid_ht, &lccp->key.cid);

	ltbl = pool_Alloc(sts, gdbroot->pool, sizeof(*ltbl) * lcp->acount);
	ndc_UpdateRemoteToNativeTable(sts, ltbl, lcp->acount, lcp, lccp, nid);
	if (ODD(*sts)) {
	   lccp->rnConv = pool_Reference(NULL, gdbroot->pool, ltbl);
	   lccp->flags.b.rnConv = 1;
	} else {
	  pool_Free(NULL, gdbroot->pool, ltbl);
          pwr_Return(NO, sts, NDC__CONVERT);      
	}
      } else {
        ltbl = pool_Address(NULL, gdbroot->pool, lccp->rnConv);
      }

      cap = &ccp->attr[raidx];
	  
      if (lccp == NULL) errh_Bugcheck(GDH__WEIRD, "can't get class");
      
      /* It could either be: */
      /* - Whole attribute, or ... */
      /* - Single array-element, or ... */
      /* - Atrribute in attribute. */
      
      if (*first) {
        if (base != 0 && offset > ap->offs) {
	  if (*size == ap->size / ap->elem) {

	    /* Single array-element */
	    /* Check if source element exist */
	    
	    if ((offset - ap->offs) / (ap->size / ap->elem) < cap->elem) {
              ndc_ConvertRemoteToNativeTable(sts, lccp, ltbl, NULL, NULL, tp, sp, size, 0,
                                             toffs + ap->offs - base, soffs, first, nid);
	    } else {
	      memset(tp + toffs + ap->offs - base, 0, *size);
	      *size = 0;
	    }
	  } else {
	  
	    /* Atrribute in attribute */

            ndc_ConvertRemoteToNativeTable(sts, lccp, ltbl, NULL, NULL, tp, sp, size, (offset - ap->offs) % (ap->size/ap->elem),
                  toffs + ap->offs - base, soffs, first, nid);	    
	  }
	} else {
	  /* Single attribute or array */
	  /* Loop n:o element */
	  /* Check boundaries */

	  atoffs = 0; /* Attribute offset - target */
	  asoffs = 0; /* Attribute offset - source */
	  for (tcount = ap->elem, scount = cap->elem; tcount > 0 && *size > 0; tcount--, scount--) {
	    if (scount > 0) {
              ndc_ConvertRemoteToNativeTable(sts, lccp, ltbl, NULL, NULL, tp, sp, size, (offset - ap->offs) % (ap->size/ap->elem),
                  toffs + atoffs + ap->offs - base, soffs + asoffs, first, nid);
	      asoffs += cap->size / cap->elem;  
	    } else {
	      memset(tp + toffs + atoffs + ap->offs - base, 0, ap->size / ap->elem);
	      *size -= ap->size / ap->elem;
	    }
	    atoffs += ap->size / ap->elem;
	  }
	}
	*first = 0;
      } else {
	/* Single attribute or array */
	/* Loop n:o element */
	/* Check boundaries */

	atoffs = 0; /* Attribute offset - target */
	asoffs = 0; /* Attribute offset - source */
	for (tcount = ap->elem, scount = cap->elem; tcount > 0 && *size > 0; tcount--, scount--) {
	  if (scount > 0) {
            ndc_ConvertRemoteToNativeTable(sts, lccp, ltbl, NULL, NULL, tp, sp, size, (offset - ap->offs) % (ap->size/ap->elem),
                  toffs + atoffs + ap->offs - base, cap->offs + soffs + asoffs, first, nid);
	    asoffs += cap->size / cap->elem;  
	  } else {
	    memset(tp + toffs + atoffs + ap->offs - base, 0, ap->size / ap->elem);
	    *size -= ap->size / ap->elem;
	  }
	  atoffs += ap->size / ap->elem;
	}
      }
    }
    else {
      cap = &ccp->attr[raidx];

      /** @note Pointers are only handled correctly for a single pointer, 
       * not arrays. See, vol_AttributeToAddress. Set private for all other
       * cases. 
       * It's quite tricky to find out if it's a single array element. Let's 
       * hope that the size has the exact size of one element. Maybe we should
       * add a flag to the attribute reference that indicates single array element.
       */
      adef = cap->flags;
      if (!*first || (adef.b.array && *size > ap->size/ap->elem))
        adef.b.privatepointer = 1; /* prevent floating point exceptions */


      if (*first) {
        *first = 0;
        roffs = 0;

        /* Check if the first attribute is an array element with index > 0 
         * and that the index exist in the remote attribute.
         */
        if (base != 0 && offset > ap->offs) {        
          pwr_Assert(ap->elem > 1);

          idx = (offset - ap->offs) / (ap->size/ap->elem);

          /* Calm down, the convert routine will only use the source if relem > 0 */
          relem = cap->elem  - idx; 
        } else
          relem = cap->elem;

      } else {
        roffs = cap->offs;
        relem = cap->elem;
      }
      
      /* Adjust size to be multiple of element size */
      if (*size < ap->size)
        *size -= *size % (ap->size / ap->elem) ;

            if(!conv_Fctn[cidx](ap->elem, ap->size/ap->elem, (char *)tp + (ap->offs - base) + toffs, (int *) size,
                        relem, cap->size/cap->elem, (const char *)sp + roffs + soffs, adef))
        pwr_Return(NO, sts, NDC__CONVERT);      
      
    }
  }

  pwr_Return(YES, sts, NDC__SUCCESS);
}

/**
 * Converts remote data that has a different class version.
 * The data has already been converted to native data format
 */
pwr_tBoolean
ndc_ConvertRemoteToNativeTableOld (
  pwr_tStatus			*sts,
  const gdb_sCclass		*ccp,	/**< Cached class */
  const ndc_sRemoteToNative	*tbl,
  const pwr_sAttrRef		*rarp,  /**< Remote attribute reference */
  const pwr_sAttrRef		*narp,  /**< Native attribute reference */
  void				*tp,	/**< Address of target.  */
  const void			*sp,	/**< Address of source.  */
  pwr_tUInt32			size	/**< Size of target buffer.  */
)
{
  const gdb_sClass	*cp;
  const gdb_sAttribute	*ap;
  const net_sCattribute	*cap;
  int			i;
  int			base;
  int 			zsize;
  int 			idx;
  int  			relem;
  pwr_tUInt32		roffs;
  pwr_tUInt32		raidx;
  conv_eIdx		cidx;
  pwr_tBoolean		first;
  pwr_mAdef		adef;
  

  gdb_AssumeLocked;

  cp = hash_Search(sts, gdbroot->cid_ht, &ccp->key.cid);
  if (cp == NULL) 
    return FALSE;

  /* Find attribute.  */

  for (i = 0, ap = cp->attr; i < cp->acount; i++, ap++)
    if (narp->Offset <= ap->moffset)
      break;

  if (i >= cp->acount) pwr_Return(NO, sts, NDC__OFFSET);

  if (narp->Offset == 0)
    base = 0;
  else
    base = ap->offs;


  for (first = 1; i < cp->acount && size > 0; i++, ap++) {
    cidx = tbl[i].cidx;
    raidx =  tbl[i].raidx;
    pwr_Assert(raidx < ccp->acount || raidx == ULONG_MAX);    

    if ( raidx == ULONG_MAX || cidx == conv_eIdx_invalid) {
      /* Attribute doesn't exist on remote node or there is no valid conversion
       * Zero the local attribute 
       */
      zsize = MIN(size, ap->size);
      memset((char *)tp + (ap->offs - base), 0, zsize);
      size -= zsize;
      if (first)
        first = 0;
    } else {
      cap = &ccp->attr[raidx];

      /** @note Pointers are only handled correctly for a single pointer, 
       * not arrays. See, vol_AttributeToAddress. Set private for all other
       * cases. 
       * It's quite tricky to find out if it's a single array element. Let's 
       * hope that the size has the exact size of one element. Maybe we should
       * add a flag to the attribute reference that indicates single array element.
       */
      adef = cap->flags;
      if (!first || (adef.b.array && size > ap->size/ap->elem))
        adef.b.privatepointer = 1; /* prevent floating point exceptions */


      if (first) {
        first = 0;
        roffs = 0;

        /* Check if the first attribute is an array element with index > 0 
         * and that the index exist in the remote attribute.
         */
        if (base != 0 && narp->Offset > ap->offs) {        
          pwr_Assert(ap->elem > 1);

          idx = (narp->Offset - ap->offs) / (ap->size/ap->elem);

          /* Calm down, the convert routine will only use the source if relem > 0 */
          relem = cap->elem  - idx; 
        } else
          relem = cap->elem;

      } else {
        roffs = cap->offs;
        relem = cap->elem;
      }
      


      if(!conv_Fctn[cidx](ap->elem, ap->size/ap->elem, (char *)tp + (ap->offs - base), (int *)&size,
                        relem, cap->size/cap->elem, (const char *)sp + roffs, adef))
        pwr_Return(NO, sts, NDC__CONVERT);      
      
    }
  }

  pwr_Return(YES, sts, NDC__SUCCESS);
}

/**
 * Converts a native attribute reference to a remote attribute reference.
 * 
 * @return The argument rarp or NULL if an error
 */
pwr_sAttrRef *
ndc_NarefToRaref(
  pwr_tStatus 		*sts,   /**< Status */
  mvol_sAttribute	*ap,    /**< Native mvol attribute */
  pwr_sAttrRef	        *narp,  /**< Native attribute reference */ 
  gdb_sCclass	        *ccp,   /**< Cached class */
  pwr_tUInt32		*ridx,  /**< Attribute index in ccp or UINT_LONG if whole object */
  pwr_sAttrRef		*rarp,  /**< Remote attribute reference */
  pwr_tBoolean		*equal, /**< Set if the attribute references are equal, not checked if whole object */
  cdh_sParseName        *pn,    /**< Not NULL if called from Get-/SetObjectInfo */
  gdb_sCclass           *ccpLocked,
  gdb_sVolume           *vp,
  gdb_sNode             *np
)
{
  pwr_tUInt32		i, j;
  const net_sCattribute	*cap;
  gdb_sClass		*acp;
  int			offset = 0;
  int			roffset = 0;
  pwr_tBoolean          fetched;
  mvol_sAttribute       attribute;
  pwr_sAttrRef          aref;
  pwr_tClassId          cid;
  gdb_sCclass	        *l_ccp;   /**< Cached class */
  pwr_tBoolean          l_equal;

  gdb_AssumeLocked;

  *sts = GDH__SUCCESS;
  *equal = 0;  
  *ridx = ULONG_MAX;

  if (ap->aop == NULL) { /* whole object */
    *rarp = *narp;
    rarp->Size = ccp->size;
    return rarp;
  }  

  /* It's a single attribute */

  pwr_Assert(ap->aix != ULONG_MAX);
  
  acp = ap->cp;
  if (acp == NULL)
    pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
  
  l_ccp = ccp;

  /* Loop until we get to the correct offset */

  while (1) {
    for (i = 0; i < acp->acount; i++) {
      if (ap->offs <= (offset + acp->attr[i].moffset))
        break;
    }
  
    if (i == acp->acount)
      pwr_Return(NULL, sts, GDH__ATTRIBUTE);

    offset += acp->attr[i].offs;
  
    for (j = 0, cap = l_ccp->attr; j < l_ccp->acount; j++, cap++) {
      if (acp->attr[i].aix == cap->aix) {
        roffset += cap->offs;
        *ridx    = j;
	break;
      }
    }

    /* Attribute doesn't exist on remote node */

    if (j == l_ccp->acount)
      pwr_Return(NULL, sts, NDC__NRATTRIBUTE);

    if (!acp->attr[i].flags.b.isclass) {
      if (acp->attr[i].elem > 1) {
	roffset += ((narp->Offset - offset) / 
	           (acp->attr[i].size / acp->attr[i].elem)) * 
		  (cap->size / cap->elem);
        offset = narp->Offset;
      }
      break;
    }
  
    if (acp->attr[i].size == narp->Size) {
      /* Fetch the class */
      if (acp->attr[i].flags.b.isclass) {
        cid = acp->attr[i].tid;
        acp = hash_Search(sts, gdbroot->cid_ht, &cid);
        if (acp == NULL)
        pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
    
        l_ccp = cmvolc_GetCachedClass(sts, np, vp, NULL, equal, &fetched, acp);
        if (EVEN(*sts)) {
          np = NULL;
          pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
        }
      }
      break;
    }
  
    if (acp->attr[i].flags.b.array) {
      for (j = 0; j < acp->attr[i].elem; j++) {
	if ( narp->Offset < (offset + acp->attr[i].size / acp->attr[i].elem))
	  break;
	offset += acp->attr[i].size / acp->attr[i].elem;
	roffset += cap->size / cap->elem; 
      }
      if (acp->attr[i].size / acp->attr[i].elem == narp->Size) {

        /* Fetch the class */
        if (acp->attr[i].flags.b.isclass) {
          cid = acp->attr[i].tid;
          acp = hash_Search(sts, gdbroot->cid_ht, &cid);
          if (acp == NULL)
          pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
    
          l_ccp = cmvolc_GetCachedClass(sts, np, vp, NULL, equal, &fetched, acp);
          if (EVEN(*sts)) {
            np = NULL;
            pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
          }
	}
      
        break;
      }
    }

    if (l_ccp != ccp) {
      cmvolc_UnlockClass(NULL, l_ccp);
    }

//    if (ccpLocked) {
//      cmvolc_UnlockClass(NULL, ccpLocked);
//      ccpLocked = NULL;
//    }
  
    /* we're not through yet, get next class */

    cid = acp->attr[i].tid;
    acp = hash_Search(sts, gdbroot->cid_ht, &cid);
    if (acp == NULL)
      pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
    
    l_ccp = cmvolc_GetCachedClass(sts, np, vp, NULL, equal, &fetched, acp);
    if (EVEN(*sts)) {
      np = NULL;
      pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
    }
  
    if (l_ccp != NULL) {
      cmvolc_LockClass(NULL, l_ccp);
    }

    /* If gdb has been unlocked, refresh pointers (cmvolc_getCached.. unlocks) */
    /** @todo Check if we can do it more efficient, eg. vol_ArefToAttribute */
    /* I cannot explain why this must be done, maybe LW has the answer ?? */

    if (fetched) {
      memset(&attribute, 0, sizeof(attribute));
      np = NULL;
    
      if (pn) {
        ap = vol_NameToAttribute(sts, &attribute, pn, gdb_mLo_global, vol_mTrans_all);
        if ((ap == NULL) || (ap->op == NULL))
          pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
        touchObject(ap->op);

        narp = mvol_AttributeToAref(sts, ap, &aref);
        if (narp == NULL)
          pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
      }
      else {
        ap = vol_ArefToAttribute(sts, &attribute, narp, gdb_mLo_global, vol_mTrans_all);
        if ((ap == NULL) || (ap->op == NULL))
          pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
        touchObject(ap->op);
      }

      acp = hash_Search(sts, gdbroot->cid_ht, &cid);
      if (acp == NULL)
        pwr_Return(NULL, sts, GDH__NOSUCHCLASS);

      vp = pool_Address(NULL, gdbroot->pool, ap->op->l.vr);
      np = hash_Search(sts, gdbroot->nid_ht, &vp->g.nid);
      if (np == NULL)
        pwr_Return(NULL, sts, GDH__NOSUCHNODE);

      l_ccp = cmvolc_GetCachedClass(sts, np, vp, NULL, equal, &fetched, acp);
      if (EVEN(*sts)) {
        np = NULL;
        pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
      }
      
      /* Refresh original cached class */

      ccp = cmvolc_GetCachedClass(sts, np, vp, ap, &l_equal, &fetched, NULL);
      if (EVEN(*sts)) {
        np = NULL;
        pwr_Return(NULL, sts, GDH__NOSUCHCLASS);
      }
      ccpLocked = ccp;      

    }  
  }
  
  if (l_ccp != ccp) {
    cmvolc_UnlockClass(NULL, l_ccp);
  }
   
  if (ap->idx == ULONG_MAX) {
    rarp->Objid   = narp->Objid;
    rarp->Body    = narp->Body;
    rarp->Offset  = roffset;
    rarp->Size    = cap->size;
    rarp->Flags.m = narp->Flags.m;
    rarp->Flags.b.Indirect = (cap->flags.b.pointer && !cap->flags.b.privatepointer);
    rarp->Flags.b.Array = cap->flags.b.array;
  } 
  else { /* It's an array element */
    if (ap->idx >= cap->elem) {
      *sts = NDC__NRELEM_IDX;
      return NULL;
    }            

    rarp->Objid   = narp->Objid;
    rarp->Body    = narp->Body;
    rarp->Size    = cap->size / cap->elem;
    rarp->Offset  = roffset;
    rarp->Flags.m = narp->Flags.m;
    rarp->Flags.b.Indirect = (cap->flags.b.pointer && !cap->flags.b.privatepointer);
    rarp->Flags.b.Array = cap->flags.b.array;
  }

  return rarp;

}

ndc_sRemoteToNative *
ndc_UpdateRemoteToNativeTable(
  pwr_tStatus		*sts, 
  ndc_sRemoteToNative	*tbl,  
  pwr_tUInt32		tcnt, /**< # table entries */ 
  const gdb_sClass	*cp,
  const gdb_sCclass	*ccp,
  pwr_tNodeId           nid
  )
{
  const gdb_sAttribute	*ap;
  const net_sCattribute	*cap;
  
  int			i,j;


  if (tcnt < cp->acount) {
    *sts = NDC__BUFOVRUN;
    return NULL;
  }
  
  for (i = 0, ap = cp->attr; i < cp->acount; i++, ap++) {
    for (j = 0, cap = ccp->attr; j < ccp->acount; j++, cap++) {
      if (ap->aix == cap->aix) {
      
        /* If attribute is class then continue with this one */
	
        if (ap->flags.b.isclass) {
/*          const gdb_sClass  *lcp = hash_Search(sts, gdbroot->cid_ht, &ap->tid);
	  gdb_sCcVolKey     ccvKey;
	  gdb_sCclassKey    ccKey;
          gdb_sCclass       *lccp;
	  gdb_sCclassVolume *ccvp;
	  
	  ccvKey.nid = nid;
	  ccvKey.vid = ap->tid >> 16;
	  
	  ccvp = hash_Search(sts, gdbroot->ccvol_ht, &ccvKey);
	  
	  ccKey.cid = ap->tid;
	  ccKey.ccvoltime = ccvp->time;

	  lccp = hash_Search(sts, gdbroot->cclass_ht, &ccKey);
	  
          if ((lcp == NULL) || (lccp == NULL)) errh_Bugcheck(GDH__WEIRD, "can't get class");
	  
	  if (!lccp->flags.b.rnConv) {
	    ndc_sRemoteToNative   *ltbl;
	    ltbl = pool_Alloc(sts, gdbroot->pool, sizeof(*tbl) * lcp->acount);
	    ndc_UpdateRemoteToNativeTable(sts, ltbl, lcp->acount, lcp, lccp, nid);
	    if (ODD(*sts)) {
	      lccp->rnConv = pool_Reference(NULL, gdbroot->pool, ltbl);
	      lccp->flags.b.rnConv = 1;
	    } else {
	      pool_Free(NULL, gdbroot->pool, ltbl);
	      return NULL;
	    }
	  } */
	  
	  tbl[i].cidx = conv_eIdx_;
	  tbl[i].raidx = j;	  	
	}
	else {
          tbl[i].cidx = conv_GetIdx(cap->type, ap->type);
          tbl[i].raidx = j;
	}
        break;
      }
    }

    if (j >= ccp->acount) {
      tbl[i].cidx = conv_eIdx_invalid;
      tbl[i].raidx = ULONG_MAX;
    } 
  }

  *sts = NDC__SUCCESS;
  
  return tbl;
  
}

