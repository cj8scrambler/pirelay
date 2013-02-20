#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#include <bcm2835.h>

#include "ctrlr.h"
#include "log.h"
#include "control_output.h"
#include "temp_reader.h"
#include "dataserver.h"

int global_debug = INFO;
static int quit = 0;

static void cleanup(int signum) {

  extern struct system_data sysdata;
  ERROR("Shutting Down\n");

  quit = 1;
  MHD_stop_daemon (sysdata.httpd);
  log_cleanup(&sysdata);
  output_disable_all(sysdata.nodes);
  bcm2835_close();
}

static void handle_kill(int signum) {
  ERROR("Caught kill signal\n");
  cleanup(signum);
  exit(-1);
}

static void timer_handler(int sig, siginfo_t *si, void *uc) {
  struct system_data *sysdata = si->si_value.sival_ptr;

  struct timeval current;
  struct timeval delta = {0};
  static struct timeval next_output_time = {0};
  static int output_overflow = 0;
  static struct timeval next_temp_time = {0};
  static int temp_overflow = 0;
  static struct timeval next_log_time = {0};
  static int log_overflow = 0;

  gettimeofday(&current, NULL);

  /* Run the output control if it's been long enough */
  if (!timercmp(&current, &next_output_time, <)) {
    output_timer_handler(sysdata->nodes);

    /* check for overflow */
    timersub(&current, &next_output_time, &delta);
    if ((delta.tv_sec * 1000 + delta.tv_usec / 1000) > TIMER_BASE_PERIOD) {
      DEBUG("output timer overflow: %.3f secs (#%d)\n", 
            (delta.tv_sec + delta.tv_usec / 1000000.0), ++output_overflow);
    }
    delta.tv_sec = TIMER_BASE_PERIOD / 1000;
    delta.tv_usec = (TIMER_BASE_PERIOD * 1000) % 1000000;
    timeradd(&current, &delta, &next_output_time);
  }

  /* Read the temp sensors if it's been long enough */
  if (!timercmp(&current, &next_temp_time, <)) {
    temp_timer_handler(sysdata);

    /* check for overflow */
    timersub(&current, &next_temp_time, &delta);
    if ((delta.tv_sec * 1000 + delta.tv_usec / 1000) > TIMER_BASE_PERIOD) {
      DEBUG("temp reader timer overflow: %.3f secs (#%d)\n", 
            (delta.tv_sec + delta.tv_usec / 1000000.0), ++temp_overflow);
    }
    delta.tv_sec = TEMP_SAMPLE_RATE / 1000;
    delta.tv_usec = (TEMP_SAMPLE_RATE * 1000) % 1000000;
    timeradd(&current, &delta, &next_temp_time);
  }

  /* Log data if it's been long enough */
  if (!timercmp(&current, &next_log_time, <)) {
    log_timer_handler(sysdata);

    /* check for overflow */
    timersub(&current, &next_log_time, &delta);
    if ((delta.tv_sec * 1000 + delta.tv_usec / 1000) > TIMER_BASE_PERIOD) {
      DEBUG("log timer overflow: %.3f secs (#%d)\n", 
            (delta.tv_sec + delta.tv_usec / 1000000.0), ++log_overflow);
    }
    delta.tv_sec = LOG_RATE / 1000;
    delta.tv_usec = (LOG_RATE * 1000) % 1000000;
    timeradd(&current, &delta, &next_log_time);
  }

}

static int hw_init(struct node_data *nodes) {

  int i;

  if (!bcm2835_init()) {
    ERROR("Could not initialize bcm2835 library.\nTry running as root\n");
    return(-1);
  }

  /* Configure GPIOs for output */
  for (i=1; i<NUM_NODES; i++){
    bcm2835_gpio_write(nodes[i].output.gpio, nodes[i].output.state);
    bcm2835_gpio_fsel(nodes[i].output.gpio, BCM2835_GPIO_FSEL_OUTP);
  }

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

int main(int argc, char **argv){
  int i;

  extern struct system_data sysdata;

  struct sigevent sev;
  struct itimerspec its;
  struct sigaction sa;
  timer_t timerid;

  // Try to cleanup on kill signals
  signal(SIGHUP, handle_kill);
  signal(SIGINT, handle_kill);
  signal(SIGTERM, handle_kill);

ERROR("Start\n");
  if (hw_init(sysdata.nodes))
    cleanup(-1);
  if (log_init(&sysdata))
    cleanup(-1);

  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = timer_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR1, &sa, NULL);

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGUSR1;
  sev.sigev_value.sival_ptr = &sysdata;
  timer_create(CLOCK_REALTIME, &sev, &timerid);

  its.it_value.tv_sec = (TIMER_BASE_PERIOD) / 1000;
  its.it_value.tv_nsec = (TIMER_BASE_PERIOD % 1000) * 1000000;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;

  INFO("Starting base timer for every %f seconds\n",
         (double)its.it_value.tv_sec + its.it_value.tv_nsec/1000000000.0);
    timer_settime(timerid, 0, &its, NULL);

  if (do_data_server(&sysdata)) {
    ERROR("Could not start data server\n");
    cleanup(-1);
  }

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

  timer_delete(&timerid);
  cleanup(0);

  return 0;
}
