#include "client.h"

int main(int argc, char *args[])
{
  struct sock_ctx ctx;
  struct sock_client client;

  /* Create the context */
  if (sock_init(&ctx)) {
    return -1;
  }
  sock_info(&ctx);

  /* Create and connect the client*/
  if (sock_client_init(&client, args[1])) {
    sock_quit(&ctx);
    return -1;
  }
  if (sock_client_connect(&client)) {
    sock_client_quit(&client);
    sock_quit(&ctx);
    return -1;
  }

  /* Communicate */
#define DEFAULT_BUFLEN 512
  int recvbuflen = DEFAULT_BUFLEN;
  int ires;

  const char *sendbuf = "this is a test";
  char recvbuf[DEFAULT_BUFLEN];

  char usrbuff[DEFAULT_BUFLEN];
  unsigned int usrbufflen = 0;
  int flag = 0;
  do {
    puts("Send a message to the server (just press enter without typing anything to exit)");
    fgets(usrbuff, DEFAULT_BUFLEN, stdin);
    usrbufflen = strlen(usrbuff) - 1; /* -1 to get rid of the newline */

    ires = send(client.socket, usrbuff, usrbufflen, 0);
    if (ires == SOCKET_ERROR) {
      ERR("send failed: %d", WSAGetLastError());
      sock_client_quit(&client);
      sock_quit(&ctx);
      return -1;
    }

  } while (usrbufflen > 0);

  /* Shutdown */
  if (shutdown(client.socket, SD_SEND) == SOCKET_ERROR) {
    ERR("shutdown failed: %d", WSAGetLastError());
    sock_client_quit(&client);
    sock_quit(&ctx);
    return -1;
  }

  /* Clean up */
  sock_client_quit(&client);
  sock_quit(&ctx);

  return 0;
}