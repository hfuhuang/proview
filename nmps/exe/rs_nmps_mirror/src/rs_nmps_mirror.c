/* 
 * Proview   $Id: rs_nmps_mirror.c,v 1.3 2005-10-25 15:28:10 claes Exp $
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

/*************************************************************************
*
* 	PROGRAM		rs_nmps_mirror
*
*       Modifierad
*		950508	Claes Sj�fors	Skapad
*
*
*
*	Funktion:	Hanterar spegling av celler.
*
**************************************************************************/

/*_Include filer_________________________________________________________*/

#ifdef OS_VMS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <libdef.h>
#include <starlet.h>
#include <lib$routines.h>
#endif

#if defined OS_LYNX || defined OS_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#endif

#ifdef OS_ELN
#include stdio
#include stdlib
#include string
#include math
#include float
#include libdef
#include lib$routines
#include starlet
#endif

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "pwr_nmpsclasses.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "rt_gdh_msg.h"
#include "rt_hash_msg.h"
#include "rs_nmps.h"
#include "rs_nmps_msg.h"
#include "rs_sutl.h"

/*_Globala variabler______________________________________________________*/

/* Global functions________________________________________________________*/

#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif
#define	LogAndExit( status) \
{\
  errh_CErrLog(NMPS__EXIT, errh_ErrArgMsg(status), NULL);\
  exit( status);\
}

#define	LogAndReturn( status1, status2) \
{\
  errh_CErrLog(status1, errh_ErrArgMsg(status2), NULL);\
  return status2;\
}

#define	Log( status1, status2) \
{\
  errh_CErrLog(status1, errh_ErrArgMsg(status2), NULL);\
}

#define	NMPS_CELLBUFF_SIZE		1000	/* This must be >= the size of
						   the largest cell */
#define	NMPS_DATABUFF_SIZE		1000	/* This is the maximum size of
						   data subscriptions */


typedef struct nmpsmir_bckremoved_tag {
	pwr_tObjid			data_objid;
	char				*data_ptr;
	gdh_tDlid			data_subid;
	pwr_tObjid			mircell_objid;
	pwr_tTime			remove_time;
	unsigned char			collect;
	pwr_tObjid			collect_cell;
	float				release_time;
	struct nmpsmir_bckremoved_tag	*next_ptr;
	} nmpsmir_t_bckremoved;

typedef struct nmpsmir_data_tag{
	pwr_tObjid			data_objid;
	pwr_tObjid			orig_data_objid;
	char				*data_ptr;
	char				*orig_data_ptr;
	gdh_tDlid			data_subid;
	gdh_tSubid			orig_data_subid;
	pwr_tObjid			mircell_objid;
	int				convert;
	pwr_sClass_NMpsConvConfig	*convconfig;
	unsigned int			subscript_old;
	unsigned char			no_init;
	unsigned char			removed;
	pwr_tTime			remove_time;
	char				*plc_data_ptr;
	gdh_tDlid			plc_dlid;
	pwr_tBoolean			local;
	unsigned char			collect;
	pwr_tObjid			collect_cell;
	float				release_time;
	pwr_tOName     			orig_data_name;
	int				error_logged;
	struct nmpsmir_data_tag		*next_ptr;
	struct nmpsmir_data_tag		*prev_ptr;
	} nmpsmir_t_data_list;

typedef struct {
	int			initialized;
	pwr_tObjid		objid;
	pwr_sClass_NMpsMirrorCell *cell;
	gdh_tDlid		subid;
	int			conversion;
	pwr_sClass_NMpsConvConfig	*convconfig;
	int			orig_cell_count;
	pwr_tObjid		orig_objid[ MMPS_CELLMIR_ORIGCELL_SIZE];
	pwr_tClassId		orig_class[ MMPS_CELLMIR_ORIGCELL_SIZE];
	pwr_sClass_NMpsCell	*orig_cell[ MMPS_CELLMIR_ORIGCELL_SIZE];
	gdh_tDlid		orig_subid[ MMPS_CELLMIR_ORIGCELL_SIZE];
	pwr_tBoolean		orig_local[ MMPS_CELLMIR_ORIGCELL_SIZE];
	pwr_tBoolean		orig_notinit[ MMPS_CELLMIR_ORIGCELL_SIZE];
	pwr_tBoolean		connect_error_logged[ MMPS_CELLMIR_ORIGCELL_SIZE];
	pwr_tBoolean		orig_restart_old[ MMPS_CELLMIR_ORIGCELL_SIZE];
	pwr_tBoolean		restart;
	int			data_count;
	pwr_tObjid		orig_data_objid[ NMPS_CELLMIR_SIZE];
	pwr_tObjid		data_objid[ NMPS_CELLMIR_SIZE];
	nmpsmir_t_data_list	*datalist_ptr[ NMPS_CELLMIR_SIZE];
	char			data_noinit[ NMPS_CELLMIR_SIZE];
	nmpsmir_t_data_list	*removed_data_list;
	unsigned char		data_collect;
	pwr_tObjid		collect_cell;
	pwr_sClass_NMpsCell	*collect_cell_ptr;
	gdh_tDlid		collect_cell_dlid;
	} nmpsmir_t_cellmir_list;

typedef struct {
	pwr_tObjid			objid;
	pwr_sClass_NMpsConvConfig	*convconfig;
	gdh_tDlid			subid;
	} nmpsmir_t_convconfig_list;

typedef struct {
	int				first_scan;
	int				MirrorCellInitCount;
	int				OrigCellCount;
	int				RemoteOrigCellCount;
	int				OrigCellDownCount;
	int				OrigCellInitCount;
	int				DataObjectCount;
	int				ConvertDataCount;
	int				RemoteDataCount;
	int				RemoteDataDownCount;
	int				PendingRemoveDataCount;
	int				RemoveDataCount;
	int				CreateDataCount;
	int				DeleteDataCount;
	int				ReconnectDataCount;
	pwr_tUInt32			LoopCount;
	pwr_sClass_NMpsMirrorConfig	*mirrorconfig;
	gdh_tDlid			mirrorconf_dlid;
	nmpsmir_t_cellmir_list		*cellmirlist;
	int				cellmir_count;
	nmpsmir_t_convconfig_list	*convconfiglist;
	int				convconfig_count;
	nmpsmir_t_data_list		*data_list;
	nmpsmir_t_bckremoved		*bckremoved_list;
	int				init_done;
	} *mir_ctx;

/*_Local functions________________________________________________________*/
static pwr_tStatus	nmpsmir_find_collect_cell(
				mir_ctx		mirctx,
				pwr_tObjid	collect_cell,
				pwr_sClass_NMpsCell	**collect_cell_ptr);
pwr_tStatus	nmpsmir_data_db_delete( 
			nmpsmir_t_data_list	**data_list, 
			nmpsmir_t_data_list	*data_ptr);
static pwr_tStatus	nmpsmir_data_copy(  nmpsmir_t_data_list	*datalist_ptr);
static pwr_tStatus	nmpsmir_data_handler( mir_ctx	mirctx);
static pwr_tStatus	nmpsmir_data_db_create(  
			mir_ctx			mirctx,
			nmpsmir_t_data_list	**data_list,
			pwr_tObjid		objid,
			pwr_tObjid		mircell_objid,
			int			convert,
			pwr_sClass_NMpsConvConfig	*convconfig,
			nmpsmir_t_data_list	**datalist_ptr);
static pwr_tStatus	nmpsmir_data_db_find_orig(  
			nmpsmir_t_data_list	*data_list,
			pwr_tObjid		objid,
			pwr_tObjid		mircell_objid,
			nmpsmir_t_data_list	**datalist_ptr);
static pwr_tStatus	nmpsmir_data_db_find_orig_name(
			nmpsmir_t_data_list	*data_list,
			pwr_tObjid		objid,
			pwr_tObjid		mircell_objid,
			nmpsmir_t_data_list	**datalist_ptr);
static pwr_tStatus	nmpsmir_cellmirlist_add( 
			mir_ctx			mirctx,
			pwr_tObjid 		objid, 
			nmpsmir_t_cellmir_list 	**cellmirlist,
			int			*cellmirlist_count);
#if 0
static pwr_tStatus	nmpsmir_get_convconfig( 
			mir_ctx				mirctx,
			pwr_tObjid 			objid,
			char				**object_ptr);
#endif
static pwr_tStatus	nmpsmir_convconfiglist_add( 
			pwr_tObjid 			objid, 
			nmpsmir_t_convconfig_list 	**convconfiglist,
			int				*convconfiglist_count);
static pwr_tStatus	nmps_cellmir_init( mir_ctx	mirctx);
static pwr_tStatus	nmpsmir_cellmir_handler( mir_ctx	mirctx);


/****************************************************************************
* Name:		nmpsmir_bckremoved_store()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Create an entry in the datalist for an objid.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_bckremoved_store(
			nmpsmir_t_bckremoved		**bckremlist,
			pwr_tObjid			objid,
			nmpsmir_t_bckremoved		**bckremlist_ptr)
{
	*bckremlist_ptr = calloc( 1, sizeof( nmpsmir_t_bckremoved));
	if ( bckremlist_ptr == 0) return NMPS__NOMEMORY;

	/* Insert first in list */
	(*bckremlist_ptr)->next_ptr = *bckremlist;

	*bckremlist = *bckremlist_ptr;
	(*bckremlist_ptr)->data_objid = objid;
	return NMPS__SUCCESS;
}


#if 0
/****************************************************************************
* Name:		nmpsmir_bckremoved_find()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Find an entry in bckremlist.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_bckremoved_find(  
				nmpsmir_t_bckremoved		*bckremlist,
				pwr_tObjid		objid,
				nmpsmir_t_bckremoved		**bckremlist_ptr)
{
	while ( bckremlist != NULL)
	{
	  if ( cdh_ObjidIsEqual(bckremlist->data_objid, objid))
	  {
	    *bckremlist_ptr = bckremlist;
	    return NMPS__SUCCESS;
	  }
	  bckremlist = bckremlist->next_ptr;
	}
	return NMPS__DATANOTFOUND;
}
#endif
#if 0

/****************************************************************************
* Name:		nmpsmir_bckremoved_free()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Free the bckremlist.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_bckremoved_free( 	
				nmpsmir_t_bckremoved		*bckremlist)
{
	nmpsmir_t_bckremoved		*bckremlist_ptr;

	while ( bckremlist != NULL)
	{
	  bckremlist_ptr = bckremlist;
	  bckremlist = bckremlist->next_ptr;
	  free( bckremlist_ptr);
	}
	return NMPS__SUCCESS;
}
#endif

/****************************************************************************
* Name:		nmpsmir_bckremoved_delete()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Delete an entry in the bckremoved list.
*
**************************************************************************/
pwr_tStatus	nmpsmir_bckremoved_delete( 
			nmpsmir_t_bckremoved	**bckremoved_list, 
			nmpsmir_t_bckremoved	*bckremoved_ptr)
{
	nmpsmir_t_bckremoved	*br_ptr;

	if ( bckremoved_ptr == *bckremoved_list)
	  /* Change the root */
	  *bckremoved_list = bckremoved_ptr->next_ptr;
	else
	{
	  /* Find the previous entry */
	  br_ptr = *bckremoved_list;
	  while (br_ptr != NULL)
	  {
	    if ( br_ptr->next_ptr == bckremoved_ptr)
	    {
	      br_ptr->next_ptr = bckremoved_ptr->next_ptr;
	      break;
	    }
	  }
	}
	free( bckremoved_ptr);

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_find_collect_cell()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Find the collect cell.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_find_collect_cell(
				mir_ctx		mirctx,
				pwr_tObjid	collect_cell,
				pwr_sClass_NMpsCell	**collect_cell_ptr)
{
	nmpsmir_t_cellmir_list	*cellmir_ptr;
	int			i;
	int			found;

	/* Loop through the CellMir objects */
	cellmir_ptr = mirctx->cellmirlist;
	for ( i = 0; i < mirctx->cellmir_count; i++)
	{
	  if ( cdh_ObjidIsEqual( cellmir_ptr->collect_cell, collect_cell))
	  {
	    *collect_cell_ptr = cellmir_ptr->collect_cell_ptr;
	    found = 1;
	    break;
	  }
	  cellmir_ptr++;
 	}
	if ( !found)
	  return NMPS__NOCOLLECT;

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_time_to_release()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Calculate if its time to release data object.
*		If the time since remove from the NMpsMirrorCell is
*		greater than release_time, it's time.
*
**************************************************************************/
static int	nmnps_time_to_release( 	pwr_tTime 	*remove_time, 
					pwr_tTime	*current_time,
					float		release_time)
{
/***********
	pwr_tTime	diff_time;
	float		time;

	lib$subx( remove_time, current_time, &diff_time);
	time = - diff_time.low/10000000.;
	return ( time > release_time);
**********/
	pwr_tDeltaTime	diff_time;

	time_Adiff( &diff_time, current_time, remove_time);
	if ( diff_time.tv_sec >= release_time)
	  return 1;
	else
	  return 0;
}

/****************************************************************************
* Name:		nmpsmir_data_copy()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Copy from original data object to mirror object for a data
*		entry.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_data_copy(  nmpsmir_t_data_list	*datalist_ptr)
{
	char	*from_ptr;
	char	*to_ptr;
	int	size;
	char	databuf[NMPS_DATABUFF_SIZE];
	int	sts;

	if ( !datalist_ptr->local)
	{
	  sts = gdh_SubData( datalist_ptr->orig_data_subid, databuf,
			sizeof( databuf));
	  if ( sts == GDH__SUBOLD) return sts;
	  else if (EVEN(sts))
	  {
	     if ( !datalist_ptr->error_logged)
	     {
	       Log( NMPS__MIRSUBDATA, sts);
	       datalist_ptr->error_logged = 1;
	     }
	     return GDH__SUBOLD;
	  }
	  if (datalist_ptr->error_logged)
	    datalist_ptr->error_logged = 0;
	  from_ptr = databuf +
		datalist_ptr->convconfig->CopyOffset;
	}
	else
	  from_ptr = datalist_ptr->orig_data_ptr +
		datalist_ptr->convconfig->CopyOffset;
	to_ptr = datalist_ptr->data_ptr +
		datalist_ptr->convconfig->PasteOffset;
	size = datalist_ptr->convconfig->CopySize;

	memcpy( to_ptr, from_ptr, size);

	return NMPS__SUCCESS;
}


/*************************************************************************
*
* Name:		nmpsmir_data_handler
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Mirroring of data objects.
*
**************************************************************************/

static pwr_tStatus	nmpsmir_data_handler( mir_ctx	mirctx)
{
	nmpsmir_t_data_list	*data_ptr;
	nmpsmir_t_data_list	*next_ptr;
	pwr_tStatus		sts;
	pwr_tTime		current_time;
	pwr_sClass_NMpsCell	*collect_cell_ptr;
	nmpsmir_t_bckremoved		*bckremoved_ptr;
	nmpsmir_t_bckremoved		*bckremoved_next_ptr;

	clock_gettime( CLOCK_REALTIME, &current_time);

	/* Loop trough the dataobject database and copy the objects */
	mirctx->DataObjectCount = 0;
	mirctx->ConvertDataCount = 0;
	mirctx->RemoteDataCount = 0;
	mirctx->RemoteDataDownCount = 0;
	mirctx->PendingRemoveDataCount = 0;
	data_ptr = mirctx->data_list;
	while ( data_ptr != NULL)
	{
	  mirctx->DataObjectCount++;
	  if ( data_ptr->removed)
	  {
	    mirctx->PendingRemoveDataCount++;

	    /* Check time since remove */
	    if ( nmnps_time_to_release( 
		&data_ptr->remove_time, &current_time, data_ptr->release_time))
	    {
	      mirctx->RemoveDataCount++;
	      if ( data_ptr->collect)
	      {
	        /* Put into collect cell */
	        sts = nmpsmir_find_collect_cell( mirctx, data_ptr->collect_cell,
				&collect_cell_ptr);
		if ( EVEN(sts)) errh_CErrLog( sts, NULL);
	          
	        /* Insert in collect cell with extern function */
		if ( collect_cell_ptr->ExternFlag)
	        {
	          /* Cell is busy, wait till next scan */
	          data_ptr = data_ptr->next_ptr;
	          continue;
	        }

		collect_cell_ptr->ExternOpType = 0;
		collect_cell_ptr->ExternObjId = data_ptr->data_objid;
		collect_cell_ptr->ExternFlag = 1;
	        
		/* Remove subscriptions */
	        sts = gdh_DLUnrefObjectInfo( data_ptr->data_subid);
		if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        if ( data_ptr->local)
	        {
	          sts = gdh_DLUnrefObjectInfo( data_ptr->orig_data_subid);
	  	  if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        }
	        else
	        {
	          sts = gdh_SubUnrefObjectInfo( data_ptr->orig_data_subid);
	  	  if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        }
	        sts = gdh_DLUnrefObjectInfo( data_ptr->plc_dlid);
		if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);

	        /* Delete the data entry */
	        next_ptr = data_ptr->next_ptr;
	        sts = nmpsmir_data_db_delete( &mirctx->data_list, data_ptr);
	        data_ptr = next_ptr;
	        continue;
	      }
	      else
	      {
		/* Delete the object */

		/* Remove subscriptions */
	        sts = gdh_DLUnrefObjectInfo( data_ptr->data_subid);
		if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        if ( data_ptr->local)
	        {
	          sts = gdh_DLUnrefObjectInfo( data_ptr->orig_data_subid);
		  if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        }
	        else
	        {
	          sts = gdh_SubUnrefObjectInfo( data_ptr->orig_data_subid);
		  if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        }
	        sts = gdh_DLUnrefObjectInfo( data_ptr->plc_dlid);
		if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);

	        if ( data_ptr->convert)
	        {
	          mirctx->DeleteDataCount++;	
	          sts = gdh_DeleteObject( data_ptr->data_objid);
	  	  if (EVEN(sts)) Log(NMPS__MIRDELDATA, sts);
	        }
	        next_ptr = data_ptr->next_ptr;
	        sts = nmpsmir_data_db_delete( &mirctx->data_list, data_ptr);
	        data_ptr = next_ptr;
	        continue;
	      }
	    }
	  }
	  if ( data_ptr->convert)
	  {
	    if ( !data_ptr->local)
	    {
	      mirctx->RemoteDataCount++;
	      /* Check subscription oldness */
/*
	      sts = gdh_GetSubscriptionOldness (
			data_ptr->orig_data_subid, 
			&data_ptr->subscript_old, NULL, &subscript_sts);
	      if ( EVEN(sts)) return sts;
*/
	      data_ptr->subscript_old = 0;
	      sts = nmpsmir_data_copy( data_ptr);
	      if ( sts == GDH__SUBOLD)
	        data_ptr->subscript_old = 1;
	      else if (EVEN(sts)) return sts;
	      if ( data_ptr->subscript_old)
	      {
	        /* Continue with next */
	        mirctx->RemoteDataDownCount++;
	        data_ptr = data_ptr->next_ptr;
	        continue;
	      }
	      mirctx->ConvertDataCount++;
	      data_ptr->no_init = 0;
	    }
	    else
	    {
	      /* Copy from original to mirror object */
	      mirctx->ConvertDataCount++;
	      sts = nmpsmir_data_copy( data_ptr);
	      data_ptr->no_init = 0;
	    }
	  }
	  data_ptr = data_ptr->next_ptr;
	}
	
	/* Loop through the backup removed list */
	bckremoved_ptr = (
		struct nmpsmir_bckremoved_tag *) mirctx->bckremoved_list;
	while ( bckremoved_ptr != NULL)
	{
	  /* Check time since remove */
	  if ( nmnps_time_to_release( 
		&bckremoved_ptr->remove_time, &current_time, 
		bckremoved_ptr->release_time))
	  {
	    mirctx->RemoveDataCount++;

	    /* Put into collect cell */
	    sts = nmpsmir_find_collect_cell( mirctx, 
			bckremoved_ptr->collect_cell,
			&collect_cell_ptr);
	    if ( EVEN(sts)) errh_CErrLog( sts, NULL);
	          
	    /* Insert in collect cell with extern function */
	    if ( collect_cell_ptr->ExternFlag)
	    {
	      /* Cell is busy, wait till next scan */
	      bckremoved_ptr = bckremoved_ptr->next_ptr;
	      continue;
	    }
	    collect_cell_ptr->ExternOpType = 0;
	    collect_cell_ptr->ExternObjId = bckremoved_ptr->data_objid;
	    collect_cell_ptr->ExternFlag = 1;

	    /* Delete the data entry */
	    bckremoved_next_ptr = bckremoved_ptr->next_ptr;
	    sts = nmpsmir_bckremoved_delete( &mirctx->bckremoved_list,
			bckremoved_ptr);
	    bckremoved_ptr = bckremoved_next_ptr;
	    continue;
	  }
	  bckremoved_ptr = bckremoved_ptr->next_ptr;
	}
	mirctx->mirrorconfig->DataObjectCount = mirctx->DataObjectCount;
	mirctx->mirrorconfig->ConvertDataCount = mirctx->DataObjectCount;
	mirctx->mirrorconfig->RemoteDataCount = mirctx->RemoteDataCount;
	mirctx->mirrorconfig->RemoteDataDownCount = mirctx->RemoteDataDownCount;
	mirctx->mirrorconfig->PendingRemoveDataCount = mirctx->PendingRemoveDataCount;
	mirctx->mirrorconfig->RemoveDataCount = mirctx->RemoveDataCount;
	mirctx->mirrorconfig->DeleteDataCount = mirctx->DeleteDataCount;

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_data_db_delete()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Delete an entry in the datalist.
*
**************************************************************************/
pwr_tStatus	nmpsmir_data_db_delete( 
			nmpsmir_t_data_list	**data_list, 
			nmpsmir_t_data_list	*data_ptr)
{
	if ( data_ptr == *data_list)
	{
	  /* Change the root */
	  *data_list = data_ptr->next_ptr;
	  if ( *data_list)
	    (*data_list)->prev_ptr = 0;
	}

	if ( data_ptr->prev_ptr )
	  data_ptr->prev_ptr->next_ptr = data_ptr->next_ptr;
	if ( data_ptr->next_ptr )
	  data_ptr->next_ptr->prev_ptr = data_ptr->prev_ptr;

	free( data_ptr);

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_data_db_create()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Create an entry in the datalist for an objid.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_data_db_create(  
			mir_ctx			mirctx,
			nmpsmir_t_data_list	**data_list,
			pwr_tObjid		objid,
			pwr_tObjid		mircell_objid,
			int			convert,
			pwr_sClass_NMpsConvConfig	*convconfig,
			nmpsmir_t_data_list	**datalist_ptr)
{
	nmpsmir_t_data_list	*next_ptr;
	pwr_tBoolean		local;
	pwr_tStatus		sts;
	pwr_tOName     		name;
	pwr_tOName     		new_name;
	pwr_sAttrRef		attrref;
	char			*s;
	pwr_tStatus		return_sts;

	return_sts = NMPS__SUCCESS;

	/* Get name and location of the original object */
	sts = gdh_GetObjectLocation ( objid, &local);
	if ( EVEN(sts))
	  local = 0;

	if ( convert)
	{
	  sts = gdh_ObjidToName( objid, name, sizeof(name), cdh_mNName);
	  if ( EVEN(sts) && !local) return NMPS__ORIGDATA;
	  else if (EVEN(sts))
	  {
	    LogAndReturn(NMPS__ORIGDATA, sts);
	  }
	}
	else
	  strcpy( name, "");

	*datalist_ptr = calloc( 1, sizeof( nmpsmir_t_data_list));
	if ( *datalist_ptr == 0) return NMPS__NOMEMORY;

	/* Insert first in list */
	next_ptr = *data_list;
	if ( next_ptr != NULL)
	  next_ptr->prev_ptr = *datalist_ptr;
	(*datalist_ptr)->next_ptr = next_ptr;

	*data_list = *datalist_ptr;
	(*datalist_ptr)->convert = convert;
	(*datalist_ptr)->orig_data_objid = objid;
	(*datalist_ptr)->mircell_objid = mircell_objid;
	strcpy( (*datalist_ptr)->orig_data_name, name);
	(*datalist_ptr)->local = local;

	if ( convert)
	{
	  (*datalist_ptr)->convconfig = convconfig;

	  /* Create a new object */
	  /* Take the same last segment name as the old object */
	  s = strrchr( name, '-');
	  if ( s)
	    strcpy( name, s+1);

	  sts = gdh_ObjidToName( convconfig->OutDataParent, new_name, sizeof(new_name), cdh_mNName);
	  if (EVEN(sts)) LogAndReturn(NMPS__MIRPARENT, sts);
	  strcat( new_name, "-");
	  strcat( new_name, name);
	  
	  sts = gdh_CreateObject( new_name, 
			cdh_ClassObjidToId( convconfig->OutDataClass),
			0, &(*datalist_ptr)->data_objid, pwr_cNObjid, 0, pwr_cNObjid);
	  if ( sts == GDH__DUPLNAME )
	  {
	    /* The object already exist, and this might be a restart,
	       use the existing object */
	    sts = gdh_NameToObjid ( new_name,  &(*datalist_ptr)->data_objid);
	    if ( EVEN(sts)) LogAndReturn(NMPS__DATARECONERR, sts);
	    if ( !mirctx->first_scan )
	      errh_CErrLog( NMPS__DATARECONNECT, errh_ErrArgAF( new_name), NULL);
	    mirctx->ReconnectDataCount++;
	    return_sts = NMPS__DATAALREXIST;
	    (*datalist_ptr)->removed = 0;
	  }
	  else if (EVEN(sts)) 
	  {
	    LogAndReturn(NMPS__MIRCREADATA, sts);
	  }
	  else
	  {
	    mirctx->CreateDataCount++;
	  }

	  if ( local)
	  {
	    /* Get a direct link to the original object */
	    memset( &attrref, 0, sizeof(attrref));
	    attrref.Objid = objid;
	    sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &(*datalist_ptr)->orig_data_ptr,
		&(*datalist_ptr)->orig_data_subid);
	    if (EVEN(sts)) LogAndReturn(NMPS__MIRORIGDATA, sts);
	  }
	  else
	  {
	    /* Get a subscription to the original object */
	    memset( &attrref, 0, sizeof(attrref));
            sts = gdh_ClassAttrToAttrref( 
		cdh_ClassObjidToId( convconfig->OutDataClass), 
		NULL, &attrref);
	    if (EVEN(sts)) LogAndReturn(NMPS__MIRORIGDATA, sts);

	    attrref.Objid = objid;
/*	    attrref.Size = convconfig->CopyOffset + convconfig->CopySize;
*/
/** Test
	    {
              void *infop;
	      sts = gdh_ObjidToName ( objid, name, sizeof(name), 
			cdh_mName_volumeStrict);
	      if ( EVEN(sts)) return NMPS__NODEDOWN;
	      sts = gdh_RefObjectInfo( name, (void *)&infop, 
		&(*datalist_ptr)->orig_data_subid, attrref.Size);
	      if ( EVEN(sts)) return sts;
	    }
**/
	    sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&(*datalist_ptr)->orig_data_subid);
	    if (EVEN(sts)) LogAndReturn(NMPS__MIRORIGDATA, sts);
/********* use gdh_SubData instead...
	    sts = gdh_SubAssociateBuffer(
		(*datalist_ptr)->orig_data_subid,
		&(*datalist_ptr)->orig_data_ptr,
		attrref.Size);
	    if (EVEN(sts)) LogAndReturn(NMPS__MIRASSBUFF, sts);
***/
	  }

	  /* Get a direct link to the new object */
	  memset( &attrref, 0, sizeof(attrref));
	  attrref.Objid = (*datalist_ptr)->data_objid;
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref,
		(pwr_tAddress *) &(*datalist_ptr)->data_ptr,
		&(*datalist_ptr)->data_subid);
	  if (EVEN(sts)) LogAndReturn(NMPS__MIRDATA, sts);

	  if ( local)
	  {
	    /* Copy from original to the new object */
	    sts = nmpsmir_data_copy( *datalist_ptr);
	  }
	  else
	    (*datalist_ptr)->no_init = 1;
	}
	else
	{
	  /* The object has to be local */
	  if ( !local)
	  {
	    return NMPS__NOTLOCAL;
	  }

	  (*datalist_ptr)->data_objid = objid;
	}
	return return_sts;
}


/****************************************************************************
* Name:		nmpsmir_data_db_find_orig()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Find an entry in the datalist for an objid from
*		original objid.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_data_db_find_orig(  
			nmpsmir_t_data_list	*data_list,
			pwr_tObjid		objid,
			pwr_tObjid		mircell_objid,
			nmpsmir_t_data_list	**datalist_ptr)
{
	nmpsmir_t_data_list	*data_ptr;
	int			found;

	/* Insert first in list */
	found = 0;
	data_ptr = data_list;
	while ( data_ptr != NULL)
	{
	  if ( cdh_ObjidIsEqual( data_ptr->orig_data_objid, objid))
		/* && data_ptr->mircell_objid == mircell_objid) */
	  {
	    *datalist_ptr = data_ptr;
	    found = 1;
	    break;
	  }
	  data_ptr = data_ptr->next_ptr;
	}
	
	if ( !found)
	  return NMPS__DATANOTFOUND;

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_data_db_find_orig()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Find an entry in the datalist for an objid from
*		original objid.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_data_db_find_orig_name(
			nmpsmir_t_data_list	*data_list,
			pwr_tObjid		objid,
			pwr_tObjid		mircell_objid,
			nmpsmir_t_data_list	**datalist_ptr)
{
	pwr_tOName     		name;
	nmpsmir_t_data_list	*data_ptr;
	int			found;
	int			sts;

	/* Find the name */
	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mNName);
	if ( EVEN(sts)) return NMPS__NODEDOWN;

	/* Loop trough the data areas */
	found = 0;
	data_ptr = data_list;
	while ( data_ptr != NULL)
	{
	  if ( !strcmp( name, data_ptr->orig_data_name)) 
		/* && data_ptr->mircell_objid == mircell_objid) */
	  {
	    *datalist_ptr = data_ptr;
	    found = 1;
	    break;
	  }
	  data_ptr = data_ptr->next_ptr;
	}
	
	if ( !found)
	  return NMPS__DATANOTFOUND;

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_data_db_find()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Find an entry in the datalist for an objid.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_data_db_find(  
			nmpsmir_t_data_list	*data_list,
			pwr_tObjid		objid,
			pwr_tObjid		mircell_objid,
			nmpsmir_t_data_list	**datalist_ptr)
{
	nmpsmir_t_data_list	*data_ptr;
	int			found;

	/* Insert first in list */
	found = 0;
	data_ptr = data_list;
	while ( data_ptr != NULL)
	{
	  if ( cdh_ObjidIsEqual( data_ptr->data_objid, objid))
		/*  && data_ptr->mircell_objid == mircell_objid) */
	  {
	    *datalist_ptr = data_ptr;
	    found = 1;
	    break;
	  }
	  data_ptr = data_ptr->next_ptr;
	}
	
	if ( !found)
	  return NMPS__DATANOTFOUND;

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_origcell_init()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tStatus	nmpsmir_origcell_init( 
		pwr_tObjid	objid,
		char		**orig_cell,
		gdh_tSubid	*orig_subid,
	        pwr_tBoolean	*orig_local,
		pwr_tClassId	*orig_class)
{
	pwr_sAttrRef		attrref;
	pwr_tStatus		sts;
	pwr_tAName     		name;

	sts = gdh_GetObjectLocation( objid , orig_local);
	if ( EVEN(sts)) 
	{
	  *orig_local = 0;
	  return NMPS__NODEDOWN;
	}
	sts = gdh_GetObjectClass ( objid, orig_class);
	if ( EVEN(sts))
	{
	  if ( !*orig_local)
	    return NMPS__NODEDOWN;
	  else
	    return sts;
	}


	memset( &attrref, 0, sizeof(attrref));
        sts = gdh_ClassAttrToAttrref( *orig_class, NULL, &attrref);
	if (EVEN(sts)) LogAndReturn(NMPS__MIRORIGCELL, sts);

	attrref.Objid = objid;

	if ( *orig_local)
	{
	  /* This is a local object, direct link */
	  sts = gdh_DLRefObjectInfoAttrref( &attrref,
			(pwr_tAddress *) orig_cell, orig_subid);
	  if (EVEN(sts)) LogAndReturn(NMPS__MIRORIGCELL, sts);
	}
	else
	{
	  /* This is a remote object */
	  switch( *orig_class)
	  {
	    case pwr_cClass_NMpsCell:
	    case pwr_cClass_NMpsStoreCell:
	    {
	       pwr_tUInt16 	maxsize;

	      /* Get MaxSize and calculate the size of the subscripton */
	      sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mNName);
	      if ( EVEN(sts)) return NMPS__NODEDOWN;

	      strcat( name, ".MaxSize");
	      sts = gdh_GetObjectInfo ( name, &maxsize, sizeof(maxsize));
	      if ( EVEN(sts)) return NMPS__NODEDOWN;

/*	      attrref.Size = sizeof( pwr_sClass_NMpsCell) - 
		sizeof( plc_t_DataInfo) * ( NMPS_CELL_SIZE - maxsize);
*/
	      break;
	    }
	    case pwr_cClass_NMpsMirrorCell:
	    {
	       pwr_tUInt16 		maxsize;

	      /* Get MaxSize and calculate the size of the subscripton */
	      sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mNName);
	      if ( EVEN(sts)) return NMPS__NODEDOWN;

	      strcat( name, ".MaxSize");
	      sts = gdh_GetObjectInfo ( name, &maxsize, sizeof(maxsize));
	      if ( EVEN(sts)) return NMPS__NODEDOWN;

/*	      attrref.Size = sizeof( pwr_sClass_NMpsMirrorCell) - 
		sizeof( plc_t_DataInfoMirCell) * ( NMPS_CELLMIR_SIZE - maxsize)-
		sizeof( nmps_t_mircell_copyarea);
*/
	      break;
	    }
	    default:
	    {
	      errh_CErrLog( NMPS__ORIGCLASS, errh_ErrArgL( objid.oix), NULL);
	      return NMPS__ORIGCLASS;
	    }
	  }


/** Test
	  {
            void *infop;
	    sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mNName);
	    if ( EVEN(sts)) return NMPS__NODEDOWN;
	    sts = gdh_RefObjectInfo( name, (void *)&infop, orig_subid, 
		attrref.Size);
	   if ( EVEN(sts)) return sts;
	  }
**/

	  sts = gdh_SubRefObjectInfoAttrref( &attrref, orig_subid);
	  if (EVEN(sts)) LogAndReturn(NMPS__MIRORIGCELL, sts);
/**** Use gdh_SubData instead
	  sts = gdh_SubAssociateBuffer( *orig_subid, (void *)orig_cell, attrref.Size);
	  if (EVEN(sts)) LogAndReturn(NMPS__MIRASSBUFF, sts);
***/
	}
	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_cellmirlist_add()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Add a mirror cell to the cellmirlist
*
**************************************************************************/
static pwr_tStatus	nmpsmir_cellmirlist_add( 
			mir_ctx			mirctx,
			pwr_tObjid 		objid, 
			nmpsmir_t_cellmir_list 	**cellmirlist,
			int			*cellmirlist_count)
{
	nmpsmir_t_cellmir_list	*cellmirlist_ptr;
	nmpsmir_t_cellmir_list	*new_cellmirlist;
	pwr_sAttrRef		attrref;
	int			i;
	pwr_tStatus		sts;
	int			convconfig_found;
	nmpsmir_t_convconfig_list *convconf_ptr;
	pwr_tClassId		class;

	if ( *cellmirlist_count == 0)
	{
	  *cellmirlist = calloc( 1 , sizeof(nmpsmir_t_cellmir_list));
	  if ( *cellmirlist == 0)
	    return NMPS__NOMEMORY;
	}
	else
	{
	  new_cellmirlist = calloc( *cellmirlist_count + 1 , 
			sizeof(nmpsmir_t_cellmir_list));
	  if ( new_cellmirlist == 0)
	    return NMPS__NOMEMORY;
	  memcpy( new_cellmirlist, *cellmirlist, 
			*cellmirlist_count * sizeof(nmpsmir_t_cellmir_list));
	  free( *cellmirlist);
	  *cellmirlist = new_cellmirlist;
	}
	cellmirlist_ptr = *cellmirlist + *cellmirlist_count;
	cellmirlist_ptr->objid = objid;
	cellmirlist_ptr->initialized = 1;

	/* Direct link to the cell */
	memset( &attrref, 0, sizeof(attrref));
	attrref.Objid = objid;
	sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &cellmirlist_ptr->cell,
		&cellmirlist_ptr->subid);
	if (EVEN(sts)) LogAndReturn(NMPS__MIRCELL, sts);

	/* Direct link to collect cell */
	cellmirlist_ptr->data_collect = cellmirlist_ptr->cell->DataCollect;
	if ( cellmirlist_ptr->data_collect)
	{
	  cellmirlist_ptr->collect_cell = cellmirlist_ptr->cell->CollectCell;

	  /* Check class of collect cell */
	  sts = gdh_GetObjectClass ( cellmirlist_ptr->collect_cell, 
		&class);
	  if ( EVEN(sts))
	  {
	    errh_CErrLog( NMPS__COLLECTCELL, errh_ErrArgL( objid.oix), NULL);
	    return NMPS__COLLECTCELL;
	  }

	  if ( !(class == pwr_cClass_NMpsCell ||
	         class == pwr_cClass_NMpsStoreCell ||
	         class == pwr_cClass_NMpsMirrorCell))
	  {
	    errh_CErrLog( NMPS__COLLECTCLASS, errh_ErrArgL( objid.oix), NULL);
	    return NMPS__COLLECTCLASS;
	  }	

	  memset( &attrref, 0, sizeof(attrref));
	  attrref.Objid = cellmirlist_ptr->collect_cell;
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &cellmirlist_ptr->collect_cell_ptr,
		&cellmirlist_ptr->collect_cell_dlid);
	  if ( EVEN(sts)) return NMPS__COLLECTDL;
	}

	/* Direct link or subscript to the original cells */
	cellmirlist_ptr->orig_cell_count = 0;
	for ( i = 0; i < (int)cellmirlist_ptr->cell->NumberOfCellObj; i++)
	{
	  cellmirlist_ptr->orig_objid[i] = cellmirlist_ptr->cell->CellObjects[i];
	  sts = nmpsmir_origcell_init( cellmirlist_ptr->orig_objid[i],
		(char **) &cellmirlist_ptr->orig_cell[i],
		&cellmirlist_ptr->orig_subid[i],
	        &cellmirlist_ptr->orig_local[i], 
		&cellmirlist_ptr->orig_class[i]);
	  mirctx->OrigCellCount++;
	  cellmirlist_ptr->orig_cell_count++;
	  if ( !cellmirlist_ptr->orig_local[i])
	    mirctx->RemoteOrigCellCount++;
	  if ( sts == NMPS__NODEDOWN)
	  {
	    /* Try again later */
	    cellmirlist_ptr->orig_notinit[i] = 1;
	    cellmirlist_ptr->initialized = 0;
	    mirctx->init_done = 0;
	    continue;
	  }
	  else if ( EVEN(sts)) return sts;

	  cellmirlist_ptr->orig_notinit[i] = 0;
	  mirctx->OrigCellInitCount++;
	}

	/* Get conversion data */
	cellmirlist_ptr->conversion = cellmirlist_ptr->cell->DataObjConv;
	if ( cellmirlist_ptr->conversion)
	{
	  /* Find the conversion object */
	  convconfig_found = 0;
	  convconf_ptr = mirctx->convconfiglist;
	  for ( i = 0; i < mirctx->convconfig_count; i++)
	  {
	    if ( cdh_ObjidIsEqual( convconf_ptr->objid, 
			cellmirlist_ptr->cell->ConvConfig))
	    {
	      cellmirlist_ptr->convconfig = convconf_ptr->convconfig;
	      convconfig_found = 1;
	      break;
	    }
	    convconf_ptr++;
	  }
	  if ( !convconfig_found)
	    return NMPS__NOCONVCONFIG;
	}

	if (cellmirlist_ptr->initialized)
	  mirctx->MirrorCellInitCount++;

	(*cellmirlist_count)++;
	return NMPS__SUCCESS;
}


#if 0
/****************************************************************************
* Name:		nmpsmir_get_convconfig()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Get the conversion config object.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_get_convconfig( 
			mir_ctx				mirctx,
			pwr_tObjid 			objid,
			char				**object_ptr)
{
	int				i;
	int				found;
	nmpsmir_t_convconfig_list 	*convconfig_ptr;

	found = 1;
	convconfig_ptr = mirctx->convconfiglist;
	for (i = 0; i < mirctx->convconfig_count; i++)
	{
	  if ( cdh_ObjidIsEqual( objid, convconfig_ptr->objid))
	  {
	    *object_ptr = (char *) convconfig_ptr->convconfig;
	    found = 1;
	    break;
	  }
	  convconfig_ptr++; 
	}
	if ( !found)
	  return NMPS__CONVOBJECT;

	return NMPS__SUCCESS;
}
#endif

/****************************************************************************
* Name:		nmpsmir_convconfig_add()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Add a conversion config object to the list.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_convconfiglist_add( 
			pwr_tObjid 			objid, 
			nmpsmir_t_convconfig_list 	**convconfiglist,
			int				*convconfiglist_count)
{
	nmpsmir_t_convconfig_list	*convconfiglist_ptr;
	nmpsmir_t_convconfig_list	*new_convconfiglist;
	pwr_sAttrRef		attrref;
	pwr_tStatus		sts;
	pwr_tClassId		class;

	if ( *convconfiglist_count == 0)
	{
	  *convconfiglist = calloc( 1 , sizeof(nmpsmir_t_convconfig_list));
	  if ( *convconfiglist == 0)
	    return NMPS__NOMEMORY;
	}
	else
	{
	  new_convconfiglist = calloc( *convconfiglist_count + 1,
			sizeof(nmpsmir_t_convconfig_list));
	  if ( new_convconfiglist == 0)
	    return NMPS__NOMEMORY;
	  memcpy( new_convconfiglist, *convconfiglist, 
			*convconfiglist_count * sizeof(nmpsmir_t_convconfig_list));
	  free( *convconfiglist);
	  *convconfiglist = new_convconfiglist;
	}
	convconfiglist_ptr = *convconfiglist + *convconfiglist_count;
	convconfiglist_ptr->objid = objid;

	/* Direct link to the cell */
	memset( &attrref, 0, sizeof(attrref));
	attrref.Objid = objid;
	sts = gdh_DLRefObjectInfoAttrref ( &attrref,
		(pwr_tAddress *) &convconfiglist_ptr->convconfig,
		&convconfiglist_ptr->subid);
	if ( EVEN(sts)) return sts;

	/* Check syntax of the ConvConfig object */
	sts = gdh_GetObjectClass ( convconfiglist_ptr->convconfig->OutDataClass, 
			&class);
	if ( EVEN(sts))
	{
	  errh_CErrLog( NMPS__CONVOUTDATACLASS, errh_ErrArgL(objid.oix), NULL);
	  return NMPS__CONVOUTDATACLASS;
	}
	sts = gdh_GetObjectClass ( convconfiglist_ptr->convconfig->OutDataParent, 
			&class);
	if ( EVEN(sts))
	{
	  errh_CErrLog( NMPS__CONVPARENT, errh_ErrArgL(objid.oix), NULL);
	  return NMPS__CONVPARENT;
	}

	sts = gdh_GetObjectClass ( convconfiglist_ptr->convconfig->InDataClass,
			&class);
	if ( EVEN(sts))
	{
	  errh_CErrLog( NMPS__CONVINDATACLASS, errh_ErrArgL(objid.oix), NULL);
	  return NMPS__CONVINDATACLASS;
	}

	if ( convconfiglist_ptr->convconfig->CopySize == 0)
	{
	  errh_CErrLog( NMPS__CONVCOPYSIZE, errh_ErrArgL(objid.oix), NULL);
	  return NMPS__CONVCOPYSIZE;
	}

	(*convconfiglist_count)++;
	return NMPS__SUCCESS;
}


/*************************************************************************
*
* Name:		nmps_cellmir_init
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Initiering av CellMir funktionen.
*
**************************************************************************/

static pwr_tStatus	nmps_cellmir_init( mir_ctx	mirctx)
{
	pwr_tStatus	sts;
	pwr_tObjid	objid;
	pwr_sAttrRef		attrref;

	mirctx->init_done = 1;

	/* Get pointer to the MirrorConfig object */
	sts = gdh_GetClassList ( pwr_cClass_NMpsMirrorConfig, &objid);
	if (EVEN(sts)) return NMPS__MIRRORCONFIG;

	/* Direct link to the cell */
	memset( &attrref, 0, sizeof(attrref));
	attrref.Objid = objid;
	sts = gdh_DLRefObjectInfoAttrref ( &attrref,
		(pwr_tAddress *) &mirctx->mirrorconfig,
		&mirctx->mirrorconf_dlid);
	if ( EVEN(sts)) return sts;

	/* Get the convert config objects on this node */
	sts = gdh_GetClassList ( pwr_cClass_NMpsConvConfig, &objid);
	while ( ODD(sts))
	{
          /* Store and direct link the ConvConfig objects */
          sts = nmpsmir_convconfiglist_add( objid, 
		&mirctx->convconfiglist, 
		&mirctx->convconfig_count);
	  if ( EVEN(sts)) return sts;

	  sts = gdh_GetNextObject( objid, &objid);
	}

	/* Get the CellMir objects on this node */
	sts = gdh_GetClassList ( pwr_cClass_NMpsMirrorCell, &objid);
	while ( ODD(sts))
	{
          /* Store and direct link the CellMirs */
          sts = nmpsmir_cellmirlist_add( mirctx, objid, &mirctx->cellmirlist, 
		&mirctx->cellmir_count);
	  if ( EVEN(sts)) return sts;

	  sts = gdh_GetNextObject( objid, &objid);
	}

	mirctx->mirrorconfig->MirrorCellCount = mirctx->cellmir_count;
	mirctx->mirrorconfig->OrigCellCount = mirctx->OrigCellCount;
	mirctx->mirrorconfig->RemoteOrigCellCount = mirctx->RemoteOrigCellCount;
	mirctx->OrigCellDownCount = mirctx->OrigCellCount - 
		mirctx->OrigCellInitCount;
	mirctx->mirrorconfig->OrigCellDownCount = mirctx->OrigCellDownCount;

	if ( mirctx->MirrorCellInitCount == mirctx->cellmir_count)
	  errh_CErrLog( NMPS__MIRRORINIT, NULL);
	else
	  errh_CErrLog( NMPS__MIRRORNOINIT, NULL);

	return NMPS__SUCCESS;
}


/*************************************************************************
*
* Name:		nmps_cellmir_reinit
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Initiering av CellMir funktionen.
*
**************************************************************************/

static pwr_tStatus	nmps_cellmir_reinit( mir_ctx	mirctx)
{
	nmpsmir_t_cellmir_list	*cellmir_ptr;
	int			i, j;
	pwr_tStatus		sts;

	mirctx->MirrorCellInitCount = 0;
	mirctx->init_done = 1;

	/* Loop through the CellMir objects */
	cellmir_ptr = mirctx->cellmirlist;
	for ( i = 0; i < mirctx->cellmir_count; i++)
	{
	  if ( !cellmir_ptr->initialized)
	  {
	    /* One orig_cell or more are not initialized */
	    cellmir_ptr->initialized = 1;
	    for( j = 0; j < cellmir_ptr->orig_cell_count; j++)
	    {
	      if ( cellmir_ptr->orig_notinit[j])
	      {
	        sts = nmpsmir_origcell_init( cellmir_ptr->orig_objid[j],
			(char **) &cellmir_ptr->orig_cell[j],
			&cellmir_ptr->orig_subid[j],
	        	&cellmir_ptr->orig_local[j], 
			&cellmir_ptr->orig_class[j]);
	        if ( sts == NMPS__NODEDOWN)
	        {
	          /* Try again later */
	          cellmir_ptr->initialized = 0;
	          mirctx->init_done = 0;
	          continue;
	        }
	        else if ( EVEN(sts)) return sts;

	        mirctx->OrigCellInitCount++;
	        cellmir_ptr->orig_notinit[j] = 0;
	      }
	    }
	  }
	  if ( cellmir_ptr->initialized)
	    mirctx->MirrorCellInitCount++;
	  cellmir_ptr++;
	}
	mirctx->mirrorconfig->MirrorCellInitCount = mirctx->MirrorCellInitCount;
	mirctx->OrigCellDownCount = mirctx->OrigCellCount - 
		mirctx->OrigCellInitCount;
	mirctx->mirrorconfig->OrigCellDownCount = mirctx->OrigCellDownCount;

	if ( mirctx->MirrorCellInitCount == mirctx->cellmir_count)
	  errh_CErrLog( NMPS__MIRRORINIT, NULL);

	return NMPS__SUCCESS;
}


/*************************************************************************
*
* Name:		nmpsmir_cellmir_handler
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Mirroring.
*
**************************************************************************/

static pwr_tStatus	nmpsmir_cellmir_handler( mir_ctx	mirctx)
{
	pwr_tStatus		sts;
	pwr_sClass_NMpsMirrorCell	*cellmir_object;
	nmpsmir_t_cellmir_list	*cellmir_ptr;
	int			i, j, k, l;
	pwr_tBoolean		old;
	int			orig_data_count;
	int			next_cell;
	int			objid_found;
	nmpsmir_t_data_list	*datalist_ptr;
	nmpsmir_t_bckremoved	*bckremlist_ptr;
	nmps_t_mircell_copyarea	copyarea_buff;
	nmps_t_mircell_copyarea	*copyarea = &copyarea_buff;
	int			count;
	int			found;
	char			orig_cellbuf[NMPS_CELLBUFF_SIZE];
	int			change_detected;
	pwr_tBoolean		val;
	pwr_tOName     		name;
	
	/* Loop through the CellMir objects */
	cellmir_ptr = mirctx->cellmirlist;
	for ( i = 0; i < mirctx->cellmir_count; i++)
	{
	  if ( !cellmir_ptr->initialized)
	  {
	    /* This cell is not initialized yet */
	    cellmir_ptr++;
	    continue;
	  }

	  if ( cellmir_ptr->cell->UpdateFlag == 1)
	  {
	    /* Last update is yet not executed by the plc */
	    cellmir_ptr++;
	    continue;
	  }

	  memcpy( copyarea, cellmir_ptr->cell->TempArea, sizeof(*copyarea));

	  /* Read back the plc's dlid and direct link of dataobjects */
	  if ( cellmir_ptr->cell->InitFlag )
	  {
	    /* Restart of plcpgm, read back everything... (not implemented) */

	    if (!(cellmir_ptr->cell->Function & NMPS_CELLFUNC_BACKUP))
	      /* Dont wait for backup reload done */
	      cellmir_ptr->cell->InitFlag = 0;
	  }
	  else
	  {
	    /* Read back data with pointers that was not known in last scan */

	    count = 0;
	    for ( j = 0; j < cellmir_ptr->data_count; j++)
	    {
	      if ( count >= cellmir_ptr->cell->TempLastIndex)
	        break;
	      if ( !cellmir_ptr->data_noinit[j])
	      {
	        if ( cellmir_ptr->datalist_ptr[j]->plc_data_ptr == 0)
	        {
	          if ( cdh_ObjidIsEqual( copyarea->datainfo[count].Data_ObjId,
				cellmir_ptr->data_objid[j]))
	          {
		    cellmir_ptr->datalist_ptr[j]->plc_data_ptr = 
				(char *) copyarea->datainfo[count].DataP;
		    cellmir_ptr->datalist_ptr[j]->plc_dlid =
				copyarea->datainfo[count].Data_Dlid;
	          }
	        }
	        count++;
	      }
	    }
	  }

	  orig_data_count = 0;
	  next_cell = 0;
	  cellmir_object = cellmir_ptr->cell;
	  /* Loop through the original cells and collect the data objects */
	  for ( j = 0; j < cellmir_ptr->orig_cell_count; j++)
	  {
	    /* If remote cell, check subscription oldness */
	    if ( !cellmir_ptr->orig_local[j])
	    {
	      old = 0;
	      sts = gdh_SubData( cellmir_ptr->orig_subid[j], orig_cellbuf,
			sizeof( orig_cellbuf));
	      if ( EVEN(sts))
	        old = 1;
/*	      sts = gdh_GetSubscriptionOldness( cellmir_ptr->orig_subid[j],
			&old, NULL, &sub_sts);
	      if ( EVEN(sts)) return sts;
*/
	      if ( old)
	      {
	        if ( !cellmir_ptr->connect_error_logged[j])
	        {
		  mirctx->OrigCellDownCount++;
	          errh_CErrLog( NMPS__REMCELLDOWN,
				errh_ErrArgL( cellmir_ptr->orig_objid[j].oix),
				errh_ErrArgMsg(sts), NULL);

	          cellmir_ptr->connect_error_logged[j] = 1;
	        }
	        /* Don't handle this mircell now */
	        next_cell = 1;
	        break;
	      }

	      if ( cellmir_ptr->connect_error_logged[j])
	      {
		mirctx->OrigCellDownCount--;
	        errh_CErrLog( NMPS__REMCELLUP,
				errh_ErrArgL( cellmir_ptr->orig_objid[j].oix), NULL);
	        cellmir_ptr->connect_error_logged[j] = 0;
	      }
	    }

	    switch ( cellmir_ptr->orig_class[j])
	    {
	      case pwr_cClass_NMpsCell:
	      case pwr_cClass_NMpsStoreCell:
	      {
	        pwr_sClass_NMpsCell *orig_cell_ptr;
		plc_t_DataInfo		*data_block_ptr;

	        /* Put the data objids in the orig_objid array */
	        if ( cellmir_ptr->orig_local[j])
	          orig_cell_ptr = cellmir_ptr->orig_cell[j];
	        else
	          orig_cell_ptr = (pwr_sClass_NMpsCell *) orig_cellbuf;

	        /* Detect an edge on mirror restart flag */
	        if ( orig_cell_ptr->MirrorRestart &&
	             !cellmir_ptr->orig_restart_old[j])
	        {
	          cellmir_ptr->restart = 1;
	          /* Reset the orig cell */
	          sts = gdh_ObjidToName ( cellmir_ptr->orig_objid[j],
			name, sizeof(name), cdh_mNName);
	          if ( ODD(sts))
	          {
	            strcat( name, ".MirrorRestart");
	            val = 0;
	            sts = gdh_SetObjectInfo ( name, &val, sizeof(val));
	          }
	        }
	        cellmir_ptr->orig_restart_old[j] = orig_cell_ptr->MirrorRestart;

	        data_block_ptr = (plc_t_DataInfo *) &orig_cell_ptr->Data1P;
	        for ( k = 0; k < orig_cell_ptr->LastIndex; k++)
	        {
	          /* Check if the objid already is collected */
	          objid_found = 0;
	          if ( j > 0)
	          {
	            for ( l = 0; l < orig_data_count; l++)
	            {
	              if ( cdh_ObjidIsEqual( cellmir_ptr->orig_data_objid[l],
				data_block_ptr->Data_ObjId))
	              {
	                objid_found = 1;
	                break;
	              }
	            }
	          }
	          if ( !objid_found)
	          {
	            cellmir_ptr->orig_data_objid[ orig_data_count] =
		 	data_block_ptr->Data_ObjId;
	            orig_data_count++;
	          }
		  data_block_ptr++;
	        }
	        break;
	      }
	      case pwr_cClass_NMpsMirrorCell:
	      {	      
	        pwr_sClass_NMpsMirrorCell *orig_cell_ptr;
		plc_t_DataInfoMirCell	*data_block_ptr;

	        /* Put the data objids in the orig_objid array */
	        if ( cellmir_ptr->orig_local[j])
	          orig_cell_ptr = 
			(pwr_sClass_NMpsMirrorCell *) cellmir_ptr->orig_cell[j];
	        else
	          orig_cell_ptr = (pwr_sClass_NMpsMirrorCell *) orig_cellbuf;
	        data_block_ptr = (plc_t_DataInfoMirCell *) &orig_cell_ptr->Data1P;
	        for ( k = 0; k < orig_cell_ptr->LastIndex; k++)
	        {
	          /* Check if the objid already is collected */
	          objid_found = 0;
	          if ( j > 0)
	          {
	            for ( l = 0; l < orig_data_count; l++)
	            {
	              if ( cdh_ObjidIsEqual( cellmir_ptr->orig_data_objid[l],
				data_block_ptr->Data_ObjId))
	              {
	                objid_found = 1;
	                break;
	              }
	            }
	          }
	          if ( !objid_found)
	          {
	            cellmir_ptr->orig_data_objid[ orig_data_count] =
		 	data_block_ptr->Data_ObjId;
	            orig_data_count++;
	          }
		  data_block_ptr++;
	        }
	        break;
	      }
	    }
 	  }
	  if ( next_cell)
	  {
	    /* Continue with the next mircell */
	    cellmir_ptr++;
	    continue;
	  }

	  if ( cellmir_ptr->restart && cellmir_ptr->conversion)
	  {
	    /* The orig cell is restarted and the objid's might be */
	    /* substituted. Loop through the orig objid and delete the old */
	    next_cell = 0;
	    errh_CErrLog( NMPS__MIRCELLRESTART, NULL);
	    for ( j = 0; j < orig_data_count; j++)
	    {
	      sts = nmpsmir_data_db_find_orig_name( mirctx->data_list,
			cellmir_ptr->orig_data_objid[j],
			cellmir_ptr->objid,
			&datalist_ptr);
	      if (sts == NMPS__NODEDOWN)
	      {
	        /* Link might be down, try later */
	        next_cell = 1;
	        break;
	      }
	      else if (ODD(sts))
	      {
	        errh_CErrLog( NMPS__DATARESTART, 
			errh_ErrArgAF(datalist_ptr->orig_data_name), NULL);
	        /* Remove from data the old objid form data db */
	        sts = gdh_DLUnrefObjectInfo( datalist_ptr->data_subid);
	        if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        if ( datalist_ptr->local)
	        {
	          sts = gdh_DLUnrefObjectInfo( datalist_ptr->orig_data_subid);
	          if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        }
	        else
	        {
	          sts = gdh_SubUnrefObjectInfo( datalist_ptr->orig_data_subid);
	          if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);
	        }
	        sts = gdh_DLUnrefObjectInfo( datalist_ptr->plc_dlid);
	        if (EVEN(sts)) Log(NMPS__MIRUNREF, sts);

	        sts = nmpsmir_data_db_delete( &mirctx->data_list, datalist_ptr);
	      }
	    }
	    if ( next_cell)
	    {
	      /* Continue with the next mircell */
	      cellmir_ptr++;
	      continue;
	    }
	  }

	  for ( j = 0; j < orig_data_count; j++)
	  {
	    /* Search for the objid in the objid database */
	    sts = nmpsmir_data_db_find_orig( mirctx->data_list,
		cellmir_ptr->orig_data_objid[j],
		cellmir_ptr->objid,
		&datalist_ptr);
	    if ( sts == NMPS__DATANOTFOUND)
	    {
	      /* Insert the object */
	      sts = nmpsmir_data_db_create( mirctx,
		  &mirctx->data_list,
		  cellmir_ptr->orig_data_objid[j],
	  	  cellmir_ptr->objid,
		  cellmir_ptr->conversion,
		  cellmir_ptr->convconfig,
		  &datalist_ptr);
	      if ( sts == NMPS__ORIGDATA)
	      {
	        /* Link might be down, try later */
	        next_cell = 1;
	        break;
	      }
	      else if ( sts == NMPS__DATAALREXIST && mirctx->first_scan)
	      {
	        /* Try to find info of data in the mircell */
	        for ( k = 0; k < cellmir_ptr->cell->TempLastIndex; k++)
	        {
	          if ( cdh_ObjidIsEqual( copyarea->datainfo[k].Data_ObjId,
				datalist_ptr->data_objid))
	          {
		    datalist_ptr->plc_data_ptr =
				(char *) copyarea->datainfo[k].DataP;
		    datalist_ptr->plc_dlid =
				copyarea->datainfo[k].Data_Dlid;
	            break;
	          }
	        }
	      }
	      else if ( EVEN(sts)) return sts;
	    }
	    cellmir_ptr->data_noinit[j] = datalist_ptr->no_init;
	    cellmir_ptr->data_objid[j] = datalist_ptr->data_objid;
	    cellmir_ptr->datalist_ptr[j] = datalist_ptr;
	    datalist_ptr->collect = cellmir_ptr->data_collect;
	    datalist_ptr->collect_cell = cellmir_ptr->collect_cell;
	    datalist_ptr->release_time = cellmir_ptr->cell->ReleaseTime;
	    datalist_ptr->removed = 0;
	  }
	  if ( next_cell)
	  {
	    /* Continue with the next mircell */
	    cellmir_ptr++;
	    continue;
	  }
	  cellmir_ptr->data_count = orig_data_count;

	  /* Detect any change since last time */
	  change_detected = 0;
	  if ( cellmir_ptr->cell->TempLastIndex == orig_data_count)
	  {
	    for ( j = 0; j < cellmir_ptr->cell->TempLastIndex; j++)
	    {
	      if ( cdh_ObjidIsNotEqual( copyarea->datainfo[j].Data_ObjId,
	     			cellmir_ptr->data_objid[j]))
	      {
	        change_detected = 1;
	        break;
	      }
	    }
	  }
	  else
	    change_detected = 1;

	  /* Find objects that has been removed */
	  for ( j = 0; j < cellmir_ptr->cell->TempLastIndex; j++)
	  {
	    found = 0;
	    for ( k = 0; k < orig_data_count; k++)
	    {
	      if ( cdh_ObjidIsEqual( copyarea->datainfo[j].Data_ObjId,
	     			cellmir_ptr->data_objid[k]))
	      {
	        found = 1;
	        break;
	      }
	    }
	    if ( !found)
	    {
	      /* This object is probably removed */
	      sts = nmpsmir_data_db_find( mirctx->data_list,
			copyarea->datainfo[j].Data_ObjId,
		        cellmir_ptr->objid,
			&datalist_ptr);
	      if ( EVEN(sts))
	      {
	        if ( cellmir_ptr->cell->InitFlag &&
		     (cellmir_ptr->cell->Function & NMPS_CELLFUNC_BACKUP) &&
	             cellmir_ptr->cell->ReloadDone &&
		     cellmir_ptr->data_collect)
	        {
	          /* Object is reloaded by backup function but removed */
	          /* Insert the object in the backup removed list */
	          sts = nmpsmir_bckremoved_store(
		  		&mirctx->bckremoved_list,
				copyarea->datainfo[j].Data_ObjId,
				&bckremlist_ptr);
	          if (ODD(sts))
	          {
	            clock_gettime( CLOCK_REALTIME, &bckremlist_ptr->remove_time);
	            bckremlist_ptr->collect_cell = cellmir_ptr->collect_cell;
	            bckremlist_ptr->release_time = cellmir_ptr->cell->ReleaseTime;
	          }
	        }
	        else
	          continue;
	      }
	      else
	      {
	        datalist_ptr->removed = 1;
	        clock_gettime( CLOCK_REALTIME, &datalist_ptr->remove_time);
	      }
	    }
	  }

	  if ( cellmir_ptr->cell->InitFlag &&
	       (cellmir_ptr->cell->Function & NMPS_CELLFUNC_BACKUP) &&
	       cellmir_ptr->cell->ReloadDone)
	  {
	    cellmir_ptr->cell->ReloadDone = 0;
	    cellmir_ptr->cell->InitFlag = 0;
	    change_detected = 1;
	  }
	  if ( cellmir_ptr->restart)
	    cellmir_ptr->restart = 0;

	  if ( cellmir_ptr->cell->InitFlag )
	  {
	    /* Dont put anything in the mircell */
	  }
	  else if ( change_detected)
	  {
	    /* Put objects into the mircell */
	    memset( copyarea, 0, sizeof(*copyarea));

	    count = 0;
	    for ( j = 0; j < orig_data_count; j++)
	    {
	      if ( count >= cellmir_ptr->cell->MaxSize)
	        break;
	      if ( !cellmir_ptr->data_noinit[j])
	      {
	        copyarea->datainfo[count].Data_ObjId = cellmir_ptr->data_objid[j];
	        copyarea->datainfo[count].DataP =
	          (pwr_tFloat32 *) cellmir_ptr->datalist_ptr[j]->plc_data_ptr;
	        copyarea->datainfo[count].Data_Dlid =
		  cellmir_ptr->datalist_ptr[j]->plc_dlid;
	        count++;
	      }
	    }
	    memcpy( cellmir_ptr->cell->TempArea, copyarea, sizeof( *copyarea));
	    cellmir_ptr->cell->TempLastIndex = count;
	    cellmir_ptr->cell->UpdateFlag = 1;
	    cellmir_ptr->cell->BackupNowMsg = 1;
	  }
	  cellmir_ptr++;
	}
	mirctx->mirrorconfig->OrigCellDownCount = mirctx->OrigCellDownCount;
	mirctx->mirrorconfig->CreateDataCount = mirctx->CreateDataCount;
	mirctx->mirrorconfig->ReconnectDataCount = mirctx->ReconnectDataCount;
	mirctx->mirrorconfig->MirrorCellInitCount = mirctx->MirrorCellInitCount;

	return NMPS__SUCCESS;
}


/****************************************************************************
* Name:		nmpsmir_free()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Free the mirror context.
*
**************************************************************************/
static pwr_tStatus	nmpsmir_free( mir_ctx mirctx)
{
	nmpsmir_t_data_list	*data_ptr;
	nmpsmir_t_data_list	*next_ptr;

	/* Free the object database */
	data_ptr = mirctx->data_list;
	gdh_SubUnrefObjectInfoAll();
	gdh_DLUnrefObjectInfoAll();
	while ( data_ptr != NULL)
	{
	  next_ptr = data_ptr->next_ptr;
	  free( data_ptr);
	  data_ptr = next_ptr;
	}

	/* Free the cellmir and convconfig lists */	
	if ( mirctx->cellmirlist)
	  free( mirctx->convconfiglist);
	if ( mirctx->cellmirlist)
	  free( mirctx->convconfiglist);

	/* Free the context */
	free( mirctx);
	return NMPS__SUCCESS;
}

int main()
{
	mir_ctx		mirctx;
	pwr_tStatus	sts;
	float		scantime;
	int		reinit_count;
	int		init_retry = 5;

	/* Init pams and gdh */
	sts = gdh_Init("rs_nmps_mirror");
	if (EVEN(sts)) LogAndExit( sts);

	for (;;)
	{
	  mirctx = calloc( 1 , sizeof( *mirctx));
	  if ( mirctx == 0 ) return NMPS__NOMEMORY;
	  mirctx->first_scan = 1;

	  sts = nmps_cellmir_init( mirctx);
	  if (EVEN(sts)) LogAndExit( sts);

	  sutl_sleep( 5.0);
	  scantime = mirctx->mirrorconfig->ScanTime;
	  for (;;)
	  {
	    if ( !mirctx->init_done)
	    {
	      reinit_count++;
	      if ( reinit_count > init_retry)
	      {
	        sts = nmps_cellmir_reinit( mirctx);
	        if (EVEN(sts)) LogAndExit( sts);
	        reinit_count = 0;
	      }
	    }

	    sts = nmpsmir_data_handler( mirctx);
	    if (EVEN(sts)) LogAndExit( sts);

	    sts = nmpsmir_cellmir_handler( mirctx);
	    if (EVEN(sts)) LogAndExit( sts);

	    mirctx->mirrorconfig->LoopCount++;

	    sutl_sleep( scantime);
	    mirctx->first_scan = 0;

	    if ( mirctx->mirrorconfig->Initialize)
	    {
	      mirctx->mirrorconfig->Initialize = 0;
	      nmpsmir_free( mirctx);
	      break;
	    }
	  }
	}
}
