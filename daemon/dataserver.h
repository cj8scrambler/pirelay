#ifndef DATASERVER_H
#define DATASERVER_H

#define MAX_URL_LEN      256
#define MAX_NODENAME_LEN  16

int dataserver_start(struct system_data *sysdata);
int dataserver_stop(struct system_data *sysdata);

#endif
