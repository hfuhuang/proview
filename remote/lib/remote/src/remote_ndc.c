/* 
 * Proview   $Id: remote_ndc.c,v 1.2 2006-09-15 10:04:54 claes Exp $
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

/* remote_ndc.c -- Network data conversion.  */

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

union{
	float	ff;
	int	ii;
	short int	sii;
	char	cc[4];
	}intern;
union{
        unsigned int	ii;
	char		cc[4];
     }tmp;
/*************************************************************************
*
*       Float32_AXP_(&intel)
*
* Example:	Mess->VaxValue = Intel_Vax_Float(&Value);
* Description:  Convert from Intel-float into VAX f-float
*               Integer function in Lynx
*
*************************************************************************
*************************************************************************/

int Float32_AXP(float *intel)
{
        char    c;

        intern.ff = *intel;

/* Factor 4 */
        if (((intern.cc[3] & 127) == 0) && ((intern.cc[2] & 128) == 0))
                intern.ff = 0.0;
        else if ((intern.cc[3] & 127) == 127)
        {
          if (intern.ff > 0.0) intern.ii = 0x7fffffff;
          else intern.ii = 0xffffffff;
        }
        else intern.ff *= 4;

/* Byte order */
        c = intern.cc[0];
        intern.cc[0] = intern.cc[2];
        intern.cc[2] = c;
        c = intern.cc[1];
        intern.cc[1] = intern.cc[3];
        intern.cc[3] = c;

        return intern.ii;       /* Return with answer */
}
/*************************************************************************
*
*       AXP_Float32(&axp)
*
* Example:	Value = VaxIntel_Float(&Mess->VaxValue);
* Description:  Convert from VAX f-float into Intel-float
*               Floatfunction in Lynx
*
*************************************************************************
*************************************************************************/

float AXP_Float32(int *axp)
{
        char *p;                /* Pointer to ieee data-area */

        p = (char *) axp;       /* Pointer to buffer */
        intern.cc[2] = *p++;    /* Get data in the right order */
        intern.cc[3] = *p++;
        intern.cc[0] = *p++;
        intern.cc[1] = *p;

        return intern.ff / 4;
}


/* .  */

#if 0
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
#endif

#if 0
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

static pwr_tBoolean
encode_vms_sfloat (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{
  float f;

  for (; count > 0 && *size >= sizeof(float); count--) {
    f = AXP_Float32( (int *)sp);

    memcpy( tp, &f, sizeof(f));

    tp += sizeof(float);
    sp += sizeof(float);
    *size -= sizeof(float);
  }

  return TRUE;
}    

#if 0
static pwr_tBoolean
encode_intel_sfloat (
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

static pwr_tBoolean
decode_vms_sfloat (
  int	count,
  int	asize,
  char	*tp,
  char	*sp,
  int	*size
)
{
  int vf;

  for (; count > 0 && *size >= sizeof(float); count--) {
    vf = Float32_AXP( (float *)sp);

    memcpy( tp, &vf, sizeof(vf));
    tp += sizeof(float);
    sp += sizeof(float);
    *size -= sizeof(float);
  }

  return TRUE;
}    

#if 0
static pwr_tBoolean
decode_intel_sfloat(
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

#if 0
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

#if 0
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

/* Convert a short integer.

  NOTA BENE   A short occupies 4 bytes in the Proview rtdb
              and thus this routine is not currently used.  */

#if 0
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

/* Convert a tiny integer (8 bits).

  NOTA BENE   A tiny occupies 4 bytes in the Proview rtdb
              and thus this routine is not currently used.  */

#if 0
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

#if 0
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
#endif

static pwr_tBoolean (*decode_vms_to_linx86[pwr_eTix_])() = {
  decode_null,		/* pwr_eTix__		*/
  decode_copy,		/* pwr_eTix_Boolean	*/
  decode_vms_sfloat,   	/* pwr_eTix_Float32	*/
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
static pwr_tBoolean (*encode_vms_to_linx86[pwr_eTix_])() = {
  decode_null,		/* pwr_eTix__		*/
  decode_copy,		/* pwr_eTix_Boolean	*/
  encode_vms_sfloat,   	/* pwr_eTix_Float32	*/
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


/**
 * @note There is no support for double. 
 *       if we want to implement it we have too look at the OS to decide what 
 *       format to use. Because for backward compatibility is the float format set 
 *       to vaxF for VAX to AXP and not vaxD and vaxG.
 */

pwr_tBoolean
rndc_ConvertData (
  pwr_tStatus		*sts,
  const gdb_sNode	*np,
  gdb_sClass		*cp, 
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
          rndc_ConvertData(sts, np, lcp, tp, sp, size, op, (offset - ap->offs) % (ap->size / ap->elem), ap->offs - base + aoffs + offs);
	  aoffs += ap->size / ap->elem;  
	}
      } else {
        if(!encode_vms_to_linx86[pwr_Tix(ap->type)](ap->elem, ap->size, (char *)tp + (ap->offs - base + offs),
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
          rndc_ConvertData(sts, np, lcp, tp, sp, size, op, (offset - ap->offs) % (ap->size / ap->elem), ap->offs - base + aoffs + offs);
	  aoffs += ap->size / ap->elem;  
	}
      } else {
        if(!decode_vms_to_linx86[pwr_Tix(ap->type)](ap->elem, ap->size, (char *)tp + (ap->offs - base + offs),
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

