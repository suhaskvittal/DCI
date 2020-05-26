//
//  main.c
//  DCI
//
//  Created by Suhas Vittal on 5/17/20.
//

#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "node.h"
#include "command.h"

#if defined(WIN32)
#include <conio.h>
#endif

#include <forward_list>

SOCKET client;
int network_size;
std::forward_list<struct node> network_sll;

static int instance_state;

int main(int argc, char* argv[]) {
	printf("Starting up...\n");
#if defined(_WIN32)
    // init winsock
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Winsock initialization failed.\n");
        return 1;
    }
#endif
    printf("Started. Initializing local instance...\n");
    SOCKET max_master_socket;
    fd_set master;
    FD_ZERO(&master);
    
    if (argc == 2) {
        // create new network instance
        char* port = argv[1];
        printf("Port number: %s\n", port);
        if (init_local(port, &client)) {
        	printf("Failed.\n");
            fprintf(stderr, "Initializing local instance failed (%d).\n", GETSOCKETERRNO());
            return 1;
        }

        printf("Initialized. Starting network...\n");

        max_master_socket = client;
        FD_SET(client, &master);
        add_to_network(NODE(client, "", port));
        
        instance_state = INSTANCE_STATE_DEFAULT;
    } else if (argc == 4) {
        char* client_port = argv[1];
        char* access_host = argv[2];
        char* access_port = argv[3];
        
        SOCKET access_peer;
        if (init_local(client_port, &client)) {
            fprintf(stderr, "Initializing local instance failed (%d).\n", GETSOCKETERRNO());
            return 1;
        }

        printf("Connecting to access point...\n");
        if (connect_to_access(access_host, access_port, &access_peer)) {
            fprintf(stderr, "Connect to access point failed (%d).\n", GETSOCKETERRNO());
            return 1;
        }
        
        max_master_socket = client;
        if (access_peer > max_master_socket) { max_master_socket = access_peer; }
        FD_SET(client, &master);
        add_to_network(NODE(client, "", client_port));
        FD_SET(access_peer, &master);
        add_to_network(NODE(access_peer, access_host, access_port));
        
        instance_state = INSTANCE_STATE_ACCESS_PENDING;
    } else {
        printf("usage: main <local-port> or main <local-port> <target-host> <target-port>\n");
        return 1;
    }
    
#if !defined(_WIN32)
    FD_SET(0, &master);  // 0 is file descriptor for stdin on UNIX and LINUX
#endif
    
    printf("Instance initialized. Ready to send input to network: \n");
    while (1) {
		struct timeval select_timeout = { 0, 100000 };  // 0.1 s timeout
        fd_set reads = master;
        if (select(max_master_socket + 1, &reads, 0, 0, &select_timeout) < 0) {
            fprintf(stderr, "select() failed (%d).\n", GETSOCKETERRNO());
            return 1;
        }
        
        auto nsll_iter = network_sll.begin();
        while (nsll_iter != network_sll.end()) {
            struct node network_node = *nsll_iter;
            SOCKET node_socket = network_node.socket;
            
            if (FD_ISSET(node_socket, &reads)) {
                if (node_socket == client) {
                    // check for new connection
                    struct sockaddr_storage peer_addr;

                    socklen_t peer_addrlen = sizeof(struct sockaddr_storage);
#if defined(_WIN32)
                    SOCKET peer = accept(client, (struct sockaddr*) &peer_addr, (int*) &peer_addrlen);
#else
                    SOCKET peer = accept(client, (struct sockaddr*) &peer_addr, &peer_addrlen);
#endif
                    if (!ISVALIDSOCKET(peer)) {
                        fprintf(stderr, "accept() failed (%d).\n", GETSOCKETERRNO());
                        instance_state = INSTANCE_STATE_CLOSED;
                        break;
                    }
                    
                    // get host and port strings
                    char host_buf[NI_MAXHOST];
                    char port_buf[NI_MAXSERV];
                    
                    getnameinfo((struct sockaddr*) &peer_addr, peer_addrlen,
                                host_buf, sizeof(host_buf),
                                port_buf, sizeof(port_buf),
                                NI_NUMERICHOST | NI_NUMERICSERV);
                    printf("\t%s %s has connected to network using this instance as the access point.\n", host_buf, port_buf);
                    // add peer to fd_set and the sll
                    FD_SET(peer, &master);
                    add_to_network(NODE(peer, host_buf, port_buf));
                    
                    if (max_master_socket < peer) { max_master_socket = peer; }
                } else if (instance_state == INSTANCE_STATE_ACCESS_PENDING) {
                    /* The peer should be sending us a list of peers to connect to. The message comes in several chunks:
                           (1) Number of peers
                           (2) For each peer:
                               (a) Size of ip address
                               (b) Ip address
                               (c) Size of port
                               (d) Port
                    */
                    int n_peers;
                    i_recv(node_socket, &n_peers);
                    printf("\tReceiving %d peers from access point. (%x)\n", n_peers, n_peers);
                    for (int i = 0; i < n_peers; i++) {
                        char* host_buf; char* port_buf;
                        if (b_recv(node_socket, &host_buf) < 0 || b_recv(node_socket, &port_buf) < 0) {
                            fprintf(stderr, "Network access failed (%d).\n", GETSOCKETERRNO());
                            instance_state = INSTANCE_STATE_CLOSED;
                            break;
                        }
                        
                        SOCKET peer;
                        if (connect_to_peer(host_buf, port_buf, &peer)) { return 1; }
                        if (peer > max_master_socket) { max_master_socket = peer; }
                        FD_SET(peer, &master);
                        add_to_network(NODE(peer, host_buf, port_buf));
                    }
                    if (instance_state == INSTANCE_STATE_CLOSED) { break; }  // some error has occurred
                    printf("\tSucceeded connecting to network.\n");
                    instance_state = INSTANCE_STATE_DEFAULT;
                } else if (instance_state == INSTANCE_STATE_DEFAULT) {
                    char* msg;
                    if (b_recv(node_socket, &msg) < 0) {
                        // if any error occurs, close the socket and remove from peer network
                        CLOSESOCKET(node_socket);
                        nsll_iter = network_sll.erase_after(nsll_iter);
                        continue;
                    }
                    if (strcmp(msg, REQ_NET_ACCESS_STRING) == 0) {
                        // send other peers to new peer
                        auto k = network_sll.begin();
                        int adjusted_network_size = network_size - 2;  // not sending client or peer
                        i_send(node_socket, adjusted_network_size);
						while (k != network_sll.end()) {
                            struct node n = *(k++);
                            if (n.socket == client || n.socket == node_socket) { continue; }  // the new peer has already connected to this socket
                            b_send(node_socket, (char*) n.ipv4_host.c_str());
                            b_send(node_socket, (char*) n.ipv4_port.c_str());
                        }
                    } else if (strcmp(msg, REQ_CMD_SENT) == 0) {
                        char* buf;
                        recv_and_parse(node_socket, &buf);
                        // send the buffer output back to the sender
                        b_send(node_socket, (char*) REQ_TXT_SENT);  // indicate we are sending text
                        b_send(node_socket, buf);
                    } else if (strcmp(msg, REQ_TXT_SENT) == 0) {
                        // simply print the text that was sent
                        char* buf;
                        if (b_recv(node_socket, &buf) < 0) { printf("\tMessage from node %d not received.\n", node_socket); }
                        else {
                            char* fbuf = format_string(buf);
                            printf("\tFrom node %d:\n%s\n", node_socket, fbuf);
                            free(buf);
                            free(fbuf);
                        }
                    } else if (strcmp(msg, REQ_AUTH_SENT) == 0) {
                        auth_provided(node_socket);
                        printf("\tSocket %d has given you authentication.\n", node_socket);
                    }
                    
                    free(msg);
                }
            } /* FD_ISSET */
            nsll_iter++;
        } /* while */
        
        if (instance_state == INSTANCE_STATE_CLOSED) {
            printf("Closing socket...\n");
            // do something
            break;
        }
        
#if defined(_WIN32)
        if (_kbhit()) {
#else
        if (FD_ISSET(0, &reads)) {
#endif
            char msg[MAX_MSG_SIZE + 1];
            if (!fgets(msg, sizeof(msg), stdin)) { break; }
            msg[MAX_MSG_SIZE] = '\0';
            parse_and_send(msg);
        }
    }
    CLOSESOCKET(client);
#if defined(WIN32)
    WSACleanup();  // shutdown winsock
#endif
    return 0;
}

void add_to_network(struct node node) {
    network_sll.push_front(node);
    network_size++;
}
