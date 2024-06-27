#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "helpers.h"
#include "parson.h"
#include "requests.h"

/**
 * Function to create a json string for the login and register requests.
 * @param username the username
 * @param password the password
 * @return the json string
 */
char *create_json_login(char *username, char *password) {
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  json_object_set_string(root_object, "username", username);
  json_object_set_string(root_object, "password", password);

  char *json_string = json_serialize_to_string_pretty(root_value);
  json_value_free(root_value);
  return json_string;
}

/**
 * Register function, sends a POST request to the server to register a new user.
 * @param sockfd the socket file descriptor
 */
void register_user(int sockfd) {
  char username[BUFLEN];
  char password[BUFLEN];
  char aux[BUFLEN];
  fgets(aux, BUFLEN, stdin);

  // username
  printf("username=");
  memset(username, 0, BUFLEN);
  fgets(username, BUFLEN, stdin);
  username[strlen(username) - 1] = '\0';

  // password
  printf("password=");
  memset(password, 0, BUFLEN);
  fgets(password, BUFLEN, stdin);
  password[strlen(password) - 1] = '\0';

  if (strchr(username, ' ') != NULL || strchr(password, ' ') != NULL) {
    printf("Error, invalid username or password.\n");
    return;
  }

  // create json payload
  char *json_string = create_json_login(username, password);

  char *message =
      compute_post_request(HOST, REGISTER, PAYLOAD, json_string, NULL, NULL);
  send_to_server(sockfd, message);

  json_free_serialized_string(json_string);
  free(message);

  char *response = receive_from_server(sockfd);

  if (strstr(response, ERROR_STRING) != NULL) {
    printf("Error, user already exists.\n");
    free(response);
    return;
  }

  printf("User %s registered successfully.\n", username);
  free(response);
}

/**
 * Login function, sends a POST request to the server to login a user.
 * If the login is successful, the cookies are saved.
 * @param sockfd the socket file descriptor
 * @param cookies the cookies received from the server
 */
int login(int sockfd, char **cookies) {
  char username[BUFLEN];
  char password[BUFLEN];
  char aux[BUFLEN];
  fgets(aux, BUFLEN, stdin);

  // username
  printf("username=");
  memset(username, 0, BUFLEN);
  fgets(username, BUFLEN, stdin);
  username[strlen(username) - 1] = '\0';

  // password
  printf("password=");
  memset(password, 0, BUFLEN);
  fgets(password, BUFLEN, stdin);
  password[strlen(password) - 1] = '\0';

  if (strchr(username, ' ') != NULL || strchr(password, ' ') != NULL) {
    printf("Error, invalid username or password.\n");
    return 0;
  }

  // create json payload
  char *json_string = create_json_login(username, password);

  char *message =
      compute_post_request(HOST, LOGIN, PAYLOAD, json_string, NULL, NULL);
  send_to_server(sockfd, message);

  json_free_serialized_string(json_string);
  free(message);

  char *response = receive_from_server(sockfd);

  if (strstr(response, ERROR_STRING) != NULL) {
    char *error = strstr(response, ERROR_STRING);
    if (strstr(error, "Credentials") != NULL) {
      printf("Error, invalid credentials.\n");
      return 0;
    } else {
      printf("Error, no user found.\n");
      return 0;
    }
  }

  char *cookie_start = strstr(response, "Set-Cookie: ");
  if (cookie_start) {
    // Move past "Set-Cookie: "
    cookie_start += 12;

    char *cookie_end = strstr(cookie_start, "\r\n");
    if (!cookie_end) {
      cookie_end = strchr(cookie_start, '\n');
    }

    if (cookie_end) {
      size_t cookie_length = cookie_end - cookie_start;
      *cookies = (char *)malloc(cookie_length + 1);
      if (*cookies) {
        strncpy(*cookies, cookie_start, cookie_length);
        (*cookies)[cookie_length] = '\0';
      }
    }
  }

  printf("User %s logged in successfully.\n", username);
  free(response);
  return 1;
}

/**
 * Enter library function, sends a GET request to the server to enter the
 * library. If the request is successful, the token is saved.
 * @param sockfd the socket file descriptor
 * @param cookies the cookies received from the server
 * @param token the token received from the server
 */
void enter_library(int sockfd, char *cookies, char **token) {
  char *message = compute_get_request(HOST, GET_LIBRARY, cookies, NULL);
  send_to_server(sockfd, message);
  free(message);

  char *response = receive_from_server(sockfd);

  char *token_start = strstr(response, "\"token\":\"");
  if (token_start) {
    token_start += 9;

    char *token_end = strchr(token_start, '\"');
    if (token_end) {
      size_t token_length = token_end - token_start;
      *token = (char *)malloc(token_length + 1);
      if (*token) {
        strncpy(*token, token_start, token_length);
        (*token)[token_length] = '\0';
      }
    }
  }
  printf("Successfully entered library.\n");
  free(response);
}

/**
 * Get books function, sends a GET request to the server to get all the books
 * from the library.
 * @param sockfd the socket file descriptor
 * @param token the token received from the server
 */
void get_books(int sockfd, char *token) {
  char *message = compute_get_request(HOST, GET_BOOKS, NULL, token);
  send_to_server(sockfd, message);
  free(message);

  char *response = receive_from_server(sockfd);
  char *json = strstr(response, "[");
  JSON_Value *root_value = json_parse_string(json);

  if (root_value == NULL) {
    printf("Error, no books found.\n");
    free(response);
    return;
  }

  char *json_pretty = json_serialize_to_string_pretty(root_value);
  printf("%s\n", json_pretty);

  json_value_free(root_value);
  json_free_serialized_string(json_pretty);
  free(response);
}

/**
 * Check if a string is a number.
 * @param str the string to check
 * @return 1 if the string is a number, 0 otherwise
 */
int is_number(char *str) {
  for (int i = 0; i < strlen(str); i++) {
    if (!isdigit(str[i])) {
      return 0;
    }
  }
  return 1;
}

/**
 * Create the path for a book by id.
 * @param id the id of the book
 * @return the path
 */
char *create_path(char *id) {
  char *new_path = (char *)malloc(BUFLEN);
  strcpy(new_path, BOOK_ID);
  strcat(new_path, id);
  return new_path;
}

/**
 * Get book function, sends a GET request to the server to get a book by id.
 * @param sockfd the socket file descriptor
 * @param token the token received from the server
 */
void get_book(int sockfd, char *token) {
  char id[BUFLEN];

  printf("id=");
  scanf("%s", id);
  if (!is_number(id)) {
    printf("Error, invalid id.\n");
    return;
  }

  char *path = create_path(id);

  char *message = compute_get_request(HOST, path, NULL, token);
  send_to_server(sockfd, message);
  free(message);

  char *response = receive_from_server(sockfd);
  if (strstr(response, ERROR_STRING) != NULL) {
    printf("Error, book not found.\n");
    free(response);
    return;
  }
  char *json = strstr(response, "{");
  JSON_Value *root_value = json_parse_string(json);
  char *json_pretty = json_serialize_to_string_pretty(root_value);
  printf("%s\n", json_pretty);

  json_value_free(root_value);
  json_free_serialized_string(json_pretty);
  free(response);
  free(path);
}

/**
 * Add book function, sends a POST request to the server to add a book.
 * @param sockfd the socket file descriptor
 * @param token the token received from the server
 */
void add_book(int sockfd, char *token) {
  char title[BUFLEN];
  char author[BUFLEN];
  char genre[BUFLEN];
  char publisher[BUFLEN];
  char page_count[BUFLEN];
  char aux[BUFLEN];
  fgets(aux, BUFLEN, stdin);

  // create json payload
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  // title
  printf("title=");
  memset(title, 0, BUFLEN);
  fgets(title, BUFLEN, stdin);
  title[strlen(title) - 1] = '\0';
  json_object_set_string(root_object, "title", title);

  // author
  printf("author=");
  memset(author, 0, BUFLEN);
  fgets(author, BUFLEN, stdin);
  author[strlen(author) - 1] = '\0';
  json_object_set_string(root_object, "author", author);

  // genre
  printf("genre=");
  memset(genre, 0, BUFLEN);
  fgets(genre, BUFLEN, stdin);
  genre[strlen(genre) - 1] = '\0';
  json_object_set_string(root_object, "genre", genre);

  // publisher
  printf("publisher=");
  memset(publisher, 0, BUFLEN);
  fgets(publisher, BUFLEN, stdin);
  publisher[strlen(publisher) - 1] = '\0';
  json_object_set_string(root_object, "publisher", publisher);

  // page_count
  printf("page_count=");
  memset(page_count, 0, BUFLEN);
  fgets(page_count, BUFLEN, stdin);
  page_count[strlen(page_count) - 1] = '\0';
  json_object_set_string(root_object, "page_count", page_count);

  if (!is_number(page_count)) {
    printf("Error, invalid page count.\n");
    json_value_free(root_value);
    return;
  }

  if (strlen(author) == 0 || strlen(title) == 0 || strlen(genre) == 0 ||
      strlen(publisher) == 0 || strlen(page_count) == 0) {
    printf("Error, empty field found.\n");
    json_value_free(root_value);
    return;
  }

  char *json_string = json_serialize_to_string(root_value);
  char *message =
      compute_post_request(HOST, GET_BOOKS, PAYLOAD, json_string, NULL, token);
  send_to_server(sockfd, message);

  json_free_serialized_string(json_string);
  json_value_free(root_value);
  free(message);

  char *response = receive_from_server(sockfd);

  if (strstr(response, ERROR_STRING) != NULL) {
    printf("Error, book not added.\n");
    free(response);
    return;
  }

  printf("Book added successfully.\n");
  free(response);
}

/**
 * Delete book function, sends a DELETE request to the server to delete a book
 * by id.
 * @param sockfd the socket file descriptor
 * @param token the token received from the server
 */
void delete_book(int sockfd, char *token) {
  char id[BUFLEN];

  printf("id=");
  scanf("%s", id);
  if (!is_number(id)) {
    printf("Error, invalid id.\n");
    return;
  }

  char *path = create_path(id);

  char *message = compute_delete_request(HOST, path, NULL, token);
  send_to_server(sockfd, message);
  free(message);

  char *response = receive_from_server(sockfd);
  if (strstr(response, ERROR_STRING) != NULL) {
    printf("Error, book not found.\n");
    free(response);
    return;
  }

  printf("Book deleted successfully.\n");
  free(response);
  free(path);
}

/**
 * Logout function, sends a GET request to the server to logout a user.
 * @param sockfd the socket file descriptor
 * @param cookies the cookies received from the server
 * @param token the token received from the server
 */
void logout(int sockfd, char **cookies, char **token) {
  char *message = compute_get_request(HOST, LOGOUT, *cookies, NULL);
  send_to_server(sockfd, message);
  free(message);

  char *response = receive_from_server(sockfd);
  if (strstr(response, ERROR_STRING) != NULL) {
    printf("Error, logout failed.\n");
    free(response);
    return;
  }

  printf("Successfully logged out.\n");
  free(*cookies);
  free(*token);
  free(response);
}

int main() {
  int sockfd;
  char *cookies = NULL;
  char *token = NULL;
  int login_ok = 0;    // if the user is logged in
  int library_ok = 0;  // if the user is in the library

  while (1) {
    int ok = 0;  // if the command is valid
    sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      printf("Error opening connection\n");
      return -1;
    }

    char buffer[BUFLEN];
    memset(buffer, 0, BUFLEN);
    scanf("%s", buffer);

    if (strcmp(buffer, "register") == 0) {
      register_user(sockfd);
      ok = 1;
      continue;
    }
    if (strcmp(buffer, "login") == 0) {
      if (login_ok == 1) {
        printf("Error, you are already logged in.\n");
        continue;
      }

      ok = 1;
      login_ok = login(sockfd, &cookies);
      continue;
    }
    if (strcmp(buffer, "enter_library") == 0) {
      if (login_ok == 0) {
        printf("Error, you must login first.\n");
        continue;
      }
      enter_library(sockfd, cookies, &token);
      ok = 1;
      library_ok = 1;
      continue;
    }
    if (strcmp(buffer, "get_books") == 0) {
      if (login_ok == 0) {
        printf("Error, you must login first.\n");
        continue;
      }
      if (library_ok == 0) {
        printf("Error, you must enter library first.\n");
        continue;
      }
      get_books(sockfd, token);
      ok = 1;
      continue;
    }
    if (strcmp(buffer, "get_book") == 0) {
      if (login_ok == 0) {
        printf("Error, you must login first.\n");
        continue;
      }
      if (library_ok == 0) {
        printf("Error, you must enter library first.\n");
        continue;
      }
      get_book(sockfd, token);
      ok = 1;
      continue;
    }
    if (strcmp(buffer, "add_book") == 0) {
      if (login_ok == 0) {
        printf("Error, you must login first.\n");
        continue;
      }
      if (library_ok == 0) {
        printf("Error, you must enter library first.\n");
        continue;
      }
      add_book(sockfd, token);
      ok = 1;
      continue;
    }
    if (strcmp(buffer, "delete_book") == 0) {
      if (login_ok == 0) {
        printf("Error, you must login first.\n");
        continue;
      }
      if (library_ok == 0) {
        printf("Error, you must enter library first.\n");
        continue;
      }
      delete_book(sockfd, token);
      ok = 1;
      continue;
    }
    if (strcmp(buffer, "logout") == 0) {
      if (login_ok == 0) {
        printf("Error, you must login first.\n");
        continue;
      }
      logout(sockfd, &cookies, &token);
      login_ok = 0;
      library_ok = 0;
      ok = 1;
      cookies = NULL;
      token = NULL;
      continue;
    }
    if (strcmp(buffer, "exit") == 0) {
      close(sockfd);
      break;
    }
    if (ok == 0) {
      printf("Invalid command.\n");
      continue;
    }
  }
  close(sockfd);
  return 0;
}
