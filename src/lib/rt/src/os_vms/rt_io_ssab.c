/* rt_io_ssab.c -- init, read and write ssab cards.

   PROVIEW/R
   Copyright (C) 1994 by Comator Process AB.  */

# include <string.h>
# include <stdlib.h>
# include <stdio.h>

#include "pwr.h"
#include "rt_io_msg.h"
#include "co_cdh.h"
#include "rt_errh.h"
#include "rt_io_base.h"
#include "rt_io_ssab.h"


int ssab_close(	
		void	*hv_device)
{
  return 1;
}

int ssabai_ini(	
		char	*inidev,
		void	*adressini,
		void	*hv_device,
		int	relvec)
{
  pwr_tStatus sts;

  * (int *)adressini = 12345;
  sts = 1;
  return sts;
}

unsigned short ssabai_read(	
		void	*reg_diP,
		int	grupp)
{
  unsigned short di_data;

  di_data = 3020;
  return di_data;
}

unsigned short ssabaiup_read(	
		void	*reg_diP,
		int	grupp)
{
  unsigned short di_data;

  di_data = 3020;
  return di_data;
}

int ssabao_ini(	
		char	*inidev,
		void	*adressini,
		void	*hv_device,
		int	relvec)
{
  pwr_tStatus sts;

  * (int *)adressini = 12345;
  sts = 1;
  return sts;
}
void ssabao_write(	
		short	data,
		void	*reg_diP,
		int	channel)
{
  io_writeerr = 0;
  io_fatal_error = 0;
}

int ssabd_ini(	
		char	*inidev,
		void	*adressini,
		void	*hv_device,
		int	relvec)
{
  pwr_tStatus sts;

  * (int *)adressini = 12345;
  sts = 1;
  return sts;
}

unsigned short ssabdi_read(	
		void	*reg_diP,
		int	grupp)
{
  unsigned short di_data;

  di_data = 21845;
  return di_data;
}

void ssabdo_write(	
		short	data,
		void	*reg_diP,
		int	grupp)
{
  io_writeerr = 0;
  io_fatal_error = 0;
}

void ssabco_read(	
		void	*reg_diP,
		int	numofword,
		int	*co_id,
		unsigned short	*data,
		int	channel)
{
  unsigned short di_data;

  *data = 21845;
}
void ssabco_write(
		unsigned short	*data,
		void	*reg_P,
		int	channel)
{
  io_writeerr = 0;
  io_fatal_error = 0;
}

