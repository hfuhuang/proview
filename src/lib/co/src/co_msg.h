#ifndef co_msg_h
#define co_msg_h

#ifndef pwr_h
# include "pwr.h"
#endif

#if defined __cplusplus
extern "C" {
#endif

char *
msg_GetMsg (
  const int	sts,
  char		*buf,
  int		bufSize
);

char *
msg_GetText (
  const int sts,
  char *buf,
  int bufSize
);

char *
msg_GetMessage (
  const pwr_tStatus	sts,
  unsigned int		flags,
  char			*buf,
  int			bufSize
);

#if defined __cplusplus
}
#endif
#endif








