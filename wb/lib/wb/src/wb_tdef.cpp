/* 
 * Proview   $Id: wb_tdef.cpp,v 1.4 2005-09-06 10:43:32 claes Exp $
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

#include "wb_tdef.h"
#include "wb_adef.h"


wb_tdef::wb_tdef() : wb_status(LDH__NOTYPE), m_tdrep(0)
{
}

wb_tdef::wb_tdef( wb_tdrep *tdrep) : wb_status(LDH__SUCCESS), m_tdrep(tdrep) 
{
  if ( tdrep == 0)
    m_sts = LDH__NOTYPE;
  else
    tdrep->ref();
}

wb_tdef::wb_tdef( wb_adef& a)
{
  const wb_adrep *adrep = a;
  try {
    m_tdrep = new wb_tdrep( *adrep);
    m_tdrep->ref();
    m_sts = m_tdrep->sts();
  }
  catch ( wb_error& e) {
    m_sts = e.sts();
  }
}

wb_tdef::wb_tdef(wb_mvrep *mvrep, pwr_tTid tid)
{
  try {
    m_tdrep = new wb_tdrep( mvrep, tid);
    m_tdrep->ref();
    m_sts = m_tdrep->sts();
  }
  catch ( wb_error& e) {
    m_sts = e.sts();
  }
}

wb_tdef::~wb_tdef()
{
  if ( m_tdrep)
    m_tdrep->unref();
}

wb_tdef::wb_tdef(const wb_tdef& x) : wb_status(x.sts()), m_tdrep(x.m_tdrep)
{
  if ( m_tdrep)
    m_tdrep->ref();
}

wb_tdef& wb_tdef::operator=(const wb_tdef& x)
{
  if ( x.m_tdrep)
    x.m_tdrep->ref();
  if ( m_tdrep)
    m_tdrep->unref();
  m_tdrep = x.m_tdrep;
  m_sts = x.m_sts;
  return *this;
}

void wb_tdef::check() const
{
  if ( !m_tdrep) throw wb_error(m_sts);
}

size_t wb_tdef::size()
{
  check();
  return m_tdrep->size();
}

pwr_tTid wb_tdef::tid()
{
  check();
  return m_tdrep->tid();
}

const char *wb_tdef::name() const
{
  check();
  return m_tdrep->name();
}

wb_name wb_tdef::longName()
{
  check();
  return m_tdrep->longName();
}

