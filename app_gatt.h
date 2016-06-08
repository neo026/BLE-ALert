/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      app_gatt.h
 *
 *  DESCRIPTION
 *      Header definitions for common application attributes
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __APP_GATT_H__
#define __APP_GATT_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <panic.h>
#include <time.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Invalid UCID indicating we are not currently connected */
#define GATT_INVALID_UCID              (0xFFFF)

/* Maximum number of words in central device IRK */
#define MAX_WORDS_IRK                  (8)

/*Number of IRKs that application can store */
#define MAX_NUMBER_IRK_STORED          (1)

/* Extract low order byte of 16-bit  */
#define LE8_L(x)                       ((x) & 0xff)

/* Extract high order byte of 16-bit  */
#define LE8_H(x)                       (((x) >> 8) & 0xff)

/* Maximum Length of Device Name
 * Note: Do not increase device name length beyond (DEFAULT_ATT_MTU -3 = 20)
 * octets as GAP service at the moment doesn't support handling of Prepare
 * write and Execute write procedures.
 */
#define DEVICE_NAME_MAX_LENGTH         (20)

/* For services 0 is an invalid NVM offset as application stores bonding flag
 * information at 0 offset
 */
#define SERVICE_INVALID_NVM_OFFSET     (0)

/* CS KEY Index for PTS */
/* Application have eight CS keys for its use. Index for these keys : [0-7]
 * First CS key at index 0 will be used for PTS test cases which require
 * application behaviour different from our current behaviour.
 * For such PTS test cases, we have implemented some work arounds which will
 * get enabled by setting user CSkey at following index
 */
#define PTS_CS_KEY_INDEX               (0x0000)

/* Note : Both these bits should not be enabled togather as normal use case
 * does not want this.
 */


/* bit0 of CSkey will be used for those PTS test cases which want application to
 * write disable notification and indication on the client configuration
 * descriptor.
 */
#define PTS_DISABLE_NOTIFY_CS_KEY_MASK (0x0001)


/* Bit1 will be used for generating context in every record */
#define PTS_WRITE_ONLY_SILENT_MODE     (0x0002)


/* This constant is used in defining  some arrays so it should always be large
 * enough to hold the advertisement data.
 */
#define MAX_ADV_DATA_LEN               (31)


#define GAP_CONN_PARAM_TIMEOUT         (30 * SECOND)

/* Timer value for remote device to re-encrypt the link using old keys */
#define BONDING_CHANCE_TIMER           (30*SECOND)

/* Timer value for remote device to retry pairing */
#define PAIRING_WAIT_TIMER             (2 * MINUTE)

/* GATT ERROR codes:
 * Going forward the following codes will be included in the firmware APIs.
 */

/* This error codes should be returned when a remote connected device writes a
 * configuration which the application does not support.
 */
#define gatt_status_desc_improper_config    \
                                       (0xFD| gatt_status_app_mask)

/* The following error codes shall be returned when a procedure is already
 * ongoing and the remote connected device request for the same procedure
 * again.
 */
#define gatt_status_proc_in_progress   (0xFE| gatt_status_app_mask)

/* This error code shall be returned if the written value is out of the
 * supported range.
 */
#define gatt_status_att_val_oor        (0xFF| gatt_status_app_mask)

/* Highest possible handle for ATT database. */
#define ATT_HIGHEST_POSSIBLE_HANDLE    (0xFFFF)

/* Extract 3rd byte (bits 16-23) of a uint24 variable */
#define THIRD_BYTE(x)                   \
                                       ((uint8)((((uint24)x) >> 16) & 0x0000ff))

/* Extract 2rd byte (bits 8-15) of a uint24 variable */
#define SECOND_BYTE(x)                  \
                                       ((uint8)((((uint24)x) >> 8) & 0x0000ff))

/* Extract least significant byte (bits 0-7) of a uint24 variable */
#define FIRST_BYTE(x)                  ((uint8)(((uint24)x) & 0x0000ff))

/* Convert a word-count to bytes */
#define WORDS_TO_BYTES(_w_)            (_w_ << 1)

/* Convert bytes to word-count*/
#define BYTES_TO_WORDS(_b_)            (((_b_) + 1) >> 1)

/* The Maximum Transmission Unit length supported by this device. */
#define ATT_MTU                        23

/* The maximum user data that can be carried in each radio packet.
 * MTU minus 3 bytes header
 */
#define MAX_DATA_LENGTH                (ATT_MTU-3)

/*============================================================================*
 *  Public Data Types
 *============================================================================*/

/* GATT client characteristic configuration value [Ref GATT spec, 3.3.3.3].
 * Client configuration is a bit field.value where each bit maps to a
 * particular configuration.
 * Bit-Configuration Mapping
 * Bit 0 - Notification
 * Bit 1 - Indication
 */
typedef enum
{
    gatt_client_config_none = 0x0000,
    gatt_client_config_notification = 0x0001,
    gatt_client_config_indication = 0x0002,
    gatt_client_config_reserved = 0xFFF4

} gatt_client_config;



/* Application defined panic codes
 * Persistent storage which is used to hold panic code is intialized to zero,
 * so the application shall not use 0 for panic codes
 */
typedef enum
{
    /* Failure while setting advertisement parameters */
    app_panic_set_advert_params = 1,

    /* Failure while setting advertisement data */
    app_panic_set_advert_data,

    /* Failure while setting scan response data */
    app_panic_set_scan_rsp_data,

    /* Failure while establishing connection */
    app_panic_connection_est,

    /* Failure while registering GATT DB with firmware */
    app_panic_db_registration,

    /* Failure while reading NVM */
    app_panic_nvm_read,

    /* Failure while writing NVM */
    app_panic_nvm_write,

    /* Failure while reading Tx Power Level */
    app_panic_read_tx_pwr_level,

    /* Failure while deleting device from whitelist */
    app_panic_delete_whitelist,

    /* Failure while adding device to whitelist */
    app_panic_add_whitelist,

    /* Failure while triggering connection parameter update procedure */
    app_panic_con_param_update,

    /* Event received in an unexpected application state */
    app_panic_invalid_state,

    /* Unexpected beep type */
    app_panic_unexpected_beep_type,

    /* Failure while setting advertisement parameters */
    app_panic_gap_set_mode,


    /* Not supported UUID*/
    app_panic_uuid_not_supported,

    /* Failure while setting scan parameters */
    app_panic_set_scan_params,

    /* Failure while erasing NVM */
    app_panic_nvm_erase,


}app_panic_code;



/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function starts the service discovery procedure */
extern void StartDiscoveryProcedure(void);

/* This function checks if application is bonded to any device or not */
extern bool AppIsDeviceBonded(void);

/* This function returns the connection identifier for the connection */
extern uint16 AppGetConnectionCid(void);

/* This function is used to enable notifications on client configuration
 * descriptors.
 */
extern void MainEnableNotifications(uint16 cid, uint16 handle);

/* This is used to report panic which results in chip reset */
extern void ReportPanic(app_panic_code panic_code);

#ifdef NVM_TYPE_FLASH
/* This function writes the application data to NVM. This function should
 * be called on getting nvm_status_needs_erase
 */
extern void WriteApplicationAndServiceDataToNVM(void);
#endif /* NVM_TYPE_FLASH */

#endif /* __APP_GATT_H__ */

