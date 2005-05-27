#include "wb_attribute.h"
#include "wb_cdrep.h"
#include "wb_bdrep.h"
#include "wb_bdef.h"
#include "wb_merep.h"
#include "wb_attrname.h"
#include "pwr.h"


wb_attribute::wb_attribute() : 
  wb_status(LDH__NOSUCHATTR), m_orep(0), m_adrep(0), 
  m_size(0), m_offset(0), m_idx(0), m_tid(0), m_elements(0),
  m_type(pwr_eType_), m_flags(0), m_bix(pwr_eBix__), m_body(0), m_shadowed(false)
{
}

wb_attribute::wb_attribute(const wb_attribute& x) : 
  wb_status(x.m_sts),m_orep(x.m_orep), m_adrep( x.m_adrep), m_size(x.m_size), 
  m_offset(x.m_offset) , m_idx(x.m_idx), m_tid(x.m_tid), m_original_tid(x.m_original_tid),
  m_elements(x.m_elements), m_type(x.m_type), 
  m_flags(x.m_flags), m_bix(x.m_bix), m_body(0), m_shadowed(x.m_shadowed)
{
  if ( m_orep)
    m_orep->ref();
  if ( m_adrep)
    m_adrep->ref();
}

wb_attribute::wb_attribute(pwr_tStatus sts, wb_orep * orep) : 
  wb_status(sts), m_orep(orep), m_adrep(0), m_size(0), m_offset(0), m_idx(0), m_tid(0),
  m_elements(1), m_type(pwr_eType_), m_flags(0), m_bix(pwr_eBix__), m_body(0), m_shadowed(false)
{
  if ( orep == 0)
    m_sts = LDH__NOSUCHATTR;
  else {
    m_orep->ref();

    wb_cdef cdef(*orep);
    if (EVEN(cdef.sts())) {
      m_sts = cdef.sts();
      return;
    }
    wb_bdef bdef(((wb_cdrep *)cdef)->bdrep(&m_sts, pwr_eBix_rt));
    if (EVEN(bdef.sts())) {
      m_sts = bdef.sts();
      return;
    }

    m_size = bdef.size();
    m_bix = bdef.bix();
    m_tid = m_original_tid = orep->cid();
  }
}

wb_attribute::wb_attribute(pwr_tStatus sts, wb_orep* orep, wb_adrep* adrep, int idx) : 
  wb_status(sts), m_orep(orep), m_adrep(adrep), m_size(0), m_offset(0), m_idx(0), m_tid(0),
  m_elements(1), m_type(pwr_eType_), m_flags(0), m_bix(pwr_eBix__), m_body(0), m_shadowed(false)
{
  if ( orep == 0)
    m_sts = LDH__NOSUCHATTR;
  else {
    m_orep->ref();

    if ( m_adrep) {
      m_adrep->ref();
      m_size = m_adrep->size();
      m_offset = m_adrep->offset();
      m_tid = m_original_tid = m_adrep->tid();
      m_elements = m_adrep->nElement();
      m_type = m_adrep->type();
      m_flags = m_adrep->flags();
      m_bix = m_adrep->bix();

      if (m_flags & PWR_MASK_ARRAY) {
        if (idx >= m_elements)
          throw wb_error_str("wb_attribute() subscript out of range");
        m_idx = idx;
        if (idx != -1) { // element
          m_size = m_adrep->size() / m_adrep->nElement();
          m_offset = m_adrep->offset() + m_idx * m_size;
          m_elements = 1;
        }
      } else
        m_idx = 0;

      if ( m_flags & PWR_MASK_CASTATTR) {
	pwr_tCastId castid;

	castId( &castid);
	if ( castid != pwr_cNCastId)
	  m_tid = castid;
      }
    }
    else {
      // m_size == get rtbody size... Fix
      wb_cdef cdef(*orep);
      if (EVEN(cdef.sts())) {
        char msg[256];
        m_sts = cdef.sts();
        sprintf(msg, "Can't get cdef for orep: %s", orep->name());  
        throw wb_error_str(m_sts, msg);
      }
      wb_bdef bdef(((wb_cdrep *)cdef)->bdrep(&m_sts, pwr_eBix_rt));
      if (EVEN(bdef.sts())) {
        char msg[256];
        m_sts = bdef.sts();
        sprintf(msg, "Can't get bdef for orep: %s", orep->name());  
        throw wb_error_str(m_sts, msg);
      }

      m_size = bdef.size();
      m_bix = bdef.bix();
    }
  }
}

wb_attribute::wb_attribute(pwr_tStatus sts, wb_orep* orep, const char *bname) :
  wb_status(sts), m_orep(orep), m_adrep(0), m_size(0), m_offset(0),  m_idx(0), m_tid(0),
  m_elements(1), m_type(pwr_eType_), m_flags(0), m_bix(pwr_eBix__), m_body(0), m_shadowed(false)
{
  if ( orep == 0)
    m_sts = LDH__NOSUCHATTR;
  else {
    m_orep->ref();

    wb_cdrep *cd = m_orep->vrep()->merep()->cdrep( &m_sts, m_orep->cid());
    if ( oddSts()) {

      wb_bdrep *bd = cd->bdrep( &m_sts, bname);
      if ( oddSts()) {
        m_size = bd->size();
	m_bix = bd->bix();
        delete bd;
      }
      delete cd;
    }
  }
}

wb_attribute::wb_attribute(pwr_tStatus sts, wb_orep* orep, const char *bname, const char *aname) :
  wb_status(sts), m_orep(orep), m_adrep(0), m_size(0), m_offset(0), m_idx(0), m_tid(0),
  m_elements(1), m_type(pwr_eType_), m_flags(0), m_bix(pwr_eBix__), m_body(0), m_shadowed(false)
{
  if ( orep == 0)
    m_sts = LDH__NOSUCHATTR;
  else {
    m_orep->ref();

    wb_attrname n = wb_attrname(aname);  
    wb_bdrep *bd = 0;
    wb_cdrep *cd = m_orep->vrep()->merep()->cdrep( &m_sts, m_orep->cid());
    if ( oddSts()) {
      if ( bname) {
        bd = cd->bdrep( &m_sts, bname);
        if ( oddSts()) {
          m_adrep = bd->adrep( &m_sts, n.attributesAllTrue());
	  m_bix = bd->bix();
	}
      }
      else {
        m_adrep = cd->adrep( &m_sts, n.attributesAllTrue());
	m_bix = pwr_eBix_rt;
      }
      if ( oddSts()) {
        m_adrep->ref();

	m_size = m_adrep->size();
	m_offset = m_adrep->offset();
	m_tid = m_original_tid = m_adrep->tid();
	m_elements = m_adrep->nElement();
	m_flags = m_adrep->flags();
	m_type = m_adrep->type();

	if ( m_flags & PWR_MASK_CASTATTR) {
	  pwr_tCastId castid;

	  castId( &castid);
	  if ( castid != pwr_cNCastId)
	    m_tid = castid;
	}
      } else
        m_adrep = 0;
      
      delete cd;
      if ( bd) 
        delete bd;
    }
  }
}

wb_attribute::wb_attribute(const wb_attribute& pa, int idx, const char *aname) :
  wb_status(LDH__NOSUCHATTR), m_orep(0), m_adrep(0), m_size(0), m_offset(0), m_tid(0),
  m_elements(1), m_type(pwr_eType_), m_flags(0), m_bix(pwr_eBix__), m_body(0), m_shadowed(false)
{
  pwr_tCid cid;
  wb_attrname n;
  char attrname[120];
  
  if ( !cdh_tidIsCid( pa.tid()) || pa.m_orep == 0)
    return;

  if ( pa.m_flags & PWR_MASK_ARRAY && 
       (idx < 0 || idx >= pa.m_elements))
    throw wb_error_str("Invalid subscript");
    
  strcpy( attrname, pa.attrName());
  if ( pa.m_flags & PWR_MASK_ARRAY) {
    if ( attrname[strlen(attrname)-1] == ']') {
      // Replace the index
      char *s = strchr( attrname, '[');
      if ( s)
	sprintf( s, "[%d]", idx);
    }
    else
      // Add index
      sprintf( &attrname[strlen(attrname)], "[%d]", idx);
  }
  strcat( attrname, ".");
  if ( aname != 0)
    strcat( attrname, aname);
  else {
    // First attribute
    if ( pa.isClass())
      cid = pa.subClass();
    else
      cid = pa.tid();
    wb_cdrep *cd = pa.m_orep->vrep()->merep()->cdrep(&m_sts, cid);
    if ( evenSts()) return;
    
    wb_bdrep* bd = cd->bdrep(&m_sts, pwr_eBix_sys);
    if ( evenSts()) { delete cd; return;}

    wb_adrep *adrep = bd->adrep(&m_sts);
    if ( evenSts()) { delete cd; delete bd; return;}

    strcat( attrname, adrep->name());
    delete adrep;
    delete bd;
    delete cd;
  }

  n = attrname;

  cid = pa.cid();

  wb_cdrep *cd = pa.m_orep->vrep()->merep()->cdrep(&m_sts, cid);
  if (evenSts()) return;

  wb_bdrep* bd = cd->bdrep(&m_sts, pwr_eBix_sys);
  if (evenSts()) {
    // Try devbody
    bd = cd->bdrep(&m_sts, pwr_eBix_dev);
    if (evenSts()) { delete cd; return;}
  }

  m_adrep = bd->adrep(&m_sts, n.attributesAllTrue());
  if (evenSts()) {
    // Try devbody
    bd = cd->bdrep(&m_sts, pwr_eBix_dev);
    if (evenSts()) { delete cd; return;}

    m_adrep = bd->adrep(&m_sts, n.attributesAllTrue());
    if (evenSts()) { delete cd; delete bd; return;}
  }
  m_adrep->ref();
  m_size = m_adrep->size();
  m_offset = m_adrep->offset();
  m_tid = m_original_tid = m_adrep->tid();
  m_elements = m_adrep->nElement();
  m_flags = m_adrep->flags();
  m_type = m_adrep->type();  
  m_idx = idx;

  m_orep = pa.m_orep;
  m_orep->ref();
  m_bix = bd->bix();

  if ( m_flags & PWR_MASK_CASTATTR) {
    pwr_tCastId castid;

    castId( &castid);
    if ( castid != pwr_cNCastId)
      m_tid = castid;
  }

#if 0
  if ( pa.isClass()) {
    m_flags |= PWR_MASK_SUBCLASS;

    if (pa.m_flags & PWR_MASK_SUBCLASS)
      m_bix = pa.m_bix;
    else
      m_bix = pa.m_adrep->bix();
  }
  else
    m_bix = pa.m_bix;
#endif

  delete bd;
  delete cd;
}

wb_attribute::~wb_attribute()
{
  if ( m_orep) {
    m_orep->unref();
    m_orep = 0;
  }
  
  if ( m_adrep) {
    m_adrep->unref();
    m_adrep = 0;
  }
  
  if (m_body) {
    //printf("a: %8.8x free\n", m_body);
    free(m_body);
  }
  
}

bool wb_attribute::operator==(const wb_attribute& x) const
{
  if ( m_orep->oid().vid == x.m_orep->oid().vid &&
       m_orep->oid().oix == x.m_orep->oid().oix &&
       m_size == x.m_size &&
       m_offset == x.m_offset)
    return true;
  return false;
}

wb_attribute& wb_attribute::operator=(const wb_attribute& x)
{
  if ( x.m_orep)
    x.m_orep->ref();
  if ( m_orep)
    m_orep->unref();
  if ( x.m_adrep)
    x.m_adrep->ref();
  if ( m_adrep)
    m_adrep->unref();
  m_orep = x.m_orep;
  m_adrep = x.m_adrep;
  m_sts = x.m_sts;
  m_size = x.m_size;
  m_offset = x.m_offset;
  m_idx = x.m_idx;
  m_tid = x.m_tid;
  m_original_tid = x.m_original_tid;
  m_elements = x.m_elements;
  m_type = x.m_type;
  m_flags = x.m_flags;
  m_bix = x.m_bix;
  if (m_body) {
    //printf("a: %8.8x free ==\n", m_body);
    free(m_body);
  }
  m_body = 0;
  
  return *this;
}

void wb_attribute::check() const
{
  if ( evenSts())
    throw wb_error( m_sts);
}

//
// Return object identifier of attribute.
//
pwr_sAttrRef wb_attribute::aref() const
{
  check();

  pwr_sAttrRef ar;
  aref( &ar);

  return ar;
}

pwr_sAttrRef *wb_attribute::aref(pwr_sAttrRef *arp) const
{
  check();

  memset( arp, 0, sizeof(*arp));
    
  arp->Objid = m_orep->oid();
  arp->Offset = m_offset;
  arp->Size = m_size;
  arp->Body = m_orep->cid() | m_bix;

  if ( m_flags & PWR_MASK_POINTER)
    arp->Flags.b.Indirect = 1;

  if ( m_flags & PWR_MASK_ARRAY &&
       m_idx == -1)
    arp->Flags.b.Array = 1;

  if ( m_tid == m_orep->cid()) {
    arp->Flags.b.Object = 1;

    arp->Body = m_tid;
  }
  else if ( cdh_tidIsCid( m_tid))
    arp->Flags.b.ObjectAttr = 1;

  if ( m_shadowed)
    arp->Flags.b.Shadowed = 1;

  if ( m_flags & PWR_MASK_CASTATTR)
    arp->Flags.b.CastAttr = 1;

  return arp;
}

pwr_tOid wb_attribute::aoid() const
{
  check();
  return m_orep->oid();
}

size_t wb_attribute::size() const
{
  check();
  return m_size;
}

size_t wb_attribute::offset() const
{
  check();
  return m_offset;
}

pwr_eType wb_attribute::type() const
{
  check();
  return m_type;
}

pwr_tTid wb_attribute::tid() const
{
  check();
  return m_tid;
}

pwr_tTid wb_attribute::originalTid() const
{
  check();
  return m_original_tid;
}

int wb_attribute::nElement() const
{
  check();
  return m_elements;
}

int wb_attribute::index() const
{
  if ( m_adrep)
    return m_adrep->index();
  return 0;
}

int wb_attribute::flags() const
{
  check();
  return m_flags;
}

pwr_tAix wb_attribute::aix() const
{
  throw wb_error_str("wb_attribute::aix() NYI");
  return 0; // Fix
}

pwr_tCid wb_attribute::cid() const
{
  check();
  return m_orep->cid();
}

pwr_eBix wb_attribute::bix() const
{
  check();

  //if (!m_adrep || m_flags & PWR_MASK_SUBCLASS)
    return m_bix;

  // return m_adrep->bix();
}

pwr_tOid wb_attribute::boid() const
{
  throw wb_error_str("wb_attribute::boid() NYI");

  pwr_tOid oid;
  return oid;  // Fix
}

pwr_tCid wb_attribute::subClass() const
{
  if ( m_adrep)
    return m_adrep->subClass();
  return 0;
}

bool wb_attribute::checkXref() const
{
  throw wb_error_str("wb_attribute::checkXref() NYI");
  return true; // Fix
}

pwr_sAttrXRef *wb_attribute::xref() const
{
  throw wb_error_str("wb_attribute::xref() NYI");
  return (pwr_sAttrXRef*)0; // Fix
}

pwr_sObjXRef *wb_attribute::oxref() const
{
  throw wb_error_str("wb_attribute::oxref() NYI");
  return (pwr_sObjXRef*)0; // Fix
}

void *wb_attribute::value( void *p)
{
  pwr_eBix bix;
  pwr_tStatus sts;
  check();

  if (!p) {
    if (!m_body && m_orep->vrep()->type() == ldh_eVolRep_Db) {
      m_body = (void *)calloc(1, m_size);
      //printf("a: %8.8x alloc %d\n", m_body, m_size);
    }
    p = m_body;
  }
  
  if ( m_adrep == 0) {
    if ( m_bix == pwr_eBix_dev)
      return m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_dev, p);
    else
      return m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_rt, p);
  }
  

  // if (m_flags & PWR_MASK_SUBCLASS)
  bix = m_bix;
  //else
  //  bix = m_adrep->bix();

  return m_orep->vrep()->readAttribute( &sts, m_orep, bix, m_offset, m_size, p);
}

void *wb_attribute::value(void *vp, size_t size, pwr_tStatus *sts)
{
  return 0;
}

void wb_attribute::castId( pwr_tCastId *castid) 
{
  m_orep->vrep()->readAttribute( &m_sts, m_orep, m_bix, 
					m_offset - sizeof(pwr_tCastId), sizeof(pwr_tCastId), 
					castid);
}    
    
string wb_attribute::toString() const
{
  throw wb_error_str("wb_attribute::toString() NYI");
  string a;
  return a;
}

pwr_tStatus wb_attribute::fromString(string) const
{
  throw wb_error_str("wb_attribute::fromString() NYI");    
  pwr_tStatus sts;
  return sts;
}

pwr_tStatus wb_attribute::fromString(char *) const
{
  throw wb_error_str("wb_attribute::fromString() NYI");    
  pwr_tStatus sts;
  return sts;
}
   
wb_attribute wb_attribute::after() const
{
  pwr_tStatus sts;

  check();
  if (m_adrep == 0)
    return wb_attribute();

  wb_adrep* adrep = m_adrep->next(&sts);
  if (EVEN(sts))
    return wb_attribute();
  
  wb_attribute a(LDH__SUCCESS, m_orep, adrep);
  a.m_bix = m_bix;
  
#if 0
  // Fix for sub classes
  if (m_flags & PWR_MASK_SUBCLASS) {
    a.m_flags |= PWR_MASK_SUBCLASS;
    a.m_bix = m_bix;
    if ((m_flags & PWR_MASK_ARRAY) && m_idx != -1) // array element
      a.m_offset = (m_offset - m_size * m_idx) + m_adrep->size();
    else
      a.m_offset = m_offset + m_size;
  }
#endif
  return a;
}

wb_attribute wb_attribute::before() const
{
  pwr_tStatus sts;

  check();
  if (m_adrep == 0)
    return wb_attribute();

  wb_adrep* adrep = m_adrep->prev(&sts);
  if (EVEN(sts))
    return wb_attribute();
  
  wb_attribute a(LDH__SUCCESS, m_orep, adrep);
  a.m_bix = m_bix;
  
#if 0
  // Fix for sub classes
  if (m_flags & PWR_MASK_SUBCLASS) {
    a.m_flags |= PWR_MASK_SUBCLASS;
    a.m_bix = m_bix;
    if ((m_flags & PWR_MASK_ARRAY) && m_idx != -1) // array element
      a.m_offset = (m_offset - m_size * m_idx) - adrep->size();
    else
      a.m_offset = m_offset - adrep->size();
  }
#endif

  return a;
}   


wb_attribute wb_attribute::first(int idx) const
{
  if (!cdh_tidIsCid( m_tid))
    return wb_attribute();    

  return wb_attribute(*this, idx, NULL);
    
}


wb_attribute wb_attribute::child(const char* name, int idx) const
{
  if (!cdh_tidIsCid( m_tid))
    return wb_attribute();    

  return wb_attribute(*this, idx, name);
    
}


const char *wb_attribute::name() const
{
  check();

  return m_orep->name();
}

const char *wb_attribute::attrName() const
{
  check();

  static char str[200];
  
  if ( m_adrep) {
    strcpy( str, m_adrep->subName());
    if (m_flags & PWR_MASK_ARRAY && m_idx != -1)
      sprintf(&str[strlen(str)], "[%d]", m_idx);
  }
  else
    strcpy( str, "");

  return str;
}


wb_name wb_attribute::longName()  const
{
  check();

  if ( !m_adrep)
    return m_orep->longName();

  char str[512];
  int len;

  len = sprintf( str, "%s.%s", m_orep->longName().c_str(), m_adrep->subName());
  if (m_flags & PWR_MASK_ARRAY && m_idx != -1)
    sprintf(&str[len], "[%d]", m_idx);
  
  wb_name n( str);
  if ( m_shadowed)
    n.setShadowed( true);
  return n;
}

void wb_attribute::name(const char *name)
{
}

void wb_attribute::name(wb_name *name)
{
}

