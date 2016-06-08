/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      alert_client.h
 *
 *  DESCRIPTION
 *      Header file for the application
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __ALERT_CLIENT_H__
#define __ALERT_CLIENT_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <bluetooth.h>
#include <timer.h>
#include <config_store.h>
/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "alert_client_hw.h"
#include "app_gatt.h"
#include "user_config.h"

/*============================================================================*
 *  Public Definitions
 *============================================================================*/
/* This compiler flag is for adding delay of 100ms before reading Phone alert 
 * status charcteristic.
 */
#define ADD_DELAY_IN_READING

#define OTA_WAIT_TIME                   (15 * MINUTE)

/*============================================================================*
 *  Public Data Types
 *============================================================================*/

/* Profiles discovered on the remote (connected) device */
typedef enum {
    ans_supported = 0x0001, /* Alert notification service */

    pas_supported = 0x0002, /* phone alert status service */
     
    /* We can add more such profiles here */
} connected_device_supported_profile;

typedef enum app_state_tag
{
    /* Initial state */
    app_init = 0,

    /* Fast undirected advertisements configured */
    app_fast_advertising,

    /* Slow undirected advertisements configured */
    app_slow_advertising,

    /* Application has established connection to the host */
    app_connected,

    /* Enters when application starts primary service discovery */
    app_connected_discovering,

    /* Enters when disconnect is initiated by the application */
    app_disconnecting,

    /* Idle state */
    app_idle,

} app_state;


/* Structure defined for central device IRK */
typedef struct
{
    uint16                            irk[MAX_WORDS_IRK];

} CENTRAL_DEVICE_IRK_T;


typedef struct
{

    app_state                               state;

    /* Value for which advertisement timer needs to be started. 
     *
     * The timer is initially started for 30 seconds 
     * to enable fast connections from any device in the vicinity.
     * This is then followed by reduced power advertisements.
     */
    uint32                                  advert_timer_value;

    /* This timer will be used for advertising timeout in advertising state and 
     * for discovery procedure in connected state.
     */
    timer_id                                app_tid;

    /* TYPED_BD_ADDR_T of the host to which application is connected */
    TYPED_BD_ADDR_T                         con_bd_addr;

    /* Track the UCID as clients connect and disconnect */
    uint16                                  st_ucid;
    
    /* Flag to track the authentication failure during the disconnection */
    bool                                    auth_failure;

    /* Boolean flag to indicated whether the device is bonded */
    bool                                    bonded;

    /* TYPED_BD_ADDR_T of the host to which application is bonded.*/
    TYPED_BD_ADDR_T                         bonded_bd_addr;

    /* Boolean flag to indicate whether encryption is enabled with the bonded 
     * host
     */
    bool                                    encrypt_enabled;

    /* variable to keep track of number of connection parameter update 
     * request made 
     */
    uint8                                   num_conn_update_req;

    /* Timer to hold the time elapsed after the last 
     * L2CAP_CONNECTION_PARAMETER_UPDATE Request failed.
     */
    timer_id                                conn_param_update_tid;

    /* Connection Parameter Update timer value. Upon a connection, it's started
     * for a period of TGAP_CPP_PERIOD, upon the expiry of which it's restarted
     * for TGAP_CPC_PERIOD. When this timer is running, if a GATT_ACCESS_IND is
     * received, it means, the central device is still doing the service discov-
     * -ery procedure. So, the connection parameter update timer is deleted and
     * recreated. Upon the expiry of this timer, a connection parameter update
     * request is sent to the central device.
     */
     uint32                                 cpu_timer_value;

    /* Profiles supported by the remote (connected) device */
    connected_device_supported_profile      connected_device_profiles;


    /* Diversifier associated with the LTK of the bonded device */
    uint16                                  diversifier;

    /* Central private address resolution IRK will only be used when
     * central device used resolvable random address. 
     */
    CENTRAL_DEVICE_IRK_T                    central_device_irk;

    /* This boolean variabled will be used to keep track of pairing remove 
     * button press.In this application, it will be set to TRUE when user 
     * performs an extra long button press.
     */
    bool                                    pairing_remove_button_pressed;

    /* Handy pointer */
    uint16                                  *conf_handle_ptr;

#ifdef ADD_DELAY_IN_READING
    /* Timer to be used to add delay in reading alert status charcteristics of
     * Phone alert status service.
     */
    timer_id                                delay_read_tid;
#endif /* ADD_DELAY_IN_READING */

    /* This timer will be used if the application is already bonded to the 
     * remote host address but the remote device wanted to rebond which we had 
     * declined. In this scenario, we give ample time to the remote device to 
     * encrypt the link using old keys. If remote device doesn't encrypt the 
     * link, we will disconnect the link on this timer expiry.
     */
    timer_id                                bonding_reattempt_tid;

    /* Varibale to store the current connection interval being used. */
    uint16                                  conn_interval;

    /* Variable to store the current slave latency. */
    uint16                                  conn_latency;

    /*Variable to store the current connection timeout value. */
    uint16                                  conn_timeout;
    
    /* OTA connection timeout timer */
    timer_id                                ota_wait_tid;

} APP_DATA_T;

/*============================================================================*
 *  Public Data Declarations
 *============================================================================*/
/* Application global data structure */
extern APP_DATA_T g_app_data;

/* This boolean flag will be used to send SILENT_MODE without checking the 
 * already present ringer setting 
 */
extern bool g_pts_send_only_silent_mode;

/* This boolean flag will be used to write disable notification and indication
 * on the client configuration descriptor.
 */
extern bool g_pts_disable_notify_ccd;

/*============================================================================*
 *  Public definition
 *============================================================================*/
#define MIN_SET(_a_, _b_)   \
    (!_a_) ? _b_ : ((!_b_) ? _a_ : ((_a_ < _b_) ? _a_ : _b_))

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function is used to set application state */
extern void SetAppState(app_state new_state);

/* This function handles the extra long button press. */
extern void HandleExtraLongButtonPressTimerExpiry(timer_id tid);

/* This function gets called on expiry of OTA wait timer. */
extern void OtaTimerHandler(timer_id tid);

/* This function handles the PIO changes event. */
extern void HandlePIOChangedEvent(uint32 pio_changed);
#endif /* __ALERT_CLIENT_H__ */
