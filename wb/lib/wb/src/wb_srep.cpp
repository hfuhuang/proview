/** 
 * Proview   $Id: wb_srep.cpp,v 1.5 2005-09-01 14:57:58 claes Exp $
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

#include "wb_srep.h"
#include "wb_vrep.h"
#include "wb_error.h"
#include "wb_ldh_msg.h"
#include "wb_object.h"

wb_srep::wb_srep(wb_vrep *vrep) : m_access(ldh_eAccess_ReadOnly), m_utility(ldh_eUtility__),
                                  m_nUpdate(0), m_refcount(0),
                                  m_editorContext(0), m_thisSessionCb(0), m_otherSessionCb(0),
				  m_events(0)
{
  m_vrep = vrep->ref();
  m_vrep->addSrep( this);
}

wb_srep::wb_srep(wb_srep *srep) // Fix ????? access utility ...
{
  m_vrep = ((wb_vrep*)srep)->ref();
}

wb_srep::~wb_srep()
{
  pwr_tStatus sts = LDH__SUCCESS;

  if (m_nUpdate != 0) {
    m_vrep->abort(&sts);
  }
    
  if (m_vrep != 0) {
    m_vrep->unref();
    m_vrep->removeSrep( this);
  }
}

wb_srep::operator wb_vrep*() const
{
  return m_vrep;
}


void wb_srep::unref()
{
  if (--m_refcount == 0)
    delete this;
}

wb_srep *wb_srep::ref()
{
  m_refcount++;
  return this;
}

ldh_eAccess wb_srep::access(pwr_tStatus *sts) const
{
  *sts = LDH__SUCCESS; // Fix
  return m_access;
}

bool wb_srep::access(pwr_tStatus *sts, ldh_eAccess access) // Fix
{
  pwr_tStatus lsts;

  if ( ldh_eAccess__ < access && access < ldh_eAccess_) { 
    if ( !m_vrep->accessSupported( access)) {
      *sts = LDH__ACCESS;
      return false;
    }

    if ( access == ldh_eAccess_ReadWrite) {
      // Check that no other session is ReadWrite
      for ( wb_srep *srep = m_vrep->srep(&lsts); srep; srep = m_vrep->nextSrep( &lsts, srep)) {
	if ( srep != this && srep->access(&lsts) == ldh_eAccess_ReadWrite) {
	  *sts = LDH__OTHERSESS;
	  return false;
	}
      }
    }
    else if ( access == ldh_eAccess_SharedReadWrite) {
      // Check that any ReadWrite session is empty
      for ( wb_srep *srep = m_vrep->srep(&lsts); srep; srep = m_vrep->nextSrep( &lsts, srep)) {
	if ( srep != this && srep->access(&lsts) == ldh_eAccess_ReadWrite) {
	  if ( !srep->isEmpty( &lsts)) {
	    *sts = LDH__SESSNOTEMPTY;
	    return false;
	  }
	}
      }
    }
    m_access = access;
    *sts = LDH__SUCCESS;
    return true;
  }
  *sts = LDH__NYI;
  return false;
}

ldh_eUtility wb_srep::utility(pwr_tStatus *sts) const // Fix
{
  *sts = LDH__SUCCESS;
  return m_utility;
}

bool wb_srep::utility(pwr_tStatus *sts, ldh_eUtility utility) // Fix
{
  if ( ldh_eUtility__ < utility && utility < ldh_eUtility_) {
    m_utility = utility;
    *sts = LDH__SUCCESS;
    return true;
  }
  *sts = LDH__NYI;
  return false;
}

bool wb_srep::isReadonly(pwr_tStatus *sts) const
{
  return (m_access == ldh_eAccess_ReadOnly);
}

bool wb_srep::isEmpty(pwr_tStatus *sts) const
{
  return (m_nUpdate == 0);
}

bool wb_srep::commit(pwr_tStatus *sts)
{
  bool ok;
    
  ok = m_vrep->commit(sts);
  if (ok) {
    m_nUpdate = 0;
  }
  return ok;
}

bool wb_srep::abort(pwr_tStatus *sts) // Fix was inline...
{
  bool ok;
    
  ok = m_vrep->abort(sts);
  if (ok) {
    m_nUpdate = 0;
  }
  return ok;
}


ldh_sEvent *wb_srep::newEvent()
{
  ldh_sEvent *e = (ldh_sEvent *) calloc( 1, sizeof( ldh_sEvent));

  // Add event last in eventlist
  if ( m_events) {
    ldh_sEvent *ep = m_events;
    while ( ep->nep)
      ep = ep->nep;
    ep->nep = e;
  }
  else
    m_events = e;
  return e;
}

void wb_srep::deleteEvents()
{
  ldh_sEvent *nep;
  ldh_sEvent *ep = m_events;
  while ( ep) {
    nep = ep->nep;
    free( ep);
    ep = nep;
  }
  m_events = 0;
}

void wb_srep::eventNewFamily( ldh_sEvent *ep, wb_object o)
{

  if ( !m_thisSessionCb)
    return;

  if ( !ep)
    return;

  wb_object parent = o.parent();
  if ( parent)
    ep->NewParent = parent.oid();

  wb_object before = o.before();
  if ( before)
    ep->NewLsibling = before.oid();

  wb_object after = o.after();
  if ( after)
    ep->NewRsibling = after.oid();
  
}

void wb_srep::eventOldFamily( ldh_sEvent *ep, wb_object o)
{

  if ( !m_thisSessionCb)
    return;

  if (ep == NULL)
    return;

  wb_object parent = o.parent();
  if ( parent)
    ep->OldParent = parent.oid();

  wb_object before = o.before();
  if ( before)
    ep->OldLsibling = before.oid();

  wb_object after = o.after();
  if ( after)
    ep->OldRsibling = after.oid();
}

void wb_srep::eventSend( ldh_sEvent *ep)
{
  if (!ep)
    return;

  if ( ep != m_events)
    // Send later
    return;

  if ( m_thisSessionCb)
    (m_thisSessionCb)( m_editorContext, ep);

  deleteEvents();
}

pwr_tStatus wb_srep::sendThisSession( void *editorContext, ldh_sEvent *event)
{
  if ( m_thisSessionCb)
    return (m_thisSessionCb)( editorContext, event);
  return LDH__SUCCESS;
}

pwr_tStatus wb_srep::sendOtherSession( void *editorContext, ldh_sEvent *event)
{
  if ( m_otherSessionCb)
    return (m_otherSessionCb)( editorContext, event);
  return LDH__SUCCESS;
}

void wb_srep::eventSendAllSessions( ldh_eEvent event)
{
  pwr_tStatus sts;
  ldh_sEvent *ep;

  ep = newEvent();
  ep->Event = event;

  wb_srep *srep = m_vrep->srep( &sts);
  while ( ODD(sts)) {
    if ( srep == this)
      srep->sendThisSession( m_editorContext, ep);
    else
      srep->sendOtherSession( m_editorContext, ep);
    srep = m_vrep->nextSrep( &sts, srep);
  }
  deleteEvents();
}

void wb_srep::eventSendSession( ldh_eEvent event)
{
  ldh_sEvent		*ep;

  ep = newEvent();

  ep->Event = event;

  if ( m_thisSessionCb)
    (m_thisSessionCb)( m_editorContext, ep);

  deleteEvents();
}

ldh_sEvent *wb_srep::eventStart( pwr_tOid oid, ldh_eEvent event)
{
  ldh_sEvent		*ep;

  if ( !m_thisSessionCb)
    return 0;

  ep = newEvent();

  ep->Event   = event;
  ep->Object  = oid;

  return ep;
}











