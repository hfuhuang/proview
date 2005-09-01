/* 
 * Proview   $Id: co_time_os.c,v 1.4 2005-09-01 14:57:52 claes Exp $
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

/* co_time_os.c -- OS specific time routines.
*/


#ifndef OS_LINUX
# error This file is only for Linux
#endif
#include <unistd.h>
#include <sys/times.h>
#include <string.h>
#include "pwr.h"
#include "co_time.h"
#include "co_time_msg.h"


/* Return delta time since system start.
   Add delta time 'add'.  */

#if 0
pwr_tDeltaTime *
time_Uptime (
  pwr_tStatus *status,
  pwr_tDeltaTime *tp,
  pwr_tDeltaTime *ap
)
{
  pwr_tDeltaTime time;
  long tics;
  struct tms buff;
  static int tics_per_sec = 0;
  static pwr_tTime boot_time = {0,0};
  static pwr_tDeltaTime max_diff = {0,20000000};
  pwr_tDeltaTime uptime_tics;
  pwr_tTime current_time;
  pwr_tDeltaTime diff;
  pwr_dStatus(sts, status, TIME__SUCCESS);
 
  if ( !tics_per_sec)
    tics_per_sec = sysconf(_SC_CLK_TCK);

  if (tp == NULL)
    tp = &time;

  tics = times(&buff);
  uptime_tics.tv_sec = tics / tics_per_sec;
  uptime_tics.tv_nsec = (tics % tics_per_sec) * (1000000000 / tics_per_sec);

  // pwr_Assert(tp->tv_sec >= 0 && tp->tv_nsec >= 0);

  clock_gettime( CLOCK_REALTIME, &current_time);
  if ( !boot_time.tv_sec) {
    time_Asub( &boot_time, &current_time, &uptime_tics);
    *tp = uptime_tics;
  }
  else {
    time_Adiff( tp, &current_time, &boot_time);
    time_Dsub( &diff, tp, &uptime_tics);
    time_Dabs(NULL, &diff);
    if ( time_Dcomp(&diff, &max_diff) > 0) {
      time_Asub( &boot_time, &current_time, &uptime_tics);
      *tp = uptime_tics;
      *status = TIME__CLKCHANGE;
    }
  }

  if (ap != NULL)
    return time_Dadd(tp, tp, ap);
  else
    return tp;
}
#endif

/* Modified to keep uptime tics as a 64-bit unsigned.
 * This way uptime tics won't wrap around for another 8000 years or so 
 * when HZ is at a 1000.
 * RK 031112
 */

pwr_tDeltaTime *
time_Uptime (
  pwr_tStatus *status,
  pwr_tDeltaTime *tp,
  pwr_tDeltaTime *ap
)
{
  pwr_tDeltaTime time;
  unsigned long tics;
  static unsigned long long tics_64;
  struct tms buff;
  static int tics_per_sec = 0;
  static pwr_tTime boot_time = {0,0};
  static pwr_tDeltaTime max_diff = {0, 20000000};
  pwr_tDeltaTime uptime_tics;
  pwr_tTime current_time;
  pwr_tDeltaTime diff;
  static pwr_tUInt16 msb_flips = 0;
  static pwr_tBoolean old_high_bit = 0;
  pwr_tBoolean high_bit;
  lldiv_t uptime_s;
  pwr_dStatus(sts, status, TIME__SUCCESS);
 
  if ( !tics_per_sec)
    tics_per_sec = sysconf(_SC_CLK_TCK);

  if (tp == NULL)
    tp = &time;

  tics = times(&buff);

  high_bit = tics >> (32 - 1);
  if (!high_bit && old_high_bit)
    msb_flips++;
  old_high_bit = high_bit;

  tics_64  = ((unsigned long long) msb_flips << 32) | tics;
  uptime_s = lldiv(tics_64, (long long) tics_per_sec);
  
  uptime_tics.tv_sec  = (unsigned long) uptime_s.quot;
  uptime_tics.tv_nsec = ((unsigned long) uptime_s.rem) * (1000000000 / tics_per_sec);

  // pwr_Assert(tp->tv_sec >= 0 && tp->tv_nsec >= 0);

  clock_gettime( CLOCK_REALTIME, &current_time);
  if ( !boot_time.tv_sec) {
    time_Asub( &boot_time, &current_time, &uptime_tics);
    *tp = uptime_tics;
  }
  else {
    time_Adiff( tp, &current_time, &boot_time);
    time_Dsub( &diff, tp, &uptime_tics);
    time_Dabs(NULL, &diff);
    if ( time_Dcomp(&diff, &max_diff) > 0) {
      time_Asub( &boot_time, &current_time, &uptime_tics);
      *tp = uptime_tics;
      if (status != NULL) {
        *status = TIME__CLKCHANGE;
      }
    }
  }

  if (ap != NULL)
    return time_Dadd(tp, tp, ap);
  else
    return tp;
}

/* Return number of clock ticks since system start.
   Add number of tics corresponding to delta time 'add'.  */

time_tClock
time_Clock (
  pwr_tStatus *status,
  pwr_tDeltaTime *ap
)
{
  long		tics;
  struct tms	buff;
  static int    tics_per_sec = 0;
  pwr_dStatus(sts, status, TIME__SUCCESS);

  if ( !tics_per_sec)
    tics_per_sec = sysconf(_SC_CLK_TCK);

  tics = times(&buff);

  if (ap != NULL) {
    tics += (ap->tv_sec * tics_per_sec) + (ap->tv_nsec / (1000000000 / tics_per_sec));
  }

  return tics;
}

time_tOs *
time_Os (
  pwr_tStatus *status,
  time_tOs *tp
)
{
  static time_tOs os_time;
  pwr_dStatus(sts, status, TIME__SUCCESS);

  if (tp == NULL)
    tp = &os_time;

  clock_gettime(CLOCK_REALTIME, tp);

  return tp;
}

/* Set system time */
pwr_tStatus
time_SetTime(
  pwr_tTime *pt
)
{
  pwr_tStatus sts = TIME__SUCCESS;

  return sts;
}


time_tOs *
time_AtoOs (
  pwr_tStatus *status,
  time_tOs *tp,
  pwr_tTime *ap
)
{
  time_tOs os_time;
  pwr_dStatus(sts, status, TIME__SUCCESS);

  if (tp == NULL)
    tp = &os_time;

  *tp = *ap;

  return tp;
}

/* Convert from Proview delta time format
   to native time format. */

time_tOs *
time_DtoOs (
  pwr_tStatus *status,
  time_tOs *tp,
  pwr_tDeltaTime *dp
)
{
  time_tOs os_time;
  pwr_dStatus(sts, status, TIME__SUCCESS);

  if (tp == NULL)
    tp = &os_time;

  memcpy(tp, dp, sizeof(*tp));

  return tp;
}
