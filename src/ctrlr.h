#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sqlite3.h>
#include "config.h"

#define MAX_NAME_LEN 24
#define NUM_TEMP_SAMPLES 10

#define TEMP_F(c) (c * 0.0018 + 32.0)
#define TEMP_C(c) (c / 1000.0)

enum power_state {
  OFF = 0,
  ON = 1,
};

enum ctrlr_type {
  PID,
  ON_OFF,
  COMPRESSOR, /* ON_OFF plus minimum ON/OFF times */
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
  int   gpio;
  char  power;      /* % power * 100 */
  int  state;
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
  int range;             /* Ignored for PID type */
};

struct node_data {
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
  struct profile_data profile;
  struct node_data nodes[NUM_NODES];
};

int pid(int val);
enum power_state power_is_on(struct node_data *data);

extern int global_debug;

#define DEBUG(format, args...) if (global_debug >= DEBUG){ fprintf (stdout, "%s(): " format , __func__, ##args); }
#define INFO(format, args...)  if (global_debug >= INFO){  fprintf (stdout, "%s(): " format , __func__, ##args); }
#define WARN(format, args...)  if (global_debug >= WARN){  fprintf (stdout, "%s(): " format , __func__, ##args); }
#define ERROR(format, args...) if (global_debug >= ERROR){ fprintf (stderr, "%s(): " format , __func__, ##args); }
