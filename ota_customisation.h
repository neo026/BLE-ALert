/******************************************************************************
 *  Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      ota_customisation.h
 *
 *  DESCRIPTION
 *      Customisation requirements for the CSR OTAU functionality.
 *
 *****************************************************************************/

#ifndef __OTA_CUSTOMISATION_H__
#define __OTA_CUSTOMISATION_H__

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"

/* ** CUSTOMISATION **
 * The following header file names may need altering to match your application.
 */

#include "app_gatt.h"
#include "app_gatt_db.h"
#include "alert_client.h"

/*=============================================================================*
 *  Private Definitions
 *============================================================================*/

/* ** CUSTOMISATION **
 * Change these definitions to match your application.
 */
#define CONNECTION_CID      g_app_data.st_ucid
#define IS_BONDED           g_app_data.bonded
#define CONN_CENTRAL_ADDR   g_app_data.con_bd_addr
#define CONNECTION_IRK      g_app_data.central_device_irk.irk
#define LINK_DIVERSIFIER    g_app_data.diversifier

#endif /* __OTA_CUSTOMISATION_H__ */

