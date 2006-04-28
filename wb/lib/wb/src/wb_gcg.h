/* 
 * Proview   $Id: wb_gcg.h,v 1.8 2006-04-28 05:01:02 claes Exp $
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

#ifndef wb_gcg_h
#define wb_gcg_h

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef pwr_class_h
#include "pwr_class.h"
#endif

#include <stdio.h>

#include "wb_vldh.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	GCG_PRINT_ALLPAR 0
#define	GCG_PRINT_CONPAR 1
#define	GCG_PRINT_VALUE  2

#define	GCG_SPAWN 0
#define	GCG_NOSPAWN 1

#define	GCG_FLOAT 0
#define	GCG_BOOLEAN 1
#define	GCG_INT32 2
#define	GCG_STRING 3
#define	GCG_ATIME 4
#define	GCG_DTIME 5


#define	GCG_MAXFILES 6

#define	GCG_MAX_NO_TIMEBASE 150     /* Max no of timebases that the module can 
                                    * handle */
#define	GCG_MAX_NO_TIMEBASE_ELN 150 /* Max no of allowed ELN timebases. */ 
#define	GCG_MAX_NO_TIMEBASE_LYNX 150 /* Max no of allowed ELN timebases. */ 
#define	GCG_MAX_NO_TIMEBASE_LINUX 150 /* Max no of allowed LINUX timebases. */ 
#define	GCG_MAX_NO_TIMEBASE_VMS 150  /* Max no of allowed VMS timebases. */


#define	GCGM0_MAXFILES 1
#define	GCGM0_MODULE_FILE 0

#define	GCGM1_MAXFILES 6
#define	GCGM1_DECL_FILE 0
#define	GCGM1_RTDBREF_FILE 1
#define	GCGM1_RTDBREF2_FILE 2
#define	GCGM1_REF_FILE 3
#define	GCGM1_CODE_FILE 4
#define	GCGM1_MODULE_FILE 5

#define GCG_PREFIX_IOW 'W'
#define GCG_PREFIX_REF 'Z'
#define GCG_PREFIX_IOC 'Y'
#define GCG_PREFIX_IOCW 'X'
#define GCG_PREFIX_MOD 'M'
#define GCG_PREFIX_PROC	'P'

#define GCG_REFTYPE_REF 0
#define GCG_REFTYPE_IOR 1
#define GCG_REFTYPE_IOW 2
#define GCG_REFTYPE_IOC 3
#define GCG_REFTYPE_IOCW 4

#define GCG_OTYPE_OID 0
#define GCG_OTYPE_AREF 1

#define GCG_PROC 0
#define GCG_PLC 1
#define GCG_WIND 2 
#define GCG_RTNODE 3
#define GCG_LIBRARY 4

#define IF_PR if ( gcgctx->print )

typedef	FILE	*gcg_t_files[ GCG_MAXFILES ];

typedef	union	{
    int		bo;
    float	fl;
    char        str[80];
    pwr_tTime	atime;
    pwr_tDeltaTime dtime;
} gcg_t_nocondef;

typedef struct {
	pwr_tObjid	objdid;
	pwr_tObjid	thread;
	unsigned long	executeorder;
	pwr_tOName     	name;
	} gcg_t_plclist;

typedef struct {
	pwr_tObjid	objdid;
	float		scantime;
	unsigned long	prio;
	char		name[120];
	} gcg_t_threadlist;

typedef struct {
	pwr_tObjid	objdid;
	char		prefix;
	} gcg_t_reflist;

typedef struct {
	pwr_sAttrRef	attrref;
	char		prefix;
	} gcg_t_areflist;

typedef struct {
	vldh_t_wind	wind;
	vldh_t_node	node;
	char		filenames[ GCG_MAXFILES ][80];
	gcg_t_files	files;
	gcg_t_areflist 	*ioread;
	unsigned long	ioreadcount;
	gcg_t_areflist	*iowrite;
	unsigned long	iowritecount;
	gcg_t_reflist	*ref;
	unsigned long	refcount;
	gcg_t_areflist	*aref;
	unsigned long	arefcount;
	unsigned long	compobjcount;
	pwr_sAttrRef	reset_object;
	unsigned long	reset_checked;
	unsigned long	errorcount;
	unsigned long	warningcount;
	unsigned long	order_comp;  /* compilation of an order is going on */
	unsigned long	step_comp;   /* compilation of a step is going on */
	unsigned long	print;	     /* if true code generation, else syntax
					control only */
	ldh_tSesContext ldhses;
  	vldh_t_node	current_cmanager;
  	int		cmanager_active;
	} gcg_t_ctx, *gcg_ctx;


int	gcg_comp_rtnode( 
    char	    *nodename,
    pwr_mOpSys 	    os,
    pwr_tUInt32	    bus,
    unsigned long   codetype,
    unsigned long   *errorcount,
    unsigned long   *warningcount,
    int		    debug,
    pwr_tVolumeId   *volumelist,
    int		    volume_count,
    unsigned long   plc_version,
    pwr_tFloat32    single_scantime
);

int gcg_executeorder_nodes ( 
  unsigned long		node_count,
  vldh_t_node		*nodelist
);

int gcg_get_connected_parameter (
  vldh_t_node	node,
  unsigned long	point,
  vldh_t_node	*conn_node,
  char		*conn_obj,
  char		*conn_par
);

int gcg_get_conpoint_nodes (
  vldh_t_node	  node,
  unsigned long	  point,
  unsigned long	  *point_count,
  vldh_t_conpoint **pointlist,
  unsigned long	  conmask
);

int gcg_get_debug (
  vldh_t_node	node,
  char		*debug_parname,
  char		*conn_obj,
  char		*conn_par,
  pwr_eType	*par_type);

int gcg_get_debug_virtual (
  vldh_t_node	node,
  char		*debug_parname,
  char		*conn_obj,
  char		*conn_par,
  pwr_eType	*par_type,
  int		*par_inverted);

int gcg_get_inputpoint (
  vldh_t_node	node,
  unsigned long	index,
  unsigned long	*pointptr,
  unsigned long	*inverted
);

int gcg_get_output (
  vldh_t_node	node,
  unsigned long	point,
  unsigned long	*output_count,
  vldh_t_node	*output_node,
  unsigned long	*output_point,
  ldh_sParDef 	*output_bodydef,
  unsigned long	conmask
);

int gcg_get_outputstring ( 
  gcg_ctx	gcgctx,
  vldh_t_node	output_node,
  ldh_sParDef 	*output_bodydef,
  pwr_sAttrRef	*parattrref,
  int		*partype,
  char		*parprefix,
  char		*parstring
);

int gcg_wind_msg( 
    gcg_ctx		gcgctx,
    unsigned long 	sts,
    vldh_t_wind	wind
);

int gcg_get_point ( 
  vldh_t_node	node,
  unsigned long	index,
  unsigned long	*pointptr,
  unsigned long	*inverted
);

int gcg_plc_compile ( 
  vldh_t_plc	plc,
  unsigned long	codetype,
  unsigned long	*errorcount,
  unsigned long	*warningcount,
  unsigned long	spawn,
  int		debug
);

int gcg_plcpgm_to_operating_system ( 
  ldh_tSesContext ldhses, 
  pwr_tObjid	plcpgm, 
  pwr_mOpSys	*os
);

int gcg_plcwindow_compile ( 
  vldh_t_wind	wind,
  unsigned long	codetype,
  unsigned long	*errorcount,
  unsigned long	*warningcount,
  unsigned long	spawn,
  int		debug
);

int	gcg_print_inputs( 
    gcg_ctx		gcgctx,
    vldh_t_node		node,
    char		*delimstr,
    unsigned long	printtype,
    gcg_t_nocondef	*nocondef,
    unsigned long	*nocontype
);


int gcg_wind_check_modification (
  ldh_tSesContext ldhses,
  pwr_tObjid	objdid
);

int gcg_wind_to_operating_system ( 
  ldh_tSesContext ldhses, 
  pwr_tObjid	wind, 
  pwr_mOpSys	*os
);

int gcg_wind_comp_all( 
  ldh_tWBContext  ldhwb,
  ldh_tSesContext ldhses,
  pwr_tObjid	window,
  unsigned long	codetype,
  int		modified,
  int		debug
);

int	gcg_comp_volume( 
    ldh_tSesContext ldhses
);

#ifdef __cplusplus
}
#endif

#endif
