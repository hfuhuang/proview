/* 
 * Proview   $Id: wb.h,v 1.12 2007-01-04 07:29:02 claes Exp $
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

#ifndef wb_h
#define wb_h

/* wb.h -- work bench */

#if defined __cplusplus
extern "C" {
#endif

#define WB_CLASS_NAME "PWR_DEV"

typedef enum {
	wb_eType_Volume,
	wb_eType_Directory,
	wb_eType_Class,
	wb_eType_Buffer,
	wb_eType_ClassEditor,
	wb_eType_ExternVolume
} wb_eType;


#if defined __cplusplus
}
#endif
#endif




