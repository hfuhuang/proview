/* 
 * Proview   $Id: Papi.c,v 1.2 2007-01-17 12:40:30 claes Exp $
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
/*******************************************************************************
*                                                                             *
*                                 SOFTING AG                                  *
*                          Richard-Reitzner-Allee 6                           *
*                                D-85540 Haar                                 *
*                        Phone: (++49)-(0)89-45656-0                          *
*                        Fax:   (++49)-(0)89-45656-399                        *
*                                                                             *
*                      Copyright (C) SOFTING AG 1995-2003                     *
*                              All Rights Reserved                            *
*                                                                             *
*******************************************************************************

FILE_NAME               PAPI.C

PROJECT_NAME            PROFIBUS

MODULE                  PAPI

COMPONENT_LIBRARY       PAPI Lib
                        PAPI DLL

AUTHOR                  SOFTING AG

VERSION                 5.26.1.00.release

DATE                    27-June-2003

STATUS                  finished


FUNCTIONAL_MODULE_DESCRIPTION


PROFIBUS FMS/DPV1 Master and DP-Slave Application Program Interface (PAPI) for WinXP/Win2K/WinNT
------------------------------------------------------------------------------------------------

This modul contains the PROFIBUS FMS/DPV1 Master and DP-Slave Application Program Interface
functions.



RELATED_DOCUMENTS
=============================================================================*/
#include "keywords.h"

INCLUDES

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
#include <windows.h>
#include <winioctl.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "pb_type.h"

#if defined (WIN32) || defined (_WIN32)
#include "pb_ntdrv.h"
#endif

#ifdef _LINUX
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pb_ioctl.h"
#endif

#include "pb_err.h"
#include "pb_if.h"
#include "pb_fms.h"
#include "pb_fm7.h"
#include "pb_fmb.h"
#include "version.h"

GLOBAL_DEFINES

LOCAL_DEFINES


EXPORT_TYPEDEFS

LOCAL_TYPEDEFS

FUNCTION_DECLARATIONS

#if defined (WIN32) || defined (_WIN32)
extern BOOL    ReadBoardRegistryEntries(USIGN8,USIGN32*);
#endif

extern USIGN16 swap_16_intel_motorola(USIGN16);
extern USIGN32 swap_32_intel_motorola(USIGN32);

extern INT16   dpgdl_get_data_len (INT16,USIGN8,USIGN8,USIGN8*,INT16*);
extern INT16   dpsgdl_get_data_len(INT16,USIGN8,USIGN8,USIGN8*,INT16*);
extern INT16   fdlgdl_get_data_len(INT16,USIGN8,USIGN8,USIGN8*,INT16*);
extern INT16   fm7gdl_get_data_len(INT16,USIGN8,USIGN8,USIGN8*,INT16*);
extern INT16   fmsgdl_get_data_len(INT16,USIGN8,USIGN8,USIGN8*,INT16*);
extern INT16   fmbgdl_get_data_len(INT16,USIGN8,USIGN8,USIGN8*,INT16*);


EXPORT_DATA

IMPORT_DATA

LOCAL_DATA

// --- copyright
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
static const char copyright[] = "PROFIBUS WinXP/Win2K/WinNT API (c) Copyright 1995-2005. SOFTING AG. All Rights Reserved.";

// --- Operation Mode
static USIGN32 OperationMode;

#endif

#ifdef _LINUX
static const char copyright[] = "PROFIBUS Linux API (c) Copyright 1995-2005. SOFTING AG. All Rights Reserved.";
#endif

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
#pragma check_stack (off)
#endif

// ***************************************************************************
// ***************************************************************************
// *
// *                         LOCAL FUNCTIONs
// *                         ---------------
// *
// ***************************************************************************
// ***************************************************************************


FUNCTION LOCAL INT16 papi_get_last_error(VOID)

/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION
This function is used to return the last error code generated by the PROFIBUS
driver.

Possible return values:
- last error code
- E_IF_OS_ERROR
----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  DWORD  LastErrorCode;
#endif

  FUNCTION_BODY

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  LastErrorCode = GetLastError();

  if (CUSTOMER_FLAG_SET(LastErrorCode)) return ((INT16)LastErrorCode);
  else                                  return (E_IF_OS_ERROR);
#endif

#ifdef _LINUX
  return ((INT16) errno);
#endif
}





// ***************************************************************************
// ***************************************************************************
// *
// *                         EXTENDED API
// *                         ------------
// *
// ***************************************************************************
// ***************************************************************************

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL HANDLE CALL_CONV profi_open_basic_management
         (
           IN USIGN8 Board,
           IN USIGN8 Channel,
           IN INT32  DesiredAccess
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to initialize the enhanced mode of the PROFIBUS API.

It opens the the BASIC MANAGEMENT DEVICE and updates the PROFIBUS firmware onto
PROFIBUS controller if necessary.

IN:  Board           -> number of the PROFIBUS board (0..9)
IN:  Channel         -> channel number
IN:  DesiredAccess   -> GENERIC_READ  specifies the read access to the device
                        GENERIC_WRITE specifies the write access to the device

Possible return values:
- open handle of BASIC MANAGEMENT DEVICE if function succeeds successfully
- INVALID_HANDLE_VALUE  if function fails

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  char  DeviceName[64];

  FUNCTION_BODY

  Channel = 0;                                                // for future use

  // --- open basic management device
  sprintf(DeviceName, "\\\\.\\PROFIBUS\\Board%u\\Pb%u\\Management",Board,Channel);

  return(CreateFile(DeviceName,
                    DesiredAccess,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                   ));
}

#endif


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL HANDLE CALL_CONV profi_open
         (
           IN HANDLE  hBasicMgmtDevice,
           IN INT32   DeviceType,
           IN USIGN32 Index,
           IN INT32   DesiredAccess
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to open a PROFIBUS device specified by 'device_type'.

IN:  hBasicMgmtDevice  -> basic management device handle
IN:  DeviceType        -> type of the device to open
                          DEVICE_DP_MANAGEMENT  -> DP management device
                          DEVICE_DP_SERVICE     -> DP service device
                          DEVICE_DP_SLAVE_DATA  -> DP slave data device
                          DEVICE_DP_MSAC        -> DP master slave acyclic device
                          DEVICE_FDL_MANAGEMENT -> FDL management device
                          DEVICE_FDL_SAP        -> FDL SAP device
                          DEVICE_FMS_MANAGEMENT -> FMS management device
                          DEVICE_FMS_CR         -> FMS CR device
IN:  Index             -> index of the device to open
                          1..128           DP service-, DP slave data- and DP/V1 service devices
                          0..63,DEFAUT_SAP FDL SAP device
                          1..64            FMS CR device
IN:  DesiredAccess     -> GENERIC_READ  specifies the read access to the device
                          GENERIC_WRITE specifies the write access to the device

Possible return values:
- open handle of the specified device if function succeeds successfuly
- INVALID_HANDLE_VALUE  if function fails

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD  IOControlCode;
  char   DeviceName[64];
  DWORD  BytesReturned;

  FUNCTION_BODY

  // --- select IOCTL code specified by device type
  switch (DeviceType)
  {
    case DEVICE_DP_MANAGEMENT:
         IOControlCode = (DWORD)IOCTL_PROFI_CREATE_DP_MANAGEMENT;
         break;

    case DEVICE_DP_SERVICE:
         IOControlCode = (DWORD)IOCTL_PROFI_CREATE_DP_SERVICE;
         break;

    case DEVICE_DP_SLAVE_DATA:
         IOControlCode = (DWORD)IOCTL_PROFI_CREATE_DP_SLAVE_DATA;
         break;

    case DEVICE_DP_MSAC:
         IOControlCode = (DWORD)IOCTL_PROFI_CREATE_DP_MSAC;
         break;

    case DEVICE_FDL_MANAGEMENT:
         IOControlCode = (DWORD)IOCTL_PROFI_CREATE_FDL_MANAGEMENT;
         break;

    case DEVICE_FDL_SAP:
         IOControlCode = (DWORD)IOCTL_PROFI_CREATE_FDL_SAP;
         break;

    case DEVICE_FMS_MANAGEMENT:
         IOControlCode = (DWORD)IOCTL_PROFI_CREATE_FMS_MANAGEMENT;
         break;

    case DEVICE_FMS_CR:
         IOControlCode = (DWORD)IOCTL_PROFI_CREATE_FMS_CR;
         break;

    default:
         SetLastError(ERROR_INVALID_PARAMETER);
         return(INVALID_HANDLE_VALUE);
  }


  // --- get device name
  if (!DeviceIoControl((HANDLE)  hBasicMgmtDevice,
                       (DWORD)   IOControlCode,
                       (LPVOID)  &Index,
                       (DWORD)   sizeof(ULONG),
                       (LPVOID)  DeviceName,
                       (DWORD)   128,
                       (LPDWORD) &BytesReturned,
                       NULL
                      ))
   {
     return(INVALID_HANDLE_VALUE);
   }


  // --- open device
  return(CreateFile(DeviceName,
                    DesiredAccess,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                   ));

}

#endif

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL BOOL CALL_CONV profi_close
         (
           IN HANDLE hDevice
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to close a device opened with 'profi_open_basic_management'
function or 'profi_open' function.

IN:  hDevice   -> handle of the device to close

Possible return values:
- TRUE        -> device is closed
- FALSE       -> device can not be closed

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  if(CloseHandle(hDevice)) return TRUE;
  else                     return FALSE;
}

#endif

FUNCTION GLOBAL INT16 CALL_CONV profi_read_service
         (
           IN    HANDLE                  hDevice,
           OUT   T_PROFI_SERVICE_DESCR * pSdb,
           OUT   VOID                  * pData,
           INOUT USIGN16               * pDataLength
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to receive a Service-Indication or a Service-Confirmation
from a PROFIBUS service device.

IN:     hDevice           -> device handle
OUT:    pSdb              -> pointer to SERVICE-DESCRIPTION-BLOCK
OUT:    pData             -> pointer to data buffer
INOUT:  pDatalen          -> IN:   length of data buffer
                             OUT:  length of received data
Possible return values:

- CON_IND_RECEIVED                 -> a confirmation or indication has been received
- NO_CON_IND_RECEIVED              -> no confirmation or indication has been received
- E_IF_NO_CNTRL_RES                -> controller does not respond
- E_IF_FATAL_ERROR                 -> unrecoverable error in PROTOCOL SW
- E_IF_CMI_ERROR                   -> serious CMI error
- E_IF_INVALID_DATA_SIZE           -> size of data block provided not sufficient
- E_IF_OS_ERROR                    -> NT system error
- E_IF_RESOURCE_UNAVAILABLE        -> no resource available
-----------------------------------------------------------------------------*/
 {
  // LOCAL_VARIABLES

  USIGN8 * pSdbData;


  ssize_t  BytesRead = 0;

 // FUNCTION_BODY

 // printf("papi: profi_read_service\n");

  // --- allocate memory for SDB and DATABLOCK
  if (!(pSdbData = malloc((*pDataLength + sizeof(T_PROFI_SERVICE_DESCR)))))
  {
    return(E_IF_OS_ERROR);
  }






  while ((BytesRead = read (hDevice, 
                         pSdbData, 
                         (int) (*pDataLength + sizeof(T_PROFI_SERVICE_DESCR)))
                         ) < 0)
  {
    

    

    if (errno != EAGAIN)
    {
    	free(pSdbData);
	printf ("%s\n", strerror (errno));
       // printf("bytesRead=%u\n",BytesRead);
    	return(papi_get_last_error());
    }
    else
    {
      *pDataLength = 0;
      free(pSdbData);
     // printf("bytesRead=%u\n",BytesRead);
     // printf ("%s errno=%u hdevice=%u 2\n", strerror (errno), errno, hDevice);
      return(NO_CON_IND_RECEIVED);
    }
  }

  //printf("bytesRead=%u\n",BytesRead);
  // --- check if IND or CON is available
  if (BytesRead == 0)
  {
    *pDataLength = 0;
   // printf ("%s 3\n", strerror (errno));
    free(pSdbData);
    return(NO_CON_IND_RECEIVED);
  }
  else
  {
   // printf("bytesRead=%u\n",BytesRead);
  }


  // --- copy SDB
  memcpy(pSdb,pSdbData,sizeof(T_PROFI_SERVICE_DESCR));
  pSdb->comm_ref &= 0xFF;


  // --- calculate size of read data
  *pDataLength = (USIGN16) BytesRead - sizeof(T_PROFI_SERVICE_DESCR);

  // --- copy DATABLOCK
  if (*pDataLength)
  {
    memcpy(pData,&pSdbData[sizeof(T_PROFI_SERVICE_DESCR)],*pDataLength);
  }

  // --- free allocated memory
  free(pSdbData);

  return(CON_IND_RECEIVED);
}



FUNCTION GLOBAL INT16 CALL_CONV profi_write_service
         (
           IN HANDLE                  hDevice,
           IN T_PROFI_SERVICE_DESCR * pSdb,
           IN VOID                  * pData
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to send a Service-Request or a Service-Response
to a PROFIBUS service device.

IN:  hDevice                  -> device handle
IN:  pSdp                     -> pointer to SERVICE-DESCRIPTION-BLOCK
IN:  pData                    -> pointer to service specific data


Possible return values:

- E_OK                           -> no error occured

- E_IF_INVALID_LAYER             -> invalid layer
- E_IF_INVALID_SERVICE           -> invalid service identifier
- E_IF_INVALID_PRIMITIVE         -> invalid service primitive
- E_IF_RESOURCE_UNAVAILABLE      -> no resource available
- E_IF_NO_PARALLEL_SERVICES      -> no parallel services allowed
- E_IF_SERVICE_CONSTR_CONFLICT   -> service temporarily not executable
- E_IF_SERVICE_NOT_SUPPORTED     -> service not supported in subset
- E_IF_SERVICE_NOT_EXECUTABLE    -> service not executable

- E_IF_NO_CNTRL_RES              -> controller does not respond  (CMI_TIMEOUT)
- E_IF_INVALID_DATA_SIZE         -> not enough CMI memory available for REQ or RES
- E_IF_INVALID_PARAMETER         -> invalid wrong parameter in REQ or RES
- E_IF_CMI_ERROR                 -> serious CMI error
- E_IF_INVALID_CMI_CALL          -> invalid CMI call
- E_IF_OS_ERROR                  -> NT system error

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  INT16   RetVal;                                             //  return value
  INT16   DataLength;                                    //  size of DATABLOCK
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  DWORD   BytesWritten;
#endif
  USIGN8* pSdbData;                               // pointer to SdbData buffer

  FUNCTION_BODY

  RetVal     = E_OK;
  DataLength = 0;

  pSdb->comm_ref &= 0x00FF;                    // clear all BITs in upper byte

  // --- check Service-Primitive
  if ((pSdb->primitive == IND) || (pSdb->primitive == CON))
  {
    return (E_IF_INVALID_PRIMITIVE);
  }

  // --- set the result parameter to POS if a request is send
  if (pSdb->primitive == REQ) pSdb->result = POS;


  // --- get service specific data length
  switch(pSdb->layer)
  {
    case FMS:
         RetVal = fmsgdl_get_data_len(pSdb->result,
                                      pSdb->service,
                                      pSdb->primitive,
                                      pData,
                                      &DataLength
                                     );
         break;

    case FM7:
         RetVal = fm7gdl_get_data_len(pSdb->result,
                                      pSdb->service,
                                      pSdb->primitive,
                                      pData,
                                      &DataLength
                                     );
         break;

    case FDLIF:
         RetVal = fdlgdl_get_data_len(pSdb->result,
                                      pSdb->service,
                                      pSdb->primitive,
                                      pData,
                                      &DataLength
                                     );
         break;

    case DP:
         RetVal = dpgdl_get_data_len(pSdb->result,
                                     pSdb->service,
                                     pSdb->primitive,
                                     pData,
                                     &DataLength
                                    );
         break;


    case FMB:
         RetVal = fmbgdl_get_data_len(pSdb->result,
                                      pSdb->service,
                                      pSdb->primitive,
                                      pData,
                                      &DataLength
                                     );
         break;


    case DPS:
         RetVal = dpsgdl_get_data_len(pSdb->result,
                                      pSdb->service,
                                      pSdb->primitive,
                                      pData,
                                      &DataLength
                                     );
         break;


    default:
         RetVal = E_IF_INVALID_LAYER;
         break;
  }

  if (RetVal == E_OK)
  {
    // allocate memory for SDB and DATABLOCK
    if (!(pSdbData = (USIGN8*) malloc((DataLength + sizeof(T_PROFI_SERVICE_DESCR)))))
    {
      return(E_IF_OS_ERROR);
    }

    // copy SDB and DATABLOCK to allocated memory
    memcpy(pSdbData,pSdb,sizeof(T_PROFI_SERVICE_DESCR));
    memcpy(&pSdbData[sizeof(T_PROFI_SERVICE_DESCR)],pData,DataLength);

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
    // write SDB and DATABLOCK
    if(!WriteFile((HANDLE)  hDevice,
                  (LPVOID)  pSdbData,
                  (DWORD)   (DataLength + sizeof(T_PROFI_SERVICE_DESCR)),
                  (LPDWORD) &BytesWritten,
                  NULL
                 ))
#endif

#ifdef _LINUX
    if ((write (hDevice, pSdbData, DataLength + sizeof(T_PROFI_SERVICE_DESCR))) < 0)
#endif
    {
      free(pSdbData);
      return(papi_get_last_error());
    }

    // free allocated memory
    free(pSdbData);
  }

  return(RetVal);
}


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL INT16 CALL_CONV profi_read_multi
         (
           OUT   T_PROFI_SERVICE_DESCR * pSdb,
           OUT   VOID                  * pData,
           INOUT USIGN16               * pDataLength,
           IN    USIGN16                 NrOfHandles,
           IN    HANDLE                * phDevices
          )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to receive a Service-Indication or a Service-Confirmation
from one ore more PROFIBUS service devices.

OUT:    pSdb           -> pointer to SERVICE-DESCRIPTION-BLOCK
OUT:    pData          -> pointer to data buffer
INOUT:  pDataLength    -> IN:   length of data buffer
                          OUT:  length of received data
IN:     NrOfHandles    -> number of device handles
IN:     phDevices      -> pointer to list of device handles

Possible return values:

- CON_IND_RECEIVED                 -> a confirmation or indication has been received
- NO_CON_IND_RECEIVED              -> no confirmation or indication has been received
- E_IF_NO_CNTRL_RES                -> controller does not respond
- E_IF_FATAL_ERROR                 -> unrecoverable error in PROTOCOL SW
- E_IF_CMI_ERROR                   -> serious CMI error
- E_IF_INVALID_DATA_SIZE           -> size of data block provided not sufficient
- E_IF_OS_ERROR                    -> NT system error
- E_IF_RESOURCE_UNAVAILABLE        -> no resource available

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  USIGN8* pSdbData;
  DWORD   BytesRead = 0;

  FUNCTION_BODY

  // --- allocate memory for SDB and DATABLOCK
  if (!(pSdbData = malloc((*pDataLength + sizeof(T_PROFI_SERVICE_DESCR)))))
  {
    return(E_IF_OS_ERROR);
  }


  // --- read service data from device
  if(!DeviceIoControl((HANDLE)  *phDevices,
                      (DWORD)   IOCTL_PROFI_READ_MULTI,
                      (LPVOID)  phDevices,
                      (DWORD)   NrOfHandles * sizeof(HANDLE),
                      (LPVOID)  pSdbData,
                      (DWORD)   (*pDataLength + sizeof(T_PROFI_SERVICE_DESCR)),
                      (LPDWORD) &BytesRead,
                      NULL
                     ))
  {
    free(pSdbData);
    return(papi_get_last_error());
  }

  // --- check if IND or CON is available
  if (BytesRead == 0)
  {
    *pDataLength = 0;
    free(pSdbData);
    return(NO_CON_IND_RECEIVED);
  }


  // --- copy SDB
  memcpy((VOID*)pSdb,(VOID*)pSdbData,sizeof(T_PROFI_SERVICE_DESCR));
  pSdb->comm_ref &= 0xFF;


  // --- calculate size of read data
  *pDataLength = (USIGN16) BytesRead - sizeof(T_PROFI_SERVICE_DESCR);

  // --- copy DATABLOCK
  if (*pDataLength)
  {
    memcpy(pData,&pSdbData[sizeof(T_PROFI_SERVICE_DESCR)],*pDataLength);
  }

  // --- free allocated memory
  free(pSdbData);

  return(CON_IND_RECEIVED);

}

#endif

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL INT16 CALL_CONV profi_read_data
         (
           IN    HANDLE    hDevice,
           OUT   VOID    * pData,
           INOUT USIGN16 * pDataLength
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to read DP-Slave-I/O-Data from a DP SLAVE DATA DEVICE.

IN:     hDevice           -> device handle
OUT:    pData             -> pointer to data buffer
INOUT:  pDataLength       -> IN:   length of data buffer
                             OUT:  length of received data

Possible return values:
- E_OK                             -> no error occured
- E_IF_INVALID_DP_STATE            -> DP is not in operational state
- E_IF_SLAVE_ERROR                 -> no valid communication with DP-Slave
- E_IF_SLAVE_DIAG_DATA             -> new DP diagnostics data available
- E_IF_INVALID_CMI_CALL            -> invalid offset inside data image
- E_IF_CMI_ERROR                   -> serious CMI error
- E_IF_INVALID_DATA_SIZE           -> size of data block provided not sufficient
- E_IF_SERVICE_CONSTR_CONFLICT     -> service not executable at time
- E_IF_OS_ERROR                    -> NT system error

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD  BytesRead = 0;

  FUNCTION_BODY

  if (!ReadFile(hDevice,pData,*pDataLength,&BytesRead,NULL))
  {
    *pDataLength = 0;
    return(papi_get_last_error());
  }
  else
  {
   *pDataLength = (USIGN16)BytesRead;
   return(E_OK);
  }
}

#endif


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL INT16 CALL_CONV profi_write_data
         (
           IN HANDLE    hDevice,
           IN VOID    * pData,
           IN USIGN16   DataLength
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to write DP-Slave-I/O-Data to a DP SLAVE DATA DEVICE.

IN:  hDevice            -> device handle
IN:  pData              -> pointer to data buffer
IN:  DataLength         -> length of data to write

Possible return values:
- E_OK                             -> no error occured
- E_IF_INVALID_DP_STATE            -> DP is not in operational state
- E_IF_SLAVE_ERROR                 -> no valid communication with DP-Slave
- E_IF_SLAVE_DIAG_DATA             -> new DP diagnostics data available
- E_IF_INVALID_CMI_CALL            -> invalid offset inside data image
- E_IF_CMI_ERROR                   -> serious CMI error
- E_IF_INVALID_DATA_SIZE           -> size of data block provided not sufficient
- E_IF_SERVICE_CONSTR_CONFLICT     -> service not executable at time
- E_IF_OS_ERROR                    -> NT system error

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD BytesWritten = 0;

  FUNCTION_BODY

  // write data
  if (!WriteFile(hDevice,pData,DataLength,&BytesWritten,NULL))
  {
    return(papi_get_last_error());
  }
  else
  {
    return(E_OK);
  }
}

#endif


FUNCTION GLOBAL INT16 CALL_CONV profi_read_dps_data
         (
           IN    HANDLE   hDevice,
           OUT   USIGN8 * pData,
           INOUT USIGN8 * pDataLength,
           OUT   USIGN8 * pState
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to read data from CMI DPS_SLAVE_INPUT_IMAGE or
DPS_SLAVE_OUTPUT_IMAGE.

IN:     hDevice     -> device handle
OUT:    pData       -> destination buffer for input data
INOUT:  pDataLength -> IN:  length of destination buffer
                       OUT: number of bytes read
OUT:    pState      -> status of input or output data if read or write was successfully


Possible return values:
- E_OK                             -> no error occured
- E_IF_INVALID_CMI_CALL            -> invalid offset inside data image
- E_IF_CMI_ERROR                   -> serious CMI error
- E_IF_INVALID_DATA_SIZE           -> size of data block provided not sufficient
- E_IF_SERVICE_CONSTR_CONFLICT     -> service not executable at time
- E_IF_OS_ERROR                    -> NT system error

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  DWORD    BytesRead = 0;
#endif

#ifdef _LINUX
  ssize_t  BytesRead = 0;
#endif

  char   DataBuffer[256];

  FUNCTION_BODY

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

  // --- read DPS input/output data and status
  if (ReadFile(hDevice,(LPVOID) DataBuffer,(DWORD) (*pDataLength + 1),(LPDWORD) &BytesRead,NULL))
#endif

#ifdef _LINUX
  if ((BytesRead = read (hDevice, DataBuffer, (int) (*pDataLength))) > 0)
#endif
  {
    *pState      = (USIGN8) DataBuffer[BytesRead-1];
    *pDataLength = (USIGN8) (BytesRead - 1);
    memcpy(pData,DataBuffer,*pDataLength);
    return(E_OK);
  }
  else
  {
    *pDataLength = 0;
    return(papi_get_last_error());
  }
}





FUNCTION GLOBAL INT16 CALL_CONV profi_write_dps_data
        (
          IN    HANDLE    hDevice,
          IN    USIGN8  * pData,
          IN    USIGN8    DataLength,
          OUT   USIGN8  * pState
        )

/*-----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to write DP-Slave input data to DPS_SLAVE_INPUT_IMAGE

IN:   hDevice    -> device handle
IN:   pData      -> pointer to new input data
IN:   DataLength -> number bytes of input data
OUT:  pState     -> pointer to a status variable for the recent input data status

possible return values:
- E_OK                          -> OK
- E_IF_INVALID_DATA_SIZE        -> data_size does not match the expected input data size
- E_IF_OS_ERROR                 -> NT system error

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  DWORD   BytesReturned = 0;
#endif

#ifdef _LINUX
  NTIoctl                  SlaveInput;
#endif

  FUNCTION_BODY

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

  // --- write DPS input data and read status
  if (!DeviceIoControl((HANDLE)  hDevice,
                       (DWORD)   IOCTL_PROFI_SET_DPS_DATA,
                       (LPVOID)  pData,
                       (DWORD)   DataLength,
                       (LPVOID)  pState,
                       (DWORD)   sizeof(USIGN8),
                       (LPDWORD) &BytesReturned,
                       NULL
                      ))
#endif

#ifdef _LINUX
  SlaveInput.InBuf        = (USIGN8*) pData;
  SlaveInput.InBufLength  = (int) DataLength;
  SlaveInput.OutBuf       = (USIGN8*) pData;
  SlaveInput.OutBufLength = DataLength;

  if(ioctl (hDevice, IOCTL_PROFI_SET_SLAVE_IN, &SlaveInput) == -1)
#endif
  {
    return(papi_get_last_error());
  }
  else
  {
    return(E_OK);
  }
}


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL BOOL CALL_CONV profi_get_cntrl_info
         (
           IN  USIGN8   Board,
           OUT char*    pFirmwareVersion,
           OUT USIGN32* pSerialDeviceNumber
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to get the version of the firmware and the serial
device number of PROFIBUS controller.

IN:     Board                -> board number
OUT:    pFirmwareVersion     -> data buffer for firmware version string
OUT:    pSerialDeviceNumber  -> serial device number

NOTE: There must be at least 100 Bytes (VERSION_STRING_LENGTH) free space for
      firmware version buffer.

Possible return values:
- TRUE   -> function returns sucessfully
- FALSE  -> function returns not successfully

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD                  BytesRead = 0;
  HANDLE                 hDevice;
  PROFI_DATA_IMAGE_DESCR DataImageDescr;
  char                   DeviceName[64];

  FUNCTION_BODY

  // --- open BOARD device
  sprintf(DeviceName, "\\\\.\\PROFIBUS\\Board%u\\Board",Board);
  if (INVALID_HANDLE_VALUE == (hDevice = CreateFile(DeviceName,
                                                    GENERIC_READ,
                                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                    NULL,
                                                    OPEN_EXISTING,
                                                    FILE_ATTRIBUTE_NORMAL,
                                                    NULL
                                                   )))
  {
    return(FALSE);
  }


  // --- get FIRMWARE VERSION
  DataImageDescr.imageId = ID_FW_VERS_IMAGE;
  DataImageDescr.offset  = 0;
  DataImageDescr.bus     = 0;

  if (!DeviceIoControl((HANDLE)  hDevice,
                       (DWORD)   IOCTL_PROFI_GET_DATA_IMAGE,
                       (LPVOID)  &DataImageDescr,
                       (DWORD)   sizeof(DataImageDescr),
                       (LPVOID)  pFirmwareVersion,
                       (DWORD)   VERSION_STRING_LENGTH,
                       (LPDWORD) &BytesRead,
                       NULL))
  {
    CloseHandle(hDevice);
    return(FALSE);
  }

  // --- get SERIAL DEVICE NUMBER
  DataImageDescr.imageId = ID_SERIAL_DEVICE_NUMBER;
  DataImageDescr.offset  = 0;
  DataImageDescr.bus     = 0;

  if (!DeviceIoControl((HANDLE)  hDevice,
                       (DWORD)   IOCTL_PROFI_GET_DATA_IMAGE,
                       (LPVOID)  (VOID*)&DataImageDescr,
                       (DWORD)   sizeof(DataImageDescr),
                       (LPVOID)  pSerialDeviceNumber,
                       (DWORD)   sizeof(USIGN32),
                       (LPDWORD) &BytesRead,
                       NULL))
  {
    CloseHandle(hDevice);
    return(FALSE);
  }

  CloseHandle(hDevice);
  return(TRUE);
}

#endif


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL BOOL CALL_CONV profi_set_timeout
         (
           IN HANDLE  hBasicMgmtDevice,
           IN USIGN32 ReadTimeout,
           IN USIGN32 WriteTimeout
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to set send and receive TIMEOUTs for SEND/RECEIVE interface
functions

IN:  hBasicMgmtDevice  -> basic management device handle
IN:  ReadTimeout       -> receive timeout in ms (WAIT_FOREVER for infinite wait)
IN:  WriteTimeout      -> send timeout in ms (WAIT_FOREVER for infinite wait)

Possible return values:
- TRUE   -> function returns sucessfully
- FALSE  -> function returns not successfully

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD   BytesReturned;
  USIGN32 ReadWriteTimeout[2];

  FUNCTION_BODY

  ReadWriteTimeout[0] = ReadTimeout;
  ReadWriteTimeout[1] = WriteTimeout;

  return(DeviceIoControl((HANDLE)  hBasicMgmtDevice,
                         (DWORD)   IOCTL_PROFI_SET_TIMEOUT,
                         (LPVOID)  ReadWriteTimeout,
                         (DWORD)   2 * sizeof(USIGN32),
                         (LPVOID)  NULL,
                         (DWORD)   0,
                         (LPDWORD) &BytesReturned,
                         NULL
                        ));
}

#endif


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL BOOL CALL_CONV profi_get_timeout
         (
           IN  HANDLE   hBasicMgmtDevice,
           OUT USIGN32* pReadTimeout,
           OUT USIGN32* pWriteTimeout
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to get the set send and receive TIMEOUTs for SEND/RECEIVE
interface functions

IN:   hBasicMgmtDevice  -> basic management device handle
OUT:  pReadTimeout      -> pointer to receive timeout in ms (WAIT_FOREVER for infinite wait)
OUT:  pWriteTimeout     -> pointer to send timeout in ms (WAIT_FOREVER for infinite wait)

Possible return values:
- TRUE   -> function returns sucessfully
- FALSE  -> function returns not successfully

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD   BytesReturned;
  USIGN32 ReadWriteTimeout[2];

  FUNCTION_BODY

  if (!DeviceIoControl((HANDLE)  hBasicMgmtDevice,
                       (DWORD)   IOCTL_PROFI_GET_TIMEOUT,
                       (LPVOID)  NULL,
                       (DWORD)   0,
                       (LPVOID)  ReadWriteTimeout,
                       (DWORD)   2 * sizeof(USIGN32),
                       (LPDWORD) &BytesReturned,
                       NULL
                      ))
  {
    return(FALSE);
  }

  if (BytesReturned == 8)
  {
    *pReadTimeout  = ReadWriteTimeout[0];
    *pWriteTimeout = ReadWriteTimeout[1];
    return(TRUE);
  }
  else
  {
    SetLastError(ERROR_INVALID_DATA);
    return(FALSE);
  }
}

#endif


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL BOOL CALL_CONV profi_set_queue_size
         (
           IN HANDLE  hBasicMgmtDevice,
           IN USIGN32 QueueSize
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to set the max. receive queue size for the PROFIBUS driver.

IN:  hBasicMgmtDevice  -> basic management device handle
IN:  QueueSize         -> max. receive queue size

Possible return values:
- TRUE   -> function returns sucessfully
- FALSE  -> function returns not successfully

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD   BytesReturned;

  FUNCTION_BODY

  return(DeviceIoControl((HANDLE)  hBasicMgmtDevice,
                         (DWORD)   IOCTL_PROFI_SET_QUEUE_SIZE,
                         (LPVOID)  &QueueSize,
                         (DWORD)   sizeof(USIGN32),
                         (LPVOID)  NULL,
                         (DWORD)   0,
                         (LPDWORD) &BytesReturned,
                         NULL
                        ));
}

#endif

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL BOOL CALL_CONV profi_get_queue_size
         (
           IN  HANDLE   hBasicMgmtDevice,
           OUT USIGN32* pQueueSize
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to get the set max. receive queue size for the PROFIBUS driver.

IN:  hBasicMgmtDevice  -> basic management device handle
OUT: pQueueSize        -> pointer to max. receive queue size

Possible return values:
- TRUE   -> function returns sucessfully
- FALSE  -> function returns not successfully

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD   BytesReturned;

  FUNCTION_BODY

  return(DeviceIoControl((HANDLE)  hBasicMgmtDevice,
                         (DWORD)   IOCTL_PROFI_GET_QUEUE_SIZE,
                         (LPVOID)  NULL,
                         (DWORD)   0,
                         (LPVOID)  pQueueSize,
                         (DWORD)   sizeof(USIGN32),
                         (LPDWORD) &BytesReturned,
                         NULL
                        ));
}

#endif


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION GLOBAL BOOL CALL_CONV profi_get_overrun_count
         (
           IN  HANDLE   hBasicMgmtDevice,
           OUT USIGN32* pOverrunCount
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION


IN:  hBasicMgmtDevice  -> basic management device handle
OUT: pOverrunCount     -> pointer to overrun counts

Possible return values:
- TRUE   -> function returns sucessfully
- FALSE  -> function returns not successfully

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  DWORD   BytesReturned;

  FUNCTION_BODY

  return(DeviceIoControl((HANDLE)  hBasicMgmtDevice,
                         (DWORD)   IOCTL_PROFI_GET_OVERRUN_COUNT,
                         (LPVOID)  NULL,
                         (DWORD)   0,
                         (LPVOID)  pOverrunCount,
                         (DWORD)   sizeof(USIGN32),
                         (LPDWORD) &BytesReturned,
                         NULL
                        ));
}

#endif



// ***************************************************************************
// ***************************************************************************
// *
// *                         COMPATIBILITY API
// *                         -----------------
// *
// ***************************************************************************
// ***************************************************************************


FUNCTION GLOBAL INT16 CALL_CONV profi_set_default
         (
	   OUT T_PROFI_DEVICE_HANDLE * hDevice,
           IN USIGN8  Board,
           IN USIGN8  Channel,
           IN USIGN32 ReadTimeout,
           IN USIGN32 WriteTimeout
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to initialize the WIN32 PROFIBUS-API. The function function
has to be called before any other function of PROFIBUS-API is called.

IN:  Board          -> number of the PROFIBUS board (0..9)
IN:  Channel        -> channel number (0)
IN:  ReadTimeout    -> receive timeout in ms (WAIT_FOREVER for infinite wait)
IN:  Writetimeout   -> send timeout in ms (WAIT_FOREVER for infinite wait)

Possible return values:
- E_OK                             -> Interface is initialized
- E_IF_NO_CNTRL_RES                -> controller does not respond
- E_IF_INVALID_CNTRL_TYPE_VERSION  -> invalid controller type or software version
- E_IF_INIT_INVALID_PARAMETER      -> invalid initialize parameter
- E_IF_INVALID_VERSION             -> invalid version
- E_IF_NO_CNTRL_PRESENT            -> no controller present
- E_IF_INIT_FAILED                 -> initialization failed

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  char    DeviceName[64];
  USIGN32 ReadWriteTimeout[2];

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  DWORD   BytesReturned;
#endif 

  FUNCTION_BODY

  if (hDevice->hServiceReadDevice || hDevice->hServiceWriteDevice || hDevice->hDpDataDevice)
  {
    return E_IF_SERVICE_NOT_EXECUTABLE; // Application has called init_profibus before
  }
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

  if (!ReadBoardRegistryEntries(Board,&OperationMode))
  {
    return(E_IF_READING_REGISTRY);
  }


  // get service device name
  sprintf(DeviceName, "\\\\.\\PROFIBUS\\Board%u\\Pb%u\\Service", Board, Channel);

  // Open service device for read access
  hServiceReadDevice = CreateFile(DeviceName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  0,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL
                                 );

  if (hServiceReadDevice == INVALID_HANDLE_VALUE)
  {
    hServiceReadDevice = NULL;
    return(E_IF_OS_ERROR);
  }


  // Open service device for write access
  hServiceWriteDevice = CreateFile(DeviceName,
                                   GENERIC_WRITE,
                                   0,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL
                                  );

  if (hServiceWriteDevice == INVALID_HANDLE_VALUE)
  {
    CloseHandle (hServiceReadDevice);
    hServiceReadDevice  = NULL;
    hServiceWriteDevice = NULL;
    return(E_IF_OS_ERROR);
  }



  if (OperationMode == FMS_DPV1_MASTER_MODE)
  {
    // --- get DP-Master's Data device name
    sprintf(DeviceName, "\\\\.\\PROFIBUS\\Board%u\\Pb%u\\DpData", Board, Channel);

    hDpDataDevice = CreateFile(DeviceName,
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL
                              );

    if (hDpDataDevice == INVALID_HANDLE_VALUE)
    {
      CloseHandle(hServiceReadDevice);
      CloseHandle(hServiceWriteDevice);
      hServiceReadDevice  = NULL;
      hServiceWriteDevice = NULL;
      hDpDataDevice       = NULL;
      return(E_IF_OS_ERROR);
    }
  }
  else
  {
    // get DP-Slave Input-Data device name
    sprintf(DeviceName, "\\\\.\\PROFIBUS\\Board%u\\Pb%u\\DpSlaveInputData", Board, Channel);

    hDpsInputDataDevice = CreateFile(DeviceName,
                                     GENERIC_READ | GENERIC_WRITE,
                                     0,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL
                                    );

    if (hDpsInputDataDevice == INVALID_HANDLE_VALUE)
    {
      CloseHandle(hServiceReadDevice);
      CloseHandle(hServiceWriteDevice);
      hServiceReadDevice  = NULL;
      hServiceWriteDevice = NULL;
      hDpsInputDataDevice = NULL;
      return(E_IF_OS_ERROR);
    }

    // get DP-Slave Output-Data device name
    sprintf(DeviceName, "\\\\.\\PROFIBUS\\Board%u\\Pb%u\\DpSlaveOutputData", Board, Channel);

    hDpsOutputDataDevice = CreateFile(DeviceName,
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL
                                     );

    if (hDpsOutputDataDevice == INVALID_HANDLE_VALUE)
    {
      CloseHandle(hServiceReadDevice);
      CloseHandle(hServiceWriteDevice);
      CloseHandle(hDpsInputDataDevice);
      hServiceReadDevice   = NULL;
      hServiceWriteDevice  = NULL;
      hDpsInputDataDevice  = NULL;
      hDpsOutputDataDevice = NULL;
      return(E_IF_OS_ERROR);
    }
  }

  ReadWriteTimeout[0] = ReadTimeout;
  ReadWriteTimeout[1] = WriteTimeout;

  if (!DeviceIoControl((HANDLE)  hServiceReadDevice,
                       (DWORD)   IOCTL_PROFI_SET_TIMEOUT,
                       (LPVOID)  ReadWriteTimeout,
                       (DWORD)   2 * sizeof(USIGN32),
                       (LPVOID)  NULL,
                       (DWORD)   0,
                       (LPDWORD) &BytesReturned,
                       NULL
                      ))
  {
    CloseHandle(hServiceReadDevice);
    CloseHandle(hServiceWriteDevice);
    if (hDpDataDevice)       CloseHandle(hDpDataDevice);
    if (hDpsInputDataDevice) CloseHandle(hDpsInputDataDevice);
    if (hDpsOutputDataDevice)CloseHandle(hDpsOutputDataDevice);

    hServiceReadDevice   = NULL;
    hServiceWriteDevice  = NULL;
    hDpDataDevice        = NULL;
    hDpsInputDataDevice  = NULL;
    hDpsOutputDataDevice = NULL;

    return(E_IF_CMI_ERROR);
  }

  CurrentBoardNumber = Board;
#endif

#ifdef _LINUX
  ReadWriteTimeout[0] = ReadTimeout;
  ReadWriteTimeout[1] = WriteTimeout;

  // get service device name
  sprintf(DeviceName, "/dev/pbservice%u", Board + Channel);

  // Open service device for read access
  hDevice->hServiceReadDevice = open (DeviceName, O_RDWR | O_NONBLOCK);

  if (hDevice->hServiceReadDevice == INVALID_HANDLE_VALUE)
  {
    hDevice->hServiceReadDevice = (HANDLE) NULL;
    return(E_IF_OS_ERROR);
  }

  // Open service device for write access
  hDevice->hServiceWriteDevice = hDevice->hServiceReadDevice;

  // --- get DP-Master's Data device name
  sprintf(DeviceName, "/dev/pbdata%u", Board + Channel);

  hDevice->hDpDataDevice = open (DeviceName, O_RDWR | O_NONBLOCK);

  // get DP-Slave Input-Data device name
  sprintf(DeviceName, "/dev/pbslin%u", Board + Channel);

  hDevice->hDpsInputDataDevice = open (DeviceName, O_RDWR | O_NONBLOCK);
  
  // get DP-Slave Output-Data device name
  sprintf(DeviceName, "/dev/pbslout%u", Board + Channel);

  hDevice->hDpsOutputDataDevice = open (DeviceName, O_RDWR | O_NONBLOCK);

	if ( (hDevice->hDpDataDevice == INVALID_HANDLE_VALUE) &&
       (hDevice->hDpsInputDataDevice == INVALID_HANDLE_VALUE) &&
       (hDevice->hDpsOutputDataDevice == INVALID_HANDLE_VALUE) )
  {
    close (hDevice->hServiceReadDevice);
    close (hDevice->hServiceWriteDevice);
    close (hDevice->hDpDataDevice);
    close (hDevice->hDpsInputDataDevice);
    close (hDevice->hDpsOutputDataDevice);
    hDevice->hServiceReadDevice   = (HANDLE) NULL;
    hDevice->hServiceWriteDevice  = (HANDLE) NULL;
    hDevice->hDpDataDevice        = (HANDLE) NULL;
    hDevice->hDpsInputDataDevice  = (HANDLE) NULL;
    hDevice->hDpsOutputDataDevice = (HANDLE) NULL;
    return(E_IF_OS_ERROR);
  }
  
	hDevice->CurrentBoardNumber = Board + Channel;

#endif

  return(E_OK);
}


FUNCTION GLOBAL INT16 CALL_CONV profi_init
         (
	   OUT T_PROFI_DEVICE_HANDLE * hDevice,
           IN USIGN8  Board,
           IN USIGN32 ReadTimeout,
           IN USIGN32 WriteTimeout
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to initialize the WIN32 PROFIBUS-API. The function function
has to be called before any other function of PROFIBUS-API is called.

IN:  Board          -> number of the PROFIBUS board (0..9)
IN:  Channel        -> channel number (0)
IN:  ReadTimeout    -> receive timeout in ms (WAIT_FOREVER for infinite wait)
IN:  Writetimeout   -> send timeout in ms (WAIT_FOREVER for infinite wait)

Possible return values:
- E_OK                             -> Interface is initialized
- E_IF_NO_CNTRL_RES                -> controller does not respond
- E_IF_INVALID_CNTRL_TYPE_VERSION  -> invalid controller type or software version
- E_IF_INIT_INVALID_PARAMETER      -> invalid initialize parameter
- E_IF_INVALID_VERSION             -> invalid version
- E_IF_NO_CNTRL_PRESENT            -> no controller present
- E_IF_INIT_FAILED                 -> initialization failed

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  if (   (hDevice->hServiceReadDevice && hDevice->hServiceWriteDevice)
      && (hDevice->hDpDataDevice || (hDevice->hDpsInputDataDevice && hDevice->hDpsOutputDataDevice))
     )
  {
    return(E_OK);                                 // PAPI is already initialized
  }
  else
  {
    return(profi_set_default(hDevice, Board,0,ReadTimeout,WriteTimeout));     // initalize PAPI
  }
}



FUNCTION GLOBAL INT16 CALL_CONV init_profibus
        (
          IN  USIGN32  DprAdress,
          IN  USIGN16  IoPortAdress,
          IN  PB_BOOL  Dummy
        )

/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used initialize the PROFIBUS-API with default values and start
the PROFIBUS controller. The function function has to be called before any other
function of PROFIBUS-API is called.

IN:  DprAdress         -> dummy
IN:  IoPortAdress      -> dummy
IN:  Dummy             -> dummy

Possible return values:

- E_OK                             -> Interface is initialized
- E_IF_NO_CNTRL_RES                -> controller does not respond
- E_IF_INVALID_CNTRL_TYPE_VERSION  -> invalid controller type or software version
- E_IF_INIT_INVALID_PARAMETER      -> invalid initialize parameter
- E_IF_INVALID_VERSION             -> invalid version
- E_IF_NO_CNTRL_PRESENT            -> no controller present
- E_IF_INIT_FAILED                 -> initialization failed
-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  T_PROFI_DEVICE_HANDLE hDevice;

  FUNCTION_BODY

  if (   (hDevice.hServiceReadDevice && hDevice.hServiceWriteDevice)
      && (hDevice.hDpDataDevice || (hDevice.hDpsInputDataDevice && hDevice.hDpsOutputDataDevice))
     )
  {
    return(E_OK);                                 // PAPI is already initialized
  }
  else
  {
    return(profi_set_default(&hDevice, 0,0,0,0));                        // initalize PAPI
  }
}



FUNCTION GLOBAL INT16 CALL_CONV profi_end(IN T_PROFI_DEVICE_HANDLE * hDevice)

/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to shutdown the PROFIBUS-API and shutdown the PROFIBUS
controller.

Possible return values:
- E_OK

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  // close service device
  if (hDevice->hServiceReadDevice)
  {
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
    CloseHandle(hServiceReadDevice);
#endif

#ifdef _LINUX
    close (hDevice->hServiceReadDevice);
#endif

    hDevice->hServiceReadDevice = (HANDLE) NULL;
  }

  if (hDevice->hServiceWriteDevice)
  {
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
    CloseHandle(hServiceWriteDevice);
#endif

#ifdef _LINUX
    close (hDevice->hServiceWriteDevice);
#endif

    hDevice->hServiceWriteDevice = (HANDLE) NULL;
  }


  // close DP-Data device
  if (hDevice->hDpDataDevice)
  {
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
    CloseHandle(hDpDataDevice);
#endif

#ifdef _LINUX
    close (hDevice->hDpDataDevice);
#endif

    hDevice->hDpDataDevice = (HANDLE) NULL;
  }


  // close DP-Slave Input-Data device
  if (hDevice->hDpsInputDataDevice)
  {
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
    CloseHandle(hDpsInputDataDevice);
#endif

#ifdef _LINUX
    close (hDevice->hDpsInputDataDevice);
#endif

    hDevice->hDpsInputDataDevice = (HANDLE) NULL;
  }

  // close DP-Slave Output-Data device
  if (hDevice->hDpsOutputDataDevice)
  {
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
    CloseHandle(hDpsOutputDataDevice);
#endif

#ifdef _LINUX
    close (hDevice->hDpsOutputDataDevice);
#endif

    hDevice->hDpsOutputDataDevice = (HANDLE) NULL;
  }

  return(E_OK);
}



FUNCTION GLOBAL INT16 CALL_CONV profi_snd_req_res
         (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
          IN T_PROFI_SERVICE_DESCR * pSdb,
          IN VOID                  * pData,
          IN PB_BOOL                 dummy
         )

/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to send a Service-Request or a Service-Response
to the PROFIBUS controller.

IN:  pSdb                  -> pointer to SERVICE-DESCRIPTION-BLOCK
IN:  pData                 -> pointer to service specific data
IN:  dummy                 -> dummy


Possible return values:

- E_OK                           -> no error occured

- E_IF_INVALID_LAYER             -> invalid layer
- E_IF_INVALID_SERVICE           -> invalid service identifier
- E_IF_INVALID_PRIMITIVE         -> invalid service primitive
- E_IF_RESOURCE_UNAVAILABLE      -> no resource available
- E_IF_NO_PARALLEL_SERVICES      -> no parallel services allowed
- E_IF_SERVICE_CONSTR_CONFLICT   -> service temporarily not executable
- E_IF_SERVICE_NOT_SUPPORTED     -> service not supported in subset
- E_IF_SERVICE_NOT_EXECUTABLE    -> service not executable

- E_IF_NO_CNTRL_RES              -> controller does not respond  (CMI_TIMEOUT)
- E_IF_INVALID_DATA_SIZE         -> not enough CMI memory available for REQ or RES
- E_IF_INVALID_VERSION           -> invalid version (only PROFDI-IF)
- E_IF_INVALID_PARAMETER         -> invalid parameter in REQ or RES
- E_IF_PAPI_NOT_INITIALIZED      -> API not initialized
----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  // --- check if service device is open
  if (!hDevice->hServiceWriteDevice) return (E_IF_PAPI_NOT_INITIALIZED);

  return(profi_write_service(hDevice->hServiceWriteDevice,pSdb,pData));
}




FUNCTION GLOBAL INT16 CALL_CONV profi_rcv_con_ind
         (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
          OUT   T_PROFI_SERVICE_DESCR * pSdb,
          OUT   VOID                  * pData,
          INOUT USIGN16               * pDataLength
         )

/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to receive a Service-Indication or a Service-Confirmation
from the PROFIBUS controller.


OUT:    pSdb             -> pointer to SERVICE-DESCRIPTION-BLOCK
OUT:    pData            -> pointer to data buffer
INOUT:  pDataLength      -> IN:   length of data buffer
                            OUT:  length of received data
Possible return values:

- CON_IND_RECEIVED          -> a confirmation or indication has been received
- NO_CON_IND_RECEIVED       -> no confirmation or indication has been received

- E_IF_FATAL_ERROR          -> unrecoverable error in PROTOCOL SW
- E_IF_INVALID_DATA_SIZE    -> size of data block provided not sufficient
- E_IF_PAPI_NOT_INITIALIZED -> API not initialized
----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  // --- check if service device is open
  if (!hDevice->hServiceReadDevice) return (E_IF_PAPI_NOT_INITIALIZED);

  return(profi_read_service(hDevice->hServiceReadDevice,pSdb,pData,pDataLength));
}



FUNCTION GLOBAL INT16 CALL_CONV profi_set_data
        (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
          IN  USIGN8    DataId,
          IN  USIGN16   Offset,
          IN  USIGN16   DataLength,
          IN  VOID    * pData
        )

/*-----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to write data to CMI data area

IN:   DataId         -> data image identifier
                         - ID_DP_SLAVE_IO_IMAGE     (DPM: image for slave io data)
                         - ID_DP_STATUS_IMAGE       (DPM: image for status data)

IN:   Offset         -> write at offset in data image
IN:   DataLength     -> length of  data to write
IN:   pData          -> pointer to data to write


possible return values:
- E_OK                          -> OK
- E_IF_SERVICE_CONSTR_CONFLICT  -> service not executable at time
- E_IF_SERVICE_NOT_SUPPORTED    -> service not supported
- E_IF_INVALID_DATA_SIZE        -> invalid user data size
- E_IF_PAPI_NOT_INITIALIZED     -> API not initialized

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  OVERLAPPED overlapped;
  DWORD      BytesWritten;
#endif

  FUNCTION_BODY


  // --- check if DP-Data device is open
  if (!hDevice->hDpDataDevice) return (E_IF_PAPI_NOT_INITIALIZED);

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  // --- init overlapped structure
  memset(&overlapped,0,sizeof(OVERLAPPED));
  overlapped.Offset = Offset;

  // write data
  if (!WriteFile(hDpDataDevice,
                 pData,
                 DataLength,
                 &BytesWritten,
                 &overlapped
                ))
  {
    return(papi_get_last_error());
  }
#endif

#ifdef _LINUX
  if (lseek (hDevice->hDpDataDevice, Offset, SEEK_SET) < 0)
  {
    return(papi_get_last_error());
  }

  if (write (hDevice->hDpDataDevice, pData, DataLength) < 0)
  {
    return(papi_get_last_error());
  }
#endif

  return(E_OK);
}



FUNCTION GLOBAL INT16 CALL_CONV profi_get_data
        (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
          IN     USIGN8     DataId,
          IN     USIGN16    Offset,
          INOUT  USIGN16  * pDataLength,
          OUT    VOID     * pData
        )

/*-----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to read  data from CMI data area

IN:    DataId        -> data image identifier
                         - ID_DP_SLAVE_IO_IMAGE        (DPM: image for slave io data)
                         - ID_DP_STATUS_IMAGE          (DPM: image for status data)

IN:    Offset        -> read at offset in data image
INOUT: pDataLength      IN:  length of data buffer
                        OUT: length of received data
OUT:   pData         -> pointer to data buffer

possible return values:
- E_OK                          -> OK
- E_IF_SERVICE_CONSTR_CONFLICT  -> service not executable at time
- E_IF_SERVICE_NOT_SUPPORTED    -> service not supported
- E_IF_INVALID_DATA_SIZE        -> invalid user data size
- E_IF_PAPI_NOT_INITIALIZED     -> API not initialized
- E_IF_OS_ERROR                 -> NT system error
-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  INT16                    RetVal = E_OK;
  HANDLE                   hBoard = (HANDLE) NULL;
  char                     DeviceName[64];

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  DWORD                    BytesRead = 0;

  T_PROFI_DATA_IMAGE_DESCR DataImageDescr;
  OVERLAPPED               Overlapped;
#endif

#ifdef _LINUX
  ssize_t                 BytesRead = 0;
  DataImage                DataImageDescr;
#endif

switch(DataId)
{

  
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
    case ID_SERIAL_DEVICE_NUMBER:
    case ID_FW_VERS_IMAGE:
    case ID_EXCEPTION_IMAGE:
         // --- open BOARD-Device
         sprintfprofi_get_data(DeviceName, "\\\\.\\PROFIBUS\\Board%u\\Board", CurrentBoardNumber);
         if (INVALID_HANDLE_VALUE == (hBoard = CreateFile(DeviceName,
                                                          0,
                                                          0,
                                                          NULL,
                                                          OPEN_ALWAYS,
                profi_get_data                                          FILE_ATTRIBUTE_NORMAL,
                                                          NULL
                                                         )))
         {BytesRead
           return(E_IF_OS_ERROR);
         }

         // --- set input IOCTL data structure
         DataImageDescr.imageId = DataId;
         DataImageDescr.offset  = Offset;
         DataImageDescr.bus     = 0;

         // --- read DATA-Image
         if (DeviceIoprofi_get_dataControl((HANDLE)  hBoard,
                             (DWORD)   IOCTL_PROFI_GET_DATA_IMAGE,
                             (LPVOID)  &DataImageDescr,BytesRead
                             (DWORD)   sizeof(DataImageDescr),
                             (LPVOID)  pData,
                             (DWORD)   *pDataLength,
                             (LPDWORD) &BytesRead,
                             NULL
                            ))
         {profi_get_data
           *pDataLength = (USIGN16) BytesRead;
           RetVal       =  E_OK;
         }
         else
         {
           *pDataLength = 0;
           RetVal       = papi_get_last_error();
         }

         CloseHandle(hBoard);
#endif //BytesRead

#ifdef _LINUX
    case ID_SERIAL_DEVICE_NUMBER:
    case ID_FW_VERS_IMAGE:
         sprintf(DeviceName, "/dev/pbboard%u", hDevice->CurrentBoardNumber);

         if ((hBoard = open (DeviceName, O_RDONLY)) < 0)
         {
           return(papi_get_last_error());
         }

         if (DataId == ID_SERIAL_DEVICE_NUMBER)
         {
           if (lseek (hBoard, Offset, SEEK_SET) < 0)
           {
             return(papi_get_last_error());
           }

           DataImageDescr.id     = ID_SERIAL_DEVICE_NUMBER;
           DataImageDescr.Buf    = pData;
           DataImageDescr.Length = *pDataLength;

           if(ioctl (hBoard, IOCTL_PROFI_SERIAL_NUMBER, &DataImageDescr) == -1)
           {
             *pDataLength = 0;
             RetVal       = papi_get_last_error();
           }
           else
           {
             *pDataLength = (USIGN16) 4;
             *(USIGN32*)pData = *(USIGN32*)&DataImageDescr;
             RetVal       =  E_OK;
            // printf("papi: Serialnumber = %lu len = %hu\n",*(USIGN32*)pData, BytesRead);
           }
         }
         else
         {
           if ((BytesRead = read (hBoard, pData, *pDataLength)) < 0)
           {
             *pDataLength = 0;
             RetVal       = papi_get_last_error();
           }
           else
           {
             *pDataLength = (USIGN16) BytesRead;
             RetVal       =  E_OK;
           }
         }

         close (hBoard);
#endif

         return(RetVal);


    case ID_DP_SLAVE_IO_IMAGE:
         // --- check if DP-Data device is open
         if (!hDevice->hDpDataDevice) return (E_IF_PAPI_NOT_INITIALIZED);

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
         // --- init overlapped structure
         memset(&Overlapped,0,sizeof(OVERLAPPED));
         Overlapped.OfhBoardfset = Offset;

         if (!ReadFile(hDpDataDevice,
                       pData,
                       *pDataLength,
                       &BytesRead,
                       &Overlapped
                       ))
         {
           return(papi_get_last_error());
         }

#endif

#ifdef _LINUX

         if (lseek (hDevice->hDpDataDevice, Offset, SEEK_SET) < 0)
         {
           return(papi_get_last_error());
         }

         DataImageDescr.id     = ID_DP_SLAVE_IO_IMAGE;
         DataImageDescr.Buf    = pData;
         DataImageDescr.Length = *pDataLength;

         /*printf("papi: ioctl = 0x%x board=0x%x, desc=0x%x boardnr=%u\n",
                  IOCTL_PROFI_GET_DATA_IMAGE, 
                  hDpDataDevice, 
                  &DataImageDescr,
                  CurrentBoardNumber);
*/

         if(ioctl (hDevice->hDpDataDevice, IOCTL_PROFI_GET_DATA_IMAGE, &DataImageDescr) == -1)
         {
           BytesRead = 0;
           RetVal       = papi_get_last_error();
         }
         else
         {

           BytesRead = (USIGN16)DataImageDescr.Length;
         //  printf("papi data length = %u\n", BytesRead);
           RetVal       =  E_OK;
         }

#endif

         *pDataLength = (USIGN16)BytesRead;
         return(E_OK);

     default:
         return(E_IF_SERVICE_NOT_SUPPORTED);
  }
}



FUNCTION GLOBAL INT16 CALL_CONV profi_set_dps_input_data
        (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
          IN    USIGN8  * pData,
          IN    USIGN8    DataLength,
          OUT   USIGN8  * pState
        )

/*-----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to write DP Slave input data to DPS_SLAVE_INPUT_IMAGE

IN:   pData      -> pointer to new input data
IN:   DataLength -> number bytes of input data
OUT:  pState     -> pointer to a status variable for the recent input data status

possible return values:
- E_OK                          -> OK
- E_IF_INVALID_DATA_SIZE        -> data_size does not match the expected input data size
- E_IF_NO_CNTRL_RES             -> timeout controller does not response
- E_IF_PAPI_NOT_INITIALIZED     -> API not initialized

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES


  FUNCTION_BODY

  if (hDevice->hDpsInputDataDevice == (HANDLE) NULL)
  {
    return (E_IF_PAPI_NOT_INITIALIZED);
  }

  return(profi_write_dps_data(hDevice->hDpsInputDataDevice,pData,DataLength,pState));
}




FUNCTION GLOBAL INT16 CALL_CONV profi_get_dps_input_data
        (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
          OUT   USIGN8 * pData,
          INOUT USIGN8 * pDataLength,
          OUT   USIGN8 * pState
        )

/*-----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to read data CMI DPS_SLAVE_INPUT_IMAGE

OUT:    pData       -> destination buffer for input data
INOUT:  pDataLength -> sizeof destination buffer and number of bytes read
OUT:    pState      -> status of input data if read successfully

possible return values:
- E_OK                          -> OK
- E_IF_INVALID_DATA_SIZE        -> invalid user data size
- E_IF_PAPI_NOT_INITIALIZED     -> API not initialized

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  if (hDevice->hDpsInputDataDevice == (HANDLE) NULL)
  {
    return (E_IF_PAPI_NOT_INITIALIZED);
  }

  return(profi_read_dps_data(hDevice->hDpsInputDataDevice,pData,pDataLength,pState));
}




FUNCTION GLOBAL INT16 CALL_CONV profi_get_dps_output_data
        (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
          OUT   USIGN8 * pData,
          INOUT USIGN8 * pDataLength,
          OUT   USIGN8 * pState
        )

/*-----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to read data from DPS_SLAVE_OUTPUT_IMAGE.

OUT:    pData       -> destination buffer for output data
INOUT:  pDataLength -> sizeof destination buffer and number of bytes read
OUT:    pState      -> status of output data if read successfully

possible return values:
- E_OK                          -> OK
- E_IF_SERVICE_NOT_SUPPORTED    -> service not supported
- E_IF_SERVICE_CONSTR_CONFLICT  -> service not executable at time
- E_IF_INVALID_DATA_SIZE        -> invalid user data size
- E_IF_PAPI_NOT_INITIALIZED     -> API not initialized

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  if (hDevice->hDpsOutputDataDevice == (HANDLE) NULL)
  {
    return (E_IF_PAPI_NOT_INITIALIZED);
  }

  return(profi_read_dps_data(hDevice->hDpsOutputDataDevice,pData,pDataLength,pState));

}




FUNCTION GLOBAL INT16 CALL_CONV profi_get_versions
         (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
           OUT CSTRING * pPapiVersion,
           OUT CSTRING * pFirmwareVersion
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to get the vesrion strings of PAPI and firmware.

OUT:    pPapiVersion      -> data buffer for PAPI version string
OUT:    pFirmwareVersion  -> data buffer for firmware version string

NOTE: There must be at least 100 Bytes (VERSION_STRING_LENGTH) free space for
      each INOUT buffer.

Possible return values:
- E_OK

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  USIGN16 DataLength;
  USIGN16 i;

  FUNCTION_BODY

  // --- PAPI version
#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  strcpy(pPapiVersion,(CSTRING*) "PROFIBUS WinXP/Win2K/WinNT API ");
#endif

#ifdef _LINUX
  strcpy(pPapiVersion,(CSTRING*) "PROFIBUS Linux API ");
#endif

  strcat(pPapiVersion,(CSTRING*) SW_VERSION);
  strcat(pPapiVersion,(CSTRING*) __DATE__);

  // --- get PROFIBUS firmware version
  DataLength = (USIGN16) VERSION_STRING_LENGTH;
  if (profi_get_data(hDevice, ID_FW_VERS_IMAGE,0,&DataLength,pFirmwareVersion) != E_OK)
  {
    strcpy(pFirmwareVersion,(CSTRING*) "controller not initialized");
  }


  // --- convert \n to ' '
  for (i=0; i < VERSION_STRING_LENGTH; i++)
  {
    if      (pFirmwareVersion[i] == '\n') pFirmwareVersion[i] = ' ';
    else if (pFirmwareVersion[i] == '\0') break;
    else                                  continue;
  }

  return(E_OK);
}



FUNCTION GLOBAL INT16 CALL_CONV profi_get_serial_device_number
         (
	  IN T_PROFI_DEVICE_HANDLE * hDevice,
           OUT USIGN32 * pSerialDeviceNumber
         )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to get the serial device number of the PROFIBUS
controller

OUT:  pSerialDeviceNumber -> serial device number

Possible return values:
- E_OK
- E_IF_PAPI_NOT_INITIALIZED      -> API not initialized

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  USIGN16 DataLength;

  FUNCTION_BODY

  DataLength = (USIGN16) sizeof(USIGN32);

  // --- get serial device number
  return(profi_get_data(hDevice, ID_SERIAL_DEVICE_NUMBER,0,&DataLength,pSerialDeviceNumber));
}



FUNCTION GLOBAL INT16 CALL_CONV profi_get_last_error(VOID)

/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is returns the additional last error code for INTERFACE-ERRORs
controller

Possible return values:
- additional last error code

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  DWORD  LastErrorCode;
#endif

  FUNCTION_BODY

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
  LastErrorCode = GetLastError();

  if (CUSTOMER_FLAG_SET(LastErrorCode)) return ((INT16)LastErrorCode);
  else                                  return (E_IF_OS_ERROR);
#endif

#ifdef _LINUX
  return ((INT16) errno);
#endif

}




#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)

FUNCTION BOOL WINAPI DllMain
         (
            IN HINSTANCE  hInstDLL,
            IN ULONG      Reason,
            IN LPVOID     pReserved
          )
/*----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

32Bit-DLL main function.
The function is called by MS-Windows during loading DLL.

Possible return values:
- TRUE

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  FUNCTION_BODY

  switch (Reason)
  {
     case DLL_PROCESS_ATTACH:
          hServiceReadDevice   = NULL;
          hServiceWriteDevice  = NULL;
          hDpDataDevice        = NULL;

          hDpsInputDataDevice  = NULL;
          hDpsOutputDataDevice = NULL;

          CurrentBoardNumber = 0;
          break;

     case DLL_THREAD_ATTACH:
          break;

     case DLL_THREAD_DETACH:
          break;

     case DLL_PROCESS_DETACH:
          profi_end();
          break;

     default:
          break;
  }

  return TRUE;
}
#endif

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
#pragma check_stack
#endif

