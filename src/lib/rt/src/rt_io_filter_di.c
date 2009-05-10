/* 
 * Proview   $Id: rt_io_filter_di.c,v 1.2 2005-09-01 14:57:55 claes Exp $
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

/* rt_io_filter_di.c -- Runtime environment - I/O filter Di */

/* Include files */

#include <stdio.h>
#include <stdlib.h>

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef pwr_class_h
#include "pwr_class.h"
#endif

#ifndef pwr_baseclasses_h
#include "pwr_baseclasses.h"
#endif

#ifndef rt_gdh_h
#include "rt_gdh.h"
#endif

#ifndef rt_io_msg_h
#include "rt_io_msg.h"
#endif

#ifndef rt_io_filter_di_h
#include "rt_io_filter_di.h"
#endif


/* Defines */
#define _MaxNoOfDi	16 

#define	_FilterType_0	0 
#define	_FilterType_1	1
#define	_FilterType_100	100

/* Structures */
typedef struct s_FilterData_1 sFilterData_1;
/* User structures */
typedef struct s_FilterData_100 sFilterData_100;



/*
* Name:    
*   _eState_1
* Description:
*   The different states used in filter type = 1 
*/
enum e_State_1
    {
    _eState__		= 0,
    _eState_1_Neutral	= 1,
    _eState_1_DelayOn	= 2,
    _eState_1_DelayOff	= 3,
    _eState_
    };

/* Enumerations */
typedef enum e_State_1 _eState_1;


/*
* Name:    
*   sFilterData_1
* Description:
*   The structure of filter data used in filter type = 1 
*/
struct s_FilterData_1
    {
    _eState_1	  Type;
    pwr_tFloat32  ScanTime;
    pwr_tUInt32	  TimerDelayOn;
    pwr_tUInt32	  TimerDelayOff;
    pwr_tBoolean  OldValue;
    pwr_tBoolean  *ActualValue;
    };


/*
* Name:    
*   sFilterData_100
* Description:
*   The user structure of filter data used in filter type = 100 
*/
struct s_FilterData_100
    {
    pwr_tFloat32  Dummy;
    };



/*
* Name:    
*   io_FilterDi
*
*
* Function:
*   Filtration of Di.
* Description:
*   
*/
pwr_tStatus io_DiFilter
    (
    pwr_sClass_Di   *SignalObj[],
    pwr_tUInt16	    *Data,
    void	    *FilterData[]
    )
{
    pwr_tUInt16	Mask, OldData;
    pwr_tUInt16	ResetMask;
    pwr_tUInt16	InvMask = 65535;
    sFilterData_1 *Data_1;
    sFilterData_100 *Data_100;
    int Idx;	

    /* Save data local */

    OldData = *Data;


    /* Scan through the signals and do the filtration */

    for ( Mask = 1, Idx = 0; Idx < _MaxNoOfDi; Idx++, Mask <<= 1) {
       if ( SignalObj[Idx] == NULL ) continue;

       switch ( SignalObj[Idx]->FilterType ) {

       case _FilterType_0:
	   break;

       case _FilterType_1:
           if ( FilterData[Idx] == NULL ) break;
           ResetMask = 0;
           Data_1 = ( sFilterData_1 *) FilterData[Idx];

           if ( (pwr_tBoolean)(( *Data & Mask ) != 0) == TRUE &&
                Data_1->OldValue == FALSE ) { /* Pos. flank */
	     switch ( Data_1->Type ) {

	     case _eState_1_Neutral:
                 Data_1->TimerDelayOn  = SignalObj[Idx]->FilterAttribute[0]/Data_1->ScanTime + 0.5;
                 *Data ^= Mask;		    /* Invert bit */
		 Data_1->Type = _eState_1_DelayOn;
		 break;

	     case _eState_1_DelayOn:
                 Data_1->TimerDelayOn  = SignalObj[Idx]->FilterAttribute[0]/Data_1->ScanTime + 0.5;
                 *Data ^= Mask;		    /* Invert bit */
                 break;

	     case _eState_1_DelayOff:
		 if ( Data_1->TimerDelayOff > 0 )
		   Data_1->TimerDelayOff--;
		 else {
		   Data_1->Type = _eState_1_Neutral;
		   break;
		 }

                 break;

	     default:
		 break;

	     }
           }
           else if ( (pwr_tBoolean)(( *Data & Mask ) != 0) == FALSE &&
                     Data_1->OldValue == TRUE ) { /* Neg. flank */
             switch ( Data_1->Type ) {

             case _eState_1_Neutral:
                 Data_1->TimerDelayOff  = SignalObj[Idx]->FilterAttribute[1]/Data_1->ScanTime + 0.5;
                 *Data ^= Mask;		    /* Invert bit */
                 Data_1->Type = _eState_1_DelayOff;
                 break;

	     case _eState_1_DelayOn:
		 if ( Data_1->TimerDelayOn > 0 )
		   Data_1->TimerDelayOn--;
		 else {
		   Data_1->Type = _eState_1_Neutral;
                   *Data ^= Mask;	    /* Invert bit */
		   break;
		 }

                 break;

	     case _eState_1_DelayOff:
                 Data_1->TimerDelayOff  = SignalObj[Idx]->FilterAttribute[1]/Data_1->ScanTime + 0.5;
                 *Data ^= Mask;		    /* Invert bit */
                 break;

	     default:
	         break;

	     }
           }
           else {					      /* No flank */
	     switch ( Data_1->Type ) {

	     case _eState_1_Neutral:
		 break;

	     case _eState_1_DelayOn:
		 if ( Data_1->TimerDelayOn > 0 )
		   Data_1->TimerDelayOn--;
		 else {
		   Data_1->Type = _eState_1_Neutral;
		   break;
		 }

		 ResetMask = Mask ^ InvMask;
		 *Data &= ResetMask;	    /* Reset bit */
		 break;

	     case _eState_1_DelayOff:
		 if ( Data_1->TimerDelayOff > 0 )
		   Data_1->TimerDelayOff--;
		 else {
		   Data_1->Type = _eState_1_Neutral;
		   break;
		 }

		 *Data |= Mask;		    /* Set bit */
		 break;

	     default:
		 break;

	     }
           }

           Data_1->OldValue = (pwr_tBoolean) ((OldData & Mask) != 0); 

	   break;

       case _FilterType_100:		    /* User filter */
           Data_100 = ( sFilterData_100 *) FilterData[Idx];
					  
	   break;

       default:
           break;

       }
    }

    return IO__SUCCESS;
} /* END io_FilterDi */


/*
* Name:    
*   io_InitFilterDi
*
*
* Function:
*   Initialize filter for max 16 Di ( one Di-card ).
* Description:
*   
*/
pwr_tStatus io_InitDiFilter
    (
    pwr_sClass_Di   *SignalObj[],
    pwr_tBoolean    *Filter,
    void	    *FilterData[],
    pwr_tFloat32    ScanTime    
    )
{
    int	    Idx;
    sFilterData_1  *Data_1;
    sFilterData_100  *Data_100;

    *Filter = FALSE;			  /* Supose no signals with filter */

    for ( Idx = 0; Idx < _MaxNoOfDi; Idx++) {
       if ( SignalObj[Idx] == NULL ) continue;

       switch (SignalObj[Idx]->FilterType) {

       case _FilterType_0:
	   break;

       case _FilterType_1:
           if ( ScanTime > 0 ) {
             Data_1 = (sFilterData_1 *) malloc( sizeof(sFilterData_1) );
             Data_1->ScanTime = ScanTime;
             Data_1->TimerDelayOn  = SignalObj[Idx]->FilterAttribute[0]/Data_1->ScanTime + 0.5;
             Data_1->TimerDelayOff  = SignalObj[Idx]->FilterAttribute[1]/Data_1->ScanTime + 0.5;
	     Data_1->ActualValue = gdh_TranslateRtdbPointer((unsigned long)SignalObj[Idx]->ActualValue);
	     Data_1->OldValue = *Data_1->ActualValue;
             Data_1->Type  = _eState_1_Neutral;
             FilterData[Idx] = Data_1; 
             *Filter = TRUE; 
           }
	   break;

       case _FilterType_100:		  /* User filter */
           Data_100 = (sFilterData_100 *) malloc( sizeof(sFilterData_100) );

	   break;

       default:
           break;

       }
    }

    return IO__SUCCESS;
} /* END io_InitFilterDi */


/*
* Name:    
*   io_CloseFilterDi
*
*
* Function:
*   Close filter for max 16 Di ( one Di-card ).
* Description:
*   
*/
void io_CloseDiFilter
    (
    void	    *FilterData[]
    )
{
    int	i;

    for ( i = 0; i < _MaxNoOfDi; i++) {
      if ( FilterData[i] != NULL )
        free( FilterData[i]);
    }
}
