/** 
 * Proview   $Id: wb_c_xttgraph.c,v 1.4 2005-09-01 14:57:57 claes Exp $
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

/* wb_c_xttgraph.c -- work bench methods of the XttGraph class. */

#include <X11/Intrinsic.h>
#undef Status
#include "wb_pwrs.h"
#include "pwr_baseclasses.h"
#include "wb_ldh_msg.h"
#include "wb_ldh.h"
#include "ge.h"
#include "wb_pwrb_msg.h"
#include "wb_wsx.h"
#include "wb_wsx_msg.h"
#include "co_cdh.h"


/*----------------------------------------------------------------------------*\
  To open Ge.
\*----------------------------------------------------------------------------*/

static pwr_tStatus OpenGraph (
  ldh_sMenuCall *ip
)
{
  int 		sts;
  char		*action;
  int		size;
  char		graph_name[80];

  sts = ldh_GetObjectPar( ip->PointedSession, ip->Pointed.Objid, "RtBody",
			"Action", &action, &size); 
  if ( EVEN(sts)) return sts;

  cdh_ToLower( graph_name, action);
  free( (char *)action);
  if ( strstr( graph_name, ".pwg"))
  {
    ge_new( NULL, ip->WindowContext, ip->PointedSession, 0, graph_name);
    return 1;
  }
  return PWRB__GRAPHACTION;
}


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindMethods(XttGraph) = {
  pwr_BindMethod(OpenGraph),
  pwr_NullMethod
};





