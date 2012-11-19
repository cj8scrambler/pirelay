#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h> 
#include <unistd.h>
#include <pthread.h> 

#include "ctrlr.h"

#define TEMP_F(c) (c * 0.0018 + 32.0)
#define TEMP_C(c) (c / 1000.0)

#define W1_DEVICE_DIR "/sys/bus/w1/devices"
#define MAX_SENSOR_FILENAME_LEN  256

static int read_sensor(struct node_data *node);
static int temp_sensor_init(struct node_data *nodes);
char *get_last_word(char *line);

void *do_temp_read(struct node_data *data) {

  int i;

  temp_sensor_init(data);

  for (i=0; i< 100; i++) {
    read_sensor(&data[0]);
    WARN("%s() %s: %.3fF / %.3fC\n", __func__, data[0].name, TEMP_F(data[0].temp.last_reading), TEMP_C(data[0].temp.last_reading));
    usleep(1000000); /* 1 sec */
  }
  pthread_exit(0); 
}

int read_sensor(struct node_data *node) {

  char sensor_filename[MAX_SENSOR_FILENAME_LEN];
  char oneline[MAX_SENSOR_FILENAME_LEN];
  char *temp;
  int ret = -1;
  FILE *fd;

  DEBUG("Reading temp for %s\n", node->name);
  if (node && node->temp.family_id && node->temp.serial_no) {
    sprintf(sensor_filename, "%s/%x-%012llx/w1_slave", W1_DEVICE_DIR,
            node->temp.family_id, node->temp.serial_no);
    fd = fopen(sensor_filename, "r");
    if (fd) {
      if (fgets(oneline, MAX_SENSOR_FILENAME_LEN, fd) != NULL) {
        if (!strcmp(get_last_word(oneline), "YES")) {
          if (fgets(oneline, MAX_SENSOR_FILENAME_LEN, fd) != NULL) {
            temp = get_last_word(oneline);
            temp++;
            temp++;
            node->temp.last_reading = atoi(temp);
          } else {
            ret = -1;
            WARN("Unable to read 2nd line of temp sensor for %s\n", node->name);
          }
        } else {
          ret = -1;
          WARN("got bad CRC\n");
        }
      } else
        WARN("Unable to read sensor for %s\n", node->name);
      fclose(fd);
    } else
      ERROR("%s temp sensor configured but not found: %s\n", node->name,
            strerror(errno));
  } else 
      WARN("No sensor configured for %s\n", node->name);
  return ret;
}

char *get_last_word(char *line) {

  char *token, *last_token = NULL;

  if (line) {
    for (token = strtok(line, " "); token != NULL; token = strtok(NULL, " "))
      last_token = token;

    if (last_token[strlen(last_token)-1] == '\n')
      last_token[strlen(last_token)-1] = '\0';
  }
  return last_token;
}

int temp_sensor_init(struct node_data *nodes) {

  return 0;
}
