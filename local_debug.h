/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      local_debug.h
 *
 *  DESCRIPTION
 *      Header file for the debug functions
 *
 *  NOTES
 *
 ******************************************************************************/

#ifndef _LOCAL_DEBUG_H
#define _LOCAL_DEBUG_H

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <debug.h>
#include "user_config.h"



/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "app_gatt.h"

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

#ifdef DEBUG_THRU_UART
/* Writes the string on UART */
extern void writeString(const char *string);

/* Writes numbers on UART */
extern void writeNumAlerts(uint8 const val);
#else

#define writeString(s)
#define writeNumAlerts(val)

#endif /* DEBUG_THRU_UART */
#endif /* _LOCAL_DEBUG_H */
