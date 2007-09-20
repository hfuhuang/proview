/* 
 * Proview   $Id: wb_adrep.cpp,v 1.20 2007-09-20 15:09:18 claes Exp $
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
 **/

#include "wb_adrep.h"
#include "wb_bdrep.h"
#include "wb_vrep.h"
#include "wb_cdef.h"
#include "wb_cdrep.h"
#include "wb_orep.h"

void wb_adrep::unref()
{
  if ( --m_nRef == 0)
    delete this;
}

wb_adrep *wb_adrep::ref()
{
  m_nRef++;
  return this;
}

wb_adrep::wb_adrep( wb_orep& o): m_nRef(0), m_orep(&o), m_sts(LDH__SUCCESS),
				 m_subClass(pwr_eClass__), m_isSubattr(false)
{
  m_orep->ref();
  strcpy( m_subName, "");

  pwr_tStatus sts;
  switch ( m_orep->cid()) {
  case pwr_eClass_Param:
  {
    pwr_sParam attr;

    m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_sys, (void *) &attr);
    if ( EVEN(sts)) throw wb_error(sts);

    strcpy( m_pgmname, attr.Info.PgmName);
    m_size = attr.Info.Size;
    m_type = attr.Info.Type;
    m_offset = m_suboffset = attr.Info.Offset;
    m_elements = attr.Info.Elements;
    m_paramindex = attr.Info.ParamIndex;
    m_flags = attr.Info.Flags;
    m_tid = attr.TypeRef;
    if ( m_flags & PWR_MASK_CLASS)
      m_subClass = attr.TypeRef;
    else
      m_subClass = 0;
    break;
  }
  case pwr_eClass_Intern:
  case pwr_eClass_Input:
  case pwr_eClass_Output:
  {
    pwr_sIntern attr;

    m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_sys, (void *) &attr);
    if ( EVEN(sts)) throw wb_error(sts);

    strcpy( m_pgmname, attr.Info.PgmName);
    m_size = attr.Info.Size;
    m_type = attr.Info.Type;
    m_offset = m_suboffset = attr.Info.Offset;
    m_elements = attr.Info.Elements;
    m_paramindex = attr.Info.ParamIndex;
    m_flags = attr.Info.Flags;
    m_tid = attr.TypeRef;
    if ( m_flags & PWR_MASK_CLASS)
      m_subClass = attr.TypeRef;
    else
      m_subClass = 0;

    break;
  }
  case pwr_eClass_ObjXRef:
  {
    pwr_sObjXRef attr;

    m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_sys, (void *) &attr);
    if ( EVEN(sts)) throw wb_error(sts);

    strcpy( m_pgmname, attr.Info.PgmName);
    m_size = attr.Info.Size;
    m_type = attr.Info.Type;
    m_offset = m_suboffset = attr.Info.Offset;
    m_elements = attr.Info.Elements;
    m_paramindex = attr.Info.ParamIndex;
    m_flags = attr.Info.Flags;
    m_tid = m_type;

    break;
  }
  case pwr_eClass_AttrXRef:
  {
    pwr_sAttrXRef attr;

    m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_sys, (void *) &attr);
    if ( EVEN(sts)) throw wb_error(sts);

    strcpy( m_pgmname, attr.Info.PgmName);
    m_size = attr.Info.Size;
    m_type = attr.Info.Type;
    m_offset = m_suboffset = attr.Info.Offset;
    m_elements = attr.Info.Elements;
    m_paramindex = attr.Info.ParamIndex;
    m_flags = attr.Info.Flags;
    m_tid = m_type;

    break;
  }
  case pwr_eClass_Buffer:
  {
    pwr_sBuffer attr;

    m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_sys, (void *) &attr);
    if ( EVEN(sts)) throw wb_error(sts);

    strcpy( m_pgmname, attr.Info.PgmName);
    m_size = attr.Info.Size;
    m_type = attr.Info.Type;
    m_offset = m_suboffset = attr.Info.Offset;
    m_elements = attr.Info.Elements;
    m_paramindex = attr.Info.ParamIndex;
    m_flags = attr.Info.Flags;
    m_flags |= PWR_MASK_BUFFER;
    m_subClass = attr.Class;
    m_tid = m_subClass;

    break;
  }
  default:
    throw wb_error(LDH__NYI);
  }
}

wb_adrep::~wb_adrep()
{
  m_orep->unref();
}

wb_adrep *wb_adrep::next( pwr_tStatus *sts)
{
  wb_orep *orep = m_orep->after( sts);
  if ( EVEN(*sts))
    return 0;

  wb_adrep *adrep = new wb_adrep( (wb_orep& ) *orep);
  if ( m_isSubattr) {
    adrep->m_isSubattr = true;
    adrep->m_offset = m_offset + adrep->m_offset - m_suboffset;
    strcpy( adrep->m_subName, m_subName);
    char *s;
    if ( (s = strrchr( adrep->m_subName, '.')))
      strcpy( s+1, adrep->name());
    else
      strcpy( adrep->m_subName, "");
  }
  return adrep;
}

wb_adrep *wb_adrep::prev( pwr_tStatus *sts)
{
  wb_orep *orep = m_orep->before( sts);
  if ( EVEN(*sts))
    return 0;

  wb_adrep *adrep = new wb_adrep( (wb_orep& ) *orep);
  if ( m_isSubattr) {
    adrep->m_isSubattr = true;
    adrep->m_offset = m_offset - (adrep->m_offset - m_suboffset);
    strcpy( adrep->m_subName, m_subName);
    char *s;
    if ( (s = strrchr( adrep->m_subName, '.')))
      strcpy( s+1, adrep->name());
    else
      strcpy( adrep->m_subName, "");
  }
  return adrep;
}

wb_cdrep *wb_adrep::cdrep()
{
  return new wb_cdrep( this);
}

wb_bdrep *wb_adrep::bdrep()
{
  return new wb_bdrep( this);
}

pwr_tOid wb_adrep::aoid()
{
  return m_orep->oid();
}

int wb_adrep::aix()
{
  return cdh_oixToAix( m_orep->oid().oix);
}

pwr_eBix wb_adrep::bix()
{
  return (pwr_eBix) cdh_oixToBix( m_orep->oid().oix);
}

pwr_sAttrRef wb_adrep::aref()
{
  pwr_sAttrRef aref = pwr_cNAttrRef;

  aref.Objid = pwr_cNObjid;
  aref.Body = bix();
  aref.Offset = m_offset;
  aref.Size = m_size;

  if ( m_flags & PWR_MASK_POINTER)
    aref.Flags.b.Indirect = 1;

  if ( m_flags & PWR_MASK_ARRAY)
    aref.Flags.b.Array = 1;

  if ( cdh_tidIsCid( m_tid))
    aref.Flags.b.ObjectAttr = 1;

  if ( m_flags & PWR_MASK_CASTATTR)
    aref.Flags.b.CastAttr = 1;
  if ( m_flags & PWR_MASK_DISABLEATTR)
    aref.Flags.b.DisableAttr = 1;


  return aref;
}

//
// Return object identity of body that owns this attribute
//
pwr_tOid wb_adrep::boid()
{
  pwr_tOid oid;
    

  //dbs_sBdef *b = (dbs_sBdef *)dbs_Address(sts, m_v->m_env, m_a->bdef);
    
  return oid;
}

//
// Return identity of class that owns this attribute
//
pwr_tCid wb_adrep::cid()
{
  return m_orep->cid();
}

wb_vrep *wb_adrep::vrep() const
{
  if (EVEN(m_sts)) throw wb_error(m_sts);
  return m_orep->vrep();
}

const char *wb_adrep::name() const
{
  return m_orep->name();
}

void wb_adrep::add( wb_adrep *ad, int idx)
{
  m_isSubattr = true;

  if ( ad->m_flags & PWR_MASK_ARRAY)
    m_offset += ad->m_offset + idx * ad->m_size / ad->m_elements;
  else
    m_offset += ad->m_offset;

  if ( !ad->m_isSubattr)
    strcpy( m_subName, ad->name());
  else
    strcpy( m_subName, ad->m_subName);

  if ( ad->m_flags & PWR_MASK_ARRAY)
    sprintf( &m_subName[strlen(m_subName)], "[%d]", idx);
  strcat( m_subName, ".");
  strcat( m_subName, m_orep->name());
}

const char *wb_adrep::subName() const
{
  if ( strcmp( m_subName, "") == 0)
    return m_orep->name();
  else
    return m_subName;
}

wb_name wb_adrep::longName()
{
  return m_orep->longName();
}

void *wb_adrep::body( void *p)
{
  pwr_tStatus sts;
  int size;

  switch ( m_orep->cid()) {
  case pwr_eClass_Param:
    size = sizeof( pwr_sParam);
    break;
  case pwr_eClass_Intern:
  case pwr_eClass_Input:
  case pwr_eClass_Output:
    size = sizeof( pwr_sIntern);
    break;
  case pwr_eClass_ObjXRef:
    size = sizeof( pwr_sObjXRef);
    break;
  case pwr_eClass_AttrXRef:
    size = sizeof( pwr_sAttrXRef);
    break;
  case pwr_eClass_Buffer:
    size = sizeof( pwr_sBuffer);
    break;
  default:
    throw wb_error(LDH__NYI);
  }

  return m_orep->vrep()->readBody( &sts, m_orep, pwr_eBix_sys, p);
}

bool wb_adrep::renameAttribute( pwr_tStatus *sts, wb_name &name)
{
  return m_orep->vrep()->renameObject( sts, m_orep, name);
}
