/* 
 * Proview   $Id: dpsgdl.c,v 1.2 2007-01-17 12:40:30 claes Exp $
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
/*****************************************************************************
*                                                                            *
*                                 SOFTING AG                                 *
*                          Richard-Reitzner-Allee 6                          *
*                                D-85540 Haar                                *
*                        Phone: ++49-89-4 56 56-0                            *
*                          Fax: ++49-89-4 56 56-3 99                         *
*                                                                            *
*                      Copyright (C) SOFTING AG 1998-2003                    *
*                             All Rights Reserved                            *
*                                                                            *
******************************************************************************

FILE_NAME               DPSGDL.C

PROJECT_NAME            PROFIBUS

MODULE                  DPSGDL

COMPONENT_LIBRARY       PAPI Lib
                        PAPI DLL

AUTHOR                  SOFTING AG

VERSION                 1.20.0.00.release (DP-Slave Stand-Alone for DOS)
                        5.22.0.00.release (WIN95/WIN98 and WinNT)

DATE                    26-February-1999

STATUS                  finished

FUNCTIONAL_MODULE_DESCRIPTION

This modul contains DP-Slave-Service-Specific-Functions which return the length
length of the Request- or Response-Datas.


RELATED_DOCUMENTS
=============================================================================*/

#include "keywords.h"

INCLUDES

#if defined (WIN16) || defined (WIN32)
#include <windows.h>
#endif

#include "pb_type.h"
#include "pb_if.h"

#ifndef DPS_STANDALONE_MODE
#include "pb_err.h"
#include "pb_dp.h"
#endif

#include "pb_dps.h"


FUNCTION_DECLARATIONS


LOCAL_DEFINES

LOCAL_TYPEDEFS

EXPORT_DATA

IMPORT_DATA

LOCAL_DATA


#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
#pragma check_stack (off)
#endif
FUNCTION PUBLIC INT16 dpsgdl_get_data_len
  (
  IN    INT16          result,
  IN    USIGN8         service,
  IN    USIGN8         primitive,
  IN    USIGN8    FAR* data_ptr,
  OUT   INT16     FAR* data_len_ptr
  )
  /*------------------------------------------------------------------------*/
  /* FUNCTIONAL_DESCRIPTION                                                 */
  /*------------------------------------------------------------------------*/
  /* - returns the data length of any called PROFIBUS DP-Slave service      */
  /*------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  T_DPS_INIT_SLAVE_REQ    FAR* init_slave_req_ptr;
  T_DPS_SLAVE_DIAG_REQ    FAR* slave_diag_req_ptr;


  FUNCTION_BODY

  switch (primitive)
  {
    /*--- USER REQUESTS ----------------------------------------------------*/

    case REQ:
    {
      switch (service)
      {
        case DPS_INIT_SLAVE   :
        {
          init_slave_req_ptr = (T_DPS_INIT_SLAVE_REQ FAR*) data_ptr;

          if ( (init_slave_req_ptr->cfg_data_len == 0                  ) ||
               (init_slave_req_ptr->cfg_data_len  > DP_MAX_CFG_DATA_LEN) )
          {
            result = E_IF_INVALID_DATA_SIZE;
            return (E_IF_INVALID_DATA_SIZE);
          }

          if (init_slave_req_ptr->enhanced_init_data_len > sizeof (init_slave_req_ptr->enhanced_init_data))
          {
            result = E_IF_INVALID_DATA_SIZE;
            return (E_IF_INVALID_DATA_SIZE);
          }

          *data_len_ptr = sizeof (T_DPS_INIT_SLAVE_REQ) -
                          (sizeof (init_slave_req_ptr->enhanced_init_data) - init_slave_req_ptr->enhanced_init_data_len);
          break;
        } /* case DPS_INIT_SLAVE */

        case DPS_EXIT_SLAVE   : *data_len_ptr = 0; break;

        case DPS_SLAVE_DIAG   :
        {
          slave_diag_req_ptr = (T_DPS_SLAVE_DIAG_REQ FAR*) data_ptr;

          if (slave_diag_req_ptr->ext_diag_data_len > DP_MAX_EXT_DIAG_DATA_LEN)
          {
            result = E_IF_INVALID_DATA_SIZE;
            return (E_IF_INVALID_DATA_SIZE);
          }

          *data_len_ptr = sizeof (T_DPS_SLAVE_DIAG_REQ) - (DP_MAX_EXT_DIAG_DATA_LEN - slave_diag_req_ptr->ext_diag_data_len);
          break;
        }

        case DPS_GET_STATUS   : *data_len_ptr = 0; break;

        case DPS_CHK_CFG      :
        case DPS_SET_PRM      :
        case DPS_SET_SLAVE_ADD: result = E_IF_INVALID_PRIMITIVE; return (E_IF_INVALID_PRIMITIVE);

        default               : result = E_IF_INVALID_SERVICE; return (E_IF_INVALID_SERVICE);

      } /* switch serivce */

      break;
    } /* case REQ */

    /*---- USER RESPONSES --------------------------------------------------*/

    case RES:
    {
      switch (service)
      {
        case DPS_CHK_CFG: *data_len_ptr = sizeof (T_DPS_CHK_CFG_RES); break;
        case DPS_SET_PRM: *data_len_ptr = sizeof (T_DPS_SET_PRM_RES); break;

        default: result = E_IF_INVALID_SERVICE; return (E_IF_INVALID_SERVICE);

      } /* switch serivce */

      break;
    } /* case RES */

    /*---- WRONG PRIMITIVE -------------------------------------------------*/

    default: result = E_IF_INVALID_PRIMITIVE; return (E_IF_INVALID_PRIMITIVE);

  } /* switch primitive */

  result = E_OK;
  return (E_OK);

} /* dpsgdl_get_data_len */

#if defined (WIN32) || defined (_WIN32) || defined (WIN16) || defined (_WIN16)
#pragma check_stack
#endif
