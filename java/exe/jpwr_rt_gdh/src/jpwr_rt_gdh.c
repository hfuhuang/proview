/* 
 * Proview   $Id: jpwr_rt_gdh.c,v 1.16 2008-03-14 07:38:44 claes Exp $
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


#include <string.h>
#include <stdio.h>
#include "jpwr_rt_gdh.h"
#include "pwr.h"
#include "rt_gdh.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"
#include "co_api.h"
#include "co_tree.h"
#include "co_msg.h"
#include "co_cdh_msg.h"
#include "rt_gdh_msg.h"

// Defined in ge_graph.h...
typedef enum {
  graph_eType_Bit = (1 << 15) + 1 //!< Type for a bit in a bitmask
} graph_eType;

typedef struct {
  char		TypeStr[32];
  pwr_eType	Type;
  int		Size;
} gdh_sSuffixTable;

typedef struct {
  tree_sNode 	node;
  int 		jid;
  void 		*p;
  pwr_tRefId 	refid;
} sJid;

#if defined OS_LINUX && defined HW_X86_64
static tree_sTable *jid_table = 0;
static int jid_next = 1;
#endif

static int gdh_ExtractNameSuffix(	char   *Name,
                          		char   **Suffix);
static void  gdh_TranslateSuffixToClassData (
    char        *SuffixPtr,
    pwr_eType 	*PwrType,
    int		*PwrSize,
    int  	*NoOfElements);
static int gdh_StringToAttr( char *str_value, char *buffer_p, int buffer_size,
	pwr_eType attrtype, int attrsize);
static void  gdh_AttrToString( int type_id, void *value_ptr,
        char *str, int size, int *len, char *format);
static void gdh_ConvertUTFstring( char *out, char *in);
static int gdh_JidToPointer( int id, void **p);
static int gdh_JidStore( void *p, pwr_tRefId r, int *id);
static int gdh_JidRemove( pwr_tRefId r);
	
JNIEXPORT jint JNICALL Java_jpwr_rt_Gdh_init
  (JNIEnv *env, jclass obj)
{
  int sts;

  sts = gdh_Init("java_native");
  return sts;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_setObjectInfoFloat
  (JNIEnv *env, jclass obj, jstring name, jfloat value) 
{
  int 		sts;
  const char 	*str;
  char 		*cstr;
  pwr_tFloat32 	val;
  jclass 	pwrtStatus_id;
  jmethodID 	pwrtStatus_cid;
  jobject 	return_obj;
  jint		jsts;
  char 		*s, *s1;

  pwrtStatus_id = (*env)->FindClass( env, "jpwr/rt/PwrtStatus");
  pwrtStatus_cid = (*env)->GetMethodID( env, pwrtStatus_id,
    	"<init>", "(I)V");

  val = value;
  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#'))) {
    *s = 0;
    if ( (s1 = strchr( s+1, '[')))
      strcat( cstr, s1);
  }
  sts = gdh_SetObjectInfo( cstr, (void *) &val, sizeof(val));
  (*env)->ReleaseStringUTFChars( env, name, cstr);
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, pwrtStatus_id,
  	pwrtStatus_cid, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_setObjectInfoInt
  (JNIEnv *env, jclass obj, jstring name, jint value) 
{
  int sts;
  const char *str;
  char *cstr;
  pwr_tInt32 val;
  jclass 	pwrtStatus_id;
  jmethodID 	pwrtStatus_cid;
  jobject 	return_obj;
  jint		jsts;
  char 		*s, *s1;

  pwrtStatus_id = (*env)->FindClass( env, "jpwr/rt/PwrtStatus");
  pwrtStatus_cid = (*env)->GetMethodID( env, pwrtStatus_id,
    	"<init>", "(I)V");

  val = value;
  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#'))) {
    *s = 0;
    if ( (s1 = strchr( s+1, '[')))
      strcat( cstr, s1);
  }
  sts = gdh_SetObjectInfo( cstr, (void *) &val, sizeof(val));
  (*env)->ReleaseStringUTFChars( env, name, cstr);
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, pwrtStatus_id,
  	pwrtStatus_cid, jsts);
  return return_obj;
}


JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_setObjectInfoBoolean
  (JNIEnv *env, jclass obj, jstring name, jboolean value) 
{
  int 		sts;
  const char 	*str;
  char 		*cstr;
  pwr_tBoolean 	val;
  jclass 	pwrtStatus_id;
  jmethodID 	pwrtStatus_cid;
  jobject 	return_obj;
  jint		jsts;
  char 		*s, *s1;

  pwrtStatus_id = (*env)->FindClass( env, "jpwr/rt/PwrtStatus");
  pwrtStatus_cid = (*env)->GetMethodID( env, pwrtStatus_id,
    	"<init>", "(I)V");

  val = value;
  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#'))) {
    *s = 0;
    if ( (s1 = strchr( s+1, '[')))
      strcat( cstr, s1);
  }
  sts = gdh_SetObjectInfo( cstr, (void *) &val, sizeof(val));
  (*env)->ReleaseStringUTFChars( env, name, cstr);
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, pwrtStatus_id,
  	pwrtStatus_cid, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_setObjectInfoString
  (JNIEnv *env, jclass obj, jstring name, jstring value) 
{
  int 		sts;
  const char 	*str;
  char 		*cstr;
  const char 	*str_value;
  char 		*cstr_value;
  jclass 	pwrtStatus_id;
  jmethodID 	pwrtStatus_cid;
  jobject 	return_obj;
  jint		jsts;
  pwr_tTypeId	attrtype;
  unsigned int	attrsize, attroffs, attrelem;
  char 		*s, *s1;
  char		buffer[256];
  int           element = 0;
  
  pwrtStatus_id = (*env)->FindClass( env, "jpwr/rt/PwrtStatus");
  pwrtStatus_cid = (*env)->GetMethodID( env, pwrtStatus_id,
    	"<init>", "(I)V");
  
  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  str_value = (*env)->GetStringUTFChars( env, value, 0);
  cstr_value = (char *)str_value;
  gdh_ConvertUTFstring( cstr_value, cstr_value);
  if ( (s = (char *)strchr( cstr, '#'))) {
    *s = 0;
    if ( (s1 = strchr( s+1, '[')))
      element = 1;
  }  
  /* Get typeid and convert string to attr */
  sts = gdh_GetAttributeCharacteristics( cstr,
                &attrtype, &attrsize, &attroffs, &attrelem);
  if ( ODD(sts))
  {
    if ( element) {
      attrsize /= attrelem;
      strcat( cstr, s1);
    }
    sts = gdh_StringToAttr( cstr_value, buffer, sizeof(buffer),
	attrtype, attrsize);
    if (ODD(sts)) {
      sts = gdh_SetObjectInfo( cstr, (void *) buffer, attrsize);
    }
  }
  (*env)->ReleaseStringUTFChars( env, name, cstr);
  (*env)->ReleaseStringUTFChars( env, value, cstr_value);
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, pwrtStatus_id,
  	pwrtStatus_cid, jsts);
  return return_obj;
}

JNIEXPORT jfloat JNICALL Java_jpwr_rt_Gdh_getObjectRefInfoFloat
  (JNIEnv *env, jclass obj, jint id)
{
  pwr_tStatus sts;
  pwr_tFloat32 *p;
  
  sts = gdh_JidToPointer( id, (void **)&p);
  if ( EVEN(sts)) return (jfloat) 0;

  return (jfloat) *p;
}

JNIEXPORT jboolean JNICALL Java_jpwr_rt_Gdh_getObjectRefInfoBoolean
  (JNIEnv *env, jclass obj, jint id)
{
  pwr_tStatus sts;
  pwr_tBoolean *p;

  sts = gdh_JidToPointer( id, (void **)&p);
  if ( EVEN(sts)) return 0;

  if ( *p)
    return 1;
  else
    return 0;
}

JNIEXPORT jint JNICALL Java_jpwr_rt_Gdh_getObjectRefInfoInt
  (JNIEnv *env, jclass obj, jint id)
{
  pwr_tStatus sts;
  pwr_tInt32 *p;

  sts = gdh_JidToPointer( id, (void **)&p);
  if ( EVEN(sts)) return 0;

  return (jint) *p;
}

JNIEXPORT jstring JNICALL Java_jpwr_rt_Gdh_getObjectRefInfoString
  (JNIEnv *env, jclass obj, jint id, jint jtypeid)
{
  jstring jvalue;
  pwr_tTypeId typeid;
  pwr_tStatus sts;
  char *p;

  sts = gdh_JidToPointer( id, (void **)&p);
  if ( EVEN(sts)) return 0;
  
  typeid = (pwr_tTypeId) jtypeid;
  if ( typeid == 0 || typeid == pwr_eType_String)
  {
    /* String is default */
    jvalue = (*env)->NewStringUTF( env, p);
    return jvalue;
  }
  else
  {
    char buffer[256];
    int len;
    
    gdh_AttrToString( typeid, (void *)p, buffer, sizeof(buffer), 
    	&len, NULL);
    jvalue = (*env)->NewStringUTF( env, buffer);
    return jvalue;
  }
}

JNIEXPORT jfloatArray JNICALL Java_jpwr_rt_Gdh_getObjectRefInfoFloatArray
  (JNIEnv *env, jclass obj, jint id, jint size)
{
  jfloatArray jfloatArr = (*env)->NewFloatArray(env, size);
  pwr_tStatus sts;
  float *p;

  sts = gdh_JidToPointer( id, (void **)&p);
  if ( EVEN(sts)) return 0;

  if(jfloatArr == NULL || p == NULL)
  {
    //something very weird has happen
    return (jfloatArray)NULL;
  }

  (*env)->SetFloatArrayRegion(env, jfloatArr, 0, size, (jfloat *)p);

  return jfloatArr;
}


JNIEXPORT jbooleanArray JNICALL Java_jpwr_rt_Gdh_getObjectRefInfoBooleanArray
  (JNIEnv *env, jclass obj, jint id, jint size)
{
  jbooleanArray jbooleanArr = (*env)->NewBooleanArray(env, size);
  pwr_tStatus sts;
  pwr_tBoolean *p;

  sts = gdh_JidToPointer( id, (void **)&p);
  if ( EVEN(sts)) return 0;
  

  if(jbooleanArr == NULL || p == NULL)
  {
    //something very weird has happen
    return (jbooleanArray)NULL;
  }

  (*env)->SetBooleanArrayRegion(env, jbooleanArr, 0, size, (jboolean *)p);

  return jbooleanArr;
}


JNIEXPORT jintArray JNICALL Java_jpwr_rt_Gdh_getObjectRefInfoIntArray
  (JNIEnv *env, jclass obj, jint id, jint size)
{
  jintArray jintArr = (*env)->NewIntArray(env, size);
  pwr_tStatus sts;
  pwr_tInt32 *p;

  sts = gdh_JidToPointer( id, (void **)&p);
  if ( EVEN(sts)) return 0;
  
  if(jintArr == NULL || p == NULL)
  {
    //something very weird has happen
    return (jintArray)NULL;
  }

  (*env)->SetIntArrayRegion(env, jintArr, 0, size, (jint *)p);

  return jintArr;

}

JNIEXPORT jobjectArray JNICALL Java_jpwr_rt_Gdh_getObjectRefInfoStringArray
  (JNIEnv *env, jclass obj, jint id, jint jtypeid, jint size, jint elements)
{

  pwr_tTypeId typeid;
  jobjectArray jobjectArr;
  int i = 0;
  pwr_tStatus sts;
  char *p;

  sts = gdh_JidToPointer( id, (void **)&p);
  if ( EVEN(sts)) return 0;

  //find the class for String[]
  jclass strArrCls = (*env)->FindClass(env, "java/lang/String");

  if(strArrCls == NULL)
  {
    return (jobjectArray)NULL;
  }
  //create a new String[]
  jobjectArr = (*env)->NewObjectArray(env, elements, strArrCls, NULL);
  

  typeid = (pwr_tTypeId) jtypeid;
  if ( typeid == 0 || typeid == pwr_eType_String)
  {
    // String is default
    //put the result in an objectarray of Strings
    for(i=0;i<elements;i++)
    {
      (*env)->SetObjectArrayElement(env, jobjectArr, i, (*env)->NewStringUTF( env, p));
      id += size;
    }
  }
  else
  {
    char buffer[256];
    int len;
    /*    
    switch( typeid) {
    case pwr_eType_Objid: 
      size = sizeof(pwr_tObjid);
      break;
    case pwr_eType_AttrRef: 
      size = sizeof(pwr_tAttrRef);
      break;
    case pwr_eType_Time: 
      size = sizeof(pwr_tTime);
      break;
    case pwr_eType_DeltaTime: 
      size = sizeof(pwr_tDeltaTime);
      break;
    default:
      size = 4;
    }
*/
    //put the result in an objectarray of Strings
    for(i=0;i<elements;i++)
    {
      gdh_AttrToString( typeid, (void *)p, buffer, sizeof(buffer), 
      	                &len, NULL);

      (*env)->SetObjectArrayElement(env, jobjectArr, i, (*env)->NewStringUTF( env, (char *)buffer));
      id += size;
    }
  }
  return jobjectArr;
}


JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_refObjectInfo
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  void *attr_p;
  pwr_tSubid subid;
  jint id;
  jobject refid_obj = NULL;
  jint rix, nid;
  jobject return_obj;
  jint jsts;
  jclass gdhrRefObjectInfo_id;
  
  static jmethodID gdhrRefObjectInfo_cid = NULL;
  
  jclass PwrtRefId_id;
  
  static jmethodID PwrtRefId_cid = NULL;
  
  char *suffix_p;
  int size;
  int elements;
  pwr_eType typeid;

  gdhrRefObjectInfo_id = (*env)->FindClass( env, "jpwr/rt/GdhrRefObjectInfo");
  
  if(gdhrRefObjectInfo_cid == NULL)
  {
    gdhrRefObjectInfo_cid = (*env)->GetMethodID( env, gdhrRefObjectInfo_id,
    	"<init>", "(Ljpwr/rt/PwrtRefId;IIIII)V");

    if(gdhrRefObjectInfo_cid == NULL)
    {
      printf("fel vid init av gdhrRefObjectInfo_cid\n"); 
      return NULL;
    }
  }
  PwrtRefId_id = (*env)->FindClass( env, "jpwr/rt/PwrtRefId");
  if(PwrtRefId_cid == NULL)
  {
    PwrtRefId_cid = (*env)->GetMethodID( env, PwrtRefId_id,
    	"<init>", "(II)V");
    //printf("PwrtRefId_cid initierad\n");
    if(PwrtRefId_cid == NULL)
    {
      printf("fel vid init av PwrtRefId_cid\n"); 
      return NULL;
    }
  }
  /* The pointer and the subid should be stored somewhere... */
  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;  
  gdh_ConvertUTFstring( cstr, cstr);

  /* Extract the type and size from the suffix */
  if ( gdh_ExtractNameSuffix( cstr, &suffix_p)) {
    gdh_TranslateSuffixToClassData( suffix_p, &typeid, &size, &elements);
    if ( typeid == graph_eType_Bit) {
      char *s = strrchr( cstr, '[');
      if ( s)
	*s = 0;
    }
  }
  else
  {
    typeid = pwr_eType_Boolean;
    size = sizeof(pwr_tBoolean);
    elements = 1;
  }

  sts = gdh_RefObjectInfo( cstr, &attr_p, &subid, size);
  (*env)->ReleaseStringUTFChars( env, name, cstr);

  if ( ODD(sts))
  {
    gdh_JidStore( attr_p, subid, (int *) &id);
    rix = (jint) subid.rix;
    nid = (jint) subid.nid;
    refid_obj = (*env)->NewObject( env, PwrtRefId_id, PwrtRefId_cid, 
    	rix, nid);
  }
  else
    id = 0;

  jsts = (jint) sts;
  //we want the size of each element not the hole object
  if(elements > 0)
      size = size/elements;
  return_obj = (*env)->NewObject( env, gdhrRefObjectInfo_id,
  	gdhrRefObjectInfo_cid, refid_obj, id, jsts, (jint)typeid, (jint)elements, (jint)size);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_unrefObjectInfo
  (JNIEnv *env, jclass obj, jobject refid_obj)
{
  pwr_tRefId 	refid;
  int 		sts;
  jclass 	pwrtRefId_id;
  jmethodID 	pwrtRefId_getRix;
  jmethodID 	pwrtRefId_getNid;
  jclass 	pwrtStatus_id;
  jmethodID 	pwrtStatus_cid;
  jobject 	return_obj;
  jint		jsts;

  pwrtStatus_id = (*env)->FindClass( env, "jpwr/rt/PwrtStatus");
  pwrtStatus_cid = (*env)->GetMethodID( env, pwrtStatus_id,
    	"<init>", "(I)V");

  pwrtRefId_id = (*env)->FindClass( env, "jpwr/rt/PwrtRefId");
  pwrtRefId_getRix = (*env)->GetMethodID( env, pwrtRefId_id, "getRix", "()I");
  pwrtRefId_getNid = (*env)->GetMethodID( env, pwrtRefId_id, "getNid", "()I");

  refid.rix = (*env)->CallIntMethod( env, refid_obj, pwrtRefId_getRix);
  refid.nid = (*env)->CallIntMethod( env, refid_obj, pwrtRefId_getNid);

  sts = gdh_JidRemove( refid);
  sts = gdh_UnrefObjectInfo( refid);
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, pwrtStatus_id,
  	pwrtStatus_cid, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_toggleObjectInfo
  (JNIEnv *env, jclass obj, jstring name)
{
  int 		sts;
  const 	char *str;
  char 		*cstr;
  pwr_tBoolean 	val;
  jclass 	pwrtStatus_id;
  jmethodID 	pwrtStatus_cid;
  jobject 	return_obj;
  jint		jsts;
  char 		*s, *s1;

  pwrtStatus_id = (*env)->FindClass( env, "jpwr/rt/PwrtStatus");
  pwrtStatus_cid = (*env)->GetMethodID( env, pwrtStatus_id,
    	"<init>", "(I)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#'))) {
    *s = 0;
    if ( (s1 = strchr( s+1, '[')))
      strcat( cstr, s1);
  }
  sts = gdh_GetObjectInfo( cstr, (void *) &val, sizeof(val));
  if (ODD(sts))
  {
    val = !val;
    sts = gdh_SetObjectInfo( cstr, (void *) &val, sizeof(val));
  }
  (*env)->ReleaseStringUTFChars( env, name, cstr);

  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, pwrtStatus_id,
  	pwrtStatus_cid, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_nameToObjid
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  pwr_tObjid objid;
  jclass cdhrObjid_id;
  jmethodID cdhrObjid_cid;
  jclass PwrtObjid_id;
  jmethodID PwrtObjid_cid;
  jobject objid_obj = NULL;
  jint oix, vid;
  jobject return_obj;
  jint jsts;


  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;I)V");

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	"<init>", "(II)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  sts = gdh_NameToObjid( cstr, &objid);
  (*env)->ReleaseStringUTFChars( env, name, cstr);

  if ( ODD(sts))
  {
    oix = (jint) objid.oix;
    vid = (jint) objid.vid;
    objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, objid_obj, jsts);
  return return_obj; 
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_nameToAttrRef
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  pwr_tAttrRef aref;
  jclass cdhrAttrRef_id;
  jmethodID cdhrAttrRef_cid;
  jclass PwrtObjid_id;
  jmethodID PwrtObjid_cid;
  jclass PwrtAttrRef_id;
  jmethodID PwrtAttrRef_cid;
  jobject objid_obj = NULL;
  jobject attrref_obj = NULL;
  jint oix, vid;
  jint body, offset, size, flags;
  jobject return_obj;
  jint jsts;


  cdhrAttrRef_id = (*env)->FindClass( env, "jpwr/rt/CdhrAttrRef");
  cdhrAttrRef_cid = (*env)->GetMethodID( env, cdhrAttrRef_id,
    	"<init>", "(Ljpwr/rt/PwrtAttrRef;I)V");

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	"<init>", "(II)V");

  PwrtAttrRef_id = (*env)->FindClass( env, "jpwr/rt/PwrtAttrRef");
  PwrtAttrRef_cid = (*env)->GetMethodID( env, PwrtAttrRef_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;IIII)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  sts = gdh_NameToAttrref( pwr_cNOid, cstr, &aref);
  (*env)->ReleaseStringUTFChars( env, name, cstr);

  if ( ODD(sts)) {
    oix = (jint) aref.Objid.oix;
    vid = (jint) aref.Objid.vid;
    objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid,
    	oix, vid);

    body = (jint) aref.Body;
    offset = (jint) aref.Offset;
    size = (jint) aref.Size;
    flags = (jint) aref.Flags.m;
    attrref_obj = (*env)->NewObject( env, PwrtAttrRef_id, PwrtAttrRef_cid,
    	objid_obj, body, offset, size, flags);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrAttrRef_id,
  	cdhrAttrRef_cid, attrref_obj, jsts);
  return return_obj; 
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_objidToName
  (JNIEnv *env, jclass obj, jobject objid_obj, jint nameType)
{
  int 		sts;
  pwr_tOName	name;
  pwr_tObjid 	objid;
  jclass 	cdhrString_id;
  static jmethodID 	cdhrString_cid = NULL;
  jobject 	return_obj;
  jint 		jsts;
  jclass 	PwrtObjid_id;
  static jmethodID 	PwrtObjid_getOix = NULL;
  static jmethodID 	PwrtObjid_getVid = NULL;
  jstring	jname = NULL;

  cdhrString_id = (*env)->FindClass( env, "jpwr/rt/CdhrString");
  if(cdhrString_cid == NULL)
  {
    cdhrString_cid = (*env)->GetMethodID( env, cdhrString_id,
    	  "<init>", "(Ljava/lang/String;I)V");
  }

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  if(PwrtObjid_getOix == NULL || PwrtObjid_getVid == NULL)
  {
    PwrtObjid_getOix = (*env)->GetMethodID( env, PwrtObjid_id, "getOix", "()I");
    PwrtObjid_getVid = (*env)->GetMethodID( env, PwrtObjid_id, "getVid", "()I");
  }

  objid.oix = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getOix);
  objid.vid = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getVid);

  sts = gdh_ObjidToName( objid, name, sizeof(name), nameType);

  if ( ODD(sts))
    jname = (*env)->NewStringUTF( env, name);
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrString_id,
  	cdhrString_cid, jname, jsts);
  return return_obj;  
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_attrRefToName
  (JNIEnv *env, jclass obj, jobject aref_obj, jint nameType)
{
  int 		sts;
  pwr_tOName	name;
  pwr_tAttrRef 	aref;
  jclass 	cdhrString_id;
  static jmethodID 	cdhrString_cid = NULL;
  jobject 	return_obj;
  jint 		jsts;
  jclass 	PwrtAttrRef_id;
  static jmethodID 	PwrtAttrRef_getOix = NULL;
  static jmethodID 	PwrtAttrRef_getVid = NULL;
  static jmethodID 	PwrtAttrRef_getBody = NULL;
  static jmethodID 	PwrtAttrRef_getOffset = NULL;
  static jmethodID 	PwrtAttrRef_getSize = NULL;
  static jmethodID 	PwrtAttrRef_getFlags = NULL;
  jstring	jname = NULL;

  cdhrString_id = (*env)->FindClass( env, "jpwr/rt/CdhrString");
  if(cdhrString_cid == NULL) {
    cdhrString_cid = (*env)->GetMethodID( env, cdhrString_id,
    	  "<init>", "(Ljava/lang/String;I)V");
  }

  PwrtAttrRef_id = (*env)->FindClass( env, "jpwr/rt/PwrtAttrRef");
  if(PwrtAttrRef_getOix == NULL || PwrtAttrRef_getVid == NULL) {
    PwrtAttrRef_getOix = (*env)->GetMethodID( env, PwrtAttrRef_id, "getOix", "()I");
    PwrtAttrRef_getVid = (*env)->GetMethodID( env, PwrtAttrRef_id, "getVid", "()I");
    PwrtAttrRef_getBody = (*env)->GetMethodID( env, PwrtAttrRef_id, "getBody", "()I");
    PwrtAttrRef_getOffset = (*env)->GetMethodID( env, PwrtAttrRef_id, "getOffset", "()I");
    PwrtAttrRef_getSize = (*env)->GetMethodID( env, PwrtAttrRef_id, "getSize", "()I");
    PwrtAttrRef_getFlags = (*env)->GetMethodID( env, PwrtAttrRef_id, "getFlags", "()I");
  }

  aref.Objid.oix = (*env)->CallIntMethod( env, aref_obj, PwrtAttrRef_getOix);
  aref.Objid.vid = (*env)->CallIntMethod( env, aref_obj, PwrtAttrRef_getVid);
  aref.Body = (*env)->CallIntMethod( env, aref_obj, PwrtAttrRef_getBody);
  aref.Offset = (*env)->CallIntMethod( env, aref_obj, PwrtAttrRef_getOffset);
  aref.Size = (*env)->CallIntMethod( env, aref_obj, PwrtAttrRef_getSize);
  aref.Flags.m = (*env)->CallIntMethod( env, aref_obj, PwrtAttrRef_getFlags);

  sts = gdh_AttrrefToName( &aref, name, sizeof(name), nameType);

  if ( ODD(sts))
    jname = (*env)->NewStringUTF( env, name);
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrString_id,
  	cdhrString_cid, jname, jsts);
  return return_obj;  
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getRootList
  (JNIEnv *env, jclass obj)
{
  int		sts;
  jclass 	PwrtObjid_id;
  jmethodID 	PwrtObjid_cid;
  pwr_tObjid 	objid;
  jclass 	cdhrObjid_id;
  jmethodID 	cdhrObjid_cid;
  jobject 	objid_obj = NULL;
  jint 		oix, vid;
  jobject 	return_obj;
  jint 		jsts;

  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;I)V");

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	"<init>", "(II)V");

  sts = gdh_GetRootList( &objid);
  if ( ODD(sts))
  {
    oix = (jint) objid.oix;
    vid = (jint) objid.vid;
    objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, objid_obj, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getChild
  (JNIEnv *env, jclass obj, jobject objid_obj)
{
  int		sts;
  jclass 	PwrtObjid_id;
  static jmethodID 	PwrtObjid_getOix;
  static jmethodID 	PwrtObjid_getVid;
  static jmethodID 	PwrtObjid_cid;
  pwr_tObjid 	objid;
  pwr_tObjid	child_objid;
  jclass 	cdhrObjid_id;
  static jmethodID 	cdhrObjid_cid;
  jobject 	child_objid_obj = NULL;
  jint 		oix, vid;
  jobject 	return_obj;
  jint 		jsts;

  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  if(cdhrObjid_cid == NULL)
  {
    cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	  "<init>", "(Ljpwr/rt/PwrtObjid;I)V");
  }

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  
  if(PwrtObjid_cid == NULL || PwrtObjid_getOix == NULL || PwrtObjid_getVid == NULL)
  {
    PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	  "<init>", "(II)V");
    PwrtObjid_getOix = (*env)->GetMethodID( env, PwrtObjid_id, "getOix", "()I");
    PwrtObjid_getVid = (*env)->GetMethodID( env, PwrtObjid_id, "getVid", "()I");
  }

  objid.oix = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getOix);
  objid.vid = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getVid);

  sts = gdh_GetChild( objid, &child_objid);
  if ( ODD(sts))
  {
    oix = (jint) child_objid.oix;
    vid = (jint) child_objid.vid;
    child_objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, child_objid_obj, jsts);
  return return_obj;
}
JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getParent
  (JNIEnv *env, jclass obj, jobject objid_obj)
{
  int		sts;
  jclass 	PwrtObjid_id;
  static jmethodID 	PwrtObjid_getOix;
  static jmethodID 	PwrtObjid_getVid;
  static jmethodID 	PwrtObjid_cid;
  pwr_tObjid 	objid;
  pwr_tObjid	parent_objid;
  jclass 	cdhrObjid_id;
  static jmethodID 	cdhrObjid_cid;
  jobject 	parent_objid_obj = NULL;
  jint 		oix, vid;
  jobject 	return_obj;
  jint 		jsts;

  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  if(cdhrObjid_cid == NULL)
  {
    cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	  "<init>", "(Ljpwr/rt/PwrtObjid;I)V");
    //printf("cdhrObjid initierad\n");
  }

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  
  if(PwrtObjid_cid == NULL || PwrtObjid_getOix == NULL || PwrtObjid_getVid == NULL)
  {
    PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	  "<init>", "(II)V");
    PwrtObjid_getOix = (*env)->GetMethodID( env, PwrtObjid_id, "getOix", "()I");
    PwrtObjid_getVid = (*env)->GetMethodID( env, PwrtObjid_id, "getVid", "()I");
    //printf("PwrtObjid_yyy initierade\n");
  }

  objid.oix = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getOix);
  objid.vid = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getVid);

  sts = gdh_GetParent( objid, &parent_objid);
  if ( ODD(sts))
  {
    oix = (jint) parent_objid.oix;
    vid = (jint) parent_objid.vid;
    parent_objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, parent_objid_obj, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getNextObject
  (JNIEnv *env, jclass obj, jobject objid_obj)
{
  int		sts;
  jclass 	PwrtObjid_id;
  jmethodID 	PwrtObjid_getOix;
  jmethodID 	PwrtObjid_getVid;
  jmethodID 	PwrtObjid_cid;
  pwr_tObjid 	objid;
  pwr_tObjid	next_objid;
  jclass 	cdhrObjid_id;
  jmethodID 	cdhrObjid_cid;
  jobject 	next_objid_obj = NULL;
  jint 		oix, vid;
  jobject 	return_obj;
  jint 		jsts;

  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;I)V");

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	"<init>", "(II)V");
  PwrtObjid_getOix = (*env)->GetMethodID( env, PwrtObjid_id, "getOix", "()I");
  PwrtObjid_getVid = (*env)->GetMethodID( env, PwrtObjid_id, "getVid", "()I");

  objid.oix = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getOix);
  objid.vid = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getVid);

  sts = gdh_GetNextObject( objid, &next_objid);
  if ( ODD(sts))
  {
    oix = (jint) next_objid.oix;
    vid = (jint) next_objid.vid;
    next_objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, next_objid_obj, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getNextSibling
  (JNIEnv *env, jclass obj, jobject objid_obj)
{
  int		sts;
  jclass 	PwrtObjid_id;
  jmethodID 	PwrtObjid_getOix;
  jmethodID 	PwrtObjid_getVid;
  jmethodID 	PwrtObjid_cid;
  pwr_tObjid 	objid;
  pwr_tObjid	next_objid;
  jclass 	cdhrObjid_id;
  jmethodID 	cdhrObjid_cid;
  jobject 	next_objid_obj = NULL;
  jint 		oix, vid;
  jobject 	return_obj;
  jint 		jsts;

  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;I)V");

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	"<init>", "(II)V");
  PwrtObjid_getOix = (*env)->GetMethodID( env, PwrtObjid_id, "getOix", "()I");
  PwrtObjid_getVid = (*env)->GetMethodID( env, PwrtObjid_id, "getVid", "()I");

  objid.oix = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getOix);
  objid.vid = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getVid);

  sts = gdh_GetNextSibling( objid, &next_objid);
  if ( ODD(sts))
  {
    oix = (jint) next_objid.oix;
    vid = (jint) next_objid.vid;
    next_objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, next_objid_obj, jsts);
  return return_obj;

}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getObjectClass
  (JNIEnv *env, jclass obj, jobject objid_obj)
{
  int		sts;
  jclass 	pwrtObjid_id;
  static jmethodID 	pwrtObjid_getOix = NULL;
  static jmethodID 	pwrtObjid_getVid = NULL;
  static jmethodID 	pwrtObjid_cid = NULL;
  pwr_tObjid 	objid;
  jclass 	cdhrClassId_id;
  static jmethodID 	cdhrClassId_cid;
  jint	 	jclassid = 0;
  jobject 	return_obj;
  jint 		jsts;
  pwr_tClassId	classid;

  cdhrClassId_id = (*env)->FindClass( env, "jpwr/rt/CdhrClassId");
  if(cdhrClassId_cid == NULL)
  {
    cdhrClassId_cid = (*env)->GetMethodID( env, cdhrClassId_id,
    	  "<init>", "(II)V");
    //printf("cdhrClassId_cid initierad\n");
  } 

  pwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  if(pwrtObjid_cid == NULL || pwrtObjid_getOix == NULL || pwrtObjid_getVid == NULL)
  {
    pwrtObjid_cid = (*env)->GetMethodID( env, pwrtObjid_id,
    	  "<init>", "(II)V");
    pwrtObjid_getOix = (*env)->GetMethodID( env, pwrtObjid_id, "getOix", "()I");
    pwrtObjid_getVid = (*env)->GetMethodID( env, pwrtObjid_id, "getVid", "()I");
    //printf("pwrtObjid_yyy initierade\n");
  }

  objid.oix = (*env)->CallIntMethod( env, objid_obj, pwrtObjid_getOix);
  objid.vid = (*env)->CallIntMethod( env, objid_obj, pwrtObjid_getVid);

  sts = gdh_GetObjectClass( objid, &classid);
  if ( ODD(sts))
  {
    jclassid = (jint)classid;
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrClassId_id,
  	cdhrClassId_cid, jclassid, jsts);
  return return_obj;

}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getAttrRefTid
  (JNIEnv *env, jclass obj, jobject aref_obj)
{
  int		sts;
  jclass 	pwrtAttrRef_id;
  static jmethodID 	pwrtAttrRef_getOix = NULL;
  static jmethodID 	pwrtAttrRef_getVid = NULL;
  static jmethodID 	pwrtAttrRef_getBody = NULL;
  static jmethodID 	pwrtAttrRef_getOffset = NULL;
  static jmethodID 	pwrtAttrRef_getSize = NULL;
  static jmethodID 	pwrtAttrRef_getFlags = NULL;
  pwr_tAttrRef 	aref;
  jclass 	cdhrTypeId_id;
  static jmethodID 	cdhrTypeId_cid;
  jint	 	jtypeid = 0;
  jobject 	return_obj;
  jint 		jsts;
  pwr_tTid	tid;

  cdhrTypeId_id = (*env)->FindClass( env, "jpwr/rt/CdhrTypeId");
  if(cdhrTypeId_cid == NULL)
  {
    cdhrTypeId_cid = (*env)->GetMethodID( env, cdhrTypeId_id,
    	  "<init>", "(II)V");
  } 

  pwrtAttrRef_id = (*env)->FindClass( env, "jpwr/rt/PwrtAttrRef");
  if( pwrtAttrRef_getOix == NULL || pwrtAttrRef_getVid == NULL) {
    pwrtAttrRef_getOix = (*env)->GetMethodID( env, pwrtAttrRef_id, "getOix", "()I");
    pwrtAttrRef_getVid = (*env)->GetMethodID( env, pwrtAttrRef_id, "getVid", "()I");
    pwrtAttrRef_getBody = (*env)->GetMethodID( env, pwrtAttrRef_id, "getBody", "()I");
    pwrtAttrRef_getOffset = (*env)->GetMethodID( env, pwrtAttrRef_id, "getOffset", "()I");
    pwrtAttrRef_getSize = (*env)->GetMethodID( env, pwrtAttrRef_id, "getSize", "()I");
    pwrtAttrRef_getFlags = (*env)->GetMethodID( env, pwrtAttrRef_id, "getFlags", "()I");
  }

  aref.Objid.oix = (*env)->CallIntMethod( env, aref_obj, pwrtAttrRef_getOix);
  aref.Objid.vid = (*env)->CallIntMethod( env, aref_obj, pwrtAttrRef_getVid);
  aref.Body = (*env)->CallIntMethod( env, aref_obj, pwrtAttrRef_getBody);
  aref.Offset = (*env)->CallIntMethod( env, aref_obj, pwrtAttrRef_getOffset);
  aref.Size = (*env)->CallIntMethod( env, aref_obj, pwrtAttrRef_getSize);
  aref.Flags.m = (*env)->CallIntMethod( env, aref_obj, pwrtAttrRef_getFlags);

  sts = gdh_GetAttrRefTid( &aref, &tid);
  if ( ODD(sts)) {
    jtypeid = (jint)tid;
  }
  printf( "GetAttrRefTid %d\n", tid);
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrTypeId_id,
  	cdhrTypeId_cid, jtypeid, jsts);
  return return_obj;

}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getClassList
  (JNIEnv *env, jclass obj, jint jclassid)
{
  int		sts;
  jclass 	pwrtObjid_id;
  jmethodID 	pwrtObjid_cid;
  pwr_tObjid 	objid;
  jclass 	cdhrObjid_id;
  jmethodID 	cdhrObjid_cid;
  jobject 	objid_obj = NULL;
  jobject 	return_obj;
  jint 		jsts;
  pwr_tClassId	classid;
  jint 		oix, vid;


  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;I)V");

  pwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  pwrtObjid_cid = (*env)->GetMethodID( env, pwrtObjid_id,
    	"<init>", "(II)V");

  classid = (pwr_tClassId)jclassid;

  sts = gdh_GetClassList( classid, &objid);
  if ( ODD(sts))
  {
    oix = (jint) objid.oix;
    vid = (jint) objid.vid;
    objid_obj = (*env)->NewObject( env, pwrtObjid_id, pwrtObjid_cid, 
    	oix, vid);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, objid_obj, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_classIdToObjid
  (JNIEnv *env, jclass obj, jint jclassid)
{
  jclass 	pwrtObjid_id;
  jmethodID 	pwrtObjid_cid;
  pwr_tObjid 	objid;
  jclass 	cdhrObjid_id;
  jmethodID 	cdhrObjid_cid;
  jobject 	objid_obj = NULL;
  jobject 	return_obj;
  jint 		jsts;
  pwr_tClassId	classid;
  jint 		oix, vid;


  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;I)V");

  pwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  pwrtObjid_cid = (*env)->GetMethodID( env, pwrtObjid_id,
    	"<init>", "(II)V");

  classid = (pwr_tClassId)jclassid;

  objid = cdh_ClassIdToObjid( classid);
  oix = (jint) objid.oix;
  vid = (jint) objid.vid;
  objid_obj = (*env)->NewObject( env, pwrtObjid_id, pwrtObjid_cid, 
    	oix, vid);
  
  jsts = (jint) CDH__SUCCESS;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, objid_obj, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getObjectInfoBoolean
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  jclass cdhrBoolean_id;
  jmethodID cdhrBoolean_cid;
  jobject return_obj;
  jint jsts;
  jboolean jvalue;
  pwr_tBoolean value;
  char 		*s;

  cdhrBoolean_id = (*env)->FindClass( env, "jpwr/rt/CdhrBoolean");
  cdhrBoolean_cid = (*env)->GetMethodID( env, cdhrBoolean_id,
    	"<init>", "(ZI)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#')))
    *s = 0;
  sts = gdh_GetObjectInfo( cstr, &value, sizeof(value));
  (*env)->ReleaseStringUTFChars( env, name, cstr);
  
  jsts = (jint) sts;
  jvalue = value;
  return_obj = (*env)->NewObject( env, cdhrBoolean_id,
  	cdhrBoolean_cid, jvalue, jsts);
  return return_obj; 
}


JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getObjectInfoFloat
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  jclass cdhrFloat_id;
  jmethodID cdhrFloat_cid;
  jobject return_obj;
  jint jsts;
  jfloat jvalue;
  pwr_tFloat32 value;
  char 		*s;

  cdhrFloat_id = (*env)->FindClass( env, "jpwr/rt/CdhrFloat");
  cdhrFloat_cid = (*env)->GetMethodID( env, cdhrFloat_id,
    	"<init>", "(FI)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#')))
    *s = 0;
  sts = gdh_GetObjectInfo( cstr, &value, sizeof(value));
  (*env)->ReleaseStringUTFChars( env, name, cstr);
  
  jsts = (jint) sts;
  jvalue = value;
  return_obj = (*env)->NewObject( env, cdhrFloat_id,
  	cdhrFloat_cid, jvalue, jsts);
  return return_obj; 
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getObjectInfoFloatArray
  (JNIEnv *env, jclass obj, jstring name, jint size)
{
  int sts;
  const char *str;
  char *cstr;
  jclass cdhrFloatArray_id;
  jmethodID cdhrFloatArray_cid;
  jobject return_obj;
  jint jsts;
  jfloatArray jvalue = 0;
  pwr_tFloat32 *value;
  char 		*s;

  cdhrFloatArray_id = (*env)->FindClass( env, "jpwr/rt/CdhrFloatArray");
  cdhrFloatArray_cid = (*env)->GetMethodID( env, cdhrFloatArray_id,
    	"<init>", "([FI)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#')))
    *s = 0;

  value = (pwr_tFloat32 *)calloc( size, sizeof(pwr_tFloat32));
  sts = gdh_GetObjectInfo( cstr, value, size * 4);
  (*env)->ReleaseStringUTFChars( env, name, cstr);
  
  jsts = (jint) sts;
  if ( ODD(sts)) {
    jvalue = (*env)->NewFloatArray( env, size);
    (*env)->SetFloatArrayRegion( env, jvalue, 0, (int)size, value);
  }
  free( value);
  
  return_obj = (*env)->NewObject( env, cdhrFloatArray_id,
  	cdhrFloatArray_cid, jvalue, jsts);
  return return_obj; 
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getObjectInfoInt
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  jclass cdhrInt_id;
  jmethodID cdhrInt_cid;
  jobject return_obj;
  jint jsts;
  jint jvalue;
  pwr_tInt32 value = 0;
  char 		*s;

  cdhrInt_id = (*env)->FindClass( env, "jpwr/rt/CdhrInt");
  cdhrInt_cid = (*env)->GetMethodID( env, cdhrInt_id,
    	"<init>", "(II)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#')))
    *s = 0;
  sts = gdh_GetObjectInfo( cstr, &value, sizeof(value));
  (*env)->ReleaseStringUTFChars( env, name, cstr);

  jsts = (jint) sts;
  jvalue = value;
  return_obj = (*env)->NewObject( env, cdhrInt_id,
  	cdhrInt_cid, jvalue, jsts);
  return return_obj; 
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getObjectInfoIntArray
  (JNIEnv *env, jclass obj, jstring name, jint size)
{
  int sts;
  const char *str;
  char *cstr;
  jclass cdhrIntArray_id;
  jmethodID cdhrIntArray_cid;
  jobject return_obj;
  jint jsts;
  jintArray jvalue = 0;
  pwr_tInt32 *value;
  char 		*s;

  cdhrIntArray_id = (*env)->FindClass( env, "jpwr/rt/CdhrIntArray");
  cdhrIntArray_cid = (*env)->GetMethodID( env, cdhrIntArray_id,
    	"<init>", "([FI)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = (char *)strchr( cstr, '#')))
    *s = 0;

  value = (pwr_tInt32 *)calloc( size, sizeof(pwr_tInt32));
  sts = gdh_GetObjectInfo( cstr, value, size * 4);
  (*env)->ReleaseStringUTFChars( env, name, cstr);
  
  jsts = (jint) sts;
  if ( ODD(sts)) {
    jvalue = (*env)->NewIntArray( env, size);
    (*env)->SetIntArrayRegion( env, jvalue, 0, (int)size, value);
  }
  free( value);
  
  return_obj = (*env)->NewObject( env, cdhrIntArray_id,
  	cdhrIntArray_cid, jvalue, jsts);
  return return_obj; 
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getObjectInfoString
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  jclass cdhrString_id;
  static jmethodID cdhrString_cid = NULL;
  jobject return_obj;
  jint jsts;
  jstring jvalue = NULL;
  char buf[256];
  char value[256];
  char *s;
  pwr_sAttrRef attrref;
  int len;
  pwr_tTypeId a_type;
  unsigned int a_size, a_offs, a_dim;

  cdhrString_id = (*env)->FindClass( env, "jpwr/rt/CdhrString");
  if(cdhrString_cid == NULL)
  {
    cdhrString_cid = (*env)->GetMethodID( env, cdhrString_id,
    	"<init>", "(Ljava/lang/String;I)V");
    //printf("cdhrString_cid initierad\n");
  }

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = strchr( cstr, '#')))
    *s = 0;

  sts = gdh_NameToAttrref( pwr_cNObjid, cstr, &attrref);
  if ( ODD(sts)) {
    sts = gdh_GetObjectInfoAttrref( &attrref, buf, sizeof(buf));
    (*env)->ReleaseStringUTFChars( env, name, cstr);

    if ( ODD(sts)) {
      sts = gdh_GetAttributeCharAttrref( &attrref, &a_type, &a_size, &a_offs, &a_dim);
      if ( ODD(sts)) {
        gdh_AttrToString( a_type, (void *)buf, value, sizeof(value), 
			  &len, NULL);
        jvalue = (*env)->NewStringUTF( env, value);
      }
    }
  }
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrString_id,
  	cdhrString_cid, jvalue, jsts);
  return return_obj; 
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getObjectInfoObjid
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  pwr_tObjid objid;
  jclass cdhrObjid_id;
  jmethodID cdhrObjid_cid;
  jclass PwrtObjid_id;
  jmethodID PwrtObjid_cid;
  jobject objid_obj = NULL;
  jint oix, vid;
  jobject return_obj;
  jint jsts;
  pwr_sAttrRef attrref;
  char *s;

  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;I)V");

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	"<init>", "(II)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  if ( (s = strchr( cstr, '#'))) {
    *s = 0;
    if ( (s = strrchr( ++s, '[')))
      strcat( cstr, s);
  }
  printf( "RefObjid: %s\n", cstr);

  sts = gdh_NameToAttrref( pwr_cNObjid, cstr, &attrref);
  if ( ODD(sts)) {
    sts = gdh_GetObjectInfoAttrref( &attrref, &objid, sizeof(objid));
    (*env)->ReleaseStringUTFChars( env, name, cstr);

    if ( ODD(sts)) {
      oix = (jint) objid.oix;
      vid = (jint) objid.vid;
      objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, 
				     oix, vid);
    }
  }  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, objid_obj, jsts);
  return return_obj; 
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getNodeObject
  (JNIEnv *env, jclass obj, jint jnix)
{
  int		sts;
  jclass 	PwrtObjid_id;
  jmethodID 	PwrtObjid_cid;
  pwr_tObjid 	objid;
  jclass 	cdhrObjid_id;
  jmethodID 	cdhrObjid_cid;
  jobject 	objid_obj = NULL;
  jint 		oix, vid;
  jobject 	return_obj;
  jint 		jsts;
  int           nix;

  cdhrObjid_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjid");
  cdhrObjid_cid = (*env)->GetMethodID( env, cdhrObjid_id,
    	"<init>", "(Ljpwr/rt/PwrtObjid;I)V");

  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id,
    	"<init>", "(II)V");

  nix = jnix;
  sts = gdh_GetNodeObject( nix, &objid);
  if ( ODD(sts))
  {
    oix = (jint) objid.oix;
    vid = (jint) objid.vid;
    objid_obj = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrObjid_id,
  	cdhrObjid_cid, objid_obj, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getAttributeChar
  (JNIEnv *env, jobject obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  jobject return_obj;
  jint jsts;
  jclass gdhrGetAttributeChar_id;
  jmethodID gdhrGetAttributeChar_cid;
  unsigned int size;
  unsigned int elements;
  unsigned int offset;
  pwr_eType type_id;

  gdhrGetAttributeChar_id = (*env)->FindClass( env, 
		    "jpwr/rt/GdhrGetAttributeChar");
  gdhrGetAttributeChar_cid = (*env)->GetMethodID( env, 
        gdhrGetAttributeChar_id, "<init>", "(IIIII)V");

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;  
  gdh_ConvertUTFstring( cstr, cstr);

  sts = gdh_GetAttributeCharacteristics( cstr, &type_id, &size, &offset,
					 &elements);
  (*env)->ReleaseStringUTFChars( env, name, cstr);

  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, gdhrGetAttributeChar_id,
  	gdhrGetAttributeChar_cid, (jint)type_id, (jint)size, 
	(jint)offset, (jint)elements, jsts);
  return return_obj;
}

JNIEXPORT jstring JNICALL Java_jpwr_rt_Gdh_translateFilename
  (JNIEnv *env, jclass obj, jstring filename)
{
  jstring jvalue;
  int sts;
  const char *str;
  char *cstr;
  char buffer[256];

  str = (*env)->GetStringUTFChars( env, filename, 0);
  cstr = (char *)str;

  printf( "translate: %s\n", str);
  sts = dcli_translate_filename( buffer, cstr);
  printf( "result: %s \n", buffer);
  (*env)->ReleaseStringUTFChars( env, filename, cstr);

  jvalue = (*env)->NewStringUTF( env, buffer);
  return jvalue;
}

static void  gdh_TranslateSuffixToClassData (
    char        *SuffixPtr,
    pwr_eType 	*PwrType,
    int		*PwrSize,
    int  	*NoOfElements
    )
{

  static const  gdh_sSuffixTable   XlationTbl[] = {
    {"BOOLEAN",pwr_eType_Boolean, sizeof(pwr_tBoolean)},
    {"FLOAT32",pwr_eType_Float32, sizeof(pwr_tFloat32)},
    {"FLOAT64",pwr_eType_Float64, sizeof(pwr_tFloat64)},
    {"CHAR"   ,pwr_eType_Char,    sizeof(pwr_tChar)},
    {"INT8"   ,pwr_eType_Int8,    sizeof(pwr_tInt8)},
    {"INT16"  ,pwr_eType_Int16,   sizeof(pwr_tInt16)},
    {"INT32"  ,pwr_eType_Int32,   sizeof(pwr_tInt32)},
    {"INT64"  ,pwr_eType_Int64,   sizeof(pwr_tInt64)},
    {"UINT8"  ,pwr_eType_UInt8,   sizeof(pwr_tUInt8)},
    {"UINT16" ,pwr_eType_UInt16,  sizeof(pwr_tInt16)},
    {"UINT32" ,pwr_eType_UInt32,  sizeof(pwr_tInt32)},
    {"UINT64" ,pwr_eType_UInt64,  sizeof(pwr_tInt64)},
    {"OBJID"  ,pwr_eType_Objid,   sizeof(pwr_tObjid)},
//    {"STRING" ,pwr_eType_String,  sizeof(void *)},
    {"TIME"   ,pwr_eType_Time,    sizeof(pwr_tTime)},
    {"DELTATIME" ,pwr_eType_DeltaTime, sizeof(pwr_tDeltaTime)},
    {"ATTRREF" ,pwr_eType_AttrRef, sizeof(pwr_sAttrRef)},
    {"STATUS" ,pwr_eType_Status, sizeof(pwr_tStatus)},
    {"NETSTATUS" ,pwr_eType_NetStatus, sizeof(pwr_tNetStatus)},
    {"BIT" ,graph_eType_Bit, sizeof(pwr_tBit)}
  };

  static const int    XlationTblLen = sizeof(XlationTbl)/sizeof(XlationTbl[0]);
  int                 Index;
  char                *Ptr;
  pwr_tBoolean        Found;

  /* Check if there is a array size */

  if ((Ptr = (char *)strchr(SuffixPtr,'#')) != 0)
  {
    *(Ptr++)='\0';
    if ( ! strchr( Ptr, '['))
      *NoOfElements = atoi(Ptr);
    else
      *NoOfElements = 1;
    --Ptr;
  }
  else
    *NoOfElements = 1;

  /* Default to float if you can't find it */

  *PwrType = pwr_eType_Float32;
  *PwrSize = 4;

  for (Index = 0, Found = FALSE; Index < XlationTblLen; Index++)
    if (!cdh_NoCaseStrcmp(XlationTbl[Index].TypeStr,SuffixPtr))
    {
      *PwrSize = XlationTbl[Index].Size;
      *PwrType = XlationTbl[Index].Type;
      Found = TRUE;
      break;
    }

  if (!Found && !cdh_NoCaseStrncmp("STRING", SuffixPtr, 6))
  {
    *PwrSize = atoi(SuffixPtr + 6);
    *PwrType = pwr_eType_String;
  }

  if ( Ptr != NULL && *NoOfElements > 1)
  {
    *PwrSize *= *NoOfElements;
    *Ptr = '#';
  }
}

static int gdh_ExtractNameSuffix(	char   *Name,
                          		char   **Suffix)
{
  char    	*TempPtr, *s;
  static char	Str[80];

  if ((TempPtr = (char *)strstr(Name,"##")) == NULL) /* if not found */
  {
    if (Suffix != NULL)
      *Suffix = NULL;
    return 0;
  }
  /* Continue here if found */
  *TempPtr = 0;

  TempPtr+=2;

  if (Suffix != NULL)
  {
    cdh_ToUpper(Str,TempPtr);
    *Suffix = Str;
  }

  /* Add array index to name */
  if ( (s = strchr( TempPtr + 1, '[')))
    strcat( Name, s);

  return 1;
}

static int gdh_StringToAttr( char *str_value, char *buffer_p, int buffer_size,
	pwr_eType attrtype, int attrsize)
{
  int sts;
  
  switch ( attrtype)
  {
    case pwr_eType_Boolean:
      if ( sscanf( str_value, "%d", (pwr_tBoolean *)buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_Float32:
      if ( sscanf( str_value, "%f", (pwr_tFloat32 *)buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_Float64:
    {
      pwr_tFloat32 f;
      pwr_tFloat64 d;
      if ( sscanf( str_value, "%f", &f) != 1)
        return GDH__BADARG;
      d = f;
      memcpy( buffer_p, (char *) &d, sizeof(d));
    }
    case pwr_eType_Char:
      if ( sscanf( str_value, "%c", buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_Int8:
    {
      pwr_tInt8	  vint8;
      pwr_tInt16  vint16;
      if ( sscanf( str_value, "%hd", &vint16) != 1)
        return GDH__BADARG;
      vint8 = vint16;
      memcpy( buffer_p, (char *)&vint8, sizeof(vint8));
      break;
    }
    case pwr_eType_Int16:
      if ( sscanf( str_value, "%hd", (pwr_tInt16 *)buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_Int32:
      if ( sscanf( str_value, "%d", (pwr_tInt32 *)buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_Int64:
      if ( sscanf( str_value, pwr_dFormatInt64, (pwr_tInt64 *)buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_UInt8:
    {
      pwr_tUInt8   vint8;
      pwr_tUInt16  vint16;
      if ( sscanf( str_value, "%hu", &vint16) != 1)
        return GDH__BADARG;
      vint8 = vint16;
      memcpy( buffer_p, (char *)&vint8, sizeof(vint8));
      break;
    }
    case pwr_eType_UInt16:
      if ( sscanf( str_value, "%hu", (pwr_tUInt16 *)buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_UInt32:
    case pwr_eType_Enum:
    case pwr_eType_Mask:
    case pwr_eType_Status:
    case pwr_eType_NetStatus:
      if ( sscanf( str_value, "%u", (pwr_tUInt32 *)buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_UInt64:
      if ( sscanf( str_value, pwr_dFormatUInt64, (pwr_tUInt64 *)buffer_p) != 1)
        return GDH__BADARG;
      break;
    case pwr_eType_String:
      strncpy( buffer_p, str_value, buffer_size);
      break;
    case pwr_eType_Objid:
    {
      pwr_tObjid objid;
	
      sts = gdh_NameToObjid( str_value, &objid);
      if (EVEN(sts)) return sts;

      memcpy( buffer_p, &objid, sizeof(objid));
      break;
    }  
    case pwr_eType_TypeId:
    {
      pwr_tTypeId typeid;
      pwr_tObjid objid;

      sts = gdh_NameToObjid ( str_value, &objid);
      if (EVEN(sts)) return sts;

      typeid = cdh_TypeObjidToId( objid);
      memcpy( buffer_p, (char *) &typeid, sizeof(typeid));
      break;
    }
    case pwr_eType_ObjectIx:
    {
      pwr_tObjectIx objectix;

      sts = cdh_StringToObjectIx( str_value, &objectix);
      if (EVEN(sts)) return sts;

      memcpy( buffer_p, (char *) &objectix, sizeof(objectix));
      break;
    }
    case pwr_eType_VolumeId:
    {
      pwr_tVolumeId volumeid;
      
      sts = cdh_StringToVolumeId( str_value, &volumeid);
      if (EVEN(sts)) return sts;

      memcpy( buffer_p, (char *) &volumeid, sizeof(volumeid));
      break;
    }
    case pwr_eType_RefId:
    {
      pwr_tRefId subid;
      
      sts = cdh_StringToSubid( str_value, &subid);
      if (EVEN(sts)) return sts;

      memcpy( buffer_p, (char *) &subid, sizeof(subid));
      break;
    }
    case pwr_eType_AttrRef:
    {
      pwr_sAttrRef attrref;
      
      sts = gdh_NameToAttrref ( pwr_cNObjid, str_value, &attrref);
      if (EVEN(sts)) return sts;

      memcpy( buffer_p, &attrref, sizeof(attrref));
      break;
    }
    case pwr_eType_Time:
    {
      pwr_tTime time;
      
      sts = time_AsciiToA( str_value, &time);
      if (EVEN(sts)) return GDH__BADARG;

      memcpy( buffer_p, (char *) &time, sizeof(time));
      break;
    }
    case pwr_eType_DeltaTime:
    {
      pwr_tDeltaTime deltatime;
      
      sts = time_AsciiToD( str_value, &deltatime);
      if (EVEN(sts)) return GDH__BADARG;

      memcpy( buffer_p, (char *) &deltatime, sizeof(deltatime));
      break;
    }
    default:
      ;
  }
  return GDH__SUCCESS;
}  

static void gdh_AttrToString( int type_id, void *value_ptr,
        char *str, int size, int *len, char *format)
{
  pwr_tObjid            objid;
  pwr_sAttrRef          *attrref;
  int                   sts;
  char                  timstr[64];

  if ( value_ptr == 0)
  {
    strcpy( str, "UNDEFINED");
    return;
  }

  switch ( type_id )
  {
    case pwr_eType_Boolean:
    {
      if ( !format)
        *len = sprintf( str, "%d", *(pwr_tBoolean *)value_ptr);
      else
        *len = sprintf( str, format, *(pwr_tBoolean *)value_ptr);
      break;
    }
    case pwr_eType_Float32:
    {
      if ( !format)
        *len = sprintf( str, "%f", *(float *)value_ptr);
      else
        *len = sprintf( str, format, *(float *)value_ptr);
      break;
    }
    case pwr_eType_Float64:
    {
      if ( !format)
        *len = sprintf( str, "%f", *(double *)value_ptr);
      else
        *len = sprintf( str, format, *(double *)value_ptr);
      break;
    }
    case pwr_eType_Char:
    {
      if ( !format)
        *len = sprintf( str, "%c", *(char *)value_ptr);
      else
        *len = sprintf( str, format, *(char *)value_ptr);
      break;
    }
    case pwr_eType_Int8:
    {
      if ( !format)
        *len = sprintf( str, "%d", *(char *)value_ptr);
      else
        *len = sprintf( str, format, *(char *)value_ptr);
      break;
    }
    case pwr_eType_Int16:
    {
      if ( !format)
        *len = sprintf( str, "%hd", *(short *)value_ptr);
      else
        *len = sprintf( str, format, *(short *)value_ptr);
      break;
    }
    case pwr_eType_Int32:
    {
      if ( !format)
        *len = sprintf( str, "%d", *(int *)value_ptr);
      else
        *len = sprintf( str, format, *(int *)value_ptr);
      break;
    }
    case pwr_eType_Int64:
    {
      if ( !format)
        *len = sprintf( str, pwr_dFormatInt64, *(pwr_tInt64 *)value_ptr);
      else
        *len = sprintf( str, format, *(pwr_tInt64 *)value_ptr);
      break;
    }
    case pwr_eType_UInt8:
    {
      if ( !format)
        *len = sprintf( str, "%d", *(unsigned char *)value_ptr);
      else
        *len = sprintf( str, format, *(unsigned char *)value_ptr);
      break;
    }
    case pwr_eType_UInt16:
    {
      if ( !format)
        *len = sprintf( str, "%hd", *(unsigned short *)value_ptr);
      else
        *len = sprintf( str, format, *(unsigned short *)value_ptr);
      break;
    }
    case pwr_eType_UInt32:
    case pwr_eType_Enum:
    case pwr_eType_Mask:
    case pwr_eType_Status:
    case pwr_eType_NetStatus:
    {
      if ( !format)
        *len = sprintf( str, "%d", *(unsigned int *)value_ptr);
      else
        *len = sprintf( str, format, *(unsigned int *)value_ptr);
      break;
    }
    case pwr_eType_UInt64:
    {
      if ( !format)
        *len = sprintf( str, pwr_dFormatUInt64, *(pwr_tUInt64 *)value_ptr);
      else
        *len = sprintf( str, format, *(pwr_tUInt64 *)value_ptr);
      break;
    }
    case pwr_eType_String:
    {
      strncpy( str, (char *)value_ptr, size);
      str[size-1] = 0;
      *len = strlen(str);
      break;
    }
    case pwr_eType_Objid:
    {
      pwr_tOName            hiername;

      objid = *(pwr_tObjid *)value_ptr;
      if ( !objid.oix)
        sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername),
                         cdh_mName_volumeStrict);
      else
        sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername),
                cdh_mNName);
      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
        break;
      }
      *len = sprintf( str, "%s", hiername);
      break;
    }
    case pwr_eType_AttrRef:
    {
      pwr_tAName            hiername;

      attrref = (pwr_sAttrRef *) value_ptr;
      sts = gdh_AttrrefToName ( attrref, hiername, sizeof(hiername), 
         cdh_mNName);
      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
        break;
      }
      *len = sprintf( str, "%s", hiername);
      break;
    }
    case pwr_eType_Time:
    {
      sts = time_AtoAscii( (pwr_tTime *) value_ptr, time_eFormat_DateAndTime,
                timstr, sizeof(timstr));
      if ( EVEN(sts))
        strcpy( timstr, "-");
      *len = sprintf( str, "%s", timstr);
      break;
    }
    case pwr_eType_DeltaTime:
    {
      sts = time_DtoAscii( (pwr_tDeltaTime *) value_ptr, 1,
                timstr, sizeof(timstr));
      if ( EVEN(sts))
        strcpy( timstr, "Undefined time");
      *len = sprintf( str, "%s", timstr);
      break;
    }
    case pwr_eType_ObjectIx:
    {
      *len = sprintf( str, "%s", cdh_ObjectIxToString( NULL,
                *(pwr_tObjectIx *) value_ptr, 1));
      break;
    }
    case pwr_eType_ClassId:
    {
      pwr_tOName            hiername;

      objid = cdh_ClassIdToObjid( *(pwr_tClassId *) value_ptr);
      sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
        break;
      }
      *len = sprintf( str, "%s", hiername);
      break;
    }
    case pwr_eType_TypeId:
    {
      pwr_tOName            hiername;

      objid = cdh_TypeIdToObjid( *(pwr_tTypeId *) value_ptr);
      sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
      if (EVEN(sts))
      {
        strcpy( str, "");
        *len = 0;
        break;
      }
      *len = sprintf( str, "%s", hiername);
      break;
    }
    case pwr_eType_VolumeId:
    {
      *len = sprintf( str, "%s", cdh_VolumeIdToString( NULL,
                *(pwr_tVolumeId *) value_ptr, 1, 0));
      break;
    }
    case pwr_eType_RefId:
    {
      *len = sprintf( str, "%s", cdh_SubidToString( NULL,
                *(pwr_tSubid *) value_ptr, 1));
      break;
    }
  }
}

static void gdh_ConvertUTFstring( char *out, char *in)
{
  char *s, *t;

  s = in;
  t = out;
  while ( *s)
  {
    if ( *s == -61)
    {
      if ( *(s+1) == -91)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -92)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -74)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -123)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -124)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -106)
      {
        *t = '�';
        s++;
      }
      else
        *t = *s;
    }   
    else
     *t = *s;
    s++;
    t++;
  }
  *t = 0;
}

/*author: Jonas Nylund */
JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getClassAttribute
  (JNIEnv *env, jclass obj, jint classid, jobject objid_obj)
{
  int		sts;
  pwr_tOName   	classname;
  pwr_tOName   	hiername;
  char		parname[80];
  pwr_tOName   	fullname;
  char		*s;
  pwr_sParInfo	parinfo;
  pwr_tObjid   	body;
  pwr_tObjid 	objid;
  pwr_tObjid   	parameter;
  pwr_tClassId	parameter_class;
  pwr_tClassId	cid;
  int		attr_exist;
  int		j;
  
  jclass 	PwrtObjid_id;
  static jmethodID 	PwrtObjid_getOix = NULL;
  static jmethodID 	PwrtObjid_getVid = NULL;
  static jmethodID 	PwrtObjid_cid = NULL;
  jclass 	cdhrObjAttr_id;
  static jmethodID 	cdhrObjAttr_cid = NULL;
  jobject       return_obj;
  jobject       jobjid = NULL;
  jint          jType = 0;
  jint          jSize = 0;
  jint          jFlags = 0;
  jint          jElements = 0;
  jstring       jparname; 
  jint          joix;
  jint          jvid;



  PwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  if(PwrtObjid_id == NULL) printf("Pwrtobjid_id �r NULL");
  
  if(PwrtObjid_cid == NULL || PwrtObjid_getOix == NULL || PwrtObjid_getVid == NULL)
  {
    PwrtObjid_cid = (*env)->GetMethodID( env, PwrtObjid_id, "<init>", "(II)V");
    if(PwrtObjid_cid == NULL) printf("Pwrtobjid_cid �r NULL");

    PwrtObjid_getOix = (*env)->GetMethodID( env, PwrtObjid_id, "getOix", "()I");
    if(PwrtObjid_getOix == NULL) printf("Pwrtobjid_getOix �r NULL");
    PwrtObjid_getVid = (*env)->GetMethodID( env, PwrtObjid_id, "getVid", "()I");
    if(PwrtObjid_getVid == NULL) printf("Pwrtobjid_getVid �r NULL");
    //printf("initierat Pwrtobjid metodIds\n");
  }
  
  cdhrObjAttr_id = (*env)->FindClass( env, "jpwr/rt/CdhrObjAttr");
  if(cdhrObjAttr_id == NULL) printf("cdhrObjAttr_id �r NULL");
  
  if(cdhrObjAttr_cid == NULL)
  { 
    cdhrObjAttr_cid = (*env)->GetMethodID( env, cdhrObjAttr_id,
    	  "<init>", "(Ljpwr/rt/PwrtObjid;Ljava/lang/String;IIII)V");
    //printf("initierat cdhrObjAttr_cid metodId\n");
    if(cdhrObjAttr_cid == NULL) printf("cdhrObjAttr_cid �r NULL");
  }

 
  cid = (pwr_tClassId)classid;  
  if(objid_obj != NULL)
  {
    objid.oix = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getOix);
    objid.vid = (*env)->CallIntMethod( env, objid_obj, PwrtObjid_getVid);
  }
  
  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(cid), classname,
		sizeof(classname), cdh_mName_volumeStrict);
  if ( EVEN(sts))
  {
    printf("return NULL after objidtoname\n");
    return NULL;
  }


  attr_exist = 0;
  for ( j = 0; j < 2; j++)
  {
    strcpy( hiername, classname);
    if ( j == 0)
      strcat( hiername, "-RtBody");
    else
      strcat( hiername, "-SysBody");
      
    sts = gdh_NameToObjid( hiername, &body);
    if ( EVEN(sts))
    { 
      continue;
    }
      
    /* if we dont have a objid then we must find the child*/
    if(!(objid.oix + objid.vid))
    { 
      sts = gdh_GetChild( body, &parameter);
    }
    /* we have the objid for a child to the class then we want the next
       sibling*/ 
    else
    {
      sts = gdh_GetNextSibling ( objid, &parameter);
    }
    while ( ODD(sts))
    {
      sts = gdh_ObjidToName ( parameter, hiername, sizeof(hiername),
			cdh_mName_volumeStrict);
      if ( EVEN(sts))
      {
        printf("return NULL after objidtoname 2\n");
        return NULL;
      }


      /* Skip hierarchy */
      s = strrchr( hiername, '-');
      if ( s == 0)
        strcpy( parname, hiername);
      else
        strcpy( parname, s + 1);

      /* Get parameter info for this parameter */
      strcpy( fullname, hiername);
      sts = gdh_GetObjectInfo( fullname, &parinfo, sizeof(parinfo));
      if (EVEN(sts))
      {
        printf("return NULL after getobjectinfo\n");
        return NULL;
      }
      sts = gdh_GetObjectClass( parameter, &parameter_class);
      if ( EVEN(sts))
      {
        printf("return NULL after getobjecclass 2\n");
        return NULL;
      }


      if ( parinfo.Flags & PWR_MASK_RTVIRTUAL || 
           parinfo.Flags & PWR_MASK_PRIVATE)
      {
        /* This parameter does not contain any useful information, take the
	    next one */
        sts = gdh_GetNextSibling ( parameter, &parameter);
	continue;
      }
      jType = (jint)parinfo.Type;
      jSize = (jint)parinfo.Size;
      jFlags = (jint)parinfo.Flags;
      jElements = (jint)parinfo.Elements;
      jparname = (*env)->NewStringUTF( env, parname);
      joix = (jint) parameter.oix;
      jvid = (jint) parameter.vid;
      jobjid = (*env)->NewObject( env, PwrtObjid_id, PwrtObjid_cid, joix, jvid);
      
      /*make a new javaobject CdhrObjAttr*/
      return_obj = (*env)->NewObject(env, 
	                             cdhrObjAttr_id,
  	                             cdhrObjAttr_cid, 
				     jobjid,
				     jparname,
				     jType, 
				     jSize,
				     jFlags,
				     jElements);
      return return_obj;
    }
  }

  return NULL;
}

static void gdh_crr_insert_cb( void *ctx, void *parent_node, 
				navc_eItemType item_type,
				char *text1, char *text2, int write)
{
  char *buf = (char *)ctx;

  switch( item_type) {
    case navc_eItemType_Crossref:
      if ( strcmp( buf, "") != 0)
        strcat( buf, "\n");
      switch ( write) {
      case 0: 
        strcat( buf, "0");
	break;
      case 1:
	strcat( buf, "1");
	break;
      case 2:
	strcat( buf, "2");
	break;
      }
      printf( "Insert %s %s\n", text1, text2);
      strcat( buf, text1);
      strcat( buf, "  ");
      strcat( buf, text2);
      // new ItemCrossref( brow, text1, text2, 
      //		write, parent_node, flow_eDest_IntoLast);
      break;
    case navc_eItemType_Header:
      // new ItemHeader( brow, "crr", text1, parent_node, flow_eDest_IntoLast);
      break;
    case navc_eItemType_Text:
      // new ItemText( brow, "crr", text1, parent_node, flow_eDest_IntoLast);
      break;
  }
}

static int gdh_crr_name_to_objid_cb( void *ctx, char *name, pwr_tObjid *objid)
{
  int sts;
  sts =  gdh_NameToObjid( name, objid);
  printf( "name_to_objid_cb: %s %d\n", name, sts);
  return sts;
}

static int gdh_crr_get_volume_cb( void *ctx, pwr_tVolumeId *volid)
{
  int sts;
  pwr_tObjid objid;

  sts = gdh_GetNodeObject( 0, &objid);
  if ( EVEN(sts)) return sts;

  *volid = objid.vid;
  return GDH__SUCCESS;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_crrObject
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  jobject return_obj;
  jint jsts;
  jstring	jbuf = NULL;
  char          *buf;
  jclass cdhrString_id;
  static jmethodID cdhrString_cid = NULL;

  cdhrString_id = (*env)->FindClass( env, "jpwr/rt/CdhrString");
  if(cdhrString_cid == NULL)
  {
    cdhrString_cid = (*env)->GetMethodID( env, cdhrString_id,
    	  "<init>", "(Ljava/lang/String;I)V");
    //printf("cdhrString_cid initierad\n");
  }

  buf = (char *)calloc( 1, 2000);

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  printf( "crrObject name: %s %s\n", str, cstr);
  sts = crr_object( buf, cstr, gdh_crr_insert_cb, gdh_crr_name_to_objid_cb,
		    gdh_crr_get_volume_cb);
  (*env)->ReleaseStringUTFChars( env, name, cstr);

  if ( ODD(sts))
    jbuf = (*env)->NewStringUTF( env, buf);
  
  free( buf);

  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrString_id,
  	cdhrString_cid, jbuf, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_crrSignal
  (JNIEnv *env, jclass obj, jstring name)
{
  int sts;
  const char *str;
  char *cstr;
  jobject return_obj;
  jint jsts;
  jstring	jbuf = NULL;
  char          *buf;
  jclass cdhrString_id;
  static jmethodID cdhrString_cid = NULL;

  cdhrString_id = (*env)->FindClass( env, "jpwr/rt/CdhrString");
  if(cdhrString_cid == NULL)
  {
    cdhrString_cid = (*env)->GetMethodID( env, cdhrString_id,
    	  "<init>", "(Ljava/lang/String;I)V");
    //printf("cdhrString_cid initierad\n");
  }

  buf = (char *)calloc( 1, 2000);

  str = (*env)->GetStringUTFChars( env, name, 0);
  cstr = (char *)str;
  gdh_ConvertUTFstring( cstr, cstr);
  printf( "crrObject name: %s %s\n", str, cstr);
  sts = crr_signal( buf, cstr, gdh_crr_insert_cb, gdh_crr_name_to_objid_cb,
		    gdh_crr_get_volume_cb);
  (*env)->ReleaseStringUTFChars( env, name, cstr);

  if ( ODD(sts))
    jbuf = (*env)->NewStringUTF( env, buf);
  
  free( buf);

  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrString_id,
  	cdhrString_cid, jbuf, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getMsg
  (JNIEnv *env, jclass obj, jint sts)
{
  int status;
  jobject return_obj;
  jint jsts;
  jstring	jbuf = NULL;
  char          buf[200];
  jclass cdhrString_id;
  static jmethodID cdhrString_cid = NULL;

  cdhrString_id = (*env)->FindClass( env, "jpwr/rt/CdhrString");
  if(cdhrString_cid == NULL)
  {
    cdhrString_cid = (*env)->GetMethodID( env, cdhrString_id,
    	  "<init>", "(Ljava/lang/String;I)V");
    //printf("cdhrString_cid initierad\n");
  }

  status = (pwr_tStatus) sts;

  msg_GetMsg( status, buf, sizeof(buf));

  jbuf = (*env)->NewStringUTF( env, buf);
  jsts = (jint) GDH__SUCCESS;
  return_obj = (*env)->NewObject( env, cdhrString_id,
  	cdhrString_cid, jbuf, jsts);
  return return_obj;
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getMsgText
  (JNIEnv *env, jclass obj, jint sts)
{
  int status;
  jobject return_obj;
  jint jsts;
  jstring	jbuf = NULL;
  char          buf[200];
  jclass cdhrString_id;
  static jmethodID cdhrString_cid = NULL;

  cdhrString_id = (*env)->FindClass( env, "jpwr/rt/CdhrString");
  if(cdhrString_cid == NULL)
  {
    cdhrString_cid = (*env)->GetMethodID( env, cdhrString_id,
    	  "<init>", "(Ljava/lang/String;I)V");
    //printf("cdhrString_cid initierad\n");
  }

  status = (pwr_tStatus) sts;

  if ( status == 0)
    strcpy( buf, "");
  else
    msg_GetText( status, buf, sizeof(buf));

  jbuf = (*env)->NewStringUTF( env, buf);
  jsts = (jint) GDH__SUCCESS;
  return_obj = (*env)->NewObject( env, cdhrString_id,
  	cdhrString_cid, jbuf, jsts);
  return return_obj;
}


JNIEXPORT jobject JNICALL Java_jpwr_rt_Gdh_getSuperClass
  (JNIEnv *env, jclass obj, jint classid, jobject objid_obj)
{
  int		sts;
  jclass 	pwrtObjid_id;
  static jmethodID 	pwrtObjid_getOix = NULL;
  static jmethodID 	pwrtObjid_getVid = NULL;
  static jmethodID 	pwrtObjid_cid = NULL;
  pwr_tObjid 	objid;
  jclass 	cdhrClassId_id;
  static jmethodID 	cdhrClassId_cid;
  jint	 	jsupercid = 0;
  jobject 	return_obj;
  jint 		jsts;
  pwr_tClassId	cid, supercid;

  cdhrClassId_id = (*env)->FindClass( env, "jpwr/rt/CdhrClassId");
  if(cdhrClassId_cid == NULL)
  {
    cdhrClassId_cid = (*env)->GetMethodID( env, cdhrClassId_id,
    	  "<init>", "(II)V");
    //printf("cdhrClassId_cid initierad\n");
  } 

  pwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  if(pwrtObjid_cid == NULL || pwrtObjid_getOix == NULL || pwrtObjid_getVid == NULL)
  {
    pwrtObjid_cid = (*env)->GetMethodID( env, pwrtObjid_id,
    	  "<init>", "(II)V");
    pwrtObjid_getOix = (*env)->GetMethodID( env, pwrtObjid_id, "getOix", "()I");
    pwrtObjid_getVid = (*env)->GetMethodID( env, pwrtObjid_id, "getVid", "()I");
  }

  cid = (pwr_tCid) classid;

  if ( objid_obj != 0) {
    objid.oix = (*env)->CallIntMethod( env, objid_obj, pwrtObjid_getOix);
    objid.vid = (*env)->CallIntMethod( env, objid_obj, pwrtObjid_getVid);

    sts = gdh_GetSuperClass( cid, &supercid, objid);
  }
  else
    sts = gdh_GetSuperClass( cid, &supercid, pwr_cNObjid);

  if ( ODD(sts)) {
    jsupercid = (jint)supercid;
  }
  
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrClassId_id,
  	cdhrClassId_cid, jsupercid, jsts);
  return return_obj;

}

JNIEXPORT jobjectArray JNICALL Java_jpwr_rt_Gdh_getObjectBodyDef
  (JNIEnv *env, jobject obj, jint classid, jobject objid_obj)
{
  int		sts,i;
  int j = 0;
  jclass                pwrsParInfo_id;
  static jmethodID      pwrsParInfo_cid = NULL;

  jclass                gdhrsAttrDef_id;
  static jmethodID      gdhrsAttrDef_cid = NULL;

  //  jclass 	        cdhrClassId_id;
  //  static jmethodID 	cdhrClassId_cid;

  jclass 	        pwrtObjid_id;
  static jmethodID 	pwrtObjid_getOix = NULL;
  static jmethodID 	pwrtObjid_getVid = NULL;
  static jmethodID 	pwrtObjid_cid = NULL;
  pwr_tObjid 	        objid = pwr_cNObjid;

  jobjectArray 	        gdhrsAttrDefArr = NULL;
  jobject               gdhrsAttrDef;
  jobject 	        pwrsParInfo;
  //  jint 		        jsts;
  pwr_tClassId	        cid;

  gdh_sAttrDef *bd;
  int rows;
  pwr_sAttrRef aref;
  pwr_sAttrRef aaref;
  pwr_tDisableAttr disabled;

  pwrtObjid_id = (*env)->FindClass( env, "jpwr/rt/PwrtObjid");
  if(pwrtObjid_cid == NULL || pwrtObjid_getOix == NULL || pwrtObjid_getVid == NULL)
  {
    pwrtObjid_cid = (*env)->GetMethodID( env, pwrtObjid_id,
    	  "<init>", "(II)V");
    pwrtObjid_getOix = (*env)->GetMethodID( env, pwrtObjid_id, "getOix", "()I");
    pwrtObjid_getVid = (*env)->GetMethodID( env, pwrtObjid_id, "getVid", "()I");
  }

  //find the class for PwrsParInfo
  pwrsParInfo_id = (*env)->FindClass(env, "jpwr/rt/PwrsParInfo");
  gdhrsAttrDef_id = (*env)->FindClass(env, "jpwr/rt/GdhrsAttrDef");
  if(pwrsParInfo_id == NULL || gdhrsAttrDef_id == NULL)
  {
    printf("Fel vid FindClass getObjectBodyDef\n");
    return (jobjectArray)NULL;
  }
  if(pwrsParInfo_cid == NULL)
  {
    pwrsParInfo_cid = (*env)->GetMethodID( env, pwrsParInfo_id,
    	  "<init>", "(Ljava/lang/String;IIIIII)V");
  }
  if(gdhrsAttrDef_cid == NULL)
  {
    gdhrsAttrDef_cid = (*env)->GetMethodID( env, gdhrsAttrDef_id,
    	  "<init>", "(Ljava/lang/String;IIIILjpwr/rt/PwrsParInfo;I)V");
  }
  if(pwrsParInfo_cid == NULL || gdhrsAttrDef_cid == NULL)
  {
    printf("Fel vid GetMethodId getObjectBodyDef\n");
    return (jobjectArray)NULL;
  }
  if ( objid_obj != 0) {

    
    objid.oix = (*env)->CallIntMethod( env, objid_obj, pwrtObjid_getOix);
    objid.vid = (*env)->CallIntMethod( env, objid_obj, pwrtObjid_getVid);

    sts = gdh_GetObjectClass(objid, &cid);
    if(EVEN(sts))
    {
      printf("Fel fr�n GetObjectClass %d\n", sts);
      //return (jobjectArray)NULL;
    }
    

    sts = gdh_GetObjectBodyDef( cid, &bd, &rows, objid);
    if(EVEN(sts))
    {
      printf("Fel fr�n GetObjectBodyDef %d\n", sts);
      return (jobjectArray)NULL;
    }

    //create a new GdhrsAttrDef[]
    gdhrsAttrDefArr = (*env)->NewObjectArray(env, (jint)rows, gdhrsAttrDef_id, NULL);

    for(i = 0;i < rows;i++)
    {
      if ( (bd[i].flags & gdh_mAttrDef_Shadowed) ||
           (bd[i].attr->Param.Info.Flags & PWR_MASK_RTHIDE) ||
           (bd[i].attr->Param.Info.Flags & PWR_MASK_RTVIRTUAL) || 
	   (bd[i].attr->Param.Info.Flags & PWR_MASK_PRIVATE) ||
           (bd[i].attr->Param.Info.Type == pwr_eType_CastId) ||
	   (bd[i].attr->Param.Info.Type == pwr_eType_DisableAttr) )
	continue;
      if(bd[i].attr->Param.Info.Flags & PWR_MASK_DISABLEATTR)
      { 
	aref = cdh_ObjidToAref( objid);

	sts = gdh_ArefANameToAref( &aref, bd[i].attrName, &aaref);
	if ( EVEN(sts)) printf("Fel fr�n ArefANameToAref %d\n", sts);


	sts = gdh_ArefDisabled( &aaref, &disabled);
	if ( EVEN(sts)) printf("Fel fr�n ArefDisabled %d\n", sts);

	if ( disabled)
	  continue;
      }

      pwrsParInfo = (*env)->NewObject( env, pwrsParInfo_id,
  	                               pwrsParInfo_cid,
				       NULL,
				       bd[i].attr->Param.Info.Type,
				       bd[i].attr->Param.Info.Offset,
				       bd[i].attr->Param.Info.Size,
				       bd[i].attr->Param.Info.Flags,
				       bd[i].attr->Param.Info.Elements,
				       bd[i].attr->Param.Info.ParamIndex);


    gdhrsAttrDef = (*env)->NewObject( env, gdhrsAttrDef_id,
  	                              gdhrsAttrDef_cid,
				      (*env)->NewStringUTF( env, (char *)bd[i].attrName),
				      (jint)bd[i].attrLevel,
				      (jint)bd[i].attrClass,
				      (jint)bd[i].flags,
				      (jint)(bd[i].attr->Param.TypeRef),
				      pwrsParInfo,
				      (jint)sts);

      (*env)->SetObjectArrayElement(env, gdhrsAttrDefArr, j, gdhrsAttrDef);
      j++;
    }
    free((char *)bd);
    return gdhrsAttrDefArr;


  }

  printf("Fel i getObjectBodyDef Objid �r 0\n");
  return (jobjectArray)NULL;
}

static int gdh_JidToPointer( int id, void **p)
{
#if defined OS_LINUX && defined HW_X86_64
  pwr_tStatus sts;
  sJid *jp;

  jp = tree_Find( &sts, jid_table, &id);
  if ( !jp) return 0;

  *p = jp->p;
  return 1;
#else
  *p = (void *)id;
  return 1;
#endif
}

static int gdh_JidStore( void *p, pwr_tRefId r, int *id)
{
#if defined OS_LINUX && defined HW_X86_64
  sJid *jp;
  pwr_tStatus sts;

  if ( !jid_table) {
    jid_table = tree_CreateTable( &sts, sizeof(int), offsetof(sJid, jid),
				  sizeof(sJid), 10, tree_Comp_int32);
    if ( EVEN(sts)) return sts;
  }

  *id = jid_next++;
  jp = tree_Insert(&sts, jid_table, id);
  if ( !jp) return sts;

  jp->p = p;
  jp->refid = r;

  printf( "Jid store: %d\n", *id);
  return 1;
#else
  *id = (int)p;
  return 1;
#endif
}

static int gdh_JidRemove( pwr_tRefId r)
{
#if defined OS_LINUX && defined HW_X86_64
  sJid *jp;
  pwr_tStatus sts;

  for (jp = tree_Minimum( &sts, jid_table); jp != NULL; jp = tree_Successor( &sts, jid_table, jp)) { 
    if ( jp->refid.nid == r.nid && jp->refid.rix == r.rix) {
      printf( "Jid remove: %d\n", jp->jid);
      tree_Remove( &sts, jid_table, jp);
      return 1;
    }
  }
  return 0;
#else
  return 1;
#endif
}

