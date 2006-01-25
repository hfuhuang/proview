/* 
 * Proview   $Id: nmps_plc_display.c,v 1.1 2006-01-12 05:57:43 claes Exp $
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
 */

#if defined(OS_VMS) || defined(OS_LYNX) || defined(OS_LINUX)
#include <stdio.h>
#include <float.h>
#include <string.h>
#endif

#ifdef OS_ELN
#include stdio
#include float
#include string
#endif

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "pwr_nmpsclasses.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "rt_plc.h"
#include "co_cdh.h"
#include "nmps.h"
#include "rs_nmps_msg.h"

#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define	NMPS_OPTYPE_FRONT	0
#define	NMPS_OPTYPE_BACK	1
#define	NMPS_OPTYPE_UNIT	2
#define	NMPS_OPTYPE_FORWARD_FRONT	3
#define	NMPS_OPTYPE_FORWARD_BACK	4
#define	NMPS_OPTYPE_FORWARD_UNIT	5
#define	NMPS_OPTYPE_REVERSE_FRONT	6
#define	NMPS_OPTYPE_REVERSE_BACK	7
#define	NMPS_OPTYPE_REVERSE_UNIT	8

#define NMPS_DISP_SIZE			60
#define NMPS_DISP_CELLNUM		10

#define NMPS_DISP_DIRECT_FORW		0
#define NMPS_DISP_DIRECT_BACKW		1

#define NMPS_DISP_ORDERTYPE_NEXT 	1
#define NMPS_DISP_ORDERTYPE_PREV 	2
#define NMPS_DISP_ORDEROWNER_LINK	9999

#define NMPS_DISPFUNC_REVERSE		1

/* 		NMPS RUTINER			*/



typedef struct {
  pwr_tFloat32                        **CellP;
  pwr_tFloat32                        *Cell;
} nmps_sCellInput;

/**
  @aref celldisp CellDisp
*/
void CellDisp_init( pwr_sClass_CellDisp  *object)
{
	pwr_tStatus	sts;
	int		i;
	char		attr_str[32];
	pwr_sAttrRef	attr_ref;
	char		classname[120];
	pwr_sObjBodyDef	*bodyp;
	pwr_tObjid	rtbody_objid;
	pwr_sClass_DispLink  *link;

	/* Get size of data objects */
	for (;;)
	{
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(object->DataClass), 
		classname, sizeof(classname), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }
	  strcat( classname, "-RtBody");
	  sts = gdh_NameToObjid ( classname, &rtbody_objid);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }

	  sts = gdh_ObjidToPointer( rtbody_objid, (void *)&bodyp);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }

	  object->DataSize = bodyp->Size;	
	  break;
	}

	/* Get offset for the attributes */
	for ( i = 0; i < 5; i++)
	{
	  if ( object->FloatAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->FloatAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->FloatAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->FloatAttrOffs[i] = -1;
              errh_CErrLog(NMPS__DISPFATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else	    
	    object->FloatAttrOffs[i] = -1;
	}
	for ( i = 0; i < 5; i++)
	{
	  if ( object->BooleanAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->BooleanAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->BooleanAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->BooleanAttrOffs[i] = -1;
	      errh_CErrLog(NMPS__DISPBATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else	    
	    object->BooleanAttrOffs[i] = -1;
	}
	for ( i = 0; i < 5; i++)
	{
	  if ( object->IntAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->IntAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->IntAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->IntAttrOffs[i] = -1;
	      errh_CErrLog(NMPS__DISPIATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else
	    object->IntAttrOffs[i] = -1;
	}

	link = (pwr_sClass_DispLink *) object->LinkP;
	if ( object->LinkP == &object->Link)
	  link = 0;

	if ( link && link->MaxDispNumber < object->Number)
	  link->MaxDispNumber = object->Number;

}

void CellDisp_exec( 
  plc_sThread		*tp,
  pwr_sClass_CellDisp  *object)
{
	pwr_sClass_NMpsCell  *cell;
	pwr_sClass_DispLink  *link;
	nmps_sCellInput	*cellp;
	plc_t_DataInfo	*data_info;
	int	i, j;
	char	*datap;
	int	select_exist;
	int	num;
	int	full;
	int	max_size;
	char	*dataptr[NMPS_DISP_SIZE];

	max_size = min( object->MaxSize, NMPS_DISP_SIZE);
	link = (pwr_sClass_DispLink *) object->LinkP;
	if ( object->LinkP == &object->Link)
	  link = 0;

	num = 0;
	full = 0;
	cellp = (nmps_sCellInput *) &object->Cell1P;
	if ( object->Function == NMPS_DISPFUNC_REVERSE)
	  cellp += NMPS_DISP_CELLNUM - 1;

        /* Check that the cell is initialized */
	for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	{
	  if ( cellp->CellP != &cellp->Cell)
	  {

	    cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);

	    if ( !(cell->ReloadDone & NMPS_CELL_INITIALIZED))
	      return;
	    if ( object->Function != NMPS_DISPFUNC_REVERSE)
	      cellp++;
	    else
	      cellp--;

          }
        }

	cellp = (nmps_sCellInput *) &object->Cell1P;
	if ( object->Function == NMPS_DISPFUNC_REVERSE)
	  cellp += NMPS_DISP_CELLNUM - 1;

	for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	{
	  if ( cellp->CellP != &cellp->Cell)
	  {

	    cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);

	    data_info = (plc_t_DataInfo *) &cell->Data1P;
	    if ( object->Function == NMPS_DISPFUNC_REVERSE)
	      data_info += cell->LastIndex - 1;
	    for ( i = num; i < num + cell->LastIndex; i++)
	    {
	      datap = (char *) data_info->DataP;
	      dataptr[i] = datap;
	      if ( datap)
	      {
	        object->Objid[i] = data_info->Data_ObjId;
	        if ( object->FloatAttrOffs[0] >= 0)
	          object->F1[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[0]);
	        if ( object->FloatAttrOffs[1] >= 0)
	          object->F2[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[1]);
	        if ( object->FloatAttrOffs[2] >= 0)
	          object->F3[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[2]);
	        if ( object->FloatAttrOffs[3] >= 0)
	          object->F4[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[3]);
	        if ( object->FloatAttrOffs[4] >= 0)
	          object->F5[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[4]);
	        if ( object->BooleanAttrOffs[0] >= 0)
	          object->B1[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[0]);
	        if ( object->BooleanAttrOffs[1] >= 0)
	          object->B2[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[1]);
	        if ( object->BooleanAttrOffs[2] >= 0)
	          object->B3[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[2]);
	        if ( object->BooleanAttrOffs[3] >= 0)
	          object->B4[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[3]);
	        if ( object->BooleanAttrOffs[4] >= 0)
	          object->B5[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[4]);
	        if ( object->IntAttrOffs[0] >= 0)
	          object->I1[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[0]);
	        if ( object->IntAttrOffs[1] >= 0)
	          object->I2[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[1]);
	        if ( object->IntAttrOffs[2] >= 0)
	          object->I3[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[2]);
	        if ( object->IntAttrOffs[3] >= 0)
	          object->I4[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[3]);
	        if ( object->IntAttrOffs[4] >= 0)
	          object->I5[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[4]);
	      }
	      if ( i == max_size - 1)
	      {
	        full = 1;
	        num = i + 1;
	        break;
	      }
	      if ( object->Function != NMPS_DISPFUNC_REVERSE)
	        data_info++;
	      else
	        data_info--;
	    }
	    if ( full)
	      break;
	    num += cell->LastIndex;
	  }
	  if ( object->Function != NMPS_DISPFUNC_REVERSE)
	    cellp++;
	  else
	    cellp--;
	}

	if ( object->OldLastIndex > num)
	{
	  /* Reset values */
	  memset( &object->Objid[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tObjid));
	  memset( &object->Select[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  memset( &object->OldSelect[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->FloatAttrOffs[0] >= 0)
	    memset( &object->F1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->FloatAttrOffs[1] >= 0)
	    memset( &object->F2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->FloatAttrOffs[2] >= 0)
	    memset( &object->F3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->FloatAttrOffs[3] >= 0)
	    memset( &object->F4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->FloatAttrOffs[4] >= 0)
	    memset( &object->F5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->BooleanAttrOffs[0] >= 0)
	    memset( &object->B1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->BooleanAttrOffs[1] >= 0)
	    memset( &object->B2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->BooleanAttrOffs[2] >= 0)
	    memset( &object->B3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->BooleanAttrOffs[3] >= 0)
	    memset( &object->B4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->BooleanAttrOffs[4] >= 0)
	    memset( &object->B5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->IntAttrOffs[0] >= 0)
	    memset( &object->I1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	  if ( object->IntAttrOffs[1] >= 0)
	    memset( &object->I2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	  if ( object->IntAttrOffs[2] >= 0)
	    memset( &object->I3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	  if ( object->IntAttrOffs[3] >= 0)
	    memset( &object->I4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	  if ( object->IntAttrOffs[4] >= 0)
	    memset( &object->I5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	}

	memset( &object->Select[object->OldLastIndex], 0, (max_size - object->OldLastIndex) * sizeof(pwr_tBoolean));

	if ( link && link->ResetData)
	  if ( object->DisplayObjectP != &object->DisplayObject)
	  {
	    memset( *object->DisplayObjectP, 0, object->DataSize);
	    link->ResetData = 0;
	  }

	/* Detect new selection */
	select_exist = 0;
	for ( i = 0; i < num; i++)
	{
	  if ( object->Select[i] && !object->OldSelect[i])
	  {
	    /* New selection */
	    if ( link)
	    {
	      link->SelectObjid = object->Objid[i];
	      link->SelectExist = 1;
	    }
	    else
	      object->SelectObjid = object->Objid[i];
	    object->OldSelect[i] = object->Select[i];
	  }
	  else if ( !object->Select[i] && object->OldSelect[i])
	  {
	    /* New deselection */
	    if ( link)
	    {
	      if ( cdh_ObjidIsEqual( object->Objid[i], link->SelectObjid))
	        link->SelectObjid = pwr_cNObjid;
	    }
	    else
	    {
	      if ( cdh_ObjidIsEqual( object->Objid[i], object->SelectObjid))
	        object->SelectObjid = pwr_cNObjid;
	    }
	    object->OldSelect[i] = object->Select[i];
	  }

	  if ( link)
	  {
	    if ( cdh_ObjidIsEqual( object->Objid[i], link->SelectObjid))
	    {
	      object->Select[i] = 1;
	      link->SelectExist = 1;
	      if ( object->DisplayObjectP != &object->DisplayObject)
	      {
	        if ( dataptr[i])
	          memcpy( *object->DisplayObjectP, dataptr[i], object->DataSize);
	      }
	    }
	    else
	      object->Select[i] = 0;
	  }
	  else
	  {
	    if ( cdh_ObjidIsEqual( object->Objid[i], object->SelectObjid))
	    {
	      select_exist = 1;
	      object->Select[i] = 1;
	    }
	    else
	      object->Select[i] = 0;
	  }
	}
	if ( !link && !select_exist)
	  object->SelectObjid = pwr_cNObjid;

	if ( link && link->DoRemove)
	{
	  cellp = (nmps_sCellInput *) &object->Cell1P;
	  for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	  {
	    if ( cellp->CellP != &cellp->Cell)
	    {
	      cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);
	      if ( !cell->ExternFlag)
	      {
	        cell->ExternObjId = link->SelectObjid;
	        cell->ExternOpType = NMPS_OPTYPE_EXTDELETE_OBJID;
	        cell->ExternFlag = 1;
	      }
	    }
	    cellp++;
	  }
	}
	else if ( link && link->DoMoveForward)
	{
	  cellp = (nmps_sCellInput *) &object->Cell1P;
	  for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	  {
	    if ( cellp->CellP != &cellp->Cell)
	    {
	      cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);
	      if ( !cell->ExternFlag)
	      {
	        cell->ExternObjId = link->SelectObjid;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	          cell->ExternOpType = NMPS_OPTYPE_EXTMOVEFORW_OBJID;
	        else
	          cell->ExternOpType = NMPS_OPTYPE_EXTMOVEBACKW_OBJID;
	        cell->ExternFlag = 1;
	      }
	    }
	    cellp++;
	  }
	}
	else if ( link && link->DoMoveBackward)
	{
	  cellp = (nmps_sCellInput *) &object->Cell1P;
	  for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	  {
	    if ( cellp->CellP != &cellp->Cell)
	    {
	      cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);
	      if ( !cell->ExternFlag)
	      {
	        cell->ExternObjId = link->SelectObjid;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	          cell->ExternOpType = NMPS_OPTYPE_EXTMOVEBACKW_OBJID;
	        else
	          cell->ExternOpType = NMPS_OPTYPE_EXTMOVEFORW_OBJID;
	        cell->ExternFlag = 1;
	      }
	    }
	    cellp++;
	  }
	}

	if ( link && link->SelectOrder && link->SelectOrderOwner == object->Number)
	  /* Noone responded, selection remains */
	  link->SelectOrder = 0;

	if ( link && link->DoSelectNext)
	{
	  for ( i = 0; i < num; i++)
	  {
	    if ( object->Select[i])
	    {
	      if ( ((i == num - 1 && 
	           object->SelDirection == NMPS_DISP_DIRECT_FORW) ||
		   (i == 0 &&
	           object->SelDirection != NMPS_DISP_DIRECT_FORW)) &&
	           !(object->Number == 1 && link->MaxDispNumber == 1))
	      {
	        /* Transfer selection to next cell */
	        if ( object->Number != link->MaxDispNumber)
	          link->SelectOrderNumber = object->Number + 1;
	        else
	          link->SelectOrderNumber = 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Move selection in the cell */
	        object->Select[i] = 0;
	        object->OldSelect[i] = 0;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	        {
	          if ( i != num - 1)
	          {
	            object->Select[i+1] = 1;
	            object->OldSelect[i+1] = 1;
	            link->SelectObjid = object->Objid[i+1];
	          }
	          else
	          {
	            object->Select[0] = 1;
	            object->OldSelect[0] = 1;
	            link->SelectObjid = object->Objid[0];
	          }
	        }
	        else
	        {
	          if ( i != 0)
	          {
	            object->Select[i-1] = 1;
	            object->OldSelect[i-1] = 1;
	            link->SelectObjid = object->Objid[i-1];
	          }
	          else
	          {
	            object->Select[num-1] = 1;
	            object->OldSelect[num-1] = 1;
	            link->SelectObjid = object->Objid[num-1];
	          }
	        }
	      }
	      break;
	    }
	  }
	}

	if ( link && link->DoSelectPrevious)
	{
	  for ( i = 0; i < num; i++)
	  {
	    if ( object->Select[i])
	    {
	      if ( ((i == num - 1 && 
	           object->SelDirection != NMPS_DISP_DIRECT_FORW) ||
		   (i == 0 &&
	           object->SelDirection == NMPS_DISP_DIRECT_FORW)) &&
	           !(object->Number == 1 && link->MaxDispNumber == 1))
	      {
	        /* Transfer selection to next cell */
	        if ( object->Number != 1)
	          link->SelectOrderNumber = object->Number - 1;
	        else
	          link->SelectOrderNumber = link->MaxDispNumber;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Move selection in the cell */
	        object->Select[i] = 0;
	        object->OldSelect[i] = 0;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	        {
	          if ( i != 0)
	          {
	            object->Select[i-1] = 1;
	            object->OldSelect[i-1] = 1;
	            link->SelectObjid = object->Objid[i-1];
	          }
	          else
	          {
	            object->Select[num-1] = 1;
	            object->OldSelect[num-1] = 1;
	            link->SelectObjid = object->Objid[num-1];
	          }
	        }
	        else
	        {
	          if ( i != num - 1)
	          {
	            object->Select[i+1] = 1;
	            object->OldSelect[i+1] = 1;
	            link->SelectObjid = object->Objid[i+1];
	          }
	          else
	          {
	            object->Select[0] = 1;
	            object->OldSelect[0] = 1;
	            link->SelectObjid = object->Objid[0];
	          }
	        }
	      }
	      break;
	    }
	  }
	}

	if ( link && link->SelectOrder && link->SelectOrderNumber == object->Number)
	{
	  if ( link->SelectOrderType == NMPS_DISP_ORDERTYPE_NEXT)
	  {
	    if ( num == 0 )
	    {
	      if ( object->Number != link->MaxDispNumber)
	      {
	        /* Transfer to next */
	        link->SelectOrderNumber = object->Number + 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Transfer to next */
	        link->SelectOrderNumber = 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
            }
	    else
	    {
	      if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	      {
	        /* Select first */
	        object->Select[0] = 1;
	        object->OldSelect[0] = 1;
	        link->SelectObjid = object->Objid[0];
	      }
	      else
	      {
	        /* Select last */
	        object->Select[num-1] = 1;
	        object->OldSelect[num-1] = 1;
	        link->SelectObjid = object->Objid[num-1];
	      }
	      link->SelectOrder = 0;
	    }
	  }

	  if ( link->SelectOrderType == NMPS_DISP_ORDERTYPE_PREV)
	  {
	    if ( num == 0)
	    {
	      if ( object->Number != 1)
	      {
	        /* Transfer to previous */
	        link->SelectOrderNumber = object->Number - 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Transfer to last */
	        link->SelectOrderNumber = link->MaxDispNumber;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
            }
	    else
	    {
	      if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	      {
	        /* Select last */
	        object->Select[num - 1] = 1;
	        object->OldSelect[num - 1] = 1;
	        link->SelectObjid = object->Objid[num-1];
	      }
	      else
	      {
	        /* Select first */
	        object->Select[0] = 1;
	        object->OldSelect[0] = 1;
	        link->SelectObjid = object->Objid[0];
	      }
	      link->SelectOrder = 0;
	    }
	  }
	}
	object->OldLastIndex = num;
}

/**
  @aref celldispmir CellDispMir
*/
void CellDispMir_init( pwr_sClass_CellDispMir  *object)
{
	pwr_tStatus	sts;
	int		i;
	char		attr_str[32];
	pwr_sAttrRef	attr_ref;
	char		classname[120];
	pwr_sObjBodyDef	*bodyp;
	pwr_tObjid	rtbody_objid;
	pwr_sClass_DispLink  *link;

	/* Get size of data objects */
	for (;;)
	{
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(object->DataClass), 
		classname, sizeof(classname), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }
	  strcat( classname, "-RtBody");
	  sts = gdh_NameToObjid ( classname, &rtbody_objid);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }

	  sts = gdh_ObjidToPointer( rtbody_objid, (void *)&bodyp);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }

	  object->DataSize = bodyp->Size;	
	  break;
	}

	/* Get offset for the attributes */
	for ( i = 0; i < 5; i++)
	{
	  if ( object->FloatAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->FloatAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->FloatAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->FloatAttrOffs[i] = -1;
              errh_CErrLog(NMPS__DISPFATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else	    
	    object->FloatAttrOffs[i] = -1;
	}
	for ( i = 0; i < 5; i++)
	{
	  if ( object->BooleanAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->BooleanAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->BooleanAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->BooleanAttrOffs[i] = -1;
	      errh_CErrLog(NMPS__DISPBATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else	    
	    object->BooleanAttrOffs[i] = -1;
	}
	for ( i = 0; i < 5; i++)
	{
	  if ( object->IntAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->IntAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->IntAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->IntAttrOffs[i] = -1;
	      errh_CErrLog(NMPS__DISPIATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else
	    object->IntAttrOffs[i] = -1;
	}

	link = (pwr_sClass_DispLink *) object->LinkP;
	if ( object->LinkP == &object->Link)
	  link = 0;

	if ( link && link->MaxDispNumber < object->Number)
	  link->MaxDispNumber = object->Number;

}

void CellDispMir_exec( 
  plc_sThread		*tp,
  pwr_sClass_CellDispMir  *object)
{
	pwr_sClass_NMpsMirrorCell  *cell;
	pwr_sClass_DispLink  *link;
	nmps_sCellInput	*cellp;
	plc_t_DataInfoMirCell	*data_info;
	int	i, j;
	char	*datap;
	int	select_exist;
	int	num;
	int	full;
	int	max_size;
	char	*dataptr[NMPS_DISP_SIZE];

	max_size = min( object->MaxSize, NMPS_DISP_SIZE);
	link = (pwr_sClass_DispLink *) object->LinkP;
	if ( object->LinkP == &object->Link)
	  link = 0;

	num = 0;
	full = 0;
	cellp = (nmps_sCellInput *) &object->Cell1P;
	if ( object->Function == NMPS_DISPFUNC_REVERSE)
	  cellp += NMPS_DISP_CELLNUM - 1;

        /* Check that the cell is initialized */
	for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	{
	  if ( cellp->CellP != &cellp->Cell)
	  {

	    cell = (pwr_sClass_NMpsMirrorCell *) *(cellp->CellP);

	    if ( !(cell->ReloadDone & NMPS_CELL_INITIALIZED))
	      return;
	    if ( object->Function != NMPS_DISPFUNC_REVERSE)
	      cellp++;
	    else
	      cellp--;

          }
        }

	cellp = (nmps_sCellInput *) &object->Cell1P;
	if ( object->Function == NMPS_DISPFUNC_REVERSE)
	  cellp += NMPS_DISP_CELLNUM - 1;

	for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	{
	  if ( cellp->CellP != &cellp->Cell)
	  {

	    cell = (pwr_sClass_NMpsMirrorCell *) *(cellp->CellP);

	    data_info = (plc_t_DataInfoMirCell *) &cell->Data1P;
	    if ( object->Function == NMPS_DISPFUNC_REVERSE)
	      data_info += cell->LastIndex - 1;
	    for ( i = num; i < num + cell->LastIndex; i++)
	    {
	      datap = (char *) data_info->DataP;
	      dataptr[i] = datap;
	      if ( datap)
	      {
	        object->Objid[i] = data_info->Data_ObjId;
	        if ( object->FloatAttrOffs[0] >= 0)
	          object->F1[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[0]);
	        if ( object->FloatAttrOffs[1] >= 0)
	          object->F2[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[1]);
	        if ( object->FloatAttrOffs[2] >= 0)
	          object->F3[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[2]);
	        if ( object->FloatAttrOffs[3] >= 0)
	          object->F4[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[3]);
	        if ( object->FloatAttrOffs[4] >= 0)
	          object->F5[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[4]);
	        if ( object->BooleanAttrOffs[0] >= 0)
	          object->B1[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[0]);
	        if ( object->BooleanAttrOffs[1] >= 0)
	          object->B2[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[1]);
	        if ( object->BooleanAttrOffs[2] >= 0)
	          object->B3[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[2]);
	        if ( object->BooleanAttrOffs[3] >= 0)
	          object->B4[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[3]);
	        if ( object->BooleanAttrOffs[4] >= 0)
	          object->B5[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[4]);
	        if ( object->IntAttrOffs[0] >= 0)
	          object->I1[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[0]);
	        if ( object->IntAttrOffs[1] >= 0)
	          object->I2[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[1]);
	        if ( object->IntAttrOffs[2] >= 0)
	          object->I3[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[2]);
	        if ( object->IntAttrOffs[3] >= 0)
	          object->I4[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[3]);
	        if ( object->IntAttrOffs[4] >= 0)
	          object->I5[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[4]);
	      }
	      if ( i == max_size - 1)
	      {
	        full = 1;
	        num = i + 1;
	        break;
	      }
	      if ( object->Function != NMPS_DISPFUNC_REVERSE)
	        data_info++;
	      else
	        data_info--;
	    }
	    if ( full)
	      break;
	    num += cell->LastIndex;
	  }
	  if ( object->Function != NMPS_DISPFUNC_REVERSE)
	    cellp++;
	  else
	    cellp--;
	}

	if ( object->OldLastIndex > num)
	{
	  /* Reset values */
	  memset( &object->Objid[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tObjid));
	  memset( &object->Select[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  memset( &object->OldSelect[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->FloatAttrOffs[0] >= 0)
	    memset( &object->F1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->FloatAttrOffs[1] >= 0)
	    memset( &object->F2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->FloatAttrOffs[2] >= 0)
	    memset( &object->F3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->FloatAttrOffs[3] >= 0)
	    memset( &object->F4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->FloatAttrOffs[4] >= 0)
	    memset( &object->F5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  if ( object->BooleanAttrOffs[0] >= 0)
	    memset( &object->B1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->BooleanAttrOffs[1] >= 0)
	    memset( &object->B2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->BooleanAttrOffs[2] >= 0)
	    memset( &object->B3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->BooleanAttrOffs[3] >= 0)
	    memset( &object->B4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->BooleanAttrOffs[4] >= 0)
	    memset( &object->B5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->IntAttrOffs[0] >= 0)
	    memset( &object->I1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	  if ( object->IntAttrOffs[1] >= 0)
	    memset( &object->I2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	  if ( object->IntAttrOffs[2] >= 0)
	    memset( &object->I3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	  if ( object->IntAttrOffs[3] >= 0)
	    memset( &object->I4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	  if ( object->IntAttrOffs[4] >= 0)
	    memset( &object->I5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	}

	memset( &object->Select[object->OldLastIndex], 0, (max_size - object->OldLastIndex) * sizeof(pwr_tBoolean));

	if ( link && link->ResetData)
	  if ( object->DisplayObjectP != &object->DisplayObject)
	  {
	    memset( *object->DisplayObjectP, 0, object->DataSize);
	    link->ResetData = 0;
	  }

	/* Detect new selection */
	select_exist = 0;
	for ( i = 0; i < num; i++)
	{
	  if ( object->Select[i] && !object->OldSelect[i])
	  {
	    /* New selection */
	    if ( link)
	    {
	      link->SelectObjid = object->Objid[i];
	      link->SelectExist = 1;
	    }
	    else
	      object->SelectObjid = object->Objid[i];
	    object->OldSelect[i] = object->Select[i];
	  }
	  else if ( !object->Select[i] && object->OldSelect[i])
	  {
	    /* New deselection */
	    if ( link)
	    {
	      if ( cdh_ObjidIsEqual( object->Objid[i], link->SelectObjid))
	        link->SelectObjid = pwr_cNObjid;
	    }
	    else
	    {
	      if ( cdh_ObjidIsEqual( object->Objid[i], object->SelectObjid))
	        object->SelectObjid = pwr_cNObjid;
	    }
	    object->OldSelect[i] = object->Select[i];
	  }

	  if ( link)
	  {
	    if ( cdh_ObjidIsEqual( object->Objid[i], link->SelectObjid))
	    {
	      object->Select[i] = 1;
	      link->SelectExist = 1;
	      if ( object->DisplayObjectP != &object->DisplayObject)
	      {
	        if ( dataptr[i])
	          memcpy( *object->DisplayObjectP, dataptr[i], object->DataSize);
	      }
	    }
	    else
	      object->Select[i] = 0;
	  }
	  else
	  {
	    if ( cdh_ObjidIsEqual( object->Objid[i], object->SelectObjid))
	    {
	      select_exist = 1;
	      object->Select[i] = 1;
	    }
	    else
	      object->Select[i] = 0;
	  }
	}
	if ( !link && !select_exist)
	  object->SelectObjid = pwr_cNObjid;

	if ( link && link->SelectOrder && link->SelectOrderOwner == object->Number)
	  /* Noone responded, selection remains */
	  link->SelectOrder = 0;

	if ( link && link->DoSelectNext)
	{
	  for ( i = 0; i < num; i++)
	  {
	    if ( object->Select[i])
	    {
	      if ( ((i == num - 1 && 
	           object->SelDirection == NMPS_DISP_DIRECT_FORW) ||
		   (i == 0 &&
	           object->SelDirection != NMPS_DISP_DIRECT_FORW)) &&
	           !(object->Number == 1 && link->MaxDispNumber == 1))
	      {
	        /* Transfer selection to next cell */
	        if ( object->Number != link->MaxDispNumber)
	          link->SelectOrderNumber = object->Number + 1;
	        else
	          link->SelectOrderNumber = 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Move selection in the cell */
	        object->Select[i] = 0;
	        object->OldSelect[i] = 0;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	        {
	          if ( i != num - 1)
	          {
	            object->Select[i+1] = 1;
	            object->OldSelect[i+1] = 1;
	            link->SelectObjid = object->Objid[i+1];
	          }
	          else
	          {
	            object->Select[0] = 1;
	            object->OldSelect[0] = 1;
	            link->SelectObjid = object->Objid[0];
	          }
	        }
	        else
	        {
	          if ( i != 0)
	          {
	            object->Select[i-1] = 1;
	            object->OldSelect[i-1] = 1;
	            link->SelectObjid = object->Objid[i-1];
	          }
	          else
	          {
	            object->Select[num-1] = 1;
	            object->OldSelect[num-1] = 1;
	            link->SelectObjid = object->Objid[num-1];
	          }
	        }
	      }
	      break;
	    }
	  }
	}

	if ( link && link->DoSelectPrevious)
	{
	  for ( i = 0; i < num; i++)
	  {
	    if ( object->Select[i])
	    {
	      if ( ((i == num - 1 && 
	           object->SelDirection != NMPS_DISP_DIRECT_FORW) ||
		   (i == 0 &&
	           object->SelDirection == NMPS_DISP_DIRECT_FORW)) &&
	           !(object->Number == 1 && link->MaxDispNumber == 1))
	      {
	        /* Transfer selection to next cell */
	        if ( object->Number != 1)
	          link->SelectOrderNumber = object->Number - 1;
	        else
	          link->SelectOrderNumber = link->MaxDispNumber;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Move selection in the cell */
	        object->Select[i] = 0;
	        object->OldSelect[i] = 0;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	        {
	          if ( i != 0)
	          {
	            object->Select[i-1] = 1;
	            object->OldSelect[i-1] = 1;
	            link->SelectObjid = object->Objid[i-1];
	          }
	          else
	          {
	            object->Select[num-1] = 1;
	            object->OldSelect[num-1] = 1;
	            link->SelectObjid = object->Objid[num-1];
	          }
	        }
	        else
	        {
	          if ( i != num - 1)
	          {
	            object->Select[i+1] = 1;
	            object->OldSelect[i+1] = 1;
	            link->SelectObjid = object->Objid[i+1];
	          }
	          else
	          {
	            object->Select[0] = 1;
	            object->OldSelect[0] = 1;
	            link->SelectObjid = object->Objid[0];
	          }
	        }
	      }
	      break;
	    }
	  }
	}

	if ( link && link->SelectOrder && link->SelectOrderNumber == object->Number)
	{
	  if ( link->SelectOrderType == NMPS_DISP_ORDERTYPE_NEXT)
	  {
	    if ( num == 0 )
	    {
	      if ( object->Number != link->MaxDispNumber)
	      {
	        /* Transfer to next */
	        link->SelectOrderNumber = object->Number + 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Transfer to next */
	        link->SelectOrderNumber = 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
            }
	    else
	    {
	      if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	      {
	        /* Select first */
	        object->Select[0] = 1;
	        object->OldSelect[0] = 1;
	        link->SelectObjid = object->Objid[0];
	      }
	      else
	      {
	        /* Select last */
	        object->Select[num-1] = 1;
	        object->OldSelect[num-1] = 1;
	        link->SelectObjid = object->Objid[num-1];
	      }
	      link->SelectOrder = 0;
	    }
	  }

	  if ( link->SelectOrderType == NMPS_DISP_ORDERTYPE_PREV)
	  {
	    if ( num == 0)
	    {
	      if ( object->Number != 1)
	      {
	        /* Transfer to previous */
	        link->SelectOrderNumber = object->Number - 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Transfer to last */
	        link->SelectOrderNumber = link->MaxDispNumber;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
            }
	    else
	    {
	      if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	      {
	        /* Select last */
	        object->Select[num - 1] = 1;
	        object->OldSelect[num - 1] = 1;
	        link->SelectObjid = object->Objid[num-1];
	      }
	      else
	      {
	        /* Select first */
	        object->Select[0] = 1;
	        object->OldSelect[0] = 1;
	        link->SelectObjid = object->Objid[0];
	      }
	      link->SelectOrder = 0;
	    }
	  }
	}
	object->OldLastIndex = num;
}


/**
  @aref cellupdate CellUpdate
*/
void CellUpdate_init( pwr_sClass_CellUpdate  *object)
{
	pwr_tStatus	sts;
	int		i;
	char		attr_str[32];
	pwr_sAttrRef	attr_ref;
	char		classname[120];
	pwr_sObjBodyDef	*bodyp;
	pwr_tObjid	rtbody_objid;
	pwr_sClass_DispLink  *link;

	/* Get size of data objects */
	for (;;)
	{
	  sts = gdh_ObjidToName ( cdh_ClassIdToObjid(object->DataClass), 
		classname, sizeof(classname), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }
	  strcat( classname, "-RtBody");
	  sts = gdh_NameToObjid ( classname, &rtbody_objid);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }

	  sts = gdh_ObjidToPointer( rtbody_objid, (void *)&bodyp);
	  if ( EVEN(sts)) 
	  {
	    errh_CErrLog(NMPS__DISPCLASS, errh_ErrArgMsg(sts), NULL);
	    break;
	  }

	  object->DataSize = bodyp->Size;	
	  break;
	}

	/* Get offset for the attributes */
	for ( i = 0; i < 5; i++)
	{
	  if ( object->FloatAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->FloatAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->FloatAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->FloatAttrOffs[i] = -1;
	      errh_CErrLog(NMPS__DISPFATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else	    
	    object->FloatAttrOffs[i] = -1;
	}
	for ( i = 0; i < 5; i++)
	{
	  if ( object->BooleanAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->BooleanAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->BooleanAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->BooleanAttrOffs[i] = -1;
	      errh_CErrLog(NMPS__DISPBATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else	    
	    object->BooleanAttrOffs[i] = -1;
	}
	for ( i = 0; i < 5; i++)
	{
	  if ( object->IntAttr[i][0] != 0)
	  {
	    strcpy( attr_str, ".");
	    strcat( attr_str, object->IntAttr[i]);
	    sts = gdh_ClassAttrToAttrref( object->DataClass,
		attr_str, &attr_ref);			
	    if ( ODD(sts))
	      object->IntAttrOffs[i] = attr_ref.Offset;
	    else
	    {
	      object->IntAttrOffs[i] = -1;
	      errh_CErrLog(NMPS__DISPIATTR, errh_ErrArgMsg(sts), NULL);
	    }
	  }
	  else
	    object->IntAttrOffs[i] = -1;
	}

	link = (pwr_sClass_DispLink *) object->LinkP;
	if ( object->LinkP == &object->Link)
	  link = 0;

	if ( link && link->MaxDispNumber < object->Number)
	  link->MaxDispNumber = object->Number;

}

void CellUpdate_exec( 
  plc_sThread		*tp,
  pwr_sClass_CellUpdate  *object)
{
	pwr_sClass_NMpsCell  *cell;
	pwr_sClass_DispLink  *link;
	nmps_sCellInput	*cellp;
	plc_t_DataInfo	*data_info;
	int	i, j;
	char	*datap;
	int	select_exist;
	int	num;
	int	full;
	int	max_size;
	char	*dataptr[NMPS_DISP_SIZE];

	max_size = min( object->MaxSize, NMPS_DISP_SIZE);
	link = (pwr_sClass_DispLink *) object->LinkP;
	if ( object->LinkP == &object->Link)
	  link = 0;

	num = 0;
	full = 0;
	cellp = (nmps_sCellInput *) &object->Cell1P;
	if ( object->Function == NMPS_DISPFUNC_REVERSE)
	  cellp += NMPS_DISP_CELLNUM - 1;

        /* Check that the cell is initialized */
	for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	{
	  if ( cellp->CellP != &cellp->Cell)
	  {

	    cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);

	    if ( !(cell->ReloadDone & NMPS_CELL_INITIALIZED))
	      return;
	    if ( object->Function != NMPS_DISPFUNC_REVERSE)
	      cellp++;
	    else
	      cellp--;

          }
        }

	cellp = (nmps_sCellInput *) &object->Cell1P;
	if ( object->Function == NMPS_DISPFUNC_REVERSE)
	  cellp += NMPS_DISP_CELLNUM - 1;

	for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	{
	  if ( cellp->CellP != &cellp->Cell)
	  {

	    cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);

	    data_info = (plc_t_DataInfo *) &cell->Data1P;
	    if ( object->Function == NMPS_DISPFUNC_REVERSE)
	      data_info += cell->LastIndex - 1;
	    for ( i = num; i < num + cell->LastIndex; i++)
	    {
	      datap = (char *) data_info->DataP;
	      dataptr[i] = datap;
	      if ( datap)
	      {
	        object->Objid[i] = data_info->Data_ObjId;
	        if ( object->FloatAttrOffs[0] >= 0)
                {
	          if ( object->F1[i] != object->F1Old[i])
	            *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[0]) = object->F1[i];
                  else
	            object->F1[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[0]);
	          object->F1Old[i] = object->F1[i];
                }
	        if ( object->FloatAttrOffs[1] >= 0)
	        {
	          if ( object->F2[i] != object->F2Old[i])
	            *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[1]) = object->F2[i];
	          else
	            object->F2[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[1]);
	          object->F2Old[i] = object->F2[i];
	        }
	        if ( object->FloatAttrOffs[2] >= 0)
	        {
	          if ( object->F3[i] != object->F3Old[i])
	            *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[2]) = object->F3[i];
	          else
	            object->F3[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[2]);
	          object->F3Old[i] = object->F3[i];
	        }
	        if ( object->FloatAttrOffs[3] >= 0)
	        {
	          if ( object->F4[i] != object->F4Old[i])
	            *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[3]) = object->F4[i];
	          else
	            object->F4[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[3]);
	          object->F4Old[i] = object->F4[i];
	        }
	        if ( object->FloatAttrOffs[4] >= 0)
	        {
	          if ( object->F5[i] != object->F5Old[i])
	            *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[4]) = object->F5[i];
	          else
	            object->F5[i] = *(pwr_tFloat32 *)(datap + object->FloatAttrOffs[4]);
	          object->F5Old[i] = object->F5[i];
	        }
	        if ( object->BooleanAttrOffs[0] >= 0)
	        {
	          if ( object->B1[i] != object->B1Old[i])
	            *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[0]) = object->B1[i];
	          else
	            object->B1[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[0]);
	          object->B1Old[i] = object->B1[i];
	        }
	        if ( object->BooleanAttrOffs[1] >= 0)
	        {
	          if ( object->B2[i] != object->B2Old[i])
	            *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[1]) = object->B2[i];
	          else
	            object->B2[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[1]);
	          object->B2Old[i] = object->B2[i];
	        }
	        if ( object->BooleanAttrOffs[2] >= 0)
	        {
	          if ( object->B3[i] != object->B3Old[i])
	            *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[2]) = object->B3[i];
	          else
	            object->B3[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[2]);
	          object->B3Old[i] = object->B3[i];
	        }
	        if ( object->BooleanAttrOffs[3] >= 0)
	        {
	          if ( object->B4[i] != object->B4Old[i])
	            *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[3]) = object->B4[i];
	          else
	            object->B4[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[3]);
	          object->B4Old[i] = object->B4[i];
	        }
	        if ( object->BooleanAttrOffs[4] >= 0)
	        {
	          if ( object->B5[i] != object->B5Old[i])
	            *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[4]) = object->B5[i];
	          else
	            object->B5[i] = *(pwr_tBoolean *)(datap + object->BooleanAttrOffs[4]);
	          object->B5Old[i] = object->B5[i];
	        }
	        if ( object->IntAttrOffs[0] >= 0)
	        {
	          if ( object->I1[i] != object->I1Old[i])
	            *(pwr_tInt32 *)(datap + object->IntAttrOffs[0]) = object->I1[i];
	          else
	            object->I1[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[0]);
	          object->I1Old[i] = object->I1[i];
	        }
	        if ( object->IntAttrOffs[1] >= 0)
	        {
	          if ( object->I2[i] != object->I2Old[i])
	            *(pwr_tInt32 *)(datap + object->IntAttrOffs[1]) = object->I2[i];
	          else
	            object->I2[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[1]);
	          object->I2Old[i] = object->I2[i];
	        }
	        if ( object->IntAttrOffs[2] >= 0)
	        {
	          if ( object->I3[i] != object->I3Old[i])
	            *(pwr_tInt32 *)(datap + object->IntAttrOffs[2]) = object->I3[i];
	          else
	            object->I3[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[2]);
	          object->I3Old[i] = object->I3[i];
	        }
	        if ( object->IntAttrOffs[3] >= 0)
	        {
	          if ( object->I4[i] != object->I4Old[i])
	            *(pwr_tInt32 *)(datap + object->IntAttrOffs[3]) = object->I4[i];
	          else
	            object->I4[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[3]);
	          object->I4Old[i] = object->I4[i];
	        }
	        if ( object->IntAttrOffs[4] >= 0)
	        {
	          if ( object->I5[i] != object->I5Old[i])
	            *(pwr_tInt32 *)(datap + object->IntAttrOffs[4]) = object->I5[i];
	          else
	            object->I5[i] = *(pwr_tInt32 *)(datap + object->IntAttrOffs[4]);
	          object->I5Old[i] = object->I5[i];
	        }
	      }
	      if ( i == max_size - 1)
	      {
	        full = 1;
	        num = i + 1;
	        break;
	      }
	      if ( object->Function != NMPS_DISPFUNC_REVERSE)
	        data_info++;
	      else
	        data_info--;
	    }
	    if ( full)
	      break;
	    num += cell->LastIndex;
	  }
	  if ( object->Function != NMPS_DISPFUNC_REVERSE)
	    cellp++;
	  else
	    cellp--;
	}

	if ( object->OldLastIndex > num)
	{
	  /* Reset values */
	  memset( &object->Objid[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tObjid));
	  memset( &object->Select[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  memset( &object->OldSelect[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	  if ( object->FloatAttrOffs[0] >= 0) {
	    memset( &object->F1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	    memset( &object->F1Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
          }
	  if ( object->FloatAttrOffs[1] >= 0) {
	    memset( &object->F2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	    memset( &object->F2Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
          }
	  if ( object->FloatAttrOffs[2] >= 0) {
	    memset( &object->F3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	    memset( &object->F3Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
          }
	  if ( object->FloatAttrOffs[3] >= 0) {
	    memset( &object->F4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	    memset( &object->F4Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
          }
	  if ( object->FloatAttrOffs[4] >= 0) {
	    memset( &object->F5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	    memset( &object->F5Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tFloat32));
	  }
	  if ( object->BooleanAttrOffs[0] >= 0) {
	    memset( &object->B1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	    memset( &object->B1Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
          }
	  if ( object->BooleanAttrOffs[1] >= 0) {
	    memset( &object->B2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	    memset( &object->B2Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
          }
	  if ( object->BooleanAttrOffs[2] >= 0) {
	    memset( &object->B3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	    memset( &object->B3Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
          }
	  if ( object->BooleanAttrOffs[3] >= 0) {
	    memset( &object->B4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	    memset( &object->B4Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
          }
	  if ( object->BooleanAttrOffs[4] >= 0) {
	    memset( &object->B5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
	    memset( &object->B5Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tBoolean));
          }
	  if ( object->IntAttrOffs[0] >= 0) {
	    memset( &object->I1[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	    memset( &object->I1Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
          }
	  if ( object->IntAttrOffs[1] >= 0) {
	    memset( &object->I2[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	    memset( &object->I2Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
          }
	  if ( object->IntAttrOffs[2] >= 0) {
	    memset( &object->I3[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	    memset( &object->I3Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
          }
	  if ( object->IntAttrOffs[3] >= 0) {
	    memset( &object->I4[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	    memset( &object->I4Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
          }
	  if ( object->IntAttrOffs[4] >= 0) {
	    memset( &object->I5[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
	    memset( &object->I5Old[num], 0, (object->OldLastIndex - num ) * sizeof(pwr_tInt32));
          }
	}

	memset( &object->Select[object->OldLastIndex], 0, (max_size - object->OldLastIndex) * sizeof(pwr_tBoolean));

	if ( link && link->ResetData)
	  if ( object->DisplayObjectP != &object->DisplayObject)
	  {
	    memset( *object->DisplayObjectP, 0, object->DataSize);
	    link->ResetData = 0;
	  }

	/* Detect new selection */
	select_exist = 0;
	for ( i = 0; i < num; i++)
	{
	  if ( object->Select[i] && !object->OldSelect[i])
	  {
	    /* New selection */
	    if ( link)
	    {
	      link->SelectObjid = object->Objid[i];
	      link->SelectExist = 1;
	    }
	    else
	      object->SelectObjid = object->Objid[i];
	    object->OldSelect[i] = object->Select[i];
	  }
	  else if ( !object->Select[i] && object->OldSelect[i])
	  {
	    /* New deselection */
	    if ( link)
	    {
	      if ( cdh_ObjidIsEqual( object->Objid[i], link->SelectObjid))
	        link->SelectObjid = pwr_cNObjid;
	    }
	    else
	    {
	      if ( cdh_ObjidIsEqual( object->Objid[i], object->SelectObjid))
	        object->SelectObjid = pwr_cNObjid;
	    }
	    object->OldSelect[i] = object->Select[i];
	  }

	  if ( link)
	  {
	    if ( cdh_ObjidIsEqual( object->Objid[i], link->SelectObjid))
	    {
	      object->Select[i] = 1;
	      link->SelectExist = 1;
	      if ( object->DisplayObjectP != &object->DisplayObject)
	      {
	        if ( dataptr[i])
	          memcpy( *object->DisplayObjectP, dataptr[i], object->DataSize);
	      }
	    }
	    else
	      object->Select[i] = 0;
	  }
	  else
	  {
	    if ( cdh_ObjidIsEqual( object->Objid[i], object->SelectObjid))
	    {
	      select_exist = 1;
	      object->Select[i] = 1;
	    }
	    else
	      object->Select[i] = 0;
	  }
	}
	if ( !link && !select_exist)
	  object->SelectObjid = pwr_cNObjid;

	if ( link && link->DoRemove)
	{
	  cellp = (nmps_sCellInput *) &object->Cell1P;
	  for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	  {
	    if ( cellp->CellP != &cellp->Cell)
	    {
	      cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);
	      if ( !cell->ExternFlag)
	      {
	        cell->ExternObjId = link->SelectObjid;
	        cell->ExternOpType = NMPS_OPTYPE_EXTDELETE_OBJID;
	        cell->ExternFlag = 1;
	      }
	    }
	    cellp++;
	  }
	}
	else if ( link && link->DoMoveForward)
	{
	  cellp = (nmps_sCellInput *) &object->Cell1P;
	  for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	  {
	    if ( cellp->CellP != &cellp->Cell)
	    {
	      cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);
	      if ( !cell->ExternFlag)
	      {
	        cell->ExternObjId = link->SelectObjid;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	          cell->ExternOpType = NMPS_OPTYPE_EXTMOVEFORW_OBJID;
	        else
	          cell->ExternOpType = NMPS_OPTYPE_EXTMOVEBACKW_OBJID;
	        cell->ExternFlag = 1;
	      }
	    }
	    cellp++;
	  }
	}
	else if ( link && link->DoMoveBackward)
	{
	  cellp = (nmps_sCellInput *) &object->Cell1P;
	  for ( j = 0; j < NMPS_DISP_CELLNUM; j++)
	  {
	    if ( cellp->CellP != &cellp->Cell)
	    {
	      cell = (pwr_sClass_NMpsCell *) *(cellp->CellP);
	      if ( !cell->ExternFlag)
	      {
	        cell->ExternObjId = link->SelectObjid;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	          cell->ExternOpType = NMPS_OPTYPE_EXTMOVEBACKW_OBJID;
	        else
	          cell->ExternOpType = NMPS_OPTYPE_EXTMOVEFORW_OBJID;
	        cell->ExternFlag = 1;
	      }
	    }
	    cellp++;
	  }
	}

	if ( link && link->SelectOrder && link->SelectOrderOwner == object->Number)
	  /* Noone responded, selection remains */
	  link->SelectOrder = 0;

	if ( link && link->DoSelectNext)
	{
	  for ( i = 0; i < num; i++)
	  {
	    if ( object->Select[i])
	    {
	      if ( ((i == num - 1 && 
	           object->SelDirection == NMPS_DISP_DIRECT_FORW) ||
		   (i == 0 &&
	           object->SelDirection != NMPS_DISP_DIRECT_FORW)) &&
	           !(object->Number == 1 && link->MaxDispNumber == 1))
	      {
	        /* Transfer selection to next cell */
	        if ( object->Number != link->MaxDispNumber)
	          link->SelectOrderNumber = object->Number + 1;
	        else
	          link->SelectOrderNumber = 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Move selection in the cell */
	        object->Select[i] = 0;
	        object->OldSelect[i] = 0;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	        {
	          if ( i != num - 1)
	          {
	            object->Select[i+1] = 1;
	            object->OldSelect[i+1] = 1;
	            link->SelectObjid = object->Objid[i+1];
	          }
	          else
	          {
	            object->Select[0] = 1;
	            object->OldSelect[0] = 1;
	            link->SelectObjid = object->Objid[0];
	          }
	        }
	        else
	        {
	          if ( i != 0)
	          {
	            object->Select[i-1] = 1;
	            object->OldSelect[i-1] = 1;
	            link->SelectObjid = object->Objid[i-1];
	          }
	          else
	          {
	            object->Select[num-1] = 1;
	            object->OldSelect[num-1] = 1;
	            link->SelectObjid = object->Objid[num-1];
	          }
	        }
	      }
	      break;
	    }
	  }
	}

	if ( link && link->DoSelectPrevious)
	{
	  for ( i = 0; i < num; i++)
	  {
	    if ( object->Select[i])
	    {
	      if ( ((i == num - 1 && 
	           object->SelDirection != NMPS_DISP_DIRECT_FORW) ||
		   (i == 0 &&
	           object->SelDirection == NMPS_DISP_DIRECT_FORW)) &&
	           !(object->Number == 1 && link->MaxDispNumber == 1))
	      {
	        /* Transfer selection to next cell */
	        if ( object->Number != 1)
	          link->SelectOrderNumber = object->Number - 1;
	        else
	          link->SelectOrderNumber = link->MaxDispNumber;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Move selection in the cell */
	        object->Select[i] = 0;
	        object->OldSelect[i] = 0;
	        if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	        {
	          if ( i != 0)
	          {
	            object->Select[i-1] = 1;
	            object->OldSelect[i-1] = 1;
	            link->SelectObjid = object->Objid[i-1];
	          }
	          else
	          {
	            object->Select[num-1] = 1;
	            object->OldSelect[num-1] = 1;
	            link->SelectObjid = object->Objid[num-1];
	          }
	        }
	        else
	        {
	          if ( i != num - 1)
	          {
	            object->Select[i+1] = 1;
	            object->OldSelect[i+1] = 1;
	            link->SelectObjid = object->Objid[i+1];
	          }
	          else
	          {
	            object->Select[0] = 1;
	            object->OldSelect[0] = 1;
	            link->SelectObjid = object->Objid[0];
	          }
	        }
	      }
	      break;
	    }
	  }
	}

	if ( link && link->SelectOrder && link->SelectOrderNumber == object->Number)
	{
	  if ( link->SelectOrderType == NMPS_DISP_ORDERTYPE_NEXT)
	  {
	    if ( num == 0 )
	    {
	      if ( object->Number != link->MaxDispNumber)
	      {
	        /* Transfer to next */
	        link->SelectOrderNumber = object->Number + 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Transfer to next */
	        link->SelectOrderNumber = 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
            }
	    else
	    {
	      if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	      {
	        /* Select first */
	        object->Select[0] = 1;
	        object->OldSelect[0] = 1;
	        link->SelectObjid = object->Objid[0];
	      }
	      else
	      {
	        /* Select last */
	        object->Select[num-1] = 1;
	        object->OldSelect[num-1] = 1;
	        link->SelectObjid = object->Objid[num-1];
	      }
	      link->SelectOrder = 0;
	    }
	  }

	  if ( link->SelectOrderType == NMPS_DISP_ORDERTYPE_PREV)
	  {
	    if ( num == 0)
	    {
	      if ( object->Number != 1)
	      {
	        /* Transfer to previous */
	        link->SelectOrderNumber = object->Number - 1;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
	      else
	      {
	        /* Transfer to last */
	        link->SelectOrderNumber = link->MaxDispNumber;
	        link->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	        link->SelectOrder = 1;
	        link->SelectOrderOwner = object->Number;
	      }
            }
	    else
	    {
	      if ( object->SelDirection == NMPS_DISP_DIRECT_FORW)
	      {
	        /* Select last */
	        object->Select[num - 1] = 1;
	        object->OldSelect[num - 1] = 1;
	        link->SelectObjid = object->Objid[num-1];
	      }
	      else
	      {
	        /* Select first */
	        object->Select[0] = 1;
	        object->OldSelect[0] = 1;
	        link->SelectObjid = object->Objid[0];
	      }
	      link->SelectOrder = 0;
	    }
	  }
	}
	object->OldLastIndex = num;
}


void DispLink_exec( 
  plc_sThread		*tp,
  pwr_sClass_DispLink  *object)
{
	if ( !object->SelectExist)
	  object->ResetData = 1;

	if ( object->SelectOrder && object->SelectOrderOwner == NMPS_DISP_ORDEROWNER_LINK)
	  object->SelectOrder = 0;

	object->DoRemove = 0;
	object->DoMoveForward = 0;
	object->DoMoveBackward = 0;
	object->DoSelectNext = 0;
	object->DoSelectPrevious = 0;

	if ( object->Remove)
	{
	  object->DoRemove = 1;
	  object->Remove = 0;
	}
	if ( object->MoveForward)
	{
	  object->DoMoveForward = 1;
	  object->MoveForward = 0;
	}
	if ( object->MoveBackward)
	{
	  object->DoMoveBackward = 1;
	  object->MoveBackward = 0;
	}
	if ( object->SelectNext)
	{
	  if ( !object->SelectExist)
	  {
	    /* Give an order to first cell */
	    object->SelectOrderNumber = 1;
	    object->SelectOrderType = NMPS_DISP_ORDERTYPE_NEXT;
	    object->SelectOrder = 1;
	    object->SelectOrderOwner = NMPS_DISP_ORDEROWNER_LINK;
	  }
	  else
	    object->DoSelectNext = 1;
	  object->SelectNext = 0;
	}
	if ( object->SelectPrevious)
	{
	  if ( !object->SelectExist)
	  {
	    /* Give an order to last cell */
	    object->SelectOrderNumber = object->MaxDispNumber;
	    object->SelectOrderType = NMPS_DISP_ORDERTYPE_PREV;
	    object->SelectOrder = 1;
	    object->SelectOrderOwner = NMPS_DISP_ORDEROWNER_LINK;
	  }
	  else
	    object->DoSelectPrevious = 1;
	  object->SelectPrevious = 0;
	}

	object->SelectExist = 0;

}
