/* 
 * Proview   $Id: wb_utl.cpp,v 1.1 2007-01-04 07:29:04 claes Exp $
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

/* wb_utl.c
   Utilitys for getting information about the plcprogram structure.  */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#if defined OS_VMS
#include <descrip.h>
#include <processes.h>
#include <starlet.h>
#include <lib$routines.h>
#include <libdef.h>
#include <libdtdef.h>
#include <clidef.h>
#include <ssdef.h>
#include <lbrdef.h>
#include <lbr$routines.h>
#include <mhddef.h>
#include <credef.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_version.h"
#include "pwr_baseclasses.h"
#include "flow_ctx.h"
#include "co_cdh.h"
#include "co_time.h"
#include "co_dcli.h"
#include "wb_ldh.h"
#include "wb_ldh_msg.h"
#include "wb_foe_msg.h"
#include "wb_vldh_msg.h"
#include "wb_trv.h"
#include "wb_vldh.h"
#include "wb_gcg.h"
#include "wb_goen.h"
#include "wb_gre.h"
#include "wb_foe.h"
#include "wb_lfu.h"
#include "wb_utl.h"
#include "wb_utl_api.h"
#include "wb_dir.h"

#define	UTL_LIST_MAX		5
#define	UTL_INPUTLIST_MAX	50
#define	UTL_PARDESCRIPTION	15
#define	UTL_NOELEMENT		-1

#define UTL_PORTRAIT		0
#define UTL_LANDSCAPE		1

#define	UTL_PAGE_BREAK_PORTR	55
#define	UTL_PAGE_BREAK_LANDS	41

#define	UTL_FULLPRINT_COND	2
#define	UTL_FULLPRINT_NORTBODY	4
#define	UTL_FULLPRINT_NODEVBODY 8
#define	UTL_FULLPRINT_NOSYSBODY 32
#define	UTL_FULLPRINT_SIGNAL	64

/* Library for the plcmodules */
/* Libraries for the plcmodules */
#define PLCLIB_VAX_ELN "pwrp_root:[vax_eln.lib]plc"
#define PLCLIB_VAX_VMS "pwrp_root:[vax_vms.lib]plc"
#define PLCLIB_AXP_VMS "pwrp_root:[axp_vms.lib]plc"

/* Frozen libraries for the plcmodules */
#define PLCLIB_FROZEN_VAX_ELN "pwrp_root:[vax_eln.lib]plcf"
#define PLCLIB_FROZEN_VAX_VMS "pwrp_root:[vax_vms.lib]plcf"
#define PLCLIB_FROZEN_AXP_VMS "pwrp_root:[axp_vms.lib]plcf"

typedef struct {
  pwr_tString40                       Parameter;
  pwr_tString40                       ColumnHeader;
  pwr_tString40                       MarginString;
  pwr_tBoolean                        PrintParName;
  pwr_tBoolean                        CarriageRet;
  pwr_tInt16                          SizeTabs;
  char                                filler_11[2];
  pwr_tInt16                          Segments;
  char                                filler_12[2];
} utl_t_listpar;

typedef struct {
  pwr_tString80                       Title;
  pwr_tString80                       PageHeader;
  pwr_tBoolean                        Crossreference;
  pwr_tEnum                           Externreference;
  pwr_tBoolean                        Deep;
  pwr_tEnum                           AlphaOrder;
  pwr_tBoolean                        NoPrint;
  pwr_tBoolean                        NoPrintIfNoList;
  pwr_tMask                           Full;
  pwr_tBoolean                        Landscape;
  pwr_tEnum                           PageBreak;
  pwr_tUInt8                          RowsToPageBreak;
  char                                filler_4[3];
  pwr_tUInt8                          ClearRowsPre;
  char                                filler_5[3];
  pwr_tUInt8                          ClearRowsPost;
  char                                filler_6[3];
  pwr_tUInt8                          ClearRowsPreList;
  char                                filler_7[3];
  pwr_tUInt8                          ClearRowsPostList;
  char                                filler_8[3];
  pwr_tBoolean                        ColumnHeader;
  pwr_tBoolean                        ColHead_____;
  pwr_tEnum                           TableOfContents;
  pwr_tUInt8                          TCSegments;
  char                                filler_10[3];
  pwr_tString16                       TCMarginString;
  pwr_tString80                       Hierarchyobject;
  pwr_tString80                       Name;
  pwr_tString80                       Class;
  pwr_tString80                       Parameter;
  utl_t_listpar		ParDescription[ UTL_PARDESCRIPTION ];
} utl_t_listbody;


typedef struct utl_s_list {
	pwr_sAttrRef  		o;
	unsigned long		specification;
	pwr_sAttrRef		refo;
	struct utl_s_list	*sublist[UTL_LIST_MAX];
	int			sublistcount[UTL_LIST_MAX];
	struct utl_s_list	*next;
	} utl_t_list;

typedef struct s_contlist{
	int			page;
	char			text[120];
	struct s_contlist	*next;
	} utl_t_contlist;

typedef struct {
	FILE		*output_file;
	int		terminal;
	int		confirm;
	int		log;
	int		append;
	int		page;
	int		row;
	utl_t_list	*list[UTL_LIST_MAX];	
	int		listcount[UTL_LIST_MAX];
	ldh_tSesContext ldhses;
	char		page_title[80];
	int		page_first;
	int		landscape;
	int		rows;
	utl_t_listbody	*listbody;
	utl_t_contlist	*contlist;
	int		contcount;
	} utl_t_ctx, *utl_ctx;


typedef struct {
	pwr_tCid 	cid;
	char		body[32];
	char		attr[32];
	unsigned char	write;
        int		list;
	} crr_t_searchlist;


#define IF_OUT if (utlctx->output_file != NULL)
#define IF_TER if (utlctx->terminal != 0)


#define CRR_READ	0
#define CRR_WRITE	1
#define CRR_GETFROMOBJECT 2
#define CRR_IGNORE	3
#define CRR_REF		4


static void	u_row( 
    utl_ctx		utlctx
);

static void	u_pagebreak(
    utl_ctx		utlctx
);

static void	u_pageend(
    utl_ctx		utlctx
);

static void	u_force_pagebreak(
    utl_ctx		utlctx
);

static void	u_header( 
    utl_ctx		utlctx,
    ldh_tSesContext ldhses,
    char		*title
);

static void	u_subheader(
    utl_ctx		utlctx,
    char		*title,
    char		*spec
);

static void	u_posit(
    utl_ctx		utlctx,
    int		tabs,
    int		len
);

static int	u_open(
    utl_ctx		utlctx,
    char		*filename,
    int			terminal,
    int			append
);

static void	u_close(
    utl_ctx		utlctx
);

static void	u_print( 
    utl_ctx		utlctx,
    char		*format,
    ...
    /*	unsigned long	a1, */
    /*  unsigned long	a2, */
    /*  unsigned long	a3, */
    /*  unsigned long	a4 */
);







/* 
 * Prototypes
 */
static int utl_list_sort (
  utl_t_list	**list,
  int		size,
  ldh_tSesContext ldhses,
  unsigned long	type
);

static int utl_list_classsort (
  utl_t_list	**list,
  int		size,
  ldh_tSesContext ldhses,
  unsigned long	type
);

static int utl_list_insert ( 
  utl_t_list	**list,
  int		*count,
  pwr_sAttrRef  *arp,
  unsigned long	specification,
  int		check,
  int		dum
);

static int utl_list_sublist (
  utl_ctx	utlctx,
  pwr_tObjid	listobjdid,
  utl_t_list	**list,
  int		*listcount,
  pwr_sAttrRef	*rootarp
);

static int utl_list_sublist_print ( 
  utl_ctx	utlctx,
  pwr_tObjid	listobjdid,
  utl_t_list	*list,
  int		listcount,
  int		*first
);

static int utl_list_print_par (
  utl_ctx	utlctx,
  pwr_sAttrRef	*o,
  int		specification,
  pwr_sAttrRef	*oref,
  utl_t_listbody  *listbody_ptr,
  int		parindex,
  int		new_row
);

static int utl_list_get_parvalue (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_listpar	*list_desc,
  char		*text
);

static int utl_list_print_columnheader ( 
  utl_ctx		utlctx,
  utl_t_listbody	*listbody_ptr
);






static void utl_ctx_new (
  utl_ctx	  *utlctx,
  ldh_tSesContext ldhses,
  char		*page_title,
  int		landscape
);

static int utl_ctx_delete (
  utl_ctx		utlctx
);

static int utl_ctx_free_sublist (
  utl_t_list	*list,
  int		listcount
);



static int utl_tableofcont_insert (
  utl_ctx	utlctx,
  pwr_sAttrRef	*arp,
  int		segments,
  char		*marginstr,
  int		page
);

static int utl_tableofcont_print (
 utl_ctx	utlctx
);

static int utl_ctxlist_insert (
  pwr_sAttrRef  *arp,
  utl_t_list	**list,
  int		*count,
  unsigned long	specification,
  int	check,
  int	dum
);



static int utl_print_object ( 
  pwr_tObjid    objid,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  int		full,
  char		*parameter,
  int		*element
);
static int utl_print_aref ( 
  pwr_sAttrRef  *arp,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  int		full,
  char		*parameter,
  int		*element
);

static int utl_print_object_full ( 
  pwr_sAttrRef	  *arp,
  ldh_tSesContext ldhses,	
  utl_ctx	  utlctx,
  int		  code
);

static int utl_print_object_par ( 
  pwr_sAttrRef	*arp,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  char		*parameter,
  int		*element
);

static int utl_print_class (
  pwr_sAttrRef	*arp,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  int		full,
  unsigned long	dum3,
  unsigned long	dum4
);

static int utl_print_class_full (
  pwr_tObjid	klass,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx
);




static int utl_crossref ( 
  ldh_tSesContext ldhses,
  pwr_sAttrRef	*arp,
  utl_ctx	utlctx,
  unsigned long	dum2,
  unsigned long	dum3,
  unsigned long	dum4
);

static int utl_externref (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_list	**crrlist,
  int		*crrcount
);

static int utl_signalref (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_list	**crrlist,
  int		*crrcount
);

static int utl_childref (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_list	**crrlist,
  int		*crrcount
);

static int crr_refobject (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_list	**crrlist,
  int		*crrcount
);

static int utl_object_changed ( 
  pwr_tObjid	Objdid,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  int		*changed,
  int		code
);

static int utl_set_parameter (
  pwr_sAttrRef	*arp,
  ldh_tSesContext ldhses,	
  char		*parameter,
  char		*invaluestr,
  int		element,
  utl_ctx	utlctx
);

static int utl_content ( 
  pwr_tObjid	Objdid,
  ldh_tSesContext ldhses,
  utl_ctx	utlctx,
  unsigned long	dum2,
  unsigned long	dum3,
  unsigned long	dum4
);


static int utl_object_delete (
  pwr_tObjid  Objdid,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  unsigned long	dum1,
  unsigned long	dum2,
  unsigned long	dum3
);

static int utl_tree_delete ( 
  pwr_tObjid  Objdid,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  unsigned long	dum1,
  unsigned long	dum2,
  unsigned long	dum3
);

static int	  utl_config_replace( 
    char	*instr,
    char	*outstr,
    int	index
);

static int utl_get_listconfig_object(
  ldh_tSesContext ldhses,
  int		  *landscape_rows,
  int		  *portrait_rows
);


/*************************************************************************
*
* Cross reference list functions.
*
**************************************************************************/
typedef struct crossdoc_tag {
	pwr_tObjid		objid;
	pwr_tObjid		parent;
	float			ll_x;
	float			ll_y;
	float			ur_x;
	float			ur_y;
	char			page[8];
	struct crossdoc_tag	*next;
	} crossdoc_t_list;

typedef struct cross_tag{
	pwr_sAttrRef		o;
	pwr_sAttrRef		refo;
	pwr_tObjid		parent;
	unsigned long		specification;
	struct cross_tag	*next;
	} cross_t_list;



static int	cross_doclist_add( 	crossdoc_t_list	**doclist, 
				int		*doclist_count, 
				pwr_tObjid	objid,
				pwr_tObjid	parent,
				float		ll_x,
				float		ll_y,
				float		ur_x,
				float		ur_y,
				char		*page
);

static int	cross_doclist_object_insert(
		pwr_sAttrRef	*arp,
		ldh_tSesContext	ldhses,
		int		dum1,
		int		dum2,
		int		dum3,
		int		dum4
);

static int	cross_doclist_load( ldh_tSesContext ldhses);

static int	cross_doclist_unload();

static int	cross_get_object_page(
			ldh_tSesContext ldhses,
			pwr_tObjid	objid,
			char		*page
);

static int	cross_crosslist_add( 	cross_t_list	**crosslist, 
				int		*crosslist_count, 
				int		dum, 
				pwr_sAttrRef	*arp, 
				pwr_sAttrRef	*refarp,
				unsigned long	specification
);

static int	cross_crosslist_object_insert(
		pwr_sAttrRef	*arp,
		ldh_tSesContext	ldhses,
		int		dum1,
		int		dum2,
		int		dum3,
		int		dum4
);

static int 	cross_crosslist_load( ldh_tSesContext ldhses);

static int 	cross_crosslist_unload();

static int	crr_crossref_children(
			ldh_tSesContext ldhses,
			pwr_tObjid	objid,
			utl_t_list	**crrlist,
			int		*crrcount
);

static int	crr_crossref(
			ldh_tSesContext ldhses,
			pwr_sAttrRef	*arp,
			utl_t_list	**crrlist,
			int		*crrcount
);




/*************************************************************************
*
* Name:		print_plc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned int 	argc		I	command arguments
* char 		**argv		I	command arguments
* ldh_tSesContext ldhses		I	ldh session
* ldh_tWBContext  ldhwb		I	ldh workbench
* char *	plcstring	I	Name of Plcpgm object
*
* Description: 	Prints all documents in a plcpgm.
*
**************************************************************************/

int wb_utl::print_plc( 
  ldh_tSesContext ldhses,
  ldh_tWBContext  ldhwb,
  char		  *plcstring,
  int		  document,
  int		  overview
)
{
  int		sts;
  pwr_tObjid	plc;

  /* Get objdid for the plcpgm */
  sts = ldh_NameToObjid( ldhses, &plc, plcstring);
  if ( EVEN(sts)) {
    return FOE__PLCNOTFOUND;
  }
  /* Check that this is a plcpgm object */
  if ( !vldh_check_plcpgm( ldhses, plc)) {
    return FOE__NOPLC;
  }
 
  /* Print the plc */
  printf( "Plcpgm  %s\n", plcstring);

  sts = print_document( plc, ldhses, ldhwb, document, overview);
  return sts;
}

/*************************************************************************
*
* Name:		print_plc_hier()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session
* ldh_tWBContext  ldhwb		I	ldh workbench
* char 		*hiername	I	Name of an object in the hierarchy
*
* Description: 	Prints all documents in plcpgm's that
*		is found below the specified object in the hierarchy.
*
**************************************************************************/

int wb_utl::print_plc_hier (
  ldh_tSesContext ldhses,
  ldh_tWBContext  ldhwb,
  char		*hiername,
  char		*fromname,
  int		document,
  int		overview,
  int		all
)
{
  int		sts, size;
  pwr_tClassId	*classp;
  pwr_tObjid	hierobjdid;
  pwr_tClassId	class_vect[2];
  utl_t_objidlist *list_ptr;
  utl_t_objidlist *plcpgmlist;
  int		plcpgmcount;
  pwr_tOName    plcname;
  pwr_tObjid    fromobjdid;
  int		from;
  int		from_found;

  /* Get class */
  class_vect[0] = pwr_cClass_plc;
  class_vect[1] = 0;
  classp = class_vect;

  if ( !all) {
    /* Get objdid for the hierarchy object */
    sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
    if ( EVEN(sts))
      return FOE__HIERNAME;
  }
  else
    hierobjdid = pwr_cNObjid;

  if ( fromname != NULL) {
    sts = ldh_NameToObjid( ldhses, &fromobjdid, fromname);
    if ( EVEN(sts)) {
      return FOE__OBJECT;
    }	
    from = 1;
  }
  else
    from = 0;

  plcpgmcount = 0;
  plcpgmlist = 0;
  sts = trv_get_objects_hier_class_name( ldhses, hierobjdid, classp, NULL,
		&utl_objidlist_insert, &plcpgmlist, &plcpgmcount,
		0, 0, 0);
  if ( EVEN (sts)) return sts;

  list_ptr = plcpgmlist;
  from_found = 0;
  while ( list_ptr) {
    if ( from) {
      if ( !from_found ) {
	if ( cdh_ObjidIsEqual( list_ptr->objid, fromobjdid)) {
	  /* Start to print from now on 	*/
	  from_found = 1;
	}
	else {
	  list_ptr = list_ptr->next;
	  continue;
	}
      }
    } 

    sts = ldh_ObjidToName( ldhses, list_ptr->objid, ldh_eName_Hierarchy,
		plcname, sizeof( plcname), &size);
    if ( EVEN(sts)) return sts;
    printf( "Plcpgm  %s\n", plcname);

    sts = print_document( list_ptr->objid, 
			  ldhses, ldhwb, document, overview);
    if ( EVEN(sts)) return sts;
	  
    list_ptr = list_ptr->next;
  }
  utl_objidlist_free( plcpgmlist);
  
  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_print_document()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  Objdid		I	objdid of plcpgm object
* Wiget		parent_widget	I	parent widget
* ldh_tSesContext ldhses		I	ldh session
* ldh_tWBContext  ldhwb		I	ldh workbench
* unsigned long	dum1		I	dummy argument.
*
* Description: 	Prints all documents found in a plcpgm.
*
**************************************************************************/

int wb_utl::print_document (
  pwr_tObjid	Objdid,
  ldh_tSesContext ldhses,
  ldh_tWBContext  ldhwb,
  unsigned long	document,
  unsigned long	overview
)
{
  int		sts, size;
  int		j;
  pwr_eClass	eclass;
  unsigned long	wind_count;
  pwr_tObjid	*windlist;
  pwr_tObjid	*windlist_ptr;
  pwr_tObjid	plc;
  pwr_tObjid	window;
  pwr_sPlcWindow	*windbuffer;
  WFoe		*foe;
  pwr_tObjid	nodeobjdid;
  pwr_tObjid	parwindobjdid;
  vldh_t_wind	parentwind;
  vldh_t_node	node;
  int		new_window;
  unsigned long	windowindex;

  /* Get objdid for the plcpgm */
  plc = Objdid;
 
  /* Get the windows */
  sts = trv_get_plc_window( ldhses, plc, &window);
  if ( sts == GSX__NOSUBWINDOW) {
    /* No subwindows on this window, return */
    return FOE__SUCCESS;
  }
  else if ( EVEN(sts)) return sts;
  
  sts = trv_get_window_windows( ldhses, window, &wind_count, &windlist);
  if ( EVEN(sts)) return sts;
  
  /* We don't want to see foe on the screen */

  /* Start foe for the root window */
  sts = utl_foe_new( "AutoPrint", plc, ldhwb, ldhses,
		     &foe, 0, ldh_eAccess_ReadOnly);
  if ( EVEN(sts)) return sts;

  if ( document) {
    /* Print the documents */
    sts = foe->print_document();
    if ( EVEN(sts)) return sts;
  }

  if ( overview) {
    sts = foe->print_overview();
    if ( EVEN(sts)) return sts;
  }

  windlist_ptr = windlist;
  windlist_ptr++;

  for ( j = 1; j < (int) wind_count; j++) {
    /* Get parent in ldh and find him in vldh */
    sts = ldh_GetParent( ldhses, *windlist_ptr, &nodeobjdid);
    if ( EVEN(sts)) return sts;

    /* Get the window of the parent */
    sts = ldh_GetParent( ldhses, nodeobjdid, &parwindobjdid);
    if ( EVEN(sts)) return sts;

    sts = vldh_get_wind_objdid( parwindobjdid, &parentwind);
    if ( EVEN(sts)) return sts;
    
    sts = vldh_get_node_objdid( nodeobjdid, parentwind, &node);
    if ( EVEN(sts)) return sts;
	       
    /* Get the window index for this window */
    sts = ldh_GetObjectBuffer( ldhses,
		*windlist_ptr,	"DevBody", "PlcWindow", &eclass,
		(char **)&windbuffer, &size);
    if( EVEN(sts)) return sts;

    windowindex = windbuffer->subwindowindex;
    free((char *) windbuffer);

    new_window = FALSE;
    foe = (WFoe *)parentwind->hw.foe;

    /* Create subwindow */
    sts = utl_foe_new_local( foe,
		node->hn.name, pwr_cNObjid, 0, parentwind->hw.ldhses,
		node, windowindex, new_window, &foe, 0, 
		ldh_eAccess_ReadOnly, foe_eFuncAccess_Edit);

    /* Print the documents */
    if ( document) {	
      sts = foe->print_document();
      if ( EVEN(sts)) return sts;
    }

    if ( overview) {
      sts = foe->print_overview();
      if ( EVEN(sts)) return sts;
    }

    windlist_ptr++;
  }

  /* Delete all foe */
  for ( j = 0; j < (int)wind_count; j++) {
    windlist_ptr--;
    sts = vldh_get_wind_objdid( *windlist_ptr, &parentwind);
    if ( EVEN(sts)) return sts;

    ((WFoe *)parentwind->hw.foe)->quit();
  }


  if ( wind_count > 0) free((char *) windlist);
  if ( wind_count == 0)
    return FOE__NOWIND;

  return FOE__SUCCESS;
}




/*************************************************************************
*
* Name:		redraw_plc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned int 	argc		I	command arguments
* char 		**argv		I	command arguments
* ldh_tSesContext ldhses		I	ldh session
* ldh_tWBContext  ldhwb		I	ldh workbench
* char *	plcstring	I	Name of Plcpgm object
*
* Description: 	Redraw all documents in a plcpgm.
*
**************************************************************************/

int wb_utl::redraw_plc( 
  ldh_tSesContext ldhses,
  ldh_tWBContext  ldhwb,
  char		*plcstring
)
{
  int		sts, sts2, size;
  pwr_tObjid	plc;
  ldh_sSessInfo	info;
  char		plc_objid_str[80];

  /* Get objdid for the plcpgm */
  sts = ldh_NameToObjid( ldhses, &plc, plcstring);
  if ( EVEN(sts)) {
    return FOE__PLCNOTFOUND;
  }
  /* Check that this is a plcpgm object */
  if ( !vldh_check_plcpgm( ldhses, plc)) {
    return FOE__NOPLC;
  }
 
  /* Check that the utilily session is saved */
  sts = ldh_GetSessionInfo( ldhses, &info);
  if ( EVEN(sts)) return sts;
  if ( !info.Empty)
    return GSX__NOTSAVED;

  /* To be able to redraw the windows, the session has to 
     be set to ReadOnly */
  sts = ldh_SetSession( ldhses, ldh_eAccess_ReadOnly);
  if ( EVEN(sts)) return sts;

  sts = ldh_ObjidToName( 
		ldhses, plc, ldh_eName_Objid,
		plc_objid_str, sizeof( plc_objid_str), &size);

  /* Redraw the plc */
  printf( "Plcpgm  %s		%s\n",
	  plcstring, plc_objid_str);

  sts = redraw_windows( plc, ldhses, ldhwb);

  /* Return to session access ReadWrite */ 
  sts2 = ldh_SetSession( ldhses, ldh_eAccess_ReadWrite);
  if ( EVEN(sts2)) return sts2;

  if ( EVEN(sts)) return sts;
  return FOE__SUCCESS;

}

/*************************************************************************
*
* Name:		utl_redraw_plc_hier()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session
* ldh_tWBContext  ldhwb		I	ldh workbench
* char 		*hiername	I	Name of an object in the hierarchy
*
* Description: 	Redraw all documents in plcpgm's that
*		is found below the specified object in the hierarchy.
*
**************************************************************************/

int wb_utl::redraw_plc_hier (
  ldh_tSesContext ldhses,
  ldh_tWBContext  ldhwb,
  char		*hiername,
  char		*fromname,
  int		all
)
{
  int			sts, sts2, size;
  pwr_tClassId		*classp;
  pwr_tObjid		hierobjdid;
  pwr_tClassId		class_vect[2];
  utl_t_objidlist	*list_ptr;
  utl_t_objidlist	*plcpgmlist;
  int			plcpgmcount;
  pwr_tOName     	plcname;
  pwr_tObjid		fromobjdid;
  int			from;
  int			from_found;
  ldh_sSessInfo		info;
  char			plc_objid_str[80];

  /* Get class */
  class_vect[0] = pwr_cClass_plc;
  class_vect[1] = 0;
  classp = class_vect;

  if ( !all) {
    /* Get objdid for the hierarchy object */
    sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
    if ( EVEN(sts))
      return FOE__HIERNAME;
  }
  else
    hierobjdid = pwr_cNObjid;

  if ( fromname != NULL) {
    sts = ldh_NameToObjid( ldhses, &fromobjdid, fromname);
    if ( EVEN(sts)) {
      return FOE__OBJECT;
    }	
    from = 1;
  }
  else
    from = 0;

  /* Check that the utilily session is saved */
  sts = ldh_GetSessionInfo( ldhses, &info);
  if ( EVEN(sts)) return sts;
  if ( !info.Empty)
    return GSX__NOTSAVED;

  /* To be able to redraw the windows, the session has to 
     be set to ReadOnly */
  sts = ldh_SetSession( ldhses, ldh_eAccess_ReadOnly);
  if ( EVEN(sts)) return sts;

  plcpgmcount = 0;
  plcpgmlist = 0;
  sts = trv_get_objects_hier_class_name( ldhses, hierobjdid, classp, NULL,
		&utl_objidlist_insert, &plcpgmlist, &plcpgmcount,
		0, 0, 0);
  if ( EVEN (sts)) goto error_return;

  list_ptr = plcpgmlist;
  from_found = 0;
  while ( list_ptr) {
    if ( from) {
      if ( !from_found ) {
	if ( cdh_ObjidIsEqual( list_ptr->objid, fromobjdid)) {
	  /* Start to redraw from now on 	*/
	  from_found = 1;
	}
	else {
	  list_ptr = list_ptr->next;
	  continue;
	}
      }
    } 
    
    sts = ldh_ObjidToName( ldhses, list_ptr->objid, ldh_eName_Hierarchy,
		plcname, sizeof( plcname), &size);
    if ( EVEN (sts)) goto error_return;
    sts = ldh_ObjidToName( ldhses, list_ptr->objid, ldh_eName_Objid,
		plc_objid_str, sizeof( plc_objid_str), &size);
    if ( EVEN (sts)) goto error_return;

    printf( "Plcpgm  %s		%s\n", 
	    plcname, plc_objid_str);

    sts = redraw_windows( list_ptr->objid,
			  ldhses, ldhwb);
    if ( EVEN (sts)) goto error_return;
	  
    list_ptr = list_ptr->next;
  }
  utl_objidlist_free( plcpgmlist);

  /* Return to session access ReadWrite */ 
  sts2 = ldh_SetSession( ldhses, ldh_eAccess_ReadWrite);
  if ( EVEN(sts2)) return sts2;

  return FOE__SUCCESS;

error_return:
  sts2 = ldh_SetSession( ldhses, ldh_eAccess_ReadWrite);
  if ( EVEN(sts2)) return sts2;

  return sts;
}


/*************************************************************************
*
* Name:		redraw_windows()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	objdid of plcpgm object
* ldh_tSesContext ldhses		I	ldh session
* ldh_tWBContext  ldhwb		I	ldh workbench
* unsigned long	dum1		I	dummy argument.
*
* Description: 	Redraws all windows found in a plcpgm.
*
**************************************************************************/

int wb_utl::redraw_windows ( 
  pwr_tObjid	  Objdid,
  ldh_tSesContext ldhses,
  ldh_tWBContext  ldhwb
)
{
  int		sts, size;
  int		j;
  pwr_eClass	eclass;
  unsigned long	wind_count;
  pwr_tObjid	*windlist;
  pwr_tObjid	*windlist_ptr;
  pwr_tObjid	plc;
  pwr_tObjid	window;
  pwr_sPlcWindow	*windbuffer;
  WFoe		*foe;
  pwr_tObjid	nodeobjdid;
  pwr_tObjid	parwindobjdid;
  vldh_t_wind	parentwind;
  vldh_t_node	node;
  int		new_window;
  unsigned long	windowindex;

  /* Get objdid for the plcpgm */
  plc = Objdid;
 
  /* Get the windows */
  sts = trv_get_plc_window( ldhses, plc, &window);
  if ( sts == GSX__NOSUBWINDOW) {
    /* No subwindows on this window, return */
    return FOE__SUCCESS;
  }
  else if ( EVEN(sts)) return sts;

  sts = trv_get_window_windows( ldhses, window, &wind_count, &windlist);
  if ( EVEN(sts)) return sts;
	  
  /* We don't want to see foe on the screen */

  /* Start foe for the root window */
  sts = utl_foe_new( "AutoPrint", plc, ldhwb, ldhses,
		    &foe, 0, ldh_eAccess_SharedReadWrite);
  if ( EVEN(sts)) return sts;

  /* Redraw the window */
  sts = foe->redraw_and_save();
  if ( EVEN(sts)) return sts;

  windlist_ptr = windlist;
  windlist_ptr++;

  for ( j = 1; j < (int)wind_count; j++) {
    /* Get parent in ldh and find him in vldh */
    sts = ldh_GetParent( ldhses, *windlist_ptr, &nodeobjdid);
    if ( EVEN(sts)) return sts;

    /* Get the window of the parent */
    sts = ldh_GetParent( ldhses, nodeobjdid, &parwindobjdid);
    if ( EVEN(sts)) return sts;

    sts = vldh_get_wind_objdid( parwindobjdid, &parentwind);
    if ( EVEN(sts)) return sts;

    sts = vldh_get_node_objdid( nodeobjdid, parentwind, &node);
    if ( EVEN(sts)) return sts;
	       
    /* Get the window index for this window */
    sts = ldh_GetObjectBuffer( ldhses,
		*windlist_ptr,	"DevBody", "PlcWindow", &eclass,	
		(char **)&windbuffer, &size);
    if( EVEN(sts)) return sts;

    windowindex = windbuffer->subwindowindex;
    free((char *) windbuffer);

    new_window = FALSE;
    foe = (WFoe *)parentwind->hw.foe;

    /* Create subwindow */
    sts = utl_foe_new_local( foe,
		node->hn.name, pwr_cNObjid, 0, parentwind->hw.ldhses,
		node, windowindex, new_window, &foe, 0, 
		ldh_eAccess_SharedReadWrite, foe_eFuncAccess_Edit);

    /* Redraw the window */
    sts = foe->redraw_and_save();
    if ( EVEN(sts)) return sts;

    windlist_ptr++;
  }

  /* Delete all foe */
  for ( j = 0; j < (int)wind_count; j++) {
    windlist_ptr--;
    sts = vldh_get_wind_objdid( *windlist_ptr, &parentwind);
    if ( EVEN(sts)) return sts;

    ((WFoe *)parentwind->hw.foe)->quit();
  }


  if ( wind_count > 0) free((char *) windlist);
  if ( wind_count == 0)
    return FOE__NOWIND;

  return FOE__SUCCESS;
}


/*_Methods defined for this module_______________________________________*/


/*************************************************************************
*
* Name:		utl_toupper()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*str		I	input string.
* char		*upper_str	I	string converted to upper case.
*
* Description:
*	Converts a string to upper case.
*
**************************************************************************/

int utl_toupper ( 
  char	*str_upper,
  char	*str
)
{
	cdh_ToUpper( str_upper, str);
	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_parse()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* char		*string		I	string to be parsed.
* char		*parse_char	I	parse charachter(s).
* char		*inc_parse_char	I	parse charachter(s) that will be
*					included in the parsed string.
* char		*outstr		O	parsed strings.
* int		max_rows	I	maximum number of chars in a parsed
*					string.
* int 		max_cols	I	maximum number of parsed elements.
*
* Description:
*	Parses a string.
*
**************************************************************************/

int utl_parse (
  char	*instring,
  char	*parse_char,
  char	*inc_parse_char,
  char	*outstr,
  int	max_rows,
  int 	max_cols
)
{
	char	*string;
	int	row;
	int	col;
	char	*char_ptr;	
	char	*inc_char_ptr;	
	int	parsechar_found;
	int	inc_parsechar_found;
	int	next_token;
	int	char_found;
	int	one_token = 0;
	int	nullstr;

	string = instring;
	row = 0;
	col = 0;
	char_found = 0;
	next_token = 0;
	nullstr = 0;
	while ( *string != '\0')	
	{
	  char_ptr = parse_char;
	  inc_char_ptr = inc_parse_char;
	  parsechar_found = 0;
	  inc_parsechar_found = 0;
	  if ( *string == '"')
	  {
	    one_token = !one_token;
	    if ( !one_token && col == 0)
	      nullstr = 1;
	    else
	      nullstr = 0;
	    string++;
	    continue;
	  }
	  if ( !one_token)
	  {
	    while ( *char_ptr != '\0')
	    {
	      /* Check if this is a parse charachter */
	      if ( *string == *char_ptr)
	      {
	        parsechar_found = 1;
	        /* Next token */
	        if ( col > 0 || nullstr)	
	        {
	          *(outstr + row * max_cols + col) = '\0';
	          row++;
	          if ( row >= max_rows )
	            return row;
	          col = 0;
	          next_token = 0;
	        }
	        break;
	      }
	      char_ptr++;	    
	    }
	    while ( *inc_char_ptr != '\0')
	    {
	      /* Check if this is a parse charachter */
	      if ( *string == *inc_char_ptr)
	      {
	        parsechar_found = 1;
	        inc_parsechar_found = 1;
	        /* Next token */
	        if ( col > 0)	
	        {
	          *(outstr + row * max_cols + col) = '\0';
	          row++;
	          if ( row >= max_rows )
	            return row;
	          col = 0;
	          next_token = 0;
	        }
	        break;
	      }
	      inc_char_ptr++;	    
	    }
	  }
	  if ( !parsechar_found && !next_token)
	  {
	    char_found++;
	    *(outstr + row * max_cols + col) = *string;
	    col++;
	  }
	  if ( inc_parsechar_found)
	  {
	    *(outstr + row * max_cols + col) = *inc_char_ptr;
	    col++;
	  }
	  string++; 
	  if ( col >= (max_cols - 1))
	    next_token = 1;	    
	  nullstr = 0;
	}
	*(outstr + row * max_cols + col) = '\0';
	row++;

	if ( char_found == 0)
	  return 0;

	return row;
}


/*************************************************************************
*
* Name:		utl_parse_indexstring()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	This parses a string of numbers and returns a byte
*	array with the specified numbers set to 1.
*	ex 2,4,7-10,12 will return 001010011110100.. 
*
**************************************************************************/

int utl_parse_indexstring (
  char	*indexstring,
  char	*indexarray,
  int	size
)
{
	char	index_str_b[50][15];
	char	index_str[2][15];
	int	index[2];
	int	nr, num, i, j;

	memset( indexarray, 0, size);

	nr = utl_parse( indexstring, " ", "", (char *)index_str_b, 
		sizeof( index_str_b) / sizeof( index_str_b[0]), 
		sizeof( index_str_b[0]));
	for ( i = 0; i < nr; i++)
	{
	  num = utl_parse( index_str_b[i], "-", "", (char *)index_str, 
		sizeof( index_str) / sizeof( index_str[0]), 
		sizeof( index_str[0]));
	  if ( num == 1)
	  {	  
	    sscanf((char *)&index_str[0], "%d", &index[0]);
	    index[1] = index[0];
	  }
	  else if ( num == 2)
	  {	  
	    sscanf((char *)index_str[0], "%d", &index[0]);
	    sscanf((char *)index_str[1], "%d", &index[1]);
	  }
	  else
	    return 0;

	  for ( j = 0; j < size; j++)
	  {
	    if ( (j >= index[0]) && (j <= index[1]))
	      *(indexarray + j ) = 1;
	  }
	}
	return 1;

}


/*************************************************************************
*
* Name:		utl_list_sort()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* utl_t_list	*list		I	object list.
*
* Description:
*	This function sorts objects.
*
**************************************************************************/

static int utl_list_sort (
  utl_t_list	**list,
  int		size,
  ldh_tSesContext ldhses,
  unsigned long	type
)
{
	int	i, j;
	utl_t_list	*next_ptr;
	utl_t_list	*list_ptr;
	utl_t_list	**prev;
	char		*np;
	pwr_tAName     	name1;
	pwr_tAName     	name2;
	int		nsize;
	int		sts;

	if ( *list == NULL)
	  return FOE__SUCCESS;
	if ( (*list)->next == NULL)
	  return FOE__SUCCESS;

	for ( i = size - 1; i > 0; i--) {
	  prev = list;
	  list_ptr = *list;
	  next_ptr = list_ptr->next;
	  for ( j = 0; j < i; j++) {
	    if ( type < 2) {
	      /* Compare hierarchy name */
	      sts = ldh_AttrRefToName( ldhses, &list_ptr->o, 
		ldh_eName_Hierarchy, &np, &nsize);
	      if ( EVEN(sts)) return sts;
	      strncpy( name1, np, sizeof(name1));

	      sts = ldh_AttrRefToName( ldhses, &next_ptr->o, 
		ldh_eName_Hierarchy, &np, &nsize);
	      if ( EVEN(sts)) return sts;
	      strncpy( name2, np, sizeof(name2));
	    }
	    else if ( type == 2) {
	      /* Compare last segment in object name */
	      sts = ldh_AttrRefToName( ldhses, &list_ptr->o, 
		ldh_eName_Object, &np, &nsize);
	      if ( EVEN(sts)) return sts;
	      strncpy( name1, np, sizeof(name1));

	      sts = ldh_AttrRefToName( ldhses, &next_ptr->o, 
		ldh_eName_Object, &np, &nsize);
	      if ( EVEN(sts)) return sts;
	      strncpy( name2, np, sizeof(name2));
	    }
	    else if ( type == 3) {
	      /* Compare referenced object name */
	      sts = ldh_AttrRefToName( ldhses, &list_ptr->refo, 
		ldh_eName_Object, &np, &nsize);
	      if ( EVEN(sts)) return sts;
	      strncpy( name1, np, sizeof(name1));

	      sts = ldh_AttrRefToName( ldhses, &next_ptr->refo, 
		ldh_eName_Object, &np, &nsize);
	      if ( EVEN(sts)) return sts;
	      strncpy( name2, np, sizeof(name2));
	    }
	    else if ( type == 4) {
	      pwr_sAttrRef	*conobj_ptr;

	      /* Compare SigChanCon */
	      sts = ldh_GetAttrObjectPar( ldhses, &list_ptr->o, "RtBody", 
					  "SigChanCon", (char **)&conobj_ptr, &nsize);
	      if ( ODD(sts)) {
		if ( cdh_ObjidIsNull( conobj_ptr->Objid)) {
		  strcpy( name1, "-");
		  free((char *) conobj_ptr);
		}
		else {
		  sts = ldh_AttrRefToName( ldhses, conobj_ptr, 
					   ldh_eName_Hierarchy, &np, &nsize);
		  if ( ODD(sts))
		    strcpy( name1, np);
		  else
		    strcpy( name1, "-");
		}
	      }

	      sts = ldh_GetAttrObjectPar( ldhses, &next_ptr->o, "RtBody", 
					  "SigChanCon", (char **)&conobj_ptr, &nsize);
	      if ( ODD(sts)) {
		if ( cdh_ObjidIsNull( conobj_ptr->Objid)) {
		  strcpy( name2, "-");
		  free((char *) conobj_ptr);
		}
		else {
		  sts = ldh_AttrRefToName( ldhses, conobj_ptr, 
					   ldh_eName_Hierarchy, &np, &nsize);
		  if ( ODD(sts))
		    strcpy( name2, np);
		  else
		    strcpy( name2, "-");
		}
	      }
	    }

	    if ( cdh_NoCaseStrcmp( name1, name2) > 0) {
	      /* Change order */
	      *prev = next_ptr;
	      list_ptr->next = next_ptr->next;
	      next_ptr->next = list_ptr;
	      list_ptr = next_ptr;
	      next_ptr = list_ptr->next;
	    }
	    prev = &list_ptr->next;
	    list_ptr = list_ptr->next;
	    next_ptr = list_ptr->next;
	  }
	}
	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_list_classsort()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* utl_t_list	*list		I	object list.
*
* Description:
*	This function sorts objects.
*
**************************************************************************/

static int utl_list_classsort (
  utl_t_list	**list,
  int		size,
  ldh_tSesContext ldhses,
  unsigned long	type
)
{
	int	i, j;
	utl_t_list	*next_ptr;
	utl_t_list	*list_ptr;
	utl_t_list	**prev;
	pwr_tAName     	name1;
	pwr_tAName     	name2;
	int		namesize;
	int		sts;
	pwr_tClassId	class1;
	pwr_tClassId	class2;
	pwr_tObjName   	classname1;
	pwr_tObjName   	classname2;
	pwr_tAName     	dummytxt;
	char		*np;

	if ( *list == NULL)
	  return FOE__SUCCESS;
	if ( (*list)->next == NULL)
	  return FOE__SUCCESS;

	for ( i = size - 1; i > 0; i--) {
	  prev = list;
	  list_ptr = *list;
	  next_ptr = list_ptr->next;
	  for ( j = 0; j < i; j++)
	  {
	    /* Compare last segment in object name */
	    sts = ldh_AttrRefToName( ldhses, &list_ptr->o, 
		ldh_eName_Object, &np, &namesize);
	    if ( EVEN(sts)) return sts;
	    strncpy( name1, np, sizeof(name1));

	    sts = ldh_AttrRefToName( ldhses, &next_ptr->o, 
		ldh_eName_Object, &np, &namesize);
	    if ( EVEN(sts)) return sts;
	    strncpy( name2, np, sizeof(name2));

	    utl_toupper( name2, name2);
	    utl_toupper( name1, name1);

            /* get the object class */
      	    sts = ldh_GetAttrRefTid( ldhses, &list_ptr->o, &class1);
	    if ( EVEN(sts)) return sts;
      	    sts = ldh_GetAttrRefTid( ldhses, &next_ptr->o, &class2);
	    if ( EVEN(sts)) return sts;
	    sts = ldh_ObjidToName( ldhses, cdh_ClassIdToObjid( class1), 
		ldh_eName_Object, classname1, sizeof( classname1), &namesize);
	    if ( EVEN(sts)) return sts;
	    sts = ldh_ObjidToName( ldhses, cdh_ClassIdToObjid( class2), 
		ldh_eName_Object, classname2, sizeof( classname2), &namesize);
	    if ( EVEN(sts)) return sts;

	    if ( type <= 2) {
	      /* Put hierarchy and plcpgm objects first */
	      if (strcmp( classname1, "$PlantHier") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "000");
	        strcat( name1, dummytxt);
	      }
	      else if (strcmp( classname1, "PlcPgm") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "001");
	        strcat( name1, dummytxt);
	      }

	      if (strcmp( classname2, "$PlantHier") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "000");
	        strcat( name2, dummytxt);
	      }
	      else if (strcmp( classname2, "PlcPgm") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "001");
	        strcat( name2, dummytxt);
	      }
	    }

	    if ( type == 2) {
	      if (strcmp( classname1, "Ai") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "002");
	        strcat( name1, dummytxt);
	      }
	      else if (strcmp( classname1, "Ao") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "003");
	        strcat( name1, dummytxt);
	      }
	      else if (strcmp( classname1, "Av") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "004");
	        strcat( name1, dummytxt);
	      }
	      else if (strcmp( classname1, "Co") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "005");
	        strcat( name1, dummytxt);
	      }
	      else if (strcmp( classname1, "Di") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "006");
	        strcat( name1, dummytxt);
	      }
	      else if (strcmp( classname1, "Do") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "007");
	        strcat( name1, dummytxt);
	      }
	      else if (strcmp( classname1, "Dv") == 0) {
	        strcpy( dummytxt, name1);
	        strcpy( name1, "008");
	        strcat( name1, dummytxt);
	      }

	      if (strcmp( classname2, "Ai") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "002");
	        strcat( name2, dummytxt);
	      }
	      else if (strcmp( classname2, "Ao") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "003");
	        strcat( name2, dummytxt);
	      }
	      else if (strcmp( classname2, "Av") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "004");
	        strcat( name2, dummytxt);
	      }
	      else if (strcmp( classname2, "Co") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "005");
	        strcat( name2, dummytxt);
	      }
	      else if (strcmp( classname2, "Di") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "006");
	        strcat( name2, dummytxt);
	      }
	      else if (strcmp( classname2, "Do") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "007");
	        strcat( name2, dummytxt);
	      }
	      else if (strcmp( classname2, "Dv") == 0) {
	        strcpy( dummytxt, name2);
	        strcpy( name2, "008");
	        strcat( name2, dummytxt);
	      }

	    }

	    if ( type == 3) {
	      /* Order first in classes and then in alpabeth order */
	      strcpy( dummytxt, name1);
	      strcpy( name1, "00000000000000000000");
	      strncpy( name1, classname1, strlen( classname1));
	      strcat( name1, dummytxt);

	      strcpy( dummytxt, name2);
	      strcpy( name2, "00000000000000000000");
	      strncpy( name2, classname2, strlen( classname2));
	      strcat( name2, dummytxt);
	    }

	    if ( strcmp( name1, name2) > 0) {
	      /* Change order */
	      *prev = next_ptr;
	      list_ptr->next = next_ptr->next;
	      next_ptr->next = list_ptr;
	      list_ptr = next_ptr;
	      next_ptr = list_ptr->next;
	    }
	    prev = &list_ptr->next;
	    list_ptr = list_ptr->next;
	    next_ptr = list_ptr->next;
	  }
	}
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_ctx_new()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	*utlctx		O	created utl context
* ldh_tSesContext ldhses		I	ldh session;
*
* Description:
*	Create a utl context.
*
**************************************************************************/

static void utl_ctx_new (
  utl_ctx	  *utlctx,
  ldh_tSesContext ldhses,
  char		*page_title,
  int		landscape
)
{
	int	sts;
	int	landscape_rows;
	int	portrait_rows;

	/* Create the context */
	*utlctx = (utl_ctx)calloc( 1, sizeof( **utlctx ) );
	(*utlctx)->ldhses = ldhses;
	strcpy( ((*utlctx)->page_title), page_title);
	(*utlctx)->page_first = 0;
	(*utlctx)->landscape = landscape;
	sts = utl_get_listconfig_object( ldhses, &landscape_rows,
		&portrait_rows);
	if ( (*utlctx)->landscape == UTL_LANDSCAPE)
	  (*utlctx)->rows = landscape_rows;
	else
	  (*utlctx)->rows = portrait_rows;
}


/*************************************************************************
*
* Name:		utl_ctx_delete()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Delete a context.
*	Free's all allocated memory in the utl context.
*
**************************************************************************/

static int utl_ctx_delete (
  utl_ctx		utlctx
)
{
	int	i;

	/* Free the memory for the list and the context */

	/* Free sublists here !!! ... */

	for ( i = 0; i < UTL_LIST_MAX; i++)
	{
	  if ( utlctx->list[i] != 0)
	  {
	    utl_ctx_free_sublist( utlctx->list[i], utlctx->listcount[i]);
	  }
	}
	free((char *) utlctx);
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_ctx_free_sublist()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Free's allocated memory in a sublist.
*
**************************************************************************/

static int utl_ctx_free_sublist (
  utl_t_list	*list,
  int		listcount
)
{
  int	i;

  for ( i = 0; i < UTL_LIST_MAX; i++) {
    if ( list->sublistcount[i] != 0) {
      utl_ctx_free_sublist( list->sublist[i], list->sublistcount[i]);
    }
  }
  free((char *) list);
  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_list_insert()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Insert the node in the list in utl context.
*	Check first that the object is not already inserted.
*
**************************************************************************/

static int utl_list_insert ( 
  utl_t_list	**list,
  int		*count,
  pwr_sAttrRef  *arp,
  unsigned long	specification,
  int		check,
  int		dum
)
{
	int		found;
	utl_t_list	*list_ptr;
	
	/* Check if the attrref already is inserted */
	found = 0;
	if ( *list) {
	  list_ptr = *list;
	  if ( check) {
	    while( list_ptr->next) {
	      if ( cdh_ArefIsEqual( &list_ptr->o, arp)) {
	        found = 1;
	        break;
	      }
	      list_ptr = list_ptr->next;
	    }
	  }
	  else {
	    while( list_ptr->next)
	      list_ptr = list_ptr->next;
	  }
	}
	if ( found)
	  return FOE__SUCCESS;

	/* The objdid was not found, insert it */
	if ( *list) {
	  list_ptr->next = (utl_t_list *) calloc( 1, sizeof(utl_t_list));
	  if ( list_ptr->next == 0)
	    return FOE__NOMEMORY;
	  list_ptr = list_ptr->next;
	}
	else {
	  list_ptr = (utl_t_list *) calloc( 1, sizeof(utl_t_list));
	  if ( list_ptr == 0)
	    return FOE__NOMEMORY;
	  *list = list_ptr;
	}
	list_ptr->o = *arp;
	list_ptr->specification = specification;
	(*count)++;

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_objidlist_free()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Free a utl short list.
*
**************************************************************************/
void utl_objidlist_free( utl_t_objidlist *list)
{
	utl_t_objidlist	*list_ptr;
	utl_t_objidlist	*next_ptr;

	list_ptr = list;
	while ( list_ptr)
	{
	  next_ptr = list_ptr->next;
	  free((char *) list_ptr);
	  list_ptr = next_ptr;
	}
}

/*************************************************************************
*
* Name:		utl_objidlist_insert()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Insert the node in a utl short list.
*
**************************************************************************/

int utl_objidlist_insert (
  pwr_sAttrRef  *arp,
  void		*l,
  void		*c,
  void		*dum1,
  void		*dum2,
  void		*dum3
)
{
	utl_t_objidlist	*list_ptr;
	utl_t_objidlist **list = (utl_t_objidlist **) l;
	int *count = (int *) c;
	
	
	if ( *list)
	{
	  list_ptr = *list;
	  while( list_ptr->next)
	    list_ptr = list_ptr->next;

	  list_ptr->next = (utl_t_objidlist *) calloc( 1, sizeof(utl_t_objidlist));
	  if ( list_ptr->next == 0)
	    return FOE__NOMEMORY;
	  list_ptr = list_ptr->next;
	}
	else
	{
	  list_ptr = (utl_t_objidlist *) calloc( 1, sizeof(utl_t_objidlist));
	  if ( list_ptr == 0)
	    return FOE__NOMEMORY;
	  *list = list_ptr;
	}
	list_ptr->objid = arp->Objid;
	(*count)++;
	return FOE__SUCCESS;
}

int	utl_list_count( pwr_tObjid  Objdid, int *count,
			int dum1, int dum2, int dum3, int dum4)
{
	(*count)++;
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_tableofcont_insert()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Insert the node in the table of contentes list in utl context.
*
**************************************************************************/

static int utl_tableofcont_insert (
  utl_ctx	utlctx,
  pwr_sAttrRef	*arp,
  int		segments,
  char		*marginstr,
  int		page
)
{
	int		size, sts;
	utl_t_contlist	*list_ptr;
	pwr_tAName     	hier_name;
	char		*np;
	
	/* Get object name */
	sts = ldh_AttrRefToName( utlctx->ldhses,
		arp, ldh_eName_Aref,
		&np, &size);
	if ( EVEN(sts)) return sts;
	strcpy( hier_name, np);

	if ( segments != 0)
	  utl_cut_segments( hier_name, hier_name, segments);

	if ( utlctx->contlist)
	{
	  list_ptr = utlctx->contlist;
	  while( list_ptr->next)
	    list_ptr = list_ptr->next;

	  list_ptr->next = (utl_t_contlist *) calloc( 1, sizeof(utl_t_contlist));
	  if ( list_ptr->next == 0)
	    return FOE__NOMEMORY;
	  list_ptr = list_ptr->next;
	}
	else
	{
	  list_ptr = (utl_t_contlist *) calloc( 1, sizeof(utl_t_contlist));
	  if ( list_ptr == 0)
	    return FOE__NOMEMORY;
	  utlctx->contlist = list_ptr;
	}
	utlctx->contcount++;
	list_ptr->page = page;
	strcpy( (list_ptr->text), marginstr);
	strcat( (list_ptr->text), hier_name);

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_tableofcont_print()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Print the table of contentes list in utl context.
*
**************************************************************************/

static int utl_tableofcont_print (
 utl_ctx	utlctx
)
{
	utl_t_contlist	*list_ptr;
	int		j;
	char		text[120];

	list_ptr = utlctx->contlist;
	while ( list_ptr)
	{
	  strcpy( text, list_ptr->text);
	  strcat( text, " .");
	  for ( j = strlen( list_ptr->text); j < 65; j++)
	  {
	    strcat( text, ".");
	  }
	  u_print( utlctx, "%s", text);
	  u_print( utlctx, "%3d", list_ptr->page);
	  u_row( utlctx);
	  list_ptr = list_ptr->next;
	}

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_ctxlist_insert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Backcall routine used to insert an object into a list.
*
**************************************************************************/

static int utl_ctxlist_insert (
  pwr_sAttrRef  *arp,
  utl_t_list	**list,
  int		*count,
  unsigned long	specification,
  int		check,
  int		dum
)
{
	utl_list_insert( list, count, arp, specification, check, dum);
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		u_pagebreak()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Prints a page break.
*
**************************************************************************/

static void	u_pagebreak(
    utl_ctx		utlctx
)
{
	int	tabs;

	if ( utlctx->landscape)
	  tabs = 8;
	else
	  tabs = 5;

	if ( ! utlctx->page_first)
	{
	  IF_OUT u_print( utlctx, utlctx->page_title);
	  IF_OUT u_row( utlctx);
	  utlctx->page_first = 1;
	}
	if ( utlctx->row >= utlctx->rows)
	{
	  IF_OUT u_print( utlctx, "\n");
	  IF_OUT u_posit( utlctx, tabs, 0);
	  IF_OUT u_print( utlctx, "- %d -\n\f", utlctx->page);
	  utlctx->page++;
	  utlctx->row = 0;
	  IF_OUT u_print( utlctx, utlctx->page_title);
	  IF_OUT u_row( utlctx);
	  IF_OUT u_row( utlctx);

	  /* Print column header if defined */
	  if ( utlctx->listbody != 0)
	    IF_OUT utl_list_print_columnheader( utlctx, utlctx->listbody);

	}
}


/*************************************************************************
*
* Name:		u_row()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Prints a new row.
*
**************************************************************************/

static void	u_row( 
    utl_ctx		utlctx
){
	u_print( utlctx, "\n");
	utlctx->row++;
}


/*************************************************************************
*
* Name:		u_pageend()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Prints a page break on last page
*
**************************************************************************/

static void	u_pageend(
    utl_ctx		utlctx
)
{
	int	tabs;

	if ( utlctx->landscape)
	  tabs = 8;
	else
	  tabs = 5;

	while ( utlctx->row < utlctx->rows)
	  IF_OUT u_row( utlctx);

	IF_OUT u_print( utlctx, "\n");
	IF_OUT u_posit( utlctx, tabs, 0);
	IF_OUT u_print( utlctx, "- %d -", utlctx->page);

	utlctx->page++;
	utlctx->row = 0;
}


/*************************************************************************
*
* Name:		u_force_pagebreak()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* utl_ctx	utlctx		I	utl context.
*
* Description:
*	Prints a page break
*
**************************************************************************/

static void	u_force_pagebreak(
    utl_ctx		utlctx
)
{
	while ( utlctx->row < utlctx->rows)
	{
	  u_row( utlctx);
	}

	u_pagebreak( utlctx);
}


/*************************************************************************
*
* Name:		u_header()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: Prints a header.
*
**************************************************************************/

static void	u_header( 
    utl_ctx		utlctx,
    ldh_tSesContext ldhses,
    char		*title
)
{
	char		systemname[80];
	char		systemgroup[80];
	char		timstr[32];
	int		tabs;

	time_AtoAscii(NULL, time_eFormat_DateAndTime, timstr, sizeof(timstr));         
        timstr[strlen(timstr) - 6] = '\0'; /* Cut seconds */

		/* Get the system name */
	utl_get_systemname( systemname, systemgroup);

	if ( utlctx->landscape)
	  tabs = 8;
	else
	  tabs = 4;

	u_row( utlctx);
	u_row( utlctx);
	u_print( utlctx, "%s", title);
	u_posit( utlctx, tabs, strlen(title));
	u_print( utlctx, "%s", timstr);
	u_row( utlctx);
	u_posit( utlctx, tabs, 0);
	u_print( utlctx, "System");
	u_posit( utlctx, 2, 6);
	u_print( utlctx, " %s", systemname);	
	u_row( utlctx);

}

/*************************************************************************
*
* Name:		u_subheader()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 
*
**************************************************************************/

static void	u_subheader(
    utl_ctx		utlctx,
    char		*title,
    char		*spec
)
{
	int	tabs;
	int	max_spec;
	char	str[50];
	int	offs;
	char	*s;
	int	row;

	if ( utlctx->landscape)
	{
	  tabs = 8;
	  max_spec = 46;
	}
	else
	{
	  tabs = 4;
	  max_spec = 31;
	}

	IF_OUT u_posit( utlctx, tabs, 0);
	if ( title != 0)
	  IF_OUT u_print( utlctx, "%s", title);
	if ( spec != 0)
	{
	  row = 0;
	  offs = 0;
	  while ( offs < (int)strlen(spec))
	  {
	    strncpy( str, spec + offs, max_spec);
	    str[max_spec] = 0;
	    if ( strlen(str) < strlen( spec + offs))
	    {
	      s = strrchr( str, ',');
	      if ( s != 0)
	      {
	        s++;
	        *s = 0;
	      }	
	    }
	    if ( row > 0)
	      IF_OUT u_posit( utlctx, tabs, 0);
	    IF_OUT u_posit( utlctx, 2, strlen( title));
	    IF_OUT u_print( utlctx, " %s", str);
	    IF_OUT u_row( utlctx);
	    row++;
	    offs += strlen( str);
	    while( *(spec + offs) == ' ')
	      offs++;
	  }
	}
}



/*************************************************************************
*
* Name:		u_posit()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 
*
**************************************************************************/

static void	u_posit(
    utl_ctx		utlctx,
    int		tabs,
    int		len
)
{
	int 	j;
	int	nr_of_tabs;

	if ( 8 * tabs <= len) 
	  return;

/* 	nr_of_tabs = ( 8 * tabs - len - 1 ) / 8; 
	for ( j = 0; j < nr_of_tabs; j++ )
	  u_print( utlctx, "	");
*/
 	nr_of_tabs = ( 8 * tabs - len - 1) / 8; 
	for ( j = 0; j < nr_of_tabs + 1; j++ )
	  u_print( utlctx, "	");
}

/*************************************************************************
*
* Name:		u_open()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 
*
**************************************************************************/

static int	u_open(
    utl_ctx		utlctx,
    char		*filename,
    int			terminal,
    int			append
)
{
	pwr_tFileName fname;

	/* Open file */
	if ( *filename == '\0')
	  utlctx->output_file = NULL;
	else
	{
	  dcli_translate_filename( fname, filename);
	  if ( append)
	    utlctx->output_file = fopen( fname,"a");
	  else
	    utlctx->output_file = fopen( fname,"w");
	  if ( utlctx->output_file == 0) return FOE__NOFILE;
	}

	utlctx->terminal = terminal;
	utlctx->row = 0;
	utlctx->page = 1;

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		u_close()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 
*
**************************************************************************/

static void	u_close(
    utl_ctx		utlctx
)
{
	IF_OUT fclose( utlctx->output_file);
}


/*************************************************************************
*
* Name:		u_print()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 
*
**************************************************************************/

static void	u_print( 
    utl_ctx		utlctx,
    char		*format,
    ...
)
{
	va_list ap; 
		    
	va_start (ap, format);
	IF_OUT vfprintf( utlctx->output_file, format, ap);
	IF_TER vprintf( format, ap);
	va_end (ap);
}

/*************************************************************************
*
* Name:		utl_show_node()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: Prints all objects of class $Node found in the database.
*
**************************************************************************/

int utl_show_node (
  ldh_tSesContext ldhses,
  int		terminal,
  char		*filename
)
{
	utl_ctx	utlctx;
	int		sts, size;
	int		i;
	unsigned long	rtnode_count;
	pwr_tObjid	*rtnodelist;
	pwr_tObjid	*rtnodelist_ptr;
	pwr_tOName     	hier_name;

	  /* Open file */
	  utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	  sts = u_open( utlctx, filename, terminal, 0);
	  if ( EVEN(sts)) 
	  {
	    utl_ctx_delete( utlctx);
	    return sts;
	  }

	  IF_OUT u_header( utlctx, ldhses, "LIST OF NODES");
	  IF_OUT u_row( utlctx);

	  /* Show all nodes */
	  sts = trv_get_rtnodes( ldhses, &rtnode_count, &rtnodelist);
	  if ( EVEN(sts)) return sts;

	  rtnodelist_ptr = rtnodelist;
	  for ( i = 0; i < (int)rtnode_count; i++)
	  {
	    /* Get the name of the connected object */
	    sts = ldh_ObjidToName( 
		ldhses,
		*rtnodelist_ptr, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);

	    IF_OUT u_pagebreak( utlctx);
	    u_print( utlctx,  "  %s", hier_name);
	    u_row ( utlctx);
	    rtnodelist_ptr++;
	  }
	  IF_OUT u_pageend( utlctx);
	  if ( rtnode_count > 0) free((char *) rtnodelist);
	  if ( rtnode_count == 0)
	    printf("No nodes found\n");

	  u_close( utlctx);
	  utl_ctx_delete( utlctx);

	  if ( rtnode_count == 0)
	    return FOE__NONODES;

	  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_show_plcpgm()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: 	Prints all Plcpgm objects in a volume.
*
**************************************************************************/

int utl_show_plcpgm (
  ldh_tSesContext ldhses,
  int		terminal,
  char		*filename
)
{
	utl_ctx		utlctx;
	int		sts, size;
	int		j;
	unsigned long	plc_count;
	pwr_tObjid	*plclist;
	pwr_tObjid	*plclist_ptr;
	pwr_tOName     	hier_name;	
	float		*scantime_ptr;


	  /* Open file */
	  utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	  sts = u_open( utlctx, filename, terminal, 0);
	  if ( EVEN(sts)) 
	  {
	    utl_ctx_delete( utlctx);
	    return sts;
	  }

	  IF_OUT u_header( utlctx, ldhses, "LIST OF PLCPGM");
	  IF_OUT u_row ( utlctx);

	  IF_OUT u_pagebreak( utlctx);

	  /* Get the plcprograms for this node */
	  sts = trv_get_plcpgms( ldhses, &plc_count, &plclist);
	  if ( EVEN(sts)) return sts;
	    
	  plclist_ptr = plclist;
	  for ( j = 0; j < (int)plc_count; j++)
	  {
	    /* Get the name of the connected object */
	    sts = ldh_ObjidToName( 
			ldhses,
			*plclist_ptr, ldh_eName_Hierarchy,
			hier_name, sizeof( hier_name), &size);
	    /* Get the scantime */
	    sts = ldh_GetObjectPar( ldhses, *plclist_ptr, "DevBody", 
			"ScanTime", (char **)&scantime_ptr, &size);
	    if ( EVEN(sts)) return sts;

	    IF_OUT u_pagebreak( utlctx);
	    u_print( utlctx,  "  %s", hier_name);
	    u_posit( utlctx, 7, strlen(hier_name) + 2);
	
	    u_print( utlctx,  " %3.2f s", *scantime_ptr); 
	    u_row( utlctx);

	    free((char *) scantime_ptr);
	    
	    plclist_ptr++;
	  }
	  if ( plc_count > 0) free((char *) plclist);

	  IF_OUT u_pageend( utlctx);

	  u_close( utlctx);
	  utl_ctx_delete( utlctx);

	  return FOE__SUCCESS;
}



/*************************************************************************
*
* Name:		utl_show_window()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	plcstring	I	Name of Plcpgm object
*
* Description: 	Prints all windows found in a plcpgm.
*
**************************************************************************/

int utl_show_window (
  ldh_tSesContext ldhses,
  char		*plcstring,
  int		terminal,
  char		*filename
)
{
	utl_ctx	utlctx;
	int		sts, size;
	int		i;
	unsigned long	wind_count;
	pwr_tObjid	*windlist;
	pwr_tObjid	*windlist_ptr;
	pwr_tOName     	hier_name;	
	pwr_tObjid	plc;
	pwr_tObjid	window;

	  /* Get objdid for the plcpgm */
	  sts = ldh_NameToObjid( ldhses, &plc, plcstring);
	  if ( EVEN(sts))
	  {
	    return FOE__PLCNOTFOUND;
	  }
 
	  /* Open file */
	  utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	  sts = u_open( utlctx, filename, terminal, 0);
	  if ( EVEN(sts)) 
	  {
	    utl_ctx_delete( utlctx);
	    return sts;
	  }

	  IF_OUT u_header( utlctx, ldhses, "LIST OF WINDOWS");
	  IF_OUT u_row( utlctx);

	  /* Print the plc */
	  u_print( utlctx,  "Plcpgm  %s", plcstring);
	  u_row( utlctx);

	  /* Get the windows */
	  sts = trv_get_plc_window( ldhses, plc, &window);
	  if ( EVEN(sts)) return sts;

	  sts = trv_get_window_windows( ldhses, window, &wind_count, &windlist);
	  if ( EVEN(sts)) return sts;
	  
	  windlist_ptr = windlist;
	  for ( i = 0; i < (int) wind_count; i++)
	  {
	    IF_OUT u_pagebreak( utlctx);
	    /* Get the name of the object */
	    sts = ldh_ObjidToName( 
		ldhses,
		*windlist_ptr, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);
	    u_print( utlctx,  "  %s", hier_name);
	    u_row( utlctx);
	    windlist_ptr++;
	  }
	  IF_OUT u_pageend( utlctx);
	  if ( wind_count > 0) free((char *) windlist);

	  u_close( utlctx);
	  utl_ctx_delete( utlctx);

	  if ( wind_count == 0)
	    return FOE__NOWIND;
	  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_show_modules()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	objdid		I	Objdid of the window or plcpgm
* char *	name		I	Name of the window or plcpgm
*
* Description: show modules from the library pwrp_lib:ra_plc.olb_eln.
* If name is send should only the modules with that name.
*
**************************************************************************/

int utl_show_modules (
  ldh_tSesContext ldhses,
  char 		*objdidstr,
  char 		*name,
  char 		*hiername,
  int		terminal,
  char		*filename
)
{
	pwr_tTime	comp_time;
	pwr_tTime	mod_time;
	pwr_tObjid	objdid;
	pwr_tOName     	hier_name;
	char		*s;
	pwr_tClassId	class_vect[5];
	int		single_object = 0;
	int		sts, size;
	pwr_tClassId	*classp;
	pwr_tObjid	hierobjdid;
	utl_ctx		utlctx;
	utl_t_list	*list_ptr;
	pwr_tTime	*mod_time_ptr;
	static char	mod_time_str[64];
	pwr_tTime	*comp_time_ptr;
	static char	comp_time_str[64];
	int		modification;
        pwr_mOpSys  	os;
	pwr_sAttrRef 	aref;
	char		title[] =
"   Object				          Saved               Compiled";


	if ( objdidstr != 0 )
	{
	  sts = vldh_StrToId( objdidstr, &objdid);
	  if ( EVEN(sts)) return sts;

          /* Get the name of the object */
          sts = ldh_ObjidToName( ldhses, objdid, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);
	  if ( EVEN(sts)) return FOE__OBJECT;
	  name = hier_name;
	}

	class_vect[0] = pwr_cClass_windowplc;
	class_vect[1] = pwr_cClass_windowcond;
	class_vect[2] = pwr_cClass_windoworderact;
	class_vect[3] = pwr_cClass_windowsubstep;
/*	class_vect[4] = pwr_cClass_plc;*/
	class_vect[4] = 0;
	classp = class_vect;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
	  if ( EVEN(sts))
	  {
	    return FOE__HIERNAME;
	  }	
	}
	else
	  hierobjdid = pwr_cNObjid;

	utl_ctx_new( &utlctx, ldhses, title, UTL_LANDSCAPE);

	/* Check if name */
	if ( name != NULL )
	{
	  /* Check that name includes a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Print this object */
	    /* Get objdid for the object */
	    sts = ldh_NameToObjid( ldhses, &objdid, name);
	    if ( EVEN(sts))
	    {
	      return FOE__OBJECT;
	    }
	    aref = cdh_ObjidToAref( objdid);
	    utl_ctxlist_insert( &aref,  
		&(utlctx->list[0]), &(utlctx->listcount[0]), 0, 0, 0);
	    if ( EVEN(sts)) return sts;
	    single_object = 1;
	  }
	}

	if ( !single_object)
	{
	  sts = trv_get_attrobjects( ldhses, hierobjdid, classp, name, 
				     trv_eDepth_Deep,
				     (trv_tBcFunc) utl_ctxlist_insert, 
		&(utlctx->list[0]), &(utlctx->listcount[0]), 0, 0, 0);
	  if ( EVEN (sts)) return sts;
	}
	utl_list_sort( &utlctx->list[0], utlctx->listcount[0], ldhses, 0);

	/* Open file */
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts)) 
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	IF_OUT u_header( utlctx, ldhses, "SHOW MODULES");
	if ( cdh_ObjidIsNotNull( hierobjdid))
	  IF_OUT u_subheader( utlctx, "Hierarchy", hiername);
	if ( name != NULL )
	  IF_OUT u_subheader( utlctx, "Name", name);
	IF_OUT u_row( utlctx);

	IF_OUT u_pagebreak( utlctx);
	if ( !utlctx->output_file) u_print( utlctx, utlctx->page_title);
	if ( !utlctx->output_file) u_row( utlctx);

	list_ptr = utlctx->list[0];
	while ( list_ptr)
	{
	  objdid = list_ptr->o.Objid;

          /* Get the name of the object */
          sts = ldh_ObjidToName( ldhses, objdid, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);
	  if ( EVEN(sts)) return sts;

          sts = gcg_wind_to_operating_system( ldhses, objdid, &os);

	  /* Get modification time in parameter Modified */
	  sts = ldh_GetObjectPar( ldhses, objdid, "DevBody",   
			"Compiled", (char **)&comp_time_ptr, &size); 
	  if ( ODD(sts))
	  {
	    memcpy( &comp_time, comp_time_ptr, sizeof( comp_time));
	    /* Convert to ascii */
	    sts = time_AtoAscii(comp_time_ptr, time_eFormat_DateAndTime, 
                                     comp_time_str, sizeof(comp_time_str)); 
	    if ( EVEN(sts))
	      strcpy( comp_time_str, "-");
	    else
	    {
	      comp_time_str[20] = 0;
	      strcpy( &comp_time_str[7], &comp_time_str[9]);
	    }
	    free((char *) comp_time_ptr);
	  }
	  else
	  {
	    strcpy( comp_time_str, "-");
	  }

	  /* Get modification time in parameter Modified */
	  sts = ldh_GetObjectPar( ldhses, objdid, "DevBody",   
			"Modified", (char **)&mod_time_ptr, &size); 
	  if ( ODD(sts))
	  {
	    memcpy( &mod_time, mod_time_ptr, sizeof( mod_time));
	    /* Convert to ascii */
	    sts = time_AtoAscii(mod_time_ptr, time_eFormat_DateAndTime, 
                                     mod_time_str, sizeof(mod_time_str)); 
	    if ( EVEN(sts))
	      strcpy( mod_time_str, "-");
	    else
	    {
	      mod_time_str[20] = 0;
	      strcpy( &mod_time_str[7], &mod_time_str[9]);
	    }
	    free((char *) mod_time_ptr);
	  }
	  else
	  {
	    strcpy( mod_time_str, "-");
	  }

	  /* Check if modified after compiled */
	  if (time_Acomp(&mod_time, &comp_time) > 0)
	    modification = 1;
	  else
	    modification = 0;

	  IF_OUT u_pagebreak( utlctx);
	  if ( modification)
	    u_print( utlctx,  "-> %s", hier_name);
          else
	    u_print( utlctx,  "   %s", hier_name);
	  u_posit( utlctx, 6, strlen(hier_name) + 3);
	  u_print( utlctx,  "  %s", mod_time_str);
	  u_print( utlctx,  "  %s", comp_time_str);
	  u_row( utlctx);

	  list_ptr = list_ptr->next;
	}
	IF_OUT u_pageend( utlctx);

	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_show_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	windowstring	I	Name of the window or plcpgm
* unsigned long	sendobjdid	I	Objdid of the window.
*
* Description: 	Print all objects ( not connectionsobjects) 
*		found in a window. Either the
*		name of the window or the objdid can be sent as input.
*
**************************************************************************/

int utl_show_object (
  ldh_tSesContext ldhses,
  char		*windowstring,
  pwr_tObjid	sendobjdid,
  int		terminal,
  char		*filename
)
{
	utl_ctx	utlctx;
	int		sts, size;
	int		i;
	pwr_tClassId	cid;
	pwr_tOName     	hier_name;	
	pwr_tOName     	class_hier_name;	
	pwr_tObjid	window;
	unsigned long	object_count;
	pwr_tObjid	*objectlist;
	pwr_tObjid	*objectlist_ptr;
	char		objidstr[80];


  if ( cdh_ObjidIsNull( sendobjdid))
  {
    /* Get objdid for the plcpgm */
    sts = ldh_NameToObjid( ldhses, &window, windowstring);
    if ( EVEN(sts))
    {
      return FOE__WINDNOTFOUND;
    }
   
    /* Open file */
    utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
    sts = u_open( utlctx, filename, terminal, 0);
    if ( EVEN(sts)) 
    {
      utl_ctx_delete( utlctx);
      return sts;
    }

    IF_OUT u_header( utlctx, ldhses, "LIST OF OBJECTS");
    IF_OUT u_row( utlctx);

    /* Print the window */
    u_print( utlctx,  "Window  %s", windowstring);
    u_row( utlctx);

    sts = trv_get_window_objects( ldhses, window, &object_count, &objectlist);
    if ( EVEN(sts)) return sts;
	  
    objectlist_ptr = objectlist;

  }
  else
  {
    /* an objdid has been send */
    object_count = 1;
    objectlist_ptr = &sendobjdid;
  }

    for ( i = 0; i < (int) object_count; i++ , objectlist_ptr++ )
    {
      /* Get the name of the object */
      sts = ldh_ObjidToName( 
   		ldhses,
		*objectlist_ptr, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);
      if ( EVEN(sts)) return sts;

      /* get the object class */
      sts = ldh_GetObjectClass( ldhses, *objectlist_ptr,  &cid);
      if ( EVEN(sts)) return sts;

      /* Get the name of the class object */
      sts = ldh_ObjidToName( 
   		ldhses,
		cdh_ClassIdToObjid( cid),ldh_eName_Object,
		class_hier_name, sizeof( class_hier_name), &size);
      if ( EVEN(sts)) return sts;
      sts = ldh_ObjidToName( 
   		ldhses,
		*objectlist_ptr, ldh_eName_Object,
		objidstr, sizeof(objidstr), &size);
      if ( EVEN(sts)) return sts;

      IF_OUT u_pagebreak( utlctx);
      u_print( utlctx,  "  %s", hier_name);
      u_posit( utlctx, 7, strlen(hier_name) + 2);
      u_print( utlctx,  " %s   %s", objidstr, class_hier_name);
      u_row( utlctx);
     }
     if ( cdh_ObjidIsNull( sendobjdid))
     { 
       if ( object_count > 0) free((char *) objectlist);
     }

  IF_OUT u_pageend( utlctx);
  u_close( utlctx);
  utl_ctx_delete( utlctx);

  if ( object_count == 0)
     return FOE__NOOBJFOUND;

  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_set_object_parameter()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	hiername	I	Name of a object in the hierarchy.
* char *	class		I	Name of the class.
*
*
* Description: 	Prints all objects of a specified class that is found
*		below a specific object in the hierarchy.
*
**************************************************************************/

int utl_revert ( 
  ldh_tSesContext ldhses,
  int		confirm
)
{
	static char	yes_or_no[200];	
	int		sts;
#if defined OS_VMS
	$DESCRIPTOR ( yon_prompt_desc , "(Y/N: " );
	static $DESCRIPTOR ( yon_desc , yes_or_no );
  	short 		len ;
#endif

	if ( confirm )	
	{
#if defined OS_VMS
	  printf("Do you really want to revert ");
 	  sts = lib$get_input ( &yon_desc , &yon_prompt_desc , &len ) ; 
	  if ( EVEN(sts)) return sts;
#else
	  // TODO
	  printf( "(Y/N: ");
	  sts = scanf( "%s", yes_or_no);
#endif

	  if ( !((yes_or_no[0] == 'Y') || (yes_or_no[0] == 'y')))
            return FOE__SUCCESS;
	}

	sts = ldh_RevertSession( ldhses);
	return sts;
}


/*************************************************************************
*
* Name:		utl_set_object_parameter()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	hiername	I	Name of a object in the hierarchy.
* char *	class		I	Name of the class.
*
*
* Description: 	Prints all objects of a specified class that is found
*		below a specific object in the hierarchy.
*
**************************************************************************/

int utl_set_object_parameter ( 
  ldh_tSesContext ldhses,
  char		*hiername,
  char		*classname,
  char		*name,
  char		*parameter,
  char		*valuestr,
  int		terminal,
  char		*filename,
  int		confirm,
  int		log
)
{
	utl_ctx	utlctx;
	int		sts, i;
	pwr_tClassId	*classp;
	pwr_tObjid	hierobjdid;
	char		*s;
	pwr_tObjid  	objdid;
	char		*t;
	char		elementstr[20];
	int		element;
	int		len;
	char		class_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tClassId	class_vect[UTL_INPUTLIST_MAX + 1];
	int		nr;
	pwr_sAttrRef	aref;

	if ( name != NULL )
	{
	  if ( (strchr( name, '*') != 0) && (classname == NULL) &&
		(parameter != NULL))
	  {
	    /* Wildcard and no class is not allowed for show parameter */
	    return FOE__CLASSQUAL;
	  }
	}

	/* Check if class */
	if ( classname != NULL )
	{
	  nr = utl_parse( classname, ", ", "", (char *)class_str, 
		sizeof( class_str) / sizeof( class_str[0]), sizeof( class_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
            return FOE__CLASSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    sts = ldh_ClassNameToId( ldhses, &class_vect[i], class_str[i]);
	    if ( EVEN(sts))
	    {
	      return FOE__CLASSNAME;
	    }
	  }
	  class_vect[nr] = 0;
	  classp = class_vect;
	}	
	else
	  classp = 0;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
	  if ( EVEN(sts))
	  {
	    return FOE__HIERNAME;
	  }	
	}
	else
	  hierobjdid = pwr_cNObjid;

	/* Check index in parameter */
	s = strchr( parameter, '[');
	if ( s == 0)
	  element = UTL_NOELEMENT;
	else
	{
	  t = strchr( parameter, ']');
	  if ( t == 0)
	  {
	    return FOE__PARSYNT;
	  }
	  else
	  {
	    len = t - s - 1;
	    strncpy( elementstr, s + 1, len);
	    elementstr[ len ] = 0;
	    sscanf( elementstr, "%d", &element);
	    *s = '\0';
	    if ( (element < 0) || (element > 10000) )
	    {
	      return FOE__PARELSYNT;
	    }
	  }
	}
	
	/* Check if name */
	if ( name != NULL )
	{
	  /* Check that name includes a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Print this object */
	    /* Get objdid for the object */
	    sts = ldh_NameToObjid( ldhses, &objdid, name);
	    if ( EVEN(sts))
	    {
	      return FOE__OBJECT;
	    }
	    utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	    utlctx->confirm = confirm;
	    utlctx->log = log;
	    sts = u_open( utlctx, filename, terminal, 0);
	    if ( EVEN(sts)) 
	    {
	      utl_ctx_delete( utlctx);
	      return sts;
	    }

	    IF_OUT u_header( utlctx, ldhses, "SET PARAMETER");
	    IF_OUT u_subheader( utlctx, "Name", name);
	    IF_OUT u_row( utlctx);

	    aref = cdh_ObjidToAref( objdid);
	    sts = utl_set_parameter( &aref, ldhses, parameter, 
				     valuestr, element, 
				     utlctx);
	    if ( sts != FOE__ABORTSEARCH)
	      if (EVEN(sts)) return sts;

	    u_close( utlctx);
	    utl_ctx_delete( utlctx);

	    return FOE__SUCCESS;
	  }
	}

	/* Open file */
	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	utlctx->confirm = confirm;
	utlctx->log = log;
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts)) 
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	IF_OUT u_header( utlctx, ldhses, "SET PARAMETER");
	if ( classp != 0)
	  IF_OUT u_subheader( utlctx, "Class", classname);
	if ( cdh_ObjidIsNotNull( hierobjdid))
	  IF_OUT u_subheader( utlctx, "Hierarchy", hiername);
	if ( name != NULL )
	  IF_OUT u_subheader( utlctx, "Name", name);
	IF_OUT u_row( utlctx);

	sts = trv_get_objects_hier_class_name( ldhses, hierobjdid, classp, name, 
		(trv_tBcFunc) utl_set_parameter, ldhses, parameter, valuestr, 
		(void *)element, utlctx);
	if ( sts != FOE__ABORTSEARCH)
	  if (EVEN(sts)) return sts;

	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_show_objects_hier_class_name()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	hiername	I	Name of a object in the hierarchy.
* char *	class		I	Name of the class.
*
*
* Description: 	Prints all objects of a specified class that is found
*		below a specific object in the hierarchy.
*
**************************************************************************/

int utl_show_obj_hier_class_name (
  ldh_tSesContext ldhses,
  char		*hiername,
  char		*classname,
  char		*name,
  char		*parameter,
  char		*volume,
  int		terminal,
  char		*filename,
  int		full,
  int		allvolumes,
  int		append,
  int 		exactorder
)
{
	utl_ctx	utlctx;
	int		sts, i;
	pwr_tClassId	*classp;
	pwr_tObjid	hierobjdid;
	char		*s;
	pwr_tObjid  	objdid;
	char		*t;
	char		elementstr[20];
	int		element[10];
	int		len;
	utl_t_list	*list_ptr;
	char		par_str[UTL_INPUTLIST_MAX + 1][80];
	char		vol_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tVolumeId	volume_vect[UTL_INPUTLIST_MAX + 1];
	pwr_tVolumeId	*volume_p;
	char		class_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tClassId	class_vect[UTL_INPUTLIST_MAX + 1];
	int		nr;
	trv_tCtx       	trvctx;
	pwr_tClassId	vol_class;
	pwr_tVolumeId	vol_id;
	pwr_sAttrRef	aref;
	char		page_title[] = 
	"  Object							 Class";

	if ( name != NULL )
	{
	  if ( (strchr( name, '*') != 0) && (classname == NULL) &&
		(parameter != NULL))
	  {
	    /* Wildcard and no class is not allowed for show parameter */
	    return FOE__CLASSQUAL;
	  }
	}

	/* Check if class */
	if ( classname != NULL )
	{
	  nr = utl_parse( classname, ", ", "", (char *)class_str, 
		sizeof( class_str) / sizeof( class_str[0]), sizeof( class_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
	    return FOE__CLASSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    sts = ldh_ClassNameToId( ldhses, &class_vect[i], class_str[i]);
	    if ( EVEN(sts))
	    {
	      return FOE__CLASSNAME;
	    }
	  }
	  class_vect[nr] = 0;
	  classp = class_vect;
	}	
	else
	  classp = 0;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
	  if ( EVEN(sts))
	  {
	    return FOE__HIERNAME;
	  }	
	}
	else
	  hierobjdid = pwr_cNObjid;

	if ( parameter != NULL )
	{
	  nr = utl_parse( parameter, ", ", "", (char *)par_str, 
		sizeof( par_str) / sizeof( par_str[0]), sizeof( par_str[0]));
	  par_str[nr][0] = 0; 
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
	    return FOE__PARSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    /* Check index in parameter */
	    s = strchr( par_str[i], '[');
	    if ( s == 0)
	      element[i] = UTL_NOELEMENT;
	    else
	    {
	      t = strchr( par_str[i], ']');
	      if ( t == 0)
	      {
	        return FOE__PARSYNT;
	      }
	      else
	      {
	        len = t - s - 1;
	        strncpy( elementstr, s + 1, len);
	        elementstr[ len ] = 0;
	        sscanf( elementstr, "%d", &element[i]);
	        *s = '\0';
	        if ( (element[i] < 0) || (element[i] > 100) )
	        {
	          return FOE__PARELSYNT;
	        }
	      }
	    }
	  }
	}

	if ( volume != NULL)
	{
	  /* Parse the volumestr */
	  nr = utl_parse( volume, ", ", "", (char *)vol_str, 
		sizeof( vol_str) / sizeof( vol_str[0]), sizeof( vol_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
	    return FOE__PARSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    sts = ldh_VolumeNameToId( ldh_SessionToWB( ldhses), vol_str[i],
			&volume_vect[i]);
	    if ( EVEN(sts)) return sts;
	  }
	  volume_vect[nr] = 0;
	  volume_p = volume_vect;
	}
	else
	  volume_p = 0;
	
	if ( allvolumes)
	{
	  /* Get all volumes that is not class and wb volumes */
	  i = 0;
	  sts = ldh_GetVolumeList( ldh_SessionToWB( ldhses), &vol_id);
	  while ( ODD(sts) )
	  {
	    sts = ldh_GetVolumeClass( ldh_SessionToWB( ldhses), vol_id,
			&vol_class);
	    if (EVEN(sts)) return sts;

	    if ( !(vol_class == pwr_eClass_ClassVolume ||
 	        vol_class == pwr_eClass_WorkBenchVolume ))
	    {
	      volume_vect[i] = vol_id;
	      i++;
	      if ( i > UTL_INPUTLIST_MAX)
	        return FOE__PARSYNT;
	    }
	    sts = ldh_GetNextVolume( ldh_SessionToWB( ldhses), vol_id, &vol_id);
	  }
	  volume_vect[i] = 0;
	  volume_p = volume_vect;
	}

	/* Check if name */
	if ( name != NULL )
	{
	  /* Check that name includes a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Print this object */
	    /* Get objdid for the object */
	    sts = ldh_NameToObjid( ldhses, &objdid, name);
	    if ( EVEN(sts))
	    {
	      return FOE__OBJECT;
	    }
	    aref = cdh_ObjidToAref( objdid);
	    utl_ctx_new( &utlctx, ldhses, page_title, UTL_PORTRAIT);
	    sts = u_open( utlctx, filename, terminal, append);
	    if ( EVEN(sts)) 
	    {
	      utl_ctx_delete( utlctx);
	      return sts;
	    }

	    IF_OUT u_header( utlctx, ldhses, "LIST OF OBJECT");
	    IF_OUT u_subheader( utlctx, "Name", name);
	    if ( parameter != NULL )
	      IF_OUT u_subheader( utlctx, "Parameter", parameter);
	    if ( full ) 
	      IF_OUT u_subheader( utlctx, "Full", 0);
	    IF_OUT u_row( utlctx);

	    if ( parameter == NULL)
	      utl_print_object( objdid, ldhses, utlctx, full, 0, 0); 
	    else
	      utl_print_object( objdid, ldhses, utlctx, full, (char *)par_str, 
			element);
	    u_close( utlctx);
	    utl_ctx_delete( utlctx);

	    return FOE__SUCCESS;
	  }
	}

	/* Create context and search for objects */
	utl_ctx_new( &utlctx, ldhses, page_title, UTL_PORTRAIT);

	sts = trv_create_ctx( &trvctx, ldhses, hierobjdid, classp, name,
		volume_p);
	if ( EVEN(sts)) return sts;

	sts = trv_object_search( trvctx,
		(trv_tBcFunc) utl_ctxlist_insert,
		&(utlctx->list[0]), &(utlctx->listcount[0]), 0, 0, 0);
	if ( EVEN (sts)) return sts;

	sts = trv_delete_ctx( trvctx);

	if ( !exactorder)
	  utl_list_sort( &utlctx->list[0], utlctx->listcount[0], ldhses, 0);

	/* Open file */
	sts = u_open( utlctx, filename, terminal, append);
	if ( EVEN(sts)) 
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	IF_OUT u_header( utlctx, ldhses, "LIST OF OBJECTS");
	if ( classp != 0)
	  IF_OUT u_subheader( utlctx, "Class", classname);
	if ( cdh_ObjidIsNotNull( hierobjdid))
	  IF_OUT u_subheader( utlctx, "Hierarchy", hiername);
	if ( name != NULL )
	  IF_OUT u_subheader( utlctx, "Name", name);
	if ( parameter != NULL )
	  IF_OUT u_subheader( utlctx, "Parameter", parameter);
	if ( full )
	  IF_OUT u_subheader( utlctx, "Full", 0);
	IF_OUT u_row( utlctx);

	list_ptr = utlctx->list[0];
	while( list_ptr)
	{	
	  objdid = list_ptr->o.Objid;
	  if ( parameter == NULL)
	    utl_print_object( objdid, ldhses, utlctx, full, 0, 0); 
	  else
	    utl_print_object( objdid, ldhses, utlctx, full, (char *)par_str, 
		element);
	  list_ptr = list_ptr->next;
	}
	IF_OUT u_pageend( utlctx);

	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_create_volume()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char		*objdidstring 	I	objdidstring.
*
*
* Description: 	Create a volume.
*
**************************************************************************/

pwr_tStatus utl_create_volume( 
		ldh_tWBContext  ldhwb,
		char 	*volumename, 
		char	*volumeclass_str, 
		char	*volumeid_str)
{
	pwr_tStatus 	sts;
	pwr_tClassId	cid;
	ldh_tVolContext volctx;
	ldh_tSesContext	ldhses;
	pwr_tVolumeId	volid;

	utl_toupper( volumeclass_str, volumeclass_str);
	if ( !strcmp( volumeclass_str, "ROOTVOLUME") ||
	     !strcmp( volumeclass_str, "$ROOTVOLUME"))
	  cid = pwr_eClass_RootVolume;
	else if ( !strcmp( volumeclass_str, "SUBVOLUME") ||
	          !strcmp( volumeclass_str, "$SUBVOLUME"))
	  cid = pwr_eClass_SubVolume;
	else if ( !strcmp( volumeclass_str, "SHAREDVOLUME") ||
	          !strcmp( volumeclass_str, "$SHAREDVOLUME"))
	  cid = pwr_eClass_SharedVolume;
	else if ( !strcmp( volumeclass_str, "CLASSVOLUME") ||
	          !strcmp( volumeclass_str, "$CLASSVOLUME"))
	  cid = pwr_eClass_ClassVolume;
	else if ( !strcmp( volumeclass_str, "DIRECTORYVOLUME") ||
	          !strcmp( volumeclass_str, "$DIRECTORYVOLUME"))
	  cid = pwr_eClass_DirectoryVolume;
	else
	  return FOE__VOLUMECLASS;

	sts = cdh_StringToVolumeId ( volumeid_str, &volid);
	if ( EVEN(sts))
	  return FOE__VOLUMEID;

	sts = ldh_CreateVolume( ldhwb, &ldhses, volid, volumename,
			cid);
	if ( EVEN(sts)) return sts;

	volctx = ldh_SessionToVol( ldhses);

	/* Save and close the created ldh session */
	sts = ldh_SaveSession( ldhses);
	if ( EVEN(sts)) return sts;
 
	sts = ldh_CloseSession( ldhses);
	if ( EVEN(sts)) return sts;

	sts = ldh_DetachVolume( ldhwb, volctx);
	if ( EVEN(sts)) return sts;

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_show_volumes()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char		*objdidstring 	I	objdidstring.
*
*
* Description: 	Prints all objects of a specified class that is found
*		below a specific object in the hierarchy.
*
**************************************************************************/

pwr_tStatus	utl_show_volumes( 
  ldh_tSesContext	ldhses,
  int			allvolumes
)
{
	pwr_tStatus		sts;
	int			size;
	ldh_sVolumeInfo 	info;
	utl_ctx			utlctx;
	pwr_tVolumeId		vol_id;
	char			vol_name[32];
	char			class_name[32];
	pwr_tClassId		vol_class;

	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	sts = u_open( utlctx, "", 1, 0);

	/* Get all volumes that is not class and wb volumes */
	sts = ldh_GetVolumeList( ldh_SessionToWB( ldhses), &vol_id);
	while ( ODD(sts) )
	{
	  sts = ldh_GetVolumeClass( ldh_SessionToWB( ldhses), vol_id,
			&vol_class);
	  if (EVEN(sts)) return sts;

	  if ( vol_class == pwr_eClass_ClassVolume ||
 	        vol_class == pwr_eClass_WorkBenchVolume )
	  {
	    if ( !allvolumes)
	    {
	      sts = ldh_GetNextVolume( ldh_SessionToWB( ldhses), vol_id, &vol_id);
	      continue;
	    }
	  }
	  sts = ldh_VolumeIdToName( ldh_SessionToWB( ldhses), vol_id,
		vol_name, sizeof(vol_name), &size);
	  if ( EVEN(sts)) return sts;

	  sts = ldh_ClassIdToName( ldhses, vol_class, class_name, 
		sizeof(class_name), &size);
	  if ( EVEN(sts)) return sts;
	  utl_cut_segments( class_name, class_name, 1);

	  sts = ldh_GetVolumeInfo( ldh_SessionToVol(ldhses), &info);
	  if ( EVEN(sts)) return sts;

	  u_print( utlctx,  "  %s", vol_name);
	  u_posit( utlctx, 3, strlen(vol_name) + 2);
	  if ( vol_id == info.Volume)
	    u_print( utlctx ,"Attached");
	  else
	    u_print( utlctx, "        ");
	  sts = ldh_GetVidInfo( ldh_SessionToWB(ldhses), vol_id, &info);
	  if ( EVEN(sts)) return sts;

	  switch ( info.VolRep) {
	  case ldh_eVolRep_Db:
	    u_print( utlctx, " Db ");
	    break;
	  case ldh_eVolRep_Dbs:
	    u_print( utlctx, " Dbs");
	    break;
	  case ldh_eVolRep_Dbms:
	    u_print( utlctx, " Dbms");
	    break;
	  case ldh_eVolRep_Wbl:
	    u_print( utlctx, " Wbl");
	    break;
	  case ldh_eVolRep_Mem:
	    u_print( utlctx, " Mem");
	    break;
	  case ldh_eVolRep_Ref:
	    u_print( utlctx, " Ref");
	    break;
	  case ldh_eVolRep_Ext:
	    u_print( utlctx, " Ext");
	    break;
	  }
	  u_print( utlctx, " %s", class_name);
	  u_posit( utlctx, 2, strlen(class_name) + 2);
	  u_print( utlctx, " %s", cdh_VolumeIdToString( NULL, vol_id, 0, 0));
	  u_row( utlctx);

	  sts = ldh_GetNextVolume( ldh_SessionToWB( ldhses), vol_id, &vol_id);
	}
	utl_ctx_delete( utlctx);
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_show_object_objdid()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char		*objdidstring 	I	objdidstring.
*
*
* Description: 	Prints all objects of a specified class that is found
*		below a specific object in the hierarchy.
*
**************************************************************************/

int utl_show_object_objdid (
  ldh_tSesContext ldhses,
  char		*objdidstring,
  int		terminal,
  char		*filename
)
{
	utl_ctx		utlctx;
	int		sts, size;
	pwr_tClassId	cid;
	pwr_tObjid  	objdid;
	char		objdidstr[80];

	sts = vldh_StrToId( objdidstring, &objdid);
	if ( EVEN(sts)) return sts;

	/* Check if the objdid exists */
	sts = ldh_GetObjectClass( ldhses, objdid,  &cid);
	if ( EVEN(sts))
	{
	  return FOE__OBJDID;
	}

	/* Print this object */
	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts)) 
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	sts = ldh_ObjidToName( ldhses, objdid, ldh_eName_Objid,
		objdidstr, sizeof( objdidstr), &size);
	if ( EVEN(sts)) return sts;

	IF_OUT u_header( utlctx, ldhses, "LIST OF OBJECT");
	IF_OUT u_subheader( utlctx, "Objid", objdidstr);
	IF_OUT u_row( utlctx);

	utl_print_object( objdid, ldhses, utlctx, 0, 0, 0);
	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_show_hierarchy()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: 	Prints all objects of class $Hierarchy found in
*		planthier or nodehier.
*
**************************************************************************/

int utl_show_hierarchy (
  ldh_tSesContext ldhses,
  int		terminal,
  char		*filename
)
{
	utl_ctx		utlctx;
	int		sts;
	pwr_tClassId	cid;

	/* Get objdid and class for the object */
	sts = ldh_ClassNameToId( ldhses, &cid, "$PlantHier");
	if ( EVEN(sts))
	{
	  return FOE__CLASSNAME;
	}	

	/* Open file */
	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts)) 
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	IF_OUT u_header( utlctx, ldhses, "LIST OF HIERARCHY");
	IF_OUT u_row( utlctx);

	sts = trv_get_objects_class( ldhses, cid, 
		(trv_tBcFunc) utl_print_aref, ldhses, utlctx, 0, 0, 0);
	if ( EVEN (sts)) return sts;

	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_show_class_classhier()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	classhiername	I	Name of the $ClassHier object.
*
*
* Description: 	Prints all objects of class $ClassDef found below
* 		the specified $ClassHier object. That is prints all
*		classes in BaseClasses, SystemClasses or MpsClasses etc.
*
**************************************************************************/

pwr_tStatus utl_show_class_classhier (
  ldh_tSesContext ldhses,
  char		*hiername,
  char		*name,
  int		terminal,
  char		*filename,
  int		full,
  int		contents,
  int		all
)
{
	utl_ctx	utlctx;
	pwr_tStatus	sts;
	pwr_tObjid	hierobjdid;
	char		*s;
	pwr_tObjid  objdid;
	pwr_tClassId	classclass[2] = { 0, 0};
	pwr_tClassId	*classclass_ptr;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
	  if ( EVEN(sts))
	  {
	    return FOE__HIERNAME;
	  }	
	}
	else
	{
	  if ( name == NULL)
	  {
	    /* Not hier and not name, take baseclasses as default */
	    sts = ldh_NameToObjid( ldhses, &hierobjdid, "pwrb:Class");
	    if ( EVEN(sts)) return sts;
	  }
	  else
	    hierobjdid = pwr_cNObjid;
	}

	if ( all )
	  classclass_ptr = NULL;
	else
	{
	  classclass[0] = pwr_eClass_ClassDef; 
	  classclass_ptr = classclass;
	}

	/* Check if name */
	if ( name != NULL )
	{
	  /* Check that name includes a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Print this object */
	    /* Get objdid for the object */
	    sts = ldh_NameToObjid( ldhses, &objdid, name);
	    if ( EVEN(sts))
	    {
	      return FOE__OBJECT;
	    }
	    utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	    sts = u_open( utlctx, filename, terminal, 0);
	    if ( EVEN(sts)) 
	    {
	      utl_ctx_delete( utlctx);
	      return sts;
	    }

	    IF_OUT u_header( utlctx, ldhses, "LIST OF CLASSES");
	    IF_OUT u_subheader( utlctx, "Name", name);
	    IF_OUT u_row( utlctx);

	    if ( contents || all)
	      utl_print_object( objdid, ldhses, utlctx, contents, 0, 0);
	    else {
	      pwr_sAttrRef aref = cdh_ObjidToAref( objdid);
	      utl_print_class( &aref, ldhses, utlctx, full, 0, 0);
	    }
	    u_close( utlctx);
	    utl_ctx_delete( utlctx);

	    return FOE__SUCCESS;
	  }
	}

	/* Open file */
	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts)) 
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	IF_OUT u_header( utlctx, ldhses, "LIST OF CLASSES");
	if ( cdh_ObjidIsNotNull( hierobjdid))
	  IF_OUT u_subheader( utlctx, "Hierarchy", hiername);
	if ( name != NULL )
	  IF_OUT u_subheader( utlctx, "Name", name);
	IF_OUT u_row( utlctx);

	if ( contents || all )
	{
	  sts = trv_get_class_hier( ldhses, hierobjdid, name, classclass_ptr, 
		(trv_tBcFunc) utl_print_aref, ldhses, utlctx, (void *)contents, 0, 0);
	  if ( EVEN (sts)) return sts;
	}
	else
	{
	  sts = trv_get_class_hier( ldhses, hierobjdid, name, classclass_ptr,
		(trv_tBcFunc) utl_print_class, ldhses, utlctx, (void *)full, 0, 0);
	  if ( EVEN (sts)) return sts;
	}

	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_print_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	dum1		I	dummy parameter.
* unsigned long	dum2		I	dummy parameter.
* unsigned long	dum3		I	dummy parameter.
* unsigned long	dum4		I	dummy parameter.
*
*
* Description: 	Backcall routine used by utl_show_... routines
*		to print information of an object.
*
**************************************************************************/

static int utl_print_object ( 
  pwr_tObjid  Objdid,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  int		full,
  char		*parameter,
  int		*element
)
{
	int		sts, size;
	pwr_tClassId	cid;
	pwr_tOName     	hier_name;
	char		class_name[80];
	char		objdid_str[40];
	pwr_sAttrRef	aref = cdh_ObjidToAref( Objdid);

	/* Print the name of the actual object */
	sts = ldh_ObjidToName( 
		ldhses, Objdid, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);
	if ( EVEN(sts)) return sts;

	sts = ldh_GetObjectClass( utlctx->ldhses, Objdid,  &cid);
	if ( EVEN(sts)) return sts;
	sts = ldh_ObjidToName( utlctx->ldhses, cdh_ClassIdToObjid( cid),
		ldh_eName_Object,
		class_name, sizeof( class_name), &size);
	if ( EVEN(sts)) return sts;

	IF_OUT u_pagebreak( utlctx);
	u_print( utlctx,  "  %s", hier_name);
	if ( full && ( parameter == NULL))
	{
          sts = ldh_ObjidToName( ldhses, Objdid, ldh_eName_Objid,
		objdid_str, sizeof( objdid_str), &size);
	  if ( EVEN(sts)) return sts;

	  u_posit( utlctx, 6, strlen(hier_name) + 2);
	  u_print( utlctx, " %s", objdid_str);
	  u_posit( utlctx, 1, strlen(objdid_str) + 1);
	  u_print( utlctx, " %s", class_name);
	}
	else
	{
	  u_posit( utlctx, 8, strlen(hier_name) + 2);
	  u_print( utlctx, " %s", class_name);
	}
	u_row( utlctx);

	if ( full && ( parameter == NULL))
	{
	  sts = utl_print_object_full( &aref, ldhses, utlctx, 0);
	  if ( EVEN(sts)) return sts;
	}
	if ( parameter != NULL)
	{
	  sts = utl_print_object_par( &aref, ldhses, utlctx, parameter, element);
	  if ( EVEN(sts)) return sts;
	}
	return FOE__SUCCESS;
}

static int utl_print_aref ( 
  pwr_sAttrRef  *arp,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  int		full,
  char		*parameter,
  int		*element
)
{
	int		sts, size;
	pwr_tClassId	cid;
	pwr_tAName     	hier_name;
	char		class_name[80];
	char		objdid_str[40];
	char		*np;

	/* Print the name of the actual object */
	sts = ldh_AttrRefToName( ldhses, arp, ldh_eName_Hierarchy,
				 &np, &size);
	if ( EVEN(sts)) return sts;
	strcpy( hier_name, np);

	sts = ldh_GetAttrRefTid( utlctx->ldhses, arp,  &cid);
	if ( EVEN(sts)) return sts;
	sts = ldh_ObjidToName( utlctx->ldhses, cdh_ClassIdToObjid( cid),
		ldh_eName_Object,
		class_name, sizeof( class_name), &size);
	if ( EVEN(sts)) return sts;

	IF_OUT u_pagebreak( utlctx);
	u_print( utlctx,  "  %s", hier_name);
	if ( full && ( parameter == NULL)) {
          sts = ldh_ObjidToName( ldhses, arp->Objid, ldh_eName_Objid,
		objdid_str, sizeof( objdid_str), &size);
	  if ( EVEN(sts)) return sts;

	  u_posit( utlctx, 6, strlen(hier_name) + 2);
	  u_print( utlctx, " %s", objdid_str);
	  u_posit( utlctx, 1, strlen(objdid_str) + 1);
	  u_print( utlctx, " %s", class_name);
	}
	else
	{
	  u_posit( utlctx, 8, strlen(hier_name) + 2);
	  u_print( utlctx, " %s", class_name);
	}
	u_row( utlctx);

	if ( full && ( parameter == NULL))
	{
	  sts = utl_print_object_full( arp, ldhses, utlctx, 0);
	  if ( EVEN(sts)) return sts;
	}
	if ( parameter != NULL)
	{
	  sts = utl_print_object_par( arp, ldhses, utlctx, parameter, element);
	  if ( EVEN(sts)) return sts;
	}
	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_object_changed()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  Objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	utlctx		I	output specification.
*
*
* Description: 	
*
**************************************************************************/

static int utl_object_changed ( 
  pwr_tObjid	Objdid,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  int		*changed,
  int		code
)
{
	int		sts, size, i, j;
	pwr_tClassId	cid;
	ldh_sParDef 	*bodydef;
	int		rows;
	pwr_tObjid	templ;
	char		body[20];	
	char		parname[32];
	char		name[32];
	char		*object_par;
	char		*template_par;
	char		*object_element;
	char		*template_element;
	int		parsize;
	int		elements;

	*changed = 0;

	/* Get the template object of this class */
	sts = ldh_GetObjectClass( ldhses, Objdid,  &cid);
	if ( EVEN(sts)) return sts;

	/* Get the first child to the object */
	sts = ldh_GetChild( ldhses, cdh_ClassIdToObjid( cid), &templ);
	while ( ODD(sts) )
	{
	  sts = ldh_ObjidToName( 
		ldhses, templ, ldh_eName_Object,
		name, sizeof( name), &size);
	  if ( EVEN(sts)) return sts;
	  
	  if ( strcmp( name, "Template") == 0)
	    break;
	  sts = ldh_GetNextSibling( ldhses, templ, &templ);
	}
	if ( EVEN(sts)) return sts;
	
	/* Print wanted node if there is one */
/*******
	sts = ldh_GetObjectInfo( utlctx->ldhses, Objdid,  &info);
	if ( EVEN(sts)) return sts;
	sts = ldh_ObjidToName( ldhses, info.Node, ldh_eName_Hierarchy,
   		        hier_name, sizeof( hier_name), &size);
	if ( ODD(sts))
	{
	  *changed = 1;
	  return FOE__SUCCESS;
	}
********/
	for ( i = 0; i < 3; i++ )
	{
	  if ( i == 0)
	  {
	    if ( code & UTL_FULLPRINT_NORTBODY)
	      /* Exclude rtbody */
	      continue;
	    strcpy( body, "RtBody");
	  }
	  else if ( i == 1)
	  {
	    if ( code & UTL_FULLPRINT_NODEVBODY)
	      /* Exclude devbody */
	      continue;
	    strcpy( body, "DevBody");
	  }
	  else 
	  {
	    if ( code & UTL_FULLPRINT_NOSYSBODY)
	      /* Exclude sysbody */
	      continue;
	    strcpy( body, "SysBody");
	  }

    	  /* Get the runtime paramters for this class */
	  sts = ldh_GetObjectBodyDef(ldhses, cid, body, 1, 
	  		&bodydef, &rows);
	  if ( EVEN(sts) ) continue;

	  for ( j = 0; j < rows; j++)
	  {
	    strcpy( parname, bodydef[j].ParName);
	    /* Get the parameter value in the object */
	    sts = ldh_GetObjectPar( ldhses, Objdid, body,   
			parname, (char **)&object_par, &parsize); 
	    if ( EVEN(sts)) return sts;

	    if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_POINTER )
	      /* Don't write pointers */
	      continue;

	    if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_ARRAY )
	      elements = bodydef[j].Par->Output.Info.Elements;
	    else
	      elements = 1;

	    /* Get the parameter value in the template object */
	    sts = ldh_GetObjectPar( ldhses, templ, body,   
			parname, (char **)&template_par, &size); 
	    if ( EVEN(sts)) return sts;

	    object_element = object_par;
	    template_element = template_par;	    

	    switch ( bodydef[j].Par->Output.Info.Type )
	    {
	      case pwr_eType_Boolean:
	      case pwr_eType_Float32:
	      case pwr_eType_Float64:
	      case pwr_eType_Char:
	      case pwr_eType_Int8:
	      case pwr_eType_Int16:
	      case pwr_eType_Int32:
	      case pwr_eType_UInt8:
	      case pwr_eType_UInt16:
	      case pwr_eType_UInt32:
	      case pwr_eType_String:
	      case pwr_eType_ObjDId:
	      case pwr_eType_AttrRef:
	      case pwr_eType_Time:
	      case pwr_eType_Text:
	      case pwr_eType_ClassId:
	      case pwr_eType_TypeId:
	      case pwr_eType_CastId:
	      case pwr_eType_VolumeId:
	      case pwr_eType_ObjectIx:
	      case pwr_eType_Mask:
	      case pwr_eType_Enum:
	      {
	        /* If the parameters in object and template are node equal, 
	             print them */
	        if ( memcmp( template_element, object_element, parsize) != 0)
	        {
	          *changed = 1;
	        }

	        free((char *) object_par);
	        free((char *) template_par);

	        break;
	      }
              default:
                ;
	    }

	    if ( *changed )
	      break;
	  }
	  free((char *) bodydef);	
	  if ( *changed )
	    break;
	}

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_print_object_full()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  Objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	utlctx		I	output specification.
*
*
* Description: 	
*
**************************************************************************/

static int utl_print_object_full ( 
  pwr_sAttrRef	  *arp,
  ldh_tSesContext ldhses,	
  utl_ctx	  utlctx,
  int		  code
)
{
	int		sts, size, i, j, k;
	pwr_tClassId	cid;
	pwr_tAName     	aname;
	char		pname[120];
	char 		*np, *s;
	pwr_tOName     	hier_name;
	char		*hier_name_p;
	ldh_sParDef 	*bodydef;
	int		rows;
	pwr_tObjid	templ;
	char		body[20];	
	char		parname[32];
	char		name[32];
	char		*object_par;
	char		*template_par;
	char		*object_element;
	char		*template_element;
	int		parsize;
	int		elements;
	pwr_tBoolean	*p_Boolean;	
	pwr_tFloat32	*p_Float32;
	pwr_tFloat64	*p_Float64;
	char		*p_Char;
	pwr_tInt8	*p_Int8;
	pwr_tInt16	*p_Int16;
	pwr_tInt32	*p_Int32;
	pwr_tUInt8	*p_UInt8;
	pwr_tUInt16	*p_UInt16;
	pwr_tUInt32	*p_UInt32;	
	pwr_tObjid	*p_ObjDId;	
	pwr_sAttrRef	*p_AttrRef;	
	char		*p_String;
	char		timbuf[32];

	sts = ldh_AttrRefToName( ldhses, arp, ldh_eName_Aref, &np, &size);
	if ( EVEN(sts)) return sts;
	strcpy( aname, np);
	
	/* Get the template object of this class */
	sts = ldh_GetAttrRefTid( ldhses, arp,  &cid);
	if ( EVEN(sts)) return sts;

	/* Get the first child to the object */
	sts = ldh_GetChild( ldhses, cdh_ClassIdToObjid( cid), &templ);
	while ( ODD(sts) ) {
	  sts = ldh_ObjidToName( 
		ldhses, templ, ldh_eName_Object,
		name, sizeof( name), &size);
	  if ( EVEN(sts)) return sts;
	  
	  if ( strcmp( name, "Template") == 0)
	    break;
	  sts = ldh_GetNextSibling( ldhses, templ, &templ);
	}
	if ( EVEN(sts)) return sts;
	
	for ( i = 0; i < 3; i++ )
	{
	  if ( i == 0) {
	    if ( code & UTL_FULLPRINT_NORTBODY)
	      /* Exclude rtbody */
	      continue;
	    strcpy( body, "RtBody");
	  }
	  else if ( i == 1) {
	    if ( code & UTL_FULLPRINT_NODEVBODY)
	      /* Exclude devbody */
	      continue;
	    strcpy( body, "DevBody");
	  }
	  else {
	    if ( code & UTL_FULLPRINT_NOSYSBODY)
	      /* Exclude sysbody */
	      continue;
	    strcpy( body, "SysBody");
	  }

    	  /* Get the runtime paramters for this class */
	  sts = ldh_GetObjectBodyDef(ldhses, cid, body, 1, 
	  		&bodydef, &rows);
	  if ( EVEN(sts) ) continue;

	  for ( j = 0; j < rows; j++) {
	    strcpy( parname, bodydef[j].ParName);
	    /* Get the parameter value in the object */
	    s = strchr( aname, '.');
	    if ( s) {
	      strcpy( pname, s + 1);
	      strcat( pname, ".");
	      strcat( pname, parname);
	    }
	    else
	      strcpy( pname, parname);
	    sts = ldh_GetObjectPar( ldhses, arp->Objid, body,
			pname, (char **)&object_par, &parsize); 
	    if ( EVEN(sts)) return sts;

	    if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_POINTER )
	      /* Don't write pointers */
	      continue;

	    if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_ARRAY )
	      elements = bodydef[j].Par->Output.Info.Elements;
	    else
	      elements = 1;

	    /* Get the parameter value in the template object */
	    sts = ldh_GetObjectPar( ldhses, templ, body,   
			parname, (char **)&template_par, &size); 
	    if ( EVEN(sts)) return sts;

	    object_element = object_par;
	    template_element = template_par;	    

	    for ( k = 0; k < elements; k++) {
	      IF_OUT u_pagebreak( utlctx);

	      /* If the parameters in object and template are node equal, 
	         print them */
	      if ( memcmp( template_element, object_element, parsize/elements) != 0) {
	        if ( code & UTL_FULLPRINT_SIGNAL) {
	          if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_RTVIRTUAL)
	            /* This is probably a getdi, or get.., print the
			signalobject also */
	            code = code & ~UTL_FULLPRINT_NODEVBODY;
	        }

	        IF_OUT u_pagebreak( utlctx);
	        switch ( bodydef[j].Par->Output.Info.Type ) {
	          case pwr_eType_Boolean:
	          {
	            p_Boolean = (pwr_tBoolean *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            if ( *p_Boolean == 0)
	              u_print( utlctx, " FALSE");
	            else
	              u_print( utlctx, " TRUE");
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_Float32:
	          {
	            p_Float32 = (pwr_tFloat32 *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %f", *p_Float32);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_Float64:
	          {
	            p_Float64 = (pwr_tFloat64 *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %f", *p_Float64);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_Char:
	          {
	            p_Char = object_element;
	            u_print( utlctx,  "    %s", parname);
	            u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %c", *p_Char);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_Int8:
	          {
	            p_Int8 = (pwr_tInt8 *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %d", *p_Int8);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_Int16:
	          {
	            p_Int16 = (pwr_tInt16 *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %d", *p_Int16);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_Int32:
	          case pwr_eType_ClassId:
	          case pwr_eType_TypeId:
	          case pwr_eType_CastId:
	          case pwr_eType_VolumeId:
	          case pwr_eType_ObjectIx:
	          {
	            p_Int32 = (pwr_tInt32 *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %d", *p_Int32);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_UInt8:
	          {
	            p_UInt8 = (pwr_tUInt8 *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %u", *p_UInt8);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_UInt16:
	          {
	            p_UInt16 = (pwr_tUInt16 *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %u", *p_UInt16);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_UInt32:
	          case pwr_eType_Mask:
	          case pwr_eType_Enum:
	          {
	            p_UInt32 = (pwr_tUInt32 *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %lu", *p_UInt32);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_String:
	          {
	            p_String = object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
                    u_print( utlctx, " \"%s\"", p_String);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_Text:
	          {
	            p_String = object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_row( utlctx);
                    u_print( utlctx, "\"%s\"", p_String);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_ObjDId:
	          {
 	            /* Get the object name from ldh */
	            p_ObjDId = (pwr_tObjid *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            sts = ldh_ObjidToName( ldhses, 
	           	*p_ObjDId, ldh_eName_Hierarchy,
  		        hier_name, sizeof( hier_name), &size);
  	            if ( EVEN(sts)) 
	              u_print( utlctx, " Undefined Object");
	            else
	              u_print( utlctx, " %s", hier_name);
	            u_row( utlctx);
	            break;
	          }
	          case pwr_eType_AttrRef:
	          {
 	            /* Get the object name from ldh */
	            p_AttrRef = (pwr_sAttrRef *)object_element;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            sts = ldh_AttrRefToName( ldhses, 
	           	p_AttrRef,  ldh_eName_Aref, &hier_name_p, &size);
  	            if ( EVEN(sts)) 
	              u_print( utlctx, " Undefined attribute");
	            else
	              u_print( utlctx, " %s", hier_name_p);
	            u_row( utlctx);
	            break;
	          }
                  case pwr_eType_Time:
                  {
	            /* Convert time to ascii */
		    	
	            sts = time_AtoAscii((pwr_tTime *)object_element, 
                                                time_eFormat_DateAndTime, 
                                                timbuf, sizeof(timbuf)); 
	            timbuf[20] = 0;
	            u_print( utlctx,  "    %s", parname);
	            if ( elements > 1 )
	            {
	              u_print( utlctx,  "[%2d]", k);
	              u_posit( utlctx, 4, strlen(parname) + 8);
	            }
	            else
	              u_posit( utlctx, 4, strlen(parname) + 4);
	            u_print( utlctx, " %s", timbuf);
	            u_row( utlctx);
	            break;
                  }
                  default:
                    ;
	        }
	      }
   	      object_element += parsize / elements;
	      template_element += parsize / elements;
   	    }
	    free((char *) object_par);
	    free((char *) template_par);
	  }
	  free((char *) bodydef);	
	}


	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_print_object_par()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  Objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	utlctx		I	output specification.
*
*
* Description: 	
*
**************************************************************************/

static int utl_print_object_par ( 
  pwr_sAttrRef	*arp,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  char		*parameter,
  int		*element
)
{
	int		sts, size, i, j, k;
	pwr_tClassId	cid;
	pwr_tAName     	pname;
	char		*np, *s;
	pwr_tAName     	hier_name;
	char		*hier_name_p;
	ldh_sParDef 	*bodydef;
	int		rows;
	char		body[20];	
	char		parname[32];
	char		*object_par;
	char		*object_element;
	int		elements;
	int		found;
	pwr_tBoolean	*p_Boolean;	
	pwr_tFloat32	*p_Float32;
	pwr_tFloat64	*p_Float64;
	char		*p_Char;
	pwr_tInt8	*p_Int8;
	pwr_tInt16	*p_Int16;
	pwr_tInt32	*p_Int32;
	pwr_tUInt8	*p_UInt8;
	pwr_tUInt16	*p_UInt16;
	pwr_tUInt32	*p_UInt32;	
	pwr_tObjid	*p_ObjDId;	
	pwr_sAttrRef	*p_AttrRef;	
	char		*p_String;
	int		first_element;
	int		last_element;
	char		timbuf[32];

	sts = ldh_AttrRefToName( ldhses, 
	           	arp, ldh_eName_Hierarchy,
  		        &np, &size);
  	if ( EVEN(sts)) return sts;
	strcpy( hier_name, np);

	/* Get the class of the object */
	sts = ldh_GetAttrRefTid( ldhses, arp, &cid);

	while ( *parameter != 0) {
	  /* Find the parameter */
	  if ( strcmp( parameter, "WANTEDNODE") == 0) {
            u_print( utlctx, " %s", "WantedNode is obsolete");
	    u_row( utlctx);
	  }
	  else {
	    found = 0;
	    for ( i = 0; i < 3; i++ ) {
	      if ( i == 0)
	        strcpy( body, "RtBody");
	      else if ( i == 1 )
	        strcpy( body, "DevBody");
	      else
	        strcpy( body, "SysBody");

    	      /* Get the paramters for this body */
	      sts = ldh_GetObjectBodyDef(ldhses, cid, body, 1, 
	  		&bodydef, &rows);
	      if ( EVEN(sts) ) continue;

	      for ( j = 0; j < rows; j++) {
	        if (cdh_NoCaseStrcmp( parameter, bodydef[j].ParName) == 0) {
	          found = 1;
	          break;
	        }
	      }
	      if ( found )
	        break;
	      free((char *) bodydef);	
	    }
	    if ( !found) {
	      /* The attribute didn't exist */
	      return FOE__NOPAR;
	    }

	    strcpy( parname, bodydef[j].ParName);

	    if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_ARRAY )
	      elements = bodydef[j].Par->Output.Info.Elements;
	    else
	      elements = 1;

	    if ( *element != UTL_NOELEMENT)
	      if ( *element + 1 > elements)
	      {
	        /* Error in element */
	        free((char *) bodydef);	
	        return FOE__ELEMENT;
	      }
	  
	    /* Get the parameter value in object */
	    s = strchr( hier_name, '.');
	    if ( s) {
	      strcpy( pname, s+1);
	      strcat( pname, ".");
	      strcat( pname, parname);
	    }
	    else
	      strcpy( pname, parname);
	    sts = ldh_GetObjectPar( ldhses, arp->Objid, body,   
			pname, (char **)&object_par, &size); 
	    if ( EVEN(sts)) return sts;

	    if ( *element == UTL_NOELEMENT) {
	      /* No element given, show all the elements */
	      first_element = 0;
	      last_element = elements - 1;
	    }
	    else {
	      first_element = *element;
	      last_element = *element;
	    }

	    for ( k = first_element; k < last_element + 1; k++) 
	    {
	      IF_OUT u_pagebreak( utlctx);

	      object_element = object_par + k * size / elements;
              switch ( bodydef[j].Par->Output.Info.Type )
              {
                case pwr_eType_Boolean:
                {
                  p_Boolean = (pwr_tBoolean *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  if ( *p_Boolean == 0)
                    u_print( utlctx, " FALSE");
                  else
                    u_print( utlctx, " TRUE");
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_Float32:
                {
                  p_Float32 = (pwr_tFloat32 *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %f", *p_Float32);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_Float64:
                {
                  p_Float64 = (pwr_tFloat64 *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %f", *p_Float64);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_Char:
                {
                  p_Char = object_element;
                  u_print( utlctx,  "    %s", parname);
                  u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %c", *p_Char);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_Int8:
                {
                  p_Int8 = (pwr_tInt8 *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %d", *p_Int8);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_Int16:
                {
                  p_Int16 = (pwr_tInt16 *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %d", *p_Int16);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_Int32:
                {
                  p_Int32 = (pwr_tInt32 *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %d", *p_Int32);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_UInt8:
                {
                  p_UInt8 = (pwr_tUInt8 *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %u", *p_UInt8);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_UInt16:
                {
                  p_UInt16 = (pwr_tUInt16 *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %u", *p_UInt16);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_UInt32:
                case pwr_eType_Mask:
                case pwr_eType_Enum:
	        case pwr_eType_ClassId:
	        case pwr_eType_TypeId:
	        case pwr_eType_CastId:
	        case pwr_eType_VolumeId:
	        case pwr_eType_ObjectIx:
                {
                  p_UInt32 = (pwr_tUInt32 *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " %lu", *p_UInt32);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_String:
                {
                  p_String = object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  u_print( utlctx, " \"%s\"", p_String);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_Text:
                {
                  p_String = object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
	          u_row( utlctx);
                  u_print( utlctx, "\"%s\"", p_String);
	          u_row( utlctx);
                  break;
                }
                case pwr_eType_ObjDId:
                {
                  /* Get the object name from ldh */
                  p_ObjDId = (pwr_tObjid *)object_element;
                  u_print( utlctx,  "    %s", parname);
                  if ( elements > 1 )
                  {
                    u_print( utlctx,  "[%2d]", k);
                    u_posit( utlctx, 4, strlen(parname) + 8);
                  }
                  else
                    u_posit( utlctx, 4, strlen(parname) + 4);
                  sts = ldh_ObjidToName( ldhses, 
                 	*p_ObjDId, ldh_eName_Hierarchy,
   		        hier_name, sizeof( hier_name), &size);
 	          if ( EVEN(sts)) 
                    u_print( utlctx, " Undefined Object");
                  else
                    u_print( utlctx, " %s", hier_name);
	          u_row( utlctx);
                  break;
                }
	        case pwr_eType_AttrRef:
	        {
 	          /* Get the object name from ldh */
	          p_AttrRef = (pwr_sAttrRef *)object_element;
	          u_print( utlctx,  "    %s", parname);
	          if ( elements > 1 )
	          {
	            u_print( utlctx,  "[%2d]", k);
	            u_posit( utlctx, 4, strlen(parname) + 8);
	          }
	          else
	            u_posit( utlctx, 4, strlen(parname) + 4);
	          sts = ldh_AttrRefToName( ldhses, 
	           	p_AttrRef,  ldh_eName_Aref, &hier_name_p, &size);
  	          if ( EVEN(sts)) 
	            u_print( utlctx, " Undefined attribute");
	          else
	            u_print( utlctx, " %s", hier_name_p);
	          u_row( utlctx);
	          break;
	        }
                case pwr_eType_Time:
                {
	          /* Convert time to ascii */	
	          sts = time_AtoAscii((pwr_tTime *)object_element, 
                                        time_eFormat_DateAndTime, 
                                        timbuf, sizeof(timbuf));
		  timbuf[20] = 0;
	          u_print( utlctx,  "    %s", parname);
	          if ( elements > 1 )
	          {
	            u_print( utlctx,  "[%2d]", k);
	            u_posit( utlctx, 4, strlen(parname) + 8);
	          }
	          else
	            u_posit( utlctx, 4, strlen(parname) + 4);
	          u_print( utlctx, " %s", timbuf);
	          u_row( utlctx);
	          break;
                }
                default:
 	          ;
              }
	    }
	    free((char *) object_par);	
	    free((char *) bodydef);	
	  }
	  parameter += 80;
	  element++;
        }

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_print_class()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

static int utl_print_class (
  pwr_sAttrRef	*arp,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  int		full,
  unsigned long	dum3,
  unsigned long	dum4
)
{
	int		sts, size;
	pwr_tOName     	hier_name;
	char		class_str[80];

	/* Print the name of the actual object */
	sts = ldh_ObjidToName( 
		ldhses, arp->Objid, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);
	if ( EVEN(sts)) return sts;

	sts = ldh_ObjidToName( 
		ldhses, arp->Objid, ldh_eName_Objid,
		class_str, sizeof( class_str), &size);
	if ( EVEN(sts)) return sts;

	IF_OUT u_pagebreak( utlctx);
	u_print( utlctx,  "  %s", hier_name);
	u_posit( utlctx, 7, strlen(hier_name) + 2);
	u_print( utlctx, " %s", class_str);
	u_row( utlctx); 

	if ( full)
	  utl_print_class_full( arp->Objid, ldhses, utlctx);

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_print_class_full()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  Objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	utlctx		I	output specification.
*
*
* Description: 	
*
**************************************************************************/

static int utl_print_class_full (
  pwr_tObjid	klass,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx
)
{
	int		sts, i, j;
	ldh_sParDef 	*bodydef;
	int		rows;
	char		body[20];	
	char		parname[32];

	for ( i = 0; i < 3; i++ )
	{
	  if ( i == 0)
	    strcpy( body, "RtBody");
	  else if ( i == 1 )
	    strcpy( body, "DevBody");
	  else
	    strcpy( body, "SysBody");

    	  /* Get the runtime paramters for this class */
	  sts = ldh_GetObjectBodyDef(ldhses, cdh_ClassObjidToId( klass), 
			body, 1, &bodydef, &rows);
	  if ( EVEN(sts) ) continue;

	  u_print( utlctx, "    %s", body);
	  u_row( utlctx);
	  
	  for ( j = 0; j < rows; j++)
	  {
	    strcpy( parname, bodydef[j].ParName);
	    if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_ARRAY )
	      sprintf( &parname[strlen(parname)], "[%d]",
			bodydef[j].Par->Output.Info.Elements);
	    IF_OUT u_pagebreak( utlctx);
	    u_print( utlctx, "      %s", parname);
	    u_posit( utlctx, 4, strlen(parname) + 6);

	    switch ( bodydef[j].ParClass )
	    {
	      case pwr_eClass_Input:
	      {
	        u_print( utlctx, " $Input		");
	        break;
	      }
	      case pwr_eClass_Intern:
	      {
	        u_print( utlctx, " $Intern	");
	        break;
	      }
	      case pwr_eClass_Output:
	      {
	        u_print( utlctx, " $Output	");
	        break;
	      }
	      case pwr_eClass_Param:
	      {
	        u_print( utlctx, " $Param		");
	        break;
	      }
	      case pwr_eClass_Buffer:
	      {
	        u_print( utlctx, " $Buffer	");
	        break;
	      }
	      case pwr_eClass_ObjXRef:
	      {
	        u_print( utlctx, " $ObjXRef	");
	        break;
	      }
	      default:
	      {
	        u_print( utlctx, "		");
	      }
 	    }

	    /* If the parameters in object and template are node equal, 
	       print them */
	    switch ( bodydef[j].Par->Output.Info.Type )
	    {
	      case pwr_eType_Boolean:
	      {
	        u_print( utlctx,  "Boolean	");
	        break;
	      }
	      case pwr_eType_Float32:
	      {
	        u_print( utlctx,  "Float32	");
	        break;
	      }
	      case pwr_eType_Float64:
	      {
	        u_print( utlctx,  "Float64	");
	        break;
	      }
	      case pwr_eType_Int8:
	      {
	        u_print( utlctx,  "Int8	");
	        break;
	      }
	      case pwr_eType_Int16:
	      {
	        u_print( utlctx,  "Int16	");
	        break;
	      }
	      case pwr_eType_Int32:
	      {
	        u_print( utlctx,  "Int32	");
	        break;
	      }
	      case pwr_eType_UInt8:
	      {
	        u_print( utlctx,  "UInt8	");
	        break;
	      }
	      case pwr_eType_UInt16:
	      {
	        u_print( utlctx,  "UInt16	");
	        break;
	      }
	      case pwr_eType_UInt32:
	      {
	        u_print( utlctx,  "UInt32	");
	        break;
	      }
	      case pwr_eType_String:
	      {
	        u_print( utlctx,  "String	");
	        break;
	      }
	      case pwr_eType_Text:
	      {
	        u_print( utlctx,  "Text  	");
	        break;
	      }
	      case pwr_eType_ObjDId:
	      {
	        u_print( utlctx,  "ObjDId	");
	        break;
	      }
	      case pwr_eType_AttrRef:
	      {
	        u_print( utlctx,  "AttrRef	");
	        break;
	      }
	      case pwr_eType_Buffer:
	      {
	        u_print( utlctx, "Buffer	");
	        break;
	      }
	      case pwr_eType_Enum:
	      {
	        u_print( utlctx, "Enum	");
	        break;
	      }
	      case pwr_eType_Struct:
	      {
	        u_print( utlctx, "Struct	");
	        break;
	      }
	      case pwr_eType_Mask:
	      {
	         u_print( utlctx, "Mask	");
	         break;
	      }
	      case pwr_eType_Array:
	      {
	        u_print( utlctx, "Array	");
	        break;
	      }
	      case pwr_eType_Time:
	      {
	        u_print( utlctx, "Time	");
	        break;
	      }
	      default:
	      {
	        u_print( utlctx, "	");
	      }
	    }

	    if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_POINTER )
	      u_print( utlctx, "Pointer	");
	    else
	      u_print( utlctx, "	");

	    u_print( utlctx, "%d", bodydef[j].Par->Output.Info.Size);
	    u_row( utlctx);
	  }
	  free((char *) bodydef);	
	}

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_set_parameter()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	utlctx		I	output specification.
*
*
* Description: 	
*
**************************************************************************/

static int utl_set_parameter (
  pwr_sAttrRef	*arp,
  ldh_tSesContext ldhses,	
  char		*parameter,
  char		*invaluestr,
  int		element,
  utl_ctx	utlctx
)
{
  char		*valuestr;
  static char	yes_or_no[200];	
  int		sts, size, k;
  int		parsize;
  pwr_tClassId	cid;
  pwr_tOName     	hier_name;
  char		*object_par;
  char		*object_element;
  int		elements;
  char		logstr[200];
  static 	char   	value[200];
  char		*logstrptr = logstr;
  pwr_tBoolean	*p_Boolean;	
  pwr_tFloat32	*p_Float32;
  pwr_tFloat64	*p_Float64;
  char		*p_Char;
  pwr_tInt8	*p_Int8;
  pwr_tInt16	*p_Int16;
  pwr_tInt32	*p_Int32;
  pwr_tUInt8	*p_UInt8;
  pwr_tUInt16	*p_UInt16;
  pwr_tUInt32	*p_UInt32;	
  pwr_tObjid	*p_ObjDId;	
  pwr_sAttrRef	*p_AttrRef;	
  char		*p_String;
  int		first_element;
  int		last_element;
  pwr_tObjid	objid = arp->Objid;
  pwr_tAName	aname;
  pwr_sAttrRef aref;
  ldh_sAttrRefInfo info;
  char *np;

#if defined OS_VMS
	int len;
  	$DESCRIPTOR ( prompt_desc , "Enter value: " );
  	static $DESCRIPTOR ( value_desc , value );
  	$DESCRIPTOR ( yon_prompt_desc , "(Y/N/Q/A): " );
  	static $DESCRIPTOR ( yon_desc , yes_or_no );
#endif

	sts = ldh_ObjidToName( ldhses, 
	           	objid, ldh_eName_Hierarchy,
  		        hier_name, sizeof( hier_name), &size);
  	if ( EVEN(sts)) return sts;

	/* Get the class of the object */
	sts = ldh_GetObjectClass( ldhses, objid, &cid);

	strcpy( aname, hier_name);
	strcat( aname, ".");
	strcat( aname, parameter);


	sts = ldh_NameToAttrRef( ldhses, aname, &aref);
	if ( EVEN(sts)) return sts;

	sts = ldh_AttrRefToName( ldhses, &aref, cdh_mNName, &np,&size);
	if ( EVEN(sts)) return sts;

	strcpy( aname, np);

	sts = ldh_GetAttrRefInfo( ldhses, &aref, &info);
	if ( EVEN(sts)) return sts;

	if ( info.flags & PWR_MASK_POINTER ) {
	  /* Don't write pointers */
	  return FOE__SETPTR;
	}

	if ( info.flags & PWR_MASK_ARRAY )
	 elements = info.nElement;
	else
	  elements = 1;

	if ( element != UTL_NOELEMENT)
	  if ( element + 1 > elements) {
	    /* Error in element */
	    return FOE__ELEMENT;
	  }
	  
	if ( element == UTL_NOELEMENT) {
	  /* No element given, show all the elements */
	  first_element = 0;
	  last_element = elements - 1;
	}
	else {
	  first_element = element;
	  last_element = element;
	}

	parsize = info.size;
	for ( k = first_element; k < last_element + 1; k++) {
	  /* Get the parameter */
	  object_par = (char *) calloc( 1, info.size);
	  sts = ldh_ReadAttribute( ldhses, &aref, object_par, info.size);
	  if ( EVEN(sts)) return sts;
	  object_element = object_par + k * parsize / elements;

	  strcpy( logstr, aname);
	  if ( info.flags & PWR_MASK_ARRAY )
 	    sprintf( logstrptr + strlen(logstr), "[%d]",k);

	  /* Get the value if not passed */
	  if ( invaluestr == NULL ) {
	    printf( "%s\n", logstr);
#if defined OS_VMS
 	    sts = lib$get_input ( &value_desc , &prompt_desc , &len ) ; 
	    if ( EVEN(sts)) return sts;
            value[len] = '\0' ;
#else
	    // TODO
	    printf( "Enter value: ");
	    sts = scanf( "%s", value);
#endif
	    valuestr = (char *)&value;
	    if ( *valuestr == '\0' )
	      continue;
	  }
	  else
	    valuestr = invaluestr;

	  sprintf( logstrptr + strlen(logstr), " = ");

	  /* Put the value in the parameter buffer */
	  switch ( info.type )
	  {
	    case pwr_eType_Boolean:
	    {
	      p_Boolean = (pwr_tBoolean *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %d ) ", *p_Boolean);
	      sscanf( valuestr, "%d", p_Boolean);
	      sprintf( logstrptr + strlen(logstr), "%d", *p_Boolean);
	      break;
	    }
	    case pwr_eType_Float32:
	    {
	      p_Float32 = (pwr_tFloat32 *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %f ) ", *p_Float32);
	      sscanf( valuestr, "%f", p_Float32);
	      sprintf( logstrptr + strlen(logstr), "%f", *p_Float32);
	      break;
	    }
	    case pwr_eType_Float64:
	    {
	      p_Float64 = (pwr_tFloat64 *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %f ) ", *p_Float64);
	      sscanf( valuestr, "%lf", p_Float64);
	      sprintf( logstrptr + strlen(logstr), "%f", *p_Float64);
	      break;
	    }
	    case pwr_eType_Char:
	    {
	      p_Char = object_element;
	      sprintf( logstrptr + strlen(logstr), "( %c ) ", *p_Char);
	      sscanf( valuestr, "%c", p_Char);
	      sprintf( logstrptr + strlen(logstr), "%c", *p_Char);
	      break;
	    }
	    case pwr_eType_Int8:
	    {
	      pwr_tInt16	i16;
	      pwr_tInt8		i8;
	      p_Int8 = (pwr_tInt8 *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %d ) ", *p_Int8);
	      sscanf( valuestr, "%hd", &i16);
	      i8 = i16;
	      memcpy( object_element, (char *) &i8, sizeof(i8));
	      sprintf( logstrptr + strlen(logstr), "%d", *p_Int8);
	      break;
	    }
	    case pwr_eType_Int16:
	    {
	      p_Int16 = (pwr_tInt16 *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %d ) ", *p_Int16);
	      sscanf( valuestr, "%hd", p_Int16);
	      sprintf( logstrptr + strlen(logstr), "%d", *p_Int16);
	      break;
	    }
	    case pwr_eType_Int32:
	    {
	      p_Int32 = (pwr_tInt32 *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %d ) ", *p_Int32);
	      sscanf( valuestr, "%d", p_Int32);
	      sprintf( logstrptr + strlen(logstr), "%d", *p_Int32);
	      break;
	    }
	    case pwr_eType_UInt8:
	    {
	      pwr_tUInt16	i16;
	      pwr_tUInt8	i8;
	      p_UInt8 = (pwr_tUInt8 *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %u ) ", *p_UInt8);
	      sscanf( valuestr, "%hd", &i16);
	      i8 = i16;
	      memcpy( object_element, (char *) &i8, sizeof(i8));
	      sprintf( logstrptr + strlen(logstr), "%u", *p_UInt8);
	      break;
	    }
	    case pwr_eType_UInt16:
	    {
	      p_UInt16 = (pwr_tUInt16 *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %u ) ", *p_UInt16);
	      sscanf( valuestr, "%hd", p_UInt16);
	      sprintf( logstrptr + strlen(logstr), "%u", *p_UInt16);
	      break;
	    }
	    case pwr_eType_UInt32:
	    case pwr_eType_Mask:
	    case pwr_eType_Enum:
	    case pwr_eType_ClassId:
	    case pwr_eType_TypeId:
	    case pwr_eType_CastId:
	    case pwr_eType_VolumeId:
	    case pwr_eType_ObjectIx:
	    {
	      p_UInt32 = (pwr_tUInt32 *)object_element;
	      sprintf( logstrptr + strlen(logstr), "( %u ) ", *p_UInt32);
	      sscanf( valuestr, "%d", p_UInt32);
	      sprintf( logstrptr + strlen(logstr), "%u", *p_UInt32);
	      break;
	    }
	    case pwr_eType_String:
	    {
	      p_String = object_element;
	      sprintf( logstrptr + strlen(logstr), "( %s ) ", p_String);
	      strncpy( p_String, valuestr, info.size/info.nElement);
	      *(p_String + info.size/info.nElement - 1) = 0;
	      sprintf( logstrptr + strlen(logstr), "%s", p_String);
	      if ( (int) strlen( valuestr) > (info.size/info.nElement -1))
	        printf( "%%FOE-W-LONG_STRING, parameter size is exceeded\n");
	      break;
	    }
	    case pwr_eType_ObjDId:
	    {
	      pwr_tOName  	objdid_name;

 	      /* Get the object name from ldh */
	      p_ObjDId = (pwr_tObjid *)object_element;
	      sts = ldh_ObjidToName( ldhses, *p_ObjDId, ldh_eName_Hierarchy,  
		objdid_name, sizeof( objdid_name), &size);
	      if ( EVEN(sts))
	      {
	        objdid_name[0] = '\0';
	      }
	      sprintf( logstrptr + strlen(logstr), "( %s ) ", objdid_name);
	      if ( strncmp( valuestr, "_O", 2) == 0)
		sts = cdh_StringToObjid( valuestr, p_ObjDId);
	      else
		sts = ldh_NameToObjid( ldhses, p_ObjDId, valuestr);
	      if ( EVEN(sts))
	      {
	        printf("Object does not exist\n");
	        return FOE__SUCCESS;
	      }
	      sts = ldh_ObjidToName( ldhses, *p_ObjDId, ldh_eName_Hierarchy,
		objdid_name, sizeof( objdid_name), &size);
	      if ( EVEN(sts))
	      {
	        objdid_name[0] = '\0';
	      }  
	      sprintf( logstrptr + strlen(logstr), "%s", objdid_name);
	      break;
	    }
	    case pwr_eType_AttrRef:
	    {
	      pwr_tAName  	objdid_name;
	      char  		*objdid_name_p;

 	      /* Get the object name from ldh */
	      p_AttrRef = (pwr_sAttrRef *)object_element;
	      sts = ldh_AttrRefToName( ldhses, p_AttrRef, 
		 ldh_eName_Aref, &objdid_name_p, &size);
	      if ( EVEN(sts))
	        objdid_name[0] = '\0';
	      else
	        strcpy( objdid_name, objdid_name_p);
	      sprintf( logstrptr + strlen(logstr), "( %s ) ", objdid_name);
	      sts = ldh_NameToAttrRef( ldhses, valuestr, p_AttrRef);
	      if ( EVEN(sts))
	      {
	        printf("Attribute does not exist\n");
	        return FOE__SUCCESS;
	      }
	      sts = ldh_AttrRefToName( ldhses, p_AttrRef, 
		 ldh_eName_Aref, &objdid_name_p, &size);
	      if ( EVEN(sts))
	        objdid_name[0] = '\0';
	      else
	        strcpy( objdid_name, objdid_name_p);
	      sprintf( logstrptr + strlen(logstr), "%s", objdid_name);
	      break;
	    }
	    default:
	    {
	      printf("Parameter type not correct\n");
	    }
 	  }

	  if ( utlctx->confirm )	
	  {
	    printf("%s ", logstr);
#if defined OS_VMS
 	    sts = lib$get_input ( &yon_desc , &yon_prompt_desc , &len ) ; 
	    if ( EVEN(sts)) return sts;
#else
	    // TODO
	    printf( "(Y/N/Q/A): ");
	    sts = scanf( "%s", yes_or_no);
#endif
	    if ( ((yes_or_no[0] == 'Q') || (yes_or_no[0] == 'q')))
	      return FOE__ABORTSEARCH;
	    else if ( ((yes_or_no[0] == 'A') || (yes_or_no[0] == 'a')))
	      utlctx->confirm = 0;
	    else if ( !((yes_or_no[0] == 'Y') || (yes_or_no[0] == 'y')))
	      continue;
	  }
	  if ( utlctx->log )	
	    u_print( utlctx, "Set %s\n", logstr);

	  /* Set the parameter */
	  sts = ldh_WriteAttribute( ldhses, &aref, object_par, parsize);
	  if ( EVEN(sts)) return sts;

	  /* Specific updates for grafcet orders */
	  sts = vldh_object_update_spec( ldhses, objid);
	  if ( EVEN(sts)) return sts;
	}

	free((char *) object_par);


	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_show_connection()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	windowstring	I	Name of the window
*
* Description: 	Prints all connection objects found in a window.
*
**************************************************************************/

int utl_show_connection (
  ldh_tSesContext ldhses,
  char		*windowstring,
  int		terminal,
  char		*filename
)
{
	utl_ctx	utlctx;
	int		sts, size;
	int		i;
	pwr_tOName     	hier_name;	
	pwr_tObjid	window;
	unsigned long	object_count;
	pwr_tObjid	*objectlist;
	pwr_tObjid	*objectlist_ptr;
	char		objid_str[80];

	  /* Get objdid for the plcpgm */
	  sts = ldh_NameToObjid( ldhses, &window, windowstring);
	  if ( EVEN(sts))
	  {
	    return FOE__WINDNOTFOUND;
	  }
 
	  /* Open file */
	  utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	  sts = u_open( utlctx, filename, terminal, 0);
	  if ( EVEN(sts)) 
	  {
	    utl_ctx_delete( utlctx);
	    return sts;
	  }

	  IF_OUT u_header( utlctx, ldhses, "LIST OF CONNECTIONS");
	  IF_OUT u_row( utlctx);

	  /* Print the window */
	  u_print( utlctx,  "Window  %s", windowstring);
	  u_row( utlctx);

	  sts = trv_get_window_connections( ldhses, window, 
			&object_count, &objectlist);
	  if ( EVEN(sts)) exit(sts);
	  
	  objectlist_ptr = objectlist;
	  for ( i = 0; i < (int)object_count; i++)
	  {
	    /* Get the name of the object */
	    sts = ldh_ObjidToName( 
		ldhses, *objectlist_ptr, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);
	    sts = ldh_ObjidToName( 
		ldhses, *objectlist_ptr, ldh_eName_Objid,
		objid_str, sizeof( objid_str), &size);
	    IF_OUT u_pagebreak( utlctx);
	    u_print( utlctx,  "  %s", hier_name);
	    u_posit( utlctx, 7, strlen(hier_name) + 2);
	    u_print( utlctx,  " %s", objid_str);
	    u_row( utlctx);
	    objectlist_ptr++;
	  }
	  IF_OUT u_pageend( utlctx);
	  if ( object_count > 0) free((char *) objectlist);

	  u_close( utlctx);
	  utl_ctx_delete( utlctx);

	  if ( object_count == 0)
	    return FOE__NOCONS;
	  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_link()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	nodename	I	Name of the node.
*
* Description: 	Creates process modules and links the plcprogram of a node.
*
**************************************************************************/

int utl_link ( 
  ldh_tSesContext ldhses,
  char		*nodename,
  int		debug
)
{
	unsigned long sts;
	pwr_tObjid	rtnode;
	
	/* Get the node objdid */
	sts = ldh_NameToObjid( ldhses, &rtnode, nodename);
	if ( EVEN(sts)) 
	  return FOE__NODENAME;
/*
	sts = gcg_comp_rtnode( ldhses, rtnode, 0, 1, &errorcount, 
		&warningcount, debug);
	if (EVEN(sts)) return sts;
*/
	return FOE__SUCCESS;
}	


/*************************************************************************
*
* Name:		utl_compile()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* ldh_tWBContext  ldhwb		I	ldh workbench.
* char *	objectname	I	Name of a window, plcpgm or node.
*
* Description: 	Compiling one or a number of windows depending of 
*		the class of the object given in objectname. If the
* 		class is a window the window is compiled. If the class
*		is plcpgm all windows in the plcpgm is compiled. If the
*		class i node all windows in plcpgms connected to the 
*		node is compiled.
*		Everytime a window is compiled, the module for the plcpgm
*		is also compiled.
*		
*
**************************************************************************/

int utl_compile ( 
  ldh_tSesContext ldhses,
  ldh_tWBContext  ldhwb,
  char		*plcname,
  char		*windowname,
  char		*hiername,
  char		*fromname,
  int		modified,
  int		debug,
  int		allvolumes,
  char		*volumes
)
{
	int		sts, size;
	int		i, nr;
	char		*windbuffer;
	pwr_tClassId	cid;
	pwr_eClass	e_class;
	pwr_tObjid	window;
	pwr_tObjid	plcobjdid;
	pwr_tObjid	fromobjdid;
	pwr_tObjid	hierobjdid;
	utl_t_objidlist	*list_ptr;
	utl_t_objidlist	*plcpgmlist;
	int		plcpgmcount;
	int		from;
	int		from_found;
	pwr_tClassId	*class_ptr;
	pwr_tClassId	class_vect[2];
	ldh_sSessInfo	info;
	int		status = GSX__NOMODIF;
	int		other_volume_attached;
	ldh_tVolContext	volctx;
	ldh_tSesContext	l_ldhses;
	char		vol_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tVolumeId	volume_vect[UTL_INPUTLIST_MAX + 1];
	pwr_tVolumeId	*volume_p;
	trv_tCtx       	trvctx;
	pwr_tVolumeId	vol_id;
        int             thisvolume;
        pwr_tVolumeId   current_volid;
	ldh_sVolumeInfo volinfo;	
	int 		access;

	/* Check that the utilily session is saved */
	sts = ldh_GetSessionInfo( ldhses, &info);
	if ( EVEN(sts)) return sts;
	if ( !info.Empty)
	  return GSX__NOTSAVED;

	/* To be able to compile the windows, the session has to 
	   be set to ReadOnly */
	access = info.Access;
	sts = ldh_SetSession( ldhses, ldh_eAccess_ReadOnly);
	if ( EVEN(sts)) return sts;


	  if ( fromname != NULL)
	  {
	    sts = ldh_NameToObjid( ldhses, &fromobjdid, fromname);
	    if ( EVEN(sts))
	    {
	      status = FOE__OBJECT;
	      goto error_return;
	    }	
	    from = 1;
	  }
	  else
	    from = 0;


          if ( volumes == NULL &&  !allvolumes && hiername == NULL &&
                 plcname == NULL && windowname == NULL) {
	    sts = ldh_GetVolumeInfo( ldh_SessionToVol( ldhses), &volinfo);
	    if (EVEN(sts)) return sts;
	    current_volid = volinfo.Volume;
            thisvolume = 1;
	  }
          else
            thisvolume = 0;

	  if ( hiername != NULL || allvolumes || volumes != NULL || thisvolume)
	  {
	    if ( volumes != NULL)
	    {
	      /* Parse the volumestr */
	      nr = utl_parse( volumes, ", ", "", (char *)vol_str, 
		sizeof( vol_str) / sizeof( vol_str[0]), sizeof( vol_str[0]));
	      if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
	      {
	        status = FOE__PARSYNT;
	        goto error_return;
	      }	

	      for ( i = 0; i < nr; i++)
	      {
	        sts = ldh_VolumeNameToId( ldh_SessionToWB( ldhses), vol_str[i],
			&volume_vect[i]);
	        if ( EVEN(sts))
	        {
	          status = sts;
	          goto error_return;
	        }	
	      }
	      volume_vect[nr] = 0;
	      volume_p = volume_vect;
	    }
	    else
	      volume_p = 0;
	
	    if ( allvolumes)
	    {
	      /* Get all volumes that is not class and wb volumes */
	      i = 0;
	      sts = ldh_GetVolumeList( ldh_SessionToWB( ldhses), &vol_id);
	      while ( ODD(sts) )
	      {
		sts = ldh_GetVidInfo( ldh_SessionToWB( ldhses), vol_id, &volinfo);
	        if (EVEN(sts)) return sts;

	        if ( volinfo.VolRep == ldh_eVolRep_Db) { // Todo!!!! Handle dbms
	          volume_vect[i] = vol_id;
	          i++;
	          if ( i > UTL_INPUTLIST_MAX)
	          {
	            return FOE__MAXITEM;
	            goto error_return;
	          }
	        }
	        sts = ldh_GetNextVolume( ldh_SessionToWB( ldhses), vol_id, &vol_id);
	      }
	      volume_vect[i] = 0;
	      volume_p = volume_vect;
	    }

	    if ( hiername != NULL)
	    {
	      sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
	      if ( EVEN(sts))
	      {
	        status = FOE__OBJNAME;
	        goto error_return;
	      }	

	      if ( !(allvolumes || volumes != NULL))
	      {
	        /* Hierobjdid has to be in the current volume */
	        if  ( ! ldh_LocalObject( ldhses, hierobjdid))
	        {
	           status = FOE__OBJECTVOL;
	           goto error_return;
	        }	
	      }
	    }
	    else
	      hierobjdid = pwr_cNObjid;

            if ( thisvolume) {
	      volume_vect[0] = current_volid;
	      volume_vect[1] = 0;
            }


	    /* Get class */
	    class_vect[0] = pwr_cClass_plc;
	    class_vect[1] = 0;
	    class_ptr = class_vect;

	    plcpgmcount = 0;
	    plcpgmlist = 0;

	    sts = trv_create_ctx( &trvctx, ldhses, hierobjdid, class_ptr, NULL,
		volume_p);
	    if ( EVEN(sts)) return sts;

	    sts = trv_object_search( trvctx,
		&utl_objidlist_insert, &plcpgmlist, &plcpgmcount, 0, 0, 0);
	    if ( EVEN (sts)) return sts;

	    sts = trv_delete_ctx( trvctx);

	    list_ptr = plcpgmlist;
	    from_found = 0;
	    while ( list_ptr)
	    {
	      if ( from)
	      {
	        if ( !from_found )
	        {
	          if ( cdh_ObjidIsEqual( list_ptr->objid, fromobjdid))
	          {
	            /* Start to complie from now on 	*/
	            from_found = 1;
	          }
	          else
	          {
	            list_ptr = list_ptr->next;
	            continue;
	          }
	        }
	      }

	      other_volume_attached = 0;
	      if ( !ldh_LocalObject( ldhses, list_ptr->objid))
	      {
	        /* Attach this volume */
	        sts = ldh_AttachVolume( ldhwb, list_ptr->objid.vid, &volctx);
	        if ( EVEN(sts))
	        {
	          status = sts;
	          goto error_return;
	        }
	        other_volume_attached = 1;

		/* Open a read session */
		sts = ldh_OpenSession(&l_ldhses, volctx, ldh_eAccess_ReadOnly, 
			ldh_eUtility_Pwr);
	        if ( EVEN(sts)) 
	        {
	          if ( other_volume_attached)
	            ldh_DetachVolume( ldhwb, volctx);
	          status = sts;
	          goto error_return;
                }
	      }
	      else
	        l_ldhses = ldhses;

	      /* Get the rootwindow to this plcpgm */
	      sts = trv_get_plc_window( ldhses, list_ptr->objid, &window);
	      if ( EVEN(sts)) return sts;
	
	      /* Compile the windows */
	      sts = gcg_wind_comp_all( ldhwb, l_ldhses, window, 1, modified, debug);
	      if ( other_volume_attached)
	      {
		ldh_CloseSession( l_ldhses);
	        ldh_DetachVolume( ldhwb, volctx);
	      }
 	      if ( EVEN(sts))
	      {
	        status = sts;
	        goto error_return;
	      }	
	      else if ( sts == GSX__NOMODIF)
		status = sts;
	      list_ptr = list_ptr->next;
	    }
	    utl_objidlist_free( plcpgmlist);
	  }

	  /* Check if this is a plcpgm */
	  if ( plcname != NULL)
	  {
	    /* This i a plc */
	    sts = ldh_NameToObjid( ldhses, &plcobjdid, plcname);
	    if ( EVEN(sts))
	    {
	      status = FOE__OBJNAME;
	      goto error_return;
	    }	

	    /* Plc has to be in the current volume */
	    if ( !ldh_LocalObject( ldhses, plcobjdid))
	    {
	       status = FOE__OBJECTVOL;
	       goto error_return;
	    }	

	    /* Find out which typ of object it is  */
	    sts = ldh_GetObjectClass( ldhses, plcobjdid, &cid);

	    if ( cid != pwr_cClass_plc)
	    {
	      status = FOE__CLASS;
	      goto error_return;
	    }	

	    /* Get the rootwindow to this plcpgm */
	    sts = trv_get_plc_window( ldhses, plcobjdid, &window);
	    if ( EVEN(sts)) {
	      // No window, skip this plc
	      status = GSX__SUCCESS;
	      goto error_return;
	    }
	
	    /* Compile the windows */
	    sts = gcg_wind_comp_all( ldhwb, ldhses, window, 1, modified, debug);
	    if ( EVEN(sts))
	    {
	      status = sts;
	      goto error_return;
	    }	
	    else if ( sts != GSX__NOMODIF)
	      status = sts;
	  }

	  if ( windowname != NULL)
	  {
	    sts = ldh_NameToObjid( ldhses, &window, windowname);
	    if ( EVEN(sts))
	    {
	      status = FOE__OBJNAME;
	      goto error_return;
	    }	
	    /* Window has to be in the current volume */
	    if ( !ldh_LocalObject( ldhses, window))
	    {
	       status = FOE__OBJECTVOL;
	       goto error_return;
	    }	

	    if ( EVEN( ldh_GetObjectBuffer( ldhses,
			window, "DevBody", "PlcWindow", &e_class,	
			&windbuffer, &size) ))
	    {
	      status = FOE__CLASS;
	      goto error_return;
	    }

	    /* This is a window */
	    free((char *) windbuffer);

	    /* Compile the windows */
	    sts = gcg_wind_comp_all( ldhwb, ldhses, window, 1, modified, debug);
	    if ( EVEN(sts))
	    {
	      status = sts;
	      goto error_return;
	    }	
	    else if ( sts != GSX__NOMODIF)
	      status = sts;
	  }

	  /* Return to session access ReadWrite */ 
	  sts = ldh_SetSession( ldhses, (ldh_eAccess) access);
	  if ( EVEN(sts)) return sts;

	  return status;

error_return:
	  sts = ldh_SetSession( ldhses, (ldh_eAccess) access);
	  if ( EVEN(sts)) return sts;

	  return status;
	
}


/*************************************************************************
*
* Name:		utl_crossref_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	objectname	I	Name of the object
*
* Description: 	Searches for crossreferenses to the specified object.
*		All found references is printed.
*
**************************************************************************/

int utl_crossref_object ( 
  ldh_tSesContext ldhses,
  char		*objectname,
  int		terminal,
  char		*filename
)
{
	int		sts;
	pwr_sAttrRef  	aref;
	utl_ctx		utlctx;

	/* Get objdid and class for the object */
	sts = ldh_NameToAttrRef( ldhses, objectname, &aref);
	if ( EVEN(sts))
	  return FOE__OBJNAME;

	/* Open file */
	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts))  {
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	IF_OUT u_header( utlctx, ldhses, "CROSS REFERENCE LIST");
	IF_OUT u_row( utlctx);

	sts = utl_crossref( ldhses, &aref, utlctx, 0, 0, 0);
	if ( EVEN (sts)) return sts;

	cross_crosslist_unload();

	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_crossref(_hier_class()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	hiername	I	Name of an object in the hierarchy
* char *	classname	I	Name of class
*
* Description: 	Searches for crossreferenses to all objects in the
*		database that is a member of the specified class and that
*		is found below the specified object in the hierarchy.
*		All found references is printed.
*
**************************************************************************/

int utl_crossref_hier_class_name ( 
  ldh_tSesContext ldhses,
  char		*hiername,
  char		*classname,
  char		*name,
  int		terminal,
  char		*filename
)
{
	int		sts, size, i;
	pwr_tClassId	*classp;
	pwr_tObjid	hierobjdid;
	utl_ctx	utlctx;
	char		*s;
	utl_t_list	*list_ptr;
	utl_t_list	*crrlist_ptr;
	char		title[] = 
"  Object							 Class";
	pwr_tAName     	crrhier_name;
	char		crrclass_name[80];
	pwr_tClassId	crrclass;
	char		class_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tClassId	class_vect[UTL_INPUTLIST_MAX + 1];
	int		nr;
	int		single_object = 0;
	pwr_sAttrRef	aref;
	char		*np;

	/* Check if class */
	if ( classname != NULL ) {
	  nr = utl_parse( classname, ", ", "", (char *)class_str, 
		sizeof( class_str) / sizeof( class_str[0]), sizeof( class_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
	    return FOE__CLASSYNT;

	  for ( i = 0; i < nr; i++) {
	    sts = ldh_ClassNameToId( ldhses, &class_vect[i], class_str[i]);
	    if ( EVEN(sts))
	    {
	      return FOE__CLASSNAME;
	    }
	  }
	  class_vect[nr] = 0;
	  classp = class_vect;
	}	
	else
	  classp = 0;

	/* Check if hierarchy */
	if ( hiername != NULL ) {
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
	  if ( EVEN(sts))
	  {
	    return FOE__HIERNAME;
	  }	
	}
	else
	  hierobjdid = pwr_cNObjid;

	utl_ctx_new( &utlctx, ldhses, title, UTL_PORTRAIT);

	/* Check if name */
	if ( name != NULL ) {
	  /* Check that name includes a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0) {
	    /* Print this object */
	    /* Get objdid for the object */
	    sts = ldh_NameToAttrRef( ldhses, name, &aref);
	    if ( EVEN(sts)) {
	      return FOE__OBJECT;
	    }
	    sts = utl_ctxlist_insert( &aref,
		&(utlctx->list[0]), &(utlctx->listcount[0]), 0, 0, 0);
	    if ( EVEN(sts)) return sts;
	    single_object = 1;
	  }
	}

	if ( !single_object) {
	  sts = trv_get_attrobjects( ldhses, hierobjdid, classp, name, 
				     trv_eDepth_Deep,
		(trv_tBcFunc) utl_ctxlist_insert, 
		&(utlctx->list[0]), &(utlctx->listcount[0]), 0, 0, 0);
	  if ( EVEN (sts)) return sts;
	}
	utl_list_sort( &utlctx->list[0], utlctx->listcount[0], ldhses, 0);

	list_ptr = utlctx->list[0];
	while( list_ptr) {	
	  /* Call the crossreference method for this objdid */
	  sts = crr_crossref( ldhses, &list_ptr->o, &(list_ptr->sublist[0]), 
		&(list_ptr->sublistcount[0]));
	  if ( EVEN(sts)) return sts;
	  utl_list_sort( &list_ptr->sublist[0], list_ptr->sublistcount[0], 
		ldhses, 0);
	  list_ptr = list_ptr->next;
	}
	cross_crosslist_unload();

	/* Open file */
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts)) 
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	IF_OUT u_header( utlctx, ldhses, "CROSS REFERENCE LIST");
	if ( classp != 0)
	  IF_OUT u_subheader( utlctx, "Class", classname);
	if ( cdh_ObjidIsNotNull( hierobjdid))
	  IF_OUT u_subheader( utlctx, "Hierarchy", hiername);
	if ( name != NULL )
	  IF_OUT u_subheader( utlctx, "Name", name);
	IF_OUT u_row( utlctx);

	list_ptr = utlctx->list[0];
	while ( list_ptr) {	
	  sts = utl_print_aref( &list_ptr->o, ldhses, utlctx, 0, 0, 
		0);
	  if ( EVEN(sts)) return sts;

	  crrlist_ptr = list_ptr->sublist[0];
	  while ( crrlist_ptr) {
	    IF_OUT u_pagebreak( utlctx);

	    /* Get the name of the object */
	    sts = ldh_AttrRefToName( ldhses, &crrlist_ptr->o, 
		ldh_eName_Hierarchy, &np, &size);
	    if ( EVEN(sts)) return sts;
	    strcpy( crrhier_name, np);

	    /* Get the object class */
	    sts = ldh_GetAttrRefTid( ldhses, &crrlist_ptr->o, &crrclass);
	    if ( EVEN(sts)) return sts;

	    sts = ldh_ObjidToName( ldhses, cdh_ClassIdToObjid( crrclass), 
		ldh_eName_Object, crrclass_name, sizeof( crrclass_name), &size);
	    if ( EVEN(sts)) return sts;

	    IF_OUT u_pagebreak( utlctx);
	    if ( crrlist_ptr->specification ) {
	        u_print( utlctx, "     #");
	    }
	    else {
	      u_print( utlctx, "      ");
	    }
	    u_print( utlctx, "  %s", crrhier_name);
	    u_posit( utlctx, 7, strlen(crrhier_name) + 8);
	    u_print( utlctx, " %s", crrclass_name);
	    u_row( utlctx);
	    crrlist_ptr = crrlist_ptr->next;
	  }
	  list_ptr = list_ptr->next;
	}
	IF_OUT u_pageend( utlctx);

	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_crossref()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	Objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	dum1		I	dummy parameter.
* unsigned long	dum2		I	dummy parameter.
* unsigned long	dum3		I	dummy parameter.
* unsigned long	dum4		I	dummy parameter.
*
* Description: 	Backcallroutine for utl_crossref_... routines.
*		The routine is called when traversing the database
*		an object of the desired class and hierachy is found.
*		The crossreference method for the object is called
*		and the returnd crossreferencelist is printed.
*
**************************************************************************/

static int utl_crossref ( 
  ldh_tSesContext ldhses,
  pwr_sAttrRef	*arp,
  utl_ctx	utlctx,
  unsigned long	dum2,
  unsigned long	dum3,
  unsigned long	dum4
)
{
	int		sts;
	utl_t_list	*crrlist;
	int		crrcount;

	/* Call the crossreference method for this objdid */
	sts = crr_crossref( ldhses, arp, &crrlist, &crrcount);
	if ( EVEN(sts)) return sts;

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_externref()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

static int utl_externref (
  utl_ctx	utlctx,
  pwr_tObjid	oid,
  utl_t_list	**crrlist,
  int		*crrcount
)
{
	int		sts, size;
	pwr_tClassId	cid;
	pwr_tClassId	refclass;
	utl_t_objidlist	*childlist;
	int		childlistcount;
	utl_t_objidlist	*list_ptr;
	utl_t_list	*crrlist_ptr;
	crr_t_searchlist    *searchlist_ptr;
	pwr_sAttrRef	*aref_ptr;

	static crr_t_searchlist	searchlist[] = {
		{ pwr_cClass_stodp, "DevBody", "Object", CRR_WRITE, 0 },
		{ pwr_cClass_setdp, "DevBody", "Object", CRR_WRITE, 0 },
		{ pwr_cClass_resdp, "DevBody", "Object", CRR_WRITE, 0 },
		{ pwr_cClass_GetDp, "DevBody", "DpObject", CRR_READ, 0 },
		{ pwr_cClass_cstoap, "DevBody", "Object", CRR_WRITE, 0 },
 		{ pwr_cClass_GetAp, "DevBody", "ApObject", CRR_READ, 0 },
		{ pwr_cClass_stoap, "DevBody", "Object", CRR_WRITE, 0 },
		{ pwr_cClass_CStoIp, "DevBody", "Object", CRR_WRITE, 0 },
 		{ pwr_cClass_GetIp, "DevBody", "IpObject", CRR_READ, 0 },
		{ pwr_cClass_StoIp, "DevBody", "Object", CRR_WRITE, 0 },
 		{ 0, }};


	/* get all the children to the object */
	childlistcount = 0;
	childlist = 0;
	sts = trv_get_children_class_name( utlctx->ldhses, oid, 0, 0, 
		(trv_tBcFunc) utl_objidlist_insert, 
		&childlist, &childlistcount, 0, 0, 0);
	if ( EVEN (sts)) return sts;

	list_ptr = childlist;
	while ( list_ptr) {
	  /* Look if it is a member of the searchlist */
	  sts = ldh_GetObjectClass( utlctx->ldhses, list_ptr->objid, &cid);
	  if ( EVEN(sts)) return sts;

	  searchlist_ptr = searchlist;
	  while ( searchlist_ptr->cid != 0 ) {
	    if ( cid == searchlist_ptr->cid) {
	      /* Check if the objdid in the parameter is correct */
	      sts = ldh_GetObjectPar( utlctx->ldhses, list_ptr->objid, 
			searchlist_ptr->body, 
			searchlist_ptr->attr, (char **)&aref_ptr, &size);
	      if ( EVEN(sts)) return sts;

	      /* Check if objdid is ok */
	      sts = ldh_GetAttrRefTid( utlctx->ldhses, aref_ptr, &refclass);
	      if ( ODD(sts)) {
	        /* Insert object i list */
	        utl_list_insert( crrlist, crrcount, aref_ptr, 
			searchlist_ptr->write, 0, 0);
	        /* Set the current object as refobj */
	        crrlist_ptr = (*crrlist);
	        while ( crrlist_ptr->next)
	          crrlist_ptr = crrlist_ptr->next;
	        crrlist_ptr->refo = cdh_ObjidToAref( list_ptr->objid);
	      }
	      free((char *) aref_ptr);
	    }
	    searchlist_ptr++;
	  }
	  list_ptr = list_ptr->next;
	}
	utl_objidlist_free( childlist);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_signalref()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

static int utl_signalref (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_list	**crrlist,
  int		*crrcount
)
{
	int		sts, size;
	pwr_tClassId	cid;
	pwr_tClassId	refclass;
	utl_t_objidlist	*childlist;
	int		childlistcount;
	int		childcount;
	utl_t_objidlist	*list_ptr;
	utl_t_list	*crrlist_ptr;
	crr_t_searchlist	*searchlist_ptr;
	pwr_sAttrRef	*aref_ptr;

	static crr_t_searchlist	searchlist[] = {
		{ pwr_cClass_resdv, "DevBody", "DvObject", CRR_WRITE, 0},
		{ pwr_cClass_setdv, "DevBody", "DvObject", CRR_WRITE, 0},
		{ pwr_cClass_stodv, "DevBody", "DvObject", CRR_WRITE, 0},
		{ pwr_cClass_GetDv, "DevBody", "DvObject", CRR_READ, 0},
		{ pwr_cClass_GetDo, "DevBody", "DoObject", CRR_READ, 0},
		{ pwr_cClass_resdo, "DevBody", "DoObject", CRR_WRITE, 0},
		{ pwr_cClass_setdo, "DevBody", "DoObject", CRR_WRITE, 0},
		{ pwr_cClass_stodo, "DevBody", "DoObject", CRR_WRITE, 0},
		{ pwr_cClass_GetDi, "DevBody", "DiObject", CRR_READ, 0},
		{ pwr_cClass_cstoav, "DevBody", "AvObject", CRR_WRITE, 0},
		{ pwr_cClass_GetAv, "DevBody", "AvObject", CRR_READ, 0},
		{ pwr_cClass_stoav, "DevBody", "AvObject", CRR_WRITE, 0},
		{ pwr_cClass_cstoao, "DevBody", "AoObject", CRR_WRITE, 0},
		{ pwr_cClass_GetAo, "DevBody", "AoObject", CRR_READ, 0},
		{ pwr_cClass_stoao, "DevBody", "AoObject", CRR_WRITE, 0},
		{ pwr_cClass_GetAi, "DevBody", "AiObject", CRR_READ, 0},
		{ pwr_cClass_pos3p, "DevBody", "DoOpen", CRR_WRITE, 0},
		{ pwr_cClass_pos3p, "DevBody", "DoClose", CRR_WRITE, 0},
		{ pwr_cClass_inc3p, "DevBody", "DoOpen", CRR_WRITE, 0},
		{ pwr_cClass_inc3p, "DevBody", "DoClose", CRR_WRITE, 0},
 		{ 0, }};

	/* get all the children to the object */
	childcount = 0;
	childlist = 0;

	sts = trv_get_children_class_name( utlctx->ldhses, Objdid, 0, 0, 
		(trv_tBcFunc) utl_objidlist_insert, 
		&childlist, &childlistcount, 0, 0, 0);
	if ( EVEN (sts)) return sts;

	*crrlist = 0;
	list_ptr = childlist;
	while ( list_ptr) {
	  /* Look if a member of the searchlist */
	  sts = ldh_GetObjectClass( utlctx->ldhses, list_ptr->objid, &cid);
	  if ( EVEN(sts)) return sts;

	  searchlist_ptr = searchlist;
	  while ( searchlist_ptr->cid != 0 ) {
	    if ( cid == searchlist_ptr->cid) {
	      /* Check if the objdid in the parameter is correct */
	      sts = ldh_GetObjectPar( utlctx->ldhses, list_ptr->objid, 
			searchlist_ptr->body, 
			searchlist_ptr->attr, (char **)&aref_ptr, &size);
	      if ( EVEN(sts)) return sts;

	      /* Check if objdid is ok */
	      sts = ldh_GetAttrRefTid( utlctx->ldhses, aref_ptr, &refclass);
	      if ( ODD(sts)) {
	        /* Insert object i list */
	        utl_list_insert( crrlist, crrcount, aref_ptr,
			searchlist_ptr->write, 1, 0);
	        /* Set the current object as refobj */
	        crrlist_ptr = *crrlist;
	        while ( crrlist_ptr->next)
	          crrlist_ptr = crrlist_ptr->next;
	        crrlist_ptr->refo = cdh_ObjidToAref( list_ptr->objid);
	      }
	      free((char *) aref_ptr);
	   }
	   searchlist_ptr++;
	  }
	  list_ptr = list_ptr->next;
	}

	utl_objidlist_free( childlist);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_childref()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

static int utl_childref (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_list	**crrlist,
  int		*crrcount
)
{
	int		sts;

	sts = crr_crossref_children( utlctx->ldhses, Objdid, crrlist, crrcount);
	if ( EVEN(sts)) return sts;

	return FOE__SUCCESS;
}




#if 0

/*************************************************************************
*
* Name:		utl_create_mainwindow()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

int utl_create_mainwindow (
  int argc,
  char **argv,
  Widget *widget
)
{
  Widget toplevel;
  Widget mainwindow;
  char uid_filename[200] = "pwr_exe:wb.uid";
  char *uid_filename_p = uid_filename;
  Arg args[20];
  int sts;

  MrmInitialize();

  sts = dcli_translate_filename( uid_filename, uid_filename);
  if ( EVEN(sts))
  {
    printf( "** pwr_exe is not defined\n");
    exit(0);
  }

  toplevel = XtInitialize ("AutoPrint", "svn", NULL, 0, &argc, argv);
  XtSetArg (args[0], XtNallowShellResize, TRUE);
  XtSetValues (toplevel, args, 1);

  uilutil_fetch( &uid_filename_p, 1, 0, 0,
		toplevel, "mainwindow", "svn_svn", 0, 0,
		&mainwindow, NULL );

  XtSetArg    (args[0], XmNheight, 500);
  XtSetValues (mainwindow, args, 1);
  XtManageChild(mainwindow);

  *widget = mainwindow;

  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_destroy_mainwindow()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Destroy the main window.
*
**************************************************************************/

int utl_destroy_mainwindow (
Widget widget
)
{
        XtDestroyWidget(XtParent(widget));
	return FOE__SUCCESS;
}
#endif

/*************************************************************************
*
* Name:		utl_print_repage()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
*
* Description: 	
*
**************************************************************************/

int utl_print_repage (
  ldh_tSesContext ldhses
)
{
	int		sts;
	pwr_tClassId	*classp;
	pwr_tClassId	class_vect[2];
	unsigned long	plcpgm_page;
	unsigned long	doc_page;
	utl_t_objidlist	*plcpgmlist;
	utl_t_objidlist	*list_ptr;
	int		plcpgmcount;

	/* Get class document objects */
	class_vect[0] = pwr_cClass_plc;
	class_vect[1] = 0;
	classp = class_vect;

	plcpgmcount = 0;
	plcpgmlist = 0;
	sts = trv_get_objects_hier_class_name( ldhses, pwr_cNObjid, classp, NULL,
		&utl_objidlist_insert, &plcpgmlist, &plcpgmcount,
		0, 0, 0);
	if ( EVEN (sts)) return sts;

	plcpgm_page = 1;
	list_ptr = plcpgmlist;
	while ( plcpgmlist)
	{
	  doc_page = 1;
	  sts = trv_get_docobjects( ldhses, list_ptr->objid, 
		(trv_tBcFunc) utl_repage, ldhses, (void *)plcpgm_page, &doc_page, 0, 0);
	  if ( EVEN (sts)) return sts;
	  plcpgm_page++;
	  list_ptr = list_ptr->next;
	} 
	utl_objidlist_free( plcpgmlist);

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_print_content()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
*
* Description: 
*
**************************************************************************/

int utl_print_content (
  ldh_tSesContext ldhses,
  int		terminal,
  char		*filename
)
{
	int		sts;
	pwr_tObjid	objdid;
	utl_ctx		utlctx;
	utl_t_list	*list_ptr;

	/* Open file */
	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts)) 
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}

	/* Print head */
	IF_OUT u_header( utlctx, ldhses, "LIST OF DOCUMENTS");
	IF_OUT u_row( utlctx);

	sts = trv_get_docobjects( ldhses, pwr_cNObjid,
/*		utl_content, ldhses, utlctx, 0, 0, 0);*/
		(trv_tBcFunc) utl_ctxlist_insert, 
		&(utlctx->list[0]), &(utlctx->listcount[0]), 0, 0, 0);
	if ( EVEN (sts)) return sts;

	utl_list_sort( &utlctx->list[0], utlctx->listcount[0], ldhses, 0);

	list_ptr = utlctx->list[0];
	while( list_ptr)
	{	
	  objdid = list_ptr->o.Objid;
	  utl_content( objdid, ldhses, utlctx, 0, 0, 0);
	  list_ptr = list_ptr->next;
	}
	IF_OUT u_pageend( utlctx);

	u_close( utlctx);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_repage()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	dum1		I	dummy parameter.
* unsigned long	dum2		I	dummy parameter.
* unsigned long	dum3		I	dummy parameter.
* unsigned long	dum4		I	dummy parameter.
*
*
* Description: 	
*
**************************************************************************/

int utl_repage (
  pwr_tObjid  Objdid,
  ldh_tSesContext ldhses,
  unsigned long	plcpgm_page,
  unsigned long	*doc_page,
  unsigned long	dum3,
  unsigned long	dum4
)
{
	int		sts, size;
	char		page_str[20];


	/* Store the page in the Page parameter */

	size = sprintf( page_str, "%ld.%ld", plcpgm_page, *doc_page);
	sts = ldh_SetObjectPar( ldhses,
			Objdid, "DevBody", "Page", page_str, size+1); 
	(*doc_page)++;

	return FOE__SUCCESS;
}



/*************************************************************************
*
* Name:		utl_content()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	dum1		I	dummy parameter.
* unsigned long	dum2		I	dummy parameter.
* unsigned long	dum3		I	dummy parameter.
* unsigned long	dum4		I	dummy parameter.
*
*
* Description: 	
*
**************************************************************************/

static int utl_content ( 
  pwr_tObjid	Objdid,
  ldh_tSesContext ldhses,
  utl_ctx		utlctx,
  unsigned long	dum2,
  unsigned long	dum3,
  unsigned long	dum4
)
{
	int		sts, size, i;
	pwr_tOName     	hier_name;
	char		*page;

	/* Get the page parameter if there is one */
	sts = ldh_GetObjectPar( ldhses, Objdid, "DevBody", 
			"Page", (char **)&page, &size);
	if ( EVEN(sts)) return FOE__SUCCESS;

	/* Print the name of the actual object and page */
	sts = ldh_ObjidToName( 
		ldhses, Objdid, ldh_eName_Hierarchy,
		hier_name, sizeof( hier_name), &size);
	if ( EVEN(sts)) return sts;

	IF_OUT u_pagebreak( utlctx);
	u_print( utlctx, "%s .",hier_name);
	for ( i = strlen( hier_name); i < 55; i++)
	{
	    u_print( utlctx, ".");
	}

	u_print( utlctx, "%6s", page);
	u_row( utlctx);
	free((char *) page);

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_list()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: 	
*
**************************************************************************/

int utl_list (
  ldh_tSesContext ldhses,
  char		*list_str,
  char		*hier_str,
  char		*object_str,
  char		*filename,
  int		print,
  int		allvolumes,
  char		*volumes,
  int		keep_crosslist
)
{
	pwr_tObjid	listobjdid;
	int		i, j;
	int		changed;
	utl_t_listbody	*listbody_ptr;

	char		*hiername;
	char		*classname;
	char		*name;
	char		*parameter;
	int		terminal = 0;
	utl_ctx		utlctx;
	int		sts, size;
	pwr_tClassId	*classp;
	pwr_tObjid	hierobjdid;
	char		*s;
	pwr_sAttrRef    aref;
	char		*t;
	char		elementstr[20];
	int		element[10];
	int		len;
	utl_t_list	*list_ptr;
	char		par_str[10][80];
	char		*par_ptr;
	int		single_object;
	pwr_tClassId	listclass[2] = {0,0};
	utl_t_objidlist	*listobject_list;
	utl_t_objidlist	*listobject_ptr;
	int		listobject_count;
	utl_t_list	*sublist_ptr;
	char		class_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tClassId	class_vect[UTL_INPUTLIST_MAX + 1];
	int		nr;
	int		new_row;
	int		print_ok;
	pwr_tCmd       	cmd;
	int		page;
	int		first;
	char		vol_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tVolumeId	volume_vect[UTL_INPUTLIST_MAX + 1];
	pwr_tVolumeId	*volume_p;
	trv_tCtx       	trvctx;
	pwr_tClassId	vol_class;
	pwr_tVolumeId	vol_id;
	char		volumestr[250];
	char		volumename[80];
	ldh_sVolumeInfo info;
	
	/* Get the node objdid */
	sts = ldh_NameToObjid( ldhses, &listobjdid, list_str);
	if ( EVEN(sts)) 
	{
	  return FOE__LISTOBJECT;
	}	

	/* Read the list object */
	sts = ldh_GetObjectBody( ldhses, listobjdid, "DevBody", (void **)&listbody_ptr, &size);
	if ( EVEN(sts)) return sts;

	if ( *filename == '\0')
	{
	  /* No file is supplied, create one */
	  utl_get_filename( filename);
	}

	/* If object is specified, this has priority over name */
	if ( object_str != NULL)
	  name = object_str;
	else if ( strcmp( (listbody_ptr->Name), "") == 0)
	  name = NULL;
	else
	  name = listbody_ptr->Name;

	if ( strcmp( (listbody_ptr->Class), "") == 0)
	  classname = NULL;
	else
	  classname = listbody_ptr->Class;

	/* If hierarchy is defined in cli, take this as hierarchy object */
	if ( hier_str != NULL)
	  hiername = hier_str;
	else if ( strcmp( (listbody_ptr->Hierarchyobject), "") == 0)
	  hiername = NULL;
	else
	  hiername = listbody_ptr->Hierarchyobject;
	if ( strcmp( (listbody_ptr->Parameter), "") == 0)
	  parameter = NULL;
	else
	  parameter = listbody_ptr->Parameter;

	if ( name != NULL )
	{
	  if ( (strchr( name, '*') != 0) && (classname == NULL) &&
		(parameter != NULL))
	  {
	    /* Wildcard and no class is not allowed for show parameter */
	    return FOE__CLASSQUAL;
	  }
	}

	/* Check if class */
	if ( classname != NULL )
	{
	  nr = utl_parse( classname, ", ", "", (char *)class_str, 
		sizeof( class_str) / sizeof( class_str[0]), sizeof( class_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
	    return FOE__CLASSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    sts = ldh_ClassNameToId( ldhses, &class_vect[i], class_str[i]);
	    if ( EVEN(sts))
	    {
	      return FOE__CLASSNAME;
	    }
	  }
	  class_vect[nr] = 0;
	  classp = class_vect;
	}	
	else
	  classp = 0;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
	  if ( EVEN(sts))
	  {
	    return FOE__HIERNAME;
	  }	
	}
	else
	  hierobjdid = pwr_cNObjid;

	if ( parameter != NULL )
	{
	  utl_toupper( parameter, parameter);
	  nr = utl_parse( parameter, ", ", "", (char *)par_str, 
		sizeof( par_str) / sizeof( par_str[0]), sizeof( par_str[0]));
	  par_str[nr][0] = 0; 
	  if ( nr == 0)
	    return FOE__PARSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    /* Check index in parameter */
	    s = strchr( (char *)par_str[i], '[');
	    if ( s == 0)
	      element[i] = 0;
	    else
	    {
	      t = strchr( (char *)par_str[i], ']');
	      if ( t == 0)
	      {
	        return FOE__PARSYNT;
	      }
	      else
	      {
	        len = t - s - 1;
	        strncpy( elementstr, s + 1, len);
	        elementstr[ len ] = 0;
	        sscanf( elementstr, "%d", &element[i]);
	        *s = '\0';
	        if ( (element[i] < 0) || (element[i] > 100) )
	        {
	          return FOE__PARELSYNT;
	        }
	      }
	    }
	  }
	  par_ptr = (char *)par_str;
	}
	else
	  par_ptr = NULL;
	
	if ( volumes != NULL)
	{
	  /* Parse the volumestr */
	  nr = utl_parse( volumes, ", ", "", (char *)vol_str, 
		sizeof( vol_str) / sizeof( vol_str[0]), sizeof( vol_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
	    return FOE__PARSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    sts = ldh_VolumeNameToId( ldh_SessionToWB( ldhses), vol_str[i],
			&volume_vect[i]);
	    if ( EVEN(sts)) return sts;
	  }
	  volume_vect[nr] = 0;
	  volume_p = volume_vect;
	}
	else
	  volume_p = 0;
	
	if ( allvolumes)
	{
	  /* Get all volumes that is not class and wb volumes */
	  i = 0;
	  sts = ldh_GetVolumeList( ldh_SessionToWB( ldhses), &vol_id);
	  while ( ODD(sts) )
	  {
	    sts = ldh_GetVolumeClass( ldh_SessionToWB( ldhses), vol_id,
			&vol_class);
	    if (EVEN(sts)) return sts;

	    if ( !(vol_class == pwr_eClass_ClassVolume ||
 	        vol_class == pwr_eClass_WorkBenchVolume ))
	    {
	      volume_vect[i] = vol_id;
	      i++;
	      if ( i > UTL_INPUTLIST_MAX)
	        return FOE__PARSYNT;
	    }
	    sts = ldh_GetNextVolume( ldh_SessionToWB( ldhses), vol_id, &vol_id);
	  }
	  volume_vect[i] = 0;
	  volume_p = volume_vect;
	}

	/* Create a string of the volumes for presentation */
	if ( volume_p)
	{
	  i = 0;
	  strcpy( volumestr, "");
	  while( volume_vect[i])
	  {
	    sts = ldh_VolumeIdToName( ldh_SessionToWB( ldhses), volume_vect[i],
			volumename, sizeof(volumename), &size);
	    if ( i != 0)
	      strncat( volumestr, ", ", 
		sizeof(volumestr)-strlen(volumestr)-1);
	    strncat( volumestr, volumename,
		sizeof(volumestr)-strlen(volumestr)-1);
	    i++;
	  }
	}
	else
	{
	  /* Get the name of the current volume */
	  sts = ldh_GetVolumeInfo( ldh_SessionToVol(ldhses), &info);
	  if ( EVEN(sts)) return sts;

	  sts = ldh_VolumeIdToName( ldh_SessionToWB( ldhses), info.Volume,
			volumename, sizeof(volumename), &size);
	  if ( EVEN(sts)) return sts;
	  strcpy( volumestr, volumename);
	}

	utl_ctx_new( &utlctx, ldhses, "", listbody_ptr->Landscape);

	single_object = 0;

	/* Check if name */
	if ( name != NULL )
	{
	  /* Check that name includes a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Print this object */
	    /* Get objdid for the object */
	    sts = ldh_NameToAttrRef( ldhses, name, &aref);
	    if ( EVEN(sts))
	    {
	      return FOE__OBJECT;
	    }
	   
	    utl_ctxlist_insert( &aref,  
		&(utlctx->list[0]), &(utlctx->listcount[0]), 0, 0, 0);
	    single_object = 1;
	   
	  }
	}

	if ( !single_object)
	{
	  if ( listbody_ptr->Externreference == 4)
	  {
	    /* List all object in cross referencelist that is not signals */
	    sts = crr_refobject( utlctx, hierobjdid, &(utlctx->list[0]), 
			&(utlctx->listcount[0]));
	    if ( EVEN(sts)) return sts;
	  }
	  else
	  {
	    /* Search for objects */
	    sts = trv_create_ctx( &trvctx, ldhses, hierobjdid, classp, name,
		volume_p);
	    if ( EVEN(sts)) return sts;

	    sts = trv_object_search( trvctx,
		(trv_tBcFunc) utl_ctxlist_insert,
		&(utlctx->list[0]), &(utlctx->listcount[0]), 0, 0, 0);
	    if ( EVEN (sts)) return sts;

	    sts = trv_delete_ctx( trvctx);
	  }
	}

	/* Get child listobjects */
	sts = ldh_ClassNameToId( ldhses, &listclass[0], "ListDescriptor");
	if ( EVEN(sts)) return sts;

	listobject_list = 0;
	listobject_count = 0;
	sts = trv_get_children_class_name( ldhses, listobjdid, listclass, 0, 
		(trv_tBcFunc) utl_objidlist_insert, 
		&listobject_list, &listobject_count, 0, 0, 0);
	if ( EVEN (sts)) return sts;
	
	listobject_ptr = listobject_list;
	for ( i = 0; i < co_min( listobject_count, UTL_LIST_MAX); i++)
	{
	  sublist_ptr = utlctx->list[0];
	  while( sublist_ptr)
	  {
	    sts = utl_list_sublist( utlctx, listobject_ptr->objid, 
		&(sublist_ptr->sublist[i]), &(sublist_ptr->sublistcount[i]),
		&sublist_ptr->o);
            if ( EVEN(sts)) return sts;
	    sublist_ptr = sublist_ptr->next;
	  }
	  listobject_ptr = listobject_ptr->next;
	}
	if ( !keep_crosslist)
	  cross_crosslist_unload();

	/* Open file */
	sts = u_open( utlctx, filename, terminal, 0);
	if ( EVEN(sts))
	{
	  utl_ctx_delete( utlctx);
	  return sts;
	}
	strcpy( (utlctx->page_title), listbody_ptr->PageHeader);

	if ( !listbody_ptr->TableOfContents )
	{
	  /* No page title of first page */
	  utlctx->page_first = 1;
	  u_header( utlctx, ldhses, listbody_ptr->Title);
	  u_subheader( utlctx, "Volume", volumestr);
	  u_subheader( utlctx, "Descriptor", list_str);
	  if ( object_str != 0)
	    u_subheader( utlctx, "Object", object_str);
	  if ( hier_str != 0)
	    u_subheader( utlctx, "Hierarchy", hier_str);
	  u_row( utlctx);
	}
	else
	{
	  utlctx->page_first = 0;
	  u_pagebreak( utlctx);
	}

	if ( listbody_ptr->ColumnHeader )
	  utl_list_print_columnheader( utlctx, listbody_ptr);

	if ( listbody_ptr->AlphaOrder)
	  utl_list_sort( &utlctx->list[0], utlctx->listcount[0], ldhses,
		listbody_ptr->AlphaOrder);
	
	first = 1;
	list_ptr = utlctx->list[0];
	while( list_ptr)
	{	
	  pwr_sAttrRef *arefp = &list_ptr->o;

	  print_ok = 1;
	  if ( listbody_ptr->NoPrint)
	    print_ok = 0;

	  if ( listbody_ptr->NoPrintIfNoList)
	  {
	    print_ok = 0;
	    for ( j = 0; j < co_min( listobject_count, UTL_LIST_MAX); j++)
	    {
	      if ( list_ptr->sublistcount[j] !=  0)
	      {
	        print_ok = 1;
	        break;
	      }
	    }
	  }

	  page = utlctx->page;

	  if ( print_ok)
	  {
	    /* Print this object */

	    /* Print a columnheader at pagebreak */
	    if ( listbody_ptr->ColumnHeader )
	      utlctx->listbody = listbody_ptr;

	    /* If close to pagebreak, force a pagebreak if specified */
	    if ( (utlctx->rows - utlctx->row) < listbody_ptr->RowsToPageBreak)
	      u_force_pagebreak( utlctx);

	    /* Print pagebreak if specified */
	    if ( (listbody_ptr->PageBreak) && ( !first))
	    {
	      u_force_pagebreak( utlctx);
	      if ( listbody_ptr->PageBreak == 2)
	      {
	        /* Reset page nr */
	        utlctx->page = 1;	      
	        page = utlctx->page;
	      }
	    }

	    u_pagebreak( utlctx);

	    /* Print clear rows if specified */
	    if ( listbody_ptr->Full != UTL_FULLPRINT_COND )
	    {
	      for ( j = 0; j < listbody_ptr->ClearRowsPre; j++)
	      {
	        u_pagebreak( utlctx);
	        u_row( utlctx);
	      }
	    }
	    page = utlctx->page;
	    new_row = 1;
	    for ( j = 0; j < UTL_PARDESCRIPTION; j++)
	    {
	      sts = utl_list_print_par( utlctx, arefp,
			list_ptr->specification, &list_ptr->refo,
			listbody_ptr, j, new_row);
	      new_row = listbody_ptr->ParDescription[j].CarriageRet;
	      if ( listbody_ptr->ParDescription[j].Parameter[0] == 0)
	        break;
	    }
	    if ( j == 0)
	    {
	      if ( listbody_ptr->Full == UTL_FULLPRINT_COND )
	      {
	        /* No print if object not modified */
	        sts = utl_object_changed( arefp->Objid, ldhses, utlctx, 
			&changed, listbody_ptr->Full);
	        if ( EVEN(sts)) return sts;
	        if ( changed )
	        {
	          for ( j = 0; j < listbody_ptr->ClearRowsPre; j++)
	          {
	            u_pagebreak( utlctx);
	            u_row( utlctx);
	          }
	        }
	      }
	      else
	        changed = 1;

	      if ( changed)
	      {
	        sts = utl_print_aref( arefp, ldhses, utlctx, 0, 0, 0);
	        if ( EVEN(sts)) return sts;
	      }
	    }
	    if ( listbody_ptr->Full )
	    {
	      if ( changed)
	      {
	        sts = utl_print_object_full( arefp, ldhses, utlctx, 
			listbody_ptr->Full);
	        if ( EVEN(sts)) return sts;
	      }
	    }
	    else if ( parameter != NULL)
	    {
	      sts = utl_print_object_par( arefp, ldhses, utlctx, par_ptr, 
			element);
	      if ( EVEN(sts)) return sts;
	    }

	    /* Print clear rows if specified */
	    if (( listbody_ptr->Full != UTL_FULLPRINT_COND ) ||
	       (( listbody_ptr->Full == UTL_FULLPRINT_COND ) && changed)) 
	    {
	      for ( j = 0; j < listbody_ptr->ClearRowsPost; j++)
	      {
	        u_pagebreak( utlctx);
	        u_row( utlctx);
	      }
	    }
	    first = 0;
	  }

	  if ( (listbody_ptr->TableOfContents == 2) ||
	     ( (listbody_ptr->TableOfContents == 1) && print_ok))
	  {
	    sts = utl_tableofcont_insert( utlctx, arefp, 
		listbody_ptr->TCSegments, listbody_ptr->TCMarginString, 
		page);
	  }

	  listobject_ptr = listobject_list;
	  for ( j = 0; j < co_min( listobject_count, UTL_LIST_MAX); j++)
	  {
	    sts = utl_list_sublist_print( utlctx, listobject_ptr->objid, 
		list_ptr->sublist[j], list_ptr->sublistcount[j], &first); 
	    if ( EVEN(sts)) return sts;
	    listobject_ptr = listobject_ptr->next;
	  }
	  list_ptr = list_ptr->next;
	}

	if ( listbody_ptr->TableOfContents )
	{
	  u_force_pagebreak( utlctx);
	  u_header( utlctx, ldhses, listbody_ptr->Title);
	  u_subheader( utlctx, "Descriptor", list_str);
	  if ( object_str != 0)
	    u_subheader( utlctx, "Object", object_str);
	  if ( hier_str != 0)
	    u_subheader( utlctx, "Hierarchy", hier_str);
	  u_row( utlctx);

	  utl_tableofcont_print( utlctx);
	}
	else
	  IF_OUT u_pageend( utlctx);

	u_close( utlctx);

	if ( print)
	{
	  /* Send the file to the printer */
	  if ( utlctx->landscape)
	    sprintf( cmd, "pwr_foe_print_land %s", filename);
	  else
	    sprintf( cmd, "pwr_foe_print %s", filename);
	  sts = system( cmd);
	  if ( EVEN(sts))
	  {
	    utl_objidlist_free( listobject_list);
	    free((char *) listbody_ptr);
	    utl_ctx_delete( utlctx);
	    return sts;
	  }
	}

	cross_doclist_unload();
	free((char *) listbody_ptr);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}	



/*************************************************************************
*
* Name:		utl_list_sublist()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

static int utl_list_sublist (
  utl_ctx	utlctx,
  pwr_tObjid	listobjdid,
  utl_t_list	**list,
  int		*listcount,
  pwr_sAttrRef	*rootarp
)
{
	int		i, j;
	utl_t_listbody	*listbody_ptr;

	char		*hiername;
	char		*classname;
	char		*name;
	char		*parameter;
	int		sts, size;
	pwr_tClassId	*classp;
	char		*s;
	pwr_tClassId	listclass[2] = {0,0};
	utl_t_objidlist	*listobject_list;
	utl_t_objidlist	*listobject_ptr;
	int		listobject_count;
	utl_t_list	*sublist_ptr;
	char		class_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tClassId	class_vect[UTL_INPUTLIST_MAX + 1];
	int		nr;
	
	/* Read the list object */
	sts = ldh_GetObjectBody( utlctx->ldhses, listobjdid, "DevBody", 
		    (void **)&listbody_ptr, &size);
	if ( EVEN(sts)) return sts;

	if ( strcmp( (listbody_ptr->Name), "") == 0)
	  name = NULL;
	else
	  name = listbody_ptr->Name;
	if ( strcmp( (listbody_ptr->Class), "") == 0)
	  classname = NULL;
	else
	  classname = listbody_ptr->Class;
	if ( strcmp( (listbody_ptr->Hierarchyobject), "") == 0)
	  hiername = NULL;
	else
	  hiername = listbody_ptr->Hierarchyobject;
	if ( strcmp( (listbody_ptr->Parameter), "") == 0)
	  parameter = NULL;
	else
	  parameter = listbody_ptr->Parameter;

	if ( name != NULL )
	{
	  if ( (strchr( name, '*') != 0) && (classname == NULL) &&
		(parameter != NULL))
	  {
	    /* Wildcard and no class is not allowed for show parameter */
	    return FOE__CLASSQUAL;
	  }
	}

	/* Check if class */
	if ( classname != NULL )
	{
	  nr = utl_parse( classname, ", ", "", (char *)class_str, 
		sizeof( class_str) / sizeof( class_str[0]), sizeof( class_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
	    return FOE__CLASSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    sts = ldh_ClassNameToId( utlctx->ldhses, &class_vect[i], 
		class_str[i]);
	    if ( EVEN(sts))
	    {
	      return FOE__CLASSNAME;
	    }
	  }
	  class_vect[nr] = 0;
	  classp = class_vect;
	}	
	else
	  classp = 0;

	/* Check if name */
	if ( name != NULL )
	{
	  /* Check that name includes a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    return FOE__NOWILD;
	  }
	}

	if ( listbody_ptr->Crossreference)
	{
	  /* Call the crossreference method for this objdid */
	  sts = crr_crossref( utlctx->ldhses, rootarp, list, 
		listcount);
	  if ( EVEN(sts)) return sts;
	}
	else if ( listbody_ptr->Externreference == 1)
	{
	  /* Call the externreference for this objdid */
	  sts = utl_externref( utlctx, rootarp->Objid, list, listcount);
	  if ( EVEN(sts)) return sts;
	}
	else if ( listbody_ptr->Externreference == 2)
	{
	  /* Call the externreference for this objdid */
	  sts = utl_signalref( utlctx, rootarp->Objid, list, listcount);
	  if ( EVEN(sts)) return sts;
	}
	else if ( listbody_ptr->Externreference == 3)
	{
	  /* Call the externreference for this objdid */
	  sts = utl_childref( utlctx, rootarp->Objid, list, listcount);
	  if ( EVEN(sts)) return sts;
	}
	else if ( listbody_ptr->Externreference == 4)
	{
	  /* List all object in referencelist that is node objects */
	  sts = crr_refobject( utlctx, rootarp->Objid, list, listcount);
	  if ( EVEN(sts)) return sts;
	}
	else if ( listbody_ptr->Deep)
	{
	  if ( classp) {
	    sts = trv_get_attrobjects( utlctx->ldhses,
		rootarp->Objid, classp, name, trv_eDepth_Deep,
		(trv_tBcFunc) utl_ctxlist_insert, list, listcount, 0, 0, 0);
	  }
	  else {
	    /* Search for objects */
	    sts = trv_get_objects_hier_class_name( utlctx->ldhses, 
		rootarp->Objid, classp, name, 
		(trv_tBcFunc) utl_ctxlist_insert, 
		list, listcount, 0, 0, 0);
	  }
	  if ( EVEN (sts)) return sts;
	}
	else {
	  /* Search for children */
	  if ( classp)
	    trv_get_attrobjects( utlctx->ldhses,
		rootarp->Objid, classp, name, trv_eDepth_Children,
		(trv_tBcFunc) utl_ctxlist_insert, list, listcount, 0, 0, 0);
	  else
	    sts = trv_get_children_class_name( utlctx->ldhses, 
		rootarp->Objid, classp, name, 
		(trv_tBcFunc) utl_ctxlist_insert, 
		list, listcount, 0, 0, 0);
	  if ( EVEN (sts)) return sts;
	}

	if ( listbody_ptr->AlphaOrder)
	  utl_list_sort( list, *listcount, utlctx->ldhses, 0); 

	/* Get child listobjects */
	sts = ldh_ClassNameToId( utlctx->ldhses, &listclass[0], 
		"ListDescriptor");
	if ( EVEN(sts)) return sts;

	listobject_count = 0;
	listobject_list = 0;
	sts = trv_get_children_class_name( utlctx->ldhses, listobjdid, listclass, 0, 
		(trv_tBcFunc) utl_objidlist_insert, 
		&listobject_list, &listobject_count, 0, 0, 0);
	if ( EVEN (sts)) return sts;
	sublist_ptr = *list;
	while ( sublist_ptr)
	{
	  listobject_ptr = listobject_list;
	  for ( j = 0; j < co_min( listobject_count, UTL_LIST_MAX); j++)
	  {
	    sts = utl_list_sublist( utlctx, listobject_ptr->objid, 
		&(sublist_ptr->sublist[j]), 
		&(sublist_ptr->sublistcount[j]), &sublist_ptr->o);
	    if ( EVEN(sts)) return sts;
	    listobject_ptr = listobject_ptr->next;	
	  }
	  sublist_ptr = sublist_ptr->next;
	}
	utl_objidlist_free( listobject_list);
	free((char *) listbody_ptr);

	return FOE__SUCCESS;
}	

/*************************************************************************
*
* Name:		utl_list_sublist_print()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

static int utl_list_sublist_print ( 
  utl_ctx	utlctx,
  pwr_tObjid	listobjdid,
  utl_t_list	*list,
  int		listcount,
  int		*first
)
{
	int		i, j, k;
	int		changed;
	utl_t_listbody	*listbody_ptr;

	char		*parameter;
	int		sts, size;
	char		*s;
	char		*t;
	char		elementstr[20];
	int		element[10];
	int		len;
	utl_t_list	*list_ptr;
	char		par_str[10][80];
	char		*par_ptr;
	int		nr;
	pwr_tClassId	listclass[2] = {0,0};
	utl_t_objidlist	*listobject_list;
	utl_t_objidlist	*listobject_ptr;
	int		listobject_count;
	int		new_row;
	int		print_ok;
	int		printlist_ok;
	int		page;
	
	/* Read the list object */
	sts = ldh_GetObjectBody( utlctx->ldhses, listobjdid, "DevBody", (void **)&listbody_ptr, &size);
	if ( EVEN(sts)) return sts;

	if ( strcmp( (listbody_ptr->Parameter), "") == 0)
	  parameter = NULL;
	else
	  parameter = listbody_ptr->Parameter;

	if ( parameter != NULL )
	{
	  utl_toupper( parameter, parameter);
	  nr = utl_parse( parameter, ", ", "", (char *)par_str, 
		sizeof( par_str) / sizeof( par_str[0]), sizeof( par_str[0]));
	  par_str[nr][0] = 0; 
	  if ( nr == 0)
	    return  FOE__PARSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    /* Check index in parameter */
	    s = strchr( (char *)par_str[i], '[');
	    if ( s == 0)
	      element[i] = 0;
	    else
	    {
	      t = strchr( (char *)par_str[i], ']');
	      if ( t == 0)
	      {
	        return FOE__PARSYNT;
	      }
	      else
	      {
	        len = t - s - 1;
	        strncpy( elementstr, s + 1, len);
	        elementstr[ len ] = 0;
	        sscanf( elementstr, "%d", &element[i]);
	        *s = '\0';
	        if ( (element[i] < 0) || (element[i] > 100) )
	        {
	          return FOE__PARELSYNT;
	        }
	      }
	    }
	  }
	  par_ptr = (char *)par_str;
	}
	else
	  par_ptr = NULL;
	
	/* Get child listobjects */
	sts = ldh_ClassNameToId( utlctx->ldhses, &listclass[0], 
		"ListDescriptor");
	if ( EVEN(sts)) return sts;

	listobject_count = 0;
	listobject_list = 0;
	sts = trv_get_children_class_name( utlctx->ldhses, listobjdid, 
		listclass, 0, (trv_tBcFunc) utl_objidlist_insert, 
		&listobject_list, &listobject_count, 0, 0, 0);
	if ( EVEN (sts)) return sts;

	if ( listbody_ptr->AlphaOrder)
	  utl_list_sort( &list, listcount, utlctx->ldhses, 
		listbody_ptr->AlphaOrder);

	if ( listcount > 0)
	{
	  /* Check if any object will written, if not don't write header */
	  if ( listbody_ptr->NoPrintIfNoList || listbody_ptr->NoPrint)
	  {
	    list_ptr = list;
	    while( list_ptr)
	    {
	      printlist_ok = 0;
	      for ( j = 0; j < co_min( listobject_count, UTL_LIST_MAX); j++)
	      {
	        if ( list_ptr->sublistcount[j] !=  0)
	        {
	          printlist_ok = 1;
	          break;
	        }
	      }
	      if ( printlist_ok) 
	        break;
	      list_ptr = list_ptr->next;
	    }
	  }
	  else
	    printlist_ok = 1;

	  if ( printlist_ok)
	  {
	    for ( j = 0; j < listbody_ptr->ClearRowsPreList; j++)
	    {
	      u_row( utlctx);
	      u_pagebreak( utlctx);
	    }
	    if ( listbody_ptr->ColumnHeader )
 	      utl_list_print_columnheader( utlctx, listbody_ptr);
	  }
	}

	list_ptr = list;
	while ( list_ptr)
	{
	  print_ok = 1;
	  if ( listbody_ptr->NoPrint)
	    print_ok = 0;

	  if ( listbody_ptr->NoPrintIfNoList)
	  {
	    for ( j = 0; j < co_min( listobject_count, UTL_LIST_MAX); j++)
	    {
	      print_ok = 0;
	      if ( list_ptr->sublistcount[j] !=  0)
	      {
	        print_ok = 1;
	        break;
	      }
	    }
	  }

	  page = utlctx->page;
	  if ( print_ok)
	  {
	    /* Print this objekt */

	    /* Print a columnheader at pagebreak */
	    if ( listbody_ptr->ColumnHeader )
	      utlctx->listbody = listbody_ptr;

	    /* If close to pagebreak, force a pagebreak if specified */
	    if ( (utlctx->rows - utlctx->row) < listbody_ptr->RowsToPageBreak)
	      u_force_pagebreak( utlctx);

	    /* Print pagebreak if specified */
	    if ( (listbody_ptr->PageBreak) && ( !(*first)))
	    {
	      u_force_pagebreak( utlctx);
	      if ( listbody_ptr->PageBreak == 2)
	      {
	        /* Reset page nr */
	        utlctx->page = 1;	      
	        page = utlctx->page;
	      }
	    }

	    u_pagebreak( utlctx);

	    /* Print clear rows if specified */
	    if ( listbody_ptr->Full != UTL_FULLPRINT_COND )
	    {
	      for ( j = 0; j < listbody_ptr->ClearRowsPre; j++)
	      {
	        u_pagebreak( utlctx);
	        u_row( utlctx);
	      }
	    }

	    page = utlctx->page;
	    new_row = 1;
	    for ( j = 0; j < UTL_PARDESCRIPTION; j++)
	    {
	      sts = utl_list_print_par( utlctx, &list_ptr->o,
			list_ptr->specification, &list_ptr->refo,
			listbody_ptr, j, new_row);
	      new_row = listbody_ptr->ParDescription[j].CarriageRet;
	      if ( listbody_ptr->ParDescription[j].Parameter[0] == 0)
	        break;
	    }
	    if ( j == 0)
	    {
	      if ( listbody_ptr->Full & UTL_FULLPRINT_COND )
	      {
	        /* No print if object not modified */
	        sts = utl_object_changed( list_ptr->o.Objid, utlctx->ldhses, 
					  utlctx, &changed, listbody_ptr->Full);
	        if ( EVEN(sts)) return sts;
	        if ( changed )
	        {
	          for ( j = 0; j < listbody_ptr->ClearRowsPre; j++)
	          {
	            u_pagebreak( utlctx);
	            u_row( utlctx);
	          }
	        }
	      }
	      else
	        changed = 1;

	      if ( changed)
	      {
	        sts = utl_print_aref( &list_ptr->o, utlctx->ldhses, utlctx, 0, 0, 0);
	        if ( EVEN(sts)) return sts;
	      }
	    }
	    if ( listbody_ptr->Full )
	    {
	      if ( changed)
	      {
	        sts = utl_print_object_full( &list_ptr->o, utlctx->ldhses, utlctx,
			listbody_ptr->Full);
	        if ( EVEN(sts)) return sts;
	      }
	    }
	    else if ( parameter != NULL)
	    {
	      utl_print_object_par( &list_ptr->o, utlctx->ldhses, utlctx, par_ptr, 
			element);
	      if ( EVEN(sts)) return sts;
	    }

	    /* Print clear rows if specified */
	    if (( listbody_ptr->Full != UTL_FULLPRINT_COND ) ||
	       (( listbody_ptr->Full == UTL_FULLPRINT_COND ) && changed)) 
	    {
	      for ( j = 0; j < listbody_ptr->ClearRowsPost; j++)
	      {
	        u_pagebreak( utlctx);
	        u_row( utlctx);
	      }
	    }
	    *first = 0;
	  }

	  if ( (listbody_ptr->TableOfContents == 2) ||
	       ( (listbody_ptr->TableOfContents == 1) && print_ok))
	  {
	    sts = utl_tableofcont_insert( utlctx, &list_ptr->o, 
		listbody_ptr->TCSegments, listbody_ptr->TCMarginString, 
		page);
	  }
	  listobject_ptr = listobject_list;
	  for ( k = 0; k < co_min( listobject_count, UTL_LIST_MAX); k++)
	  {
	    utl_list_sublist_print( utlctx, listobject_ptr->objid, 
		list_ptr->sublist[k], 
		list_ptr->sublistcount[k], first); 
	    listobject_ptr = listobject_ptr->next;
	  }
	  list_ptr = list_ptr->next;
	}
	if ( printlist_ok)
	{
	  /* Print clear rows if specified */
	  for ( j = 0; j < listbody_ptr->ClearRowsPostList; j++)
	  {
	    u_row( utlctx);
	    u_pagebreak( utlctx);
	  }	    
	}
	if ( listbody_ptr->ColumnHeader )
	  utlctx->listbody = 0;

	utl_objidlist_free( listobject_list);
	free((char *) listbody_ptr);
	return FOE__SUCCESS;
}	



/*************************************************************************
*
* Name:		utl_list_print_par()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

static int utl_list_print_par (
  utl_ctx	utlctx,
  pwr_sAttrRef	*o,
  int		specification,
  pwr_sAttrRef	*refo,
  utl_t_listbody  *listbody_ptr,
  int		parindex,
  int		new_row
)
{
	char		text[1200];
	pwr_tAName     	hier_name;
	int		sts, size;
	pwr_tClassId	cid;
	utl_t_listpar	*list_pardesc;
	char		*parameter;
	pwr_sAttrRef	*conobj_ptr;
	unsigned char	*write_ptr;
	char		page[8];
	char		*np;

	list_pardesc = &(listbody_ptr->ParDescription[ parindex]); 
	if ( list_pardesc->Parameter[0] == 0)
	  return FOE__SUCCESS;
	
	u_pagebreak( utlctx);
	if ( new_row)
	  strcpy( text, list_pardesc->MarginString);
	else
	  strcpy( text, "");

	if ( strcmp( (list_pardesc->Parameter), "PrintObjName") == 0)
	{
	  /* If crossreference list, print read or write */
	  if ( (listbody_ptr->Crossreference) || 
		(listbody_ptr->Externreference == 3))
	  {
	    if ( specification == CRR_WRITE)
	      strcat( text, "# ");
	    else if ( specification == CRR_REF)
	      strcat( text, "& ");
	    else
	      strcat( text, "  ");
	  }

	  if ( cdh_ObjidIsNull( o->Objid))
	    strcat( text, "-");
          else
	  {
	    /* Print the name of the object */
	    sts = ldh_AttrRefToName( utlctx->ldhses, o, 
				     ldh_eName_Aref, &np, &size);
	    if ( EVEN(sts))
	      strcpy( hier_name, "-");
	    else
	      strcpy( hier_name, np);
	    if ( list_pardesc->PrintParName)
	      strcat( text, "Object Name = ");
	    if ( list_pardesc->Segments != 0)
	      utl_cut_segments( hier_name, hier_name, list_pardesc->Segments);

	    strcat( text, hier_name);
	  }
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintObjClass") == 0)
	{
	  /* Print the name of the object */
	  sts = ldh_GetAttrRefTid( utlctx->ldhses, o,  &cid);
	  if ( ODD(sts))
	    sts = ldh_ObjidToName( utlctx->ldhses, cdh_ClassIdToObjid( cid),
		ldh_eName_Object, hier_name, sizeof( hier_name), &size);
	  if ( EVEN(sts))
	    strcpy( hier_name, "-");
	  if ( list_pardesc->PrintParName)
	    strcat( text, "Class = ");
	  strcat( text, hier_name);
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintNode") == 0)
	{
	  /* Print the name of the object */
	  strcat( text, "* Obsolete * ");
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintVolume") == 0)
	{
	  /* Print the name of the volume */
	  if ( list_pardesc->PrintParName)
	    strcat( text, "Volume = ");

	  sts = ldh_VolumeIdToName( ldh_SessionToWB( utlctx->ldhses),
			o->Objid.vid, hier_name, sizeof( hier_name), &size);
	  if ( EVEN(sts)) 
	    strcat( text, "-");
	  else
	    strcat( text, hier_name);
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintObjRefPar") == 0)
	{
	  /* Print the name of the object and the referenced parameter */
	  if ( cdh_ObjidIsNull( o->Objid))
	    strcat( text, "-");
          else
	  {
	    sts = ldh_AttrRefToName( utlctx->ldhses, o, ldh_eName_Aref,
				     &np, &size);
	    strcpy( hier_name, np);
	    if ( list_pardesc->PrintParName)
	      strcat( text, "Object Name = ");
	    if ( list_pardesc->Segments != 0)
	      utl_cut_segments( hier_name, hier_name, list_pardesc->Segments);

	    strcat( text, hier_name);

	    /* Print the parameter if the object has a parameter named parameter*/
	    sts = ldh_GetAttrObjectPar( utlctx->ldhses, refo, "DevBody", 
			"Parameter", (char **)&parameter, &size);
	    if ( ODD(sts))
	    {
	      strcat( text, ".");
	      strcat( text, parameter);
	      free((char *) parameter);
	    }
   	  }
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintRefObjPar") == 0)
	{
	  /* Print the name of the referenced object */
	  if ( cdh_ObjidIsNull( refo->Objid))
	    strcat( text, "-");
          else
          {
	    sts = ldh_AttrRefToName( utlctx->ldhses, refo, ldh_eName_Aref,
		&np, &size);
	    strcpy( hier_name, np);
	    if ( list_pardesc->PrintParName)
	      strcat( text, "Refobject Name = ");
	    if ( list_pardesc->Segments != 0)
	      utl_cut_segments( hier_name, hier_name, list_pardesc->Segments);

	    strcat( text, hier_name);

	    /* Print the parameter if the object has a parameter named parameter*/
	    sts = ldh_GetAttrObjectPar( utlctx->ldhses, o, "DevBody", 
			"Parameter", (char **)&parameter, &size);
	    if ( ODD(sts))
	    {
	      strcat( text, ".");
	      strcat( text, parameter);
	      free((char *) parameter);
	    }
 	  }
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintRefObj") == 0)
	{
	  /* Print the name of the referenced object */
	  if ( cdh_ObjidIsNull( refo->Objid))
	    strcat( text, "-");
          else
	  {
	    sts = ldh_AttrRefToName( utlctx->ldhses, refo, ldh_eName_Aref,
		&np, &size);
	    strcpy( hier_name, np);
	    if ( EVEN(sts))
	      strcpy( hier_name, "-");
	    if ( list_pardesc->PrintParName)
	      strcat( text, "Refobject Name = ");
	    if ( list_pardesc->Segments != 0)
	      utl_cut_segments( hier_name, hier_name, list_pardesc->Segments);

	    strcat( text, hier_name);
	  }
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintRefClass") == 0)
	{
	  /* Print the name of the object */
	  sts = ldh_GetAttrRefTid( utlctx->ldhses, refo,  &cid);
	  if ( ODD(sts))
	    sts = ldh_ObjidToName( utlctx->ldhses, cdh_ClassIdToObjid( cid), 
		ldh_eName_Object, hier_name, sizeof( hier_name), &size);
	  if ( EVEN(sts))
	    strcpy( hier_name, "-");
	  if ( list_pardesc->PrintParName)
	    strcat( text, "RefClass = ");
	  strcat( text, hier_name);
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintSigChan") == 0)
	{
	  /* Print the name of the referenced object */
	  sts = ldh_GetAttrObjectPar( utlctx->ldhses, o, "RtBody", 
			"SigChanCon", (char **)&conobj_ptr, &size);
	  if ( ODD(sts))
	  {
	    if ( cdh_ObjidIsNull( conobj_ptr->Objid))
   	    {
	      strcat( text, "-");
	      free((char *) conobj_ptr);
	    }
            else
	    {
	      sts = ldh_AttrRefToName( utlctx->ldhses, conobj_ptr, 
		ldh_eName_Hierarchy, &np, &size);
	      if ( ODD(sts))
		strcpy( hier_name, np);
	      else
	        strcpy( hier_name, "-");
	      free((char *) conobj_ptr);
	      if ( list_pardesc->PrintParName)
	        strcat( text, "SigChanCon = ");
	      if ( list_pardesc->Segments != 0)
	        utl_cut_segments( hier_name, hier_name, list_pardesc->Segments);
	
	      strcat( text, hier_name);
	    }
	  }
	  else
	  {
	    /* Just print the class */
	    sts = ldh_GetAttrRefTid( utlctx->ldhses, o,  &cid);
	    if ( ODD(sts))
	      sts = ldh_ObjidToName( utlctx->ldhses, cdh_ClassIdToObjid(cid), 
		  ldh_eName_Object, hier_name, sizeof( hier_name), &size);
	    if ( EVEN(sts))
	      strcpy( hier_name, "-");
	    if ( list_pardesc->PrintParName)
	      strcat( text, "Class = ");
	    strcat( text, hier_name);
	  }
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintSigChanId") == 0)
	{
	  /* Print the Id of the referenced object */
	  sts = ldh_GetAttrObjectPar( utlctx->ldhses, o, "RtBody", 
			"SigChanCon", (char **)&conobj_ptr, &size);
	  if ( ODD(sts))
	  {
	    sts = ldh_GetAttrObjectPar( utlctx->ldhses, conobj_ptr, "RtBody", 
			"Identity", (char **)&parameter, &size);
	    free((char *) conobj_ptr);
	    if ( ODD(sts))
	    {
	      if ( list_pardesc->PrintParName)
	        strcat( text, "SigChanId = ");
	
	      strcat( text, parameter);
	      free((char *) parameter);
	    }
	  }
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintWrite") == 0)
	{
	  /* Print read or write, defined in the parameter Write.
 	     Used for ExternRef objects */
	  sts = ldh_GetAttrObjectPar( utlctx->ldhses, o, "DevBody", 
			"Write", (char **)&write_ptr, &size);
	  if ( ODD(sts))
	  {
	    if ( *write_ptr )
	      strcat( text, "# ");
	    else
	      strcat( text, "  ");
	  }
	}
	else if ( strcmp( (list_pardesc->Parameter), "PrintObjPage") == 0)
	{
	  /* Print the page of the object */
	  sts = cross_get_object_page( utlctx->ldhses, o->Objid, page);
	  strcat( text, page);
	}
	else
	{
	  sts = utl_list_get_parvalue( utlctx, o->Objid, list_pardesc, text);
	  if ( EVEN(sts)) return sts;
	}

	if ( list_pardesc->SizeTabs == -1) {
	  u_print( utlctx, " ");
	  u_print( utlctx, "%s", text);
	}
	else {
	  if ( list_pardesc->SizeTabs != 0)
	    text[ list_pardesc->SizeTabs * 8 - 1 ] = 0;
	  u_print( utlctx, "%s", text);
	  if ( list_pardesc->SizeTabs != 0)
	    u_posit( utlctx, list_pardesc->SizeTabs, strlen(text));
	}
	if ( list_pardesc->CarriageRet )
	  u_row( utlctx);
	  
	return FOE__SUCCESS;
}



/*************************************************************************
*
* Name:		utl_list_get_parvalue()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	utlctx		I	output specification.
* pwr_tObjid  objdid		I	objdid for the object.
*
*
* Description: 	
*
**************************************************************************/

static int utl_list_get_parvalue (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_listpar	*list_desc,
  char		*text
)
{
	int		sts, size, i, j;
	pwr_tClassId	cid;
	ldh_sParDef 	*bodydef;
	int		rows;
	char		body[20];	
	char		parname[32];
	char		*object_par;
	char		*object_element;
	int		elements;
	int		found;
	char		upper_name[80];
	char		inupper_name[80];
	pwr_tBoolean	*p_Boolean;	
	pwr_tFloat32	*p_Float32;
	pwr_tFloat64	*p_Float64;
	char		*p_Char;
	pwr_tInt8	*p_Int8;
	pwr_tInt16	*p_Int16;
	pwr_tInt32	*p_Int32;
	pwr_tUInt8	*p_UInt8;
	pwr_tUInt16	*p_UInt16;
	pwr_tUInt32	*p_UInt32;	
	pwr_tObjid	*p_ObjDId;	
	pwr_sAttrRef	*p_AttrRef;	
	char		*p_String;
	int		element = 0;
	char		*parameter;
	char		parameter_str[80];
	char		*t, *s;
	int		len;
	char		elementstr[20];
	char		timbuf[32];

	/* Get the class of the object */
	sts = ldh_GetObjectClass( utlctx->ldhses, Objdid, &cid);

	/* Get the element if there is one */
	strcpy( parameter_str, list_desc->Parameter);
	parameter = parameter_str;
	s = strchr( parameter, '[');
	if ( s == 0)
	  element = 0;
	else
	{
	  t = strchr( parameter, ']');
	  if ( t == 0)
	  {
	    return FOE__PARSYNT;
	  }
	  else
	  {
	    len = t - s - 1;
	    strncpy( elementstr, s + 1, len);
	    elementstr[ len ] = 0;
	    sscanf( elementstr, "%d", &element);
	    *s = '\0';
	    if ( (element < 0) || (element > 100) )
	    {
	      return FOE__PARELSYNT;
	    }
	  }
	}

	  /* Find the parameter */
	  found = 0;
	  for ( i = 0; i < 3; i++ )
	  {
	    if ( i == 0)
	      strcpy( body, "RtBody");
	    else if ( i == 1 )
	      strcpy( body, "DevBody");
	    else
	      strcpy( body, "SysBody");

    	    /* Get the paramters for this body */
	    sts = ldh_GetObjectBodyDef(utlctx->ldhses, cid, body, 1, 
	  		&bodydef, &rows);
	    if ( EVEN(sts) ) continue;

	    for ( j = 0; j < rows; j++)
	    {
	      /* Convert parname to upper case */
	      utl_toupper( upper_name, bodydef[j].ParName);
	      utl_toupper( inupper_name, parameter);

	      if (strcmp( inupper_name, upper_name) == 0)
	      {
	        found = 1;
	        break;
	      }
	    }
	    if ( found )
	      break;
	    free((char *) bodydef);	
	  }
	  if ( !found)
	  {
	    /* Parametern fanns ej */
	    return FOE__NOPAR;
	  }

	  strcpy( parname, bodydef[j].ParName);

	  if ( bodydef[j].Par->Output.Info.Flags & PWR_MASK_ARRAY )
	    elements = bodydef[j].Par->Output.Info.Elements;
	  else
	    elements = 1;

	  if ( element + 1 > elements)
	  {
	    /* Error in element */
	    return FOE__ELEMENT;
	  }
	  
	  /* Get the parameter value in object */
	  sts = ldh_GetObjectPar( utlctx->ldhses, Objdid, body,   
			parname, (char **)&object_par, &size); 
	  if ( EVEN(sts)) return sts;

	  object_element = object_par + element * size / elements;

	  if ( list_desc->PrintParName)
	  {
            strcat( text, parname);
            if ( elements > 1 )
	      sprintf( text + strlen(text), "[%2d]", element);
	    strcat( text, " = ");
	  }

          switch ( bodydef[j].Par->Output.Info.Type )
          {
            case pwr_eType_Boolean:
            {
              p_Boolean = (pwr_tBoolean *)object_element;
              if ( *p_Boolean == 0)
                strcat( text, "FALSE");
              else
                strcat( text, "TRUE");
              break;
            }
            case pwr_eType_Float32:
            {
              p_Float32 = (pwr_tFloat32 *)object_element;
              sprintf( text + strlen(text), "%f", *p_Float32);
              break;
            }
            case pwr_eType_Float64:
            {
              p_Float64 = (pwr_tFloat64 *)object_element;
              sprintf( text + strlen(text), "%f", *p_Float64);
              break;
            }
            case pwr_eType_Char:
            {
              p_Char = object_element;
              sprintf( text + strlen(text), "%c", *p_Char);
              break;
            }
            case pwr_eType_Int8:
            {
              p_Int8 = (pwr_tInt8 *)object_element;
              sprintf( text + strlen(text), "%d", *p_Int8);
              break;
            }
            case pwr_eType_Int16:
            {
              p_Int16 = (pwr_tInt16 *)object_element;
              sprintf( text + strlen(text), "%d", *p_Int16);
              break;
            }
            case pwr_eType_Int32:
            {
              p_Int32 = (pwr_tInt32 *)object_element;
              sprintf( text + strlen(text), "%d", *p_Int32);
              break;
            }
            case pwr_eType_UInt8:
            {
              p_UInt8 = (pwr_tUInt8 *)object_element;
              sprintf( text + strlen(text), "%u", *p_UInt8);
              break;
            }
            case pwr_eType_UInt16:
            {
              p_UInt16 = (pwr_tUInt16 *)object_element;
              sprintf( text + strlen(text), "%u", *p_UInt16);
              break;
            }
            case pwr_eType_UInt32:
            case pwr_eType_Mask:
            case pwr_eType_Enum:
            {
              p_UInt32 = (pwr_tUInt32 *)object_element;
              sprintf( text + strlen(text), "%u", *p_UInt32);
              break;
            }
            case pwr_eType_String:
            {
              p_String = object_element;
              sprintf( text + strlen(text), "%s", p_String);
              break;
            }
            case pwr_eType_Text:
            {
              p_String = object_element;
              sprintf( text + strlen(text), "%s", p_String);
              break;
            }
            case pwr_eType_ObjDId:
            {
	      pwr_tOName     	hier_name;

              /* Get the object name from ldh */
              p_ObjDId = (pwr_tObjid *)object_element;
	      if ( cdh_ObjidIsNull( *p_ObjDId))
                strcat( text, "-");
	      else
	      {
                sts = ldh_ObjidToName( utlctx->ldhses, 
             		*p_ObjDId, ldh_eName_Hierarchy,
   		        hier_name, sizeof( hier_name), &size);
 	        if ( EVEN(sts)) 
                  strcat( text, "-");
                else
	        {
	        if ( list_desc->Segments != 0)
	          utl_cut_segments( hier_name, hier_name, 
				list_desc->Segments);

                  strcat( text, hier_name);
	        }
	      }
              break;
            }
            case pwr_eType_AttrRef:
            {
	      pwr_tAName     	hier_name;
	      char		*hier_name_p;

              /* Get the object name from ldh */
              p_AttrRef = (pwr_sAttrRef *)object_element;
              sts = ldh_AttrRefToName( utlctx->ldhses, 
             	p_AttrRef,  ldh_eName_Aref, &hier_name_p, &size);
 	      if ( EVEN(sts)) 
                strcat( text, "-");
              else
	      {
	        strcpy( hier_name, hier_name_p);
	        if ( list_desc->Segments != 0)
	          utl_cut_segments( hier_name, hier_name,
				list_desc->Segments);

                strcat( text, hier_name);
	      }
              break;
            }
            case pwr_eType_Time:
            {
	      /* Convert time to ascii */
	      	
	      sts = time_AtoAscii((pwr_tTime *)object_element, 
                                        time_eFormat_DateAndTime, 
                                        timbuf, sizeof(timbuf));
	      timbuf[20] = 0;
	      strcat( text, timbuf);
              break;
            }
            default:
              ;
          }
	  free((char *) bodydef);	

	  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_list_print_columnheader()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* unsigned long	utlctx		I	output specification.
* pwr_tObjid  objdid		I	objdid for the object.
*
*
* Description: 	
*
**************************************************************************/

static int utl_list_print_columnheader ( 
  utl_ctx		utlctx,
  utl_t_listbody	*listbody_ptr
)
{
	char	text[160];
	int	found;
	int	new_row;
	int	i, j;

	if ( !listbody_ptr->ColumnHeader )
	  return FOE__SUCCESS;

	found = 0;
	for ( j = 0; j < UTL_PARDESCRIPTION; j++)
	{
	  if ( (listbody_ptr->ParDescription[j].ColumnHeader[0] != 0 ) &&
		(listbody_ptr->ParDescription[j].SizeTabs != 0))
	  {
	    found = 1;
	    break;
	  }
	}

	if ( !found ) 
	  return FOE__SUCCESS;

	/* Print column header */
	new_row = 1;
	for ( j = 0; j < UTL_PARDESCRIPTION; j++)
	{
	  text[0] = 0;
	  if ( new_row)
	  {
	    u_pagebreak( utlctx);
	    for ( i = 0; 
	          i < (int) strlen( listbody_ptr->ParDescription[j].MarginString);
		  i++)
	      strcat ( text, " ");
	  }
	  strcat ( text, listbody_ptr->ParDescription[j].ColumnHeader);
	  u_print( utlctx, "%s", text);
	  if ( listbody_ptr->ParDescription[j].SizeTabs == -1)
	    u_print( utlctx, " ");
	  else
	    u_posit( utlctx, listbody_ptr->ParDescription[j].SizeTabs, 
		     strlen(text));
	  if ( listbody_ptr->ParDescription[j].CarriageRet)
	    u_row( utlctx);
	  new_row = listbody_ptr->ParDescription[j].CarriageRet;
	}

	/* Print underscore */
	if ( listbody_ptr->ColHead_____ )
	{
	  new_row = 1;
	  for ( j = 0; j < UTL_PARDESCRIPTION; j++)
	  {
	    text[0] = 0;
	    if ( new_row)
	    {
	      u_pagebreak( utlctx);
	      for ( i = 0; 
	          i < (int) strlen( listbody_ptr->ParDescription[j].MarginString);
		  i++)
	        strcat ( text, " ");
	    }
	    for ( i = 0; 
	          i < (int) strlen( listbody_ptr->ParDescription[j].ColumnHeader);
		  i++)
	        strcat ( text, "-");
	    u_print( utlctx, "%s", text);
	    if ( listbody_ptr->ParDescription[j].SizeTabs == -1)
	      u_print( utlctx, " ");
	    else
	      u_posit( utlctx, listbody_ptr->ParDescription[j].SizeTabs, 
		       strlen(text));
	    if ( listbody_ptr->ParDescription[j].CarriageRet)
	      u_row( utlctx);
	    if ( listbody_ptr->ParDescription[j].CarriageRet)
	      break;
	  }
	}

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_cut_segments()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

int utl_cut_segments (
  char	*outname,
  char	*name,
  int	segments
)
{
	char	*s[20];
	int	i, j, last_i;

	for( i = 0; i < segments; i++)
	{
	  s[i] = strrchr( name, '-');
	  if ( s[i] == 0)
	  {
	    last_i = i;
	    break;
	  }
	  *s[i] = '+';
	  last_i = i;
	}
	for ( j = 0; j <= last_i; j++)
	{
	  if ( s[j] != 0)
	    *s[j] = '-';
	}
	if ( s[last_i] == 0)
	  strcpy( outname, name);
	else
	  strcpy( outname, s[last_i] + 1);

	return FOE__SUCCESS;
}



/*************************************************************************
*
* Name:		utl_configure_card()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	hiername	I	Name of a object in the hierarchy.
* char *	class		I	Name of the class.
*
*
* Description: 	
*
**************************************************************************/

int utl_configure_card (
  ldh_tSesContext ldhses,
  char		*rackname,
  char		*cardname,
  char		*cardclassname,
  char		*channame,
  char		*chanidentity,
  char		*chandescription,
  char		*table
)
{
	int		sts, size, i;
	pwr_tObjid	rackobjdid;
	pwr_tClassId	cardclass;
	pwr_tObjid	cardobjdid;
	short		*channels;
	char		chanclassname[32];
	pwr_tClassId	chanclass;
	char		chantclassname[32];
	pwr_tClassId	chantclass;
	char		objectname[32];
	char		replaced_str[80];
	pwr_tObjid  objdid;
	char		tablearray[65];
	char		*s;

	/* Get rack objdid */
	sts = ldh_NameToObjid( ldhses, &rackobjdid, rackname);
	if ( EVEN(sts))
	{
	  return FOE__OBJNAME;
	}	

	/* Get card class */
	sts = ldh_ClassNameToId( ldhses, &cardclass, cardclassname);
	if ( EVEN(sts))
	{
	  return FOE__CLASSNAME;
	}

	s = strrchr( channame, '#');
	if ( (s == 0) && ( strstr( cardclassname, "CO") == NULL))
	  return FOE__QUALSYNT;

	/* Create the card */
	sts = ldh_CreateObject( ldhses, &cardobjdid, cardname, 
		cardclass, rackobjdid, ldh_eDest_IntoLast);
	if ( EVEN(sts)) return sts;

	/* Get number of channels in the card */
	sts = ldh_GetObjectPar( ldhses, cardobjdid, "RtBody", 
			"MaxNoOfChannels", (char **)&channels, &size);
	if ( EVEN(sts))
	{
	  /* This is probably a co card */
	  sts = ldh_GetObjectPar( ldhses, cardobjdid, "RtBody", 
			"MaxNoOfCounters", (char **)&channels, &size);
	  if ( EVEN(sts)) return sts;
	}

	/* Get the class of the channels */
	if ( strstr( cardclassname, "DI") != NULL)
	{
	  strcpy( chanclassname, "ChanDi");
	  strcpy( chantclassname, "ChanDi");
	}
	else if ( strstr( cardclassname, "DO") != NULL)
	{
	  strcpy( chanclassname, "ChanDo");
	  strcpy( chantclassname, "ChanDo");
	}
	else if ( strstr( cardclassname, "AI") != NULL)
	{
	  strcpy( chanclassname, "ChanAi");
	  strcpy( chantclassname, "ChanAit");
	}
	else if ( strstr( cardclassname, "AO") != NULL)
	{
	  strcpy( chanclassname, "ChanAo");
	  strcpy( chantclassname, "ChanAo");
	}
	else if ( strstr( cardclassname, "CO") != NULL)
	{
	  strcpy( chanclassname, "ChanCo");
	  strcpy( chantclassname, "ChanCot");
	}

	sts = ldh_ClassNameToId( ldhses, &chanclass, chanclassname);
	if ( EVEN(sts)) return sts;
	sts = ldh_ClassNameToId( ldhses, &chantclass, chantclassname);
	if ( EVEN(sts)) return sts;

	memset( &tablearray, 0, sizeof(tablearray));
	if ( table != 0)
	  utl_parse_indexstring( table, tablearray, sizeof(tablearray));

	/* Create the channels */
	for ( i = 0; i < *channels; i++)
	{
	  /* Get the channel name */
	  utl_config_replace( channame, objectname, i + 1);

	  /* Create the chan object */
	  if ( tablearray[i + 1])
	  {
	    sts = ldh_CreateObject( ldhses, &objdid, objectname, 
		chantclass, cardobjdid, ldh_eDest_IntoLast);
	    if ( EVEN(sts)) return sts;
	  }
	  else
	  {
	    sts = ldh_CreateObject( ldhses, &objdid, objectname, 
		chanclass, cardobjdid, ldh_eDest_IntoLast);
	    if ( EVEN(sts)) return sts;
	  }

	  /* Set the number */
	  sts = ldh_SetObjectPar( ldhses,
			objdid, "RtBody", "Number", (char *)&i, size); 
	  if ( EVEN(sts)) return sts;

	  /* Set the identity */
	  if ( chanidentity != 0)
	  {
	    utl_config_replace( chanidentity, replaced_str, i + 1);
	    sts = ldh_SetObjectPar( ldhses,
			objdid, "RtBody", "Identity", replaced_str, size); 
	    if ( EVEN(sts)) return sts;
	  }

	  /* Set the description */
	  if ( chandescription != 0)
	  {
	    utl_config_replace( chandescription, replaced_str, i + 1);
	    sts = ldh_SetObjectPar( ldhses,
			objdid, "RtBody", "Description", replaced_str, size); 
	    if ( EVEN(sts)) return sts;
	  }
	}
	free((char *) channels);

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_config_replace()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Replaces a '#' with an index.
*
**************************************************************************/

static int	  utl_config_replace( 
    char	*instr,
    char	*outstr,
    int	index
)
{
	char	indexstr[10];
	char	par_str[5][80];
	int	i, nr;

	sprintf( indexstr, "%2d", index);
	if ( indexstr[0] == ' ')
	  indexstr[0] = '0';

	nr = utl_parse( instr, "#", "",(char *) par_str, 
		sizeof( par_str) / sizeof( par_str[0]), sizeof( par_str[0]));

	if ( *instr == '#')
	{
	  strcpy( outstr, indexstr);
	  strcat( outstr, (char *)par_str[0]);
	}
	else
	  strcpy( outstr, (char *)par_str[0]);

	for ( i = 1; i < co_min( 5, nr); i++)
	{
	  strcat( outstr, indexstr);
	  strcat( outstr, (char *)par_str[i]);
	}

/*	s = instr + strlen( instr) -1;
	if ( (*s == '#') && ( s != instr))
	  strcat( outstr, indexstr);
*/
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_get_filename()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	returns a utic filename.
*
**************************************************************************/

int utl_get_filename (
  char	*filename
)
{
	int 	val;
	pwr_tTime time;

	clock_gettime(CLOCK_REALTIME, &time);
	srand( time.tv_sec);
	val = rand();
	sprintf( filename, "pwrp_tmp:foe_%x.lis", val);
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_get_projectname()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get the project name.
*	Under VMS project and systemname are equal.
*	Under Linux project name i defined in env.
*
**************************************************************************/

int utl_get_projectname(
  char		*projectname
)
{
#if defined OS_VMS
  char systemgroup[80];

  return utl_get_systemname( projectname, systemgroup);
#else
  char *env_p;

  env_p = getenv( "pwrp_projectname");
  if ( env_p == 0)
    return 0;

  strcpy( projectname, env_p);
  return FOE__SUCCESS;
#endif
}


/*************************************************************************
*
* Name:		utl_get_systemname()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get the system name from the system object file.
*
**************************************************************************/

int utl_get_systemname(
  char		*systemname,
  char		*systemgroup
)
{
	static char	stored_name[80] = "";
	static char	stored_group[80] = "";
	static int	name_is_stored = 0;
	int		sts;

	*systemname = '\0';
	if ( !name_is_stored)
	{
	  sts = lfu_ReadSysObjectFile( systemname, systemgroup);
	  if ( EVEN(sts)) return sts;

	  strcpy( stored_name, systemname);
	  strcpy( stored_group, systemgroup);
	  name_is_stored = 1;
	}
	else
        {
	  strcpy( systemname, stored_name);
	  strcpy( systemgroup, stored_group);
        }
	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_get_systemobject()
*
* Type		void
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses	I	ldh session.
* pwr_tObjid *   system_objid	O	system objdid.
*
* Description:
*	Get the system object objid and the systemname attribute.
*
**************************************************************************/

int utl_get_systemobject(
  ldh_tSesContext ldhses,
  pwr_tObjid	*system_objid,
  char		*systemname,
  char		*systemgroup
)
{
	int		sts, size, found;
	pwr_tClassId	cid;
	char		*sysname_ptr;
	char		*sysgroup_ptr;
	pwr_tObjid	objid;

	sts = ldh_GetRootList( ldhses, &objid);
	found = 0;
	while ( ODD(sts) )
	{
	  sts = ldh_GetObjectClass( ldhses, objid, &cid);
	  if ( EVEN(sts)) return sts;

	  if ( cid == pwr_eClass_System)
	  {
	    found = 1;
	    break;
	  }
	  sts = ldh_GetNextSibling( ldhses, objid, &objid);
	}

	if ( !found )
	  return FOE__OBJECT;

	sts = ldh_GetObjectPar( ldhses, objid, "SysBody",
			"SystemName", (char **)&sysname_ptr, &size); 
	if ( EVEN(sts)) return sts;

	strcpy( systemname, sysname_ptr);
	free((char *) sysname_ptr);

	sts = ldh_GetObjectPar( ldhses, objid, "SysBody",
			"SystemGroup", (char **)&sysgroup_ptr, &size); 
	if ( EVEN(sts)) return sts;

	strcpy( systemgroup, sysgroup_ptr);
	free((char *) sysgroup_ptr);

	*system_objid = objid;
	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_get_listconfig_object()
*
* Type		void
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get info in the ListConfig object.
*
**************************************************************************/

static int utl_get_listconfig_object(
  ldh_tSesContext ldhses,
  int		  *landscape_rows,
  int		  *portrait_rows
)
{
	int		sts, size, found;
	pwr_tUInt32	*landscape_rows_p;
	pwr_tUInt32	*portrait_rows_p;
	pwr_tClassId	cid;
	pwr_tObjid	objid;
	pwr_tObjid	child;


	found = 0;
	/* Try to find the configobject in the local hierarchy */
	sts = ldh_NameToObjid( ldhses, &objid, "wb:Local-Lists");
	if ( ODD(sts))
	{
	  sts = ldh_GetChild( ldhses, objid, &child);
	  while ( ODD(sts) )
	  {
	    sts = ldh_GetObjectClass( ldhses, child, &cid);
	    if (EVEN(sts)) return sts;

	    if ( cid == vldh_eclass( ldhses, "ListConfig"))
	    {
	      found = 1;
	      break;
	    }
	    sts = ldh_GetNextSibling( ldhses, child, &child);
	  }
	}
	if ( !found)
	{	
	  /* Try to find the configobject in the layout hierarchy */
	  sts = ldh_NameToObjid( ldhses, &objid, "wb:layout-Lists");
	  if ( ODD(sts))
	  {
	    sts = ldh_GetChild( ldhses, objid, &child);
	    while ( ODD(sts) )
	    {
	      sts = ldh_GetObjectClass( ldhses, child, &cid);
	      if (EVEN(sts)) return sts;

	      if ( cid == vldh_eclass( ldhses, "ListConfig"))
	      {
	        found = 1;
	        break;
	      }
	      sts = ldh_GetNextSibling( ldhses, child, &child);
	    }
	  }
	}
	if ( !found)
	{
	  /* Use default values */
	  *landscape_rows = UTL_PAGE_BREAK_LANDS;
	  *portrait_rows = UTL_PAGE_BREAK_PORTR;
	  return FOE__SUCCESS;
	}

	sts = ldh_GetObjectPar( ldhses, child, "DevBody",
			"LandscapePageRows", (char **)&landscape_rows_p, &size); 
	if ( EVEN(sts)) return sts;
	sts = ldh_GetObjectPar( ldhses, child, "DevBody",
			"PortraitPageRows", (char **)&portrait_rows_p, &size); 
	if ( EVEN(sts)) return sts;

	*landscape_rows = *landscape_rows_p;
	*portrait_rows = *portrait_rows_p;
	free((char *) landscape_rows_p);
	free((char *) portrait_rows_p);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_get_module_time()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get the insert time of a library module.
*
**************************************************************************/

pwr_tStatus utl_get_module_time (
    char		*libr_name,
    char		*module_name,
    pwr_tTime		*time
)
{
#if defined OS_VMS	
	pwr_tStatus	sts;
	int		size;
	unsigned long	libr_index;
	int		libr_func = LBR$C_READ;
#if defined  __ALPHA
	int		libr_type = LBR$C_TYP_EOBJ;	
#else
	int		libr_type = LBR$C_TYP_OBJ;	
#endif
	int		txtrfa[2];
	char 		modulebuf[100];
	int		modulebuf_len;

/* The size of the mhdbuf must be greater than struct mhddef
 * otherwise lbr$set_module returns HDRTRUNC.
 */ 
	char		mhdbuf[LBR$C_MAXHDRSIZ];
	struct	mhddef  *mhd = (struct mhddef *)mhdbuf;
	int		mhd_len;
	struct dsc$descriptor_s libr_name_desc = 
			{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
	struct dsc$descriptor_s module_name_desc = 
			{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
	struct dsc$descriptor_s mhd_desc = 
			{0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};


	libr_name_desc.dsc$w_length = strlen(libr_name);
	libr_name_desc.dsc$a_pointer = libr_name;
	module_name_desc.dsc$w_length = strlen(module_name);
	module_name_desc.dsc$a_pointer = module_name;
	mhd_desc.dsc$w_length = sizeof(mhdbuf);
	mhd_desc.dsc$a_pointer = mhdbuf;

	sts = lbr$ini_control( &libr_index, &libr_func, &libr_type, 0);
	if ( EVEN(sts)) return sts;
	sts = lbr$open( &libr_index, &libr_name_desc);
	if ( EVEN(sts) && !(!(sts & 2) && !(sts & 1))) return sts; /* Accept warnings */
	sts = lbr$lookup_key( &libr_index, &module_name_desc, txtrfa); 
	if ( EVEN(sts)) 
	{	
	  lbr$close( &libr_index);
	  return sts;
	}
	sts = lbr$set_module( &libr_index, txtrfa, &mhd_desc, 
		&mhd_len, 0);
	if ( EVEN(sts)) 
	{	
	  lbr$close( &libr_index);
	  return sts;
	}

	time_VmsToPwr((pwr_tVaxTime *)&mhd->mhd$l_datim, time);

	sts = lbr$close( &libr_index);
	if ( EVEN(sts)) return sts;
	return FOE__SUCCESS;
#else
	return 0;
#endif
}



/*************************************************************************
*
* Name:		utl_sortchildren()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: 	
*
**************************************************************************/

int utl_sortchildren (
  ldh_tSesContext ldhses,
  char		*parent_name,
  int		classort
)
{
	pwr_tObjid	parent_objdid;
	utl_ctx		utlctx;
	int		sts;
	utl_t_list	*child_listptr;
	utl_t_list	*child_list = 0;
	utl_t_list	*next;
	int		child_count;

	  sts = ldh_NameToObjid( ldhses, &parent_objdid, parent_name);
	  if ( EVEN(sts))
	  {
	    return FOE__OBJECT;
	  }
	   
	  /* Open file */
	  utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);

	  /* Get all children */
	  child_count = 0;
	  sts = trv_get_children_class_name( utlctx->ldhses, parent_objdid, 
		0, 0, 
		(trv_tBcFunc) utl_ctxlist_insert, 
		&child_list, &child_count, 0, 0, 0);
	  if ( EVEN (sts)) return sts;

	  sts = utl_list_classsort( &child_list, child_count, ldhses,
			classort);
	  if ( EVEN (sts)) return sts;

	  /* Move the children */
	  child_listptr = child_list;
	  while( child_listptr)
	  {
	
	    sts = ldh_MoveObject( ldhses, child_listptr->o.Objid, 
			parent_objdid, ldh_eDest_IntoLast);
	    if ( EVEN(sts)) return sts;
	    child_listptr = child_listptr->next;
	  }

	  child_listptr = child_list;
	  while( child_listptr)
	  {
	    next = child_listptr->next;
	    free((char *) child_list);
	    child_listptr = next;
	  }

	  utl_ctx_delete( utlctx);

	  return FOE__SUCCESS;
}



/*************************************************************************
*
* Name:		utl_create_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	object_name	I	Name object.
* char *	class_name	I	class of the object.
* char *	destination_name I	Name of created destination object.
* int		first		I	Position relativ destination.
* int		last		I	Position relativ destination.
* int		after		I	Position relativ destination.
* int		before		I	Position relativ destination.
*
* Description: 	Create an object.
*
**************************************************************************/

int utl_create_object (
  ldh_tSesContext ldhses,
  char		*object_name,
  char		*class_name,
  char		*destination_name,
  int		first,
  int		last,
  int		after,
  int		before
)
{
	int		sts;
	pwr_tObjid	destination;
	int		code;
	pwr_tClassId	cid;
	pwr_tObjid  objdid;

	if ( first) 
	  code = ldh_eDest_IntoFirst;
	else if ( last)
	  code = ldh_eDest_IntoLast;
	else if ( after)
	  code = ldh_eDest_After;
	else if ( before)
	  code = ldh_eDest_Before;
	else
	  code = ldh_eDest_IntoLast;

	/* Get class */
	sts = ldh_ClassNameToId( ldhses, &cid, class_name);
	if ( EVEN(sts))
	{
	  return FOE__CLASSNAME;
	}

	/* Check destination name */
	if ( strcmp( destination_name, "") == 0)
	{
	  /* A root object, no destination */
	  destination = pwr_cNObjid;
	}	
	else
	{
	  sts = ldh_NameToObjid( ldhses, &destination, destination_name);
	  if ( EVEN(sts))
	  {
	    return FOE__OBJNAME;
	  }	
	}
	sts = ldh_CreateObject( ldhses, &objdid, object_name,
		cid, destination, (ldh_eDest)code);
	if (EVEN(sts)) return sts;

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_move_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	object_name	I	Name object.
* char *	class_name	I	class of the object.
* char *	destination_name I	Name of created destination object.
* int		first		I	Position relativ destination.
* int		last		I	Position relativ destination.
* int		after		I	Position relativ destination.
* int		before		I	Position relativ destination.
*
* Description: 	Create an object.
*
**************************************************************************/

int utl_move_object ( 
  ldh_tSesContext ldhses,
  char		*source_name,
  char		*destination_name,
  char		*name,
  int		first,
  int		last,
  int		after,
  int		before
)
{
	int		sts;
	pwr_tObjid	destination;
	pwr_tObjid	source;
	int		code;

	if ( first) 
	  code = ldh_eDest_IntoFirst;
	else if ( last)
	  code = ldh_eDest_IntoLast;
	else if ( after)
	  code = ldh_eDest_After;
	else if ( before)
	  code = ldh_eDest_Before;
	else
	  code = ldh_eDest_IntoLast;

	/* Check source name */
	sts = ldh_NameToObjid( ldhses, &source, source_name);
	if ( EVEN(sts))
	{
	  return FOE__OBJNAME;
	}	

	if ( destination_name != NULL)
	{
	  /* Check destination name */
	  sts = ldh_NameToObjid( ldhses, &destination, destination_name);
	  if ( EVEN(sts))
	  {
	    return FOE__OBJNAME;
	  }	

	  sts = ldh_MoveObject( ldhses, source, destination, 
			(ldh_eDest)code);
	  if ( EVEN(sts)) return sts;
	}

	/* Rename the object if name is present */
	if ( name != NULL)
	{
	  sts = ldh_ChangeObjectName( ldhses, source, name);
	  if ( EVEN(sts)) return sts;
	}

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_copy_objects()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	object_name	I	Name object.
* char *	class_name	I	class of the object.
* char *	destination_name I	Name of created destination object.
* int		first		I	Position relativ destination.
* int		last		I	Position relativ destination.
* int		after		I	Position relativ destination.
* int		before		I	Position relativ destination.
*
* Description: 	Copy an object.
*
**************************************************************************/

int utl_copy_objects ( 
  ldh_tSesContext ldhses,
  char		*source_name,
  char		*destination_name,
  char		*name,
  int		hier,
  int		first,
  int		last,
  int		after,
  int		before
)
{
#if defined OS_VMS
	int		sts;
	pwr_tObjid	destination;
	pwr_tObjid	source;
	int		code;
	pwr_tObjid	new_objid;
	pwr_sAttrRef	attrref[2];
	pwr_tObjid	objid;

	if ( first) 
	  code = ldh_eDest_IntoFirst;
	else if ( last)
	  code = ldh_eDest_IntoLast;
	else if ( after)
	  code = ldh_eDest_After;
	else if ( before)
	  code = ldh_eDest_Before;
	else
	  code = ldh_eDest_IntoLast;

	/* Check source name */
	sts = ldh_NameToObjid( ldhses, &source, source_name);
	if ( EVEN(sts))
	{
	  return FOE__OBJNAME;
	}	

	/* Check destination name */
	sts = ldh_NameToObjid( ldhses, &destination, destination_name);
	if ( EVEN(sts))
	{
	  return FOE__OBJNAME;
	}	

	if ( !hier)
	{
	  sts = ldh_CopyObject( ldhses, &new_objid, name,
		source, destination, code);
	  if (EVEN(sts)) return sts;
	}
	else
	{
	  memset( attrref, 0, sizeof(attrref));
	  attrref[0].Objid = source;
	  attrref[1].Objid = pwr_cNObjid;

	  sts = ldh_CopyObjectTrees( ldhses, &attrref, destination,
		code,  0, 0);
	  if ( EVEN(sts)) return sts;

	  /* Fetch the created root object, and change the name */
	  sts = foe_show_get_created_object( &objid);
	  if (EVEN(sts)) return sts;

	  sts = ldh_ChangeObjectName( ldhses, objid, name);
	  if ( EVEN(sts)) return sts;
	}
#endif
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_move_window()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	source_name	I	Name of source object.
* char *	destination_name I	Name of destination object.
*
* Description: 	Moves all the object in one window to another window.
*
**************************************************************************/

int utl_move_window (
  ldh_tSesContext ldhses,
  char		*source_name,
  char		*destination_name
)
{
	int		sts, size, get_sts;
	int		i;
	pwr_tObjid	destination;
	pwr_tObjid	source;
	pwr_tObjid	child;
	pwr_tObjid	next_child;
	pwr_eClass	eclass;
	pwr_sPlcNode	*nodebuffer;
	pwr_sPlcConnection	*conbuffer;
	pwr_sPlcProgram	*dest_plcbuffer;
	pwr_sPlcProgram	*source_plcbuffer;
	pwr_tObjid	dest_plcobjdid;
	pwr_tObjid	source_plcobjdid;
	vldh_t_plc	plc;
	pwr_tObjid	*parentlist;
	unsigned long	parent_count;

	/* Check source name */
	sts = ldh_NameToObjid( ldhses, &source, source_name);
	if ( EVEN(sts))
	{
	  return FOE__OBJNAME;
	}	

	/* Check source name */
	sts = ldh_NameToObjid( ldhses, &destination, destination_name);
	if ( EVEN(sts))
	{
	  return FOE__OBJNAME;
	}	

	{

	  /* Get the plcprogram for the source window */
	  sts = trv_get_parentlist( ldhses, source, &parent_count, &parentlist);
	  if ( EVEN(sts)) return sts;

	  /* Check if the plcpgm is loaded in vldh */
	  source_plcobjdid = *(parentlist + parent_count -1);
	  sts = vldh_get_plc_objdid( source_plcobjdid, &plc);
	  if ( ODD(sts)) return FOE__PLCLOADED;
	    
	  /* Get the plcprogram for the dest window */
	  sts = trv_get_parentlist( ldhses, destination, &parent_count, &parentlist);
	  if ( EVEN(sts)) return sts;

	  /* Check if the plcpgm is loaded in vldh */
	  dest_plcobjdid = *(parentlist + parent_count -1);
	  sts = vldh_get_plc_objdid( dest_plcobjdid, &plc);
	  if ( ODD(sts)) return FOE__PLCLOADED;

	  /* Get the plc buffer */	    
	  sts = ldh_GetObjectBuffer( ldhses,
		source_plcobjdid, "DevBody", "PlcProgram", &eclass,	
		(char **)&source_plcbuffer, &size);
	  if( EVEN(sts)) return sts;

	  sts = ldh_GetObjectBuffer( ldhses,
		dest_plcobjdid, "DevBody", "PlcProgram", &eclass,	
		(char **)&dest_plcbuffer, &size);
	  if( EVEN(sts)) return sts;

	  /* Copy the default max values */
	  for ( i = 0; i < PWR_OBJTYPES_MAX; i++)
	  {
	    source_plcbuffer->defnamecount[i] = 
		co_max( source_plcbuffer->defnamecount[i], 
		dest_plcbuffer->defnamecount[i]);
	    dest_plcbuffer->defnamecount[i] = 
		source_plcbuffer->defnamecount[i];
	  }

	  sts = ldh_SetObjectBuffer( ldhses, source_plcobjdid, "DevBody", 
			"PlcProgram", (char *)source_plcbuffer);
	  if( EVEN(sts)) return sts;
	  free((char *) source_plcbuffer);

	  sts = ldh_SetObjectBuffer( ldhses, dest_plcobjdid, "DevBody", 
			"PlcProgram", (char *)dest_plcbuffer);
	  if( EVEN(sts)) return sts;
	  free((char *) dest_plcbuffer);


	  get_sts = ldh_GetChild( ldhses, source, &next_child);
	  while ( ODD(get_sts) )
	  {
	    /* Get next sibling before the move */
	    child = next_child;
	    get_sts = ldh_GetNextSibling( ldhses, child, &next_child);

	    /* Move the object to the new window */
	    sts = ldh_MoveObject( ldhses, child, destination, 
			ldh_eDest_IntoLast);
	    if ( EVEN(sts)) return sts;

	    /* Change values in the node and connection buffer */	  
	    sts = ldh_GetObjectBuffer( ldhses,
		child, "DevBody", "PlcNode", &eclass,	
		(char **)&nodebuffer, &size);
	    if( ODD(sts))
	    {
	      nodebuffer->woid = source;

	      sts = ldh_SetObjectBuffer( ldhses, child, "DevBody", "PlcNode", 
			(char *)nodebuffer);
	      if( EVEN(sts)) return sts;
	      free((char *) nodebuffer);
	    }
	    else
	    {
	      /* This is a connection */	  
	      sts = ldh_GetObjectBuffer( ldhses,
		child, "DevBody", "PlcConnection", &eclass,	
		(char **)&conbuffer, &size);
	      if( EVEN(sts)) return sts;

	      conbuffer->woid = source;

	      sts = ldh_SetObjectBuffer( ldhses, child, "DevBody", 
			"PlcConnection", (char *)conbuffer);
	      if( EVEN(sts)) return sts;
	      free((char *) conbuffer);
	    }
	  }
	}	
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_connect()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: 	Connect to objects.
*
**************************************************************************/

int utl_connect ( 
  ldh_tSesContext ldhses,
  char		*object_name,
  char		*connect_name,
  int		reconnect
)
{

	printf("%%FOE-E-CONNECT, Connect is obsolete\n");

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_disconnect()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: 	Disconnect an object.
*
**************************************************************************/

int utl_disconnect (
  ldh_tSesContext ldhses,
  char		*object_name
)
{
	printf("%%FOE-E-DISCONNECT, Disconnect is obsolete\n");
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_object_delete()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	utlctx		I	output specification.
*
*
* Description: 	
*
**************************************************************************/

static int utl_object_delete (
  pwr_tObjid  Objdid,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  unsigned long	dum1,
  unsigned long	dum2,
  unsigned long	dum3
)
{
	static char	yes_or_no[200];	
	pwr_tOName     	hier_name;
	int		sts, size;

#if defined OS_VMS
	int		len;
  	$DESCRIPTOR ( yon_prompt_desc , "(Y/N/Q/A): " );
  	static $DESCRIPTOR ( yon_desc , yes_or_no );
#endif


	sts = ldh_ObjidToName( ldhses, 
	           	Objdid, ldh_eName_Hierarchy,
  		        hier_name, sizeof( hier_name), &size);
  	if ( EVEN(sts)) return sts;

	if ( utlctx->confirm )
	{
	  printf("Delete object %s ", hier_name);
#if defined OS_VMS
 	  sts = lib$get_input ( &yon_desc , &yon_prompt_desc , &len ) ; 
	  if ( EVEN(sts)) return sts;
#else
	  // TODO
	  printf( "(Y/N/Q/A): ");
	  sts = scanf( "%s", yes_or_no);
#endif
	  if ( ((yes_or_no[0] == 'Q') || (yes_or_no[0] == 'q')))
	    return FOE__ABORTSEARCH;
	  else if ( ((yes_or_no[0] == 'A') || (yes_or_no[0] == 'a')))
	    utlctx->confirm = 0;
	  else if ( !((yes_or_no[0] == 'Y') || (yes_or_no[0] == 'y')))
	    return FOE__SUCCESS;
	}

	/* Delete the object */
	sts = ldh_DeleteObject( ldhses, Objdid);
	if ( ODD(sts))
	{
	  if ( utlctx->log )	
	    printf( "Object deleted %s\n", hier_name);
	}
	else if ( sts == LDH__HAS_CHILD)
	  printf( "%%FOE-E-UNABLE_TO_DELETE object %s, object has child\n", hier_name);
	else if ( EVEN(sts)) return sts;

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_tree_delete()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	utlctx		I	output specification.
*
*
* Description: 	
*
**************************************************************************/

static int utl_tree_delete ( 
  pwr_tObjid  Objdid,
  ldh_tSesContext ldhses,	
  utl_ctx	utlctx,
  unsigned long	dum1,
  unsigned long	dum2,
  unsigned long	dum3
)
{
	static char	yes_or_no[200];	
	pwr_tOName     	hier_name;
	int		sts, size;

#if defined OS_VMS
	int		len;
  	$DESCRIPTOR ( yon_prompt_desc , "(Y/N/Q): " );
  	static $DESCRIPTOR ( yon_desc , yes_or_no );
#endif

	sts = ldh_ObjidToName( ldhses, 
	           	Objdid, ldh_eName_Hierarchy,
  		        hier_name, sizeof( hier_name), &size);
  	if ( EVEN(sts)) return sts;

	if ( utlctx->confirm )
	{
	  printf("Delete tree %s ", hier_name);
#if defined OS_VMS
 	  sts = lib$get_input ( &yon_desc , &yon_prompt_desc , &len ) ; 
	  if ( EVEN(sts)) return sts;
#else
	  // TODO
	  printf( "(Y/N/Q): ");
	  sts = scanf( "%s", yes_or_no);
#endif
	  if ( !((yes_or_no[0] == 'Y') || (yes_or_no[0] == 'y')))
	    return FOE__SUCCESS;
	}

	/* Delete the object tree */
	sts = ldh_DeleteObjectTree( ldhses, Objdid);
	if ( ODD(sts))
	{
	  if ( utlctx->log )	
	    printf( "Object tree deleted %s\n", hier_name);
	}
	else if ( EVEN(sts)) return sts;

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_delete_objects()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	hiername	I	Name of a object in the hierarchy.
* char *	class		I	Name of the class.
*
*
* Description: 	Delete all objects of a specified class that is found
*		below a specific object in the hierarchy.
*
**************************************************************************/

int utl_delete_objects (
  ldh_tSesContext ldhses,
  char		*hiername,
  char		*classname,
  char		*name,
  int		confirm,
  int		log
)
{
	utl_ctx	utlctx;
	int		sts, i;
	pwr_tClassId	*classp;
	pwr_tObjid	hierobjdid;
	char		*s;
	pwr_tObjid  objdid;
	char		class_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tClassId	class_vect[UTL_INPUTLIST_MAX + 1];
	int		nr;
	utl_t_objidlist	*object_list;
	utl_t_objidlist	*object_ptr;
	int		object_count;


	/* Check if class */
	if ( classname != NULL )
	{
	  nr = utl_parse( classname, ", ", "", (char *)class_str, 
		sizeof( class_str) / sizeof( class_str[0]), sizeof( class_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX))
            return FOE__CLASSYNT;

	  for ( i = 0; i < nr; i++)
	  {
	    sts = ldh_ClassNameToId( ldhses, &class_vect[i], class_str[i]);
	    if ( EVEN(sts))
	    {
	      return FOE__CLASSNAME;
	    }
	  }
	  class_vect[nr] = 0;
	  classp = class_vect;
	}	
	else
	  classp = 0;

	/* Check if hierarchy */
	if ( hiername != NULL )
	{
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &hierobjdid, hiername);
	  if ( EVEN(sts))
	  {
	    return FOE__HIERNAME;
	  }	
	}
	else
	  hierobjdid = pwr_cNObjid;

	/* Check if name */
	if ( name != NULL )
	{
	  /* Check that name includes a wildcard */
	  s = strchr( name, '*');
	  if ( s == 0)
	  {
	    /* Print this object */
	    /* Get objdid for the object */
	    sts = ldh_NameToObjid( ldhses, &objdid, name);
	    if ( EVEN(sts))
	    {
	      return FOE__OBJECT;
	    }

	    utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	    utlctx->confirm = confirm;
	    utlctx->log = log;

	    sts = utl_object_delete( objdid, ldhses, utlctx, 0, 0, 0);
	    if ( sts != FOE__ABORTSEARCH)
	      if (EVEN(sts)) return sts;

	    utl_ctx_delete( utlctx);
	    return FOE__SUCCESS;
	  }
	}

	/* Open file */
	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	utlctx->confirm = confirm;
	utlctx->log = log;

	/* Search for objects */
	object_count = 0;
	object_list = 0;
	sts = trv_get_objects_hier_class_name( ldhses, hierobjdid, classp, name, 
		&utl_objidlist_insert, 
		&object_list, &object_count, 0, 0, 0);
	if (EVEN(sts)) return sts;

	if ( utlctx->confirm )
	  printf( "Number of objects found: %d\n", object_count);

	object_ptr = object_list;
	while( object_ptr)
	{
	  sts = utl_object_delete( object_ptr->objid, ldhses, utlctx, 0, 0, 0);
	  if ( sts == FOE__ABORTSEARCH) break;
	  else if (EVEN(sts)) return sts;

	  object_ptr = object_ptr->next;
	}

	utl_objidlist_free( object_list);
	utl_ctx_delete( utlctx);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_delete_objects()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* char *	hiername	I	Name of a object in the hierarchy.
* char *	class		I	Name of the class.
*
*
* Description: 	Delete all objects of a specified class that is found
*		below a specific object in the hierarchy.
*
**************************************************************************/

int utl_delete_tree (
  ldh_tSesContext ldhses,
  char		*name,
  int		confirm,
  int		log
)
{
	utl_ctx		utlctx;
	int		sts;
	pwr_tClassId	*classp;
	pwr_tObjid  objdid;

	classp = 0;

	/* Check if hierarchy */
	if ( name != NULL )
	{
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &objdid, name);
	  if ( EVEN(sts))
	  {
	    return FOE__OBJECT;
	  }	
	}

	utl_ctx_new( &utlctx, ldhses, "", UTL_PORTRAIT);
	utlctx->confirm = confirm;
	utlctx->log = log;

	sts = utl_tree_delete( objdid, ldhses, utlctx, 0, 0, 0);
	if ( sts != FOE__ABORTSEARCH)
	  if (EVEN(sts)) return sts;

	utl_ctx_delete( utlctx);
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_delete_volume()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
*
* Description: 	Delete a volume.
*
**************************************************************************/

int utl_delete_volume (
  ldh_tWBContext  ldhwb,
  char		*name,
  int		confirm,
  int		log
)
{
	utl_ctx		utlctx;
	int		sts;
	static char	yes_or_no[200];	
	pwr_tVolumeId	volid;

#if defined OS_VMS
	int		len;
  	$DESCRIPTOR ( yon_prompt_desc , "(Y/N/Q): " );
  	static $DESCRIPTOR ( yon_desc , yes_or_no );
#endif

	/* Get volid for the volume */
	sts = ldh_VolumeNameToId( ldhwb, name, &volid);
	if ( EVEN(sts)) return sts;

	utl_ctx_new( &utlctx, 0, "", UTL_PORTRAIT);
	utlctx->confirm = confirm;
	utlctx->log = log;

	if ( utlctx->confirm )
	{
	  printf("Delete volume %s ", name);
#if defined OS_VMS
 	  sts = lib$get_input ( &yon_desc , &yon_prompt_desc , &len ) ; 
	  if ( EVEN(sts)) return sts;
#else
	  // TODO
	  printf( "(Y/N/Q): ");
	  sts = scanf( "%s", yes_or_no);
#endif
	  if ( !((yes_or_no[0] == 'Y') || (yes_or_no[0] == 'y')))
	    return FOE__SUCCESS;
	}

	/* Delete the volume */
	sts = ldh_DeleteVolume( ldhwb, volid);
	if ( ODD(sts))
	{
	  if ( utlctx->log )	
	    printf( "Volume deleted %s\n", name);
	}
	else if ( EVEN(sts)) return sts;

	utl_ctx_delete( utlctx);
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_export_object()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: 	Export an object. Print the object name in a file.
*
**************************************************************************/

int utl_export_object ( 
  ldh_tSesContext ldhses,
  char		*name,
  char		*prefix,
  int		debugparameter,
  int		gms,
  char		*filename,
  int		append
)
{
	int		sts, size, j;
	pwr_tClassId	cid;
	char		*s;
	pwr_tObjid  objdid;
	FILE		*output_file;
	char		str[200];
	pwr_tOName     	hiername;
	pwr_tClassId	bodyclass;
	pwr_sGraphPlcNode 	*graphbody;
	ldh_sParDef 	*bodydef;
	int		rows;
	int		found;
	char		debugpar[32];
	pwr_tFileName	fname;

	/* Check if prefix */
	if ( prefix != NULL )
	{
	  /* Add the prefix to the object name */
	  strcpy( str, prefix);
	}
	else
	  strcpy( str, "");

	if ( name != NULL )
	{
	  /* Get objdid for the hierarchy object */
	  sts = ldh_NameToObjid( ldhses, &objdid, name);
	  if ( EVEN(sts))
	  {
	    return FOE__OBJECT;
	  }	
	  sts = ldh_ObjidToName( ldhses, 
	           	objdid, ldh_eName_Hierarchy,
  		        hiername, sizeof( hiername), &size);
  	  if ( EVEN(sts)) return sts;

	  if ( gms )
	  {
	    /* Convert the name to prooper syntax */
	    s = hiername;
	    while ( *s != 0)
	    {
	      if ( *s == '-')
	        *s = '$';
	      s++;
	    }
	  }
	  strcat( str, hiername);
	}
	  

	/* Check if debugparameter */
	if ( debugparameter )
	{
	  /* Add the debugparameter to the name */

	  /* Get the debugparameter */
	  sts = ldh_GetObjectClass( ldhses, objdid, &cid);
	  if ( EVEN(sts)) return sts;

	  sts = ldh_GetClassBody( ldhses, cid, 
		"GraphPlcNode", &bodyclass, (char **)&graphbody, &size);
	  if ( ODD(sts))
	  {
	    if ( graphbody->debugpar[0] != 0)
	      strcpy( debugpar, graphbody->debugpar);
	    else
	      strcpy( debugpar, "ActVal");
	  }
	  else
	  {
	    /* Assume that there is an actualvalue in the object */
	    strcpy( debugpar, "ActualValue");
	  }

	  sts = ldh_GetObjectBodyDef(ldhses, cid, "RtBody", 1,
	  		&bodydef, &rows);
	  if ( EVEN(sts) ) return sts;

	  found = 0;
	  for ( j = 0; j < rows; j++)
	  {
	    if (strcmp( debugpar, bodydef[j].ParName) == 0)
	    {
	      found = 1;
	      break;
	    }
	  }

	  if ( found)
	  {
	    if ( gms)
	      strcat( str, "$_");
	    else
	      strcat( str, ".");
	    strcat( str, debugpar);
	    if ( gms)
	    {
	      /* Add the parameter type with hashmarks */

              switch ( bodydef[j].Par->Output.Info.Type )
              {
                case pwr_eType_Boolean:
	          strcat( str, "##Boolean");
                  break;
                case pwr_eType_Float32:
	          strcat( str, "##Float32");
                  break;
                case pwr_eType_Float64:
	          strcat( str, "##Foat64");
                  break;
                case pwr_eType_Char:
	          strcat( str, "##Char");
                  break;
                case pwr_eType_Int8:
	          strcat( str, "##Int8");
                  break;
                case pwr_eType_Int16:
	          strcat( str, "##Int16");
                  break;
                case pwr_eType_UInt8:
	          strcat( str, "##UInt8");
                  break;
                case pwr_eType_UInt16:
	          strcat( str, "##UInt16");
                  break;
                case pwr_eType_UInt32:
	          strcat( str, "##UInt32");
                  break;
                case pwr_eType_ObjDId:
	          strcat( str, "##ObjDId");
                  break;
                case pwr_eType_String:
	          strcat( str, "##String");
                  break;
                default:
                  ;
              }
	    }
	    free((char *) bodydef);	
	  }
	}


	if ( filename != NULL)
	{
	  /* Open file */
	  dcli_translate_filename( fname, filename);
	  if ( append)
	    output_file = fopen( filename, "a");
	  else
	    output_file = fopen( filename, "w");

	  if ( output_file == 0)
	  {
	    return FOE__NOFILE;
	  }
	  fprintf( output_file, "%s\n", str);
	  fclose( output_file);
	}
	else
	  /* Print on terminal */
	  printf( "%s\n", str);

	return FOE__SUCCESS;
}

/*************************************************************************
*
* Name:		utl_realloc_s()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* Description: 	
*
**************************************************************************/

int utl_realloc_s( 	char	**list_ptr, 
			int	count, 
			int	size, 
			int	*alloc)
{
#define UTL_ALLOC 20

	char	*new_list;

	if ( count == 0)
	{
	  *list_ptr = (char *)calloc( UTL_ALLOC, size);
	  if ( *list_ptr == 0)
	    return FOE__NOMEMORY;
	  *alloc = UTL_ALLOC;
	}
	else if ( *alloc <= count)
	{
	  new_list = (char *)calloc( *alloc + UTL_ALLOC, size);
	  if ( new_list == 0)
	    return FOE__NOMEMORY;
	  memcpy( new_list, *list_ptr, count * size);
	  free( *list_ptr);
	  *list_ptr = new_list;
	  (*alloc) += UTL_ALLOC;
	}
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_realloc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* Description: 	
*
**************************************************************************/

int utl_realloc (
  char	**list_ptr,
  int	old_size,
  int	new_size
)
{
	char	*new_list;

	if ( old_size >= new_size)
	  return FOE__SUCCESS;

	if ( old_size == 0)
	{
	  *list_ptr = (char *)calloc( 1, new_size);
	  if ( *list_ptr == 0)
	    return FOE__NOMEMORY;
	}
	else
	{
	  new_list = (char *)calloc( 1, new_size);
	  if ( new_list == 0)
	    return FOE__NOMEMORY;
	  memcpy( new_list, *list_ptr, old_size);
	  free((char *) *list_ptr);
	  *list_ptr = new_list;
	}
	return FOE__SUCCESS;
}



/*************************************************************************
*
* Name:		utl_create_loadfiles()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
*
* Description: 	Create loadfiles.
*
**************************************************************************/

int utl_create_loadfiles (
  ldh_tSesContext ldhses,
  char		*volumes,
  int		allvolumes
)
{
	int		sts;
	int		i, nr;
	ldh_sSessInfo	info;
	int		status;
	int		other_volume_attached;
	ldh_tVolContext	volctx;
	ldh_tSesContext	l_ldhses;
	char		vol_str[UTL_INPUTLIST_MAX + 1][80];
	pwr_tVolumeId	volume_vect[UTL_INPUTLIST_MAX + 1];
	pwr_tClassId	vol_class;
	pwr_tVolumeId	vol_id;
	pwr_tVolumeId	current_volid;
	ldh_sVolumeInfo volinfo;

	/* Session has to be saved */
	sts = ldh_GetSessionInfo( ldhses, &info);
	if ( EVEN(sts)) return sts;
	if ( !info.Empty)
	  return GSX__NOTSAVED;

	sts = ldh_GetVolumeInfo( ldh_SessionToVol( ldhses), &volinfo);
	if (EVEN(sts)) return sts;
	current_volid = volinfo.Volume;

	if ( volumes != NULL) {
	  /* Parse the volumestr */
	  nr = utl_parse( volumes, ", ", "", (char *)vol_str, 
		sizeof( vol_str) / sizeof( vol_str[0]), sizeof( vol_str[0]));
	  if ( (nr == 0) || ( nr > UTL_INPUTLIST_MAX)) {
	    status = FOE__PARSYNT;
	    goto error_return;
	  }	

	  for ( i = 0; i < nr; i++) {
	    sts = ldh_VolumeNameToId( ldh_SessionToWB( ldhses), vol_str[i],
			&volume_vect[i]);
	    if ( EVEN(sts)) {
	      status = sts;
	      goto error_return;
	    }	
	  }
	  volume_vect[nr] = 0;
	}
	else
	  volume_vect[0] = 0;
	
	if ( allvolumes) {
	  // Get all volumes that is not class, directory and wb volumes
	  i = 0;
	  sts = ldh_GetVolumeList( ldh_SessionToWB( ldhses), &vol_id);
	  while ( ODD(sts) ) {
	    sts = ldh_GetVolumeClass( ldh_SessionToWB( ldhses), vol_id,
			&vol_class);
	    if (EVEN(sts)) return sts;

	    if ( vol_class == pwr_eClass_WorkBenchVolume ||
		 vol_class == pwr_eClass_ClassVolume ||
		 vol_class == pwr_eClass_DirectoryVolume) {
	      sts = ldh_GetNextVolume( ldh_SessionToWB( ldhses), vol_id, &vol_id);
	      continue;
	    }

	    volume_vect[i] = vol_id;
	    i++;
	    if ( i > UTL_INPUTLIST_MAX)
	      return FOE__MAXITEM;

	    sts = ldh_GetNextVolume( ldh_SessionToWB( ldhses), vol_id, &vol_id);
	  }
	  volume_vect[i] = 0;
	}

        if ( volumes == NULL && !allvolumes) {
          // Take the current volume
          volume_vect[0] = current_volid;
          volume_vect[1] = 0;
	}

	for ( i = 0; volume_vect[i] != 0; i++) {
	  other_volume_attached = 0;
	  if ( current_volid != volume_vect[i]) {
	    /* Attach this volume */
	    sts = ldh_AttachVolume( ldh_SessionToWB(ldhses), volume_vect[i], &volctx);
	    if ( EVEN(sts)) return sts;
	    other_volume_attached = 1;

	    /* Open a read session */
	    sts = ldh_OpenSession(&l_ldhses, volctx, ldh_eAccess_ReadOnly, 
				  ldh_eUtility_Pwr);
	    if ( EVEN(sts)) return sts;
	  }
	  else {
	    l_ldhses = ldhses;
	    /* Set session to ReadOnly */
	    sts = ldh_SetSession( ldhses, ldh_eAccess_ReadOnly);
	    if ( EVEN(sts)) return sts;
	  }

	  status = lfu_create_loadfile( l_ldhses);
	  if ( ODD(sts))
	    status = ldh_CreateLoadFile( l_ldhses);

	  if ( other_volume_attached) {
	      ldh_CloseSession( l_ldhses);
	      ldh_DetachVolume( ldh_SessionToWB(ldhses), volctx);
	  }
	  else {
	    /* Return to session access ReadWrite */
	    sts = ldh_SetSession( ldhses, ldh_eAccess_ReadWrite);
	    if ( EVEN(sts)) return sts;
	  }
	  if ( EVEN(status)) return status;
	}
	return FOE__SUCCESS;

error_return:
	sts = ldh_SetSession( ldhses, ldh_eAccess_ReadWrite);
	if ( EVEN(sts)) return sts;

	return status;
}


/*************************************************************************
*
* Cross document list functions.
*
**************************************************************************/


#define	CROSS_DOCALLOC	500

static	int		cross_doclist_loaded = 0;
static	crossdoc_t_list	*cross_doclist = 0;
static	int		cross_doclist_count = 0;


/*************************************************************************
*
* Name:		cross_doclist_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Add an item to the document list.
*
**************************************************************************/

static int	cross_doclist_add( 	crossdoc_t_list	**doclist, 
				int		*doclist_count, 
				pwr_tObjid	objid,
				pwr_tObjid	parent,
				float		ll_x,
				float		ll_y,
				float		ur_x,
				float		ur_y,
				char		*page)
{
	crossdoc_t_list	*doclist_ptr;

	if ( *doclist)
	{
	  doclist_ptr = *doclist;
	  while( doclist_ptr->next)
	    doclist_ptr = doclist_ptr->next;

	  doclist_ptr->next = (crossdoc_t_list *) calloc( 1, sizeof(crossdoc_t_list));
	  if ( doclist_ptr->next == 0)
	    return FOE__NOMEMORY;
	  doclist_ptr = doclist_ptr->next;
	}
	else
	{
	  doclist_ptr = (crossdoc_t_list *) calloc( 1, sizeof(crossdoc_t_list));
	  if ( doclist_ptr == 0)
	    return FOE__NOMEMORY;
	  *doclist = doclist_ptr;
	}
	doclist_ptr->objid = objid;
	doclist_ptr->parent = parent;
	doclist_ptr->ll_x = ll_x;
	doclist_ptr->ll_y = ll_y;
	doclist_ptr->ur_x = ur_x;
	doclist_ptr->ur_y = ur_y;
	strncpy( doclist_ptr->page, page, sizeof( doclist_ptr->page));
	(*doclist_count)++;
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		cross_doclist_object_insert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Insert an object in the doclist.
*
**************************************************************************/

static int cross_doclist_object_insert(
		pwr_sAttrRef	*arp,
		ldh_tSesContext	ldhses,
		int		dum1,
		int		dum2,
		int		dum3,
		int		dum4)
{
	int		sts, size;
	pwr_tClassId	cid;
	pwr_tClassId	bufclass;
	pwr_eClass	eclass;
	pwr_sGraphPlcNode 	*graphbody;
	pwr_sPlcNode	*nodebuffer;
	char		*page;

	/* Get class for the found object */
	sts = ldh_GetObjectClass( ldhses, arp->Objid, &cid);

	/* If graphmethod == 6 this is a document object */
	sts = ldh_GetClassBody( ldhses, cid, 
		"GraphPlcNode", &bufclass, (char **)&graphbody, &size);
	if ( EVEN(sts))
	  return FOE__SUCCESS;

	if ( graphbody->graphmethod != 6)
	  return FOE__SUCCESS;

	/* Store parent, koordinates and page */
	sts = ldh_GetObjectPar( ldhses, arp->Objid, "DevBody",
			"Page", (char **)&page, &size);
	if ( EVEN(sts))
	  return FOE__SUCCESS;

	sts = ldh_GetObjectBuffer( ldhses,
		arp->Objid, "DevBody", "PlcNode", &eclass,	
		(char **)&nodebuffer, &size);
	if( EVEN(sts)) return sts;

	sts = cross_doclist_add( &cross_doclist,
			&cross_doclist_count, 
			arp->Objid,
			nodebuffer->woid,
			nodebuffer->x,
			nodebuffer->y,
			nodebuffer->x + nodebuffer->width,
			nodebuffer->y + nodebuffer->height,
			page);
	if ( EVEN(sts)) return sts;
	free((char *) nodebuffer);
	free((char *) page);

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		cross_doclist_load()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Load the doc reference list.
*
**************************************************************************/
static int	cross_doclist_load( ldh_tSesContext ldhses)
{
	pwr_tStatus	sts;

	if ( cross_doclist_loaded)
	  /* Already loaded */
	  return FOE__SUCCESS;

	sts = trv_get_objects_hier_class_name( ldhses, pwr_cNObjid, 0, NULL,
		(trv_tBcFunc)cross_doclist_object_insert, ldhses, 0,0,0,0);
	if (EVEN(sts)) return sts;
	
	cross_doclist_loaded = 1;
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		cross_doclist_unload()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Unload the doc reference list.
*
**************************************************************************/

static int	cross_doclist_unload()
{
	crossdoc_t_list	*doclist_ptr;
	crossdoc_t_list	*next;

	doclist_ptr = cross_doclist;
	while ( doclist_ptr)
	{
	  next = doclist_ptr->next;
	  free( (char *) doclist_ptr);
	  doclist_ptr = next;
	}
	cross_doclist = 0;
	cross_doclist_count = 0;

	cross_doclist_loaded = 0;
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		cross_get_object_page()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* pwr_tObjid  objdid of the object
*
* Description: 	Returns the document page of a plc object.
*
**************************************************************************/

static int	cross_get_object_page(
			ldh_tSesContext ldhses,
			pwr_tObjid	objid,
			char		*page)
{
	pwr_tStatus	sts;
	pwr_tObjid	parent;
	crossdoc_t_list	*doclist_ptr;
	pwr_sPlcNode	*nodebuffer;
	pwr_eClass	bufclass;
	float		x, y;
	int		size;

	sts = cross_doclist_load( ldhses);
	if ( EVEN(sts)) return sts;

	if ( cross_doclist == 0)
	{
	  /* No document objects found */
	  *page = 0;
	  return FOE__SUCCESS;
	}

	sts = ldh_GetParent ( ldhses, objid, &parent);
	if ( EVEN(sts))
	{
	  *page = 0;
	  return FOE__SUCCESS;
	}

	/* Get the koordinates of the object */
	sts = ldh_GetObjectBuffer( ldhses,
		objid, "DevBody", "PlcNode", &bufclass,	
		(char **)&nodebuffer, &size);
	if( EVEN(sts)) 
	{
	  *page = 0;
	  return FOE__SUCCESS;
	}

	x = nodebuffer->x + nodebuffer->width/2;
	y = nodebuffer->y + nodebuffer->height/2;

	doclist_ptr = cross_doclist;
	while( doclist_ptr)
	{
	  if ( cdh_ObjidIsEqual( parent, doclist_ptr->parent))
	  {
	    if ( x >= doclist_ptr->ll_x && x <= doclist_ptr->ur_x &&
		 y >= doclist_ptr->ll_y && y <= doclist_ptr->ur_y)
	    {
	      strcpy( page, doclist_ptr->page);
	      return FOE__SUCCESS;
	    } 
	  }
	  doclist_ptr = doclist_ptr->next;
	}
	/* Not found */
	*page = 0;
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Cross reference list functions.
*
**************************************************************************/


#define	CROSS_CROSSALLOC	500

#define CROSSLIST_DI	0
#define CROSSLIST_DO	1
#define CROSSLIST_DV	2
#define CROSSLIST_AI	3
#define CROSSLIST_AO	4
#define CROSSLIST_AV	5
#define CROSSLIST_IO	6
#define CROSSLIST_II	7
#define CROSSLIST_IV	8
#define CROSSLIST_OBJ	9
#define CROSSLIST_SIZE  10

static	int		cross_crosslist_loaded = 0;
static	cross_t_list	*cross_crosslist[CROSSLIST_SIZE] = {0,0,0,0,0,0,0,0,0,0};
static	int		cross_crosslist_count[CROSSLIST_SIZE] = {0,0,0,0,0,0,0,0,0,0};


pwr_tStatus utl_replace_symbol( ldh_tSesContext ldhses, 
				pwr_tObjid oid,
				pwr_sAttrRef *arp)
{
  switch ( arp->Objid.vid) {
  case ldh_cPlcFoVolume: {
    pwr_tStatus sts;
    pwr_tOid host;

    sts = ldh_GetParent( ldhses, oid, &host);
    if ( EVEN(sts)) return sts;
    sts = ldh_GetParent( ldhses, host, &host);
    if ( EVEN(sts)) return sts;

    arp->Objid = host;
    break;
  }
  case ldh_cPlcMainVolume: {
    pwr_tStatus sts;
    pwr_tOid host;
    pwr_sAttrRef *con_arp;
    int size;

    sts = ldh_GetParent( ldhses, oid, &host);
    if ( EVEN(sts)) return sts;
    sts = ldh_GetParent( ldhses, host, &host);
    if ( EVEN(sts)) return sts;

    sts = ldh_GetObjectPar( ldhses, host, "RtBody",
			"PlcConnect", (char **)&con_arp, &size);
    if ( EVEN(sts)) return sts;

    if ( cdh_ObjidIsNull( con_arp->Objid))
      return 0;

    arp->Objid = con_arp->Objid;
    arp->Offset += con_arp->Offset;
    
    free( (char *)con_arp);
    break;
  }
  case ldh_cIoConnectVolume:
    break;
  default:
    ;
  }
  return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		cross_crosslist_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Add an item to the crosslist.
*
**************************************************************************/

static int	cross_crosslist_add( 	cross_t_list	**crosslist, 
				int		*crosslist_count, 
				int		dum, 
				pwr_sAttrRef	*arp, 
				pwr_sAttrRef	*refarp,
				unsigned long	specification)
{
	cross_t_list	*crosslist_ptr;

	if ( *crosslist)
	{
	  crosslist_ptr = *crosslist;
	  while( crosslist_ptr->next)
	    crosslist_ptr = crosslist_ptr->next;

	  crosslist_ptr->next = (cross_t_list *) calloc( 1, sizeof(cross_t_list));
	  if ( crosslist_ptr->next == 0)
	    return FOE__NOMEMORY;
	  crosslist_ptr = crosslist_ptr->next;
	}
	else
	{
	  crosslist_ptr = (cross_t_list *) calloc( 1, sizeof(cross_t_list));
	  if ( crosslist_ptr == 0)
	    return FOE__NOMEMORY;
	  *crosslist = crosslist_ptr;
	}
	crosslist_ptr->o = *arp;
	crosslist_ptr->refo = *refarp;
	crosslist_ptr->specification = specification;
	(*crosslist_count)++;
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		cross_crosslist_object_insert()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Insert an object in the crosslist.
*
**************************************************************************/

static int	cross_crosslist_object_insert(
		pwr_sAttrRef	*arp,
		ldh_tSesContext	ldhses,
		int		dum1,
		int		dum2,
		int		dum3,
		int		dum4)
{
  static crr_t_searchlist	searchlist[] = {
    { pwr_cClass_plc, "DevBody", "ResetObject", CRR_READ, CROSSLIST_DV},
    { pwr_cClass_resdv, "DevBody", "DvObject", CRR_WRITE, CROSSLIST_DV},
    { pwr_cClass_setdv, "DevBody", "DvObject", CRR_WRITE, CROSSLIST_DV},
    { pwr_cClass_stodv, "DevBody", "DvObject", CRR_WRITE, CROSSLIST_DV},
    { pwr_cClass_GetDv, "DevBody", "DvObject", CRR_READ, CROSSLIST_DV},
    { pwr_cClass_GetDo, "DevBody", "DoObject", CRR_READ, CROSSLIST_DO},
    { pwr_cClass_resdo, "DevBody", "DoObject", CRR_WRITE, CROSSLIST_DO},
    { pwr_cClass_setdo, "DevBody", "DoObject", CRR_WRITE, CROSSLIST_DO},
    { pwr_cClass_stodo, "DevBody", "DoObject", CRR_WRITE, CROSSLIST_DO},
    { pwr_cClass_GetDi, "DevBody", "DiObject", CRR_READ, CROSSLIST_DI},
    { pwr_cClass_cstoav, "DevBody", "AvObject", CRR_WRITE, CROSSLIST_AV},
    { pwr_cClass_GetAv, "DevBody", "AvObject", CRR_READ, CROSSLIST_AV},
    { pwr_cClass_stoav, "DevBody", "AvObject", CRR_WRITE, CROSSLIST_AV},
    { pwr_cClass_cstoao, "DevBody", "AoObject", CRR_WRITE, CROSSLIST_AO},
    { pwr_cClass_GetAo, "DevBody", "AoObject", CRR_READ, CROSSLIST_AO},
    { pwr_cClass_stoao, "DevBody", "AoObject", CRR_WRITE, CROSSLIST_AO},
    { pwr_cClass_GetAi, "DevBody", "AiObject", CRR_READ, CROSSLIST_AI},
    { pwr_cClass_pos3p, "DevBody", "DoOpen", CRR_WRITE, CROSSLIST_DO},
    { pwr_cClass_pos3p, "DevBody", "DoClose", CRR_WRITE, CROSSLIST_DO},
    { pwr_cClass_inc3p, "DevBody", "DoOpen", CRR_WRITE, CROSSLIST_DI},
    { pwr_cClass_inc3p, "DevBody", "DoClose", CRR_WRITE, CROSSLIST_DO},
    { pwr_cClass_stodp, "DevBody", "Object", CRR_WRITE, CROSSLIST_OBJ},
    { pwr_cClass_setdp, "DevBody", "Object", CRR_WRITE, CROSSLIST_OBJ},
    { pwr_cClass_resdp, "DevBody", "Object", CRR_WRITE, CROSSLIST_OBJ},
    { pwr_cClass_GetDp, "DevBody", "DpObject", CRR_READ, CROSSLIST_OBJ},
    { pwr_cClass_cstoap, "DevBody", "Object", CRR_WRITE, CROSSLIST_OBJ},
    { pwr_cClass_GetAp, "DevBody", "ApObject", CRR_READ, CROSSLIST_OBJ},
    { pwr_cClass_stoap, "DevBody", "Object", CRR_WRITE, CROSSLIST_OBJ},
    { pwr_cClass_CStoIp, "DevBody", "Object", CRR_WRITE, CROSSLIST_OBJ},
    { pwr_cClass_GetIp, "DevBody", "IpObject", CRR_READ, CROSSLIST_OBJ},
    { pwr_cClass_StoIp, "DevBody", "Object", CRR_WRITE, CROSSLIST_OBJ},
    { pwr_cClass_stoii, "DevBody", "IiObject", CRR_WRITE, CROSSLIST_II},
    { pwr_cClass_stoio, "DevBody", "IoObject", CRR_WRITE, CROSSLIST_IO},
    { pwr_cClass_stoiv, "DevBody", "IvObject", CRR_WRITE, CROSSLIST_IV},
    { pwr_cClass_cstoii, "DevBody", "IiObject", CRR_WRITE, CROSSLIST_II},
    { pwr_cClass_cstoio, "DevBody", "IoObject", CRR_WRITE, CROSSLIST_IO},
    { pwr_cClass_cstoiv, "DevBody", "IvObject", CRR_WRITE, CROSSLIST_IV},
    { pwr_cClass_GetIi, "DevBody", "IiObject", CRR_READ, CROSSLIST_II},
    { pwr_cClass_GetIo, "DevBody", "IoObject", CRR_READ, CROSSLIST_IO},
    { pwr_cClass_GetIv, "DevBody", "IvObject", CRR_READ, CROSSLIST_IV},
    { pwr_cClass_ExternRef, "DevBody", "Object", CRR_GETFROMOBJECT, CROSSLIST_OBJ},
    { pwr_cClass_reset_so, "DevBody", "OrderObject", CRR_READ, CROSSLIST_OBJ},
    { pwr_cClass_GetData, "DevBody", "DataObject", CRR_REF, CROSSLIST_OBJ},
    { 0, }};

	int		sts, size;
	pwr_tClassId	cid;
	crr_t_searchlist *searchlist_ptr;
	pwr_sAttrRef	*aref_ptr;
	pwr_sAttrRef	aref;
	pwr_tClassId	refobjid_class;
	unsigned char	*write_ptr;
	unsigned long	write;
	
	/* Get class for the found object */
	sts = ldh_GetObjectClass( ldhses, arp->Objid, &cid);
	
	searchlist_ptr = searchlist;
	while ( searchlist_ptr->cid != 0 ) {
	  if ( cid == searchlist_ptr->cid) {
	    /* Check if the objdid in the parameter is correct */
	    sts = ldh_GetObjectPar( ldhses, arp->Objid, searchlist_ptr->body, 
			searchlist_ptr->attr, (char **)&aref_ptr, &size);
	    if ( EVEN(sts)) return sts;

	    aref = *aref_ptr;
	    free((char *) aref_ptr);

	    utl_replace_symbol( ldhses, arp->Objid, &aref);

	    if ( cdh_ObjidIsNotNull( aref.Objid)) {
	      /* Check if object exists */
	      sts = ldh_GetAttrRefTid( ldhses, &aref, &refobjid_class);
	      if ( ODD(sts)) {

		/* If attribute, get object or attrobject */
		if ( !cdh_tidIsCid( refobjid_class)) {
		  char *np, *s;
		  int nsize;

		  sts = ldh_AttrRefToName( ldhses, &aref, 
					   ldh_eName_Hierarchy, &np, &nsize);
		  if ( EVEN(sts)) return FOE__SUCCESS;
		  if ((s = strrchr( np, '.')))
		    *s = 0;
		  sts = ldh_NameToAttrRef( ldhses, np, &aref);
		  if ( EVEN(sts)) return FOE__SUCCESS;
		}
	        if ( searchlist_ptr->write == CRR_GETFROMOBJECT) {
	          sts = ldh_GetObjectPar( ldhses, arp->Objid, "DevBody", 
			"Write", (char **)&write_ptr, &size);
	          if ( EVEN(sts)) return sts;
	          if ( *write_ptr)
	            write = 1;
	          else
	  	    write = 0;
	          free((char *) write_ptr);
	        }
	        else
	          write = searchlist_ptr->write;

	        /* Store in crosslist */
		sts = cross_crosslist_add( 
		        &cross_crosslist[searchlist_ptr->list], 
			&cross_crosslist_count[searchlist_ptr->list], 0,
			&aref, arp, write); 
	        if ( EVEN(sts)) return sts;
	      }
	    }
	  }
	  searchlist_ptr++;
	}

	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		cross_crosslist_load()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Load thes cross reference list.
*
**************************************************************************/
static int cross_crosslist_load( ldh_tSesContext ldhses)
{
	pwr_tStatus	sts;
	pwr_tVolumeId	volume_vect[UTL_INPUTLIST_MAX + 1];
	pwr_tVolumeId	*volume_p;
	trv_tCtx       	trvctx;
	pwr_tClassId	vol_class;
	pwr_tVolumeId	vol_id;
	int		i;
	int		allvolumes = 1;

	if ( cross_crosslist_loaded)
	  /* Already loaded */
	  return FOE__SUCCESS;

	if ( allvolumes)
	{
	  /* Get all volumes that is not class and wb volumes */
	  i = 0;
	  sts = ldh_GetVolumeList( ldh_SessionToWB( ldhses), &vol_id);
	  while ( ODD(sts) )
	  {
	    sts = ldh_GetVolumeClass( ldh_SessionToWB( ldhses), vol_id,
			&vol_class);
	    if (EVEN(sts)) return sts;

	    if ( !(vol_class == pwr_eClass_ClassVolume ||
 	        vol_class == pwr_eClass_WorkBenchVolume ))
	    {
	      volume_vect[i] = vol_id;
	      i++;
	      if ( i > UTL_INPUTLIST_MAX)
	        return FOE__PARSYNT;
	    }
	    sts = ldh_GetNextVolume( ldh_SessionToWB( ldhses), vol_id, &vol_id);
	  }
	  volume_vect[i] = 0;
	  volume_p = volume_vect;
	}
	else
	  volume_p = NULL;

	sts = trv_create_ctx( &trvctx, ldhses, pwr_cNObjid, 0, NULL,
		volume_p);
	if ( EVEN(sts)) return sts;

	sts = trv_object_search( trvctx,
		(trv_tBcFunc) cross_crosslist_object_insert, ldhses, 0,0,0,0);
	if (EVEN(sts)) return sts;
	
	sts = trv_delete_ctx( trvctx);

	cross_crosslist_loaded = 1;
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		cross_crosslist_unload()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	Unload thes cross reference list.
*
**************************************************************************/

static int	cross_crosslist_unload()
{
	int	i;
	cross_t_list	*crosslist_ptr;
	cross_t_list	*next;

	for ( i = 0; i < CROSSLIST_SIZE; i++)
	{
	  crosslist_ptr = cross_crosslist[i];
	  while ( crosslist_ptr)
	  {
	    next = crosslist_ptr->next;
	    free( (char *) crosslist_ptr);
	    crosslist_ptr = next;
	  }
	  cross_crosslist[i] = 0;
	  cross_crosslist_count[i] = 0;
	}
	cross_crosslist_loaded = 0;
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		crr_refobject()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description: 	
*
**************************************************************************/

static int crr_refobject (
  utl_ctx	utlctx,
  pwr_tObjid	Objdid,
  utl_t_list	**crrlist,
  int		*crrcount
)
{
	pwr_tStatus	sts;
	cross_t_list	*crosslist_ptr;
	cross_t_list	*cr_ptr;
	int		cr_index;
	ldh_tSesContext ldhses;
	int		found;

	ldhses = utlctx->ldhses;

	sts = cross_crosslist_load( ldhses);
	if ( EVEN(sts)) return sts;

	*crrcount = 0;
	*crrlist = 0;

	/* Select crosslist */
	cr_index = CROSSLIST_OBJ;

	crosslist_ptr = cross_crosslist[cr_index];
	while ( crosslist_ptr)
	{
	  /* Check that object is not already found */
	  cr_ptr = cross_crosslist[cr_index];
	  found = 0;
	  while ( cr_ptr)
	  {
	    if ( cdh_ArefIsEqual( &cr_ptr->o, &crosslist_ptr->o))
	    {
	      if ( cr_ptr != crosslist_ptr)
	        found = 1;
	      break;
	    }
	    cr_ptr = cr_ptr->next;
	  }

	  if ( !found)
	    utl_list_insert( crrlist, crrcount, &crosslist_ptr->o, 
			crosslist_ptr->specification, 0, 0);

	  crosslist_ptr = crosslist_ptr->next;
	}
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		crr_crossref_children()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* pwr_tObjid  objdid		I	objdid of the object
* utl_t_list	** crrlist	IO	list of found crossreferenses.
* int *		crrcount	IO	count of references in the list.
*
* Description: 	Crossreference the children of a window object.
*
**************************************************************************/

static int	crr_crossref_children(
			ldh_tSesContext ldhses,
			pwr_tObjid	objid,
			utl_t_list	**crrlist,
			int		*crrcount)
{
	pwr_tStatus	sts;
	cross_t_list	*crosslist_ptr;
	int		cr_index;
	utl_t_list	*crrlist_ptr;
	pwr_tClassId	cid;
	
	sts = cross_crosslist_load( ldhses);
	if ( EVEN(sts)) return sts;

	*crrcount = 0;
	*crrlist = 0;

	sts = ldh_GetObjectClass( ldhses, objid, &cid);
	if ( EVEN(sts)) return sts;

	/* Select crosslist */
	cr_index = CROSSLIST_OBJ;

	crosslist_ptr = cross_crosslist[cr_index];
	while ( crosslist_ptr) {
	  if ( cdh_ObjidIsNotNull( crosslist_ptr->parent)) {
	    sts = ldh_GetParent ( ldhses, crosslist_ptr->o.Objid, 
			&crosslist_ptr->parent);
	    if ( EVEN(sts)) {
	      crosslist_ptr = crosslist_ptr->next;
	      continue;
	    }
	  }
	  if ( cdh_ObjidIsEqual( crosslist_ptr->parent, objid)) {
	    utl_list_insert( crrlist, crrcount, &crosslist_ptr->refo, 
			crosslist_ptr->specification, 0, 0);
	    /* Set the current object as refobj */
	    crrlist_ptr = *crrlist;
	    while ( crrlist_ptr->next)
	      crrlist_ptr = crrlist_ptr->next;
	    crrlist_ptr->refo = crosslist_ptr->o;
	  }
	  crosslist_ptr = crosslist_ptr->next;
	}
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		crr_crossref()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext ldhses		I	ldh session.
* pwr_tObjid  objdid		I	objdid of the object
* utl_t_list	** crrlist	IO	list of found crossreferenses.
* int *		crrcount	IO	count of references in the list.
*
* Description: 	Crossreference method for a common object.
*
**************************************************************************/

static int	crr_crossref(
			ldh_tSesContext ldhses,
			pwr_sAttrRef	*arp,
			utl_t_list	**crrlist,
			int		*crrcount
)
{
	pwr_tStatus	sts;
	cross_t_list	*crosslist_ptr;
	int		cr_index;
	utl_t_list	*crrlist_ptr;
	pwr_tClassId	cid;

	sts = cross_crosslist_load( ldhses);
	if ( EVEN(sts)) return sts;

	*crrcount = 0;
	*crrlist = 0;

	sts = ldh_GetAttrRefTid( ldhses, arp, &cid);
	if ( EVEN(sts)) return sts;

	/* Select crosslist */
	if ( cid == pwr_cClass_Di)
	  cr_index = CROSSLIST_DI;
	else if ( cid == pwr_cClass_Do ||
	          cid == pwr_cClass_Po)
	  cr_index = CROSSLIST_DO;
	else if ( cid == pwr_cClass_Dv)
	  cr_index = CROSSLIST_DV;
	else if ( cid == pwr_cClass_Ai)
	  cr_index = CROSSLIST_AI;
	else if ( cid == pwr_cClass_Ao)
	  cr_index = CROSSLIST_AO;
	else if ( cid == pwr_cClass_Av)
	  cr_index = CROSSLIST_AV;
	else if ( cid == pwr_cClass_Ii)
	  cr_index = CROSSLIST_II;
	else if ( cid == pwr_cClass_Io)
	  cr_index = CROSSLIST_IO;
	else if ( cid == pwr_cClass_Iv)
	  cr_index = CROSSLIST_IV;
	else
	  cr_index = CROSSLIST_OBJ;

	crosslist_ptr = cross_crosslist[cr_index];
	while ( crosslist_ptr) {
	  if ( cdh_ArefIsEqual( &crosslist_ptr->o, arp)) {
	    sts = utl_list_insert( crrlist, crrcount, &crosslist_ptr->refo, 
			crosslist_ptr->specification, 0, 0);
	    if ( EVEN(sts)) return sts;
	    /* Set the current object as refobj */
	    crrlist_ptr = *crrlist;
	    while ( crrlist_ptr->next)
	      crrlist_ptr = crrlist_ptr->next;
	    crrlist_ptr->refo = *arp;
	  }
	  crosslist_ptr = crosslist_ptr->next;
	}
	return FOE__SUCCESS;
}


/*************************************************************************
*
* Name:		utl_set_template()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* pwr_tObjid  objdid		I	objdid for the object.
* ldh_tSesContext ldhses		I	ldh session.
* unsigned long	utlctx		I	output specification.
*
*
* Description: 	
*
**************************************************************************/

int utl_set_template( 	
  ldh_tSesContext	ldhses, 
  int			signalobjectsegments,
  int			showsigchancon,
  int			sigchanconsegments,
  int			showdetecttext
)
{
#define UTL_MODIFY_OBJECTSEGMENTS( klass, attribute)\
	  sts = ldh_NameToObjid( ldhses, &objid,\
		"pwrb:Class-"#klass"-Template");\
	  if ( EVEN(sts)) return sts;\
	  sts = ldh_SetObjectPar( ldhses, objid, "DevBody",\
		attribute,\
		(char *)&signalobjectsegments, sizeof( signalobjectsegments));\
	  if ( EVEN(sts)) return sts;

#define UTL_MODIFY_SIGCHANCONSEGMENTS( klass)\
	  sts = ldh_NameToObjid( ldhses, &objid,\
		"pwrb:Class-"#klass"-Template");\
	  if ( EVEN(sts)) return sts;\
	  sts = ldh_SetObjectPar( ldhses, objid, "DevBody",\
		"SigChanConSegments",\
		(char *)&sigchanconsegments, sizeof( sigchanconsegments));\
	  if ( EVEN(sts)) return sts;

#define UTL_MODIFY_SHOWSIGCHANCON( klass)\
	  sts = ldh_NameToObjid( ldhses, &objid,\
		"pwrb:Class-"#klass"-Template");\
	  if ( EVEN(sts)) return sts;\
	  sts = ldh_SetObjectPar( ldhses, objid, "DevBody",\
		"ShowSigChanCon",\
		(char *)&value, sizeof( value));\
	  if ( EVEN(sts)) return sts;

#define UTL_MODIFY_SHOWDETECTTEXT( klass)\
	  sts = ldh_NameToObjid( ldhses, &objid,\
		"pwrb:Class-"#klass"-Template");\
	  if ( EVEN(sts)) return sts;\
	  sts = ldh_SetObjectPar( ldhses, objid, "DevBody",\
		"ShowDetectText",\
		(char *)&value, sizeof( value));\
	  if ( EVEN(sts)) return sts;

	pwr_tObjid	objid;
	int		sts;
	pwr_tBoolean	value;

	if ( signalobjectsegments != 0)
	{
	  UTL_MODIFY_OBJECTSEGMENTS( GetDi, "DiObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetDo, "DoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetDv, "DvObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetDp, "DpObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetAi, "AiObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetAo, "AoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetAv, "AvObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetAp, "ApObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetIp, "IpObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( GetPi, "CoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoDi, "DiObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( SetDi, "DiObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( ResDi, "DiObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoDo, "DoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( SetDo, "DoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( ResDo, "DoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoDv, "DvObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( SetDv, "DvObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( ResDv, "DvObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoDp, "ObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( SetDp, "ObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( ResDp, "ObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoAi, "AiObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoAo, "AoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoAv, "AvObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoAp, "ObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoIp, "ObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( StoPi, "CoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( CStoAi, "AiObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( CStoAo, "AoObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( CStoAv, "AvObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( CStoAp, "ObjectSegments");
	  UTL_MODIFY_OBJECTSEGMENTS( CStoIp, "ObjectSegments");
	}

	if ( sigchanconsegments != 0)
	{
	  UTL_MODIFY_SIGCHANCONSEGMENTS( GetDi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( GetDo);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( GetAi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( GetAo);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( GetPi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( StoDi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( SetDi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( ResDi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( StoDo);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( SetDo);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( ResDo);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( StoAi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( StoAo);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( StoPi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( CStoAi);
	  UTL_MODIFY_SIGCHANCONSEGMENTS( CStoAo);
	}

	if ( showsigchancon)
	{
	  value = showsigchancon - 1;
	  UTL_MODIFY_SHOWSIGCHANCON( GetDi);
	  UTL_MODIFY_SHOWSIGCHANCON( GetDo);
	  UTL_MODIFY_SHOWSIGCHANCON( GetAi);
	  UTL_MODIFY_SHOWSIGCHANCON( GetAo);
	  UTL_MODIFY_SHOWSIGCHANCON( GetPi);
	  UTL_MODIFY_SHOWSIGCHANCON( StoDi);
	  UTL_MODIFY_SHOWSIGCHANCON( SetDi);
	  UTL_MODIFY_SHOWSIGCHANCON( ResDi);
	  UTL_MODIFY_SHOWSIGCHANCON( StoDo);
	  UTL_MODIFY_SHOWSIGCHANCON( SetDo);
	  UTL_MODIFY_SHOWSIGCHANCON( ResDo);
	  UTL_MODIFY_SHOWSIGCHANCON( StoAi);
	  UTL_MODIFY_SHOWSIGCHANCON( StoAo);
	  UTL_MODIFY_SHOWSIGCHANCON( StoPi);
	  UTL_MODIFY_SHOWSIGCHANCON( CStoAi);
	  UTL_MODIFY_SHOWSIGCHANCON( CStoAo);
	}
	if ( showdetecttext)
	{
	  value = showdetecttext - 1;
	  UTL_MODIFY_SHOWDETECTTEXT( DSup);
	  UTL_MODIFY_SHOWDETECTTEXT( ASup);
	}
	return FOE__SUCCESS;
}

void	utl_objidstr_to_filestr( 
			char *instr, 
			char *outstr)
{
	char	str[80];
	char	*t;

	strcpy( outstr, instr + 2);
	t = str;
	for ( t = outstr; *t != 0; t++)
	{
	  if ( !isdigit( *t))
	    *t = '_';
	}
}

/*************************************************************************
*
* Name:		utl_read_line()
*
* Type		void
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Read a line for a file.
*
**************************************************************************/

pwr_tStatus utl_read_line(	char	*line,
					int	maxsize,
					FILE	*file,
					int	*line_count)
{ 
	char	*s;

	for (;;)
	{
	  if (fgets( line, maxsize, file) == NULL)
	    return 0;
	  if ( line_count)
	    (*line_count)++;
	  if ( line[0] != '!')
	  {
	    s = strchr( line, 10);
	    if ( s != 0)
	      *s = 0;
	    break;
	  }
	}
	return 1;
}
