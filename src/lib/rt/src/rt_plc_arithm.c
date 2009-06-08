/* 
 * Proview   $Id: rt_plc_arithm.c,v 1.10 2007-11-01 15:34:55 claes Exp $
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

#ifdef OS_ELN
#  include stdio
#else
#  include <stdio.h>
#endif
#include <limits.h>
#include <math.h>
#include <float.h>
#include <malloc.h>

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "rt_plc.h"


/* 		PLC RUTINER			*/

/*_*
  SUM
  funktion:       Add  up to 8 analogue values.
                  Each value is multiplied with  a factor.

  @aref sum Sum
*/
void sum_exec(
  plc_sThread		*tp,
  pwr_sClass_sum	*object)
{
#define sumsize 8
	char    *charp;
        float	**ptr;          /* Pointer to ptr to input */
	static ptrdiff_t offset = (char *)&object->In2 - (char *)&object->In1;
				/* Offset to next input */
        int     i;              /* Loopindex */
        float   sum;            /* Result */
/* Initialize */
        sum = object->Const;
        ptr = &object->In1P;
/* SUM loop */
        for ( i = 0 ; i < sumsize ; i++ )
        {
          if ( *ptr != NULL )
            sum += **ptr * object->FVect[i];

          charp = (char *) ptr;
          charp += offset;
          ptr = (float **)charp;
        }
/* Result */
        object->ActVal = sum;
}

/*_*
  MAXMIN
  funktion:       Select largest and smallest value of up to
                  8 input values.

  @aref maxmin MaxMin
*/

void maxmin_exec(
  plc_sThread		*tp,
  pwr_sClass_maxmin       *object)
{
#define maxminsize      8
#define maxmin_maxfloat        1.E37
#define maxmin_minfloat        -1.E37

        float   minval;            /* Lowest value */
        float   maxval;            /* Highest value */
        int     i;              /* Loopcounter */
	static ptrdiff_t offset = (char *)&object->In2 - (char *)&object->In1; /* Offset to next input */
        char    *charp;
        float   **ptr;          /* Pointer to ptr to digin */

/* Initialize */
        ptr = &object->In1P;
        minval = maxmin_maxfloat;
        maxval = maxmin_minfloat;
/* MaxMin loop */
        for ( i = 0 ; i < maxminsize ; i++)
        {
          if ( *ptr != NULL )
          {
	    if ( **ptr > maxval) maxval = **ptr;
	    if ( **ptr < minval) minval = **ptr;
          }
          charp = (char *) ptr;
          charp += offset;
          ptr = (float **)charp;
        }
/* Set Output */
        object->MaxVal = maxval;
        object->MinVal = minval;
}

/*_*
  LIMIT
  funktion:	Upper and lower limit of analog value

  @aref limin Limit
*/
void limit_exec(
  plc_sThread		*tp,
  pwr_sClass_limit 	*object)
{
	object->Max = *object->MaxP;
	object->Min = *object->MinP;
	object->In = *object->InP;

  	if (object->In > object->Max)
	{
	  object->ActVal = object->Max;
	  object->High = TRUE;
	  object->Low = FALSE;
	}
  	else if (object->In < object->Min)
	{
	  object->Low = TRUE;
	  if (object->Min <= object->Max)
	  {
	    object->ActVal = object->Min;
	    object->High = FALSE;
	  }
	  else
	  {
	    object->ActVal = object->Max;
	    object->High = TRUE;
	  }
	}
  	else
	{
	  object->ActVal = object->In;
	  object->High = FALSE;
	  object->Low = FALSE;
	}
}

/*_*
  COMPH
  funktion:	Compare Analog Value with high limit

  @aref comph Comph
*/
void comph_exec(
  plc_sThread		*tp,
  pwr_sClass_comph 	*object)
{
	object->In = *object->InP;
	object->Lim = *object->LimP;

  	if (object->High)
	{
	  if ( object->In <= (object->Lim - object->Hysteres))
  	    object->High = FALSE;
	}
	else
	  if (object->In > object->Lim ) object->High = TRUE;
}

/*_*
  COMPL
  funktion:	Compare Analog Value with low limit

  @aref compl Compl
*/
void compl_exec(
  plc_sThread		*tp,
  pwr_sClass_compl 	*object)
{
	object->In = *object->InP;
	object->Lim = *object->LimP;

  	if (object->Low)
	{
	  if ( object->In >= (object->Lim + object->Hysteres))
  	    object->Low = FALSE;
	}
	else
	  if (object->In < object->Lim ) object->Low = TRUE;
}

/*_*
  SELECT
  funktion:	Select between two analog values depending on
		one logical control value

  @aref select Select
*/
void select_exec(
  plc_sThread		*tp,
  pwr_sClass_select 	*object)
{
	object->Control = *object->ControlP;

	if (object->Control)
	{
  	  object->ActVal = *object->HighP;
  	  object->NotActVal = *object->LowP;
  	}
  	else
	{
  	  object->ActVal = *object->LowP;
  	  object->NotActVal = *object->HighP;
  	}
}

/*_*
  RAMP
  funktion:	Limit output change.
 		If external feedback is used we limit change
		from this signal.

  @aref ramp Ramp
*/

void ramp_init(
  pwr_sClass_ramp 	*object)
{

	object->In = *object->InP;
	object->ActVal = object->In;
}

/* New ramp_exec with RampUp used when absolute value is increasing */
/*  and RampDown used when absolute value is decreasing */
/* New functionality will be used when new Boolean parameter 'RampUpAbs' */
/*  is on. For Proview 3.0 we will use 'AccUp' not zero for new functionality */
/* If this parameter is zero the routine will workin the old way with */
/*  RampUp for increasing and RampDown for decreasing */
/* 2001-03-13 Hans Werner */

void ramp_exec(
  plc_sThread		*tp,
  pwr_sClass_ramp 	*object)
{
	float limit1;		/* First limit */
	float limit2;		/* Second limit at sign change */
	float old;		/* Actual start value */
	float out;		/* New output */
	float scan;		/* scantime in seconds */

	/* Assume new output as unlimited as a start */
	out = object->In = *object->InP;	/* Get aimed value */
	old = *object->FeedBP;			/* Startvalue */
	scan = *object->ScanTime;		/* Get scantime */

	if (out > old)			/* Increase */
	{
	  if (old >= 0.0)		/* Positive rising even more */
	  {
	    limit1 = scan * object->RampUp;
	    if ((limit1 > 0.0) && (out > old + limit1)) out = old + limit1;
	  }
	  else if (out <= 0.0)		/* Negative rising towards zero */
	  {
	    if (object->RampUpAbs) limit1 = scan * object->RampDown;
	    else limit1 = scan * object->RampUp;
	    if ((limit1 > 0.0) && (out > old + limit1)) out = old + limit1;
	  }
	  else				/* From Neg to Pos */
	  {
	    if (object->RampUpAbs)
	    {
	      limit1 = scan * object->RampDown;
	      limit2 = scan * object->RampUp;
	    }
	    else limit1 = limit2 = scan * object->RampUp;

	    if (limit1 <= 0.0)	/* No limit up to zero */
	    {
	      if ((limit2 > 0.0) && (out > limit2)) out = limit2;
	    }
	    else if (old <= -limit1)	/* Will still not reach zero */
	    {
	      if (out > old + limit1) out = old + limit1;
	    }
	    else if (limit2 > 0)	/* Use second limitation above zero */
	    {
	      if (old < -limit2) out = 0.0;
	      else if (out > old + limit2) out = old + limit2;
	    }
	  } /* End Neg to Pos */
	} /* End Increasing */
	else if (out < old)			/* Decrease */
	{
	  if (old <= 0.0)		/* Negative falling even more */
	  {
	    if (object->RampUpAbs) limit1 = scan * object->RampUp;
	    else limit1 = scan * object->RampDown;
	    if ((limit1 > 0.0) && (out < old - limit1)) out = old - limit1;
	  }
	  else if (out >= 0.0)		/* Positive falling towards zero */
	  {
	    limit1 = scan * object->RampDown;
	    if ((limit1 > 0.0) && (out < old - limit1)) out = old - limit1;
	  }
	  else				/* From Pos to Neg */
	  {
	    if (object->RampUpAbs)
	    {
	      limit1 = scan * object->RampDown;
	      limit2 = scan * object->RampUp;
	    }
	    else limit1 = limit2 = scan * object->RampDown;

	    if (limit1 <= 0.0)	/* No limit down to zero */
	    {
	      if ((limit2 > 0.0) && (out < -limit2)) out = -limit2;
	    }
	    else if (old >= limit1)	/* Will still not reach zero */
	    {
	      if (out < old - limit1) out = old - limit1;
	    }
	    else if (limit2 > 0)	/* Use second limitation below zero */
	    {
	      if (old > limit2) out = 0.0;
	      else if (out < old - limit2) out = old - limit2;
	    }
	  } /* End Neg to Pos */
	} /* End Decreasing */

	object->ActVal = out;	/* New output */
}

/*_*
  FILTER
  funktion:	Exponential filter.
		If external feedback is used we filter
		from this signal.
	out = old + (in-old)* Ts / Tf

  @aref filter Filter
*/
void filter_init(object)
pwr_sClass_filter 	*object;
{
	object->In = *object->InP;
	object->ActVal = object->In;
}
void filter_exec(
  plc_sThread		*tp,
  pwr_sClass_filter 	*object)
{
	object->In = *object->InP;
	if (object->FiltCon > *object->ScanTime)
	  object->ActVal = *object->FeedBP +
	    (object->In - *object->FeedBP) * *object->ScanTime / object->FiltCon;
	else
	  object->ActVal = object->In;
}

/*_*
  SPEED
  Function:	Calculate difference divided to scantime

  @aref speed Speed
*/
void speed_exec(
  plc_sThread		*tp,
  pwr_sClass_speed 	*object)
{
	float	old;

	old = object->In;
	object->In = *object->InP;
	object->ActVal = (object->In - old) / *object->ScanTime * object->TimFact;
}

/*_*
  TIMINT
  Function:       Integration in time

  @aref timint Timint
*/
void timint_exec(
  plc_sThread		*tp,
  pwr_sClass_timint       *object)
{
/* Clear ? */
        if (*object->ClearP && !object->Clear)
        {
          object->OldAcc = object->ActVal;
          object->ActVal = 0;
        }
        object->Clear = *object->ClearP;
/* Ackumulate new value */
        object->ActVal += *object->InP * *object->ScanTime / object->TimFact;
}

/*_*
  CURVE
  Funktion:	Interpollation in a table

  @aref curve Curve
*/
void curve_exec(
  plc_sThread		*tp,
  pwr_sClass_curve		*object)
{
	float	x0;
	float	x1;
	float	y0;
	float	y1;
	float	number;
	float	*tabpointer;
/* Get input */
	object->In = *object->InP;
	if (object->TabP == NULL) object->ActVal = object->In;
	else
/* Get pointer to table, and number of pairs in table */
	{
	  tabpointer = object->TabP;
	  number = *tabpointer++;
	  if (number <= 0) object->ActVal = object->In;
	  else
/* Search in table */
	  {
	    x1 = *tabpointer++;
	    y1 = *tabpointer++;
	    if ( object->In <= x1 ) object->ActVal = y1;
	    else
	    {
	      for ( ; (number > 1) && ( object->In > x1) ; number--)
	      {
	        x0 = x1;
	        x1 = *tabpointer++;
	        y0 = y1;
	        y1 = *tabpointer++;
	      }
	      if ( object->In > x1) object->ActVal = y1; /* End of table */
	      else  object->ActVal = y0 + (y1 - y0) *
		  (object->In - x0) / (x1 - x0); /* Interpollation */
	    }
	  }
	}
}

#if 0

	TABLE
	Funktion:	Table for interpollation
#define table_exec(tableobject)  /* Table. No code for execution */
#endif

/*_*
	ADELAY

	Funktion:	
	This routine performes the function of delay for an analog or any
	float value. Values are stored in the structure for the object.
	There is  a small difference in operation depending on the maximum
	delay time (MaxTime). If MaxTime <= vektorsize*scantime, 
	all numbers are stored in the vektor.
	Otherwise if MaxTime is greater then vektorsize*scantime 
	MaxTime/vektorsize*scantime=Count numbers are via a filter algorithm
	used to give the next value that is stored in the vektor.
	The value for MaxTime is set by means of the editor. The vektorsize is
	set in the stucture.
	
	Structure data:
     	In		Analog input data
     	Tim		Actual delay time
     	MaxCount	Number of cycles per step
     	StoredNumbers	Number of stored values, set to 0 at init
     	StoInd		Index to last stored value
     	Count		Actual number of loop since last shift
     	Values		Vektor with saved values
     	ActualValue	Result from the routine 

  @aref adelay Adelay
*/

void adelay_init(object)
pwr_sClass_adelay		*object;
{
	object->StoredNumbers = 0;
	object->StoInd = -1;
	object->Count = object->MaxCount;
	object->ActVal = *object->InP;
}
void adelay_exec(
  plc_sThread		*tp,
  pwr_sClass_adelay		*object)
{

/* Local variables */
	int	maxindex;
	int	actindex;
	int	actoff;

	maxindex = sizeof(object->TimVect)/4;

/*		Get input
*/
	object->In = *object->InP;
	object->Tim = *object->TimP;

/*		Calculate average in each storeplace.
		MaxCount is number of cycles before each shift
*/
	object->Count++;
	if (object->Count >= object->MaxCount )
	{
	  object->StoInd++;
	  if ((object->StoInd >=  maxindex) || (object->StoInd < 0))
	 	object->StoInd = 0;
	  if (object->StoredNumbers < maxindex) 
		object->StoredNumbers++;
	  object->Count = 0;
	  object->TimVect[object->StoInd] = object->In;
	}
	else
	  object->TimVect[object->StoInd] = (object->TimVect[object->StoInd] *
		object->Count + object->In) / (object->Count + 1);

/*		Calculate position for output
*/
	actoff = (object->Tim / tp->f_scan_time - object->Count)
		/ object->MaxCount;
	if (actoff >= object->StoredNumbers)
		actoff = object->StoredNumbers - 1;
	actindex = object->StoInd - actoff;
	if (actindex < 0) actindex += maxindex;

	object->ActVal=object->TimVect[actindex];
}	

/*_*
  PISPEED
  function:       Measure flow with pulse input

  @aref pispeed PiSpeed
*/

void pispeed_init(object)
pwr_sClass_pispeed              *object;
{
/* Read input */
        object->PulsIn = *object->PulsInP;
}

void pispeed_exec(
  plc_sThread		*tp,
  pwr_sClass_pispeed              *object)
{
  int   piold;          /* Old input */

/* Read input */
        piold = object->PulsIn;
        object->PulsIn = *object->PulsInP;
/* Calculate flow */
        object->ActVal = (object->PulsIn - piold) * object->Gain *
                object->TimFact / *object->ScanTime;
}

/*_*
  DtoMask
  funktion:	Assamble digital signals to integer bitmask

  @aref dtomask DtoMask
*/
void DtoMask_exec(
  plc_sThread		*tp,
  pwr_sClass_DtoMask 	*object)
{
  int i;
  pwr_tBoolean *d, **dp;
  pwr_tInt32 val = 0;
  pwr_tInt32 m = 1;

  d = &object->d1;
  dp = &object->d1P;
  for ( i = 0; i < 32; i++) {
    *d = **dp;
    if ( *d)
      val |= m;
    d += 2;
    dp += 2;
    m = m << 1;
  }
  object->Mask = val;
}

/*_*
  MaskToD
  funktion:	Deassamble integer bitmask to digital signals

  @aref masktod MaskToD
*/
void MaskToD_exec(
  plc_sThread		*tp,
  pwr_sClass_MaskToD 	*object)
{
  int i;
  pwr_tInt32 m = 1;
  pwr_tBoolean *d;

  d = &object->od1;
  object->Mask = *object->MaskP;
  for ( i = 0; i < 32; i++) {
    if ( object->Mask & m)
      *d = TRUE;
    else
      *d = FALSE;
    d++;
    m = m << 1;
  }
}

/*_*
  DtoEnum
  funktion:	Select enumeration value from digital inputs

  @aref dtoenum DtoEnum
*/
void DtoEnum_exec(
  plc_sThread		*tp,
  pwr_sClass_DtoEnum 	*object)
{
  int i;
  pwr_tBoolean *d, **dp;
  pwr_tInt32 val = object->DefaultValue;

  d = &object->d0;
  dp = &object->d0P;
  for ( i = 0; i < 32; i++) {
    *d = **dp;
    if ( *d) {
      val = object->EnumValues[i];
      break;
    }
    d += 2;
    dp += 2;
  }
  object->Enum = val;
}

/*_*
  EnumToD
  funktion: Identify enumeration value.

  @aref enumtod EnumToD
*/
void EnumToD_exec(
  plc_sThread		*tp,
  pwr_sClass_EnumToD 	*object)
{
  int i;
  pwr_tBoolean *d;

  d = &object->od0;
  object->Enum = *object->EnumP;
  for ( i = 0; i < 32; i++) {
    if ( object->Enum == object->EnumValues[i])
      *d = TRUE;
    else
      *d = FALSE;
    d++;
  }
}

/*_*
  @aref fmod Mod
*/
void Mod_exec(
  plc_sThread		*tp,
  pwr_sClass_Mod 	*o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->ActVal = fmodf( o->In1, o->In2);
}

/*_*
  @aref equal Equal
*/
void Equal_exec(
  plc_sThread		*tp,
  pwr_sClass_Equal 	*o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (fabsf(o->In1 - o->In2) < FLT_EPSILON);
}

/*_*
  @aref notequal NotEqual
*/
void NotEqual_exec(
  plc_sThread		*tp,
  pwr_sClass_NotEqual 	*o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = !(fabsf(o->In1 - o->In2) < FLT_EPSILON);
}

/*_*
  @aref greaterequal GreaterEqual
*/
void GreaterEqual_exec(
  plc_sThread		*tp,
  pwr_sClass_GreaterEqual *o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 >= o->In2) || (fabsf(o->In1 - o->In2) < FLT_EPSILON);
}

/*_*
  @aref greaterthan GreaterThan
*/
void GreaterThan_exec(
  plc_sThread		*tp,
  pwr_sClass_GreaterThan *o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 > o->In2);
}

/*_*
  @aref lessequal LessEqual
*/
void LessEqual_exec(
  plc_sThread		*tp,
  pwr_sClass_LessEqual  *o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 <= o->In2) || (fabsf(o->In1 - o->In2) < FLT_EPSILON);
}

/*_*
  @aref lessthan LessThan
*/
void LessThan_exec(
  plc_sThread		*tp,
  pwr_sClass_LessThan   *o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 < o->In2);
}

/*_*
  @aref iequal IEqual
*/
void IEqual_exec(
  plc_sThread		*tp,
  pwr_sClass_IEqual 	*o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 == o->In2);
}

/*_*
  @aref iequal INotEqual
*/
void INotEqual_exec(
  plc_sThread		*tp,
  pwr_sClass_INotEqual 	*o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 != o->In2);
}

/*_*
  @aref igreaterequal IGreaterEqual
*/
void IGreaterEqual_exec(
  plc_sThread		*tp,
  pwr_sClass_IGreaterEqual *o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 >= o->In2);
}

/*_*
  @aref igreaterthan IGreaterThan
*/
void IGreaterThan_exec(
  plc_sThread		*tp,
  pwr_sClass_IGreaterThan *o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 > o->In2);
}

/*_*
  @aref ilessequal ILessEqual
*/
void ILessEqual_exec(
  plc_sThread		*tp,
  pwr_sClass_ILessEqual  *o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 <= o->In2);
}

/*_*
  @aref ilessthan ILessThan
*/
void ILessThan_exec(
  plc_sThread		*tp,
  pwr_sClass_ILessThan   *o)
{
  o->In1 = *o->In1P;
  o->In2 = *o->In2P;
  o->Status = (o->In1 < o->In2);
}


/*_*
  IAdd Integer addition.
  @aref iadd IAdd
*/
void IAdd_exec(
  plc_sThread		*tp,
  pwr_sClass_IAdd	*o)
{
#define IADD_SIZE 8
  static ptrdiff_t offset = (char *)&o->In2 - (char *)&o->In1;
  int     	i;
  pwr_tInt32	**inp = &o->In1P;
  pwr_tInt32   	sum = 0;

  for ( i = 0; i < IADD_SIZE; i++) {
    sum += **inp;
    
    inp = (pwr_tInt32 **)((char *)inp + offset);
  }
  o->ActVal = sum;
}

/*_*
  IMul Integer multiplication.
  @aref imul IMul
*/
void IMul_exec(
  plc_sThread		*tp,
  pwr_sClass_IMul	*o)
{
#define IMUL_SIZE 8
  static ptrdiff_t offset = (char *)&o->In2 - (char *)&o->In1;
  int     	i;
  pwr_tInt32	**inp = &o->In1P;
  pwr_tInt32   	result = **inp;

  for ( i = 1; i < IMUL_SIZE; i++) {
    inp = (pwr_tInt32 **)((char *)inp + offset);
    result *= **inp;    
  }
  o->ActVal = result;
}

/*_*
  ISub Integer subtraction.
  @aref isub ISub
*/
void ISub_exec(
  plc_sThread		*tp,
  pwr_sClass_ISub	*o)
{
  o->ActVal = *o->In1P - *o->In2P;
}

/*_*
  IDiv Integer division.
  @aref idiv IDiv
*/
void IDiv_exec(
  plc_sThread		*tp,
  pwr_sClass_IDiv	*o)
{
  if ( *o->In2P == 0)
    o->ActVal = 0;
  else
    o->ActVal = *o->In1P / *o->In2P;
}

/*_*
  IMax Maximum function.
  @aref imax IMax
*/
void IMax_exec(
  plc_sThread		*tp,
  pwr_sClass_IMax	*o)
{
#define IMAX_SIZE 8
  static ptrdiff_t offset = (char *)&o->In2 - (char *)&o->In1;
  int     	i;
  pwr_tInt32	**inp = &o->In1P;
  pwr_tInt32   	result = INT_MIN;

  for ( i = 0; i < IMAX_SIZE; i++) {
    if ( **inp > result)
      result = **inp;    
    inp = (pwr_tInt32 **)((char *)inp + offset);
  }
  o->ActVal = result;
}

/*_*
  IMin Minimum function.
  @aref imin IMin
*/
void IMin_exec(
  plc_sThread		*tp,
  pwr_sClass_IMin	*o)
{
#define IMIN_SIZE 8
  static ptrdiff_t offset = (char *)&o->In2 - (char *)&o->In1;
  int     	i;
  pwr_tInt32	**inp = &o->In1P;
  pwr_tInt32   	result = INT_MAX;

  for ( i = 0; i < IMIN_SIZE; i++) {
    if ( **inp < result)
      result = **inp;
    inp = (pwr_tInt32 **)((char *)inp + offset);
  }
  o->ActVal = result;
}

/*_*
  ISel Select function.
  @aref isel ISel
*/
void ISel_exec(
  plc_sThread		*tp,
  pwr_sClass_ISel	*o)
{
  o->Control = *o->ControlP;
  if ( o->Control)
    o->ActVal = *o->In1P;
  else
    o->ActVal = *o->In2P;
}

/*_*
  ILimit Integer limiter.
  @aref ilimit ILimit
*/
void ILimit_exec(
  plc_sThread		*tp,
  pwr_sClass_ILimit	*o)
{
  o->Max = *o->MaxP;
  o->Min = *o->MinP;
  o->In = *o->InP;

  if (o->In > o->Max) {
    o->ActVal = o->Max;
    o->High = TRUE;
    o->Low = FALSE;
  }
  else if (o->In < o->Min) {
    o->Low = TRUE;
    if ( o->Min <= o->Max) {
      o->ActVal = o->Min;
      o->High = FALSE;
    }
    else {
      o->ActVal = o->Max;
      o->High = TRUE;
    }
  }
  else {
    o->ActVal = o->In;
    o->High = FALSE;
    o->Low = FALSE;
  }
}

/*_*
  IMux Integer multiplexer.
  @aref imux IMux
*/
void IMux_exec(
  plc_sThread		*tp,
  pwr_sClass_IMux	*o)
{
#define IMUX_SIZE 24
  static ptrdiff_t offset = (char *)&o->In1 - (char *)&o->In0;
  int     	idx;
  pwr_tInt32	**inp = &o->In0P;

  idx = o->Index = *o->IndexP;
  idx = idx < 0 ? 0 : ( idx > IMUX_SIZE - 1 ? IMUX_SIZE - 1 : idx);
  inp = (pwr_tInt32 **)((char *)inp + idx * offset);
  o->ActVal = **inp;
}

/*_*
  Mux Analog multiplexer.
  @aref mux Mux
*/
void Mux_exec(
  plc_sThread		*tp,
  pwr_sClass_Mux	*o)
{
#define MUX_SIZE 24
  static ptrdiff_t offset = (char *)&o->In1 - (char *)&o->In0;
  int     	idx;
  pwr_tFloat32	**inp = &o->In0P;

  idx = o->Index = *o->IndexP;
  idx = idx < 0 ? 0 : ( idx > MUX_SIZE - 1 ? MUX_SIZE - 1 : idx);
  inp = (pwr_tFloat32 **)((char *)inp + idx * offset);
  o->ActVal = **inp;
}

/*_*
  Demux Analog demultiplexer.
  @aref demux Demux
*/
void Demux_exec(
  plc_sThread		*tp,
  pwr_sClass_Demux	*o)
{
#define DEMUX_SIZE 24
  int     	idx, i;
  pwr_tFloat32	*outp = &o->Out0;

  idx = o->Index = *o->IndexP;
  for ( i = 0; i < DEMUX_SIZE; i++) {
    if ( i == idx)
      *outp = *o->InP;
    else
      *outp = 0;
    outp++;
  }
}

/*_*
  IDemux Integer demultiplexer.
  @aref idemux IDemux
*/
void IDemux_exec(
  plc_sThread		*tp,
  pwr_sClass_IDemux	*o)
{
#define IDEMUX_SIZE 24
  int     	idx, i;
  pwr_tInt32	*outp = &o->Out0;

  idx = o->Index = *o->IndexP;
  for ( i = 0; i < IDEMUX_SIZE; i++) {
    if ( i == idx)
      *outp = *o->InP;
    else
      *outp = 0;
    outp++;
  }
}

/*_*
  Add Analog addition.
  @aref add Add
*/
void Add_exec(
  plc_sThread		*tp,
  pwr_sClass_Add	*o)
{
#define ADD_SIZE 8
  static ptrdiff_t offset = (char *)&o->In2 - (char *)&o->In1;
  int     	i;
  pwr_tFloat32	**inp = &o->In1P;
  pwr_tFloat32  sum = 0;

  for ( i = 0; i < ADD_SIZE; i++) {
    sum += **inp;
    
    inp = (pwr_tFloat32 **)((char *)inp + offset);
  }
  o->ActVal = sum;
}

/*_*
  Mul Analog multiplication.
  @aref mul Mul
*/
void Mul_exec(
  plc_sThread		*tp,
  pwr_sClass_Mul	*o)
{
#define MUL_SIZE 8
  static ptrdiff_t offset = (char *)&o->In2 - (char *)&o->In1;
  int     	i;
  pwr_tFloat32	**inp = &o->In1P;
  pwr_tFloat32  result = **inp;

  for ( i = 1; i < MUL_SIZE; i++) {
    inp = (pwr_tFloat32 **)((char *)inp + offset);
    result *= **inp;
  }
  o->ActVal = result;
}

/*_*
  Sub Analog subtraction.
  @aref sub Sub
*/
void Sub_exec(
  plc_sThread		*tp,
  pwr_sClass_Sub	*o)
{
  o->ActVal = *o->In1P - *o->In2P;
}

/*_*
  Div Analog division.
  @aref div Div
*/
void Div_exec(
  plc_sThread		*tp,
  pwr_sClass_Div	*o)
{
  o->ActVal = *o->In1P / *o->In2P;
}

/*_*
  Max Maximum function.
  @aref max Max
*/
void Max_exec(
  plc_sThread		*tp,
  pwr_sClass_Max	*o)
{
#define AMAX_SIZE 8
  static ptrdiff_t offset = (char *)&o->In2 - (char *)&o->In1;
  int     	i;
  pwr_tFloat32	**inp = &o->In1P;
  pwr_tFloat32  result = -1E37;

  for ( i = 0; i < AMAX_SIZE; i++) {
    if ( **inp > result)
      result = **inp;    
    inp = (pwr_tFloat32 **)((char *)inp + offset);
  }
  o->ActVal = result;
}

/*_*
  Min Minimum function.
  @aref min Min
*/
void Min_exec(
  plc_sThread		*tp,
  pwr_sClass_Min	*o)
{
#define AMIN_SIZE 8
  static ptrdiff_t offset = (char *)&o->In2 - (char *)&o->In1;
  int     	i;
  pwr_tFloat32	**inp = &o->In1P;
  pwr_tFloat32  result = 1E37;

  for ( i = 0; i < AMIN_SIZE; i++) {
    if ( **inp < result)
      result = **inp;
    inp = (pwr_tFloat32 **)((char *)inp + offset);
  }
  o->ActVal = result;
}

/*_*
  BwShiftLeft Bitwise shift left.
  @aref bwshiftleft BwShiftLeft
*/
void BwShiftLeft_exec(
  plc_sThread			*tp,
  pwr_sClass_BwShiftLeft	*o)
{
  o->Out = ((unsigned int)*o->InP) << (*o->NumP);
}

/*_*
  BwShiftRight Bitwise shift right.
  @aref bwshiftright BwShiftRight
*/
void BwShiftRight_exec(
  plc_sThread			*tp,
  pwr_sClass_BwShiftRight	*o)
{
  o->Out = ((unsigned int)*o->InP) >> (*o->NumP);
}

/*_*
  BwRotateRight Bitwise rotate right.
  @aref bwrotateright BwRotateRight
*/
void BwRotateRight_exec(
  plc_sThread			*tp,
  pwr_sClass_BwRotateRight	*o)
{
  o->Out = ((unsigned int)(*o->InP) << (32 - *o->NumP)) | 
	((unsigned int)(*o->InP) >> (*o->NumP));
}

/*_*
  BwRotateLeft Bitwise rotate left.
  @aref bwrotateleft BwRotateLeft
*/
void BwRotateLeft_exec(
  plc_sThread			*tp,
  pwr_sClass_BwRotateLeft	*o)
{
  o->Out = ((unsigned int)(*o->InP) >> (32 - *o->NumP)) | 
	((unsigned int)(*o->InP) << (*o->NumP));
}






