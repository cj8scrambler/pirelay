#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "ctrlr.h"

static int
answer_to_connection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
  const char *page = "<html><body>Hello, browser!</body></html>";
  struct MHD_Response *response;
  int ret;
  
  response = MHD_create_response_from_data (strlen(page), (void *) page,
                                            MHD_NO, MHD_NO);

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int do_data_server(struct system_data *sysdata) {
  sysdata->httpd = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, HTTP_PORT, NULL, NULL,
                             &answer_to_connection, NULL, MHD_OPTION_END);
  return (sysdata->httpd)?0:1;
}
