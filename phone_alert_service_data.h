/*****************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      phone_alert_service_data.h
 *
 *  DESCRIPTION
 *      Header file for the application
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __PHONE_ALERT_SERVICE_DATA_H__
#define __PHONE_ALERT_SERVICE_DATA_H__
/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <bt_event_types.h>
#include <gatt.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "local_debug.h"


/*============================================================================*
 *  Public Data Types
 *============================================================================*/


typedef struct
{
    /* phone alert status service start handle */
    uint16       pas_start_hndl;

    /* phone alert status service End handle */
    uint16       pas_end_hndl;

    /*phone alert status characteristic */
    uint16       phone_alert_hndl;

    /* this handle will be used while discovering characteristic descriptors.*/
    uint16       phone_alert_char_end_hndl;

    /* phone alert client configuration descriptor */
    uint16       phone_alert_ccd_hndl;

    /* ringer setting status characteristic */
    uint16       ringer_setting_hndl;

    /* this handle will be used while discovering characteristic descriptors.*/
    uint16       ringer_setting_end_hndl;

    /* ringer setting client configuration descriptor */
    uint16       ringer_setting_ccd_hndl;

    /*Ringer control point*/
    uint16       ringer_ctrl_hndl;

    /* Phone Alert status value */
    uint8        phone_alert_status_value;

    /* Ringer setting value */
    uint8        ringer_setting_value;

    /* Boolean to keep check on functioning of the button 
     * This variable will be set to TRUE on application initialization
     * and connection creation and will be set to FALSE only when PAS
     * profile configuration is complete and application has read the initial
     * values in the alert status characteristic and ringer setting 
     * characteristic. This variable will also act as a check while reading 
     * phone alert status and ringer setting one after other
     */
    bool         pas_config_ongoing;
}PHONE_ALERT_SERVICE_DATA_T;

/*============================================================================*
 *  Public Definitions
 *============================================================================*/
#define UUID_PHONE_ALERT_STATUS                            (0x180E)
#define UUID_ALERT_STATUS                                  (0x2A3F)
#define UUID_RINGER_SETTING                                (0x2A41)
#define UUID_RINGER_CONTROL_POINT                          (0x2A40)


/* Ringer state defined values.
 * https://developer.bluetooth.org/gatt/characteristics/Pages/
 * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.alert_status.xml
 */
typedef enum {
    ringer_bitmask_ringer  = 0x1,    /* Ringer State */
    ringer_bitmask_vibrate = 0x2,   /* Vibrate State */
    ringer_bitmask_display = 0x4    /* Display Alert Status */
} ringer_state_bitmask;

/* Ringer setting defined values. 
 * https://developer.bluetooth.org/gatt/characteristics/Pages/
 * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.ringer_setting.xml
 */
typedef enum {
    ringer_setting_silent  = 0x00,    /* Ringer Silent */
    ringer_setting_normal  = 0x01     /* Ringer Normal */
} ringer_setting;


/* Ringer Control Point defined values
 * http://developer.bluetooth.org/gatt/characteristics/Pages/
 * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.
 * ringer_control_point.xml
 */
 typedef enum {

    /* To put the ringer in silent mode */
    ringer_silent_mode            = 0x01,

    /* To mute the ringer once but does not change the ringer setting */
    ringer_mute_once              = 0x02,

    /* To cancel the silent mode */
    ringer_cancel_silent_mode     = 0x03
    /* Rest are reserved for future use */
}ringer_ctrl_point_value;

/*============================================================================*
 *  Private Definitions
 *============================================================================*/
extern PHONE_ALERT_SERVICE_DATA_T g_app_pas_data;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function configures the phone alert server */
extern void ConfigurePhoneAlertServer(uint16 cid);

/* This function handles the phone alert status profile specific 
 * GATT_CHAR_VAL_IND received 
 */
extern void HandlePASGattCharValInd(GATT_CHAR_VAL_IND_T *ind);

/* This function handes the phone alert status profile specific gatt read value
 * confirmation signal received.
 */
extern void HandlePASGattReadCharValCFM(GATT_READ_CHAR_VAL_CFM_T *cfm);

/* This function reads the phone alert characteristic */
extern void ReadPASPhoneAlertChar(uint16 cid);

/* This function reads the ringer setting of the remote server */
extern void ReadPASRingerSettingChar(uint16 cid);

/* This function writes on the ringer control point of remote server */
extern void WritePASRingerCtrlPoint(ringer_ctrl_point_value val);

/* This function initializes the PAS data */
extern void InitPASData(void);
#endif /* __PHONE_ALERT_SERVICE_DATA_H__ */
