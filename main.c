#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <bcm2835.h>

#include "ctrlr.h"

void hw_init(struct node_data *nodes);
static int get_trippoint(struct node_data *node, enum power_state state);
static void set_node_output(struct node_data *node, enum power_state state);

int global_debug = WARN;

int main(int argc, char **argv){
  pthread_t temp_read_thread, pwm_out_thread, data_server_thread;
  void *do_temp_read(), *do_pwm_out(), *do_data_server();
  int i, quit=0;

  extern struct node_data data[NUM_NODES];

  hw_init(data);

  pthread_create(&temp_read_thread, NULL, do_temp_read, &data);
  pthread_create(&pwm_out_thread, NULL, do_pwm_out, &data);
  pthread_create(&data_server_thread, NULL, do_data_server, &data);

  while (!quit) {
    for (i=1; i<NUM_NODES; i++){
      if (data[i].setting.type == PID){
//        data[i].output.power= pid(data[i].temp.average);
      }
      else if ((data[i].setting.type == ON_OFF) ||
               (data[i].setting.type == COMPRESSOR)) {
        if (data[i].setting.mode == HEAT) {
          if ((data[i].temp.average < get_trippoint(&data[i], ON)) &&
              (!power_is_on(&data[i])))
                set_node_output(&data[i], ON);
          if ((data[i].temp.average > get_trippoint(&data[i], OFF)) &&
              (power_is_on(&data[i])))
                set_node_output(&data[i], OFF);
        } else if (data[i].setting.mode == COOL) {
          if ((data[i].temp.average > get_trippoint(&data[i], ON)) &&
              (!power_is_on(&data[i])))
                set_node_output(&data[i], ON);
          if ((data[i].temp.average < get_trippoint(&data[i], OFF)) &&
              (power_is_on(&data[i])))
                set_node_output(&data[i], OFF);
        }
      }
    }
  }

  bcm2835_close();

  pthread_join(temp_read_thread,NULL);
  pthread_join(pwm_out_thread,NULL);
  pthread_join(data_server_thread,NULL);

  return 0;
}

int get_trippoint(struct node_data *node, enum power_state state)
{
  if (state == OFF)
    return (node->setting.setpoint);
  else if (state == ON) {
    if (node->setting.mode == HEAT)
      return (node->setting.setpoint - node->setting.range);
    else if (node->setting.mode == COOL)
      return (node->setting.setpoint + node->setting.range);
    else
      ERROR("%s has invalid mode: %d\n", node->name, node->setting.mode);
  } else
    ERROR("%s has invalid state: %d\n", node->name, node->setting.setpoint);

   return -1;
}

void set_node_output(struct node_data *node, enum power_state state)
{
  time_t deltatime;

  if (node->setting.type == COMPRESSOR) {
    deltatime = time(NULL) - node->output.lasttime;
    if (deltatime < node->setting.min_compressor_time)
      return;
  }

  if (state == ON)
    node->output.power = 100;
  else
    node->output.power = 0;

  node->output.lasttime = time(NULL);
}

void hw_init(struct node_data *nodes) {

  int i;

  if (!bcm2835_init()) {
    ERROR("Could not initialize bcm2835 library.\nTry running as root\n");
    exit(1);
  }

bcm2835_gpio_write(25,1);
  /* Configure GPIOs for output */
  for (i=1; i<NUM_NODES; i++){
    bcm2835_gpio_write(nodes[i].output.gpio, nodes[i].output.state);
    bcm2835_gpio_fsel(nodes[i].output.gpio, BCM2835_GPIO_FSEL_OUTP);
  }
bcm2835_gpio_write(25,0);

/* debug GPIO */
bcm2835_gpio_fsel(25, BCM2835_GPIO_FSEL_OUTP);
}
