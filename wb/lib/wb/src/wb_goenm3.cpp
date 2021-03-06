/* 
 * Proview   $Id: wb_goenm3.cpp,v 1.1 2007-01-04 07:29:03 claes Exp $
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

/* wb_goenm3.c 

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
#include "pwr.h"
#include "flow_ctx.h"
#include "flow_api.h"
#include "wb_ldh.h"
#include "wb_foe_msg.h"
#include "wb_vldh.h"
#include "wb_goen.h"
#include "wb_gre.h"
#include "wb_goenm3.h"

/*_Local variables_______________________________________________________*/


	/* 	 name        value    name in drawing */
static	float	f_defwidth   = GOEN_F_MINWIDTH * GOEN_F_OBJNAME_STRWIDTH;
static	float	f_strlength  = GOEN_F_OBJNAME_STRWIDTH;
static	float	f_delta      = GOEN_F_GRID*0.06;
static	float	f_yoffs      = GOEN_F_GRID/2 - GOEN_F_GRID*0.06;
static	float	f_height;
static	float	f_width;


/*_Methods defined for this module_______________________________________*/

/*************************************************************************
*
* Name:		goen_create_nodetype_m3()
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


int goen_create_nodetype_m3( 
    pwr_sGraphPlcNode	*graphbody,
    pwr_tClassId	cid,
    ldh_tSesContext	ldhses,
    flow_tCtx		ctx,
    unsigned int 	*mask,
    unsigned long	subwindowmark,
    unsigned long	node_width,
    flow_tNodeClass	*node_class,
    vldh_t_node		node)
{
	int		sts, size;
	int		parvalue_size;
	char		*parvalue;
	double		width;
	double		height;
	unsigned long	*text_attribute_ptr;
	unsigned long	*frame_attribute_ptr;
	unsigned long	text_attribute;
	unsigned long	frame_attribute;
	float		*frame_width_ptr;
	float		frame_width;
	float		*frame_height_ptr;
	float		frame_height;
	char		name[80];
	flow_tNodeClass	nc;
	flow_eDrawType	line_type;
	flow_eDrawType	text_type;
	int		text_size;
	int		line_width;
	flow_eAnnotType	annot_type;
	int		annot_rows;
        static int	idx = 0;

	sts = ldh_ClassIdToName(ldhses, cid, name, sizeof(name), &size);
  	if ( EVEN(sts) ) return sts;
        sprintf( &name[strlen(name)], "%d", idx++);

	/* Get the text in the parameter Text */
	sts = ldh_GetObjectPar(
			(node->hn.wind)->hw.ldhses,  
			node->ln.oid, 
			"DevBody",
			"Text",
			(char **)&parvalue, &parvalue_size); 
	if ( EVEN(sts)) return sts;

	sts = ldh_GetObjectPar(
			(node->hn.wind)->hw.ldhses,  
			node->ln.oid, 
			"DevBody",
			"TextAttribute",
			(char **)&text_attribute_ptr, &size); 
	if ( ODD(sts))
	{
	  text_attribute = *text_attribute_ptr;
	  free((char *) text_attribute_ptr);
	}
	else 
	  text_attribute = 0;

	sts = ldh_GetObjectPar(
			(node->hn.wind)->hw.ldhses,  
			node->ln.oid, 
			"DevBody",
			"FrameAttribute",
			(char **)&frame_attribute_ptr, &size); 
	if ( ODD(sts))
	{
	  frame_attribute = *frame_attribute_ptr;
	  free((char *) frame_attribute_ptr);
	}
	else 
	  frame_attribute = 0;

	sts = ldh_GetObjectPar(
			(node->hn.wind)->hw.ldhses,  
			node->ln.oid, 
			"DevBody",
			"FrameWidth",
			(char **)&frame_width_ptr, &size); 
	if ( ODD(sts))
	{
	  frame_width = *frame_width_ptr;
	  free((char *) frame_width_ptr);
	}
	else 
	  frame_width = 0.0;

	sts = ldh_GetObjectPar(
			(node->hn.wind)->hw.ldhses,  
			node->ln.oid, 
			"DevBody",
			"FrameHeight",
			(char **)&frame_height_ptr, &size); 
	if ( ODD(sts))
	{
	  frame_height = *frame_height_ptr;
	  free((char *) frame_height_ptr);
	}
	else 
	  frame_height = 0.0;

	switch ( text_attribute)
	{
	  case 0:
	    text_size =  GOEN_F_TEXTSIZE;
	    text_type = flow_eDrawType_TextHelveticaBold;
	    break;
	  case 1:
	    text_size =  GOEN_F_TEXTSIZE;
	    text_type = flow_eDrawType_TextHelvetica;
	    break;
	  case 2:
	    text_size = 5;
	    text_type = flow_eDrawType_TextHelveticaBold;
	    break;
	  case 3:
	    text_size = 8;
	    text_type = flow_eDrawType_TextHelveticaBold;
	    break;
	  default:
	    text_size = GOEN_F_TEXTSIZE;
	    text_type = flow_eDrawType_TextHelveticaBold;
	    break;
	}	
	switch ( frame_attribute)
	{
	  case 0:
	    line_type = flow_eDrawType_LineGray;
	    line_width = 3;
	    break;
	  case 1:
	    line_type = flow_eDrawType_Line;
	    line_width = 2;
	    break;
	  case 2:
	    line_type = flow_eDrawType_LineErase;
	    line_width = 1;
	    break;
	  default:
	    line_type = flow_eDrawType_LineGray;
	    line_width = GOEN_F_TEXTSIZE;
	    break;
	}
	/* Make an empty text and invisible object visible */		
	if ( (*parvalue == 0) && (frame_attribute == 2))
	{
	  line_type = flow_eDrawType_LineGray;
	  line_width = 3;
	}

        if ( parvalue_size <= 80)
	  annot_type = flow_eAnnotType_OneLine;
        else
	  annot_type = flow_eAnnotType_MultiLine;

        flow_MeasureAnnotText( ctx, parvalue,
	     	text_type, text_size, annot_type,
		&width, &height, &annot_rows);
	free((char *) parvalue);	

	f_height = (floor( (height + f_delta*2)/ GOEN_F_GRID) + 1) * GOEN_F_GRID - f_delta*2;
	f_width = co_max( width + f_strlength *2, f_defwidth) - f_delta*2;
	if ( (height < GOEN_F_GRID-f_delta*2) && (f_height > GOEN_F_GRID))
	  f_height = GOEN_F_GRID - f_delta*2; 

	if ( frame_height > f_height)
	  f_height = frame_height;
	if ( frame_width > f_width)
	  f_width = frame_width;


	flow_CreateNodeClass( ctx, name, flow_eNodeGroup_Common, 
		&nc);

	if ( ! (frame_width == 0.0 && frame_height == 0.0))
	{
	  /* Draw a rectangle that is 'hot' only on the lines */
	  /* Draw the rectangle for the fram */
	  flow_AddLine( nc, f_delta, -f_yoffs, f_width, -f_yoffs, line_type, line_width);
	  flow_AddLine( nc, f_width, -f_yoffs, f_width, f_height - f_yoffs, line_type, line_width);
	  flow_AddLine( nc, f_width, f_height - f_yoffs, f_delta, f_height - f_yoffs, line_type, line_width);
	  flow_AddLine( nc, f_delta, f_height - f_yoffs, 0, -f_yoffs, line_type, line_width);
	}
	else
	{
	  /* Draw a rectangle that is sensitive all over... */
          flow_AddRect( nc, f_delta, -f_yoffs, f_width, 
		f_height, line_type, line_width, flow_mDisplayLevel_1);
	}

        flow_AddAnnot( nc, f_strlength, (f_height - height)/2 +
		height/annot_rows - f_yoffs, 0, text_type, text_size, 
		annot_type, flow_mDisplayLevel_1);
        flow_AddConPoint( nc, f_width+f_delta, f_height / 2 - f_yoffs, 0, flow_eDirection_Right);

	*node_class = nc;
	return GOEN__SUCCESS;
}



/*************************************************************************
*
* Name:		goen_get_point_info_m3()
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
int goen_get_point_info_m3( WGre *grectx, pwr_sGraphPlcNode *graphbody, 
			    unsigned long point, unsigned int *mask, 
			    unsigned long node_width, goen_conpoint_type *info_pointer, 
			    vldh_t_node node)
{
	   info_pointer->x = 0;
	   info_pointer->y = 0;
	   info_pointer->type = CON_RIGHT;
	   return GOEN__NOPOINT;
}



/*************************************************************************
*
* Name:		goen_get_parameter_m3()
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
int	goen_get_parameter_m3( pwr_sGraphPlcNode *graphbody, pwr_tClassId cid, 
			       ldh_tSesContext ldhses, unsigned long con_point, 
			       unsigned int *mask, unsigned long *par_type, 
			       unsigned long *par_inverted, unsigned long *par_index)
{
	return GOEN__NOPOINT;
}


/*************************************************************************
*
* Name:		goen_get_location_point_m3()
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
int goen_get_location_point_m3( WGre *grectx, pwr_sGraphPlcNode *graphbody, 
				unsigned int *mask, unsigned long node_width, 
				goen_point_type *info_pointer, vldh_t_node node)
{
#if 0
	int		sts, size;
	float		ll_x,ll_y,mid_x,mid_y;

	sts = NetedMeasureNode( grectx->neted_window,
			  node->hn.node_id,
			  &ll_x,&ll_y,&mid_x,&mid_y,&f_width,&f_height);
	tst_neted( sts, "NetedMeasureNode", 204 );

	f_width -= 2* GOEN_F_LINEWIDTH;
	f_height -= 2* GOEN_F_LINEWIDTH;

	info_pointer->x = f_width / 2;
	info_pointer->y = - f_height / 2 + GOEN_F_GRID / 2.;
#endif
	return GOEN__SUCCESS;
}
