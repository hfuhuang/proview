/* wb_c_node.c -- work bench methods of the Node class.

   PROVIEW/R
   Copyright (C) 1994 by Comator Process AB.  */

#include "wb_pwrs.h"
#include "wb_pwrs_msg.h"
#include "wb_ldh.h"

static pwr_tStatus PostCreate (
  ldh_tSesContext   Session,
  pwr_tObjid	    Object,
  pwr_tObjid	    Father,
  pwr_tClassId	    Class
) {
  pwr_tObjid oid;
  pwr_tClassId cid;
  pwr_tStatus sts;

  sts = ldh_ClassNameToId(Session, &cid, "MessageHandler");
  sts = ldh_CreateObject(Session, &oid, "MessageHandler", cid, Object, ldh_eDest_IntoLast); 

  sts = ldh_ClassNameToId(Session, &cid, "IOHandler");
  sts = ldh_CreateObject(Session, &oid, "IOHandler", cid, Object, ldh_eDest_IntoLast); 

  sts = ldh_ClassNameToId(Session, &cid, "Backup_Conf");
  sts = ldh_CreateObject(Session, &oid, "Backup", cid, Object, ldh_eDest_IntoLast); 

  sts = ldh_ClassNameToId(Session, &cid, "OpPlace");
  sts = ldh_CreateObject(Session, &oid, "Op", cid, Object, ldh_eDest_IntoLast); 
  sts = ldh_CreateObject(Session, &oid, "Maintenance", cid, Object, ldh_eDest_IntoLast); 
  sts = ldh_ClassNameToId(Session, &cid, "RttConfig");
  sts = ldh_CreateObject(Session, &oid, "RttConfig", cid, Object, ldh_eDest_IntoLast); 

  sts = ldh_ClassNameToId(Session, &cid, "PlcProcess");
  sts = ldh_CreateObject(Session, &oid, "Plc", cid, Object, ldh_eDest_IntoLast); 

  sts = ldh_ClassNameToId(Session, &cid, "WebHandler");
  sts = ldh_CreateObject(Session, &oid, "WebHandler", cid, Object, ldh_eDest_IntoLast); 

  sts = ldh_ClassNameToId(Session, &cid, "WebBrowserConfig");
  sts = ldh_CreateObject(Session, &oid, "WebBrowser", cid, Object, ldh_eDest_IntoLast); 

  return PWRS__SUCCESS;
}

pwr_dExport pwr_BindMethods($Node) = {
  pwr_BindMethod(PostCreate),
  pwr_NullMethod
};









