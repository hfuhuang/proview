/*********************************************************************
**********************************************************************
*
*       S S A B  O X E L � S U N D
*       = = = =  = = = = = = = = =
**********************************************************************
*
* Filename      : 
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

#include "ssabox_plc_antisway.h"


/*_*
  @aref ssab_antisway Ssab_AntiSway
*/
void Ssab_AntiSway_init(object)
     pwr_sClass_Ssab_AntiSway *object;
{
  int i;
  AS_OBJ_SETP->N = 0;
  AS_OBJ_SETP->extSum = 0.0;
  AS_OBJ_SETP->ph = NULL;
  object->other=NULL;
  for (i=0; i<2; i++) {
    object->messageQ[(i)]  =  NULL;
//    AS_OBJ_MESSAGEQP(i) =  NULL;
    (object->message[i])[79]='\0';
  }
}


void Ssab_AntiSway_exec(plc_sThread *tp, pwr_sClass_Ssab_AntiSway *object)
{
  int hoisting = FALSE;
  int newCall = FALSE;
  int hoisted = FALSE;
  int i;
  char *mstr;
  double omega, dt;
  const AS_shaper *shp;

  dt = tp->PlcThread->ScanTime; /* Constant scan time (the ideal value, not the measuremen of last cycle time) */
  object->done = TRUE;


  /***********************************************
   *  Step 1. Check current error status. 
   *          Reset if enable has been switched.
   ***********************************************/

  if (object->errstatus > AS_ERR_MINOR) { /* crucial error has occured since last reset. */

    if (!(object->errstatus & AS_ERR_DISABLED)) { /* Not already disabled */
      object->errstatus |= AS_ERR_DISABLED; /* set disabled flag */
      if (object->enable) /* the object was enabled when the crucial error occured */
 	object->errstatus |= AS_ERR_WASENABLED; /* set wasenabled flag */
      if (object->verbose >= AS_VERB_DISABLED)/* Print disabled message if verbose */
	printf("\nAntiSway object disabled. Error status=%d\n",object->errstatus);
      AS_ADDMESSAGE(AS_ALARMTYPE_DISABLED, &mstr,"AntiSway object stopped. Error status=%d",object->errstatus);
    }
    else if (object->errstatus & AS_ERR_WASENABLED) {/* already disabled, from enable mode. Reset errstatus if enable is switched off */
      if (!object->enable) 
	object->errstatus=0;
    }
    else if (object->enable)/* already disabled, from not enable mode. Reset errstatus if enable is switched on */
      object->errstatus=0;

    newCall = TRUE;  //why? needed if errstatus is reset. in which case this will be an ordinary execution cycle. 
  }

   

  /************************************************
   *  Step 2. Receive input and check its validity
   ************************************************/
 
  /* 2.1 Determine if new call and get new input */

  object->enable = *object->enableP; 
  object->xc = *object->xcP;
  object->DLc = *object->DLcP;
  object->mode = *object->modeP;
  object->manual = *object->manualP;


  /* Get a pointer to the shaper corresponding to the current mode. */
  shp = AS_SetupIST(object->mode); 
  if (shp == NULL) { /* Non-valid mode - disable! */
    if (!(object->errstatus & AS_ERR_MODE)) { /* Error flag not already set. */
      object->errstatus |= AS_ERR_MODE; /* Set error flag */
      if (object->verbose >= AS_VERB_DISABLED) /* Print error message if verbose */
	printf("\nAntiSway: No shaper for current mode! Mode=%d\n",object->mode);
      AS_ADDMESSAGE(AS_ALARMTYPE_DISABLED, &mstr,"AS: No shaper for current mode! Mode=%d",object->mode);  
    }
  }


  if ( object->manual && (object->autostatus & AS_AUTO_ON) ) { //switched from auto to manual
    AS_collapseSet(AS_OBJ_SETP, shp, AS_OBJ_AMAXSM);
    object->autostatus &= (~AS_AUTO_ON);
    newCall = TRUE;
  }
  else if ( !object->manual && !((object->autostatus & AS_AUTO_ON)) ) { //switched from manual to auto. 
    AS_collapseSet(AS_OBJ_SETP, shp, AS_OBJ_AMAXSA);
    object->autostatus |= AS_AUTO_ON;
    newCall = TRUE;
  }

  
  /* Fetch command velocity and command position and check its validity. This is done independently of mode */
  if (object->uCommand != *object->uCommandP) {
    object->uCommand = *object->uCommandP;
    /* Check command velocity */
    if (fabs(object->uCommand) > AS_OBJ_UMAXSM) { /* Non-valid command velocity - disable! */
      if (!(object->errstatus & AS_ERR_UCOM)) { /* Error flag not already set. */
	object->errstatus |= AS_ERR_UCOM; /* Set error flag */
	if (object->verbose >= AS_VERB_DISABLED) /* Print error message if verbose */
	  printf("\nAntiSway: command velocity greater than soft limit, uCommand=%f, umaxSM=%f\n",object->uCommand,AS_OBJ_UMAXSM);
	AS_ADDMESSAGE(AS_ALARMTYPE_DISABLED, &mstr,"AS: Command velocity greater than soft limit, uCommand=%.3f umaxSM=%.3f",object->uCommand,AS_OBJ_UMAXSM);
      }
    }
    else if (fabs(object->uCommand) < AS_OBJ_UR_MIN)
      object->uCommand = 0.0;
    if (object->manual)
      newCall = TRUE;
  }
  
  if (object->xCommand != *object->xCommandP) {
    if (fabs(object->xCommand-*object->xCommandP) > AS_OBJ_XCOM_MIN_CHANGE) {
      object->xCommand = *object->xCommandP; //Q: New command is always written. What if switched from man, and the same position is commanded?
      if (!object->manual)                   //A: newCall is always set in case of a switch.
	newCall = TRUE;
    }
    /* Check command position */
    if ((object->xCommand > AS_OBJ_XMAX) || (object->xCommand < AS_OBJ_XMIN)) { /* Non-valid command position - disable! */
      if (!(object->errstatus & AS_ERR_XCOM)) { /* Error flag not already set. */
	object->errstatus |= AS_ERR_XCOM; /* Set error flag */
	if (object->verbose >= AS_VERB_DISABLED) /* Print error message if verbose */
	  printf("\nAntiSway: Command position greater than xmax or less than xmin, xCommand=%f, xmax=%f, xmin=%f\n",object->xCommand,AS_OBJ_XMAX,AS_OBJ_XMIN);
	AS_ADDMESSAGE(AS_ALARMTYPE_DISABLED, &mstr,"AS: Command position greater than xmax or less than xmin, xCommand=%.1f, xmax=%.1f, xmin=%.1f.",object->xCommand,AS_OBJ_XMAX,AS_OBJ_XMIN);
      }
    }
  }

  if (object->manual) { //man-specific L retreival
    /* Check for new L and hoisting */
    if (fabs(object->Lc - *object->LcP) > AS_OBJ_L_MIN_CHANGE) {
      hoisted = TRUE;
      object->Lc = *object->LcP; 
    }
    hoisting = ( fabs(object->DLc) > AS_OBJ_DL_MIN ); 
  }
  else { //auto mode. auto-specific input..

     /* Check for new L. */
    if (fabs(object->Lc - *object->LcP) > AS_OBJ_L_MIN_CHANGE) {
      if (object->compensate >= AS_AUTO_COMP_L) {
	hoisted = TRUE;
	object->Lc = *object->LcP;
	if (object->compensate >= AS_AUTO_COMP_DL)
	  hoisting = ( fabs(object->DLc) > AS_OBJ_DL_MIN );
      }
      else if ( newCall || (AS_OBJ_SETP->N == 0) ) /* No compensation. Update Lc when new call or if not working on a set. */
	object->Lc = *object->LcP; 
    } 


    /* Reset reference position to actual position if new call and not in travel */
    if ( newCall &&  (AS_OBJ_SETP->N == 0) ) { 
      AS_OBJ_XR = object->xc;
      AS_OBJ_XRM = object->xc;
    }

    /* 2D-mode if other points at something (i.e. the other antisway object) that is enable. */
    object->other = *object->otherP;
    if ( object->other != NULL && (AS_OBJ_OTHER->enable && !AS_OBJ_OTHER->manual) ) //second condition will not be tested if first is not true. Important!
      object->autostatus |= AS_AUTO_2D;
    else /* other is NULL pointer, or not enable */
      object->autostatus &= (~AS_AUTO_2D);

    /*  Check that other object is not disabled due to error. Note that there is no checking that other is really an AntiSway object */
    if ( (object->autostatus & AS_AUTO_2D) && (AS_OBJ_OTHER->errstatus > AS_ERR_MINOR) ) {
      object->autostatus &= (~AS_AUTO_2D);
      if (!(object->errstatus & AS_ERR_OTHER)) { /* Error flag not already set. */
	object->errstatus |= AS_ERR_OTHER; /* Set error flag */
	if (object->verbose >= AS_VERB_ERR) /* Print error message if verbose */
	  printf("\nAntiSway: Other AntiSway object stopped!\n");
	AS_ADDMESSAGE(AS_ALARMTYPE_ERR, &mstr,"AS: Other AntiSway object stopped!");
      }
    }
    
    /* Check for new local low-speed restriction (auto mode). Determines if umax[4] is used as umaxsa instead of umax[3] */  
    if (object->umax2 != *object->umax2P) {
      object->umax2 = *object->umax2P;
      newCall = TRUE;
    }  
  } //end of auto-specific input
  

  if (object->Lc > 0.0) 
    omega = sqrt(AS_GRAV_ACCEL / object->Lc); /* Natural frequency of oscillation */
  else { /* Non-valid pendulum length? disable! */
    if ( !(object->errstatus & AS_ERR_LSIGN)) { /* Error flag not already set. */
      object->errstatus |= AS_ERR_LSIGN; /* Set error flag */
      if (object->verbose >= AS_VERB_DISABLED) /* Print error message if verbose */
	printf("\nAntiSway: Non-valid pendulum length. L=%f\n",object->Lc);
      AS_ADDMESSAGE(AS_ALARMTYPE_DISABLED, &mstr,"AS: Non-valid pendulum length. L=%.1f",object->Lc);
    }
  }


  /* 2.4 Write error messages to message attribute and set alarm outputs. */

  for (i=0; i<2; i++) {
    if (AS_OBJ_MESSAGEQP(i) != NULL) { // There is a message in the queue
      if ( (AS_OBJ_MESSAGEQP(i)->count)++ == 0) //copy to string
	strncpy(object->message[i], AS_OBJ_MESSAGEQP(i)->mess, AS_OBJ_M_LENGTH);
      else if ( AS_OBJ_MESSAGEQP(i)->count == AS_OBJ_M_ALWAIT) { //set corresponding alarm
	AS_OBJ_SETALARM(i);
      }
      else if ( AS_OBJ_MESSAGEQP(i)->count == (AS_OBJ_M_ALWAIT + AS_OBJ_M_ALHOLD) ) { //dequeue message or move and reset counter. Reset alarm.
	if ( strlen(AS_OBJ_MESSAGEQP(i)->mess) > AS_OBJ_M_LENGTH ) {
	  /*move not copied substring to beginning of queue message */
	  memmove( AS_OBJ_MESSAGEQP(i)->mess, AS_OBJ_MESSAGEQP(i)->mess + AS_OBJ_M_LENGTH, strlen(AS_OBJ_MESSAGEQP(i)->mess + AS_OBJ_M_LENGTH) + 1 ); 
	  AS_OBJ_MESSAGEQP(i)->count = 0;
	}
	else
	  AS_deQMessage(AS_OBJ_MESSAGEQPP(i)); /* remove message from queue. */
	AS_OBJ_RESETALARM(i);
      }
    } //Q: is wait and hold really needed??  
  }   //A: otherwise another solution for flashing alarm outputs is needed.


  
  /* 2.5 Possibly reset status masks, and set status flags.*/
  
  if (object->reset) {
    object->errstatus &= (~AS_ERR_MINOR);
    object->autostatus &= (AS_AUTO_NORESET);
    object->reset = FALSE;
  }
  for (i=0; i<AS_NERR_FLAGS; i++) 
    object->flags[i] =  ( (object->errstatus & (int) (.1 + AS_ROUND( pow( 2.0, (double) i) ))) && TRUE );
  for ( ; i<(AS_NERR_FLAGS + AS_NAUTO_FLAGS); i++)
    object->flags[i] = ( (object->autostatus & (int) (.1 + AS_ROUND( pow( 2.0, (double) (i-AS_NERR_FLAGS) ) ))) && TRUE );  
  

  /* If crucial error or not enabled - Reset reference values and exit. */ 
  if ( !object->enable || (object->errstatus > AS_ERR_MINOR) ) {
    AS_emptySet(AS_OBJ_SETP);
    object->uR=0.0;
    AS_OBJ_XR=object->xc;
    AS_OBJ_XRM=object->xc;
    AS_OBJ_THR=0.0;
    AS_OBJ_THRM=0.0;
    object->aRO=0.0;
    object->uRO=0.0;
    object->xRO=object->xc;
    object->thRO=0.0;      
    return;
  }
  

  
  /************************************************
   *  Step 3. Create or change the phasor set, and
   *          get current reference acceleration.
   ************************************************/


  if (object->manual) {
  
    /* Call Engine function if new call or working on an existing Set/Queue */
  
    if (newCall || (AS_OBJ_SETP->N != 0)) {
      object->errstatus |= AS_Engine_man( newCall, hoisting, hoisted, object->verbose, shp,
					  object->uCommand, object->uR, object->DLc, omega, AS_OBJ_AMAXHM, AS_OBJ_AMAXSM, dt,
					  &(object->aR), AS_OBJ_SETP );
      object->done = FALSE;
    }

    /* Not working on a set - reset reference values */

    else { 
      object->aR = 0.0;
      if (fabs(object->uR) < AS_OBJ_UR_ZEROLIM) { /* Reset xR if at rest */
	AS_OBJ_XR=object->xc;
	AS_OBJ_XRM=object->xc;
      }
      if (fabs(object->uR - object->uCommand) > AS_OBJ_UR_ATTAIN_ERRLIM) {/* Error in velocity attainment ? A small error will always occur in the floating point operations.*/
	object->errstatus |= AS_ERR_UR_ATTAIN;
	if (object->verbose >= AS_VERB_ERR)
	  printf("\n\tAntiSway: Error in command velocity attainment larger than %f\n\tuR=%f, uCommand=%f \n\n",AS_OBJ_UR_ATTAIN_ERRLIM,object->uR,object->uCommand);
	AS_ADDMESSAGE(AS_ALARMTYPE_ERR, &mstr,"AS: Error in command velocity attainment larger than %.4f. uR=%.4f, uCommand=%.4f ",AS_OBJ_UR_ATTAIN_ERRLIM,object->uR,object->uCommand);
      }
      object->uR=object->uCommand; /* In all cases, reset uR to uCommand exactly */
    }

  }
  else { //auto

    /* Call Engine function if new call or working on an existing Set/Queue */

    if (newCall || (AS_OBJ_SETP->N != 0) ) {
      object->errstatus |= AS_Engine_auto( newCall, hoisting, hoisted, object->verbose, AS_OBJ_OTHER, shp,
					   object->xCommand, AS_OBJ_XR, object->uR, object->DLc, omega, 
					   AS_OBJ_AMAXHA, AS_OBJ_AMAXSA, AS_OBJ_UMAXHA, AS_OBJ_UMAXSA, AS_OBJ_UR_MIN, dt,
					   &(object->aR), &(object->autostatus), AS_OBJ_SETP ); 
      object->done = FALSE;
    }

    /* Not working on a set - reset reference values */

    else { 
      object->aR = 0.0;
      /* Reset a possible NEEDMATCH flag in other object */
      if (object->autostatus & AS_AUTO_2D)
	AS_OBJ_OTHER->autostatus &= (~AS_AUTO_NEEDMATCH);

      /* Error in velocity attainment ? A small error will always occur in the floating point operations.*/
      if (fabs(object->uR) > AS_OBJ_UR_ATTAIN_ERRLIM) { // in auto mode, "uCommand" is always zero when not working on a set.
	object->errstatus |= AS_ERR_UR_ATTAIN;
	if (object->verbose >= AS_VERB_ERR)
	  printf("\n\tAntiSway: Error in command velocity attainment larger than %f\n\tuR=%f, uCommand=%f \n\n",AS_OBJ_UR_ATTAIN_ERRLIM,object->uR,0.0);
	AS_ADDMESSAGE(AS_ALARMTYPE_ERR, &mstr,"AS: Error in command velocity attainment larger than %.4f. uR=%.4f, uCommand=%.4f.",AS_OBJ_UR_ATTAIN_ERRLIM,object->uR,0.0);
      }
      object->uR=0.0; /* Reset uR to zero at end of travel */   
 
      /* Error in position attainment ? A small error will always occur in the floating point operations.*/
      if (fabs(AS_OBJ_XR - object->xCommand) > AS_OBJ_XR_ATTAIN_ERRLIM) {
	object->errstatus |= AS_ERR_XR_ATTAIN;
	if (object->verbose >= AS_VERB_ERR)
	  printf("\n\tAntiSway: Error in command position attainment larger than %f\n\txR=%f, xCommand=%f \n\n",AS_OBJ_XR_ATTAIN_ERRLIM,AS_OBJ_XR,object->xCommand);
	AS_ADDMESSAGE(AS_ALARMTYPE_ERR, &mstr,"AS: Error in command position attainment larger than %.4f, xR=%.4f, xCommand=%.4f.",AS_OBJ_XR_ATTAIN_ERRLIM,AS_OBJ_XR,object->xCommand);
      }
      AS_OBJ_XR=object->xCommand; /* Reset xR to reference position at end of travel */
      AS_OBJ_XRM=object->xCommand;
    }
  }


  
  /***************************************************
   *  Step 4. Calculate output and check its validity.
   ***************************************************/
  
  /* 4.1 Check reference acceleration for bounds. Store 32-bit output */
  
  if ((fabs(object->aR) - shp->AMax*AS_OBJ_AMAXHMA) > 100.0*FLT_EPSILON) {/* If too large error, disable. This should never happen.. */
    object->errstatus |= AS_ERR_AMAX;
    if (object->verbose >= AS_VERB_DISABLED)
      printf("\nAntiSway: reference acceleration greater than hard limit. aR=%f, amaxH=%f\n",object->aR, AS_OBJ_AMAXHMA);
    AS_ADDMESSAGE(AS_ALARMTYPE_DISABLED, &mstr,"AS: Reference acceleration greater than hard limit. aR=%.3f, amaxH=%.3f",object->aR, AS_OBJ_AMAXHMA);
    object->aR=0.0;
  }
  object->aRO = object->aR;


  /* 4.2 Update values of acceleration-dependent reference variables */ 

  AS_xIntegratorUA(&AS_OBJ_XRM, &AS_OBJ_XR, object->uR, object->aR, dt); 
  AS_uIntegratorFwd(&object->uR, object->aR, dt);
  if (object->zeroTheta) {
    AS_OBJ_THR = 0.0;
    AS_OBJ_THRM = 0.0;
  }
  else 
    AS_thetaIntegrator(&AS_OBJ_THRM, &AS_OBJ_THR, object->Lc, object->DLc, object->aR, dt);
 

  /* 4.3 Check reference velocity for bounds. Store 32-bit output */

  if ((fabs(object->uR) - AS_OBJ_UMAXHMA) > AS_EPSILON) { /* uR > hard limit ? */
    if (fabs(object->uR) > AS_OBJ_UMAXHMA*AS_OBJ_UR_MAX_ERRFACT) { 
      /* If too large error, disable. A small violation is allowed, since many shapers can temporarily cause... */ 
      object->errstatus |= AS_ERR_UMAX; /*...uR to exceed umax slightly when changing commands frequently. uRO is held down however.*/
      if (object->verbose >= AS_VERB_DISABLED)
	printf("\nAntiSway: reference velocity much greater than hard limit. uR=%f, umaxH=%f\n",object->uR,AS_OBJ_UMAXHMA);
      AS_ADDMESSAGE(AS_ALARMTYPE_DISABLED, &mstr,"AS: Reference velocity much greater than hard limit. uR=%.2f, umaxH=%.3f",object->uR,AS_OBJ_UMAXHMA);
    }
    else { /* small error */
      if (object->verbose >= AS_VERB_ERR) 
	printf("\nAntiSway: reference velocity slightly greater than hard limit. uR=%f, umaxH=%f. Changed output to sign*umaxH.\n",object->uR,AS_OBJ_UMAXHMA);
    AS_ADDMESSAGE(AS_ALARMTYPE_ERR, &mstr,"AS: Reference velocity slightly greater than hard limit. uR=%.3f, umaxH=%.3f. Changed output to sign*umaxH.",object->uR,AS_OBJ_UMAXHMA);
    }
    object->uRO = AS_SIGN(object->uR)*AS_OBJ_UMAXHMA;
  }
  else if (fabs(object->uR) < AS_OBJ_UR_ZEROLIM) /* Reset to zero if close enough. Sometimes the reset to uCommand in step 3 is insufficient. */
    object->uRO = 0.0;                       /* Exact zero output is often used as a condition in plc programs. (e.g. for disabling of a frequency converter) */ 
  else 
    object->uRO = object->uR; 


  if (object->manual) { //manual xR check

    /* 4.4 Check reference position for bounds. Store 32-bit output. Will only perform control if the position input object->xc is used. */ 
    if ( (object->xcP != &object->xc) && (((AS_OBJ_XR - AS_OBJ_XMAX) > AS_EPSILON) || ((AS_OBJ_XR - AS_OBJ_XMIN) < -AS_EPSILON)) ) {
      object->errstatus |= AS_ERR_MAN_XLIM;
      if (object->verbose >= AS_VERB_ERR)
	printf("\nAntiSway: reference position off limits. xR=%f, xmax=%f, xmin=%f. Changed xR to xmin or xmax\n",AS_OBJ_XR,AS_OBJ_XMAX,AS_OBJ_XMIN);
      AS_ADDMESSAGE(AS_ALARMTYPE_ERR, &mstr,"AS: Reference position off limits. xR=%.1f, xmax=%.1f, xmin=%.1f. Changed xR to xmin or xmax.",AS_OBJ_XR,AS_OBJ_XMAX,AS_OBJ_XMIN);
      if (AS_OBJ_XR < AS_OBJ_XMIN) {
	AS_OBJ_XR=AS_OBJ_XMIN;
	AS_OBJ_XRM=AS_OBJ_XMIN;
      }
      else {
	AS_OBJ_XR=AS_OBJ_XMAX;
	AS_OBJ_XRM=AS_OBJ_XMAX;
      }
    }  
  }
  else { //auto xR check

    /* 4.4 Check reference position for bounds. In auto mode, disable if not valid. Store 32-bit output */

    if (((AS_OBJ_XR - AS_OBJ_XMAX > AS_EPSILON)) || ((AS_OBJ_XR - AS_OBJ_XMIN) < -AS_EPSILON)) {
      object->errstatus |= AS_ERR_XLIM;
      if (object->verbose >= AS_VERB_DISABLED)
	printf("\nAntiSway: reference position off limits. xR=%f, xmax=%f, xmin=%f. Changed last xR to xmin or xmax\n",AS_OBJ_XR,AS_OBJ_XMAX,AS_OBJ_XMIN);
      AS_ADDMESSAGE(AS_ALARMTYPE_DISABLED, &mstr,"AS: reference position off limits. xR=%.1f, xmax=%.1f, xmin=%.1f. Changed last xR to xmin or xmax.",AS_OBJ_XR,AS_OBJ_XMAX,AS_OBJ_XMIN);
      if (AS_OBJ_XR < AS_OBJ_XMIN) {
	AS_OBJ_XR=AS_OBJ_XMIN;
	AS_OBJ_XRM=AS_OBJ_XMIN;
      }
      else {
	AS_OBJ_XR=AS_OBJ_XMAX;
	AS_OBJ_XRM=AS_OBJ_XMAX;
      }
    }
  }

  object->xRO = AS_OBJ_XR; /* Reference position. 32-bit output. */
  object->thRO = 180.0/AS_PI*AS_OBJ_THR; /* "Reference" sway angle. Conversion from radians to degrees. 32-bit output. */
    

  /* Display Set / Queue if new call and verbose object */

  if (newCall && (object->verbose >= AS_VERB_ALL )) {
    AS_displaySet(AS_OBJ_SETP, omega);
    printf("\n\tSway Vector: X=%f Y=%f\n",(AS_OBJ_THR - AS_OBJ_THRM)/(omega * dt) , AS_OBJ_THR);
    printf("\tLc=%f \tAcceleration: aR=%f ScanTime dt=%f\n", object->Lc, object->aR, dt);
  } 


} /* End of AntiSway_exec */

/* -------------------------------------------------------------------------------------------------------------- */


