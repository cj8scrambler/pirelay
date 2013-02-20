#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h> 
#include <unistd.h>
#include <values.h>
#include <ctype.h>

#include "ctrlr.h"

#define W1_DEVICE_DIR "/sys/bus/w1/devices"
#define MAX_SENSOR_FILENAME_LEN  64
#define MAX_LINE_LEN  160

static int read_sensor(struct node_data *node);
char *get_last_word(char *line);

void temp_timer_handler(struct system_data *sysdata) {
  int i;

  for (i=0; i< NUM_NODES; i++){
    struct node_data *node = &sysdata->nodes[i];
    if (node && node->temp.family_id && node->temp.serial_no) {
      read_sensor(node);
      WARN("%s: raw: %.1fF  lowpass: %.1fF\n", node->name,
           TEMP_F(node->temp.raw_reading),
           TEMP_F(node->temp.lowpass_reading) );
    }
  }
}

int lowpass(struct node_data *node, int temp) {
  #define dt    (TEMP_SAMPLE_RATE / 1000.0)
  #define alpha .15

  if (node)
    node->temp.lowpass_reading = (alpha * temp) + (1.0 - alpha) *
                                 node->temp.lowpass_reading;

  return 0;
}

int read_sensor(struct node_data *node) {

  char sensor_filename[MAX_SENSOR_FILENAME_LEN];
  char oneline[MAX_LINE_LEN];
  char *temp;
  int value;
  int ret = -EINVAL;
  FILE *fd;

  DEBUG("Reading temp for %s\n", node->name);
  if (node && node->temp.family_id && node->temp.serial_no) {
    sprintf(sensor_filename, "%s/%x-%012llx/w1_slave", W1_DEVICE_DIR,
            node->temp.family_id, node->temp.serial_no);
    DEBUG("filename: %s\n", sensor_filename);
    fd = fopen(sensor_filename, "r");
    if (fd) {
      if (fgets(oneline, MAX_LINE_LEN, fd) != NULL) {
        DEBUG("%s", oneline);
        if (!strcmp(get_last_word(oneline), "YES")) {
          if (fgets(oneline, MAX_LINE_LEN, fd) != NULL) {
            DEBUG("%s", oneline);
            temp = get_last_word(oneline);
            /* drop the leading 't=' */
            temp++;
            temp++;
            value = atoi(temp);
            DEBUG("Got reading of %d\n", value);
            if (value != 85000) {
              ret = 0;
            } else
              ret = -EFAULT;
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
        WARN("Failed to read sessor for %s\n", node->name);
      }
      fclose(fd);
    } else {
      ret = -EFAULT;
      ERROR("%s temp sensor configured but not found.  %s: %s\n",
            node->name, sensor_filename, strerror(errno));
    }
  } else {
      ret = -ENODEV;
      INFO("No sensor configured for %s\n", node->name);
  }

  if (ret == 0) {
    node->temp.raw_reading = value;
    lowpass(node, value);
  } else if (node->setting.mode == HEAT)
    node->temp.raw_reading = MAXINT;
  else if (node->setting.mode == COOL)
    node->temp.raw_reading = MININT;

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
