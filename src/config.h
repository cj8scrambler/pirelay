/* Port used to for JSON request/respsonses */
#define HTTP_PORT 8888

/* Period of 1 PWM cycle (ms) */
#define PWM_PERIOD 1000

/* # of PWM updates per period 
  resolution = 1/PWM_UPDATES */
#define PWM_UPDATES 60  /* 5% resolution */

/* Number of measured (and optionally controlled) nodes */
#define NUM_NODES 8

/* Rate that temperature samples are taken (ms) */
#define TEMP_SAMPLE_RATE 2500

/* Rate that all data is logged (seconds) */
#define LOG_RATE 5

/* sqlite setings */
#define SQLITE_DBPATH    "/home/pi/pirelay/logs.db"

/* MySQL settings */
#define MYSQL_SERVER    "localhost"
#define MYSQL_USER      "pi"
#define MYSQL_PASSWORD  "pipass"
#define MYSQL_DATABASE  "raspi_ctrlr"
