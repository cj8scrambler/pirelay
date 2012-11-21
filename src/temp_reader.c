#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <limits.h> 
#include <unistd.h>
#include <pthread.h> 
#include <values.h>

#include "ctrlr.h"

#define TEMP_F(c) (c * 0.0018 + 32.0)
#define TEMP_C(c) (c / 1000.0)

#define W1_DEVICE_DIR "/sys/bus/w1/devices"
#define MAX_SENSOR_FILENAME_LEN  64
#define MAX_LINE_LEN  160

static int read_sensor(struct node_data *node);
static int temp_sensor_init(struct node_data *nodes);
char *get_last_word(char *line);
float convert_temp(short reading);

static void timer_handler(int sig, siginfo_t *si, void *uc) {
    struct system_data *sysdata = si->si_value.sival_ptr;
    int i;

    for (i=0; i< NUM_NODES; i++){
      struct node_data *node = &sysdata->nodes[i];
      read_sensor(node);
      WARN("%s: 0x%x / %.3fC\n", node->name, node->temp.last_reading, convert_temp(node->temp.last_reading));
    }
}

void *do_temp_read(struct system_data *sysdata) {

  struct sigevent sev;
  struct itimerspec its;
  struct sigaction sa;
  timer_t timerid;

  temp_sensor_init(sysdata->nodes);

  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = timer_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR2, &sa, NULL);

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGUSR2;
  sev.sigev_value.sival_ptr = sysdata;
  timer_create(CLOCK_REALTIME, &sev, &timerid);

  its.it_value.tv_sec = TEMP_SAMPLE_RATE / 1000;
  its.it_value.tv_nsec = (TEMP_SAMPLE_RATE % 1000) * 1000000;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;

  INFO("Starting temp read timer for every %f seconds\n",
         (double)its.it_value.tv_sec + its.it_value.tv_nsec/1000000000.0);
  timer_settime(timerid, 0, &its, NULL);
  while(1);
  pthread_exit(0);
}


int read_sensor(struct node_data *node) {

  char sensor_filename[MAX_SENSOR_FILENAME_LEN];
  char oneline[MAX_LINE_LEN];
  char *temp;
  short value;
  int ret = -EINVAL;
  FILE *fd;

  DEBUG("Reading temp for %s\n", node->name);
  if (node && node->temp.family_id && node->temp.serial_no) {
    sprintf(sensor_filename, "%s/%x-%012llx/w1_slave", W1_DEVICE_DIR,
            node->temp.family_id, node->temp.serial_no);
    fd = fopen(sensor_filename, "r");
    if (fd) {
      if (fgets(oneline, MAX_LINE_LEN, fd) != NULL) {
        if (!strcmp(get_last_word(oneline), "YES")) {
          if (fgets(oneline, MAX_LINE_LEN, fd) != NULL) {
            temp = get_last_word(oneline);
            /* drop the leading 't=' */
            temp++;
            temp++;
            value = atoi(temp);
            ret = 0;
          } else {
            ret = -EFAULT;
            WARN("Unable to read 2nd line of temp sensor for %s\n", node->name);
          }
        } else {
          ret = -EIO;
          WARN("got bad CRC\n");
        }
      } else {
        ret = -EFAULT;
        WARN("Unable to read sensor for %s\n", node->name);
      }
      fclose(fd);
    } else {
      ret = -EFAULT;
      ERROR("%s temp sensor configured but not found: %s\n", node->name,
            strerror(errno));
    }
  } else {
      ret = -ENODEV;
      WARN("No sensor configured for %s\n", node->name);
  }

  if (ret == 0)
{
    node->temp.last_reading = value;
WARN("good: set reading to %d\n", node->temp.last_reading);
}
  else if (node->setting.mode == HEAT)
{
    node->temp.last_reading = MAXSHORT;
WARN("bad heat: set reading to %d\n", node->temp.last_reading);
}
  else if (node->setting.mode == COOL)
{
    node->temp.last_reading = MINSHORT;
WARN("bad cool: set reading to %d\n", node->temp.last_reading);
}
  return ret;
}

char *get_last_word(char *line) {

  char *token, *last_token = NULL;

  if (line) {
    for (token = strtok(line, " "); token != NULL; token = strtok(NULL, " "))
      last_token = token;

    /* strip trailing newline char */
    if (last_token[strlen(last_token)-1] == '\n')
      last_token[strlen(last_token)-1] = '\0';
  }
  return last_token;
}

int temp_sensor_init(struct node_data *nodes) {

  return 0;
}

float convert_temp(short reading) {
  float val = (reading >> 5) + (reading & 0x1F)/ 37.0;
  printf ("0x%x  int: 0x%x  dec: 0x%x = %f\n", reading, reading >> 5, reading & 0x1F, val);
  return val;
}
