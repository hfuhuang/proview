/* rt_io_m_pb_do.c
   PROVIEW/R	*/

#pragma pack(1)

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include "pb_type.h"
#include "pb_if.h"
#include "pb_fmb.h"
#include "pb_dp.h"
#include "rt_io_pb_locals.h"

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "rt_io_base.h"
#include "rt_io_msg.h"
#include "rt_errh.h"
#include "rt_io_profiboard.h"


/*----------------------------------------------------------------------------*\
   Init method for the Pb module Do
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoCardInit (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp,
  io_sCard	*cp
) 
{
  io_sCardLocal *local;
  pwr_sClass_Pb_Do *op;
  pwr_sClass_Pb_DP_Slave *slave;

  op = (pwr_sClass_Pb_Do *) cp->op;
  local = (io_sCardLocal *) cp->Local;

  local->byte_swap = 0;

  if (rp->Class == pwr_cClass_Pb_DP_Slave) {
    slave = (pwr_sClass_Pb_DP_Slave *) rp->op;

    /* Byte swap if Big Endian (bit 0) and
       Digital modules wordoriented  (bit 1) */

    if ((slave->ByteOrdering & 1) &&
        (slave->ByteOrdering & 1<<1))
      local->byte_swap = 1;

  }
  else {
    errh_Info( "Error initializing Pb module Do %s", cp->Name );
    return 1;
  }

  if (op->Status < 1) {
    errh_Info( "Error initializing Pb module Do %s", cp->Name );
  }

  return 1;
}


/*----------------------------------------------------------------------------*\
   Write method for the Pb module Do
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoCardWrite (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp,
  io_sCard	*cp
) 
{
  io_sCardLocal *local;
  pwr_sClass_Pb_Do *op;
  pwr_tUInt16 data[2] = {0, 0};
//  pwr_tUInt16 invmask;
//  pwr_tUInt16 convmask;
  int i;

  local = (io_sCardLocal *) cp->Local;
  op = (pwr_sClass_Pb_Do *) cp->op;

  if (op->Status >= 1) {

    for (i=0; i<2; i++) {
      if (i==1 && op->NumberOfChannels <= 16) break;
      io_DoPackWord(cp, &data[i], i);
      if (local->byte_swap == 1) data[i] = swap16(data[i]);      
    }
    memcpy(local->output_area + op->OffsetOutputs, &data, op->BytesOfOutput);
  }
  return 1;
}


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoCardClose (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp,
  io_sCard	*cp
) 
{
  io_sCardLocal *local;
  local = rp->Local;

  free ((char *) local);

  return 1;
}



/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Pb_Do) = {
  pwr_BindIoMethod(IoCardInit),
  pwr_BindIoMethod(IoCardWrite),
  pwr_BindIoMethod(IoCardClose),
  pwr_NullMethod
};
