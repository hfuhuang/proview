/* rt_io_m_pb_io.c
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
   Init method for the Pb module Io
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoCardInit (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp,
  io_sCard	*cp
) 
{
  io_sCardLocal *local;
  pwr_sClass_Pb_Io *op;

  op = (pwr_sClass_Pb_Io *) cp->op;
  local = (io_sCardLocal *) cp->Local;

  if (rp->Class != pwr_cClass_Pb_DP_Slave) {
    errh_Info( "Illegal object type %s", cp->Name );
    return 1;
  }

  if (op->Status < 1) {
    errh_Info( "Error initializing Pb module Io %s", cp->Name );
  }

  return 1;
}


/*----------------------------------------------------------------------------*\
   Write method for the Pb module Io
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoCardWrite (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp,
  io_sCard	*cp
) 
{

  io_sCardLocal *local;
  pwr_sClass_Pb_Io *op;
  pwr_sClass_Pb_DP_Slave *slave;
  int i;
  pwr_tInt8 data8 = 0;
  pwr_tInt16 data16 = 0;
  pwr_tInt32 data32 = 0;
  pwr_sClass_ChanIo *cop;
  pwr_sClass_Io *sop;
  io_sChannel *chanp;

  local = (io_sCardLocal *) cp->Local;
  op = (pwr_sClass_Pb_Io *) cp->op;
  slave = (pwr_sClass_Pb_DP_Slave *) rp->op;

  if (op->Status >= 1) {

    for (i=0; i<cp->ChanListSize; i++) {

      chanp = &cp->chanlist[i];
      if (!chanp->cop) continue;

      cop = (pwr_sClass_ChanIo *) chanp->cop;
      sop = (pwr_sClass_Io *) chanp->sop;

      if (cop->TestOn != 0) continue;

      data32 = *(pwr_tInt32 *) chanp->vbp;

      if (op->BytesPerChannel == 4) {
        if (slave->ByteOrdering == PB_BYTEORDERING_BE) data32 = swap32(data32);
        memcpy(local->output_area + op->OffsetOutputs + 4*i, &data32, 4);
      }
      else if (op->BytesPerChannel == 2) {
        data16 = (pwr_tInt16) data32;
        if (slave->ByteOrdering == PB_BYTEORDERING_BE) data16 = swap16(data16);
        memcpy(local->output_area + op->OffsetOutputs + 2*i, &data16, 2);
      }
      else if (op->BytesPerChannel == 1) {
        data8 = (pwr_tInt8) data32;
        memcpy(local->output_area + op->OffsetOutputs + i, &data8, 1);
      }
    }
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

pwr_dExport pwr_BindIoMethods(Pb_Io) = {
  pwr_BindIoMethod(IoCardInit),
  pwr_BindIoMethod(IoCardWrite),
  pwr_BindIoMethod(IoCardClose),
  pwr_NullMethod
};
