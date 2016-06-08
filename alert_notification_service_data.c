/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      alert_notification_service_data.c
 *
 *  DESCRIPTION
 *      This file keeps information related to Alert Notification Service 
 *      Discovered
 *
 *  NOTES
 *
 ******************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <gatt.h>
#include <mem.h>
#include <types.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "alert_notification_service_data.h"
#include "local_debug.h"
#include "app_gatt.h"
#include "alert_client_hw.h"
#include "alert_client.h"
#include "phone_alert_service_data.h"

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Global application alert notification(ans) data */
ALERT_NOTIFICATION_DATA_T g_app_ans_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
/* This function checks if alert category is in valid range or not */
static bool IsAlertCategoryValid(alert_category_id alert_cat);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      IsAlertCategoryValid
 *
 *  DESCRIPTION
 *      This function checks if alert category is in valid range or not.
 *      Future devices may have more supported categories than we currently have
 *      In such cases, we should ignore those categories.
 *
 *  RETURNS
 *      boolean : TRUE if alert category is in valid range.
 *----------------------------------------------------------------------------*/
static bool IsAlertCategoryValid(alert_category_id alert_cat)
{
    /* Please refer enum alert_category_id defined in file 
     * alert_notification_service_data.h for this range.
     */
    if(alert_cat <= alert_cat_instant_msg)
    {
        /* Alert category is in valid range, return TRUE */
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      SendAlertControlPointCommand
 *
 *  DESCRIPTION
 *      This function writes alert notification control point command on the 
 *      server
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void SendAlertControlPointCommand(uint16 cid, uint8 command_id, 
                                                            uint8 cat_id)
{
    uint8 interesting_alerts[2];

    /* Command id */
    interesting_alerts[0] = command_id;

    /* Store the alert notification control point command. When application will
     * receive gatt write characteristic value confirmation then this variable 
     * will be used to check which command was sent 
     */
    g_app_ans_data.last_ans_command = command_id;

    interesting_alerts[1] = cat_id;

    /* Write the configuration to the Server to receive appropriate 
     * notifications.
     */
     /* Please note that it is a write request */
    GattWriteCharValueReq(cid,
                              GATT_WRITE_REQUEST,
                              g_app_ans_data.alert_ctrl_hndl,
                              sizeof(interesting_alerts),
                              interesting_alerts);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ConfigureAlertNotificationServer
 *
 *  DESCRIPTION
 *      This function starts the configuration process for alert notification
 *      service. Configuration process consists of 8 steps mentioned below.
 *      This function will start the first step of configuration process and
 *      when application receives response for this first step, 2nd step will
 *      be triggered. Here application is enabling notifications on the client
 *      configuration descriptor of new alert characteristic and when it 
 *      receives response signal GATT_WRITE_CHAR_VAL_CFM for this 1st step, it
 *      triggers the 2nd step in function handleGattWriteCharValCFM defined in
 *      file alert_client.c and so on.
 *
 *      Whole configuration process will take following steps:
 *      1. Enable notification on client descriptors of new alert 
 *         characteristics.
 *      2. Enable notification on client descriptors  of unread alert 
 *         characteristics
 *      3. Read the supported new alert category.
 *      4. Read the supported unread alert category.
 *      5. Enable the all supported new alerts notifications
 *      6. Enable all unread supported alert notifications
 *
 *      7. Write immediate new alert notification.
 *      8. Write immediate unread alert notification.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void ConfigureAlertNotificationServer(uint16 cid)
{

    /* Start with enabling notifications on client configuration descriptors of 
     * new alert characteristic.
     */
    g_app_data.conf_handle_ptr = &g_app_ans_data.new_alert_ccd_hndl;
    MainEnableNotifications(cid, g_app_ans_data.new_alert_ccd_hndl);

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleANSGattReadCharValCFM
 *
 *  DESCRIPTION
 *      This function handles GATT_READ_CHAR_VAL_CFM messages related to alert
 *      notification profile received from the firmware.
 *
  *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void handleANSGattReadCharValCFM(GATT_READ_CHAR_VAL_CFM_T *cfm)
{
    uint8 cat_id;
    if(g_app_data.conf_handle_ptr == &g_app_ans_data.new_alert_cat_hndl)
    {
        /* This read characteristic value confirmation is for new alert category
         * read.
         */

        /* Record this value */
        MemCopy(g_app_ans_data.supported_new_alert_cat_value, cfm->value, 2);

        DisplaySupportedCategories(alert_type_new);

        /* Read the supported unread alert categories */
        g_app_data.conf_handle_ptr = 
                        &g_app_ans_data.unread_alert_cat_hndl;
        GattReadCharValue(cfm->cid, 
                        g_app_ans_data.unread_alert_cat_hndl);
    }
    else if(g_app_data.conf_handle_ptr == 
                            &g_app_ans_data.unread_alert_cat_hndl)
    {
        /* This read characteristic value confirmation is for unread alert 
         * category read.
         */

        /* Record this value */
        MemCopy(g_app_ans_data.supported_unread_alert_cat_value, cfm->value, 2);
        DisplaySupportedCategories(alert_type_unread);

        /* There is nothing else left to read. The next step is to write
         * required configurations to the various configuration characteristics.
         */

        /* Start the writing with this characteristic */
        g_app_data.conf_handle_ptr = &g_app_ans_data.alert_ctrl_hndl;

        /* We now know which categories are supported by
         * the remote device. This means that we can now subscribe to those
         * of interest.
         * If we want to enable all the categories, Use the special value 0xFF
         * If we don't want all the categories, then refer following page:
         *
         * http://developer.bluetooth.org/gatt/characteristics/Pages/
         * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.
         * alert_notification_control_point.xml.
         *
         * and enable all the required categories one by one
         */
        /* Here we will enable all the categories */

        cat_id = 0xFF; /* Enable all the supported categories */
        SendAlertControlPointCommand(cfm->cid , 
                        alert_ctrl_enable_new_incoming_alerts, cat_id);
    }
    else
    {
        /* This signal may be for other services(PAS) supported by this 
         * application.
         */
    }
}




/*----------------------------------------------------------------------------*
 *  NAME
 *      handleANSGattCharValInd
 *
 *  DESCRIPTION
 *      This function handles alert notification profile specific 
 *      GATT_CHAR_VAL_IND messages received from the firmware. 
 *      These are received when a value on the server is changed,
 *      causing a notification to be generated.
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void handleANSGattCharValInd(GATT_CHAR_VAL_IND_T *ind)
{
    uint8 alerts_cat;
    if(ind->handle == g_app_ans_data.new_alert_hndl)
    {
        /* Record this value. In this case, the value may be many
         * bytes long because it contains some text. We are not
         * interested in the text because we have no display.
         */
        MemCopy(g_app_ans_data.new_alert_value, ind->value, 2);

        /* New alert will be indicated only if number does not match the last 
         * indicated number 
         */
        alerts_cat = g_app_ans_data.new_alert_value[0];
        /* check if alert category is in valid range or not. If it is not valid,
         * ignore this indication.
         */
        if(IsAlertCategoryValid(alerts_cat) &&
           (g_app_ans_data.last_num_new_alert[alerts_cat] != 
                                            g_app_ans_data.new_alert_value[1]))
        {
            /* Store the new number received. 
             * Will be used in next comparison 
             */
            g_app_ans_data.last_num_new_alert[alerts_cat] = 
                                            g_app_ans_data.new_alert_value[1];
            /* A new alert has been received. Indicate this with a beep sound */
            SoundBuzzer(beep_short);
            /* Process this value */
            HandleAlertUpdate(alert_type_new);
        }
    }
    else if(ind->handle == g_app_ans_data.unread_alert_hndl)
    {
        /* Record this value */
        MemCopy(g_app_ans_data.unread_alert_status_value, ind->value, 2);

        /* Unread alert will be indicated only if number does not match the last
         * indicated number 
         */
        alerts_cat = g_app_ans_data.unread_alert_status_value[0];

        /* check if alert category is in valid range or not. If it is not valid,
         * ignore this indication.
         */
        if(IsAlertCategoryValid(alerts_cat) &&
           (g_app_ans_data.last_num_unread_alert[alerts_cat] != 
                                g_app_ans_data.unread_alert_status_value[1]))
        {
            /* Store the new number received. 
             * Will be used in next comparison 
             */
            g_app_ans_data.last_num_unread_alert[alerts_cat] = 
                                  g_app_ans_data.unread_alert_status_value[1];
            /* Process this value */
            HandleAlertUpdate(alert_type_unread);
        }
    }
    else
    {
        /* This signal may be for other services(PAS) supported by this 
         * application.
         */
    }
}



