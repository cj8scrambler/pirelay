/* Port used to for JSON request/respsonses */
#define HTTP_PORT 8888

/* Main timer period in ms
     This timer is used as the basis for all events in the system.
 */
#define TIMER_BASE_PERIOD 100

/* Rate that temperature samples are taken (ms) */
#define TEMP_SAMPLE_RATE (50 * TIMER_BASE_PERIOD) /* 5 seconds */

/* Rate that data is logged (ms) */
#define LOG_RATE (100 * TIMER_BASE_PERIOD) /* 10 seconds */

/* Number of measured (and optionally controlled) nodes */
#define NUM_NODES 9

/* sqlite database */
#define SQLITE_DBPATH    "/var/www/pirelay/logs.db"
