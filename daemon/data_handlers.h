#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include "dataserver.h"

struct element_handler {
  char name[MAX_URL_LEN];
  int (*get_handler)  (struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
  int (*post_handler)  (struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
  int (*put_handler)  (struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
  int (*del_handler)  (struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
};

extern const struct element_handler handler_table[];

int sizeof_handler_table(void);
int present_error(struct MHD_Connection *connection, char * format, ...);
int unsupported_handler(struct MHD_Connection *connection);

int list_all_nodes(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int create_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int update_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int remove_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_a_setting(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int update_a_setting(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_an_output(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_a_therm(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int update_a_therm(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_all_profiles(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_a_profile(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int create_a_profile(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int update_a_profile(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int remove_a_profile(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_all_logs(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_a_log(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int remove_a_log(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int list_all_therms(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);
int scan_therms(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num);

#endif
