#include <stdio.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <bcm2835.h>

#include "ctrlr.h"

enum power_state power_is_on(struct node_data *data)
{
  return data->output.state;
}

static void set_output(struct node_data *data, enum power_state state){
  DEBUG("%s(%d, %d)\n", __func__, data->output.gpio, (uint8_t)state);
  bcm2835_gpio_write(data->output.gpio, (uint8_t) state);
  data->output.state = state;
}

void output_timer_handler(struct node_data *data){
  struct timeval  tv;
  static unsigned long lasttime = 0;
  static unsigned long cumulative = 0;
  unsigned long newtime;
  char percent;
  int i;

  gettimeofday(&tv, NULL);
  newtime = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

  /* calculate how far into the period we are */
  cumulative += (newtime - lasttime);
  lasttime = newtime;
  if (cumulative >= (PWM_UPDATES * TIMER_BASE_PERIOD))
    cumulative = 0;
  percent = 100 * cumulative / (PWM_UPDATES * TIMER_BASE_PERIOD);

  DEBUG("Output timer newtime=%ld cumulative=%ld percent=%d\n", newtime, cumulative, percent);
  for (i=0; i< NUM_NODES; i++){
    if (data[i].output.power > percent && ! data[i].output.state)
      set_output(&data[i], ON);
    else if (data[i].output.power < percent && data[i].output.state)
      set_output(&data[i], OFF);
  }
}

void output_disable_all(struct node_data *data){
  int i;

  for (i=0; i< NUM_NODES; i++){
      set_output(&data[i], OFF);
  }
}
