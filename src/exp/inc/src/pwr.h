#ifndef pwr_h
#define pwr_h
#ifndef PWR_LOADED
#  define PWR_LOADED 1
#endif

/* pwr.h -- basic definitions for PROVIEW/R

   PROVIEW/R
   Copyright (C) 1989-1996 by Comator Process AB.

   <Description>.  */

#if defined(OS_ELN)
# include stdio
# include time
#elif defined(OS_VMS)
# include <stdio.h>
# include <time.h>
# include <lib$routines.h>
#elif defined(OS_LYNX) || defined(OS_LINUX)
# include <stdio.h>
# include <stdlib.h>  /* EXIT_FAILURE */
# include <time.h>
#endif

/*! \file pwr.h
    \brief Basic type definitions.
   This include file contains the Proview basic type definitions.
*/
/*! \addtogroup Pwr */
/*@{*/

#ifdef VAXC
# define pwr_dExport globaldef  
# define pwr_dImport globalref
#else
# define pwr_dExport  
# define pwr_dImport extern
#endif

/* Environment variable which MUST be defined.
 * eg. export PWR_BUS_ID="300"    
 */
# define pwr_dEnvBusId "PWR_BUS_ID" 

#ifndef pwr_dPacked
# if defined(__GNUC__) || defined(__GNUG__)
#   define pwr_dPacked __attribute__ ((packed))
# else
#   define pwr_dPacked
# endif
#endif

#if defined OS_VMS && defined __cplusplus
  namespace std {};
#endif 

#define pwr_dLittleEndian 1
#define pwr_dBigEndian	  2

#if defined (__powerpc__)
# define pwr_dHost_byteOrder pwr_dBigEndian
# if 0
#   define pwr_Bit(b) (1<<(31-b))
#   define pwr_SetByte(byte, val) (val<<((3-byte)<<3))
# endif
#else
# define pwr_dHost_byteOrder pwr_dLittleEndian
#endif

#define pwr_Bit(b) (1<<b)
#define pwr_SetByte(byte, val) (val<<(byte<<3))

/* PROVIEW/R types  */

#define pwr_cSizObjName		31
#define pwr_cSizPgmName		31
#define pwr_cSizStructName	31
#define pwr_cSizGraphName	15
#define pwr_cSizXRef		31
#define pwr_cSizAttrName	31
#define pwr_cSizPathName	63
#define pwr_cSizFullName	79

typedef void            *pwr_tAddress;		//!< Generic pointer type.
typedef unsigned int    pwr_tBit;		//!< Bit type.
typedef unsigned int    pwr_tBitMask;		//!< Bitmask type.
/*_*
  @aref boolean Boolean
*/
typedef unsigned int    pwr_tBoolean;		//!< Boolean type.
/*_*
  @aref float32 Float32
*/
typedef float           pwr_tFloat32;		//!< 32-bit float.
/*_*
  @aref float64 Float64
*/
typedef double          pwr_tFloat64;		//!< 64-big float.
/*_*
  @aref char Char
*/
typedef char            pwr_tChar;		//!< Character type.
/*_*
  @aref string String
*/
typedef char            pwr_tString[];		//!< String type.
/*_*
  @aref text Text
*/
typedef char            pwr_tText[];		//!< Text type.
/*_*
  @aref int8 Int8
*/
typedef char            pwr_tInt8;		//!< 8-bit integer type.
/*_*
  @aref int16 Int16
*/
typedef short           pwr_tInt16;		//!< 16-bit integer type.
/*_*
  @aref int32 Int32
*/
typedef int             pwr_tInt32;		//!< 32-bit integer type.


//! 64-bit integer type.
/*_*
  @aref float64 Float64
*/
typedef struct {
  unsigned int low;
  int          high;
} pwr_tInt64;

//! 64-bit unsigned integer type.
/*_*
  @aref int64 Int64
*/
typedef struct {
  unsigned int low;
  unsigned int high;
} pwr_tUInt64;

/*_*
  @aref uint8 UInt8
*/
typedef unsigned char   pwr_tUInt8;		//!< 8-bit unsigned integer type.
/*_*
  @aref uint16 UInt16
*/
typedef unsigned short  pwr_tUInt16;		//!< 16-bit unsigned integer type.
/*_*
  @aref uint32 UInt32
*/
typedef unsigned int    pwr_tUInt32;		//!< 32-bit unsigned integer type.
/*_*
  @aref volumeid VolumeId
*/
typedef unsigned int    pwr_tVid;		//!< Volume identity type.
typedef pwr_tVid        pwr_tVolumeId;		//!< Volume identity type.
typedef unsigned int    pwr_tAix;		//!< Attribute index type.
/*_*
  @aref objectix ObjectIx
*/
typedef unsigned int    pwr_tOix;		//!< Object index type.
typedef pwr_tOix        pwr_tObjectIx;		//!< Object index type.
/*_*
  @aref mask Mask
*/
typedef unsigned int	pwr_tMask;		//!< Mask type.
/*_*
  @aref enum Enum
*/
typedef unsigned int	pwr_tEnum;		//!< Enumeration type.

//! Object identity type.
/*_*
  @aref objid Objid
*/
typedef struct {
  pwr_tOix    oix;
  pwr_tVid    vid;
} pwr_tOid;

typedef pwr_tOid        pwr_tObjid;	//!< Object identity type.
typedef pwr_tOid        pwr_tObjDId;
/*_*
  @aref classid ClassId
*/
typedef unsigned int    pwr_tCid;	//!< Class identity type.
typedef pwr_tCid        pwr_tClassId;	//!< Class identity type.
/*_*
  @aref typeid TypeId
*/
typedef unsigned int    pwr_tTid;	//!< Type identity type.
typedef pwr_tTid        pwr_tTypeId;	//!< Type identity type.

/*_*
  @aref status Status
*/
typedef int             pwr_tStatus;		//!< Status type.
/*_*
  @aref netstatus NetStatus
*/
typedef int             pwr_tNetStatus;		//!< Network status type.

typedef unsigned int    pwr_tGeneration;
typedef unsigned int    pwr_tBid;
typedef unsigned int    pwr_tVersion;
typedef unsigned int    pwr_tPwrVersion;
typedef unsigned int    pwr_tProjVersion;
typedef unsigned int    pwr_tUserId;
typedef unsigned int    pwr_tDbId;
typedef pwr_tVolumeId	pwr_tNid;	//!< Node identity type.
typedef pwr_tNid        pwr_tNodeId;	//!< Node identity type.
typedef pwr_tNid        pwr_tNodeIndex;	//!< Node index type.

/*_*
  @aref refid RefId
*/
typedef struct {
  pwr_tUInt32    rix;
  pwr_tNid       nid;
} pwr_tRid;				//!< Reference identity type.

typedef pwr_tRid        pwr_tRefId;	//!< Reference identity type.
typedef pwr_tRid        pwr_tDlid;	//!< Direct link identity type.
typedef pwr_tRid        pwr_tSubid;	//!< Subscription identity type.


/*_*
  @aref bix Bix
*/
//! Body index enumeration.
typedef enum {
  pwr_eBix__	    = 0,
  pwr_eBix_sys	    = 1,
  pwr_eBix_rt	    = 1,
  pwr_eBix_dev	    = 2,
  pwr_eBix_template = 7,
  pwr_eBix_
} pwr_eBix;

//! Vax time.
typedef struct {
  int low;
  int high;
} pwr_tVaxTime;

//! Proview version type
typedef union {
  pwr_tPwrVersion i;
  //! Word representation
  struct {
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)

    pwr_tChar	Char;
    pwr_tUInt8	Major;
    pwr_tUInt8	Minor;
    pwr_tUInt8	Update;

#elif (pwr_dHost_byteOrder == pwr_dBigEndian)

    pwr_tUInt8	Update;
    pwr_tUInt8	Minor;
    pwr_tUInt8	Major;
    pwr_tChar	Char;

#endif
  } s;
} pwr_uPwrVersion;

#if (defined OS_VMS || defined OS_ELN) && (! defined _TIMESPEC_T_ && !defined TIMERS_INCLUDED)
# define _TIMESPEC_T_
  typedef struct timespec {
    unsigned long tv_sec;
    long	  tv_nsec;
  } timespec_t;
#endif

/*_*
  @aref time Time
*/
typedef struct timespec pwr_tTime;	//!< Abolute time type.


//! Delta time type.
/*_*
  @aref deltatime DeltaTime
*/
typedef struct {
  int tv_sec;		//!< Seconds.
  int tv_nsec;		//!< Nano seconds.
} pwr_tDeltaTime;

/*_*
  @aref objname ObjName
*/
typedef char		pwr_tObjName	[pwr_cSizObjName + 1];	//!< Object name type.
/*_*
  @aref pgmname PgmName
*/
typedef char		pwr_tPgmName	[pwr_cSizPgmName + 1];	//!< PgmName type.

/*_*
  @aref xref XRef
*/
typedef char		pwr_tXRef	[pwr_cSizXRef + 1];	//!< XRef type.

/*_*
  @aref graphname GraphName
*/
typedef char		pwr_tGraphName	[pwr_cSizGraphName + 1]; //!< GraphName type.
typedef char		pwr_tStructName	[pwr_cSizStructName + 1]; //!< StructName type.
typedef char		pwr_tAttrName	[pwr_cSizAttrName + 1];	//!< AttrName type.
typedef char		pwr_tPathName	[pwr_cSizPathName + 1];	//!< PathName type.
typedef char		pwr_tFullName	[pwr_cSizFullName + 1];	//!< FullName type.
/*_*
  @aref string256 String256
*/
typedef char		pwr_tString256	[256];			//!< 256 byte string type.
/*_*
  @aref string132 String132
*/
typedef char		pwr_tString132	[132];			//!< 132 byte string type.
/*_*
  @aref string80 String80
*/
typedef char		pwr_tString80	[80];			//!< 80 byte string type.
/*_*
  @aref string40 String40
*/
typedef char		pwr_tString40	[40];			//!< 40 byte string type.
/*_*
  @aref string32 String32
*/
typedef char		pwr_tString32	[32];			//!< 32 byte string type.
/*_*
  @aref string16 String16
*/
typedef char		pwr_tString16	[16];			//!< 16 byte string type.
/*_*
  @aref string8 String8
*/
typedef char		pwr_tString8	[8];			//!< 8 byte string type.
/*_*
  @aref string1 String1
*/
typedef char		pwr_tString1	[1];			//!< 1 byte string type.
/*_*
  @aref text1024 Text1024
*/
typedef char		pwr_tText1024	[1024];			//!< 1024 byte text type.
/*_*
  @aref url URL
*/
typedef char		pwr_tURL	[160];			//!< URL type.


/*_*
  @aref attrrefflag AttrRefFlag
*/
//! Attribute reference flags type.
typedef union {
  pwr_tBitMask m;
  //! Bitmask representation.
  struct {
#if (pwr_dHost_byteOrder == pwr_dLittleEndian)

    pwr_tBit  Indirect	: 1;

    pwr_tBit  fill	: 31;

#elif (pwr_dHost_byteOrder == pwr_dBigEndian)

    pwr_tBit  fill	: 31;

    pwr_tBit  Indirect	: 1;

#endif
  } b;

#define pwr_mAttrRef_Indirect	pwr_Bit(0)

} pwr_mAttrRef;

//! Attribute reference.
/*_*
  @aref attrref AttrRef
*/
typedef struct {
  pwr_tOid	    Objid;	//!< Object identity.
  pwr_tCid          Body;	//!< Class identity of body.
  pwr_tUInt32       Offset;	//!< Offset in body.
  pwr_tUInt32       Size;	//!< Attribute size.
  pwr_mAttrRef      Flags;	//!< Attribute flags.
} pwr_sAttrRef;

/*_*
  @aref constants Constants
*/
//! Zero attribute reference constant.
#ifdef OS_ELN
static const pwr_sAttrRef   pwr_cNAttrRef   = {0, 0, 0, 0, 0};
#else
static const pwr_sAttrRef   pwr_cNAttrRef   = {{0, 0}, 0, 0, 0, {0}};
#endif
static const pwr_tOid       pwr_cNOid       = {0, 0};	//!< Zero object identity constant.
static const pwr_tObjid	    pwr_cNObjid	    = {0, 0};	//!< Zero object identity constant.
static const pwr_tDlid	    pwr_cNRefId	    = {0, 0};	//!< Zero reference identity constant.
#define			    pwr_cNSubid	    pwr_cNRefId //!< Zero subscription identity constant.
#define			    pwr_cNDlid	    pwr_cNRefId	//!< Zero direct link identity constant.
static const pwr_tOix       pwr_cNOix       = 0;	//!< Zero object index constant.
static const pwr_tObjectIx  pwr_cNObjectIx  = 0;	//!< Zero object index constant.
static const pwr_tClassId   pwr_cNClassId   = 0;	//!< Zero class identity constant.
static const pwr_tTypeId    pwr_cNTypeId    = 0;	//!< Zero type identity constant.
static const pwr_tVolumeId  pwr_cNVolumeId  = 0;	//!< Zero volume identity constant.
static const pwr_tNodeId    pwr_cNNodeId    = 0;	//!< Zero node identity constant.
static const pwr_tCid       pwr_cNCid       = 0;	//!< Zero class identity constant.
static const pwr_tTid       pwr_cNTid       = 0;	//!< Zero type identity constant.
static const pwr_tVid       pwr_cNVid       = 0;	//!< Zero volume identity constant.
static const pwr_tNid       pwr_cNNid       = 0;	//!< Zero node identity constant.
static const pwr_tStatus    pwr_cNStatus    = 0;	//!< Zero status constant.

/* Gereral macro definitions  */

#ifndef MIN
//! Return the smallest of two values
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
//! Return the largest of two values
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef ODD
//! Check if value is odd
#define ODD(a)	(((int)(a) & 1) != 0)
#endif

#ifndef EVEN
//! Check if value is even
#define EVEN(a)	(((int)(a) & 1) == 0)
#endif

/* General definitions  */

#ifndef EQUAL
#define EQUAL	0
#endif

#ifndef ON
#define ON	1
#endif

#ifndef OFF
#define OFF	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef YES
#define YES	1
#endif

#ifndef NO
#define NO	0
#endif

#ifndef pwr_Offset
#define pwr_Offset(base, field) ((unsigned int)&((base)->field)-(unsigned int)(base))
#endif

#define pwr_Field(a, n) a n;
#define pwr_Bits(a, n) pwr_tBit a : n;
	
#if pwr_dHost_byteOrder == pwr_dLittleEndian
# define pwr_32Bits(\
    a00, a01, a02, a03, a04, a05, a06, a07, \
    a08, a09, a10, a11, a12, a13, a14, a15, \
    a16, a17, a18, a19, a20, a21, a22, a23, \
    a24, a25, a26, a27, a28, a29, a30, a31) \
  struct {\
    a00 a01 a02 a03 a04 a05 a06 a07 \
    a08 a09 a10 a11 a12 a13 a14 a15 \
    a16 a17 a18 a19 a20 a21 a22 a23 \
    a24 a25 a26 a27 a28 a29 a30 a31 \
  }

# define pwr_Endian_32(\
    a00, a01, a02, a03, a04, a05, a06, a07, \
    a08, a09, a10, a11, a12, a13, a14, a15, \
    a16, a17, a18, a19, a20, a21, a22, a23, \
    a24, a25, a26, a27, a28, a29, a30, a31) \
\
    a00 a01 a02 a03 a04 a05 a06 a07 \
    a08 a09 a10 a11 a12 a13 a14 a15 \
    a16 a17 a18 a19 a20 a21 a22 a23 \
    a24 a25 a26 a27 a28 a29 a30 a31

# define pwr_Endian_16(\
    a00, a01, a02, a03, a04, a05, a06, a07, \
    a08, a09, a10, a11, a12, a13, a14, a15) \
\
    a00 a01 a02 a03 a04 a05 a06 a07 a08 a09 a10 a11 a12 a13 a14 a15

# define pwr_Endian_8(a00, a01, a02, a03, a04, a05, a06, a07) \
    a00 a01 a02 a03 a04 a05 a06 a07

# define pwr_Endian_4(a00, a01, a02, a03) \
    a00 a01 a02 a03

#elif pwr_dHost_byteOrder == pwr_dBigEndian
# define pwr_32Bits(\
    a00, a01, a02, a03, a04, a05, a06, a07, \
    a08, a09, a10, a11, a12, a13, a14, a15, \
    a16, a17, a18, a19, a20, a21, a22, a23, \
    a24, a25, a26, a27, a28, a29, a30, a31) \
  struct {\
    a31 a30 a29 a28 a27 a26 a25 a24 \
    a23 a22 a21 a20 a19 a18 a17 a16 \
    a15 a14 a13 a12 a11 a10 a09 a08 \
    a07 a06 a05 a04 a03 a02 a01 a00 \
  }

# define pwr_Endian_32(\
    a00, a01, a02, a03, a04, a05, a06, a07, \
    a08, a09, a10, a11, a12, a13, a14, a15, \
    a16, a17, a18, a19, a20, a21, a22, a23, \
    a24, a25, a26, a27, a28, a29, a30, a31) \
\
    a31 a30 a29 a28 a27 a26 a25 a24 \
    a23 a22 a21 a20 a19 a18 a17 a16 \
    a15 a14 a13 a12 a11 a10 a09 a08 \
    a07 a06 a05 a04 a03 a02 a01 a00

# define pwr_Endian_16(\
    a00, a01, a02, a03, a04, a05, a06, a07, \
    a08, a09, a10, a11, a12, a13, a14, a15) \
\
    a15 a14 a13 a12 a11 a10 a09 a08 \
    a07 a06 a05 a04 a03 a02 a01 a00

# define pwr_Endian_8(a00, a01, a02, a03, a04, a05, a06, a07) \
    a07 a06 a05 a04 a03 a02 a01 a00

# define pwr_Endian_4(a00, a01, a02, a03) \
    a03 a02 a01 a00

#endif


#ifndef pwr_dStatus
#define pwr_dStatus(sts, status, ists)\
  pwr_tStatus pwr__sts_;\
  pwr_tStatus *sts = (status == NULL) ?  &pwr__sts_ : status;\
  *sts = ists
#endif

#ifndef pwr_Status
# define pwr_Status(sts, lsts) ((void*)(sts)?((*sts) = (lsts)):(lsts))
#endif

#ifndef pwr_StatusBreak
# define pwr_StatusBreak(a, b) {a = b; break;}
#endif

#ifndef pwr_Return
# define pwr_Return(a, sts, lsts) return (((void*)(sts) ? (*sts) = (lsts) : lsts), a)
#endif

#ifndef pwr_ReturnVoid
# define pwr_ReturnVoid(sts, lsts) (((void*)(sts)?(*sts) = (lsts):lsts), return)
#endif

#ifdef PWR_NDEBUG
# define pwr_Assert(a) ((void) 0)
#elif defined OS_VMS
# define pwr_Assert(a) ((a)?(void)0:(void)(printf("pwr assertion (%s) failed,\n    in file %s, at line %d\n",\
			#a,__FILE__,__LINE__),lib$signal(8)))
#elif defined OS_ELN
# define pwr_Assert(a) ((a)?(void)0:(void)(printf("pwr assertion (%s) failed,\n    in file %s, at line %d\n",\
			"a",__FILE__,__LINE__),lib$signal(8)))
#else
# define pwr_Assert(a) ((a)?(void)0:(void)(printf("pwr assertion (%s) failed,\n    in file %s, at line %d\n",\
			#a,__FILE__,__LINE__),exit(EXIT_FAILURE)))
#endif

/*@}*/

#endif


