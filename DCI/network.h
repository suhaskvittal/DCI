//
//  network.h
//  DCI
//
//  Created by Suhas Vittal on 5/21/20.
//

#ifndef network_h
#define network_h

#define MAX_CONNECTIONS 100

#define REQ_NET_ACCESS_STRING "NEED_NTWK_ACCESS"
#define REQ_CMD_SENT "SENT_CMD"
#define REQ_TXT_SENT "SENT_TXT"
#define REQ_AUTH_SENT "SENT_AUTH"

#define MAX_MSG_SIZE 4096

#define INSTANCE_STATE_DEFAULT 0
#define INSTANCE_STATE_CLOSED -1
#define INSTANCE_STATE_ACCESS_PENDING 1

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

int init_local(char* port, SOCKET* socket_p);
int connect_to_access(char* access_host, char* access_port, SOCKET* socket_p);
int connect_to_peer(char* host, char* port, SOCKET* socket_p);

int b_recv(SOCKET from, char** buf_p);
int b_send(SOCKET to, char* buf);
int i_recv(SOCKET from, int* n_p);
int i_send(SOCKET to, int n);

char* format_string(char* orig);

#endif /* network_h */
