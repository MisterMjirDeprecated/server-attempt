#include "client.h"

#define DEFAULT_PORT "27015"

/*
 * sock_host_init
 * 
 * @desc
 *   Creates a host socket
 * @param host
 *   sock_host struct
 * @return
 *   Success
 */
int sock_client_init(struct sock_client *client, char *addr)
{
  /* Set hints */
  ZeroMemory(&client->hints, sizeof(client->hints));
  client->hints.ai_family = AF_INET;
  client->hints.ai_socktype = SOCK_STREAM;
  client->hints.ai_protocol = IPPROTO_TCP;

  /* Get address info */
  int res;
  if (res = (getaddrinfo(addr, DEFAULT_PORT, &client->hints, &client->info))) {
    ERR("CLIENT> INIT> getaddrinfo failed: %d", res);
    return -1;
  }

  /* Create the socket */
  client->socket = INVALID_SOCKET;
  client->socket = socket(client->info->ai_family, client->info->ai_socktype, client->info->ai_protocol);
  if (client->socket == INVALID_SOCKET) {
    ERR("CLIENT> INIT> socket failed: %ld", WSAGetLastError());
    freeaddrinfo(client->info);
    return -1; 
  }

  return 0;
}

/*
 * sock_host_quit
 * 
 * @desc
 *   Cleans up a host socket
 * @param host
 *   sock_host struct
 */
void sock_client_quit(struct sock_client *client)
{
  freeaddrinfo(client->info);
  closesocket(client->socket);
}

int sock_client_connect(struct sock_client *client)
{
  int ires = connect(client->socket, client->info->ai_addr, (int) client->info->ai_addrlen);
  if (ires == SOCKET_ERROR) {
    closesocket(client->socket);
    client->socket = INVALID_SOCKET;
  }

  // Should try next address from info

  if (client->socket == INVALID_SOCKET) {
    ERR("Unable to connect to server");
    return -1;
  }

  return 0;
}