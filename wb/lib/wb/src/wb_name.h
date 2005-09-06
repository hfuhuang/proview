/* 
 * Proview   $Id: wb_name.h,v 1.11 2005-09-06 10:43:31 claes Exp $
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

#ifndef wb_name_h
#define wb_name_h

#include "pwr.h"
#include "wb_error.h"
#include "wb_nrep.h"
#include "wb_ldh_msg.h"

/* Describe the format of a namestring.

   Let us assume we have an object of class Ai.
   The object has an attribute called FilterAttribute.

   Object name:	Eobj
   Object id  : 1234567890
   Class name :	pwrb:Class-Ai
   Class id   : 0.2:34
   Volume name:	Avol
   Volume id  : 0.123.34.63
   Parents    : Bobj, Cobj, Dobj
   Attribute  : FilterAttribute
   Index      : 2
   Offset     : 60
   Size	      : 4
   Body name  :	pwrb:Class-Ai-RtBody
   Body id    : 0.2:0.34.1

   The name of this object can be written in different ways.
   The type cdh_mName is used to define the way an object is named.

   V P O B B A I E S  I   Form  Fallback        String
   o a b o o t n s e  d   
   l t j d d t d c p  T 
   u h e y y r e a a  y 
   m   c I N i x p r  p 
   e   t d a b   e a  e 
   m u   G t    
   e t   M o    
   e   S r    

   1 * * * * * * * 0  1   Id    *               _V0.123.34.63
   1 * * * * * * * 1  1   Id    *               _V0.123.34.63:
   1 * * * * * * * 0  0   Id    *                 0.123.34.63
   1 * * * * * * * 1  0   Id    *                 0.123.34.63:

   0 * 1 * * * * * *  1   Id    *               _O0.123.34.63:1234567890
   0 * 1 * * * * * *  0   Id    *                 0.123.34.63:1234567890

   0 * 0 1 * 1 0 * *  *   Id    *               _A0.123.34.63:1234567890(_T0.2:0.34.1)
   0 * 0 1 * 1 1 * *  *   Id    *               _A0.123.34.63:1234567890(_T0.2:0.34.1)[60.4]

   1 * * * * * * * *  *   Std   Export          _V0.123.34.63:
   0 0 0 * * 1 * * *  *   Std   Export          _O0.123.34.63:1234567890
   0 0 0 0 1 1 * * *  *   Std   Export          _A0.123.34.63:1234567890(pwrb:Class-Ai-RtBody)FilterAttribute[2]

   1 1 1 0 0 1 1 0 *  *   Std   Strict          Avol:Bobj-Cobj-Dobj-Eobj.FilterAttribute[2]
   0 1 1 0 0 1 1 0 *  *   Std   Strict               Bobj-Cobj-Dobj-Eobj.FilterAttribute[2]
   0 0 1 0 0 1 1 0 *  *   Std   Strict                              Eobj.FilterAttribute[2]
   0 0 0 0 0 1 1 0 0  *   Std   Strict                                   FilterAttribute[2]
   0 0 0 0 0 1 1 0 1  *   Std   Strict                                  .FilterAttribute[2]
   0 0 0 0 0 1 0 0 0  *   Std   Strict                                   FilterAttribute
   0 0 0 0 0 1 0 0 1  *   Std   Strict                                  .FilterAttribute
   1 1 1 0 0 1 0 0 *  *   Std   Strict          Avol:Bobj-Cobj-Dobj-Eobj.FilterAttribute
   1 1 1 0 0 0 0 0 0  *   Std   Strict          Avol:Bobj-Cobj-Dobj-Eobj
   1 1 1 0 0 0 0 0 1  *   Std   Strict          Avol:Bobj-Cobj-Dobj-Eobj-
   1 1 0 0 0 0 0 0 0  *   Std   Strict          Avol:Bobj-Cobj-Dobj
   1 1 0 0 0 0 0 0 1  *   Std   Strict          Avol:Bobj-Cobj-Dobj-
   1 0 0 0 0 0 0 0 0  *   Std   Strict          Avol
   1 0 0 0 0 0 0 0 1  *   Std   Strict          Avol:
   
   1 1 1 0 0 1 1 1 0  *   Std   Strict          Avol\:Bobj\-Cobj\-Dobj\-Eobj\.FilterAttribute[2]
   0 1 1 0 0 1 1 1 0  *   Std   Strict                Bobj\-Cobj\-Dobj\-Eobj\.FilterAttribute[2]
   0 0 1 0 0 1 1 1 0  *   Std   Strict                                  Eobj\.FilterAttribute[2]
   0 0 0 0 0 1 1 1 0  *   Std   Strict                                        FilterAttribute[2]
   1 1 1 0 0 1 0 1 0  *   Std   Strict          Avol\:Bobj\-Cobj\-Dobj\-Eobj\.FilterAttribute
   1 1 1 0 0 0 0 1 0  *   Std   Strict          Avol\:Bobj\-Cobj\-Dobj\-Eobj
   1 1 0 0 0 0 0 1 0  *   Std   Strict          Avol\:Bobj\-Cobj\-Dobj
   1 0 0 0 0 0 0 1 0  *   Std   Strict          Avol
   
   1 1 1 0 0 1 1 0 *  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/Eobj.FilterAttribute[2]
   0 1 1 0 0 1 1 0 *  *   Root  Strict                /Bobj/Cobj/Dobj/Eobj.FilterAttribute[2]
   0 0 1 0 0 1 1 0 *  *   Root  Strict                                Eobj.FilterAttribute[2]
   0 0 0 0 0 1 1 0 0  *   Root  Strict                                     FilterAttribute[2]
   1 1 1 0 0 1 0 0 0  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/Eobj.FilterAttribute
   1 1 1 0 0 0 0 0 0  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/Eobj
   1 1 1 0 0 0 0 0 1  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/Eobj/
   1 1 0 0 0 0 0 0 0  *   Root  Strict          //Avol/Bobj/Cobj/Dobj
   1 1 0 0 0 0 0 0 1  *   Root  Strict          //Avol/Bobj/Cobj/Dobj/
   1 0 0 0 0 0 0 0 0  *   Root  Strict          //Avol
   1 0 0 0 0 0 0 0 1  *   Root  Strict          //Avol/
   
*/

class wb_nrep;

class wb_name : public wb_status
{
protected:
  wb_nrep *m_nrep;
  static const char* m_emptyString;
public:

  wb_name() : wb_status(LDH__NONAME), m_nrep(0) {}
  wb_name(const char*);
  wb_name(wb_nrep*);
  wb_name(const wb_name&);
    
  ~wb_name();

  const char* c_str() const;

  wb_name& operator=(const wb_name& n);

  operator bool() const { return oddSts();}
  operator wb_nrep*() const;
  bool operator==(wb_name&);
    
  // Name representation
  bool hasVolume() const throw (wb_error) { check(); return m_nrep->hasVolume();}
  bool hasPath() const throw (wb_error) { check(); return m_nrep->hasPath();}
  bool hasObject() const throw (wb_error) { check(); return m_nrep->hasObject();}
  bool hasSegment( int idx = 0) const throw (wb_error) { check(); return m_nrep->hasSegment( idx);}
  bool hasAttribute( int idx = 0) const throw (wb_error) { check(); return m_nrep->hasAttribute( idx);}
  bool hasAttrIndex( int idx = 0) const throw (wb_error) { check(); return m_nrep->hasAttrIndex( idx);}
  int attributes() { check(); return m_nrep->attributes();}
  int segments() { check(); return m_nrep->segments();}
  int attrIndex( int idx = 0) const throw (wb_error) { check(); return m_nrep->attrIndex( idx);}
  char *segment( int idx = 0, char *res = 0) const throw (wb_error) { check(); return m_nrep->segment( idx, res);}
  char *path( char *res = 0) const throw (wb_error) { check(); return m_nrep->path( res);}
  char *volume( char *res = 0) const throw (wb_error) { check(); return m_nrep->volume( res);}
  char *object( char *res = 0) const throw (wb_error) { check(); return m_nrep->object( res);}
  char *attribute( int idx = 0, char *res = 0) const throw (wb_error) { check(); return m_nrep->attribute( idx, res);}
  char *normSegment( int idx = 0, char *res = 0) const throw (wb_error) { check(); return m_nrep->normSegment( idx, res);}
  char *normPath( char *res = 0) const throw (wb_error) { check(); return m_nrep->normPath( res);}
  char *normVolume( char *res = 0) const throw (wb_error) { check(); return m_nrep->normVolume( res);}
  char *normObject( char *res = 0) const throw (wb_error) { check(); return m_nrep->normObject( res);}
  char *normAttribute( int idx = 0, char *res = 0) const throw (wb_error) { check(); return m_nrep->normAttribute( idx, res);}
  char *name() const throw (wb_error) { check(); return m_nrep->name();}
  char *normName() const throw (wb_error) { check(); return m_nrep->normName();}
  char *name( int ntype) const throw (wb_error) { check(); return m_nrep->name( ntype);}
  char *normName( int ntype) const throw (wb_error) { check(); return m_nrep->normName( ntype);}
  bool volumeIsEqual( const char *n) const throw (wb_error) { check(); return m_nrep->volumeIsEqual( n);}
  bool objectIsEqual( const char *n) const throw (wb_error) { check(); return m_nrep->objectIsEqual( n);}
  bool segmentIsEqual( const char *n, int idx = 0) const throw (wb_error) { check(); return m_nrep->segmentIsEqual( n, idx);}
  bool attributeIsEqual( const char *n, int idx = 0) const throw (wb_error) { check(); return m_nrep->attributeIsEqual( n, idx);}
  char *segmentsAll( int idx = 0) const throw (wb_error) { check(); return m_nrep->segmentsAll(idx);}
  char *attributesAll( int idx = 0) const throw (wb_error) { check(); return m_nrep->attributesAll(idx);}
  char *attributesAllTrue( int idx = 0) const throw (wb_error) { check(); return m_nrep->attributesAll(idx, 0, true);}

  // Id representation
  char *vid( char *res = 0) const throw (wb_error) { check(); return m_nrep->volume( res);}
  char *oix( char *res = 0) const throw (wb_error) { check(); return m_nrep->segment( 1, res);}
  char *bodyName( char *res = 0) const throw (wb_error) { check(); return m_nrep->segment( 2, res);}
  int bodyOffset() const throw (wb_error) { check(); return m_nrep->bodyOffset();}
  int bodySize() const throw (wb_error) { check(); return m_nrep->bodySize();}
  bool hasSuper() const throw (wb_error) { check(); return m_nrep->hasSuper();}
  void setShadowed( bool shadowed) throw (wb_error) { check(); m_nrep->setShadowed(shadowed);}

  // bool bodyId();
  // bool bodyName();
    
  static char *unatName( const char *name) { return wb_nrep::unatName( name);} 
  static bool checkObjectName( const char *name) { return wb_nrep::checkObjectName( name);}
private:
  void check() const { if ( evenSts()) throw wb_error(m_sts);}
};


inline const char* wb_name::c_str() const
{
  if (m_nrep == 0)
    return m_emptyString;
  else
    return m_nrep->name();
}



inline ostream& operator<< (ostream& os, const wb_name& name)
{
  return os << name.c_str();
}


#endif





