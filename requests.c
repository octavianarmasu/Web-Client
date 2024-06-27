#include "requests.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "helpers.h"

char *compute_get_request(char *host, char *url, char *cookies, char *token) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char line[LINELEN];

  memset(message, 0, BUFLEN);
  memset(line, 0, LINELEN);

  sprintf(line, "GET %s HTTP/1.1", url);
  compute_message(message, line);

  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  if (cookies != NULL) {
    sprintf(line, "Cookie: %s", cookies);
    compute_message(message, line);
  }

  if (token != NULL) {
    sprintf(line, "Authorization: Bearer %s", token);
    compute_message(message, line);
  }

  compute_message(message, "");
  return message;
}

char *compute_post_request(char *host, char *url, char *payload,
                           char *body_data, char *cookies, char *token) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char line[LINELEN];

  memset(line, 0, LINELEN);

  sprintf(line, "POST %s HTTP/1.1", url);
  compute_message(message, line);

  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  sprintf(line, "Content-Type: %s", payload);
  compute_message(message, line);

  int len = strlen(body_data);
  sprintf(line, "Content-Length: %d", len);
  compute_message(message, line);

  if (cookies != NULL) {
    sprintf(line, "Cookie: %s", cookies);
    compute_message(message, line);
  }

  if (token != NULL) {
    sprintf(line, "Authorization: Bearer %s", token);
    compute_message(message, line);
  }

  compute_message(message, "");
  strcat(message, body_data);

  return message;
}

char *compute_delete_request(char *host, char *url, char *cookies,
                             char *token) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char line[LINELEN];

  memset(line, 0, LINELEN);
  sprintf(line, "DELETE %s HTTP/1.1", url);
  compute_message(message, line);

  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  if (cookies != NULL) {
    sprintf(line, "Cookie: %s", cookies);
    compute_message(message, line);
  }

  if (token != NULL) {
    sprintf(line, "Authorization: Bearer %s", token);
    compute_message(message, line);
  }

  compute_message(message, "");
  return message;
}