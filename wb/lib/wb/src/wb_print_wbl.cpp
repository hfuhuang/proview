/**
 * @file wb_print_wbl.cpp
 *
 * Prints a volume on wbl format
 *
 */

#include <cfloat>
#include <assert.h>

#include "wb_print_wbl.h"

#include "co_time.h"

#include "wb_attribute.h"
#include "wb_object.h"
#include "wb_volume.h"

wb_print_wbl::wb_print_wbl(ostream& os, int levelIndentation) :
  m_errCnt(0),
  m_idxFlag(true),
  m_level(0),
  m_levelInd(levelIndentation),
  m_keepName(false),
  m_os(os)
{
  memset(m_indBuf, ' ', sizeof(m_indBuf));
  m_indBuf[sizeof(m_indBuf) -1] = 0;
}


wb_print_wbl::~wb_print_wbl()
{
}







//
// printAttribute
//
void wb_print_wbl::printAttribute(wb_volume& v, 
                                  wb_attribute& attr, 
                                  wb_attribute& tattr, ///< template 
                                  wb_adef& adef)
{
  if (adef.flags() & PWR_MASK_POINTER || 
      adef.flags() & PWR_MASK_NOWBL)
    return;

  if (attr.isClass() && adef.cid() == pwr_eClass_Buffer)
    printBuffer(v, attr, tattr, adef);
  else if (attr.isClass())
    printClass(v, attr, tattr, adef);
  else {
    switch (adef.cid()) {
    case pwr_eClass_Param:
    case pwr_eClass_Input:
    case pwr_eClass_Intern:
    case pwr_eClass_Output:
    case pwr_eClass_ObjXRef:
      printParameter(v, attr, tattr, adef);
      break;
    default:
      m_os << "! %%WBDUMP-E-Error Unknown attribute class: " 
           << adef.name() << ", cid: " << adef.cid() << endl;
      m_errCnt++;
      break;
    }
  }
  
}

//
// printBody
//
void wb_print_wbl::printBody(wb_volume& v, 
                             wb_object& o, 
                             wb_object& templ,
                             wb_cdef& cdef,
                             pwr_eBix bix)
{
  wb_adef adef;
  wb_attribute attr;
  wb_attribute tattr;
  const char* bname;
    
  wb_bdef bdef = cdef.bdef(bix);
    

  if (!bdef)
    return;

  bname = bdef.name();

  wb_bdef tbdef = templ.bdef(bix);
  if (!tbdef) {
    m_errCnt++;
    m_os << "Couldn't find template body def for object " << o.name() << endl;
    return;
  }
    
    
  indent(1) << "Body " << bdef.name() << endl;
  for (adef = bdef.adef(); adef; adef = adef.next()) {
    attr = o.attribute(bname, adef.name());
    tattr = templ.attribute(bname, adef.name());
    //    if (tattr == attr)
    //  continue;
    printAttribute(v, attr, tattr, adef);
  }

  indent(-1) << "EndBody" << endl;
    
  return;
}


//
// printBuffer
//
void wb_print_wbl::printBuffer(wb_volume& v,
                              wb_attribute& attr,
                              wb_attribute& tattr,
                              wb_adef& adef) 
{
  pwr_tCid subClass = attr.subClass();
  wb_object templ;
  wb_object sysbo;
  wb_attribute tattr2;
  wb_attribute attr2;
  wb_adef adef2;
  const char* bname;

  if ( strcmp( attr.name(), "Template") == 0 && v.cid() == pwr_eClass_ClassVolume)
    // The parser can't handle subclasses in template objects yet...
    return;

  wb_object co = v.object(cdh_ClassIdToObjid(subClass));
  if (!co) {
    m_os << "! %WBDUMP-E-Error Unknown sub class: " << subClass << endl;
    m_errCnt++;
    return;
  }

  wb_cdef cdef = v.cdef(subClass);
  if (!cdef) {
    m_os << "! %WBDUMP-E-Error Unknown sub class: " << subClass << endl;
    m_errCnt++;
    return;
  }

  wb_name t("Template");
  
  templ = co.child(t);
  if (!templ) {
    m_errCnt++;
    m_os << "! %WBDUMP-E-Error Template not found for class " << cdef.longName() << endl;
    return;
  }

  wb_bdef bdef = cdef.bdef(pwr_eBix_sys);
  if (!bdef) {
    m_os << "! %WBDUMP-E-Error sub class: " << subClass 
         << " not defined" << endl;
    m_errCnt++;
    return;
  }    
  bname = bdef.name();

  for (int i = 0; i < adef.nElement(); i++) {
    if (adef.flags() & PWR_MASK_ARRAY)
      indent(1) << "Buffer " << adef.name() << "[" << i << "]" << endl;
    else
      indent(1) << "Buffer " << adef.name() << endl;


    adef2 = bdef.adef(); 
    attr2 = attr.first(i);
    
    while (1) {
      tattr2 = templ.attribute(bname, adef2.name());
      printAttribute(v, attr2, tattr2, adef2);

      if (!(adef2 = adef2.next()))
        break;
      
      if (!(attr2 = attr2.after()))
        break;
    }                   

    indent(-1) << "EndBuffer" << endl;
  }
}

//
// printClass
//
void wb_print_wbl::printClass(wb_volume& v,
                              wb_attribute& attr,
                              wb_attribute& tattr,
                              wb_adef& adef) 
{
  pwr_tCid subClass = attr.subClass();
  wb_object templ;
  wb_object sysbo;
  wb_attribute tattr2;
  wb_attribute attr2;
  wb_adef adef2;

  //  if ( strcmp( attr.name(), "Template") == 0 && v.cid() == pwr_eClass_ClassVolume)
    // The parser can't handle subclasses in template objects yet...
  //  return;

  wb_cdef cdef = v.cdef(attr.cid());
  if (!cdef) {
    m_os << "! %WBDUMP-E-Error Unknown sub class: " << subClass << endl;
    m_errCnt++;
    return;
  }

  wb_bdef bdef = cdef.bdef(pwr_eBix_sys);
  if (!bdef) {
    m_os << "! %WBDUMP-E-Error sub class: " << subClass 
         << " not defined" << endl;
    m_errCnt++;
    return;
  }    

  for (int i = 0; i < adef.nElement(); i++) {
    attr2 = attr.first(i);
    tattr2 = tattr.first(i);
  
    while ( attr2.oddSts()) {

      adef2 = bdef.adef( attr2.attrName()); 
      
      printAttribute(v, attr2, tattr2, adef2);
    
      attr2 = attr2.after();
      tattr2 = tattr2.after();
    }
  }

#if 0
  wb_object co = v.object(cdh_ClassIdToObjid(subClass));
  if (!co) {
    m_os << "! %WBDUMP-E-Error Unknown sub class: " << subClass << endl;
    m_errCnt++;
    return;
  }

  wb_cdef cdef = v.cdef(subClass);
  if (!cdef) {
    m_os << "! %WBDUMP-E-Error Unknown sub class: " << subClass << endl;
    m_errCnt++;
    return;
  }

  wb_name t("Template");
  
  templ = co.child(t);
  if (!templ) {
    m_errCnt++;
    m_os << "! %WBDUMP-E-Error Template not found for class " << cdef.longName() << endl;
    return;
  }

  wb_bdef bdef = cdef.bdef(pwr_eBix_sys);
  if (!bdef) {
    m_os << "! %WBDUMP-E-Error sub class: " << subClass 
         << " not defined" << endl;
    m_errCnt++;
    return;
  }    
  bname = bdef.name();

  for (int i = 0; i < adef.nElement(); i++) {

    if (adef.flags() & PWR_MASK_ARRAY)
      indent(1) << "Buffer " << adef.name() << "[" << i << "]" << endl;
    else
      indent(1) << "Buffer " << adef.name() << endl;


    adef2 = bdef.adef(); 
    attr2 = attr.first(i);
    
    while (1) {
      tattr2 = templ.attribute(bname, adef2.name());
      printAttribute(v, attr2, tattr2, adef2);

      if (!(adef2 = adef2.next()))
        break;
      
      if (!(attr2 = attr2.after()))
        break;
    }                   

    indent(-1) << "EndBuffer" << endl;

    if (adef.flags() & PWR_MASK_ARRAY)
      indent(1) << "Buffer " << adef.name() << "[" << i << "]" << endl;
    else
      indent(1) << "Buffer " << adef.name() << endl;


    adef2 = bdef.adef(); 
    attr2 = attr.first(i);
    
    while (1) {
      strcpy( aname, adef.subName());
      strcat( aname, ".");
      strcat( aname, attr2.name());

      attr2
      tattr2 = templ.attribute(bname, adef2.name());
      printAttribute(v, attr2, tattr2, adef2);

      if (!(adef2 = adef2.next()))
        break;
      
      if (!(attr2 = attr2.after()))
        break;
    }                   

  }
#endif
}

//
// printHierarchy
//
void wb_print_wbl::printHierarchy(wb_volume& v, wb_object& o)
{
  if (v.object() == o)
    indent(1) << "SObject " << v.name() << ":" << endl;
  else
    indent(1) << "SObject " << o.parent().longName() << endl;
  
  printObject(v, o);
  
  indent(-1) << "EndSObject" << endl;
}


//
// printObject
//
void wb_print_wbl::printObject(wb_volume& v, wb_object& o, bool recursive)
{
  wb_object to = o;
  wb_object templ;
  cdh_uObjid	uid;
  unsigned int idx;
  wb_cdef cdef = v.cdef(o);
  const char* cname = cdef.name();
  char *block;
  int size;

  if ( o.docBlock( &block, &size) && strcmp( block, "") != 0) {
    indent(0) << "!/**" << endl;
    indent(0) << "! ";
    for ( char *s = block; *s; s++) {
      if ( *s == '\n') {
	m_os << *s;
	indent(0) << "! ";
	continue;
      }
      m_os << *s;
    }
    m_os << endl;
    indent(0) << "!*/" << endl;
  }

  indent(1) << "Object " << o.name() << " " << cname;

  if (m_idxFlag) {
    switch (cdef.cid()) {
    case pwr_eClass_ClassDef:
      uid.pwr = o.oid();
      idx = uid.c.cix;
      break;
    case pwr_eClass_TypeDef:
      uid.pwr = o.oid();
      idx = uid.t.tix;
      break;
    case pwr_eClass_ObjBodyDef:
      uid.pwr = o.oid();
      idx = uid.c.bix;
      break;
    case pwr_eClass_Param:
    case pwr_eClass_Input:
    case pwr_eClass_Output:
    case pwr_eClass_Intern:
    case pwr_eClass_Buffer:
    case pwr_eClass_ObjXRef:
      uid.pwr = o.oid();
      idx = uid.c.aix;
      break;
    default:
      idx = (unsigned long) o.oix();
    }
    m_os << " " << idx;
  }
  m_os << endl;

  wb_object co = v.object(cdh_ClassIdToObjid(cdef.cid()));
  wb_name t("Template");
  
  templ = co.child(t);
  if (!templ) {
    m_errCnt++;
    m_os << "Template not found for class " << cdef.name() << endl;
    return;
  }
 
  printBody(v, o, templ, cdef, pwr_eBix_rt);
  printBody(v, o, templ, cdef, pwr_eBix_dev);

  if (recursive) {
    for (to = o.first(); to; to = to.after())
      printObject(v, to);
  }    

  indent(-1) << "EndObject" << endl;
}

//
// printParameter
//
void wb_print_wbl::printParameter(wb_volume& v, 
                                  wb_attribute& attr, 
                                  wb_attribute& tattr, ///< template 
                                  wb_adef& adef)
{

  int nElement = adef.nElement();
  int varSize = adef.size() / nElement;
  char* valueb = (char *)attr.value();
  char* val;
  char* tvalueb = (char *)tattr.value();
  char* tval;
  char* svalp;
  int varOffset;
  bool parValOk;
  const char* name = adef.subName();   

  if (valueb == NULL) {
    m_os << "! %WBDUMP-E-Error Failed to get attribute address for " 
         << adef.name() << endl;
    m_errCnt++;
    return;
  }

  if (adef.type() == pwr_eType_Text) {
    printText(v, adef, valueb, adef.size());
    return;
  }

  if ( attr == tattr)
    // This is the template object itself, print all nonzero
    tvalueb = (char *)calloc( 1, tattr.size());

  for (int i = 0; i < nElement; i++) {
    switch (adef.type()) {
    case pwr_eType_Boolean:
    case pwr_eType_Float32:
    case pwr_eType_Float64:
    case pwr_eType_Char:
    case pwr_eType_String:
    case pwr_eType_Int8:
    case pwr_eType_Int16:
    case pwr_eType_Int32:
    case pwr_eType_UInt8:
    case pwr_eType_UInt16:
    case pwr_eType_UInt32:
    case pwr_eType_Objid:
    case pwr_eType_TypeId:
    case pwr_eType_ClassId:
    case pwr_eType_AttrRef:
    case pwr_eType_Time:
    case pwr_eType_VolumeId:
    case pwr_eType_ObjectIx:
    case pwr_eType_RefId:
    case pwr_eType_DeltaTime:
    case pwr_eType_Mask:
    case pwr_eType_Enum:
    case pwr_eType_Status:
    case pwr_eType_NetStatus:
      varOffset = varSize * i;
      val = valueb + varOffset;
      tval = tvalueb + varOffset;
            
      if (memcmp(val, tval, varSize) == 0 && !(adef.flags() & PWR_MASK_ALWAYSWBL))
        continue;

      parValOk = printValue(v, adef, val, varSize, &svalp);
      if (parValOk)
        indent();
      else
        m_os << "! %WBDUMP-E-Error ";
      
      if (adef.flags() & PWR_MASK_ARRAY) {
        m_os << "Attr " << name << "[" << i << "] = " << svalp << endl;
      } else {
        m_os << "Attr " << name << " = " << svalp << endl;
      }
      break;
    case pwr_eType_Array:
      m_os << "! %WBDUMP-E-Error Type pwr_eType_Array is not yet implemented" << endl;
      m_errCnt++;
      break;
    case pwr_eType_Buffer:
      m_os << "! %WBDUMP-E-Error Type pwr_eType_Buffer is not yet implemented" << endl; 
      m_errCnt++;
      break;
    case pwr_eType_Struct:
      m_os << "! %WBDUMP-E-Error Type pwr_eType_Struct is not yet implemented" << endl;
      m_errCnt++;
      break;
    default:
      m_os << "! %WBDUMP-E-Error Attribute " << adef.name() 
           << " is of unknown type: " <<  adef.type() <<  endl;
      m_errCnt++;
      break;
    }
  }
  if ( attr == tattr)
    free( tvalueb);
}

//
// printText
//
void wb_print_wbl::printText(wb_volume& v, 
                             wb_adef& adef,
                             const char *text,
                             int varSize)
{
  const char* ip;
  int i;
  int end = varSize - 1;

  indent() << "Attr " << adef.name() << " = \"";

  for (ip = text, i = 0; *ip != 0 && i < end; ip++) {
    if (*ip == '"')
      m_os << "\\";
    m_os << *ip;
  }
    
  m_os << "\"" << endl;
    
  return;      
}


//
// printValue
//
bool wb_print_wbl::printValue (wb_volume& v,
                               wb_adef& adef,
                               void *val,
                               int varSize,
                               char **svalp) 
{
  unsigned long sts;
  char timbuf[24];
  static char sval[512];
  bool retval = true;
  pwr_tOid oid;
  wb_object o;
  

  sval[0] = '\0';

  if (adef.flags() & PWR_MASK_POINTER) {
    sprintf(sval, "%u", *(unsigned int *) val);
    *svalp = sval;
    return TRUE;
  }

  switch (adef.type()) {
  case pwr_eType_Boolean:
    sprintf(sval, "%d", *(pwr_tBoolean *) val);
    break;
  case pwr_eType_Float32:
    sprintf(sval, "%.*e", FLT_DIG, *(pwr_tFloat32 *) val);
    break;
  case pwr_eType_Float64:
    sprintf(sval, "%.*e", DBL_DIG, *(pwr_tFloat64 *) val);
    break;
  case pwr_eType_Char:
    if (*(pwr_tChar *) val == 0)
      sprintf(sval, "\"\"");
    else
      sprintf(sval, "\"%c\"", *(pwr_tChar *) val);
    break;
  case pwr_eType_Int8:
    sprintf(sval, "%d", *(pwr_tInt8 *) val);
    break;
  case pwr_eType_Int16:
    sprintf(sval, "%d", *(pwr_tInt16 *) val);
    break;
  case pwr_eType_Int32:
    sprintf(sval, "%d", *(pwr_tInt32 *) val);
    break;
  case pwr_eType_UInt8:
    sprintf(sval, "%u", *(pwr_tUInt8 *) val);
    break;
  case pwr_eType_UInt16:
    sprintf(sval, "%u", *(pwr_tUInt16 *) val);
    break;
  case pwr_eType_UInt32:
    sprintf(sval, "%u", *(pwr_tUInt32 *) val);
    break;
  case pwr_eType_Mask:
    sprintf(sval, "%u", *(pwr_tUInt32 *) val);
    break;
  case pwr_eType_Enum:
    sprintf(sval, "%u", *(pwr_tUInt32 *) val);
    break;
  case pwr_eType_RefId:
    sprintf(sval, "0");
    break;
  case pwr_eType_Objid:
    if (cdh_ObjidIsNull(*(pwr_tOid *) val))
      sprintf(sval, "0");
    else {
      o = v.object(*(pwr_tOid *)val);
      if (o) {
	if ( o.oid().vid >= cdh_cUserVolMin && o.oid().vid != v.vid() && !m_keepName)
	  // Other user volume. Loadfile might not be created yet at load time.
	  sprintf(sval, "\"%s\"", cdh_ObjidToString(NULL, *(pwr_tObjid *)val, 1));
	else
	  sprintf(sval, "\"%s\"", o.longName().c_str());	  
      }
      else 
        sprintf(sval, "\"%s\"", cdh_ObjidToString(NULL, *(pwr_tObjid *)val, 1));
    }    
    break;
  case pwr_eType_ObjectIx:
    if ( *(pwr_tObjectIx *) val == 0)
      sprintf(sval, "0");
    else
      sprintf(sval, "\"%s\"", cdh_ObjectIxToString(NULL, *(pwr_tObjectIx *) val,1));
    break;
  case pwr_eType_VolumeId:
    if ( *(pwr_tVolumeId *) val == 0)
      sprintf(sval, "0");
    else
      sprintf(sval, "\"%s\"", cdh_VolumeIdToString(NULL, *(pwr_tVolumeId *) val,1,0));
    break;
  case pwr_eType_ClassId:
    if (*(pwr_tClassId *) val == 0)
      sprintf(sval, "0");
    else {
      wb_cdef cdef = v.cdef(*(pwr_tCid *)val);
      if (cdef)
        sprintf(sval, "\"%s\"", cdef.longName().c_str());
      else {
        sprintf(sval, "Unknown class, identity: %d", (*(pwr_tClassId *) val));
        m_errCnt++;
        retval = false;
      }
            
    }
    break;
  case pwr_eType_TypeId: /** @todo Modify when wb_tdef is OK q*/
    if (*(pwr_tTypeId *) val == 0)
      sprintf(sval, "0");
    else {
      oid = cdh_TypeIdToObjid(*(pwr_tTid *)val);
      o = v.object(oid);
      if (o)
        sprintf(sval, "\"%s\"", o.longName().c_str());
      else {
        sprintf(sval, "Unknown type, identity: %d", (*(pwr_tTypeId *) val));
        m_errCnt++;
        retval = false;
      }
    }    
    break;
  case pwr_eType_AttrRef: /** @todo */
    if (cdh_ObjidIsNull(((pwr_sAttrRef*)val)->Objid))
      sprintf(sval, "0");
    else {
      wb_attribute a = v.attribute((pwr_sAttrRef*)val);
      if (a)
        sprintf(sval, "\"%s\"", a.longName().c_str());
      else {
        sprintf(sval, "\"%s\"", cdh_ArefToString(NULL, (pwr_sAttrRef*)val, 1));
      }
    }

#if 0      
    } else {
      ConvertObjectName( root, sp, conv_name);
      sprintf(sval, "\"%s\"", conv_name);
    }
#endif
    break;
  case pwr_eType_String: {
    char *s = sval;
    char *t = (char *)val;
    *s++ = '"';
    for ( int i = 0; i < varSize; i++) {
      if ( *t == 0)
	break;
      if ( *t == '"')
	*s++ = '\\'; 
      *s++ = *t++;
    }
    *s++ = '"';
    *s = 0;
    // sprintf(sval, "\"%.*s\"", varSize, (char *)val);
    break;
  }
  case pwr_eType_Time:
    sts = time_AtoAscii((pwr_tTime *)val, time_eFormat_DateAndTime,
			timbuf, sizeof(timbuf)); 
    if (ODD(sts)) {
      sprintf(sval, "\"%s\"", timbuf);
    } else {
      sprintf(sval, "Bad time value");
      m_errCnt++;
      retval = FALSE;
    }
    break;
  case pwr_eType_DeltaTime:
    sts = time_DtoAscii((pwr_tDeltaTime *)val, 1, timbuf, sizeof(timbuf));
    if (ODD(sts)) {
      sprintf(sval, "\"%s\"", timbuf);
    } else {
      sprintf(sval, "Bad time value");
      m_errCnt++;
      retval = FALSE;
    }
    break;
  case pwr_eType_Status:
    sprintf(sval, "%d", *(pwr_tStatus *) val);
    break;
  case pwr_eType_NetStatus:
    sprintf(sval, "%d", *(pwr_tNetStatus *) val);
    break;
  default:
    sprintf(sval, "Unknown attribute type: %d", adef.type());
    m_errCnt++;
    retval = FALSE;
    break;
}

*svalp = sval;
return retval;
}

//
// printVolume
//
void wb_print_wbl::printVolume(wb_volume& v, bool recursive)
{
  if (!v) {
    m_os << "%WBDUMP-E-Error Not a valid volume" << endl;
    m_errCnt++;
    return;
  }

  wb_object o = v.object();
  const char* cname = v.cdef(v.cid()).name();
    
    
  indent(1) << "Volume " << v.name() << " " <<  cname << " "
            << cdh_VolumeIdToString(NULL, v.vid(), 0, 0) << " " << endl;


  // Print volume body
  pwr_tOid oid;
  oid.vid = v.vid();
  oid.oix= 0;
  wb_object vo = v.object( oid);
  wb_cdef cdef = v.cdef(vo);

  wb_object co = v.object(cdh_ClassIdToObjid(v.cid()));
  wb_name t("Template");
  
  wb_object templ = co.child(t);
  if (!templ) {
    m_errCnt++;
    m_os << "Template not found for class " << cdef.name() << endl;
    return;
  }
  printBody(v, vo, templ, cdef, pwr_eBix_sys);

  // Print top objects and their children 
  if (recursive) {
    for (; o; o = o.after())
      printObject(v, o, recursive);
  }
    
  indent(-1) << "EndVolume" << endl;
}


//
// indent
//
ostream& wb_print_wbl::indent(int levelIncr)
{

  if (levelIncr < 0)
    m_level += levelIncr;

  assert(m_level >= 0);
    
  m_indBuf[m_level * m_levelInd] = '\0';

  m_os << m_indBuf;

  m_indBuf[m_level * m_levelInd] = ' ';

  if (levelIncr > 0) 
    m_level += levelIncr;
    
  return m_os;
}
