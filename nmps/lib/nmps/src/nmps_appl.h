/* 
 * Proview   $Id: nmps_appl.h,v 1.1 2006-01-12 05:57:43 claes Exp $
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

#ifndef ra_nmps_appl_h
#define ra_nmps_appl_h

/* ra_nmps_appl.h -- <short description>
   NMps API.  */

#if defined __cplusplus
extern "C" {
#endif


#define NMPSAPPL_CELLIST_MAX		32	/* Max number of cells in a
						   appl_init call */
#define NMPSAPPL_APPLSESS_MAX		32	/* Max number of application
						   sessions */

#define nmpsappl_mOption_Remove		(1 << 0)
#define nmpsappl_mOption_NamePath	(1 << 1)
#define nmpsappl_mOption_ReverseOrder	(1 << 2)

typedef struct nmpsappl_s_cellist {
	pwr_tString80		name;
	pwr_tObjid		objid;
	pwr_tAddress		object_ptr;
	gdh_tDlid		subid;
	pwr_tClassId		classid;
	unsigned long		index_mask[NMPSAPPL_APPLSESS_MAX];
	int			tmp_size;
	void			*tmp_cell;
	struct nmpsappl_s_cellist *next;
	} nmpsappl_t_cellist;

typedef struct {
	pwr_tObjid		objid;
	pwr_tString80		name;
	pwr_tAddress		object_ptr;
	pwr_tBoolean		select;
	pwr_tBoolean		front;
	pwr_tBoolean		back;
	pwr_tBoolean		newdata;
	pwr_tBoolean		removed;
	unsigned long		cell_mask;
	} nmpsappl_t_datainfo;

typedef struct nmpsappl_s_ctx {
	nmpsappl_t_cellist	*cellist[NMPSAPPL_CELLIST_MAX];
	unsigned long		options;
	int			index;
	unsigned long		index_mask;
	int			cellist_count;
	int			total_cellsize;
	int			data_count;
	nmpsappl_t_datainfo	*datainfo;
	struct nmpsappl_s_ctx	*next;
	} *nmpsappl_t_ctx;

pwr_tStatus nmpsappl_MirrorInit(	pwr_tString80	*cell_array,
					unsigned long	options,
					nmpsappl_t_ctx	*ctx);

pwr_tStatus nmpsappl_Mirror( 	nmpsappl_t_ctx		applctx,
				int			*data_count,
				nmpsappl_t_datainfo	**datainfo);
pwr_tStatus nmpsappl_RemoveData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid		objid);
pwr_tStatus nmpsappl_RemoveAndDeleteData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid);
pwr_tStatus nmpsappl_SelectData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid);
pwr_tStatus	nmpsappl_TransportData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid,
			unsigned int		from_cell_mask,
			unsigned int		to_cell_mask);
pwr_tStatus	nmpsappl_InsertData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid,
			unsigned int		cell_mask);
pwr_tStatus	nmpsappl_RemoveAndKeepData(
			nmpsappl_t_ctx		applctx,
			pwr_tObjid 		objid,
			unsigned int		cell_mask);


#if defined __cplusplus
}
#endif
#endif