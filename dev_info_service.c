/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      dev_info_service.c
 *
 *  DESCRIPTION
 *      This file defines routines for using Device Information Service.
 *
 ******************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <gatt_prim.h>
#include <bluetooth.h>
#include <gatt.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "app_gatt_db.h"
#include "dev_info_service.h"



/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/


/*----------------------------------------------------------------------------*
 *  NAME
 *      DeviceInfoHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operation on device info service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP 
 *      message.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void DeviceInfoHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16 length = 0;
    uint8  *p_value = NULL;
    sys_status rc = gatt_status_irq_proceed;

    /* Let the firmware handle reads on this service.
     * Send response indication 
     */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc,
                  length, p_value);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      DeviceInfoCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the device info 
 *      service
 *
 *  RETURNS/MODIFIES
 *      Boolean - Indicating whether handle falls in range or not.
 *
 *----------------------------------------------------------------------------*/

extern bool DeviceInfoCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_DEVICE_INFO_SERVICE) &&
            (handle <= HANDLE_DEVICE_INFO_SERVICE_END))
            ? TRUE : FALSE;
}

