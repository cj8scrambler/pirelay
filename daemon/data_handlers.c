#include <string.h>

#include "ctrlr.h"
#include "data_handlers.h"

#define PAGE_BUFFER_SIZE  4096
static char pagebuff[PAGE_BUFFER_SIZE];

#define MAX_STR_SIZE 16

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

static char *str_type(enum ctrlr_type type)
{
  static char string[MAX_STR_SIZE];

  switch (type)
  {
    case PID:
      strncpy(string, "PID", MAX_STR_SIZE);
      break;
    case ON_OFF:
      strncpy(string, "ON_OFF", MAX_STR_SIZE);
      break;
    case COMPRESSOR:
      strncpy(string, "COMPRESSOR", MAX_STR_SIZE);
      break;
    case DISABLED:
      strncpy(string, "DISABLED", MAX_STR_SIZE);
      break;
    default:
      strncpy(string, "UNKOWN", MAX_STR_SIZE);
      break;
  }

  return string;
};

static char *str_mode(enum ctrlr_mode mode)
{
  static char string[MAX_STR_SIZE];

  switch (mode)
  {
    case HEAT:
      strncpy(string, "HEAT", MAX_STR_SIZE);
      break;
    case COOL:
      strncpy(string, "COOL", MAX_STR_SIZE);
      break;
    default:
      strncpy(string, "UNKOWN", MAX_STR_SIZE);
      break;
  }

  return string;
};

int present_error(struct MHD_Connection *connection, char * format, ...)
{
  va_list args;
  struct MHD_Response *response;
  int ret;

  DEBUG("\n");
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

  DEBUG("\n");
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response (response);

  return ret;
}

static int print_output_json(char *buf, struct output_data *output)
{
  int ret=0;

  if (!buf) {
    ERROR("Received NULL buffer\n");
    return -1;
  }

  /* Make sure there are at lease 128 bytes left in buffer */
  if (strlen(buf) >= PAGE_BUFFER_SIZE - 128) {
    ERROR("Buffer running out of room\n");
    return -1;
  }

  strcat(buf, "    {\n");
  sprintf(&buf[strlen(buf)], "      \"state\":%d,\n", output->state);
  sprintf(&buf[strlen(buf)], "      \"power\":%d,\n", output->power);
  sprintf(&buf[strlen(buf)], "      \"lasttime\":%ld\n", output->lasttime);
  strcat(buf, "    }");

  return ret;
}

static int print_therm_json(char *buf, struct temp_data *temp)
{
  int ret=0;

  if (!buf) {
    ERROR("Received NULL buffer\n");
    return -1;
  }

  /* Make sure there are at lease 128 bytes left in buffer */
  if (strlen(buf) >= PAGE_BUFFER_SIZE - 128) {
    ERROR("Buffer running out of room\n");
    return -1;
  }

  strcat(buf, "    {\n");
  sprintf(&buf[strlen(buf)], "      \"family\":\"0x%02x\",\n", temp->family_id);
  sprintf(&buf[strlen(buf)], "      \"serial\":\"0x%012llx\",\n", temp->serial_no);
  sprintf(&buf[strlen(buf)], "      \"raw_temp\":%.1f,\n", TEMP_C(temp->raw_reading));
  sprintf(&buf[strlen(buf)], "      \"lowpass_temp\":%.1f\n", TEMP_C(temp->lowpass_reading));
  strcat(buf, "    }");

  return ret;
}

static int print_setting_json(char *buf, struct setting_data *setting)
{
  int ret=0;

  if (!buf) {
    ERROR("Received NULL buffer\n");
    return -1;
  }

  /* Make sure there are at lease 128 bytes left in buffer */
  if (strlen(buf) >= PAGE_BUFFER_SIZE - 128) {
    ERROR("Buffer running out of room\n");
    return -1;
  }

  strcat(buf, "    {\n");
  sprintf(&buf[strlen(buf)], "      \"type\":\"%s\",\n", str_type(setting->type));
  sprintf(&buf[strlen(buf)], "      \"mode\":\"%s\",\n", str_mode(setting->mode));
  sprintf(&buf[strlen(buf)], "      \"setpoint\":%.1f,\n", TEMP_C(setting->setpoint));
  sprintf(&buf[strlen(buf)], "      \"comp_time\":%d,\n", setting->min_compressor_time);
  sprintf(&buf[strlen(buf)], "      \"range\":%.1f\n", TEMP_C(setting->range));
  strcat(buf, "    }");

  return ret;
}

static int print_node_json(char *buf, struct node_data *node)
{
  int ret=0;

  if (!buf) {
    ERROR("Received NULL buffer\n");
    return -2;
  }

  /* Make sure there are at lease 128 bytes left in buffer */
  if (strlen(buf) >= PAGE_BUFFER_SIZE - 128) {
    ERROR("Buffer running out of room\n");
    return -3;
  }

  strcat(buf, "  {\n");
  sprintf(&buf[strlen(buf)], "    \"id\":%d,\n", node->id);
  sprintf(&buf[strlen(buf)], "    \"name\":\"%s\",\n", node->name);
  strcat (buf, "    \"setting\" :\n");
  ret = print_setting_json(buf, &node->setting);
  if (ret) {
    ERROR("Error generating setting json\n");
    return -1;
  }
  strcat (buf, ",\n");
  strcat (buf, "    \"therm\" :\n");
  ret = print_therm_json(buf, &node->temp);
  if (ret) {
    ERROR("Error generating therm json\n");
    return -1;
  }
  strcat (buf, ",\n");
  strcat (buf, "    \"output\" :\n");
  ret = print_output_json(buf, &node->output);
  if (ret) {
    ERROR("Error generating output json\n");
    return -1;
  }
  strcat (buf, "\n");

  strcat(buf, "  }");

  return ret;
}

int list_all_nodes(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  struct MHD_Response *response;
  int i, ret;

  DEBUG("\n");
  strcpy (pagebuff, "{\n");
  strcat (pagebuff, "  \"node\" : [\n");

  for (i=0; i < NUM_NODES; i++) {
    ret = print_node_json(pagebuff, &sysdata->nodes[i]);
    sprintf(&pagebuff[strlen(pagebuff)], "%c\n", i==NUM_NODES-1?' ':',');
    if (ret)
      return present_error(connection, "Error generating node #: %d\n", i+1);
  }
  strcat (pagebuff, "  ]\n}\n");
    
  response = MHD_create_response_from_data (strlen(pagebuff), (void *) pagebuff,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int list_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  struct MHD_Response *response;
  int ret;

  DEBUG("\n");
  if ((uri_num >=1) && (uri_num <= NUM_NODES)) {
    strcpy (pagebuff, "{\n");
    strcat (pagebuff, "  \"node\" :\n");
    ret = print_node_json(pagebuff, &sysdata->nodes[uri_num-1]);
    if (ret)
      return present_error(connection, "Error generating node %d\n", uri_num);
    strcat (pagebuff, "\n}\n");

    response = MHD_create_response_from_data (strlen(pagebuff), (void *) pagebuff,
                                            MHD_NO, MHD_NO);
    
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
  }
  return present_error(connection, "Invalid node #: %d\n", uri_num);
}

int create_a_node(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>Create a node</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int list_a_setting(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  struct MHD_Response *response;
  int ret;

  DEBUG("\n");
  if ((uri_num >=1) && (uri_num <= NUM_NODES)) {
    strcpy (pagebuff, "{\n");
    strcat (pagebuff, "  \"setting\" :\n");
    ret = print_setting_json(pagebuff, &(sysdata->nodes[uri_num-1].setting));
    strcat (pagebuff, "\n}\n");

    response = MHD_create_response_from_data (strlen(pagebuff), (void *) pagebuff,
                                            MHD_NO, MHD_NO);
    
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
  }
  return present_error(connection, "Invalid node #: %d\n", uri_num);
}

int update_a_setting(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>List a setting</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("\n");
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int list_an_output(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  struct MHD_Response *response;
  int ret;

  DEBUG("\n");
  if ((uri_num >=1) && (uri_num <= NUM_NODES)) {
    strcpy (pagebuff, "{\n");
    strcat (pagebuff, "  \"output\" :\n");
    ret = print_output_json(pagebuff, &(sysdata->nodes[uri_num-1].output));
    strcat (pagebuff, "\n}\n");

    response = MHD_create_response_from_data (strlen(pagebuff), (void *) pagebuff,
                                            MHD_NO, MHD_NO);
    
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
  }
  return present_error(connection, "Invalid node #: %d\n", uri_num);
}


int list_a_therm(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  struct MHD_Response *response;
  int ret;

  DEBUG("\n");
  if ((uri_num >=1) && (uri_num <= NUM_NODES)) {
    strcpy (pagebuff, "{\n");
    strcat (pagebuff, "  \"therm\" :\n");
    ret = print_therm_json(pagebuff, &(sysdata->nodes[uri_num-1].temp));
    strcat (pagebuff, "\n}\n");

    response = MHD_create_response_from_data (strlen(pagebuff), (void *) pagebuff,
                                            MHD_NO, MHD_NO);
    
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
  }
  return present_error(connection, "Invalid node #: %d\n", uri_num);
}


int update_a_therm(struct MHD_Connection *connection, struct system_data *sysdata, int uri_num)
{
  const char *page = "<html><body>update a therm</body></html>";
  struct MHD_Response *response;
  int ret;

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
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

  DEBUG("\n");
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}
