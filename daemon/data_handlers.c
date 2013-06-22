#include <string.h>

#include "ctrlr.h"
#include "data_handlers.h"

#define PAGE_BUFFER_SIZE 1024

char pagebuff[PAGE_BUFFER_SIZE];

const struct element_handler handler_table[] = {
/*  URI format        GET                POST              PUT               DELETE          */
  {"node",            list_all_nodes,    NULL,             NULL,             NULL             },
  {"node/#",          list_a_node,       create_a_node,    update_a_node,    remove_a_node    },
  {"node/#/setting",  list_a_setting,    NULL,             update_a_setting, NULL             },
  {"node/#/output",   list_an_output,    NULL,             NULL,             NULL             },
  {"node/#/therm",    list_a_therm,      NULL,             update_a_therm,   NULL             },
  {"profile",         list_all_profiles, NULL,             NULL,             NULL             },
  {"profile/#",       list_a_profile,    create_a_profile, update_a_profile, remove_a_profile },
  {"log",             list_all_logs,     NULL,             NULL,             NULL             },
  {"log/#",           list_a_log,        NULL,             NULL,             remove_a_log     },
  {"therm",           list_all_therms,   scan_therms,      NULL,             NULL             },
};

int sizeof_handler_table(void)
{
  return ARRAY_SIZE(handler_table);
}

int present_error(struct MHD_Connection *connection, char * format, ...)
{
  va_list args;
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  strcpy (pagebuff, "<html><body>");

  va_start (args, format);
  vsnprintf (&pagebuff[strlen(pagebuff)], PAGE_BUFFER_SIZE-strlen(pagebuff), format, args);

  strcat (pagebuff, "</body></html>");

  response = MHD_create_response_from_data (strlen(pagebuff), (void *)pagebuff, MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
  MHD_destroy_response (response);
  va_end(args);

  return ret;
}

int unsupported_handler(struct MHD_Connection *connection)
{
  const char *page = "<html><body>Unsupported request</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response (response);

  return ret;
}

int list_all_nodes(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Got list all nodes</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int list_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List a node</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int create_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Create a node</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int update_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Update a node</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int remove_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Remove a node</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int list_a_setting(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List a setting</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int update_a_setting(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List a setting</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int list_an_output(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List an output</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int list_a_therm(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List a therm</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int update_a_therm(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>update a therm</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int list_all_profiles(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List all profiles</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int list_a_profile(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List a profile</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int create_a_profile(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Create a profile</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int update_a_profile(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Update a profile</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int remove_a_profile(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Remove a profile</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int list_all_logs(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List all logs</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int list_a_log(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List a log</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int remove_a_log(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Remove a log</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int list_all_therms(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List all therms</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int scan_therms(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Scan therms</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("%s()\n", __func__);
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}
