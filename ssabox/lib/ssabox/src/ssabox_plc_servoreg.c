/*********************************************************************
**********************************************************************
*
*       S S A B  O X E L � S U N D
*       = = = =  = = = = = = = = =
**********************************************************************
*
* Filename      : $pwrp_src:ra_plc_user.c
*
* Description   : Contains main functions for anti-sway control,
*               : called from plc
*                 
*               Date    Sign           Action
* Revision      050213  jonas_h        First edition
*               050721  jonas_h        Improved error handling: Errors are written to an internal buffer instead of being printed with printf(..). From the buffer, errors are stored in a String80 output, that can be connected e.g. to a DSup.
*                                      Extended functionality for automatic object: The soft maximum velocity, umaxS, can be changed while in travel (if e.g. in a low-speed area).
*                                      Transfer of set to manual object when disabling, allowing for continued anti-sway, and smooth travel.
*
*
**********************************************************************
**********************************************************************/

#include "ssabox_plc_servoreg.h"

/*_*
  @aref ssab_servoreg Ssab_ServoReg
*/
void Ssab_ServoReg_exec(plc_sThread *tp, pwr_sClass_Ssab_ServoReg *object)
{
  double aD, uD, xD, dt, control, uRamp, xDiff;
  int delaysteps;

  dt = tp->PlcThread->ScanTime; /* Constant scan time (the ideal value, not the measurement of last cycle time) */
  object->enable = *object->enableP;
 
 if (!object->enable) { //bypass the reference and reset active flags and elapsed time
    object->uReg = *object->uRP;
    object->RampActive = FALSE;
    object->DZActive = FALSE;
    object->TDZElapsedTime = 0.0;
    object->TDZActive = FALSE;
    return;
  }


  /* 1. Retrieve input */

  object->aR = *object->aRP;
  object->uR = *object->uRP;
  control = object->uR;
  object->xR = *object->xRP;
  object->ac = *object->acP;
  object->uc = *object->ucP;
  object->xc = *object->xcP;
  object->xCommand = *object->xCommandP;
  xDiff = object->xCommand - object->xc;
  object->umaxP = *object->umaxPP;
  object->umaxN = *object->umaxNP;
  object->positioning = *object->positioningP;
  if (object->positioning && object->enableRamp) //uR not used - use the regulator for positioning
    control = (xDiff > 0.0 ? object->umaxP : object->umaxN);

  /* 2. PID regulator */

  // Should change some stuff here: Relative gain kP, Integration time Ti, Derivation time Td. 

  if (object->enablePID && !object->positioning) {
    SR_addNewRef(SR_OBJ_LISTPP, object->aR, object->uR, object->xR);
    delaysteps = AS_ROUND(object->DelayPID/dt);   
    if (delaysteps < 0)
      delaysteps = 0;
    if (delaysteps > object->maxdelaysteps)
      delaysteps = object->maxdelaysteps;
    SR_extractRef(SR_OBJ_LISTPP, delaysteps, &aD, &uD, &xD);
    control = uD + object->kPID[0]*(uD - object->uc) + object->kPID[1]*(xD - object->xc) + object->kPID[2]*(aD - object->ac);
  }


  /* 3. Square root ramp */
  //This will give erroneous behavior if the command position is changed to a position close to the current in the midst of a travel.
  //Possibly solve this problem by taking into account further conditions. 

  if (object->enableRamp) {
    uRamp = AS_SIGN(xDiff) * ( -object->DelayRamp*object->amaxS + 
	   sqrt(object->DelayRamp*object->DelayRamp*object->amaxS*object->amaxS + 2.0*fabs(xDiff)*object->amaxS) );
    if ( (xDiff >= 0.0 && control > uRamp) || (xDiff < 0.0 && control < uRamp) ) {
      control = uRamp;
      object->RampActive = TRUE;
    }
    else
      object->RampActive = FALSE;
  }
  else
    object->RampActive = FALSE;
   

  /* 4. Dead zone */
  
  if (object->enableDZ && (object->uR == 0.0) && (fabs(xDiff) < object->DeadZone)) {
    control = 0.0;
    object->DZActive = TRUE;
  }
  else 
    object->DZActive = FALSE;



  /* 5. Timer dead zone */

  if (object->enableTDZ && (object->uR == 0.0) && (fabs(xDiff) < object->TimerDeadZone)) {
    object->TDZElapsedTime += *(object->ScanTime);
    if (object->TDZElapsedTime > object->TDZTime) {
      control = 0.0;
      object->TDZActive = TRUE;
      object->TDZElapsedTime = object->TDZTime;
    }
    else
      object->TDZActive = FALSE;
  }
  else {
    object->TDZElapsedTime = 0.0;
    object->TDZActive = FALSE;
  }


  /* 6. Check bounds */
  
  if (control > object->umaxP)
    control = object->umaxP;
  if (control < object->umaxN)
    control = object->umaxN;

  
  /* 8. Set output */

  object->uReg = control;
  
} 
/* End of ServoReg */


