#include "jpwr_rt_qcom.h"
#include "pwr.h"
#include "rt_qcom.h"
#include "co_cdh.h"
#include "co_time.h"
#include "co_cdh_msg.h"
#include "rt_qcom_msg.h"
#include "rt_errh.h"

#include "rt_pwr_msg.h"
#include "rt_ini_event.h"



JNIEXPORT jobject JNICALL Java_jpwr_rt_Qcom_createQ
  (JNIEnv *env, jobject object, jint qix, jint nid, jstring jname)
{
  jclass 	qcomrCreateQ_id;
  jmethodID 	qcomrCreateQ_cid;
  jobject 	return_obj;
  jint		jsts;
  qcom_sQid	qid;
  qcom_sQattr	attr;
  int		sts;
  char		*cstr;
  const char 	*name;

//  qcom_sQid other_que = qcom_cQapplEvent;

  qcomrCreateQ_id = (*env)->FindClass( env, "jpwr/rt/QcomrCreateQ");
  qcomrCreateQ_cid = (*env)->GetMethodID( env, qcomrCreateQ_id,
    	"<init>", "(III)V");

  name = (*env)->GetStringUTFChars( env, jname, 0);
  cstr = (char *)name;

  attr.type = qcom_eQtype_private;
  attr.quota = 100;
  qid.qix = qix;
  qid.nid = nid;
  qcom_CreateQ( &sts, &qid, &attr, cstr);
  (*env)->ReleaseStringUTFChars( env, jname, cstr);
  //printf( "Create que, qix %d, nid %d, sts %d\n", qid.qix, qid.nid, sts);
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, qcomrCreateQ_id,
  	qcomrCreateQ_cid, qid.qix, qid.nid, jsts);

  // Bind to broadcast que
  // qcom_Bind( &sts, &qid, &other_que);
  // if ( EVEN(sts))
  //   printf("** Unable to bind to qcom broadcast que\n");

  return return_obj;
}



/*
 * Class:     jpwr_rt_Qcom
 * Method:    createIniEventQ
 * Signature: ()Ljpwr/rt/QcomrCreateQ;
 */
JNIEXPORT jobject JNICALL Java_jpwr_rt_Qcom_createIniEventQ
  (JNIEnv *env, jobject object, jstring jname)
{

  jclass 	qcomrCreateQ_id;
  jmethodID 	qcomrCreateQ_cid;
  jobject 	return_obj;
  jint		jsts;
  qcom_sQid	qid;
  qcom_sQattr	qAttr;
  qcom_sQid     qini;
  int		sts;
  char		*cstr;
  const char 	*name;

  qcomrCreateQ_id = (*env)->FindClass( env, "jpwr/rt/QcomrCreateQ");
  qcomrCreateQ_cid = (*env)->GetMethodID( env, qcomrCreateQ_id,
    	"<init>", "(III)V");

  name = (*env)->GetStringUTFChars( env, jname, 0);
  cstr = (char *)name;


  //printf("%s\n", cstr);
  // Create a queue to receive stop and restart events
  if (!qcom_Init(&sts, 0, cstr)) {
    errh_Fatal("qcom_Init, %m", sts); 
    errh_SetStatus( PWR__APPLTERM);
    exit(sts);
  } 



  qAttr.type = qcom_eQtype_private;
  qAttr.quota = 100;
  if (!qcom_CreateQ(&sts, &qid, &qAttr, cstr)) {
    errh_Fatal("qcom_CreateQ, %m", sts);
    errh_SetStatus( PWR__APPLTERM);
    exit(sts);
  } 

  (*env)->ReleaseStringUTFChars( env, jname, cstr);


  //printf( "Create que, qix %d, nid %d, sts %d\n", qid.qix, qid.nid, sts);
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, qcomrCreateQ_id,
  	qcomrCreateQ_cid, qid.qix, qid.nid, jsts);


  qini = qcom_cQini;
  if (!qcom_Bind(&sts, &qid, &qini)) {
    errh_Fatal("qcom_Bind(Qini), %m", sts);
    errh_SetStatus( PWR__APPLTERM);
    exit(-1);
  }


  return return_obj;

}

/*
 * Class:     jpwr_rt_Qcom
 * Method:    getIniEvent
 * Signature: (III)Ljpwr/rt/QcomrGetIniEvent;
 */
JNIEXPORT jobject JNICALL Java_jpwr_rt_Qcom_getIniEvent
  (JNIEnv *env, jobject object, jint qix, jint nid, jint timeoutTime)
{
  jclass 	qcomrGetIniEvent_id;
  jmethodID 	qcomrGetIniEvent_cid;
  jobject 	return_obj;
  jint		jsts;
  jboolean jterminate = FALSE;
  jboolean jswapInit = FALSE;
  jboolean jswapDone = FALSE;
  jboolean jtimeout = FALSE;
  qcom_sQid     qid;


  int		sts;



  int tmo = timeoutTime;
  pwr_tBoolean terminate = FALSE;
  pwr_tBoolean swapInit = FALSE;
  pwr_tBoolean swapDone = FALSE;
  pwr_tBoolean timeout = FALSE;
  char mp[2000];
  qcom_sGet get;

  ini_mEvent  new_event;
  qcom_sEvent *ep;



  qcomrGetIniEvent_id = (*env)->FindClass( env, "jpwr/rt/QcomrGetIniEvent");
  qcomrGetIniEvent_cid = (*env)->GetMethodID( env, qcomrGetIniEvent_id,
    	"<init>", "(ZZZZI)V");

  qid.qix = qix;
  qid.nid = nid;



  get.maxSize = sizeof(mp);
  get.data = mp;
  qcom_Get( &sts, &qid, &get, tmo);
  if (sts == QCOM__TMO || sts == QCOM__QEMPTY) {    
    timeout = TRUE;
  } 
  else {
    ep = (qcom_sEvent*) get.data;

    new_event.m  = ep->mask;
    if (new_event.b.swapInit) {
      errh_SetStatus( PWR__APPLRESTART);
      swapInit = TRUE;
    } 
    else if (new_event.b.swapDone) {
      swapDone = TRUE;
      errh_SetStatus( PWR__ARUN);
    } 
    else if (new_event.b.terminate) {
      terminate = TRUE;
    }
  }

  jsts        = (jint) 1;
  jterminate  = (jboolean) terminate;
  jswapInit   = (jboolean) swapInit;
  jswapDone   = (jboolean) swapDone;
  jtimeout    = (jboolean) timeout;
  
  return_obj = (*env)->NewObject( env, qcomrGetIniEvent_id,
  	                          qcomrGetIniEvent_cid, 
                                  jterminate,
                                  jswapInit,
                                  jswapDone,
                                  jtimeout,
                                  jsts);
  return return_obj;


}




JNIEXPORT jobject JNICALL Java_jpwr_rt_Qcom_getString
  (JNIEnv *env, jobject object, jint qix, jint nid)
{
  int 		sts;
  jclass 	cdhrString_id;
  jmethodID 	cdhrString_cid;
  jobject 	return_obj;
  jint 		jsts;
  qcom_sQid	qid;
  qcom_sGet	get;
  jstring	jdata = NULL;
  int 		sts2;
//  static qcom_sAid op_aid = {0,0};
//  qcom_sAid aid;

  cdhrString_id = (*env)->FindClass( env, "jpwr/rt/CdhrString");
  cdhrString_cid = (*env)->GetMethodID( env, cdhrString_id,
    	"<init>", "(Ljava/lang/String;I)V");
  qid.qix = qix;
  qid.nid = nid;
  memset( &get, 0, sizeof(get));
  qcom_Get( &sts, &qid, &get, 0);
  if ( ODD(sts))
  {
    //printf("Qcom_get: Received data: %s\n", (char *)get.data);
    jdata = (*env)->NewStringUTF( env, (char *)get.data);


    // Get disconnect broadcast...
    // if ( get.type.b == qcom_eBtype_qcom &&
    //      get.type.s == qcom_eStype_applDisconnect) 
    // {
    //   aid = ((qcom_sAppl*) get.data)->aid;
    //   printf("applDisconnect received %d\n", aid.aix);

    //   if ( qcom_AidIsEqual( &aid, &op_aid))
    //   {
    //     jdata = (*env)->NewStringUTF( env, "qcom_exit");
    //   }
    // }
    // else
    // {
    //   op_aid = get.sender;
    //   printf( "Aid received: %d\n", op_aid.aix);
    //   jdata = (*env)->NewStringUTF( env, (char *)get.data);
    // }

    qcom_Free( &sts2, get.data);
  }

  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, cdhrString_id,
  	cdhrString_cid, jdata, jsts);
  return return_obj;  
}

JNIEXPORT jobject JNICALL Java_jpwr_rt_Qcom_putString
  (JNIEnv *env, jobject object, jint qix, jint nid, jstring jdata)
{
  int 		sts;
  const char 	*data;
  jclass 	pwrtStatus_id;
  jmethodID 	pwrtStatus_cid;
  jobject 	return_obj;
  jint		jsts;
  qcom_sQid	qid;
  qcom_sPut	put;
  char		*cstr;
//  int		sts2;

  pwrtStatus_id = (*env)->FindClass( env, "jpwr/rt/PwrtStatus");
  pwrtStatus_cid = (*env)->GetMethodID( env, pwrtStatus_id,
    	"<init>", "(I)V");

  data = (*env)->GetStringUTFChars( env, jdata, 0);
  cstr = (char *)data;
  put.data = (char *)data;
  put.size = strlen(data) + 1;
  put.type.b = qcom_eBtype__;
  put.type.s = qcom_eStype__;
  put.reply.qix = 0;
  put.reply.nid = 0;
  qid.qix = qix;
  qid.nid = nid;
  sts = qcom_Put( &sts, &qid, &put);
  (*env)->ReleaseStringUTFChars( env, jdata, cstr);
//  if ( ODD(sts))
//    qcom_Free( &sts2, put.data);
  jsts = (jint) sts;
  return_obj = (*env)->NewObject( env, pwrtStatus_id,
  	pwrtStatus_cid, jsts);
  return return_obj;
}

