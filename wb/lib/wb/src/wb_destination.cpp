#include "wb_destination.h"
#include "wb_object.h"
#include "pwr_systemclasses.h"
#include "pwr_baseclasses.h"

wb_destination::wb_destination(pwr_tOid oid, ldh_eDest code) :
  m_oid(oid), m_code(code)
{
}

wb_destination::wb_destination(wb_object &o, ldh_eDest code) :
  m_oid(o.oid()), m_code(code)
{    
}

bool wb_destination::canAdopt( pwr_tCid cid)
{
  switch ( cid) {
  case pwr_cClass_MountObject:
  case pwr_cClass_CreateVolume:
    return false;
  default:
    return true;
  }
}
