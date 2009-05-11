/* 
 * Proview   $Id: rt_errh.c,v 1.13 2008-10-31 12:51:30 claes Exp $
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

/* rt_errh.c -- Logg errors and more.
  The routines in this module are the error logging package.  */


#ifdef OS_ELN

# include ctype
# include $kerneldef
# include $vaxelnc
# include stdio
# include stdarg
# include descrip
#else
# include <stdlib.h>
# include <stdarg.h>
# include <stdio.h>    /* for EOF */
# include <string.h>
# include <ctype.h>
#endif

#if defined(OS_LYNX) || defined(OS_LINUX)
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <limits.h>
# include <pthread.h>  /* for tid_t */
# include <unistd.h>
# include <errno.h>
# include "rt_errl.h"
# if defined(OS_LINUX)
#   include <time.h>
#   include <mqueue.h>
# elif defined(OS_LYNX)
#   include <mqueue.h>
# endif

#elif defined(OS_VMS)
# include <descrip.h>
# include <starlet.h>
# include <jpidef.h>
# include "co_time.h"
#elif defined(OS_ELN)
# include "co_time.h"
#endif

#include "pwr_msg.h"
#include "co_msg.h"
#include "rt_errh.h"
#include "pwr_class.h"
#include "rt_gdh.h"
#include "rt_errh_msg.h"
#include "rt_pwr_msg.h"

#define UNKNOWN_PROGRAM_NAME "Unknown name   "

typedef void *aa_list[];
#define aa_arg(ap, vap, type) (ap ? ((type) *ap++) : va_arg(vap, type))

typedef enum {
  eArg_sts,
  eArg_int,
  eArg_string
} eArg;

typedef struct {
  eArg class;
  union {
    pwr_tStatus sts;
    int	intval;
    char *string;
  } u;
} sArg;

#if defined OS_VMS

# define LOG_MAX_MSG_SIZE 256
  typedef int sPid;

  typedef struct {
    short buffer_length;	
    short item_code;	
    void *buffer_addr;
    unsigned short *return_length_addr;
  } item_descr;

  static sPid pid = 0;

#elif defined OS_ELN

# define LOG_MAX_MSG_SIZE 255

  typedef struct {
    int job;
    int proc;
  } sPid;

  static sPid pid = {0, 0};

#elif defined OS_LYNX || defined OS_LINUX

  typedef pid_t sPid;

  static mqd_t mqid = (mqd_t)-1;
  static unsigned int prio = 0;
  static int mq_send_errno = 0;
#endif

static const char *indentStr = "  ";
static char programName[16];
static int interactive = 0;
static int initDone = 0;
static errh_eAnix errh_anix = errh_eNAnix;

static char *get_header (char, char*);
static char *get_message (const int, unsigned int, char*, int);
static char *get_name (char*, int);
static char get_severity (pwr_tStatus);
static void openLog ();
static void set_name (const char*);
static void errh_send (char*, char, pwr_tStatus, errh_eMsgType);
static void log_message (errh_sLog*, char, const char*, va_list);
static int msg_vsprintf (char *, const char *, aa_list, va_list);

#if defined(OS_LYNX) || defined(OS_LINUX) || defined(OS_ELN)
 static size_t errh_strnlen (const char*, size_t);
#else
#define errh_strnlen strnlen
#endif

static unsigned int do_div (int*, unsigned int);
static int skip_atoi (const char**);
static char * number (char*, int, int, int, int, int);


void
errh_Interactive ()
{

  interactive = 1;
}

/* Set up a queue to write to, set module name.  */

pwr_tStatus
errh_Init (
  const char *name,
  errh_eAnix anix
)
{
  get_name(programName, sizeof(programName) - 1);
  if (name != NULL && name[0] != '\0')
    set_name(name);
  errh_anix = anix;

  if ( !initDone) {
    initDone = 1;
    openLog();
  }

  return 1;
}

void
errh_SetStatus( pwr_tStatus sts)
{
  /* Send close message */
  errh_send(0, 0, sts, errh_eMsgType_Status);
}

errh_eAnix
errh_Anix()
{
  return errh_anix;
}


/* Check if a given messagenumber exists,
   return string representation if valid.  */

char *
errh_GetMsg (
  const int	sts,
  char		*buf,
  int		bufSize
)
{

  return get_message(sts, 0xf, buf, bufSize);
}

/* Checks if a given messagenumber exists,
  return string representation if valid.  */

char *
errh_GetText (
  const int sts,
  char *buf,
  int bufSize
)
{

  return get_message(sts, 1, buf, bufSize);
}

/* Log a message.  */
char *
errh_Log (
  char *buff,
  char severity,
  const char *msg,
  ...
)
{
  char *s;
  va_list ap;

  s = get_header(severity, buff);
  va_start(ap, msg);
  msg_vsprintf(s, msg, NULL, ap);
  va_end(ap); 

  if (interactive)
    printf("%s\n", buff);
  else
    errh_send(buff, severity, 0, errh_eMsgType_Log);

  return buff;
}

/* Log a success message.  */
void
errh_Success (
  const char *msg,
  ...
)
{
  va_list args;

  va_start(args, msg);
  log_message(NULL, 'S', msg, args);
  va_end(args);
}

/* Log an info message  */

void
errh_Info (const char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  log_message(NULL, 'I', msg, args);
  va_end(args);
}

/* Log a warning message  */

void
errh_Warning(const char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  log_message(NULL, 'W', msg, args);
  va_end(args);
}

/* Log an error message  */

void
errh_Error(const char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  log_message(NULL, 'E', msg, args);
  va_end(args);
}

/* Log a fatal message  */

void
errh_Fatal(const char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  log_message(NULL, 'F', msg, args);
  va_end(args);
}

/* Log a fatal message.  */
void
errh_LogFatal (
  errh_sLog *lp,
  const char *msg,
  ...
)
{
  va_list args;

  va_start(args, msg);
  log_message(lp, 'F', msg, args);
  va_end(args);
}

/* Log a warning message.  */
void
errh_LogWarning (
  errh_sLog *lp,
  const char *msg,
  ...
)
{
  va_list args;

  va_start(args, msg);
  log_message(lp, 'W', msg, args);
  va_end(args);
}

/* Log a error message.  */
void
errh_LogError (
  errh_sLog *lp,
  const char *msg,
  ...
)
{
  va_list args;

  va_start(args, msg);
  log_message(lp, 'E', msg, args);
  va_end(args);
}

/* Log a info message.  */
void
errh_LogInfo (
  errh_sLog *lp,
  const char *msg,
  ...
)
{
  va_list args;

  va_start(args, msg);
  log_message(lp, 'I', msg, args);
  va_end(args);
}

/* Log a success message.  */
void
errh_LogSuccess (
  errh_sLog *lp,
  const char *msg,
  ...
)
{
  va_list args;

  va_start(args, msg);
  log_message(lp, 'S', msg, args);
  va_end(args);
}

/* Insert a status message in a message.  */
 
void *
errh_ErrArgMsg (
  pwr_tStatus  sts
)
{
  sArg *eap;

  eap = (sArg *)malloc(sizeof(*eap));
  eap->class = eArg_sts;
  eap->u.sts = sts;

  return eap;
}

/* Insert a string argument in a message.  */

void *
errh_ErrArgAF (
  char *str
)
{
  sArg  *eap;

  eap = (sArg *)malloc(sizeof(*eap));

  eap->class = eArg_string;
  eap->u.string = str;

  return eap;
}

/* Insert a integer value in a message.  */

void *
errh_ErrArgL (
  int val
)
{
  sArg *eap;

  eap = (sArg *)malloc(sizeof(*eap));

  eap->class = eArg_int;
  eap->u.intval = val;

  return eap;
}

void
errh_CErrLog (
  pwr_tStatus sts,
  ...
)
{
  va_list	ap;
  void		*args[25];
  char		msg[200];
  sArg		*eap;
  int		argno = 0;
  char		*s;
  char		string[1000];

  get_message(sts, 0xb, msg, sizeof(msg));

  va_start(ap, sts);
  while ((eap = va_arg(ap, sArg *)) != NULL) {
    switch (eap->class) {
    case eArg_sts:
      args[argno++] = (char *)((long int)eap->u.sts);
      strcat(msg, "\n%m");
      break;
    case eArg_int:
      args[argno++] = (char *)((long int)eap->u.intval);
      break;
    case eArg_string:
      args[argno++] = eap->u.string;
      break;
    }
    free(eap);
  }
  va_end(ap); 

  args[argno] = NULL;

  s = get_header(get_severity(sts), string);
  msg_vsprintf(s, msg, args, NULL);
  errh_send(string, get_severity(sts), sts, errh_eMsgType_Log);
}

/* Format a string.  */

char *
errh_Message (
  char *string,
  char severity,
  char *msg,
  ...
)
{
  char *s;
  va_list ap;

  s = get_header(severity, string);
  va_start(ap, msg);
  msg_vsprintf(s, msg, NULL, ap);
  va_end(ap); 

  return string;
}

/* Get a message string, VMS style.

   %FACILITYNAME-S-MESSAGENAME, message string

   flags:
    0x1		message string
    0x2		MESSAGENAME
    0x4		S (severity)
    0x8		FACILITYNAME
    0xf		% + all

    any other	gives a combination of above excluding %  */

static char *
get_message (
  const pwr_tStatus	sts,
  unsigned int		flags,
  char			*buf,
  int			bufSize
)
{
  return msg_GetMessage( sts, flags, buf, bufSize);
}


static void
set_name (const char *name)
{

  strncpy(programName, name, sizeof(programName) - 1);
  programName[sizeof(programName)-1] = '\0';
}

static void
openLog ()
{
#if defined OS_LYNX || defined OS_LINUX
  if (mqid == (mqd_t)-1) {
    char name[64];
    char *busid = getenv(pwr_dEnvBusId);

    sprintf(name, "%s_%s", LOG_QUEUE_NAME, busid ? busid : "");  
    mqid = mq_open(name, O_WRONLY | O_NONBLOCK, 0, 0);
    if (mqid == (mqd_t)-1) {
      char string[256];
      char *s;

      s = errh_Message(string, 'E', "Open messageQ, mq_open(%s)\n%s", name, strerror(errno));
      printf("%s\n", s);
      return;
    }
  }
#endif
}

#if defined OS_VMS
static char *
get_name (char *name, int size)
{
  item_descr itmlst[2];
  pwr_tStatus sts = 1;
  unsigned short int len = 0;
  int iosb[2];

  itmlst[0].item_code = JPI$_PRCNAM;
  itmlst[0].buffer_length = size;
  itmlst[0].buffer_addr = name;
  itmlst[0].return_length_addr = &len;

  itmlst[1].item_code = 0;
  itmlst[1].buffer_length = 0;

  sts = sys$getjpiw(0, 0, 0, itmlst, iosb, 0, 0);
  if (ODD(sts))
    name[MIN(size, len)] = '\0';
   
  return name;
}
static sPid *
get_pid (sPid *pid)
{
  item_descr itmlst[2];
  pwr_tStatus sts = 1;
  int iosb[2];

  itmlst[0].item_code = JPI$_PID;
  itmlst[0].buffer_length = sizeof(*pid);
  itmlst[0].buffer_addr = pid;
  itmlst[0].return_length_addr = 0;

  itmlst[1].item_code = 0;
  itmlst[1].buffer_length = 0;

  sts = sys$getjpiw(0, 0, 0, itmlst, iosb, 0, 0);
  if (EVEN(sts))
    return NULL;
   
  return pid;
}
#elif defined OS_ELN
static char *
get_name (char *name, int size)
{
  pwr_tStatus	sts; 
  struct jcb	*jcb;
  struct pcb	*pcb;

  ker$get_jcb(&sts, &jcb);
  pcb = (struct pcb *)jcb->jcb$a_current_pcb;

  strncpy(name, jcb->jcb$a_program->prg$t_name.string_text, 
    MIN(jcb->jcb$a_program->prg$t_name.string_length, size));

  name[size] = '\0';

  return sts;
}
static sPid *
get_pid (sPid *pid)
{
  pwr_tStatus	sts; 
  struct jcb	*jcb;
  struct pcb	*pcb;

  ker$get_jcb(&sts, &jcb);
  pcb = (struct pcb *)jcb->jcb$a_current_pcb;

  pid->job = jcb->jcb$w_generation;
  pid->proc = pcb->pcb$w_generation;

  return pid;
}
#elif defined OS_LYNX || defined OS_LINUX
static char *
get_name (char *name, int size)
{
  int len = strlen(UNKNOWN_PROGRAM_NAME);

  strncat(name, UNKNOWN_PROGRAM_NAME, MIN(size, len));

  name[MIN(size, len)] = '\0';

  return name;
}
static sPid *
get_pid (sPid *pid)
{

  *pid = getpid();

  return pid;
}
#endif


static char *
get_header (char severity, char *s)
{
  sPid pid;
  struct timespec time;
  struct tm tp, *t;

  if (!initDone)
    errh_Init(NULL, 0);

  if (interactive) {
    s += sprintf(s, "%c ", severity);
    return s;
  }

  clock_gettime(CLOCK_REALTIME, &time);

  get_pid(&pid);

  s += sprintf(s, "%c %-*.*s", severity, (int)sizeof(programName), (int)sizeof(programName), programName);

# if defined OS_LYNX
  localtime_r(&tp, &time.tv_sec);
  t = &tp;
  s += sprintf(s, " % 4d,% 4d ", (int)PIDGET(pid), (int)pthread_self());
# elif defined OS_LINUX
  localtime_r(&time.tv_sec, &tp);
  t = &tp;
  s += sprintf(s, " %8d ", pid);
# elif defined OS_VMS 
  t = localtime(&time.tv_sec);
  s += sprintf(s, " %08X ", pid);
# elif defined OS_ELN 
  t = localtime(&time.tv_sec);
  s += sprintf(s, " % 4d,% 4d ", pid.job, pid.proc);
# endif 
  
  s += sprintf(s, "%02d-%02d-%02d %02d:%02d:%02d.%02d ",
    t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour,
    t->tm_min, t->tm_sec, (int)(time.tv_nsec / 10000000));

  return s;
}

/* Format a string and write it to log devices.  */

static void
log_message (errh_sLog *lp, char severity, const char *msg, va_list ap)
{
  char *s;
  char string[1000];

  s = get_header(severity, string);
  msg_vsprintf(s, msg, NULL, ap);
  if (interactive)
    printf("%s\n", string);
  else
    errh_send(string, severity, 0, errh_eMsgType_Log);

  if (lp != NULL && lp->send) {
    lp->put.data = string;
    lp->put.size = strlen(string);
    qcom_Put(NULL, &lp->logQ, &lp->put);
  }
}

/* Print a message on error log.  */

static char
get_severity (
  pwr_tStatus sts
)
{

  switch (sts & 7) {
  case 0:  return 'W';
  case 1:  return 'S';
  case 2:  return 'E';
  case 3:  return 'I';
  case 4:  return 'F';
  default: return '?';
  }
}

/****** internals ************************************************************/

/* PWR-version of vsprintf. replaces %m represents
   a message number and is replaced by the string.  */

#define ZEROPAD 1   /* pad with zero */
#define SIGN  2     /* unsigned/signed long */
#define PLUS  4     /* show plus */
#define SPACE 8     /* space if plus */
#define LEFT  16    /* left justified */
#define SPECIAL 32  /* 0x */
#define LARGE 64    /* use 'ABCDEF' instead of 'abcdef' */

/* We use this so that we can do without the ctype library.  */

#define is_digit(c) ((c) >= '0' && (c) <= '9')



static int
msg_vsprintf (
  char		*buf,
  const char	*fmt,
  aa_list	ap,
  va_list	vap
)
{
  int len, sts = 0;
  unsigned long num;
  int i, base;
  char *str;
  char msg[256];
  char *s;
  const char *cs;
  int flags;        /* flags to number() */

  long int field_width;  /* width of output field */
  long int precision;    /* min. # of digits for integers; max
                       number of chars for from string */
  int qualifier;    /* 'h', 'l', or 'L' for integer fields */

  for (str=buf ; *fmt ; ++fmt) {
    if (*fmt != '%') {
      *str++ = *fmt;
      if (*fmt == '\n') {
        /* I hope we are not running a Fu.. PC */
         /* *str++ ='\r'; */
        cs = indentStr;
        while (*cs != '\0')
	  *str++ = *cs++;
      }
      continue;
    }

    /* process flags */
    flags = 0;

repeat:

    ++fmt;    /* this also skips first '%' */
    switch (*fmt) {
    case '-': flags |= LEFT; goto repeat;
    case '+': flags |= PLUS; goto repeat;
    case ' ': flags |= SPACE; goto repeat;
    case '#': flags |= SPECIAL; goto repeat;
    case '0': flags |= ZEROPAD; goto repeat;
    }

    /* get field width */
    field_width = -1;
    if (is_digit(*fmt)) {
      field_width = skip_atoi(&fmt);
    } else if (*fmt == '*') {
      ++fmt;
      /* it's the next argument */
      field_width = aa_arg(ap, vap, long int);
      if (field_width < 0) {
        field_width = -field_width;
        flags |= LEFT;
      }
    }

    /* get the precision */
    precision = -1;
    if (*fmt == '.') {
      ++fmt;
      if (is_digit(*fmt)) {
        precision = skip_atoi(&fmt);
      } else if (*fmt == '*') {
        ++fmt;
        /* it's the next argument */
        precision = aa_arg(ap, vap, long int);
      }
      if (precision < 0)
        precision = 0;
    }

    /* get the conversion qualifier */
    qualifier = -1;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
      qualifier = *fmt;
      ++fmt;
    }

    /* default base */
    base = 10;

    switch (*fmt) {
    case 'c':
      if (!(flags & LEFT))
        while (--field_width > 0)
          *str++ = ' ';
      *str++ = (unsigned char) aa_arg(ap, vap, long int);
      while (--field_width > 0)
        *str++ = ' ';
      continue;

    case 's':
      s = aa_arg(ap, vap, char *);
      if (!s)
        s = "<NULL>";

      len = errh_strnlen(s, precision);

      if (!(flags & LEFT))
        while (len < field_width--)
          *str++ = ' ';
      for (i = 0; i < len; ++i)
        *str++ = *s++;
      while (len < field_width--)
        *str++ = ' ';
      continue;

    case 'm' :
      sts = aa_arg(ap, vap, long int);
      errh_GetMsg(sts, msg, sizeof(msg));
      s = msg;
      while (*s != '\0')
	*str++ = *s++;
      continue;

    case 'p':
      if (field_width == -1) {
        field_width = 2 * sizeof(void *);
        flags |= ZEROPAD;
      }
      str = number(str, (unsigned long) aa_arg(ap, vap, void *), 16,
        field_width, precision, flags);
      continue;


    case 'n':
      if (qualifier == 'l') {
        long * ip = aa_arg(ap, vap, long *);
        *ip = (str - buf);
      } else {
        int * ip = aa_arg(ap, vap, int *);
        *ip = (str - buf);
      }
      continue;

    /* integer number formats - set up the flags and "break" */
    case 'o':
      base = 8;
      break;

    case 'X':
      flags |= LARGE;
    case 'x':
      base = 16;
      break;

    case 'd':
    case 'i':
      flags |= SIGN;
    case 'u':
      break;

    default:
      if (*fmt != '%')
        *str++ = '%';
      if (*fmt)
        *str++ = *fmt;
      else
        --fmt;
      continue;
    }
    if (qualifier == 'l')
      num = aa_arg(ap, vap, unsigned long);
    else if (qualifier == 'h')
      if (flags & SIGN)
        num = aa_arg(ap, vap, long);
      else
        num = aa_arg(ap, vap, unsigned long);
    else if (flags & SIGN)
      num = aa_arg(ap, vap, long int);
    else
      num = aa_arg(ap, vap, unsigned long);
    str = number(str, num, base, field_width, precision, flags);
  }
  *str = '\0';
  return str - buf;
}



#if defined(OS_LYNX)  || defined(OS_LINUX) || defined(OS_ELN)
/* Different strlen function, returns len OR count,
   whatever comes true first.  */

static size_t
errh_strnlen (
  const char  *s,
  size_t      count
)
{
  const char *sc;

  for (sc = s; *sc != '\0' && count--; ++sc)
    /* nothing */;
  return sc - s;
}
#endif


static int
skip_atoi (
  const char **s
)
{
  int i=0;

  while (is_digit(**s))
    i = i*10 + *((*s)++) - '0';
  return i;
}


static unsigned int
do_div (
  int		*n,
  unsigned int	base
)
{
  unsigned int res;

  res = ((unsigned int) *n) % base; 
  *n = ((unsigned int) *n) / base;
  return res; 
}

/* Handle numerics.  */

static char *
number (
  char	*str,
  int	num,
  int	base,
  int	size,
  int	precision,
  int	type
)
{
  char	c;
  char	sign;
  char	tmp[66];
  const	char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
  int	i;

  if (type & LARGE)
    digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  if (type & LEFT)
    type &= ~ZEROPAD;
  if (base < 2 || base > 36)
    return 0;
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (type & SIGN) {
    if (num < 0) {
      sign = '-';
      num = -num;
      size--;
    } else if (type & PLUS) {
      sign = '+';
      size--;
    } else if (type & SPACE) {
      sign = ' ';
      size--;
    }
  }
  if (type & SPECIAL) {
    if (base == 16)
      size -= 2;
    else if (base == 8)
      size--;
  }
  i = 0;
  if (num == 0)
    tmp[i++]='0';
  else while (num != 0)
    tmp[i++] = digits[do_div(&num,base)];
  if (i > precision)
    precision = i;
  size -= precision;
  if (!(type&(ZEROPAD+LEFT)))
    while(size-->0)
      *str++ = ' ';
  if (sign)
    *str++ = sign;
  if (type & SPECIAL) {
    if (base==8)
      *str++ = '0';
    else if (base==16) {
      *str++ = '0';
      *str++ = digits[33];
    }
  }
  if (!(type & LEFT))
    while (size-- > 0)
      *str++ = c;
  while (i < precision--)
    *str++ = '0';
  while (i-- > 0)
    *str++ = tmp[i];
  while (size-- > 0)
    *str++ = ' ';
  return str;
}

static void
errh_send (char *s, char severity, pwr_tStatus sts, errh_eMsgType message_type)
{

#if defined OS_LYNX || defined OS_LINUX

  int len;
  if (mqid != (mqd_t)-1) {
    errh_sMsg msg;

    switch ( message_type) {
    case errh_eMsgType_Log:
      strncpy( msg.str, s, LOG_MAX_MSG_SIZE);
      msg.str[LOG_MAX_MSG_SIZE-1] = 0;
      msg.message_type = message_type;
      msg.severity = severity;
      msg.sts = sts;
      msg.anix = errh_anix;
      len = sizeof(msg) - sizeof(msg.message_type) - sizeof(msg.str) + strlen(msg.str) + 1;
      break;
    case errh_eMsgType_Status:
      msg.message_type = message_type;
      msg.sts = sts;
      msg.anix = errh_anix;
      len = sizeof(msg) - sizeof(msg.message_type) - sizeof(msg.str);
      break;
    }
    if ( prio == 0)
      prio = sysconf(_SC_MQ_PRIO_MAX) - 1;
    if (mq_send(mqid, (char *)&msg, MIN(len, LOG_MAX_MSG_SIZE - 1), prio) == -1) {
      if ( mq_send_errno != errno) {
	mq_send_errno = errno;
	perror("mq_send");
      }
    }
  } else if (s) {
    puts(s);
    return;
  }

#elif defined OS_ELN

  pwr_tStatus		sts; 
  char                 *ms;
  MESSAGE		mid;
  static PORT	  	port;
  static pwr_tBoolean   portTrans = FALSE;
  static int		errorPrinted = 0;

  if ( message_type != errh_eMsgType_Log)
    return;

  if (!portTrans) {
    $DESCRIPTOR(dsc, "ERR_LOG_PORT");
    ker$translate_name(&sts, &port, &dsc, NAME$LOCAL);
    if (ODD(sts)) {
      portTrans = TRUE;
    } else if (!errorPrinted) {
      printf("errh: ker$translate_name failed, status = %d\n", sts);
      errorPrinted = 1;
    } 
  }

  if (portTrans) {
    ker$create_message(&sts, &mid, &ms, len);
    if (EVEN(sts)) {
      printf("errh: ker$create_message failed, status = %d\n", sts);
      return;
    }

    strncpy(ms, s, len);
    ker$send(&sts, mid, -1, &port, NULL, FALSE);
    if (EVEN(sts)) {
      printf("errh: ker$send failed, status = %d\n", sts);
      ker$delete(NULL, mid);
    }
  } else {
    printf("%s\n", s);
  }

#else

  if ( message_type != errh_eMsgType_Log)
    return;

  printf("%s\n", s);

#endif
}

errh_eSeverity errh_Severity( pwr_tStatus sts)
{
  if ( sts == 0)
    return errh_eSeverity_Null;

  switch ( sts & 7) {
  case 1: return errh_eSeverity_Success; 
  case 3: return errh_eSeverity_Info; 
  case 0: return errh_eSeverity_Warning; 
  case 2: return errh_eSeverity_Error; 
  case 4: return errh_eSeverity_Fatal; 
  default: return errh_eSeverity_Null;
  }
}


#if 0
int main(int argc, char **argv)
{
  #include <stdlib.h>
  int sts;
  char buf[256];
  int bufSize = 255;

  sts = atoi(argv[1]);

  errh_Error("sts: %d", sts);
  errh_Init("Kallekanna");
  errh_Info("sts: %d", sts);
  errh_Warning("Hj�lp. �len �r slut, reason: \n%m\n!! %d\noskar\n%m\n%m", sts, 666, 12, 38);
  errh_CErrLog(sts, errh_ErrArgL(55), errh_ErrArgL(56), errh_ErrArgL(57),  errh_ErrArgL(58), errh_ErrArgMsg(24), NULL);
  errh_Info("Int: %d, float: %f, double: %e, sts: %m", 23, 55.55, 567.34e-23, 24);

  get_message(sts, 0x1, buf, bufSize); printf("0x1 |%s|\n", buf);
  get_message(sts, 0x2, buf, bufSize); printf("0x2 |%s|\n", buf);
  get_message(sts, 0x3, buf, bufSize); printf("0x3 |%s|\n", buf);
  get_message(sts, 0x4, buf, bufSize); printf("0x4 |%s|\n", buf);
  get_message(sts, 0x5, buf, bufSize); printf("0x5 |%s|\n", buf);
  get_message(sts, 0x6, buf, bufSize); printf("0x6 |%s|\n", buf);
  get_message(sts, 0x7, buf, bufSize); printf("0x7 |%s|\n", buf);
  get_message(sts, 0x8, buf, bufSize); printf("0x8 |%s|\n", buf);
  get_message(sts, 0x9, buf, bufSize); printf("0x9 |%s|\n", buf);
  get_message(sts, 0xa, buf, bufSize); printf("0xa |%s|\n", buf);
  get_message(sts, 0xb, buf, bufSize); printf("0xb |%s|\n", buf);
  get_message(sts, 0xc, buf, bufSize); printf("0xc |%s|\n", buf);
  get_message(sts, 0xd, buf, bufSize); printf("0xd |%s|\n", buf);
  get_message(sts, 0xe, buf, bufSize); printf("0xe |%s|\n", buf);
  get_message(sts, 0xf, buf, bufSize); printf("0xf |%s|\n", buf);

}
#endif
