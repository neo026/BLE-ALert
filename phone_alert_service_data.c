/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      phone_alert_service_data.c
 *
 *  DESCRIPTION
 *      This file keeps information related to Phone alert service 
 *      Discovered
 *
 *  NOTES
 *
 ******************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "phone_alert_service_data.h"
#include "app_gatt.h"
#include "alert_client.h"
 
/*============================================================================*
 *  Private Data
 *============================================================================*/

/* GLobal application phone alert status Service data */
PHONE_ALERT_SERVICE_DATA_T g_app_pas_data;


/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      ConfigurePhoneAlertServer
 *
 *  DESCRIPTION
 *      This function starts the configuration setting.
 *      PAS configuration involves following steps in order :
 *            1. Configure notification of phone alert status characteristic
 *            2. Configure notification of ringer setting characteristic
 *            This completes the PAS configuration
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void ConfigurePhoneAlertServer(uint16 cid)
{
    g_app_data.conf_handle_ptr = 
                     &g_app_pas_data.phone_alert_ccd_hndl;
    MainEnableNotifications(cid, 
                      g_app_pas_data.phone_alert_ccd_hndl);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePASGattCharValInd
 *
 *  DESCRIPTION
 *      This function handles phone alert status profile specific 
 *      GATT_CHAR_VAL_IND messages received from the firmware. 
 *      These are received when a value on the server is changed,
 *      causing a notification to be generated.
 *
 * RETURNS
        Nothing
 *----------------------------------------------------------------------------*/
extern void HandlePASGattCharValInd(GATT_CHAR_VAL_IND_T *ind)
{
    if(ind->handle == g_app_pas_data.phone_alert_hndl)
    {
        /* Read the phone alert status */
        g_app_pas_data.phone_alert_status_value = *ind->value;
        /* Process this value */
        HandlePhoneAlertStatusValue();
    }
    else if(ind->handle == g_app_pas_data.ringer_setting_hndl)
    {
        /* Read the phone alert status */
        g_app_pas_data.ringer_setting_value = *ind->value;
        /* Process this value */
        HandleRingerSettingValue();
    }
    else
    {
        /*This signal is not for phone alert service, ignore it. */
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePASGattReadCharValCFM
 *
 *  DESCRIPTION
 *      This function handles GATT_READ_CHAR_VAL_CFM messages related to Alert
 *      Notification Profile received from the firmware.
 *
  *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void HandlePASGattReadCharValCFM(GATT_READ_CHAR_VAL_CFM_T *cfm)
{
    if(g_app_data.conf_handle_ptr == &g_app_pas_data.phone_alert_hndl)
    {
        /* Read the phone alert status */
        g_app_pas_data.phone_alert_status_value = *cfm->value;
        /* Process this value */
        HandlePhoneAlertStatusValue();

        if(g_app_pas_data.pas_config_ongoing == TRUE)
        {
            /* This boolean variable will be true only in the initial
             * configuration phase. So we need to read ringer setting.
             */
            ReadPASRingerSettingChar(cfm->cid);

        }
    }
    else if(g_app_data.conf_handle_ptr == &g_app_pas_data.ringer_setting_hndl)
    {
        /* Read the phone alert status */
        g_app_pas_data.ringer_setting_value = *cfm->value;
        /* Process this value */
        HandleRingerSettingValue();

        /* Set the boolean flag to FALSE. It will now enable the 
         * button press. If it is already enabled, means we are not in 
         * Initial configuration phase. Then also this assignment won't harm.
         */
        g_app_pas_data.pas_config_ongoing = FALSE;
    }
    else
    {
        /*This signal is not for phone alert service, ignore it. */
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ReadPASPhoneAlertChar
 *
 *  DESCRIPTION
 *      This function reads the phone alert status characteristic
 *
  *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void ReadPASPhoneAlertChar(uint16 cid)
{
    /* Read the phone alert status characteristic */
    g_app_data.conf_handle_ptr = &g_app_pas_data.phone_alert_hndl;
    GattReadCharValue(cid, g_app_pas_data.phone_alert_hndl);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      ReadPASRingerSettingChar
 *
 *  DESCRIPTION
 *      This function reads the Ringer setting characteristic
 *
  *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void ReadPASRingerSettingChar(uint16 cid)
{
    /* Read the ringer setting characteristic */
    g_app_data.conf_handle_ptr = &g_app_pas_data.ringer_setting_hndl;
    GattReadCharValue(cid, g_app_pas_data.ringer_setting_hndl);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitPASData
 *
 *  DESCRIPTION
 *      This function initializes the PAS data
 *
  *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void InitPASData(void)
{
    /* This has been initialized to TRUE because button press will get activated
     * only if this flag changes to FALSE. This flag will become FALSE only if 
     * remote host supports phone alert status service and we have completed our
     * configuration.
     */
    g_app_pas_data.pas_config_ongoing = TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      WritePASRingerCtrlPoint
 *
 *  DESCRIPTION
 *      This function writes different commands on the ringer control point
 *      characterictic of remote phone alert server.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void WritePASRingerCtrlPoint(ringer_ctrl_point_value val)
{
    /* Start the writing with this characteristic */
    g_app_data.conf_handle_ptr = &g_app_pas_data.ringer_ctrl_hndl;

    /* Note that it is a write command */
    GattWriteCharValueReq(g_app_data.st_ucid,
                              GATT_WRITE_COMMAND,
                              g_app_pas_data.ringer_ctrl_hndl,
                              0x01, /* Size of ringer control point value is
                                     * one byte 
                                     */
                              (uint8 *)&val);
}