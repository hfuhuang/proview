/* 
 * Proview   $Id: rt_plc_macro_string.h,v 1.4 2005-09-01 14:57:56 claes Exp $
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

/*		 Preprocessor routines for string operations	    */

/*_*
  STOSV
  store string value			
  @aref stosv StoSv
*/
#define stosv_exec(obj,in) \
  strncpy( obj->ActualValue, in, sizeof(obj->ActualValue)); \
  obj->ActualValue[sizeof(obj->ActualValue)-1] = 0;

/*_*
  CSTOSV								
  store conditionally into string value		
  @aref cstosv CStoSv
*/
#define cstosv_exec(obj,in,cond) \
  if ( cond ) { \
    strncpy( obj->ActualValue, in, sizeof(obj->ActualValue)); \
    obj->ActualValue[sizeof(obj->ActualValue)-1] = 0; \
  }

/*_*
  STRCAT
  @aref strcat Strcat
*/
#define Strcat_exec(obj,str1,str2) \
  strncpy(obj->ActVal, str1, sizeof(obj->ActVal)); \
  if ( strlen(str1) < sizeof(obj->ActVal)) \
    strncat(obj->ActVal, str2, sizeof(obj->ActVal)-strlen(str1)); \
  obj->ActVal[sizeof(obj->ActVal)-1] = 0;

/*_*
  ATOSTR
  @aref atostr AtoStr
*/
#define AtoStr_exec(obj,in) \
  sprintf(obj->ActVal, obj->Format, in);

/*_*
  DTOSTR
  @aref dtostr DtoStr
*/
#define DtoStr_exec(obj,in) \
  sprintf(obj->ActVal, obj->Format, in);

/*_*
  ITOSTR
  @aref itostr ItoStr
*/
#define ItoStr_exec(obj,in) \
  sprintf(obj->ActVal, obj->Format, in);

/*_*
  STOSP								
  Store into string attribute			
  @aref stosp StoSp
*/
#define stosp_exec(ut,in,size) \
  strncpy( ut, in, size); \
  ut[size-1] = 0;

/*_*
  CSTOSP
  Store conditionally into string attribute	
  @aref cstosp CStoSp
*/
#define cstosp_exec(ut,in,cond,size) \
  if ( cond) { \
    strncpy( ut, in, size); \
    ut[size-1] = 0; \
  }

/*_*
  STONUMSP								
  Store a number of characters into string attribute			
  @aref stonumsp StoNumSp
*/
#define stonumsp_exec(ut,in,size,num) \
  strncpy( ut, in, num < size ? num : size); \
  ut[size-1] = 0;

/*_*
  CSTONUMSP
  Store conditionally a number of characters into string attribute	
  @aref cstonumsp CStoNumSp
*/
#define cstonumsp_exec(ut,in,cond,size, num) \
  if ( cond) { \
    strncpy( ut, in, num < size ? num : size); \
    ut[size-1] = 0; \
  }

/*_*
  SUBSTR
  @aref substr SubStr
*/
#define SubStr_exec(obj,in) \
  if ( obj->Start < strlen(in)) { \
    strncpy( obj->ActVal, &in[obj->Start], \
	     MIN(obj->Length,sizeof(obj->ActVal))); \
    obj->ActVal[MIN(obj->Length,sizeof(obj->ActVal)-1)] = 0; \
  } \
  else \
    strcpy( obj->ActVal, "");










