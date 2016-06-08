/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      local_debug.c
 *
 *  DESCRIPTION
 *      This file defines debug functions
 *
 *  NOTES
 *
 ******************************************************************************/

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "local_debug.h"
#include "user_config.h"



/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

#ifdef DEBUG_THRU_UART

extern void writeString(const char *string)
{
    DebugWriteString(string);
    DebugWriteString("\n\r");
}

extern void writeNumAlerts(uint8 const val)
{
    DebugWriteString("\tNum of Alerts = ");
    DebugWriteUint8(val);
}
#endif /* DEBUG_THRU_UART */
