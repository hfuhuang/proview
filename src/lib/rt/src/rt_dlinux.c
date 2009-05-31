/* 
 * Proview   $Id: rt_dlinux.c,v 1.2 2005-09-01 14:57:55 claes Exp $
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

/* rt_dlinux.c -- Encode Linux-data
   Functions to convert data to load in file, from linux to VMS format */


#if defined(OS_VMS) || defined(OS_ELN)
static char dlinux_id = 0;
#else

#include <stdio.h>
#include <string.h>

#include "pwr.h"
#include "pwr_class.h"
#include "rt_dlinux.h"
#include "rt_gdb.h"
#include "rt_mvol.h"

/* Vax f-float on a little endian machine.


   3             2 2             1 1 1           0 0 0           0
   1             4 3             6 5 4           8 7 6           0
  +---------------+---------------+-+-------------+-+-------------+
  |       fraction 15 <- 0        |s|  exponent   : | f 22 <- 16  |
  +---------------+---------------+-+-------------+-+-------------+
   1             0 0             0   0           0 0 2           1
   5             8 7             0   7           1 0 2           6


*/

union vax_f_le {
  unsigned int	i;
  struct {
    unsigned int    f22_16	:  7;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
    unsigned int    f15_0	: 16;
  } b;
};

/* Vax f-float on a big endian machine.



  +-+-------------+   +-+-------------+   +---------------+   +---------------+
  : | f 22 <- 16  |   |s|  exponent   :   : 15 <- 0       |   |      fraction :
  +-+-------------+   +-+-------------+   +---------------+   +---------------+
   0 2           1       0           0     0             0     1             0
   0 2           6       7           1     7             0     5             8

   0             0 0             1 1 1           2 2             3
   0             7 8             5 6 7           3 4             1
  +---------------+---------------+-+-------------+-+-------------+
  |       fraction 0 -> 15        |s|  exponent   : | f 16 -> 22  |
  +---------------+---------------+-+-------------+-+-------------+
   0             0 0             1   0             0 1           2
   0             7 8             5   0             7 6           2

*/

union vax_f_be {
  unsigned int	i;
  struct {
    unsigned int    f0_15	: 16;
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f16_22	:  7;
  } b;
};

/* IEEE single on a little endian machine.

   3             2 2             1 1             0 0             0
   1             4 3             6 5             8 7             0
  +-+-------------+-+-------------+---------------+---------------+
  |s|  exponent   : |        fraction 22 <- 0     :               |
  +-+-------------+-+-------------+---------------+---------------+
     0             0 2           1 1                             0
     7             0 2           6 5                             0

*/

union i3e_s_le {
  unsigned int	i;
  struct {
    unsigned int    f22_0	: 23;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
  } b;
  struct {
    unsigned int    f15_0	: 16;
    unsigned int    f22_16	:  7;
    unsigned int    exp		:  8;
    unsigned int    sign	:  1;
  } v;
};

/* IEEE single on a big endian machine.

   0             0 0             1 1             2 2             3
   0             7 8             5 6             3 4             1
  +-+-------------+-+-------------+---------------+---------------+
  |s|  exponent   : |        fraction 0 -> 22     :               |
  +-+-------------+-+-------------+---------------+---------------+
     0           0 0 0           0 0                             2
     0           6 7 0           6 7                             2

*/

union i3e_s_be {
  unsigned int	i;
  struct {
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f0_22	: 23;
  } b;
  struct {
    unsigned int    sign	:  1;
    unsigned int    exp		:  8;
    unsigned int    f0_6	:  7;
    unsigned int    f7_22	: 16;
  } v;
};

#define VAX_F_BIAS    0x81
#define I3E_S_BIAS    0x7f
#define VAX_D_BIAS    0x81
#define VAX_G_BIAS    0x401
#define I3E_D_BIAS    0x3ff

#define IBYTE0(i) ((i >> 0x18) & 0x000000ff) 
#define IBYTE1(i) ((i >> 0x08) & 0x0000ff00) 
#define IBYTE2(i) ((i << 0x08) & 0x00ff0000) 
#define IBYTE3(i) ((i << 0x18) & 0xff000000) 

#define ENDIAN_SWAP_INT(t, s)\
  {int i = *(int *)s; *(int *)t = (IBYTE0(i) | IBYTE1(i) | IBYTE2(i) | IBYTE3(i));}

#define SBYTE0(s) ((s >> 0x08) & 0x00ff) 
#define SBYTE1(s) ((s << 0x08) & 0xff00) 

#define ENDIAN_SWAP_SHORT(t, s)\
  {short int i = *(short *)s; *(short *)t = (SBYTE0(i) | SBYTE1(i));}

#if 0
# define ENDIAN_SWAP_BOOL(t, s) (*(int *)t = (0 != *(int *)s))
#else
# define ENDIAN_SWAP_BOOL(t, s) ENDIAN_SWAP_INT(t, s)
#endif

#if (defined(OS_LINUX)) && (pwr_dHost_byteOrder == pwr_dBigEndian)

static pwr_tBoolean
dlinux_sfloat (
  char			*p,
  gdb_sAttribute	*ap
)
{
  union vax_f_le	*vp;
  union i3e_s_le	i3e;

  for (i = ap->elem; i > 0; i--) {
    vp = ((union vax_f_le *)tp);
    ENDIAN_SWAP_INT(&i3e.i, sp);
    
    /* PPC some times sets the sign bit for zero (-0), which isn't valid
     * for f-float. ML 971025
     */ 
    if (i3e.i == 0x80000000)
      i3e.i = 0; /* Clear sign bit */

    if (i3e.b.f22_0 == 0x0 && i3e.b.exp == 0xff) {  /* High value.  */
      vp->b.f22_16 = 0x7f;
      vp->b.exp	   = 0xff;
      vp->b.f15_0  = 0xffff;
    } else if (i3e.b.f22_0 == 0x0 && i3e.b.exp == 0x00) {  /* Low value.  */
      vp->i = 0;
    } else {
      vp->b.exp	   = i3e.v.exp - I3E_S_BIAS + VAX_F_BIAS;
      vp->b.f22_16 = i3e.v.f22_16;
      vp->b.f15_0  = i3e.v.f15_0;
    }

    vp->b.sign = i3e.b.sign;

    tp += sizeof(float);
    sp += sizeof(float);
    *size -= sizeof(float);
  }

  return TRUE;
}    

#elif defined(OS_LINUX) && (defined(HW_X86) || defined(HW_X86_64))

static pwr_tBoolean
dlinux_sfloat (
  char			*p,
  int			size
)
{
  int			i;
  union vax_f_le	*vp;
  union i3e_s_le	i3e;
  int			elem;

  elem = size / sizeof(float);

  for (i = elem; i > 0; i--) {
    vp = ((union vax_f_le *)p);
    i3e = *(union i3e_s_le *)p;
    
    /* PPC some times sets the sign bit for zero (-0), which isn't valid
     * for f-float. ML 971025
     */ 
    if (i3e.i == 0x80000000)
      i3e.i = 0; /* Clear sign bit */

    if (i3e.b.f22_0 == 0x0 && i3e.b.exp == 0xff) {  /* High value.  */
      vp->b.f22_16 = 0x7f;
      vp->b.exp	   = 0xff;
      vp->b.f15_0  = 0xffff;
    } else if (i3e.b.f22_0 == 0x0 && i3e.b.exp == 0x00) {  /* Low value.  */
      vp->i = 0;
    } else {
      vp->b.exp	   = i3e.v.exp - I3E_S_BIAS + VAX_F_BIAS;
      vp->b.f22_16 = i3e.v.f22_16;
      vp->b.f15_0  = i3e.v.f15_0;
    }

    vp->b.sign = i3e.b.sign;

    p += sizeof(float);
  }

  return TRUE;
}    
#endif

#if defined(OS_LINUX)

pwr_tBoolean (*dlinux[pwr_eTix_])() = {
  NULL,			/* pwr_eTix__		*/
  NULL,			/* pwr_eTix_Boolean	*/
  dlinux_sfloat,	/* pwr_eTix_Float32	*/
  NULL,			/* pwr_eTix_Float64	*/
  NULL,			/* pwr_eTix_Char	*/
  NULL,			/* pwr_eTix_Int8	*/
  NULL,			/* pwr_eTix_Int16	*/
  NULL,			/* pwr_eTix_Int32	*/
  NULL,			/* pwr_eTix_UInt8	*/
  NULL,			/* pwr_eTix_UInt16	*/
  NULL,			/* pwr_eTix_UInt32	*/
  NULL,	       		/* pwr_eTix_Objid	*/
  NULL,			/* pwr_eTix_Buffer	*/
  NULL,			/* pwr_eTix_String	*/
  NULL,			/* pwr_eTix_Enum	*/
  NULL,			/* pwr_eTix_Struct	*/
  NULL,			/* pwr_eTix_Mask	*/
  NULL,			/* pwr_eTix_Array	*/
  NULL,			/* pwr_eTix_Time	*/
  NULL,			/* pwr_eTix_Text	*/
  NULL,			/* pwr_eTix_AttrRef	*/
  NULL,			/* pwr_eTix_UInt64	*/
  NULL,			/* pwr_eTix_Int64	*/
  NULL,			/* pwr_eTix_ClassId	*/
  NULL,			/* pwr_eTix_TypeId	*/
  NULL,			/* pwr_eTix_VolumeId	*/
  NULL,			/* pwr_eTix_ObjectIx	*/
  NULL,			/* pwr_eTix_RefId	*/
};

#endif

#endif
