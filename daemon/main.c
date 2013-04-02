#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <bcm2835.h>

#include "ctrlr.h"
#include "log.h"

void hw_init(struct node_data *nodes);
static int get_trippoint(struct node_data *node, enum power_state state);
static void set_node_output(struct node_data *node, enum power_state state);

int global_debug = INFO;

int main(int argc, char **argv){
  pthread_t temp_read_thread, pwm_out_thread, data_server_thread, log_thread;
  void *do_temp_read(), *do_pwm_out(), *do_data_server(), *do_log();
  int i, quit=0;

  extern struct system_data sysdata;

for (i=0; i<NUM_NODES; i++){
WARN("%s(): node-%d (%s): GPIO-%d power=%d (%s)\n", __func__, i+1, sysdata.nodes[i].name, sysdata.nodes[i].output.gpio, sysdata.nodes[i].output.power, sysdata.nodes[i].output.state?"ON":"OFF");
}

  hw_init(sysdata.nodes);

  pthread_create(&temp_read_thread, NULL, do_temp_read, &sysdata);
  pthread_create(&pwm_out_thread, NULL, do_pwm_out, &sysdata.nodes);
  pthread_create(&log_thread, NULL, do_log, &sysdata);
  pthread_create(&data_server_thread, NULL, do_data_server, &sysdata.nodes);

  while (!quit) {
    for (i=0; i<NUM_NODES; i++){

      if (sysdata.nodes[i].setting.type == PID){
//        sysdata.nodes[i].output.power= pid(sysdata.nodes[i].temp.average);
      }
      else if ((sysdata.nodes[i].setting.type == ON_OFF) ||
               (sysdata.nodes[i].setting.type == COMPRESSOR)) {
        if (sysdata.nodes[i].setting.mode == HEAT) {
          if ((sysdata.nodes[i].temp.lowpass_reading < get_trippoint(&sysdata.nodes[i], ON)) &&
              (!power_is_on(&sysdata.nodes[i])))
                set_node_output(&sysdata.nodes[i], ON);
          if ((sysdata.nodes[i].temp.lowpass_reading > get_trippoint(&sysdata.nodes[i], OFF)) &&
              (power_is_on(&sysdata.nodes[i])))
                set_node_output(&sysdata.nodes[i], OFF);
        } else if (sysdata.nodes[i].setting.mode == COOL) {
          if ((sysdata.nodes[i].temp.lowpass_reading > get_trippoint(&sysdata.nodes[i], ON)) &&
              (!power_is_on(&sysdata.nodes[i])))
                set_node_output(&sysdata.nodes[i], ON);
          if ((sysdata.nodes[i].temp.lowpass_reading < get_trippoint(&sysdata.nodes[i], OFF)) &&
              (power_is_on(&sysdata.nodes[i])))
                set_node_output(&sysdata.nodes[i], OFF);
        }
      }
    }
  }

  bcm2835_close();

  pthread_join(temp_read_thread,NULL);
  pthread_join(pwm_out_thread,NULL);
  pthread_join(log_thread,NULL);
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
