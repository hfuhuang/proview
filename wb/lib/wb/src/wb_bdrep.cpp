
#include "wb_bdrep.h"
#include "wb_adrep.h"
#include "wb_cdrep.h"
#include "wb_orep.h"
#include "wb_attrname.h"
#include "wb_merep.h"

void wb_bdrep::unref()
{
  if ( --m_nRef == 0)
    delete this;
}

wb_bdrep *wb_bdrep::ref()
{
  m_nRef++;
  return this;
}

wb_bdrep::wb_bdrep(wb_orep& o) : m_nRef(0), m_orep(&o), m_sts(LDH__SUCCESS)
{
  m_orep->ref();
}

wb_bdrep::wb_bdrep( wb_adrep *adrep) : m_nRef(0)
{
  pwr_tStatus sts;
  m_orep = adrep->m_orep->parent( &sts);
  if ( EVEN(sts)) throw wb_error(sts);
  m_sts = LDH__SUCCESS;
}

wb_bdrep::~wb_bdrep()
{
  m_orep->unref();
}

wb_adrep *wb_bdrep::adrep( pwr_tStatus *sts)
{
  wb_orep *orep = m_orep->vrep()->first( sts, m_orep);
  if ( EVEN(*sts)) return 0;

  return new wb_adrep(*orep);
}

wb_adrep *wb_bdrep::adrep( pwr_tStatus *sts, const char *aname)
{
#if 0
  wb_attrname n(aname);
  if ( n.evenSts()) {
    *sts = n.sts();
    return 0;
  }

  char fullname[120];
  wb_bdrep *bd = this;
  wb_adrep *adrep = 0;
  int offset;

  for ( int i = 0; i < n.attributes(); i++) {
    wb_name an(n.attribute(i));
    wb_orep *orep = bd->m_orep->vrep()->child( sts, bd->m_orep, an);
    if ( EVEN(*sts)) return 0;

    if ( adrep)
      delete adrep;
    adrep = new wb_adrep( *orep);
    if ( i == 0) {
      offset = adrep->offset();
      strcpy( fullname, adrep->name());
    }
    else {
      offset += adrep->offset();
      strcat( fullname, ".");
      strcat( fullname, adrep->name());
    }
    if ( n.hasAttrIndex(i)) {
      sprintf( &fullname[strlen(fullname)], "[%d]", n.attrIndex(i));
      if ( n.attrIndex(i) >= adrep->nElement() || n.attrIndex(i) < 0) {
	*sts = LDH__ATTRINDEX;
	return 0;
      }

      offset += n.attrIndex(i) * adrep->size() / adrep->nElement();
    }

    if ( (i != n.attributes() - 1) && adrep->isClass()) {
      wb_cdrep *cd = m_orep->vrep()->merep()->cdrep( sts, adrep->subClass());
      if ( EVEN(*sts)) return 0;

      if ( bd != this)
	delete bd;
      bd = cd->bdrep( sts, pwr_eBix_rt);
      if ( EVEN(*sts)) { delete cd; return 0;}

      delete cd;
    }

  }
  adrep->setSubattr( offset, fullname);
#endif

  wb_attrname n(aname);
  if ( n.evenSts()) {
    *sts = n.sts();
    return 0;
  }

  wb_bdrep *bd = this;
  wb_adrep *adrep = 0;
  wb_adrep *old = 0;

  for ( int i = 0; i < n.attributes(); i++) {
    bool next_attr = false;
    wb_name an(n.attribute(i));
    wb_orep *orep = bd->m_orep->vrep()->child( sts, bd->m_orep, an);
    while ( EVEN(*sts)) {
      // Try Super attribute
      orep = bd->m_orep->vrep()->first( sts, bd->m_orep);
      if ( EVEN(*sts)) {
	if ( bd != this) delete bd;
	if ( adrep) delete adrep;
	return 0;
      }
	
      if ( cdh_NoCaseStrcmp( orep->name(), "Super") == 0) {
	if ( adrep)
	  old = adrep;

	adrep = new wb_adrep( *orep);
	if ( old)
	  adrep->add( old);
	delete old;

	wb_cdrep *cd = m_orep->vrep()->merep()->cdrep( sts, adrep->subClass());
	if ( EVEN(*sts)) return 0;

	if ( bd != this)
	  delete bd;
	bd = cd->bdrep( sts, pwr_eBix_rt);
	if ( EVEN(*sts)) { delete cd; return 0;}

	delete cd;

	orep = bd->m_orep->vrep()->child( sts, bd->m_orep, an);
      }
      else {
	if ( adrep && adrep->flags() & PWR_MASK_CASTATTR && n.hasSuper()) {
	  // Allow additional super attributesegement for casted attributes
	  next_attr = true;
	  break;
	}
	orep->ref();
	orep->unref();
	*sts = LDH__NOSUCHATTR;
	return 0;
      }
    }
    if ( next_attr)
      continue;
    if ( adrep)
      old = adrep;

    adrep = new wb_adrep( *orep);
    if ( i != 0) {
      if ( n.hasAttrIndex( i - 1))
	adrep->add( old, n.attrIndex( i - 1));
      else
	adrep->add( old);
      delete old;
    }
    if ( n.hasAttrIndex(i) &&
	 (n.attrIndex(i) >= adrep->nElement() || n.attrIndex(i) < 0)) {
      *sts = LDH__ATTRINDEX;
      return 0;
    }

    if ( (i != n.attributes() - 1) && adrep->isClass()) {
      wb_cdrep *cd = m_orep->vrep()->merep()->cdrep( sts, adrep->subClass());
      if ( EVEN(*sts)) return 0;

      if ( bd != this)
	delete bd;
      bd = cd->bdrep( sts, pwr_eBix_rt);
      if ( EVEN(*sts)) { delete cd; return 0;}

      delete cd;
    }

  }

  return adrep;
}

wb_adrep *wb_bdrep::super( pwr_tStatus *sts)
{
  if ( bix() != pwr_eBix_rt) {
    *sts = LDH__NOSUCHATTR;
    return 0;
  }

  wb_orep *orep = m_orep->vrep()->first( sts, m_orep);
  if ( EVEN(*sts)) return 0;

  if ( cdh_NoCaseStrcmp( orep->name(), "Super") != 0) {
    *sts = LDH__NOSUCHATTR;
    orep->ref();
    orep->unref();
    return 0;
  }

  wb_adrep *adrep = new wb_adrep( *orep);
  return adrep;
}

pwr_eBix wb_bdrep::bix()
{
  return  cdh_oixToBix( m_orep->oid().oix);
}

size_t wb_bdrep::size()
{
  pwr_tStatus sts;
  pwr_sObjBodyDef body;

  m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_sys, (void *) &body);
  if ( EVEN(sts)) throw wb_error(sts);

  return body.Size;
}

int wb_bdrep::nAttribute()
{
  pwr_tStatus sts;
  int attr_count = 0;
  wb_orep *old;

  wb_orep *orep = m_orep->vrep()->first( &sts, m_orep);
  while ( ODD(sts)) {
    switch ( orep->cid()) {
    case pwr_eClass_Param:
    case pwr_eClass_Intern:
    case pwr_eClass_Input:
    case pwr_eClass_Output:
    case pwr_eClass_ObjXRef:
    case pwr_eClass_AttrXRef:
    case pwr_eClass_Buffer:
      attr_count++;
      break;
    default:
      ;
    }
    old = orep;
    orep = orep->after( &sts);
    old->ref();
    old->unref();
  }
  return attr_count;
}

pwr_tOid wb_bdrep::boid() 
{ 
  return m_orep->oid();
}

pwr_tCid wb_bdrep::bcid() 
{ 
  return m_orep->cid();
}


const char* wb_bdrep::name() const
{
  return m_orep->name();
}

wb_name wb_bdrep::longName() const
{
  return m_orep->longName();
}
