#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ctrlr.h"

#define MAX_QUERY_LEN  512
#define TIMEOUT       1000
#define MODE_755      (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)

void log_timer_handler(struct system_data *sysdata) {
  int i;
  char query[MAX_QUERY_LEN];
  sqlite3_stmt *stmt;

  for (i=0; i< NUM_NODES; i++){
    /* send SQL query */
    snprintf(query, MAX_QUERY_LEN,
      "INSERT INTO logdata (time, profile_id, node, set_temp, actual_temp, power) "
      "VALUES(datetime('now'), %d, %d, %.2f, %.2f, %d)", sysdata->profile.id, i, 
      TEMP_C(sysdata->nodes[i].setting.setpoint),
      TEMP_C(sysdata->nodes[i].temp.lowpass_reading),
      sysdata->nodes[i].output.power);
      DEBUG(" %s\n", query);
    if (sqlite3_prepare_v2(sysdata->sqlite_db, query, -1, &stmt, NULL)) {
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE)
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
  }

finalize:
  sqlite3_finalize(stmt);
}

static int log_set_profile_id(struct system_data *sysdata)
{
  char query[MAX_NAME_LEN + MAX_QUERY_LEN];
  sqlite3_stmt *stmt;
  int ret=-1;

  if ( !sysdata->sqlite_db) {
    ERROR("Database not open\n");
    goto ret;
  }

  snprintf(query, MAX_NAME_LEN + MAX_QUERY_LEN,
    "SELECT id FROM profile WHERE name ='%s'", sysdata->profile.name);
  if (sqlite3_prepare_v2(sysdata->sqlite_db, query, -1, &stmt, NULL)) {
    ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
    goto finalize;
  }

  ret = sqlite3_step(stmt);
  if (ret == SQLITE_DONE) {
    /* Name doesn't exist, need to add it */
    sqlite3_finalize(stmt);
    snprintf(query, MAX_NAME_LEN + MAX_QUERY_LEN,
      "INSERT INTO profile (name) VALUES('%s')", sysdata->profile.name);
    if (sqlite3_prepare_v2(sysdata->sqlite_db, query, -1, &stmt, NULL)) {
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }

    sqlite3_finalize(stmt);
    snprintf(query, MAX_NAME_LEN + MAX_QUERY_LEN,
      "SELECT id FROM profile WHERE name='%s';", sysdata->profile.name);
    if (sqlite3_prepare_v2(sysdata->sqlite_db, query, -1, &stmt, NULL)) {
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW) {
      ERROR("Could not find newly added profile (%d: %s)\n", ret, sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }
    INFO("Added new profile %s\n", sysdata->profile.name);
  }

  if(ret == SQLITE_ROW) {
    sysdata->profile.id = sqlite3_column_int(stmt, 0);
    INFO("Set profile id for '%s' to: %d\n", sysdata->profile.name, sysdata->profile.id);
    ret=0;
    goto finalize;
  }

finalize:
  sqlite3_finalize(stmt);
ret:
  return (ret);
}

static int does_table_exist(struct sqlite3 *db, const char *tablename)
{
  sqlite3_stmt *stmt;
  int ret=0;
  char query[MAX_QUERY_LEN];

  sprintf( query, "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';", tablename);
  if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL)) {
    ERROR("%s\n", sqlite3_errmsg(db));
    goto finalize;
  }

  if(SQLITE_ROW == sqlite3_step(stmt))
    ret=1;
  else
    INFO("Table '%s' does not exist\n", tablename);

finalize:
  sqlite3_finalize(stmt);
  return (ret);
}

int log_init(struct system_data *sysdata)
{
  sqlite3_stmt *stmt;
  int ret=-1;

  char logdir[128];
  char *cptr;
  struct stat checkstat;

  /* Make sure DB parent directory exists and is world r/w/x */
  strcpy(logdir, SQLITE_DBPATH);
  cptr = strrchr(logdir, '/');
  if (!cptr) {
    ERROR("Error: Invalid DB pathname: %s\n", SQLITE_DBPATH);
    goto ret;
  }  
  *cptr='\0';
  if (stat(logdir, &checkstat)) {
    if (mkdir(logdir, MODE_755)) {
      ERROR("Error: Could not create database directory (%s): %s\n",
            logdir, strerror(errno));
      goto ret;
    }
    INFO("Created database directory %s\n", logdir);
    stat(logdir, &checkstat);
  }
  if (!S_ISDIR(checkstat.st_mode)){
    ERROR("Error: DB directory (%s) is not a valid directory\n", logdir);
    goto ret;
  }
  if (!(checkstat.st_mode & (S_IROTH|S_IWOTH|S_IXOTH))) {
    if (chmod(logdir, (S_IROTH|S_IXOTH))) {
      ERROR("Error: could not make database directory (%s) accessible: %s\n",
            logdir, strerror(errno));
      goto ret;
    }
  }

  if (sqlite3_open_v2(SQLITE_DBPATH, &(sysdata->sqlite_db),
                 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) { 
    ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
    goto ret;
  }

  /* Make sure DB is world readable */
  if (stat(SQLITE_DBPATH, &checkstat)) {
    ERROR("Error: could not find database file: %s\n", SQLITE_DBPATH);
    goto ret;
  }
  if (!(checkstat.st_mode & S_IROTH)) {
    if (chmod(SQLITE_DBPATH, S_IROTH)) {
      ERROR("Error: could not make database world readable\n");
      goto ret;
    }
    INFO("Updated database permissions to world readable\n");
  }

  sqlite3_busy_timeout(sysdata->sqlite_db, TIMEOUT);
  if (!does_table_exist(sysdata->sqlite_db, "profile")) {
    if (sqlite3_prepare_v2(sysdata->sqlite_db,
          "CREATE TABLE profile (id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "name TEXT);", -1, &stmt, NULL)) {
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }
    INFO("Created 'profile' table\n");
  }

  if (!does_table_exist(sysdata->sqlite_db, "logdata")) {
    if (sqlite3_prepare_v2(sysdata->sqlite_db,
          "CREATE TABLE logdata (id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "time INTEGER, profile_id INTEGER, "
          "node INTEGER, set_temp REAL, actual_temp REAL, power INTEGER, "
          "event TEXT);", -1, &stmt, NULL)) {
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ERROR("%s\n", sqlite3_errmsg(sysdata->sqlite_db));
      goto finalize;
    }
    INFO("Created 'logdata' table\n");
  }

strcpy(sysdata->profile.name, "logset 3"); /* temporarily hardcode */
  return (log_set_profile_id(sysdata));

finalize:
  sqlite3_finalize(stmt);
ret:
  return (ret);
}

void log_cleanup(struct system_data *sysdata) {
  if (sysdata->sqlite_db)
    sqlite3_close(sysdata->sqlite_db);
}
