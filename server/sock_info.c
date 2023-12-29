#include "sock.h"

#define LBYTE(word) (word & 0x0F)
#define HBYTE(word) ((word & 0x0F) >> 8)
void sock_ctx_info_wsadata(WSADATA *data)
{
  LOG("Caller version:  %u.%u", LBYTE(data->wVersion), HBYTE(data->wVersion));
  LOG("Highest version: %u.%u", LBYTE(data->wHighVersion), HBYTE(data->wHighVersion));
  LOG("Description:     %s",    data->szDescription);
  LOG("Status/config:   %s",    data->szSystemStatus);
}

void sock_ctx_info(struct sock_ctx *ctx)
{
  LOG("--------- WINSOCK INFO ---------");
  sock_ctx_info_wsadata(&ctx->data);
  LOG("--------------------------------");
}

void sock_info_addrinfo_flags(int flags)
{
  LOG("> Flags: %d", flags);
  if (flags & AI_PASSIVE) {
    LOG("| > PASSIVE (socket will be used in the bind function)");
  }
  if (flags & AI_CANONNAME) {
    LOG("| > CANONNAME (the canonical name will be in the canonname member)");
  }
  if (flags & AI_NUMERICHOST) {
    LOG("| > NUMERICHOST (nodename paramter must be numeric)");
  }
  if (flags & AI_ALL) {
    LOG("| > ALL (request IPv6 and IPv4 with AI_V4MAPPED flag)")
  }
  if (flags & AI_ADDRCONFIG) {
    LOG("| > ADDRCONFIG (global address must be configured)");
  }
  if (flags & AI_V4MAPPED) {
    LOG("| > V4MAPPED (if IPv6 fails, requests IPv4 and converts to a IPv4-mapped IPv6 address)");
  }
  if (flags & AI_NON_AUTHORITATIVE) {
    LOG("| > NON_AUTHORITATIVE (address info can be from a non-authoritative namespace provider)");
  }
  if (flags & AI_SECURE) {
    LOG("| > SECURE (address info is from a secure channel)");
  }
  if (flags & AI_RETURN_PREFERRED_NAMES) {
    LOG("| > RETURN_PREFERRED_NAMES (adress info is for a preferred name for a user)");
  }
  if (flags & AI_FQDN) {
    LOG("| > FQDN (fully qualified domain name will be in the canonname member)");
  }
  if (flags & AI_FILESERVER) {
    LOG("| > FILESERVER (will be used in a file share scenario)");
  }
}

void sock_info_addrinfo_family(int family)
{
  LOG("> Family: %d", family);
  switch (family) {
  case AF_UNSPEC:
    LOG("| > UNSPEC (Unspecified)");
    break;
  case AF_INET:
    LOG("| > INET (IPv4)");
    break;
  case AF_NETBIOS:
    LOG("| > NETBIOS (NetBIOS)");
    break;
  case AF_INET6:
    LOG("| > INET6 (IPv6)");
    break;
  case AF_IRDA:
    LOG("| > IRDA (IrDA)");
    break;
  case AF_BTH:
    LOG("| > BTH (Bluetooth)");
    break;
  default:
    LOG("| > Unknown");
  }
}

void sock_info_addrinfo_socktype(int socktype)
{
  LOG("> Socket type: %d", socktype);
  switch (socktype) {
  case SOCK_STREAM:
    LOG("| > STREAM (Uses TCP for internet address family)");
    break;
  case SOCK_DGRAM:
    LOG("| > DGRAM (datagram, uses UDP for internet address family)");
    break;
  case SOCK_RAW:
    LOG("| > RAW (allows manipulation of the next upper-layer protocol header)");
    break;
  case SOCK_RDM:
    LOG("| > RDM (reliable message datagram)");
    break;
  case SOCK_SEQPACKET:
    LOG("| > SEQPACKET (psudeo-stream packet based on datagrams)");
    break;
  default:
    LOG("| > Unknown");
  }
}

void sock_info_addrinfo_protocol(int protocol)
{
  LOG("> Protocol type: %d", protocol);
  switch (protocol) {
  case IPPROTO_TCP:
    LOG("| > TCP (Transmission Control Protocol)");
    break;
  case IPPROTO_UDP:
    LOG("| > UDP (User Datagram Protocol)");
    break;
  default:
    LOG("| > Unknown");
  }
}

void sock_info_addrinfo_sockaddr(struct addrinfo *data)
{
  LOG("> Socket information");
  char nodebuff[NI_MAXHOST];
  char servicebuff[NI_MAXSERV];

  if (getnameinfo(data->ai_addr, data->ai_addrlen, nodebuff, NI_MAXHOST, servicebuff, NI_MAXSERV, 0)) {
    return;
  }

  LOG("| > Host: %s", nodebuff);
  LOG("| > Port: %s", servicebuff);
}

void sock_info_addrinfo(struct addrinfo *data)
{
  LOG("Address info");
  /* Does not give info on sockaddr structs */
  sock_info_addrinfo_flags(data->ai_flags);
  sock_info_addrinfo_family(data->ai_family);
  sock_info_addrinfo_socktype(data->ai_socktype);
  sock_info_addrinfo_protocol(data->ai_protocol);
  LOG("> Host name: %s", data->ai_canonname);
  sock_info_addrinfo_sockaddr(data);
}

/*
 * sock_info
 * 
 * @desc
 *   Logs information about the host
 * @param host
 *   sock_host struct
 */
void sock_info(struct sock *sock)
{
  LOG("--------- SOCKET  INFO ---------");
  struct addrinfo *ptr = sock->info;
  while (ptr) {
    sock_info_addrinfo(sock->info);
    ptr = ptr->ai_next;
  }
  LOG("--------------------------------");
}