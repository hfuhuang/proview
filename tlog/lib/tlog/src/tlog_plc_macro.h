/* 
 * Proview   $Id: tlog_plc_macro.h,v 1.1 2006-01-12 06:41:28 claes Exp $
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

/*		 PREPROCESSOR RUTINER					    */


/*	DtLog							*/
/*	Function:	Test Logging of Digital Value		*/
/*								*/
#define DtLog_exec( object, in)					\
  if ( in && !object->InOld && !object->UpSignal)		\
  {								\
    object->UpSignal = 1;					\
    clock_gettime( CLOCK_REALTIME, &object->UpTime);		\
  }								\
  if ( !in && object->InOld && !object->DownSignal)		\
  {								\
    object->DownSignal = 1;					\
    clock_gettime( CLOCK_REALTIME, &object->DownTime);		\
  }								\
  object->InOld = in;

/*	AtLog							*/
/*	Function:	Test Logging of Analog Value		*/
/*								*/
#define AtLog_exec( object, in)					\
  if ( in >= object->Limit && object->InOld < object->Limit &&	\
	!object->UpSignal)					\
  {								\
    object->UpSignal = 1;					\
    clock_gettime( CLOCK_REALTIME, &object->UpTime);		\
  }								\
  if ( in <= object->Limit && object->InOld > object->Limit && \
	!object->DownSignal)					\
  {								\
    object->DownSignal = 1;					\
    clock_gettime( CLOCK_REALTIME, &object->DownTime);		\
  }								\
  object->InOld = in;

/*	TLogOpen						*/
/*	Function:	Open and close Test Logging File	*/
/*								*/
#define TLogOpen_exec( object, in)				\
  if ( in && !object->InOld && !object->OpenSignal)		\
    object->OpenSignal = 1;					\
  else if ( !in && object->InOld && !object->CloseSignal)	\
  {								\
    object->CloseSignal = 1;					\
    clock_gettime( CLOCK_REALTIME, &object->CloseTime);		\
  }								\
  if ( object->FileOpen && !object->FileOpenOld)		\
    clock_gettime( CLOCK_REALTIME, &object->OpenTime);		\
  object->InOld = in;						\
  object->FileOpenOld = object->FileOpen;

/*	TLogModify						*/
/*	Function:	Modify selectlists for Test Logging File */
/*								*/
#define TLogModify_exec( object, in)				\
  if ( in && !object->InOld && !object->ModifySignal)		\
  {								\
    object->ModifySignal = 1;					\
    clock_gettime( CLOCK_REALTIME, &object->ModifyTime);	\
  }								\
  else if ( !in && object->InOld && !object->UnModifySignal)	\
  {								\
    object->UnModifySignal = 1;					\
    clock_gettime( CLOCK_REALTIME, &object->UnModifyTime);	\
  }								\
  object->InOld = in;

/*	CDtLog							*/
/*	Function:	Conditional Test Logging of Digital 	*/
/*			Value					*/
/*								*/
#define CDtLog_exec( object, in, cond)				\
  if ( cond && !object->CondOld && !object->LogSignal)		\
  {								\
    object->LogSignal = 1;					\
    object->LogValue = in;					\
    clock_gettime( CLOCK_REALTIME, &object->LogTime);		\
  }								\
  object->CondOld = cond;

/*	CAtLog							*/
/*	Function:	Conditional Test Logging of Analog 	*/
/*			Value					*/
/*								*/
#define CAtLog_exec( object, in, cond)				\
  if ( cond && !object->CondOld && !object->LogSignal)		\
  {								\
    object->LogSignal = 1;					\
    object->LogValue = in;					\
    clock_gettime( CLOCK_REALTIME, &object->LogTime);		\
  }								\
  object->CondOld = cond;

/*	ChgAtLog						*/
/*	Function:	Chg Test Logging of Analog 		*/
/*			Value					*/
/*								*/
#define ChgAtLog_exec( object, in)				\
  if ( object->InOld != in && !object->LogSignal)		\
  {								\
    object->LogSignal = 1;					\
    object->LogValue = in;					\
    clock_gettime( CLOCK_REALTIME, &object->LogTime);		\
  }								\
  object->InOld = in;

/*	COtLog							*/
/*	Function:	Conditional Test Logging of an Objid 	*/
/*			Value					*/
/*								*/
#define COtLog_exec( object, in, cond)				\
  if ( cond && !object->CondOld && !object->LogSignal)		\
  {								\
    object->LogSignal = 1;					\
    object->LogValue = in;					\
    clock_gettime( CLOCK_REALTIME, &object->LogTime);		\
  }								\
  object->CondOld = cond;

/*	ChgOtLog						*/
/*	Function:	Test Logging of an Objid 		*/
/*			Value when value is changed		*/
/*								*/
#define ChgOtLog_exec( object, in)				\
  if ( cdh_ObjidIsNotEqual( object->InOld, in) && !object->LogSignal)\
  {								\
    object->LogSignal = 1;					\
    object->LogValue = in;					\
    clock_gettime( CLOCK_REALTIME, &object->LogTime);		\
  }								\
  object->InOld = in;

/*	ExecOn							*/
/*	Function:	Turn exectution of all plcpgm's on	*/
/*								*/
#define TLogExecAllOn_exec( object, in)\
  if ( in && !object->InOld && !object->ExecOnSignal)\
  {\
    object->ExecOnSignal = 1;\
    clock_gettime( CLOCK_REALTIME, &object->LogTime);\
  }\
  if ( object->ExecOn && object->ExecOnOld )\
    object->ExecOn = 0;\
  object->ExecOnOld = object->ExecOn;\
  object->InOld = in;

/*	ExecOff							*/
/*	Function:	Turn exectution of all plcpgm's off	*/
/*								*/
#define TLogExecAllOff_exec( object, in)\
  if ( in && !object->InOld && !object->ExecOffSignal)\
  {\
    object->ExecOffSignal = 1;\
    clock_gettime( CLOCK_REALTIME, &object->LogTime);\
  }\
  if ( object->ExecOff && object->ExecOffOld )\
    object->ExecOff = 0;\
  object->ExecOffOld = object->ExecOff;\
  object->InOld = in;

/*	ExecPlcOn						*/
/*	Function:	Turn exectution of one plcpgm's on	*/
/*								*/
#define ExecPlcOn_exec( object, in, scanoff)\
  object->SetOn = 0;\
  if ( in && !object->InOld)\
  {\
    scanoff = 0;\
    object->SetOn = 1;\
  }\
  object->InOld = in;

/*	ExecPlcOff						*/
/*	Function:	Turn exectution of one plcpgm's off	*/
/*								*/
#define ExecPlcOff_exec( object, in, scanoff)\
  object->SetOff = 0;\
  if ( in && !object->InOld)\
  {\
    scanoff = 1;\
    object->SetOff = 1;\
  }\
  object->InOld = in;

