/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      alert_client.c
 *
 *  DESCRIPTION
 *      This file defines a simple Alert Tag application.
 *
 *  NOTES
 *
 ******************************************************************************/


/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <pio.h>
#include <main.h>
#include <gatt.h>
#include <ls_app_if.h>
#include <nvm.h>
#include <security.h>
#include <mem.h>
#include <macros.h>
#include <gatt_prim.h>
#include <csr_ota.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "user_config.h"
#include "app_gatt.h"
#include "alert_client_gatt.h"
#include "alert_client.h"
#include "gap_service.h"
#include "alert_notification_service_data.h"
#include "nvm_access.h"
#include "alert_client_hw.h"
#include "app_gatt_db.h"
#include "gap_conn_params.h"
#include "gatt_service_data.h"
#include "gatt_service_uuids.h"
#include "phone_alert_service_data.h"
#include "battery_service.h"
#include "local_debug.h"
#include "gatt_service.h"
#include "csr_ota_service.h"
#include "byte_queue.h"   /* Imported from uartio example */
#include <reset.h>

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Intializes the application data */
static void appDataInit(void);

/* This function reads the NVM */
static void readPersistentStore(void);

/* This function handles the signal LM_EV_CONNECTION_COMPLETE */
static void handleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data);

/* This function handles the gatt cancel connect confirmation signal */
static void handleSignalGattCancelConnectCFM(
        GATT_CANCEL_CONNECT_CFM_T *p_event_data);

/* Handles the gatt connect confirmation signal */
static void handleSignalGattConnectCFM(GATT_CONNECT_CFM_T *event_data);

/* This function is called to request remote master to update the connection
 * parameters.
 */
static void requestConnParamUpdate(void);

/* This function is used to request a connection paramter update upon a timer
 * expiry.
 */
static void handleConnParamUpdateRequestTimerExpiry(timer_id tid);

/* This function is used to handle the bonding chance timer expiry. */
static void handleBondingChanceTimerExpiry(timer_id tid);

/* This function handles the connection update confirmation signal received for
 * the request made by application.
 */
static void handleSignalLsConnUpdateSignalCfm(
                                LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data);

/* This function handles the expiry of TGAP(conn_pause_peripheral) timer. */
static void handleGapCppTimerExpiry(timer_id tid);

/* This function handles the event LM_EV_CONNECTION_UPDATE */
static void handleSignalLmConnectionUpdate(
                                       LM_EV_CONNECTION_UPDATE_T* p_event_data);

/* This function handles the connection parameter update indication received. */
static void handleSignalLsConnParamUpdateInd(
                    LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data);

/* This function handles GATT_ACCESS_IND message for attributes maintained by
 * the application.
 */
static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data);

/* This gets called on reception of disconnect complete event.*/
static void handleSignalLmDisconnectComplete(
                HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data);

/*This function handles the simple pairing complete indication */
static void handleSignalSmSimplePairingCompleteInd(
                              SM_SIMPLE_PAIRING_COMPLETE_IND_T  *event_data);

/* This function handles the primary service discovery complete confirmation */
static void handleGattDiscAllPrimServCfm(GATT_DISC_ALL_PRIM_SERV_CFM_T *cfm);

/* This handles the gatt service info indication received */
static void handleGattServInfoInd(GATT_SERV_INFO_IND_T *ind);

/* This function handles the gatt db registeration confirmation signal and
 * starts advertising
 */
static void handleSignalGattDbCfm(GATT_ADD_DB_CFM_T *p_event_data);

/* This function handles the gatt characteristic descriptor info indication
 * received from firmware
 */
static void handleGattCharDescriptorInfoInd(GATT_CHAR_DESC_INFO_IND_T *ind);

/* Handles the signal SM_DIV_APPROVE_IND*/
static void handleSignalSmDivApproveInd(SM_DIV_APPROVE_IND_T *p_event_data);

/* Handles the SM keys indication */
static void handleSignalSmKeysInd(SM_KEYS_IND_T *event_data);

/* This function handles the signal SM_PAIRING_AUTH_IND_T */
static void handleSignalSmPairingAuthInd(SM_PAIRING_AUTH_IND_T *p_event_data);

/* Handles the encryption change signal received from firmware */
static void handleSignalLMEncryptionChange(LM_EVENT_T *event_data);

/* This function handles the gatt characteristic discovery complete
 * comfirmation.
 */
static void handleGattDiscServCharCfm(GATT_DISC_SERVICE_CHAR_CFM_T *ind);

/* This function handles the gatt characteristic info indication received */
static void handleGattCharDeclInfoInd(GATT_CHAR_DECL_INFO_IND_T *ind);

/* Handles the gatt write characteristicvalue comfirmation */
static void handleGattWriteCharValCFM(GATT_WRITE_CHAR_VAL_CFM_T *cfm);

/* This function gets called on exiting app_init state.*/
static void appInitExit(void);

/* This function starts the server configuration */
static void ConfigureTheRemoteServerForNotification(void);

#ifdef ADD_DELAY_IN_READING
/* This function is timer expiry handler function for the timer which
 * application starts to add some delay in reading phone alert status
 */
static void delayInReadingTimerHandler(timer_id tid);
#endif /* ADD_DELAY_IN_READING */
/*============================================================================*
 *  Private Definitions
 *============================================================================*/
/* Maximum number of timers. Two timer will be used for normal application
 * functioning, one will be used for buzzer sounding, one in discovery
 * procedure call, one for button press and one for bonding re-attempt if
 * it fails.
 */
#ifdef ADD_DELAY_IN_READING
/* Two extra timers will be used for adding delay in reading phone Alert
 * characteristic and OTA wait timer.OTA wait timer allows the device to
 * update the application software on it if required using over the air(OTA)
 * service.If no OTA update is there for OTA_WAIT_TIME,disconnect and wait
 * for some one else to connect.
 */
#define MAX_APP_TIMERS                 (8)
#else
#define MAX_APP_TIMERS                 (7)
#endif /* ADD_DELAY_IN_READING */

/* Magic value to check the sanity of NVM region used by the application */
#define NVM_SANITY_MAGIC               (0xAB02)

/* NVM offset for NVM sanity word */
#define NVM_OFFSET_SANITY_WORD         (0)

/* NVM offset for bonded flag */
#define NVM_OFFSET_BONDED_FLAG         (NVM_OFFSET_SANITY_WORD + 1)

/* NVM offset for bonded device bluetooth address */
#define NVM_OFFSET_BONDED_ADDR         (NVM_OFFSET_BONDED_FLAG + \
                                        sizeof(g_app_data.bonded))

/* NVM offset for diversifier */
#define NVM_OFFSET_SM_DIV              (NVM_OFFSET_BONDED_ADDR + \
                                        sizeof(g_app_data.bonded_bd_addr))

/* NVM offset for IRK */
#define NVM_OFFSET_SM_IRK              (NVM_OFFSET_SM_DIV + \
                                        sizeof(g_app_data.diversifier))

/* Number of words of NVM used by application. Memory used by supported
 * services is not taken into consideration here.
 */
#define NVM_MAX_APP_MEMORY_WORDS       (NVM_OFFSET_SM_IRK + \
                                        MAX_WORDS_IRK)

/* TGAP(conn_pause_peripheral) defined in Core Specification Addendum 3 Revision
 * 2. A Peripheral device should not perform a Connection Parameter Update proc-
 * -edure within TGAP(conn_pause_peripheral) after establishing a connection.
 */
#define TGAP_CPP_PERIOD                 (5 * SECOND)

/* TGAP(conn_pause_central) defined in Core Specification Addendum 3 Revision 2.
 * After the Peripheral device has no further pending actions to perform and the
 * Central device has not initiated any other actions within TGAP(conn_pause_ce-
 * -ntral), then the Peripheral device may perform a Connection Parameter Update
 * procedure.
 */
#define TGAP_CPC_PERIOD                 (1 * SECOND)

/*============================================================================*
 *  Private Data
 *============================================================================*/


/* Declare space for application timers. */
static uint16 app_timers[SIZEOF_APP_TIMER * MAX_APP_TIMERS];

/*============================================================================*
 *  Public Data
 *============================================================================*/

/* Alert Notification application data structure */
APP_DATA_T g_app_data;


/* PTS testing flag
 * Following PTS test cases require alert tag application to silent mode on the
 * phone alert notification control point. But our use case keep record of the
 * already present ringer setting and if it finds it NORMAL then only it writes
 * SILENT mode. If it is SILENT then it will write NORMAL.
 * For PTS to pass, we will be sending SILENT even if ringer setting is already
 * silent and this functionality has been kept under a  flag which is required
 * to be defined when any of following PTS test cases is to be executed.
 *
 * This flag will be enabled via user CS keys.
 *
 * PTS test cases:
 *    TC_PPWF_PPC_BV_01_C
 */
bool g_pts_send_only_silent_mode = FALSE;


/* PTS testing flag
 * Following PTS test cases require alert tag application to write disable -
 * notifications and indications on the client configuration descriptor of the
 * new alert and unread alert characteristics. But our use case does not want
 * this. So we are defining following flag which is required to be enabled
 * when any of following PTS test cases is to be executed. This flag
 * will enable the client configuration descriptor notification disabling and
 * make following PTS test cases pass.
 *
 * This flag will be enabled via User CS keys.
 *
 * PTS test cases:
 *  Alert notification Profile test cases:
 *    TC_ANPCF_ANPC_BC_02_C
 *    TC_ANPCF_ANPC_BC_04_C
 *  Phone alert notification profile test cases:
 *    TC_PPCF_PPC_BV_02_C
 *    TC_PPCF_PPC_BV_04_C
 */
bool g_pts_disable_notify_ccd = FALSE;

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      appDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialize Alert tag application data
 *      structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

static void appDataInit(void)
{
    /* Make connection Id invalid */
    g_app_data.st_ucid = GATT_INVALID_UCID;
    
    /* Reset the authentication failure flag */
    g_app_data.auth_failure = FALSE;

    /* Reset the application timer */
    TimerDelete(g_app_data.app_tid);
    g_app_data.app_tid = TIMER_INVALID;

    /* Reset the OTA wait timer */
    TimerDelete(g_app_data.ota_wait_tid);
    g_app_data.ota_wait_tid = TIMER_INVALID;

    /* Reset the connection paramter update timer. */
    TimerDelete(g_app_data.conn_param_update_tid);
    g_app_data.conn_param_update_tid = TIMER_INVALID;
    g_app_data.cpu_timer_value = 0;


#ifdef ADD_DELAY_IN_READING
    TimerDelete(g_app_data.delay_read_tid);
    g_app_data.delay_read_tid = TIMER_INVALID;
#endif /* ADD_DELAY_IN_READING */

    /* Delete the bonding chance timer */
    TimerDelete(g_app_data.bonding_reattempt_tid);
    g_app_data.bonding_reattempt_tid = TIMER_INVALID;

    /* Reset advertising timer value */
    g_app_data.advert_timer_value = 0;

    /* Reset encryption enabled flag */
    g_app_data.encrypt_enabled = FALSE;

    /* Reset the conf handle pointer*/
    g_app_data.conf_handle_ptr = NULL;

    g_app_ans_data.config_in_progress = FALSE;

    g_app_data.connected_device_profiles = 0;

    /* Reset the connection parameter variables. */
    g_app_data.conn_interval = 0;
    g_app_data.conn_latency = 0;
    g_app_data.conn_timeout = 0;

    /* Initialize the hardware data */
    AppHwDataInit();

    /* Initialise GAP Data structure */
    GapDataInit();

    /* Battery Service data initialisation */
    BatteryDataInit();

    /* Initialize the PAS data */
    InitPASData();
  
    /* Initialise GATT Data structure */
    GattDataInit();

    /* OTA Service data initialisation */
    OtaDataInit();

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      readPersistentStore
 *
 *  DESCRIPTION
 *      This function is used to initialize and read NVM data
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void readPersistentStore(void)
{
    /* NVM offset for supported services */
    uint16 nvm_offset = NVM_MAX_APP_MEMORY_WORDS;
    uint16 nvm_sanity = 0xffff;

    /* Read persistent storage to know if the device was last bonded.
     * If the device was bonded, trigger fast undirected advertisements by
     * setting the white list for bonded host. If the device was not bonded,
     * trigger undirected advertisements for any host to connect.
     */

    Nvm_Read(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);

    if(nvm_sanity == NVM_SANITY_MAGIC)
    {
        /* Read bonded flag from NVM */
        Nvm_Read((uint16*)&g_app_data.bonded, sizeof(g_app_data.bonded),
                           NVM_OFFSET_BONDED_FLAG);

        if(g_app_data.bonded)
        {

            /* Bonded host BD address will only be stored if bonded flag
             * is set to TRUE.
             */
            Nvm_Read((uint16*)&g_app_data.bonded_bd_addr,
                       sizeof(TYPED_BD_ADDR_T),
                       NVM_OFFSET_BONDED_ADDR);
        }

        else /* Case when we have only written NVM_SANITY_MAGIC to NVM but
              * application didn't get bonded to any host in the last powered
              * session.
              */
        {
            g_app_data.bonded = FALSE;
        }

        /* Read the diversifier associated with the presently bonded/last
         * bonded device.
         */
        Nvm_Read(&g_app_data.diversifier,
                 sizeof(g_app_data.diversifier),
                 NVM_OFFSET_SM_DIV);

        /* Read device name and length from NVM */
        GapReadDataFromNVM(&nvm_offset);
    }
    else /* NVM sanity check failed means either the device is being brought up
          * for the first time or memory has got corrupted in which case
          * discard the data and start fresh.
          */
    {

        nvm_sanity = NVM_SANITY_MAGIC;

        /* Write NVM sanity word to the NVM */
        Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);

        /* The device will not be bonded as it is coming up for the first time
         */
        g_app_data.bonded = FALSE;

        /* Write bonded status to NVM */
        Nvm_Write((uint16*)&g_app_data.bonded, sizeof(g_app_data.bonded),
                            NVM_OFFSET_BONDED_FLAG);

        /* When the application is coming up for the first time after flashing
         * the image to it, it will not have bonded to any device. So, no LTK
         * will be associated with it. Hence, set the diversifier to 0.
         */
        g_app_data.diversifier = 0;

        /* Write the same to NVM. */
        Nvm_Write(&g_app_data.diversifier,
                  sizeof(g_app_data.diversifier),
                  NVM_OFFSET_SM_DIV);

        /* Initialize the NVM offset for the gap service data and store GAP
         * data in NVM
         */
        GapInitWriteDataToNVM(&nvm_offset);
    }

    /* If device is bonded and bonded address is resolvable then read the
     * bonded device's IRK
     */
    if(g_app_data.bonded == TRUE &&
        GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr))
    {
        Nvm_Read((uint16*)g_app_data.central_device_irk.irk,
                            MAX_WORDS_IRK,
                            NVM_OFFSET_SM_IRK);
    }
    /* Read Battery service data from NVM if the devices are bonded and
     * update the offset with the number of word of NVM required by
     * this service
     */
    BatteryReadDataFromNVM(&nvm_offset);

    GattReadDataFromNVM(&nvm_offset);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      requestConnParamUpdate
 *
 *  DESCRIPTION
 *      This function is used to send L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void requestConnParamUpdate(void)
{
    ble_con_params app_pref_conn_param;

    /* Decide which parameter values are to be used.
     */
    if(g_app_data.num_conn_update_req <= CPU_SELF_PARAMS_MAX_ATTEMPTS)
    {
        app_pref_conn_param.con_max_interval = PREFERRED_MAX_CON_INTERVAL;
        app_pref_conn_param.con_min_interval = PREFERRED_MIN_CON_INTERVAL;
        app_pref_conn_param.con_slave_latency = PREFERRED_SLAVE_LATENCY;
        app_pref_conn_param.con_super_timeout = PREFERRED_SUPERVISION_TIMEOUT;
    }
    else
    {
        app_pref_conn_param.con_max_interval = APPLE_MAX_CON_INTERVAL;
        app_pref_conn_param.con_min_interval = APPLE_MIN_CON_INTERVAL;
        app_pref_conn_param.con_slave_latency = APPLE_SLAVE_LATENCY;
        app_pref_conn_param.con_super_timeout = APPLE_SUPERVISION_TIMEOUT;
    }


    /* Send connection parameter update request. */
    if(LsConnectionParamUpdateReq(&(g_app_data.con_bd_addr),
                                     &app_pref_conn_param) != ls_err_none)
    {
        /* Connection parameter update request should not have failed.
         * Report panic
         */
        ReportPanic(app_panic_con_param_update);
    }
}

/*---------------------------------------------------------------------------
 *
 *  NAME
 *      handleSignalLmEvConnectionComplete
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_CONNECTION_COMPLETE.
 *
 *  RETURNS
 *      Nothing.
 *

*----------------------------------------------------------------------------*/
static void handleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data)
{
    /* Store the connection parameters. */
    g_app_data.conn_interval = p_event_data->data.conn_interval;
    g_app_data.conn_latency = p_event_data->data.conn_latency;
    g_app_data.conn_timeout = p_event_data->data.supervision_timeout;
}




/*----------------------------------------------------------------------------*
 *  NAME
 *      handleConnParamUpdateRequestTimerExpiry
 *
 *  DESCRIPTION
 *      This function is used to request a connection parameter update upon a
 *      timer expiry.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleConnParamUpdateRequestTimerExpiry(timer_id tid)
{
    if(g_app_data.conn_param_update_tid == tid)
    {
        g_app_data.conn_param_update_tid = TIMER_INVALID;
        g_app_data.cpu_timer_value = 0;

        /* Increment the counter */
        g_app_data.num_conn_update_req++;

        /* Request connection parameter update. */
        requestConnParamUpdate();

    }/* Else it may be due to some race condition. Ignore it. */
}


/*-----------------------------------------------------------------------------*
 *  NAME
 *      handleGapCppTimerExpiry
 *
 *  DESCRIPTION
 *      This function handles the expiry of TGAP(conn_pause_peripheral) timer.
 *      It starts the TGAP(conn_pause_central) timer, during which, if no activ-
 *      -ity is detected from the central device, a connection parameter update
 *      request is sent.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleGapCppTimerExpiry(timer_id tid)
{
    if(g_app_data.conn_param_update_tid == tid)
    {
        g_app_data.conn_param_update_tid =
                           TimerCreate(TGAP_CPC_PERIOD, TRUE,
                                       handleConnParamUpdateRequestTimerExpiry);
        g_app_data.cpu_timer_value = TGAP_CPC_PERIOD;
    }
}



/*----------------------------------------------------------------------------*
 *  NAME
 *      handleBondingChanceTimerExpiry
 *
 *  DESCRIPTION
 *      This function is handle the expiry of bonding chance timer.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleBondingChanceTimerExpiry(timer_id tid)
{
    if(g_app_data.bonding_reattempt_tid== tid)
    {
        g_app_data.bonding_reattempt_tid= TIMER_INVALID;
        /* The bonding chance timer has expired. This means the remote has not
         * encrypted the link using old keys. Disconnect the link.
         */
        SetAppState(app_disconnecting);
    }/* Else it may be due to some race condition. Ignore it. */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattCancelConnectCFM
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CANCEL_CONNECT_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalGattCancelConnectCFM(GATT_CANCEL_CONNECT_CFM_T 
                                             *p_event_data)
{
    if(p_event_data->result != sys_status_success)
    {
        return;
    }
    if(g_app_data.pairing_remove_button_pressed)
    {
        /* Case when user performed an extra long button press for removing
         * pairing.
         */
        g_app_data.pairing_remove_button_pressed = FALSE;

        /* Reset and clear the whitelist */
        LsResetWhiteList();

        /* Trigger fast advertisements */
        if(g_app_data.state == app_fast_advertising)
        {
            GattTriggerFastAdverts();
        }
        else
        {
            SetAppState(app_fast_advertising);
        }
    }
    else
    {
        /*Handling signal as per current state */
        switch(g_app_data.state)
        {
            /* In case of fast advertising, Start slow advertising
             * In case of slow advertising, stop advertising
             */
            case app_fast_advertising:
            {
                SetAppState(app_slow_advertising);
            }
            break;
            case app_slow_advertising:
            {
                SetAppState(app_idle);
            }
            break;
            default:
            {
                /* Control should never come here */
                ReportPanic(app_panic_invalid_state);
            }
            break;
        }
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmDisconnectComplete
 *
 *  DESCRIPTION
 *      This function handles LM Disconnect Complete event which is received
 *      at the completion of disconnect procedure triggered either by the
 *      device or remote host or because of link loss
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

static void handleSignalLmDisconnectComplete(
                HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data)
{
    if(OtaResetRequired())
    {
        OtaReset();
        /* The OtaReset function does not return */
    }
    else
    {
        /* Reset the connection parameter variables. */
        g_app_data.conn_interval = 0;
        g_app_data.conn_latency = 0;
        g_app_data.conn_timeout = 0;


        /* LM_EV_DISCONNECT_COMPLETE event can have following disconnect
         * reasons:
         *
         * HCI_ERROR_CONN_TIMEOUT - Link Loss case
         * HCI_ERROR_CONN_TERM_LOCAL_HOST - Disconnect triggered by device
         * HCI_ERROR_OETC_* - Other end (i.e., remote host) terminated
         * connection
         */
        /*Handling signal as per current state */
        switch(g_app_data.state)
        {
            case app_connected: /* FALLTHROUGH */
            case app_connected_discovering: /* FALLTHROUGH */
            case app_disconnecting:
            {
                appDataInit();
                /* Restart advertising */
                SetAppState(app_fast_advertising);

            }
            break;

            default:
            {
                /* Control should never come here */
                ReportPanic(app_panic_invalid_state);
            }
            break;
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattConnectCFM
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CONNECT_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalGattConnectCFM(GATT_CONNECT_CFM_T *event_data)
{
    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_fast_advertising: /* FALLTHROUGH */
        case app_slow_advertising:
        {
            if(event_data->result == sys_status_success)
            {
                /* Initialize the pas_config_ongoing flag to TRUE. If remote -
                 * device supports phone alert notification service then this -
                 * gets used at the time of phone alert server configuration.
                 */
                g_app_pas_data.pas_config_ongoing = TRUE;

                /* Store bd address */
                g_app_data.con_bd_addr = event_data->bd_addr;

                /* Store received UCID */
                g_app_data.st_ucid = event_data->cid;

                SetAppState(app_connected);

                if(g_app_data.bonded == TRUE &&
                    GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr) &&
                    (SMPrivacyMatchAddress(&g_app_data.con_bd_addr,
                                           g_app_data.central_device_irk.irk,
                                           MAX_NUMBER_IRK_STORED,
                                           MAX_WORDS_IRK) < 0))
                {
                    /* Application was bonded to a remote device with
                     * resolvable random address and application has failed to
                     * resolve the remote device address to which we just
                     * connected So disconnect and start advertising again
                     */
                    g_app_data.auth_failure = TRUE;
                    SetAppState(app_disconnecting);
                }
                else
                {
                    /* If we are bonded to this host, then it may be appropriate
                     * to indicate that the database is not now what it had
                     * previously.
                     */
                    if(g_app_data.bonded)
                    {
                        GattOnConnection(event_data->cid);
                    }

                    /* Initiate slave security request if the remote host
                     * supports security feature. This is added for this device
                     * to work against legacy hosts that don't support security
                     */

                    /* Security supported by the remote host */
                    if(g_app_data.bonded == TRUE &&
                        !GattIsAddressResolvableRandom(&g_app_data.con_bd_addr))
                    {
                        /* Application sends slave security request only if it
                         * bonded to some device. If bonded device has
                         * resolvable random address, then also we don't send
                         * this request
                         */
                        SMRequestSecurityLevel(&g_app_data.con_bd_addr);
                    }

                    /* Start service discovery procedure */
                    StartDiscoveryProcedure();
                }

            }
            else
            {
                /* Else wait for user activity before we start advertising
                 * again
                 */
                SetAppState(app_idle);
            }
        }
        break;
        default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnUpdateSignalCfm
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_UPDATE_SIGNALLING_RSP
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalLsConnUpdateSignalCfm(
                                 LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: /* FALLTHROUGH */
        case app_connected_discovering: /* FALLTHROUGH */
        case app_disconnecting:
        {
            /* Received in response to the L2CAP_CONNECTION_PARAMETER_UPDATE
             * request sent from the slave after service discovery. If the
             * request has failed, the device should again send same request
             * only after Tgap(conn_param_timeout). Refer Bluetooth 4.0
             * spec Vol 3 Part C, Section 9.3.9 and profile spec.
             */
            if (((p_event_data->status) != ls_err_none)
                && (g_app_data.num_conn_update_req <
                                MAX_NUM_CONN_PARAM_UPDATE_REQS))
            {
                /* Delete timer if running */
                TimerDelete(g_app_data.conn_param_update_tid);

                g_app_data.conn_param_update_tid= TimerCreate(
                                       GAP_CONN_PARAM_TIMEOUT,
                                       TRUE,
                                       handleConnParamUpdateRequestTimerExpiry);
                g_app_data.cpu_timer_value = GAP_CONN_PARAM_TIMEOUT;
            }
        }
        break;
        default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmConnectionUpdate
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_CONNECTION_UPDATE.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalLmConnectionUpdate(
                                       LM_EV_CONNECTION_UPDATE_T* p_event_data)
{
    switch(g_app_data.state)
    {
        case app_connected:
        case app_connected_discovering:
        case app_disconnecting:
        {
            /* Store the new connection parameters. */
            g_app_data.conn_interval = p_event_data->data.conn_interval;
            g_app_data.conn_latency = p_event_data->data.conn_latency;
            g_app_data.conn_timeout = p_event_data->data.supervision_timeout;
        }
        break;

        default:
            /* Connection parameter update indication received in unexpected
             * application state.
             */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnParamUpdateInd
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_IND
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalLsConnParamUpdateInd(
                                 LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data)
{
    /* The application had already received the new connection parameters while
     * handling event LM_EV_CONNECTION_UPDATE.
     * Check if new parameters comply with application preferred parameters.
     * If not, application shall trigger Connection parameter update procedure
     */

    /* Delete timer if running */
    TimerDelete(g_app_data.conn_param_update_tid);
    g_app_data.conn_param_update_tid = TIMER_INVALID;
    g_app_data.cpu_timer_value = 0;

    if(g_app_data.conn_interval < PREFERRED_MIN_CON_INTERVAL ||
       g_app_data.conn_interval > PREFERRED_MAX_CON_INTERVAL
#if PREFERRED_SLAVE_LATENCY
       || g_app_data.conn_latency < PREFERRED_SLAVE_LATENCY
#endif
      )
    {
        /* Set the num of connection update attempts to zero */
        g_app_data.num_conn_update_req = 0;

        /* Start timer to trigger Connection Parameter Update procedure */
        g_app_data.conn_param_update_tid = TimerCreate(GAP_CONN_PARAM_TIMEOUT,
                                     TRUE,
                                     handleConnParamUpdateRequestTimerExpiry);
        g_app_data.cpu_timer_value = GAP_CONN_PARAM_TIMEOUT;


    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattAccessInd
 *
 *  DESCRIPTION
 *      This function handles GATT_ACCESS_IND message for attributes
 *      maintained by the application.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data)
{

    /*Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected:
        case app_connected_discovering:
        {
            /* If connection parameter update timer with value TGAP_CPC_PERIOD
             * is running, restart the timer.
             */
            if(g_app_data.cpu_timer_value == TGAP_CPC_PERIOD)
            {
                TimerDelete(g_app_data.conn_param_update_tid);
                g_app_data.conn_param_update_tid = TimerCreate(TGAP_CPC_PERIOD,
                                 TRUE, handleConnParamUpdateRequestTimerExpiry);
            }

            /* Received GATT ACCESS IND with write access */
            if(p_event_data->flags ==
                (ATT_ACCESS_WRITE |
                 ATT_ACCESS_PERMISSION |
                 ATT_ACCESS_WRITE_COMPLETE))
            {
                HandleAccessWrite(p_event_data);
            }
            /* Received GATT ACCESS IND with read access */
            else if(p_event_data->flags ==
                (ATT_ACCESS_READ |
                ATT_ACCESS_PERMISSION))
            {
                HandleAccessRead(p_event_data);
            }
            else
            {
                GattAccessRsp(p_event_data->cid, p_event_data->handle,
                              gatt_status_request_not_supported,
                              0, NULL);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*---------------------------------------------------------------------------
 *
 *  NAME
 *      handleSignalSmPairingAuthInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_PAIRING_AUTH_IND. This message will
 *      only be received when the peer device is initiating 'Just Works'
 *      pairing.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *

*----------------------------------------------------------------------------*/
static void handleSignalSmPairingAuthInd(SM_PAIRING_AUTH_IND_T *p_event_data)
{
    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: /* FALLTHROUGH */
        case app_connected_discovering:
        {
            /* Authorise the pairing request if the application is NOT bonded */
            if(!g_app_data.bonded)
            {
                SMPairingAuthRsp(p_event_data->data, TRUE);
            }
            else /* Otherwise Reject the pairing request */
            {
                SMPairingAuthRsp(p_event_data->data, FALSE);
            }
        }
        break;

        default:
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmSimplePairingCompleteInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_SIMPLE_PAIRING_COMPLETE_IND
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalSmSimplePairingCompleteInd(
                                SM_SIMPLE_PAIRING_COMPLETE_IND_T *event_data)
{
    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: /* FALLTHROUGH */
        case app_connected_discovering:
        {
            if(event_data->status == sys_status_success)
            {
                /* Pairing succeeded. Application is bonded now */
                
                if(g_app_data.bonding_reattempt_tid != TIMER_INVALID)
                {                   
                    /* Delete the bonding chance timer */
                    TimerDelete(g_app_data.bonding_reattempt_tid);
                    g_app_data.bonding_reattempt_tid = TIMER_INVALID;
                }

                g_app_data.bonded = TRUE;
                g_app_data.bonded_bd_addr = event_data->bd_addr;


                /* Write one word bonded flag */
                Nvm_Write((uint16*)&g_app_data.bonded,
                          sizeof(g_app_data.bonded),
                          NVM_OFFSET_BONDED_FLAG);

                /* Write typed bd address of bonded host */
                Nvm_Write((uint16*)&g_app_data.bonded_bd_addr,
                         sizeof(TYPED_BD_ADDR_T), NVM_OFFSET_BONDED_ADDR);

                if(!GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr))
                {
                    /* White list is configured with the bonded host address */
                    if(LsAddWhiteListDevice(&g_app_data.bonded_bd_addr) !=
                                        ls_err_none)
                    {
                        ReportPanic(app_panic_add_whitelist);
                    }
                }
                
                /* Notify the Gatt service about the pairing */
                GattBondingNotify();

                /* Notify the Battery service about the pairing */
                BatteryBondingNotify();
            }
            else
            {
                /* Pairing has failed.
                 * 1. If pairing has failed due to repeated attempts, the
                 *    application should immediately disconnect the link.
                 * 2. The application was bonded and pairing has failed.
                 *    Since the application was using whitelist, so the remote
                 *    device has same address as our bonded device address.
                 *    The remote connected device may be a genuine one but
                 *    instead of using old keys, wanted to use new keys. We
                 *    don't allow bonding again if we are already bonded but we
                 *    will give some time to the connected device to encrypt the
                 *    link using the old keys. if the remote device encrypts the
                 *    link in that time, it's good. Otherwise we will disconnect
                 *    the link.
                 */
                 if(event_data->status == sm_status_repeated_attempts)
                 {
                    SetAppState(app_disconnecting);
                 }
                 else if(g_app_data.bonded)
                 {
                    g_app_data.bonding_reattempt_tid = TimerCreate(
                                           BONDING_CHANCE_TIMER,
                                           TRUE,
                                           handleBondingChanceTimerExpiry);
                 }
                 else
                 {
                    /* If the application was not bonded and pairing has failed,
                     * the application will wait for PAIRING_WAIT_TIMER timer
                     * value for remote host to retry pairing.On the timer 
                     * expiry,the application will disconnect the link.
                     * Timer bonding_reattempt_tid has been reused in this case.
                     */
                    if(g_app_data.bonding_reattempt_tid == TIMER_INVALID)
                    {
                        g_app_data.bonding_reattempt_tid = TimerCreate(
                                PAIRING_WAIT_TIMER,
                                TRUE,
                                handleBondingChanceTimerExpiry);                   
                    }
                }
            }
        }
        break;
        default:
        {
            /* Firmware may send this signal after disconnection. So don't
             * panic but ignore this signal.
             */
        }
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmDivApproveInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_DIV_APPROVE_IND.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

static void handleSignalSmDivApproveInd(SM_DIV_APPROVE_IND_T *p_event_data)
{
    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        /* Request for approval from application comes only when pairing is not
         * in progress
         */
        case app_connected: /* FALLTHROUGH */
        case app_connected_discovering:
        {
            sm_div_verdict approve_div = SM_DIV_REVOKED;

            /* Check whether the application is still bonded Then check
             * whether the diversifier is the same as the one stored by the
             * application
             */
            if(g_app_data.bonded)
            {
                if(g_app_data.diversifier == p_event_data->div)
                {
                    approve_div = SM_DIV_APPROVED;
                }
            }

            SMDivApproval(p_event_data->cid, approve_div);
        }
        break;

        default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;

    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattServInfoInd
 *
 *  DESCRIPTION
 *      This function handles GATT_SERV_INFO_IND messages
 *      received from the firmware.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
static void handleGattServInfoInd(GATT_SERV_INFO_IND_T *ind)
{
    /* Check the UUID to see whether it is one of interest. */

    /* We are not interested in 128-bit UUIDs */
    if(ind->uuid_type == GATT_UUID16)
    {
        /* Alert notification service */
        if((ind->uuid[0]) == UUID_ALERT_NOTIFICATION_SERVICE)
        {
            g_app_ans_data.ans_start_handle = ind->strt_handle;
            g_app_ans_data.ans_end_handle = ind->end_handle;
            /* Alert notification service supported */
            g_app_data.connected_device_profiles |= ans_supported;
        }
        else if((ind->uuid[0]) == UUID_GATT)  /* Gatt service */
        {
            g_app_gatt_data.gatt_start_hndl = ind->strt_handle;
            g_app_gatt_data.gatt_end_hndl = ind->end_handle;
        }
        else if((ind->uuid[0]) == UUID_PHONE_ALERT_STATUS)
        {
            /* Phone alert status service */
            g_app_pas_data.pas_start_hndl = ind->strt_handle;
            g_app_pas_data.pas_end_hndl = ind->end_handle;
            /* Phone alert status service supported */
            g_app_data.connected_device_profiles |= pas_supported;
        }
        /*We are not interested in any other service */
    }
}



/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattDiscAllPrimServCfm
 *
 *  DESCRIPTION
 *      This function handles GATT_DISC_ALL_PRIM_SERV_CFM messages
 *      received from the firmware. Receipt of one of these messages
 *      means that the service discovery is complete
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleGattDiscAllPrimServCfm(GATT_DISC_ALL_PRIM_SERV_CFM_T *cfm)
{
    uint16       start_hndl;
    uint16       end_hndl;

    /* Find the range which will include all the three services */
    start_hndl = MIN_SET(
               MIN_SET(((g_app_data.connected_device_profiles & ans_supported)?
               g_app_ans_data.ans_start_handle : 0xFFFF),
               ((g_app_data.connected_device_profiles & pas_supported) ?
               g_app_pas_data.pas_start_hndl : 0xFFFF)) ,
               g_app_gatt_data.gatt_start_hndl);

    end_hndl = MAX(
               MAX(((g_app_data.connected_device_profiles & ans_supported) ?
               g_app_ans_data.ans_end_handle : 0x0000),
               ((g_app_data.connected_device_profiles & pas_supported) ?
               g_app_pas_data.pas_end_hndl : 0x0000)) ,
                g_app_gatt_data.gatt_end_hndl);

    if(cfm->result == sys_status_success)
    {
        /* Check that the remote device supports something interesting */
        if((g_app_data.connected_device_profiles & ans_supported) ||
            (g_app_data.connected_device_profiles & pas_supported))
        {
            /* Discover all the characteristics of all the services */
            GattDiscoverServiceChar(g_app_data.st_ucid,
                                    start_hndl,
                                    end_hndl,
                                    GATT_UUID_NONE,
                                    NULL);
        }
        else
        {
            /* Reset the OTA wait timer */
            TimerDelete(g_app_data.ota_wait_tid);
            g_app_data.ota_wait_tid = TIMER_INVALID;

            /* Start a timer to allow the device to update the application
             * software on it over the air (OTA) if required.
             * If no OTA update is there for OTA_WAIT_TIME,disconnect and wait
             * for some one else to connect.
             */
            g_app_data.ota_wait_tid = TimerCreate(OTA_WAIT_TIME, TRUE,
                                                  OtaTimerHandler);

        }
    }
    else
    {
        /* Something went wrong. We can't recover, so disconnect. */
        SetAppState(app_disconnecting);
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattCharDeclInfoInd
 *
 *  DESCRIPTION
 *      This function handles GATT_CHAR_DECL_INFO_IND messages
 *      received from the firmware.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
static void handleGattCharDeclInfoInd(GATT_CHAR_DECL_INFO_IND_T *ind)
{
    uint16 uuid = ind->uuid[0];

    /* Check the UUID to see whether it is one of interest. */

    /* We are not interested in 128-bit UUIDs*/
    if(ind->uuid_type == GATT_UUID16)
    {
        if(uuid == UUID_NEW_ALERT_SUPPORTED_CATEGORY ||
            uuid == UUID_NEW_ALERT_CHARACTERISTIC ||
            uuid == UUID_SUPPORTED_UNREAD_ALERT_CATEGORY ||
            uuid == UUID_UNREAD_ALERT_CHARACTERISTIC ||
            uuid == UUID_ALERT_NOTIFICATION_CONTROL_POINT ||
            uuid == UUID_ALERT_STATUS ||
            uuid == UUID_RINGER_SETTING ||
            uuid == UUID_RINGER_CONTROL_POINT)
        {
            if(g_app_data.conf_handle_ptr)
            {
                /* 2 has been subtracted because end handle of last
                 * characteristic received will be 2 less than the value handle
                 * received just now (one for characteristic declaration handle
                 * and one for value attribute)
                 */
                *g_app_data.conf_handle_ptr = (ind->val_handle) - 2;
                g_app_data.conf_handle_ptr = NULL;
            }
        }


        if(uuid == UUID_NEW_ALERT_SUPPORTED_CATEGORY)
        {
            g_app_ans_data.new_alert_cat_hndl = ind->val_handle;
        }
        else if(uuid == UUID_NEW_ALERT_CHARACTERISTIC)
        {
            g_app_ans_data.new_alert_hndl = ind->val_handle;

            /* Assign service end handle to the characteristic end handle.
             * If this was not the last characteristic handle discovered, then
             * characteristic end handle will get updated with the right value.
             * If it is the last handle discovered then characteristic last
             * handle will be same as service end handle
             */
            g_app_ans_data.new_alert_char_end_hndl =
                        g_app_ans_data.ans_end_handle;
            g_app_data.conf_handle_ptr =
                        &g_app_ans_data.new_alert_char_end_hndl;
        }
        else if(uuid == UUID_SUPPORTED_UNREAD_ALERT_CATEGORY)
        {
            g_app_ans_data.unread_alert_cat_hndl = ind->val_handle;
        }
        else if(uuid == UUID_UNREAD_ALERT_CHARACTERISTIC)
        {
            g_app_ans_data.unread_alert_hndl = ind->val_handle;

            /* Assign service end handle to the characteristic end handle.
             * If this was not the last characteristic handle discovered, then
             * characteristic end handle will get updated with the right value.
             * If it is the last handle discovered then characteristic last
             * handle will be same as service end handle
             */
            g_app_ans_data.unread_alert_char_end_hndl =
                        g_app_ans_data.ans_end_handle;
            g_app_data.conf_handle_ptr =
                        &g_app_ans_data.unread_alert_char_end_hndl;
        }
        else if(uuid == UUID_ALERT_NOTIFICATION_CONTROL_POINT)
        {
            /* Alert notification control point characteristic */
            g_app_ans_data.alert_ctrl_hndl = ind->val_handle;
        }
        else if(uuid == UUID_ALERT_STATUS)
        {
            g_app_pas_data.phone_alert_hndl = ind->val_handle;
            /* Assign service end handle to the characteristic end handle.
             * If this was not the last characteristic handle discovered, then
             * characteristic end handle will get updated with the right value.
             * If it is the last handle discovered then char last handle will
             * be same as service end handle
             */
            g_app_pas_data.phone_alert_char_end_hndl =
                        g_app_pas_data.pas_end_hndl;
            g_app_data.conf_handle_ptr =
                        &g_app_pas_data.phone_alert_char_end_hndl;
        }
        else if(uuid == UUID_RINGER_SETTING)
        {
            g_app_pas_data.ringer_setting_hndl = ind->val_handle;
            /* Assign service end handle to the characteristic end handle.
             * If this was not the last characteristic handle discovered, then
             * characteristic end handle will get updated with the right value.
             * If it is the last handle discovered then char last handle will
             * be same as service end handle
             */
            g_app_pas_data.ringer_setting_end_hndl =
                        g_app_pas_data.pas_end_hndl;
            g_app_data.conf_handle_ptr =
                        &g_app_pas_data.ringer_setting_end_hndl;
        }
        else if(uuid == UUID_RINGER_CONTROL_POINT)
        {
            g_app_pas_data.ringer_ctrl_hndl = ind->val_handle;
        }
        else if(uuid == UUID_SERVICE_CHANGED)
        {
            g_app_data.conf_handle_ptr = NULL;
            g_app_gatt_data.service_change_hndl = ind->val_handle;
        }
        else
        {
            /* Not interested in this characteristic */
            g_app_data.conf_handle_ptr = NULL;
        }

    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattDiscServCharCfm
 *
 *  DESCRIPTION
 *      This function handles GATT_DISC_SERVICE_CHAR_CFM messages
 *      received from the firmware.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
static void handleGattDiscServCharCfm(GATT_DISC_SERVICE_CHAR_CFM_T *ind)
{
    if(ind->result == sys_status_success)
    {
        /* If ANS is supported then start with the ANS otherwise start with
         * PAS
         */
        if(g_app_data.connected_device_profiles & ans_supported)
        {
            /* Discover characteristic descriptors of ANS new alert
             * characteristic
             */
            GattDiscoverAllCharDescriptors(g_app_data.st_ucid,
                                       g_app_ans_data.new_alert_hndl,
                                       g_app_ans_data.new_alert_char_end_hndl);
            g_app_data.conf_handle_ptr = &g_app_ans_data.new_alert_ccd_hndl;
        }
        else if(g_app_data.connected_device_profiles & pas_supported)
        {
            /* Discover characteristic descriptors of ANS new alert
             * characteristic
             */
            GattDiscoverAllCharDescriptors(g_app_data.st_ucid,
                                      g_app_pas_data.phone_alert_hndl,
                                      g_app_pas_data.phone_alert_char_end_hndl);
            g_app_data.conf_handle_ptr = &g_app_pas_data.phone_alert_ccd_hndl;

        }
    }
    else
    {
        /* Something went wrong. We can't recover, so disconnect. */
        SetAppState(app_disconnecting);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattCharDescriptorInfoInd
 *
 *  DESCRIPTION
 *      This function handles GATT_CHAR_DESC_INFO_IND messages
 *      received from the firmware.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
static void handleGattCharDescriptorInfoInd(GATT_CHAR_DESC_INFO_IND_T *ind)
{
    /* Check the UUID to see whether it is one of interest.
     * We are not interested in 128-bit UUIDs
     */
    if(ind->uuid_type == GATT_UUID16 &&
        ind->uuid[0] == UUID_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC)
    {
        /* Client configuration descriptor */
        if(g_app_data.conf_handle_ptr)
        {
            *g_app_data.conf_handle_ptr = ind->desc_handle;
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattDbCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_ADD_DB_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalGattDbCfm(GATT_ADD_DB_CFM_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_init:
        {
            if(p_event_data->result == sys_status_success)
            {
                 /* Database is set up. So start advertising */
                SetAppState(app_fast_advertising);
            }
            else
            {
                /* Don't expect this to happen */
                ReportPanic(app_panic_db_registration);
            }
        }
        break;

       default:
        {
            ReportPanic(app_panic_invalid_state);
            /* Control should never come here */
        }
       break;
    }

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmKeysInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_KEYS_IND and copies IRK from it
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalSmKeysInd(SM_KEYS_IND_T *event_data)
{
    /*Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: /* FALLTHROUGH */
        case app_connected_discovering:
        {
            /* If keys are present, save them */
            if((event_data->keys)->keys_present & (1 << SM_KEY_TYPE_DIV))
            {
                /* Store the diversifier which will be used for accepting/
                 * rejecting the encryption requests.
                 */
                g_app_data.diversifier = (event_data->keys)->div;

                /* Write the new diversifier to NVM */
                Nvm_Write(&g_app_data.diversifier,
                          sizeof(g_app_data.diversifier),
                          NVM_OFFSET_SM_DIV);
            }

            /* Store the IRK, it is used afterwards to validate the identity of
             * connected host
             */
            if((event_data->keys)->keys_present & (1 << SM_KEY_TYPE_ID))
            {
                /* If bonded device is resolvable random, store the IRK */
                MemCopy(g_app_data.central_device_irk.irk,
                            (event_data->keys)->irk,
                            MAX_WORDS_IRK);

                /* If bonded device address is resolvable random
                 * then store IRK to NVM
                 */
                Nvm_Write(g_app_data.central_device_irk.irk,
                              MAX_WORDS_IRK,
                              NVM_OFFSET_SM_IRK);
            }
        }
        break;
        default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLMEncryptionChange
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_ENCRYPTION_CHANGE
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalLMEncryptionChange(LM_EVENT_T *event_data)
{
    /*Handling signal as per current state */
    switch(g_app_data.state)
    {

        case app_connected: /* FALLTHROUGH */
        case app_connected_discovering:
        {
            HCI_EV_DATA_ENCRYPTION_CHANGE_T *pdata_encrypt_change =
                                        &event_data->enc_change.data;

            if(pdata_encrypt_change->status == sys_status_success)
            {
                g_app_data.encrypt_enabled = pdata_encrypt_change->enc_enable;

                /* Delete the bonding chance timer */
                TimerDelete(g_app_data.bonding_reattempt_tid);
                g_app_data.bonding_reattempt_tid = TIMER_INVALID;


                /* If configuration is not already in progress, start it */
                if(!g_app_ans_data.config_in_progress)
                {
                    /* Configure the remote server for notifications.
                     * The following function starts the configuration process
                     * only when it has already established that the remote
                     * server supports the Alert profiles.
                     */
                    ConfigureTheRemoteServerForNotification();
                }
            }

        }
        break;
        default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      gattDiscoveryHandler
 *
 *  DESCRIPTION
 *      This function gets called on expiry of timer which we had started in
 *      function StartDiscoveryProcedure and starts the dicovery procedure
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

static void gattDiscoveryHandler(timer_id tid)
{
    if(tid == g_app_data.app_tid)
    {
        g_app_data.app_tid = TIMER_INVALID;
        GattDiscoverAllPrimaryServices(g_app_data.st_ucid);
    }
    /* Else it may be because of some race condition. Ignore it */
}


#ifdef ADD_DELAY_IN_READING
/*----------------------------------------------------------------------------*
 *  NAME
 *      delayInReadingTimerHandler
 *
 *  DESCRIPTION
 *      This function gets called on expiry of timer which we had started to add
 *      a delay in reading phone alert status characteristic while handling
 *      signal GATT_WRITE_CHAR_VAL_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void delayInReadingTimerHandler(timer_id tid)
{
    if(tid == g_app_data.delay_read_tid)
    {
        g_app_data.delay_read_tid = TIMER_INVALID;

        /* Read the phone alert status characteristic */
        ReadPASPhoneAlertChar(g_app_data.st_ucid);
    }
    /* Else it may be because of some race condition. Ignore it */
}
#endif /* ADD_DELAY_IN_READING */

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattDiscAllCharDescCfm
 *
 *  DESCRIPTION
 *      This function handles Alert Notification service specific
 *      GATT_DISC_ALL_CHAR_DESC_CFM messages
 *      received from the firmware. Receipt of one of these messages
 *      means that the characteristic discovery is complete.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleGattDiscAllCharDescCfm
                        (GATT_DISC_ALL_CHAR_DESC_CFM_T *cfm)
{
    if(g_app_data.conf_handle_ptr == & g_app_ans_data.new_alert_ccd_hndl)
    {
        /* Discover characteristic descriptors of ANS unread alert
         * characteristic
         */
        GattDiscoverAllCharDescriptors(g_app_data.st_ucid,
                                    g_app_ans_data.unread_alert_hndl,
                                    g_app_ans_data.unread_alert_char_end_hndl);
        g_app_data.conf_handle_ptr = &g_app_ans_data.unread_alert_ccd_hndl;
    }
    else if(g_app_data.conf_handle_ptr ==
                            &g_app_ans_data.unread_alert_ccd_hndl)
    {
        /* Does remote device has PAS profile. If yes discover it's
         * characteristic descriptors.
         */
        if(g_app_data.connected_device_profiles & pas_supported)
        {
            /* Discover characteristic descriptors of PAS phone alert
             * status characteristic.
             */
            GattDiscoverAllCharDescriptors(g_app_data.st_ucid,
                                    g_app_pas_data.phone_alert_hndl,
                                    g_app_pas_data.phone_alert_char_end_hndl);
            g_app_data.conf_handle_ptr = &g_app_pas_data.phone_alert_ccd_hndl;
        }
        else
        {
            /* Remote does not support PAS. Jump to configuration procedure */
            g_app_data.conf_handle_ptr = NULL;
        }
    }
    else if(g_app_data.conf_handle_ptr ==
                            &g_app_pas_data.phone_alert_ccd_hndl)
    {
        /* Discover characteristic descriptors of PAS ringer setting
         * characteristic.
         */
        GattDiscoverAllCharDescriptors(g_app_data.st_ucid,
                                    g_app_pas_data.ringer_setting_hndl,
                                    g_app_pas_data.ringer_setting_end_hndl);
        g_app_data.conf_handle_ptr = &g_app_pas_data.ringer_setting_ccd_hndl;
    }
    else if(g_app_data.conf_handle_ptr ==
                            &g_app_pas_data.ringer_setting_ccd_hndl)
    {
        /* All client configuration descriptors have been discovered.
         * Start configuration process now.
         */
        g_app_data.conf_handle_ptr = NULL;
    }

    if(g_app_data.conf_handle_ptr == NULL)
    {
        /* Configure the remote server for notifications */
        ConfigureTheRemoteServerForNotification();
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      g_app_data.conf_handle_ptr
 *
 *  DESCRIPTION
 *      This function handles GATT_READ_CHAR_VAL_CFM messages
 *      received from the firmware.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
static void handleGattWriteCharValCFM(GATT_WRITE_CHAR_VAL_CFM_T *cfm)
{
    uint8 cat_id;

    if(g_app_data.conf_handle_ptr== &g_app_ans_data.alert_ctrl_hndl)
    {
        /* This is the case where we are enabling or disabling
         * new or unread alerts
         */
        if(g_app_ans_data.last_ans_command ==
                                        alert_ctrl_enable_new_incoming_alerts)
        {
            /* New Alerts have been enabled. Now enable the unread alerts
             */
            g_app_data.conf_handle_ptr = &g_app_ans_data.alert_ctrl_hndl;

            /* We now know which categories are supported by
             * the remote device. This means that we can now subscribe to those
             * of interest. If we want to enable all the categories,
             * Use the special value 0xFF
             * If we don't want all the categories, then refer following page:
             * http://developer.bluetooth.org/gatt/characteristics/Pages/
             * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.
             * alert_notification_control_point.xml.
             * and enable all the required categories one by one
             */
            /* Here we will enable all the categories */

            cat_id = 0xFF; /* Enable all the supported categories */
            SendAlertControlPointCommand(cfm->cid ,
                                  alert_ctrl_enable_unread_cat_status, cat_id);

        }
        else if(g_app_ans_data.last_ans_command ==
                                    alert_ctrl_enable_unread_cat_status)
        {
            g_app_data.conf_handle_ptr = &g_app_ans_data.alert_ctrl_hndl;


            cat_id = 0xFF; /* Ask for all supported categories */

            /* Write alert notification control point command
             * for immediate new/unread alert notification.
             */
            SendAlertControlPointCommand(cfm->cid,
                                    alert_ctrl_immediate_new_alert , cat_id);
        }
        else if(g_app_ans_data.last_ans_command ==
                                alert_ctrl_immediate_new_alert)
        {
            /* Write immediate alert on the alert notification
             * control point
             */

            cat_id = 0xFF; /* Ask for all supported unread alert categories */
            SendAlertControlPointCommand(cfm->cid,
                              alert_ctrl_immediate_unread_status , cat_id);
        }
        else
        {
            /* No more writing required on the alert notification
             * control point. Start configuring PAS if remote device supports
             * it.
             */
            if(g_app_data.connected_device_profiles & pas_supported)
            {
                ConfigurePhoneAlertServer(cfm->cid);
            }
            else
            {
                /* If PAS is not supported by remote device then
                 * this marks the end of configuration procedure
                 */
                g_app_ans_data.config_in_progress = FALSE;
                g_app_data.conf_handle_ptr = NULL;
            }
        }
    }
    else if(g_app_data.conf_handle_ptr==
                        &g_app_ans_data.new_alert_ccd_hndl)
    {
        /* Application had written on the client configuration descriptor of
         * new alert characteristic Now write unread alert client configuration
         * descriptor for notifications
         */
        g_app_data.conf_handle_ptr =
                     &g_app_ans_data.unread_alert_ccd_hndl;
        MainEnableNotifications(cfm->cid,
                      g_app_ans_data.unread_alert_ccd_hndl);

    }
    else if(g_app_data.conf_handle_ptr ==
                        &g_app_ans_data.unread_alert_ccd_hndl)
    {
        /* Application had written on the client configuration descriptor of
         * unread alert characteristic. Now read the supported new alert
         * supported catgeories.
         */

        /* Read the supported new alert category characteristic */
        g_app_data.conf_handle_ptr = &g_app_ans_data.new_alert_cat_hndl;
        GattReadCharValue(cfm->cid, g_app_ans_data.new_alert_cat_hndl);

    }
    else if(g_app_data.conf_handle_ptr ==
                     &g_app_pas_data.phone_alert_ccd_hndl)
    {
        /* Remote device has PAS service and we have just completed the step 1
         * of the PAS configuration. Now move to step 2, notification
         * configuration on the ringer setting
         */
        g_app_data.conf_handle_ptr =
                     &g_app_pas_data.ringer_setting_ccd_hndl;
        MainEnableNotifications(cfm->cid,
                      g_app_pas_data.ringer_setting_ccd_hndl);
    }
    else if(g_app_data.conf_handle_ptr ==
                     &g_app_pas_data.ringer_setting_ccd_hndl)
    {
        /* This marks the end of configuration procedure */
        g_app_ans_data.config_in_progress = FALSE;
        g_app_data.conf_handle_ptr = NULL;
    }
    else
    {
        /* When PAS ringer control point will be written, control will
         * enter this else block on reception of write confirmation signal
         */
        /* Do nothing */
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      appInitExit
 *
 *  DESCRIPTION
 *      This function is called upon exiting the app_init state.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

static void appInitExit(void)
{
    if(g_app_data.bonded == TRUE &&
        (!GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr)))
    {
        /* If the device is bonded, configure white list with the
         * bonded host address
         */
        if(LsAddWhiteListDevice(&g_app_data.bonded_bd_addr) !=
                                        ls_err_none)
        {
            ReportPanic(app_panic_add_whitelist);
        }
    }
}



/*----------------------------------------------------------------------------*
 *  NAME
 *      ConfigureTheRemoteServerForNotification
 *
 *  DESCRIPTION
 *      This function starts the server configuration
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void ConfigureTheRemoteServerForNotification(void)
{
    /* Start Alert notification configuration and
     * Set the config_in_progress flag to TRUE
     */
    g_app_ans_data.config_in_progress = TRUE;

    /* Start the configuration procedure. If remote device supports ANS
     * then we will start with ANS and after ANS configuration, we will do
     * PAS configuration
     */
    if(g_app_data.connected_device_profiles & ans_supported)
    {
        /* Start the configuration process. Configuration process will take
         * steps:
         * 1. Enable notification on client descriptors of new alert
         *    characteristics.
         * 2. Enable notification on client descriptors  of unread alert
         *    characteristics
         * 3. Read the supported new alert category.
         * 4. Read the supported unread alert category.
         * 5. Enable the all supported new alerts notifications
         * 6. Enable all unread supported alert notifications

         * 7. Write immediate new alert notification.
         * 8. Write immediate unread alert notification.
         */
        ConfigureAlertNotificationServer(g_app_data.st_ucid);
    }
    else if(g_app_data.connected_device_profiles & pas_supported)
    {
        /* Configure the PAS if ANS not supported by the remote device */

        /* PAS configuration involves following steps in order :
         * 1. Configure notification of phone alert status characteristic
         * 2. Configure notification of ringer setting characteristic
         * This completes the PAS configuration
         */
        ConfigurePhoneAlertServer(g_app_data.st_ucid);
    }
}


/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

#ifdef NVM_TYPE_FLASH
/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteApplicationAndServiceDataToNVM
 *
 *  DESCRIPTION
 *      This function writes the application data to NVM. This function should
 *      be called on getting nvm_status_needs_erase
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void WriteApplicationAndServiceDataToNVM(void)
{
    uint16 nvm_sanity = 0xffff;
    nvm_sanity = NVM_SANITY_MAGIC;

    /* Write NVM sanity word to the NVM */
    Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);

    /* Write Bonded flag to NVM. */
    Nvm_Write((uint16*)&g_app_data.bonded,
               sizeof(g_app_data.bonded),
               NVM_OFFSET_BONDED_FLAG);


    /* Write Bonded address to NVM. */
    Nvm_Write((uint16*)&g_app_data.bonded_bd_addr,
              sizeof(TYPED_BD_ADDR_T),
              NVM_OFFSET_BONDED_ADDR);

    /* Write the diversifier to NVM */
    Nvm_Write(&g_app_data.diversifier,
                sizeof(g_app_data.diversifier),
                NVM_OFFSET_SM_DIV);

    /* Store the IRK to NVM */
    Nvm_Write(g_app_data.central_device_irk.irk,
                MAX_WORDS_IRK,
                NVM_OFFSET_SM_IRK);

    /* Write GAP service data into NVM. */
    WriteGapServiceDataInNVM();

    /* Write Gatt service data into NVM. */
    WriteGattServiceDataInNvm();

    /* Write Battery service data into NVM. */
    WriteBatteryServiceDataInNvm();
}
#endif /* NVM_TYPE_FLASH */


/*----------------------------------------------------------------------------*
 *  NAME
 *      ReportPanic
 *
 *  DESCRIPTION
 *      This function raises the panic
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void ReportPanic(app_panic_code panic_code)
{
    /* If we want any debug prints, we can put them here */
    Panic(panic_code);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleExtraLongButtonPressTimerExpiry
 *
 *  DESCRIPTION
 *      This function for extra long button press handling
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleExtraLongButtonPressTimerExpiry(timer_id tid)
{
    if(tid == g_app_hw_data.button_press_tid)
    {
        /* Extra long button press has expired. */

        /* Clear the long button pressed flag.*/
        g_app_hw_data.long_button_pressed = FALSE;

        /* Re-initialise timer id */
        g_app_hw_data.button_press_tid = TIMER_INVALID;

        /* Sound three beeps to indicate pairing removal to user */
        SoundBuzzer(beep_thrice);

        /* Remove bonding information*/

        /* The device will no more be bonded */
        g_app_data.bonded = FALSE;

        /* Write bonded status to NVM */
        Nvm_Write((uint16*)&g_app_data.bonded,
                  sizeof(g_app_data.bonded),
                  NVM_OFFSET_BONDED_FLAG);


        switch(g_app_data.state)
        {
            case app_connected: /* FALLTHROUGH */
            case app_connected_discovering:
            {
                /* Disconnect with the connected host before triggering
                 * advertisements again for any host to connect. Application
                 * and services data related to bonding status will get
                 * updated while exiting disconnecting state
                 */
                SetAppState(app_disconnecting);

                /* Reset and clear the whitelist */
                LsResetWhiteList();
            }
            break;

            case app_fast_advertising: /* FALLTHROUGH */
            case app_slow_advertising:
            {
                g_app_data.pairing_remove_button_pressed = TRUE;

                /* Delete the advertising timer */
                TimerDelete(g_app_data.app_tid);
                g_app_data.app_tid = TIMER_INVALID;

                /* Stop advertisements first as it may be making use of white
                 * list. Once advertisements are stopped, reset the whitelist
                 * and trigger advertisements again for any host to connect
                 */
                GattStopAdverts();
            }
            break;

            case app_disconnecting:
            {
                /* Disconnect procedure on-going, so just reset the whitelist
                 * and wait for procedure to get completed before triggering
                 * advertisements again for any host to connect. Application
                 * and services data related to bonding status will get
                 * updated while exiting disconnecting state
                 */
                LsResetWhiteList();
            }
            break;

            default: /* app_state_init / app_state_idle handling */
            {
                /* Initialise application data. */
                appDataInit();

                /* Reset and clear the whitelist. */
                LsResetWhiteList();

                /* Start fast undirected advertisements. */
                SetAppState(app_fast_advertising);
            }
            break;

        }
    }
    /* else it may be because of some race condition, ignore it */
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePIOChangedEvent
 *
 *  DESCRIPTION
 *      This function handles PIO Changed event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void HandlePIOChangedEvent(uint32 pio_changed)
{
    uint32 pios;
    ringer_ctrl_point_value val;

    if(pio_changed & PIO_BIT_MASK(BUTTON_PIO))
    {
        writeString("HandlePIOChangedEvent");
        
        /* PIO changed */
        pios = PioGets();
        if(!(pios & PIO_BIT_MASK(BUTTON_PIO)))
        {
            /* This is for the case when pio changed event has come for button
             * press.
             */

            /* Delete any button press timer if already running */
            TimerDelete(g_app_hw_data.button_press_tid);
            g_app_hw_data.button_press_tid = TIMER_INVALID;

            /* Create a button press timer */
            g_app_hw_data.button_press_tid =
                                 TimerCreate(LONG_BUTTON_PRESS_TIMER,
                                            TRUE,
                                            HandleLongButtonPressTimerExpiry);
            /* Initialize the long button pressed flag with FALSE.*/
            g_app_hw_data.long_button_pressed = FALSE;

            /* Stop buzzer */
            SoundBuzzer(beep_off);
        }
        else
        {
            /* This is for the case when pio changed event comes for
             * the button release
             */
            if(g_app_hw_data.button_press_tid != TIMER_INVALID)
            {
                /* Button press timer is running.This means that either
                 * 1.timer LONG_BUTTON_PRESS_TIMER has not expired yet
                 * OR
                 * 2.Timer LONG_BUTTON_PRESS_TIMER has expired but a 2nd
                 * timer of length (EXTRA_LONG_BUTTON_PRESS_TIMER -
                 * LONG_BUTTON_PRESS_TIMER) which we start on 1st timer
                 * expiry, is still running.
                 */

                /* case when timer LONG_BUTTON_PRESS_TIMER is still running */
                if(g_app_hw_data.long_button_pressed == FALSE)
                {
                    /* If connected, Short Button should get enabled only if
                     * remote device has PAS and application has completed the
                     * PAS configuration and have read the phone alert as well
                     * as ringer setting once.
                     */

                    if((g_app_data.state == app_connected) &&
                       (g_app_data.connected_device_profiles & pas_supported)
                              && (g_app_pas_data.pas_config_ongoing == FALSE))
                    {
                        /* Mute the phone once */
                        WritePASRingerCtrlPoint(ringer_mute_once);
                        SoundBuzzer(beep_short);
                    }
                    else if(g_app_data.state == app_idle)
                    {
                        /* Start advertising */
                        SetAppState(app_fast_advertising);
                        /* It will get indicated by two short beeps*/
                    }
                    else
                    {
                        /* Disabled Short button press. Don't sound beep */
                    }
                }
                else if(g_app_hw_data.long_button_pressed == TRUE)
                {
                    /* Case when timer LONG_BUTTON_PRESS_TIMER has expired
                     * but 2nd timer of value (EXTRA_LONG_BUTTON_PRESS_TIMER
                     * - LONG_BUTTON_PRESS_TIMER)
                     */

                    /* If PTS test case requires application to write
                     * SILENT_MODE on ringer control point and corresponding
                     * bit in user CS key is set, then this flag
                     * g_pts_send_only_silent_mode will be enabled and
                     * application will always write SILENT_MODE on
                     * Ringer control point.Otherwise it will write based on
                     * the current value.
                     */
                    if(g_pts_send_only_silent_mode)
                    {
                        val = ringer_silent_mode;
                    }
                    else
                    {
                        /* If current ringer setting is SILENT, Set it to
                         * NORMAL, If it is NORMAL, set it to SILENT
                         */
                        if(g_app_pas_data.ringer_setting_value ==
                                                          ringer_setting_silent)
                        {
                            val = ringer_cancel_silent_mode;
                        }
                        if(g_app_pas_data.ringer_setting_value ==
                                                          ringer_setting_normal)
                        {
                            val = ringer_silent_mode;
                        }
                    }

                    /* Long Button should get enabled only if remote device
                     * has PAS and application has completed the PAS
                     * configuration and have read the phone alert as well
                     * as ringer setting once.
                     */
                    if((g_app_data.state == app_connected) &&
                        (g_app_data.connected_device_profiles & pas_supported)
                            && (g_app_pas_data.pas_config_ongoing == FALSE))
                    {
                        SoundBuzzer(beep_long);
                        /* Write it on the remote device */
                        WritePASRingerCtrlPoint(val);
                    }
                }

                /* Delete the already running button press timer */
                TimerDelete(g_app_hw_data.button_press_tid);
                g_app_hw_data.button_press_tid = TIMER_INVALID;
                g_app_hw_data.long_button_pressed = FALSE;
            }
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SetAppState
 *
 *  DESCRIPTION
 *      This function is used to set the state of the application.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void SetAppState(app_state new_state)
{
    app_state old_state = g_app_data.state;

    /* Check if the new state to be set is not the same as the present state
     * of the application.
     */
    if (old_state != new_state)
    {
        /* Handle exiting old state */
        switch (old_state)
        {
            case app_init:
            {
                appInitExit();
            }
            break;

            case app_disconnecting:
            {
                /* Common things to do whenever application exits
                 * app_disconnecting state.
                 */
                appDataInit();
            }
            break;

            case app_fast_advertising: /* FALLTHROUGH */
            case app_slow_advertising:
            {
                /* Common things to do whenever application exits
                 * APP_*_ADVERTISING state.
                 */
                /* Cancel advertisement timer */
                TimerDelete(g_app_data.app_tid);
                g_app_data.app_tid = TIMER_INVALID;
            }
            break;

            case app_connected:
                /* Nothing to do */
#ifdef DEBUG_THRU_UART
                if(new_state != app_connected_discovering)
                {
                    writeString("Alert tag state: Disconnected");
                }
#endif /* DEBUG_THRU_UART */
            break;

            case app_idle:
                /* Nothing to do */
            break;

            case app_connected_discovering:
            {
                /* Discovery/Configuration is complete.
                 * Start the connection parameter update procedure.
                 */
                if(new_state == app_connected)
                {
                    /* Connection parameters should be updated only if the new
                     * state is app_connected.
                     */

                    /* Check if the current parameter value comply with the
                     * preferred connection intervals.
                     */
                    if(g_app_data.conn_interval < PREFERRED_MIN_CON_INTERVAL ||
                       g_app_data.conn_interval > PREFERRED_MAX_CON_INTERVAL
#if PREFERRED_SLAVE_LATENCY
                       || g_app_data.conn_latency < PREFERRED_SLAVE_LATENCY
#endif
                      )
                    {
                        /* Delete timer if running */
                        TimerDelete(g_app_data.conn_param_update_tid);

                        /* Set the num of connection update attempts to zero */
                        g_app_data.num_conn_update_req = 0;

                        /* Start timer to trigger Connection Parameter Update
                         * procedure.
                         * Please note that this procedure requires all the
                         * attributes read writes to be made IRQ. If application
                         * wants firmware to reply for any of the request, it
                         * shall reply with code "gatt_status_irq_proceed".
                         */
                        g_app_data.conn_param_update_tid =
                               TimerCreate(TGAP_CPP_PERIOD,
                                            TRUE,
                                            handleGapCppTimerExpiry);
                        g_app_data.cpu_timer_value = TGAP_CPP_PERIOD;
                    }
                }
            }
            break;

            default:
                /* Nothing to do */
            break;
        }

        /* Set new state */
        g_app_data.state = new_state;

        /* Handle entering new state */
        switch (new_state)
        {
            case app_fast_advertising:
            {
                /* Start advertising and indicate this to user */
                GattTriggerFastAdverts();

                /* Indicate advertising to user */
                SetIndication(app_ind_adv);
                SoundBuzzer(beep_twice);
#ifdef DEBUG_THRU_UART
                writeString("Alert tag state: Fast advertising");
#endif /* DEBUG_THRU_UART */

            }
            break;

            case app_slow_advertising:
            {
                GattStartAdverts(FALSE);

                /* Indicate advertising to user */
                SetIndication(app_ind_adv);
#ifdef DEBUG_THRU_UART
                writeString("Alert tag state: Slow advertising");
#endif /* DEBUG_THRU_UART */
            }
            break;

            case app_idle:
            {
                /* Stop the indication which we have started for
                 * advertising/Connection
                 */
                SetIndication(app_ind_stop);

                /* Sound long beep to indicate non connectable mode*/
                SoundBuzzer(beep_long);
#ifdef DEBUG_THRU_UART
                writeString("Alert tag state: Idle");
#endif /* DEBUG_THRU_UART */
            }
            break;

            case app_connected:
            {
                /* Common things to do whenever application enters
                 * app_state_connected state.
                 */

                /* If the battery level has changed since the last read, update
                 * the conencted Host about it.
                 */
                SendBatteryLevelNotification();

                /* Indicate connected state to user */
                SetIndication(app_ind_conn);
#ifdef DEBUG_THRU_UART
                if(old_state != app_connected_discovering)
                {
                    writeString("Alert tag state: Connected");
                }
#endif /* DEBUG_THRU_UART */
            }
            break;

            case app_connected_discovering:
            {
                g_app_data.connected_device_profiles = 0;

                TimerDelete(g_app_data.app_tid);
                /* Discovery will start in 150 milliseconds */
                g_app_data.app_tid = TimerCreate(150*MILLISECOND, TRUE,
                                                    gattDiscoveryHandler);
#ifdef DEBUG_THRU_UART
                writeString("Alert tag state: Discovering services");
#endif /* DEBUG_THRU_UART */
            }
            break;

            case app_disconnecting:
            {
                if(g_app_data.auth_failure)
                {
                    /* Disconnect with an error - Authentication Failure */
                    GattDisconnectReasonReq(g_app_data.st_ucid, 
                                            ls_err_authentication);
                }
                else
                {
                    /* Disconnect with the default error */
                    GattDisconnectReq(g_app_data.st_ucid);
                }
#ifdef DEBUG_THRU_UART
                writeString("Alert tag state: Disconnected");
#endif /* DEBUG_THRU_UART */
            }
            break;


            default:
            break;
        }
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AppIsDeviceBonded
 *
 *  DESCRIPTION
 *      This function returns the status whether the connected device is
 *      bonded or not.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern bool AppIsDeviceBonded(void)
{
    return g_app_data.bonded;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppGetConnectionCid
 *
 *  DESCRIPTION
 *      This function returns the connection identifier for the connection
 *
 *  RETURNS/MODIFIES
 *      Connection identifier.
 *
 
*----------------------------------------------------------------------------*/
extern uint16 AppGetConnectionCid(void)
{
    return g_app_data.st_ucid;
}

/* Enumerate available commands */
typedef enum _cmd
{
    cmd_none,       /* No command */
    cmd_reset,        /* Reset command */
} CMD_T;

static void processRxCmd(void)
{
    /* Loop until the byte queue is empty */
    while (BQGetDataSize() > 0)
    {
        uint8 byte = '\0';
        
        /* Read in the next byte */
        if (BQPopBytes(&byte, 1) > 0)
        {
            CMD_T cmd_type;
            
            /* Convert byte into a command */
            switch (byte)
            {
                case 'r':
                case 'R':
                    cmd_type = cmd_reset;
                    break;

                //case 'h':
                //case 'H':
                //    showHelp();
                //    continue;
                default:
                    /* Discard the byte (we don't know what to do with it) and
                     * jump back to the top of the loop.
                     */
                    continue;
            }
            
             switch (cmd_type)
             {
                 /* Read the voltage level of an analogue port */
                 case cmd_reset:
                    writeString("reset command");
                    WarmReset();
                    break;

                 default:
                    break;
             }
        }
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      uartRxDataCallback
 *
 *  DESCRIPTION
 *      This is an internal callback function (of type uart_data_in_fn) that
 *      will be called by the UART driver when any data is received over UART.
 *      See DebugInit in the Firmware Library documentation for details.
 *
 * PARAMETERS
 *      p_rx_buffer [in]   Pointer to the receive buffer (uint8 if 'unpacked'
 *                         or uint16 if 'packed' depending on the chosen UART
 *                         data mode - this application uses 'unpacked')
 *
 *      length [in]        Number of bytes ('unpacked') or words ('packed')
 *                         received
 *
 *      p_additional_req_data_length [out]
 *                         Number of additional bytes ('unpacked') or words
 *                         ('packed') this application wishes to receive
 *
 * RETURNS
 *      The number of bytes ('unpacked') or words ('packed') that have been
 *      processed out of the available data.
 *----------------------------------------------------------------------------*/
static uint16 uartRxDataCallback(void   *p_rx_buffer,
                                 uint16  length,
                                 uint16 *p_additional_req_data_length)
{
    if (length > 0)
    {
        /* First copy all the bytes received into the byte queue */
        BQForceQueueBytes((const uint8 *)p_rx_buffer, length);

        /* Check whether the Enter key has been pressed, in which case parse
         * the command that has been stored in the byte queue.
         */
        uint16 i = 0;
        for (i = 0; i < length; ++i)
        /* We should be receiving this callback for every byte received which
         * means the length should always be 1. To accommodate changes in the
         * threshold without having to rewrite this section, no assumptions
         * on the received data length are made.
         */
        {
            const char ch = ((const char *)p_rx_buffer)[i];
            
            /* Echo back the data */
            DebugWriteChar(ch);

            /* Check if Enter key was pressed */
            if ((ch == '\r') || (ch == '\n'))
            {
                /* Echo back newline */
                DebugWriteChar('\n');

                /* Trigger command processing */
                processRxCmd();
            }
        }
    }

    /* Inform the UART driver that we'd like to receive another byte when it
     * becomes available
     */
    *p_additional_req_data_length = (uint16)1;

    /* Return the number of bytes that have been processed */
    return length;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AppPowerOnReset
 *
 *  DESCRIPTION
 *      This function is called just after a power-on reset (including after
 *      a firmware panic).
 *
 *      NOTE: this function should only contain code to be executed after a
 *      power-on reset or panic. Code that should also be executed after an
 *      HCI_RESET should instead be placed in the reset() function.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppPowerOnReset(void)
{
    /* Configure the application constants */
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AppInit
 *
 *  DESCRIPTION
 *      This function is called after a power-on reset (including after a
 *      firmware panic) or after an HCI Reset has been requested.
 *
 *      NOTE: In the case of a power-on reset, this function is called
 *      after app_power_on_reset.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

void AppInit(sleep_state LastSleepState)
{
    uint16 gatt_db_length = 0;
    uint16 *p_gatt_db = NULL;
    uint16 pts_cskey = 0;

    /* Initialize the application timers */
    TimerInit(MAX_APP_TIMERS, (void*)app_timers);

    
#ifdef DEBUG_THRU_UART
    /* This will enable the debug prints over the UART */
    DebugInit(1, uartRxDataCallback, NULL);
    //SleepModeChange(sleep_mode_never);
    writeString("\nAlert Tag Init");
#endif

    /* Initialize GATT entity */
    GattInit();

    /* Install GATT Client functionality for Alert Notification service*/
    GattInstallClientRole();
    GattInstallServerWrite();

#ifdef NVM_TYPE_EEPROM
    /* Configure the NVM manager to use I2C EEPROM for NVM store */
    NvmConfigureI2cEeprom();
#elif NVM_TYPE_FLASH
    /* Configure the NVM Manager to use SPI flash for NVM store. */
    NvmConfigureSpiFlash();
#endif /* NVM_TYPE_EEPROM */

    Nvm_Disable();

    /*Initialize application hardware */
    InitAlertTagHardware();

    /* Battery Initialisation on Chip reset */
    BatteryInitChipReset();

    /* Initialize the gap data. Needs to be done before readPersistentStore */
    GapDataInit();

    /* Read persistent storage */
    readPersistentStore();

    /* Tell Security Manager module about the value it needs to initialize it's
     * diversifier to.
     */
    SMInit(g_app_data.diversifier);

    /* Initialize TAG State */
    g_app_data.state = app_init;

    /*Initialize  application data structure */
    appDataInit();

    /* Read the project keyr file for user defined CS keys for PTS testcases */
    pts_cskey = CSReadUserKey(PTS_CS_KEY_INDEX);
    if(pts_cskey & PTS_DISABLE_NOTIFY_CS_KEY_MASK)
    {
        /* CS key has been set for writing disable notification and indication
         * on client configuration descriptor.
         */
        g_pts_disable_notify_ccd = TRUE;
    }

    if(pts_cskey & PTS_WRITE_ONLY_SILENT_MODE)
    {
        /* Enabling this will always write SILENT mode on ringer setting.*/
        g_pts_send_only_silent_mode = TRUE;
    }

    /* Tell GATT about our database. We will get a GATT_ADD_DB_CFM event when
     * this has completed.
     */
    p_gatt_db = GattGetDatabase(&gatt_db_length);

    GattAddDatabaseReq(gatt_db_length, p_gatt_db);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessSystemEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a system event, such
 *      as a battery low notification, is received by the system.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppProcessSystemEvent(sys_event_id id, void *data)
{
    switch(id)
    {
        case sys_event_battery_low:
        {
            /* Battery low event received - notify the connected host. If
             * not connected, the battery level will get notified when
             * device gets connected again
             */
            if(g_app_data.state == app_connected ||
               g_app_data.state == app_connected_discovering)
            {
                BatteryUpdateLevel(g_app_data.st_ucid,TRUE);
            }
            break;
        }

        case sys_event_pio_changed:
        {
            HandlePIOChangedEvent(((pio_changed_data*)data)->pio_cause);
        }
        break;

        default:
        {
            break;
        }
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessLmEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a LM-specific event is
 *      received by the system.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

bool AppProcessLmEvent(lm_event_code event_code, LM_EVENT_T *event_data)
{
    switch (event_code)
    {
        case GATT_ADD_DB_CFM:
            writeString("GATT_ADD_DB_CFM");
            handleSignalGattDbCfm((GATT_ADD_DB_CFM_T*)event_data);
        break;

        case LM_EV_CONNECTION_COMPLETE:
            writeString("LM_EV_CONNECTION_COMPLETE");
            /* Handle the LM connection complete event. */
            handleSignalLmEvConnectionComplete(
                                    (LM_EV_CONNECTION_COMPLETE_T*)event_data);
        break;

        case GATT_CONNECT_CFM:
            writeString("GATT_CONNECT_CFM");
            handleSignalGattConnectCFM((GATT_CONNECT_CFM_T *)event_data);
        break;

        case GATT_SERV_INFO_IND:
            writeString("GATT_SERV_INFO_IND");
            /* This service info indication comes for every service present on
             * the server.
             */
            handleGattServInfoInd((GATT_SERV_INFO_IND_T *)event_data);
        break;

        case GATT_DISC_ALL_PRIM_SERV_CFM:
            writeString("GATT_DISC_ALL_PRIM_SERV_CFM");
            /* This signal comes on completion of primary service discovery */
            handleGattDiscAllPrimServCfm
                                ((GATT_DISC_ALL_PRIM_SERV_CFM_T *)event_data);
        break;

        case GATT_CHAR_DECL_INFO_IND:
            writeString("GATT_CHAR_DECL_INFO_IND");
            /* This signal comes when gatt client starts procedure for
             * discovering all the characterstics
             */
            handleGattCharDeclInfoInd((GATT_CHAR_DECL_INFO_IND_T *)event_data);
        break;

        case GATT_DISC_SERVICE_CHAR_CFM:
            writeString("GATT_DISC_SERVICE_CHAR_CFM");
            /* This signal comes on completion of characteristic discovery */
            handleGattDiscServCharCfm
                    ((GATT_DISC_SERVICE_CHAR_CFM_T *) event_data);
        break;

        case GATT_CHAR_DESC_INFO_IND:
            writeString("GATT_CHAR_DESC_INFO_IND");
        /* This indication signal comes on characteristic descriptor
         * discovery
         */
        {
            handleGattCharDescriptorInfoInd(
                                (GATT_CHAR_DESC_INFO_IND_T *)event_data);
        }
        break;

        case GATT_DISC_ALL_CHAR_DESC_CFM:
            writeString("GATT_DISC_ALL_CHAR_DESC_CFM");
            /* This signal comes on completion of characteristic descriptor
             * discovery
             */
            {
                if(((GATT_DISC_ALL_CHAR_DESC_CFM_T *)event_data)->result ==
                                                sys_status_success)
                {
                    handleGattDiscAllCharDescCfm
                                 ((GATT_DISC_ALL_CHAR_DESC_CFM_T *)event_data);
                }
                else
                {
                    /* Something went wrong. We can't recover, so disconnect*/
                    SetAppState(app_disconnecting);
                }
            }

        break;

        case GATT_READ_CHAR_VAL_CFM:
            writeString("GATT_READ_CHAR_VAL_CFM");
            {
                if(((GATT_READ_CHAR_VAL_CFM_T *)event_data)->result ==
                                                    sys_status_success)
                {
                    /* If this siganl is for ANS characteristic read, It will
                     * get handled in handleANSGattReadCharValCFM or if this
                     * signal is for PAS, It will get handled in
                     * HandlePASGattReadCharValCFM
                     */
                    handleANSGattReadCharValCFM
                                    ((GATT_READ_CHAR_VAL_CFM_T *)event_data);
                    HandlePASGattReadCharValCFM
                                    ((GATT_READ_CHAR_VAL_CFM_T *)event_data);
                }
                else if(((GATT_READ_CHAR_VAL_CFM_T *)event_data)->result ==
                            GATT_RESULT_INSUFFICIENT_ENCRYPTION ||
                         (((GATT_READ_CHAR_VAL_CFM_T *)event_data)->result ==
                            GATT_RESULT_INSUFFICIENT_AUTHENTICATION))
                {
                    /* If we have received an error with error code
                     * insufficient encryption, we will start a
                     * slave security request
                     */
                    SMRequestSecurityLevel(&g_app_data.con_bd_addr);
                }
                else if(((GATT_READ_CHAR_VAL_CFM_T *)event_data)->result !=
                            GATT_RESULT_TIMEOUT) /* ATT Timerout case gets
                                                  * handled automatically
                                                  */
                {
                    /* Something went wrong. We can't recover, so disconnect*/
                    SetAppState(app_disconnecting);
                }
            }

        break;

        case GATT_WRITE_CHAR_VAL_CFM:
            writeString("GATT_WRITE_CHAR_VAL_CFM");
            {
                if(((GATT_WRITE_CHAR_VAL_CFM_T *)event_data)->result ==
                                                    sys_status_success)
                {
                    handleGattWriteCharValCFM
                                    ((GATT_WRITE_CHAR_VAL_CFM_T *)event_data);

                    if(g_app_ans_data.config_in_progress == FALSE &&
                        g_app_data.state == app_connected_discovering)
                    {
                        /* configuration process is complete.
                         * Change application state to connected
                         */
                        SetAppState(app_connected);

                        /* Phone alert status profile mandates the read of
                         * phone alert status characteristic on connection. Our
                         * application will read it after all the configuration
                         * is done and application state switches back to
                         * app_connected
                         */
                        if(g_app_data.connected_device_profiles & pas_supported)
                        {
                            /* If remote device supports PAS, then we have
                             * registered for notifications of phone alert and
                             * ringer setting characteristics.
                             * We are left with reading of PAS characteristics
                             */
#ifdef ADD_DELAY_IN_READING
                            g_app_data.delay_read_tid =
                                        TimerCreate(100*MILLISECOND,
                                                    TRUE,
                                                    delayInReadingTimerHandler);
#else
                            /* Read the phone alert status characteristic */
                            ReadPASPhoneAlertChar
                               (((GATT_WRITE_CHAR_VAL_CFM_T *)event_data)->cid);
#endif /* ADD_DELAY_IN_READING */
                        }
                    }
                }
                else if(((GATT_WRITE_CHAR_VAL_CFM_T *)event_data)->result ==
                            GATT_RESULT_INSUFFICIENT_ENCRYPTION ||
                         (((GATT_WRITE_CHAR_VAL_CFM_T *)event_data)->result ==
                            GATT_RESULT_INSUFFICIENT_AUTHENTICATION))
                {
                    /* If we have received an error with error code
                     * insufficient encryption, we will start a slave security 
                     * request. Need to reset the config_in_progress flag here 
                     * as we need to configure again on encryption complete.
                     */
                    g_app_ans_data.config_in_progress = FALSE;
                    SMRequestSecurityLevel(&g_app_data.con_bd_addr);
                }
                else if(((GATT_READ_CHAR_VAL_CFM_T *)event_data)->result !=
                            GATT_RESULT_TIMEOUT) /* Time out case gets handled
                                                  * when we receive disconnect
                                                  * indication.
                                                  */
                {
                    /*Something went wrong. We can't recover, so disconnect */
                    SetAppState(app_disconnecting);
                }
            }
        break;

        case LM_EV_ENCRYPTION_CHANGE:
            writeString("LM_EV_ENCRYPTION_CHANGE");
            handleSignalLMEncryptionChange(event_data);
        break;

        case LS_CONNECTION_PARAM_UPDATE_CFM:
            writeString("LS_CONNECTION_PARAM_UPDATE_CFM");
            handleSignalLsConnUpdateSignalCfm(
                            (LS_CONNECTION_PARAM_UPDATE_CFM_T *)event_data);
        break;

        case LM_EV_CONNECTION_UPDATE:
            writeString("LM_EV_CONNECTION_UPDATE");
            /* This event is sent by the controller on connection parameter
             * update.
             */
            handleSignalLmConnectionUpdate(
                            (LM_EV_CONNECTION_UPDATE_T*)event_data);
        break;

        case LS_CONNECTION_PARAM_UPDATE_IND:
            writeString("LS_CONNECTION_PARAM_UPDATE_IND");
            handleSignalLsConnParamUpdateInd(
                            (LS_CONNECTION_PARAM_UPDATE_IND_T *)event_data);
        break;

        case SM_DIV_APPROVE_IND:
            writeString("SM_DIV_APPROVE_IND");
            handleSignalSmDivApproveInd((SM_DIV_APPROVE_IND_T *)event_data);
        break;

        case SM_KEYS_IND:
            writeString("SM_KEYS_IND");
            handleSignalSmKeysInd((SM_KEYS_IND_T *)event_data);
        break;

        case SM_PAIRING_AUTH_IND:
            writeString("SM_PAIRING_AUTH_IND");
            /* Authorize or Reject the pairing request */
            handleSignalSmPairingAuthInd((SM_PAIRING_AUTH_IND_T*)event_data);
        break;

        case SM_SIMPLE_PAIRING_COMPLETE_IND:
            writeString("SM_SIMPLE_PAIRING_COMPLETE_IND");
            handleSignalSmSimplePairingCompleteInd(
                            (SM_SIMPLE_PAIRING_COMPLETE_IND_T *)event_data);
        break;

        case GATT_DISCONNECT_IND:
            writeString("GATT_DISCONNECT_IND");
            /* Disconnect procedure triggered by remote host or due to
             * link loss is considered complete on reception of
             * LM_EV_DISCONNECT_COMPLETE event. So, it gets handled on
             * reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case GATT_DISCONNECT_CFM:
            writeString("GATT_DISCONNECT_CFM");
            /* Confirmation for the completion of GattDisconnectReq()
             * procedure is ignored as the procedure is considered complete
             * on reception of LM_EV_DISCONNECT_COMPLETE event. So, it gets
             * handled on reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case LM_EV_DISCONNECT_COMPLETE:
            writeString("LM_EV_DISCONNECT_COMPLETE");
        {
            /* Disconnect procedures either triggered by application or remote
             * host or link loss case are considered completed on reception
             * of LM_EV_DISCONNECT_COMPLETE event
             */
             handleSignalLmDisconnectComplete(
                    &((LM_EV_DISCONNECT_COMPLETE_T *)event_data)->data);
        }
        break;

        case GATT_CANCEL_CONNECT_CFM:
            writeString("GATT_CANCEL_CONNECT_CFM");
            handleSignalGattCancelConnectCFM(
                    (GATT_CANCEL_CONNECT_CFM_T*)event_data);
        break;

        case GATT_NOT_CHAR_VAL_IND:
            writeString("GATT_NOT_CHAR_VAL_IND");
            /* A notification has been received */
            /* Depending on the handle , it will get handled in corresponding
             * function.
             */
            handleANSGattCharValInd((GATT_CHAR_VAL_IND_T *)event_data);
            HandlePASGattCharValInd((GATT_CHAR_VAL_IND_T *)event_data);
            handleGattServiceCharValInd((GATT_CHAR_VAL_IND_T *)event_data);
        break;


        case LM_EV_NUMBER_COMPLETED_PACKETS: /* FALLTHROUGH */
            writeString("LM_EV_NUMBER_COMPLETED_PACKETS");
        case GATT_CHAR_VAL_NOT_CFM: /* FALLTHROUGH */
            writeString("GATT_CHAR_VAL_NOT_CFM");
        break;
        
        case GATT_ACCESS_IND: /* GATT access indication */
            /* Indicates that an attribute controlled directly by the
             * application (ATT_ATTR_IRQ attribute flag is set) is being
             * read from or written to.
             */
            writeString("GATT_ACCESS_IND");
            handleSignalGattAccessInd((GATT_ACCESS_IND_T*)event_data);

            break;

        default:
        {
            /*Control should never come here */
            break;
        }

    }

    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      StartDiscoveryProcedure
 *
 *  DESCRIPTION
 *      This function starts the discovery procedure
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void StartDiscoveryProcedure(void)
{
    SetAppState(app_connected_discovering);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      MainEnableNotifications
 *
 *  DESCRIPTION
 *      Enable notifications on the specified handle.
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/

extern void MainEnableNotifications(uint16 cid, uint16 handle)
{
    uint8 notification_enabled[2] = {0x01, 0x00};
    if(g_pts_disable_notify_ccd)
    {
        notification_enabled[0] = 0x00;
        notification_enabled[1] = 0x00;
    }

    GattWriteCharValueReq(cid,
                          GATT_WRITE_REQUEST,
                          handle,
                          2 /*size_value*/,
                          notification_enabled);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      OtaTimerHandler
 *
 *  DESCRIPTION
 *      This function gets called on expiry of OTA wait timer.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void OtaTimerHandler(timer_id tid)
{
    if(tid == g_app_data.ota_wait_tid)
    {
        /* Reset the OTA wait timer */
        TimerDelete(g_app_data.ota_wait_tid);
        g_app_data.ota_wait_tid = TIMER_INVALID;

        /* The remote device does not support anything interesting.
         * Disconnect and wait for some one else to connect.
         */
        SetAppState(app_disconnecting);

    }
}

