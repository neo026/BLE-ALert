/******************************************************************************
 *  Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gatt_service.h
 *
 *  DESCRIPTION
 *      Header definitions for GATT service
 *
 *****************************************************************************/
#ifndef __GATT_SERVICE_H__
#define __GATT_SERVICE_H__

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <bt_event_types.h>

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"


/*=============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function is used to initialise gatt service data structure.*/
extern void GattDataInit(void);

/* This function reads the GATT service data from NVM. */
extern void GattReadDataFromNVM(uint16 *p_offset);

/* This function should be called when a bonded host connects. */
extern void GattOnConnection(uint16 st_ucid);

/* This function allows other modules to read whether the bonded device
 * has requested indications on the Service Changed characteristic. */
extern bool GattServiceChangedIndActive(void);

/* This function should be called when the device is switched into
 * over-the-air update mode.
 */
extern void GattOnOtaSwitch(void);

/* This function handles READ operations on GATT service attributes. */
extern void GattHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* This function handles READ operations on GATT service attributes. */
extern void GattHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* Determine whether a given handle is one handled by the GAP module. */
extern bool GattCheckHandleRange(uint16 handle);

/* This function is used by application to notify bonding status to 
 * gatt service
 */
extern void GattBondingNotify(void);

#ifdef NVM_TYPE_FLASH
/* This function writes Gatt service data in NVM */
extern void WriteGattServiceDataInNvm(void);
#endif /* NVM_TYPE_FLASH */

#endif /* __GATT_SERVICE_H__ */
