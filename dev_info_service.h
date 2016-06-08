/******************************************************************************
 *  Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      dev_info_service.h
 *
 *  DESCRIPTION
 *      Header definitions for Device Information Service
 *
 ******************************************************************************/

#ifndef __DEVICE_INFO_SERVICE_H__
#define __DEVICE_INFO_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>
#include <bt_event_types.h>

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function handles the read access indication for device infomation 
 * service.
 */
extern void DeviceInfoHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* This function checks if the passed handles falls in the device information
 * handle range or not.
 */
extern bool DeviceInfoCheckHandleRange(uint16 handle);

#endif /* __DEVICE_INFO_SERVICE_H__ */

