#ifndef rt_io_base_h
#define rt_io_base_h

/* rt_io_base.h -- includefile for io base.

   PROVIEW/R
   Copyright (C) 1994 by Comator Process AB.

   <Description>.  */

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef pwr_class_h
#include "pwr_class.h"
#endif

#ifndef PWR_BASECLASSES_H
#include "pwr_baseclasses.h"
#endif

typedef struct io_sCtx *io_tCtx;

#ifndef rt_io_supervise_h
#include "rt_io_supervise.h"
#endif

#ifndef NULL
#define NULL (void *) 0
#endif

#define IO_CHANLIST_SIZE 32

#define FIXOUT 2
#define IO_REBOOT       1       /* Reboot the machine */

extern  pwr_tBoolean io_writeerr;
extern  pwr_tBoolean io_readerr;
extern  pwr_tBoolean io_fatal_error;

typedef struct {
  pwr_tObjName MethodName;
  pwr_tStatus (*Method)();
} pwr_sMethodBinding;

typedef struct {
  pwr_tObjName ClassName;
  pwr_sMethodBinding (*Methods)[];
} pwr_sClassBinding;

/* Base methods */
#if defined (__DECC) || defined (OS_LYNX)  || defined (OS_LINUX)
# define pwr_BindIoMethods(Class) pwr_sMethodBinding pwr_g ## Class ## _IoMethods[]
# define pwr_BindIoClasses(Type) pwr_sClassBinding pwr_g ## Type ## _IoClassMethods[]
# define pwr_BindIoClass(Class) {#Class, (void *)pwr_g ## Class ## _IoMethods}
# define pwr_BindIoMethod(Method) {#Method, (pwr_tStatus (*)())Method}
#else
# define pwr_BindIoMethods(Class) pwr_sMethodBinding pwr_g/**/Class/**/_IoMethods[]
# define pwr_BindIoClasses(Type) pwr_sClassBinding pwr_g/**/Type/**/_IoClassMethods[]
# define pwr_BindIoClass(Class) {"Class", pwr_g/**/Class/**/_IoMethods}
# define pwr_BindIoMethod(Method) {"Method", (pwr_tStatus (*)())Method}
#endif

/* User methods */
#if defined (__DECC) || defined (OS_LYNX) || defined(OS_LINUX)
#define pwr_BindIoUserMethods(Class) pwr_sMethodBinding pwr_g ## Class ## _IoUserMethods[]
#define pwr_BindIoUserClasses(Type) pwr_sClassBinding pwr_g ## Type ## _IoUserClassMethods[]
#define pwr_BindIoUserClass(Class) {#Class, (void *)pwr_g ## Class ## _IoUserMethods}
#define pwr_BindIoUserMethod(Method) {#Method, (pwr_tStatus (*)())Method}
#else
#define pwr_BindIoUserMethods(Class) pwr_sMethodBinding pwr_g/**/Class/**/_IoUserMethods[]
#define pwr_BindIoUserClasses(Type) pwr_sClassBinding pwr_g/**/Type/**/_IoUserClassMethods[]
#define pwr_BindIoUserClass(Class) {"Class", pwr_g/**/Class/**/_IoUserMethods}
#define pwr_BindIoUserMethod(Method) {"Method", (pwr_tStatus (*)())Method}
#endif

#define pwr_NullMethod {"", NULL}

#define pwr_NullClass {"", NULL}


typedef enum {
	io_eType_Node,
	io_eType_Agent,
	io_eType_Rack,
	io_eType_Card
} io_eType;

typedef enum {
	io_mAction_None		= 0,
	io_mAction_Read		= 1 << 0,
	io_mAction_Write	= 1 << 1
} io_mAction;

typedef enum {
	io_mProcess_None	= 0,
	io_mProcess_Plc		= 1 << 0,
	io_mProcess_IoComm	= 1 << 1,
	io_mProcess_User	= 1 << 2,
	io_mProcess_All		= ~0
} io_mProcess;

typedef struct {
	void		*cop;		/* Pointer to channel object */
	pwr_tDlid	ChanDlid;	/* Dlid for pointer to channel */
	pwr_tObjid	ChanObjid;	/* Objid for channel */
	void		*sop;		/* Pointer to signal object */
	pwr_tDlid	SigDlid;	/* Dlid for pointer to signal */
	pwr_tObjid	SigObjid;	/* Objid for signal */
	void		*vbp;		/* Pointer to valuebase for signal */
	void		*abs_vbp;	/* Pointer to absvaluebase (Co only) */
	pwr_tClassId	ChanClass;	/* Class of channel object */
	pwr_tClassId	SigClass;	/* Class of signal object */
} io_sChannel;
	

typedef struct s_Card {
	pwr_tClassId	Class;		/* Class of card object */
	pwr_tObjid	Objid;		/* Objid of card object */
	pwr_tString80	Name;		/* Full name of card object */
	io_mAction	Action;		/* Type of method defined (Read/Write)*/
	io_mProcess	Process;	/* Process number */
	pwr_tStatus	(* Init) ();	/* Init method */
	pwr_tStatus	(* Close) ();	/* Close method */
	pwr_tStatus	(* Read) ();	/* Read method */
	pwr_tStatus	(* Write) ();	/* Write method */
	pwr_tAddress	*op;		/* Pointer to card object */
	pwr_tDlid	Dlid;		/* Dlid f�r card object pointer */
	int		scan_interval;	/* Interval between scans */
	int		scan_interval_cnt;/* Counter to detect next time to scan */
	int		AgentControlled;/* TRUE if kontrolled by agent */
	int		ChanListSize;	/* Size of chanlist */
	io_sChannel	*chanlist; 	/* Array of channel structures */
	void		*Local;		/* Pointer to method defined data structure */
	struct s_Card	*next;		/* Next card */
} io_sCard;

typedef struct s_Rack {
	pwr_tClassId	Class;		/* Class of rack object */
	pwr_tObjid	Objid;		/* Objid of rack object */
	pwr_tString80	Name;		/* Full name of rack object */
	io_mAction	Action;		/* Type of method defined (Read/Write)*/
	io_mProcess	Process;	/* Process number */
	pwr_tStatus	(* Init) ();	/* Init method */
	pwr_tStatus	(* Close) ();	/* Close method */
	pwr_tStatus	(* Read) ();	/* Read method */
	pwr_tStatus	(* Write) ();	/* Write method */
	void		*op;		/* Pointer to rack object */
	pwr_tDlid	Dlid;		/* Dlid f�r rack object pointer */
	int		scan_interval;	/* Interval between scans */
	int		scan_interval_cnt;/* Counter to detect next time to scan */
	int		AgentControlled;/* TRUE if kontrolled by agent */
	io_sCard	*cardlist;	/* List of card structures */
	void		*Local;		/* Pointer to method defined data structure */
	struct s_Rack	*next;		/* Next rack */
} io_sRack;

typedef struct s_Agent {
	pwr_tClassId	Class;		/* Class of agent object */
	pwr_tObjid	Objid;		/* Objid of agent object */
	pwr_tString80	Name;		/* Full name of agent object */ 
	io_mAction	Action;		/* Type of method defined (Read/Write)*/
	io_mProcess	Process;	/* Process number */
	pwr_tStatus	(* Init) ();	/* Init method */
	pwr_tStatus	(* Close) ();	/* Close method */
	pwr_tStatus	(* Read) ();	/* Read method */
	pwr_tStatus	(* Write) ();	/* Write method */
	void		*op;		/* Pointer to agent object */
	pwr_tDlid	Dlid;		/* Dlid for agent object pointer */
	int		scan_interval;	/* Interval between scans */
	int		scan_interval_cnt;/* Counter to detect next time to scan */
	io_sRack	*racklist;	/* List of rack structures */
	void		*Local;		/* Pointer to method defined data structure */
	struct s_Agent	*next;		/* Next agent */
} io_sAgent;

struct io_sCtx {
	io_sAgent	*agentlist;	/* List of agent structures */
	io_mProcess	Process;	/* Callers process number */
	pwr_tObjid      Thread;         /* Callers thread objid */
	int		RelativVector;	/* Used by plc */
	pwr_sNode	*Node;		/* Pointer to node object */
	pwr_sClass_IOHandler	*IOHandler;		/* Pointer to IO Handler object */
	float		ScanTime;	/* Scantime supplied by caller */
	io_tSupCtx	SupCtx;		/* Context for supervise object lists */
};

/*----------------------------------------------------------------------------*\
  Io functions
\*----------------------------------------------------------------------------*/

void io_DiUnpackWord( 
  io_sCard	*cp,
  pwr_tUInt16	data,
  pwr_tUInt16	mask,
  int		index
);

void io_DoPackWord( 
  io_sCard	*cp,
  pwr_tUInt16	*data,
  int		index
);

pwr_tStatus io_init( 
  io_mProcess	process,
  pwr_tObjid    thread,
  io_tCtx 	*ctx,
  int		relativ_vector,
  float		scan_time
);

pwr_tStatus io_read(
  io_tCtx 	ctx
);

pwr_tStatus io_write(
  io_tCtx 	ctx
);

pwr_tStatus io_close(
  io_tCtx 	ctx
);

pwr_tStatus io_AiRangeToCoef( 
  io_sChannel 	*chanp
);

pwr_tStatus io_AoRangeToCoef( 
  io_sChannel 	*chanp
);

void io_ConvertAi (
  pwr_sClass_ChanAi  *cop,
  pwr_tInt16	      rawvalue,
  pwr_tFloat32	      *actvalue_p
);

void io_ConvertAit (
  pwr_sClass_ChanAit  *cop,
  pwr_tInt16	      rawvalue,
  pwr_tFloat32	      *actvalue_p
);

pwr_tStatus io_init_signals( 
  void
);

pwr_tStatus io_get_iohandler_object (
  pwr_sClass_IOHandler	**ObjPtr,
  pwr_tObjid *oid
);

pwr_tStatus io_GetIoTypeClasses( 
  io_eType	type,
  pwr_tClassId 	**classes,
  int		*size
);
#endif
