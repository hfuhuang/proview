/* 
 * Proview   $Id: rt_plc_macro_mps.h,v 1.2 2005-09-01 14:57:56 claes Exp $
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

/*	MPSTRPTRIGG							*/
/*	funktion:	trigger mps transport object			*/
/*									*/
#define mpsTriggtrp_exec(obj,in)					\
  obj->InTriggFlag = in

/*	MPSTRPTRIGG							*/
/*	funktion:	trigger mps transport object			*/
/*									*/
#define mpsTrp_exec(obj)						\
  if ( obj->InTriggFlag && !obj->InTriggOld )				\
    obj->TriggFlag = TRUE;						\
  obj->InTriggOld = obj->InTriggFlag

