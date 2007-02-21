/* 
 * Proview   $Id: rt_gdhget.c,v 1.1 2007-02-21 10:00:48 claes Exp $
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


#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "pwr.h"
#include "co_cdh.h"
#include "rt_gdh.h"

void usage()
{
  printf( 
"\n\
  rt_gdhget   Print specified attribute value on stdout\n\n\
   rg_gdhget [-c 'class'] 'attribute'\n\n\
   -c   Find the first object of the specified class and print the\n\
        specified attribute.\n\
");
}

int main (int argc, char **argv)
{
  pwr_tStatus sts;
  char *c;
  int cidopt = 0;
  pwr_tObjName cidstr;
  pwr_tAName astr;
  int i;

  if ( argc <= 1) {
    usage();
    exit(1);
  }

  sts = gdh_Init("rt_gdhget");
  if ( EVEN(sts)) {
    exit(sts);
  }

  for ( i = 1; i < argc; i++) {
    c = argv[i];
    if ( *c == '-') {
      c++;
      switch ( *c) {
      case 'h':
	usage();
	exit(0);
     
      case 'c':
	if ( argc <= i+1) {
	  usage();
	  exit(1);
	}
	strncpy( cidstr, argv[i+1], sizeof(cidstr));
	cidopt = 1;
	i++;
	break;
      }
    }
    else
      strcpy( astr, argv[i]);
  }
  
  if ( cidopt) {
    // Get the first object of class cidstr, and print the value of attribute 
    // astr in this object

    pwr_tCid cid;
    pwr_tOid oid;
    pwr_tAttrRef aref, aaref;
    pwr_tTid a_tid;
    unsigned int a_size, a_offs, a_dim;
    void *a_valp;
    char str[256];

    sts = gdh_ClassNameToId( cidstr, &cid);
    if ( EVEN(sts)) {
      exit(sts);
    }

    sts = gdh_GetClassList( cid, &oid);
    if ( EVEN(sts)) {
      exit(sts);
    }

    aref = cdh_ObjidToAref( oid);
    sts = gdh_ArefANameToAref( &aref, astr, &aaref);
    if ( EVEN(sts)) {
      exit(sts);
    }

    sts = gdh_GetAttributeCharAttrref( &aaref, &a_tid, &a_size, &a_offs, &a_dim);
    if ( EVEN(sts)) {
      exit(sts);
    }
    a_valp = calloc( 1, a_size);
    sts = gdh_GetObjectInfoAttrref( &aaref, a_valp, a_size);
    if ( EVEN(sts)) {
      free( a_valp);
      exit(sts);
    }
    
    sts = cdh_AttrValueToString( a_tid, a_valp, str, sizeof(str));
    if ( EVEN(sts)) {
      free( a_valp);
      exit(sts);
    }

    printf( "%s\n", str);
    free( a_valp);

    exit(0);

  }
  else {
    // Print the value of the attriute in astr

    pwr_tTypeId a_tid;
    pwr_tUInt32 a_size, a_offs, a_elem;
    void *a_valp;
    char str[256];
    
    sts = gdh_GetAttributeCharacteristics( astr,
					 &a_tid, &a_size, &a_offs, &a_elem);
    if ( EVEN(sts)) {
      exit(sts);
    }
    a_valp = calloc( 1, a_size);

    sts = gdh_GetObjectInfo( astr, a_valp, a_size);
    if ( EVEN(sts)) {
      free( a_valp);
      exit(sts);
    }
    
    sts = cdh_AttrValueToString( a_tid, a_valp, str, sizeof(str));
    if ( EVEN(sts)) {
      free( a_valp);
      exit(sts);
    }

    printf( "%s\n", str);
    free( a_valp);

    exit(0);
  }    
  exit(1);
}





