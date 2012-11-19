#include <stdio.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <bcm2835.h>

#include "ctrlr.h"

#define CLOCKID CLOCK_REALTIME
#define SIG SIGUSR1

enum power_state power_is_on(struct node_data *data)
{
  return data->output.state;
}

static void set_output(struct node_data *data, enum power_state state){
    DEBUG("%s(%d, %d)\n", __func__, data->output.gpio, (uint8_t)state);
    bcm2835_gpio_write(data->output.gpio, (uint8_t) state);
    data->output.state = state;
}

static void timer_handler(int sig, siginfo_t *si, void *uc) {
    struct node_data *data = si->si_value.sival_ptr;
    struct timeval  tv;
    static long lasttime;
    static long cumulative;
    long newtime;
    char percent;
    int i;

    gettimeofday(&tv, NULL);
    newtime = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

    /* calculate how far into the period we are */
    cumulative += (newtime - lasttime);
    lasttime = newtime;
    if (cumulative >= PWM_PERIOD)
        cumulative = 0;
    percent = 100 * cumulative / PWM_PERIOD;

    DEBUG("Caught signal %d from timer.  cumulative=%ld percent=%d\n", sig, cumulative, percent);
    for (i=0; i< NUM_NODES; i++){
        if (data[i].output.power > percent && ! data[i].output.state)
            set_output(&data[i], ON);
        else if (data[i].output.power < percent && data[i].output.state)
            set_output(&data[i], OFF);
    }
}


void *do_pwm_out(struct node_data *data){
    struct sigevent sev;
    struct itimerspec its;
    struct sigaction sa;
    timer_t timerid;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIG, &sa, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = data;
    timer_create(CLOCKID, &sev, &timerid);

    its.it_value.tv_sec = (PWM_PERIOD / PWM_UPDATES) / 1000;
    its.it_value.tv_nsec = ((PWM_PERIOD / PWM_UPDATES) % 1000) * 1000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    INFO("Starting pwm control timer for every %f seconds\n",
           (double)its.it_value.tv_sec + its.it_value.tv_nsec/1000000000.0);
    timer_settime(timerid, 0, &its, NULL);
    while(1);
    pthread_exit(0); 
}
