/* 
 * Proview   $Id: pb_conf.h,v 1.4 2007-01-17 12:40:30 claes Exp $
 * 
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
/*****************************************************************************/
/*                                                                           */
/*                                SOFTING AG                                 */
/*                        Richard-Reitzner-Allee 6                           */
/*                              D-85540 Haar                                 */
/*                      Phone: (++49)-(0)89-45656-0                          */
/*                      Fax:   (++49)-(0)89-45656-399                        */
/*                                                                           */
/*                    Copyright (C) SOFTING AG 1995-2003                     */
/*                            All Rights Reserved                            */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*           PROFIBUS CONFIGURATION AND IMPLEMENTATION DEFINITIONS           */
/*                                                                           */
/*  Filename    : PB_CONF.H                                                  */
/*  Version     : 5.26.0.00.release                                          */
/*  Date        : 14-February-2002                                           */
/*  Author      : SOFTING AG                                                 */
/*                                                                           */
/*  Description : This file contains the PROFIBUS configuration and          */
/*                implementation definitions using Windows ME/98/95 or       */
/*                Windows XP / 2000 / NT operating system                    */
/*                                                                           */
/*****************************************************************************/

#ifndef __PB_CONF__
#define __PB_CONF__

/*****************************************************************************/
/* COMPILER SWITCHES FOR COMPABLITY                                          */
/*****************************************************************************/


#ifndef PB_VER
#define PB_VER  526
#endif


/*****************************************************************************/
/* COMPILER SWITCHES DEPENDING ON OPERATING SYSTEM                           */
/*****************************************************************************/

#undef FAR
#undef HUGE

#if defined (WIN32) || defined (_WIN32)                /* compiling as WIN32 */
   #undef  DOS16
   #undef  WIN32
   #undef  EXPORT
   #define WIN32
   #define EXPORT  __export
   #define FAR
   #define HUGE
   #define CALL_CONV  APIENTRY                /* call convention using WIN32 */
   #ifdef PB_API_FUNC_NOT_USED
      #undef  CALL_CONV
      #define CALL_CONV                      /* call convention using NT-DDK */
   #endif
#elif defined (_LINUX)                                 /* compiling as LINUX */
   #undef  DOS16
   #undef  WIN32
   #undef  EXPORT
   #define EXPORT 
   #define FAR
   #define HUGE
   #define CALL_CONV
   #define CALLBACK
   #define INVALID_HANDLE_VALUE ((HANDLE) NULL)
   #define  getch()             getchar()
   typedef int   HANDLE;
   typedef unsigned long   DWORD;
   #ifdef PB_API_FUNC_NOT_USED
      #undef  CALL_CONV
      #define CALL_CONV                      
   #endif
#else                                                  /* compiling as WIN16 */
   #if defined (WIN_DLL) || defined (_WINDOWS) || defined (_WINDLL)
       #undef  DOS16
       #undef  WIN16
       #define WIN16
   #endif

   #if defined (WIN16) || defined (_WIN16)
       #undef  DOS16
       #undef  WIN16
       #undef  EXPORT
       #undef  PASCAL
       #define WIN16
       #define EXPORT  __export
       #define FAR      _far
       #define PASCAL   _pascal
       #define CDECL    _cdecl
       #define HUGE   _huge
       #define CALL_CONV FAR pascal         /* calling convention using WIN16 */
   #else
       #error 16-BIT DOS compilation not supported !!!!
   #endif
#endif


/*****************************************************************************/
/*              SUPPORTED SERVICES                                           */
/*****************************************************************************/
#define FMS_SERVICES_SUPPORTED
#define DP_SERVICES_SUPPORTED
#define DPS_SERVICES_SUPPORTED


/*****************************************************************************/
/*                        Implementation Constants                           */
/*                                                                           */
/* The constants given below define the sizes of various data structures in  */
/* the protocol software and thus influence memory consumption.              */
/*                                                                           */
/* NOTE: Do not change the following constants without recompiling the       */
/*       the protocol software on the communication controller               */
/*****************************************************************************/

#define VERSION_STRING_LENGTH        100  /* length of version string buffer */

/* -- constants of internal sizes of byte arrays --------------------------- */

#define VFD_STRING_LENGTH             32     /* max length of the VFD string */
#define IDENT_STRING_LENGTH           32   /* max length of the Ident string */

#define ACCESS_NAME_LENGTH            32    /* max length for name adressing */
#define OBJECT_NAME_LENGTH            32        /* max length of object name */
#define EXTENSION_LENGTH              32   /* max length of object extension */
#define EXECUTION_ARGUMENT_LENGTH     32     /* max length of exec. argument */
#define ERROR_DESCR_LENGTH            32     /* max length of error descript.*/
#define CRL_SYMBOL_LENGTH             32    /* max length of crl symbol name */
#define CRL_EXTENSION_LENGTH           2      /* max length of crl extension */

#if (PB_VER < 500)
#define KBL_SYMBOL_LENGTH             CRL_SYMBOL_LENGTH
#define KBL_EXTENSION_LENGTH          CRL_EXTENSION_LENGTH
#endif

#define MAX_FMS_PDU_LENGTH      241    /* max size of the FMS/FM7-PDU-Buffer */
#define MAX_VAR_LIST_ELEMENTS    50   /* max count of variable list elements */
#define MAX_DOM_LIST_ELEMENTS    50     /* max count of domain list elements */
#define MAX_VAR_RECORD_ELEMENTS  10          /* max count of record elements */

#define MAX_COMREF            64   /* max supported communication references */
#define MAX_VFD                5                       /* max supported VFDs */

#if (PB_VER < 500)
#define MAX_KBL_LEN            MAX_COMREF              /* max entries in CRL */
#define MAX_PARA_LOC_SERVICES  5           /* max parallel local FMS-Services*/
#endif

/*****************************************************************************/
/* USEFUL MACROS                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* MACRO TO CALCULATE MAX_xxxx_NAME_LENGTH                                   */
/*                                                                           */
/* This macro calculates the internal sizes of byte arrays in a way that the */
/* desired alignment on byte, word or long word boundaries is achieved.      */
/* The alignment is specified by the constant ALIGNMENT (e. g. longword = 4) */
/*                                                                           */
/*****************************************************************************/

#define ALIGNMENT                  0x02        /* alignment on word boundary */

#define _NAME_LENGTH(length) ((length) + ALIGNMENT - ((length) % ALIGNMENT))

#endif
