/* 
 * Proview   $Id: ssabox_as_phasor_methods.h,v 1.2 2006-02-01 08:21:21 claes Exp $
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
 **/

/*********************************************************************
**********************************************************************
*
*       S S A B  O X E L � S U N D
*       = = = =  = = = = = = = = =
**********************************************************************
*
* Filename      : $pwrp_inc:AS_phasor_methods.h
*
* Description   : This header file is included by AS_phasor_methods.c,
*               : AS_ODE_solvers.h and ra_plc_user.h.
*		: It contains includes, defines, macros, structs and
*		: prototypes for antisway functions.
*                 
*               Date    Sign           Action
* Revision      050213  jonas_h        First edition.
*               
*
*
**********************************************************************
**********************************************************************/

#ifndef ssabox_as_phasor_methods_h
#define ssabox_as_phasor_methods_h 

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

#include "pwr_baseclasses.h"
#include "pwr_basecomponentclasses.h"
#include "pwr_ssaboxclasses.h"

#define AS_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AS_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AS_SIGN(a) ((a) >= 0.0 ? 1.0 : -1.0)
#define AS_ROUND(x) ((x) >= 0 ? (int)((x)+0.5) : (int)((x)-0.5)) /* rounds 1.5 to 2 and -1.5 to -2. Overflow if x larger than max int*/



/* 1. Machine and natural constants */

#define AS_PI 3.14159265358979323846
/* g in Oxel�sund, latitude 58�40'00"N = 1.02392649450334 rad. The formula is:
 * g = 9.780495*(1 + 0.0052892*(sin(p))^2 - 0.0000073*(sin(2*p))^2), where p is the latitude in radians */
#define AS_GRAV_ACCEL 9.81818072105388
/* Epsilon used in some comparisons between doubles. DBL_EPSILON itself turned out insufficient. */
#define AS_EPSILON 100.0*DBL_EPSILON 


/* 2. Error mask codes */

/* Non-crucial errors and minor mishaps */
#define AS_ERR_UR_ATTAIN 1u /* Command velocity attainmant worse than ATTAIN_ERR_LIM. */
#define AS_ERR_XR_ATTAIN 2u /* Command position attainmant worse than ATTAIN_ERR_LIM. Auto only. */
#define AS_ERR_MAN_XLIM 4u /* xR exceeds xmax or below xmin. Manual only. */
#define AS_ERR_MAN_XMAX 4u
#define AS_ERR_ACC_MARGIN 8u /* Margin amaxH-amaxS not enough for dL/dT compensation. This is not really an error, since the situation is completely dealt with by the algorithms. The time performance may be somewhat affected, however, and the flag can be relevant for tuning the margin. Manual only. */
//#define AS_ERR_AUTO_DISRUPT 16u /* autoGo switched off while in travel. This is not treated as a crucial error, to avoid having to toggle enable to continue. Auto only. */
#define AS_ERR_MERGE_ITER 32u /* Number of iterations in mergeSet while loop exceeded AS_MERGE_ITER_LIM. This will cause a residual sway vector to be created, and may affect command velocity attainment, as mergeSet gives up its attempts to allocate phasors. */
#define AS_ERR_MERGE_OFFSETLIM 64u /* Not really an error, as it will almost always be turned on when using a non-collapse shaper, that must be reflected instead. Relevant for diagnose, however. */
#define AS_ERR_OTHER 128u /* Other object disabled. Auto only. */

#define AS_ERR_MINOR 255u /* Maximum value of errstatus for non-crucial error */

/* "Internal" crucial error flags */
#define AS_ERR_DISABLED 256u 
#define AS_ERR_WASENABLED 512u 

/* Crucial errors that will cause the disabling of the object */
#define AS_ERR_UCOM 1024u /* Non-valid command velocity input. Manual only. */
#define AS_ERR_XCOM 2048u /* Non-valid command position input. Auto only. */
#define AS_ERR_MODE 4096u /* Non-valid antisway mode */
#define AS_ERR_LSIGN 8192u /* Zero or negative pendulum length */
#define AS_ERR_AMAX 16384u /* amaxH output violation */
#define AS_ERR_UMAX 32768u /* umaxH*limfact output violation */
#define AS_ERR_XLIM 65536u /* xmax or xmin output violation. Auto only. */
#define AS_ERR_XMAX 65536u
//#define AS_ERR_UMAXL 131072u /* umaxL input violation. Auto only */

#define AS_NERR_FLAGS 18 //Number of error flags


/* 3.1 Degrees of verbosity */

#define AS_VERB_DISABLED 1 /* Object will print message if disabled */
#define AS_VERB_ERR 2 /* Object will print message if any error occurs */
#define AS_VERB_ALL 3 /* Object will print message when anything new happens */


/* 3.2 Compensation degrees in automatic mode */

#define AS_AUTO_COMP_L 1 /* Compensate for new L during auto travel */
#define AS_AUTO_COMP_DL 2 /* Compensate for hoisting (DLc != 0) during auto travel */



/*3.3 Alarm and message parameters */

#define AS_ALARMTYPE_DISABLED 0
#define AS_ALARMTYPE_ERR 1
//#define AS_ALARM_WAIT 10
//#define AS_ALARM_HOLD 50
#define AS_OBJ_SETALARM(i) {          \
 if (i == 0)                          \
   object->Alarm1 = 1;                \
 else                                 \
   object->Alarm2 = 1;                \
}                                     \

#define AS_OBJ_RESETALARM(i) {        \
 if (i == 0)                          \
   object->Alarm1 = 0;                \
 else                                 \
   object->Alarm2 = 0;                \
}                                     \


/* 4. Some max and min limits. These have been made an internal array in the object for greater flexibility. */

#define AS_OBJ_UR_ZEROLIM (object->limits[0]) /* Default 0.001. Will set uR to zero if |uR| < limit */
#define AS_OBJ_UR_MAX_ERRFACT (object->limits[1]) /* Default 2.0. Will disable object if |uR|>limit*umaxH */
#define AS_OBJ_UR_MIN (object->limits[2]) /* Default 0.01. Minimum steady reference velocity that can be passed to frequency converter */
#define AS_OBJ_XMAX (object->limits[3]) /* Default 99999.0. Maximum reference position. */
#define AS_OBJ_XMIN (object->limits[4]) /* Default -99999.0. Minimum reference position. */
#define AS_OBJ_UR_ATTAIN_ERRLIM (object->limits[5]) /* Default 0.001. Will warn if final uR misses uCommand (or zero if auto) with more than limit */
#define AS_OBJ_L_MIN_CHANGE (object->limits[6]) /* Default 0.005. Length interval between compensation for new pendulum length = 5mm. */
#define AS_OBJ_DL_MIN (object->limits[7]) /* Default 0.005. Minimum hoisting speed that will be compensated for = 5 mm/s. */
#define AS_OBJ_XR_ATTAIN_ERRLIM (object->limits[8]) /* Default 0.001. Will warn if final xR misses xCommand with more than limit. Auto only. */
#define AS_OBJ_XCOM_MIN_CHANGE (object->limits[9]) /* Default 0.01. Minimum change in command position that will invoke a new call. Auto only. */

/* The following limits are of more internal character, and should probably not be made object attributes */
#define AS_COLLAPSE_LIM 100 /* Will collapse sets generated with non-collapse shapers if # of phasors exceeds limit */
#define AS_MERGE_ITER_LIM 500 /* Will exit mergeSet while loop if # of iterations exceeds limit */
#define AS_UAUTO_INCREMENT 0.01 /* Relative increment of uCommand in makeSet_auto for-loop.  - **** possibly include this in limits attribute instead. */ 
#define AS_AUTO_MATCH_LIM 0.1 /*Travel time difference between objects that makeSet_auto will tolerate. - **** possibly include this in limits attribute instead. */
//#define AS_MESSAGE_LENGTH 79 /*length of the message internal attribute. Last character is '\0', and is not included (i.e. AS_MESSAGE_LENGTH = 79 for a String80).*/
//#define AS_MESSAGE_NLIMIT 100 /* max No of messages in queue */

/* 5. Readability macros */

#define AS_OBJ_SETP ((AS_phasorSet *) &(object->Set))
#define AS_OBJ_AMAXHM (object->amax[0])
#define AS_OBJ_AMAXSM (object->amax[1])
#define AS_OBJ_UMAXHM (object->umax[0])
#define AS_OBJ_UMAXSM (object->umax[1])
#define AS_OBJ_AMAXHA (object->amax[2])
#define AS_OBJ_AMAXSA (object->amax[3])
#define AS_OBJ_UMAXHA (object->umax[2])
#define AS_OBJ_UMAXSA (object->umax2 ? object->umax[4] : object->umax[3])
#define AS_OBJ_AMAXHMA (object->manual ? object->amax[0] : object->amax[2])
#define AS_OBJ_UMAXHMA (object->manual ? object->umax[0] : object->umax[2])
#define AS_OBJ_XR (object->xR[0])
#define AS_OBJ_XRM (object->xR[1])
#define AS_OBJ_THR (object->thR[0])
#define AS_OBJ_THRM (object->thR[1])
#define AS_OBJ_M_LENGTH (object->mparams[0]) //Default 79. Maximum number of characters in message string.
#define AS_OBJ_M_NLIMIT (object->mparams[1]) //Default 80. Maximum numbber of messages in queue.
#define AS_OBJ_M_ALWAIT (object->mparams[2]) //Default 20. Wait this number of cycles before taking next message from queue.
#define AS_OBJ_M_ALHOLD (object->mparams[3]) //Default 30. Hold the current message this number of cycles.
#define AS_OBJ_MESSAGEQP(i) ((AS_messageQ *) object->messageQ[(i)])
#define AS_OBJ_MESSAGEQPP(i) ((AS_messageQ **) &object->messageQ[(i)])
#define AS_ADDMESSAGE(type, charpp, messageArgs...) {                              \
  if (asprintf(charpp, messageArgs) < 0) { /* ! a malloc() failure */              \
    fprintf(stderr, "AS/ADDMESSAGE/asprintf: Could not allocate memory!\n");       \
    exit(99);                                                                      \
  }                                                                                \
  AS_enQMessage(AS_OBJ_MESSAGEQPP(type), AS_OBJ_M_NLIMIT, *charpp);                                 \
  free(*charpp);                                                                   \
}                                                                                  \

/* Access attributes of other antisway object */
#define AS_OBJ_OTHER ((pwr_sClass_Ssab_AntiSway *) (object->other))
#define AS_OBJ_OTHER_SETP ((AS_phasorSet *) &(((pwr_sClass_Ssab_AntiSway *) (object->other))->Set))

#define AS_OTHER_SETP ((AS_phasorSet *) &(other->Set))
 #define AS_OTHER_AMAXHM (other->amax[0])
#define AS_OTHER_AMAXSM (other->amax[1])
#define AS_OTHER_UMAXHM (other->umax[0])
#define AS_OTHER_UMAXSM (other->umax[1])
#define AS_OTHER_AMAXHA (other->amax[2])
#define AS_OTHER_AMAXSA (other->amax[3])
#define AS_OTHER_UMAXHA (other->umax[2])
#define AS_OTHER_UMAXSA (other->umax2 ? other->umax[4] : other->umax[3])
#define AS_OTHER_UR_MIN (other->limits[2])
#define AS_OTHER_XR (other->xR[0])
#define AS_OTHER_XRM (other->xR[1])
#define AS_OTHER_THR (other->thR[0])
#define AS_OTHER_THRM (other->thR[1])

/* More readability */
#define AS_SHP_COLLAPSEOK ( ((shp->ID == AS_NO_AS) || (shp->ID == AS_DPULSE)) || ((shp->ID == AS_UMZV) || (shp->ID == AS_UMZVD)) )
#define AS_SHP_NEG ( ((shp->ID == AS_UMZV) || (shp->ID == AS_UMZVD)) || ((shp->ID == AS_ZV122) || (shp->ID == AS_ZVD122)) )
#define AS_SHP_ROBUST ( (shp->ID == AS_DDPULSE) || ((shp->ID == AS_UMZVD) || (shp->ID == AS_ZVD122)) )


/* 6. Auto status mask codes */

#define AS_AUTO_ON 1u  /* object in auto mode */
#define AS_AUTO_2D 2u /* Set/reset by exec if other is not/is a null pointer. Used in Engine_auto when making new sets. */
#define AS_AUTO_NEEDMATCH 4u /*Set by object when set is made upon new auto call. Reset by other object after possible matching try */
#define AS_AUTO_NORESET 7u
//8u
#define AS_AUTO_REMADE_SHP 16u /* amax exceeded in makeSet_auto. Retried with different shaper. */
#define AS_AUTO_REMADE_OVERLAP 32u /* amax or umax exceeded in makeSet_auto. Retried without overlap */
#define AS_AUTO_REMADE_MATCH 64u /* umin not exceeded in makeSet_auto. Retried without matching extension. */
#define AS_AUTO_AMAX_FAILURE 128u /* makeSet_auto could not allocate set without exceeding amax at some point. */
#define AS_AUTO_UMIN_FAILURE 256u /* makeSet_auto could not allocate set that would exceed umin */
#define AS_AUTO_UMAX_FAILURE 512u /* makeSet_auto could not allocate set without exceeding umax at some point. */
#define AS_AUTO_POSEXT_FAILURE 1024u /* makeSet_auto could not find a uCmd that would yield a negative phase (positive time) for deceleration. */


#define AS_NAUTO_FLAGS 11 //Number of autostatus flags

/* 7. Antisway mode values */

#define AS_NO_AS 0 /* No antisway */
#define AS_DPULSE 1 /* Double pulse */	
#define AS_DDPULSE 2 /* Robust, or double double pulse */
#define AS_UMZV 3 /* Unity-magnitude negative ZV, or triple pulse */
#define AS_UMZVD 4 /* Unity-magnitude negative ZVS, robust variant of triple pulse above */
#define AS_ZV122 5 /* Time-optimal negative ZV */
#define AS_ZVD122 6 /* Time-optimal negative ZVD, robust variant of ZV122. */


/* 8. Structures */

#define AS_ISTMAX 5 /* Max # of pulses in shaper */

#ifndef ssabox_as_phasor_structs
#define ssabox_as_phasor_structs 

/* struct AS_shaper: 
 * 
 *    Defines an Impulse Shaping Technique (a shaper). Members are:
 *
 *    int ID          ID number of the shaper as defined by macros AS_NO_AS, AS_DPULSE etc
 *    int N           Number of pulses in the shaper. Room for AS_ISTMAX pulses 
 *    double A[]      Array containing the relative magnitudes of the pulses (signed). 
 *                    Magnitude +/-1.0 corresponds to maximum acceleration. 
 *    double ASum     The sum of the relative magnitudes.
 *    double AMin     Minimum (relative) amplitude of pulses. Absolute value.
 *    double AMax     Maximum (relative) amplitude of pulses. Absolute value.
 *    double phi[]    Array containing the phase locations of the pulses.
 *    double DMin     Minimum phase distance between pulses. Absolute value.
 *
 *    
 * struct AS_phasor:
 *    
 *    Keeps the necessary information about a phasor. Members are:
 *
 *    double R         Magnitude of the total phasor. Signed. 
 *    double Phi       Phase of the total phasor. Negative phase corresponds to a future time.   
 *    double phiStart  Phase starting point (corresponds to starting point in time).
 *    double phiEnd    Phase end point (corresponds to end point in time).
 *    double Sa        The constant acceleration of the pulse. Signed opposite to R.
 * 
 *
 * struct AS_phasorSet:
 * 
 *    Contains all current phasors. Members are:
 *
 *    int N            Number of phasors in the set
 *    double extSum    Acceleration-weighted phase extension sum. extSum = sum (|phiEnd - phiStart|*Sa), 
 *                     where the sum is taken over all the phasors in the set. Positive for positive acc. 
 *		       Usage: Divide by omega to get the velocity change of the set.
 *    AS_phasor *ph    Pointer to the array of phasors. Memory is allocated dynamically.
 *
 */


typedef struct {
  int ID;
  int N;
  double A[AS_ISTMAX]; 
  double ASum; 
  double AMin;
  double AMax;
  double phi[AS_ISTMAX]; 
  double DMin; 
} AS_shaper;

typedef struct {
  double R; 
  double Phi; 
  double phiStart; 
  double phiEnd; 
  double Sa; 
} AS_phasor;

typedef struct {
  int N; 
  double extSum; 
  AS_phasor *ph; 
} AS_phasorSet;

typedef struct AS_messageQ {
  char *mess;
  int count;
  struct AS_messageQ *next;
} AS_messageQ;

#endif

/* 9. Prototype declarations of visible phasor methods */

unsigned int AS_Engine_man( int newCall, int hoisting, int hoisted, int verbose, const AS_shaper *shp,
			    double uCommand, double uR, double DLc, double omega, double amaxH, double amaxS, double dt, 
			    double *aRp, AS_phasorSet *Setp );

unsigned int AS_Engine_auto( int newCall, int hoisting, int hoisted, int verbose, pwr_sClass_Ssab_AntiSway *other, const AS_shaper *shp,
			     double xCommand, double xR, double uR, double DLc, double omega, double amaxH, double amaxS, double umaxH, double umaxS, double umin, double dt,
			     double *aRp, unsigned int *astsp, AS_phasorSet *Setp );

void AS_collapseSet(AS_phasorSet *Setp, const AS_shaper *shp, double amax);

void AS_displaySet(const AS_phasorSet *Setp, double omega); //mainly for diagnose

void AS_displayQ(const double *acp, const double *aEp, double uc, double dt); //obsolete

void AS_emptySet(AS_phasorSet *Setp);

void AS_emptyQ(double **aQpp, double **acpp, double **aEpp); //obsolete

int AS_cmpPhasors(const void *p1, const void *p2);

void AS_enQMessage(AS_messageQ **objectmQp, int nlimit, const char *newMessage);

void AS_deQMessage(AS_messageQ **objectmQp);

void AS_diagnose(const AS_phasorSet *Setp,double omega, int reflected,double SuDiff, double uDiff);

const AS_shaper* AS_SetupIST(int mode);


#endif
