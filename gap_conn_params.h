/******************************************************************************
 *  Copyright (c) 2012 - 2016 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.6.0
 *  Application version 2.6.0.0
 *
 * FILE
 *      gap_conn_params.h
 *
 * DESCRIPTION
 *      MACROs for connection parameter values
 *
 ******************************************************************************/

#ifndef __GAP_CONN_PARAMS_H__
#define __GAP_CONN_PARAMS_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Brackets should not be used around the value of a macro. The parser 
 * which creates .c and .h files from .db file doesn't understand  brackets 
 * and will raise syntax errors.
 */

/* NOTE:
 * Connection parameters values should be in range specified by the Bluetooth 
 * specification.
 */
/* Minimum and maximum connection interval in number of frames. */
#define PREFERRED_MAX_CON_INTERVAL     0x00c8 /* 250 ms */
#define PREFERRED_MIN_CON_INTERVAL     0x00c8 /* 250 ms */

/* Slave latency in number of connection intervals. */
#define PREFERRED_SLAVE_LATENCY        0x0008 /* 8 conn_intervals. */

/* Supervision timeout (ms) = PREFERRED_SUPERVISION_TIMEOUT * 10 ms */
#define PREFERRED_SUPERVISION_TIMEOUT  0x03e8 /* 10 seconds. */



/* APPLE Compliant connection parameters */
/* Minimum and maximum connection interval in number of frames. */
#define APPLE_MAX_CON_INTERVAL         0x00c8 /* 250 ms */
#define APPLE_MIN_CON_INTERVAL         0x00b8 /* 230 ms */

/* Slave latency in number of connection intervals. */
#define APPLE_SLAVE_LATENCY            0x0004 /* 4 conn_intervals. */

/* Supervision timeout (ms) = PREFERRED_SUPERVISION_TIMEOUT * 10 ms */
#define APPLE_SUPERVISION_TIMEOUT      0x0258 /* 6 seconds. */



/* Max num of connection parameter update that we send in one connection*/
#define MAX_NUM_CONN_PARAM_UPDATE_REQS 4


/* Number of times out of the total maximum number of connection parameter 
 * update attempts(MAX_NUM_CONN_PARAM_UPDATE_REQS) when the application requests 
 * its own preferred connection paramaters.
 */
#define CPU_SELF_PARAMS_MAX_ATTEMPTS   2

/* Number of times out of the total maximum number of connection parameter 
 * update attempts(MAX_NUM_CONN_PARAM_UPDATE_REQS) when the application 
requests 
 * its APPLE preferred connection paramaters.
 */
#define CPU_APPLE_PARAMS_MAX_ATTEMPTS  (MAX_NUM_CONN_PARAM_UPDATE_REQS -\
                                             CPU_SELF_PARAMS_MAX_ATTEMPTS)
                                             

/* Advertising parameters, time is expressed in microseconds and the firmware
 * will round this down to the nearest slot. Acceptable range is 20ms to 10.24s
 * and the minimum must be no larger than the maximum. This value needs to be 
 * modified at later stage as decided GPA for specific profile.
 *
 * To enable fast connections though the recommended 
 * range is between 20 ms to 30 ms, but it has been observed that it is way too 
 * energy expensive for applications running on coin cell battery. So, we
 * have decided to use 60 ms as the fast connection advertisement interval. For 
 * reduced power connections, the recommended range is between 1s to 2.5 s. 
 * Vendors will need to tune these values as per their requirements.
 */
#define FC_ADVERTISING_INTERVAL_MIN    (60 * MILLISECOND)
#define FC_ADVERTISING_INTERVAL_MAX    (60 * MILLISECOND)
    
#define RP_ADVERTISING_INTERVAL_MIN    (1280 * MILLISECOND)
#define RP_ADVERTISING_INTERVAL_MAX    (1280 * MILLISECOND)


#endif /* __GAP_CONN_PARAMS_H__ */
