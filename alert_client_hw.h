/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      alert_client_hw.h
 *
 *  DESCRIPTION
 *      This file defines all the function which interact with hardware
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __ALERT_CLIENT_HW_H__
#define __ALERT_CLIENT_HW_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <timer.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "alert_notification_service_data.h"
#include "app_gatt.h"
#include "user_config.h"

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* All the LED Blinking and Buzzer code has been put under these compiler flags
 * Disable these flags at the time of current consumption measurement 
 */

#define BUTTON_PIO                                      (11)

#define PIO_BIT_MASK(pio)                               (0x01 << (pio))

/* PIO direction */
#define PIO_DIRECTION_INPUT                             (FALSE)
#define PIO_DIRECTION_OUTPUT                            (TRUE)

/* PIO state */
#define PIO_STATE_HIGH                                  (TRUE)
#define PIO_STATE_LOW                                   (FALSE)

#ifdef ENABLE_LEDBLINK
/* Setup PIOs */
#define LED_PIO                                         (4)
#define LED_PWM_INDEX_1                                 (1)

/* PWM parameters for advertising */
#define DULL_LED_ON_TIME_ADV                            (2)
#define DULL_LED_OFF_TIME_ADV                           (20)
#define DULL_LED_HOLD_TIME_ADV                          (10)

#define BRIGHT_LED_OFF_TIME_ADV                         (30)
#define BRIGHT_LED_ON_TIME_ADV                          (10)
#define BRIGHT_LED_HOLD_TIME_ADV                        (10)

#define LED_RAMP_RATE                                   (0x33)

/* PWM paramters for connection state */

#define DULL_LED_ON_TIME_CONN                           (2)
#define DULL_LED_OFF_TIME_CONN                          (20)
#define DULL_LED_HOLD_TIME_CONN                         (70)

#define BRIGHT_LED_OFF_TIME_CONN                        (30)
#define BRIGHT_LED_ON_TIME_CONN                         (10)
#define BRIGHT_LED_HOLD_TIME_CONN                       (70)
#endif /* ENABLE_LEDBLINK */

#ifdef ENABLE_BUZZER
/* Setup PIOs */
#define BUZZER_PIO                                      (3)
/* The index (0-3) of the PWM unit to be configured */
#define BUZZER_PWM_INDEX_0                              (0)

/* PWM parameters for Buzzer */

/* Dull on. off and hold times */
#define DULL_BUZZ_ON_TIME                               (2)    /* 60us */
#define DULL_BUZZ_OFF_TIME                              (15)   /* 450us */
#define DULL_BUZZ_HOLD_TIME                             (0)

/* Bright on, off and hold times */
#define BRIGHT_BUZZ_ON_TIME                             (2)    /* 60us */
#define BRIGHT_BUZZ_OFF_TIME                            (15)   /* 450us */
#define BRIGHT_BUZZ_HOLD_TIME                           (0)    /* 0us */

#define BUZZ_RAMP_RATE                                  (0xFF)

#endif /* ENABLE_BUZZER */

/* Button press timer values */
#define LONG_BUTTON_PRESS_TIMER                         (2* SECOND)
#define EXTRA_LONG_BUTTON_PRESS_TIMER                   (4* SECOND)

/*============================================================================*
 *  Public Data Types
 *============================================================================*/


/* data type for different type of buzzer beeps */
typedef enum
{
    beep_off = 0,                   /* No beeps */

    beep_short,                     /* Short beep */

    beep_long,                      /* Long beep */

    beep_twice,                     /* Two beeps */

    beep_thrice                     /* Three beeps */
}buzzer_beep_type;



typedef struct
{

#ifdef ENABLE_BUZZER
    /* Buzzer timer id */
    timer_id                          buzzer_tid;
#endif /* ENABLE_BUZZER */

    /* Timer for button press */
    timer_id                          button_press_tid;

    /* This boolean variable will be used to keep track of long button presses.
     */
    bool                              long_button_pressed;

#ifdef ENABLE_BUZZER
    /* Variable for storing beep type.*/
    buzzer_beep_type                  beep_type;

    /* Variable for keeping track of beep counts. This variable will be 
     * initialized to 0 on beep start and will incremented for each beep on and 
     * off
     */
    uint16                            beep_count;
#endif /* ENABLE_BUZZER */

}APP_HW_DATA_T;


typedef enum 
{
    app_ind_stop = 0,                           /* Stop indications */

    app_ind_adv,                                /* Advertising  state */

    app_ind_conn                               /* connected state */

} app_indication;


/*============================================================================*
 *  Public Data Declarations
 *============================================================================*/

extern APP_HW_DATA_T g_app_hw_data;


/*============================================================================*
 *         Public Function Prototypes
 *============================================================================*/

/* This function sounds the buzzer */
extern void SoundBuzzer(buzzer_beep_type beep_type);

/* This function indicates different states e.g. advertising etc to the user */
extern void SetIndication(app_indication state);

/* Initializes the alert tag hardware */
extern void InitAlertTagHardware(void);

/* This function handles the button press timer expiry */
extern void HandleLongButtonPressTimerExpiry(timer_id tid);

/* This function initializes the application hardware data */
extern void AppHwDataInit(void);

#ifdef DEBUG_THRU_UART
/* Handles the alert update */
extern void HandleAlertUpdate(alert_type alert_type);

/* Displays the category of alert received */
extern void DisplayCategory(uint8 alert);

/* Displays the alert type over the UART*/
extern void DisplayType(alert_type alert_type);

/* Displays the supported categories over the UART */
extern void DisplaySupportedCategories(alert_type alert_type);

/* This function handles the phone alert status value update */
extern void HandlePhoneAlertStatusValue(void);

/* This function handles the ringer setting value and displays it over UART */
extern void HandleRingerSettingValue(void);
#else

#define HandleAlertUpdate(alert_type)
#define DisplayCategory(alert)
#define DisplaySupportedCategories(alert_type)
#define HandlePhoneAlertStatusValue()
#define HandleRingerSettingValue()
#endif /* DEBUG_THRU_UART */

#endif /*__ALERT_CLIENT_HW_H__*/

