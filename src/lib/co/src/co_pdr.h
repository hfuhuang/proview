#ifndef co_pdr_h
#define co_pdr_h

#if defined OS_VMS
#include <types.h>
#elif defined OS_ELN
#include <rpc/rpc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* co_pdr.h -- pdr routines

   PROVIEW/R
   Copyright (C) 1997 by Comator Process AB.

   This module contains the PDR-routines for 
   PROVIEW's data types */


#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef co_cdh_h
# include "co_cdh.h"
#endif

#ifndef co_platform_h
# include "co_platform.h"
#endif



typedef int pdr_tEnum;


/**
 * Pdr operations.  PDR_ENCODE causes the type to be encoded into the
 * stream.  PDR_DECODE causes the type to be extracted from the stream.
 * PDR_FREE can be used to release the space allocated by an PDR_DECODE
 * request.
 */

enum pdr_op {
    PDR_ENCODE=0,
    PDR_DECODE=1,
    PDR_FREE=2
};




typedef struct PDR {
    enum pdr_op	x_op;		/**< operation; fast additional param */
    co_mFormat  trg;
    co_mFormat  src;    
    caddr_t 	x_public;	/**< users' data */
    caddr_t     x_private;	/**< pointer to private data */
    caddr_t 	x_base;		/**< private used for position info */
    int		x_handy;	/**< extra private word */
} PDR;


/**
 * A pdrproc_t exists for each data type which is to be encoded or decoded.
 *
 * The second argument to the pdrproc_t is a pointer to an opaque pointer.
 * The opaque pointer generally points to a structure of the data type
 * to be decoded.  If this pointer is 0, then the type routines should
 * allocate dynamic storage of the appropriate size and return it.
 * pwr_tBoolean	(*pdrproc_t)(PDR *, caddr_t *);
 */
typedef	pwr_tBoolean (*pdrproc_t)(PDR*, void*, ...);


/** PDR using memory buffers 
 * Memory buffer is currently the only supported stream
 */
void pdrmem_create(PDR *pdrs, 
                   void *addr, 
                   u_int size, 
                   enum pdr_op op,
                   co_mFormat src,
                   co_mFormat trg);	




pwr_tBoolean pdr_enum(PDR *pdrs, pdr_tEnum *ep);
pwr_tBoolean pdr_float(PDR *pdrs, float *fp);
pwr_tBoolean pdr_vector(PDR *pdrs, char *arrp, u_int size, u_int elsize, pdrproc_t elproc);


#define pdr_pwr_tChar    pdr_char
#define pdr_pwr_tFloat32 pdr_float


/**
 * When encoded, pdr_mFormat should always be stored as big endian.
 * When decoded, the source must be big endian.
 *
 */

pwr_tBoolean
pdr_cdh_sFamily(PDR *pdrs, cdh_sFamily *objp);

pwr_tBoolean
pdr_cdh_sObjName(PDR *pdrs, cdh_sObjName *objp);

pwr_tBoolean
pdr_co_mFormat(PDR *pdrs, co_mFormat *objp);

#define pdr_pwr_eType(pdrs, objp) pdr_enum(pdrs, (pdr_tEnum *)objp)

pwr_tBoolean
pdr_pwr_tBit(PDR *pdrs, pwr_tBit *objp);

pwr_tBoolean
pdr_pwr_tBitMask(PDR *pdrs, pwr_tBitMask *objp);

pwr_tBoolean
pdr_pwr_tBoolean(PDR *pdrs, pwr_tBoolean *objp);

pwr_tBoolean
pdr_pwr_tGeneration(PDR *pdrs, pwr_tGeneration *objp);

pwr_tBoolean
pdr_pwr_tChar(PDR *pdrs, pwr_tChar *objp);

pwr_tBoolean
pdr_pwr_tInt8(PDR *pdrs, pwr_tInt8 *objp);

pwr_tBoolean
pdr_pwr_tInt16(PDR *pdrs, pwr_tInt16 *objp);

pwr_tBoolean
pdr_pwr_tInt32(PDR *pdrs, pwr_tInt32 *objp);

pwr_tBoolean
pdr_pwr_tInt64(PDR *pdrs, pwr_tInt64 *objp);

pwr_tBoolean
pdr_pwr_tUInt8(PDR *pdrs, pwr_tUInt8 *objp);

pwr_tBoolean
pdr_pwr_tUInt16(PDR *pdrs, pwr_tUInt16 *objp);

pwr_tBoolean
pdr_pwr_tUInt32(PDR *pdrs, pwr_tUInt32 *objp);

pwr_tBoolean
pdr_pwr_tUInt64(PDR *pdrs, pwr_tUInt64 *objp);

pwr_tBoolean
pdr_pwr_tStatus(PDR *pdrs, pwr_tStatus *objp);

#define pdr_pwr_tAix(pdrs, objp) pdr_pwr_tUInt32(pdrs, (pwr_tUInt32 *)objp)

pwr_tBoolean
pdr_pwr_tVolumeId(PDR *pdrs, pwr_tVolumeId *objp);
#define pdr_pwr_tVid(pdrs, objp) pdr_pwr_tVolumeId(pdrs, (pwr_tVolumeId *)objp)

pwr_tBoolean
pdr_pwr_tObjectIx(PDR *pdrs, pwr_tObjectIx *objp);
#define pdr_pwr_tOix(pdrs, objp) pdr_pwr_tObjectIx(pdrs, (pwr_tObjectIx *)objp)

pwr_tBoolean
pdr_pwr_tObjid(PDR *pdrs, pwr_tObjid *objp);
#define pdr_pwr_tOid(pdrs, objp) pdr_pwr_tObjid(pdrs, (pwr_tObjid *)objp)

pwr_tBoolean
pdr_pwr_tClassId(PDR *pdrs, pwr_tClassId *objp);
#define pdr_pwr_tCid(pdrs, objp) pdr_pwr_tClassId(pdrs, (pwr_tClassId *)objp)

pwr_tBoolean
pdr_pwr_tTypeId(PDR *pdrs, pwr_tTypeId *objp);
#define pdr_pwr_tTid(pdrs, objp) pdr_pwr_tTypeId(pdrs, (pwr_tTypeId *)objp)

pwr_tBoolean
pdr_pwr_tVersion(PDR *pdrs, pwr_tVersion *objp);

pwr_tBoolean
pdr_pwr_tPwrVersion(PDR *pdrs, pwr_tPwrVersion *objp);

pwr_tBoolean
pdr_pwr_tProjVersion(PDR *pdrs, pwr_tProjVersion *objp);

pwr_tBoolean
pdr_pwr_tUserId(PDR *pdrs, pwr_tUserId *objp);

pwr_tBoolean
pdr_pwr_tDbId(PDR *pdrs, pwr_tDbId *objp);

pwr_tBoolean
pdr_pwr_tNodeId(PDR *pdrs, pwr_tNodeId *objp);

pwr_tBoolean
pdr_pwr_tNodeIndex(PDR *pdrs, pwr_tNodeIndex *objp);

pwr_tBoolean
pdr_pwr_tRefId(PDR *pdrs, pwr_tRefId *objp);

pwr_tBoolean
pdr_pwr_tDlid(PDR *pdrs, pwr_tDlid *objp);

pwr_tBoolean
pdr_pwr_tSubid(PDR *pdrs, pwr_tSubid *objp);

pwr_tBoolean
pdr_pwr_tTime(PDR *pdrs, pwr_tTime *objp);

pwr_tBoolean
pdr_pwr_tDeltaTime(PDR *pdrs, pwr_tDeltaTime *objp);

pwr_tBoolean
pdr_pwr_tObjName(PDR *pdrs, pwr_tObjName *objp);

pwr_tBoolean
pdr_pwr_tPgmName(PDR *pdrs, pwr_tPgmName *objp);

pwr_tBoolean
pdr_pwr_tXRef(PDR *pdrs, pwr_tXRef *objp);

pwr_tBoolean
pdr_pwr_tGraphName(PDR *pdrs, pwr_tGraphName *objp);

pwr_tBoolean
pdr_pwr_tStructName(PDR *pdrs, pwr_tStructName *objp);

pwr_tBoolean
pdr_pwr_tAttrName(PDR *pdrs, pwr_tAttrName *objp);

pwr_tBoolean
pdr_pwr_tPathName(PDR *pdrs, pwr_tPathName *objp);

pwr_tBoolean
pdr_pwr_tFullName(PDR *pdrs, pwr_tFullName *objp);

pwr_tBoolean
pdr_pwr_tString256(PDR *pdrs, pwr_tString256 *objp);

pwr_tBoolean
pdr_pwr_tString132(PDR *pdrs, pwr_tString132 *objp);

pwr_tBoolean
pdr_pwr_tString80(PDR *pdrs, pwr_tString80 *objp);

pwr_tBoolean
pdr_pwr_tString40(PDR *pdrs, pwr_tString40 *objp);

pwr_tBoolean
pdr_pwr_tString32(PDR *pdrs, pwr_tString32 *objp);

pwr_tBoolean
pdr_pwr_tString16(PDR *pdrs, pwr_tString16 *objp);

pwr_tBoolean
pdr_pwr_tString8(PDR *pdrs, pwr_tString8 *objp);

#ifdef __cplusplus
pwr_tBoolean
pdr_pwr_tString(PDR *pdrs, char *objp);
#else
pwr_tBoolean
pdr_pwr_tString(PDR *pdrs, pwr_tString *objp);
#endif

pwr_tBoolean
pdr_pwr_mAttrRef(PDR *pdrs, pwr_mAttrRef *objp);

pwr_tBoolean 
pdr_pwr_sAttrRef(PDR *pdrs, pwr_sAttrRef *objp);

#ifdef __cplusplus
}
#endif

#endif
