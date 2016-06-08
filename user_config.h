/*******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 * FILE
 *      user_config.h
 *
 * DESCRIPTION
 *      This file contains definitions which will enable customization of the
 *      application.
 *
 ******************************************************************************/

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/*=============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Buzzer code has been put under compiler flag ENABLE_BUZZER. If required
 * this flag can be disabled like at the time of current consumption 
 * measurement 
 */
#define ENABLE_BUZZER

/* This macro is required to be disabled if user does not want to see messages
 * on UART
 */
#define DEBUG_THRU_UART

#ifdef ENABLE_BUZZER
/* TIMER values for Buzzer */
#define SHORT_BEEP_TIMER_VALUE  (100* MILLISECOND)
#define LONG_BEEP_TIMER_VALUE   (500* MILLISECOND)
#define BEEP_GAP_TIMER_VALUE    (25* MILLISECOND)
#endif /* ENABLE_BUZZER */


/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Timer values for fast and slow advertisements. */
#define FAST_CONNECTION_ADVERT_TIMEOUT_VALUE      (30 * SECOND)
#define SLOW_CONNECTION_ADVERT_TIMEOUT_VALUE      (1 * MINUTE)

#endif /* __USER_CONFIG_H__ */
