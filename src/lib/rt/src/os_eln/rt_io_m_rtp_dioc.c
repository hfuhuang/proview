/* 
 * Proview   $Id: rt_io_m_rtp_dioc.c,v 1.2 2005-09-01 14:57:56 claes Exp $
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

/* rt_io_m_rtp_dioc.c -- io method for rtp dioc objects. */

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "rt_io_base.h"
#include "rt_errh.h"
#include "rt_io_dioc.h"
#include "rt_io_agent_init.h"
#include "rt_io_agent_close.h"

/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/

static pwr_tStatus IoAgentInit (
  io_tCtx	ctx,
  io_sAgent	*ap
) 
{
  io_sRack	*rp;

  errh_Info( "Init of agent RTP DIOC %s", ap->Name );

  io_dioc_init();

  return 1;
}


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/

static pwr_tStatus IoAgentClose (
  io_tCtx	ctx,
  io_sAgent	*ap
) 
{
  io_sRack	*rp;

  errh_Info( "Closing agent RTP DIOC %s", ap->Name );

  io_dioc_terminated();

  return 1;
}


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(RTP_DIOC) = {
  pwr_BindIoMethod(IoAgentInit),
  pwr_BindIoMethod(IoAgentClose),
  pwr_NullMethod
};
