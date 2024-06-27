#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

#define SERVER_IP "34.246.184.49"
#define SERVER_PORT 8080
#define HOST "34.246.184.49:8080"
#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define LOGOUT "/api/v1/tema/auth/logout"
#define PAYLOAD "application/json"
#define GET_LIBRARY "/api/v1/tema/library/access"
#define GET_BOOKS "/api/v1/tema/library/books"
#define BOOK_ID "/api/v1/tema/library/books/"

#define ERROR_STRING "error"
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type,
                    int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

#endif
