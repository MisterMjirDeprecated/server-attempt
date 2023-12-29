#include "sock.h"
#include <stdio.h>

/* Utility macros */
#define LBYTE(word) (word & 0x0F)
#define HBYTE(word) ((word & 0x0F) >> 8)

int sock_init(struct sock_ctx *ctx)
{
  if (WSAStartup(MAKEWORD(2, 2), &ctx->data)) {
    ERR("INIT> WSAStartup failed");
    return -1;
  }

  return 0;
}

void sock_quit(struct sock_ctx *ctx)
{
  WSACleanup();
}

void sock_info_wsadata(WSADATA *data)
{
  LOG("Caller version:  %u.%u", LBYTE(data->wVersion), HBYTE(data->wVersion));
  LOG("Highest version: %u.%u", LBYTE(data->wHighVersion), HBYTE(data->wHighVersion));
  LOG("Description:     %s",    data->szDescription);
  LOG("Status/config:   %s",    data->szSystemStatus);
}

void sock_info(struct sock_ctx *ctx)
{
  LOG("--------- WINSOCK INFO ---------");
  sock_info_wsadata(&ctx->data);
  LOG("--------------------------------");
}