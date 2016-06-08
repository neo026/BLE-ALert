/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      alert_client_hw.c
 *
 *  DESCRIPTION
 *      This file defines all the function which interact with hardware
 *
 *  NOTES
 *
 ******************************************************************************/
 
/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <main.h>
#include <pio_ctrlr.h>
#include <pio.h>
#include <timer.h>
#include <ls_app_if.h>
#include <mem.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "alert_client_hw.h"
#include "alert_client.h"
#include "alert_client_gatt.h"
#include "phone_alert_service_data.h"
#include "local_debug.h"


/*============================================================================*
 *  Private Data
 *============================================================================*/
/* Alert tag application hardware data structure */
APP_HW_DATA_T g_app_hw_data;


/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
#ifdef ENABLE_BUZZER
/* This function gets called on buzzer timer expiry and stops it */
static void appBuzzerTimerHandler(timer_id tid);
#endif /* ENABLE_BUZZER */
/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

#ifdef ENABLE_BUZZER
/*----------------------------------------------------------------------------*
 *  NAME
 *      appBuzzerTimerHandler
 *
 *  DESCRIPTION
 *      This function is used to stop the Buzzer at the expiry of 
 *      timer.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void appBuzzerTimerHandler(timer_id tid)
{
    uint32 beep_timer = SHORT_BEEP_TIMER_VALUE;

    g_app_hw_data.buzzer_tid = TIMER_INVALID;

    switch(g_app_hw_data.beep_type)
    {
        case beep_short: /* FALLTHROUGH */
        case beep_long:
        {
            g_app_hw_data.beep_type = beep_off;

            /* Disable buzzer */
            PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);
        }
        break;
        case beep_twice:
        {
            if(g_app_hw_data.beep_count == 0)
            {
                /* First beep sounded. Start the silent gap*/
                g_app_hw_data.beep_count = 1;

                /* Disable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

                /* Time gap between two beeps */
                beep_timer = BEEP_GAP_TIMER_VALUE;
            }
            else if(g_app_hw_data.beep_count == 1)
            {
                g_app_hw_data.beep_count = 2;

                /* Enable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, TRUE);

                /* Start beep */
                beep_timer = SHORT_BEEP_TIMER_VALUE;
            }
            else
            {
                /* Two beeps have been sounded. Stop buzzer now*/
                g_app_hw_data.beep_count = 0;

                /* Disable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

                g_app_hw_data.beep_type = beep_off;
            }
        }
        break;
        case beep_thrice:
        {
            if(g_app_hw_data.beep_count == 0 ||
               g_app_hw_data.beep_count == 2)
            {
                /* First beep sounded. Start the silent gap*/
                g_app_hw_data.beep_count++;

                /* Disable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

                /* Time gap between two beeps */
                beep_timer = BEEP_GAP_TIMER_VALUE;
            }
            else if(g_app_hw_data.beep_count == 1 ||
                    g_app_hw_data.beep_count == 3)
            {
                g_app_hw_data.beep_count++;

                /* Enable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, TRUE);

                beep_timer = SHORT_BEEP_TIMER_VALUE;
            }
            else
            {
                /* Two beeps have been sounded. Stop buzzer now*/
                g_app_hw_data.beep_count = 0;

                /* Disable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

                g_app_hw_data.beep_type = beep_off;
            }
        }
        break;
        default:
        {
            /* No such beep type */
            ReportPanic(app_panic_unexpected_beep_type);
            /* Though break statement will not be executed after panic but this
             * has been kept to avoid any confusion for default case.
             */
        }
        break;
    }

    if(g_app_hw_data.beep_type != beep_off)
    {
        /* start the timer */
        g_app_hw_data.buzzer_tid = TimerCreate(beep_timer, TRUE, 
                                               appBuzzerTimerHandler);
    }

}
#endif /* ENABLE_BUZZER*/


/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitAlertTagHardware  -  initialize application hardware
 *
 *  DESCRIPTION
 *      This function is called upon a power reset to initialize the PIOs
 *      and configure their initial states.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void InitAlertTagHardware(void)
{
#ifndef DEBUG_THRU_UART
    /* Don't wakeup on UART RX line */
    SleepWakeOnUartRX(FALSE);
#endif
    /* Setup PIOs
     * PIO3 - Buzzer - BUZZER_PIO
     * PIO4 - LED 1 - LED_PIO
     * PIO11 - Button - BUTTON_PIO
     */

    PioSetModes(PIO_BIT_MASK(BUTTON_PIO), pio_mode_user);
    PioSetDir(BUTTON_PIO, PIO_DIRECTION_INPUT); /* input */
    PioSetPullModes(PIO_BIT_MASK(BUTTON_PIO), pio_mode_strong_pull_up); 
    /* Setup button on PIO11 */
    PioSetEventMask(PIO_BIT_MASK(BUTTON_PIO), pio_event_mode_both);


#ifdef ENABLE_LEDBLINK
    /* PWM is being used for LED glowing.*/
    PioSetModes(PIO_BIT_MASK(LED_PIO), pio_mode_pwm1);

    /* Advertising parameters are being configured for PWM right now. When
     * application moves to connection state, we change PWM parameters to
     * the ones for connection
     */

    PioConfigPWM(LED_PWM_INDEX_1, pio_pwm_mode_push_pull, DULL_LED_ON_TIME_ADV,
         DULL_LED_OFF_TIME_ADV, DULL_LED_HOLD_TIME_ADV, BRIGHT_LED_OFF_TIME_ADV,
         BRIGHT_LED_ON_TIME_ADV, BRIGHT_LED_HOLD_TIME_ADV, LED_RAMP_RATE);

    PioEnablePWM(LED_PWM_INDEX_1, FALSE);
#endif /* ENABLE_LEDBLINK */


#ifdef ENABLE_BUZZER
    PioSetModes(PIO_BIT_MASK(BUZZER_PIO), pio_mode_pwm0);
    /* Configure the buzzer on PIO3 */
    PioConfigPWM(BUZZER_PWM_INDEX_0, pio_pwm_mode_push_pull, DULL_BUZZ_ON_TIME,
                 DULL_BUZZ_OFF_TIME, DULL_BUZZ_HOLD_TIME, BRIGHT_BUZZ_ON_TIME,
                 BRIGHT_BUZZ_OFF_TIME, BRIGHT_BUZZ_HOLD_TIME, BUZZ_RAMP_RATE);


    PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);
#endif /* ENABLE_BUZZER */

    /* Set the I2C pins to pull down */
    PioSetI2CPullMode(pio_i2c_pull_mode_strong_pull_down);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleLongButtonPressTimerExpiry
 *
 *  DESCRIPTION
 *      This function is for handling the long Button press timer expiry
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void HandleLongButtonPressTimerExpiry(timer_id tid)
{
    if(tid == g_app_hw_data.button_press_tid)
    {
        /* Set the long button pressed flag. */
        g_app_hw_data.long_button_pressed = TRUE;
        g_app_hw_data.button_press_tid = TIMER_INVALID;

        /* long button press timer has expired. Now Start a timer of value 
         * (EXTRA_LONG_BUTTON_PRESS_TIMER - LONG_BUTTON_PRESS_TIMER).
         *
         * If application receives the button release event between this time 
         * (EXTRA_LONG_BUTTON_PRESS_TIMER - LONG_BUTTON_PRESS_TIMER), it means
         * that user has performed an long button press which will handled 
         * accordingly.
         *
         * else if this timer expires before button release event then it will 
         * mean that user had performed extra long button press.
         */

        g_app_hw_data.button_press_tid = 
                TimerCreate(
                         EXTRA_LONG_BUTTON_PRESS_TIMER -LONG_BUTTON_PRESS_TIMER,
                         TRUE,
                         HandleExtraLongButtonPressTimerExpiry);
    }
    /* else it may be because of some race condition, ignore it */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppHwDataInit
 *
 *  DESCRIPTION
 *      This function initializes the application hardware data
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void AppHwDataInit(void)
{
    /* Clear the button press timer id. */
    TimerDelete(g_app_hw_data.button_press_tid);
    g_app_hw_data.button_press_tid = TIMER_INVALID;

    SetIndication(app_ind_stop);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      SoundBuzzer
 *
 *  DESCRIPTION
 *      Function for sounding beeps.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void SoundBuzzer(buzzer_beep_type beep_type)
{
#ifdef ENABLE_BUZZER
    uint32 beep_timer = SHORT_BEEP_TIMER_VALUE;

    PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);
    TimerDelete(g_app_hw_data.buzzer_tid);
    g_app_hw_data.buzzer_tid = TIMER_INVALID;
    g_app_hw_data.beep_count = 0;

    /* Store the beep type in some global variable. It will used on timer expiry
     * to check the type of beeps being sounded.
     */
    g_app_hw_data.beep_type = beep_type;

    switch(g_app_hw_data.beep_type)
    {
        case beep_off:
        {
            /* Don't do anything */
        }
        break;
        case beep_short:
            /* FALLTHROUGH */
        case beep_twice: /* Two short beeps will be sounded */
            /* FALLTHROUGH */
        case beep_thrice: /* Three short beeps will be sounded */
        {
            /* One short beep will be sounded */
            beep_timer = SHORT_BEEP_TIMER_VALUE;
        }
        break;
        
        case beep_long:
        {
            /* One long beep will be sounded */
            beep_timer = LONG_BEEP_TIMER_VALUE;
        }
        break;

        break;
        default:
        {
            /* No such beep type defined */
            ReportPanic(app_panic_unexpected_beep_type);
            /* Though break statement will not be executed after panic but this
             * has been kept to avoid any confusion for default case.
             */
        }
        break;
    }

    if(g_app_hw_data.beep_type != beep_off)
    {
        /* Initialize beep count to zero */
        g_app_hw_data.beep_count = 0;

        /* Enable buzzer */
        PioEnablePWM(BUZZER_PWM_INDEX_0, TRUE);

        TimerDelete(g_app_hw_data.buzzer_tid);
        g_app_hw_data.buzzer_tid = TimerCreate(beep_timer, TRUE, 
                                               appBuzzerTimerHandler);
    }
#endif /* ENABLE_BUZZER */
}



/*----------------------------------------------------------------------------*
 *  NAME
 *      SetIndication
 *
 *  DESCRIPTION
 *      This function indicates the app state through LED blinking
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void SetIndication(app_indication state)
{

#ifdef ENABLE_LEDBLINK
    if(state == app_ind_stop)
    {
        /*Stop LED glowing */
        PioEnablePWM(LED_PWM_INDEX_1, FALSE);

        /* Reconfigure LED to pio_mode_user. This reconfiguration has been done
         * because When PWM is disabled, LED pio value remains same as it was at
         * the exact time of disabling. So if LED was on, it may remain ON even
         * after PWM disabling. So it is better to reconfigure it to user mode.
         * It will reconfigured to PWM mode while enabling.
         */
        PioSetModes(PIO_BIT_MASK(LED_PIO), pio_mode_user);
        PioSet(LED_PIO, FALSE);
    }
    else
    {
        if(state == app_ind_adv)
        {
            /* Fast Blinking for advertising */
            PioConfigPWM(LED_PWM_INDEX_1, pio_pwm_mode_push_pull, 
                DULL_LED_ON_TIME_ADV, DULL_LED_OFF_TIME_ADV,
                DULL_LED_HOLD_TIME_ADV, BRIGHT_LED_ON_TIME_ADV,
                BRIGHT_LED_OFF_TIME_ADV, BRIGHT_LED_HOLD_TIME_ADV, 
                LED_RAMP_RATE);
        }
        else if(state == app_ind_conn)
        {
            /* slow blinking for connected state */
            PioConfigPWM(LED_PWM_INDEX_1, pio_pwm_mode_push_pull, 
                DULL_LED_ON_TIME_CONN, DULL_LED_OFF_TIME_CONN, 
                DULL_LED_HOLD_TIME_CONN, BRIGHT_LED_ON_TIME_CONN,
                BRIGHT_LED_OFF_TIME_CONN, BRIGHT_LED_HOLD_TIME_CONN, 
                LED_RAMP_RATE);
        }

        PioSetModes(PIO_BIT_MASK(LED_PIO), pio_mode_pwm1);
        /*Start LED glowing */
        PioEnablePWM(LED_PWM_INDEX_1, TRUE);
        PioSet(LED_PIO, TRUE);
    }
#endif /* ENABLE_LEDBLINK */
}



#ifdef DEBUG_THRU_UART
/*----------------------------------------------------------------------------*
 *  NAME
 *      DisplayType
 *
 *  DESCRIPTION
 *      This function displays alert type
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void DisplayType(alert_type alert)
{
    if(alert == alert_type_new)
    {
        writeString("NEW ALRT");
    }
    else if (alert == alert_type_unread)
    {
        writeString("UNREAD ALRT");
    }
    else
    {
        writeString("Unknown alert!");
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      DisplayCategory
 *
 *  DESCRIPTION
 *      This function displays the category of alerts (ANS)
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void DisplayCategory(uint8 alert)
{
    switch(alert)
    {
        case alert_cat_simple:
        {
            /* Simple alert: general text alert or non-text alert */
            writeString("\tSimple");
        }
        break;

        case alert_cat_email:
        {
            /* Email: alert when email messages arrives */
            writeString("\tEmail");
        }
        break;

        case alert_cat_news:
        {
            /* News: news feeds such as RSS, atom */
            writeString("\tNews");
        }
        break;

        case alert_cat_call:
        {
            /* Call: incoming call */
            writeString("\tCall");
        }
        break;

        case alert_cat_missed_call:
        {
            /* Missed call*/
            writeString("\tMsd call");
        }
        break;

        case alert_cat_message:
        {
            /* SMS/MMS message arrives */
            writeString("\tSMS");
        }
        break;

        case alert_cat_vmail:
        {
            /* Voice mail*/
            writeString("\tVmail");
        }
        break;

        case alert_cat_schedule:
        {
            /* Schedule alert occurred on calendar planner */
            writeString("\tSchd");
        }
        break;

        case alert_cat_priority:
        {
            /* High prioritized alert: alert that should be handled as high 
             * priority 
             */
            writeString("\tHi-pri");
        }
        break;

        case alert_cat_instant_msg:
        {
            /* Instant message: alert for incoming instant messages */
            writeString("\tIM");
        }
        break;

        default:
        {
            /* Unsupported alert type */
            writeString("\tUnsup");
        }
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAlertUpdate
 *
 *  DESCRIPTION
 *      This function indicates the type and number of alerts received 
 *      over the UART (ANS)
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void HandleAlertUpdate(alert_type alert)
{

    uint8 alert_cat = alert_cat_simple;
    uint8 num_alert = 0;


    /* The first byte is the category identifier */
    /* The second byte is the number of new alerts in this category */

    if(alert == alert_type_new)
    {
        alert_cat = g_app_ans_data.new_alert_value[0];
        num_alert = g_app_ans_data.new_alert_value[1];
    }
    else
    {
        /* Unread alert */
        alert_cat= g_app_ans_data.unread_alert_status_value[0];
        num_alert = g_app_ans_data.unread_alert_status_value[1];
    }


    DisplayType(alert);
    DisplayCategory(alert_cat);
    writeNumAlerts(num_alert);
    writeString("\n");
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      DisplaySupportedCategories
 *
 *  DESCRIPTION
 *      This function displays the supported categories of ANS
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void DisplaySupportedCategories(alert_type alert)
{

    uint8 alert_cat[2];

    writeString("Supported");
    if(alert == alert_type_new)
    {
        MemCopy(alert_cat,&g_app_ans_data.supported_new_alert_cat_value[0], 2);
    }
    else if(alert == alert_type_unread)
    {
        MemCopy(alert_cat,
                &g_app_ans_data.supported_unread_alert_cat_value[0], 2);
    }
    /* Display the alert type */
    DisplayType(alert);

    /* Display the categories which remote host supports */
    if(alert_cat[0] & alert_cat_bitmask_simple)
    {
        DisplayCategory(alert_cat_simple);
    }

    if(alert_cat[0] & alert_cat_bitmask_email)
    {
        DisplayCategory(alert_cat_email);
    }

    if(alert_cat[0] & alert_cat_bitmask_news)
    {
        DisplayCategory(alert_cat_news);
    }

    if(alert_cat[0] & alert_cat_bitmask_call)
    {
        DisplayCategory(alert_cat_call);
    }

    if(alert_cat[0] & alert_cat_bitmask_missed_call)
    {
        DisplayCategory(alert_cat_missed_call);
    }

    if(alert_cat[0] & alert_cat_bitmask_message)
    {
        DisplayCategory(alert_cat_message);
    }

    if(alert_cat[0] & alert_cat_bitmask_vmail)
    {
        DisplayCategory(alert_cat_vmail);
    }

    if(alert_cat[0] & alert_cat_bitmask_schedule)
    {
        DisplayCategory(alert_cat_schedule);
    }

    if(alert_cat[1] & alert_cat_bitmask_priority)
    {
        DisplayCategory(alert_cat_priority);
    }

    if(alert_cat[1] & alert_cat_bitmask_instant_msg)
    {
        DisplayCategory(alert_cat_instant_msg);
    }

    writeString("\n");
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePhoneAlertStatusValue
 *
 *  DESCRIPTION
 *      This function indicates the phone alert status 
 *      over the UART (PAS)
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void HandlePhoneAlertStatusValue(void)
{
    /* The new value is already in g_app_pas_data.phone_alert_status_value */
    
    writeString("\nPHONE ALERT STATUS");
    
    if(g_app_pas_data.phone_alert_status_value & ringer_bitmask_ringer)
    {
        writeString("\tRinger ON");
    }
    else
    {
        writeString("\tRinger OFF");
    }

    if(g_app_pas_data.phone_alert_status_value & ringer_bitmask_vibrate)
    {
        writeString("\tVibrator ON");
    }
    else
    {
        writeString("\tVibrator OFF");
    }

    if(g_app_pas_data.phone_alert_status_value & ringer_bitmask_display)
    {
        writeString("\tDisplay ON");
    }
    else
    {
        writeString("\tDisplay OFF");
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleRingerSettingValue
 *
 *  DESCRIPTION
 *      This function indicates the ringer setting
 *      over the UART
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void HandleRingerSettingValue(void)
{
    /* The new value is aleady in g_app_pas_data.ringer_setting_value */
    writeString("\nRINGER SETTING");
    if(g_app_pas_data.ringer_setting_value == ringer_setting_silent)
    {
        writeString("\tSilent");
    }
    if(g_app_pas_data.ringer_setting_value == ringer_setting_normal)
    {
        writeString("\tNormal");
    }
}
#endif /* DEBUG_THRU_UART*/
