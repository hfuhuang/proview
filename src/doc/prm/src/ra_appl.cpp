/*! \file ra_appl.cpp
    \brief Example application subclasses of rt_appl
   This file is an example file, containing to example applications.
*/

/** @addtogroup rt */
/*@{*/


/** @defgroup rt_ex Example applications
 *  @ingroup NMps
 *  @{
 */
#include "pwr.h"
#include "rt_appl.h"


//! A simple c++ application example
/*! ra_appl is a subclass to rt_appl and implements the virtual
 * functions open(), close(), and scan().
 */
class ra_appl : public rt_appl {
public:
 ra_appl() : rt_appl( "ra_appl", errh_eAnix_appl1) {}
 void open();
 void close();
 void scan();
};


void ra_appl::open()
{
 // Link to database objects
}

void ra_appl::close()
{
 // Unlink to database objects
}

void ra_appl::scan()
{
 // Do something
}

int main()
{
 ra_appl appl;

 appl.init();
 appl.register_appl( "Noder-node-MyAppl");

 appl.mainloop();
}



#include "pwr.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "rt_aproc.h"
#include "rt_pwr_msg.h"
#include "rt_qcom_msg.h"
#include "rt_ini_event.h"
#include "co_error.h"


//! Advanced c++ application example
/*! Example of a proview application program
 */
class MyAppl {
public:
 MyAppl() {}
 void init( qcom_sQid *qid);
 void open();
 void close();
 void scan();
 float scantime() { return 1.0;}
};

void MyAppl::init( qcom_sQid *qid)
{
 qcom_sQid qini;
 qcom_sQattr qAttr;
 pwr_tStatus sts;

 // Init error and status logger with a unic application index per node.
 errh_Init( "rs_appl", errh_eAnix_appl1);
 errh_SetStatus( PWR__APPLSTARTUP);

 // Init database
 sts = gdh_Init("rs_appl");
 if ( EVEN(sts)) {
   errh_Fatal( "gdh_Init, %m", sts);
   errh_SetStatus( PWR__APPLTERM);
   exit(sts);
 }

 // Create a queue to receive stop and restart events
 if (!qcom_Init(&amp;sts, 0, "rs_appl")) {
   errh_Fatal("qcom_Init, %m", sts); 
   errh_SetStatus( PWR__APPLTERM);
   exit(sts);
 } 

 qAttr.type = qcom_eQtype_private;
 qAttr.quota = 100;
 if (!qcom_CreateQ(&amp;sts, qid, &amp;qAttr, "events")) {
   errh_Fatal("qcom_CreateQ, %m", sts);
   errh_SetStatus( PWR__APPLTERM);
   exit(sts);
 } 

 qini = qcom_cQini;
 if (!qcom_Bind(&amp;sts, qid, &amp;qini)) {
   errh_Fatal("qcom_Bind(Qini), %m", sts);
   errh_SetStatus( PWR__APPLTERM);
   exit(-1);
 }
}

void MyAppl::open()
{
 pwr_tOid oid;
 pwr_tStatus sts;

 // Get configuration object
 sts = gdh_NameToObjid( "Noder-Node-MyAppl", &amp;oid);
 if ( EVEN(sts)) throw co_error(sts);

 aproc_RegisterObject( oid);

 // Link to database objects
}

void MyAppl::close()
{
 // Unlink to database objects
}

void MyAppl::scan()
{
 aproc_TimeStamp();

 // Do something
}

int main()
{
 pwr_tStatus sts;
 MyAppl appl;
 int tmo;
 char mp[2000];
 qcom_sQid qid = qcom_cNQid;
 qcom_sGet get;
 int swap = 0;
 bool first_scan = true;

 appl.init( &amp;qid);

 try {
   appl.open();
 }
 catch ( co_error&amp; e) {
   errh_Error( (char *)e.what().c_str());
   errh_Fatal( "rs_appl aborting");
   errh_SetStatus( PWR__APPLTERM);
   exit(0);
 }

 aproc_TimeStamp();
 errh_SetStatus( PWR__ARUN);

 first_scan = true;
 for (;;) {
   if ( first_scan) {
     tmo = (int) (appl.scantime() * 1000 - 1);
   }

   get.maxSize = sizeof(mp);
   get.data = mp;
   qcom_Get( &amp;sts, &amp;qid, &amp;get, tmo);
   if (sts == QCOM__TMO || sts == QCOM__QEMPTY) {
     if ( !swap)
appl.scan();
   } 
   else {
     ini_mEvent  new_event;
     qcom_sEvent *ep = (qcom_sEvent*) get.data;

     new_event.m  = ep-&gt;mask;
     if (new_event.b.oldPlcStop &amp;&amp; !swap) {
errh_SetStatus( PWR__APPLRESTART);
       swap = 1;
appl.close();
     } else if (new_event.b.swapDone &amp;&amp; swap) {
       swap = 0;
appl.open();
errh_SetStatus( PWR__ARUN);
     } else if (new_event.b.terminate) {
exit(0);
     }
   }
   first_scan = false;
 }
}


   
/** @} */
/** @} */
