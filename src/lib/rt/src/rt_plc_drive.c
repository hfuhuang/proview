/* rt_plc_drive.c -- <short description>

   PROVIEW/R
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#include <stdio.h>
#include "pwr.h"
#include "pwr_baseclasses.h"
#include "rt_plc.h"
#include "rt_plc_timer.h"

/*_*
  Drive
  Function:
    Simple Drive with interlocks and
    watch-dog for responses.

  @aref drive Drive
*/
void drive_exec (
  plc_sThread		*tp,
  pwr_sClass_drive  *object
) {
  pwr_tBoolean	start;
  pwr_tBoolean	stop;
  pwr_tBoolean	Aold;
  pwr_tBoolean	Tflag;

  /* Save old values for flank detect */
  Aold = object->AutoStart;
  Tflag = object->TimerFlag;

  /* New Input */
  object->AutoStart = *object->AutoStartP;
  object->AutoNoStop = *object->AutoNoStopP;
  object->Local = *object->LocalP;
  object->ConOn = *object->ConOnP;
  object->Speed = *object->SpeedP;
  object->SafeStop = *object->SafeStopP;
  object->ProdStop = *object->ProdStopP;
  object->NoStart = *object->NoStartP;

  /* Is there a stop condition ? */
  stop = object->SafeStop;
  if (object->Local) {
    object->LocStart = *object->LocStartP;
    object->LocDrive = *object->LocDriveP;
    if (!object->LocDrive || (!object->LocStart && !object->ConOn))
	  stop = TRUE;
  } else {
    if ( object->ProdStop ) stop = TRUE;
    if ( !object->ManMode && !object->AutoNoStop ) stop = TRUE;
    if ( object->ManMode && object->ManStp ) stop = TRUE;

    /* Check Alarms */
    if (object->Order && !stop) {
      if (!object->ConOn) {
	if (object->Status == 2 && !Tflag) {
	  object->Alarm1 = TRUE;
	  stop = TRUE;
	} else if (object->Status > 2) {
	  object->Alarm2 = TRUE;
	  stop = TRUE;
	}
      } else if (!object->Speed && (object->Status >= 3) && !Tflag) {
	object->Alarm3 = TRUE;
	stop = TRUE;
      }
    }
  }

  /* Start condition ? */
  if (object->ManMode) {
    start = object->ManSta;
  } else {
    start = object->AutoStart && !Aold;
  }

  /* Remove alarms when new start */
  if (start) {
    object->Alarm1 = FALSE;
    object->Alarm2 = FALSE;
    object->Alarm3 = FALSE;
  }

  /* Sum alarm */
  object->SumAlarm = ( object->Alarm1 || object->Alarm2 ||
    object->Alarm3 );

  /* No start or Local */
  if (object->NoStart) start = FALSE;
  if (object->Local) start = object->LocStart;

  /* Remove manual start / stop */
  object->ManSta = FALSE;
  object->ManStp = FALSE;

  /* Start or stop */
  if (stop) {				/* Stop-condition */
    object->Order = FALSE;
    object->TimerCount = 0;
  } else if (start && !object->Order) {	/* New start */
    object->Order = TRUE;
    object->TimerTime = object->ProdTim;
    timer_in( tp, object);
    object->Status = 2;			/* Wait for contactor */
  }

  /* Check mode */
  if (object->Order) {
    if ((object->Status == 2) && object->ConOn) {
      /* Waiting for contactor is finished */
      object->TimerTime = object->SpeedTim;
      timer_in( tp, object);
      object->Status = 3;			/* Wait for Speed */
    } else if ((object->Status == 3) && object->Speed) {
      object->Status = 4;	/* Production */
    }
  } else {
    object->Status = object->ConOn || object->Speed;
  }
					  /* Stopping or stop */
/* Indication production */
  object->Ind = (object->Status == 4) && !object->Local;
}

/*_*
  Valve
  Function:
    Magnetic valve with time alarm

  @aref valve Valve
*/

void valve_exec (
  plc_sThread		*tp,
  pwr_sClass_valve  *object
) {
  pwr_tBoolean	open;

  /* Get Input-signals */
  object->AutoOpen = *object->AutoOpenP;
  object->EndOpen = *object->EndOpenP;
  object->EndClose = *object->EndCloseP;
  object->Local = *object->LocalP;
  object->LocalOpen = *object->LocalOpenP;
  object->LocalClose = *object->LocalCloseP;
  object->SafeOpen = *object->SafeOpenP;
  object->SafeClose = *object->SafeCloseP;
  object->ProdOpen = *object->ProdOpenP;
  object->ProdClose = *object->ProdCloseP;

  /* Check different orders after priority */
  if (object->SafeClose) {
    open = FALSE;
  } else if (object->SafeOpen) {
    open = TRUE;
  } else if (object->Local) {
    if (object->LocalClose) {
      open = FALSE;
    } else if (object->LocalOpen) {
      open = TRUE;
    } else {
      open = object->OrderOpen;
    }
  } else {
    if (object->ProdClose) {
      open = FALSE;
    } else if (object->ProdOpen) {
      open = TRUE;
    } else if (object->ManMode) {
      if (object->ManClose) {
	open = FALSE;
      } else if (object->ManOpen) {
	open = TRUE;
      } else {
	open = object->OrderOpen;
      }
    } else {
      open = object->AutoOpen;
    }
  }

  /* Check if new order */
  if (open != object->OrderOpen) {
    object->Status = open;	/* Opening / Closing */
    timer_in( tp, object);
  }

  /* Check if alarms */
  if (object->Status < 2) {
    if (open) {
      if (object->EndOpen) {
	object->Status = 3;	/* Open */
      } else if (!object->TimerFlag) {
	object->Alarm1 = TRUE;
      }
    } else {
      if (object->EndClose) {
	object->Status = 2;	/* Closed */
      } else if (!object->TimerFlag) {
	object->Alarm2 = TRUE;
      }
    }
  } else {
    if (open && (!object->EndOpen || object->EndClose)) {
	  object->Alarm3 = TRUE;		/* No longer Open */
    }
    if (!open && (!object->EndClose || object->EndOpen)) {
	  object->Alarm4 = TRUE;		/* No longer closed */
    }
  }

  /* Remove alarms if OK */
  if (object->Alarm1 && (!open || object->EndOpen)) {
      object->Alarm1 = FALSE;
  }

  if (object->Alarm2 && (open || object->EndClose)) {
      object->Alarm2 = FALSE;
  }

  if (object->Alarm3 && 
    (!open || (object->EndOpen && !object->EndClose))
  ) {
    object->Alarm3 = FALSE;
  }

  if (object->Alarm4 &&
    (open || (!object->EndOpen && object->EndClose))
  ) {
    object->Alarm4 = FALSE;
  }

  /* Sum Alarm */
  object->SumAlarm = object->Alarm1 || object->Alarm2 ||
    object->Alarm3 || object->Alarm4;

  /* Set order and indications */
  object->OrderOpen = open;
  object->IndOpen = open && object->EndOpen &&
    !object->EndClose && !object->Local;
  object->IndClose = !open && object->EndClose &&
    !object->EndOpen && !object->Local;

  /* Remove Manual orders */
  object->ManOpen = FALSE;
  object->ManClose = FALSE;
}

/*_*
  MValve
  Function:
    Motor-valve

  @aref mvalve MValve
*/
void mvalve_exec (
  plc_sThread		*tp,
  pwr_sClass_mvalve *object
) {
  pwr_tBoolean    open;
  pwr_tBoolean    close;
  pwr_tBoolean    oold;
  pwr_tBoolean    cold;

  /* Save old orders for flank detect */
  oold = object->AutoOpen || object->ProdOpen || object->SafeOpen;
  cold = object->AutoClose || object->ProdClose || object->SafeClose;

  /* Get new input-data */
  object->AutoOpen = *object->AutoOpenP;
  object->AutoClose = *object->AutoCloseP;
  object->EndOpen = *object->EndOpenP;
  object->EndClose = *object->EndCloseP;
  object->ConOpen = *object->ConOpenP;
  object->ConClose = *object->ConCloseP;
  object->Local = *object->LocalP;
  object->SafeOpen = *object->SafeOpenP;
  object->SafeClose = *object->SafeCloseP;
  object->SafeStop = *object->SafeStopP;
  object->ProdOpen = *object->ProdOpenP;
  object->ProdClose = *object->ProdCloseP;
  object->ProdStop = *object->ProdStopP;

  /* Remove old alarms */
  if (((object->AutoOpen || object->ProdOpen || object->SafeOpen) &&
    !oold) || object->ManOpen || object->ManStop
  ) {
    object->Alarm5 = FALSE; /* New open */
  }

  if (((object->AutoClose || object->ProdClose || object->SafeClose) &&
    !cold) || object->ManClose || object->ManStop
  ) {
    object->Alarm6 = FALSE; /* New Close */
  }

  if (object->EndOpen) {
    object->Alarm1 = FALSE;
    if (!object->EndClose)
      object->Alarm3 = FALSE;
  }

  if (object->EndClose) {
    object->Alarm2 = FALSE;
    if (!object->EndOpen)
      object->Alarm4 = FALSE;
  }

  if (object->OrderClose) {
    object->Alarm1 = FALSE;
    object->Alarm3 = FALSE;
    object->Alarm5 = FALSE;
  }

  if (object->OrderOpen) {
    object->Alarm2 = FALSE;
    object->Alarm4 = FALSE;
    object->Alarm6 = FALSE;
  }

  /* Evaluate orders */

  /* Safe conditions */
  if (object->SafeStop) {
    open = FALSE;
    close = FALSE;
  } else if (object->SafeClose) {
    open = FALSE;
    close = !object->Alarm6;
  } else if (object->SafeOpen) {
    close = FALSE;
    open = !object->Alarm5;
  } else if (object->Local) {
    /* Local */
    object->LocalOpen = *object->LocalOpenP;
    object->LocalClose = *object->LocalCloseP;
    object->LocalStop = *object->LocalStopP;
    if (object->LocalStop) {
      open = FALSE;
      close = FALSE;
    } else if (object->LocalClose) {
      open = FALSE;
      close = TRUE;
    } else if (object->LocalOpen) {
      open = TRUE;
      close = FALSE;
    } else if (object->OrderOpen) {
      close = FALSE;
      open = object->ConOpen;
    } else if (object->OrderClose) {
      open = FALSE;
      close = object->ConClose;
    } else {
      open = FALSE;
      close = FALSE;
    }
  } else if (object->ProdStop) {
    /* Production  requirement conditions */
    open = FALSE;
    close = FALSE;
  } else if (object->ProdClose) {
    open = FALSE;
    close = !object->Alarm6;
  } else if (object->ProdOpen) {
    close = FALSE;
    open = !object->Alarm5;
  } else if (object->ManMode) {
    /* Manual */
    if (object->ManStop) {
      open = FALSE;
      close = FALSE;
    } else if (object->ManClose) {
      open = FALSE;
      close = TRUE;
    } else if (object->ManOpen) {
      open = TRUE;
      close = FALSE;
    } else if (object->OrderOpen) {
      close = FALSE;
      open = !object->Alarm5;
    } else if (object->OrderClose) {
      open = FALSE;
      close = !object->Alarm6;
    } else {
      open = FALSE;
      close = FALSE;
    }
  } else {
    /* Automatic */
    if (object->AutoClose) {
      open = FALSE;
      close = !object->Alarm6;
    } else if (object->AutoOpen) {
      close = FALSE;
      open = !object->Alarm5;
    } else {
      open = FALSE;
      close = FALSE;
    }
  }

  /* Check if Error end-positions */
  if ((object->Status == 3) && (!object->EndOpen || object->EndClose))	{
    object->Alarm3 = TRUE;
    object->Status = 0;
  }

  if ((object->Status == -3) && (object->EndOpen || !object->EndClose)) {
    object->Alarm4 = TRUE;
    object->Status = 0;
  }

  if (object->EndOpen && object->EndClose) {
    object->Alarm3 = TRUE;
    object->Alarm4 = TRUE;
  }

  /* Set output order and indications */
  /* New open order */
  if (open && !object->OrderOpen) {
    if (object->ConClose) {
      object->Status = 0;
    } else {
      object->TimerTime = object->Ctime;
      timer_in( tp, object);
      object->Status = 1;
    }
  }

  /* New close order */
  if (close && !object->OrderClose) {
    if (object->ConOpen) {
      object->Status = 0;
    } else {
      object->TimerTime = object->Ctime;
      timer_in( tp, object);
      object->Status = -1;
    }
  }

  if (!open && !close && (object->Status > -3) &&
    (object->Status < 3)
  ) {
    object->Status = 0;
  }

  object->OrderOpen =
    (object->EndOpen || close || (object->Status <= 0)) ? FALSE : open;
  object->OrderClose =
    (object->EndClose || (object->Status >=0)) ? FALSE : close;
  object->IndOpen = object->EndOpen && !object->EndClose &&
    !close && !object->Local;
  object->IndClosed = !object->EndOpen && object->EndClose &&
    !open && !object->Local;

  /* Alarm for contactor and time */
  if (object->OrderOpen) {
    if (!object->ConOpen && ((object->Status > 1) || !object->TimerFlag)) {
      object->Alarm5 = TRUE;
    }

    if ((object->Status == 1) && object->ConOpen && !object->TimerFlag) {
      object->TimerTime = object->RunTime;
      timer_in( tp, object);
      object->Status = 2;
    }

    if ((object->Status == 2) && !object->TimerFlag && (object->RunTime > 0)) {
      object->Alarm1 = TRUE;
    }

  } else if (object->OrderClose) {
    if (!object->ConClose && ((object->Status < -1) || !object->TimerFlag)) {
      object->Alarm6 = TRUE;
    }

    if ((object->Status == -1) && object->ConClose && !object->TimerFlag) {
      object->TimerTime = object->RunTime;
      timer_in( tp, object);
      object->Status = -2;
    }

    if ((object->Status == -2) && !object->TimerFlag && (object->RunTime > 0)) {
      object->Alarm2 = TRUE;
    }
  }

  if (object->EndOpen && open)
    object->Status = 3;
  if (object->EndClose && close)
    object->Status = -3;

  /* Sum Alarm*/
  object->SumAlarm = object->Alarm1 || object->Alarm2 || object->Alarm3 ||
    object->Alarm4 || object->Alarm5 || object->Alarm6;

  /* Remove Manual orders */
  object->ManOpen = FALSE;
  object->ManClose = FALSE;
  object->ManStop = FALSE;
}

/*_*
  Posit
  Function:
    Order a drive to a preset position.
    Wait until stable in position.

  @aref posit Posit
*/

void posit_exec (
  plc_sThread		*tp,
  pwr_sClass_posit  *object
) {
  pwr_tBoolean	iold;		/* old value InPlace */
  pwr_tBoolean	aold;		/* old value AutoPos */

  iold = object->InPlace;
  aold = object->AutoPos;

  /* Read Input data */
  object->PosVal = *object->PosValP;
  if (!object->ManMode)
    object->SetPos = *object->SetPosP;
  object->AutoPos = *object->AutoPosP;
  object->Reset = *object->ResetP;

  /* Calculate Difference and deadzone */
  object->PosError = object->PosVal - object->SetPos;
  if ((object->PosError <= object->DeadZone1) && 
    (object->PosError >= -object->DeadZone2)
  ) {
    object->InPlace = TRUE;
  } else {
    object->InPlace = FALSE;
  }

  /* Pos or Reset Order */
  if (object->AutoPos && !aold && !object->ManMode) {
    object->PosOn = TRUE;
    iold = FALSE;
  }

  if (object->Reset)
    object->PosOn = FALSE;

  /* Positioning in progress ? */
  if (object->PosOn) {
    if (object->InPlace) {
      object->Order1 = FALSE;
      object->Order2 = FALSE;
      /* Set up new Time */
      if (!iold && object->TimerTime > 0) {
	timer_in( tp, object);
      }

      /* Check  if InPos long enough */
      if (!object->TimerFlag && object->TimerTime > 0)
	object->PosOn = FALSE;
    } else if (object->PosError > 0) {
      object->Order1 = TRUE;
      object->Order2 = FALSE;
    } else {
      object->Order1 = FALSE;
      object->Order2 = TRUE;
    }
  } else {
    /* Wait for next order */
    object->Order1 = FALSE;
    object->Order2 = FALSE;
  }
}
