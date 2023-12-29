#ifndef SOCK_CLIENT
#define SOCK_CLIENT

#include "sock.h"

struct sock_client {
  struct addrinfo *info;
  //struct addrinfo *ptr;
  struct addrinfo hints;
  SOCKET socket; /* socket for listening */
};

int  sock_client_init   (struct sock_client *client, char *addr);
void sock_client_quit   (struct sock_client *client);
int  sock_client_connect(struct sock_client *client);

#endif