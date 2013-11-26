#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sqlite3.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include "config.h"

#define MAX_NAME_LEN 24

#define TEMP_F(c) (c * 0.0018 + 32.0)
#define TEMP_C(c) (c / 1000.0)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

enum power_state {
  OFF = 0,
  ON = 1,
};

enum ctrlr_type {
  PID,
  ON_OFF,
  COMPRESSOR, /* ON_OFF plus minimum ON/OFF times */
  DISABLED,
};

enum ctrlr_mode {
  HEAT,
  COOL,
};

enum debug_level {
  DISABLE = 0,
  ERROR,
  WARN,
  INFO,
  DEBUG,
};

struct output_data {
  char  power;      /* % power (0-255) */
  enum power_state  state;
  time_t lasttime;
};

struct temp_data {
  int family_id;
  long long serial_no;
  int raw_reading;
  int lowpass_reading;
};

struct setting_data {
  enum ctrlr_mode mode;  /* HEAT or COOL */
  enum  ctrlr_type type; /* PID, ON_OFF or COMPRESSOR */
  int setpoint;
  int min_compressor_time; /* minimum time to leave a compressor type on/off */
  int range;             /* temp range around setpoint (ignored for PID) */
};

struct node_data {
  int id;
  char name[MAX_NAME_LEN];
  struct setting_data setting;
  struct temp_data temp;
  struct output_data output;
};

struct profile_data {
  int id;
  char name[MAX_NAME_LEN];
};


struct system_data {
  pthread_mutex_t lock;
  struct sqlite3 *sqlite_db;
  struct MHD_Daemon *httpd;
  struct profile_data profile;
  struct node_data nodes[NUM_NODES];
};

extern int global_debug;

#define DEBUG(format, args...) if (global_debug >= DEBUG){ fprintf (stdout, "%s(): " format , __func__, ##args); }
#define DEBUG_DUMP(format, args...) if (global_debug >= DEBUG){ fprintf (stdout, format, ##args); }
#define INFO(format, args...)  if (global_debug >= INFO){  fprintf (stdout, "%s(): " format , __func__, ##args); }
#define INFO_DUMP(format, args...)  if (global_debug >= INFO){  fprintf (stdout, format, ##args); }
#define WARN(format, args...)  if (global_debug >= WARN){  fprintf (stdout, "%s(): " format , __func__, ##args); }
#define WARN_DUMP(format, args...)  if (global_debug >= WARN){  fprintf (stdout, format, ##args); }
#define ERROR(format, args...) if (global_debug >= ERROR){ fprintf (stderr, "%s(): " format , __func__, ##args); }
#define ERROR_DUMP(format, args...) if (global_debug >= ERROR){ fprintf (stderr, format, ##args); }
