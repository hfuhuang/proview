/* 
 * Proview   $Id: op_appltable.c,v 1.2 2005-09-01 14:57:49 claes Exp $
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

/* op_appltable.c -- <short description>

   This file contains the initializations for the different
   applstates.  */

#include "pwr.h"
#include "op_appltable.h"

void appltab_Init (
    APPLTABLESTRUCT **ApplTablePP,
    int *NoInApplTableP
)
{
    static APPLTABLESTRUCT ApplTable[2];
    int I=0;

    *ApplTablePP = ApplTable;
    *NoInApplTableP = 0;
  
}
