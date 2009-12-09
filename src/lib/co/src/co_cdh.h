/** 
 * Proview   $Id: co_cdh.h,v 1.29 2008-10-16 11:11:53 claes Exp $
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
 **/

#ifndef co_cdh_h
#define co_cdh_h

/* co_cdh.h -- class definition handler
   This include file contains definitions and function prototypes
   needed to use CDH.  */

#include <limits.h>

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef pwr_class_h
#include "pwr_class.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/*! \file co_cdh.h
    \brief Class definition handler.
   This include file contains definitions and function prototypes
   needed to use CDH.
*/

/*! \defgroup Cdh_DS Cdh Data Structures
    \ingroup Cdh
*/

/*! \addtogroup Cdh_DS */
/*@{*/

#ifndef co_max
#define co_max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#endif

#ifndef co_min
#define co_min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#endif

#define cdh_cMaxVidGroup	 255
#define cdh_cMaxCix		4095
#define cdh_cMaxBix		   7
#define cdh_cMaxTix		2047
#define cdh_cMaxTyg		   1
#define cdh_cMaxOix	   ULONG_MAX

typedef enum {
  cdh_eVid3_local = 1,
  cdh_eVid3_subid = 128,
  cdh_eVid3_dlid  = 129,
  cdh_eVid3_qid   = 130
} cdh_eVId3;

#define cdh_cIoConnectVolume  (0 + ((pwr_tVolumeId)254 << 24) + (254 << 16) + (254 << 8) + 249)
#define cdh_cRtVolume  (0 + ((pwr_tVolumeId)254 << 24) + (254 << 16) + (254 << 8) + 245)

//! Get volme identity for class identity.
#define cdh_CidToVid(cid) ((cid) >> 16)
//! Get volume identity for type identity.
#define cdh_TidToVid(tid) ((tid) >> 16)
//! Get class identity for class index
#define cdh_cixToCid( Vid, Cix) (0 + ((Vid) << 16) +  ((Cix) << 3))
//! Get class index from class identity.
#define cdh_cidToCix( Cid) (((Cid) >> 3) & 0xfff)
//! Get type identity for type index.
#define cdh_tixToTid( Vid, Tyg, Tix) (0 + ((Vid) << 16) + (1 << 15) + ((Tyg) << 11) +  (Tix))
//! Get object index for class index.
#define cdh_cixToOix( Cix, Bix, Aix) (0 + (1 << 31) + ((Cix) << 18) + ((Bix) << 15) + (Aix))
//! Get object index for type index.
#define cdh_tixToOix( Tyg, Tix) (0 + (1 << 31) + (1 << 30) + ((Tyg) << 26) + ((Tix) << 15))
//! Get object index for body index.
#define cdh_oixToBix( Oix) ((pwr_eBix)(((Oix) >> 15) & 7))
//! Get class index for object index.
#define cdh_oixToCix( Oix) (((Oix) >> 18) & 0xfff)
//! Get attribute index for object index.
#define cdh_oixToAix( Oix) ((Oix) & 0xfff)
//! Get bodyid from classid.
#define cdh_cidToBid( Cid, Bix) ((Cid) & (Bix))
//! Get bix from bodyid.
#define cdh_bidToBix( Bid) ((Bid) & 7)
//! Check if type id is a class id
#define cdh_tidIsCid( Tid) (((Tid) & (1 << 15)) == 0)

//! Smallest value of volume identity for user volumes
#define cdh_cUserVolMin  (0 + ((pwr_tVolumeId)0 << 24) + (1 << 16) + (1 << 8) + 1)
//! Largest value of volume identity for user volumes
#define cdh_cUserVolMax  (0 + ((pwr_tVolumeId)0 << 24) + (254 << 16) + (254 << 8) + 254)
//! Smallest value of volume identity for manufacturer classvolumes
#define cdh_cManufactClassVolMin  (0 + ((pwr_tVolumeId)0 << 24) + (0 << 16) + (250 << 8) + 0)
//! Largest value of volume identity for manufacturer classvolumes
#define cdh_cManufactClassVolMax  (0 + ((pwr_tVolumeId)0 << 24) + (0 << 16) + (254 << 8) + 254)
//! Smallest value of volume identity for user classvolumes
#define cdh_cUserClassVolMin  (0 + ((pwr_tVolumeId)0 << 24) + (0 << 16) + (2 << 8) + 1)
//! Largest value of volume identity for user classvolumes
#define cdh_cUserClassVolMax  (0 + ((pwr_tVolumeId)0 << 24) + (0 << 16) + (249 << 8) + 254)
//! Smallest value of volume identity for system classvolumes
#define cdh_cSystemClassVolMin  (0 + ((pwr_tVolumeId)0 << 24) + (0 << 16) + (0 << 8) + 1)
//! Largest value of volume identity for system classvolumes
#define cdh_cSystemClassVolMax  (0 + ((pwr_tVolumeId)0 << 24) + (0 << 16) + (1 << 8) + 254)

#define cdh_isClassVolumeClass( Cid) ((Cid) == pwr_eClass_ClassVolume || (Cid) == pwr_eClass_DetachedClassVolume)

//! Internal representatin of object identity.
typedef struct {
  unsigned int	oix		: 32;	//! Object index.

#if (pwr_dHost_byteOrder == pwr_dBigEndian)

  unsigned int	vid_3		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_0		: 8;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

  unsigned int	vid_0		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_3		: 8;

#endif
} cdh_mOid;
typedef cdh_mOid cdh_mObjid;	//!< Internal representation of object identity.
    
//! Internal representation of reference identity.
typedef struct {
  unsigned int	rix		: 32;

#if (pwr_dHost_byteOrder == pwr_dBigEndian)

  unsigned int	vid_3		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_0		: 8;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

  unsigned int	vid_0		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_3		: 8;

#endif
} cdh_mRid;
typedef cdh_mRid cdh_mRefId;	//!< Internal representation of reference identity.

//! Internal representations of volume identity.
typedef struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

  unsigned int	vid_3		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_0		: 8;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

  unsigned int	vid_0		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_3		: 8;

#endif
} cdh_mVid;
typedef cdh_mVid cdh_mVolumeId;  //!< Internal representation of volume identity.

//! Internal representation of node identity.
typedef struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

  unsigned int	nid_3		: 8;
  unsigned int	nid_2		: 8;
  unsigned int	nid_1		: 8;
  unsigned int	nid_0		: 8;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

  unsigned int	nid_0		: 8;
  unsigned int	nid_1		: 8;
  unsigned int	nid_2		: 8;
  unsigned int	nid_3		: 8;

#endif
} cdh_mNid;
typedef cdh_mNid cdh_mNodeId;  //!< Internal representation of node identity.

//! Internal representation of $ClassDef object identity.
typedef struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

  unsigned int  must_be_two : 2;
  unsigned int	cix         : 12;
  unsigned int	bix         : 3;
  unsigned int	reserved    : 3;
  unsigned int	aix         : 12;

  unsigned int	vid_3       : 8;
  unsigned int	vid_2       : 8;
  unsigned int	vid_1       : 8;
  unsigned int	vid_0       : 8;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

  unsigned int	aix         : 12;
  unsigned int	reserved    : 3;
  unsigned int	bix         : 3;
  unsigned int	cix         : 12;
  unsigned int  must_be_two : 2;

  unsigned int	vid_0		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_3		: 8;

#endif
} cdh_mClassObjid;

//! Internal represention of class identity.
typedef struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

  unsigned int	vid_1		: 8;
  unsigned int	vid_0		: 8;

  unsigned int  must_be_zero	: 1;
  unsigned int	cix		: 12;
  unsigned int	bix		: 3;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

  unsigned int	bix		: 3;
  unsigned int	cix		: 12;
  unsigned int  must_be_zero	: 1;

  unsigned int	vid_0		: 8;
  unsigned int	vid_1		: 8;

#endif
} cdh_mCid;
typedef cdh_mCid cdh_mClassId;  //!< Internal representation of class identity.

//! Internal representation of $TypeDef object identity.
typedef struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

  unsigned int	must_be_three	: 2;
  unsigned int	tyg		: 4;
  unsigned int	tix		: 11;
  unsigned int	reserved	: 3;
  unsigned int	aix		: 12;

  unsigned int	vid_3		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_0		: 8;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

  unsigned int	aix		: 12;
  unsigned int	reserved	: 3;
  unsigned int	tix		: 11;
  unsigned int	tyg		: 4;
  unsigned int	must_be_three	: 2;

  unsigned int	vid_0		: 8;
  unsigned int	vid_1		: 8;
  unsigned int	vid_2		: 8;
  unsigned int	vid_3		: 8;

#endif
} cdh_mTypeObjid;

//! Internal representation of type identity.
typedef struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

  unsigned int	vid_1		: 8;
  unsigned int	vid_0		: 8;

  unsigned int	must_be_one	: 1;
  unsigned int	tyg		: 4;
  unsigned int	tix		: 11;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

  unsigned int	tix		: 11;
  unsigned int	tyg		: 4;
  unsigned int	must_be_one	: 1;

  unsigned int	vid_0		: 8;
  unsigned int	vid_1		: 8;

#endif
} cdh_mTid;
typedef cdh_mTid cdh_mTypeId;  //!< Internal representation of type identity.

//! Type for representions of object identity.
typedef union {
  pwr_tOid		pwr;	//!< Extern representation.
  cdh_mObjid		o;	//!< Common object representation.
  cdh_mClassObjid	c;	//!< ClassDef object representation.
  cdh_mTypeObjid	t;	//!< TypeDef object representation.
} cdh_uOid;
typedef cdh_uOid cdh_uObjid; //!< Type for representations of object identity. 

//! Type for representations of reference identity.
typedef union {
  pwr_tRid pwr;	//!< Extern representation.
  cdh_mRid r;	//!< Intern representation
} cdh_uRid;
typedef cdh_uRid cdh_uRefId;  //!< Type for representation of reference identity.

//! Type for representation of type identity.
typedef union {
  pwr_tCid pwr;		//!< Extern representation.
  cdh_mCid c;		//!< Class identity representation.
  cdh_mTid t;		//!< Type identity representation.
} cdh_uTid;
typedef cdh_uTid cdh_uTypeId;  //!< Type for representation of type identity.

//! Type for representation of volume identity.
typedef union {
  pwr_tVid pwr;		//!< Extern representation.
  cdh_mVid v;		//!< Intern representation.
} cdh_uVid;
typedef cdh_uVid cdh_uVolumeId;  //!< Type for represenation of volume identity.

//! Type for representation of node identity.
typedef union {
  pwr_tNid pwr;		//!< Extern representation.
  cdh_mNid n;		//!< Intern representation.
} cdh_uNid;
typedef cdh_uNid cdh_uNodeId;	//!< Type for representation of node idenity.

//! Enumeration for identities.
typedef enum {
  cdh_eId__ = 0,
  cdh_eId_objectIx,	//!< Object index.
  cdh_eId_objid,	//!< Object identity.
  cdh_eId_classId,	//!< Class identity.
  cdh_eId_volumeId,	//!< Volume identity.
  cdh_eId_typeId,	//!< Type identity.
  cdh_eId_subid,	//!< Subscription identity.
  cdh_eId_dlid,		//!< Direct link identity.
  cdh_eId_aref,		//!< Attribute reference.
  cdh_eId_
} cdh_eId;

//! Union for identities.
typedef union {
  pwr_tOix     oix;
  pwr_tOid     oid;
  pwr_tCid     cid;
  pwr_tVid     vid;
  pwr_tTid     tid;
  pwr_tSubid   sid;
  pwr_tDlid    did;
  pwr_sAttrRef aref;
} cdh_uId;

//! Pack name
typedef union {
  pwr_tUInt32		key;
  //! Name structure
  struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

    char		last;
    char		first;
    pwr_tUInt8		hash;
    pwr_tUInt8		len;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

    pwr_tUInt8		len;
    pwr_tUInt8		hash;
    char		first;
    char		last;

#endif
  } c;
} cdh_uPackName;

//! Object name struct
typedef struct {
  pwr_tObjName		orig;
  pwr_tObjName		norm;
  cdh_uPackName		pack;
} cdh_sObjName;

//! Family struct.
typedef struct {
  cdh_sObjName		name;
  pwr_tOid		poid;
} cdh_sFamily;

//! Parse name mask
typedef union {
  //! Bitmask representation.
  struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

    pwr_tBit		fill		: 31;
    
    pwr_tBit		ascii_7		: 1;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

    pwr_tBit		ascii_7		: 1;
    
    pwr_tBit		fill		: 31;

#endif
  } b;
  pwr_tBitMask		m;

#define cdh_mParseName__		0
#define cdh_mParseName_ascii_7		pwr_Bit(0)
#define cdh_mParseName_			(~cdh_mParseName__)
} cdh_mParseName;

//! Name string format description.
/*!
  Bitmask that denotes an object or attriubte name string, i.e. which components of the
  name that is included in the string.<br>
  Some common examples are

  <b>cdh_mName_object</b> Object.<br>
  <b>cdh_mName_object | cdh_mName_attribute</b> Object and attribute.<br>
  <b>cdh_mName_pathStrict</b>          Path, object and attribute<br>
  <b>cdh_mName_volumeStrict</b>       Volume, path, object and attribute.

   Let us assume we have an object of class Ai.
   The object has an attribute called FilterAttribute.

-   Object name:	Eobj
-   Object id  : 1234567890
-   Class name :	pwrb:Class-Ai
-   Class id   : 0.2:34
-   Volume name:	Avol
-   Volume id  : 0.123.34.63
-   Parents    : Bobj, Cobj, Dobj
-   Attribute  : FilterAttribute
-   Index      : 2
-   Offset     : 60
-   Size	      : 4
-   Body name  :	pwrb:Class-Ai-RtBody
-   Body id    : 0.2:0.34.1

   The name of this object can be written in different ways.
   The type cdh_mName is used to define the way an object is named.

-   V P O B B A I E S  I   Form  Fallback        String
-   o a b o o t n s e  d   
-   l t j d d t d c p  T 
-   u h e y y r e a a  y 
-   m   c I N i x p r  p 
-   e   t d a b   e a  e 
-           m u   G t    
-           e t   M o    
-             e   S r    

-   1 * * * * * * * 0  1   Id    *               _V0.123.34.63
-   1 * * * * * * * 1  1   Id    *               _V0.123.34.63:
-   1 * * * * * * * 0  0   Id    *                 0.123.34.63
-   1 * * * * * * * 1  0   Id    *                 0.123.34.63:

-   0 * 1 * * * * * *  1   Id    *               _O0.123.34.63:1234567890
-   0 * 1 * * * * * *  0   Id    *                 0.123.34.63:1234567890

-   0 * 0 1 * 1 0 * *  *   Id    *               _A0.123.34.63:1234567890(_T0.2:0.34.1)
-   0 * 0 1 * 1 1 * *  *   Id    *               _A0.123.34.63:1234567890(_T0.2:0.34.1)[60.4]

-   1 * * * * * * * *  *   Std   Export          _V0.123.34.63:
-   0 0 0 * * 1 * * *  *   Std   Export          _O0.123.34.63:1234567890
-   0 0 0 0 1 1 * * *  *   Std   Export          _A0.123.34.63:1234567890(pwrb:Class-Ai-RtBody)FilterAttribute[2]

-   1 1 1 0 0 1 1 0 *  *   Std   Strict          Avol:Bobj-Cobj-Dobj-Eobj.FilterAttribute[2]
-   0 1 1 0 0 1 1 0 *  *   Std   Strict               Bobj-Cobj-Dobj-Eobj.FilterAttribute[2]
-   0 0 1 0 0 1 1 0 *  *   Std   Strict                              Eobj.FilterAttribute[2]
-   0 0 0 0 0 1 1 0 0  *   Std   Strict                                   FilterAttribute[2]
-   0 0 0 0 0 1 1 0 1  *   Std   Strict                                  .FilterAttribute[2]
-   0 0 0 0 0 1 0 0 0  *   Std   Strict                                   FilterAttribute
-   0 0 0 0 0 1 0 0 1  *   Std   Strict                                  .FilterAttribute
-   1 1 1 0 0 1 0 0 *  *   Std   Strict          Avol:Bobj-Cobj-Dobj-Eobj.FilterAttribute
-   1 1 1 0 0 0 0 0 0  *   Std   Strict          Avol:Bobj-Cobj-Dobj-Eobj
-   1 1 1 0 0 0 0 0 1  *   Std   Strict          Avol:Bobj-Cobj-Dobj-Eobj-
-   1 1 0 0 0 0 0 0 0  *   Std   Strict          Avol:Bobj-Cobj-Dobj
-   1 1 0 0 0 0 0 0 1  *   Std   Strict          Avol:Bobj-Cobj-Dobj-
-   1 0 0 0 0 0 0 0 0  *   Std   Strict          Avol
-   1 0 0 0 0 0 0 0 1  *   Std   Strict          Avol:
   
-   1 1 1 0 0 1 1 1 0  *   Std   Strict          Avol\:Bobj\-Cobj\-Dobj\-Eobj\.FilterAttribute[2]
-   0 1 1 0 0 1 1 1 0  *   Std   Strict                Bobj\-Cobj\-Dobj\-Eobj\.FilterAttribute[2]
-   0 0 1 0 0 1 1 1 0  *   Std   Strict                                  Eobj\.FilterAttribute[2]
-   0 0 0 0 0 1 1 1 0  *   Std   Strict                                        FilterAttribute[2]
-   1 1 1 0 0 1 0 1 0  *   Std   Strict          Avol\:Bobj\-Cobj\-Dobj\-Eobj\.FilterAttribute
-   1 1 1 0 0 0 0 1 0  *   Std   Strict          Avol\:Bobj\-Cobj\-Dobj\-Eobj
-   1 1 0 0 0 0 0 1 0  *   Std   Strict          Avol\:Bobj\-Cobj\-Dobj
-   1 0 0 0 0 0 0 1 0  *   Std   Strict          Avol
   
-   1 1 1 0 0 1 1 0 *  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/Eobj.FilterAttribute[2]
-   0 1 1 0 0 1 1 0 *  *   Root  Strict                /Bobj/Cobj/Dobj/Eobj.FilterAttribute[2]
-   0 0 1 0 0 1 1 0 *  *   Root  Strict                                Eobj.FilterAttribute[2]
-   0 0 0 0 0 1 1 0 0  *   Root  Strict                                     FilterAttribute[2]
-   1 1 1 0 0 1 0 0 0  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/Eobj.FilterAttribute
-   1 1 1 0 0 0 0 0 0  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/Eobj
-   1 1 1 0 0 0 0 0 1  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/Eobj/
-   1 1 0 0 0 0 0 0 0  *   Root  Strict          //Avol/Bobj/Cobj/Dobj
-   1 1 0 0 0 0 0 0 1  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/
-   1 0 0 0 0 0 0 0 0  *   Root  Strict          //Avol
-   1 0 0 0 0 0 0 0 1  *   Root  Strict          //Avol/
   
*/

typedef union {
  pwr_tBitMask		m;
  //! Bit mask representation.
  struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

    pwr_tBit		fallback	: 8;

    pwr_tBit		form		: 8;

    pwr_tBit		fill		: 3;
    pwr_tBit		trueAttr       	: 1;
    pwr_tBit		ref		: 1;
    pwr_tBit		parent		: 1;
    pwr_tBit		idString	: 1;
    pwr_tBit		separator	: 1;

    pwr_tBit		escapeGMS	: 1;
    pwr_tBit		index		: 1;
    pwr_tBit		attribute	: 1;
    pwr_tBit		bodyName	: 1;
    pwr_tBit		bodyId		: 1;
    pwr_tBit		object		: 1;
    pwr_tBit		path		: 1;
    pwr_tBit		volume		: 1;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

    pwr_tBit		volume		: 1;
    pwr_tBit		path		: 1;
    pwr_tBit		object		: 1;
    pwr_tBit		bodyId		: 1;
    pwr_tBit		bodyName	: 1;
    pwr_tBit		attribute	: 1;
    pwr_tBit		index		: 1;
    pwr_tBit		escapeGMS	: 1;

    pwr_tBit		separator	: 1;
    pwr_tBit		idString	: 1;
    pwr_tBit		parent		: 1;
    pwr_tBit		ref		: 1;
    pwr_tBit		trueAttr       	: 1;
    pwr_tBit		fill		: 3;

    pwr_tBit		form		: 8;

    pwr_tBit		fallback	: 8;

#endif
  } b;
  //! Word representation.
  struct {
#if (pwr_dHost_byteOrder == pwr_dBigEndian)

    pwr_tUInt8		fallback;
    pwr_tUInt8		form;
    pwr_tUInt16		bits;

#elif (pwr_dHost_byteOrder == pwr_dLittleEndian)

    pwr_tUInt16		bits;
    pwr_tUInt8		form;
    pwr_tUInt8		fallback;

#endif
  } e;

#define cdh_mNName 0 
#define	cdh_mName__			0
#define	cdh_mName_volume		pwr_Bit(0)
#define	cdh_mName_path			pwr_Bit(1)
#define	cdh_mName_object		pwr_Bit(2)
#define	cdh_mName_bodyId		pwr_Bit(3)
#define	cdh_mName_bodyName		pwr_Bit(4)
#define	cdh_mName_attribute		pwr_Bit(5)
#define	cdh_mName_index			pwr_Bit(6)
#define cdh_mName_escapeGMS		pwr_Bit(7)
#define cdh_mName_separator		pwr_Bit(8)
#define cdh_mName_idString		pwr_Bit(9)
#define cdh_mName_parent		pwr_Bit(10)
#define cdh_mName_ref			pwr_Bit(11)
#define cdh_mName_trueAttr     		pwr_Bit(12)
#define	cdh_mName_			(~cdh_mName__)

#define cdh_mName_eForm_std		1
#define cdh_mName_eForm_root		2
#define cdh_mName_eForm_id		3

# define cdh_mName_form_std		pwr_SetByte(2, cdh_mName_eForm_std)
# define cdh_mName_form_root		pwr_SetByte(2, cdh_mName_eForm_root)
# define cdh_mName_form_id		pwr_SetByte(2, cdh_mName_eForm_id)

#define  cdh_mName_eFallback_bestTry	1
#define  cdh_mName_eFallback_strict	2
#define  cdh_mName_eFallback_export	3
#define  cdh_mName_eFallback_volumeDump	4

#define  cdh_mName_fallback_bestTry	pwr_SetByte(3, cdh_mName_eFallback_bestTry)
#define  cdh_mName_fallback_strict	pwr_SetByte(3, cdh_mName_eFallback_strict)
#define  cdh_mName_fallback_export	pwr_SetByte(3, cdh_mName_eFallback_export)
#define  cdh_mName_fallback_volumeDump	pwr_SetByte(3, cdh_mName_eFallback_volumeDump)

#define cdh_mName_pathBestTry		(cdh_mName_path | cdh_mName_object |cdh_mName_attribute |\
					 cdh_mName_index | cdh_mName_form_std | cdh_mName_Fallback_bestTry)
#define cdh_mName_volumeBestTry		(cdh_mName_volume | cdh_mName_path | cdh_mName_object |cdh_mName_attribute |\
                                         cdh_mName_index | cdh_mName_form_std | cdh_mName_fallback_bestTry)
#define cdh_mName_pathStrict		(cdh_mName_path | cdh_mName_object |cdh_mName_attribute |\
                                         cdh_mName_index | cdh_mName_form_std | cdh_mName_fallback_strict)
#define cdh_mName_volumeStrict		(cdh_mName_volume | cdh_mName_path | cdh_mName_object |cdh_mName_attribute |\
                                         cdh_mName_index | cdh_mName_form_std | cdh_mName_fallback_strict)
} cdh_mName;

//! Parse name struct.
typedef struct {
  pwr_tOid		 poid;		/* Parent objid, or NOBJID */
  cdh_mParseName parseFlags;

  cdh_mName		 flags;
  void			 *ohp;
  cdh_eId		 eId;
  cdh_uId		 uId;
  pwr_tTid		 bid;
  pwr_tUInt32    offset;
  pwr_tUInt32    size;
  pwr_tUInt32    nObject;
  pwr_tUInt32    nAttribute;
  pwr_tUInt32    nBody;
  cdh_sFamily    volume;
  cdh_sFamily    object[20];
  cdh_sFamily    body[10];
  cdh_sFamily    attribute[20];
  pwr_tUInt32    index[20];
  pwr_tBoolean   hasIndex[20];
} cdh_sParseName;

/*@}*/

/*! \defgroup Cdh_FC Cdh Functions
    \ingroup Cdh
*/

/*! \addtogroup Cdh_FC */
/*@{*/

/*  Function prototypes to exported functions.  */

int
cdh_ObjidCompare (
  pwr_tOid Object_1,
  pwr_tOid Object_2
);

int
cdh_ObjidIsEqual (
  pwr_tOid Object_1,
  pwr_tOid Object_2
);

int
cdh_ObjidIsNotEqual (
  pwr_tOid Object_1,
  pwr_tOid Object_2
);

int
cdh_ObjidIsNull (
  pwr_tOid Object
);

int
cdh_ObjidIsNotNull (
  pwr_tOid Object
);

int
cdh_SubidCompare (
  pwr_tSubid Subscription_1,
  pwr_tSubid Subscription_2
);

int
cdh_SubidIsEqual (
  pwr_tSubid Subscription_1,
  pwr_tSubid Subscription_2
);

int
cdh_SubidIsNotEqual (
  pwr_tSubid Subscription_1,
  pwr_tSubid Subscription_2
);

int
cdh_SubidIsNull (
  pwr_tSubid Subscription
);

int
cdh_SubidIsNotNull (
  pwr_tSubid Subscription
);

int
cdh_RefIdCompare (
  pwr_tRefId Reference_1,
  pwr_tRefId Reference_2
);

int
cdh_RefIdIsEqual (
  pwr_tRefId Reference_1,
  pwr_tRefId Reference_2
);

int
cdh_RefIdIsNotEqual (
  pwr_tRefId Reference_1,
  pwr_tRefId Reference_2
);

int
cdh_RefIdIsNull (
  pwr_tRefId Reference
);

int
cdh_RefIdIsNotNull (
  pwr_tRefId Reference
);

int
cdh_DlidCompare (
  pwr_tDlid DirectLink_1,
  pwr_tDlid DirectLink_2
);

int
cdh_DlidIsEqual (
  pwr_tDlid DirectLink_1,
  pwr_tDlid DirectLink_2
);

int
cdh_DlidIsNotEqual (
  pwr_tDlid DirectLink_1,
  pwr_tDlid DirectLink_2
);

int
cdh_DlidIsNull (
  pwr_tDlid DirectLink
);

int
cdh_DlidIsNotNull (
  pwr_tDlid DirectLink
);

int
cdh_ArefIsEqual (
  pwr_sAttrRef *arp1,
  pwr_sAttrRef *arp2
);

int cdh_IsClassVolume(
  pwr_tVid vid
);

pwr_tCid
cdh_ClassObjidToId (
  pwr_tOid Object
);

pwr_tOid
cdh_ClassIdToObjid (
  pwr_tCid Class
);

pwr_tTid
cdh_TypeObjidToId (
  pwr_tOid Object
);

int
cdh_TypeIdToIndex (
  pwr_tTid Type
);

pwr_tOid
cdh_TypeIdToObjid (
  pwr_tTid Type
);

pwr_sAttrRef
cdh_ObjidToAref (
  pwr_tObjid Objid
);

pwr_tStatus
cdh_AttrValueToString (
  pwr_eType Type,
  void      *Value,
  char      *String,
  int       MaxSize
);

pwr_tStatus
cdh_StringToAttrValue (
  pwr_eType		Type,
  const char		*String,
  void			*Value
);

pwr_tStatus
cdh_StringToClassId (
  const char		*s,
  pwr_tCid		*cid
);

pwr_tStatus
cdh_StringToTypeId (
  const char		*s,
  pwr_tTid		*tid
);

pwr_tStatus
cdh_StringToVolumeId (
  const char		*s,
  pwr_tVid		*tid
);

pwr_tStatus
cdh_StringToObjectIx (
  const char		*s,
  pwr_tOix		*oix
);

pwr_tStatus
cdh_StringToObjid (
  const char    *s,
  pwr_tOid		*oid
);

pwr_tStatus
cdh_StringToSubid (
  const char		*s,
  pwr_tSubid		*sid
);

pwr_tStatus
cdh_StringToDlid (
  const char    *s,
  pwr_tDlid		*did
);

char *
cdh_ClassIdToString (
  char			*s,
  pwr_tCid  cid,
  int			prefix
);

char *
cdh_ObjectIxToString (
  char			*s,
  pwr_tOix		oix,
  int			prefix
);

char *
cdh_ObjidToString (
  char			*s,
  pwr_tOid		oid,
  int			prefix
);

char *
cdh_ObjidToFnString (
  char			*s,
  pwr_tOid		oid
);

char *
cdh_ArefToString (
  char			*s,
  pwr_sAttrRef  *aref,
  int			prefix
);

char *
cdh_NodeIdToString (
  char     *s,
  pwr_tNid nid,
  int      prefix,
  int      suffix
);

char *
cdh_TypeIdToString(char *s, pwr_tTid tid, int prefix);

char *
cdh_VolumeIdToString (
    char     *s,
    pwr_tVid vid,
    int      prefix,
    int      suffix
    );

char *
cdh_SubidToString (
  char			*s,
  pwr_tSubid    sid,
  int			prefix
);

char *
cdh_DlidToString (
  char			*s,
  pwr_tDlid		did,
  int			prefix
);

cdh_sFamily *
cdh_Family (
  cdh_sFamily		*f,
  const char		*name,
  pwr_tOid		    poid
);

cdh_sObjName *
cdh_ObjName (
  cdh_sObjName		*on,
  const char		*name
);

pwr_tUInt32
cdh_PackName (
  const char		*name
);

cdh_sParseName *
cdh_ParseName (
  pwr_tStatus		*sts,
  cdh_sParseName	*pn,
  pwr_tOid		    poid,
  const char		*name,
  pwr_tUInt32		flags
);

char *
cdh_Low (
  const char		*s
);

char *
cdh_ToLower (
  char			*t,
  const char		*s
);

char *
cdh_ToUpper (
  char			*t,
  const char		*s
);

int
cdh_NoCaseStrcmp (
  const char		*s,
  const char		*t
);

int
cdh_NoCaseStrncmp (
  const char		*s,
  const char		*t,
  size_t 		n
);

int 
cdh_StrncpyCutOff(
  char			*s,
  const char	       	*t,
  size_t		n,
  int			cutleft
);

char *cdh_OpSysToStr( pwr_mOpSys opsys);

pwr_sAttrRef cdh_ArefToCastAref( pwr_sAttrRef *arp);

pwr_sAttrRef cdh_ArefToDisableAref( pwr_sAttrRef *arp);

pwr_sAttrRef cdh_ArefAdd( pwr_sAttrRef *arp1, pwr_sAttrRef *arp2);

void cdh_SuppressSuper( char *out, char *in);

void cdh_SuppressSuperAll( char *out, char *in);

int cdh_TypeToMaxStrSize( pwr_eType type, int attr_size, int attr_elements);

char *cdh_StringToObjectName( char *t, const char *s);

pwr_tStatus cdh_NextObjectName( char *t, const char *s);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif

