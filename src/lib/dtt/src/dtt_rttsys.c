/************************************************************************
*************************************************************************
*
*		 P S S - 9 0 0 0
*		=================
*************************************************************************
*
* Filename:		ds_rtt_appl_function.c
*			Date	Pgm.	Read.	Remark
* Modified		920102	CS		Initial creation
*
*
*
* Description:
*	Example of an application function in rtt.
*
**************************************************************************
**************************************************************************/

/*_Include files_________________________________________________________*/

#ifdef OS_VMS
#include <descrip.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <starlet.h>
#endif

#if defined OS_LYNX || defined OS_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif

#ifdef	OS_ELN
#include descrip
#include stdio
#include stdlib
#include ctype
#include starlet
#include $vaxelnc
#endif

#include "rt_gdb.h"
#include "rt_qdb.h"
#include "rt_hash.h"
#include "rt_pool.h"
#include "rt_net.h"
#include "rt_sub.h"
#include "rt_dvms.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_io_base.h"
#include "rt_rtt.h"
#include "rt_rtt_edit.h"
#include "rt_rtt_menu.h"
#include "rt_rtt_functions.h"
#include "rt_rtt_msg.h"
#include "dtt_rttsys_functions.h"
#include "dtt_appl_rttsys_m.rh"
#include "dtt_appl_rttsys_m.rdecl"
#include "pwr_baseclasses.h"
#include "pwr_nmpsclasses.h"
#include "pwr_ssabclasses.h"

/*_Local rtt database____________________________________________________*/
RTT_RTTSYSDB_START
#include "dtt_appl_rttsys_m.rdb1"
RTT_RTTSYSDB_CONTINUE
#include "dtt_appl_rttsys_m.rdb2"
RTT_RTTSYSDB_END

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define IF_NOGDH_RETURN \
if ( !rtt_gdh_started)\
{\
rtt_message('E',"Rtt is not connected to nethandler");\
return RTT__NOPICTURE;\
}

/* Type definitions */
/* RTTSYS_GRAFCET ***/
typedef struct {
	pwr_tObjid	objid;
	char		name[80];
	pwr_tBoolean	*value_ptr;
	gdh_tDlid	subid;
	} rttsys_t_step_list;

typedef struct {
	pwr_tObjid		objid;
	char			name[80];
	char			selected;
	rttsys_t_step_list	*initsteps;
	int			initstep_count;
	int			initstep_alloc;
	rttsys_t_step_list	*steps;
	int			step_count;
	int			step_alloc;
	rttsys_t_step_list	*orders;
	int			order_count;
	int			order_alloc;
	} rttsys_t_plcpgm_list;

/* RTTSYS_NMPSCELL ***/
typedef struct {
	pwr_tObjid	objid;
	pwr_tClassId	class;
	char		name[80];
	char		*value_ptr;
	gdh_tDlid	subid;
	int		expand;
	} rttsys_t_cell_list;

typedef struct {
	pwr_tFloat32	*DataP;
	pwr_tObjid	Data_ObjId;
	pwr_tBoolean	Data_Front;
	pwr_tBoolean	Data_Back;
	gdh_tDlid	Data_Dlid;	
	pwr_tBoolean	Data_Select;
	pwr_tBoolean	Data_OldSelect;
	} plc_t_DataInfo;

typedef struct {
	pwr_tFloat32	*DataP;
	pwr_tObjid	Data_ObjId;
	gdh_tDlid	Data_Dlid;	
	} plc_t_DataInfoMirCell;

typedef struct {
	pwr_tObjid	chan_objid;
	char		channame[80];
	int		conv_on;
	pwr_tBoolean	*conversion_on;
	pwr_tBoolean	*invert_on;
	pwr_tBoolean	*test_on;
	pwr_tBoolean	*test_value;
	char		*chan_ptr;
	gdh_tDlid	chan_subid;
	int		connected;
	char		signame[80];
	pwr_tBoolean	*value_ptr;
	gdh_tDlid	value_subid;
	int		chan_number;
	char		sig_descript[80];
	char		chan_descript[80];
	char		chan_ident[80];
	} rttsys_t_chan_list;

typedef struct {
	pwr_tObjid	device_objid;
	pwr_tClassId	device_class;
	pwr_tObjid	device_class_objid;
	char		*device_ptr;
	char		devicename[80];
	pwr_tUInt32	*error_count;
	pwr_tUInt32	*process;
	pwr_tObjid	*thread_object;
	pwr_tUInt32	*reg_address;
	gdh_tDlid	device_subid;
	} rttsys_t_device_list;

typedef struct {
	pwr_tObjid	remnode_objid;
	pwr_sClass_RemNode *remnode_ptr;
	char		remnodename[80];
	gdh_tDlid	remnode_subid;
	} rttsys_t_remnode_list;

typedef struct {
	pwr_tObjid	remtrans_objid;
	pwr_sClass_RemTrans *remtrans_ptr;
	char		remtransname[80];
	gdh_tDlid	remtrans_subid;
	} rttsys_t_remtrans_list;

typedef struct {
	pwr_tObjid	runningtime_objid;
	pwr_sClass_RunningTime *runningtime_ptr;
	char		runningtimename[80];
	gdh_tDlid	runningtime_subid;
	} rttsys_t_runningtime_list;


#ifdef OS_ELN
typedef struct {
	int	low;
	int	high;
} t_VAXTime;
#endif

/* Static variables ****/
  static  rttsys_t_plcpgm_list	*plclist;
  static  int			plclist_count;
  static int			grafcet_page;	
  static int			logging_page = 0;

/*_Local function prototypes____________________________________________________*/

static int	rttsys_get_nodename( pwr_tNodeId nid, char *nodename);
static int rttsys_plclist_bubblesort( 	rttsys_t_plcpgm_list	*plclist,
					int			size);
static int rttsys_steplist_bubblesort( 	rttsys_t_step_list	*steplist,
					int			size);
static int	rttsys_get_plcpgm( 	pwr_tObjid	initstep_objid,
					pwr_tObjid	*plc_objid);
static int	rttsys_plclist_add( 	pwr_tObjid		plc_objid,
					rttsys_t_plcpgm_list	**plclist,
					int			*plclist_count,
					int			*alloc);
static int rttsys_initsteplist_add( pwr_tObjid		initstep_objid,
				rttsys_t_step_list	**initsteplist,
				int			*initsteplist_count);
static int	rttsys_grafcet_select( 	menu_ctx	ctx,
					pwr_tObjid	dummyoi,
					void		*dummy1,
					void		*dummy2,
					void		*dummy3,
					void		*dummy4);
static int rttsys_steplist_add(	pwr_tObjid		step_objid,
				rttsys_t_step_list	**steplist,
				int			*steplist_count,
				int			*alloc,
				pwr_tClassId		class,
				int			dummy2);
static int rttsys_orderlist_add( pwr_tObjid		order_objid,
				rttsys_t_step_list	**orderlist,
				int			*orderlist_count,
				int			*alloc,
				pwr_tClassId		class,
				int			dummy2);
static int rttsys_objectlist_add( pwr_tObjid		object_objid,
				pwr_tClassId		class,
				rttsys_t_step_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				char			*attrstr,
				int			attrsize);
static int rttsys_object_add( 	pwr_tClassId		class,
				rttsys_t_step_list	**objectlist,
				int			*objectlist_count,
				int			*object_alloc,
				char			*attrstr,
				int			attrsize);
static int rttsys_threadobject_add( pwr_tClassId	class,
				rttsys_t_step_list	**objectlist,
				int			*objectlist_count,
				int			*object_alloc,
				char			*attrstr,
				int			attrsize);
static int rttsys_thread_update( menu_ctx		ctx,
				rttsys_t_step_list	*objectlist,
				int			objectlist_count,
				int			page);
static int rttsys_objectlist_modname( 	rttsys_t_step_list	*objectlist,
				int			objectlist_count);
static int rttsys_pid_object_start( 	menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4);
static int rttsys_pid_update( 	menu_ctx		ctx,
				rttsys_t_step_list	*objectlist,
				int			objectlist_count,
				int			page);
static int rttsys_objectlist_modname_plc( rttsys_t_step_list	*objectlist,
				int			objectlist_count);
static int rttsys_pidobject_add( pwr_tClassId		class,
				rttsys_t_step_list	**objectlist,
				int			*objectlist_count,
				int			*object_alloc,
				char			*attrstr,
				int			attrsize);
#ifdef OS_ELN
static int rttsys_cpu( int key, pwr_tTime *time, float cputim, 
		float *cpu_current, float *cpu_mean, int init);
static int rttsys_proc_cpu( int key, pwr_tTime *time, float cputim, 
		float *cpu_current, float *cpu_mean, int init);
#endif

static int	rttsys_cell_object_start( 	menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4);
static int rttsys_cellist_add( 	pwr_tObjid		object_objid,
				pwr_tClassId		class,
				rttsys_t_cell_list	**objectlist,
				int			*objectlist_count,
				int			*alloc);
static int	rttsys_cell_add( pwr_tClassId		class,
				rttsys_t_cell_list	**objectlist,
				int			*objectlist_count,
				int			*object_alloc);
static int	rttsys_cell_expand( 	menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4);
static int	rttsys_cell_dataobject( menu_ctx	ctx, 
				pwr_tObjid	cell_objid,
				pwr_tObjid	*data_objid,
				void		*arg2,
				void		*arg3,
				void		*arg4);
static int rttsys_dichanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local);
static int rttsys_get_conversion( pwr_tUInt16	convmask1, 
				pwr_tUInt16	convmask2, 
				int		chan_number,
				int		*conversion);
static int rttsys_chan_start( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4);
static int rttsys_dochanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local);
static int rttsys_aichanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local);
static int rttsys_aochanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local,
				int			signal_test_mode);
static int rttsys_cochanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local);
static int rttsys_remtrans_start( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4);
static int rttsys_remtrans_buffer( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4);
static int rttsys_remnodelist_add( pwr_tObjid		remnode_objid,
				rttsys_t_remnode_list	**objectlist,
				int			*objectlist_count,
				int			*alloc);
static int rttsys_remtranslist_add( pwr_tObjid		remtrans_objid,
				rttsys_t_remtrans_list	**objectlist,
				int			*objectlist_count,
				int			*alloc);
static int rttsys_runningtimelist_add( pwr_tObjid		runningtime_objid,
				rttsys_t_runningtime_list	**objectlist,
				int			*objectlist_count,
				int			*alloc);

#if 0
static int rttsys_node_start( 	menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4);
#endif

int RTTSYS_REMTRANS( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture);
int RTTSYS_RUNNINGTIME( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture);
int RTTSYS_NMPSCELL();
int RTTSYS_OBJECT_CELL();
int rttsys_cell_dataobject();
int RTTSYS_QCOM_APPL( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture);
int RTTSYS_QCOM_QUEUE( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture);
static int rttsys_qcom_queue_start( 	menu_ctx	ctx, 
					pwr_tObjid	objid,
					void		*arg1,
					void		*arg2,
					void		*arg3,
					void		*arg4);
int RTTSYS_QCOM_NODES( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture);
static int rttsys_pool_segs_start( 	menu_ctx	ctx, 
					pwr_tObjid	objid,
					void		*arg1,
					void		*arg2,
					void		*arg3,
					void		*arg4);
int RTTSYS_POOL_SEGS( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture);
static int rttsys_pool_segment_start( 	menu_ctx	ctx, 
					pwr_tObjid	objid,
					void		*arg1,
					void		*arg2,
					void		*arg3,
					void		*arg4);
int RTTSYS_POOL_SEGMENT( menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture);
static int rttsys_qcom_node_start( 	menu_ctx	ctx, 
					pwr_tObjid	objid,
					void		*arg1,
					void		*arg2,
					void		*arg3,
					void		*arg4);
int RTTSYS_QCOM_NODE( menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture);


#define RTTSYS_ALLOC 50
#define RTTSYS_CHANALLOC 32
#define RTTSYS_PLCALLOC 20
#define POOL_SSIZE 8

/*************************************************************************
*
* Name:		RTTSYS_SHOW_SUBSRV()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show nethandler subsrv.
*
**************************************************************************/

static int	rttsys_get_nodename( pwr_tNodeId nid, char *nodename)
{
	gdb_sNode 		*np;
	int			sts;

	np = hash_Search( &sts, gdbroot->nid_ht, &nid);
	if (ODD(sts)) 
	  strcpy( nodename, np->name);
	else
	  strcpy( nodename, "Undef");
	return sts;

#if 0
	NODE_STRUCT 		*nodep;
	int			i,found;

	found = 0;
	for ( i = 0; i < 256; i++)
	{
          nodep = &gdhroot->nodedb->node [i];
          if ( nodidx == nodep->nid )
	  {
             strcpy( nodename, nodep->nodename);
	     found = 1;	
	     break;
	  }
	}
	if ( !found)
	  strcpy( nodename, "Undef");
#endif
}

int RTTSYS_SHOW_SUBSRV( menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
#define SHOW_SUBSRV_PAGESIZE 16
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static int		page;	
  int			i, k, l;
  pool_sQlink		*sl;
  sub_sServer	 	*ssrvp;
/*  char			astr[80];*/
	  
  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;
      gdb_ScopeLock {
        for (
	  sl = pool_Qsucc(&sts, gdbroot->pool, &gdbroot->db->subs_lh);
	  sl != &gdbroot->db->subs_lh;
	  sl = pool_Qsucc(&sts, gdbroot->pool, sl))
	{
	  ssrvp = pool_Qitem(sl, sub_sServer, subs_ll);

          if ( (k >= page * SHOW_SUBSRV_PAGESIZE) && (k < (page + 1) * SHOW_SUBSRV_PAGESIZE))
          {
            /* Subid */
            menu_ptr->value_ptr = (char *) &ssrvp->sid;
            menu_ptr++;
            menu_ptr->value_ptr = (char *) &ssrvp->count;
            menu_ptr++;
	    rttsys_get_nodename( ssrvp->nid, menu_ptr->value_ptr);
            menu_ptr++;
            menu_ptr->value_ptr = (char *) &ssrvp->aref.Size;
            menu_ptr++;
            menu_ptr->value_ptr = (char *) &ssrvp->aref.Offset;
            menu_ptr++;
            menu_ptr->value_ptr = (char *) &ssrvp->aref;
            menu_ptr++;
/********
            if (ssrvp->aref.Flags.b.Indirect) 
	      strcpy( astr, "@");
            else
	      strcpy( astr, "");
	    cdh_ArefToString( astr, &ssrvp->aref, 1);
	    strcpy( menu_ptr->value_ptr, astr);
            menu_ptr++;
**********/

	    l++;
          }
          k++;
        }
      } gdb_ScopeUnlock;

      for ( i = l; i < SHOW_SUBSRV_PAGESIZE; i++)
      {
        /* subid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      SHOW_SUBSRV_MAXPAGE = max( 1, (k - 1)/ SHOW_SUBSRV_PAGESIZE + 1);

      SHOW_SUBSRV_COUNTER = k;

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p26_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p26_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_INIT:
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      if ( event == RTT_APPL_PREVPAGE)
      {
        page--;
        page = max( page, 0);
      }
      else if ( event == RTT_APPL_NEXTPAGE)
      {
        page++;
        page = min( page, SHOW_SUBSRV_MAXPAGE - 1);
      }
      else if ( event == RTT_APPL_INIT)
        page = 0;

      SHOW_SUBSRV_PAGE = page + 1;
      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;
      gdb_ScopeLock {
        for (
	  sl = pool_Qsucc(&sts, gdbroot->pool, &gdbroot->db->subs_lh);
	  sl != &gdbroot->db->subs_lh;
	  sl = pool_Qsucc(&sts, gdbroot->pool, sl)) 
	{
	  ssrvp = pool_Qitem(sl, sub_sServer, subs_ll);

          if ( (k >= page * SHOW_SUBSRV_PAGESIZE) && (k < (page + 1) * SHOW_SUBSRV_PAGESIZE))
          {
            /* Subid */
            menu_ptr->value_ptr = (char *) &ssrvp->sid;
            menu_ptr++;
            menu_ptr->value_ptr = (char *) &ssrvp->count;
            menu_ptr++;
	    rttsys_get_nodename( ssrvp->nid, menu_ptr->value_ptr);
            menu_ptr++;
            menu_ptr->value_ptr = (char *) &ssrvp->aref.Offset;
            menu_ptr++;
            menu_ptr->value_ptr = (char *) &ssrvp->aref.Size;
            menu_ptr++;
            menu_ptr->value_ptr = (char *) &ssrvp->aref;
            menu_ptr++;
/******
            if (ssrvp->aref.Flags.b.Indirect) 
	      strcpy( astr, "@");
            else
	      strcpy( astr, "");
	    cdh_ArefToString( astr, &ssrvp->aref, 1);
	    strcpy( menu_ptr->value_ptr, astr);
            menu_ptr++;
*****/

	    l++;
          }
          k++;
        }
      } gdb_ScopeUnlock;

      for ( i = l; i < SHOW_SUBSRV_PAGESIZE; i++)
      {
        /* subid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      SHOW_SUBSRV_MAXPAGE = max( 1, (k - 1)/ SHOW_SUBSRV_PAGESIZE + 1);

      SHOW_SUBSRV_COUNTER = k;

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_SHOW_SUBCLI()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show nethandler subcli.
*
**************************************************************************/

int RTTSYS_SHOW_SUBCLI( menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

#define SHOW_SUBCLI_PAGESIZE 16
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static int		page;	
  gdb_sNode 		*np;
  int			i, k, l;
  pool_sQlink		*nl;
  pool_sQlink		*sl;
  sub_sClient 		*sclip;
  char 			timbuf [11+1];
  net_sSubData 		*sdatap;
  int			remote_cnt;
  char			astr[80];

  IF_NOGDH_RETURN;


  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;
      remote_cnt = 0;
      gdb_ScopeLock {
        for (
	  nl = pool_Qsucc(&sts, gdbroot->pool, &gdbroot->db->nod_lh);
	  nl != &gdbroot->db->nod_lh;
	  nl = pool_Qsucc(&sts, gdbroot->pool, nl)) 
	{
	  np = pool_Qitem(nl, gdb_sNode, nod_ll);
          for (
	    sl = pool_Qsucc(&sts, gdbroot->pool, &np->subc_lh);
	    sl != &np->subc_lh;
	    sl = pool_Qsucc(&sts, gdbroot->pool, sl))
	  {
	    sclip = pool_Qitem(sl, sub_sClient, subc_ll);

            if ( (k >= page * SHOW_SUBCLI_PAGESIZE) && (k < (page + 1) * SHOW_SUBCLI_PAGESIZE))
            {
              /* Subid */
              menu_ptr->value_ptr = (char *) &sclip->sid;
              menu_ptr++;
              time_AtoAscii( &sclip->lastupdate, time_eFormat_Time, 
		timbuf, sizeof(timbuf));
	      strcpy( menu_ptr->value_ptr, timbuf);
              menu_ptr++;
              menu_ptr->value_ptr = (char *) &sclip->count;
              menu_ptr++;
	      if ( sclip->nid == pwr_cNNodeId)
	        strcpy( menu_ptr->value_ptr, "Unknwn");
	      else
	        strcpy( menu_ptr->value_ptr, np->name);
              menu_ptr++;
              sdatap = pool_Address( &sts, gdbroot->pool, sclip->subdata);
              if (sdatap == NULL)
                menu_ptr->value_ptr = (char *) RTT_ERASE;
	      else
	        menu_ptr->value_ptr = (char *) &sdatap->size;
              menu_ptr++;
	      if (sclip->sub_by_name)
	        strcpy( menu_ptr->value_ptr, sclip->name);
              else 
	      {
	        if ( sclip->aref.Flags.b.Indirect)
	          strcpy( astr, "@");
	        else
	          strcpy( astr, "");
	        cdh_ArefToString( astr, &sclip->aref, 1);
	        strcpy( menu_ptr->value_ptr, astr);
	      }
              menu_ptr++;
	      l++;
            }
            k++;
          }
	  if ( !(np == gdbroot->my_node || np == gdbroot->no_node))
	    remote_cnt += np->subc_lc;
        }
      } gdb_ScopeUnlock;

      for ( i = l; i < SHOW_SUBCLI_PAGESIZE; i++)
      {
        /* subid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
      }
      SHOW_SUBCLI_MAXPAGE = max( 1, (k - 1)/ SHOW_SUBCLI_PAGESIZE + 1);

      SHOW_SUBCLI_LOCAL = 
	gdbroot->my_node->subc_lc;
      SHOW_SUBCLI_UNKNOWN = 
	gdbroot->no_node->subc_lc;
      SHOW_SUBCLI_REMOTE = remote_cnt;

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p25_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p25_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_INIT:
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      if ( event == RTT_APPL_PREVPAGE)
      {
        page--;
        page = max( page, 0);
      }
      else if ( event == RTT_APPL_NEXTPAGE)
      {
        page++;
        page = min( page, SHOW_SUBCLI_MAXPAGE - 1);
      }
      else if ( event == RTT_APPL_INIT)
        page = 0;

      SHOW_SUBCLI_PAGE = page + 1;
      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;
      gdb_ScopeLock {
        for (
	  nl = pool_Qsucc(&sts, gdbroot->pool, &gdbroot->db->nod_lh);
	  nl != &gdbroot->db->nod_lh;
	  nl = pool_Qsucc(&sts, gdbroot->pool, nl)) 
	{
	  np = pool_Qitem(nl, gdb_sNode, nod_ll);
          for (
	    sl = pool_Qsucc(&sts, gdbroot->pool, &np->subc_lh);
	    sl != &np->subc_lh;
	    sl = pool_Qsucc(&sts, gdbroot->pool, sl))
	  {
	    sclip = pool_Qitem(sl, sub_sClient, subc_ll);

            if ( (k >= page * SHOW_SUBCLI_PAGESIZE) && (k < (page + 1) * SHOW_SUBCLI_PAGESIZE))
            {
              /* Subid */
              menu_ptr->value_ptr = (char *) &sclip->sid;
              menu_ptr++;
              time_AtoAscii( &sclip->lastupdate, time_eFormat_Time, 
		timbuf, sizeof(timbuf));
	      strcpy( menu_ptr->value_ptr, timbuf);
              menu_ptr++;
              menu_ptr->value_ptr = (char *) &sclip->count;
              menu_ptr++;
	      if ( sclip->nid == pwr_cNNodeId)
	        strcpy( menu_ptr->value_ptr, "Unknwn");
	      else
	        strcpy( menu_ptr->value_ptr, np->name);
              menu_ptr++;
              sdatap = pool_Address( &sts, gdbroot->pool, sclip->subdata);
              if (sdatap == NULL)
                menu_ptr->value_ptr = (char *) RTT_ERASE;
	      else
	        menu_ptr->value_ptr = (char *) &sdatap->size;
              menu_ptr++;
	      if (sclip->sub_by_name)
	        strcpy( menu_ptr->value_ptr, sclip->name);
              else 
	      {
	        if ( sclip->aref.Flags.b.Indirect)
	          strcpy( astr, "@");
	        else
	          strcpy( astr, "");
	        cdh_ArefToString( astr, &sclip->aref, 1);
	        strcpy( menu_ptr->value_ptr, astr);
	      }
              menu_ptr++;
	      l++;
            }
            k++;
          }
        }
      } gdb_ScopeUnlock;

      for ( i = l; i < SHOW_SUBCLI_PAGESIZE; i++)
      {
        /* subid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
      }
      SHOW_SUBCLI_MAXPAGE = max( 1, (k - 1)/ SHOW_SUBCLI_PAGESIZE + 1);

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_SHOW_NODES()
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show nethandler info.
*
**************************************************************************/

int RTTSYS_SHOW_NODES( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
#define SHOW_NODES_PAGESIZE 10
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static int		page;	
  int			i, j, k, l;
  pool_sQlink		*nl;
  gdb_sNode		*np;
  char 			timbuf[32];

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;

      gdb_ScopeLock {
        for (
	  nl = pool_Qsucc(&sts, gdbroot->pool, &gdbroot->db->nod_lh);
	  nl != &gdbroot->db->nod_lh;
	  nl = pool_Qsucc(&sts, gdbroot->pool, nl)
        ) 
        {
          if ( (k >= page * SHOW_NODES_PAGESIZE) && (k < (page + 1) * SHOW_NODES_PAGESIZE))
          {
	    np = pool_Qitem(nl, gdb_sNode, nod_ll);

	    if ( strcmp( np->name, "******") == 0)
	      continue;

            /* Name */
            menu_ptr->value_ptr = (char *) &np->name;
            menu_ptr++;
            /* Os */
	    switch ( np->os) {
	      case co_eOS_Lynx: strcpy( menu_ptr->value_ptr, "Lynx"); break;
	      case co_eOS_Linux: strcpy( menu_ptr->value_ptr, "Linux"); break;
	      case co_eOS_VMS: strcpy( menu_ptr->value_ptr, "VMS"); break;
	      case co_eOS_ELN: strcpy( menu_ptr->value_ptr, "ELN"); break;
	      default: strcpy( menu_ptr->value_ptr, "Unknwn");
	    }
            menu_ptr++;
            /* Hw */
	    switch ( np->hw) {
	      case co_eHW_x86: strcpy( menu_ptr->value_ptr, "x86"); break;
	      case co_eHW_68k: strcpy( menu_ptr->value_ptr, "68k"); break;
	      case co_eHW_VAX: strcpy( menu_ptr->value_ptr, "VAX"); break;
	      case co_eHW_Alpha: strcpy( menu_ptr->value_ptr, "AXP"); break;
	      case co_eHW_PPC: strcpy( menu_ptr->value_ptr, "PPC"); break;
	      default: strcpy( menu_ptr->value_ptr, "Unknwn");
	    }
            menu_ptr++;
            /* Link */
            if (np == gdbroot->my_node)
	      /* Local node */
              strcpy( menu_ptr->value_ptr, "Local");
	    else
	    {
              if ( np->flags.b.up)
                strcpy( menu_ptr->value_ptr, "Up");
              else if ( np->flags.b.active)
                strcpy( menu_ptr->value_ptr, "Active");
              else if ( np->flags.b.connected)
                strcpy( menu_ptr->value_ptr, "Connected");
              else
                strcpy( menu_ptr->value_ptr, "Down");
	    }
            menu_ptr++;
            /* Upcnt */
            menu_ptr->value_ptr = (char *) &np->upcnt;
            menu_ptr++;
  	    /* Timeup */
	    if ( np->timeup.tv_sec != 0 || np->timeup.tv_nsec != 0)
	    {
              time_AtoAscii( &np->timeup, time_eFormat_DateAndTime, timbuf,
		sizeof(timbuf));
	    }
	    else
              timbuf [0] = '\0';
            strcpy( menu_ptr->value_ptr, timbuf);
            menu_ptr++;
            /* Sent */
	    * (pwr_tInt32 *) (menu_ptr->value_ptr) = 0;
            for (j=0; j<net_eMsg_; j++) 
            {
	      *(pwr_tInt32 *) (menu_ptr->value_ptr) += np->txmsg[j];
            }
            menu_ptr++;
            /* Rcvd */
	    * (pwr_tInt32 *) (menu_ptr->value_ptr) = 0;
            for (j=0; j<net_eMsg_; j++) 
            {
	      * (pwr_tInt32 *) (menu_ptr->value_ptr) += np->rxmsg[j];
            }
            menu_ptr++;
  	    l++;
          }
          k++;
        }
      } gdb_ScopeUnlock;

      for ( i = l; i < SHOW_NODES_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Os */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Hw */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Link */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Upcnt */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Timeup */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Sent */
        * (pwr_tInt32 *) (menu_ptr->value_ptr) = 0;
        menu_ptr++;
        /* Rcvd */
        * (pwr_tInt32 *) (menu_ptr->value_ptr) = 0;
        menu_ptr++;
      }
      SHOW_NODES_MAXPAGE = max( 1, (k - 1)/ SHOW_NODES_PAGESIZE + 1);

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p4_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p4_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_INIT:
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      if ( event == RTT_APPL_PREVPAGE)
      {
        page--;
        page = max( page, 0);
      }
      else if ( event == RTT_APPL_NEXTPAGE)
      {
        page++;
        page = min( page, SHOW_NODES_MAXPAGE - 1);
      }
      else if ( event == RTT_APPL_INIT)
        page = 0;

      SHOW_NODES_PAGE = page + 1;

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;

      gdb_ScopeLock {
        for (
	  nl = pool_Qsucc(&sts, gdbroot->pool, &gdbroot->db->nod_lh);
	  nl != &gdbroot->db->nod_lh;
	  nl = pool_Qsucc(&sts, gdbroot->pool, nl)
        ) 
        {
          if ( (k >= page * SHOW_NODES_PAGESIZE) && (k < (page + 1) * SHOW_NODES_PAGESIZE))
          {
	    np = pool_Qitem(nl, gdb_sNode, nod_ll);

            /* Name */
            menu_ptr->value_ptr = (char *) &np->name;
            menu_ptr++;
            /* Os */
	    switch ( np->os) {
	      case co_eOS_Lynx: strcpy( menu_ptr->value_ptr, "Lynx"); break;
	      case co_eOS_Linux: strcpy( menu_ptr->value_ptr, "Linux"); break;
	      case co_eOS_VMS: strcpy( menu_ptr->value_ptr, "VMS"); break;
	      case co_eOS_ELN: strcpy( menu_ptr->value_ptr, "ELN"); break;
	      default: strcpy( menu_ptr->value_ptr, "Unknwn");
	    }
            menu_ptr++;
            /* Hw */
	    switch ( np->hw) {
	      case co_eHW_x86: strcpy( menu_ptr->value_ptr, "x86"); break;
	      case co_eHW_68k: strcpy( menu_ptr->value_ptr, "68k"); break;
	      case co_eHW_VAX: strcpy( menu_ptr->value_ptr, "VAX"); break;
	      case co_eHW_Alpha: strcpy( menu_ptr->value_ptr, "AXP"); break;
	      case co_eHW_PPC: strcpy( menu_ptr->value_ptr, "PPC"); break;
	      default: strcpy( menu_ptr->value_ptr, "Unknwn");
	    }
            menu_ptr++;
            /* Link */
            if (np == gdbroot->my_node)
	      /* Local node */
              strcpy( menu_ptr->value_ptr, "Local");
	    else
	    {
              if ( np->flags.b.up)
                strcpy( menu_ptr->value_ptr, "Up");
              else if ( np->flags.b.active)
                strcpy( menu_ptr->value_ptr, "Active");
              else if ( np->flags.b.connected)
                strcpy( menu_ptr->value_ptr, "Connected");
              else
                strcpy( menu_ptr->value_ptr, "Down");
	    }
            menu_ptr++;
            /* Upcnt */
            menu_ptr->value_ptr = (char *) &np->upcnt;
            menu_ptr++;
  	    /* Timeup */
	    if ( np->timeup.tv_sec != 0 || np->timeup.tv_nsec != 0)
	    {
              time_AtoAscii( &np->timeup, time_eFormat_DateAndTime, timbuf,
		sizeof(timbuf));
	    }
	    else
              timbuf [0] = '\0';
            strcpy( menu_ptr->value_ptr, timbuf);
            menu_ptr++;
            /* Sent */
	    * (pwr_tInt32 *) (menu_ptr->value_ptr) = 0;
            for (j=0; j<net_eMsg_; j++) 
            {
	      *(pwr_tInt32 *) (menu_ptr->value_ptr) += np->txmsg[j];
            }
            menu_ptr++;
            /* Rcvd */
	    * (pwr_tInt32 *) (menu_ptr->value_ptr) = 0;
            for (j=0; j<net_eMsg_; j++) 
            {
	      * (pwr_tInt32 *) (menu_ptr->value_ptr) += np->rxmsg[j];
            }
            menu_ptr++;
  	    l++;
          }
          k++;
        }
      } gdb_ScopeUnlock;

      for ( i = l; i < SHOW_NODES_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Os */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Hw */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Link */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Upcnt */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Timeup */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Sent */
        * (pwr_tInt32 *) (menu_ptr->value_ptr) = 0;
        menu_ptr++;
        /* Rcvd */
        * (pwr_tInt32 *) (menu_ptr->value_ptr) = 0;
        menu_ptr++;
      }
      SHOW_NODES_MAXPAGE = max( 1, (k - 1)/ SHOW_NODES_PAGESIZE + 1);

      /* Executor info */

      /* Node index */
      strcpy( menu_ptr->value_ptr, "");
      cdh_NodeIdToString( menu_ptr->value_ptr, gdbroot->my_node->nid, 0, 0);
      menu_ptr++;
      /*  Bus */
      menu_ptr->value_ptr = (char *) &qdb->g->bus;
      menu_ptr++;

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_NODE()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show node info.
*
**************************************************************************/
#if 0
static int rttsys_node_start( 	menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{

	int	sts;
	
	sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, (char *)arg1, "", 
		0, &RTTSYS_NODE);
	return sts;
}
#endif

int RTTSYS_NODE( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
#if 0
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  NODE_STRUCT 		*nodep = (NODE_STRUCT *) objectname;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p37_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p37_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    case RTT_APPL_INIT:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      /* Name */
      menu_ptr->value_ptr = (char *) &nodep->nodename;
      menu_ptr++;

      /* CF_Name receive*/
      menu_ptr->value_ptr = (char *) &nodep->rxmsg[NETH_TYPE_CFNAME];
      menu_ptr++;

      /* CF_Name transmit */
      menu_ptr->value_ptr = (char *) &nodep->txmsg[NETH_TYPE_CFNAME];
      menu_ptr++;
      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

#endif
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_PID()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Object picture for a Pid.
*
**************************************************************************/

int RTTSYS_OBJECT_PID( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static pwr_sClass_pid	*object_ptr;
  static pwr_sClass_mode *mode_ptr;
  static gdh_tSubid	pid_subid;
  static gdh_tSubid	mode_subid;
  static int		mode_found;
  pwr_tObjid 		objid;
  char			namebuf[80];
  char			modename[80];
  char			parametername[80];
  pwr_tObjid		mode_objid;
  int			value;

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    /* Check object first */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) 
    {
      rtt_message('E', "Node is down");
      return RTT__NOPICTURE;
    }
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    *picture = (char *) &dtt_systempicture_p8_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p8_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    /* Get the object */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) return sts;
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    sts = gdh_RefObjectInfo ( objectname, (pwr_tAddress *) &object_ptr, 
		&pid_subid, sizeof( pwr_sClass_pid));
    if ( EVEN(sts)) return sts;

    strcpy( parametername, objectname);
    strcat( parametername, ".ModeObjDId");
    sts = gdh_GetObjectInfo ( parametername, &mode_objid, sizeof( mode_objid));
    if ( ODD(sts))
    {
      sts = gdh_ObjidToName ( mode_objid, modename, sizeof( modename), cdh_mNName);
      if ( ODD(sts))
      {
        mode_found = 1;
        sts = gdh_RefObjectInfo ( modename, (pwr_tAddress *) &mode_ptr, 
		&mode_subid, sizeof( pwr_sClass_mode));
	if ( EVEN(sts)) return sts;
      }
      else
      {
        mode_found = 0;
	mode_subid = pwr_cNDlid;
      }
    }
    else
    {
      mode_found = 0;
      mode_subid = pwr_cNDlid;
    }
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;

    /* ProcVal, SetVal and OutVal */
    menu_ptr->value_ptr = (char *) &object_ptr->ProcVal;
    menu_ptr++;
    menu_ptr->value_ptr = (char *) &object_ptr->SetVal;
    menu_ptr++;
    menu_ptr->value_ptr = (char *) &object_ptr->OutVal;
    menu_ptr++;
    /* Bars */
    menu_ptr->value_ptr = (char *) &object_ptr->ProcVal;
    menu_ptr->minlimit = object_ptr->SetMinShow;
    menu_ptr->maxlimit = object_ptr->SetMaxShow;
    if ( menu_ptr->maxlimit == 0)
      menu_ptr->maxlimit = 100;
    menu_ptr++;
    menu_ptr->value_ptr = (char *) &object_ptr->SetVal;
    menu_ptr->minlimit = object_ptr->SetMinShow;
    menu_ptr->maxlimit = object_ptr->SetMaxShow;
    if ( menu_ptr->maxlimit == 0)
      menu_ptr->maxlimit = 100;
    menu_ptr++;
    menu_ptr->value_ptr = (char *) &object_ptr->OutVal;
    menu_ptr->minlimit = object_ptr->OutMinShow;
    menu_ptr->maxlimit = object_ptr->OutMaxShow;
    if ( menu_ptr->maxlimit == 0)
      menu_ptr->maxlimit = 100;
    menu_ptr++;

    /* Op SetVal */
    if ( mode_found)
    {
      strcpy( menu_ptr->parameter_name, modename);
      strcat( menu_ptr->parameter_name, ".SetVal");
      menu_ptr->value_ptr = (char *) &mode_ptr->SetVal;
      menu_ptr->database = RTT_DATABASE_GDH;
      menu_ptr->minlimit = mode_ptr->MinSet;
      menu_ptr->maxlimit = mode_ptr->MaxSet;
    }
    else
      menu_ptr->value_ptr = (char *) RTT_ERASE;
    menu_ptr++;
    /* Op OutVal */
    if ( mode_found)
    {
      strcpy( menu_ptr->parameter_name, modename);
      strcat( menu_ptr->parameter_name, ".ForcVal");
      menu_ptr->value_ptr = (char *) &mode_ptr->ForcVal;
      menu_ptr->database = RTT_DATABASE_GDH;
      menu_ptr->minlimit = mode_ptr->MinOut;
      menu_ptr->maxlimit = mode_ptr->MaxOut;
    }
    else
      menu_ptr->value_ptr = (char *) RTT_ERASE;
    menu_ptr++;
    /* Bias */
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".Bias");
    menu_ptr->value_ptr = (char *) &object_ptr->Bias;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;

    /* OpMode in the mode object */
    if ( mode_found)
    {
      if( mode_ptr->AutMode )
        strcpy( menu_ptr->value_ptr, "Auto");
      else if( mode_ptr->CascMod)
        strcpy( menu_ptr->value_ptr, "Casc");
      else
        strcpy( menu_ptr->value_ptr, "Man ");
    }
    else
      strcpy( menu_ptr->value_ptr, "");
    menu_ptr++;

    /* MinOut */
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".MinOut");
    menu_ptr->value_ptr = (char *) &object_ptr->MinOut;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    /* MaxOut */
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".MaxOut");
    menu_ptr->value_ptr = (char *) &object_ptr->MaxOut;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;

    strcpy( menu_ptr->parameter_name, modename);
    strcat( menu_ptr->parameter_name, ".OpMod");
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, modename);
    strcat( menu_ptr->parameter_name, ".OpMod");
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, modename);
    strcat( menu_ptr->parameter_name, ".OpMod");
    menu_ptr++;

    /* Min, Max SetVal */
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".SetMinShow");
    menu_ptr->value_ptr = (char *) &object_ptr->SetMinShow;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".SetMaxShow");
    menu_ptr->value_ptr = (char *) &object_ptr->SetMaxShow;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;

    /* Min, Max OutVal */
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".OutMinShow");
    menu_ptr->value_ptr = (char *) &object_ptr->OutMinShow;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".OutMaxShow");
    menu_ptr->value_ptr = (char *) &object_ptr->OutMaxShow;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;

    /* Gain... */
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".Gain");
    menu_ptr->value_ptr = (char *) &object_ptr->Gain;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".IntTime");
    menu_ptr->value_ptr = (char *) &object_ptr->IntTime;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".DerTime");
    menu_ptr->value_ptr = (char *) &object_ptr->DerTime;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".DerGain");
    menu_ptr->value_ptr = (char *) &object_ptr->DerGain;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".BiasGain");
    menu_ptr->value_ptr = (char *) &object_ptr->BiasGain;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    /* Unit */
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".SetEngUnit");
    menu_ptr->value_ptr = (char *) &object_ptr->SetEngUnit;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    strcpy( menu_ptr->parameter_name, objectname);
    strcat( menu_ptr->parameter_name, ".OutEngUnit");
    menu_ptr->value_ptr = (char *) &object_ptr->OutEngUnit;
    menu_ptr->database = RTT_DATABASE_GDH;
    menu_ptr++;
    /* Force */
    menu_ptr->value_ptr = (char *) &object_ptr->Force;
    menu_ptr++;
    /* PidAlg */
    switch ( object_ptr->PidAlg)
    {
      case  1: strcpy( menu_ptr->value_ptr, "I"); break;
      case  3: strcpy( menu_ptr->value_ptr, "I+P"); break;
      case  6: strcpy( menu_ptr->value_ptr, "P"); break;
      case  7: strcpy( menu_ptr->value_ptr, "PI"); break;
      case 11: strcpy( menu_ptr->value_ptr, "I+PD"); break;
      case 14: strcpy( menu_ptr->value_ptr, "P+D"); break;
      case 15: strcpy( menu_ptr->value_ptr, "PI+D"); break;
      case 30: strcpy( menu_ptr->value_ptr, "PD"); break;
      case 31: strcpy( menu_ptr->value_ptr, "PID"); break;
      default: strcpy( menu_ptr->value_ptr, "No");
    }
    menu_ptr++;
    /* Inverse */
    if ( object_ptr->Inverse ) 
      strcpy( menu_ptr->value_ptr, "Yes");
    else
      strcpy( menu_ptr->value_ptr, "No");
    menu_ptr++;
   /* Object Name */
    strcpy( menu_ptr->value_ptr, namebuf);

  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    sts = gdh_UnrefObjectInfo ( pid_subid);
    if ( memcmp( &mode_subid, &pwr_cNDlid, sizeof(pwr_cNDlid)))
    {
      sts = gdh_UnrefObjectInfo ( mode_subid);
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {

    if ( parameter_ptr == (char *) &PID_OBJECT_SETMAN)
    {
      if ( mode_ptr->AccMod & 1) 
      {
        menu_ptr = (rtt_t_menu_upd *) ctx->menu;
        while ( menu_ptr->text[0] != '\0' )
        {
          if ( menu_ptr->value_ptr == parameter_ptr)
          {
            value = 1;
            sts = gdh_SetObjectInfo ( menu_ptr->parameter_name, &value, 
			sizeof(value));
            if ( EVEN(sts)) 
	      rtt_message('E',"Unable to set mode");
	    break;
          }
	  menu_ptr++;
        }
      }
      else
        rtt_message('E',"Man is not defined for this PID");
    }
    else if ( parameter_ptr == (char *) &PID_OBJECT_SETAUTO)
    {
      if ( mode_ptr->AccMod & 2) 
      {
        menu_ptr = (rtt_t_menu_upd *) ctx->menu;
        while ( menu_ptr->text[0] != '\0' )
        {
          if ( menu_ptr->value_ptr == parameter_ptr)
          {
            value = 2;
            sts = gdh_SetObjectInfo ( menu_ptr->parameter_name, &value, 
			sizeof(value));
            if ( EVEN(sts)) 
	      rtt_message('E',"Unable to set mode");
	    break;
          }
  	  menu_ptr++;
        }
      }
      else
        rtt_message('E',"Auto is not defined for this PID");
    }
    else if ( parameter_ptr == (char *) &PID_OBJECT_SETCASC)
    {
      if ( mode_ptr->AccMod & 4) 
      {
        menu_ptr = (rtt_t_menu_upd *) ctx->menu;
        while ( menu_ptr->text[0] != '\0' )
        {
          if ( menu_ptr->value_ptr == parameter_ptr)
          {
            value = 4;
            sts = gdh_SetObjectInfo ( menu_ptr->parameter_name, &value, 
			sizeof(value));
            if ( EVEN(sts)) 
	      rtt_message('E',"Unable to set mode");
	    break;
          }
  	  menu_ptr++;
        }
      }
      else
        rtt_message('E',"Casc is not defined for this PID");
    }
    else if ( parameter_ptr == (char *) &object_ptr->SetMinShow)
    {
      ctx->update_init = 1;
    }
    else if ( parameter_ptr == (char *) &object_ptr->SetMaxShow)
    {
      ctx->update_init = 1;
    }
    else if ( parameter_ptr == (char *) &object_ptr->OutMinShow)
    {
      ctx->update_init = 1;
    }
    else if ( parameter_ptr == (char *) &object_ptr->OutMaxShow)
    {
      ctx->update_init = 1;
    }
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;

    /* ProcVal, SetVal and OutVal */
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;
    /* Bars */
    menu_ptr->minlimit = object_ptr->SetMinShow;
    menu_ptr->maxlimit = object_ptr->SetMaxShow;
    if ( menu_ptr->maxlimit == 0)
      menu_ptr->maxlimit = 100;
    menu_ptr++;
    menu_ptr->minlimit = object_ptr->SetMinShow;
    menu_ptr->maxlimit = object_ptr->SetMaxShow;
    if ( menu_ptr->maxlimit == 0)
      menu_ptr->maxlimit = 100;
    menu_ptr++;
    menu_ptr->minlimit = object_ptr->OutMinShow;
    menu_ptr->maxlimit = object_ptr->OutMaxShow;
    if ( menu_ptr->maxlimit == 0)
      menu_ptr->maxlimit = 100;
    menu_ptr++;
    /* Op SetVal */
    if ( mode_found)
    {
      menu_ptr->minlimit = mode_ptr->MinSet;
      menu_ptr->maxlimit = mode_ptr->MaxSet;
    }
    menu_ptr++;
    /* Op OutVal */
    if ( mode_found)
    {
      menu_ptr->minlimit = mode_ptr->MinOut;
      menu_ptr->maxlimit = mode_ptr->MaxOut;
    }
    menu_ptr++;
    menu_ptr++;
    /* OpMode in the mode object */
    if ( mode_found)
    {
      if ( mode_ptr->AutMode )
        strcpy( menu_ptr->value_ptr, "Auto");
      else if ( mode_ptr->CascMod)
        strcpy( menu_ptr->value_ptr, "Casc");
      else
        strcpy( menu_ptr->value_ptr, "Man ");
    }
    else
      strcpy( menu_ptr->value_ptr, "");
    menu_ptr++;

    menu_ptr += 17;
    /* PidAlg */
    switch ( object_ptr->PidAlg)
    {
      case  1: strcpy( menu_ptr->value_ptr, "I"); break;
      case  3: strcpy( menu_ptr->value_ptr, "I+P"); break;
      case  6: strcpy( menu_ptr->value_ptr, "P"); break;
      case  7: strcpy( menu_ptr->value_ptr, "PI"); break;
      case 11: strcpy( menu_ptr->value_ptr, "I+PD"); break;
      case 14: strcpy( menu_ptr->value_ptr, "P+D"); break;
      case 15: strcpy( menu_ptr->value_ptr, "PI+D"); break;
      case 30: strcpy( menu_ptr->value_ptr, "PD"); break;
      case 31: strcpy( menu_ptr->value_ptr, "PID"); break;
      default: strcpy( menu_ptr->value_ptr, "No");
    }
    menu_ptr++;
    /* Inverse */
    if ( object_ptr->Inverse ) 
      strcpy( menu_ptr->value_ptr, "Yes");
    else
      strcpy( menu_ptr->value_ptr, "No");
    menu_ptr++;
  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_OBJECT_AV()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Object picture for a Av.
*
**************************************************************************/

int RTTSYS_OBJECT_AV( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_sClass_Av		*object_ptr;
  static gdh_tSubid	av_subid;
  static gdh_tSubid	av_actval_subid;
  pwr_tObjid 		objid;
  char			namebuf[80];
  char			parameter_name[80];
  pwr_tFloat32		*actval_ptr;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    /* Check object first */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) 
    {
      rtt_message('E', "Node is down");
      return RTT__NOPICTURE;
    }
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    *picture = (char *) &dtt_systempicture_p10_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p10_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    /* Get the object */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) return sts;
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    
    sts = gdh_RefObjectInfo ( objectname, (pwr_tAddress *) &object_ptr, 
		&av_subid, sizeof( pwr_sClass_Av));
    if ( EVEN(sts)) return sts;
    strcpy( parameter_name, namebuf);
    strcat( parameter_name, ".ActualValue");
    sts = gdh_RefObjectInfo ( parameter_name, (pwr_tAddress *) &actval_ptr, 
		&av_actval_subid, sizeof( *actval_ptr));
    if ( EVEN(sts)) return sts;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;

    /* Object name */
    strcpy( menu_ptr->value_ptr, namebuf);

    menu_ptr++;
    menu_ptr->value_ptr = (char *) actval_ptr;
    strcpy( menu_ptr->parameter_name, namebuf);
    strcat( menu_ptr->parameter_name, ".ActualValue");
    
    menu_ptr++;
    menu_ptr->value_ptr = object_ptr->Description;
    strcpy( menu_ptr->parameter_name, namebuf);
    strcat( menu_ptr->parameter_name, ".Description");

    menu_ptr++;
    menu_ptr->value_ptr = object_ptr->Unit;
    strcpy( menu_ptr->parameter_name, namebuf);
    strcat( menu_ptr->parameter_name, ".Unit");

  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    sts = gdh_UnrefObjectInfo ( av_subid);
    sts = gdh_UnrefObjectInfo ( av_actval_subid);
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {

  }

  return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_GRAFCET()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show nethandler info.
*
**************************************************************************/

#define GRAFCET_PAGESIZE 20


/*************************************************************************
*
* Name:		rtt_menu_bubblesort()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	*menulist	I	menulist.
*
* Description:
*	This function sorts the items in a plclist in 
*	aphabetic order.
*
**************************************************************************/

static int rttsys_plclist_bubblesort( 	rttsys_t_plcpgm_list	*plclist,
					int			size)
{
	int	i, j;
	char	*str1, *str2;
	rttsys_t_plcpgm_list 	dum;
	rttsys_t_plcpgm_list	*plclist_ptr;

	for ( i = size - 1; i > 0; i--)
	{
	  plclist_ptr = plclist;
	  for ( j = 0; j < i; j++)
	  {
	    str1 = plclist_ptr->name;
	    str2 = (plclist_ptr + 1)->name;
	    if ( *str2 == 0)
	      break;
	    if ( strcmp( str1, str2) > 0)
	    {
	      /* Change order */
	      memcpy( &dum, plclist_ptr + 1, sizeof( rttsys_t_plcpgm_list));
	      memcpy( plclist_ptr + 1, plclist_ptr, 
			sizeof( rttsys_t_plcpgm_list));
	      memcpy( plclist_ptr, &dum, sizeof( rttsys_t_plcpgm_list));
	    }
	    plclist_ptr++;
	  }
	}
	return RTT__SUCCESS;
}

static int rttsys_steplist_bubblesort( 	rttsys_t_step_list	*steplist,
					int			size)
{
	int	i, j;
	char	*str1, *str2;
	rttsys_t_step_list 	dum;
	rttsys_t_step_list	*steplist_ptr;

	for ( i = size - 1; i > 0; i--)
	{
	  steplist_ptr = steplist;
	  for ( j = 0; j < i; j++)
	  {
	    str1 = steplist_ptr->name;
	    str2 = (steplist_ptr + 1)->name;
	    if ( *str2 == 0)
	      break;
	    if ( strcmp( str1, str2) > 0)
	    {
	      /* Change order */
	      memcpy( &dum, steplist_ptr + 1, sizeof( rttsys_t_step_list));
	      memcpy( steplist_ptr + 1, steplist_ptr, 
			sizeof( rttsys_t_step_list));
	      memcpy( steplist_ptr, &dum, sizeof( rttsys_t_step_list));
	    }
	    steplist_ptr++;
	  }
	}
	return RTT__SUCCESS;
}

static int	rttsys_get_plcpgm( 	pwr_tObjid	initstep_objid,
					pwr_tObjid	*plc_objid)
{
	pwr_tObjid	parent_objid;
	pwr_tClassId 	parent_class;
	int		sts;

	parent_class = 0;
	parent_objid = initstep_objid;
	while( parent_class != pwr_cClass_plc )
	{
	  sts = gdh_GetParent ( parent_objid, &parent_objid);
	  if ( EVEN(sts)) return sts;

	  sts = gdh_GetObjectClass ( parent_objid, &parent_class);
	  if ( EVEN(sts)) return sts;
	}
	*plc_objid = parent_objid;
	return RTT__SUCCESS;
}

static int	rttsys_plclist_add( 	pwr_tObjid		plc_objid,
					rttsys_t_plcpgm_list	**plclist,
					int			*plclist_count,
					int			*alloc)
{
	rttsys_t_plcpgm_list	*plclist_ptr;
	rttsys_t_plcpgm_list	*new_plclist;

	if ( *plclist_count == 0)
	{
	  *plclist = calloc( RTTSYS_PLCALLOC , sizeof(rttsys_t_plcpgm_list));
	  if ( *plclist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_PLCALLOC;
	}
	else if ( *alloc <= *plclist_count)
	{
	  new_plclist = calloc( *alloc + RTTSYS_PLCALLOC, 
			sizeof(rttsys_t_plcpgm_list));
	  if ( new_plclist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_plclist, *plclist, 
			*plclist_count * sizeof(rttsys_t_plcpgm_list));
	  free( *plclist);
	  *plclist = new_plclist;
	  (*alloc) += RTTSYS_PLCALLOC;
	}
	plclist_ptr = *plclist + *plclist_count;
	plclist_ptr->objid = plc_objid;
	(*plclist_count)++;
	return RTT__SUCCESS;
}

static int rttsys_initsteplist_add( pwr_tObjid		initstep_objid,
				rttsys_t_step_list	**initsteplist,
				int			*initsteplist_count)
{
	rttsys_t_step_list	*initsteplist_ptr;
	rttsys_t_step_list	*new_initsteplist;

	if ( *initsteplist_count == 0)
	{
	  *initsteplist = calloc( 1 , sizeof(rttsys_t_step_list));
	  if ( *initsteplist == 0)
	    return RTT__NOMEMORY;
	}
	else
	{
/*	    *menulist = realloc( *menulist, (index + 2) * sizeof(rtt_t_menu));
*/
	  new_initsteplist = calloc( *initsteplist_count + 1 , 
			sizeof(rttsys_t_step_list));
	  if ( new_initsteplist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_initsteplist, *initsteplist, 
			*initsteplist_count * sizeof(rttsys_t_step_list));
	  free( *initsteplist);
	  *initsteplist = new_initsteplist;
	}
	initsteplist_ptr = *initsteplist + *initsteplist_count;
	initsteplist_ptr->objid = initstep_objid;
	(*initsteplist_count)++;
	return RTT__SUCCESS;
}

static int	rttsys_grafcet_select( 	menu_ctx	ctx,
					pwr_tObjid	dummyoi,
					void		*dummy1,
					void		*dummy2,
					void		*dummy3,
					void		*dummy4)
{
	rttsys_t_plcpgm_list	*plclist_ptr;
	rtt_t_menu_upd		*menu_ptr;


	/* Get the selected object */
	if ( ctx->current_item + (GRAFCET_PAGE - 1)*GRAFCET_PAGESIZE > 
		plclist_count - 1)
	  return RTT__NOPICTURE;

	menu_ptr = (rtt_t_menu_upd *) ctx->menu;
	menu_ptr += ctx->current_item;
	plclist_ptr = plclist + grafcet_page * GRAFCET_PAGESIZE + ctx->current_item;
	if ( plclist_ptr->selected)
	{
	  plclist_ptr->selected = 0;
       	  strcpy( menu_ptr->output_text, "");
	}
	else
	{
	  plclist_ptr->selected = 1;
       	  strcpy( menu_ptr->output_text, "!");
	}
	rtt_edit_select( ctx);
        ctx->update_init = 1;
        rtt_menu_edit_update( ctx);
	return RTT__NOPICTURE;
}

int RTTSYS_GRAFCET( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  int			i, j;
  int			found;
  char			namebuf[80];
  rttsys_t_plcpgm_list	*plclist_ptr;
  rttsys_t_step_list *initsteplist_ptr;
  pwr_tObjid 		initstep_objid;
  pwr_tObjid 		plc_objid;
  unsigned int		active;
  static  char		grafcet_plc_name[] = "GRAFCET";
  int			plccount;
  pwr_sAttrRef		attrref;
  int			plc_alloc;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	Return address of menu
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p11_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p11_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    grafcet_page--;
    grafcet_page = max( grafcet_page, 0);
    GRAFCET_PAGE = grafcet_page + 1;
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    plclist_ptr = plclist + grafcet_page * GRAFCET_PAGESIZE;
    plccount = grafcet_page * GRAFCET_PAGESIZE;
    for ( i = 0; i < GRAFCET_PAGESIZE; i++)
    {
      if ( plccount < plclist_count)
      {
        if ( plclist_ptr->selected)
	{
       	  strcpy( menu_ptr->output_text, "!");
	}
        else
	{
          strcpy( menu_ptr->output_text, "");
	}
      }
      else
        strcpy( menu_ptr->output_text, "");
      menu_ptr++;
      plclist_ptr++;
      plccount++;
    }
/*
    rtt_edit_draw( ctx, (rtt_t_backgr *) &dtt_systempicture_p11_bg);
    ctx->current_item = 0;
    rtt_edit_select( ctx);
*/
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    grafcet_page++;
    grafcet_page = min( grafcet_page, GRAFCET_MAXPAGE - 1);
    GRAFCET_PAGE = grafcet_page + 1;
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    plclist_ptr = plclist + grafcet_page * GRAFCET_PAGESIZE;
    plccount = grafcet_page * GRAFCET_PAGESIZE;
    for ( i = 0; i < GRAFCET_PAGESIZE; i++)
    {
      if ( plccount < plclist_count)
      {
        if ( plclist_ptr->selected)
	{
       	  strcpy( menu_ptr->output_text, "!");
	}
        else
	{
          strcpy( menu_ptr->output_text, "");
	}
      }
      else
        strcpy( menu_ptr->output_text, "");
      menu_ptr++;
      plclist_ptr++;
      plccount++;
    }
/*
    rtt_edit_draw( ctx, (rtt_t_backgr *) &dtt_systempicture_p11_bg);
    ctx->current_item = 0;
    rtt_edit_select( ctx);
*/
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    plclist_count = 0;
    grafcet_page = 0;
    GRAFCET_PAGE = grafcet_page + 1;

    /* Get all initsteps of this node */
    sts = gdh_GetClassList ( pwr_cClass_initstep, &initstep_objid);
    while ( ODD(sts))
    {
      /* Get the plcpgm of this step */
      sts = rttsys_get_plcpgm( initstep_objid, &plc_objid);
      if ( EVEN(sts)) return sts;

      plclist_ptr = plclist;
      found = 0;
      for ( i = 0; i < plclist_count; i ++)
      {
        if ( cdh_ObjidIsEqual( plclist_ptr->objid, plc_objid))
        {
	  found = 1;
	  break;
        }
      }

      if ( !found)
      {
        /* Store this plc */
        rttsys_plclist_add( plc_objid, &plclist, &plclist_count, &plc_alloc);
        plclist_ptr = plclist + plclist_count - 1;
      }

      sts = gdh_ObjidToName ( plclist_ptr->objid, namebuf, sizeof( namebuf), cdh_mNName);
      if ( EVEN(sts)) return sts;
      strcpy( plclist_ptr->name, namebuf);

      /* Store and direct link the initstep */
      rttsys_initsteplist_add( initstep_objid, &plclist_ptr->initsteps, 
		&plclist_ptr->initstep_count);
      initsteplist_ptr = plclist_ptr->initsteps + plclist_ptr->initstep_count - 1;

      sts = gdh_ObjidToName ( initstep_objid, namebuf, sizeof( namebuf), cdh_mNName);
      if ( EVEN(sts)) return sts;

      strcpy( initsteplist_ptr->name, namebuf);

      sts = gdh_ClassAttrToAttrref( pwr_cClass_initstep, ".Order[0]", &attrref);
      if ( EVEN(sts)) return sts;

      attrref.Objid = initstep_objid;
      sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &initsteplist_ptr->value_ptr,
		&initsteplist_ptr->subid);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextObject ( initstep_objid, &initstep_objid);
    }

    rttsys_plclist_bubblesort( plclist, plclist_count);
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    for ( i = grafcet_page * GRAFCET_PAGESIZE; 
	  i < min( plclist_count, (grafcet_page + 1) * GRAFCET_PAGESIZE) ; i++)
    {
      /* Start RTTSYS_GRAFCET_PLC with PF2 */
      plclist_ptr = plclist + i;
      menu_ptr->func = &rttsys_grafcet_select;
      menu_ptr->func2 = &rtt_edit_debug_signals;
      menu_ptr->func3 = &rtt_menu_new_sysedit;
      menu_ptr->argoi = plclist_ptr->objid;
      menu_ptr->arg1 = 0;
      menu_ptr->arg2 = grafcet_plc_name;
      menu_ptr->arg3 = 0;
      menu_ptr->arg4 = (void *) &RTTSYS_GRAFCET_PLC;
      menu_ptr++;
    }
    GRAFCET_MAXPAGE = max( 1, (plclist_count - 1)/ GRAFCET_PAGESIZE + 1);
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    /* Free the steplists and the plclist */
    plclist_ptr = plclist;
    for ( i = 0; i < plclist_count; i++)
    {
      if ( plclist_ptr->initstep_count > 0)
      {
        initsteplist_ptr = plclist_ptr->initsteps;
        for ( j = 0; j < plclist_ptr->initstep_count; j++)
        {
          sts = gdh_DLUnrefObjectInfo ( initsteplist_ptr->subid);
          if ( EVEN(sts)) return sts;
          initsteplist_ptr++;
        }
        free( plclist_ptr->initsteps);
      }
      plclist_ptr++;
    }
    free( plclist);
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    plclist_ptr = plclist + grafcet_page * GRAFCET_PAGESIZE;
    for ( i = grafcet_page * GRAFCET_PAGESIZE; 
	  i < min( plclist_count, (grafcet_page + 1) * GRAFCET_PAGESIZE) ; i++)
    {
      initsteplist_ptr = plclist_ptr->initsteps;
      active = 1;
      for ( j = 0; j < plclist_ptr->initstep_count; j++)
      {
	if ( *(initsteplist_ptr->value_ptr))
	  active = 0;
	initsteplist_ptr++;
      }
      if ( active)
        strcpy( menu_ptr->value_ptr, "Active ");
      else
        strcpy( menu_ptr->value_ptr, "       ");
      
      strcat( menu_ptr->value_ptr, plclist_ptr->name);
      plclist_ptr++;
      menu_ptr++;
    }
    for ( i = i; 
	  i < (grafcet_page + 1) * GRAFCET_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func2 = 0;
      menu_ptr->func3 = &rtt_menu_new_sysedit;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr->arg1 = 0;
      menu_ptr->arg2 = grafcet_plc_name;
      menu_ptr->arg3 = 0;
      menu_ptr->arg4 = (void *) &RTTSYS_GRAFCET_PLC;
      menu_ptr++;
    }

  }

  return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_GRAFCET_PLC()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show grafcet activity in one or more plcprograms.
*
**************************************************************************/

static int rttsys_steplist_add(	pwr_tObjid		step_objid,
				rttsys_t_step_list	**steplist,
				int			*steplist_count,
				int			*alloc,
				pwr_tClassId		class,
				int			dummy2)
{
	rttsys_t_step_list	*steplist_ptr;
	rttsys_t_step_list	*new_steplist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;

	if ( *steplist_count == 0)
	{
	  *steplist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_step_list));
	  if ( *steplist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_ALLOC;
	}
	else if ( *alloc <= *steplist_count)
	{
	  new_steplist = calloc( *alloc + RTTSYS_ALLOC,
			sizeof(rttsys_t_step_list));
	  if ( new_steplist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_steplist, *steplist, 
			*steplist_count * sizeof(rttsys_t_step_list));
	  free( *steplist);
	  *steplist = new_steplist;
	  (*alloc) += RTTSYS_ALLOC;
	}
	steplist_ptr = *steplist + *steplist_count;
	steplist_ptr->objid = step_objid;

        sts = gdh_ObjidToName ( step_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        strcpy( steplist_ptr->name, namebuf);
        sts = gdh_ClassAttrToAttrref( class, ".Order[0]", &attrref);
        if ( EVEN(sts)) return sts;

        attrref.Objid = step_objid;
        sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &steplist_ptr->value_ptr,
		&steplist_ptr->subid);
        if ( EVEN(sts)) return sts;

	(*steplist_count)++;
	return RTT__SUCCESS;
}

static int rttsys_orderlist_add( pwr_tObjid		order_objid,
				rttsys_t_step_list	**orderlist,
				int			*orderlist_count,
				int			*alloc,
				pwr_tClassId		class,
				int			dummy2)
{
	rttsys_t_step_list	*orderlist_ptr;
	rttsys_t_step_list	*new_orderlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;

	if ( *orderlist_count == 0)
	{
	  *orderlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_step_list));
	  if ( *orderlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_ALLOC;
	}
	else if ( *alloc <= *orderlist_count)
	{
	  new_orderlist = calloc( *alloc + RTTSYS_ALLOC, 
			sizeof(rttsys_t_step_list));
	  if ( new_orderlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_orderlist, *orderlist, 
			*orderlist_count * sizeof(rttsys_t_step_list));
	  free( *orderlist);
	  *orderlist = new_orderlist;
	  (*alloc) += RTTSYS_ALLOC;
	}
	orderlist_ptr = *orderlist + *orderlist_count;
	orderlist_ptr->objid = order_objid;

        sts = gdh_ObjidToName ( order_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        strcpy( orderlist_ptr->name, namebuf);
        sts = gdh_ClassAttrToAttrref( class, ".Status[0]", &attrref);
        if ( EVEN(sts)) return sts;

        attrref.Objid = order_objid;
        sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &orderlist_ptr->value_ptr,
		&orderlist_ptr->subid);
        if ( EVEN(sts)) return sts;

	(*orderlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_GRAFCET_PLC( 	menu_ctx	ctx,
				int		event,
				char		*parameter_ptr,
				char		*objectname,
				char		**picture)
{

#define GRAFCET_PLC_PAGESIZE 20
  menu_ctx		parent_ctx;
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static int		page;	
  int			i, j, k, l;
  rttsys_t_plcpgm_list	*plclist_ptr;
  rttsys_t_plcpgm_list	*sel_plclist_ptr;
  rttsys_t_step_list 	*initsteplist_ptr;
  rttsys_t_step_list 	*steplist_ptr;
  rttsys_t_step_list 	*orderlist_ptr;
  char			*s;
  static  rttsys_t_plcpgm_list	*sel_plclist;
  static  int		sel_plclist_count;
  int			alloc;
	  
  /**********************************************************
  *	Return address of menu
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p12_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p12_eu;

    /* Get the selected plcprogram */
    sel_plclist_count = 0;
    plclist_ptr = plclist;
    for ( i = 0; i < plclist_count; i++)
    {
      if ( plclist_ptr->selected)
      {
        /* Add this plc in the selected plclist */
        rttsys_plclist_add( plclist_ptr->objid, &sel_plclist,
		&sel_plclist_count, &alloc);
        sel_plclist_ptr = sel_plclist + sel_plclist_count - 1;
	memcpy( sel_plclist_ptr, plclist_ptr, sizeof( rttsys_t_plcpgm_list));
      }
      plclist_ptr++;
    }
    if ( sel_plclist_count > 4)
    {
       /* To many plcpgms is selected */
       rtt_message('E', "Max amount of selected plcpgms is 4");
       return RTT__NOPICTURE;
    }

    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    page--;
    page = max( page, 0);
    GRAFCET_PLC_PAGE = page + 1;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    page++;
    page = min( page, GRAFCET_PLC_MAXPAGE -1);
    GRAFCET_PLC_PAGE = page + 1;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    page = 0;
    GRAFCET_PLC_PAGE = page + 1;
    GRAFCET_PLC_MAXPAGE = 1;

    if ( sel_plclist_count == 0)
    {
      /* No plcpgms is selected, take the selected plcpgm */
      parent_ctx = (menu_ctx) ctx->parent_ctx;
      if ( parent_ctx->current_item + (GRAFCET_PAGE - 1)*GRAFCET_PAGESIZE > 
		plclist_count - 1)
      {
        rtt_message('E', "Select a plcpgm");
      }
      else
      {
        plclist_ptr = plclist + parent_ctx->current_item + 
			(GRAFCET_PAGE - 1)*GRAFCET_PAGESIZE;
        /* Add this plc in the selected plclist */
        rttsys_plclist_add( plclist_ptr->objid, &sel_plclist,
		&sel_plclist_count, &alloc);
        sel_plclist_ptr = sel_plclist + sel_plclist_count - 1;
        memcpy( sel_plclist_ptr, plclist_ptr, sizeof( rttsys_t_plcpgm_list));
      }
    }

    /* Reset output */
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    for ( i = 0; i < 80; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;


    plclist_ptr = sel_plclist;
    for( k = 0; k < sel_plclist_count; k++)
    {
      /* Get all steps of this plcpgm */
      sts = rtt_get_objects_hier_class_name( ctx, plclist_ptr->objid, 
		pwr_cClass_step, NULL, 500, 0,
		&rttsys_steplist_add, 
		&plclist_ptr->steps, 
		&plclist_ptr->step_count, 
		&plclist_ptr->step_alloc, 
		(void *) pwr_cClass_step, 0);
      if ( EVEN(sts)) return sts;
      sts = rtt_get_objects_hier_class_name( ctx, plclist_ptr->objid, 
		pwr_cClass_substep, NULL, 500, 0,
		&rttsys_steplist_add, 
		&plclist_ptr->steps, 
		&plclist_ptr->step_count, 
		&plclist_ptr->step_alloc, 
		(void *) pwr_cClass_substep, 0);
      if ( EVEN(sts)) return sts;
      sts = rtt_get_objects_hier_class_name( ctx, plclist_ptr->objid, 
		pwr_cClass_ssbegin, NULL, 500, 0,
		&rttsys_steplist_add, 
		&plclist_ptr->steps, 
		&plclist_ptr->step_count, 
		&plclist_ptr->step_alloc, 
		(void *) pwr_cClass_ssbegin, 0);
      if ( EVEN(sts)) return sts;
      sts = rtt_get_objects_hier_class_name( ctx, plclist_ptr->objid, 
		pwr_cClass_ssend, NULL, 500, 0,
		&rttsys_steplist_add, 
		&plclist_ptr->steps, 
		&plclist_ptr->step_count, 
		&plclist_ptr->step_alloc, 
		(void *) pwr_cClass_ssend, 0);
      if ( EVEN(sts)) return sts;
      sts = rtt_get_objects_hier_class_name( ctx, plclist_ptr->objid, 
		pwr_cClass_order, NULL, 500, 0,
		&rttsys_orderlist_add, 
		&plclist_ptr->orders, 
		&plclist_ptr->order_count, 
		&plclist_ptr->order_alloc, 
		(void *) pwr_cClass_order, 0);
      if ( EVEN(sts)) return sts;
      rttsys_steplist_bubblesort( plclist_ptr->steps, plclist_ptr->step_count);
      rttsys_steplist_bubblesort( plclist_ptr->orders,plclist_ptr->order_count);

      i = 0;
      /* Print the plcname in line 1, just show the segment name */
      s = strrchr( plclist_ptr->name, '-');
      if ( s == 0)
        s = plclist_ptr->name;
      else
        s++;
      strcpy( menu_ptr->value_ptr, s);
      menu_ptr++;
      i++;
      initsteplist_ptr = plclist_ptr->initsteps;
      for ( j = 0; j < plclist_ptr->initstep_count; j++)
      {
        if ( *(initsteplist_ptr->value_ptr))
        {
	  if ( (i > 0) && (i < GRAFCET_PLC_PAGESIZE))
	  {
            strncpy( menu_ptr->value_ptr,
		&initsteplist_ptr->name[strlen( plclist_ptr->name) + 3],
		sizeof( GRAFCET_PLC_LINE1 ));
            menu_ptr++;
	  }
	  i++;
        }
        initsteplist_ptr++;
      }
      steplist_ptr = plclist_ptr->steps;
      for ( j = 0; j < plclist_ptr->step_count; j++)
      {
        if ( *(steplist_ptr->value_ptr))
        {
	  if ( (i > 0) && (i < GRAFCET_PLC_PAGESIZE))
	  {
            strncpy( menu_ptr->value_ptr, 
		&steplist_ptr->name[strlen( plclist_ptr->name) + 3],
		sizeof( GRAFCET_PLC_LINE1 ));
            menu_ptr++;
	  }
	  i++;
        }
        steplist_ptr++;
      }
      orderlist_ptr = plclist_ptr->orders;
      for ( j = 0; j < plclist_ptr->order_count; j++)
      {
        if ( *(orderlist_ptr->value_ptr))
        {
	  if ( (i > 0) && (i < GRAFCET_PLC_PAGESIZE))
	  {
            strncpy( menu_ptr->value_ptr, 
		&orderlist_ptr->name[strlen( plclist_ptr->name) + 3],
		sizeof( GRAFCET_PLC_LINE1 ));
            menu_ptr++;
	  }
	  i++;
        }
        orderlist_ptr++;
      }
      
      for ( j = i; j < (page + 1) * GRAFCET_PLC_PAGESIZE; j++)
      {
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
      }
      plclist_ptr++;
    }
  }

  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    /* Free the steplists and the sel_plclist */
    plclist_ptr = sel_plclist;
    for ( i = 0; i < sel_plclist_count; i++)
    {
      if ( plclist_ptr->step_count > 0)
      {
        steplist_ptr = plclist_ptr->steps;
        for ( j = 0; j < plclist_ptr->step_count; j++)
        {
          sts = gdh_DLUnrefObjectInfo ( steplist_ptr->subid);
          steplist_ptr++;
        }
        free( plclist_ptr->steps);
      }
      if ( plclist_ptr->order_count > 0)
      {
        orderlist_ptr = plclist_ptr->orders;
        for ( j = 0; j < plclist_ptr->order_count; j++)
        {
          sts = gdh_DLUnrefObjectInfo ( orderlist_ptr->subid);
          orderlist_ptr++;
        }
        free( plclist_ptr->orders);
      }
      sel_plclist_count = 0;
    }
    free( sel_plclist);
    sel_plclist_count = 0;
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    plclist_ptr = sel_plclist;
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;

    for ( k = 0; k < sel_plclist_count; k++)
    {
      i = 0;
      l = 1;
      menu_ptr++;
      initsteplist_ptr = plclist_ptr->initsteps;
      for ( j = 0; j < plclist_ptr->initstep_count; j++)
      {
        if ( *(initsteplist_ptr->value_ptr))
        {
	  if ( (i >= page * (GRAFCET_PLC_PAGESIZE - 1)) && 
	     (i < (page + 1) * (GRAFCET_PLC_PAGESIZE - 1)))
	  {
            strncpy( menu_ptr->value_ptr, 
		&initsteplist_ptr->name[strlen( plclist_ptr->name) + 3], 
		sizeof( GRAFCET_PLC_LINE1 ));
            menu_ptr++;
	    l++;
	  }
	  i++;
        }
        initsteplist_ptr++;
      }
      steplist_ptr = plclist_ptr->steps;
      for ( j = 0; j < plclist_ptr->step_count; j++)
      {
        if ( *(steplist_ptr->value_ptr))
        {
	  if ( (i >= page * (GRAFCET_PLC_PAGESIZE - 1)) && 
	     (i < (page + 1) * (GRAFCET_PLC_PAGESIZE - 1)))
	  {
            strncpy( menu_ptr->value_ptr, 
		&steplist_ptr->name[strlen( plclist_ptr->name) + 3], 
		sizeof( GRAFCET_PLC_LINE1 ));
            menu_ptr++;
	    l++;
	  }
	  i++;
        }
        steplist_ptr++;
      }
      orderlist_ptr = plclist_ptr->orders;
      for ( j = 0; j < plclist_ptr->order_count; j++)
      {
        if ( *(orderlist_ptr->value_ptr))
        {
	  if ( (i >= page * (GRAFCET_PLC_PAGESIZE - 1)) && 
	     (i < (page + 1) * (GRAFCET_PLC_PAGESIZE - 1)))
	  {
            strncpy( menu_ptr->value_ptr, 
		&orderlist_ptr->name[strlen( plclist_ptr->name) + 3], 
		sizeof( GRAFCET_PLC_LINE1 ));
            menu_ptr++;
	    l++;
	  }
 	  i++;
        }
        orderlist_ptr++;
      }
      for ( j = l; j < GRAFCET_PLC_PAGESIZE; j++)
      {
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
      }
      plclist_ptr++;
      GRAFCET_PLC_MAXPAGE = 
	max( GRAFCET_PLC_MAXPAGE, max( 1, i / (GRAFCET_PLC_PAGESIZE - 1) + 1));
    }
  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_PLCPGM()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Shows plcpgms.
*
**************************************************************************/

#define PLCPGM_PAGESIZE 20


int RTTSYS_PLCPGM( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  int			i;
  rttsys_t_step_list	*plclist_ptr;
  static int		page;
  static  rttsys_t_step_list	*plclist;
  static  int			plclist_count;
  static  char		nullstring[40] = "";
  int			plc_alloc;

  IF_NOGDH_RETURN;
  /**********************************************************
  *	Return address of menu
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p13_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p13_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    page--;
    page = max( page, 0);
    PLCPGM_PAGE = page + 1;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    page++;
    page = min( page, PLCPGM_MAXPAGE - 1);
    PLCPGM_PAGE = page + 1;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    plclist_count = 0;
    page = 0;
    PLCPGM_PAGE = page + 1;

    sts = rttsys_object_add( pwr_cClass_plc, &plclist, 
		&plclist_count, &plc_alloc, ".Description", 40);
    if ( EVEN(sts)) return sts;

    rttsys_steplist_bubblesort( plclist, plclist_count);
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    plclist_ptr = plclist;
    for ( i = page * PLCPGM_PAGESIZE; 
	  i < min( plclist_count, (page + 1) * PLCPGM_PAGESIZE) ; i++)
    {
      /* Define PF buttons and Return */
      menu_ptr->func = &rtt_hierarchy_child;
      menu_ptr->func2 = &rtt_edit_debug_signals;
      menu_ptr->func3 = &rtt_debug_child;
      menu_ptr->argoi = plclist_ptr->objid;
      menu_ptr->arg1 = 0;
      menu_ptr->arg2 = 0;
      menu_ptr->arg3 = 0;
      menu_ptr->arg4 = 0;
      menu_ptr++;
      menu_ptr++;
      plclist_ptr++;
    }
    PLCPGM_MAXPAGE = max( 1, (plclist_count - 1)/ PLCPGM_PAGESIZE + 1);
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    /* Free the steplists and the plclist */
    free( plclist);
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    plclist_ptr = plclist + page * PLCPGM_PAGESIZE;
    for ( i = page * PLCPGM_PAGESIZE;
	  i < min( plclist_count, (page + 1) * PLCPGM_PAGESIZE) ; i++)
    {
      menu_ptr->func = &rtt_hierarchy_child;
      menu_ptr->func2 = &rtt_edit_debug_signals;
      menu_ptr->func3 = &rtt_debug_child;
      menu_ptr->argoi = plclist_ptr->objid;
      menu_ptr->arg1 = 0;
      menu_ptr->arg2 = 0;
      menu_ptr->arg3 = 0;
      menu_ptr->arg4 = 0;
      strncpy( menu_ptr->value_ptr, plclist_ptr->name, sizeof( PLCPGM_LINE1));
      menu_ptr++;
      menu_ptr->value_ptr = (char *) plclist_ptr->value_ptr;
      menu_ptr++;
      plclist_ptr++;
    }
    for ( i = i; 
	  i < (page + 1) * PLCPGM_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr->arg1 = 0;
      menu_ptr->arg2 = 0;
      menu_ptr->arg3 = 0;
      menu_ptr->arg4 = 0;
      menu_ptr++;
      menu_ptr->value_ptr = nullstring;
      menu_ptr++;
    }

  }

  return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_ERROR()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Shows errorcounts on device objects.
*
**************************************************************************/

#define ERROR_PAGESIZE 20

static int rttsys_objectlist_add( pwr_tObjid		object_objid,
				pwr_tClassId		class,
				rttsys_t_step_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				char			*attrstr,
				int			attrsize)
{
	rttsys_t_step_list	*objectlist_ptr;
	rttsys_t_step_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;

	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_step_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_ALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_ALLOC,
			sizeof(rttsys_t_step_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_step_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_ALLOC;
	}
	objectlist_ptr = *objectlist + *objectlist_count;
	objectlist_ptr->objid = object_objid;

        sts = gdh_ObjidToName ( object_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        strcpy( objectlist_ptr->name, namebuf);

/****  !!! DLUnrefObjectInfo !!!  *******/
	if ( *attrstr != 0)
	{
	  sts = gdh_ClassAttrToAttrref( class, attrstr, &attrref);
          if ( EVEN(sts)) return sts;
	}
	else
	  memset( &attrref, 0, sizeof( attrref));


	attrref.Objid = object_objid;
	sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &objectlist_ptr->value_ptr,
		&objectlist_ptr->subid);
        if ( EVEN(sts)) return sts;

/*****
        strcat( namebuf, attrstr);
        sts = gdh_RefObjectInfo ( namebuf, &objectlist_ptr->value_ptr,
		&objectlist_ptr->subid, attrsize);
        if ( EVEN(sts)) return sts;
*****/
	(*objectlist_count)++;
	return RTT__SUCCESS;
}

static int rttsys_object_add( 	pwr_tClassId		class,
				rttsys_t_step_list	**objectlist,
				int			*objectlist_count,
				int			*object_alloc,
				char			*attrstr,
				int			attrsize)
{
    pwr_tObjid		object_objid;
    int			sts;

    /* Get all initsteps of this node */
    sts = gdh_GetClassList ( class, &object_objid);
    while ( ODD(sts))
    {
      /* Store and direct link the initstep */
      sts = rttsys_objectlist_add( object_objid, class, objectlist,
		objectlist_count, object_alloc, attrstr, attrsize);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextObject ( object_objid, &object_objid);
    }

    return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_PLCTHREAD()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Shows errorcounts on device objects.
*
**************************************************************************/

#define THREAD_PAGESIZE 16

static int rttsys_threadobject_add( pwr_tClassId	class,
				rttsys_t_step_list	**objectlist,
				int			*objectlist_count,
				int			*object_alloc,
				char			*attrstr,
				int			attrsize)
{
    pwr_tObjid		object_objid;
    int			sts;
    rttsys_t_step_list	*objectlist_ptr;

    /* Get all initsteps of this node */
    sts = gdh_GetClassList ( class, &object_objid);
    while ( ODD(sts))
    {
      /* Store and direct link the initstep */
      sts = rttsys_objectlist_add( object_objid, class, objectlist,
		objectlist_count, object_alloc, attrstr, attrsize);
      if ( EVEN(sts)) return sts;

      objectlist_ptr = *objectlist;
      objectlist_ptr += *objectlist_count - 1;

      sts = gdh_GetNextObject ( object_objid, &object_objid);
    }

    return RTT__SUCCESS;
}

static int rttsys_thread_update( menu_ctx		ctx,
				rttsys_t_step_list	*objectlist,
				int			objectlist_count,
				int			page)
{
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  int			i;
  rttsys_t_step_list	*objectlist_ptr;
  
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    objectlist_ptr = objectlist + page * THREAD_PAGESIZE;
    for ( i = page * THREAD_PAGESIZE; 
	  i < min( objectlist_count, (page + 1) * THREAD_PAGESIZE) ; i++)
    {
      strncpy( menu_ptr->value_ptr, objectlist_ptr->name, sizeof(Thread_Line1));

      /* Define PF buttons and Return */
      menu_ptr->func = &rtt_hierarchy_child;
      menu_ptr->func2 = &rtt_object_parameters;
      menu_ptr->func3 = &rtt_debug_child;
      menu_ptr->argoi = objectlist_ptr->objid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *)
	&((pwr_sClass_PlcThread *)(objectlist_ptr->value_ptr))->Prio;
      menu_ptr++;
      menu_ptr->value_ptr = (char *)
	&((pwr_sClass_PlcThread *) (objectlist_ptr->value_ptr))->Count;
      menu_ptr++;
      menu_ptr->value_ptr = (char *)
	&((pwr_sClass_PlcThread *) (objectlist_ptr->value_ptr))->ScanTime;
      menu_ptr++;
      menu_ptr->value_ptr = 
	(char *) &((pwr_sClass_PlcThread *) (objectlist_ptr->value_ptr))->ScanTimeMean;
      menu_ptr++;
      menu_ptr->value_ptr = 
	(char *) &((pwr_sClass_PlcThread *) (objectlist_ptr->value_ptr))->Coverage;
      menu_ptr++;
      menu_ptr->value_ptr = 
	(char *) &((pwr_sClass_PlcThread *) (objectlist_ptr->value_ptr))->Count_1_8;
      menu_ptr++;
      menu_ptr->value_ptr = 
	(char *) &((pwr_sClass_PlcThread *) (objectlist_ptr->value_ptr))->Count_1_4;
      menu_ptr++;
      objectlist_ptr++;
    }
    for ( i = i; 
	  i < (page + 1) * THREAD_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
    }
    return RTT__SUCCESS;
}

static int rttsys_objectlist_modname( 	rttsys_t_step_list	*objectlist,
				int			objectlist_count)
{
  rttsys_t_step_list	*objectlist_ptr;
  int			i;
  char			*s;

  objectlist_ptr = objectlist;
  for ( i = 0; i < objectlist_count; i++)
  {
    s = strrchr( objectlist_ptr->name, '-');
    if ( s != 0)
    {
      s++;
      strcpy( objectlist_ptr->name, s);
    }
    objectlist_ptr++;
  }
  return RTT__SUCCESS;
}

int RTTSYS_PLCTHREAD(	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
  int			i;
  int			sts;
  static int		page;	
  static rttsys_t_step_list	*objectlist;
  static rttsys_t_step_list	*objectlist_ptr;
  static int			objectlist_count;
  int			object_alloc;

  IF_NOGDH_RETURN;
  /**********************************************************
  *	Return address of menu
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p16_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p16_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    page--;
    page = max( page, 0);
    THREAD_PAGE = page + 1;
    rttsys_thread_update( ctx, objectlist, objectlist_count, page);
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    page++;
    page = min( page, THREAD_MAXPAGE - 1);
    THREAD_PAGE = page + 1;
    rttsys_thread_update( ctx, objectlist, objectlist_count, page);
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    objectlist_count = 0;
    page = 0;
    THREAD_PAGE = page + 1;

    sts = rttsys_threadobject_add( pwr_cClass_PlcThread, &objectlist, 
		&objectlist_count, &object_alloc, "", 
		sizeof(pwr_sClass_PlcThread));
    if ( EVEN(sts)) return sts;
    rttsys_objectlist_modname( objectlist, objectlist_count);

    rttsys_steplist_bubblesort( objectlist, objectlist_count);

    rttsys_thread_update( ctx, objectlist, objectlist_count, page);
    THREAD_MAXPAGE = max( 1, (objectlist_count - 1)/ THREAD_PAGESIZE + 1);
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    /* Free the objectlist */
    objectlist_ptr = objectlist;
    for ( i = 0; i < objectlist_count; i++)
    {
      sts = gdh_DLUnrefObjectInfo ( objectlist_ptr->subid);
      objectlist_ptr++;
    }
    free( objectlist);
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {

  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_PID()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show PID objects.
*
**************************************************************************/

#define PID_PAGESIZE 18

static int rttsys_pid_object_start( 	menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	char	objectname[80];
	int	sts;
	
	sts = gdh_ObjidToName ( objid, objectname, sizeof(objectname), cdh_mNName);
	if ( EVEN(sts)) return sts;

	sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, objectname, objectname, 
		0, &RTTSYS_OBJECT_PID);
	return sts;
}

static int rttsys_pid_update( 	menu_ctx		ctx,
				rttsys_t_step_list	*objectlist,
				int			objectlist_count,
				int			page)
{
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  int			i;
  rttsys_t_step_list	*objectlist_ptr;
  
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    objectlist_ptr = objectlist + page * PID_PAGESIZE * 2;
    for ( i = page * PID_PAGESIZE; 
	  i < min( objectlist_count, (page + 1) * PID_PAGESIZE) ; i++)
    {
      strncpy( menu_ptr->value_ptr, objectlist_ptr->name,
		sizeof( PID_LINE1));

      /* Define PF buttons and Return */
      menu_ptr->func = &rttsys_pid_object_start;
      menu_ptr->func2 = &rtt_object_parameters;
      menu_ptr->func3 = &rtt_debug_child;
      menu_ptr->argoi = objectlist_ptr->objid;
      menu_ptr++;
      /* OpMode in the mode object */
      objectlist_ptr++;
      if ( objectlist_ptr->value_ptr != 0)
      {
	if( ((pwr_sClass_mode *) (objectlist_ptr->value_ptr))->OpMod == 1)
          strcpy( menu_ptr->value_ptr, "Man ");
	else if( ((pwr_sClass_mode *) (objectlist_ptr->value_ptr))->OpMod == 2)
          strcpy( menu_ptr->value_ptr, "Auto");
	else if( ((pwr_sClass_mode *) (objectlist_ptr->value_ptr))->OpMod == 4)
          strcpy( menu_ptr->value_ptr, "Casc");
	else
          strcpy( menu_ptr->value_ptr, "");
      }
      else
        strcpy( menu_ptr->value_ptr, "");

      objectlist_ptr--;
      menu_ptr++;
      menu_ptr->value_ptr = 
	(char *) &((pwr_sClass_pid *) (objectlist_ptr->value_ptr))->Force;
      menu_ptr++;
      menu_ptr->value_ptr = 
	(char *) &((pwr_sClass_pid *) (objectlist_ptr->value_ptr))->ProcVal;
      menu_ptr++;
      menu_ptr->value_ptr = 
	(char *) &((pwr_sClass_pid *) (objectlist_ptr->value_ptr))->SetVal;
      menu_ptr++;
      menu_ptr->value_ptr = 
	(char *) &((pwr_sClass_pid *) (objectlist_ptr->value_ptr))->OutVal;
      menu_ptr++;
      objectlist_ptr++;
      objectlist_ptr++;
    }
    for ( i = i; 
	  i < (page + 1) * PID_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
    }
    return RTT__SUCCESS;
}

static int rttsys_objectlist_modname_plc( rttsys_t_step_list	*objectlist,
				int			objectlist_count)
{
  rttsys_t_step_list	*objectlist_ptr;
  int			i;
  char			*s;
  pwr_tObjid		plc_objid;
  int			offs;
  char			namebuf[80];
  int			sts;

  objectlist_ptr = objectlist;
  for ( i = 0; i < objectlist_count; i++)
  {
    rttsys_get_plcpgm( objectlist_ptr->objid, &plc_objid);
    sts = gdh_ObjidToName ( plc_objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) 
    {
      objectlist_ptr++;
      continue;
    }
    s = strrchr( namebuf, '-');
    if ( s != 0)
    {
      s++;
      offs = s - namebuf;
      s = objectlist_ptr->name + offs;
      strcpy( objectlist_ptr->name, s);
    }
    objectlist_ptr++;
    objectlist_ptr++;
  }
  return RTT__SUCCESS;
}

static int rttsys_pidobject_add( pwr_tClassId		class,
				rttsys_t_step_list	**objectlist,
				int			*objectlist_count,
				int			*object_alloc,
				char			*attrstr,
				int			attrsize)
{
    pwr_tObjid		object_objid;
    int			sts;
    rttsys_t_step_list	*objectlist_ptr;

    /* Get all initsteps of this node */
    sts = gdh_GetClassList ( class, &object_objid);
    while ( ODD(sts))
    {
      /* Store and direct link the pidobject */
      sts = rttsys_objectlist_add( object_objid, class, objectlist,
		objectlist_count, object_alloc, attrstr, attrsize);
      if ( EVEN(sts)) return sts;
      objectlist_ptr = *objectlist;
      objectlist_ptr += *objectlist_count -1;

      /* Store the mode object also */
      sts = rttsys_objectlist_add(
		((pwr_sClass_pid *) (objectlist_ptr->value_ptr))->ModeObjDId,
		pwr_cClass_mode, objectlist, objectlist_count, object_alloc, "",
		sizeof(pwr_sClass_mode));
      if ( EVEN(sts))
      {
        /* rttsys_objectlist_add has allocated memory.. mark that 
	   the mode object did not exist by setting value_ptr to zero */
	(*objectlist_count)++;
	objectlist_ptr++;
	objectlist_ptr->value_ptr = 0;
      }
      sts = gdh_GetNextObject ( object_objid, &object_objid);
    }

    return RTT__SUCCESS;
}

int RTTSYS_PID( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
  int			i;
  int			sts;
  static int		page;	
  static rttsys_t_step_list	*objectlist;
  static rttsys_t_step_list	*objectlist_ptr;
  static int			objectlist_count;
  int			object_alloc;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	Return address of menu
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture =  (char *) &dtt_systempicture_p17_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p17_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    page--;
    page = max( page, 0);
    PID_PAGE = page + 1;
    rttsys_pid_update( ctx, objectlist, objectlist_count, page);
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    page++;
    page = min( page, PID_MAXPAGE - 1);
    PID_PAGE = page + 1;
    rttsys_pid_update( ctx, objectlist, objectlist_count, page);
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    objectlist_count = 0;
    page = 0;
    PID_PAGE = page + 1;

    sts = rttsys_pidobject_add( pwr_cClass_pid, &objectlist, 
		&objectlist_count, &object_alloc, "", 
		sizeof(pwr_sClass_pid));
    if ( EVEN(sts)) return sts;
    objectlist_count /= 2;
    rttsys_objectlist_modname_plc( objectlist, objectlist_count);

/*    rttsys_steplist_bubblesort( objectlist, objectlist_count);
*/
    rttsys_pid_update( ctx, objectlist, objectlist_count, page);
    PID_MAXPAGE = max( 1, (objectlist_count - 1)/ PID_PAGESIZE + 1);
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    /* Free the objectlist */
    objectlist_ptr = objectlist;
    for ( i = 0; i < objectlist_count; i++)
    {
      if ( objectlist_ptr->value_ptr != 0)
        sts = gdh_DLUnrefObjectInfo ( objectlist_ptr->subid);
      objectlist_ptr++;
    }
    free( objectlist);
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {

  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_LOGGING()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show logging info.
*
**************************************************************************/

int rttsys_get_current_logg_entry( int *entry)
{
	*entry = logging_page + 1;
	return RTT__SUCCESS;
}

int RTTSYS_LOGGING( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

#define SHOW_NODES_PAGESIZE 10
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  char			filename[80];
  char			command[80];
  char			buffer[8];
	  
  IF_NOGDH_RETURN;

  /**********************************************************
  *	Return address of menu
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p18_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p18_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    logging_page--;
    logging_page = max( logging_page, 0);
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    logging_page++;
    logging_page = min( logging_page, 9);
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    logging_page = 0;
  }

  if (( event == RTT_APPL_INIT) ||
      ( event == RTT_APPL_NEXTPAGE) ||
      ( event == RTT_APPL_PREVPAGE))
  {
    LOGGING_ENTRY = logging_page + 1;
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;

    if ( rtt_loggtable[logging_page].occupied == 0)
      sts = rtt_logging_create( ctx, logging_page+1, 0, NULL, NULL, NULL, 0, 0, 
		0, -1, -1, 0, -1); 

    rtt_edit_unselect( ctx);
    if ( rtt_loggtable[logging_page].active )
      ctx->current_item = 18;
    else
      ctx->current_item = 0;
    rtt_edit_select( ctx);

    /* Time */
    if ( rtt_loggtable[logging_page].logg_time == 0)
      rtt_loggtable[logging_page].logg_time = 1000;
    menu_ptr->value_ptr = (char *) &rtt_loggtable[logging_page].logg_time;
    menu_ptr++;

    /* File */
    menu_ptr->value_ptr = (char *) &rtt_loggtable[logging_page].logg_filename;
    menu_ptr++;

    /* Type */
    if ( rtt_loggtable[logging_page].logg_type == 0 )
      rtt_loggtable[logging_page].logg_type = RTT_LOGG_CONT;
    if ( rtt_loggtable[logging_page].logg_type == RTT_LOGG_MOD )
      strcpy( menu_ptr->value_ptr, "Event");
    else if ( rtt_loggtable[logging_page].logg_type == RTT_LOGG_CONT )
      strcpy( menu_ptr->value_ptr, "Cont");
    else
      strcpy( menu_ptr->value_ptr, "");
    menu_ptr++;

    /* Buffer size */
    if ( rtt_loggtable[logging_page].buffer_size == 0)
      rtt_loggtable[logging_page].buffer_size = RTT_BUFFER_DEFSIZE;
    menu_ptr->value_ptr = (char *) &rtt_loggtable[logging_page].buffer_size;
    menu_ptr++;

    /* Insert */
    menu_ptr++;

    /* Active */
    if ( rtt_loggtable[logging_page].active )
      strcpy( menu_ptr->value_ptr, "*** ACTIVE ***");
    else
      strcpy( menu_ptr->value_ptr, " Not active");
    menu_ptr++;


    /* Attributes */
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;

    /* Condition attribute */
    strcpy( menu_ptr->value_ptr, rtt_loggtable[logging_page].conditionstr);
    menu_ptr++;

    /* Start, stop, store */
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;

    /* Priority */
    menu_ptr->value_ptr = (char *) &rtt_loggtable[logging_page].logg_priority;
    menu_ptr++;

    /* Buffer intern */
    menu_ptr->value_ptr = &rtt_loggtable[logging_page].intern;
    menu_ptr++;

    /* Show file, restore, more */
    menu_ptr++;
    menu_ptr++;
    menu_ptr++;

    /* Short name of parameters */
    menu_ptr->value_ptr = (char *) &rtt_loggtable[logging_page].print_shortname;
    menu_ptr++;

    /* Number of parameters */
    menu_ptr->value_ptr = (char *) &rtt_loggtable[logging_page].parameter_count;

    strcpy( LOGGING_ATTR1, rtt_loggtable[logging_page].parameterstr[0]);
    strcpy( LOGGING_ATTR2, rtt_loggtable[logging_page].parameterstr[1]);
    strcpy( LOGGING_ATTR3, rtt_loggtable[logging_page].parameterstr[2]);
    strcpy( LOGGING_ATTR4, rtt_loggtable[logging_page].parameterstr[3]);
    strcpy( LOGGING_ATTR5, rtt_loggtable[logging_page].parameterstr[4]);
    strcpy( LOGGING_ATTR6, rtt_loggtable[logging_page].parameterstr[5]);
    strcpy( LOGGING_ATTR7, rtt_loggtable[logging_page].parameterstr[6]);
    strcpy( LOGGING_ATTR8, rtt_loggtable[logging_page].parameterstr[7]);
    strcpy( LOGGING_ATTR9, rtt_loggtable[logging_page].parameterstr[8]);
    strcpy( LOGGING_ATTR10, rtt_loggtable[logging_page].parameterstr[9]);
    strcpy( LOGGING_COND, rtt_loggtable[logging_page].conditionstr);

  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {

  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    /* Type */
    if ( rtt_loggtable[logging_page].logg_type == RTT_LOGG_MOD )
      strcpy( LOGGING_TYPE, "Event");
    else if ( rtt_loggtable[logging_page].logg_type == RTT_LOGG_CONT )
      strcpy( LOGGING_TYPE, "Cont");
    else
      strcpy( LOGGING_TYPE, "");

    if ( rtt_loggtable[logging_page].active )
    {
      if ( LOGGING_ACTIVE[0] == ' ')
        strcpy( LOGGING_ACTIVE, "*** ACTIVE ***");
      else
        strcpy( LOGGING_ACTIVE, "    ******    ");
    }
    else
      strcpy( LOGGING_ACTIVE, " Not active");

    strcpy( LOGGING_ATTR1, rtt_loggtable[logging_page].parameterstr[0]);
    strcpy( LOGGING_ATTR2, rtt_loggtable[logging_page].parameterstr[1]);
    strcpy( LOGGING_ATTR3, rtt_loggtable[logging_page].parameterstr[2]);
    strcpy( LOGGING_ATTR4, rtt_loggtable[logging_page].parameterstr[3]);
    strcpy( LOGGING_ATTR5, rtt_loggtable[logging_page].parameterstr[4]);
    strcpy( LOGGING_ATTR6, rtt_loggtable[logging_page].parameterstr[5]);
    strcpy( LOGGING_ATTR7, rtt_loggtable[logging_page].parameterstr[6]);
    strcpy( LOGGING_ATTR8, rtt_loggtable[logging_page].parameterstr[7]);
    strcpy( LOGGING_ATTR9, rtt_loggtable[logging_page].parameterstr[8]);
    strcpy( LOGGING_ATTR10, rtt_loggtable[logging_page].parameterstr[9]);
    strcpy( LOGGING_COND, rtt_loggtable[logging_page].conditionstr);

  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {

    if ( parameter_ptr == (char *) &LOGGING_TYPE)
    {
      rtt_toupper( LOGGING_TYPE, LOGGING_TYPE);
      if (strcmp( LOGGING_TYPE, "EVENT") == 0)
      {
        strcpy( LOGGING_TYPE, "Event");
	rtt_loggtable[logging_page].logg_type = RTT_LOGG_MOD;
      }
      else if (strcmp( LOGGING_TYPE, "CONT") == 0)
      {
        strcpy( LOGGING_TYPE, "Cont");
	rtt_loggtable[logging_page].logg_type = RTT_LOGG_CONT;
      }
      else
        rtt_message('E', "Unknown type, type can be event or cont");
    } 
    else if ( parameter_ptr == (char *) &LOGGING_INSERT)
    {
      sts = rtt_logging_set( ctx, logging_page + 1, 0, NULL, NULL, NULL, 0, 1, 0, -1, 
	-1, 0, 0, -1); 
    }
    else if ( parameter_ptr == (char *) &LOGGING_START)
    {
      /* Set buffersize to be sure to allocate space for buffer */
      sts = rtt_logging_set( ctx, logging_page + 1, 0, NULL, NULL, NULL, 0, 0, 
	rtt_loggtable[logging_page].buffer_size, -1, -1, 0, 0, -1); 
      sts = rtt_logging_start( ctx, logging_page + 1);
    }
    else if ( parameter_ptr == (char *) &LOGGING_STOP)
    {
      sts = rtt_logging_stop( ctx, logging_page + 1);
    }
    else if ( parameter_ptr == (char *) &LOGGING_SHOW_FILE)
    {
      sts = rtt_view( ctx,  rtt_loggtable[logging_page].logg_filename, 0, 0, 
		RTT_VIEWTYPE_FILE);
      if ( EVEN(sts)) return sts;
      if ( sts != RTT__NOPICTURE)
      {
        rtt_edit_draw( ctx, (rtt_t_backgr *) &dtt_systempicture_p18_bg);
        rtt_edit_select( ctx);
        ctx->update_init = 1;
        rtt_menu_edit_update( ctx);
      }
    }
    else if ( parameter_ptr == (char *) &LOGGING_STORE)
    {
      sprintf( filename, "rtt_store_logg%d", logging_page + 1);
      sts = rtt_logging_store_entry( logging_page + 1, filename);
      return RTT__NOPICTURE;
    }
    else if ( parameter_ptr == (char *) &LOGGING_RESTORE)
    {
      sprintf( command, "@rtt_store_logg%d", logging_page + 1);
      sts = rtt_menu_command( ctx, pwr_cNObjid, command, 0,0,0);
      if ( EVEN(sts)) return RTT__NOPICTURE;
    }
    else if ( parameter_ptr == (char *) &LOGGING_MORE)
    {
      sts = rtt_logging_show( ctx, logging_page + 1);
      if ( EVEN(sts)) return sts;
      if ( sts != RTT__NOPICTURE)
      {
        rtt_edit_draw( ctx, (rtt_t_backgr *) &dtt_systempicture_p18_bg);
        rtt_edit_select( ctx);
        ctx->update_init = 1;
        rtt_menu_edit_update( ctx);
      }
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR1)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[0], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR2)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[1], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR3)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[2], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR4)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[3], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR5)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[4], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR6)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[5], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR7)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[6], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR8)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[7], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR9)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[8], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_ATTR10)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Parameter doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].parameterstr[9], parameter_ptr);
    }
    else if ( parameter_ptr == (char *) &LOGGING_COND)
    {
      sts = gdh_GetObjectInfo( parameter_ptr, &buffer, sizeof(buffer)); 
      if (EVEN(sts))
      {
        rtt_message('E',"Condition doesn't exist");
        return RTT__NOPICTURE;
      }
      strcpy( rtt_loggtable[logging_page].conditionstr, parameter_ptr);
    }
  }

  return RTT__SUCCESS;
}



/*************************************************************************
*
* Name:		RTTSYS_SHOW_SYS()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show proview LYNX processes.
*
**************************************************************************/

#if defined OS_LYNX || defined OS_LINUX

int RTTSYS_SHOW_SYS( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{
	rtt_message('E', "Not yet implemented");
	return RTT__NOPICTURE;
}
#endif

/*************************************************************************
*
* Name:		RTTSYS_VMSSYS()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show proview VMS processes.
*
**************************************************************************/

#ifdef OS_VMS

int RTTSYS_SHOW_SYS( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{

#define VMSSYS_MAXSIZE 18
#define VMSSYS_PAGESIZE 18

  int			i;
  int			sts;
  static int		page;	
  short 		uic[2];
  int			proc_pid[VMSSYS_MAXSIZE];
  static pwr_tString32	proc_name[VMSSYS_MAXSIZE];
  static pwr_tString16	proc_pidstr[VMSSYS_MAXSIZE];
  static int		proc_pri[VMSSYS_MAXSIZE];
  static pwr_tString16	proc_state[VMSSYS_MAXSIZE];
  static float		proc_cputim[VMSSYS_MAXSIZE];
  static int		proc_count;
  rtt_t_menu_upd	*menu_ptr;
  unsigned long		ident;

  switch ( event)
  {
    /**********************************************************
    *	Return address of background
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p21_bg;
      return RTT__SUCCESS;

    /**********************************************************
    * 	Return adress of menu
    ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p21_eu;
      return RTT__SUCCESS;

    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_PREVPAGE:
      return RTT__SUCCESS;

    /**********************************************************
    *	Next page
    ***********************************************************/
    case RTT_APPL_NEXTPAGE:
      return RTT__SUCCESS;

    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    case RTT_APPL_INIT:
      sts = rttvms_get_nodename( VMSSYS_NODE, sizeof( VMSSYS_NODE));
      if ( EVEN(sts)) return sts;

      sts = rttvms_get_uic( (int *) &uic);
      if ( EVEN(sts)) return sts;
      sprintf ( VMSSYS_UIC, "%o", uic[1]);

      memcpy( &ident, &uic, 4);
      sts = rttvms_get_identname( ident, VMSSYS_PROJ);

      /* Continue with update... */

    /**********************************************************
    *	Update the picture.
    ***********************************************************/
    case RTT_APPL_UPDATE:
      sts = rttvms_get_pwr_proc( proc_pid, proc_name, proc_pidstr, proc_pri, 
	proc_state, proc_cputim, VMSSYS_MAXSIZE, &proc_count);
      if ( EVEN(sts)) return sts;

      menu_ptr = (rtt_t_menu_upd *) ctx->menu;
      for ( i = 0; i < proc_count; i++)
      {
        menu_ptr->value_ptr = (char *) proc_name[i];
        menu_ptr->func = NULL;
        menu_ptr->func2 = &rtt_menu_new_sysedit;
        menu_ptr->func3 = NULL;
        menu_ptr->arg1 = (void *) proc_pid[i];
        menu_ptr->arg2 = proc_pidstr[i];
        menu_ptr->arg3 = 0;
        menu_ptr->arg4 = (void *) RTTSYS_VMSPROC;
        menu_ptr++;
	if ( 	!strcmp(proc_name[i], "Not started") ||
		!strcmp(proc_name[i], "No privileges"))
	{
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
	}
	else
	{
          menu_ptr->value_ptr = (char *) proc_pidstr[i];
          menu_ptr++;
          menu_ptr->value_ptr = (char *) &proc_pri[i];
          menu_ptr++;
          menu_ptr->value_ptr = (char *) proc_state[i];
          menu_ptr++;
          menu_ptr->value_ptr = (char *) &proc_cputim[i];
          menu_ptr++;
        }
      }
      for ( i = proc_count; i < VMSSYS_PAGESIZE; i++)
      {
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr->func2 = NULL;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      return RTT__SUCCESS;
    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      return RTT__SUCCESS;

  }
  return RTT__SUCCESS;
}
#endif

/*************************************************************************
*
* Name:		RTTSYS_VMSPROC()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show a VMS process.
*
**************************************************************************/

int RTTSYS_VMSPROC( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
#ifdef OS_ELN
    rtt_message('E', "Picture is not implemented in ELN");
    return RTT__NOPICTURE;
#elif OS_LYNX || defined OS_LINUX
    rtt_message('E', "Picture is not implemented in LYNX");
    return RTT__NOPICTURE;
#elif OS_VMS

#define VMSSYS_MAXSIZE 18
#define VMSSYS_PAGESIZE 18

  int			i;
  int			sts;
  static int		pid;
  static int		page;	
  short 		uic[2];
  rtt_t_menu_upd	*menu_ptr;
  unsigned long		ident;
  pwr_tTime		logintim;
  int			cputim;
  int			timlen;
  char			timbuf[64];
  struct dsc$descriptor_s	timbuf_desc = {sizeof(timbuf)-1,DSC$K_DTYPE_T,
					DSC$K_CLASS_S,};

  timbuf_desc.dsc$a_pointer = timbuf;

  switch ( event)
  {
    /**********************************************************
    *	Return address of background
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p22_bg;
      return RTT__SUCCESS;

    /**********************************************************
    * 	Return adress of menu
    ***********************************************************/
    case RTT_APPL_MENU:
      pid = (int) objectname;
      if ( !pid) 
      {
        rtt_message('E', "No access to process");
        return RTT__NOPICTURE;
      }
      *picture = (char *) &dtt_systempicture_p22_eu;
      return RTT__SUCCESS;

    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_PREVPAGE:
      return RTT__SUCCESS;

    /**********************************************************
    *	Next page
    ***********************************************************/
    case RTT_APPL_NEXTPAGE:
      return RTT__SUCCESS;

    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    case RTT_APPL_INIT:
      sts = rttvms_get_procinfo_full( pid, 
					VMSPROC_STATE,
				 	&VMSPROC_PRI,
					&VMSPROC_PRIB,
				 	&cputim,
					(int *) &logintim,
					(int *) &uic,
					&VMSPROC_BUFIO,
					VMSPROC_IMAGNAME,
					&VMSPROC_GPGCNT,
					&VMSPROC_JOPRCCNT,
					&VMSPROC_PAGEFLTS,
					&VMSPROC_PAGFILCNT,
					&VMSPROC_PGFLQUOTA,
					VMSPROC_PRCNAM,
					&VMSPROC_FILLM,
					&VMSPROC_VIRTPEAK,
					&VMSPROC_WSPEAK,
					&VMSPROC_WSQUOTA,
					&VMSPROC_WSEXTENT,
					&VMSPROC_WSSIZE,
					&VMSPROC_PPGCNT,
					&VMSPROC_DIRIO);

      sprintf ( VMSPROC_UIC1, "%o", uic[1]);
      sprintf ( VMSPROC_UIC2, "%03.3o", uic[0]);

      memcpy( &ident, &uic, 4);
      sts = rttvms_get_identname( ident, VMSPROC_IDENT);
      sprintf ( VMSPROC_PID, "%lx", pid);

      return RTT__SUCCESS;

    /**********************************************************
    *	Update the picture.
    ***********************************************************/
    case RTT_APPL_UPDATE:
      sts = rttvms_get_procinfo_full( pid, 
					VMSPROC_STATE,
				 	&VMSPROC_PRI,
					&VMSPROC_PRIB,
				 	&cputim,
					(int *) &logintim,
					(int *) &uic,
					&VMSPROC_BUFIO,
					VMSPROC_IMAGNAME,
					&VMSPROC_GPGCNT,
					&VMSPROC_JOPRCCNT,
					&VMSPROC_PAGEFLTS,
					&VMSPROC_PAGFILCNT,
					&VMSPROC_PGFLQUOTA,
					VMSPROC_PRCNAM,
					&VMSPROC_FILLM,
					&VMSPROC_VIRTPEAK,
					&VMSPROC_WSPEAK,
					&VMSPROC_WSQUOTA,
					&VMSPROC_WSEXTENT,
					&VMSPROC_WSSIZE,
					&VMSPROC_PPGCNT,
					&VMSPROC_DIRIO);
      if ( EVEN(sts))
      {
        strcpy( VMSPROC_PID, "");
        strcpy( VMSPROC_IDENT, "");
        strcpy( VMSPROC_STATE, "");
        VMSPROC_PRI = 0;
        VMSPROC_PRIB = 0;
        VMSPROC_CPUTIM = 0;
        strcpy( VMSPROC_LOGINTIM, "");
        strcpy( VMSPROC_UIC1, "");
        strcpy( VMSPROC_UIC2, "");
        VMSPROC_BUFIO = 0;
        strcpy( VMSPROC_IMAGNAME, "");
        VMSPROC_GPGCNT = 0;
        VMSPROC_JOPRCCNT = 0;
        VMSPROC_PAGEFLTS = 0;
        VMSPROC_PAGFILCNT = 0;
        VMSPROC_PGFLQUOTA = 0;
        strcpy( VMSPROC_PRCNAM, "No access to process");
        VMSPROC_FILLM = 0;
        VMSPROC_VIRTPEAK = 0;
        VMSPROC_WSPEAK = 0;
        VMSPROC_WSQUOTA = 0;
        VMSPROC_WSEXTENT = 0;
        VMSPROC_WSSIZE = 0;
        VMSPROC_PPGCNT = 0;
        VMSPROC_DIRIO = 0;
      }
      else
      {
        sts = sys$asctim( &timlen, &timbuf_desc, &logintim, 0);
        strcpy( VMSPROC_LOGINTIM, timbuf);
        VMSPROC_CPUTIM = (float)cputim / 100.;
      }
      return RTT__SUCCESS;
    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      return RTT__SUCCESS;

  }
#endif
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_ELNSYS()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show proview ELN processes.
*
**************************************************************************/

#ifdef OS_ELN
static int rttsys_cpu( int key, pwr_tTime *time, float cputim, 
		float *cpu_current, float *cpu_mean, int init)
{
#define RTTSYS_LOWKEY 100
#define RTTSYS_HIGHKEY 100
	int			i;
	int			sts;
	int			found;
	static t_VAXTime	start_time[RTTSYS_LOWKEY+RTTSYS_HIGHKEY];
	static t_VAXTime	last_time[RTTSYS_LOWKEY+RTTSYS_HIGHKEY];
	static float		start_cputim[RTTSYS_LOWKEY+RTTSYS_HIGHKEY];
	static float		last_cputim[RTTSYS_LOWKEY+RTTSYS_HIGHKEY];
	static short		key_array[RTTSYS_HIGHKEY];
	static int		key_count;
	int			index;
	short			timbuf[7];
	float			time_since_start;
	float			time_since_last;
	t_VAXTime		time_diff;

	if (init)
	{
	  memset( &start_time, 0, sizeof( start_time));
	  memset( &last_time, 0, sizeof( last_time));
	  memset( &start_cputim, 0, sizeof( start_cputim));
	  memset( &last_cputim, 0, sizeof( last_cputim));
	  key_count = 0;
          return RTT__SUCCESS;
	}

	/* Get the index */
	if ( key < RTTSYS_LOWKEY)
	  index = key;
	else
	{
	  /* Search in key array */
	  found = 0;
	  for ( i = 0; i < key_count; i++)
	  {
	    if ( key_array[i] == key)
	    {
              found = 1;
	      index = i + RTTSYS_LOWKEY;
              break;
            }
	  }
	  if ( !found)
	  {
	    if ( key_count >= RTTSYS_HIGHKEY)
	    {
	      /* Max size exceeded */
	      *cpu_current = -1;
	      *cpu_mean = -1;
	      return 0;
	    }
	    index = key_count + RTTSYS_LOWKEY;
	    key_array[ key_count] = key;
	    key_count++;
	  }
	}
	if ( start_time[index].high == 0 && start_time[index].low == 0)
	{
	  /* New key */
	  memcpy( &start_time[index], time, sizeof(*time));
	  memcpy( &last_time[index], time, sizeof(*time));
	  start_cputim[index] = cputim;
	  last_cputim[index] = cputim;
	  *cpu_current = 0;
	  *cpu_mean = 0;
	}
	else
	{
	  /* Time since start */
          sts = lib$sub_times( time, &start_time[index], &time_diff);
          sys$numtim( &timbuf, &time_diff);
          time_since_start = timbuf[6]/100. + timbuf[5]+
		60.*(timbuf[4] + 60.*(timbuf[3] + 24.*timbuf[2]));

	  /* Time since last */
          sts = lib$sub_times( time, &last_time[index], &time_diff);
          sys$numtim( &timbuf, &time_diff);
          time_since_last = timbuf[6]/100. + timbuf[5]+
		60.*(timbuf[4] + 60.*(timbuf[3] + 24.*timbuf[2]));

	  *cpu_mean = (cputim - start_cputim[index]) / time_since_start * 100;
	  *cpu_current = (cputim - last_cputim[index]) / time_since_last * 100;

	  memcpy( &last_time[index], time, sizeof(*time));
	  last_cputim[index] = cputim;
	}
	return RTT__SUCCESS;
} 

int RTTSYS_SHOW_SYS( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

#define VMSSYS_MAXSIZE 54
#define VMSSYS_PAGESIZE 18

  int			i;
  int			sts;
  static int		page;	
  short 		uic[2];
  float			total_cputim;
  static int		proc_pid[VMSSYS_MAXSIZE];
  static pwr_tString32	proc_name[VMSSYS_MAXSIZE];
  static int		proc_pri[VMSSYS_MAXSIZE];
  static pwr_tString16	proc_state[VMSSYS_MAXSIZE];
  static float		proc_cputim[VMSSYS_MAXSIZE];
  static float		proc_cputim_rel[VMSSYS_MAXSIZE];
  static float		proc_cputim_mean[VMSSYS_MAXSIZE];
  static int		proc_count;
  rtt_t_menu_upd	*menu_ptr;
  pwr_tTime		uptime;
  short			timbuf[7];

  switch ( event)
  {
    /**********************************************************
    *	Return address of background
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = &dtt_systempicture_p23_bg;
      return RTT__SUCCESS;

    /**********************************************************
    * 	Return adress of menu
    ***********************************************************/
    case RTT_APPL_MENU:
      *picture = &dtt_systempicture_p23_eu;
      return RTT__SUCCESS;

    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_PREVPAGE:
      page--;
      page = max( page, 0);
      return RTT__SUCCESS;

    /**********************************************************
    *	Next page
    ***********************************************************/
    case RTT_APPL_NEXTPAGE:
      page++;
      page = min( page, (proc_count - 1)/VMSSYS_PAGESIZE );
      return RTT__SUCCESS;

    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    case RTT_APPL_INIT:
      page = 0; 
      sts = rttsys_cpu( 0,0,0,0,0,1);

      /* Continue with update... */

    /**********************************************************
    *	Update the picture.
    ***********************************************************/
    case RTT_APPL_UPDATE:
      sts = rtteln_get_jobs( proc_pid, proc_name, proc_pri, 
	proc_state, proc_cputim, VMSSYS_MAXSIZE, &proc_count);
      if ( EVEN(sts)) return sts;

      ker$get_uptime( &sts, &uptime);

      /* Calculate total cpu time */
      total_cputim = 0;
      for ( i = 0; i < proc_count; i++)
        total_cputim += proc_cputim[i];

      rttsys_cpu( 1000, &uptime, total_cputim,
		&ELNSYS_CPUTIM_REL, &ELNSYS_CPUTIM_MEAN, 0);

      menu_ptr = ctx->menu;
      for ( i = page * VMSSYS_PAGESIZE ; 
		i < min( proc_count, (page+1)*VMSSYS_PAGESIZE) ; i++)
      {
        menu_ptr->value_ptr = &proc_name[i];
        menu_ptr->func = NULL;
        menu_ptr->func2 = &rtt_menu_new_sysedit;
        menu_ptr->func3 = NULL;
        menu_ptr->arg1 = (void *) proc_pid[i];
        menu_ptr->arg2 = &proc_name[i];
        menu_ptr->arg3 = 0;
        menu_ptr->arg4 = &RTTSYS_ELNPROC;
        menu_ptr++;
        menu_ptr->value_ptr = &proc_pid[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_pri[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_state[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_cputim[i];
        menu_ptr++;
	rttsys_cpu( proc_pid[i], &uptime, proc_cputim[i],
		&proc_cputim_rel[i], &proc_cputim_mean[i], 0);
        menu_ptr->value_ptr = &proc_cputim_rel[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_cputim_mean[i];
        menu_ptr++;
      }
      for ( i = min( proc_count, (page+1)*VMSSYS_PAGESIZE);
		i < (page+1)*VMSSYS_PAGESIZE; i++)
      {
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr->func2 = NULL;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      return RTT__SUCCESS;
    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      return RTT__SUCCESS;

  }
  return RTT__SUCCESS;
}
#endif

/*************************************************************************
*
* Name:		RTTSYS_ELNPROC()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show proview ELN processes.
*
**************************************************************************/

#ifdef OS_ELN

static int rttsys_proc_cpu( int key, pwr_tTime *time, float cputim, 
		float *cpu_current, float *cpu_mean, int init)
{
#define RTTSYS_LOWKEY 100
#define RTTSYS_HIGHKEY 100
	int			i;
	int			sts;
	int			found;
	static t_VAXTime	start_time[RTTSYS_LOWKEY+RTTSYS_HIGHKEY];
	static t_VAXTime	last_time[RTTSYS_LOWKEY+RTTSYS_HIGHKEY];
	static float		start_cputim[RTTSYS_LOWKEY+RTTSYS_HIGHKEY];
	static float		last_cputim[RTTSYS_LOWKEY+RTTSYS_HIGHKEY];
	static short		key_array[RTTSYS_HIGHKEY];
	static int		key_count;
	int			index;
	short			timbuf[7];
	float			time_since_start;
	float			time_since_last;
	t_VAXTime		time_diff;

	if (init)
	{
	  memset( &start_time, 0, sizeof( start_time));
	  memset( &last_time, 0, sizeof( last_time));
	  memset( &start_cputim, 0, sizeof( start_cputim));
	  memset( &last_cputim, 0, sizeof( last_cputim));
	  key_count = 0;
          return RTT__SUCCESS;
	}

	/* Get the index */
	if ( key < RTTSYS_LOWKEY)
	  index = key;
	else
	{
	  /* Search in key array */
	  found = 0;
	  for ( i = 0; i < key_count; i++)
	  {
	    if ( key_array[i] == key)
	    {
              found = 1;
	      index = i + RTTSYS_LOWKEY;
              break;
            }
	  }
	  if ( !found)
	  {
	    if ( key_count >= RTTSYS_HIGHKEY)
	    {
	      /* Max size exceeded */
	      *cpu_current = -1;
	      *cpu_mean = -1;
	      return 0;
	    }
	    index = key_count + RTTSYS_LOWKEY;
	    key_array[ key_count] = key;
	    key_count++;
	  }
	}
	if ( start_time[index].high == 0 && start_time[index].low == 0)
	{
	  /* New key */
	  memcpy( &start_time[index], time, sizeof(*time));
	  memcpy( &last_time[index], time, sizeof(*time));
	  start_cputim[index] = cputim;
	  last_cputim[index] = cputim;
	  *cpu_current = 0;
	  *cpu_mean = 0;
	}
	else
	{
	  /* Time since start */
          sts = lib$sub_times( time, &start_time[index], &time_diff);
          sys$numtim( &timbuf, &time_diff);
          time_since_start = timbuf[6]/100. + timbuf[5]+
		60.*(timbuf[4] + 60.*(timbuf[3] + 24.*timbuf[2]));

	  /* Time since last */
          sts = lib$sub_times( time, &last_time[index], &time_diff);
          sys$numtim( &timbuf, &time_diff);
          time_since_last = timbuf[6]/100. + timbuf[5]+
		60.*(timbuf[4] + 60.*(timbuf[3] + 24.*timbuf[2]));

	  *cpu_mean = (cputim - start_cputim[index]) / time_since_start * 100;
	  *cpu_current = (cputim - last_cputim[index]) / time_since_last * 100;

	  memcpy( &last_time[index], time, sizeof(*time));
	  last_cputim[index] = cputim;
	}
	return RTT__SUCCESS;
} 
#endif

int RTTSYS_ELNPROC( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

#if defined OS_VMS || defined OS_LYNX || defined OS_LINUX
    rtt_message('E', "Picture is not implemented for this os");
    return RTT__NOPICTURE;
#elif OS_ELN

#define ELNPROC_MAXSIZE 54
#define ELNPROC_PAGESIZE 13

  int			i;
  int			sts;
  static int		page;
  static int		proc_pid[ELNPROC_MAXSIZE];
  static pwr_tString32	proc_name[ELNPROC_MAXSIZE];
  static int		proc_pri[ELNPROC_MAXSIZE];
  static pwr_tString16	proc_state[ELNPROC_MAXSIZE];
  static float		proc_cputim[ELNPROC_MAXSIZE];
  static float		proc_cputim_rel[ELNPROC_MAXSIZE];
  static float		proc_cputim_mean[ELNPROC_MAXSIZE];
  static int		proc_count;
  rtt_t_menu_upd	*menu_ptr;
  static int		job_nr;
  pwr_tTime		uptime;

  switch ( event)
  {
    /**********************************************************
    *	Return address of background
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = &dtt_systempicture_p24_bg;
      return RTT__SUCCESS;

    /**********************************************************
    * 	Return adress of menu
    ***********************************************************/
    case RTT_APPL_MENU:
      job_nr = objectname;
      if ( !job_nr) 
      {
        rtt_message('E', "No access to job");
        return RTT__NOPICTURE;
      }
      *picture = &dtt_systempicture_p24_eu;
      return RTT__SUCCESS;

    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_PREVPAGE:
      page--;
      page = max( page, 0);
      return RTT__SUCCESS;

    /**********************************************************
    *	Next page
    ***********************************************************/
    case RTT_APPL_NEXTPAGE:
      page++;
      page = min( page, (proc_count - 1)/ELNPROC_PAGESIZE );
      return RTT__SUCCESS;

    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    case RTT_APPL_INIT:
      page = 0; 
      ELNPROC_NR = job_nr;
      ELNPROC_CPUTIM_MAX = 0;
      sts = rttsys_proc_cpu( 0,0,0,0,0,1);
      return RTT__SUCCESS;

    /**********************************************************
    *	Update the picture.
    ***********************************************************/
    case RTT_APPL_UPDATE:
      ker$get_uptime( &sts, &uptime);

      sts = rtteln_get_job_info( 	job_nr, 
					&ELNPROC_NAME,
					&ELNPROC_PRI,
					&ELNPROC_STATE,
					&ELNPROC_CPUTIM,
					&ELNPROC_FILE,
					&ELNPROC_MODE,
					&ELNPROC_USER_STACK,
					&ELNPROC_KERNEL_STACK,
					&ELNPROC_OPTIONS);
      if ( ODD(sts))
      {
	rttsys_cpu( job_nr, &uptime, ELNPROC_CPUTIM,
		&ELNPROC_CPUTIM_REL, &ELNPROC_CPUTIM_MEAN, 0);
        ELNPROC_CPUTIM_MAX = max( ELNPROC_CPUTIM_MAX, ELNPROC_CPUTIM_REL);

        sts = rtteln_get_job_proc( job_nr, proc_pid, proc_name, proc_pri, 
	  proc_state, proc_cputim, ELNPROC_MAXSIZE, &proc_count);
      }
      if ( EVEN(sts)) 
      {
        menu_ptr = ctx->menu;
        for ( i = 0; i < ELNPROC_PAGESIZE; i++)
        {
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
        }
        return RTT__SUCCESS;
      }
      menu_ptr = ctx->menu;
      for ( i = page * ELNPROC_PAGESIZE ; 
		i < min( proc_count, (page+1)*ELNPROC_PAGESIZE) ; i++)
      {
        menu_ptr->value_ptr = &proc_name[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_pid[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_pri[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_state[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_cputim[i];
        menu_ptr++;
	rttsys_proc_cpu( proc_pid[i], &uptime, proc_cputim[i],
		&proc_cputim_rel[i], &proc_cputim_mean[i], 0);
        menu_ptr->value_ptr = &proc_cputim_rel[i];
        menu_ptr++;
        menu_ptr->value_ptr = &proc_cputim_mean[i];
        menu_ptr++;
      }
      for ( i = min( proc_count, (page+1)*ELNPROC_PAGESIZE); 
		i < (page+1)*ELNPROC_PAGESIZE; i++)
      {
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      return RTT__SUCCESS;
    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      return RTT__SUCCESS;

  }
  return RTT__SUCCESS;
#endif
}


/*************************************************************************
*
* Name:		rttsys_start_grafcet_monitor
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Function: 	Start of grafcet monitor from command.
*	
*
**************************************************************************/

int	rttsys_start_grafcet_monitor(
			menu_ctx	ctx,
			pwr_tObjid	plc_objid)
{
  int	sts, return_sts;
  char	plc_name[80];
  int	plc_alloc;
  rttsys_t_plcpgm_list	*plclist_ptr;
  rttsys_t_step_list *initsteplist_ptr;
  int 	i, j;

  sts = gdh_ObjidToName ( plc_objid, plc_name, sizeof( plc_name), cdh_mNName);
  if ( EVEN(sts)) return sts;

  /* Insert plc in plclist */
  plclist_count = 0;
  plc_alloc = 0;
  rttsys_plclist_add( plc_objid, &plclist, &plclist_count, &plc_alloc);
  plclist->selected = 1;
  strcpy( plclist->name, plc_name);

  /* Get the initsteps */
  sts = rtt_get_objects_hier_class_name( ctx, plclist->objid, 
		pwr_cClass_initstep, NULL, 500, 0,
		&rttsys_steplist_add, 
		&plclist->initsteps, 
		&plclist->initstep_count, 
		&plclist->initstep_alloc, 
		(void *) pwr_cClass_initstep, 0);
  if ( EVEN(sts)) return sts;

  /* Start the grafcet monitor */
  return_sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, plc_name, plc_name, 
		0, &RTTSYS_GRAFCET_PLC);

  /* Free the steplists and the plclist */
  plclist_ptr = plclist;
  for ( i = 0; i < plclist_count; i++)
  {
    if ( plclist_ptr->initstep_count > 0)
    {
      initsteplist_ptr = plclist_ptr->initsteps;
      for ( j = 0; j < plclist_ptr->initstep_count; j++)
      {
        sts = gdh_DLUnrefObjectInfo ( initsteplist_ptr->subid);
        if ( EVEN(sts)) return sts;
        initsteplist_ptr++;
      }
      free( plclist_ptr->initsteps);
    }
    plclist_ptr++;
  }
  free( plclist);
  plclist_count = 0;
  return return_sts;
}

/*************************************************************************
*
* Name:		rttsys_start_system_picture
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Function: 	Start of system picture from command.
*	
*
**************************************************************************/

int rttsys_start_system_picture(
			menu_ctx	ctx,
			char		*picture_name)
{
  int	sts;

  /* Start the system picture */
  if ( strncmp( picture_name, "SYSTEM", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "SYSTEM", "SYSTEM", 
		0, &RTTSYS_SHOW_SYS);
    return sts;
  }
  else if ( strncmp( picture_name, "PLCPGM", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "PLCPGM", "PLCPGM", 
		0, &RTTSYS_PLCPGM);
    return sts;
  }
  else if ( strncmp( picture_name, "GRAFCET", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "GRAFCET", "GRAFCET", 
		0, &RTTSYS_GRAFCET);
    return sts;
  }
  else if ( strncmp( picture_name, "DEVICE", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "DEVICE", "DEVICE", 
		0, &RTTSYS_DEVICE);
    return sts;
  }
  else if ( strncmp( picture_name, "PLCTHREAD", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "PLCTHREAD", "PLCTHREAD", 
		0, &RTTSYS_PLCTHREAD);
    return sts;
  }
  else if ( strncmp( picture_name, "PID", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "PID", "PID", 
		0, &RTTSYS_PID);
    return sts;
  }
  else if ( strncmp( picture_name, "LOGGING", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "LOGGING", "LOGGING", 
		0, &RTTSYS_LOGGING);
    return sts;
  }
  else if ( strncmp( picture_name, "NODES", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "NODES", "NODES", 
		0, &RTTSYS_SHOW_NODES);
    return sts;
  }
  else if ( strncmp( picture_name, "SUBCLI", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "SUBCLI", "SUBCLI", 
		0, &RTTSYS_SHOW_SUBCLI);
    return sts;
  }
  else if ( strncmp( picture_name, "SUBSRV", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "SUBSRV", "SUBSRV", 
		0, &RTTSYS_SHOW_SUBSRV);
    return sts;
  }
  else if ( strncmp( picture_name, "NMPSCELL", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "NMPSCELL", "NMPSCELL", 
		0, &RTTSYS_NMPSCELL);
    return sts;
  }
  else if ( strncmp( picture_name, "REMNODE", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "REMNODE", "REMNODE", 
		0, &RTTSYS_REMNODE);
    return sts;
  }
  else if ( strncmp( picture_name, "REMTRANS", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "REMTRANS", "REMTRANS", 
		0, &RTTSYS_REMTRANS);
    return sts;
  }
  else if ( strncmp( picture_name, "RUNNINGTIME", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "RUNNINGTIME", "RUNNINGTIME", 
		0, &RTTSYS_RUNNINGTIME);
    return sts;
  }
  else if ( strncmp( picture_name, "QAPPLICATIONS", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "QCOM APPLICATIONS", 
		"QCOM APPLICATIONS", 0, &RTTSYS_QCOM_APPL);
    return sts;
  }
  else if ( strncmp( picture_name, "QNODES", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "QCOM NODES", 
		"QCOM NODES", 0, &RTTSYS_QCOM_NODES);
    return sts;
  }
  else if ( strncmp( picture_name, "POOLS", strlen( picture_name)) == 0)
  {
    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, "POOLS", 
		"POOLS", 0, &RTTSYS_POOLS);
    return sts;
  }
  else if ( strncmp( picture_name, "ERROR", strlen( picture_name)) == 0)
  {
    rtt_message('I', "Obsolete command, use \"show device\"");
    return RTT__NOPICTURE;
  }
  return 0;
}


/*************************************************************************
*
* Name:		RTTSYS_OBJECT_CELL
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Function: 	Start of system picture from command.
*	
*
**************************************************************************/

#define CELLOBJ_PAGESIZE 15

static int	rttsys_cell_object_start( 	menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	char	objectname[80];
	int	sts;
	
	sts = gdh_ObjidToName ( objid, objectname, sizeof(objectname), cdh_mNName);
	if ( EVEN(sts)) return sts;

	sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, objectname, objectname, 0, 
		&RTTSYS_OBJECT_CELL);
	return sts;
}

int RTTSYS_OBJECT_CELL( menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static pwr_sClass_NMpsCell	*cell_ptr;
  static gdh_tSubid	cell_subid;
  static pwr_tObjid 	cell_objid;
  static pwr_tClassId 	cell_class;
  static pwr_tObjid 	cell_class_objid;
  char			cell_name[80];
  int			i, j, k, l;
  static int		page;	

  IF_NOGDH_RETURN;

  /**********************************************************
  *	Return address of menu
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p29_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    /* Check object first */
    sts = gdh_NameToObjid ( objectname, &cell_objid);
    if ( EVEN(sts)) 
    {
      rtt_message('E', "Node is down");
      return RTT__NOPICTURE;
    }
    sts = gdh_ObjidToName ( cell_objid, cell_name, sizeof( cell_name), cdh_mNName);
    if ( EVEN(sts)) return sts;

    *picture = (char *) &dtt_systempicture_p29_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    page--;
    page = max( page, 0);
    CELLOBJ_PAGE = page + 1;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    page++;
    page = min( page, CELLOBJ_MAXPAGE - 1);
    CELLOBJ_PAGE = page + 1;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    page = 0;
    CELLOBJ_PAGE = page + 1;

    /* Get the object */
    sts = gdh_NameToObjid ( objectname, &cell_objid);
    if ( EVEN(sts)) return sts;
    sts = gdh_ObjidToName ( cell_objid, cell_name, sizeof( cell_name), cdh_mNName);
    if ( EVEN(sts)) return sts;

    /* Get object class */
    sts = gdh_GetObjectClass ( cell_objid, &cell_class);
    if ( EVEN(sts)) return sts;
    cell_class_objid = cdh_ClassIdToObjid( cell_class);

    switch( cell_class)
    {
      case pwr_cClass_NMpsCell:
      {
        sts = gdh_RefObjectInfo ( objectname, (pwr_tAddress *) &cell_ptr, 
		&cell_subid, sizeof( pwr_sClass_NMpsCell));
        if ( EVEN(sts)) return sts;
        break;
      }
      case pwr_cClass_NMpsStoreCell:
      {
        sts = gdh_RefObjectInfo ( objectname, (pwr_tAddress *) &cell_ptr, 
		&cell_subid, sizeof( pwr_sClass_NMpsStoreCell));
        if ( EVEN(sts)) return sts;
        break;
      }
      case pwr_cClass_NMpsOutCell:
      {
        sts = gdh_RefObjectInfo ( objectname, (pwr_tAddress *) &cell_ptr, 
		&cell_subid, sizeof( pwr_sClass_NMpsOutCell));
        if ( EVEN(sts)) return sts;
        break;
      }
      case pwr_cClass_NMpsMirrorCell:
      {
        sts = gdh_RefObjectInfo ( objectname, (pwr_tAddress *) &cell_ptr, &cell_subid,
		sizeof( pwr_sClass_NMpsMirrorCell));
        if ( EVEN(sts)) return sts;
        break;
      }
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    menu_ptr += 5 * (CELLOBJ_PAGESIZE);
    switch( cell_class)
    {
      case pwr_cClass_NMpsCell:
      case pwr_cClass_NMpsStoreCell:
      case pwr_cClass_NMpsOutCell:
      {
        if ( cell_class == pwr_cClass_NMpsStoreCell)
          menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->SelectIndex;
        else
          menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->LastIndex;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->DataLast_ObjId;
        menu_ptr++;
        *(pwr_tBoolean *)(menu_ptr->value_ptr) = 
		((pwr_sClass_NMpsCell *)(cell_ptr))->DataLast_Front;
        menu_ptr++;
        *(pwr_tBoolean *)(menu_ptr->value_ptr) = 
		((pwr_sClass_NMpsCell *)(cell_ptr))->DataLast_Back;
        menu_ptr++;
        *(pwr_tBoolean *)(menu_ptr->value_ptr) =
		((pwr_sClass_NMpsCell *)(cell_ptr))->DataLast_Select;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->ExternStatus;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->ExternIndex;
        menu_ptr->func3 = &rtt_object_parameters;
	menu_ptr->argoi = cell_objid;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->ExternOpType;
        menu_ptr->func3 = &rtt_object_parameters;
	menu_ptr->argoi = cell_objid;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->ExternFlag;
        menu_ptr->func3 = &rtt_object_parameters;
	menu_ptr->argoi = cell_objid;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->ExternObjId;
        menu_ptr->func3 = &rtt_object_parameters;
	menu_ptr->argoi = cell_objid;
        menu_ptr++;
        *(pwr_tObjid *)(menu_ptr->value_ptr) = cell_class_objid;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->MaxSize;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->LastIndex;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->CellFull;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->ResetObject;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->Function;
        menu_ptr++;
        *(pwr_tObjid *)(menu_ptr->value_ptr) = cell_objid;
        menu_ptr++;
        break;
      }
      case pwr_cClass_NMpsMirrorCell:
      {
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsMirrorCell *)(cell_ptr))->LastIndex;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsMirrorCell *)(cell_ptr))->DataLast_ObjId;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        *(pwr_tObjid *)(menu_ptr->value_ptr) = cell_class_objid;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->MaxSize;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->LastIndex;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->CellFull;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) &((pwr_sClass_NMpsCell *)(cell_ptr))->Function;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        *(pwr_tObjid *)(menu_ptr->value_ptr) = cell_objid;
        menu_ptr++;
        break;
      }
    }
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    sts = gdh_UnrefObjectInfo ( cell_subid);
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    j = 0;
    k = 0;

    switch( cell_class) 
    {
      case pwr_cClass_NMpsCell:
      case pwr_cClass_NMpsStoreCell:
      case pwr_cClass_NMpsOutCell:
      {
	plc_t_DataInfo	*data_info;

        data_info = (plc_t_DataInfo *)	
		&((pwr_sClass_NMpsCell *)(cell_ptr))->Data1P;
        /* Display all the data objects */
	for ( l = 0; 
	      l < ((pwr_sClass_NMpsCell *)(cell_ptr))->LastIndex;
	      l++)
	{
          if ( (k >= page * CELLOBJ_PAGESIZE) && (k < (page + 1) * CELLOBJ_PAGESIZE))
          {
	    *(pwr_tInt32 *)(menu_ptr->value_ptr) = l + 1;
	    menu_ptr++;
            menu_ptr->func2 = &rttsys_cell_dataobject;
            menu_ptr->func3 = &rtt_object_parameters;
	    menu_ptr->argoi = cell_objid;
            menu_ptr->arg1 = &data_info->Data_ObjId;
            *(pwr_tObjid *)(menu_ptr->value_ptr) = data_info->Data_ObjId;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Front;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Back;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Select;
            menu_ptr++;
            data_info++;
            j++;
	  }
	  k++;
	}
	break;
      }
      case pwr_cClass_NMpsMirrorCell:
      {
        plc_t_DataInfoMirCell	*data_info;

        data_info = (plc_t_DataInfoMirCell *)	
		&((pwr_sClass_NMpsMirrorCell *)(cell_ptr))->Data1P;

        /* Display all the data objects */
	for ( l = 0; 
	      l < ((pwr_sClass_NMpsMirrorCell *)(cell_ptr))->LastIndex;
	      l++)
	{
          if ( (k >= page * CELLOBJ_PAGESIZE) && (k < (page + 1) * CELLOBJ_PAGESIZE))
          {
	    *(pwr_tInt32 *)(menu_ptr->value_ptr) = l + 1;
	    menu_ptr++;
            menu_ptr->func2 = &rttsys_cell_dataobject;
            menu_ptr->func3 = &rtt_object_parameters;
	    menu_ptr->argoi = cell_objid;
            menu_ptr->arg1 = &data_info->Data_ObjId;
            *(pwr_tObjid *)(menu_ptr->value_ptr) = data_info->Data_ObjId;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
            menu_ptr++;
            data_info++;
            j++;
	  }
	  k++;
	}
	break;
      }
    }

    for ( i = j; i < CELLOBJ_PAGESIZE ; i++)
    {
      *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
      menu_ptr++;
      menu_ptr->func3 = &rtt_object_parameters;
      menu_ptr->argoi = cell_objid;
      menu_ptr->func2 = 0;
      menu_ptr->arg1 = 0;
      *(pwr_tObjid *)menu_ptr->value_ptr = pwr_cNObjid;
      menu_ptr++;
      *(pwr_tBoolean *)menu_ptr->value_ptr = 0;
      menu_ptr++;
      *(pwr_tBoolean *)menu_ptr->value_ptr = 0;
      menu_ptr++;
      *(pwr_tBoolean *)menu_ptr->value_ptr = 0;
      menu_ptr++;
    }
    CELLOBJ_MAXPAGE = max( 1, (k - 1)/ CELLOBJ_PAGESIZE + 1);

  }
  return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_NMPSCELL
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Function: 	Start of system picture from command.
*	
*
**************************************************************************/

static int rttsys_cellist_add( 	pwr_tObjid		object_objid,
				pwr_tClassId		class,
				rttsys_t_cell_list	**objectlist,
				int			*objectlist_count,
				int			*alloc)
{
	rttsys_t_cell_list	*objectlist_ptr;
	rttsys_t_cell_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;

	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_cell_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_ALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_ALLOC,
			sizeof(rttsys_t_cell_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_cell_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_ALLOC;
	}
	objectlist_ptr = *objectlist + *objectlist_count;
	objectlist_ptr->objid = object_objid;
	objectlist_ptr->class = class;

        sts = gdh_ObjidToName ( object_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        strcpy( objectlist_ptr->name, namebuf);

	memset( &attrref, 0, sizeof( attrref));
	attrref.Objid = object_objid;
	sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &objectlist_ptr->value_ptr,
		&objectlist_ptr->subid);
        if ( EVEN(sts)) return sts;

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

static int	rttsys_cell_add( pwr_tClassId		class,
				rttsys_t_cell_list	**objectlist,
				int			*objectlist_count,
				int			*object_alloc)
{
    pwr_tObjid		object_objid;
    int			sts;

    /* Get all initsteps of this node */
    sts = gdh_GetClassList ( class, &object_objid);
    while ( ODD(sts))
    {
      /* Store and direct link the initstep */
      sts = rttsys_cellist_add( object_objid, class, objectlist,
		objectlist_count, object_alloc);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextObject ( object_objid, &object_objid);
    }

    return RTT__SUCCESS;
}

static rttsys_t_cell_list	*cellist;
static int			cellist_count;

static int	rttsys_cell_expand( 	menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
  rttsys_t_cell_list	*cellist_ptr;
  int			i;

  cellist_ptr = cellist;
  for ( i = 0; i < cellist_count; i++)
  {
    if ( cdh_ObjidIsEqual( cellist_ptr->objid, objid))
    {
      cellist_ptr->expand = !cellist_ptr->expand;
      break;
    }
    cellist_ptr++;
  }
  return RTT__SUCCESS;
}

static int	rttsys_cell_dataobject( menu_ctx	ctx, 
				pwr_tObjid	cell_objid,
				pwr_tObjid	*data_objid,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
  int sts;

  sts = rtt_object_parameters( ctx, *data_objid, 0, 0, 0, 0);
  if (EVEN(sts)) 
  {
    rtt_message('E',"Unable to open object");
    return RTT__NOPICTURE;
  }
  return RTT__SUCCESS;
}

static int rttsys_cellist_bubblesort( 	rttsys_t_cell_list	*cellist,
					int			size)
{
	int	i, j;
	char	*str1, *str2;
	rttsys_t_cell_list 	dum;
	rttsys_t_cell_list	*cellist_ptr;

	for ( i = size - 1; i > 0; i--)
	{
	  cellist_ptr = cellist;
	  for ( j = 0; j < i; j++)
	  {
	    str1 = cellist_ptr->name;
	    str2 = (cellist_ptr + 1)->name;
	    if ( *str2 == 0)
	      break;
	    if ( strcmp( str1, str2) > 0)
	    {
	      /* Change order */
	      memcpy( &dum, cellist_ptr + 1, sizeof( rttsys_t_cell_list));
	      memcpy( cellist_ptr + 1, cellist_ptr, 
			sizeof( rttsys_t_cell_list));
	      memcpy( cellist_ptr, &dum, sizeof( rttsys_t_cell_list));
	    }
	    cellist_ptr++;
	  }
	}
	return RTT__SUCCESS;
}

#define NMPSCELL_PAGESIZE 19

int RTTSYS_NMPSCELL( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*cell_menu_ptr;
  rtt_t_menu_upd	*menulist;
  int			i, j, k, l;
  rttsys_t_cell_list	*cellist_ptr;
  static int		page;	
  int			cell_alloc;

  /**********************************************************
  *	Return address of menu
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p28_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p28_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE)
  {
    page--;
    page = max( page, 0);
    NMPSCELL_PAGE = page + 1;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next page
  ***********************************************************/
  if ( event == RTT_APPL_NEXTPAGE)
  {
    page++;
    page = min( page, NMPSCELL_MAXPAGE - 1);
    NMPSCELL_PAGE = page + 1;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    cellist_count = 0;
    page = 0;
    NMPSCELL_PAGE = page + 1;

    sts = rttsys_cell_add( pwr_cClass_NMpsCell, &cellist,
		&cellist_count, &cell_alloc);
    if ( EVEN(sts)) return sts;
    sts = rttsys_cell_add( pwr_cClass_NMpsStoreCell, &cellist,
		&cellist_count, &cell_alloc);
    if ( EVEN(sts)) return sts;
    sts = rttsys_cell_add( pwr_cClass_NMpsMirrorCell, &cellist,
		&cellist_count, &cell_alloc);
    if ( EVEN(sts)) return sts;
    sts = rttsys_cell_add( pwr_cClass_NMpsOutCell, &cellist,
		&cellist_count, &cell_alloc);
    if ( EVEN(sts)) return sts;

    rttsys_cellist_bubblesort( cellist, cellist_count);

  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    /* Free the cellist */
    cellist_ptr = cellist;
    for ( i = 0; i < cellist_count; i++)
    {
      sts = gdh_DLUnrefObjectInfo ( cellist_ptr->subid);
      cellist_ptr++;
    }
    free( cellist);
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    cellist_ptr = cellist;
    j = 0;
    k = 0;
    for ( i = 0; i < cellist_count; i++)
    {
      if ( (k >= page * NMPSCELL_PAGESIZE) && (k < (page + 1) * NMPSCELL_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, cellist_ptr->name);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_cell_object_start;
        menu_ptr->func2 = &rttsys_cell_dataobject;
        menu_ptr->func3 = &rttsys_cell_expand;
        menu_ptr->argoi = cellist_ptr->objid;

	cell_menu_ptr = menu_ptr;
        menu_ptr++;
	switch( cellist_ptr->class) 
        {
	  case pwr_cClass_NMpsCell:
	  case pwr_cClass_NMpsStoreCell:
	  case pwr_cClass_NMpsOutCell:
	  {
	    plc_t_DataInfo	*data_info;


            *(pwr_tInt32 *)(menu_ptr->value_ptr) = 
		((pwr_sClass_NMpsCell *)(cellist_ptr->value_ptr))->Function;
            menu_ptr++;
            *(pwr_tInt32 *)(menu_ptr->value_ptr) = 
		((pwr_sClass_NMpsCell *)(cellist_ptr->value_ptr))->MaxSize;
            menu_ptr++;
            *(pwr_tInt32 *)(menu_ptr->value_ptr) = 
		((pwr_sClass_NMpsCell *)(cellist_ptr->value_ptr))->LastIndex;
            menu_ptr++;
            *(pwr_tInt32 *)(menu_ptr->value_ptr) = 1;
            menu_ptr++;

            data_info = (plc_t_DataInfo	*)
		&((pwr_sClass_NMpsCell *)(cellist_ptr->value_ptr))->Data1P;
            cell_menu_ptr->arg1 = &data_info->Data_ObjId;

            *(pwr_tObjid *)(menu_ptr->value_ptr) = data_info->Data_ObjId;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Front;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Back;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Select;
            menu_ptr++;
	    j++;
	    break;
	  }
	  case pwr_cClass_NMpsMirrorCell:
	  {
	    plc_t_DataInfoMirCell	*data_info;

            *(pwr_tInt32 *)(menu_ptr->value_ptr) = 
		((pwr_sClass_NMpsMirrorCell *)(cellist_ptr->value_ptr))->Function;
            menu_ptr++;
            *(pwr_tInt32 *)(menu_ptr->value_ptr) = 
		((pwr_sClass_NMpsMirrorCell *)(cellist_ptr->value_ptr))->MaxSize;
            menu_ptr++;
            *(pwr_tInt32 *)(menu_ptr->value_ptr) = 
		((pwr_sClass_NMpsMirrorCell *)(cellist_ptr->value_ptr))->LastIndex;
            menu_ptr++;
            *(pwr_tInt32 *)(menu_ptr->value_ptr) = 1;
            menu_ptr++;

            data_info = (plc_t_DataInfoMirCell *)
		&((pwr_sClass_NMpsMirrorCell *)(cellist_ptr->value_ptr))->Data1P;
            cell_menu_ptr->arg1 = &data_info->Data_ObjId;

            *(pwr_tObjid *)(menu_ptr->value_ptr) = data_info->Data_ObjId;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
            menu_ptr++;
            *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
            menu_ptr++;
            j++;
	    break;
	  }
	}
      }
      k++;
      if ( cellist_ptr->expand)
      {
        switch( cellist_ptr->class) 
        {
	  case pwr_cClass_NMpsCell:
	  case pwr_cClass_NMpsStoreCell:
	  case pwr_cClass_NMpsOutCell:
	  {
	    plc_t_DataInfo	*data_info;

            data_info = (plc_t_DataInfo	*)
		&((pwr_sClass_NMpsCell *)(cellist_ptr->value_ptr))->Data1P;
            /* Display all the data objects */
	    for ( l = 1; 
	      l < ((pwr_sClass_NMpsCell *)(cellist_ptr->value_ptr))->LastIndex;
	      l++)
	    {
              if ( (k >= page * NMPSCELL_PAGESIZE) && (k < (page + 1) * NMPSCELL_PAGESIZE))
              {

	        data_info++;
	        strcpy( menu_ptr->value_ptr, "");
	        menu_ptr->func = 0;
                menu_ptr->func2 = &rttsys_cell_dataobject;
	        menu_ptr->func3 = 0;
	        menu_ptr->argoi = pwr_cNObjid;
	        cell_menu_ptr = menu_ptr;
	        menu_ptr++;
	        *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
	        menu_ptr++;
	        *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
	        menu_ptr++;
	        *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
	        menu_ptr++;
	        *(pwr_tInt32 *)(menu_ptr->value_ptr) = l + 1;
	        menu_ptr++;
                cell_menu_ptr->arg1 = &data_info->Data_ObjId;
                *(pwr_tObjid *)(menu_ptr->value_ptr) = data_info->Data_ObjId;
                menu_ptr++;
                *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Front;
                menu_ptr++;
                *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Back;
                menu_ptr++;
                *(pwr_tBoolean *)(menu_ptr->value_ptr) = data_info->Data_Select;
                menu_ptr++;
                j++;
	      }
	      k++;
	    }
	    break;
          }
	  case pwr_cClass_NMpsMirrorCell:
	  {
	    plc_t_DataInfoMirCell	*data_info;

            data_info = (plc_t_DataInfoMirCell *)
		&((pwr_sClass_NMpsMirrorCell *)(cellist_ptr->value_ptr))->Data1P;

            /* Display all the data objects */
	    for ( l = 1; 
	      l < ((pwr_sClass_NMpsMirrorCell *)(cellist_ptr->value_ptr))->LastIndex;
	      l++)
	    {
              if ( (k >= page * NMPSCELL_PAGESIZE) && (k < (page + 1) * NMPSCELL_PAGESIZE))
              {

	        data_info++;
	        strcpy( menu_ptr->value_ptr, "");
	        menu_ptr->func = 0;
                menu_ptr->func2 = &rttsys_cell_dataobject;
	        menu_ptr->func3 = 0;
	        menu_ptr->argoi = pwr_cNObjid;
	        cell_menu_ptr = menu_ptr;
	        menu_ptr++;
	        *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
	        menu_ptr++;
	        *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
	        menu_ptr++;
	        *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
	        menu_ptr++;
	        *(pwr_tInt32 *)(menu_ptr->value_ptr) = l + 1;
	        menu_ptr++;
                cell_menu_ptr->arg1 = &data_info->Data_ObjId;
                *(pwr_tObjid *)(menu_ptr->value_ptr) = data_info->Data_ObjId;
                menu_ptr++;
                *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
                menu_ptr++;
                *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
                menu_ptr++;
                *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
                menu_ptr++;
                j++;
	      }
	      k++;
	    }
	    break;
	  }
	}
      }
      cellist_ptr++;
    }
    for ( i = j; i < NMPSCELL_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
      menu_ptr++;
      *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
      menu_ptr++;
      *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
      menu_ptr++;
      *(pwr_tInt32 *)(menu_ptr->value_ptr) = 0;
      menu_ptr++;
      *(pwr_tObjid *)(menu_ptr->value_ptr) = pwr_cNObjid;
      menu_ptr++;
      *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
      menu_ptr++;
      *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
      menu_ptr++;
      *(pwr_tBoolean *)(menu_ptr->value_ptr) = 0;
      menu_ptr++;
    }
    NMPSCELL_MAXPAGE = max( 1, (k - 1)/ NMPSCELL_PAGESIZE + 1);

  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_CHANDI()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the ChanDi objects of a di card.
*
**************************************************************************/

int	rttsys_show_signal_from_chan(
			menu_ctx	ctx,
			pwr_tObjid	objid,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4)
{
	char 		hiername[120];
	int		sts;
	pwr_tObjid	signal_objid;

	sts = gdh_ObjidToName ( objid, hiername, sizeof(hiername), cdh_mNName);
	if ( EVEN(sts)) 
	{
	  rtt_message('E', "Signal not found");
	  return RTT__NOPICTURE;
	}
	strcat( hiername, ".SigChanCon");
	sts = gdh_GetObjectInfo ( hiername, (pwr_tAddress) &signal_objid,
		sizeof(signal_objid));
	if ( EVEN(sts)) return sts;

	sts = gdh_ObjidToName ( signal_objid, hiername, sizeof(hiername), cdh_mNName);
	if ( EVEN(sts)) 
	{
	  rtt_message('E', "Signal not found");
	  return RTT__NOPICTURE;
	}

	sts = rtt_object_parameters( ctx, signal_objid, arg1, arg2, arg3, arg4);
	return sts;
}

static int rttsys_chan_start( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	char	objectname[80];
	int	sts;
	pwr_tClassId	class;
	pwr_tObjid	chan_objid;
	
	sts = gdh_ObjidToName ( objid, objectname, sizeof(objectname), cdh_mNName);
	if ( EVEN(sts)) return sts;

	/* Get the class of the channels */
	sts = gdh_GetChild ( objid, &chan_objid);
	if ( EVEN(sts))
	{
	  rtt_message( 'E', "Card is not configured");
	  return RTT__NOPICTURE;
	}
	sts = gdh_GetObjectClass ( chan_objid, &class);
	if ( EVEN(sts)) return sts;

	switch ( class)
	{
	  case pwr_cClass_ChanDi:
	    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, objectname, 
		objectname, 0, &RTTSYS_CHANDI);
	    return sts;
	  case pwr_cClass_ChanDo:
	    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, objectname, objectname, 0, 
		&RTTSYS_CHANDO);
	    return sts;
	  case pwr_cClass_ChanAi:
	  case pwr_cClass_ChanAit:
	    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, objectname, 
		objectname, 0, &RTTSYS_CHANAI);
	    return sts;
	  case pwr_cClass_ChanAo:
	    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, objectname, 
		objectname, 0, &RTTSYS_CHANAO);
	    return sts;
	  case pwr_cClass_ChanCo:
	    sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, objectname, 
		objectname, 0, &RTTSYS_CHANCO);
	    return sts;
	  default:
	    rtt_message( 'E', "Error in channel class");
	    return RTT__NOPICTURE;
	}

}

static int rttsys_get_conversion( pwr_tUInt16	convmask1, 
				pwr_tUInt16	convmask2, 
				int		chan_number,
				int		*conversion)
{
	if ( chan_number >= 16)
	{
	  chan_number -= 16;
	  *conversion = (convmask2 & 1 << chan_number) != 0;
	}
	else
	  *conversion = (convmask1 & 1 << chan_number) != 0;

	return RTT__SUCCESS;
}
 
static int rttsys_dichanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local)
{
	rttsys_t_chan_list	*chanlist_ptr;
	rttsys_t_chan_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	int			signame_characters;

	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_chan_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_CHANALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_CHANALLOC,
			sizeof(rttsys_t_chan_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_chan_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_CHANALLOC;
	}
	chanlist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( chan_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last two segment */
        rtt_cut_segments( chanlist_ptr->channame, namebuf, 2);
	if ( strlen(chanlist_ptr->channame) > 10)
          rtt_cut_segments( chanlist_ptr->channame, namebuf, 1);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = chan_objid;
        if ( local)
        {
	  /* Get a direct link to channel object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		chanlist_ptr->chan_subid,
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
        chanlist_ptr->chan_objid = chan_objid;
        chanlist_ptr->invert_on = 
	  &((pwr_sClass_ChanDi *)(chanlist_ptr->chan_ptr))->InvertOn;
        chanlist_ptr->chan_number = 
	  ((pwr_sClass_ChanDi *)(chanlist_ptr->chan_ptr))->Number;
        strcpy( chanlist_ptr->chan_descript,
	  ((pwr_sClass_ChanDi *)(chanlist_ptr->chan_ptr))->Description);
        strcpy( chanlist_ptr->chan_ident,
	  ((pwr_sClass_ChanDi *)(chanlist_ptr->chan_ptr))->Identity);


        /* Get signal */
        sts = gdh_ObjidToName( 
		((pwr_sClass_ChanDi *)(chanlist_ptr->chan_ptr))->SigChanCon, 
		namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts))
        {
	  chanlist_ptr->connected = 0;
          strcpy( chanlist_ptr->signame, "-");
        }
        else
        {
	  chanlist_ptr->connected = 1;
	  signame_characters = 53; /* Should be fetched from menu entry... */
          if ( (int)strlen( namebuf) > signame_characters)
          {
	    /* Show the last part of the name */
	    strcpy( chanlist_ptr->signame, ".");
	    strcat( chanlist_ptr->signame,
		&namebuf[strlen( namebuf)-signame_characters+1]);
	  }
	  else
	    strcpy( chanlist_ptr->signame, namebuf);

	  /* Get a pointer to the value */
	  memset( &attrref, 0, sizeof(attrref));
          sts = gdh_ClassAttrToAttrref( pwr_cClass_Di, ".ActualValue", &attrref);
          if ( EVEN(sts)) return sts;
	  attrref.Objid = 
		((pwr_sClass_ChanDi *)(chanlist_ptr->chan_ptr))->SigChanCon;
          if ( local)
          {
	    /* Get a direct link to the original object */
	    sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		&chanlist_ptr->value_subid);
	    if (EVEN(sts)) return sts;
          }
          else
          {
	    /* Get a subscription to the original object */
	    sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->value_subid);
	    if (EVEN(sts)) return sts;

	    sts = gdh_SubAssociateBuffer(
		chanlist_ptr->value_subid,
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		attrref.Size);
	    if (EVEN(sts)) return sts;
          }
	  /* Get the signal description */
	  strcat( namebuf, ".Description");
          sts = gdh_GetObjectInfo ( namebuf, 
		(pwr_tAddress *) chanlist_ptr->sig_descript, 
		sizeof(chanlist_ptr->sig_descript));
          if ( EVEN(sts))
	    strcpy( chanlist_ptr->sig_descript, "");
        }

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_CHANDI( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid 		objid;
  char			namebuf[80];
  pwr_tBoolean	 	local;
  pwr_tObjid		chan_objid;
  int			chan_alloc;
  char			card_name[80];
  rttsys_t_chan_list	*chanlist_ptr;
  int			i, j;

  static int		page;
  static rttsys_t_chan_list *chanlist;
  static int		chanlist_count;
  static pwr_tUInt16	*convmask1;
  static gdh_tDlid	convmask1_subid;
  static pwr_tUInt16	*convmask2;
  static gdh_tDlid	convmask2_subid;
  static int		display_mode;

#define CHANDI_PAGESIZE 20
#define CHAN_DISPLAYMODE_SIGNAL 	0
#define CHAN_DISPLAYMODE_CHAN_DESCRIPT	1
#define CHAN_DISPLAYMODE_SIG_DESCRIPT 	2
#define CHAN_DISPLAYMODE_CHAN_IDENT	3
#define CHAN_DISPLAYMODE_MAX		3

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    /* Check object first */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) 
    {
      rtt_message('E', "Node is down");
      return RTT__NOPICTURE;
    }
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    *picture = (char *) &dtt_systempicture_p30_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p30_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, CHANDI_MAXPAGE - 1);
    }
    CHANDI_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANDI_PAGESIZE) && (i < (page + 1) * CHANDI_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = &rtt_crossref_channel;
        menu_ptr->argoi = chanlist_ptr->chan_objid;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
          menu_ptr->value_ptr = (char *)chanlist_ptr->value_ptr;
          menu_ptr++;
  	  sts = rttsys_get_conversion( *convmask1, *convmask2, 
			chanlist_ptr->chan_number,
			&chanlist_ptr->conv_on);
          menu_ptr->value_ptr = (char *) &chanlist_ptr->conv_on;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->invert_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
	  menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANDI_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    display_mode = 0;
    strcpy( CHANDI_DISPLAY_TITLE, "Signal");
    page = 0;
    CHANDI_PAGE = page + 1;

    /* Get the card object */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) return sts;
    sts = gdh_ObjidToName ( objid, card_name, sizeof( card_name), cdh_mNName);
    if ( EVEN(sts)) return sts;
    sts = gdh_GetObjectLocation( objid, &local);
    if ( EVEN(sts)) return sts;

    /* Get all channels and store in the channel list */
    chan_alloc = 0;
    chanlist_count = 0;
    sts = gdh_GetChild( objid, &chan_objid);
    while( ODD(sts))
    {
      sts = rttsys_dichanlist_add( chan_objid, &chanlist,
		&chanlist_count, &chan_alloc, (int) local);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextSibling ( chan_objid, &chan_objid);
    }

    /* Get a pointer to the conversion mask in the card object */
    strcpy( namebuf, card_name);
    strcat( namebuf, ".ConvMask1");
    sts = gdh_RefObjectInfo ( namebuf, (pwr_tAddress *) &convmask1, 
		&convmask1_subid, sizeof( *convmask1));
    if ( EVEN(sts)) return sts;
    strcpy( namebuf, card_name);
    strcat( namebuf, ".ConvMask2");
    sts = gdh_RefObjectInfo ( namebuf, (pwr_tAddress *) &convmask2, 
		&convmask2_subid, sizeof( *convmask2));
    if ( EVEN(sts)) return sts;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANDI_PAGESIZE) && (i < (page + 1) * CHANDI_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = &rtt_crossref_channel;
        menu_ptr->argoi = chanlist_ptr->chan_objid;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
          menu_ptr->value_ptr = (char *)chanlist_ptr->value_ptr;
          menu_ptr++;
  	  sts = rttsys_get_conversion( *convmask1, *convmask2, 
			chanlist_ptr->chan_number,
			&chanlist_ptr->conv_on);
          menu_ptr->value_ptr = (char *) &chanlist_ptr->conv_on;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->invert_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
	  menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANDI_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    CHANDI_MAXPAGE = max( 1, (chanlist_count - 1)/ CHANDI_PAGESIZE + 1);

    rtt_cut_segments( menu_ptr->value_ptr, card_name, 2);
    menu_ptr++;
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    chanlist_ptr = chanlist;
    for ( i = 0; i < chanlist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( chanlist_ptr->chan_subid);
      if ( chanlist_ptr->connected)
        sts = gdh_UnrefObjectInfo( chanlist_ptr->value_subid);
      chanlist_ptr++;
    }
    sts = gdh_UnrefObjectInfo( convmask1_subid);
    sts = gdh_UnrefObjectInfo( convmask2_subid);
    free( chanlist);
    chanlist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
    if ( parameter_ptr == (char *) &CHANDI_DISPLAY_MODE)
    {
      if (CHANDI_DISPLAY_MODE)
        display_mode++;
      else
        display_mode--;
      if ( display_mode > CHAN_DISPLAYMODE_MAX)
        display_mode = 0;
      if ( display_mode < 0)
        display_mode = CHAN_DISPLAYMODE_MAX;

      switch( display_mode)
      {
	case CHAN_DISPLAYMODE_SIGNAL:
	  strcpy( CHANDI_DISPLAY_TITLE, "Signal");
	  break;
	case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	  strcpy( CHANDI_DISPLAY_TITLE, "Channel description");
	  break;
	case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	  strcpy( CHANDI_DISPLAY_TITLE, "Signal description");
	  break;
	case CHAN_DISPLAYMODE_CHAN_IDENT:
	  strcpy( CHANDI_DISPLAY_TITLE, "Identity");
	  break;
      }

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;
      chanlist_ptr = chanlist;
      j = 0;
      for ( i = 0; i < chanlist_count; i++)
      {
        if ( (i >= page * CHANDI_PAGESIZE) && (i < (page + 1) * CHANDI_PAGESIZE))
        {
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
  	  if ( chanlist_ptr->connected)
	  {
	    switch( display_mode)
	    {
	      case CHAN_DISPLAYMODE_SIGNAL:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	        break;
	      case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_IDENT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	        break;
	    }
            menu_ptr++;
	  }
	  else
	  {
  	    strcpy( menu_ptr->value_ptr, "");
            menu_ptr++;
	  }
          j++;
        }
        chanlist_ptr++;
      }
    }
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    chanlist_ptr = chanlist;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANDI_PAGESIZE) && (i < (page + 1) * CHANDI_PAGESIZE))
      {
	if ( chanlist_ptr->connected)
	{
  	  sts = rttsys_get_conversion( *convmask1, *convmask2, 
			chanlist_ptr->chan_number,
			&chanlist_ptr->conv_on);
	}
      }
      chanlist_ptr++;
    }
  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_CHANDO()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the ChanDo objects of a do card.
*
**************************************************************************/

static int rttsys_chando_change_testmode( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	int	on;
	int	sts;

	sts = rtt_get_do_test( objid, &on);
	if ( EVEN(sts)) return sts;

	sts = rtt_set_do_test( objid, !on, 0);
	return sts;
}

static int rttsys_chando_change_testvalue( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	int	on;
	int	sts;

	sts = rtt_get_do_testvalue( objid, &on);
	if ( EVEN(sts)) return sts;

	sts = rtt_set_do_testvalue( objid, !on, 0);
	return sts;
}

static int rttsys_dochanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local)
{
	rttsys_t_chan_list	*chanlist_ptr;
	rttsys_t_chan_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	int			signame_characters;

	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_chan_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_CHANALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_CHANALLOC,
			sizeof(rttsys_t_chan_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_chan_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_CHANALLOC;
	}
	chanlist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( chan_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last two segment */
        rtt_cut_segments( chanlist_ptr->channame, namebuf, 2);
	if ( strlen(chanlist_ptr->channame) > 10)
          rtt_cut_segments( chanlist_ptr->channame, namebuf, 1);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = chan_objid;
        if ( local)
        {
	  /* Get a direct link to channel object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		chanlist_ptr->chan_subid,
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
        chanlist_ptr->chan_objid = chan_objid;
        chanlist_ptr->invert_on = 
	  &((pwr_sClass_ChanDo *)(chanlist_ptr->chan_ptr))->InvertOn;
        chanlist_ptr->test_on = 
	  &((pwr_sClass_ChanDo *)(chanlist_ptr->chan_ptr))->TestOn;
        chanlist_ptr->test_value = 
	  &((pwr_sClass_ChanDo *)(chanlist_ptr->chan_ptr))->TestValue;
        chanlist_ptr->chan_number = 
	  ((pwr_sClass_ChanDo *)(chanlist_ptr->chan_ptr))->Number;
        strcpy( chanlist_ptr->chan_descript,
	  ((pwr_sClass_ChanDi *)(chanlist_ptr->chan_ptr))->Description);
        strcpy( chanlist_ptr->chan_ident,
	  ((pwr_sClass_ChanDi *)(chanlist_ptr->chan_ptr))->Identity);

        /* Get signal */
        sts = gdh_ObjidToName( 
		((pwr_sClass_ChanDo *)(chanlist_ptr->chan_ptr))->SigChanCon, 
		namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts))
        {
	  chanlist_ptr->connected = 0;
          strcpy( chanlist_ptr->signame, "-");
        }
        else
        {
	  chanlist_ptr->connected = 1;
	  signame_characters = 53; /* Should be fetched from menu entry... */
          if ( (int)strlen( namebuf) > signame_characters)
          {
	    /* Show the last part of the name */
	    strcpy( chanlist_ptr->signame, ".");
	    strcat( chanlist_ptr->signame, 
		&namebuf[strlen( namebuf)-signame_characters+1]);
	  }
	  else
	    strcpy( chanlist_ptr->signame, namebuf);

	  /* Get a pointer to the value */
	  memset( &attrref, 0, sizeof(attrref));
          sts = gdh_ClassAttrToAttrref( pwr_cClass_Di, ".ActualValue", &attrref);
          if ( EVEN(sts)) return sts;
	  attrref.Objid = 
		((pwr_sClass_ChanDo *)(chanlist_ptr->chan_ptr))->SigChanCon;
          if ( local)
          {
	    /* Get a direct link to the original object */
	    sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		&chanlist_ptr->value_subid);
	    if (EVEN(sts)) return sts;
          }
          else
          {
	    /* Get a subscription to the original object */
	    sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->value_subid);
	    if (EVEN(sts)) return sts;

	    sts = gdh_SubAssociateBuffer(
		chanlist_ptr->value_subid,
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		attrref.Size);
	    if (EVEN(sts)) return sts;
          }
	  /* Get the signal description */
	  strcat( namebuf, ".Description");
          sts = gdh_GetObjectInfo ( namebuf, 
		(pwr_tAddress *) chanlist_ptr->sig_descript, 
		sizeof(chanlist_ptr->sig_descript));
          if ( EVEN(sts))
	    strcpy( chanlist_ptr->sig_descript, "");
        }

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_CHANDO( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid 		objid;
  char			namebuf[80];
  pwr_tBoolean	 	local;
  pwr_tObjid		chan_objid;
  int			chan_alloc;
  char			card_name[80];
  rttsys_t_chan_list	*chanlist_ptr;
  int			i, j;

  static int		page;
  static rttsys_t_chan_list *chanlist;
  static int		chanlist_count;
  static int		display_mode;
  static int		signal_test_mode;
  static int		toggle = 0;

#define CHANDO_PAGESIZE 20

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    /* Check object first */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) 
    {
      rtt_message('E', "Node is down");
      return RTT__NOPICTURE;
    }
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    *picture = (char *) &dtt_systempicture_p31_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p31_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, CHANDO_MAXPAGE - 1);
    }
    CHANDO_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANDO_PAGESIZE) && (i < (page + 1) * CHANDO_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
	if ( signal_test_mode)
	{
          menu_ptr->func2 = &rttsys_chando_change_testvalue;
          menu_ptr->func3 = &rttsys_chando_change_testmode;
	}
	else
	{
          menu_ptr->func2 = &rtt_object_parameters;
          menu_ptr->func3 = &rtt_crossref_channel;
	}
        menu_ptr->argoi = chanlist_ptr->chan_objid;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
          menu_ptr->value_ptr = (char *)chanlist_ptr->value_ptr;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->test_on;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->test_value;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->invert_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
	  menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANDO_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {

    display_mode = 0;
    signal_test_mode = rtt_signal_test_mode;
    strcpy( CHANDO_DISPLAY_TITLE, "Signal");
    page = 0;
    CHANDO_PAGE = page + 1;

    /* Get the card object */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) return sts;
    sts = gdh_ObjidToName ( objid, card_name, sizeof( card_name), cdh_mNName);
    if ( EVEN(sts)) return sts;
    sts = gdh_GetObjectLocation( objid, &local);
    if ( EVEN(sts)) return sts;

    /* Get all channels and store in the channel list */
    chan_alloc = 0;
    chanlist_count = 0;
    sts = gdh_GetChild( objid, &chan_objid);
    while( ODD(sts))
    {
      sts = rttsys_dochanlist_add( chan_objid, &chanlist,
		&chanlist_count, &chan_alloc, (int) local);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextSibling ( chan_objid, &chan_objid);
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANDO_PAGESIZE) && (i < (page + 1) * CHANDO_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
	if ( signal_test_mode)
	{
          menu_ptr->func2 = &rttsys_chando_change_testvalue;
          menu_ptr->func3 = &rttsys_chando_change_testmode;
	}
	else
	{
          menu_ptr->func2 = &rtt_object_parameters;
          menu_ptr->func3 = &rtt_crossref_channel;
	}
        menu_ptr->argoi = chanlist_ptr->chan_objid;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
          menu_ptr->value_ptr = (char *)chanlist_ptr->value_ptr;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->test_on;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->test_value;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->invert_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
	  menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANDO_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    CHANDO_MAXPAGE = max( 1, (chanlist_count - 1)/ CHANDO_PAGESIZE + 1);

    rtt_cut_segments( menu_ptr->value_ptr, card_name, 2);
    menu_ptr++;

    if ( signal_test_mode)
      strcpy( CHANDO_INFO,
      " RETURN show signal | PF1 testvalue 1/0 | PF2 test on/off | PF4 back   "
	);
    else
      strcpy( CHANDO_INFO,
      " RETURN show signal | PF1 show channel | PF2 show cross | PF4 back     "
	);
    strcpy( CHANDO_TST, "Tst");
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    chanlist_ptr = chanlist;
    for ( i = 0; i < chanlist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( chanlist_ptr->chan_subid);
      if ( chanlist_ptr->connected)
        sts = gdh_UnrefObjectInfo( chanlist_ptr->value_subid);
      chanlist_ptr++;
    }
    free( chanlist);
    chanlist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
    if ( parameter_ptr == (char *) &CHANDO_DISPLAY_MODE)
    {
      if (CHANDO_DISPLAY_MODE)
        display_mode++;
      else
        display_mode--;
      if ( display_mode > CHAN_DISPLAYMODE_MAX)
        display_mode = 0;
      if ( display_mode < 0)
        display_mode = CHAN_DISPLAYMODE_MAX;

      switch( display_mode)
      {
	case CHAN_DISPLAYMODE_SIGNAL:
	  strcpy( CHANDO_DISPLAY_TITLE, "Signal");
	  break;
	case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	  strcpy( CHANDO_DISPLAY_TITLE, "Channel description");
	  break;
	case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	  strcpy( CHANDO_DISPLAY_TITLE, "Signal description");
	  break;
	case CHAN_DISPLAYMODE_CHAN_IDENT:
	  strcpy( CHANDO_DISPLAY_TITLE, "Identity");
	  break;
      }

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;
      chanlist_ptr = chanlist;
      j = 0;
      for ( i = 0; i < chanlist_count; i++)
      {
        if ( (i >= page * CHANDO_PAGESIZE) && (i < (page + 1) * CHANDO_PAGESIZE))
        {
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
  	  if ( chanlist_ptr->connected)
	  {
	    switch( display_mode)
	    {
	      case CHAN_DISPLAYMODE_SIGNAL:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	        break;
	      case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_IDENT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	        break;
	    }
            menu_ptr++;
	  }
	  else
	  {
  	    strcpy( menu_ptr->value_ptr, "");
            menu_ptr++;
	  }
          j++;
        }
        chanlist_ptr++;
      }
    }
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    if ( signal_test_mode)
    {
      if ( toggle)
      {
        strcpy( CHANDO_TST, "Tst");
	toggle = 0;
      }
      else
      {
        strcpy( CHANDO_TST, "   ");
	toggle = 1;
      }
    }
  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_CHANAI()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the ChanAi objects of a ai card.
*
**************************************************************************/

static int rttsys_aichanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local)
{
	rttsys_t_chan_list	*chanlist_ptr;
	rttsys_t_chan_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	int			signame_characters;

	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_chan_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_CHANALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_CHANALLOC,
			sizeof(rttsys_t_chan_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_chan_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_CHANALLOC;
	}
	chanlist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( chan_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last two segment */
        rtt_cut_segments( chanlist_ptr->channame, namebuf, 2);
	if ( strlen(chanlist_ptr->channame) > 10)
          rtt_cut_segments( chanlist_ptr->channame, namebuf, 1);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = chan_objid;
        if ( local)
        {
	  /* Get a direct link to channel object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		chanlist_ptr->chan_subid,
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
        chanlist_ptr->chan_objid = chan_objid;
        chanlist_ptr->conversion_on = 
	  &((pwr_sClass_ChanAi *)(chanlist_ptr->chan_ptr))->ConversionOn;
        chanlist_ptr->chan_number = 
	  ((pwr_sClass_ChanAi *)(chanlist_ptr->chan_ptr))->Number;
        strcpy( chanlist_ptr->chan_descript,
	  ((pwr_sClass_ChanAi *)(chanlist_ptr->chan_ptr))->Description);
        strcpy( chanlist_ptr->chan_ident,
	  ((pwr_sClass_ChanAi *)(chanlist_ptr->chan_ptr))->Identity);

        /* Get signal */
        sts = gdh_ObjidToName( 
		((pwr_sClass_ChanAi *)(chanlist_ptr->chan_ptr))->SigChanCon, 
		namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts))
        {
	  chanlist_ptr->connected = 0;
          strcpy( chanlist_ptr->signame, "-");
        }
        else
        {
	  chanlist_ptr->connected = 1;
	  signame_characters = 51; /* Should be fetched from menu entry... */
          if ( (int)strlen( namebuf) > signame_characters)
          {
	    /* Show the last part of the name */
	    strcpy( chanlist_ptr->signame, ".");
	    strcat( chanlist_ptr->signame, 
		&namebuf[strlen( namebuf)-signame_characters+1]);
	  }
	  else
	    strcpy( chanlist_ptr->signame, namebuf);

	  /* Get a pointer to the value */
	  memset( &attrref, 0, sizeof(attrref));
          sts = gdh_ClassAttrToAttrref( pwr_cClass_Ai, ".RawValue", &attrref);
          if ( EVEN(sts)) return sts;
	  attrref.Objid = 
		((pwr_sClass_ChanAi *)(chanlist_ptr->chan_ptr))->SigChanCon;
          if ( local)
          {
	    /* Get a direct link to the original object */
	    sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		&chanlist_ptr->value_subid);
	    if (EVEN(sts)) return sts;
          }
          else
          {
	    /* Get a subscription to the original object */
	    sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->value_subid);
	    if (EVEN(sts)) return sts;

	    sts = gdh_SubAssociateBuffer(
		chanlist_ptr->value_subid,
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		attrref.Size);
	    if (EVEN(sts)) return sts;
          }
	  /* Get the signal description */
	  strcat( namebuf, ".Description");
          sts = gdh_GetObjectInfo ( namebuf, 
		(pwr_tAddress *) chanlist_ptr->sig_descript, 
		sizeof(chanlist_ptr->sig_descript));
          if ( EVEN(sts))
	    strcpy( chanlist_ptr->sig_descript, "");
        }

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_CHANAI( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid 		objid;
  char			namebuf[80];
  pwr_tBoolean	 	local;
  pwr_tObjid		chan_objid;
  int			chan_alloc;
  char			card_name[80];
  rttsys_t_chan_list	*chanlist_ptr;
  int			i, j;

  static int		page;
  static rttsys_t_chan_list *chanlist;
  static int		chanlist_count;
  static int		display_mode;

#define CHANAI_PAGESIZE 20

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    /* Check object first */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) 
    {
      rtt_message('E', "Node is down");
      return RTT__NOPICTURE;
    }
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    *picture = (char *) &dtt_systempicture_p32_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p32_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, CHANAI_MAXPAGE - 1);
    }
    CHANAI_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANAI_PAGESIZE) && (i < (page + 1) * CHANAI_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = &rtt_crossref_channel;
        menu_ptr->argoi = chanlist_ptr->chan_objid;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
          *(float *) menu_ptr->value_ptr = 
	    ((pwr_sClass_ChanAi *)chanlist_ptr->chan_ptr)->SigValPolyCoef0 +
	    ((pwr_sClass_ChanAi *)chanlist_ptr->chan_ptr)->SigValPolyCoef1 *
	    *(pwr_tInt16 *) chanlist_ptr->value_ptr;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->conversion_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
          *(float *) menu_ptr->value_ptr = 0;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANAI_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      *(float *) menu_ptr->value_ptr = 0;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {

    display_mode = 0;
    strcpy( CHANAI_DISPLAY_TITLE, "Signal");
    page = 0;
    CHANAI_PAGE = page + 1;

    /* Get the card object */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) return sts;
    sts = gdh_ObjidToName ( objid, card_name, sizeof( card_name), cdh_mNName);
    if ( EVEN(sts)) return sts;
    sts = gdh_GetObjectLocation( objid, &local);
    if ( EVEN(sts)) return sts;

    /* Get all channels and store in the channel list */
    chan_alloc = 0;
    chanlist_count = 0;
    sts = gdh_GetChild( objid, &chan_objid);
    while( ODD(sts))
    {
      sts = rttsys_aichanlist_add( chan_objid, &chanlist,
		&chanlist_count, &chan_alloc, (int) local);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextSibling ( chan_objid, &chan_objid);
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANAI_PAGESIZE) && (i < (page + 1) * CHANAI_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = &rtt_crossref_channel;
        menu_ptr->argoi = chanlist_ptr->chan_objid;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
          *(float *) menu_ptr->value_ptr = 
	    ((pwr_sClass_ChanAi *)chanlist_ptr->chan_ptr)->SigValPolyCoef0 +
	    ((pwr_sClass_ChanAi *)chanlist_ptr->chan_ptr)->SigValPolyCoef1 *
	    *(pwr_tInt16 *) chanlist_ptr->value_ptr;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->conversion_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
          *(float *) menu_ptr->value_ptr = 0;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANAI_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      *(float *) menu_ptr->value_ptr = 0;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    CHANAI_MAXPAGE = max( 1, (chanlist_count - 1)/ CHANAI_PAGESIZE + 1);

    rtt_cut_segments( menu_ptr->value_ptr, card_name, 2);
    menu_ptr++;
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    chanlist_ptr = chanlist;
    for ( i = 0; i < chanlist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( chanlist_ptr->chan_subid);
      if ( chanlist_ptr->connected)
        sts = gdh_UnrefObjectInfo( chanlist_ptr->value_subid);
      chanlist_ptr++;
    }
    free( chanlist);
    chanlist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
    if ( parameter_ptr == (char *) &CHANAI_DISPLAY_MODE)
    {
      if (CHANAI_DISPLAY_MODE)
        display_mode++;
      else
        display_mode--;
      if ( display_mode > CHAN_DISPLAYMODE_MAX)
        display_mode = 0;
      if ( display_mode < 0)
        display_mode = CHAN_DISPLAYMODE_MAX;

      switch( display_mode)
      {
	case CHAN_DISPLAYMODE_SIGNAL:
	  strcpy( CHANAI_DISPLAY_TITLE, "Signal");
	  break;
	case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	  strcpy( CHANAI_DISPLAY_TITLE, "Channel description");
	  break;
	case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	  strcpy( CHANAI_DISPLAY_TITLE, "Signal description");
	  break;
	case CHAN_DISPLAYMODE_CHAN_IDENT:
	  strcpy( CHANAI_DISPLAY_TITLE, "Identity");
	  break;
      }

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;
      chanlist_ptr = chanlist;
      j = 0;
      for ( i = 0; i < chanlist_count; i++)
      {
        if ( (i >= page * CHANAI_PAGESIZE) && (i < (page + 1) * CHANAI_PAGESIZE))
        {
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
  	  if ( chanlist_ptr->connected)
	  {
	    switch( display_mode)
	    {
	      case CHAN_DISPLAYMODE_SIGNAL:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	        break;
	      case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_IDENT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	        break;
	    }
            menu_ptr++;
	  }
	  else
	  {
  	    strcpy( menu_ptr->value_ptr, "");
            menu_ptr++;
	  }
          j++;
        }
        chanlist_ptr++;
      }
    }
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANAI_PAGESIZE) && (i < (page + 1) * CHANAI_PAGESIZE))
      {
        menu_ptr++;
	if ( chanlist_ptr->connected)
	  /* Calculate signal value */
          *(float *) menu_ptr->value_ptr = 
	    ((pwr_sClass_ChanAi *)chanlist_ptr->chan_ptr)->SigValPolyCoef0 +
	    ((pwr_sClass_ChanAi *)chanlist_ptr->chan_ptr)->SigValPolyCoef1 *
	    *(pwr_tInt16 *) chanlist_ptr->value_ptr;
        menu_ptr++;
        menu_ptr++;
        menu_ptr++;
        j++;
      }
      chanlist_ptr++;
    }
  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_CHANAO()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the ChanAo objects of a ao card.
*
**************************************************************************/

static int rttsys_chanao_change_testmode( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	int		sts;
	pwr_tBoolean 	test_on;
	char		name[120];

	sts = gdh_ObjidToName ( objid, name, sizeof(name),
			cdh_mName_volumeStrict);
	if ( EVEN(sts)) return sts;

	/* Get the number of the channel */
	strcat( name, ".TestOn");	
	sts = gdh_GetObjectInfo ( name, &test_on,
			sizeof(test_on));
	if (EVEN(sts)) return sts;

	test_on = !test_on;
	sts = gdh_SetObjectInfo ( name, &test_on, sizeof(test_on));
	if ( EVEN(sts)) return sts;

	return RTT__NOPICTURE;
}	

static int rttsys_aochanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local,
				int			signal_test_mode)
{
	rttsys_t_chan_list	*chanlist_ptr;
	rttsys_t_chan_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	int			signame_characters;

	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_chan_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_CHANALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_CHANALLOC,
			sizeof(rttsys_t_chan_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_chan_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_CHANALLOC;
	}
	chanlist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( chan_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last two segment */
        rtt_cut_segments( chanlist_ptr->channame, namebuf, 2);
	if ( strlen(chanlist_ptr->channame) > 10)
          rtt_cut_segments( chanlist_ptr->channame, namebuf, 1);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = chan_objid;
        if ( local)
        {
	  /* Get a direct link to channel object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		chanlist_ptr->chan_subid,
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
        chanlist_ptr->chan_objid = chan_objid;
        chanlist_ptr->test_on = 
	  &((pwr_sClass_ChanAo *)(chanlist_ptr->chan_ptr))->TestOn;
        chanlist_ptr->chan_number = 
	  ((pwr_sClass_ChanAo *)(chanlist_ptr->chan_ptr))->Number;
        strcpy( chanlist_ptr->chan_descript,
	  ((pwr_sClass_ChanAo *)(chanlist_ptr->chan_ptr))->Description);
        strcpy( chanlist_ptr->chan_ident,
	  ((pwr_sClass_ChanAo *)(chanlist_ptr->chan_ptr))->Identity);

        /* Get signal */
        sts = gdh_ObjidToName( 
		((pwr_sClass_ChanAo *)(chanlist_ptr->chan_ptr))->SigChanCon, 
		namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts))
        {
	  chanlist_ptr->connected = 0;
          strcpy( chanlist_ptr->signame, "-");
        }
        else
        {
	  chanlist_ptr->connected = 1;
	  signame_characters = 51; /* Should be fetched from menu entry... */
          if ( (int)strlen( namebuf) > signame_characters)
          {
	    /* Show the last part of the name */
	    strcpy( chanlist_ptr->signame, ".");
	    strcat( chanlist_ptr->signame, 
		&namebuf[strlen( namebuf)-signame_characters+1]);
	  }
	  else
	    strcpy( chanlist_ptr->signame, namebuf);

	  if ( !signal_test_mode)
	  {
	    /* Get a pointer to the value */
	    memset( &attrref, 0, sizeof(attrref));
            sts = gdh_ClassAttrToAttrref( pwr_cClass_Ao, ".ActualValue", &attrref);
            if ( EVEN(sts)) return sts;
	    attrref.Objid = 
		((pwr_sClass_ChanAo *)(chanlist_ptr->chan_ptr))->SigChanCon;
            if ( local)
            {
	      /* Get a direct link to the original object */
	      sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		&chanlist_ptr->value_subid);
	      if (EVEN(sts)) return sts;
            }
            else
            {
	      /* Get a subscription to the original object */
	      sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->value_subid);
	      if (EVEN(sts)) return sts;

	      sts = gdh_SubAssociateBuffer(
		chanlist_ptr->value_subid,
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		attrref.Size);
	      if (EVEN(sts)) return sts;
            }
	  }
	  else
	  {
	    /* Display testvalue instead of sigvalue */
	    chanlist_ptr->value_ptr = (pwr_tBoolean *)
		  (&((pwr_sClass_ChanAo *)(chanlist_ptr->chan_ptr))->TestValue);
	  }
	  /* Get the signal description */
	  strcat( namebuf, ".Description");
          sts = gdh_GetObjectInfo ( namebuf, 
		(pwr_tAddress *) chanlist_ptr->sig_descript, 
		sizeof(chanlist_ptr->sig_descript));
          if ( EVEN(sts))
	    strcpy( chanlist_ptr->sig_descript, "");
        }

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_CHANAO( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid 		objid;
  char			namebuf[80];
  pwr_tBoolean	 	local;
  pwr_tObjid		chan_objid;
  int			chan_alloc;
  char			card_name[80];
  rttsys_t_chan_list	*chanlist_ptr;
  int			i, j;

  static int		page;
  static rttsys_t_chan_list *chanlist;
  static int		chanlist_count;
  static int		display_mode;
  static int		signal_test_mode;
  static int		toggle = 0;

#define CHANAO_PAGESIZE 20

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    /* Check object first */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) 
    {
      rtt_message('E', "Node is down");
      return RTT__NOPICTURE;
    }
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    *picture = (char *) &dtt_systempicture_p33_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p33_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, CHANAO_MAXPAGE - 1);
    }
    CHANAO_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANAO_PAGESIZE) && (i < (page + 1) * CHANAO_PAGESIZE))
      {
        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
        menu_ptr->func2 = &rtt_object_parameters;
	if ( !signal_test_mode)
          menu_ptr->func3 = &rtt_crossref_channel;
	else
          menu_ptr->func3 = &rttsys_chanao_change_testmode;
        menu_ptr->argoi = chanlist_ptr->chan_objid;
        menu_ptr->value_ptr = (char *)chanlist_ptr->value_ptr;
	if ( signal_test_mode && chanlist_ptr->connected)
	  menu_ptr->priv |= RTT_PRIV_EL;
        menu_ptr++;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
	  if ( !signal_test_mode)
            *(float *) menu_ptr->value_ptr = 
	      ((pwr_sClass_ChanAo *)chanlist_ptr->chan_ptr)->SigValPolyCoef0 +
	      ((pwr_sClass_ChanAo *)chanlist_ptr->chan_ptr)->SigValPolyCoef1 *
	      *(pwr_tFloat32 *) chanlist_ptr->value_ptr;
          else
            menu_ptr->value_ptr = (char *) chanlist_ptr->value_ptr;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->test_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
          *(float *) menu_ptr->value_ptr = 0;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANAO_PAGESIZE ; i++)
    {
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr->priv &= ~RTT_PRIV_EL;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
      *(float *) menu_ptr->value_ptr = 0;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {

    signal_test_mode = rtt_signal_test_mode;
    display_mode = 0;
    strcpy( CHANAO_DISPLAY_TITLE, "Signal");
    page = 0;
    CHANAO_PAGE = page + 1;

    /* Get the card object */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) return sts;
    sts = gdh_ObjidToName ( objid, card_name, sizeof( card_name), cdh_mNName);
    if ( EVEN(sts)) return sts;
    sts = gdh_GetObjectLocation( objid, &local);
    if ( EVEN(sts)) return sts;

    /* Get all channels and store in the channel list */
    chan_alloc = 0;
    chanlist_count = 0;
    sts = gdh_GetChild( objid, &chan_objid);
    while( ODD(sts))
    {
      sts = rttsys_aochanlist_add( chan_objid, &chanlist,
		&chanlist_count, &chan_alloc, (int) local, 
		signal_test_mode);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextSibling ( chan_objid, &chan_objid);
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANAO_PAGESIZE) && (i < (page + 1) * CHANAO_PAGESIZE))
      {
        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
        menu_ptr->func2 = &rtt_object_parameters;
	if ( !signal_test_mode)
          menu_ptr->func3 = &rtt_crossref_channel;
	else
          menu_ptr->func3 = &rttsys_chanao_change_testmode;
        menu_ptr->argoi = chanlist_ptr->chan_objid;
        menu_ptr->value_ptr = (char *)chanlist_ptr->value_ptr;
	if ( signal_test_mode && chanlist_ptr->connected)
	  menu_ptr->priv |= RTT_PRIV_EL;
	menu_ptr++;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
	  if ( !signal_test_mode)
            *(float *) menu_ptr->value_ptr = 
	      ((pwr_sClass_ChanAo *)chanlist_ptr->chan_ptr)->SigValPolyCoef0 +
	      ((pwr_sClass_ChanAo *)chanlist_ptr->chan_ptr)->SigValPolyCoef1 *
	      *(pwr_tFloat32 *) chanlist_ptr->value_ptr;
          else
            menu_ptr->value_ptr = (char *) chanlist_ptr->value_ptr;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->test_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
          *(float *) menu_ptr->value_ptr = 0;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANAO_PAGESIZE ; i++)
    {
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr->priv &= ~RTT_PRIV_EL;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
      *(float *) menu_ptr->value_ptr = 0;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    CHANAO_MAXPAGE = max( 1, (chanlist_count - 1)/ CHANAO_PAGESIZE + 1);

    rtt_cut_segments( menu_ptr->value_ptr, card_name, 2);
    menu_ptr++;

    if ( signal_test_mode)
    {
      strcpy( CHANAO_INFO,
      "RETURN show signal |PF1 show chan |PF2 test on/off |PF3 change testvalue"
	);
      strcpy( CHANAO_TESTVALUE, "TestValue");
    }
    else
    {
      strcpy( CHANAO_INFO,
      " RETURN show signal | PF1 show channel | PF2 show cross | PF4 back      "
	);
      strcpy( CHANAO_TESTVALUE, " SigValue");
    }
    strcpy( CHANAO_TST, "Tst");
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    chanlist_ptr = chanlist;
    for ( i = 0; i < chanlist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( chanlist_ptr->chan_subid);
      if ( chanlist_ptr->connected)
        sts = gdh_UnrefObjectInfo( chanlist_ptr->value_subid);
      chanlist_ptr++;
    }
    free( chanlist);
    chanlist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
    if ( parameter_ptr == (char *) &CHANAO_DISPLAY_MODE)
    {
      if (CHANAO_DISPLAY_MODE)
        display_mode++;
      else
        display_mode--;
      if ( display_mode > CHAN_DISPLAYMODE_MAX)
        display_mode = 0;
      if ( display_mode < 0)
        display_mode = CHAN_DISPLAYMODE_MAX;

      switch( display_mode)
      {
	case CHAN_DISPLAYMODE_SIGNAL:
	  strcpy( CHANAO_DISPLAY_TITLE, "Signal");
	  break;
	case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	  strcpy( CHANAO_DISPLAY_TITLE, "Channel description");
	  break;
	case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	  strcpy( CHANAO_DISPLAY_TITLE, "Signal description");
	  break;
	case CHAN_DISPLAYMODE_CHAN_IDENT:
	  strcpy( CHANAO_DISPLAY_TITLE, "Identity");
	  break;
      }

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;
      chanlist_ptr = chanlist;
      j = 0;
      for ( i = 0; i < chanlist_count; i++)
      {
        if ( (i >= page * CHANAO_PAGESIZE) && (i < (page + 1) * CHANAO_PAGESIZE))
        {
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
  	  if ( chanlist_ptr->connected)
	  {
	    switch( display_mode)
	    {
	      case CHAN_DISPLAYMODE_SIGNAL:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	        break;
	      case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_IDENT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	        break;
	    }
            menu_ptr++;
	  }
	  else
	  {
  	    strcpy( menu_ptr->value_ptr, "");
            menu_ptr++;
	  }
          j++;
        }
        chanlist_ptr++;
      }
    }
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
    if ( signal_test_mode)
    {
      if ( toggle)
      {
        strcpy( CHANAO_TST, "Tst");
        strcpy( CHANAO_TESTVALUE, "TestValue");
	toggle = 0;
      }
      else
      {
        strcpy( CHANAO_TST, "   ");
        strcpy( CHANAO_TESTVALUE, "         ");
	toggle = 1;
      }
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANAO_PAGESIZE) && (i < (page + 1) * CHANAO_PAGESIZE))
      {
        /* Define PF buttons and Return */
        menu_ptr++;
        menu_ptr++;
	if ( chanlist_ptr->connected)
        {
	  if ( !signal_test_mode)
            *(float *) menu_ptr->value_ptr = 
	      ((pwr_sClass_ChanAo *)chanlist_ptr->chan_ptr)->SigValPolyCoef0 +
	      ((pwr_sClass_ChanAo *)chanlist_ptr->chan_ptr)->SigValPolyCoef1 *
	      *(pwr_tFloat32 *) chanlist_ptr->value_ptr;
        }
        menu_ptr++;
        menu_ptr++;
        menu_ptr++;
        j++;
      }
      chanlist_ptr++;
    }
  }
  return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_CHANCO()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the ChanCo objects of a co card.
*
**************************************************************************/

static int rttsys_cochanlist_add( pwr_tObjid		chan_objid,
				rttsys_t_chan_list	**objectlist,
				int			*objectlist_count,
				int			*alloc,
				int			local)
{
	rttsys_t_chan_list	*chanlist_ptr;
	rttsys_t_chan_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	int			signame_characters;

	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_chan_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_CHANALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_CHANALLOC,
			sizeof(rttsys_t_chan_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_chan_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_CHANALLOC;
	}
	chanlist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( chan_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last two segment */
        rtt_cut_segments( chanlist_ptr->channame, namebuf, 2);
	if ( strlen(chanlist_ptr->channame) > 10)
          rtt_cut_segments( chanlist_ptr->channame, namebuf, 1);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = chan_objid;
        if ( local)
        {
	  /* Get a direct link to channel object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->chan_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		chanlist_ptr->chan_subid,
		(pwr_tAddress *) &chanlist_ptr->chan_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
        chanlist_ptr->chan_objid = chan_objid;
        chanlist_ptr->conversion_on = 
	  &((pwr_sClass_ChanCo *)(chanlist_ptr->chan_ptr))->ConversionOn;
        chanlist_ptr->chan_number = 
	  ((pwr_sClass_ChanCo *)(chanlist_ptr->chan_ptr))->Number;
        strcpy( chanlist_ptr->chan_descript,
	  ((pwr_sClass_ChanCo *)(chanlist_ptr->chan_ptr))->Description);
        strcpy( chanlist_ptr->chan_ident,
	  ((pwr_sClass_ChanCo *)(chanlist_ptr->chan_ptr))->Identity);

        /* Get signal */
        sts = gdh_ObjidToName( 
		((pwr_sClass_ChanCo *)(chanlist_ptr->chan_ptr))->SigChanCon, 
		namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts))
        {
	  chanlist_ptr->connected = 0;
          strcpy( chanlist_ptr->signame, "-");
        }
        else
        {
	  chanlist_ptr->connected = 1;
	  signame_characters = 51; /* Should be fetched from menu entry... */
          if ( (int)strlen( namebuf) > signame_characters)
          {
	    /* Show the last part of the name */
	    strcpy( chanlist_ptr->signame, ".");
	    strcat( chanlist_ptr->signame, 
		&namebuf[strlen( namebuf)-signame_characters+1]);
	  }
	  else
	    strcpy( chanlist_ptr->signame, namebuf);

	  /* Get a pointer to the value */
	  memset( &attrref, 0, sizeof(attrref));
          sts = gdh_ClassAttrToAttrref( pwr_cClass_Co, ".RawValue", &attrref);
          if ( EVEN(sts)) return sts;
	  attrref.Objid = 
		((pwr_sClass_ChanCo *)(chanlist_ptr->chan_ptr))->SigChanCon;
          if ( local)
          {
	    /* Get a direct link to the original object */
	    sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		&chanlist_ptr->value_subid);
	    if (EVEN(sts)) return sts;
          }
          else
          {
	    /* Get a subscription to the original object */
	    sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&chanlist_ptr->value_subid);
	    if (EVEN(sts)) return sts;

	    sts = gdh_SubAssociateBuffer(
		chanlist_ptr->value_subid,
		(pwr_tAddress *) &chanlist_ptr->value_ptr,
		attrref.Size);
	    if (EVEN(sts)) return sts;
          }
	  /* Get the signal description */
	  strcat( namebuf, ".Description");
          sts = gdh_GetObjectInfo ( namebuf, 
		(pwr_tAddress *) chanlist_ptr->sig_descript, 
		sizeof(chanlist_ptr->sig_descript));
          if ( EVEN(sts))
	    strcpy( chanlist_ptr->sig_descript, "");
        }

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_CHANCO( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid 		objid;
  char			namebuf[80];
  pwr_tBoolean	 	local;
  pwr_tObjid		chan_objid;
  int			chan_alloc;
  char			card_name[80];
  rttsys_t_chan_list	*chanlist_ptr;
  int			i, j;

  static int		page;
  static rttsys_t_chan_list *chanlist;
  static int		chanlist_count;
  static int		display_mode;

#define CHANCO_PAGESIZE 20

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    /* Check object first */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) 
    {
      rtt_message('E', "Node is down");
      return RTT__NOPICTURE;
    }
    sts = gdh_ObjidToName ( objid, namebuf, sizeof( namebuf), cdh_mNName);
    if ( EVEN(sts)) return sts;

    *picture = (char *) &dtt_systempicture_p34_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p34_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, CHANCO_MAXPAGE - 1);
    }
    CHANCO_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANCO_PAGESIZE) && (i < (page + 1) * CHANCO_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = &rtt_crossref_channel;
        menu_ptr->argoi = chanlist_ptr->chan_objid;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
          menu_ptr->value_ptr = (char *)chanlist_ptr->value_ptr;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->conversion_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
	  menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANCO_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {

    display_mode = 0;
    strcpy( CHANCO_DISPLAY_TITLE, "Signal");
    page = 0;
    CHANCO_PAGE = page + 1;

    /* Get the card object */
    sts = gdh_NameToObjid ( objectname, &objid);
    if ( EVEN(sts)) return sts;
    sts = gdh_ObjidToName ( objid, card_name, sizeof( card_name), cdh_mNName);
    if ( EVEN(sts)) return sts;
    sts = gdh_GetObjectLocation( objid, &local);
    if ( EVEN(sts)) return sts;

    /* Get all channels and store in the channel list */
    chan_alloc = 0;
    chanlist_count = 0;
    sts = gdh_GetChild( objid, &chan_objid);
    while( ODD(sts))
    {
      sts = rttsys_cochanlist_add( chan_objid, &chanlist,
		&chanlist_count, &chan_alloc, (int) local);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextSibling ( chan_objid, &chan_objid);
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    chanlist_ptr = chanlist;
    j = 0;
    for ( i = 0; i < chanlist_count; i++)
    {
      if ( (i >= page * CHANCO_PAGESIZE) && (i < (page + 1) * CHANCO_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_show_signal_from_chan;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = &rtt_crossref_channel;
        menu_ptr->argoi = chanlist_ptr->chan_objid;
	strcpy( menu_ptr->value_ptr, chanlist_ptr->channame);
        menu_ptr++;
	if ( chanlist_ptr->connected)
	{
          menu_ptr->value_ptr = (char *)chanlist_ptr->value_ptr;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) chanlist_ptr->conversion_on;
          menu_ptr++;
	  switch( display_mode)
	  {
	    case CHAN_DISPLAYMODE_SIGNAL:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	      break;
	    case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	      break;
	    case CHAN_DISPLAYMODE_CHAN_IDENT:
	      strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	      break;
	  }
          menu_ptr++;
	}
	else
	{
	  menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
  	  strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
          menu_ptr++;
	}
        j++;
      }
      chanlist_ptr++;
    }
    for ( i = j; i < CHANCO_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    CHANCO_MAXPAGE = max( 1, (chanlist_count - 1)/ CHANCO_PAGESIZE + 1);

    rtt_cut_segments( menu_ptr->value_ptr, card_name, 2);
    menu_ptr++;
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    chanlist_ptr = chanlist;
    for ( i = 0; i < chanlist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( chanlist_ptr->chan_subid);
      if ( chanlist_ptr->connected)
        sts = gdh_UnrefObjectInfo( chanlist_ptr->value_subid);
      chanlist_ptr++;
    }
    free( chanlist);
    chanlist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
    if ( parameter_ptr == (char *) &CHANCO_DISPLAY_MODE)
    {
      if (CHANCO_DISPLAY_MODE)
        display_mode++;
      else
        display_mode--;
      if ( display_mode > CHAN_DISPLAYMODE_MAX)
        display_mode = 0;
      if ( display_mode < 0)
        display_mode = CHAN_DISPLAYMODE_MAX;

      switch( display_mode)
      {
	case CHAN_DISPLAYMODE_SIGNAL:
	  strcpy( CHANCO_DISPLAY_TITLE, "Signal");
	  break;
	case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	  strcpy( CHANCO_DISPLAY_TITLE, "Channel description");
	  break;
	case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	  strcpy( CHANCO_DISPLAY_TITLE, "Signal description");
	  break;
	case CHAN_DISPLAYMODE_CHAN_IDENT:
	  strcpy( CHANCO_DISPLAY_TITLE, "Identity");
	  break;
      }

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;
      chanlist_ptr = chanlist;
      j = 0;
      for ( i = 0; i < chanlist_count; i++)
      {
        if ( (i >= page * CHANCO_PAGESIZE) && (i < (page + 1) * CHANCO_PAGESIZE))
        {
          menu_ptr++;
          menu_ptr++;
          menu_ptr++;
  	  if ( chanlist_ptr->connected)
	  {
	    switch( display_mode)
	    {
	      case CHAN_DISPLAYMODE_SIGNAL:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->signame);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_descript);
	        break;
	      case CHAN_DISPLAYMODE_SIG_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->sig_descript);
	        break;
	      case CHAN_DISPLAYMODE_CHAN_IDENT:
	        strcpy( menu_ptr->value_ptr, chanlist_ptr->chan_ident);
	        break;
	    }
            menu_ptr++;
	  }
	  else
	  {
  	    strcpy( menu_ptr->value_ptr, "");
            menu_ptr++;
	  }
          j++;
        }
        chanlist_ptr++;
      }
    }
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_DEVICE()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the Device objects of the system.
*
**************************************************************************/

static int rttsys_devicelist_add( pwr_tObjid		device_objid,
				rttsys_t_device_list	**objectlist,
				int			*objectlist_count,
				int			*alloc)
{
	rttsys_t_device_list	*devicelist_ptr;
	rttsys_t_device_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	pwr_tClassId		class;
	int			local;


	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_device_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_ALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_ALLOC,
			sizeof(rttsys_t_device_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_device_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_ALLOC;
	}
	devicelist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( device_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last two segment only */
	rtt_cut_segments( devicelist_ptr->devicename, namebuf, 2);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = device_objid;
	local = 1; /* Only local object implemented so far */
        if ( local)
        {
	  /* Get a direct link to device object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &devicelist_ptr->device_ptr,
		&devicelist_ptr->device_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&devicelist_ptr->device_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		devicelist_ptr->device_subid,
		(pwr_tAddress *) &devicelist_ptr->device_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
	sts = gdh_GetObjectClass ( device_objid, &class);
	if ( EVEN(sts)) return sts;
        devicelist_ptr->device_objid = device_objid;
        devicelist_ptr->device_class = class;
        devicelist_ptr->device_class_objid = cdh_ClassIdToObjid( class);

	memset( &attrref, 0, sizeof(attrref));
        sts = gdh_ClassAttrToAttrref( class, ".ErrorCount", &attrref);
	if ( ODD(sts))
	{
	  devicelist_ptr->error_count = (pwr_tUInt32 *)
		((char *)(devicelist_ptr->device_ptr) + attrref.Offset);
	}
	else
	  devicelist_ptr->error_count = (pwr_tUInt32 *) RTT_ERASE;
        sts = gdh_ClassAttrToAttrref( class, ".Process", &attrref);
	if ( ODD(sts))
	{
	  devicelist_ptr->process = (pwr_tUInt32 *)
		((char *)(devicelist_ptr->device_ptr)+attrref.Offset);
	}
	else
	  devicelist_ptr->process = (pwr_tUInt32 *) RTT_ERASE;
        sts = gdh_ClassAttrToAttrref( class, ".ThreadObject", &attrref);
	if ( ODD(sts))
	{
	  devicelist_ptr->thread_object = (pwr_tObjid *)
		((char *)(devicelist_ptr->device_ptr)+attrref.Offset);
	}
	else
	  devicelist_ptr->thread_object = (pwr_tObjid *) RTT_ERASE;
        sts = gdh_ClassAttrToAttrref( class, ".RegAddress", &attrref);
	if ( ODD(sts))
	{
	  devicelist_ptr->reg_address = (pwr_tUInt32 *)
		((char *)(devicelist_ptr->device_ptr)+attrref.Offset);
	}
	else
	{
          sts = gdh_ClassAttrToAttrref( class, ".CardAddress", &attrref);
	  if ( ODD(sts))
	  {
	    devicelist_ptr->reg_address = (pwr_tUInt32 *)
		((char *)(devicelist_ptr->device_ptr)+attrref.Offset);
	  }
	  else
	  {
	    devicelist_ptr->reg_address = (pwr_tUInt32 *) RTT_ERASE;
	  }
	}

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_DEVICE( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid		device_objid;
  int			device_alloc;
  rttsys_t_device_list	*devicelist_ptr;
  int			i, j;
  pwr_tObjid		rack_objid;
  int			rack_class_cnt;
  pwr_tClassId		*rack_class;

  static int		page;
  static rttsys_t_device_list *devicelist;
  static int		devicelist_count;

#define DEVICE_PAGESIZE 20

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p15_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p15_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, DEVICE_MAXPAGE - 1);
    }
    DEVICE_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    devicelist_ptr = devicelist;
    j = 0;
    for ( i = 0; i < devicelist_count; i++)
    {
      if ( (i >= page * DEVICE_PAGESIZE) && (i < (page + 1) * DEVICE_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, devicelist_ptr->devicename);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_chan_start;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->argoi = devicelist_ptr->device_objid;
        menu_ptr++;
        *(pwr_tObjid *) menu_ptr->value_ptr =
			devicelist_ptr->device_class_objid;
        menu_ptr++;
        menu_ptr->value_ptr = (char *)devicelist_ptr->error_count;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) devicelist_ptr->process;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) devicelist_ptr->thread_object;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) devicelist_ptr->reg_address;
        menu_ptr++;
        j++;
      }
      devicelist_ptr++;
    }
    for ( i = j; i < DEVICE_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      *(pwr_tObjid *) menu_ptr->value_ptr = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    page = 0;
    DEVICE_PAGE = page + 1;

    /* Get the rack objects */
    sts = io_GetIoTypeClasses( io_eType_Rack, &rack_class, &rack_class_cnt);
    if ( EVEN(sts)) return sts;

    device_alloc = 0;
    devicelist_count = 0;
    for ( i = 0; i < rack_class_cnt; i++)
    {
      sts = gdh_GetClassList ( rack_class[i], &rack_objid);
      while (ODD(sts))
      {
        /* Get all children */
        sts = gdh_GetChild( rack_objid, &device_objid);
        while( ODD(sts))
        {
          sts = rttsys_devicelist_add( device_objid, &devicelist,
		&devicelist_count, &device_alloc);
          if ( EVEN(sts)) return sts;

          sts = gdh_GetNextSibling ( device_objid, &device_objid);
        }
        sts = gdh_GetNextObject ( rack_objid, &rack_objid);
      }
    }
    free( (char *)rack_class);

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    devicelist_ptr = devicelist;
    j = 0;
    for ( i = 0; i < devicelist_count; i++)
    {
      if ( (i >= page * DEVICE_PAGESIZE) && (i < (page + 1) * DEVICE_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, devicelist_ptr->devicename);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_chan_start;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->argoi = devicelist_ptr->device_objid;
        menu_ptr++;
        *(pwr_tObjid *) menu_ptr->value_ptr =
				devicelist_ptr->device_class_objid;
        menu_ptr++;
        menu_ptr->value_ptr = (char *)devicelist_ptr->error_count;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) devicelist_ptr->process;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) devicelist_ptr->thread_object;
        menu_ptr++;
        menu_ptr->value_ptr = (char *) devicelist_ptr->reg_address;
        menu_ptr++;
        j++;
      }
      devicelist_ptr++;
    }
    for ( i = j; i < DEVICE_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      *(pwr_tObjid *) menu_ptr->value_ptr = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
    }
    DEVICE_MAXPAGE = max( 1, (devicelist_count - 1)/ DEVICE_PAGESIZE + 1);
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    devicelist_ptr = devicelist;
    for ( i = 0; i < devicelist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( devicelist_ptr->device_subid);
      devicelist_ptr++;
    }
    free( devicelist);
    devicelist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_REMNODE()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the Remnode objects of the system.
*
**************************************************************************/

static int rttsys_remnodelist_add( pwr_tObjid		remnode_objid,
				rttsys_t_remnode_list	**objectlist,
				int			*objectlist_count,
				int			*alloc)
{
	rttsys_t_remnode_list	*remnodelist_ptr;
	rttsys_t_remnode_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	int			local;


	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_remnode_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_ALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_ALLOC,
			sizeof(rttsys_t_remnode_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_remnode_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_ALLOC;
	}
	remnodelist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( remnode_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last two segment only */
	rtt_cut_segments( remnodelist_ptr->remnodename, namebuf, 2);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = remnode_objid;
	local = 1; /* Only local object implemented so far */
        if ( local)
        {
	  /* Get a direct link to remnode object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &remnodelist_ptr->remnode_ptr,
		&remnodelist_ptr->remnode_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&remnodelist_ptr->remnode_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		remnodelist_ptr->remnode_subid,
		(pwr_tAddress *) &remnodelist_ptr->remnode_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
        remnodelist_ptr->remnode_objid = remnode_objid;

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_REMNODE( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid		remnode_objid;
  int			remnode_alloc;
  rttsys_t_remnode_list	*remnodelist_ptr;
  int			i, j;

  static int		page;
  static rttsys_t_remnode_list *remnodelist;
  static int		remnodelist_count;
  static int		display_mode;

#define REMNODE_PAGESIZE 20
#define REMNODE_DISPLAYMODE_DESCRIPT 	0
#define REMNODE_DISPLAYMODE_ADDRESS	1
#define REMNODE_DISPLAYMODE_MAX		1

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p35_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p35_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, REMNODE_MAXPAGE - 1);
    }
    REMNODE_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    remnodelist_ptr = remnodelist;
    j = 0;
    for ( i = 0; i < remnodelist_count; i++)
    {
      if ( (i >= page * REMNODE_PAGESIZE) && (i < (page + 1) * REMNODE_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, remnodelist_ptr->remnodename);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_remtrans_start;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->argoi = remnodelist_ptr->remnode_objid;
        menu_ptr++;
	switch( remnodelist_ptr->remnode_ptr->TransportType)
	{
	  case 1:
	    /* ALCM */
	    strcpy( menu_ptr->value_ptr, "ALCM");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        sprintf( menu_ptr->value_ptr, "%5d (Area)  %5d (Node)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	        break;
            }
	    break;
	  case 2:
	    /* PAMS/DMQ */
	    strcpy( menu_ptr->value_ptr, "PAMS/DMQ");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        sprintf( menu_ptr->value_ptr, "%5d (Group) %5d (Host Proc)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	        break;
	    }
	    break;
	  case 3:
	    /* 3964R VNET */
	    strcpy( menu_ptr->value_ptr, "3964R VNET");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        strcpy( menu_ptr->value_ptr, "");
	        break;
	    }
	    break;
	  case 5:
	    /* RK512 */
	    strcpy( menu_ptr->value_ptr, "RK512");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        strcpy( menu_ptr->value_ptr, "");
	        break;
	    }
	    break;
	  case 6:
	    /* TCP/IP Client */
	    strcpy( menu_ptr->value_ptr, "TCP/IP Cli");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        sprintf( menu_ptr->value_ptr, "%5d (Remote)%5d (Host Port)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	        break;
	    }
	    break;
	  case 7:
	    /* TCP/IP Server */
	    strcpy( menu_ptr->value_ptr, "TCP/IP Srv");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        sprintf( menu_ptr->value_ptr, "%5d (Remote)%5d (Host Port)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	        break;
	    }
	    break;
	  default:
	    strcpy( menu_ptr->value_ptr, "Unknown");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        strcpy( menu_ptr->value_ptr, "");
	        break;
	    }
	}
        menu_ptr++;
        j++;
      }
      remnodelist_ptr++;
    }
    for ( i = j; i < REMNODE_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    display_mode = 0;
    strcpy( REMNODE_DISPLAY_TITLE, "Description");
    page = 0;
    REMNODE_PAGE = page + 1;

    /* Get the RemNode objects */
    remnode_alloc = 0;
    remnodelist_count = 0;
    sts = gdh_GetClassList ( pwr_cClass_RemNode, &remnode_objid);
    while (ODD(sts))
    {
      sts = rttsys_remnodelist_add( remnode_objid, &remnodelist,
		&remnodelist_count, &remnode_alloc);
      if ( EVEN(sts)) return sts;
      sts = gdh_GetNextObject ( remnode_objid, &remnode_objid);
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    remnodelist_ptr = remnodelist;
    j = 0;
    for ( i = 0; i < remnodelist_count; i++)
    {
      if ( (i >= page * REMNODE_PAGESIZE) && (i < (page + 1) * REMNODE_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, remnodelist_ptr->remnodename);

        /* Define PF buttons and Return */
        menu_ptr->func = &rttsys_remtrans_start;
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->argoi = remnodelist_ptr->remnode_objid;
        menu_ptr++;
	switch( remnodelist_ptr->remnode_ptr->TransportType)
	{
	  case 1:
	    /* ALCM */
	    strcpy( menu_ptr->value_ptr, "ALCM");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        sprintf( menu_ptr->value_ptr, "%5d (Area)  %5d (Node)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	        break;
            }
	    break;
	  case 2:
	    /* PAMS/DMQ */
	    strcpy( menu_ptr->value_ptr, "PAMS/DMQ");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        sprintf( menu_ptr->value_ptr, "%5d (Group) %5d (Host Process)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	        break;
	    }
	    break;
	  case 3:
	    /* 3964R VNET */
	    strcpy( menu_ptr->value_ptr, "3964R VNET");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        strcpy( menu_ptr->value_ptr, "");
	        break;
	    }
	    break;
	  case 5:
	    /* RK512 */
	    strcpy( menu_ptr->value_ptr, "RK512");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        strcpy( menu_ptr->value_ptr, "");
	        break;
	    }
	    break;
	  case 6:
	    /* TCP/IP Client */
	    strcpy( menu_ptr->value_ptr, "TCP/IP Cli");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        sprintf( menu_ptr->value_ptr, "%5d (Remote)%5d (Host Port)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	        break;
	    }
	    break;
	  case 7:
	    /* TCP/IP Server */
	    strcpy( menu_ptr->value_ptr, "TCP/IP Srv");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        sprintf( menu_ptr->value_ptr, "%5d (Remote)%5d (Host Port)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	        break;
	    }
	    break;
	  default:
	    strcpy( menu_ptr->value_ptr, "Unknown");
            menu_ptr++;
	    switch( display_mode)
            {
	      case REMNODE_DISPLAYMODE_DESCRIPT:
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
	        break;
	      case REMNODE_DISPLAYMODE_ADDRESS:
	        strcpy( menu_ptr->value_ptr, "");
	        break;
	    }
	}
        menu_ptr++;
        j++;
      }
      remnodelist_ptr++;
    }
    for ( i = j; i < REMNODE_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
    }
    REMNODE_MAXPAGE = max( 1, (remnodelist_count - 1)/ REMNODE_PAGESIZE + 1);
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    remnodelist_ptr = remnodelist;
    for ( i = 0; i < remnodelist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( remnodelist_ptr->remnode_subid);
      remnodelist_ptr++;
    }
    free( remnodelist);
    remnodelist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
    if ( parameter_ptr == (char *) &REMNODE_DISPLAY_MODE)
    {
      if (REMNODE_DISPLAY_MODE)
        display_mode++;
      else
        display_mode--;
      if ( display_mode > REMNODE_DISPLAYMODE_MAX)
        display_mode = 0;
      if ( display_mode < 0)
        display_mode = REMNODE_DISPLAYMODE_MAX;

      switch( display_mode)
      {
	case REMNODE_DISPLAYMODE_DESCRIPT:
	  strcpy( REMNODE_DISPLAY_TITLE, "Description");
	  break;
	case REMNODE_DISPLAYMODE_ADDRESS:
	  strcpy( REMNODE_DISPLAY_TITLE, "Address");
	  break;
      }

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;
      remnodelist_ptr = remnodelist;
      j = 0;
      for ( i = 0; i < remnodelist_count; i++)
      {
        if ( (i >= page * REMNODE_PAGESIZE) && (i < (page + 1) * REMNODE_PAGESIZE))
        {
          menu_ptr++;
          menu_ptr++;
	  switch( remnodelist_ptr->remnode_ptr->TransportType)
	  {
	    case 1:
	      /* ALCM */
	      if (display_mode == REMNODE_DISPLAYMODE_DESCRIPT)
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
              else
	        sprintf( menu_ptr->value_ptr, "%5d (Area)  %5d (Node)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	      break;
	    case 2:
	      /* PAMS/DMQ */
	      if (display_mode == REMNODE_DISPLAYMODE_DESCRIPT)
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
              else
	        sprintf( menu_ptr->value_ptr, "%5d (Group) %5d (Host Process)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	      break;
	    case 3:
	      /* 3964R VNET */
	      if (display_mode == REMNODE_DISPLAYMODE_DESCRIPT)
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
              else
	        strcpy( menu_ptr->value_ptr, "");
	      break;
	    case 5:
	      /* RK512 */
	      if (display_mode == REMNODE_DISPLAYMODE_DESCRIPT)
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
              else
	        strcpy( menu_ptr->value_ptr, "");
	      break;
	    case 6:
	      /* TCP/IP Client */
	      if (display_mode == REMNODE_DISPLAYMODE_DESCRIPT)
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
              else
	        sprintf( menu_ptr->value_ptr, "%5d (Remote)%5d (Host Port)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	      break;
	    case 7:
	      /* TCP/IP Server */
	      if (display_mode == REMNODE_DISPLAYMODE_DESCRIPT)
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
              else
	        sprintf( menu_ptr->value_ptr, "%5d (Remote)%5d (Host Port)",
			remnodelist_ptr->remnode_ptr->Address[0],
			remnodelist_ptr->remnode_ptr->Address[1]);
	      break;
	    default:
	      if (display_mode == REMNODE_DISPLAYMODE_DESCRIPT)
	        strcpy( menu_ptr->value_ptr, 
			remnodelist_ptr->remnode_ptr->Description);
              else
	        strcpy( menu_ptr->value_ptr, "");
	  }
	  menu_ptr++;
          j++;
        }
        remnodelist_ptr++;
      }
    }
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_REMTRANS()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the Remtrans objects of the system.
*
**************************************************************************/

static int rttsys_remtrans_start( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	char	objectname[80];
	int	sts;
	pwr_tObjid	chan_objid;
	
	sts = gdh_ObjidToName ( objid, objectname, sizeof(objectname), cdh_mNName);
	if ( EVEN(sts)) return sts;

	/* Get the class of the channels */
	sts = gdh_GetChild ( objid, &chan_objid);
	if ( EVEN(sts))
	{
	  rtt_message( 'E', "RemTrans objects not found");
	  return RTT__NOPICTURE;
	}

	sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, objectname, objectname, 
		0, &RTTSYS_REMTRANS);
	return sts;
}

static int rttsys_remtranslist_add( pwr_tObjid		remtrans_objid,
				rttsys_t_remtrans_list	**objectlist,
				int			*objectlist_count,
				int			*alloc)
{
	rttsys_t_remtrans_list	*remtranslist_ptr;
	rttsys_t_remtrans_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	int			local;


	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_remtrans_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_ALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_ALLOC,
			sizeof(rttsys_t_remtrans_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_remtrans_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_ALLOC;
	}
	remtranslist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( remtrans_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last one segment only */
	rtt_cut_segments( remtranslist_ptr->remtransname, namebuf, 1);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = remtrans_objid;
	local = 1; /* Only local object implemented so far */
        if ( local)
        {
	  /* Get a direct link to remtrans object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &remtranslist_ptr->remtrans_ptr,
		&remtranslist_ptr->remtrans_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&remtranslist_ptr->remtrans_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		remtranslist_ptr->remtrans_subid,
		(pwr_tAddress *) &remtranslist_ptr->remtrans_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
        remtranslist_ptr->remtrans_objid = remtrans_objid;

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

static int rttsys_remtrans_buffer( menu_ctx	ctx, 
				pwr_tObjid	objid,
				void		*arg1,
				void		*arg2,
				void		*arg3,
				void		*arg4)
{
	int	sts;
	pwr_tObjid	buff_objid;
	char		remtrans_name[120];
	char		namebuf[140];
	char		structname[40];
	char		structfile[80];
	
	/* Show attributes of first child */
	sts = gdh_GetChild ( objid, &buff_objid);
	if ( EVEN(sts))
	{
	  rtt_message( 'E', "No buffer object found");
	  return RTT__NOPICTURE;
	}

	/* Try to find structfile and structname i RemTrans-object */
	
	sts = gdh_ObjidToName( objid, remtrans_name, sizeof(remtrans_name), 
			cdh_mNName);
	if (EVEN(sts)) return sts;

	strcpy( namebuf, remtrans_name);
	strcat( namebuf, ".StructName");
        sts = gdh_GetObjectInfo ( namebuf,
		(pwr_tAddress *) structname,
		sizeof(structname));
        if ( ODD(sts) && (strcmp(structname, "") != 0))
	{
	  strcpy( namebuf, remtrans_name);
	  strcat( namebuf, ".StructFile");
          sts = gdh_GetObjectInfo ( namebuf,
		(pwr_tAddress *) structfile,
		sizeof(structfile));
          if ( ODD(sts) && (strcmp(structfile, "") != 0))
	  {
	    sts = rtt_show_object_as_struct( ctx, buff_objid, structname, 
		structfile);
	    if ( sts == RTT__SUCCESS)
	      return sts;
	  }
	}

	/* Search for structfile was not successful, show as buffer object */
	sts = rtt_object_parameters( ctx, buff_objid, 0, 0, 0, 0);
	if (EVEN(sts)) 	
	{
	  rtt_message('E',"Unable to open object");
  	  return RTT__NOPICTURE;
	}
	return RTT__SUCCESS;
}

int RTTSYS_REMTRANS( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid		remtrans_objid;
  int			remtrans_alloc;
  rttsys_t_remtrans_list	*remtranslist_ptr;
  int			i, j;
  pwr_tObjid		remnode_objid;
  int			all;
  pwr_tClassId		class;

  static int		page;
  static rttsys_t_remtrans_list *remtranslist;
  static int		remtranslist_count;

#define REMTRANS_PAGESIZE 20

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p36_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p36_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, REMTRANS_MAXPAGE - 1);
    }
    REMTRANS_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    remtranslist_ptr = remtranslist;
    j = 0;
    for ( i = 0; i < remtranslist_count; i++)
    {
      if ( (i >= page * REMTRANS_PAGESIZE) && (i < (page + 1) * REMTRANS_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, remtranslist_ptr->remtransname);

        /* Define PF buttons and Return */
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = &rttsys_remtrans_buffer;
        menu_ptr->argoi = remtranslist_ptr->remtrans_objid;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->DataValid;
        menu_ptr++;
	if ( remtranslist_ptr->remtrans_ptr->Direction == 1) 
          strcpy( menu_ptr->value_ptr, "Rcv");
	else if ( remtranslist_ptr->remtrans_ptr->Direction == 2) 
          strcpy( menu_ptr->value_ptr, "Snd");
        menu_ptr++;

        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->TransCount;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->TransTime;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->ErrCount;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->LastSts;
        menu_ptr++;
        j++;
      }
      remtranslist_ptr++;
    }
    for ( i = j; i < REMTRANS_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    page = 0;
    REMTRANS_PAGE = page + 1;

    /* Get the RemNode object */
    if ( objectname == NULL)
    {
      /* Call from system menu, show all objects */
      all = 1;
    }
    else if ( !strcmp( objectname, "REMTRANS"))
    {
      /* Call from "show remtrans", show all objects */
      all = 1;
    }
    else
    {
      /* Call from "show remnode", show children to remnode object */
      sts = gdh_NameToObjid ( objectname, &remnode_objid);
      if ( EVEN(sts)) return sts;
      all = 0;
    }

    /* Get the rack objects */
    remtrans_alloc = 0;
    remtranslist_count = 0;
    if ( all)
    {
      /* Get all RemTrans objects */
      sts = gdh_GetClassList ( pwr_cClass_RemTrans, &remtrans_objid);
      while (ODD(sts))
      {
        sts = rttsys_remtranslist_add( remtrans_objid, &remtranslist,
		&remtranslist_count, &remtrans_alloc);
        if ( EVEN(sts)) return sts;
        sts = gdh_GetNextObject ( remtrans_objid, &remtrans_objid);
      }
    }
    else
    {
      /* Get all children */
      sts = gdh_GetChild( remnode_objid, &remtrans_objid);
      while( ODD(sts))
      {
        sts = gdh_GetObjectClass( remtrans_objid, &class);
        if ( EVEN(sts)) return sts;
	if ( class == pwr_cClass_RemTrans)
        {
          sts = rttsys_remtranslist_add( remtrans_objid, &remtranslist,
		&remtranslist_count, &remtrans_alloc);
          if ( EVEN(sts)) return sts;
        }
        sts = gdh_GetNextSibling ( remtrans_objid, &remtrans_objid);
      }
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    remtranslist_ptr = remtranslist;
    j = 0;
    for ( i = 0; i < remtranslist_count; i++)
    {
      if ( (i >= page * REMTRANS_PAGESIZE) && (i < (page + 1) * REMTRANS_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, remtranslist_ptr->remtransname);

        /* Define PF buttons and Return */
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = &rttsys_remtrans_buffer;
        menu_ptr->argoi = remtranslist_ptr->remtrans_objid;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->DataValid;
        menu_ptr++;
	if ( remtranslist_ptr->remtrans_ptr->Direction == 1) 
          strcpy( menu_ptr->value_ptr, "Rcv");
	else if ( remtranslist_ptr->remtrans_ptr->Direction == 2) 
          strcpy( menu_ptr->value_ptr, "Snd");
        menu_ptr++;

        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->TransCount;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->TransTime;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->ErrCount;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &remtranslist_ptr->remtrans_ptr->LastSts;
        menu_ptr++;
        j++;
      }
      remtranslist_ptr++;
    }
    for ( i = j; i < REMTRANS_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
    }
    REMTRANS_MAXPAGE = max( 1, (remtranslist_count - 1)/ REMTRANS_PAGESIZE + 1);
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    remtranslist_ptr = remtranslist;
    for ( i = 0; i < remtranslist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( remtranslist_ptr->remtrans_subid);
      remtranslist_ptr++;
    }
    free( remtranslist);
    remtranslist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
  }
  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_RUNNINGTIME()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the RunningTime objects of the system.
*
**************************************************************************/

static int rttsys_runningtimelist_add( pwr_tObjid		runningtime_objid,
				rttsys_t_runningtime_list	**objectlist,
				int			*objectlist_count,
				int			*alloc)
{
	rttsys_t_runningtime_list	*runningtimelist_ptr;
	rttsys_t_runningtime_list	*new_objectlist;
	char			namebuf[120];
	int			sts;
	pwr_sAttrRef		attrref;
	int			local;


	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( RTTSYS_ALLOC , sizeof(rttsys_t_runningtime_list));
	  if ( *objectlist == 0)
	    return RTT__NOMEMORY;
	  *alloc = RTTSYS_ALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + RTTSYS_ALLOC,
			sizeof(rttsys_t_runningtime_list));
	  if ( new_objectlist == 0)
	    return RTT__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof(rttsys_t_runningtime_list));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += RTTSYS_ALLOC;
	}
	runningtimelist_ptr = *objectlist + *objectlist_count;

        sts = gdh_ObjidToName ( runningtime_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        /* Last one segment only */
	rtt_cut_segments( runningtimelist_ptr->runningtimename, namebuf, 1);

        memset( &attrref, 0, sizeof(attrref));
        attrref.Objid = runningtime_objid;
	local = 1; /* Only local object implemented so far */
        if ( local)
        {
	  /* Get a direct link to runningtime object */
	  sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &runningtimelist_ptr->runningtime_ptr,
		&runningtimelist_ptr->runningtime_subid);
	  if (EVEN(sts)) return sts;
        }
        else
        {
	  /* Get a subscription to the original object */
	  sts = gdh_SubRefObjectInfoAttrref( &attrref,
		&runningtimelist_ptr->runningtime_subid);
	  if (EVEN(sts)) return sts;

	  sts = gdh_SubAssociateBuffer(
		runningtimelist_ptr->runningtime_subid,
		(pwr_tAddress *) &runningtimelist_ptr->runningtime_ptr,
		attrref.Size);
	  if (EVEN(sts)) return sts;
        }
        runningtimelist_ptr->runningtime_objid = runningtime_objid;

	(*objectlist_count)++;
	return RTT__SUCCESS;
}

int RTTSYS_RUNNINGTIME( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 

  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pwr_tObjid		runningtime_objid;
  int			runningtime_alloc;
  rttsys_t_runningtime_list	*runningtimelist_ptr;
  int			i, j;

  static int		page;
  static rttsys_t_runningtime_list *runningtimelist;
  static int		runningtimelist_count;

#define RUNNINGTIME_PAGESIZE 20

  IF_NOGDH_RETURN;
	  
  /**********************************************************
  *	Return address to menu
  ***********************************************************/
  if ( event == RTT_APPL_MENU)
  {
    *picture = (char *) &dtt_systempicture_p39_eu;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Return address of background
  ***********************************************************/
  if ( event == RTT_APPL_PICTURE)
  {
    *picture = (char *) &dtt_systempicture_p39_bg;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Next and Previous page
  ***********************************************************/
  if ( event == RTT_APPL_PREVPAGE || event == RTT_APPL_NEXTPAGE)
  {
    if ( event == RTT_APPL_PREVPAGE)
    {
      page--;
      page = max( page, 0);
    }
    else
    {
      page++;
      page = min( page, RUNNINGTIME_MAXPAGE - 1);
    }
    RUNNINGTIME_PAGE = page + 1;

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    runningtimelist_ptr = runningtimelist;
    j = 0;
    for ( i = 0; i < runningtimelist_count; i++)
    {
      if ( (i >= page * RUNNINGTIME_PAGESIZE) && (i < (page + 1) * RUNNINGTIME_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, runningtimelist_ptr->runningtimename);

        /* Define PF buttons and Return */
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = 0;
        menu_ptr->argoi = runningtimelist_ptr->runningtime_objid;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TotalNOfStarts;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TotalRunHours;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TotalRunSeconds;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TripNOfStarts;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TripRunHours;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TripRunSeconds;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->Description;
        menu_ptr++;
        j++;
      }
      runningtimelist_ptr++;
    }
    for ( i = j; i < RUNNINGTIME_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
    }
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	Initialization of the picture
  ***********************************************************/
  if ( event == RTT_APPL_INIT)
  {
    page = 0;
    RUNNINGTIME_PAGE = page + 1;


    /* Get the rack objects */
    runningtime_alloc = 0;
    runningtimelist_count = 0;
    /* Get all RunningTime objects */
    sts = gdh_GetClassList ( pwr_cClass_RunningTime, &runningtime_objid);
    while (ODD(sts))
    {
      sts = rttsys_runningtimelist_add( runningtime_objid, &runningtimelist,
	&runningtimelist_count, &runningtime_alloc);
      if ( EVEN(sts)) return sts;
      sts = gdh_GetNextObject ( runningtime_objid, &runningtime_objid);
    }

    menulist = (rtt_t_menu_upd *) ctx->menu;
    menu_ptr = menulist;
    runningtimelist_ptr = runningtimelist;
    j = 0;
    for ( i = 0; i < runningtimelist_count; i++)
    {
      if ( (i >= page * RUNNINGTIME_PAGESIZE) && (i < (page + 1) * RUNNINGTIME_PAGESIZE))
      {
        strcpy( menu_ptr->value_ptr, runningtimelist_ptr->runningtimename);

        /* Define PF buttons and Return */
        menu_ptr->func2 = &rtt_object_parameters;
        menu_ptr->func3 = 0;
        menu_ptr->argoi = runningtimelist_ptr->runningtime_objid;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TotalNOfStarts;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TotalRunHours;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TotalRunSeconds;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TripNOfStarts;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TripRunHours;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->TripRunSeconds;
        menu_ptr++;
        menu_ptr->value_ptr = 
		(char *) &runningtimelist_ptr->runningtime_ptr->Description;
        menu_ptr++;
        j++;
      }
      runningtimelist_ptr++;
    }
    for ( i = j; i < RUNNINGTIME_PAGESIZE ; i++)
    {
      strcpy( menu_ptr->value_ptr, "");
      menu_ptr->func = 0;
      menu_ptr->func2 = 0;
      menu_ptr->func3 = 0;
      menu_ptr->argoi = pwr_cNObjid;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
      menu_ptr->value_ptr = (char *) RTT_ERASE;
      menu_ptr++;
    }
    RUNNINGTIME_MAXPAGE = max( 1, (runningtimelist_count - 1)/ RUNNINGTIME_PAGESIZE + 1);
  }
  /**********************************************************
  *	Exit of the picture
  ***********************************************************/
  else if ( event == RTT_APPL_EXIT)
  {
    runningtimelist_ptr = runningtimelist;
    for ( i = 0; i < runningtimelist_count; i++)
    {
      sts = gdh_UnrefObjectInfo( runningtimelist_ptr->runningtime_subid);
      runningtimelist_ptr++;
    }
    free( runningtimelist);
    runningtimelist_count = 0;
    return RTT__SUCCESS;
  }
  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  else if ( event == RTT_APPL_VALUECHANGED)
  {
  }
  /**********************************************************
  *	Update the picture.
  ***********************************************************/
  else if ( event == RTT_APPL_UPDATE)
  {
  }
  return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_QCOM_APPL()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show QCOM applications.
*
**************************************************************************/

int RTTSYS_QCOM_APPL( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
#define QCOM_APPL_PAGESIZE 17
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static int		page;	
  int			i, k, l;
  pool_sQlink		*al;
  qdb_sAppl		*ap;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;

      	qdb_ScopeLock {
        for (
	  al = pool_Qsucc(&sts, &qdb->pool, &qdb->g->appl_lh);
	  al != &qdb->g->appl_lh;
	  al = pool_Qsucc(&sts, &qdb->pool, al)
        ) 
        {
          if ( (k >= page * QCOM_APPL_PAGESIZE) && (k < (page + 1) * QCOM_APPL_PAGESIZE))
          {
	    ap = pool_Qitem(al, qdb_sAppl, appl_ll);

            /* Name */
            menu_ptr->value_ptr =  ap->name;
            menu_ptr->func = &rttsys_qcom_queue_start;
            menu_ptr->arg1 = (void *)ap->aid.aix;
            menu_ptr++;
            /* Aid */
            menu_ptr->value_ptr = (char *) &ap->aid.aix;
            menu_ptr++;
            /* Pid */
            menu_ptr->value_ptr = (char *) &ap->pid;
            menu_ptr++;
            /* Rid */
            menu_ptr->value_ptr = (char *) &ap->rid;
            menu_ptr++;
            /* Buf */
            *(int *)menu_ptr->value_ptr = ! pool_QisEmpty( &sts, &qdb->pool, &ap->out_lh);
            menu_ptr++;
            /* AllocCount */
            menu_ptr->value_ptr = (char *) &ap->alloc_count;
            menu_ptr++;
            /* Rid */
            menu_ptr->value_ptr = (char *) &ap->alloc_quota;
            menu_ptr++;
  	    l++;
          }
          k++;
        }
      } qdb_ScopeUnlock;

      for ( i = l; i < QCOM_APPL_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr =  (char *) RTT_ERASE;
        menu_ptr->func = 0;
        menu_ptr++;
        /* Aid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Pid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Rid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Buf */
        *(int *)menu_ptr->value_ptr = 0;
        menu_ptr++;
        /* AllocCount */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Rid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      QCOM_APPL_MAXPAGE = max( 1, (k - 1)/ QCOM_APPL_PAGESIZE + 1);

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p43_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p43_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_INIT:
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      if ( event == RTT_APPL_PREVPAGE)
      {
        page--;
        page = max( page, 0);
      }
      else if ( event == RTT_APPL_NEXTPAGE)
      {
        page++;
        page = min( page, QCOM_APPL_MAXPAGE - 1);
      }
      else if ( event == RTT_APPL_INIT)
        page = 0;

      QCOM_APPL_PAGE = page + 1;

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;

      	qdb_ScopeLock {
        for (
	  al = pool_Qsucc(&sts, &qdb->pool, &qdb->g->appl_lh);
	  al != &qdb->g->appl_lh;
	  al = pool_Qsucc(&sts, &qdb->pool, al)
        ) 
        {
          if ( (k >= page * QCOM_APPL_PAGESIZE) && (k < (page + 1) * QCOM_APPL_PAGESIZE))
          {
	    ap = pool_Qitem(al, qdb_sAppl, appl_ll);

            /* Name */
            menu_ptr->value_ptr =  ap->name;
            menu_ptr->func = &rttsys_qcom_queue_start;
            menu_ptr->arg1 = (void *)ap->aid.aix;
            menu_ptr++;
            /* Aid */
            menu_ptr->value_ptr = (char *) &ap->aid.aix;
            menu_ptr++;
            /* Pid */
            menu_ptr->value_ptr = (char *) &ap->pid;
            menu_ptr++;
            /* Rid */
            menu_ptr->value_ptr = (char *) &ap->rid;
            menu_ptr++;
            /* Buf */
            *(int *)menu_ptr->value_ptr = ! pool_QisEmpty( &sts, &qdb->pool, &ap->out_lh);
            menu_ptr++;
            /* AllocCount */
            menu_ptr->value_ptr = (char *) &ap->alloc_count;
            menu_ptr++;
            /* Rid */
            menu_ptr->value_ptr = (char *) &ap->alloc_quota;
            menu_ptr++;
  	    l++;
          }
          k++;
        }
      } qdb_ScopeUnlock;

      for ( i = l; i < QCOM_APPL_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr =  (char *) RTT_ERASE;
        menu_ptr->func = 0;
        menu_ptr++;
        /* Aid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Pid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Rid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Buf */
        *(int *)menu_ptr->value_ptr = 0;
        menu_ptr++;
        /* AllocCount */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Rid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      QCOM_APPL_MAXPAGE = max( 1, (k - 1)/ QCOM_APPL_PAGESIZE + 1);

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_QCOM_QUEUE()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show QCOM applications.
*
**************************************************************************/

static int rttsys_qcom_queue_start( 	menu_ctx	ctx, 
					pwr_tObjid	objid,
					void		*arg1,
					void		*arg2,
					void		*arg3,
					void		*arg4)
{
	int	sts;
	
	sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, (char *)arg1, "", 
		0, &RTTSYS_QCOM_QUEUE);
	return sts;
}

int RTTSYS_QCOM_QUEUE( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
#define QCOM_QUE_PAGESIZE 13
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static int		page;	
  int			i, k, l;
  pool_sQlink		*al, *ql;
  qdb_sAppl		*ap;
  qdb_sQue		*qp;
  int			found;
  static int		aix;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      found = 0;
      k = 0;
      l = 0;

      qdb_ScopeLock {
        for (
	  al = pool_Qsucc(&sts, &qdb->pool, &qdb->g->appl_lh);
	  al != &qdb->g->appl_lh;
	  al = pool_Qsucc(&sts, &qdb->pool, al)
        ) 
        {
	  ap = pool_Qitem(al, qdb_sAppl, appl_ll);
          if ( ap->aid.aix == aix)
          {
            found = 1;
            for (
   		  ql = pool_Qsucc(&sts, &qdb->pool, &ap->que_lh);
		  ql != &ap->que_lh;
		  ql = pool_Qsucc(&sts, &qdb->pool, ql)
            )
            {
              if ( (k >= page * QCOM_QUE_PAGESIZE) && (k < (page + 1) * QCOM_QUE_PAGESIZE))
              {
	        qp = pool_Qitem(ql, qdb_sQue, que_ll);

                /* Name */
                menu_ptr->value_ptr =  qp->name;
                menu_ptr++;
                /* Qix */
                *(int *)menu_ptr->value_ptr = qp->qix & ~ (1 << 31);
                menu_ptr++;
                /* Type */
                if ( qp->type == qdb_eQue_forward)
                  strcpy( menu_ptr->value_ptr, "Forward");
                else if ( qp->type == qdb_eQue_private)
                  strcpy( menu_ptr->value_ptr, "Private");
                else
                  strcpy( menu_ptr->value_ptr, "Unknown");
                menu_ptr++;
                /* Rid */
                menu_ptr->value_ptr = (char *) &qp->rid;
                menu_ptr++;
                /* Flags */
                strcpy( menu_ptr->value_ptr, "");
                if ( qp->flags.m & qdb_mQue_broadcast)
                  strcat( menu_ptr->value_ptr, "Br");
                if ( qp->flags.m & qdb_mQue_system)
                  strcat( menu_ptr->value_ptr, "Sy");
                if ( qp->flags.m & qdb_mQue_event)
                  strcat( menu_ptr->value_ptr, "Ev");
                if ( qp->flags.m & qdb_mQue_reply)
                  strcat( menu_ptr->value_ptr, "Re");
                menu_ptr++;

                /* Inbuff cnt */
                menu_ptr->value_ptr = (char *) &qp->in_lc;
                menu_ptr++;
                /* Inbuff quota */
                menu_ptr->value_ptr = (char *) &qp->in_quota;
                menu_ptr++;
  	        l++;
              }
              k++;
            }
            break;
          }
        }

        for ( i = l; i < QCOM_QUE_PAGESIZE; i++)
        {
          /* Name */
          menu_ptr->value_ptr =  (char *) RTT_ERASE;
          menu_ptr++;
          /* Qix */
          *(int *)menu_ptr->value_ptr = 0;
          menu_ptr++;
          /* Type */
          strcpy( menu_ptr->value_ptr, "");
          menu_ptr++;
          /* Rid */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          /* Flags */
          strcpy( menu_ptr->value_ptr, "");
          menu_ptr++;
          /* Inbuff Count */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
          /* Inbuff quota */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
        }

        if ( !found)
        {
          /* Application call_count */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;

          /* Application alloc_count */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;

          /* Application alloc_quota */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;

          /* Application free_count */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;

          /* Application get_count */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;

          /* Application put_count */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
         menu_ptr++;

          /* Application  request_count */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
         menu_ptr++;

          /* Application  reply_count */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;

          /* Application  name */
          menu_ptr->value_ptr = (char *) RTT_ERASE;
          menu_ptr++;
        }
      } qdb_ScopeUnlock;


      QCOM_QUE_MAXPAGE = max( 1, (k - 1)/ QCOM_QUE_PAGESIZE + 1);

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p44_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p44_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_INIT:
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      if ( event == RTT_APPL_PREVPAGE)
      {
        page--;
        page = max( page, 0);
      }
      else if ( event == RTT_APPL_NEXTPAGE)
      {
        page++;
        page = min( page, QCOM_QUE_MAXPAGE - 1);
      }
      else if ( event == RTT_APPL_INIT)
        page = 0;

      QCOM_QUE_PAGE = page + 1;

      aix = (int)objectname;

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;

      	qdb_ScopeLock {
        for (
	  al = pool_Qsucc(&sts, &qdb->pool, &qdb->g->appl_lh);
	  al != &qdb->g->appl_lh;
	  al = pool_Qsucc(&sts, &qdb->pool, al)
        ) 
        {
	  ap = pool_Qitem(al, qdb_sAppl, appl_ll);
          if ( ap->aid.aix == aix)
          {
            for (
   		  ql = pool_Qsucc(&sts, &qdb->pool, &ap->que_lh);
		  ql != &ap->que_lh;
		  ql = pool_Qsucc(&sts, &qdb->pool, ql)
            )
            {
              if ( (k >= page * QCOM_QUE_PAGESIZE) && (k < (page + 1) * QCOM_QUE_PAGESIZE))
              {
	        qp = pool_Qitem(ql, qdb_sQue, que_ll);

                /* Name */
                menu_ptr->value_ptr =  qp->name;
                menu_ptr++;
                /* Qix */
                *(int *)menu_ptr->value_ptr = qp->qix & ~ (1 << 31);
                menu_ptr++;
                /* Type */
                if ( qp->type == qdb_eQue_forward)
                  strcpy( menu_ptr->value_ptr, "Forward");
                else if ( qp->type == qdb_eQue_private)
                  strcpy( menu_ptr->value_ptr, "Private");
                else
                  strcpy( menu_ptr->value_ptr, "Unknown");
                menu_ptr++;
                /* Rid */
                menu_ptr->value_ptr = (char *) &qp->rid;
                menu_ptr++;
                /* Flags */
                strcpy( menu_ptr->value_ptr, "");
                if ( qp->flags.m & qdb_mQue_broadcast)
                  strcat( menu_ptr->value_ptr, "Br");
                if ( qp->flags.m & qdb_mQue_system)
                  strcat( menu_ptr->value_ptr, "Sy");
                if ( qp->flags.m & qdb_mQue_event)
                  strcat( menu_ptr->value_ptr, "Ev");
                if ( qp->flags.m & qdb_mQue_reply)
                  strcat( menu_ptr->value_ptr, "Re");
                menu_ptr++;

                /* Inbuff cnt */
                menu_ptr->value_ptr = (char *) &qp->in_lc;
                menu_ptr++;
                /* Inbuff quota */
                menu_ptr->value_ptr = (char *) &qp->in_quota;
                menu_ptr++;
  	        l++;
              }
              k++;
            }

            break;
          }
        }
      } qdb_ScopeUnlock;

      for ( i = l; i < QCOM_QUE_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr =  (char *) RTT_ERASE;
        menu_ptr++;
        /* Qix */
        *(int *)menu_ptr->value_ptr = 0;
        menu_ptr++;
        /* Type */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Rid */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Flags */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Inbuff Count */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Inbuff quota */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }

      /* Application call_count */
      menu_ptr->value_ptr = (char *) &ap->call_count;
      menu_ptr++;

      /* Application alloc_count */
      menu_ptr->value_ptr = (char *) &ap->alloc_count;
      menu_ptr++;

      /* Application alloc_quota */
      menu_ptr->value_ptr = (char *) &ap->alloc_quota;
      menu_ptr++;

      /* Application free_count */
      menu_ptr->value_ptr = (char *) &ap->free_count;
      menu_ptr++;

      /* Application get_count */
      menu_ptr->value_ptr = (char *) &ap->get_count;
      menu_ptr++;

      /* Application put_count */
      menu_ptr->value_ptr = (char *) &ap->put_count;
      menu_ptr++;

      /* Application  request_count */
      menu_ptr->value_ptr = (char *) &ap->request_count;
      menu_ptr++;

      /* Application  reply_count */
      menu_ptr->value_ptr = (char *) &ap->reply_count;
      menu_ptr++;

      /* Application  name */
      menu_ptr->value_ptr = ap->name;
      menu_ptr++;


      QCOM_QUE_MAXPAGE = max( 1, (k - 1)/ QCOM_QUE_PAGESIZE + 1);

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_SHOW_NODES()
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show nethandler info.
*
**************************************************************************/

int RTTSYS_QCOM_NODES( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
#define QCOM_NODES_PAGESIZE 18
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static int		page;	
  int			i, k, l;
  pool_sQlink		*nl;
  qdb_sNode		*np;
  char 			timbuf[32];

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;

      gdb_ScopeLock {
        for (
	  nl = pool_Qsucc(&sts, &qdb->pool, &qdb->g->node_lh);
	  nl != &qdb->g->node_lh;
	  nl = pool_Qsucc(&sts, &qdb->pool, nl)
        ) 
        {
          if ( (k >= page * QCOM_NODES_PAGESIZE) && (k < (page + 1) * QCOM_NODES_PAGESIZE))
          {
	    np = pool_Qitem(nl, qdb_sNode, node_ll);

	    if ( strncmp( np->name, "***", 3) == 0)
	      continue;

            /* Name */
            menu_ptr->value_ptr = (char *) &np->name;
            menu_ptr->func = &rttsys_qcom_node_start;
            menu_ptr->arg1 = (void *) np->nid;
            menu_ptr++;

            /* Os */
	    switch ( np->os) {
	      case co_eOS_Lynx: strcpy( menu_ptr->value_ptr, "Lynx"); break;
	      case co_eOS_Linux: strcpy( menu_ptr->value_ptr, "Linux"); break;
	      case co_eOS_VMS: strcpy( menu_ptr->value_ptr, "VMS"); break;
	      case co_eOS_ELN: strcpy( menu_ptr->value_ptr, "ELN"); break;
	      default: strcpy( menu_ptr->value_ptr, "Unknwn");
	    }
            menu_ptr++;
            /* Hw */
	    switch ( np->hw) {
	      case co_eHW_x86: strcpy( menu_ptr->value_ptr, "x86"); break;
	      case co_eHW_68k: strcpy( menu_ptr->value_ptr, "68k"); break;
	      case co_eHW_VAX: strcpy( menu_ptr->value_ptr, "VAX"); break;
	      case co_eHW_Alpha: strcpy( menu_ptr->value_ptr, "AXP"); break;
	      case co_eHW_PPC: strcpy( menu_ptr->value_ptr, "PPC"); break;
	      default: strcpy( menu_ptr->value_ptr, "Unknwn");
	    }
            menu_ptr++;
            /* Link */
            if (np == qdb->my_node)
	      /* Local node */
              strcpy( menu_ptr->value_ptr, "Local");
	    else
	    {
              if ( np->flags.b.initiated)
                strcpy( menu_ptr->value_ptr, "Initiated");
              else if ( np->flags.b.active)
                strcpy( menu_ptr->value_ptr, "Active");
              else if ( np->flags.b.connected)
                strcpy( menu_ptr->value_ptr, "Connected");
              else
                strcpy( menu_ptr->value_ptr, "Down");
	    }
            menu_ptr++;
            /* Upcnt */
            menu_ptr->value_ptr = (char *) &np->upcnt;
            menu_ptr++;
  	    /* Timeup */
	    if ( np->timeup.tv_sec != 0 || np->timeup.tv_nsec != 0)
	    {
              time_AtoAscii( &np->timeup, time_eFormat_DateAndTime, timbuf,
		sizeof(timbuf));
	    }
	    else
              timbuf [0] = '\0';
            strcpy( menu_ptr->value_ptr, timbuf);
            menu_ptr++;
            /* Sent */
            menu_ptr->value_ptr = (char *) &np->put.segs;
            menu_ptr++;
            /* Rcvd */
            menu_ptr->value_ptr = (char *) &np->get.segs;
            menu_ptr++;
  	    l++;
          }
          k++;
        }
      } gdb_ScopeUnlock;

      for ( i = l; i < QCOM_NODES_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Os */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Hw */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Link */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Upcnt */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Timeup */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Sent */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Rcvd */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      QCOM_NODES_MAXPAGE = max( 1, (k - 1)/ QCOM_NODES_PAGESIZE + 1);

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p45_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p45_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_INIT:
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      if ( event == RTT_APPL_PREVPAGE)
      {
        page--;
        page = max( page, 0);
      }
      else if ( event == RTT_APPL_NEXTPAGE)
      {
        page++;
        page = min( page, QCOM_NODES_MAXPAGE - 1);
      }
      else if ( event == RTT_APPL_INIT)
        page = 0;

      QCOM_NODES_PAGE = page + 1;

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      k = 0;
      l = 0;

      gdb_ScopeLock {
        for (
	  nl = pool_Qsucc(&sts, &qdb->pool, &qdb->g->node_lh);
	  nl != &qdb->g->node_lh;
	  nl = pool_Qsucc(&sts, &qdb->pool, nl)
        ) 
        {
          if ( (k >= page * QCOM_NODES_PAGESIZE) && (k < (page + 1) * QCOM_NODES_PAGESIZE))
          {
	    np = pool_Qitem(nl, qdb_sNode, node_ll);

	    if ( strncmp( np->name, "***", 3) == 0)
	      continue;

            /* Name */
            menu_ptr->value_ptr = (char *) &np->name;
            menu_ptr->func = &rttsys_qcom_node_start;
            menu_ptr->arg1 = (void *) np->nid;
            menu_ptr++;

            /* Os */
	    switch ( np->os) {
	      case co_eOS_Lynx: strcpy( menu_ptr->value_ptr, "Lynx"); break;
	      case co_eOS_Linux: strcpy( menu_ptr->value_ptr, "Linux"); break;
	      case co_eOS_VMS: strcpy( menu_ptr->value_ptr, "VMS"); break;
	      case co_eOS_ELN: strcpy( menu_ptr->value_ptr, "ELN"); break;
	      default: strcpy( menu_ptr->value_ptr, "Unknwn");
	    }
            menu_ptr++;
            /* Hw */
	    switch ( np->hw) {
	      case co_eHW_x86: strcpy( menu_ptr->value_ptr, "x86"); break;
	      case co_eHW_68k: strcpy( menu_ptr->value_ptr, "68k"); break;
	      case co_eHW_VAX: strcpy( menu_ptr->value_ptr, "VAX"); break;
	      case co_eHW_Alpha: strcpy( menu_ptr->value_ptr, "AXP"); break;
	      case co_eHW_PPC: strcpy( menu_ptr->value_ptr, "PPC"); break;
	      default: strcpy( menu_ptr->value_ptr, "Unknwn");
	    }
            menu_ptr++;
            /* Link */
            if (np == qdb->my_node)
	      /* Local node */
              strcpy( menu_ptr->value_ptr, "Local");
	    else
	    {
              if ( np->flags.b.initiated)
                strcpy( menu_ptr->value_ptr, "Initiated");
              else if ( np->flags.b.active)
                strcpy( menu_ptr->value_ptr, "Active");
              else if ( np->flags.b.connected)
                strcpy( menu_ptr->value_ptr, "Connected");
              else
                strcpy( menu_ptr->value_ptr, "Down");
	    }
            menu_ptr++;
            /* Upcnt */
            menu_ptr->value_ptr = (char *) &np->upcnt;
            menu_ptr++;
  	    /* Timeup */
	    if ( np->timeup.tv_sec != 0 || np->timeup.tv_nsec != 0)
	    {
              time_AtoAscii( &np->timeup, time_eFormat_DateAndTime, timbuf,
		sizeof(timbuf));
	    }
	    else
              timbuf [0] = '\0';
            strcpy( menu_ptr->value_ptr, timbuf);
            menu_ptr++;
            /* Sent */
            menu_ptr->value_ptr = (char *) &np->put.segs;
            menu_ptr++;
            /* Rcvd */
            menu_ptr->value_ptr = (char *) &np->get.segs;
            menu_ptr++;
  	    l++;
          }
          k++;
        }
      } gdb_ScopeUnlock;

      for ( i = l; i < QCOM_NODES_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Os */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Hw */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Link */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Upcnt */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Timeup */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
        /* Sent */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Rcvd */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
      }
      QCOM_NODES_MAXPAGE = max( 1, (k - 1)/ QCOM_NODES_PAGESIZE + 1);

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		RTTSYS_POOLS()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show the qdb, gdb and rtdb pools.
*
**************************************************************************/

int RTTSYS_POOLS( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  int			i, j;
  pool_sGhead		*pool;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:
      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      for ( j = 0; j < 3; j++)
      {
        if ( j == 0)
          /* qdb pool */
          pool = qdb->pool.gphp;
        else if ( j == 1)
          /* gdb pool */
          pool = gdbroot->pool->gphp;
        else
          /* rtdb pool */
          pool = gdbroot->rtdb->gphp;

        /* Name */
        menu_ptr++;

        /* Initial size */
        *(int *)menu_ptr->value_ptr = pool->initsize * POOL_SSIZE;
        menu_ptr++;

        /* Extend size */
        *(int *)menu_ptr->value_ptr = pool->extendsize * POOL_SSIZE;
        menu_ptr++;

        /* Total size */
        *(int *)menu_ptr->value_ptr = 0;
        for ( i = 0; i < pool_cSegs; i++)
        {
          if ( pool->seg[i].generation == 0)
           break;
          *(int *)menu_ptr->value_ptr += pool->seg[i].fragsize * POOL_SSIZE;
        }  
        menu_ptr++;

        /* Generation */
        menu_ptr++;

        /* Lookaside index */
        menu_ptr++;
      }
      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p46_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p46_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      break;

    case RTT_APPL_INIT:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      for ( j = 0; j < 3; j++)
      {
        if ( j == 0)
          /* qdb pool */
          pool = qdb->pool.gphp;
        else if ( j == 1)
          /* gdb pool */
          pool = gdbroot->pool->gphp;
        else
          /* rtdb pool */
          pool = gdbroot->rtdb->gphp;

        /* Name */
        menu_ptr->value_ptr = pool->name;
        menu_ptr->func = &rttsys_pool_segs_start;
        menu_ptr->arg1 = (void *)pool;
        menu_ptr++;

        /* Initial size */
        *(int *)menu_ptr->value_ptr = pool->initsize * POOL_SSIZE;
        menu_ptr++;

        /* Extend size */
        *(int *)menu_ptr->value_ptr = pool->extendsize * POOL_SSIZE;
        menu_ptr++;

        /* Total size */
        *(int *)menu_ptr->value_ptr = 0;
        for ( i = 0; i < pool_cSegs; i++)
        {
          if ( pool->seg[i].generation == 0)
           break;
          *(int *)menu_ptr->value_ptr += pool->seg[i].fragsize * POOL_SSIZE;
        }  
        menu_ptr++;

        /* Generation */
        menu_ptr->value_ptr = (char *) &pool->generation;
        menu_ptr++;

        /* Lookaside index */
        menu_ptr->value_ptr = (char *) &pool->la_idx;
        menu_ptr++;
      }
      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_POOL_SEGS()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show pool segments.
*
**************************************************************************/

static int rttsys_pool_segs_start( 	menu_ctx	ctx, 
					pwr_tObjid	objid,
					void		*arg1,
					void		*arg2,
					void		*arg3,
					void		*arg4)
{
	int	sts;
	
	sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, (char *)arg1, "", 
		0, &RTTSYS_POOL_SEGS);
	return sts;
}

int RTTSYS_POOL_SEGS( 	menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
#define POOLSEGS_PAGESIZE 17
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static int		page;	
  int			i, k, l;
  static pool_sGhead	*pool;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      l = 0;

      for ( k = 0; k < pool_cSegs; k++)
      {
        if ( pool->seg[k].generation == 0)
          break;

        if ( (k >= page * POOLSEGS_PAGESIZE) && (k < (page + 1) * POOLSEGS_PAGESIZE))
        {
          /* Name */
          menu_ptr->value_ptr =  pool->seg[k].name;
          menu_ptr->func = &rttsys_pool_segment_start;
          menu_ptr->arg1 = (void *)&pool->seg[k];
          menu_ptr++;

          /* Size */
          *(int *)menu_ptr->value_ptr = pool->seg[k].size * POOL_SSIZE;
          menu_ptr++;

          /* Alloccnt */
          menu_ptr->value_ptr =  (char *) &pool->seg[k].alloccnt;
          menu_ptr++;

          /* Fragcnt */
          menu_ptr->value_ptr =  (char *) &pool->seg[k].fragcnt;
          menu_ptr++;

          /* Generation */
          menu_ptr->value_ptr =  (char *) &pool->seg[k].generation;
          menu_ptr++;

          /* Segment */
          menu_ptr->value_ptr =  (char *) &pool->seg[k].seg;
          menu_ptr++;

          /* Type */
          if ( pool->seg[k].type == pool_eSegType_dynamic)
            strcpy( menu_ptr->value_ptr, "Dynamic");
          else if ( pool->seg[k].type == pool_eSegType_lookaside)
            strcpy( menu_ptr->value_ptr, "Lookaside");
          else if ( pool->seg[k].type == pool_eSegType_named)
            strcpy( menu_ptr->value_ptr, "Named");
          else
            strcpy( menu_ptr->value_ptr, "Unknown");
          menu_ptr++;

          l++;
        }
      }

      for ( i = l; i < POOLSEGS_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr =  (char *) RTT_ERASE;
        menu_ptr->func = 0;
        menu_ptr++;
        /* Size */
        *(int *)menu_ptr->value_ptr = 0;
        menu_ptr++;
        /* Alloccnt */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Fragcnt */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Generation */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Segment */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Type */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
      }
      POOLSEGS_MAXPAGE = max( 1, (k - 1)/ POOLSEGS_PAGESIZE + 1);

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p47_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p47_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_INIT:
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      if ( event == RTT_APPL_PREVPAGE)
      {
        page--;
        page = max( page, 0);
      }
      else if ( event == RTT_APPL_NEXTPAGE)
      {
        page++;
        page = min( page, POOLSEGS_MAXPAGE - 1);
      }
      else if ( event == RTT_APPL_INIT)
        page = 0;

      POOLSEGS_PAGE = page + 1;

      pool = (pool_sGhead *)objectname;
      strcpy( POOLSEGS_NAME, pool->name);

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      l = 0;

      for ( k = 0; k < pool_cSegs; k++)
      {
        if ( pool->seg[k].generation == 0)
          break;

        if ( (k >= page * POOLSEGS_PAGESIZE) && (k < (page + 1) * POOLSEGS_PAGESIZE))
        {
          /* Name */
          menu_ptr->value_ptr =  pool->seg[k].name;
          menu_ptr->func = &rttsys_pool_segment_start;
          menu_ptr->arg1 = (void *)&pool->seg[k];
          menu_ptr++;

          /* Size */
          *(int *)menu_ptr->value_ptr = pool->seg[k].size * POOL_SSIZE;
          menu_ptr++;

          /* Alloccnt */
          menu_ptr->value_ptr =  (char *) &pool->seg[k].alloccnt;
          menu_ptr++;

          /* Fragcnt */
          menu_ptr->value_ptr =  (char *) &pool->seg[k].fragcnt;
          menu_ptr++;

          /* Generation */
          menu_ptr->value_ptr =  (char *) &pool->seg[k].generation;
          menu_ptr++;

          /* Segment */
          menu_ptr->value_ptr =  (char *) &pool->seg[k].seg;
          menu_ptr++;

          /* Type */
          if ( pool->seg[k].type == pool_eSegType_dynamic)
            strcpy( menu_ptr->value_ptr, "Dynamic");
          else if ( pool->seg[k].type == pool_eSegType_lookaside)
            strcpy( menu_ptr->value_ptr, "Lookaside");
          else if ( pool->seg[k].type == pool_eSegType_named)
            strcpy( menu_ptr->value_ptr, "Named");
          else
            strcpy( menu_ptr->value_ptr, "Unknown");
          menu_ptr++;

          l++;
        }
      }

      for ( i = l; i < POOLSEGS_PAGESIZE; i++)
      {
        /* Name */
        menu_ptr->value_ptr =  (char *) RTT_ERASE;
        menu_ptr->func = 0;
        menu_ptr++;
        /* Size */
        *(int *)menu_ptr->value_ptr = 0;
        menu_ptr++;
        /* Alloccnt */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Fragcnt */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Generation */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Segment */
        menu_ptr->value_ptr = (char *) RTT_ERASE;
        menu_ptr++;
        /* Type */
        strcpy( menu_ptr->value_ptr, "");
        menu_ptr++;
      }
      POOLSEGS_MAXPAGE = max( 1, (k - 1)/ POOLSEGS_PAGESIZE + 1);

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_POOL_SEGS()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show pool segments.
*
**************************************************************************/

static int rttsys_pool_segment_start( 	menu_ctx	ctx, 
					pwr_tObjid	objid,
					void		*arg1,
					void		*arg2,
					void		*arg3,
					void		*arg4)
{
	int	sts;
	
	sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, (char *)arg1, "", 
		0, &RTTSYS_POOL_SEGMENT);
	return sts;
}

int RTTSYS_POOL_SEGMENT( menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  static pool_sGsegment	*seg;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      menu_ptr++;
      *(int *)menu_ptr->value_ptr = seg->size * POOL_SSIZE;
      menu_ptr++;
      menu_ptr++;
      menu_ptr++;
      *(int *)menu_ptr->value_ptr = seg->fragmax * POOL_SSIZE;
      menu_ptr++;
      menu_ptr++;
      *(int *)menu_ptr->value_ptr = seg->fragsize * POOL_SSIZE;
      menu_ptr++;
      menu_ptr++;
      menu_ptr++;

      /* Type */
      if ( seg->type == pool_eSegType_dynamic)
        strcpy( menu_ptr->value_ptr, "Dynamic");
      else if ( seg->type == pool_eSegType_lookaside)
        strcpy( menu_ptr->value_ptr, "Lookaside");
      else if ( seg->type == pool_eSegType_named)
        strcpy( menu_ptr->value_ptr, "Named");
      else
        strcpy( menu_ptr->value_ptr, "Unknown");
      menu_ptr++;

      *(int *)menu_ptr->value_ptr = seg->la_size * POOL_SSIZE;
      menu_ptr++;

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p48_bg;
      return RTT__SUCCESS;

  /**********************************************************
  *	Previous page
  ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p48_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_NEXTPAGE:
    case RTT_APPL_PREVPAGE:
      break;
    case RTT_APPL_INIT:

      seg = (pool_sGsegment *)objectname;

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      /* Name */
      menu_ptr->value_ptr = seg->name;
      menu_ptr++;

      /* Size */
      *(int *)menu_ptr->value_ptr = seg->size * POOL_SSIZE;
      menu_ptr++;

      /* Alloccnt */
      menu_ptr->value_ptr =  (char *) &seg->alloccnt;
      menu_ptr++;

      /* Fragcnt */
      menu_ptr->value_ptr =  (char *) &seg->fragcnt;
      menu_ptr++;

      /* Fragmax */
      *(int *)menu_ptr->value_ptr = seg->fragmax * POOL_SSIZE;
      menu_ptr++;

      /* Fragmaxcnt */
      menu_ptr->value_ptr =  (char *) &seg->fragmaxcnt;
      menu_ptr++;

      /* Fragsize */
      *(int *)menu_ptr->value_ptr = seg->fragsize * POOL_SSIZE;
      menu_ptr++;

      /* Generation */
      menu_ptr->value_ptr =  (char *) &seg->generation;
      menu_ptr++;

      /* Segment */
      menu_ptr->value_ptr =  (char *) &seg->seg;
      menu_ptr++;

      /* Type */
      if ( seg->type == pool_eSegType_dynamic)
        strcpy( menu_ptr->value_ptr, "Dynamic");
      else if ( seg->type == pool_eSegType_lookaside)
        strcpy( menu_ptr->value_ptr, "Lookaside");
      else if ( seg->type == pool_eSegType_named)
        strcpy( menu_ptr->value_ptr, "Named");
      else
        strcpy( menu_ptr->value_ptr, "Unknown");
      menu_ptr++;

      /* Lookaside size */
      *(int *)menu_ptr->value_ptr = seg->la_size * POOL_SSIZE;
      menu_ptr++;

      /* Lookaside idx */
      menu_ptr->value_ptr =  (char *) &seg->la_idx;
      menu_ptr++;

      /* Lookaside next idx */
      menu_ptr->value_ptr =  (char *) &seg->next_la_seg;
      menu_ptr++;

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		RTTSYS_QCOM_NODE()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	context of the picture.
* int		event		I 	type of event.
* char		*parameter_ptr	I	pointer to the parameter which value
*					has been changed.
*
* Description:
*	Show pool segments.
*
**************************************************************************/

static int rttsys_qcom_node_start( 	menu_ctx	ctx, 
					pwr_tObjid	objid,
					void		*arg1,
					void		*arg2,
					void		*arg3,
					void		*arg4)
{
	int	sts;
	
	sts = rtt_menu_new_sysedit( ctx, pwr_cNObjid, (char *)arg1, "", 
		0, &RTTSYS_QCOM_NODE);
	return sts;
}

int RTTSYS_QCOM_NODE( menu_ctx	ctx,
			int		event,
			char		*parameter_ptr,
			char		*objectname,
			char		**picture)
{ 
  int			sts;
  rtt_t_menu_upd	*menu_ptr;
  rtt_t_menu_upd	*menulist;
  pool_sQlink		*nl;
  qdb_sNode		*np;
  static pwr_tNodeId	nid;

  IF_NOGDH_RETURN;

  /**********************************************************
  *	The value of a parameter is changed.
  ***********************************************************/
  switch ( event)
  {
    case RTT_APPL_UPDATE:

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      gdb_ScopeLock {
        for (
	  nl = pool_Qsucc(&sts, &qdb->pool, &qdb->g->node_lh);
	  nl != &qdb->g->node_lh;
	  nl = pool_Qsucc(&sts, &qdb->pool, nl)
        ) 
        {
          np = pool_Qitem(nl, qdb_sNode, node_ll);

          if ( np->nid == nid)
          {
            /* Flags */
            if (np == qdb->my_node)
	      /* Local node */
              strcpy( menu_ptr->value_ptr, "Local");
	    else
            {
              if ( np->link.flags.m & qdb_mLink_active)
                strcpy( menu_ptr->value_ptr, "Active");
              else if ( np->link.flags.m & qdb_mLink_connected)
                strcpy( menu_ptr->value_ptr, "Connected");
              else
                strcpy( menu_ptr->value_ptr, "Down");
            }
            menu_ptr++;

            /* win count */
            menu_ptr->value_ptr = (char *) &np->link.win_count;
            menu_ptr++;

            /* win max */
            menu_ptr->value_ptr = (char *) &np->link.win_max;
            menu_ptr++;

            /* rtt rxmax */
            menu_ptr->value_ptr = (char *) &np->link.rtt_rxmax;
            menu_ptr++;

            /* rtt rxmin */
            menu_ptr->value_ptr = (char *) &np->link.rtt_rxmin;
            menu_ptr++;

            /* rtt rtt */
            menu_ptr->value_ptr = (char *) &np->link.rtt_rtt;
            menu_ptr++;

            /* rtt srtt */
            menu_ptr->value_ptr = (char *) &np->link.rtt_srtt;
            menu_ptr++;

            /* rtt var */
            menu_ptr->value_ptr = (char *) &np->link.rtt_var;
            menu_ptr++;

            /* rtt rto */
            menu_ptr->value_ptr = (char *) &np->link.rtt_rto;
            menu_ptr++;

            /* lack seq */
            menu_ptr->value_ptr = (char *) &np->link.lack.seq;
            menu_ptr++;

            /* lack ts */
            menu_ptr->value_ptr = (char *) &np->link.lack.ts;
            menu_ptr++;

            /* rack seq */
            menu_ptr->value_ptr = (char *) &np->link.rack.seq;
            menu_ptr++;

            /* rack ts */
            menu_ptr->value_ptr = (char *) &np->link.rack.ts;
            menu_ptr++;

            /* seq */
            menu_ptr->value_ptr = (char *) &np->link.seq;
            menu_ptr++;

            /* pending rack */
            menu_ptr->value_ptr = (char *) &np->link.pending_rack;
            menu_ptr++;

            /* rack_tmo */
            menu_ptr->value_ptr = (char *) &np->link.rack_tmo;
            menu_ptr++;

            /* bus */
            menu_ptr->value_ptr = (char *) &np->link.bus;
            menu_ptr++;

            /* Port */
            if ( pwr_dHost_byteOrder == pwr_dLittleEndian)
            {
              unsigned short tmp;
              tmp = np->link.sa.sin_port;
              ENDIAN_SWAP_SHORTP(&tmp);
              *(int *)menu_ptr->value_ptr = tmp;
            }
            else
              *(int *)menu_ptr->value_ptr = np->link.sa.sin_port;
            menu_ptr++;

            /* Address */
#if defined OS_ELN || defined OS_VMS
            sprintf( menu_ptr->value_ptr, "%d.%d.%d.%d", 
		np->link.sa.sin_addr.S_un.S_un_b.s_b1,
		np->link.sa.sin_addr.S_un.S_un_b.s_b2,
		np->link.sa.sin_addr.S_un.S_un_b.s_b3,
		np->link.sa.sin_addr.S_un.S_un_b.s_b4);
#else
            sprintf( menu_ptr->value_ptr, "%d.%d.%d.%d", 0,0,0,0);
#endif
            menu_ptr++;

            /* timer */
            menu_ptr->value_ptr = (char *) &np->link.timer;
            menu_ptr++;

            /* err_red */
            menu_ptr->value_ptr = (char *) &np->link.err_red;
            menu_ptr++;

            /* err_seq */
            menu_ptr->value_ptr = (char *) &np->link.err_seq;
            menu_ptr++;

            /* node name */
            menu_ptr->value_ptr = (char *) np->name;
            menu_ptr++;

            break;
          }
        }
      } gdb_ScopeUnlock;

      return RTT__SUCCESS;

    /**********************************************************
    *	Return address of menu
    ***********************************************************/
    case RTT_APPL_PICTURE:
      *picture = (char *) &dtt_systempicture_p49_bg;
      return RTT__SUCCESS;

    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_MENU:
      *picture = (char *) &dtt_systempicture_p49_eu;
      return RTT__SUCCESS;


    /**********************************************************
    *	Initialization of the picture
    ***********************************************************/
    /**********************************************************
    *	Next page
    ***********************************************************/
    /**********************************************************
    *	Previous page
    ***********************************************************/
    case RTT_APPL_PREVPAGE:
    case RTT_APPL_NEXTPAGE:
      break;
    case RTT_APPL_INIT:

      nid = (pwr_tNodeId) objectname;

      menulist = (rtt_t_menu_upd *) ctx->menu;
      menu_ptr = menulist;

      gdb_ScopeLock {
        for (
	  nl = pool_Qsucc(&sts, &qdb->pool, &qdb->g->node_lh);
	  nl != &qdb->g->node_lh;
	  nl = pool_Qsucc(&sts, &qdb->pool, nl)
        ) 
        {
          np = pool_Qitem(nl, qdb_sNode, node_ll);

          if ( np->nid == nid)
          {
            /* Flags */
            if (np == qdb->my_node)
	      /* Local node */
              strcpy( menu_ptr->value_ptr, "Local");
	    else
            {
              if ( np->link.flags.m & qdb_mLink_active)
                strcpy( menu_ptr->value_ptr, "Active");
              else if ( np->link.flags.m & qdb_mLink_connected)
                strcpy( menu_ptr->value_ptr, "Connected");
              else
                strcpy( menu_ptr->value_ptr, "Down");
            }
            menu_ptr++;

            /* win count */
            menu_ptr->value_ptr = (char *) &np->link.win_count;
            menu_ptr++;

            /* win max */
            menu_ptr->value_ptr = (char *) &np->link.win_max;
            menu_ptr++;

            /* rtt rxmax */
            menu_ptr->value_ptr = (char *) &np->link.rtt_rxmax;
            menu_ptr++;

            /* rtt rxmin */
            menu_ptr->value_ptr = (char *) &np->link.rtt_rxmin;
            menu_ptr++;

            /* rtt rtt */
            menu_ptr->value_ptr = (char *) &np->link.rtt_rtt;
            menu_ptr++;

            /* rtt srtt */
            menu_ptr->value_ptr = (char *) &np->link.rtt_srtt;
            menu_ptr++;

            /* rtt var */
            menu_ptr->value_ptr = (char *) &np->link.rtt_var;
            menu_ptr++;

            /* rtt rto */
            menu_ptr->value_ptr = (char *) &np->link.rtt_rto;
            menu_ptr++;

            /* lack seq */
            menu_ptr->value_ptr = (char *) &np->link.lack.seq;
            menu_ptr++;

            /* lack ts */
            menu_ptr->value_ptr = (char *) &np->link.lack.ts;
            menu_ptr++;

            /* rack seq */
            menu_ptr->value_ptr = (char *) &np->link.rack.seq;
            menu_ptr++;

            /* rack ts */
            menu_ptr->value_ptr = (char *) &np->link.rack.ts;
            menu_ptr++;

            /* seq */
            menu_ptr->value_ptr = (char *) &np->link.seq;
            menu_ptr++;

            /* pending rack */
            menu_ptr->value_ptr = (char *) &np->link.pending_rack;
            menu_ptr++;

            /* rack_tmo */
            menu_ptr->value_ptr = (char *) &np->link.rack_tmo;
            menu_ptr++;

            /* bus */
            menu_ptr->value_ptr = (char *) &np->link.bus;
            menu_ptr++;

            /* Port */
            if ( pwr_dHost_byteOrder == pwr_dLittleEndian)
            {
              unsigned short tmp;
              tmp = np->link.sa.sin_port;
              ENDIAN_SWAP_SHORTP(&tmp);
              *(int *)menu_ptr->value_ptr = tmp;
            }
            else
              *(int *)menu_ptr->value_ptr = np->link.sa.sin_port;
            menu_ptr++;

            /* Address */
#if defined OS_ELN || defined OS_VMS
            sprintf( menu_ptr->value_ptr, "%d.%d.%d.%d", 
		np->link.sa.sin_addr.S_un.S_un_b.s_b1,
		np->link.sa.sin_addr.S_un.S_un_b.s_b2,
		np->link.sa.sin_addr.S_un.S_un_b.s_b3,
		np->link.sa.sin_addr.S_un.S_un_b.s_b4);
#else
            sprintf( menu_ptr->value_ptr, "%d.%d.%d.%d", 0,0,0,0);
#endif
            menu_ptr++;

            /* timer */
            menu_ptr->value_ptr = (char *) &np->link.timer;
            menu_ptr++;

            /* err_red */
            menu_ptr->value_ptr = (char *) &np->link.err_red;
            menu_ptr++;

            /* err_seq */
            menu_ptr->value_ptr = (char *) &np->link.err_seq;
            menu_ptr++;

            /* node name */
            menu_ptr->value_ptr = (char *) np->name;
            menu_ptr++;

            break;
          }
        }
      } gdb_ScopeUnlock;

      break;

    /**********************************************************
    *	Exit of the picture
    ***********************************************************/
    case RTT_APPL_EXIT:
      break;
  }

  return RTT__SUCCESS;
}



int	rttsys_get_login_picture( rtt_t_backgr **picture)
{
  *picture = (rtt_t_backgr *) &dtt_systempicture_p38_bg;
  return RTT__SUCCESS;  
}
