/* 
 * Proview   $Id: wb_gobj.cpp,v 1.2 2007-01-24 12:38:31 claes Exp $
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

/* wb_gobj.c -- handle connections
   This module handles the connect function in foe.
   The connect function is a relation between two objects that
   is not displayed by graphic connection in foe.  */
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "flow_ctx.h"
#include "wb_ldh.h"
#include "wb_foe_msg.h"
#include "wb_vldh.h"
#include "wb_foe.h"
#include "wb_goen.h"
#include "wb_gre.h"
#include "wb_gobj.h"
#include "wb_nav.h"
#include "wb_wtt.h"

#define	BEEP	    putchar( '\7' );

#define	GOBJ_MAX_METHOD 31

typedef int (* gobj_tMethod)( WFoe *, vldh_t_node, unsigned long);

int	gobj_get_object_m0( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m1( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m2( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m3( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m4( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m5( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m6( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m7( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m8( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m9( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m10( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m11( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m12( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m13( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m14( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m15( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m16( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m17( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m18( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m19( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m20( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m21( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m22( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m23( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m24( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m25( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m26( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m27( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m28( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m29( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m30( WFoe *foe, vldh_t_node node, unsigned long index);
int	gobj_get_object_m31( WFoe *foe, vldh_t_node node, unsigned long index);

gobj_tMethod gobj_get_object_m[40] = {
	gobj_get_object_m0,
	gobj_get_object_m1,
	gobj_get_object_m2,
	gobj_get_object_m3,
	gobj_get_object_m4,
	gobj_get_object_m5,
	gobj_get_object_m6,
	gobj_get_object_m7,
	gobj_get_object_m8,
	gobj_get_object_m9,
	gobj_get_object_m10,
	gobj_get_object_m11,
	gobj_get_object_m12,
        gobj_get_object_m13,
	gobj_get_object_m14,
	gobj_get_object_m15,
	gobj_get_object_m16,
 	gobj_get_object_m17,
 	gobj_get_object_m18,
 	gobj_get_object_m19,
 	gobj_get_object_m20,
 	gobj_get_object_m21,
 	gobj_get_object_m22,
 	gobj_get_object_m23,
 	gobj_get_object_m24,
 	gobj_get_object_m25,
 	gobj_get_object_m26,
 	gobj_get_object_m27,
 	gobj_get_object_m28,
 	gobj_get_object_m29,
 	gobj_get_object_m30,
 	gobj_get_object_m31,
	};

static int	gobj_expand_m0(	WFoe		*foe,
				vldh_t_node	node,
				int		compress);
static int	gobj_expand_m1(	WFoe		*foe,
				vldh_t_node	node,
				int		compress);
static int	gobj_expand_m2(	WFoe		*foe,
				vldh_t_node	node,
				int		compress);
/*_Local procedues_______________________________________________________*/


//
//	Function used in class template PlcPgm's.
//	If a connection to a template object is made, this is marked
//	with the vid of a reference volume. The reference is transfered
//	at compilation to the PlcFo or PlcMain object.
//
static pwr_tStatus gobj_ref_replace( ldh_tSesContext ldhses, 
				     vldh_t_node node, pwr_sAttrRef *attrref)
{
  pwr_tCid cid;
  int size;
  char name[32];
  vldh_t_plc plc;
  pwr_tOid parent, plcparent;
  pwr_tStatus sts;

  plc = (node->hn.wind)->hw.plc;

  sts = ldh_ObjidToName( ldhses, attrref->Objid, ldh_eName_Object, name, sizeof(name),
			 &size);
  if ( ODD(sts) &&  strcmp( name, "Template") == 0) {
    sts = ldh_GetObjectClass( ldhses, attrref->Objid, &cid);
    if ( EVEN(sts)) return sts;

    sts = ldh_GetParent( ldhses, attrref->Objid, &parent);
    if ( EVEN(sts)) return sts;

    sts = ldh_GetParent( ldhses, plc->lp.oid, &plcparent);
    if ( EVEN(sts)) return sts;

    if ( cdh_ObjidIsEqual( parent, plcparent)) {
      // Own template object, use $PlcFo reference
      attrref->Objid.vid = ldh_cPlcFoVolume;
      attrref->Objid.oix = cid;
      return FOE__REPLACED;
    }
    else {
      // Other template object, use $PlcMain reference
      attrref->Objid.vid = ldh_cPlcMainVolume;
      attrref->Objid.oix = cid;
      return FOE__REPLACED;
    }
  }
  return FOE__SUCCESS;
}

static int gobj_get_select( WFoe *foe, pwr_sAttrRef *attrref, int *is_attr)
{
  pwr_tStatus sts;
  char str[200];
  vldh_t_plc plc = foe->gre->wind->hw.plc;

  if ( foe->nav_palette_managed) {
    sts = foe->navctx->get_select( attrref, is_attr);
    if ( ODD(sts))
      return sts;
  }

  sts = ((Wtt *)plc->hp.hinactx)->get_select_first( attrref, is_attr);
  if ( ODD(sts))
    return sts;

  sts = foe->get_selection( str, sizeof(str));
  if ( ODD(sts)) {
    sts = ldh_NameToAttrRef( foe->gre->wind->hw.ldhses, str, attrref);
    if ( ODD(sts)) {
      if ( strchr( str, '.') != 0)
	*is_attr = 1;
      else
	*is_attr = 0;
    }
    return sts;
  }
  return sts;
}

//
//      This routine make som kind of binding that is not a graphic
//	connection. Calls the connect method for the class of
//	the object. The connect method is stored in graphbody of
//	the class object (parameter connectmethod).
//	
int	gobj_get_object( 
	WFoe		*foe,
	vldh_t_node	node,
	unsigned long	index
)
{
  int			sts, size, connectmethod;
  pwr_tClassId		bodyclass;
  pwr_sGraphPlcNode 	*graphbody;
  vldh_t_plc	plc;

  /* Fix to avoid crash if foe is started form hied */
  plc = (node->hn.wind)->hw.plc;
  if ( plc->hp.hinactx == 0 ) {
    foe->message( "Foe must be started from the navigator to connect");
    return FOE__SUCCESS;
  }
 
  sts = ldh_GetClassBody( (node->hn.wind)->hw.ldhses,
			  node->ln.cid, "GraphPlcNode", 
			  &bodyclass, (char **)&graphbody, &size);
  if( EVEN(sts) ) return sts;

  connectmethod = graphbody->connectmethod;
  if ( connectmethod > GOBJ_MAX_METHOD ) return 0;

  sts = (gobj_get_object_m[connectmethod]) ( foe, node, index);
  
  return sts;
}

//
// Description:	Method for objects with nothing to connect
//
int	gobj_get_object_m0( WFoe *foe, vldh_t_node node, unsigned long index)
{
  foe->message( "Nothing to connect for this object");
  BEEP;
  return FOE__SUCCESS;
}

//
//	Method for getdi. Inserts the selected di-object in the
//	navigator in the parameter DiObject in a GetDi object.
//
int	gobj_get_object_m1( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;


  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select a Di object in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if ( EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Di) {
    foe->message( "Selected object is not a di object");
    BEEP;
    return 0;
  }

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "DiObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);
  
  return FOE__SUCCESS;
}

//
// 	Method for getdo, setdo, resdo and stodo.
//	Inserts the selected do-object in the
//	navigator in the parameter DiObject in a GetDo object.
//
int	gobj_get_object_m2( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select a Do object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is a do object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if ( EVEN(sts)) return sts;

  if ( !(cid == pwr_cClass_Do ||
	 cid == pwr_cClass_Po)) {
    foe->message( "Selected object is not a do object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "DoObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;
  
  foe->gre->node_update( node);
  
  return FOE__SUCCESS;
}

//
//	Method for getdv, setdv, resdv, stodv. Inserts the selected dv-object 
//	in the navigator in the parameter DvObject in a GetDv object.
//
int	gobj_get_object_m3( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select a Dv object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is a dv object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Dv) {
    foe->message( "Selected object is not a dv object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "DvObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);
  
  return FOE__SUCCESS;
}

//
//	Method for getai. Inserts the selected ai-object in the
//	navigator in the parameter AiObject in a GetAi object.
//
int	gobj_get_object_m4( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an Ai object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is a ai object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Ai) {
    foe->message( "Selected object is not an ai object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "AiObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;
  
  foe->gre->node_update( node);
  
  return FOE__SUCCESS;
}

//
//	Method for getao, stoao, cstoao. Inserts the selected ao-object in the
//	navigator in the parameter AoObject in a GetAo object.
//
int	gobj_get_object_m5( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an Ao object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is an ao object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Ao) {
    foe->message( "Selected object is not an ao object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "AoObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
//	Method for getav, stoav, cstoav. Inserts the selected av-object in the
//	navigator in the parameter AvObject in a GetAv object.
//
int	gobj_get_object_m6( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an Av object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is an av object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Av) {
    foe->message( "Selected object is not an Av object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "AvObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
// Description:	Method for getdp, getap, setdp, setap etc.
//		Places the objdid in the first found parameter in devboy
//		with type objdid.
//
int	gobj_get_object_m7( WFoe *foe, vldh_t_node node, unsigned long index)
{
  ldh_tSesContext	ldhses;
  ldh_sParDef 	*bodydef;
  int 		rows;
  int		type;
  int		i, sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;
  pwr_tTid	tid;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if ( EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  if ( cdh_tidIsCid( tid)) {
    foe->message( "Select an attribute in the navigator");
    BEEP;
    return sts;
  }

  /* Get first attribute in devbody of type pwr_eType_AttrRef */
  sts = ldh_GetObjectBodyDef( ldhses,
			      node->ln.cid, "DevBody", 1, 
			      &bodydef, &rows);
  if ( EVEN(sts) ) return sts;

  for ( i = 0; i < rows; i++) {
    switch ( bodydef[i].ParClass ) {
    case pwr_eClass_Input:
      type = bodydef[i].Par->Input.Info.Type;
      break;
    case pwr_eClass_Intern:
      type = bodydef[i].Par->Intern.Info.Type;
      break;
    case pwr_eClass_Output:
      type = bodydef[i].Par->Output.Info.Type;
      break;
    default:
      ;
    }
    if ( type ==  pwr_eType_AttrRef ) {
      if ( cdh_IsClassVolume( node->ln.oid.vid)) {
	gobj_ref_replace( ldhses, node, &attrref);
	if ( EVEN(sts)) return sts;
      }

      /* Set the parameter value */
      sts = ldh_SetObjectPar( ldhses,
			      node->ln.oid, 
			      "DevBody",
			      bodydef[i].ParName,
			      (char *)&attrref, sizeof(attrref)); 
      if ( EVEN(sts)) return sts;

      foe->gre->node_update( node);
      break;
    }
  }
  free((char *)bodydef);
  return FOE__SUCCESS;
}

//
// Description:	Method for ShowPlcpgm to get the resetobject.
//
int	gobj_get_object_m8( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tObjid	objdid;
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select a Di, Do or Dv object in the navigator");
    BEEP;
    return sts;
  }
  objdid = attrref.Objid;

  ldhses =(node->hn.wind)->hw.ldhses;

  /* Check that the objdid is a di,do or dv object */
  sts = ldh_GetObjectClass( ldhses, objdid, &cid);
  if (EVEN(sts)) return sts;

  if ( ( cid != pwr_cClass_Di) &&
       ( cid != pwr_cClass_Do) &&
       ( cid != pwr_cClass_Po) &&
       ( cid != pwr_cClass_Dv) ) {
    foe->message( "Reset object has to be a di, do or dv.");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  plc->lp.oid, 
			  "DevBody",
			  "ResetObject",
			  (char *)&objdid, sizeof(objdid)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
// Description:	Method for getpi
//	Method for getpi. Inserts the selected co-object in the
//	navigator in the parameter PiObject in a GetPi object.
//
int	gobj_get_object_m9( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select a Co object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is a co object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Co) {
    foe->message( "Selected object is not a co object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "CoObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
//	Connect method for a Function object with temlate plc code.
//
int	gobj_get_object_m10( WFoe *foe, vldh_t_node node, unsigned long index)
{
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  pwr_sAttrRef	nattrref;
  int		is_attr;
  char		*aname;
  int		size;
  char		msg[450];
  pwr_tOid	woid;
  pwr_tCid	cid;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts) || is_attr == 1) { 
    foe->message( "Select an object in the navigator");
    BEEP;
    return sts;
  }

  /* Set the PlcConnect attribute in the current object */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "RtBody", "PlcConnect",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) { 
    foe->message( "No PlcConnect attribute in object");
    BEEP;
    return sts;
  }

  /* Set the PlcConnect attribute in the connected object */
  nattrref = cdh_ObjidToAref( node->ln.oid);
  sts = ldh_SetObjectPar( ldhses, attrref.Objid,
			  "RtBody", "PlcConnect",
			  (char *)&nattrref, sizeof(nattrref)); 

  foe->gre->node_update( node);

  /* Remove the subwindow which might hold old references */
  sts = ldh_GetChild( ldhses, node->ln.oid, &woid);
  if ( ODD(sts)) {
    sts = ldh_GetObjectClass( ldhses, woid, &cid);
    if ( EVEN(sts)) return sts;

    if ( cid == pwr_cClass_windowplc) {
      sts = ldh_DeleteObjectTree( ldhses, woid);
      if ( EVEN(sts)) return sts;
    }
  }

  sts = ldh_AttrRefToName( ldhses, &attrref, cdh_mNName, &aname, &size);
  if ( EVEN(sts)) return sts;

  sprintf( msg, "Connected to: %s", aname);
  foe->message( msg);

  return FOE__SUCCESS;
}

//
//	Method for reset_so. If one object (including or excluding selection
//	of the reset_so object) is selected in foe this
//	object is taken as the orderobject and inserted in the
//	parameter OrderObject in the reset_so. Otherwise the selected
//	object in the navigator is taken as the orderobject.
//	
int	gobj_get_object_m11( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tObjid	objdid;
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  unsigned long		node_count;
  vldh_t_node		*nodelist;
  vldh_t_node		object;
  pwr_sAttrRef		attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  foe->gre->get_selnodes( &node_count, &nodelist);

  if ( ((node_count == 1) && (*nodelist == node)) ||
       ( node_count == 0) ) {
    /* Take the orderobject from the navigator */
    sts = gobj_get_select( foe, &attrref, &is_attr);
    if ( EVEN(sts)) { 
      foe->message( "Select an order object in the navigator or in the current window");
      BEEP;
      return sts;
    }
    objdid = attrref.Objid;
  }
  else if ( (node_count == 2) && 
	    (( *nodelist == node) || ( *(nodelist + 1) == node))) {
    /* Check if the other node is a orderobject */
    if ( *nodelist == node)
      object = *(nodelist + 1);
    else
      object = *nodelist;
    if ( object->ln.cid == pwr_cClass_order) {
      objdid = object->ln.oid;
    }
    else {
      foe->message( "Select an order object in the navigator or in the current window first");
      BEEP;
      return 0;
    }
  }
  else if ( node_count == 1) {
    /* Check if the other node is a orderobject */
    object = *nodelist;
    if ( object->ln.cid == pwr_cClass_order) {
      objdid = object->ln.oid;
    }
    else {
      foe->message( "Select an order object in the navigator or in the current window first");
      BEEP;
      return 0;
    }
  }
  else {
    foe->message( "Select an order object in the navigator or in the current window first");
    BEEP;
    return 0;
  }
  if ( node_count > 0) free((char *) nodelist);
	
  /* Check that the objdid is a do object */
  sts = ldh_GetObjectClass( ldhses, objdid, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_order) {
    foe->message( "Selected object is not an order object");
    BEEP;
    return 0;
  }
	
  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "OrderObject",
			  (char *)&objdid, sizeof(objdid)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);
  foe->gre->unselect();

  return FOE__SUCCESS;
}

//
//	Method for PageRef. If one object (including or excluding selection
//	of the PageRef object) is selected in foe this
//	object is taken as the documentobject and inserted in the
//	parameter PageAttr in the PageRef. Otherwise the selected
//	object in the navigator is taken as the document-object.
//	
int	gobj_get_object_m12( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tObjid	objdid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  unsigned long  	node_count;
  vldh_t_node    	*nodelist;
  vldh_t_node    	object;
  pwr_tAName     	name;
  int		size;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  foe->gre->get_selnodes( &node_count, &nodelist);

  if ( ((node_count == 1) && (*nodelist == node)) ||
       ( node_count == 0) ) {
    /* Take the document from the navigator */
    sts = gobj_get_select( foe, &attrref, &is_attr);
    if ( EVEN(sts)) { 
      foe->message( "Select a document object in the navigator or in the current window first");
      BEEP;
      return sts;
    }
    objdid = attrref.Objid;
  }
  else if ( (node_count == 2) && 
	    (( *nodelist == node) || ( *(nodelist + 1) == node))) {
    /* Check if the other node is a documentobject */
    if ( *nodelist == node)
      object = *(nodelist + 1);
    else
      object = *nodelist;
    if ( vldh_check_document( ldhses,  object->ln.oid)) {
      objdid = object->ln.oid;
    }
    else {
      foe->message( "Select a document object in the navigator or in the current window first");
      BEEP;
      return 0;
    }
  }
  else if ( node_count == 1) {
    /* Check if the other node is a orderobject */
    object = *nodelist;
    if ( vldh_check_document( ldhses,  object->ln.oid)) {
      objdid = object->ln.oid;
    }
    else {
      foe->message( "Select a document object in the navigator or in the current window first");
      BEEP;
      return 0;
    }
  }
  else {
    foe->message( "Select a document object in the navigator or in the current window first");
    BEEP;
    return 0;
  }
  if ( node_count > 0) free((char *) nodelist);
	

  /* Check that the objdid is a do object */
  if ( !vldh_check_document( ldhses,  objdid)) {
    foe->message( "Selected object is not a document object");
    BEEP;
    return 0;
  }
	
  sts = ldh_ObjidToName( ldhses, objdid, ldh_eName_ArefVol,
			 name, sizeof(name), &size);
  if ( EVEN(sts)) return sts;

  strcat( name, ".Page");
  sts = ldh_NameToAttrRef( ldhses, name, &attrref);
  if ( EVEN(sts)) return sts;

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid,
			  "DevBody",
			  "PageAttr",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);
  foe->gre->unselect();

  return FOE__SUCCESS;
}

//
//	Method for GetData. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m13( WFoe *foe, vldh_t_node node, unsigned long index)
{
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;


  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  /* Take the object from the navigator */
  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an object in the navigator");
    BEEP;
    return sts;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "DataObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);
  foe->gre->unselect();

  return FOE__SUCCESS;
}

//
//	Method for GetAgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m14( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId 	create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an analog signal or attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;
	
  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( tid) {
  case pwr_eType_Float32:
    /* Create a StoAp */
    create_classid = pwr_cClass_GetAp;
    strcpy( parname, "ApObject");
    break;
  case pwr_eType_Int8:
  case pwr_eType_UInt8:
  case pwr_eType_Int16:
  case pwr_eType_UInt16:
  case pwr_eType_Int32:
  case pwr_eType_UInt32:
    /* Create a GetIpToA */
    create_classid = pwr_cClass_GetIpToA;
    strcpy( parname, "IpObject");
    break;
  case pwr_cClass_Ai:
    strcpy( parname, "AiObject");
    create_classid = pwr_cClass_GetAi;
    break;
  case pwr_cClass_Ao:
    strcpy( parname, "AoObject");
    create_classid = pwr_cClass_GetAo;
    break;
  case pwr_cClass_Av:
    strcpy( parname, "AvObject");
    create_classid = pwr_cClass_GetAv;
    break;
  default:
    foe->message( "Select an analog signal or attribute in the navigator");
    BEEP;
    return 0;
  }

  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;

  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node) {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  new_node->ln.oid, 
			  "DevBody",
			  parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;
	
  foe->gre->node_update( new_node);
  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for GetDgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m15( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId 	create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select a digital signal or attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;
	
  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( tid) {
  case pwr_eType_Boolean:
    /* Create a GetDp */
    create_classid = pwr_cClass_GetDp;
    strcpy( parname, "DpObject");
    break;
  case pwr_cClass_Di:
    strcpy( parname, "DiObject");
    create_classid = pwr_cClass_GetDi;
    break;
  case pwr_cClass_Do:
    strcpy( parname, "DoObject");
    create_classid = pwr_cClass_GetDo;
    break;
  case pwr_cClass_Dv:
    strcpy( parname, "DvObject");
    create_classid = pwr_cClass_GetDv;
    break;
  default:
    foe->message( "Select a digital signal or attribute in the navigator");
    BEEP;
    return 0;
  }
  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;

  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node) {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  new_node->ln.oid, 
			  "DevBody",
			  parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( new_node);
  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for StoAgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m16( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId 	create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select a digital signal or attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( tid)  {
  case pwr_eType_Float32:
    /* Create a StoAp */
    strcpy( parname, "Object");
    create_classid = pwr_cClass_stoap;
    break;
  case pwr_eType_Int8:
  case pwr_eType_UInt8:
  case pwr_eType_Int16:
  case pwr_eType_UInt16:
  case pwr_eType_Int32:
  case pwr_eType_UInt32:
    /* Create a StoAtoIp */
    strcpy( parname, "Object");
    create_classid = pwr_cClass_StoAtoIp;
    break;
  case pwr_cClass_Ai:
    strcpy( parname, "AiObject");
    create_classid = pwr_cClass_stoai;
    break;
  case pwr_cClass_Ao:
    strcpy( parname, "AoObject");
    create_classid = pwr_cClass_stoao;
    break;
  case pwr_cClass_Av:
    strcpy( parname, "AvObject");
    create_classid = pwr_cClass_stoav;
    break;
  default:
    foe->message( "Select a digital signal or attribute in the navigator");
    BEEP;
    return 0;
  }

  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;

  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node) {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  new_node->ln.oid, 
			  "DevBody",
			  parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( new_node);
  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for StoDgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m17( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId 	create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select a digital signal in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( tid)  {
  case pwr_eType_Boolean:
    /* Create a StoDp */
    strcpy( parname, "Object");
    create_classid = pwr_cClass_stodp;
    break;
  case pwr_cClass_Di:
    strcpy( parname, "DiObject");
    create_classid = pwr_cClass_stodi;
    break;
  case pwr_cClass_Do:
    strcpy( parname, "DoObject");
    create_classid = pwr_cClass_stodo;
    break;
  case pwr_cClass_Dv:
    strcpy( parname, "DvObject");
    create_classid = pwr_cClass_stodv;
    break;
  default:
    foe->message( "Select a digital signal in the navigator");
    BEEP;
    return 0;
  }

  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;
	
  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node) {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  new_node->ln.oid, 
			  "DevBody",
			  parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( new_node);

  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for getsv, stosv, cstosv. Inserts the selected sv-object in the
//	navigator in the parameter SvObject in a GetSv object.
//
int	gobj_get_object_m18( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select Sv object in the navigator");
    BEEP;
    return sts;
  }


  /* Check that the objdid is an av object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Sv) {
    foe->message( "Selected object is not a Sv object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "SvObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
//	Method for GetSgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m19( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId    create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select string value or attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( tid)  {
  case pwr_eType_String:
    /* Create a GetSp */
    create_classid = pwr_cClass_GetSp;
    strcpy( parname, "SpObject");
    break;
  case pwr_cClass_Sv:
    strcpy( parname, "SvObject");
    create_classid = pwr_cClass_GetSv;
    break;
  default:
    foe->message( "Select string value or attribute in the navigator");
    BEEP;
    return 0;
  }
  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;

  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node) {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  new_node->ln.oid, 
			  "DevBody",
			  parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;
	
  foe->gre->node_update( new_node);

  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for StoSgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m20( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId 	create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 	
    foe->message( "Select a string value or attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( tid) {
  case pwr_eType_String:
    /* Create a StoSp */
    strcpy( parname, "Object");
    create_classid = pwr_cClass_stosp;
    break;
  case pwr_cClass_Sv:
    strcpy( parname, "SvObject");
    create_classid = pwr_cClass_stosv;
    break;
  default:
    foe->message( "Select a string value or attribute in the navigator");
    BEEP;
    return 0;
  }

  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;

  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node)  {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  new_node->ln.oid, 
			  "DevBody",
			  parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( new_node);

  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for getiv, stoiv, cstoiv. Inserts the selected iv-object in the
//	navigator in the parameter IvObject in a GetIv object.
//
int	gobj_get_object_m21( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an Iv object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is an iv object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Iv) {
    foe->message( "Selected object is not a Iv object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "IvObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
//	Method for getii, stoii, cstoii. Inserts the selected ii-object in the
//	navigator in the parameter IiObject in a GetIi object.
//
int	gobj_get_object_m22( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an Ii object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is an ii object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Ii) {
    foe->message( "Selected object is not a Ii object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "IiObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
//	Method for getio, stoio, cstoio. Inserts the selected io-object in the
//	navigator in the parameter IoObject in a GetIo object.
//
int	gobj_get_object_m23( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an Io object in the navigator");
    BEEP;
    return sts;
  }

  /* Check that the objdid is an io object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_Io) {
    foe->message( "Selected object is not a Io object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "IoObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
//	Method for GetIgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m24( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId 	create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an integer signal or attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( tid) {
  case pwr_eType_Int32:
  case pwr_eType_UInt32:
  case pwr_eType_Int16:
  case pwr_eType_UInt16:
  case pwr_eType_Int8:
  case pwr_eType_UInt8:
  case pwr_eType_Enum:
  case pwr_eType_Mask:
    /* Create a GetIp */
    create_classid = pwr_cClass_GetIp;
    strcpy( parname, "IpObject");
    break;
  case pwr_cClass_Ii:
    strcpy( parname, "IiObject");
    create_classid = pwr_cClass_GetIi;
    break;
  case pwr_cClass_Io:
    strcpy( parname, "IoObject");
    create_classid = pwr_cClass_GetIo;
    break;
  case pwr_cClass_Iv:
    strcpy( parname, "IvObject");
    create_classid = pwr_cClass_GetIv;
    break;
  default:
    foe->message( "Select an integer signal or attribute in the navigator");
    BEEP;
    return 0;
  }

  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;

  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node) {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses, new_node->ln.oid, 
			  "DevBody", parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( new_node);

  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for StoIgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m25( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId 	create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an integer signal in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( tid) {
  case pwr_eType_Int32:
  case pwr_eType_UInt32:
  case pwr_eType_Int16:
  case pwr_eType_UInt16:
  case pwr_eType_Int8:
  case pwr_eType_UInt8:
  case pwr_eType_Enum:
  case pwr_eType_Mask:
    /* Create a StoIp */
    strcpy( parname, "Object");
    create_classid = pwr_cClass_StoIp;
    break;
  case pwr_cClass_Ii:
    strcpy( parname, "IiObject");
    create_classid = pwr_cClass_stoii;
    break;
  case pwr_cClass_Io:
    strcpy( parname, "IoObject");
    create_classid = pwr_cClass_stoio;
    break;
  case pwr_cClass_Iv:
    strcpy( parname, "IvObject");
    create_classid = pwr_cClass_stoiv;
    break;
  default:
    foe->message( "Select an integer signal in the navigator");
    BEEP;
    return 0;
  }

  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;
	
  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node) {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses, new_node->ln.oid, 
			  "DevBody", parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;
	
  foe->gre->node_update( new_node);

  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Connect method for a connected simulation function object.
//
int	gobj_get_object_m26( WFoe *foe, vldh_t_node node, unsigned long index)
{
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  pwr_sAttrRef	nattrref;
  int		is_attr;
  char		*aname;
  int		size;
  char		msg[450];
  pwr_tOid	woid;
  pwr_tCid	cid;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts) || is_attr == 1) { 
    foe->message( "Select an object in the navigator");
    BEEP;
    return sts;
  }

  /* Set the PlcConnect attribute in the current object */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "RtBody", "PlcConnect",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) { 
    foe->message( "No PlcConnect attribute in object");
    BEEP;
    return sts;
  }
  /* Set the SimConnect attribute in the connected object */
  nattrref = cdh_ObjidToAref( node->ln.oid);
  sts = ldh_SetObjectPar( ldhses, attrref.Objid,
			  "RtBody", "SimConnect",
			  (char *)&nattrref, sizeof(nattrref)); 

  /* Remove the subwindow which might hold old references */
  sts = ldh_GetChild( ldhses, node->ln.oid, &woid);
  if ( ODD(sts)) {
    sts = ldh_GetObjectClass( ldhses, woid, &cid);
    if ( EVEN(sts)) return sts;

    if ( cid == pwr_cClass_windowplc) {
      sts = ldh_DeleteObjectTree( ldhses, woid);
      if ( EVEN(sts)) return sts;
    }
  }

  foe->gre->node_update( node);

  sts = ldh_AttrRefToName( ldhses, &attrref, cdh_mNName, &aname, &size);
  if ( EVEN(sts)) return sts;

  sprintf( msg, "Connected to: %s", aname);
  foe->message( msg);

  return FOE__SUCCESS;
}

//
//	Method for Disabled. The selected attribute in the navigator is 
//	inserted.
//	
int	gobj_get_object_m27( WFoe *foe, vldh_t_node node, unsigned long index)
{
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;
  ldh_sAttrRefInfo info;


  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  /* Take the object from the navigator */
  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select an object in the navigator");
    BEEP;
    return sts;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Check that attribute can be disbled */
  sts = ldh_GetAttrRefInfo( ldhses, &attrref, &info);
  if ( EVEN(sts)) return sts;

  if ( !(info.flags & PWR_MASK_DISABLEATTR)) {
    foe->message( "Attribute can't be disabled");
    BEEP;
    return 0;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "Object",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);
  foe->gre->unselect();

  return FOE__SUCCESS;
}

//
//	Method for GetATgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m28( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId    create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select string value or attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( node->ln.cid) {
  case pwr_cClass_GetATgeneric:
    switch ( tid)  {
    case pwr_eType_Time:
      /* Create a GetATp */
      create_classid = pwr_cClass_GetATp;
      strcpy( parname, "ATpObject");
      break;
    case pwr_cClass_ATv:
      strcpy( parname, "ATvObject");
      create_classid = pwr_cClass_GetATv;
      break;
    default:
      foe->message( "Select a time value or attribute in the navigator");
      BEEP;
      return 0;
    }
    break;
  case pwr_cClass_GetDTgeneric:
    switch ( tid)  {
    case pwr_eType_DeltaTime:
      /* Create a GetDTp */
      create_classid = pwr_cClass_GetDTp;
      strcpy( parname, "DTpObject");
      break;
    case pwr_cClass_DTv:
      strcpy( parname, "DTvObject");
      create_classid = pwr_cClass_GetDTv;
      break;
    default:
      foe->message( "Select a time value or attribute in the navigator");
      BEEP;
      return 0;
    }
    break;
  }
  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;

  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node) {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  new_node->ln.oid, 
			  "DevBody",
			  parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;
	
  foe->gre->node_update( new_node);

  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for StoATgeneric. The selected object in the navigator is 
//	inserted.
//	
int	gobj_get_object_m29( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tTid	tid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  vldh_t_con  	*con_list;
  vldh_t_con  	*con_ptr;
  vldh_t_node	new_node, source, dest;
  unsigned long	source_point, dest_point;
  unsigned long	con_count;
  int		j;
  pwr_sAttrRef	attrref;
  pwr_tClassId 	create_classid;
  char		parname[40];
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 	
    foe->message( "Select a string value or attribute in the navigator");
    BEEP;
    return sts;
  }

  sts = ldh_GetAttrRefTid( ldhses, &attrref, &tid);
  if (EVEN(sts)) return sts;

  if ( !cdh_tidIsCid(tid)) {
    sts = ldh_GetAttrRefType( ldhses, &attrref, (pwr_eType *)&tid);
    if (EVEN(sts)) return sts;
  }

  sts = vldh_get_cons_node( node, &con_count, &con_list);
  if ( EVEN(sts)) return sts;

  switch ( node->ln.cid) {
  case pwr_cClass_StoATgeneric:
    switch ( tid) {
    case pwr_eType_Time:
      /* Create a StoATp */
      strcpy( parname, "Object");
      create_classid = pwr_cClass_StoATp;
      break;
    case pwr_cClass_ATv:
      strcpy( parname, "ATvObject");
      create_classid = pwr_cClass_StoATv;
      break;
    default:
      foe->message( "Select a time value or attribute in the navigator");
      BEEP;
      return 0;
    }
    break;
  case pwr_cClass_StoDTgeneric:
    switch ( tid) {
    case pwr_eType_DeltaTime:
      /* Create a StoDTp */
      strcpy( parname, "Object");
      create_classid = pwr_cClass_StoDTp;
      break;
    case pwr_cClass_DTv:
      strcpy( parname, "DTvObject");
      create_classid = pwr_cClass_StoDTv;
      break;
    default:
      foe->message( "Select a time value or attribute in the navigator");
      BEEP;
      return 0;
    }
    break;
  default:
    return 0;
  }
  sts = foe->gre->create_node( create_classid, 
				     node->ln.x, node->ln.y, &new_node);
  if ( EVEN(sts)) return sts;

  /* Create new connections */
  con_ptr = con_list;
  for ( j = 0; j < (int)con_count; j++) {
    if ( (*con_ptr)->hc.source_node == node)  {
      source = new_node;
      source_point = 0;
      dest = (*con_ptr)->hc.dest_node;
      dest_point = (*con_ptr)->lc.dest_point;
    }
    else {
      dest = new_node;
      dest_point = 0;
      source = (*con_ptr)->hc.source_node;
      source_point = (*con_ptr)->lc.source_point;
    }
    sts = foe->gre->create_con( (*con_ptr)->lc.cid, 
				      source, source_point,
				      dest, dest_point, (*con_ptr)->lc.drawtype);
    if (EVEN(sts)) return sts;
    con_ptr++;
  }
	    
  /* Remove old node and connections */
  foe->gre->delete_node( node);
  foe->popupmenu_node = 0;

  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  new_node->ln.oid, 
			  "DevBody",
			  parname,
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( new_node);

  if ( con_count > 0) free((char *) con_list);

  return FOE__SUCCESS;
}

//
//	Method for getatv, stoatv, cstoatv. Inserts the selected atv-object in the
//	navigator in the parameter AtvObject in a GetATv object.
//
int	gobj_get_object_m30( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select ATv object in the navigator");
    BEEP;
    return sts;
  }


  /* Check that the objdid is an av object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_ATv) {
    foe->message( "Selected object is not a ATv object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "ATvObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
//	Method for getatv, stodtv, cstodtv. Inserts the selected dtv-object in the
//	navigator in the parameter DtvObject in a GetDTv object.
//
int	gobj_get_object_m31( WFoe *foe, vldh_t_node node, unsigned long index)
{
  pwr_tClassId	cid;
  ldh_tSesContext	ldhses;
  int		sts;
  vldh_t_plc	plc;
  pwr_sAttrRef	attrref;
  int		is_attr;

  /* Get the selected object in the navigator */
  plc = (node->hn.wind)->hw.plc;
  ldhses =(node->hn.wind)->hw.ldhses;

  sts = gobj_get_select( foe, &attrref, &is_attr);
  if ( EVEN(sts)) { 
    foe->message( "Select DTv object in the navigator");
    BEEP;
    return sts;
  }


  /* Check that the objdid is an av object */
  sts = ldh_GetAttrRefTid( ldhses, &attrref, &cid);
  if (EVEN(sts)) return sts;

  if ( cid != pwr_cClass_DTv) {
    foe->message( "Selected object is not a DTv object");
    BEEP;
    return 0;
  }
	
  if ( cdh_IsClassVolume( node->ln.oid.vid)) {
    gobj_ref_replace( ldhses, node, &attrref);
    if ( EVEN(sts)) return sts;
  }

  /* Set the parameter value */
  sts = ldh_SetObjectPar( ldhses,
			  node->ln.oid, 
			  "DevBody",
			  "DTvObject",
			  (char *)&attrref, sizeof(attrref)); 
  if ( EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
// Description:	Method for objects with nothing to expand
//
int	gobj_expand(	WFoe		*foe,
			vldh_t_node	node,
			int		compress)
{
  int	sts;

  switch( node->ln.cid)
    {
    case pwr_cClass_GetDv:
    case pwr_cClass_GetDi:
    case pwr_cClass_GetDo:
    case pwr_cClass_GetAv:
    case pwr_cClass_GetAi:
    case pwr_cClass_GetAo:
    case pwr_cClass_GetAp:
    case pwr_cClass_GetDp:
    case pwr_cClass_GetIp:
    case pwr_cClass_GetIpToA:
    case pwr_cClass_GetPi:
    case pwr_cClass_stodv:
    case pwr_cClass_stoav:
    case pwr_cClass_stoai:
    case pwr_cClass_stoao:
    case pwr_cClass_stodi:
    case pwr_cClass_stodo:
    case pwr_cClass_setdv:
    case pwr_cClass_setdi:
    case pwr_cClass_setdo:
    case pwr_cClass_resdv:
    case pwr_cClass_resdi:
    case pwr_cClass_resdo:
    case pwr_cClass_stopi:
    case pwr_cClass_stoap:
    case pwr_cClass_stodp:
    case pwr_cClass_setdp:
    case pwr_cClass_resdp:
    case pwr_cClass_StoIp:
    case pwr_cClass_StoAtoIp:
    case pwr_cClass_cstoao:
    case pwr_cClass_cstoav:
    case pwr_cClass_cstoai:
    case pwr_cClass_CStoIp:
    case pwr_cClass_CStoAtoIp:
    case pwr_cClass_cstoap:
    case pwr_cClass_GetData:
    case pwr_cClass_stosv:
    case pwr_cClass_cstosv:
    case pwr_cClass_GetSv:
    case pwr_cClass_stosp:
    case pwr_cClass_cstosp:
    case pwr_cClass_GetSp:
    case pwr_cClass_stonumsp:
    case pwr_cClass_cstonumsp:
    case pwr_cClass_GetIi:
    case pwr_cClass_GetIo:
    case pwr_cClass_GetIv:
    case pwr_cClass_stoii:
    case pwr_cClass_stoio:
    case pwr_cClass_stoiv:
    case pwr_cClass_cstoii:
    case pwr_cClass_cstoio:
    case pwr_cClass_cstoiv:
    case pwr_cClass_GetATv:
    case pwr_cClass_GetDTv:
    case pwr_cClass_StoATv:
    case pwr_cClass_StoDTv:
    case pwr_cClass_CStoATv:
    case pwr_cClass_CStoDTv:
      sts = gobj_expand_m1( foe, node, compress);
      break;
    case pwr_cClass_and:
    case pwr_cClass_or:
    case pwr_cClass_sum:
      sts = gobj_expand_m2( foe, node, compress);
      break;
    default:
      sts = gobj_expand_m0( foe, node, compress);
    }
  return sts;
}

//
// Description:	Method for objects with nothing to expand
//
static int	gobj_expand_m0(	WFoe		*foe,
				vldh_t_node	node,
				int		compress)
{
  foe->message( "Nothing to expand for this object");
  BEEP;
  return FOE__SUCCESS;
}

//
// Description:	Method for Get, Sto, Set and Res objects.
//
static int	gobj_expand_m1(	WFoe		*foe,
				vldh_t_node	node,
				int		compress)
{
  ldh_sParDef 	*bodydef;
  int 		rows;
  char		attrname[80];	
  pwr_tUInt32	*segments_p, segments;
  int		sts, size;
  ldh_tSesContext	ldhses;

  ldhses =(node->hn.wind)->hw.ldhses;
	
  /* Get the devbody parameters for this class */
  sts = ldh_GetObjectBodyDef( ldhses,
			      node->ln.cid, "DevBody", 1,
			      &bodydef, &rows);
  if ( EVEN(sts) ) return sts;

  strcpy( attrname, bodydef[0].ParName);
  strcat( attrname, "Segments");

  sts = ldh_GetObjectPar( ldhses, node->ln.oid, "DevBody",
			  attrname, (char **)&segments_p, &size);
  if (EVEN(sts)) return sts;
	  
  segments = *segments_p;
  free((char *) bodydef);	
  free((char *) segments_p);
  if ( !compress)
    segments++;
  else
    {
      segments--;
      if ( segments < 1)
	segments = 1;
    }
  sts = ldh_SetObjectPar( ldhses, node->ln.oid, "DevBody",
			  attrname, (char *)&segments, sizeof(segments));
  if (EVEN(sts)) return sts;

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

//
// Description:	Method for and and or objects.
//
static int	gobj_expand_m2(	WFoe		*foe,
				vldh_t_node	node,
				int		compress)
{
  int		sts, size;
  ldh_tSesContext	ldhses;
  pwr_sPlcNode	*nodebuffer;
  pwr_eClass	cid;
  int		max_input = 8;
  unsigned int	m;
  int		i, i_max;

  ldhses =(node->hn.wind)->hw.ldhses;
	
  sts = ldh_GetObjectBuffer( ldhses, node->ln.oid, "DevBody",
			     "PlcNode", &cid,
			     (char **)&nodebuffer, &size);
  if (EVEN(sts)) return sts;
	  
  i_max = 0;
  for ( i = 0; i < max_input; i++) {
    m = 1 << i;
    if ( nodebuffer->mask[0] & m)
      i_max = i;
  }
  if ( !compress) {
    if ( i_max == max_input - 1) {
      free((char *) nodebuffer);
      return FOE__SUCCESS;
    }
    nodebuffer->mask[0] |= 1 << (i_max + 1);
  }
  else {
    if ( i_max == 0) {
      free((char *) nodebuffer);
      return FOE__SUCCESS;
    }
    nodebuffer->mask[0] &= ~(1 << (i_max));
  }

  sts = ldh_SetObjectBuffer( ldhses, node->ln.oid, "DevBody",
			     "PlcNode",
			     (char *)nodebuffer);
  if (EVEN(sts)) return sts;

  free((char *) nodebuffer);

  foe->gre->node_update( node);

  return FOE__SUCCESS;
}

