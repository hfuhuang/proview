/* co_errno.c -- Translate UNIX errno to Proview status.

   PROVIEW/R
   Copyright (C) 1999 by Mandator AB.

   NOTE! The actual values of errno codes in <errno.h> must match
         the vector index used in 'errno_status'.  */ 
 
#ifndef OS_LINUX
# error This file is only for Linux
#endif

#include <errno.h>
#include "pwr.h"
#include "co_errno.h"
#include "co_errno_msg.h"

static struct {
  int err_no;
  pwr_tStatus sts;
} errno_status[] = {
  {0, ERRNO__SUCCESS},
  {EPERM, ERRNO__PERM},
  {ENOENT, ERRNO__NOENT},
  {ESRCH, ERRNO__SRCH},
  {EINTR, ERRNO__INTR},
  {EIO, ERRNO__IO},
  {ENXIO, ERRNO__NXIO},
  {E2BIG, ERRNO__2BIG},
  {ENOEXEC, ERRNO__NOEXEC},
  {EBADF, ERRNO__BADF},
  {ECHILD, ERRNO__CHILD},
  {EAGAIN, ERRNO__AGAIN},
  {ENOMEM, ERRNO__NOMEM},
  {EACCES, ERRNO__ACCES},
  {EFAULT, ERRNO__FAULT},
  {ENOTBLK, ERRNO__NOTBLK},
  {EBUSY, ERRNO__BUSY},
  {EEXIST, ERRNO__EXIST},
  {EXDEV, ERRNO__XDEV},
  {ENODEV, ERRNO__NODEV},
  {ENOTDIR, ERRNO__NOTDIR},
  {EISDIR, ERRNO__ISDIR},
  {EINVAL, ERRNO__INVAL},
  {ENFILE, ERRNO__NFILE},
  {EMFILE, ERRNO__MFILE},
  {ENOTTY, ERRNO__NOTTY},
  {ETXTBSY, ERRNO__TXTBSY},
  {EFBIG, ERRNO__FBIG},
  {ENOSPC, ERRNO__NOSPC},
  {ESPIPE, ERRNO__SPIPE},
  {EROFS, ERRNO__ROFS},
  {EMLINK, ERRNO__MLINK},
  {EPIPE, ERRNO__PIPE},
  {EDOM, ERRNO__DOM},
  {ERANGE, ERRNO__RANGE},
  {EDEADLK, ERRNO__DEADLK},
  {ENAMETOOLONG, ERRNO__NAMETOOLONG},
  {ENOLCK, ERRNO__NOLCK},
  {ENOSYS, ERRNO__NOSYS},
  {ENOTEMPTY, ERRNO__NOTEMPTY},
  {ELOOP, ERRNO__LOOP},
  {EBADCODE, ERRNO__BADCODE},
  {ENOMSG, ERRNO__NOMSG},
  {EIDRM, ERRNO__IDRM},
  {ECHRNG, ERRNO__CHRNG},
  {EL2NSYNC, ERRNO__L2NSYNC},
  {EL3HLT, ERRNO__L3HLT},
  {EL3RST, ERRNO__L3RST},
  {ELNRNG, ERRNO__LNRNG},
  {EUNATCH, ERRNO__UNATCH},
  {ENOCSI, ERRNO__NOCSI},
  {EL2HLT, ERRNO__L2HLT},
  {EBADE, ERRNO__BADE},
  {EBADR, ERRNO__BADR},
  {EXFULL, ERRNO__XFULL},
  {ENOANO, ERRNO__NOANO},
  {EBADRQC, ERRNO__BADRQC},
  {EBADSLT, ERRNO__BADSLT},
  {EBADCODE, ERRNO__BADCODE},
  {EBFONT, ERRNO__BFONT},
  {ENOSTR, ERRNO__NOSTR},
  {ENODATA, ERRNO__NODATA},
  {ETIME, ERRNO__TIME},
  {ENOSR, ERRNO__NOSR},
  {ENONET, ERRNO__NONET},
  {ENOPKG, ERRNO__NOPKG},
  {EREMOTE, ERRNO__REMOTE},
  {ENOLINK, ERRNO__NOLINK},
  {EADV, ERRNO__ADV},
  {ESRMNT, ERRNO__SRMNT},
  {ECOMM, ERRNO__COMM},
  {EPROTO, ERRNO__PROTO},
  {EMULTIHOP, ERRNO__MULTIHOP},
  {EDOTDOT, ERRNO__DOTDOT},
  {EBADMSG, ERRNO__BADMSG},
  {EOVERFLOW, ERRNO__OVERFLOW},
  {ENOTUNIQ, ERRNO__NOTUNIQ},
  {EBADFD, ERRNO__BADFD},
  {EREMCHG, ERRNO__REMCHG},
  {ELIBACC, ERRNO__LIBACC},
  {ELIBBAD, ERRNO__LIBBAD},
  {ELIBSCN, ERRNO__LIBSCN},
  {ELIBMAX, ERRNO__LIBMAX},
  {ELIBEXEC, ERRNO__LIBEXEC},
  {EILSEQ, ERRNO__ILSEQ},
  {ERESTART, ERRNO__RESTART},
  {ESTRPIPE, ERRNO__STRPIPE},
  {EUSERS, ERRNO__USERS},
  {ENOTSOCK, ERRNO__NOTSOCK},
  {EDESTADDRREQ, ERRNO__DESTADDRREQ},
  {EMSGSIZE, ERRNO__MSGSIZE},
  {EPROTOTYPE, ERRNO__PROTOTYPE},
  {ENOPROTOOPT, ERRNO__NOPROTOOPT},
  {EPROTONOSUPPORT, ERRNO__PROTONOSUPPORT},
  {ESOCKTNOSUPPORT, ERRNO__SOCKTNOSUPPORT},
  {EOPNOTSUPP, ERRNO__OPNOTSUPP},
  {EPFNOSUPPORT, ERRNO__PFNOSUPPORT},
  {EAFNOSUPPORT, ERRNO__AFNOSUPPORT},
  {EADDRINUSE, ERRNO__ADDRINUSE},
  {EADDRNOTAVAIL, ERRNO__ADDRNOTAVAIL},
  {ENETDOWN, ERRNO__NETDOWN},
  {ENETUNREACH, ERRNO__NETUNREACH},
  {ENETRESET, ERRNO__NETRESET},
  {ECONNABORTED, ERRNO__CONNABORTED},
  {ECONNRESET, ERRNO__CONNRESET},
  {ENOBUFS, ERRNO__NOBUFS},
  {EISCONN, ERRNO__ISCONN},
  {ENOTCONN, ERRNO__NOTCONN},
  {ESHUTDOWN, ERRNO__SHUTDOWN},
  {ETOOMANYREFS, ERRNO__TOOMANYREFS},
  {ETIMEDOUT, ERRNO__TIMEDOUT},
  {ECONNREFUSED, ERRNO__CONNREFUSED},
  {EHOSTDOWN, ERRNO__HOSTDOWN},
  {EHOSTUNREACH, ERRNO__HOSTUNREACH},
  {EALREADY, ERRNO__ALREADY},
  {EINPROGRESS, ERRNO__INPROGRESS},
  {ESTALE, ERRNO__STALE},
  {EUCLEAN, ERRNO__UCLEAN},
  {ENOTNAM, ERRNO__NOTNAM},
  {ENAVAIL, ERRNO__NAVAIL},
  {EISNAM, ERRNO__ISNAM},
  {EREMOTEIO, ERRNO__REMOTEIO},
  {EDQUOT, ERRNO__DQUOT},
  {ENOMEDIUM, ERRNO__NOMEDIUM},
  {EMEDIUMTYPE, ERRNO__MEDIUMTYPE}
};

#define cMaxErrno ((int) (sizeof(errno_status) / sizeof(errno_status[0])))


int
errno_ExitStatus (
  pwr_tStatus sts
)
{
  if (ODD(sts))
    return 0;
  else
    return (int)sts;
}

int
errno_ExitErrno (
  int err_no
)
{
  return err_no;
}

/* Check a POSIX return code and return
   status on PWR format, using errno. */

pwr_tStatus
errno_Pstatus (
  int psts
)
{

  if (psts == 0)
    return ERRNO__SUCCESS;

// pthread_cond_timedwait returns other values than -1
// pwr_Assert(psts == -1);

  if (errno < 0 || errno >= cMaxErrno)
    return ERRNO_BADCODE(errno);

  if (errno_status[errno].err_no == errno)
    return errno_status[errno].sts;

  return ERRNO_BADCODE(errno);
}

pwr_tStatus
errno_Status (
  int err_no
)
{

  if (err_no < 0 || err_no >= cMaxErrno)
    return ERRNO_BADCODE(err_no);

  if (errno_status[err_no].err_no == err_no)
    return errno_status[err_no].sts;

  return ERRNO_BADCODE(err_no);
}

pwr_tStatus
errno_GetStatus ()
{

  if (errno < 0 || errno >= cMaxErrno)
    return ERRNO_BADCODE(errno);

  if (errno_status[errno].err_no == errno)
    return errno_status[errno].sts;

  return ERRNO_BADCODE(errno);
}
