#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "ctrlr.h"
#include "data_handlers.h"

/* 
 * Parse a URL to find the next level node
 *
 *   value parsed is returned in root
 *   return:
 *      0       No more nodes found
 *    positive  new index into URL string
 *    negative  error
 */ 
int parse_url_node(const char *url, char *root, int start) {
  int i=start,j=0;

  while ((url[i]=='/') && (i < MAX_URL_LEN))
    i++;

  while (url[i] != '/' && url[i] != '\0' && i < MAX_URL_LEN)
    root[j++] = url[i++];

  root[j]='\0';

  if (i==MAX_URL_LEN)
    return -1;

  if (j == 0)
    return 0;

  return i;
}

static int
answer_to_connection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
  struct system_data *sysdata = cls;
  char uri_node1[MAX_URL_LEN] = {0};
  char uri_node2[MAX_URL_LEN] = {0};
  char handler_name[MAX_URL_LEN]={0};
  int i, pos, uri_num1=0;

  pos = parse_url_node(url, uri_node1, 0);
  if (pos <=0)
    return present_error(connection, "Could not parse first element of URL: %s\n", url);

  pos = parse_url_node(url, uri_node2, pos);
  if (pos < 0)
    return present_error(connection, "Could not parse second element of URL: %s\n", url);

  if (pos) {
    if (isdigit(uri_node2[0])) {
      uri_num1=atoi(uri_node2);
      uri_node2[0]='\0';
      /* 2nd element was a number, get next node */
      pos = parse_url_node(url, uri_node2, pos);
      if (pos < 0)
        return present_error(connection, "Could not parse third element of URL: %s\n", url);
    }
  }
    
  strcat(handler_name, uri_node1);
  if (uri_num1)
    strcat(handler_name, "/#");
  if (strlen(uri_node2)) {
    strcat(handler_name, "/");
    strcat(handler_name, uri_node2);
  }
  DEBUG("looking for handler: %s\n", handler_name);

  for(i=0; i<sizeof_handler_table(); i++) {
    if (!strcmp(handler_table[i].name, handler_name)) {
      DEBUG("Found match on entry %d: %s\n", i, handler_table[i].name);
      if (!strcmp(method, "GET")) {
        if ( handler_table[i].get_handler )
          return handler_table[i].get_handler(connection, sysdata, uri_num1);
        else
          return unsupported_handler(connection);
      } else if (!strcmp(method, "POST")) {
        if ( handler_table[i].post_handler )
          return handler_table[i].post_handler(connection, sysdata, uri_num1);
        else
          return unsupported_handler(connection);
      } else if (!strcmp(method, "PUT")) {
        if ( handler_table[i].put_handler )
          return handler_table[i].put_handler(connection, sysdata, uri_num1);
        else
          return unsupported_handler(connection);
      } else if (!strcmp(method, "DELETE")) {
        if ( handler_table[i].del_handler )
          return handler_table[i].del_handler(connection, sysdata, uri_num1);
        else
          return unsupported_handler(connection);
      } else {
        return unsupported_handler(connection);
      }
      break;
    }
  }

  if (i==sizeof_handler_table())
    return unsupported_handler(connection);

  /* Should never reach here */
  return MHD_NO;
}

static int check_access (void *cls, const struct sockaddr *addr, socklen_t addrlen)
{
  /* comment out to avoid compiler warning of unused var:
  struct system_data *sysdata = cls;
   */

  /* TODO: restrict access to localhost by default, and only allow all
           access in a development mode
   */
  return MHD_YES;
}

int dataserver_start(struct system_data *sysdata) {
  sysdata->httpd = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, HTTP_PORT, check_access, sysdata,
                             &answer_to_connection, sysdata, MHD_OPTION_END);
  return (sysdata->httpd)?0:1;
}

int dataserver_stop(struct system_data *sysdata) {
  MHD_stop_daemon( sysdata->httpd);
  return 0;
}
