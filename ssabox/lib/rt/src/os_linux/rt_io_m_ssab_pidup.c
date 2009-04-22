/* 
 * Proview   $Id: rt_io_m_ssab_aiup.c,v 1.4 2007-04-30 12:08:08 claes Exp $
 * Copyright (C) 2005 SSAB Oxelösund AB.
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

/* rt_io_m_ssab_pid.c -- io methods for ssab cards. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>


#include "pwr.h"
#include "co_cdh.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "pwr_baseclasses.h"
#include "pwr_basecomponentclasses.h"
#include "pwr_ssaboxclasses.h"
#include "rt_io_base.h"
#include "rt_io_msg.h"
#include "rt_io_ssab.h"
#include "rt_io_card_init.h"
#include "rt_io_card_close.h"
#include "rt_io_card_read.h"
#include "qbus_io.h"
#include "rt_io_m_ssab_locals.h"
#include "rt_io_bfbeth.h"


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/


#define init_mask		0x0001
#define inc3p_mask		0x0002
#define pos3p_mask		0x0004
#define ao_mask			0x0008
#define dyn_bias_mask		0x0010
#define stall_freeze_mask	0x0020
#define stall_cont_mask		0x0040
#define setup_req_mask		0x1000 // bit 12
#define act_dat_mask		0x2000 // bit 13
#define setup_mask		0x4000 // bit 14
#define dyn_mask		0x8000 // bit 15
#define typ_mask		0xf001 // bit 12, 13, 14, 15 + bit 1


typedef struct
        {
            unsigned short      csr1;
            unsigned short      csr2;
        }register_def;

typedef struct
        {
            float			SetVal;
            float			BiasD;
            float			ForcVal;
            short int			Force;
            short int			IntOff;
        } card_dyn;

typedef struct
        {
            float			inc3pGain;
            float			MinTim;
            float			MaxTim;
            float			MaxInteg;
            float			AVcoeff[2];
            float			BVcoeff[2];
            float			OVcoeff[2];
            short int			PidAlg;
            short int			Inverse;
            float			PidGain;
            float			IntTime;
            float			DerTime;
            float			DerGain;
            float			BiasGain;
            float			MinOut;
            float			MaxOut;
            float			EndHys;
            float			PVcoeff[2];
            float			ErrSta;
            float			ErrSto;
            float			pos3pGain;
            float			ProcFiltTime;
            float			BiasFiltTime;
            float			PosFiltTime;
        }card_par;

typedef struct
        {
            float			PosVal;
            float			Acc;
            float			ProcVal;
            float			OutVal;
            float			ControlDiff;
            short int			EndMax;
            short int			EndMin;
            float			Bias;
            float			ScanTime;
        }card_stat;

typedef struct
        {
            float		Max;		/* Max ingenjörsvärde */
            float		Min;		/* Min ingenjörsvärde */
            float		RawMax;		/* Råvärde vid Max (+- 30000) */
            float		RawMin;		/* Råvärde vid Min (+- 30000) */
        }card_range;

typedef struct {
            unsigned short int		Istat[2];
            card_dyn			Dyn;
            card_par			Par;
            card_stat			Stat;
            card_range			AoRange;
            card_range			ProcRange;
            card_range			BiasRange;
            card_range			PosRange;
            pwr_sClass_PidX		*objP;
            unsigned int		Address;
	    int	                        Qbus_fp;
            unsigned short              dyn_ind;
            unsigned short              par_ind;
            unsigned short              stat_ind;
            unsigned short              ran_ind;
            unsigned short              init;
            pwr_tTime                   ErrTime;
            unsigned short              Valid;
} io_sLocal;

void swap_word(unsigned int *out, unsigned int *in) 
{
  unsigned int result = 0;;
  result  = (*in << 16) & 0xFFFF0000;
  result |= (*in >>  16) & 0x0000FFFF;
  *out = result;
}

static int ssabpid_read(unsigned short index,
                         unsigned short *data,
                         io_sLocal      *local) {

  qbus_io_read 		rb;
  qbus_io_write 	wb;
  int                   sts;

  wb.Address = local->Address;
  wb.Data = index; 

  sts = write( local->Qbus_fp, &wb, sizeof(wb));

  if (sts != -1) {
    rb.Address = local->Address + 2;
    sts = read( local->Qbus_fp, &rb, sizeof(rb));
    *data = (unsigned short) rb.Data;
  }

  return sts;

}
static int ssabpid_write(unsigned short index,
                         unsigned short *data,
                         io_sLocal      *local) {

  qbus_io_write 	wb;
  int                   sts;

  wb.Address = local->Address;
  wb.Data = index; 

  sts = write( local->Qbus_fp, &wb, sizeof(wb));

  if (sts != -1) {
    wb.Address = local->Address + 2;
    wb.Data = *data; 
    sts = write( local->Qbus_fp, &wb, sizeof(wb));
  }

  return sts;

}
static pwr_tStatus IoCardInit (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp,
  io_sCard	*cp  
) 
{
  pwr_sClass_Ssab_PIDuP *op;
  io_sLocal 		*local;
  pwr_tStatus		sts;

  op = (pwr_sClass_Ssab_PIDuP *) cp->op;
  local = calloc( 1, sizeof(*local));
  cp->Local = local;
  local->Address = op->RegAddress;
  local->Qbus_fp = ((io_sRackLocal *)(rp->Local))->Qbus_fp;

  errh_Info( "Init of pid card '%s'", cp->Name);

  sts = gdh_ObjidToPointer(op->PidXCon, (void *) &local->objP);

  if ( EVEN(sts)) {
    errh_Error( "PID-card not properly connected, %s", cp->Name);
    return IO__ERRDEVICE;
  }

  /* Calculate indexes in I/O-area */

  local->dyn_ind = ((char *) &local->Dyn - (char *) local) / 2;
  local->par_ind = ((char *) &local->Par - (char *) local) / 2;
  local->stat_ind = ((char *) &local->Stat - (char *) local) / 2;
  local->ran_ind = ((char *) &local->AoRange - (char *) local) / 2;

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
  io_sLocal 		*local;

  errh_Info( "IO closing pid card '%s'", cp->Name);

  local = (io_sLocal *) cp->Local;
  free( (char *) local);

  return 1;
}


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoCardRead (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp,
  io_sCard	*cp  
) 
{
  io_sLocal 		*local;
  pwr_sClass_Ssab_PIDuP *op;
  int			sts, ii;
  card_stat             c_stat;
  unsigned short        *datap;
  unsigned short        diff;
  pwr_tTime             now;

  local = (io_sLocal *) cp->Local;
  op = (pwr_sClass_Ssab_PIDuP *) cp->op;

  sts = ssabpid_read(0, &local->Istat[0], local);
  if (sts != -1) {
    sts = ssabpid_read(1, &local->Istat[1], local);
  }

  if (sts == -1) {
    op->ErrorCount++;
    local->Valid = 0;
  } else local->Valid = 1;

  diff = local->Istat[0] ^ local->Istat[1];

  memset(&c_stat, 0, sizeof(c_stat));

  if (local->Valid && (diff & act_dat_mask)) {
    for (ii = local->stat_ind, datap = (unsigned short *) &c_stat; ii < local->ran_ind; ii++, datap++) {

      sts = ssabpid_read(ii, datap, local);

      if ( sts == -1) {
        /* Increase error count and check error limits */
        clock_gettime(CLOCK_REALTIME, &now);

        if (op->ErrorCount > op->ErrorSoftLimit) {
          /* Ignore if some time has expired */
          if (now.tv_sec - local->ErrTime.tv_sec < 600)
            op->ErrorCount++;
        }
        else
          op->ErrorCount++;
        local->ErrTime = now;

        if ( op->ErrorCount == op->ErrorSoftLimit)
          errh_Error( "IO Error soft limit reached on card '%s'", cp->Name);

        continue;
      }
    } 

    /* Move data to PidX-object */

    swap_word((unsigned int *) &local->objP->ProcVal, (unsigned int *) &c_stat.ProcVal);
    swap_word((unsigned int *) &local->objP->PosVal, (unsigned int *) &c_stat.PosVal);
    swap_word((unsigned int *) &local->objP->OutVal, (unsigned int *) &c_stat.OutVal);
    swap_word((unsigned int *) &local->objP->ControlDiff, (unsigned int *) &c_stat.ControlDiff);
    if (local->objP->BiasGain != 0) swap_word((unsigned int *) &local->objP->Bias, (unsigned int *) &c_stat.Bias);
    local->objP->EndMin = c_stat.EndMin;
    local->objP->EndMax = c_stat.EndMax;
//    local->objP->ScanTime = c_stat.ScanTime;

    local->Istat[0] ^= act_dat_mask;

  }

  if ( op->ErrorCount >= op->ErrorHardLimit)
  {
     errh_Error( "IO Error hard limit reached on card '%s', IO stopped", cp->Name);
     ctx->Node->EmergBreakTrue = 1;
     return IO__ERRDEVICE;
  }

  return 1;
}
/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoCardWrite (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp,
  io_sCard	*cp  
) 
{
  io_sLocal 		*local;
  pwr_sClass_Ssab_PIDuP	*op;
  int			ii;
  int			sts;
  pwr_tTime             now;
  unsigned short        diff;
  card_dyn		c_dyn;
  card_par		c_par;
  int			paramc = 0;		
  int			dynparc = 0;	
  unsigned short	*datap;		

  local = (io_sLocal *) cp->Local;
  op = (pwr_sClass_Ssab_PIDuP *) cp->op;

  if (!local->Valid) return 1;

  diff = local->Istat[0] ^ local->Istat[1];

  if (diff & setup_req_mask) paramc = 1, dynparc = 1;

  /* Check if static parameters has changed */
  if ((local->objP->PidAlg != local->Par.PidAlg) ||
      (local->objP->Inverse != local->Par.Inverse) ||
      (local->objP->PidGain != local->Par.PidGain) ||
      (local->objP->IntTime != local->Par.IntTime) ||
      (local->objP->DerTime != local->Par.DerTime) ||
      (local->objP->DerGain != local->Par.DerGain) ||
      (local->objP->BiasGain != local->Par.BiasGain) ||
      (local->objP->MinOut != local->Par.MinOut) ||
      (local->objP->MaxOut != local->Par.MaxOut) ||
      (local->objP->EndHys != local->Par.EndHys) ||
      (local->objP->ProcFiltTime != local->Par.ProcFiltTime) ||
      (local->objP->ProcMax != local->ProcRange.Max) ||
      (local->objP->ProcMin != local->ProcRange.Min) ||
      (local->objP->AoMax != local->AoRange.Max) ||
      (local->objP->AoMin != local->AoRange.Min) ||
      (local->Par.inc3pGain != local->objP->Inc3pGain) ||
      (local->Par.MinTim != local->objP->MinTim) ||
      (local->Par.MaxTim != local->objP->MaxTim) ||
      (local->Par.MaxInteg != local->objP->MaxInteg)) paramc = 1;

  /* Check if dynamic parameters has changed */
  if ((local->objP->SetVal != local->Dyn.SetVal) ||
      (local->objP->Bias != local->Dyn.BiasD) ||
      (local->objP->ForcVal != local->Dyn.ForcVal) ||
      (local->objP->IntOff != local->Dyn.IntOff) ||
      (local->objP->Force != local->Dyn.Force)) dynparc = 1;

  /* Move parameters to local */
  if (paramc && ((diff & setup_mask) == 0)) {

    local->Par.inc3pGain = local->objP->Inc3pGain;
    local->Par.MinTim = local->objP->MinTim;
    local->Par.MaxTim = local->objP->MaxTim;
    local->Par.MaxInteg = local->objP->MaxInteg;

    local->ProcRange.Max = local->objP->ProcMax;
    local->ProcRange.Min = local->objP->ProcMin;
    local->ProcRange.RawMax = local->objP->ProcRawMax;
    local->ProcRange.RawMin = local->objP->ProcRawMin;
    if (local->ProcRange.RawMax != local->ProcRange.RawMin) {
      local->Par.AVcoeff[0] = (local->ProcRange.Max - local->ProcRange.Min) /
		(local->ProcRange.RawMax - local->ProcRange.RawMin);
      local->Par.AVcoeff[1] = local->ProcRange.Min -
		local->ProcRange.RawMin * local->Par.AVcoeff[0];
    }
    local->BiasRange.Max = local->objP->BiasMax;
    local->BiasRange.Min = local->objP->BiasMin;
    local->BiasRange.RawMax = local->objP->BiasRawMax;
    local->BiasRange.RawMin = local->objP->BiasRawMin;
    if (local->BiasRange.RawMax != local->BiasRange.RawMin) {
      local->Par.BVcoeff[0] = (local->BiasRange.Max - local->BiasRange.Min) /
		(local->BiasRange.RawMax - local->BiasRange.RawMin);
      local->Par.BVcoeff[1] = local->BiasRange.Min -
		local->BiasRange.RawMin * local->Par.BVcoeff[0];
    }
    local->AoRange.Max = local->objP->AoMax;
    local->AoRange.Min = local->objP->AoMin;
    local->AoRange.RawMax = local->objP->AoRawMax;
    local->AoRange.RawMin = local->objP->AoRawMin;
    if (local->AoRange.Max != local->AoRange.Min)
    {
      local->Par.OVcoeff[0] = (local->AoRange.RawMax - local->AoRange.RawMin) /
		(local->AoRange.Max - local->AoRange.Min);
      local->Par.OVcoeff[1] = local->AoRange.RawMin -
		local->AoRange.Min * local->Par.OVcoeff[0];
    }
    local->PosRange.Max = local->objP->PosMax;
    local->PosRange.Min = local->objP->PosMin;
    local->PosRange.RawMax = local->objP->PosRawMax;
    local->PosRange.RawMin = local->objP->PosRawMin;
    if (local->PosRange.RawMax != local->PosRange.RawMin) {
      local->Par.PVcoeff[0] = (local->PosRange.Max - local->PosRange.Min) /
		(local->PosRange.RawMax - local->PosRange.RawMin);
      local->Par.PVcoeff[1] = local->PosRange.Min -
		local->PosRange.RawMin * local->Par.PVcoeff[0];
    }

    local->Par.PidAlg = local->objP->PidAlg;
    local->Par.Inverse = local->objP->Inverse;
    local->Par.PidGain = local->objP->PidGain;
    local->Par.IntTime = local->objP->IntTime;
    local->Par.DerTime = local->objP->DerTime;
    local->Par.DerGain = local->objP->DerGain;
    local->Par.BiasGain = local->objP->BiasGain;
    local->Par.MinOut = local->objP->MinOut;
    local->Par.MaxOut = local->objP->MaxOut;
    local->Par.EndHys = local->objP->EndHys;
    local->Par.ErrSta = local->objP->ErrSta;
    local->Par.ErrSto = local->objP->ErrSto;
    local->Par.pos3pGain = local->objP->Pos3pGain;
    local->Par.ProcFiltTime = local->objP->ProcFiltTime;
    local->Par.BiasFiltTime = local->objP->BiasFiltTime;
    local->Par.PosFiltTime = local->objP->PosFiltTime;

    /* Write parameters to card */
    c_par.PidAlg = local->Par.PidAlg;
    c_par.Inverse = local->Par.Inverse;
    swap_word((unsigned int *) &c_par.inc3pGain, (unsigned int *) &local->Par.inc3pGain);
    swap_word((unsigned int *) &c_par.MinTim, (unsigned int *) &local->Par.MinTim);
    swap_word((unsigned int *) &c_par.MaxTim, (unsigned int *) &local->Par.MaxTim);
    swap_word((unsigned int *) &c_par.MaxInteg, (unsigned int *) &local->Par.MaxInteg);
    swap_word((unsigned int *) &c_par.AVcoeff[0], (unsigned int *) &local->Par.AVcoeff[0]);
    swap_word((unsigned int *) &c_par.AVcoeff[1], (unsigned int *) &local->Par.AVcoeff[1]);
    swap_word((unsigned int *) &c_par.BVcoeff[0], (unsigned int *) &local->Par.BVcoeff[0]);
    swap_word((unsigned int *) &c_par.BVcoeff[1], (unsigned int *) &local->Par.BVcoeff[1]);
    swap_word((unsigned int *) &c_par.OVcoeff[0], (unsigned int *) &local->Par.OVcoeff[0]);
    swap_word((unsigned int *) &c_par.OVcoeff[1], (unsigned int *) &local->Par.OVcoeff[1]);
    swap_word((unsigned int *) &c_par.PidGain, (unsigned int *) &local->Par.PidGain);
    swap_word((unsigned int *) &c_par.IntTime, (unsigned int *) &local->Par.IntTime);
    swap_word((unsigned int *) &c_par.DerTime, (unsigned int *) &local->Par.DerTime);
    swap_word((unsigned int *) &c_par.DerGain, (unsigned int *) &local->Par.DerGain);
    swap_word((unsigned int *) &c_par.BiasGain, (unsigned int *) &local->Par.BiasGain);
    swap_word((unsigned int *) &c_par.MinOut, (unsigned int *) &local->Par.MinOut);
    swap_word((unsigned int *) &c_par.MaxOut, (unsigned int *) &local->Par.MaxOut);
    swap_word((unsigned int *) &c_par.EndHys, (unsigned int *) &local->Par.EndHys);
    swap_word((unsigned int *) &c_par.PVcoeff[0], (unsigned int *) &local->Par.PVcoeff[0]);
    swap_word((unsigned int *) &c_par.PVcoeff[1], (unsigned int *) &local->Par.PVcoeff[1]);
    swap_word((unsigned int *) &c_par.ErrSta, (unsigned int *) &local->Par.ErrSta);
    swap_word((unsigned int *) &c_par.ErrSto, (unsigned int *) &local->Par.ErrSto);
    swap_word((unsigned int *) &c_par.pos3pGain, (unsigned int *) &local->Par.pos3pGain);
    swap_word((unsigned int *) &c_par.ProcFiltTime, (unsigned int *) &local->Par.ProcFiltTime);
    swap_word((unsigned int *) &c_par.BiasFiltTime, (unsigned int *) &local->Par.BiasFiltTime);
    swap_word((unsigned int *) &c_par.PosFiltTime, (unsigned int *) &local->Par.PosFiltTime);


    for (ii = local->par_ind, datap = (unsigned short *)&c_par; ii < local->stat_ind; ii++, datap++) {

      sts = ssabpid_write(ii, datap, local);

      if ( sts == -1) {
        /* Increase error count and check error limits */
        clock_gettime(CLOCK_REALTIME, &now);

        if (op->ErrorCount > op->ErrorSoftLimit) {
          /* Ignore if some time has expired */
          if (now.tv_sec - local->ErrTime.tv_sec < 600)
            op->ErrorCount++;
        }
        else
          op->ErrorCount++;
        local->ErrTime = now;

        if ( op->ErrorCount == op->ErrorSoftLimit)
          errh_Error( "IO Error soft limit reached on card '%s'", cp->Name);

        continue;
      }
    } 

    /* Update status word */
    if ((diff & setup_req_mask) != 0) local->Istat[0] ^= setup_req_mask;
    local->Istat[0] ^= setup_mask;

  } /* End - if new parameters */

  /* Send dynamic parameters ? */
  if (dynparc && ((diff & dyn_mask) == 0)) {

    /* Move parameters to local */

    local->Dyn.SetVal = local->objP->SetVal;
    local->Dyn.BiasD = local->objP->Bias;
    local->Dyn.ForcVal = local->objP->ForcVal;
    local->Dyn.IntOff = local->objP->IntOff;
    local->Dyn.Force = local->objP->Force;

    /* Write parameters to card */

    swap_word((unsigned int *) &c_dyn.SetVal, (unsigned int *) &local->Dyn.SetVal);
    swap_word((unsigned int *) &c_dyn.BiasD, (unsigned int *) &local->Dyn.BiasD);
    swap_word((unsigned int *) &c_dyn.ForcVal, (unsigned int *) &local->Dyn.ForcVal);
    c_dyn.Force = local->Dyn.Force;
    c_dyn.IntOff = local->Dyn.IntOff;

    for (ii = local->dyn_ind, datap = (unsigned short *)&c_dyn; ii < local->par_ind; ii++, datap++) {

      sts = ssabpid_write(ii, datap, local);

      if ( sts == -1) {
        /* Increase error count and check error limits */
        clock_gettime(CLOCK_REALTIME, &now);

        if (op->ErrorCount > op->ErrorSoftLimit) {
          /* Ignore if some time has expired */
          if (now.tv_sec - local->ErrTime.tv_sec < 600)
            op->ErrorCount++;
        }
        else
          op->ErrorCount++;
        local->ErrTime = now;

        if ( op->ErrorCount == op->ErrorSoftLimit)
          errh_Error( "IO Error soft limit reached on card '%s'", cp->Name);

        continue;
      }
    } 

    /* Update status word */
    local->Istat[0] ^= dyn_mask;
  } /* End if nya dyn par */

  /* Update status-word */
  local->Istat[0] &= typ_mask;
  local->Istat[0] |= ao_mask;
  if (local->objP->UseInc3p) local->Istat[0] |= inc3p_mask;
  if (local->objP->UsePos3p) local->Istat[0] |= pos3p_mask;
  if (local->objP->UseDynBias) local->Istat[0] |= dyn_bias_mask;
  if (local->objP->UseAo) local->Istat[0] |= stall_freeze_mask;
  else local->Istat[0] |= stall_cont_mask;

  datap = &local->Istat[0];

  sts = ssabpid_write(0, datap, local);

  if ( sts == -1)
    op->ErrorCount++;

  if ( op->ErrorCount >= op->ErrorHardLimit)
  {
     errh_Error( "IO Error hard limit reached on card '%s', IO stopped", cp->Name);
     ctx->Node->EmergBreakTrue = 1;
     return IO__ERRDEVICE;
  }

  return 1;
  
}


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Ssab_PIDuP) = {
  pwr_BindIoMethod(IoCardInit),
  pwr_BindIoMethod(IoCardClose),
  pwr_BindIoMethod(IoCardRead),
  pwr_BindIoMethod(IoCardWrite),
  pwr_NullMethod
};
