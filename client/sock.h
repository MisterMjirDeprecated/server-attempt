#ifndef SOCK
#define SOCK

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

int  sock_init(struct sock_ctx *ctx);
void sock_quit(struct sock_ctx *ctx);
void sock_info(struct sock_ctx *ctx);

#endif