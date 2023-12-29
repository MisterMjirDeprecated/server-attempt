#include "sock.h"
#include <mswsock.h>
#include <stdlib.h>

#define DEFAULT_BUFLEN 512

#define MAX_IO_PEND 10

#define OP_READ 0x10
#define OP_WRITE 0x20

struct sock_state {
  WSAOVERLAPPED *overlap;
  SOCKET socket;
  SOCKET socket_accepted;
  DWORD op;
  char buff[128];
  WSABUF databuff;
};

struct data {
  WSAEVENT          handles[MAX_IO_PEND];
  struct sock_state states[MAX_IO_PEND];
  unsigned int      count;
};

/* Wait function to keep track of events */
int do_wait(struct data *data)
{
  DWORD wait_rc = 0;
  WSAEVENT h_temp = WSA_INVALID_EVENT;
  struct sock_state ss_temp;

  /* Rotate the array (avoid index 0, the listening socket) */
  for (int i = 1; i < data->count; ++i) {
    h_temp = data->handles[i + 1];
    data->handles[i + 1] = data->handles[i];
    data->handles[i] = h_temp;

    ss_temp = data->states[i + 1];
    data->states[i + 1] = data->states[i];
    data->states[i] = ss_temp;
  }

  if ((wait_rc = WSAWaitForMultipleEvents(data->count, data->handles, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED) {
    ERR("WSAWaitForMultipleEvents()");
    return -1;
  }

  return wait_rc - WSA_WAIT_EVENT_0;
}

/* handles all the events */
void handle_event(int index, struct data *data)
{
  WSAOVERLAPPED *overlap;
  SOCKET sock = INVALID_SOCKET;
  DWORD bytes = 0;
  DWORD flags = 0;
  DWORD lasterr = 0;

  overlap = data->states[index].overlap;

  if (!WSAResetEvent(data->handles[index])) {
    ERR("WSAResetEvent failed: %d", WSAGetLastError());
  }

  /* Check the handle */
  if (!WSAGetOverlappedResult(data->states[index].socket, overlap, &bytes, FALSE, &flags)) {
    ERR("WSAGetOverlappedresult failed: %d", WSAGetLastError());
  }

  sock = data->states[index].socket_accepted;

  /* If the connection is closed, move last element into index */
  if (index != 0 && bytes == 0) {
    closesocket(sock);
    free(overlap);
    WSACloseEvent(data->handles[index]);
    data->handles[index] = data->handles[data->count];
    data->states[index] = data->states[data->count];
    --data->count;
    return;
  }

  /* Check for max pending connections */
  if (index == 0) {
    if (data->count >= MAX_IO_PEND) {
      shutdown(sock, SD_BOTH);
      closesocket(sock);
      ERR("Too many pending requests");
      return;
    }
  }

  /* Use event handle for AcceptEx to queue WSARecv */
}

int main()
{
  struct sock_ctx ctx;
  struct sock listen_sock;
  struct data data;
  ZeroMemory(&data, sizeof(data));
  char *buffer = NULL;
  DWORD bytes = 0;
  DWORD bytes_read = 0;
  WSAOVERLAPPED *overlap = NULL;

  /* Microsoft extensions */
  LPFN_ACCEPTEX pfnAcceptEx;
  GUID acceptex_guid = WSAID_ACCEPTEX;

  if (!(buffer = malloc((2 * sizeof(SOCKADDR_STORAGE) + 32) * sizeof(char)))) {
    ERR("malloc failed");
    return -1;
  }

  /* Create the context */
  if (sock_ctx_init(&ctx)) {
    free(buffer);
    return -1;
  }
  sock_ctx_info(&ctx);

  /* Create the listening socket */
  if (sock_init(&listen_sock, AI_PASSIVE, AF_INET, SOCK_STREAM, IPPROTO_TCP)) {
    sock_ctx_quit(&ctx);
    free(buffer);
    return -1;
  }
  sock_info(&listen_sock);

  /* Bind the listening socket */
  if (sock_bind(&listen_sock)) {
    sock_quit(&listen_sock);
    sock_ctx_quit(&ctx);
    free(buffer);
    return -1;
  }

  /* Have the listening socket listen */
  if (sock_listen(&listen_sock, SOMAXCONN)) {
    sock_quit(&listen_sock);
    sock_ctx_quit(&ctx);
    free(buffer);
    return -1;
  }

  data.states[0].socket = listen_sock.socket;
  ++data.count;

  for (unsigned int i = 0; i < MAX_IO_PEND; ++i) {
    data.handles[i] = WSA_INVALID_EVENT;
  }

  /* Load the extension functions */
  if (WSAIoctl(listen_sock.socket,
               SIO_GET_EXTENSION_FUNCTION_POINTER,
               &acceptex_guid,
               sizeof(acceptex_guid),
               &pfnAcceptEx,
               sizeof(pfnAcceptEx),
               &bytes,
               NULL,
               NULL) == SOCKET_ERROR) {
    ERR("WSAIoctl failed (trying to get AcceptEx pointer): %d", WSAGetLastError());
  }

  /* Loop */
  SOCKET accept_sock;
  while (1) {
    /* Create a socket for AcceptEx */
    accept_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (accept_sock == INVALID_SOCKET) {
      ERR("WSASocket failed (trying to create an accept socket): %d", WSAGetLastError());
    }
    /* Allocate an overlapped structure, use offset field to keep track of the socket handle */
    if (!(overlap = calloc(1, sizeof(WSAOVERLAPPED)))) {
      ERR("malloc failed");
      closesocket(accept_sock);
      sock_quit(&listen_sock);
      sock_ctx_quit(&ctx);
      free(buffer);
      return -1;
    }
    if ((overlap->hEvent = WSACreateEvent()) == WSACreateEvent()) {
      ERR("WSACreateEvent failed: %d", WSAGetLastError());
      free(overlap);
      closesocket(accept_sock);
      sock_quit(&listen_sock);
      sock_ctx_quit(&ctx);
      free(buffer);
      return -1;
    }

    /* Set data */
    data.handles[0] = overlap->hEvent;
    data.states[0].overlap = overlap;
    data.states[0].socket_accepted = accept_sock;

    /* Call AcceptEx */
    if (!pfnAcceptEx(listen_sock.socket, 
                     accept_sock,
                     buffer,
                     0,
                     sizeof(SOCKADDR_STORAGE) + 16,
                     sizeof(SOCKADDR_STORAGE) + 16,
                     &bytes_read,
                     overlap)) {
      int error = WSAGetLastError();
      if (error != WSA_IO_PENDING) {
        ERR("AcceptEx failed: %d", error);
        for (int i = 0; i < data.count; ++i) {
          shutdown(data.states[i].socket, SD_BOTH);
          closesocket(data.states[i].socket);
          free(data.states[i].overlap);
          WSACloseEvent(data.handles[i]);
        }
        WSACloseEvent(overlap->hEvent);
        free(overlap);
        closesocket(accept_sock);
        sock_quit(&listen_sock);
        sock_ctx_quit(&ctx);
        free(buffer);
        return -1;
      }
    }
    /* Check handles for signals */
    int j = 0;
    while (1) {
      int j = do_wait(&data);
      if (j < 0)
        break;
      handle_event(j, &data);
      if (j == 0)
        break;
    }
    if (j < 0) {
      for (int i = 0; i < data.count; ++i) {
        for (int i = 0; i < data.count; ++i) {
          shutdown(data.states[i].socket, SD_BOTH);
          closesocket(data.states[i].socket);
          free(data.states[i].overlap);
          WSACloseEvent(data.handles[i]);
        }
        WSACloseEvent(overlap->hEvent);
        free(overlap);
        closesocket(accept_sock);
        sock_quit(&listen_sock);
        sock_ctx_quit(&ctx);
        free(buffer);
        return -1;
      }
    }
  }

  /* (shouldn't get here I think) */
  return 0;
}