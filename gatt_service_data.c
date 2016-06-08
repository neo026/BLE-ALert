/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gatt_service_data.c
 *
 *  DESCRIPTION
 *      This file keeps information related to Gatt Service in the client role
 *
 *  NOTES
 *
 ******************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
 #include <gatt.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "gatt_service_data.h"
#include "gatt_service_uuids.h"
#include "app_gatt.h"

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Global application Gatt service data */
GATT_SERVICE_DATA_T g_app_gatt_data;

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattServiceCharValInd
 *
 *  DESCRIPTION
 *      This function handles the service changed characteristic indication
 *      and starts discovery procedure .
 *
 * RETURNS
        Nothing
 *----------------------------------------------------------------------------*/
extern void handleGattServiceCharValInd(GATT_CHAR_VAL_IND_T *ind)
{
    if(ind->handle == g_app_gatt_data.service_change_hndl)
    {
        /* Remote Server wants client to rediscover all the handles.
         * start discovery procedure 
         */
        StartDiscoveryProcedure();
    }
}

