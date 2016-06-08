/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      battery_service.h
 *
 *  DESCRIPTION
 *      Header definitions for Battery service
 *
 *****************************************************************************/

#ifndef __BATTERY_SERVICE_H__
#define __BATTERY_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>
#include <bt_event_types.h>

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function is used to initialise battery service data structure.*/
extern void BatteryDataInit(void);

/* This function is used to initialise battery service data structure at 
 * chip reset
 */
extern void BatteryInitChipReset(void);

/* This function handles read operation on battery service attributes
 * maintained by the application
 */
extern void BatteryHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* This function handles write operation on battery service attributes 
 * maintained by the application
 */
extern void BatteryHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* This function checks if the current battery voltage level has dropped below 
 * low threshold voltage
 */
extern bool CheckLowBatteryVoltage(void);

/* This function sends the battery level notification on conenction to the
 * remote host.
 */
extern void SendBatteryLevelNotification(void);

/* This function is to monitor the battery level and trigger notifications
 * (if configured) to the connected host
 */
extern void BatteryUpdateLevel(uint16 ucid,bool low_battery);

/* This function is used to read battery service specific data stored in 
 * NVM
 */
extern void BatteryReadDataFromNVM(uint16 *p_offset);

/* This function is used to check if the handle belongs to the Battery 
 * service
 */
extern bool BatteryCheckHandleRange(uint16 handle);

/* This function is used by application to notify bonding status to 
 * battery service
 */
extern void BatteryBondingNotify(void);


/* This function writes Battery service data in NVM */
extern void WriteBatteryServiceDataInNvm(void);

#endif /* __BATTERY_SERVICE_H__ */
