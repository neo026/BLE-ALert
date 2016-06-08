/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      alert_notification_service_data.h
 *
 *  DESCRIPTION
 *      Header file for the application
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __ALERT_NOTIFICATION_H__
#define __ALERT_NOTIFICATION_H__
/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <bt_event_types.h>

/*============================================================================*
 *  Public Data Types
 *============================================================================*/

typedef struct
{
    /* Supported new alert categories */
    uint8        supported_new_alert_cat_value[2];

    /* Supported unread alert categories */
    uint8        supported_unread_alert_cat_value[2];

    /* New alert value */
    uint8        new_alert_value[2];

    /* Unread alert status value */
    uint8        unread_alert_status_value[2];

    /* Alert notification service start handle */
    uint16       ans_start_handle;

    /* Alert notification service end handle*/
    uint16       ans_end_handle;

    /* Supported new alert category characteristic */
    uint16       new_alert_cat_hndl;

    /* New alert characteristic */
    uint16       new_alert_hndl;

    /* This handle will be used while discovering characteristic descriptors.*/
    uint16       new_alert_char_end_hndl;

    /* Supported unread alert category*/
    uint16       unread_alert_cat_hndl;

    /* Unread alert status characteristic */
    uint16       unread_alert_hndl;

    /* This handle will be used while discovering characteristic descriptors.*/
    uint16       unread_alert_char_end_hndl;

    /*Alert notification control point*/
    uint16       alert_ctrl_hndl;

    /* New alert characteristic */
    uint16       new_alert_ccd_hndl;

    /* Unread alert status characteristic */
    uint16       unread_alert_ccd_hndl;

    /* Variable for storing the command which we have written on the ANS
     * control point.
     */
    uint8        last_ans_command;

    /* This array is for storing the last num of new alerts received for each 
     * category.
     */
    uint8        last_num_new_alert[10];

    /* This array is for storing the last num of unread alerts received for 
     * each category
     */
    uint8        last_num_unread_alert[10]; 

    /* Variable which will be TRUE is application is still configuring
     * the alert notification server.
     */
     bool        config_in_progress;
}ALERT_NOTIFICATION_DATA_T;

/* Alert category id. defined values.
 * https://developer.bluetooth.org/gatt/characteristics/Pages/
 * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.
 * alert_category_id.xml
 * Note : If you are changing this enum, then you need to make corresponding 
 * change in function IsAlertCategoryValid too. This function is checking if the
 * alert category received is falling with in the range defined by this enum or
 * not.
 */
typedef enum {
    /* Simple alert: general text alert or non-text alert */
    alert_cat_simple                                = 0x0,

    /* Email: Alert when Email messages arrives */
    alert_cat_email                                 = 0x1,

    /* News: News feeds such as RSS, Atom */
    alert_cat_news                                  = 0x2,

    /* Call: Incoming call */
    alert_cat_call                                  = 0x3,

    /* Missed call: Missed Call */
    alert_cat_missed_call                           = 0x4,

    /* SMS/MMS: SMS/MMS message arrives */
    alert_cat_message                               = 0x5,

    /* Voice mail: Voice mail */
    alert_cat_vmail                                 = 0x6,

    /* Schedule: Alert occurred on calendar, planner */
    alert_cat_schedule                              = 0x7,

    /* High Prioritized Alert: Alert that should be handled as high priority */
    alert_cat_priority                              = 0x8,

    /* Instant Message: Alert for incoming instant messages */
    alert_cat_instant_msg                           = 0x9
} alert_category_id;

/* Alert category id. bitmask defined values.
 * https://developer.bluetooth.org/gatt/characteristics/Pages/
 *          CharacteristicViewer.aspx?u=org.bluetooth.characteristic.
 *          alert_category_id_bit_mask.xml
 */
typedef enum {
    /* Byte 1 */
    /* Simple Alert: General text alert or non-text alert */
    alert_cat_bitmask_simple                        = 0x01,

    /* Email: Alert when Email messages arrives */
    alert_cat_bitmask_email                         = 0x02,

    /*News: News feeds such as RSS, Atom*/
    alert_cat_bitmask_news                          = 0x04,

    /* Call: Incoming call */
    alert_cat_bitmask_call                          = 0x08,

    /* Missed call: Missed Call */
    alert_cat_bitmask_missed_call                   = 0x10,

    /* SMS/MMS: SMS/MMS message arrives */
    alert_cat_bitmask_message                       = 0x20,

    /* Voice mail: Voice mail */
    alert_cat_bitmask_vmail                         = 0x40,

    /* Schedule: Alert occurred on calendar, planner */
    alert_cat_bitmask_schedule                      = 0x80,

    /* Byte 2 */
    /* High Prioritized Alert: Alert that should be handled as high priority */
    alert_cat_bitmask_priority                      = 0x01,

    /* Instant Message: Alert for incoming instant messages */
    alert_cat_bitmask_instant_msg                   = 0x02
} alert_category_id_bitmask;

/* Alert notification control command identifiers.
 * https://developer.bluetooth.org/gatt/characteristics/Pages/
 *      CharacteristicViewer.aspx?u=org.bluetooth.characteristic.
 *      alert_notification_control_point.xml
 */
typedef enum {
    /* Enable new incoming alert notification */
    alert_ctrl_enable_new_incoming_alerts           = 0,

    /* Enable unread category status notification */
    alert_ctrl_enable_unread_cat_status             = 1,

    /* Disable new incoming alert notification */
    alert_ctrl_disable_new_incoming_alerts          = 2,

    /* Disable unread category status notification */
    alert_ctrl_disable_unread_cat_status            = 3,

    /* Notify new incoming alert immediately */
    alert_ctrl_immediate_new_alert                  = 4,

    /* Notify unread category status immediately */
    alert_ctrl_immediate_unread_status              = 5
} alert_notification_ctrl_id;


typedef enum {
    alert_type_new = 0, /* New alert */
    alert_type_unread
}alert_type;

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

#define UUID_ALERT_NOTIFICATION_SERVICE               (0x1811)
#define UUID_NEW_ALERT_SUPPORTED_CATEGORY             (0x2A47)
#define UUID_NEW_ALERT_CHARACTERISTIC                 (0x2A46)
#define UUID_SUPPORTED_UNREAD_ALERT_CATEGORY          (0x2A48)
#define UUID_UNREAD_ALERT_CHARACTERISTIC              (0x2A45)
#define UUID_ALERT_NOTIFICATION_CONTROL_POINT         (0x2A44)
#define UUID_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC (0x2902)


/*============================================================================*
 *  Private Definitions
 *============================================================================*/
extern ALERT_NOTIFICATION_DATA_T g_app_ans_data;


/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function configures the alert notification server */
extern void ConfigureAlertNotificationServer(uint16 cid);

/* This handles the alert notification profile specific gatt read value 
 * confirmation received.
 */
extern void handleANSGattReadCharValCFM(GATT_READ_CHAR_VAL_CFM_T *cfm);

/* This function handles the alert notification profile specific gatt 
 * characterstic value indication received 
 */
extern void handleANSGattCharValInd(GATT_CHAR_VAL_IND_T *ind);

/* This function writes on the alert notification control point of server */
extern void SendAlertControlPointCommand(uint16 cid, uint8 command_id, 
                                                            uint8 cat_id);
#endif /* __ALERT_NOTIFICATION_H__ */

