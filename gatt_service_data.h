/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gatt_service_data.h
 *
 *  DESCRIPTION
 *      Header file for the service
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __GATT_SERVICE_DATA_H__
#define __GATT_SERVICE_DATA_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
 #include <bt_event_types.h>


/*============================================================================*
 *  Public Data Types
 *============================================================================*/
typedef struct
{
    /* Gatt service start handle */
    uint16                                                 gatt_start_hndl;

    /* Gatt service end handle */
    uint16                                                 gatt_end_hndl;

    /* Service changed characteristic handle */
    uint16                                                 service_change_hndl;
}GATT_SERVICE_DATA_T;


/*============================================================================*
 *  Private Definitions
 *============================================================================*/
extern GATT_SERVICE_DATA_T g_app_gatt_data;


/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function restarts the service discovery on reception of service changed
 * indication 
 */
extern void handleGattServiceCharValInd(GATT_CHAR_VAL_IND_T *ind);
#endif /* __GATT_SERVICE_DATA_H__ */
