/******************************************************************************
 *  Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 *  FILE
 *      gatt_service.c
 *
 *  DESCRIPTION
 *      This file defines routines for using GATT service.
 *
 *****************************************************************************/

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <gatt.h>
#include <gatt_uuid.h>
#include <buf_utils.h>
#include <nvm.h>

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "gatt_service.h"
#include "app_gatt_db.h"
#include "app_gatt.h"
#include "nvm_access.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* The position of the Service Changed configuration information in the NVM */
#define SERV_CHANGED_CLIENT_CONFIG_OFFSET  \
                                      0

/* The position of the "this device might have been updated" flag in the NVM */
#define SERV_CHANGED_SEND_IND         ((SERV_CHANGED_CLIENT_CONFIG_OFFSET + \
                                       sizeof(g_gatt_data.serv_changed_config)))

/* The maximum number of NVM words used by this GATT implementation */
#define SERV_CHANGED_NVM_MEMORY_WORDS ((SERV_CHANGED_SEND_IND + \
                                        sizeof(g_gatt_data.serv_changed)))

/*=============================================================================*
 *  Private Data Types
 *============================================================================*/

/* GATT service data structure */
typedef struct
{
    uint16 serv_changed;

    /* The current configuration for the Service Changed characteristic */
    gatt_client_config serv_changed_config;

    uint16 nvm_offset;

} GATT_DATA_T;

/*=============================================================================*
 *  Private Data
 *============================================================================*/

/* GATT service data instance */
static GATT_DATA_T g_gatt_data;

/*=============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise gatt service data
 *      structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void GattDataInit(void)
{
    if(!AppIsDeviceBonded())
    {
        /* Initialise Service Changed Client Configuration Characterisitic
         * descriptor value only if device is not bonded
         */
        g_gatt_data.serv_changed_config = gatt_client_config_none;
        g_gatt_data.serv_changed = FALSE;

        /* Reset the information in the NVM */
        Nvm_Write((uint16*)&(g_gatt_data.serv_changed_config),
                 sizeof(g_gatt_data.serv_changed_config),
                 (g_gatt_data.nvm_offset + SERV_CHANGED_CLIENT_CONFIG_OFFSET));

        Nvm_Write((uint16*)&(g_gatt_data.serv_changed),
                 sizeof(g_gatt_data.serv_changed),
                 (g_gatt_data.nvm_offset + SERV_CHANGED_SEND_IND));
    }

}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattReadDataFromNVM
 *
 *  DESCRIPTION
 *      This function reads the GATT service data from NVM.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void GattReadDataFromNVM(uint16 *p_offset)
{
    g_gatt_data.nvm_offset = *p_offset;

    /* Read NVM only if devices are bonded */
    if(AppIsDeviceBonded())
    {
        /* Read Service Changed client configuration */
        Nvm_Read((uint16*)&(g_gatt_data.serv_changed_config),
                sizeof(g_gatt_data.serv_changed_config),
                (g_gatt_data.nvm_offset + SERV_CHANGED_CLIENT_CONFIG_OFFSET));

        /* Read Service Has Changed flag */
        Nvm_Read((uint16*)&(g_gatt_data.serv_changed),
                sizeof(g_gatt_data.serv_changed),
                (g_gatt_data.nvm_offset + SERV_CHANGED_SEND_IND));
    }
    else
    {
        g_gatt_data.serv_changed_config = gatt_client_config_none;
        g_gatt_data.serv_changed = FALSE;

        /* Reset the information in the NVM */

        Nvm_Write((uint16*)&(g_gatt_data.serv_changed_config),
                 sizeof(g_gatt_data.serv_changed_config),
                 (g_gatt_data.nvm_offset + SERV_CHANGED_CLIENT_CONFIG_OFFSET));

        Nvm_Write((uint16*)&(g_gatt_data.serv_changed),
                 sizeof(g_gatt_data.serv_changed),
                 (g_gatt_data.nvm_offset + SERV_CHANGED_SEND_IND));
    }

    /* Increment the offset by the number of words of NVM memory required
     * by GATT service
     */
    *p_offset += SERV_CHANGED_NVM_MEMORY_WORDS;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattOnConnection
 *
 *  DESCRIPTION
 *      This function should be called when a bonded host connects.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void GattOnConnection(uint16 st_ucid)
{
    uint8 service_changed_data[4];

    service_changed_data[0] = WORD_LSB((HANDLE_GATT_SERVICE_END+1));
    service_changed_data[1] = WORD_MSB((HANDLE_GATT_SERVICE_END+1));
    service_changed_data[2] = WORD_LSB(ATT_HIGHEST_POSSIBLE_HANDLE);
    service_changed_data[3] = WORD_MSB(ATT_HIGHEST_POSSIBLE_HANDLE);


    if((g_gatt_data.serv_changed) &&
       (g_gatt_data.serv_changed_config == gatt_client_config_indication))
    {
        GattCharValueIndication(st_ucid,
                                HANDLE_SERVICE_CHANGED,
                                sizeof(service_changed_data),
                                service_changed_data);

        /* Now that the indication has been sent, clear the flag in the NVM */
        g_gatt_data.serv_changed = FALSE;

        Nvm_Write((uint16*)&(g_gatt_data.serv_changed),
                 sizeof(g_gatt_data.serv_changed),
                 (g_gatt_data.nvm_offset + SERV_CHANGED_SEND_IND));
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattOnOtaSwitch
 *
 *  DESCRIPTION
 *      This function should be called when the device is switched into
 *      over-the-air update mode.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void GattOnOtaSwitch(void)
{
    if((AppIsDeviceBonded()) &&
       (g_gatt_data.serv_changed_config == gatt_client_config_indication))
    {
        /* Record that a Service Changed indication will likely need to be sent
         * to the bonded device next time it connects.
         */
        g_gatt_data.serv_changed = TRUE;

        Nvm_Write((uint16*)&(g_gatt_data.serv_changed),
                 sizeof(g_gatt_data.serv_changed),
                 (g_gatt_data.nvm_offset + SERV_CHANGED_SEND_IND));
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattServiceChangedIndActive
 *
 *  DESCRIPTION
 *      This function allows other modules to read whether the bonded device
 *      has requested indications on the Service Changed characteristic.
 *
 *  RETURNS
 *      True if indications are requested, false otherwise.
 *
 *----------------------------------------------------------------------------*/

extern bool GattServiceChangedIndActive(void)
{
    return (AppIsDeviceBonded() &&
            (g_gatt_data.serv_changed_config == gatt_client_config_indication));
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles READ operations on GATT service attributes.
 *       It responds with the GATT_ACCESS_RSP message.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void GattHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16  data_length = 0;
    uint8   value[2];
    uint8   *p_value = NULL;
    sys_status rc = gatt_status_read_not_permitted;

    if(p_ind->handle == HANDLE_SERVICE_CHANGED_CLIENT_CONFIG)
    {
        /* Service changed client characteristic configuration descriptor read
         * has been requested
         */
        data_length = 2;
        p_value = value;
        BufWriteUint16((uint8 **)&p_value, g_gatt_data.serv_changed_config);
        rc = sys_status_success;
    }

    /* Send Access Response */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, data_length, value);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattHandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles WRITE operations on GATT service attributes.
 *      It responds with the GATT_ACCESS_RSP message.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void GattHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    uint16  client_config;
    uint8  *p_value = p_ind->value;
    sys_status rc = gatt_status_write_not_permitted;

    if(p_ind->handle == HANDLE_SERVICE_CHANGED_CLIENT_CONFIG)
    {
        client_config = BufReadUint16(&p_value);

        /* Client configuration is a bit field value, so ideally bit wise
         * comparison should be used but since the application supports only
         * indications or nothing, direct comparison should be used.
         */
        if((client_config == gatt_client_config_indication) ||
           (client_config == gatt_client_config_none))
        {
            g_gatt_data.serv_changed_config = client_config;
            rc = sys_status_success;
            if(AppIsDeviceBonded())
            {
                Nvm_Write((uint16*)&(g_gatt_data.serv_changed_config),
                          sizeof(g_gatt_data.serv_changed_config),
                          (g_gatt_data.nvm_offset +
                           SERV_CHANGED_CLIENT_CONFIG_OFFSET));
            }
        }
        else
        {
            rc = gatt_status_desc_improper_config;
        }
    }
    /* Send Access Response */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the GATT
 *      service
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern bool GattCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_GATT_SERVICE) &&
            (handle <= HANDLE_GATT_SERVICE_END))
            ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattBondingNotify
 *
 *  DESCRIPTION
 *      This function is used by application to notify bonding status to
 *      gatt service
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void GattBondingNotify(void)
{
    /* Write data to NVM if bond is established */
    if(AppIsDeviceBonded())
    {
        /* Write to NVM the Service Changed Client Configuration value */
        Nvm_Write((uint16*)&(g_gatt_data.serv_changed_config),
                  sizeof(g_gatt_data.serv_changed_config),
                  (g_gatt_data.nvm_offset +
                   SERV_CHANGED_CLIENT_CONFIG_OFFSET));
    }
}

#ifdef NVM_TYPE_FLASH
/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteGattServiceDataInNvm
 *
 *  DESCRIPTION
 *      This function writes Gatt service data in NVM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void WriteGattServiceDataInNvm(void)
{
    /* Write to NVM the Service Changed Client Configuration value */
    Nvm_Write((uint16*)&(g_gatt_data.serv_changed_config),
              sizeof(g_gatt_data.serv_changed_config),
              (g_gatt_data.nvm_offset +
               SERV_CHANGED_CLIENT_CONFIG_OFFSET));

    /* Write to NVM the Service Changed flag */
    Nvm_Write((uint16*)&(g_gatt_data.serv_changed),
              sizeof(g_gatt_data.serv_changed),
              (g_gatt_data.nvm_offset + SERV_CHANGED_SEND_IND));
}

#endif /* NVM_TYPE_FLASH */