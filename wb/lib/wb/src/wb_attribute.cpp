#include "wb_attribute.h"
#include "pwr.h"


wb_attribute::wb_attribute()
{
}

wb_attribute::wb_attribute(const wb_attribute&)
{
}

wb_attribute::wb_attribute(pwr_tStatus sts, wb_orep * const orep) :
    m_orep(orep), m_sts(sts)
{
}

wb_attribute::wb_attribute(pwr_tStatus sts, wb_orep * const orep, const char *name) :
    m_orep(orep), m_sts(sts)
{
}

wb_attribute& wb_attribute::operator=(const wb_attribute&)
{
    return *this;
}

//
// Return object identifier of attribute.
//
pwr_tOid wb_attribute::aoid()
{
    return m_adrep->aoid();
}

pwr_sAttrRef wb_attribute::aref()
{
    pwr_sAttrRef aref;
    
    return aref;
}

pwr_sAttrRef *wb_attribute::aref(pwr_sAttrRef *arp)
{
    return arp;
}

size_t wb_attribute::size()
{
    return m_adrep->size();
}

int wb_attribute::offset()
{
    return m_adrep->offset();
}

int wb_attribute::type()
{
    return m_adrep->type();
}

int wb_attribute::nElement()
{
    return m_adrep->nElement();
}

int wb_attribute::index()
{
    return m_adrep->index();
}

int wb_attribute::flags()
{
    return m_adrep->flags();
}

pwr_tAix wb_attribute::aix()
{
    return m_adrep->aix();
}

pwr_tCid wb_attribute::cid()
{
    return m_adrep->cid();
}

pwr_tAix wb_attribute::bix()
{
    return m_adrep->bix();
}

pwr_tOid wb_attribute::boid()
{
    return m_adrep->boid();
}

bool wb_attribute::checkXref()
{
    return true;
}

pwr_sAttrXRef *wb_attribute::xref()
{
    return (pwr_sAttrXRef*)0;
}

pwr_sObjXRef *wb_attribute::oxref()
{
    return (pwr_sObjXRef*)0;
}

void *wb_attribute::value()
{
    return (void*)0;
}

pwr_tStatus wb_attribute::value(void *vp)
{
    pwr_tStatus sts;
    return sts;
    
}

pwr_tStatus wb_attribute::value(void *vp, size_t size)
{
    pwr_tStatus sts;
    return sts;
}

    
string wb_attribute::toString()
{
    string a;
    return a;
}

pwr_tStatus wb_attribute::fromString(string)
{
    
    pwr_tStatus sts;
    return sts;
}

pwr_tStatus wb_attribute::fromString(char *)
{
    pwr_tStatus sts;
    return sts;
}
   
wb_attribute wb_attribute::next()
{
    wb_attribute a;
    return a;
}

wb_attribute wb_attribute::prev()
{
    wb_attribute a;
    return a;
}   

wb_name wb_attribute::name()
{
    wb_name n;
    return n;
}

wb_name wb_attribute::name(ldh_eName type)
{
    wb_name n;
    return n;
}

void wb_attribute::name(const char *name)
{
}

void wb_attribute::name(wb_name *name)
{
}
