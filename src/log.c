#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <mysql/mysql.h>

#include "ctrlr.h"

static void timer_handler(int sig, siginfo_t *si, void *uc) {
  struct system_data *sysdata = si->si_value.sival_ptr;
  int i;
  MYSQL_RES *res;
  MYSQL_ROW row;

  for (i=0; i< NUM_NODES; i++){
    /* send SQL query */
    if (mysql_query(sysdata->data_conn, "show tables")) {
       fprintf(stderr, "%s\n", mysql_error(sysdata->data_conn));
//       return(-1);
    }

    res = mysql_use_result(sysdata->data_conn);
    /* output table name */
    printf("MySQL Tables in mysql database:\n");
    while ((row = mysql_fetch_row(res)) != NULL)
       printf("%s \n", row[0]);
    /* close connection */
    mysql_free_result(res);
  }
}

static int log_init(struct system_data *sysdata)
{

  printf("MySQL client version: %s\n", mysql_get_client_info());

  sysdata->data_conn = mysql_init(NULL);

  /* Connect to database */
  if (!mysql_real_connect(sysdata->data_conn, MYSQL_SERVER,
       MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, 0, NULL, 0)) {
    fprintf(stderr, "%s\n", mysql_error(sysdata->data_conn));
    return(-1);
  }

  return 0;
}

static void log_cleanup(struct system_data *sysdata) {
  mysql_close(sysdata->data_conn);
}

void *do_log(struct system_data *sysdata) {
  struct sigevent sev;
  struct itimerspec its;
  struct sigaction sa;
  timer_t timerid;

  if (log_init(sysdata))
    return(NULL);

  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = timer_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGALRM, &sa, NULL);

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGALRM;
  sev.sigev_value.sival_ptr = sysdata;
  timer_create(CLOCK_REALTIME, &sev, &timerid);

  its.it_value.tv_sec = LOG_RATE;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;

  INFO("Starting logger timer for every %d seconds\n", (int)its.it_value.tv_sec);
  timer_settime(timerid, 0, &its, NULL);
  while(1);
  log_cleanup(sysdata);
  pthread_exit(0);
}
