/* Port used to for JSON request/respsonses */
#define HTTP_PORT 8888

/* Main timer period in ms
     This timer is used as the basis for all events in the system.
     This is the max resolution of the PWM output signal and is also
     used as a base timer for temp sampling and data logging.
 */
#define TIMER_BASE_PERIOD 100

/* Number of PWM updates per PWM period
     PWM resolution % = 1/PWM_UPDATES
     PWM period = TIMER_BASE_PERIOD * PWM_UPDATES */
#define PWM_UPDATES 20  /* 5% resolution & 2 ms period */

/* Rate that temperature samples are taken (ms) */
#define TEMP_SAMPLE_RATE (50 * TIMER_BASE_PERIOD) /* 5 seconds */

/* Rate that data is logged (ms) */
#define LOG_RATE (100 * TIMER_BASE_PERIOD) /* 10 seconds */

/* Number of measured (and optionally controlled) nodes */
#define NUM_NODES 8

/* sqlite setings */
#define SQLITE_DBPATH    "/var/www/pirelay/logs.db"
