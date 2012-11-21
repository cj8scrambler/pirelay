/* Port used to for JSON request/respsonses */
#define HTTP_PORT 8888

/* Period of 1 PWM cycle (ms) */
#define PWM_PERIOD 1000

/* # of PWM updates per period 
  resolution = 1/PWM_UPDATES */
#define PWM_UPDATES 20  /* 5% resolution */

/* Number of measured (and optionally controlled) nodes */
#define NUM_NODES 7

/* Rate that temperature samples are taken (ms) */
#define TEMP_SAMPLE_RATE 5000
