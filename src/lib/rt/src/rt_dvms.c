/* rt_dvms.c -- Decode VMS-data

   PROVIEW/R
   Copyright (C) 1997 by Comator Process AB.

   Functions to convert data loaded from file
   in VAX format to big endian format.  */



#include <stdio.h>
#include <string.h>

#include "pwr.h"
#include "pwr_class.h"
#include "co_float.h"
#include "rt_dvms.h"
#include "rt_errh.h"
#include "rt_gdb.h"
#include "rt_mvol.h"







static pwr_tBoolean
dvms_bool (
  char			*p,
  gdb_sAttribute	*ap,
  co_eBO                 bo
)
{
  int			i;

  p += ap->offs;

  for (i = ap->elem; i > 0; i--) {
    ENDIAN_SWAP_BOOLP(p);
    p += sizeof(int);
  }

  return TRUE;
}
static pwr_tBoolean
dvms_error (
  char			*p,
  gdb_sAttribute	*ap,
  co_eBO               bo
)
{
    errh_Error("dvms_error, type not supported, pwr_tFloat64");
    

    return FALSE;
}

static pwr_tBoolean
dvms_ieee2vaxf (
  char			*p,
  gdb_sAttribute	*ap,
  co_eBO               bo
  )
{
    int			i;

    p += ap->offs;

    for (i = ap->elem; i > 0; i--) {
        co_ieee2vaxf(bo, co_dHostByteOrder, p, p);
        p += sizeof(float);
    }

    return TRUE;
    
}

static pwr_tBoolean
dvms_vaxf2ieee (
  char			*p,
  gdb_sAttribute	*ap,
  co_eBO               bo
  )
{
    int			i;

    p += ap->offs;

    for (i = ap->elem; i > 0; i--) {
        co_vaxf2ieee(bo, co_dHostByteOrder, p, p);
        p += sizeof(float);
    }

    return TRUE;
    
}



static pwr_tBoolean
dvms_int (
  char			*p,
  gdb_sAttribute	*ap,
  co_eBO               bo
)
{
  int			i;

  p += ap->offs;

  for (i = ap->elem; i > 0; i--) {
    ENDIAN_SWAP_INTP(p);
    p += sizeof(int);
  }

  return TRUE;
}

static pwr_tBoolean
dvms_2_int (
  char			*p,
  gdb_sAttribute	*ap,
  co_eBO               bo
)
{
  int			i;

  p += ap->offs;

  for (i = ap->elem * 2; i > 0; i--) {
    ENDIAN_SWAP_INTP(p);
    p += sizeof(int);
  }

  return TRUE;
}

static pwr_tBoolean
dvms_short (
  char			*p,
  gdb_sAttribute	*ap,
  co_eBO               bo
)
{
  int			i;

  p += ap->offs;

  for (i = ap->elem; i > 0; i--) {
    ENDIAN_SWAP_SHORTP(p);
    p += sizeof(short);
  }

  return TRUE;
}

static pwr_tBoolean
dvms_aref (
  char			*p,
  gdb_sAttribute	*ap,
  co_eBO               bo
)
{
  int			i;

  p += ap->offs;

  for (i = ap->elem; i > 0; i--) {
    ENDIAN_SWAP_INTP(p);
    p += sizeof(int);
    ENDIAN_SWAP_INTP(p);
    p += sizeof(int);

    ENDIAN_SWAP_INTP(p);
    p += sizeof(int);

    ENDIAN_SWAP_INTP(p);
    p += sizeof(int);

    ENDIAN_SWAP_INTP(p);
    p += sizeof(int);

    ENDIAN_SWAP_INTP(p);
    p += sizeof(int);
  }

  return TRUE;
}





static dvmsFctn dvmsSwap[pwr_eTix_] = {
  NULL,			/* pwr_eTix__		*/
  dvms_bool,		/* pwr_eTix_Boolean	*/
  dvms_error,		/* pwr_eTix_Float32	*/
  dvms_error,		/* pwr_eTix_Float64	*/
  NULL,			/* pwr_eTix_Char	*/
  NULL,			/* pwr_eTix_Int8	*/
  dvms_short,		/* pwr_eTix_Int16	*/
  dvms_int,		/* pwr_eTix_Int32	*/
  NULL,			/* pwr_eTix_UInt8	*/
  dvms_short,		/* pwr_eTix_UInt16	*/
  dvms_int,		/* pwr_eTix_UInt32	*/
  dvms_2_int,		/* pwr_eTix_Objid	*/
  NULL,			/* pwr_eTix_Buffer	*/
  NULL,			/* pwr_eTix_String	*/
  dvms_int,		/* pwr_eTix_Enum	*/
  NULL,			/* pwr_eTix_Struct	*/
  dvms_int,		/* pwr_eTix_Mask	*/
  NULL,			/* pwr_eTix_Array	*/
  dvms_2_int,		/* pwr_eTix_Time	*/
  NULL,			/* pwr_eTix_Text	*/
  dvms_aref,		/* pwr_eTix_AttrRef	*/
  dvms_2_int,		/* pwr_eTix_UInt64	*/
  dvms_2_int,		/* pwr_eTix_Int64	*/
  dvms_int,		/* pwr_eTix_ClassId	*/
  dvms_int,		/* pwr_eTix_TypeId	*/
  dvms_int,		/* pwr_eTix_VolumeId	*/
  dvms_int,		/* pwr_eTix_ObjectIx	*/
  dvms_2_int,		/* pwr_eTix_RefId	*/
};


static dvmsFctn dvmsNull[pwr_eTix_] = {
  NULL,			/* pwr_eTix__		*/
  NULL,			/* pwr_eTix_Boolean	*/
  dvms_error,           /* pwr_eTix_Float32	*/
  dvms_error,           /* pwr_eTix_Float64	*/
  NULL,			/* pwr_eTix_Char	*/
  NULL,			/* pwr_eTix_Int8	*/
  NULL,			/* pwr_eTix_Int16	*/
  NULL,			/* pwr_eTix_Int32	*/
  NULL,			/* pwr_eTix_UInt8	*/
  NULL,			/* pwr_eTix_UInt16	*/
  NULL,			/* pwr_eTix_UInt32	*/
  NULL,	       		/* pwr_eTix_Objid	*/
  NULL,			/* pwr_eTix_Buffer	*/
  NULL,			/* pwr_eTix_String	*/
  NULL,			/* pwr_eTix_Enum	*/
  NULL,			/* pwr_eTix_Struct	*/
  NULL,			/* pwr_eTix_Mask	*/
  NULL,			/* pwr_eTix_Array	*/
  NULL,			/* pwr_eTix_Time	*/
  NULL,			/* pwr_eTix_Text	*/
  NULL,			/* pwr_eTix_AttrRef	*/
  NULL,			/* pwr_eTix_UInt64	*/
  NULL,			/* pwr_eTix_Int64	*/
  NULL,			/* pwr_eTix_ClassId	*/
  NULL,			/* pwr_eTix_TypeId	*/
  NULL,			/* pwr_eTix_VolumeId	*/
  NULL,			/* pwr_eTix_ObjectIx	*/
  NULL,			/* pwr_eTix_RefId	*/
};



dvmsFctn* dvms_GetFctns(const co_mFormat* fmp)
{
    dvmsFctn *fctns;
    
    if (fmp->b.bo == co_dHostByteOrder) {
        if (fmp->b.ft == co_dHostFloatType)
            return NULL;

        fctns = dvmsNull;
    }
    else
        fctns = dvmsSwap;
    
    if (fmp->b.ft == co_dHostFloatType) {
        fctns[pwr_eTix_Float32] = NULL;
        fctns[pwr_eTix_Float64] = NULL;
    }
    else if (co_dHostFloatType == co_eFT_ieeeS) {
        fctns[pwr_eTix_Float32] = dvms_vaxf2ieee;
        fctns[pwr_eTix_Float64] = dvms_error;
    }
    else {
        fctns[pwr_eTix_Float32] = dvms_ieee2vaxf;
        fctns[pwr_eTix_Float64] = dvms_error;
    }
    
    return fctns;
}
