/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      alert_client_gatt.h
 *
 *  DESCRIPTION
 *      Header file for alert client GATT-related routines
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __ALERT_CLIENT_GATT_H__
#define __ALERT_CLIENT_GATT_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <time.h>
#include <panic.h>


/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function handles read operation on attributes (as received in 
 * GATT_ACCESS_IND message) maintained by the application
 */
extern void HandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* This function handles Write operation on attributes (as received in 
 * GATT_ACCESS_IND message) maintained by the application.
 */
extern void HandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* This function is used to start undirected advertisements and moves to 
 * ADVERTISING state
 */

/* This function starts advertising */
extern void GattStartAdverts(bool fast_connection);

/* This function stops advertising */
extern void GattStopAdverts(void);

/* This function checks if the argument address is resolvable or not */
extern bool GattIsAddressResolvableRandom(TYPED_BD_ADDR_T *addr);

/* Triggers fast advertising */
extern void GattTriggerFastAdverts(void);
#endif /* __ALERT_CLIENT_GATT_H__ */

