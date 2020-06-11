//
//  main.cpp
//  DCI
//
//  Created by Suhas Vittal on 5/17/20.
//

#include <stdlib.h>
#include <string.h>

#include "../include/node.h"
#include "../include/command.h"

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

#include <iostream>
#include <forward_list>

// TODO implement OpenSSL

using namespace std;

SOCKET client;
int network_size;
std::forward_list<struct node*> network_sll;

static int instance_state;

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // init winsock
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        cerr << color_error("Winsock initialization failed.") << endl;
        return 1;
    }

    // enable ANSI color codes
    HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h_stdout, &mode);
    mode = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(h_stdout, mode);
#endif
    cout << color_client("Started. Initializing local instance...") << endl;
    SOCKET max_master_socket;
    fd_set master;
    FD_ZERO(&master);
    
    char* client_port = argv[1];
    if (init_local(client_port, &client)) {
        cerr << color_error("Initializing local instance failed (" + to_string(GETSOCKETERRNO()) + ").") << endl;
        return 1;
    }
    if (argc == 2) {
        cout << color_client("Initialized. Starting network...") << endl;

        max_master_socket = client;
        FD_SET(client, &master);
        add_to_network(NODE(client, "", client_port));
        
        instance_state = INSTANCE_STATE_DEFAULT;
    } else if (argc == 4) {
        char* access_host = argv[2];
        char* access_port = argv[3];
        
        SOCKET access_peer;
        cout << color_client("Connecting to access point...") << endl;
        if (connect_to_access(access_host, access_port, &access_peer)) {
            cerr << color_error("Connect to access point failed (" + to_string(GETSOCKETERRNO()) + ").") << endl;
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
        cout << color_error("usage: main <local-port> or main <local-port> <target-host> <target-port>") << endl;
        return 1;
    }
    
#if !defined(_WIN32)
    FD_SET(0, &master);  // 0 is file descriptor for stdin on UNIX and LINUX
#endif
    
    cout << color_client("Instance initialized. Ready to send input to network (to close your connection to the network, type \"" + string(EXIT_KEYWORD) + "\"): ") << endl;
    while (1) {
		struct timeval select_timeout = { 0, 100000 };  // 0.1 s timeout
        fd_set reads = master;
        if (select(max_master_socket + 1, &reads, 0, 0, &select_timeout) < 0) {
            cerr << color_client("select() failed (" + to_string(GETSOCKETERRNO()) + ").") << endl;
            break;  // close the connection
        }
        
        auto nsll_prev = network_sll.before_begin();  // we need this for removal
        auto nsll_curr = network_sll.begin();
        while (nsll_curr != network_sll.end()) {
            struct node* network_node = *nsll_curr;
            SOCKET node_socket = network_node->socket;
            
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
                        cerr << color_error("accept() failed (" + to_string(GETSOCKETERRNO()) + ").") << endl;
                        instance_state = INSTANCE_STATE_CLOSED;
                        break;
                    }
                    
                    // get host and port strings
                    char host_buf[NI_MAXHOST];
                    
                    getnameinfo((struct sockaddr*) &peer_addr, peer_addrlen,
                                host_buf, sizeof(host_buf),
                                0, 0,
                                NI_NUMERICHOST);
                    cout << color_client("User at " + string(host_buf) + " has connected to the network.") << endl;  // add peer to fd_set and the sll
                    FD_SET(peer, &master);
                    add_to_network(NODE(peer, host_buf, ""));
                    
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
                    cout << color_client("Receiving " + to_string(n_peers) + " peers from access point.") << endl;
                    for (int i = 0; i < n_peers; i++) {
                        char* host_buf; char* port_buf;
                        if (b_recv(node_socket, &host_buf) < 0 || b_recv(node_socket, &port_buf) < 0) {
                            cerr << color_error("Network access failed (" + to_string(GETSOCKETERRNO()) + ").") << endl;
                            instance_state = INSTANCE_STATE_CLOSED;
                            break;
                        }
                        
                        SOCKET peer;
                        if (connect_to_peer(host_buf, port_buf, &peer)) { return 1; }
                        if (peer > max_master_socket) { max_master_socket = peer; }
                        FD_SET(peer, &master);
                        add_to_network(NODE(peer, host_buf, port_buf));
                        delete[] host_buf; delete[] port_buf;
                    }
                    if (instance_state == INSTANCE_STATE_CLOSED) { break; }  // some error has occurred
                    cout << color_client("Succeeded connecting to network.") << endl;
                    instance_state = INSTANCE_STATE_DEFAULT;
                } else if (instance_state == INSTANCE_STATE_DEFAULT) {
                    char* msg;
                    if (b_recv(node_socket, &msg) < 0) {
                        // if any error occurs, close the socket and remove from peer network
                        FD_CLR(node_socket, &master);
                        CLOSESOCKET(node_socket);
                        nsll_curr = network_sll.erase_after(nsll_prev);
                        delete network_node;
                        cout << color_error("Closing connection with node " + to_string(node_socket) + ".") << endl;
                        continue;
                    }
                    if (strcmp(msg, REQ_PORT_SENT) == 0) {
                        // get the port the peer has sent us
                        char* pport;
                        if (b_recv(node_socket, &pport) < 0) {
                            FD_CLR(node_socket, &master);
                            CLOSESOCKET(node_socket);
                            nsll_curr = network_sll.erase_after(nsll_prev);
                            delete network_node;
                            cout << color_error("Closing connection with node " + to_string(node_socket) + ".") << endl;
                            continue;  // just close the connection with the peer
                        }
                        network_node->ipv4_port = string(pport);
                        delete[] pport;
                    } else if (strcmp(msg, REQ_NET_ACCESS_STRING) == 0) {
                        // send other peers to new peer
                        auto k = network_sll.begin();
                        int adjusted_network_size = network_size - 2;  // not sending client or peer
                        i_send(node_socket, adjusted_network_size);
						while (k != network_sll.end()) {
                            struct node* n = *(k++);
                            if (n->socket == client || n->socket == node_socket) { continue; }  // the new peer has already connected to this socket
                            b_send(node_socket, (char*) n->ipv4_host.c_str());
                            b_send(node_socket, (char*) n->ipv4_port.c_str());
                        }
                    } else if (strcmp(msg, REQ_CMD_SENT) == 0) {
                        char* buf;
                        if (!recv_and_parse(node_socket, &buf)) {
                            // send the buffer output back to the sender
                            if (strlen(buf) > 0) {
                                b_send(node_socket, (char*) REQ_TXT_SENT);  // indicate we are sending text
                                b_send(node_socket, buf);
                            }
                        } else {
                            b_send(node_socket, (char*) REQ_TXT_SENT);  // indicate we are sending text
                            b_send(node_socket, (char*) "An error has occurred.\0");
                        }
                    } else if (strcmp(msg, REQ_TXT_SENT) == 0) {
                        // simply print the text that was sent
                        char* buf;
                        if (b_recv(node_socket, &buf) < 0) {
                            cout << color_error("Message from node " + to_string(node_socket) + " not received.") << endl;
                        }
                        else {
                            char* fbuf = format_string(buf);
                            string s = "FROM [" + to_string(node_socket) + "]: " +  fbuf;
                            cout << color_peer(s) << endl;
                            delete[] buf;
                            delete[] fbuf;
                        }
                    } else if (strcmp(msg, REQ_AUTH_YES_SENT) == 0) {
                        give_auth(node_socket);
                        cout << color_peer("Node " + to_string(node_socket) + " has given you authentication.") << endl;
                    } else if (strcmp(msg, REQ_AUTH_NO_SENT) == 0) {
                        remove_auth(node_socket);
                        cout << color_error("Node " + to_string(node_socket) + " has revoked your authentication. You can no longer perform privileged commands that affect their system.") << endl;
                    }
                    
                    delete[] msg;
                }
            } /* FD_ISSET */
            nsll_curr++;
            nsll_prev++;
        } /* while */
        
        if (instance_state == INSTANCE_STATE_CLOSED) {
            cout << "Closing socket..." << endl;
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
            
            if (strncmp(msg, (char*) EXIT_KEYWORD, strlen(EXIT_KEYWORD)) == 0) {
                break;
            }
            parse_and_send(msg);
        }
    }
    // Clean up all the memory that has been allocated
    auto nsll_iter = network_sll.begin();
    while (nsll_iter != network_sll.end()) {
        delete *nsll_iter;
        nsll_iter++;
    }
    network_sll.clear();
        
    cout << color_client("Closing down...") << endl;
    CLOSESOCKET(client);
#if defined(WIN32)
    WSACleanup();  // shutdown winsock
#endif
    return 0;
}

void add_to_network(struct node node) {
    struct node* node_p = new struct node(node);
    network_sll.push_front(node_p);
    network_size++;
}
    
string color_client(string s) {
    return CLIENT_MSG_COLOR + s + ANSI_COLOR_RESET;
}

string color_peer(string s) {
    return PEER_MSG_COLOR + s + ANSI_COLOR_RESET;
}

string color_error(string s) {
    return ERROR_MSG_COLOR + s + ANSI_COLOR_RESET;
}
