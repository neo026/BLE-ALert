/*
 * THIS FILE IS AUTOGENERATED, DO NOT EDIT!
 *
 * generated by gattdbgen from depend_Release_CSR101x_A05/app_gatt_db.db_
 */

#include "depend_Release_CSR101x_A05/app_gatt_db.h"

/* GATT database */
uint16 gattDatabase[] = {
    /* 0001: Primary Service 1801 */
    0x0002, 0x0118,
    /* 0002: Characteristic Declaration 2a05 */
    0x3005, 0x2003, 0x0005, 0x2a00,
    /* 0003:  */
    0xd000,
    /* 0004: Client Characteristic Configuration */
    0x6400,
    /* 0005: Primary Service 1800 */
    0x0002, 0x0018,
    /* 0006: Characteristic Declaration 2a00 */
    0x3005, 0x0a07, 0x0000, 0x2a00,
    /* 0007: . */
    0xd501, 0x0000,
    /* 0008: Characteristic Declaration 2a01 */
    0x3005, 0x0209, 0x0001, 0x2a00,
    /* 0009: .. */
    0xd402, 0x0002,
    /* 000a: Characteristic Declaration 2a04 */
    0x3005, 0x020b, 0x0004, 0x2a00,
    /* 000b: ........ */
    0xd408, 0xc800, 0xc800, 0x0800, 0xe803,
    /* 000c: Primary Service 180a */
    0x0002, 0x0a18,
    /* 000d: Characteristic Declaration 2a25 */
    0x3045, 0x020e, 0x0025, 0x2a00,
    /* 000e: BLE-ALERT-001 */
    0xd40d, 0x424c, 0x452d, 0x414c, 0x4552, 0x542d, 0x3030, 0x3100,
    /* 000f: Characteristic Declaration 2a27 */
    0x3045, 0x0210, 0x0027, 0x2a00,
    /* 0010: CSR101x A05 */
    0xd40b, 0x4353, 0x5231, 0x3031, 0x7820, 0x4130, 0x3500,
    /* 0011: Characteristic Declaration 2a26 */
    0x3045, 0x0212, 0x0026, 0x2a00,
    /* 0012: CSR uEnergy SDK 2.6.0 */
    0xd415, 0x4353, 0x5220, 0x7545, 0x6e65, 0x7267, 0x7920, 0x5344, 0x4b20, 0x322e, 0x362e, 0x3000,
    /* 0013: Characteristic Declaration 2a28 */
    0x3045, 0x0214, 0x0028, 0x2a00,
    /* 0014: Application version 2.6.0.0 */
    0xd41b, 0x4170, 0x706c, 0x6963, 0x6174, 0x696f, 0x6e20, 0x7665, 0x7273, 0x696f, 0x6e20, 0x322e, 0x362e, 0x302e, 0x3000,
    /* 0015: Characteristic Declaration 2a29 */
    0x3045, 0x0216, 0x0029, 0x2a00,
    /* 0016: Cambridge Silicon Radio */
    0xd417, 0x4361, 0x6d62, 0x7269, 0x6467, 0x6520, 0x5369, 0x6c69, 0x636f, 0x6e20, 0x5261, 0x6469, 0x6f00,
    /* 0017: Characteristic Declaration 2a50 */
    0x3045, 0x0218, 0x0050, 0x2a00,
    /* 0018: ...L... */
    0xd407, 0x010a, 0x004c, 0x0100, 0x0100,
    /* 0019: Primary Service 00001016-d102-11e1-9b23-00025b00a5a5 */
    0x0010, 0xa5a5, 0x005b, 0x0200, 0x239b, 0xe111, 0x02d1, 0x1610, 0x0000,
    /* 001a: Characteristic Declaration 00001013-d102-11e1-9b23-00025b00a5a5 */
    0x3053, 0x0a1b, 0x00a5, 0xa500, 0x5b02, 0x0023, 0x9be1, 0x1102, 0xd113, 0x1000, 0x0000,
    /* 001b: . */
    0xc501, 0x0000,
    /* 001c: Characteristic Declaration 00001018-d102-11e1-9b23-00025b00a5a5 */
    0x3013, 0x081d, 0x00a5, 0xa500, 0x5b02, 0x0023, 0x9be1, 0x1102, 0xd118, 0x1000, 0x0000,
    /* 001d: . */
    0xc501, 0x0000,
    /* 001e: Characteristic Declaration 00001014-d102-11e1-9b23-00025b00a5a5 */
    0x3053, 0x121f, 0x00a5, 0xa500, 0x5b02, 0x0023, 0x9be1, 0x1102, 0xd114, 0x1000, 0x0000,
    /* 001f: .................... */
    0xc414, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* 0020: Client Characteristic Configuration */
    0x6400,
    /* 0021: Characteristic Declaration 00001011-d102-11e1-9b23-00025b00a5a5 */
    0x3053, 0x0222, 0x00a5, 0xa500, 0x5b02, 0x0023, 0x9be1, 0x1102, 0xd111, 0x1000, 0x0000,
    /* 0022: . */
    0xc401, 0x0600,
    /* 0023: Primary Service 180f */
    0x0002, 0x0f18,
    /* 0024: Characteristic Declaration 2a19 */
    0x3005, 0x1225, 0x0019, 0x2a00,
    /* 0025: . */
    0xd401, 0x0000,
    /* 0026: Client Characteristic Configuration */
    0x6400,
};

uint16 *GattGetDatabase(uint16 *len)
{
    *len = sizeof(gattDatabase);
    return gattDatabase;
}

/* End-of-File */