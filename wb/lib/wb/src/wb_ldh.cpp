#if 1
/* wb_ldh.c -- local data handler

   PROVIEW/R
   Copyright (C) 1996 by Comator Process AB.

   This module contains the API-routines to the Local Data Handler, LDH.  */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#ifdef OS_VMS
#include <descrip.h>
#include <libdef.h>
#include <lnmdef.h>
#include <starlet.h>
#include <lib$routines.h>
#endif
#include <stdarg.h>
#include <X11/Intrinsic.h>
#include "pwr.h"
#include "wb_ldhi.h"
#include "pwr_class.h"
#include "wb_ldh_msg.h"
#include "co_cdh.h"
#include "pwr_vararg.h"
#include "co_ver.h"
#include "rt_gdh.h"
#include "wb_ldh.h"
#include "wb_ldhld.h"
// #include "wb_ldhdb.h"
#include "wb_pwrs.h"
#include "wb_env.h"
#include "wb_session.h"
#include "wb_object.h"
#include "wb_volume.h"
#include "wb_error.h"

pwr_dImport pwr_BindClasses(System);
pwr_dImport pwr_BindClasses(Base);
#if NOT_YET_IMPLEMENTED
static ldh_sMenuItem ldh_lMenuItem[100];

static int ldh_gLog_a = 0;
static int ldh_gLog_b = 0;
static int ldh_gLog_c = 0;

static struct {
  pwr_tString32 Name;
  ldh_eUtility Value;
} ldh_lUtility[] = {
  {"__",		ldh_eUtility__},
  {"Navigator",		ldh_eUtility_Navigator},
  {"Configurator",	ldh_eUtility_Configurator},
  {"-",			ldh_eUtility_}
};

static struct {
  pwr_tString32 Name;
  pwr_tChar Char;
  ldh_eMenuSet Value;
} ldh_lMenuSet[] = {
  {"__",		'\0', ldh_eMenuSet__},
  {"Attribute",		'a',  ldh_eMenuSet_Attribute},
  {"Class",		'c',  ldh_eMenuSet_Class},
  {"Many",		'm',  ldh_eMenuSet_Many},
  {"None",		'n',  ldh_eMenuSet_None},
  {"Object",		'o',  ldh_eMenuSet_Object},
  {"-",			'\0', ldh_eMenuSet_}
};
#endif
/*
  Open a db volume.
*/
pwr_tStatus
ldh_AttachVolume(ldh_tWorkbench	workbench, pwr_tVid vid, ldh_tVolume *volume)
{
    wb_env *wb = (wb_env *)workbench;

    wb_volume *v = new wb_volume();
    *v = wb->volume(vid);
    if (!*v) return v->sts();
    
    *volume = (ldh_tVolume) v;

    return LDH__SUCCESS;
}

/*
  Delete a db volume
*/
pwr_tStatus
ldh_DeleteVolume(ldh_tWorkbench workbench, pwr_tVid vid)
{
    return LDH__SUCCESS;
}

pwr_tStatus
ldh_DetachVolume(ldh_tWorkbench workbench, ldh_tVolume volume)
{
    wb_volume *v = (wb_volume *)volume;
    delete v;

    return LDH__SUCCESS;
}

pwr_tStatus
ldh_GetVolumeList(ldh_tWorkbench workbench, pwr_tVid *vid)
{
    wb_env *wb = (wb_env *)workbench;
    wb_volume v = wb->volume();
    if (!v) return v.sts();
    
    *vid = v.vid();

    return LDH__SUCCESS;
}

pwr_tStatus
ldh_GetNextVolume(ldh_tWorkbench workbench, pwr_tVid vid, pwr_tVid *new_vid)
{
    wb_env *wb = (wb_env *)workbench;
    wb_volume v = wb->volume(vid);
    if (!v) return v.sts();
    v = v.next();
    if (!v) return v.sts();
    
    *new_vid = v.vid();

    return LDH__SUCCESS;
}

pwr_tStatus
ldh_GetPreviousVolume(ldh_tWorkbench workbench, pwr_tVid vid, pwr_tVid *new_vid)
{
    wb_env *wb = (wb_env *)workbench;
    wb_volume v = wb->volume(vid);
    if (!v) return v.sts();
    //v = v.prev();
    if (!v) return v.sts();
    
    *new_vid = v.vid();

    return LDH__SUCCESS;
}

pwr_tStatus
ldh_LoadVolume(ldh_tWorkbench	workbench, char *name, ldh_tVolume *volume)
{
#if 0
    //wb_env *wb = (wb_env *)workbench;

    wb_vrepwbl lv(name);
    
    wb_volume *v = new wb_volume(lv);
    
    *volume = (ldh_tVolume) v;
#endif
    return LDH__SUCCESS;
}

pwr_tStatus
ldh_CreatVolumeSnapshot(ldh_tWorkbench	workbench, char *name, ldh_tVolume *volume)
{
    //wb_env *wb = (wb_env *)workbench;
    
    wb_volume *v = (wb_volume *)volume;
    v->createSnapshot(name);

    return LDH__SUCCESS;
}

/*  Convert a volume name to its corresponding
    volume identity.  */

pwr_tStatus
ldh_VolumeNameToId(ldh_tWorkbench workbench, char *name, pwr_tVid *vid)
{
    wb_env *wb = (wb_env *)workbench;
    wb_volume v = wb->volume(name);
    if (!v) return v.sts();

    *vid = v.vid();
    
    return v.sts();
}

pwr_tStatus
ldh_GetVolumeClass(ldh_tWorkbench workbench, pwr_tVid vid, pwr_tCid *cid)
{
    wb_env *wb = (wb_env *)workbench;
    wb_volume v = wb->volume(vid);
    if (!v) return v.sts();

    *cid = v.cid();
    
    return v.sts();
}

/* Open a load file volume */
pwr_tStatus
ldh_OpenVolume(ldh_tWorkbench workbench, ldh_tSession *session, pwr_tVid vid)
{
    wb_env *wb = (wb_env *)workbench;
    wb_volume v = wb->snapshot(vid);
    if (!v) return v.sts();

    // wb_srep *srep = new wb_srep(v);
    wb_session *sp = new wb_session(v);
    
    *session = (ldh_tSession)sp;

    return LDH__SUCCESS;
}

pwr_tStatus
ldh_CallMenuMethod(ldh_sMenuCall *mcp, int index)
{
    pwr_tStatus		sts = LDH__SUCCESS;
//    pwr_tStatus		(*method)(ldh_sMenuCall*) = mcp->ItemList[index].Method;
    //ldhi_sSession	*sp = mcp->PointedSession; 

    //wb_event e = sp->event(pwr_cNOid, ldh_eEvent_MenuMethodCalled);

    //if (method != NULL)
    //sts = (*method)(mcp);

    //e.send();

    return sts;
}

pwr_tStatus
ldh_ChangeObjectName(ldh_tSession session, pwr_tOid oid, char *name)
{
    wb_session *sp = (wb_session *)session;
    wb_name n(name);
    
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    if (!sp->isLocal(o)) return LDH__OTHERVOLUME;

    return sp->renameObject(o, n);
}


pwr_tStatus
ldh_CheckAttrXRef(ldh_tSession session, pwr_sAttrRef *aref)
{
    wb_session *sp = (wb_session *)session;
    
    wb_attribute a = sp->attribute(aref);
    return a.checkXref();
    
}

/* Check an object cross reference.
   The reference is identified by object id and name of
   cross reference attribute.  */
pwr_tStatus
ldh_CheckObjXRef(ldh_tSession session, pwr_tObjid oid, pwr_tObjName aname)
{
    wb_session *sp = (wb_session *)session;

    wb_object o = sp->object(oid);
    return o.checkXref(aname);    
}

pwr_tStatus
ldh_CheckObjXRefs(ldh_tSession session, pwr_tOid p, pwr_tObjName paname, pwr_tOid s, pwr_tObjName saname)
{
  return LDH__NYI;
}

pwr_tStatus
ldh_ClassNameToId(ldh_tSession session, pwr_tCid *cid, char *name)
{
    wb_session *sp = (wb_session *)session;
    wb_name n(name);
    
    wb_cdef c = sp->cdef(n);
    if (!c) return c.sts();
    
    *cid = c.cid();
    return c.sts();
}


pwr_tStatus
ldh_ClassNameToObjid(ldh_tSession session, pwr_tOid *oid, char *name)
{
    wb_session *sp = (wb_session *)session;

    wb_cdef c = sp->cdef(name);
    if (!c) return c.sts();
    
    *oid = c.oid();
    return c.sts();
}

pwr_tStatus
ldh_CloseSession(ldh_tSession session)
{
    try {
        wb_session *sp = (wb_session *)session;

        //sp->close();
        //sp->unref();
        delete sp;
        
        return LDH__SUCCESS;
    }
    catch (wb_error& e) {
        return e.sts();
    }
    return LDH__SUCCESS;
}


pwr_tStatus
ldh_CloseWB(ldh_tWorkbench workbench)
{
    try {
        wb_env *env = (wb_env *)workbench;

        delete env;;

        return LDH__SUCCESS;
    }
    catch (wb_error& e) {
        return e.sts();
    }
}


pwr_tStatus
ldh_CopyObject(ldh_tSession session, pwr_tObjid *oid, char *name, pwr_tObjid srcoid, pwr_tObjid dstoid, ldh_eDest dest)
{
    wb_session *sp = (wb_session *)session;

    wb_object s_o = sp->object(srcoid);
    wb_object d_o = sp->object(dstoid);
    wb_destination d = d_o.destination(dest);
    wb_object o = sp->copyObject(s_o, d, name);

    if (!o) return o.sts();
    
    *oid = o.oid();
    return o.sts();
}

pwr_tStatus
ldh_CreateObject(ldh_tSession session, pwr_tOid *oid, char *name, pwr_tCid cid, pwr_tOid doid, ldh_eDest dest)
{
    wb_session *sp = (wb_session *)session;

    try {
        wb_name n(name);
        
        wb_cdef cdef = sp->cdef(cid);
    
        wb_object d_o = sp->object(doid);
        wb_destination d = d_o.destination(dest);
        wb_object o = sp->createObject(cdef, d, n);
        if (!o) return o.sts();
    
        *oid = o.oid();
        return o.sts();
    }
    catch (wb_error& e) {
        return e.sts();
    }
}

pwr_tStatus
ldh_CreateVolume(ldh_tWorkbench workbench, ldh_tSession *session, pwr_tVid vid, char *name, pwr_tCid cid)
{
    wb_env *ep = (wb_env *)workbench;

    wb_name n(name);
    wb_cdef cdef;
    
    wb_volume v = ep->createVolume(cdef, vid, name);
    if (!v) return v.sts();
    
    // wb_srep *srep = new wb_srep(v);
    wb_session *sp = new wb_session(v);
    *session = (ldh_tSession)sp;
    
    return sp->sts();
}

pwr_tStatus
ldh_DeleteObject(ldh_tSession session, pwr_tOid oid)
{
    wb_session *sp = (wb_session *)session;

    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    
    return sp->deleteObject(o);
}

pwr_tStatus
ldh_DeleteObjectTree(ldh_tSession session, pwr_tOid oid)
{
    wb_session *sp = (wb_session *)session;

    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    
    return sp->deleteFamily(o);
}


pwr_tStatus
ldh_GetAttrDef(ldh_tSession session, pwr_tCid cid, char *bname, char *aname, ldh_sParDef *adef)
{
    wb_session *sp = (wb_session *)session;

    wb_adef a = sp->adef(cid, bname, aname);
    if (!a) return a.sts();
    
#if NOT_YET_IMPLEMENTED
    a.size();
    a.offset();
    a.name();
    strcpy(adef->ParName, a.name());
    adef->ParLevel = 1;
    adef->ParClass = a.cid();
    
    adef->Par.PgmName;
    adef->Par.Type = a.type();
    adef->Par.Offset = a.offset();
    adef->Par.Size = a.size();
    adef->Par.Flags = a.flags();
    adef->Par.Elements = a.nElement();
    adef->Par.ParamIndex = a.index();
#endif
    return LDH__NYI;
}

pwr_tStatus
ldh_GetAttrRef(ldh_tSession session, pwr_tOid oid, char *aname, pwr_sAttrRef *aref)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    wb_name n(aname);
    wb_attribute a = sp->attribute(o, n);
    if (!a) return a.sts();
    
    *aref = a.aref();

    return LDH__NYI;
}

pwr_tStatus
ldh_GetAttrXRefDef(ldh_tSession session, pwr_sAttrRef *aref, pwr_sAttrXRef *xref)
{
    wb_session *sp = (wb_session *)session;
    wb_attribute a = sp->attribute(aref);
    if (!a) return a.sts();
    
//    *xref = a.xref();
    return LDH__NYI;
}

/*  Get first child of an object.  */

pwr_tStatus
ldh_GetChild(ldh_tSession session, pwr_tOid oid, pwr_tOid *coid)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    o = o.first();
    if (!o) return o.sts();    
    *coid = o.oid();

    return o.sts();
}


pwr_tStatus
ldh_GetClassBody(ldh_tSession session, pwr_tCid cid, char *bname, pwr_tCid *bcid, char **body, int *size)
{
#if NOT_YET_IMPLEMENTED
    wb_session *sp = (wb_session *)session;
    wb_cdef c = sp->cdef(cid);
    wb_classBody b = c.body(bodyname);

    *bcid = b.cid();
    *size = b.size();
    *body = b.data();

    return b.sts();
#endif
    return LDH__NYI;
}

pwr_tStatus
ldh_GetClassList(ldh_tSession session, pwr_tCid cid, pwr_tOid *oid)
{
    wb_session *sp = (wb_session *)session;

    wb_object o = sp->object(cid);
    if (!o) return o.sts();

    *oid = o.oid();
    return o.sts();
}

/* Returns the objid of the next object of the same class.  */

pwr_tStatus
ldh_GetNextObject(ldh_tSession session, pwr_tOid oid, pwr_tOid *new_oid)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();

    o = o.next();
    if (!o) return o.sts();
    
    *new_oid = o.oid();
    
    return o.sts();
}

pwr_tStatus
ldh_GetMenu(ldh_sMenuCall *ip)
{
    return LDH__NYI;
}

pwr_tStatus
ldh_GetNextSibling(ldh_tSession session, pwr_tOid oid, pwr_tOid *noid)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    
    o = o.after();
    if (!o) return o.sts();
    
    *noid = o.oid();
    return o.sts();
}

/* Gets a named body of an object. Buffer space to keep the body is
   allocated by the routine and a pointer to the buffer is returned
   in the body parameter. It is the responsibility of the caller to free
   this space when no longer needed. Use XtFree().  */

pwr_tStatus
ldh_GetObjectBody(ldh_tSession session, pwr_tOid oid, char *bname, void **buff, int *size)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    wb_attribute a = sp->attribute(o, bname);

    *buff = XtMalloc(a.size());
    if (*buff == NULL) return LDH__INSVIRMEM;
    memcpy(*buff, a.value(), a.size());
    if (size != NULL) *size = a.size();
  
    return LDH__SUCCESS;

}

pwr_tStatus
ldh_GetObjectBodyDef(ldh_tSession session, pwr_tCid cid, char *bname,
                     int maxlev, ldh_sParDef **bdef, int *rows)
{
    wb_session *sp = (wb_session *)session;
    wb_cdef c = sp->cdef(cid);
    if (!c) return c.sts();
    wb_bdef b = c.bdef(bname);
    if (!b) return b.sts();
    
    *bdef = (ldh_sParDef *) calloc(1, sizeof(ldh_sParDef) * b.nAttribute());
    if (*bdef == NULL) return LDH__INSVIRMEM;

    for (wb_adef a = b.adef(); a; a = a.next()) {
        //for (wb_adef a = b.adef(); a; a++) {
        // do something a.size(); a.name();
        strcpy((*bdef)[a.index()].ParName, a.name().c_str());
        (*bdef)[a.index()].ParLevel = 1;
        (*bdef)[a.index()].ParClass = (pwr_eClass)a.cid();
        //(*bdef)[a.index()].Par = (void *) obp->body;
    }
    
    return LDH__SUCCESS;
}

pwr_tStatus
ldh_GetObjectBuffer(ldh_tSession session, pwr_tOid oid, char *bname,
                    char *aname, pwr_eClass *bufferclass, char **value, int *size)
{
    wb_session *sp = (wb_session*)session;
    wb_attribute a = sp->attribute(oid, bname, aname);
    if (!a) return a.sts();

    *value = (char *)calloc(1, a.size());
    *size = a.size();
    //*bufferclass = a.?;
    
    //wb_value v(value, size);
    //if (!v) return v.sts();
    
    //v = a.value();
    //return v.sts();
    return LDH__NYI;
}

pwr_tStatus
ldh_GetObjectClass(ldh_tSession session, pwr_tOid oid, pwr_tCid *cid)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    
    *cid = o.cid();
    
    return o.sts();
}

pwr_tStatus
ldh_GetObjectContext(ldh_tSession session, pwr_tOid oid, ldh_sObjContext **octx)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    o.user();
    
    return LDH__NYI;
}

/*  Give information about an object.  */

pwr_tStatus
ldh_GetObjectInfo(ldh_tSession session, pwr_tOid oid, ldh_sObjInfo *ip)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();

    ip->Volume = o.vid();

    return o.sts();
}

pwr_tStatus
ldh_GetObjectPar(ldh_tSession session, pwr_tOid oid, char *bname, char *aname, char **buff, int *size)
{
    wb_session *sp = (wb_session *)session;
    wb_attribute a = sp->attribute(oid, bname, aname);
    if (!a) return a.sts();
    
    
    *buff = (char *)calloc(1, a.size());
    if (*buff == NULL) return LDH__INSVIRMEM;
    memcpy(*buff, a.value(), a.size());
    if (size != 0)
        *size = a.size();

    return a.sts();
}

pwr_tStatus
ldh_GetObjXRefDef(ldh_tSession session, pwr_sAttrRef *aref, pwr_sObjXRef *ObjXRef)
{
    wb_session *sp = (wb_session *)session;
    wb_attribute a = sp->attribute(aref);
    if (!a) return a.sts();
    
    //pwr_sObjXref *x = a.oxref();
   
    return a.sts();
}

pwr_tStatus
ldh_GetParent(ldh_tSession session, pwr_tOid oid, pwr_tOid *poid)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();

    o = o.parent();
    if (!o) return o.sts();
    
    *poid = o.oid();
    
    return o.sts();
}

/* Get previous object in the list of objects of one class
   in one volume.  */
pwr_tStatus
ldh_GetPreviousObject(ldh_tSession session, pwr_tOid oid, pwr_tOid *noid)
{
    try {
        wb_session *sp = (wb_session *)session;

        wb_object o = sp->object(oid);
        if (!o) return o.sts();

        o = o.previous();
        if (!o) return o.sts();
    
        *noid = o.oid();
    
        return o.sts();

    } catch (wb_error& e) {

        return e.sts();

    }
}

pwr_tStatus
ldh_GetPreviousSibling(ldh_tSession session, pwr_tOid oid, pwr_tOid *noid)
{
    try {
        wb_session *sp = (wb_session *)session;

        wb_object o = sp->object(oid);
        o = o.before();
        *noid = o.oid();
    
        return o.sts();

    } catch (wb_error& e) {

        return e.sts();

    }    
}

/* Get the number of different references from the current
   object to other objects.  */

pwr_tStatus
ldh_GetReferenceInfo(ldh_tSession session, pwr_tOid oid, ldh_sRefInfo *rip)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    
    o.refinfo(rip);
    return o.sts();
}

/* Get first object in root list.  */

pwr_tStatus
ldh_GetRootList(ldh_tSession session, pwr_tOid *oid)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object();
    if (!o) return o.sts();

    *oid = o.oid();

    return o.sts();
}

/* Get first object in root list of volume with volume id 'vid'.  */

pwr_tStatus
ldh_GetVolumeRootList(ldh_tSession session, pwr_tVid vid, pwr_tOid *oid)
{
#if NOT_YET_IMPLEMENTED
    pwr_tStatus		sts;
    ldhi_sVidEntry	*vtp;

    if (!hasAccess(sp, ldh_eAccess_ReadOnly, &sts)) return sts;
    if (objid == NULL) return LDH__BADPARAM;

    vtp = (ldhi_sVidEntry *) ldh_TreeFind(sp->wb->vidtab, &vid);
    if (vtp == NULL) return LDH__NOSUCHVOL;

    if (vtp->vhp->ohp->chhp == NULL)
        return LDH__NOSUCHOBJ;

    *objid = vtp->vhp->ohp->chhp->db.oid;

    return LDH__SUCCESS;
#endif
    return LDH__NYI;
}

extern "C" pwr_tStatus
ldh_GetSessionInfo(ldh_tSession session, ldh_sSessInfo *ip)
{
    wb_session *sp = (wb_session *)session;

    ip->Access   = sp->access();
    ip->Utility = sp->utility();
    ip->Empty   = sp->isEmpty() ? 1 : 0;
    
    return LDH__SUCCESS;
}

pwr_tStatus
ldh_GetVolumeInfo(ldh_tVolume volume, ldh_sVolumeInfo *ip)
{
    wb_volume *vp = (wb_volume *)volume;
    
    ip->Volume  = vp->vid();
    ip->Class   = vp->cid();
    //ip->Version = vp->time();

    return LDH__SUCCESS;
}

pwr_tStatus
ldh_GetUniqueObjectName(ldh_tSession session, pwr_tOid oid, char *name)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    
    o.uniqueName(name);

    return o.sts();
}

pwr_tStatus
ldh_IsOkCreateObject(ldh_tSession session, pwr_tCid cid, pwr_tOid oid, ldh_eDest dest)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();

    wb_destination d = o.destination(dest);
    if (!d.canAdopt(cid)) {
        return LDH__NOADOPT;
    }
    
    return LDH__SUCCESS;    
}

pwr_tStatus
ldh_IsOkCopyObject(ldh_tSession session, pwr_tOid oid, pwr_tOid doid, ldh_eDest dest)
{
    wb_session *sp = (wb_session *)session;
    wb_object d_o = sp->object(doid);
    if (!d_o) return d_o.sts();
    wb_object o = sp->object(oid);
    if (!o) return o.sts();

    wb_destination d = d_o.destination(dest);
    if (!d.canAdopt(o.cid())) {
        return LDH__NOADOPT;
    }
    
    return LDH__SUCCESS;    
}

pwr_tStatus
ldh_IsOkMoveObject(ldh_tSession session, pwr_tOid oid, pwr_tOid doid, ldh_eDest dest)
{
    wb_session *sp = (wb_session *)session;
    wb_object d_o = sp->object(doid);
    if (!d_o) return d_o.sts();
    wb_object o = sp->object(oid);
    if (!o) return o.sts();

    wb_destination d = d_o.destination(dest);
    if (!d.canAdopt(o)) {
        return LDH__NOADOPT;
    }
    
    return LDH__SUCCESS;    
}

pwr_tStatus
ldh_MoveObject(ldh_tSession session, pwr_tOid oid, pwr_tOid doid, ldh_eDest dest)
{
    wb_session *sp = (wb_session *)session;
    wb_object d_o = sp->object(doid);
    if (!d_o) return d_o.sts();
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    wb_destination d = d_o.destination(dest);

    return sp->moveObject(o, d);
}

pwr_tStatus
ldh_NameToAttrRef(ldh_tSession session, char *name, pwr_sAttrRef *arp)
{
    wb_session *sp = (wb_session *)session;
    wb_attribute a = sp->attribute(name);
    if (!a) return a.sts();
    
    a.aref(arp);

    return a.sts();
}


/*  Get the object identifier of a named object.  */

pwr_tStatus
ldh_NameToObjid(ldh_tSession session, pwr_tOid *oid, char *name)
{
    wb_session *sp = (wb_session *)session;
    wb_object o = sp->object(name);
    if (!o) return o.sts();
    
    *oid = o.oid();

    return o.sts();
}

/* Returns the name of an object identified by its objid.
   The caller is responsible for supplying a buffer at
   least as big as maxsize.  */

pwr_tStatus
ldh_ObjidToName(ldh_tSession session, pwr_tOid oid, ldh_eName type, char *buf, int maxsize, int *size)
{
    wb_session *sp = (wb_session *)session;

    switch ( type) {
      case ldh_eName_Object:
      case ldh_eName_Hierarchy:
      case ldh_eName_Path:
      case ldh_eName_VolPath:
      case ldh_eName_Volume:
      {
        wb_object o = sp->object(oid);
        if (!o) return o.sts();

        try {
	  char name[200];
          strcpy( name, o.longName().name( type));
	  *size = strlen( name);
	  if ( *size > maxsize - 1)
	    return LDH__NAMEBUF;
	  strcpy( buf, name);
        }
        catch ( wb_error& e) {
	  return e.sts();
        }
        break;
      }
      case ldh_eName_Objid:
      case ldh_eName_ObjectIx:
      case ldh_eName_OixString:
      case ldh_eName_VolumeId:
      case ldh_eName_VidString:
      {
        char str[80];

        wb_name n = wb_name( cdh_ObjidToString( NULL, oid, 1));
	strcpy( str, n.name( type));
	*size = strlen(str);
	if ( *size > maxsize - 1)
	  return LDH__NAMEBUF;
	strcpy( buf, str);
	break;
      }
      default:
        return LDH__NYI;
    }
    return LDH__SUCCESS;
}

/* Returns the name of a type identified by its type identifier.
   The caller is responsible for supplying a buffer at
   least as big as maxsize.  */

pwr_tStatus
ldh_TypeIdToName(ldh_tSession session, pwr_tTid tid, char *buff, int maxsize, int *size)
{
    wb_session *sp = (wb_session *)session;
    wb_tdef t = sp->tdef(tid);
    if (!t) return t.sts();
    
    //buff = t.name();
    
    return t.sts();
}

/* Returns the name of a class identified by its classid.
   The caller is responsible for supplying a buffer at
   least as big as maxsize.  */

pwr_tStatus
ldh_ClassIdToName(ldh_tSession session, pwr_tCid cid, char *buff, int maxsize, int *size)
{
    wb_session *sp = (wb_session *)session;
    wb_cdef c = sp->cdef(cid);
    if (!c) return c.sts();
    
    //buff = c.name();
    
    return c.sts();
}

/*  Returns the name of an object attribute identified by an
    attribute reference. A pointer to the name and the size
    of it is returned.  */

pwr_tStatus
ldh_AttrRefToName(ldh_tSession session, pwr_sAttrRef *arp, ldh_eName nametype, char **aname, int *size)
{
    static char str[512];
    wb_session *sp = (wb_session *)session;
    
    switch ( nametype) {
      case ldh_eName_Object:
      case ldh_eName_Hierarchy:
      case ldh_eName_Path:
      case ldh_eName_VolPath:
      case ldh_eName_Volume:
      case ldh_eName_Aref:
      case ldh_eName_ArefVol:
      {
        wb_attribute a = sp->attribute(arp);
        if (!a) return a.sts();
    
        //wb_name n = a.name(nametype);
        break;
      }
      case ldh_eName_ArefExport:
      case ldh_eName_Objid:
      case ldh_eName_ObjectIx:
      case ldh_eName_OixString:
      case ldh_eName_VolumeId:
      case ldh_eName_VidString:
      {
        wb_name n = wb_name( cdh_ArefToString( NULL, arp, 1));
	strcpy( str, n.name( nametype));
	*aname = str;
	*size = strlen(str);
	break;
      }
      default:
        return LDH__NYI;
    }
    return LDH__SUCCESS;
}

/*  Convert a volume identity to a corresponding name.
    
    If the name of the volume is not available, the name
    is given in an alphanumerical form:

	_V0.123.34.63

    */
 
pwr_tStatus
ldh_VolumeIdToName(ldh_tWorkbench workbench, pwr_tVid vid, char *name, int maxsize, int *size)
{
    wb_env *wb = (wb_env *)workbench;
    wb_volume v = wb->volume(vid);
    if (!v) return v.sts();

    *size = strlen(v.name());
    strncpy( name, v.name(), maxsize);
    return LDH__SUCCESS;
}


void
ldh_AddThisSessionCallback(ldh_tSession session, void *editorContext,
                           pwr_tStatus (*receiveThisSession) (
                               void *editorContext, ldh_sEvent *event))
{
#if NOT_YET_IMPLEMENTED
    wb_session *sp = (wb_session *)session;
    sp->editorContext(editorContext);
    sp->sendThisSession(receiveThisSession);
#endif
}


void
ldh_AddOtherSessionCallback(ldh_tSession session, void *editorContext,
                            pwr_tStatus (*receiveOtherSession)(
                                void *editorContext, ldh_sEvent *event))
{
#if NOT_YET_IMPLEMENTED
    wb_session *sp = (wb_session *)session;
    sp->editorContext(editorContext);
    sp->sendOtherSession(receiveOtherSession);
#endif
}


pwr_tStatus
ldh_OpenSession(ldh_tSession *session, ldh_tVolume volume,
                ldh_eAccess access, ldh_eUtility utility)
{
    wb_volume *vp = (wb_volume *)volume;
    
    if (!*vp) return vp->sts();

    // wb_srep *srep = new wb_srep(vp);
    wb_session *sp = new wb_session(*vp);
    
    sp->access(access);
    sp->utility(utility);
    
    *session = (ldh_tSession)sp;

    return sp->sts();
}

/* This routine creates a new memory resident workbench and populates
   it with objects from database on disk.  */

pwr_tStatus
ldh_OpenWB(ldh_tWorkbench *workbench)
{
    wb_erep *erep = new wb_erep();
    wb_env *env = new wb_env(erep);
    env->load();

    *workbench = (ldh_tWorkbench)env;
    return env->sts();
}

pwr_tStatus
ldh_ReadAttribute(ldh_tSession session, pwr_sAttrRef *arp, void *value, int size)
{
    wb_session *sp = (wb_session*)session;

    wb_attribute a = sp->attribute(arp);
    if (!a) return a.sts();
#if NOT_YET_IMPLEMENTED
    wb_value v(value, size);    
    if (!v) return v.sts();
    
    v = a.value();
    return v.sts();
#else
    return LDH__NYI;
#endif
}

/* Reads a named body of an object into a buffer supplied in the call.  */

pwr_tStatus
ldh_ReadObjectBody(ldh_tSession *session, pwr_tObjid oid, char *bname, void *value, int size)
{
#if NOT_YET_IMPLEMENTED
    wb_session *sp = (wb_session*)session;
    wb_body b = sp->body(oid, bname);
    if (!b) return b.sts();
    wb_value v(value, size);    
    if (!v) return v.sts();
    
    v = b.value();
    return v.sts();
#else
    return LDH__NYI;
#endif
}

pwr_tStatus
ldh_RevertSession(ldh_tSession session)
{
    wb_session *sp = (wb_session*)session;

    return sp->abort();
}

/* Save all changes to objects done in the session.  */

pwr_tStatus
ldh_SaveSession(ldh_tSession session)
{
    wb_session *sp = (wb_session*)session;

    return sp->commit();
}

ldh_tVolume
ldh_SessionToVol(ldh_tSession session)
{
    wb_session *sp = (wb_session*)session;

    
    return (ldh_tVolume) sp;
}

ldh_tWorkbench
ldh_SessionToWB(ldh_tSession session)
{
    wb_session *sp = (wb_session *)session;
    
    wb_env *e = new wb_env(sp->env());         // Fix ... this is never deleted !!!
    return (ldh_tWorkbench) e;
}

/* Updates a named body of an object.  */

pwr_tStatus
ldh_SetObjectBody(ldh_tSession session, pwr_tOid oid, char *bname, char *value, int size)
{
    wb_session *sp = (wb_session*)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
#if NOT_YET_IMPLEMENTED
    wb_body b = o.body(bname);
    if (!b) return b.sts();
    
    return b.value(value, size);
#endif
    return LDH__NYI;
}

pwr_tStatus
ldh_SetObjectBuffer(ldh_tSession session, pwr_tOid oid, char *bname, char *aname, char *value)
{
    wb_session *sp = (wb_session*)session;

    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    wb_attribute a = o.attribute(bname, aname);
    if (!a) return a.sts();
    
    sp->writeAttribute(/*a, value*/);

    return sp->sts();
}


/* The same as ChangeObjectName but without notification.  */

pwr_tStatus
ldh_SetObjectName(ldh_tSession session, pwr_tOid oid, char *name)
{
    wb_session *sp = (wb_session*)session;

    wb_object o = sp->object(oid);
    if (!o) return o.sts();

    pwr_tStatus sts = sp->renameObject(o, name);
    
    return sts;
}

pwr_tStatus
ldh_SetObjectPar(ldh_tSession session, pwr_tOid oid, char *bname,
                 char *aname, char *value, int size)
{
    wb_session *sp = (wb_session*)session;

    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    wb_attribute a = o.attribute(bname, aname);
    if (!a) return a.sts();
    
    return sp->writeAttribute(/*a, value, size*/);
}

pwr_tStatus
ldh_SetSession(ldh_tSession session, ldh_eAccess access)
{
    wb_session *sp = (wb_session*)session;

    return sp->access(access);
}

pwr_tStatus
ldh_StringGetAttribute(ldh_tSession session, pwr_sAttrRef *arp, pwr_tUInt32 maxsize, char *string)
{
    wb_session *sp = (wb_session*)session;

    wb_attribute a = sp->attribute(arp);
    if (!a) return a.sts();
    
    //return a.valueToString(string, size);
    return LDH__NYI;
}

/* If write is false this routine only checks the string.  */
pwr_tStatus
ldh_StringSetAttribute(ldh_tSession session, pwr_sAttrRef *arp, char *string, pwr_tBoolean write)
{
    wb_session *sp = (wb_session*)session;
    wb_attribute a = sp->attribute(arp);
    if (!a) return a.sts();
    
    //return a.stringToValue(string, write);
    return LDH__NYI;
}

/*  Writes an attribute of an object from a buffer supplied in the call.  */

pwr_tStatus
ldh_WriteAttribute(ldh_tSession session, pwr_sAttrRef *arp, void *value, int size)
{
    wb_session *sp = (wb_session*)session;
    wb_attribute a = sp->attribute(arp);
    if (!a) return a.sts();
    //wb_value v(value, size);    
    //if (!v) return v.sts();
    
    //return a.value(v);
    return LDH__NYI;
}

/*  Returns 1 if object is owned by the volume attached to
    the current session.  */

pwr_tBoolean
ldh_LocalObject(ldh_tSession session, pwr_tOid oid)
{
    wb_session *sp = (wb_session*)session;
    wb_object o = sp->object(oid);
    if (!o) return o.sts();
    
    return sp->isLocal(o);
}

pwr_tStatus
ldh_SyntaxCheck(ldh_tSession session, int *errorcount, int *warningcount)
{
    //wb_session *sp = (wb_session*)session;

    return LDH__NYI;
}

pwr_tStatus
ldh_CopyObjectTrees(ldh_tSession session, pwr_sAttrRef *arp, pwr_tOid doid, ldh_eDest dest, pwr_tBoolean self)
{
    //wb_session *sp = (wb_session*)session;

    return LDH__NYI;
}


/* Make a copy of object trees pointed at by AREF and
   put the copy in a pastebuffer associated whith the
   workbench context. The original object trees are
   left untouched.  */

pwr_tStatus
ldh_Copy(ldh_tSession session, pwr_sAttrRef *arp)
{
    //wb_session *sp = (wb_session*)session;

    //wb_oset s = sp->objectset(arp);
    //wb->copySet(s);

    return LDH__NYI;
}

/* Make a copy of object trees pointed at by AREF and
   put the copy in a pastebuffer associated whith the
   workbench context. The original object trees are
   deleted but can be retrieved by reverting. */

pwr_tStatus
ldh_Cut(ldh_tSession session, pwr_sAttrRef *arp)
{
#if NOT_YET_IMPLEMENTED
    wb_session *sp = (wb_session*)session;

    wb_objectset s = sp->objectSet(arp);
    wb->copySet(s);
    sp->deleteObject(s);    
#endif
    return LDH__NYI;
}

pwr_tStatus
ldh_Paste(ldh_tSession session, pwr_tOid doid, ldh_eDest dest)
{
    //wb_session *sp = (wb_session*)session;

    return LDH__NYI;
}

pwr_tStatus
ldh_CreateLoadFile(ldh_tSession session)
{
  return LDH__NYI;
}
#endif


