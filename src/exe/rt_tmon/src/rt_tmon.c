/* rt_tmon.c -- Timer Monitor

   PROVIEW/R
   Copyright (C) 1997-98 by Mandator AB.

   .  */

#if defined(OS_ELN)
# include $vaxelnc
# include stdio.h
# include stddef.h
# include stdlib.h
# include string.h
#else
# include <stdio.h>
# include <stddef.h>
# include <stdlib.h>
# include <string.h>
#endif

#ifdef OS_VMS
# include <starlet.h>
#endif

#include "pwr.h"
#include "pwr_lst.h"
#include "co_time.h"
#include "rt_qcom.h"
#include "rt_cvol.h"
#include "rt_cvolcm.h"
#include "rt_gdb.h"
#include "rt_pool.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "rt_errh.h"
#include "rt_sancm.h"
#include "rt_sansm.h"
#include "rt_subcm.h"
#include "rt_subsm.h"
#include "rt_aproc.h"
#include "rt_pwr_msg.h"
#include "rt_ini_event.h"
#include "rt_qcom.h"
#include "rt_qcom_msg.h"

typedef struct s_Timer	    sTimer;

LstType(sTimer);

struct s_Timer {
  LstLink(sTimer)	ll;
  time_tClock		clock;
  pwr_tBoolean		wrapped;
  void			*data;
  void			(*exec)();
};

#if 0
typedef struct s_TimerEntry sTimerEntry;
struct s_TimerEntry {
  tree_sNode		node;
  time_tClock		clock;
  LstHead(sTimer)	lh;
};
static tree_sTable	*teq;
#endif

static LstHead(sTimer)	timer_lh;
static LstHead(sTimer)	wrap_lh;
static LstHead(sTimer)	free_lh;

static time_tClock	now_clock;
static time_tClock	last_clock;


#ifdef OS_VMS
  static int		timer_flag;
#endif

static void
event (
  qcom_sGet	*get
);

static void
sancAdd (
  sTimer	*tp
);

static void
sancExpired (
  sTimer	*tp
);

static void
sansCheck (
  sTimer	*tp
);

static void
subbCheck (
  sTimer	*tp
);

static void
subcCheck (
  sTimer	*tp
);

static void
cacheTrim (
  sTimer	*tp
);

static void
insertTimer (
  sTimer		*tp
);

static sTimer *
newTimer (
  time_tClock		*clock,
  void			*data,
  void			(*exec)()
);

static void
freeTimer (
  sTimer		*tp
);

static sTimer *
allocTimer (
);

static void
setTimer (
  sTimer		*tp,
  time_tClock		offs
);

static time_tClock
msToClock (
  time_tClock		*ic,
  int			ms
);

static void
waitClock (
  time_tClock		c
);


static void
setInterval (		
  time_tClock		*c,
  pwr_tUInt32		i
);


static void
executeExpired (
  LstHead(sTimer)	*lh,
  pwr_tBoolean		force
);

static void
getNewTimers (
);

static void
getWaitClock(
  time_tClock		*wait_clock,
  time_tClock		last_clock
);

static void
init(
);

static void
toggleWrapped (
);


#if 0
time_tClock
_time_Clock (
  pwr_tStatus *status,
  pwr_tDeltaTime *ap
)
{
  time_tClock	c;
  static int 	first = 1;
  static time_tClock 	offs;

  c = time_Clock(status, ap);

  if (first) {
    offs = 0xFFFFFFFF - c - 12036;
    first = 0;
  }
  
  return c + offs;
}
#endif


int
main (
  int			argc,
  char			**argv
)
{
  pwr_tStatus           sts;
  time_tClock		wait_clock;
  qcom_sQid	        my_q = qcom_cNQid;
  qcom_sGet	        get;
  int                   tmo = 0;

  init();

  if (!qcom_CreateQ(&sts, &my_q, NULL, "events")) {
    exit(sts);
  }
  if (!qcom_Bind(&sts, &my_q, &qcom_cQini)) {
    exit(-1);
  }

  gdbroot->db->log.b.tmon = 0;

  for (wait_clock = 0;;) {

    get.data = NULL;
    qcom_Get(&sts, &my_q, &get, tmo);
    if (sts != QCOM__TMO && sts != QCOM__QEMPTY) {
      if (get.type.b == qcom_eBtype_event) {
        event(&get);
      }
      qcom_Free(&sts, get.data);
    }

    if (wait_clock != 0)
      waitClock(wait_clock);

    aproc_TimeStamp();

    now_clock = time_Clock(NULL, NULL);
    if (now_clock < last_clock) { 
      errh_Info("The uptime clock has wrapped");
      toggleWrapped();
      executeExpired(&wrap_lh, 1);
    }

    getNewTimers();
    executeExpired(&timer_lh, 0);

    last_clock = now_clock;
    now_clock = time_Clock(NULL, NULL);

    getWaitClock(&wait_clock, last_clock);
    
  }
}

static void
event (
  qcom_sGet	*get
)
{
  qcom_sEvent  *ep = (qcom_sEvent*) get->data;
  ini_mEvent            new_event;

  if (get->type.s != qcom_cIini)
    return;

  new_event.m = ep->mask;
  if (new_event.b.terminate) {
    exit(0);
  }
}

/* .  */

static void
sancAdd (
  sTimer	*tp
)
{
  static time_tClock	cycle;
  static pwr_tBoolean   first = 1;
  gdb_sNode		*np;
  pool_sQlink		*nl;
  
  if (first) {
#ifdef OS_LINUX
    cycle = 1 * sysconf(_SC_CLK_TCK);
#else
    cycle = 1 * CLK_TCK;
#endif
    setInterval(&cycle, gdbroot->db->sanc_add_int);
    first = 0;
  }

  if (gdbroot->db->log.b.tmon)
    errh_Info("sancAdd: %u", tp->clock); 

  for (
    nl = pool_Qsucc(NULL, gdbroot->pool, &gdbroot->db->nod_lh);
    nl != &gdbroot->db->nod_lh;
    nl = pool_Qsucc(NULL, gdbroot->pool, nl)
  ) {
    np = pool_Qitem(nl, gdb_sNode, nod_ll);

    if (!np->flags.b.active
        || np == gdbroot->my_node
	|| np == gdbroot->no_node
    ) continue;

    sancm_Add(NULL, np);
  }

  setTimer(tp, cycle);

  insertTimer(tp);
}

/* .  */

static void
sancExpired (
  sTimer	*tp
)
{
  static time_tClock	cycle;
  static pwr_tBoolean   first = 1;
  gdb_sNode		*np;
  pool_sQlink		*nl;
  
  if (first) {
#ifdef OS_LINUX
    cycle = 60 * sysconf(_SC_CLK_TCK);
#else
    cycle = 60 * CLK_TCK;
#endif
    setInterval(&cycle, gdbroot->db->sanc_exp_int);
    first = 0;
  }

  if (gdbroot->db->log.b.tmon)
    errh_Info("sancExpired: %u", tp->clock);

  for (
    nl = pool_Qsucc(NULL, gdbroot->pool, &gdbroot->db->nod_lh);
    nl != &gdbroot->db->nod_lh;
  ) {
    np = pool_Qitem(nl, gdb_sNode, nod_ll);
    nl = pool_Qsucc(NULL, gdbroot->pool, nl);

    if (!np->flags.b.active
        || np == gdbroot->my_node
	|| np == gdbroot->no_node
    ) continue;
    sancm_Remove(NULL, np);
    sancm_MoveExpired(NULL, np);
  }


  setTimer(tp, cycle);

  insertTimer(tp);
}

/* .  */

static void
sansCheck (
  sTimer	*tp
)
{
  static time_tClock	cycle;
  static pwr_tBoolean   first = 1;
  gdb_sNode		*np;
  pool_sQlink		*nl;
  
  if (first) {
#ifdef OS_LINUX
    cycle = 2 * sysconf(_SC_CLK_TCK);
#else
    cycle = 2 * CLK_TCK;
#endif
    setInterval(&cycle, gdbroot->db->sans_chk_int);
    first = 0;
  }

  if (gdbroot->db->log.b.tmon)
    errh_Info("sansCheck: %u", tp->clock);

  sansm_Check();

  for (
    nl = pool_Qsucc(NULL, gdbroot->pool, &gdbroot->db->nod_lh);
    nl != &gdbroot->db->nod_lh;
  ) {
    np = pool_Qitem(nl, gdb_sNode, nod_ll);
    nl = pool_Qsucc(NULL, gdbroot->pool, nl);

    if (!np->flags.b.active
        || np == gdbroot->my_node
	|| np == gdbroot->no_node
    ) continue;

    sansm_Update(np);
  }


  setTimer(tp, cycle);

  insertTimer(tp);
}

/* .  */

static void
subcCheck (
  sTimer	*tp
)
{
  static time_tClock	cycle;
  static pwr_tBoolean   first = 1;

  if (first) {
    setInterval(&cycle, gdbroot->db->subc_chk_int);
    first = 0;
  }
  
  if (gdbroot->db->log.b.tmon)
    errh_Info("subcCheck: %u", tp->clock);

  subcm_CheckTimeout();

  setTimer(tp, cycle);

  insertTimer(tp);
}

/* Send a subscription buffer and reschedule it.  */

static void
subbCheck (
  sTimer	*tp
)
{
  sub_sBuffer	*bp = (sub_sBuffer *)tp->data;

  if (gdbroot->db->log.b.tmon)
    errh_Info("subbCheck: %u", tp->clock);

#if 0
  if (EXCL_ON) {
    /* Exclusive mode is on, requeue the
       buffer for later transmission */

    addTime(nowTime(&tp->time), msToTime(NULL, bp->dt)); 
    insertTimer(tp);
  }
#endif

  if (subsm_SendBuffer(bp)) {
    setTimer(tp, msToClock(NULL, bp->dt));
    insertTimer(tp);
  } else {
    freeTimer(tp);
  }
}

/* .  */

static void
cacheTrim (
  sTimer	*tp
)
{
  static time_tClock	cycle;
  static pwr_tBoolean   first = 1;

  if (first) {
#ifdef OS_LINUX
    cycle = 1 * sysconf(_SC_CLK_TCK);
#else
    cycle = 1 * CLK_TCK;
#endif
    setInterval(&cycle, gdbroot->db->cache_trim_int);
    first = 0;
  }

  if (gdbroot->db->log.b.tmon)
    errh_Info("cacheTrim: %u", tp->clock);

  cvolcm_TrimOld();
  cvol_Qtrim(&gdbroot->db->cacheNew);

  setTimer(tp, cycle);

  insertTimer(tp);
}

/* .  */

static void
insertTimer (
  sTimer		*tp
)
{
  LstLink(sTimer)	*tl;
  sTimer		*tip;
  
  if (LstEmp(&timer_lh)) {
    (void)LstIns(LstEnd(&timer_lh), tp, ll);
    return;
  }


  for (tl = LstLas(&timer_lh); tl != LstEnd(&timer_lh); tl = LstPre(tl)) {
    tip = LstObj(tl);
    
    if (tp->wrapped) {
      if (!tip->wrapped || tip->clock < tp->clock)
	break;
    } else if (!tip->wrapped && tip->clock < tp->clock)
      break;
  }  	

  tl = LstNex(tl);
  (void)LstIns(tl, tp, ll);
}

/* .  */

static sTimer *
newTimer (
  time_tClock		*clock,
  void			*data,
  void			(*exec)()
)
{
  sTimer		*tp = allocTimer();

  tp->data  = data;
  tp->exec  = exec;

  if (clock == NULL)
    tp->clock = now_clock;
  else {

    tp->clock = *clock;
    tp->wrapped = (*clock < now_clock);
  }

  return tp;
}

/* .  */

static void
freeTimer (
  sTimer		*tp
)
{

  memset(tp, 0, sizeof(*tp));
  LstIns(&LstEnd(free_lh), tp, ll);  
}

/* .  */

static sTimer *
allocTimer (
)
{
  const int		cAllocCount = 100;
  sTimer		*ftp;
  LstLink(sTimer)	*ftl;
  int			i;

  if (LstEmp(&free_lh)) {
    ftp = (sTimer *) calloc(cAllocCount, sizeof(sTimer));
    for (i=0; i < cAllocCount; i++, ftp++) {
      LstIns(&LstEnd(free_lh), ftp, ll);
    }
  }

  ftl = LstFir(&free_lh);
  LstRem(ftl);
  return LstObj(ftl);
}

static void
setTimer (
  sTimer		*tp,
  time_tClock		offs
)
{
  
  tp->clock = time_Clock(NULL, NULL) + offs;
  tp->wrapped = (tp->clock < now_clock);
   
}

/* Convert mille seconds to clock format.  */

static time_tClock
msToClock (
  time_tClock		*ic,
  int			ms
)
{
  time_tClock		c;

#ifdef OS_LINUX
  c  = ms * sysconf(_SC_CLK_TCK) / 1000;
#else
  c  = ms * CLK_TCK / 1000;
#endif


  if (ic != NULL) *ic = c;

  return c;
}

/* Wait for a while.  */

static void
waitClock (
  time_tClock		diff
)
{
#if defined OS_VMS || defined OS_ELN
    pwr_tStatus		sts;
    int			sec;
    int			nsec;
    int			vmstime[2];
    int			multiplier = -10000000;	      /* Used to convert 1 s to 100 ns, delta time.  */

    sec = diff / CLK_TCK;
    nsec = - (diff % CLK_TCK * 10000000 / CLK_TCK);   /* Convert to 100 ns.  */
    sts = lib$emul(&sec, &multiplier, &nsec, vmstime);

# if defined OS_VMS
    sts = sys$clref(timer_flag);
    sts = sys$setimr(timer_flag, vmstime, 0, 0, 0);
    sts = sys$waitfr(timer_flag);
# elif defined OS_ELN
    ker$wait_any(&sts, NULL, vmstime);
# endif

#elif defined OS_LYNX || defined OS_LINUX
    pwr_tTime rmt;
    pwr_tTime wait;

//    printf("waitClock: %d\n", diff);
    time_ClockToD(NULL, (pwr_tDeltaTime *)&wait, diff);
    nanosleep(&wait, &rmt);
#endif
}


/* .  */

static void
setInterval (		
  time_tClock		*c,

  pwr_tUInt32		i
)
{
  if (i == 0)
    return;

#ifdef OS_LINUX
  *c = i * sysconf(_SC_CLK_TCK) / 1000;
#else
  *c = i * CLK_TCK / 1000;
#endif

}



static void
executeExpired (
  LstHead(sTimer)	*lh,
  pwr_tBoolean		force
)
{
  LstLink(sTimer)	*tl;
  sTimer		*tp;

  gdb_AssumeUnlocked;
    
  gdb_ScopeLock {

    for (tl = LstFir(lh); tl != LstEnd(lh); tl = LstFir(lh)) {
      tp = LstObj(tl);

      if (force || (!tp->wrapped && tp->clock <= now_clock)) {
        LstRem(tl);
        LstNul(tl);
        tp->exec(tp);			       
      } else
        break;
    }

  } gdb_ScopeUnlock;
}


static void
getNewTimers (
)
{
  pool_sQlink		*tql;
  sub_sBuffer		*bp;
  gdb_sTmonQlink	*tqp;
  time_tClock		clock;

  gdb_AssumeUnlocked;

  gdb_ScopeLock {

  /* Get all new timers and insert them into local timer queue.  */

    for (
      tql = pool_Qsucc(NULL, gdbroot->pool, &gdbroot->db->tmonq_lh);
      tql != &gdbroot->db->tmonq_lh; 
      tql = pool_Qsucc(NULL, gdbroot->pool, &gdbroot->db->tmonq_lh)
    ) {
      tqp = pool_Qitem(tql, gdb_sTmonQlink, ll);

      switch (tqp->type) {
      case gdb_eTmon_subbCheck:
	bp = pool_Qitem(tql, sub_sBuffer, tmonq.ll);
	if (gdbroot->db->log.b.tmon)
	  errh_Info("New buffer: %x", bp->nid); 
	clock = msToClock(&clock, tqp->dt) + now_clock; 
	insertTimer(newTimer(&clock, bp, subbCheck)); 
	break;
      default:
	break;
      }

      pool_Qremove(NULL, gdbroot->pool, &tqp->ll);
    }


  } gdb_ScopeUnlock;

}


static void
getWaitClock(
  time_tClock		*wait_clock,
  time_tClock		last_clock
)
{
  int			diff;
  LstLink(sTimer)	*tl;
  sTimer		*tp;

  tl = LstFir(&timer_lh);
  tp = LstObj(tl);

  if (now_clock < last_clock) {
    if (tp->wrapped) {
      diff = tp->clock - now_clock;
      if (diff < 0)
	*wait_clock = 0;
      else
	*wait_clock = diff;
    } else
      *wait_clock = 0;

  } else {
    diff = tp->clock - now_clock;
    if (diff < 0)
      *wait_clock = 0;
    else
       *wait_clock = diff;

  }

}

static void
init(
)
{
  pwr_tStatus		sts;

  /* Initialize.  */
  errh_Init("pwr_tmon", errh_eAnix_tmon);
  errh_SetStatus( PWR__SRVSTARTUP);

  qcom_Init(&sts, 0, "pwr_tmon");
  if (EVEN (sts)) {
    errh_Error("qcom_Init, %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(sts);
  }

  sts = gdh_Init("pwr_tmon");
  if (EVEN (sts)) {
    errh_Error("gdh_Init, %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(sts);
  }

  gdbroot->db->tmon = gdbroot->my_qid;
  gdbroot->is_tmon = 1;

#ifdef OS_VMS
  sts = lib$get_ef(&timer_flag);
  if (EVEN (sts)) {
    errh_Error("lib$get_ef, %m", sts);
    exit(sts);
  }
#endif

  LstIni(&timer_lh);
  LstIni(&wrap_lh);
  LstIni(&free_lh);

  last_clock = now_clock = time_Clock(NULL, NULL);

  gdb_ScopeLock {
    cacheTrim(newTimer(NULL, NULL, cacheTrim)); 
    sancAdd(newTimer(NULL, NULL, sancAdd)); 
    sancExpired(newTimer(NULL, NULL, sancExpired)); 
    sansCheck(newTimer(NULL, NULL, sansCheck)); 
    subcCheck(newTimer(NULL, NULL, subcCheck)); 
  } gdb_ScopeUnlock;

  errh_SetStatus( PWR__SRUN);
}

static void
toggleWrapped (
)
{
  LstLink(sTimer)	*tl;
  LstLink(sTimer)	*ntl;
  sTimer		*tp;
  LstHead(sTimer)	*tlh = &timer_lh;

  for (tl = LstFir(tlh); tl != LstEnd(tlh); tl = ntl) {
    tp = LstObj(tl);
    ntl = LstNex(tl);

    if (!tp->wrapped) {
       LstRem(tl);
      (void)LstIns(LstEnd(&wrap_lh), tp, ll);       
    } else
      tp->wrapped = 0;
  }
}

