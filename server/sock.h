#ifndef SOCK
#define SOCK

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

/* Log/Error macros */
#define ERR(msg, ...) { printf("SOCK> "); printf(msg, __VA_ARGS__); printf("\n"); }
#define LOG(msg, ...) { printf("SOCK> "); printf(msg, __VA_ARGS__); printf("\n"); }

struct sock_ctx {
  WSADATA data;
};

int  sock_ctx_init(struct sock_ctx *ctx);
void sock_ctx_quit(struct sock_ctx *ctx);
void sock_ctx_info(struct sock_ctx *ctx);

struct sock {
  struct addrinfo *info;
  SOCKET socket;
};

int  sock_init  (struct sock *sock);
void sock_quit  (struct sock *sock);
int  sock_bind  (struct sock *sock);
int  sock_listen(struct sock *sock);
void sock_info  (struct sock *sock);

#endif