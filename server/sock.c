#include "sock.h"

int sock_ctx_init(struct sock_ctx *ctx)
{
  if (WSAStartup(MAKEWORD(2, 2), &ctx->data)) {
    ERR("INIT> WSAStartup failed");
    return -1;
  }

  return 0;
}

void sock_ctx_quit(struct sock_ctx *ctx)
{
  WSACleanup();
}

#define DEFAULT_PORT "27015"

/*
 * sock_init
 * 
 * @desc
 *   Creates a host socket
 * @param host
 *   sock_host struct
 * @return
 *   Success
 */
int sock_init(struct sock *sock, int flags, int family, int socktype, int protocol)
{
  struct addrinfo hints;
  /* Set hints */
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_flags = flags;
  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_protocol = protocol;

  /* Get address info */
  int res;
  if (res = (getaddrinfo(NULL, DEFAULT_PORT, &hints, &sock->info))) {
    ERR("SOCKET> INIT> getaddrinfo failed: %d", res);
    return -1;
  }

  /* Create the socket */
  sock->socket = INVALID_SOCKET;
  sock->socket = WSASocket(sock->info->ai_family, sock->info->ai_socktype, sock->info->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
  if (sock->socket == INVALID_SOCKET) {
    ERR("SOCKET> INIT> WSASocket failed: %ld", WSAGetLastError());
    freeaddrinfo(sock->info);
    return -1; 
  }

  return 0;
}

/*
 * sock_quit
 * 
 * @desc
 *   Cleans up a host socket
 * @param host
 *   sock_host struct
 */
void sock_quit(struct sock *sock)
{
  freeaddrinfo(sock->info);
  closesocket(sock->socket);
}

/*
 * sock_bind
 * 
 * @desc
 *   Binds a host socket
 * @param host
 *   sock_host struct
 */
int sock_bind(struct sock *sock)
{
  /* Bind the socket */
  if (bind(sock->socket, sock->info->ai_addr, (int) sock->info->ai_addrlen) == SOCKET_ERROR) {
    ERR("SOCKET> bind failed: %d", WSAGetLastError());
    return -1;
  }

  return 0;
}

/*
 * sock_listen
 * 
 * @desc
 *   idk what to write
 * @param host
 *   sock_host struct
 */
int sock_listen(struct sock *sock, int backlog)
{
  if (listen(sock->socket, backlog) == SOCKET_ERROR) {
    ERR("SOCKET> listen failed: %ld", WSAGetLastError());
    return -1;
  }

  return 0;
}
