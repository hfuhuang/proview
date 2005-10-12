/* 
 * Proview   $Id: wb_goenm11.c,v 1.2 2005-10-12 13:01:32 claes Exp $
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

/* wb_goenm11.c 

   This module creates a nodetype from the objecttype specification
   and the instanceobject's mask.
	
	-----------------
	!		!
 	!  'text'       !
 	!  'more text'  !
	!		!
	-----------------
   */

#include <stdio.h>
#include <math.h>
#include "wb_foe_macros.h"

#include <Xm/Xm.h>

#include "wb_ldh.h"
#include "wb_foe_msg.h"
#include "wb_vldh.h"
#include "wb_goen.h"
#include "wb_gre.h"

extern int goen_create_nodetype_m7( 
    pwr_sGraphPlcNode	*graphbody,
    pwr_tClassId	class,
    ldh_tSesContext	ldhses,
    flow_tCtx		ctx,
    unsigned long 	*mask,
    unsigned long	subwindowmark,
    unsigned long	node_width,
    flow_tNodeClass	*node_class,
    vldh_t_node		node);


extern int goen_get_point_info_m7( gre_ctx		 grectx,
				   pwr_sGraphPlcNode	*graphbody,
				   unsigned long	 point,
				   unsigned long 	 *mask,
				   unsigned long	 node_width,
				   goen_conpoint_type	 *info_pointer,
				   vldh_t_node		 node);

extern int	goen_get_parameter_m7( pwr_sGraphPlcNode	*graphbody,
			       pwr_tClassId		class,
			       ldh_tSesContext		ldhses,
			       unsigned long		con_point,
			       unsigned long		*mask,
			       unsigned long		*par_type,
			       unsigned long		*par_inverted,
			       unsigned long		*par_index);

/*_Local variables_______________________________________________________*/




/*_Methods defined for this module_______________________________________*/

/*************************************************************************
*
* Name:		goen_create_nodetype_m11()
*
* Type		
*
* Type		Parameter	IOGF	Description
*    pwr_sGraphPlcNode	*graphbody	Pointer to objecttype data
*    Widget	        widget			Neted widget
*    unsigned long 	*mask			Mask for drawing inputs/outputs
*    int		color			Highlight color
*    Cursor		cursor			Hot cursor
*    unsigned long      *node_type_id		Nodetypeid for created nodetype
*
* Description:
*	Create a nodetype
*
**************************************************************************/


int goen_create_nodetype_m11( 
    pwr_sGraphPlcNode	*graphbody,
    pwr_tClassId	class,
    ldh_tSesContext	ldhses,
    flow_tCtx		ctx,
    unsigned long 	*mask,
    unsigned long	subwindowmark,
    unsigned long	node_width,
    flow_tNodeClass	*node_class,
    vldh_t_node		node)
{
  int		sts, size;
  float		*frame_width_ptr;
  float		frame_width;
  float		*frame_height_ptr;
  float		frame_height;
  char		name[80];
  flow_tNodeClass nc;
  float		f_height;
  float		f_width;
  int		line_width;
  flow_eDrawType line_type;
  float	y_offs      = GOEN_F_GRID/2;
  static int	idx = 0;

  sts = ldh_ClassIdToName(ldhses, class, name, sizeof(name), &size);
  if ( EVEN(sts) ) return sts;
  sprintf( &name[strlen(name)], "%d", idx++);

  sts = ldh_GetObjectPar( ldhses, node->ln.oid, "DevBody","AreaWidth",
			(char **)&frame_width_ptr, &size); 
  if ( ODD(sts)) {
    frame_width = *frame_width_ptr;
    free((char *) frame_width_ptr);
  }
  else 
    frame_width = 0.0;

  sts = ldh_GetObjectPar( ldhses, node->ln.oid, "DevBody", "AreaHeight",
			  (char **)&frame_height_ptr, &size); 
  if ( ODD(sts)) {
    frame_height = *frame_height_ptr;
    free((char *) frame_height_ptr);
  }
  else 
    frame_height = 0.0;
  

  line_type = flow_eDrawType_Line;
  line_width = 2;

  f_height = frame_height;
  f_width = frame_width;
  
  flow_CreateNodeClass( ctx, name, flow_eNodeGroup_Common, 
			&nc);
  flow_SetNoConObstacle( nc, 1);

  /* Draw the rectangle for the frame */
  flow_AddRect( nc, 0, -y_offs, f_width, y_offs * 2, 
		flow_eDrawType_Line, 2, flow_mDisplayLevel_1);
  // flow_AddLine( nc, 0, -y_offs, f_width, -y_offs, line_type, line_width);
  flow_AddLine( nc, f_width, -y_offs, f_width, f_height-y_offs, line_type, line_width);
  flow_AddLine( nc, f_width, f_height-y_offs, 0, f_height-y_offs, line_type, line_width);
  flow_AddLine( nc, 0, f_height-y_offs, 0, -y_offs, line_type, line_width);

  switch( graphbody->graphindex) {
  case 7:
    sts = goen_create_nodetype_m7( graphbody, class, ldhses, ctx, mask,
				   subwindowmark, node_width, &nc, node);
    if ( EVEN(sts)) return sts;
    break;
  default:
    return 0;
  }

  *node_class = nc;
  return GOEN__SUCCESS;
}



/*************************************************************************
*
* Name:		goen_get_point_info_m11()
*
* Type		
*
* Type		Parameter	IOGF	Description
*    pwr_sGraphPlcNode	*graphbody	Pointer to objecttype data
*    unsigned long	point			Connection point nr
*    unsigned long 	*mask			Mask for drawing inputs/outputs
*    goen_conpoint_type	*info_pointer		Pointer to calculated data
*
* Description:
*	Calculates relativ koordinates for a connectionpoint and investigates
*	the connectionpoint type.
*
**************************************************************************/
int goen_get_point_info_m11( grectx, graphbody, point, mask, node_width,
			info_pointer, node)
    gre_ctx		 grectx;
    pwr_sGraphPlcNode	*graphbody;
    unsigned long	 point;
    unsigned long 	 *mask;
    unsigned long	 node_width;
    goen_conpoint_type	 *info_pointer;
    vldh_t_node		 node;
{
  switch( graphbody->graphindex) {
  case 7:
    return goen_get_point_info_m7( grectx, graphbody, point, mask, node_width,
				  info_pointer, node);
  default: ;
  }
  return GOEN__NOPOINT;
}



/*************************************************************************
*
* Name:		goen_get_parameter_m11()
*
* Type		
*
* Type		Parameter	IOGF	Description
*    pwr_sGraphPlcNode	*graphbody	Pointer to objecttype data
*    unsigned long	point			Connection point nr
*    unsigned long 	*mask			Mask for drawing inputs/outputs
*    unsigned long	*par_type		Input or output parameter	
*    godd_parameter_type **par_pointer		Pointer to parameter data
*
* Description:
*	Gets pointer to parameterdata for connectionpoint.
*
**************************************************************************/
int	goen_get_parameter_m11( graphbody, class, ldhses,
			con_point, mask, par_type, par_inverted, par_index)
pwr_sGraphPlcNode	*graphbody;
pwr_tClassId		class;
ldh_tSesContext		ldhses;
unsigned long		con_point;
unsigned long		*mask;
unsigned long		*par_type;
unsigned long		*par_inverted;
unsigned long		*par_index;
{
  switch( graphbody->graphindex) {
  case 7:
    return goen_get_parameter_m7( graphbody, class, ldhses, con_point, mask, par_type,
				  par_inverted, par_index);
  default: ;
  }
  return GOEN__NOPOINT;
}


/*************************************************************************
*
* Name:		goen_get_location_point_m11()
*
* Type		
*
* Type		Parameter	IOGF	Description
*    pwr_sGraphPlcNode	*graphbody	Pointer to objecttype data
*    goen_point_type	*info_pointer		Locationpoint
*
* Description:
*	Calculates kooridates for locationpoint relativ geomtrical center.
*
**************************************************************************/
int goen_get_location_point_m11( grectx, graphbody, mask, node_width, 
		info_pointer, node)
    gre_ctx		 grectx;
    pwr_sGraphPlcNode	 *graphbody;
    unsigned long 	 *mask;
    unsigned long	 node_width;
    goen_point_type	 *info_pointer;
    vldh_t_node		 node;
{
	return GOEN__SUCCESS;
}
