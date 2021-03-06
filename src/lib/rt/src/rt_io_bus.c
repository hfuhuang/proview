/* 
 * Proview   $Id: rt_io_bus.c,v 1.1 2008-02-29 13:06:03 claes Exp $
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

/* rt_io_bus.c -- general io bus routines. */

#if defined(OS_ELN)
# include string
# include stdlib
# include stdio
# include float
# include math
#else
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <float.h>
# include <math.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "rt_io_msg.h"
#include "co_cdh.h"
#include "rt_errh.h"
#include "rt_io_supervise.h"
#include "rt_io_base.h"

#define PB_UDATA_DIAG 1


/*----------------------------------------------------------------------------*\
  Help functions
\*----------------------------------------------------------------------------*/
int is_diag( pwr_tAttrRef *aref)
{
  pwr_tStatus sts;
  pwr_tOName name;
  char *s;
  
  if ( aref->Objid.oix == 0)
    return 0;

  sts = gdh_AttrrefToName( aref, name, sizeof(name), 
			   cdh_mName_object | cdh_mName_attribute);
  if ( EVEN(sts)) return 0;

  if ( (s = strrchr( name, '.'))) {
    if ( strncmp( s+1, "Diag_", 5) == 0)
      return 1;
  }
  else if ( strncmp( name, "Diag_", 5) == 0)
    return 1;

  return 0;
}

pwr_tInt32 GetChanSize(pwr_eDataRepEnum rep)
{
  switch (rep) {
    case pwr_eDataRepEnum_Int64:
    case pwr_eDataRepEnum_UInt64:
    case pwr_eDataRepEnum_Float64:
      return 8;
      break;
    case pwr_eDataRepEnum_Bit32:
    case pwr_eDataRepEnum_Int32:
    case pwr_eDataRepEnum_UInt32:
    case pwr_eDataRepEnum_Float32:
      return 4;
      break;
    case pwr_eDataRepEnum_Int24:
    case pwr_eDataRepEnum_UInt24:
      return 3;
      break;
    case pwr_eDataRepEnum_Bit16:
    case pwr_eDataRepEnum_Int16:
    case pwr_eDataRepEnum_UInt16:
      return 2;
      break;
    default:
      return 1;
      break;
  }
}

unsigned short swap16(unsigned short in) 
{
  unsigned short result = 0;
  result = (in << 8) & 0xFF00;
  result |= (in >> 8) & 0x00FF;
  return(result);
}

unsigned int swap32(unsigned int in) 
{
  unsigned int result = 0;;
  result  = (in << 24) & 0xFF000000;
  result |= (in <<  8) & 0x00FF0000;
  result |= (in >>  8) & 0x0000FF00;
  result |= (in >> 24) & 0x000000FF;
  return(result);
}

/*----------------------------------------------------------------------------*\
  Convert ai from raw float value to signal value and actual value
\*----------------------------------------------------------------------------*/
static void ConvertAi ( io_tCtx ctx,
                 pwr_tFloat32 f_raw,
		 pwr_sClass_ChanAi *chan_ai,
		 pwr_sClass_Ai *sig_ai,
		 io_sChannel *chanp)
{
  pwr_tFloat32		sigvalue;
  pwr_tFloat32		actvalue;
  pwr_tFloat32		*polycoef_p;
  int			i;

  sigvalue = chan_ai->SigValPolyCoef0 + chan_ai->SigValPolyCoef1 * f_raw;

  switch (chan_ai->SensorPolyType)
  {
    case 0:
      actvalue = sigvalue;
      break;
    case 1:
      actvalue = chan_ai->SensorPolyCoef0 + chan_ai->SensorPolyCoef1 * f_raw;
      break;
    case 2:
      polycoef_p = &chan_ai->SensorPolyCoef2;
      actvalue = 0;
      for ( i = 0; i < 3; i++)
      {
        actvalue = sigvalue * actvalue + *polycoef_p;
        polycoef_p--;
      }
      break;
    case 3:
      actvalue = chan_ai->SensorPolyCoef0 + chan_ai->SensorPolyCoef1 * sigvalue;
      if ( actvalue >= 0)
        actvalue = chan_ai->SensorPolyCoef2 * sqrt(actvalue);
      else
        actvalue = 0;
      break;      
    case 4:
      actvalue = chan_ai->SensorPolyCoef0 + chan_ai->SensorPolyCoef1 * sigvalue;
      if ( actvalue >= 0)
        actvalue = chan_ai->SensorPolyCoef2 * sqrt(actvalue);
      else
        actvalue = -chan_ai->SensorPolyCoef2 * sqrt(-actvalue);
      break;      
  }

  if (sig_ai->FilterType == 1 && sig_ai->FilterAttribute[0] > 0 && sig_ai->FilterAttribute[0] > ctx->ScanTime) {
    actvalue = *(sig_ai->ActualValue) + ctx->ScanTime / sig_ai->FilterAttribute[0] * (actvalue - *(sig_ai->ActualValue));
  }

  sig_ai->SigValue = sigvalue;
  *(pwr_tFloat32 *) chanp->vbp = actvalue;
  
  return;
}

/*----------------------------------------------------------------------------*\
  Initialization of ai signals and channels.
\*----------------------------------------------------------------------------*/

void io_card_read( 
  io_tCtx	         ctx,
  io_sRack	         *rp, 
  io_sCard	         *cp, 
  void                   *input_area, 
  void                   *diag_area,
  pwr_tByteOrderingEnum  byte_order, 
  pwr_tFloatRepEnum      float_rep
  )
{
  io_sChannel *chanp;
  pwr_sClass_ChanDi *chan_di;
  pwr_sClass_Di *sig_di;
  pwr_sClass_ChanAi *chan_ai;
  pwr_sClass_Ai *sig_ai;
//  pwr_sClass_ChanAit *chan_ait;
  pwr_sClass_ChanIi *chan_ii;
  pwr_sClass_Ii *sig_ii;
  pwr_tUInt8 udata8 = 0;
  pwr_tUInt16 udata16 = 0;
  pwr_tUInt32 udata32 = 0;
  pwr_tInt8 data8 = 0;
  pwr_tInt16 data16 = 0;
  pwr_tInt32 data32 = 0;
  pwr_tUInt8 *udata8p;
  pwr_tUInt16 *udata16p;
  pwr_tUInt32 *udata32p;
  pwr_tFloat32 f_raw = 0.0;
  int i;

  for (i = 0; i < cp->ChanListSize; i++) {
    chanp = &cp->chanlist[i];
    switch (chanp->ChanClass) {

      // Channel type is Di (digital input)

      case pwr_cClass_ChanDi:
	chan_di = (pwr_sClass_ChanDi *) chanp->cop;
	sig_di = (pwr_sClass_Di *) chanp->sop;
	if (chan_di && sig_di && chan_di->ConversionOn) {

	  switch (chan_di->Representation) {

            case pwr_eDataRepEnum_Bit8:
	      udata8p = input_area + cp->offset + chanp->offset;
	      * (pwr_tUInt16 *) (chanp->vbp) = chan_di->InvertOn ? ((*udata8p & chanp->mask) == 0) :
		                                                   ((*udata8p & chanp->mask) != 0);
	      break;

            case pwr_eDataRepEnum_Bit16:
	      udata16p = input_area + cp->offset + chanp->offset;
	      * (pwr_tUInt16 *) (chanp->vbp) = chan_di->InvertOn ? ((*udata16p & chanp->mask) == 0) :
		                                                   ((*udata16p & chanp->mask) != 0);
	      break;

            case pwr_eDataRepEnum_Bit32:
	      udata32p = input_area + cp->offset + chanp->offset;
	      * (pwr_tUInt16 *) (chanp->vbp) = chan_di->InvertOn ? ((*udata32p & chanp->mask) == 0) :
		                                                   ((*udata32p & chanp->mask) != 0);
	      break;

	  }

	}
	break;

      // Channel type is Ai (analog input)

      case pwr_cClass_ChanAi:
	chan_ai = (pwr_sClass_ChanAi *) chanp->cop;
	sig_ai = (pwr_sClass_Ai *) chanp->sop;
	if (chan_ai && sig_ai && chan_ai->ConversionOn) {

          if (chan_ai->CalculateNewCoef) io_AiRangeToCoef(chanp);

	  switch (chan_ai->Representation) {

            case pwr_eDataRepEnum_Int8:
	      memcpy(&data8, input_area + cp->offset + chanp->offset, 1);
	      f_raw = (float) data8;
	      sig_ai->RawValue = data8;
	      break;

            case pwr_eDataRepEnum_UInt8:
	      memcpy(&udata8, input_area + cp->offset + chanp->offset, 1);
	      f_raw = (float) udata8;
	      sig_ai->RawValue = udata8;
	      break;

            case pwr_eDataRepEnum_Int16:
	      memcpy(&data16, input_area + cp->offset + chanp->offset, 2);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) data16 = swap16(data16);
	      f_raw = (float) data16;
	      sig_ai->RawValue = data16;
	      break;

            case pwr_eDataRepEnum_UInt16:
	      memcpy(&udata16, input_area + cp->offset + chanp->offset, 2);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) data16 = swap16(udata16);
	      f_raw = (float) udata16;
	      sig_ai->RawValue = udata16;
	      break;

            case pwr_eDataRepEnum_Int24:
	      data32 = 0;
	      memcpy(&data32, input_area + cp->offset + chanp->offset, 3);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) {
		data32 = swap32(data32);
		data32 = data32 >> 8;
	      }
	      f_raw = (float) data32;
	      sig_ai->RawValue = data32;
	      break;

            case pwr_eDataRepEnum_UInt24:
	      udata32 = 0;
	      memcpy(&udata32, input_area + cp->offset + chanp->offset, 3);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) {
		udata32 = swap32(udata32);
		udata32 = udata32 >> 8;
	      }
	      f_raw = (float) udata32;
	      sig_ai->RawValue = udata32;
	      break;

            case pwr_eDataRepEnum_Int32:
	      memcpy(&data32, input_area + cp->offset + chanp->offset, 4);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) data32 = swap32(data32);
	      f_raw = (float) data32;
	      sig_ai->RawValue = data32;
	      break;

            case pwr_eDataRepEnum_UInt32:
	      memcpy(&udata32, input_area + cp->offset + chanp->offset, 4);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata32 = swap32(udata32);
	      f_raw = (float) udata32;
	      sig_ai->RawValue = udata32;
	      break;

            case pwr_eDataRepEnum_Float32:
	      memcpy(&udata32, input_area + cp->offset + chanp->offset, 4);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian ||
		  float_rep == pwr_eFloatRepEnum_FloatIEEE) udata32 = swap32(udata32);
	      memcpy(&f_raw, &udata32, 4);
	      sig_ai->RawValue = udata32;
	      break;

          }

//	    sig_ai->RawValue = 0;
	  ConvertAi(ctx, f_raw, chan_ai, sig_ai, chanp);

	}
	break;

      // Channel type is Ait (analog input with table conversion)

      case pwr_cClass_ChanAit:
	break;

      // Channel type is Ii (integer input)

      case pwr_cClass_ChanIi:
	chan_ii = (pwr_sClass_ChanIi *) chanp->cop;
	sig_ii = (pwr_sClass_Ii *) chanp->sop;
	if (chan_ii && sig_ii && chan_ii->ConversionOn) {
	  if ( (chanp->udata & PB_UDATA_DIAG) == 0) {
	    /* Fetch data from input area */
	    switch (chan_ii->Representation) {

            case pwr_eDataRepEnum_Int8:
	      memcpy(&data8, input_area + cp->offset + chanp->offset, 1);
              *(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) data8;
	      break;

            case pwr_eDataRepEnum_UInt8:
	      memcpy(&udata8, input_area + cp->offset + chanp->offset, 1);
              *(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) udata8;
	      break;

            case pwr_eDataRepEnum_Int16:
	      memcpy(&data16, input_area + cp->offset + chanp->offset, 2);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) data16 = swap16(data16);
              *(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) data16;
	      break;

            case pwr_eDataRepEnum_UInt16:
	      memcpy(&udata16, input_area + cp->offset + chanp->offset, 2);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata16 = swap16(udata16);
              *(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) udata16;
	      break;

            case pwr_eDataRepEnum_Int24:
	      data32 = 0;
	      memcpy(&data32, input_area + cp->offset + chanp->offset, 3);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) {
		data32 = swap32(data32);
		data32 = data32 >> 8;
	      }
              *(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) data32;
	      break;

            case pwr_eDataRepEnum_UInt24:
	      udata32 = 0;
	      memcpy(&udata32, input_area + cp->offset + chanp->offset, 3);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) {
		udata32 = swap32(udata32);
		udata32 = udata32 >> 8;
	      }
              *(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) udata32;
	      break;

            case pwr_eDataRepEnum_Int32:
	      memcpy(&data32, input_area + cp->offset + chanp->offset, 4);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) data32 = swap32(data32);
              *(pwr_tInt32 *) chanp->vbp = data32;
	      break;

            case pwr_eDataRepEnum_UInt32:
	      memcpy(&udata32, input_area + cp->offset + chanp->offset, 4);
	      if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata32 = swap32(udata32);
              *(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) udata32;
	      break;

	    }
	  }
	  else {
	    /* Fetch value from diagnostic area */
	    
	    if (diag_area) {

	      switch (chan_ii->Representation) {

              case pwr_eDataRepEnum_Int8:
		memcpy(&data8, diag_area + chanp->offset, 1);
        	*(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) data8;
		break;

              case pwr_eDataRepEnum_UInt8:
		memcpy(&udata8, diag_area + chanp->offset, 1);
        	*(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) udata8;
		break;

              case pwr_eDataRepEnum_Int16:
		memcpy(&data16, diag_area + chanp->offset, 2);
		if (byte_order == pwr_eByteOrderingEnum_BigEndian) data16 = swap16(data16);
        	*(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) data16;
		break;

              case pwr_eDataRepEnum_UInt16:
		memcpy(&udata16, diag_area + chanp->offset, 2);
		if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata16 = swap16(udata16);
        	*(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) udata16;
		break;

              case pwr_eDataRepEnum_Int24:
		data32 = 0;
		memcpy(&data32, diag_area + chanp->offset, 3);
		if (byte_order == pwr_eByteOrderingEnum_BigEndian) {
		  data32 = swap32(data32);
		  data32 = data32 >> 8;
		}
        	*(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) data32;
		break;

              case pwr_eDataRepEnum_UInt24:
		udata32 = 0;
		memcpy(&udata32, diag_area + chanp->offset, 3);
		if (byte_order == pwr_eByteOrderingEnum_BigEndian) {
		  udata32 = swap32(udata32);
		  udata32 = udata32 >> 8;
		}
        	*(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) udata32;
		break;

              case pwr_eDataRepEnum_Int32:
		memcpy(&data32, diag_area + chanp->offset, 4);
		if (byte_order == pwr_eByteOrderingEnum_BigEndian) data32 = swap32(data32);
        	*(pwr_tInt32 *) chanp->vbp = data32;
		break;

              case pwr_eDataRepEnum_UInt32:
		memcpy(&udata32, diag_area + chanp->offset, 4);
		if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata32 = swap32(udata32);
        	*(pwr_tInt32 *) chanp->vbp = (pwr_tInt32) udata32;
		break;

	      }
	    }
	  }
	}
	break;
    }
  }


}
/*----------------------------------------------------------------------------*\
  Write method for bus-card
\*----------------------------------------------------------------------------*/


void io_card_write( 
  io_tCtx	         ctx,
  io_sCard	         *cp, 
  void                   *output_area, 
  pwr_tByteOrderingEnum  byte_order,
  pwr_tFloatRepEnum      float_rep

  )
{

  io_sChannel *chanp;
  
  pwr_sClass_ChanDo *chan_do;
  pwr_sClass_Do *sig_do;
  pwr_sClass_ChanAo *chan_ao;
  pwr_sClass_Ao *sig_ao;
  pwr_sClass_ChanIo *chan_io;
  pwr_sClass_Io *sig_io;

  pwr_tUInt8 *udata8p;
  pwr_tUInt16 *udata16p;
  pwr_tUInt32 *udata32p;
  pwr_tUInt8 udata8 = 0;
  pwr_tUInt16 udata16 = 0;
  pwr_tUInt32 udata32 = 0;
  pwr_tInt8 data8 = 0;
  pwr_tInt16 data16 = 0;
  pwr_tInt32 data32 = 0;
  pwr_tInt32 do_actval;;
  pwr_tFloat32 value, rawvalue;
  int fixout;
  int i;



  fixout = ctx->Node->EmergBreakTrue && ctx->Node->EmergBreakSelect == FIXOUT;

  for (i = 0; i < cp->ChanListSize; i++) {
    chanp = &cp->chanlist[i];
    switch (chanp->ChanClass) {
      
      case pwr_cClass_ChanDo:
	chan_do = (pwr_sClass_ChanDo *) chanp->cop;
	sig_do = (pwr_sClass_Do *) chanp->sop;
	if (chan_do && sig_do) {

	  if (fixout)
	    do_actval = chan_do->FixedOutValue;
          else if (chan_do->TestOn != 0)
	    do_actval = chan_do->TestValue;
	  else
	    do_actval = *(pwr_tInt32 *) chanp->vbp;

	  switch (chan_do->Representation) {

            case pwr_eDataRepEnum_Bit8:
	      udata8p = output_area + cp->offset + chanp->offset;
              if (do_actval ^ chan_do->InvertOn)
		*udata8p |= chanp->mask;
              else
		*udata8p &= ~chanp->mask;
	      break;

            case pwr_eDataRepEnum_Bit16:
	      udata16p = output_area + cp->offset + chanp->offset;
	      if (do_actval ^ chan_do->InvertOn)
		*udata16p |= chanp->mask;
	      else
		*udata16p &= ~chanp->mask;
	      break;

            case pwr_eDataRepEnum_Bit32:
	      udata32p = output_area + cp->offset + chanp->offset;
              if (do_actval ^ chan_do->InvertOn)
		*udata32p |= chanp->mask;
              else
		*udata32p &= ~chanp->mask;
	      break;

	  }
	    
	}
	  
	break;

      // Channel type is Ao (analog output)

      case pwr_cClass_ChanAo:
	chan_ao = (pwr_sClass_ChanAo *) chanp->cop;
	sig_ao = (pwr_sClass_Ao *) chanp->sop;
	if (chan_ao && sig_ao) {

          if (fixout)
            value = chan_ao->FixedOutValue;
          else if (chan_ao->TestOn)
            value = chan_ao->TestValue;
          else
	    value = *(pwr_tFloat32 *) chanp->vbp;

          if (chan_ao->CalculateNewCoef) io_AoRangeToCoef(chanp);

          if (value > chan_ao->ActValRangeHigh)
            value = chan_ao->ActValRangeHigh;
          else if ( value < chan_ao->ActValRangeLow)
            value = chan_ao->ActValRangeLow;

          rawvalue = chan_ao->OutPolyCoef1 * value + chan_ao->OutPolyCoef0;
          if ( rawvalue > 0)
            rawvalue = rawvalue + 0.5;
          else
            rawvalue = rawvalue - 0.5;

//            sig_ao->RawValue = 0;

          sig_ao->SigValue = chan_ao->SigValPolyCoef1 * value + chan_ao->SigValPolyCoef0;
	    
	  switch (chan_ao->Representation) {

            case pwr_eDataRepEnum_Int8:
	      data8 = (pwr_tInt8) rawvalue;
	      memcpy(output_area + cp->offset + chanp->offset, &data8, 1);
              sig_ao->RawValue = data8;
	      break;

            case pwr_eDataRepEnum_UInt8:
	      udata8 = (pwr_tUInt8) rawvalue;
	      memcpy(output_area + cp->offset + chanp->offset, &udata8, 1);
              sig_ao->RawValue = udata8;
	      break;
		
            case pwr_eDataRepEnum_Int16:
	      data16 = (pwr_tInt16) rawvalue;
              sig_ao->RawValue = data16;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) data16 = swap16(data16);
	      memcpy(output_area + cp->offset + chanp->offset, &data16, 2);
	      break;
		
            case pwr_eDataRepEnum_UInt16:
	      udata16 = (pwr_tUInt16) rawvalue;
              sig_ao->RawValue = udata16;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata16 = swap16(udata16);
	      memcpy(output_area + cp->offset + chanp->offset, &udata16, 2);
	      break;

            case pwr_eDataRepEnum_Int24:
	      data32 = (pwr_tInt32) rawvalue;
              sig_ao->RawValue = data32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) data32 = swap32(data32) << 8;
	      memcpy(output_area + cp->offset + chanp->offset, &data32, 3);
	      break;
		
            case pwr_eDataRepEnum_UInt24:
	      udata32 = (pwr_tUInt32) rawvalue;
              sig_ao->RawValue = udata32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata32 = swap32(udata32) << 8;
	      memcpy(output_area + cp->offset + chanp->offset, &udata32, 3);
	      break;

            case pwr_eDataRepEnum_Int32:
	      data32 = (pwr_tInt32) rawvalue;
              sig_ao->RawValue = data32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) data32 = swap32(data32);
	      memcpy(output_area + cp->offset + chanp->offset, &data32, 4);
	      break;
		
            case pwr_eDataRepEnum_UInt32:
	      udata32 = (pwr_tUInt32) rawvalue;
              sig_ao->RawValue = udata32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata32 = swap32(udata32);
	      memcpy(output_area + cp->offset + chanp->offset, &udata32, 4);
	      break;
		
            case pwr_eDataRepEnum_Float32:
	      memcpy(&udata32, &rawvalue, 4);
              sig_ao->RawValue = udata32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian ||
		  float_rep == pwr_eFloatRepEnum_FloatIEEE) udata32 = swap32(udata32);
	      memcpy(output_area + cp->offset + chanp->offset, &udata32, 4);
	      break;
		
          }
	    
	}
	break;
	  
      // Channel type is Io (integer output)
	
      case pwr_cClass_ChanIo:
	chan_io = (pwr_sClass_ChanIo *) chanp->cop;
	sig_io = (pwr_sClass_Io *) chanp->sop;
	if (chan_io && sig_io) {
	  
          if (fixout)
            data32 = (pwr_tInt32) chan_io->FixedOutValue;
          else if (chan_io->TestOn)
            data32 = (pwr_tInt32) chan_io->TestValue;
          else
	    data32 = *(pwr_tInt32 *) chanp->vbp;

	  switch (chan_io->Representation) {

            case pwr_eDataRepEnum_Int8:
	      data8 = (pwr_tInt8) data32;
	      memcpy(output_area + cp->offset + chanp->offset, &data8, 1);
	      break;
		
            case pwr_eDataRepEnum_UInt8:
	      udata8 = (pwr_tUInt8) data32;
	      memcpy(output_area + cp->offset + chanp->offset, &udata8, 1);
	      break;

            case pwr_eDataRepEnum_Int16:
	      data16 = (pwr_tInt16) data32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) data16 = swap16(data16);
	      memcpy(output_area + cp->offset + chanp->offset, &data16, 2);
	      break;
		
            case pwr_eDataRepEnum_UInt16:
	      udata16 = (pwr_tUInt16) data32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata16 = swap16(udata16);
	      memcpy(output_area + cp->offset + chanp->offset, &udata16, 2);
	      break;

            case pwr_eDataRepEnum_Int24:
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) data32 = swap32(data32) << 8;
	      memcpy(output_area + cp->offset + chanp->offset, &data32, 3);
	      break;
		
            case pwr_eDataRepEnum_UInt24:
	      udata32 = (pwr_tUInt32) data32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata32 = swap32(udata32) << 8;
	      memcpy(output_area + cp->offset + chanp->offset, &udata32, 3);
	      break;
		
            case pwr_eDataRepEnum_Int32:
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) data32 = swap32(data32);
	      memcpy(output_area + cp->offset + chanp->offset, &data32, 4);
	      break;
		
            case pwr_eDataRepEnum_UInt32:
	      udata32 = (pwr_tUInt32) data32;
              if (byte_order == pwr_eByteOrderingEnum_BigEndian) udata32 = swap32(udata32);
	      memcpy(output_area + cp->offset + chanp->offset, &udata32, 4);
	      break;
				
          }
	}
	break;
    }
  }

}

