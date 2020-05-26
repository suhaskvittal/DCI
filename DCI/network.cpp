//
//  network.cpp
//  DCI
//
//  Created by Suhas Vittal on 5/22/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"

#include <vector>

int init_local(char* port, SOCKET* socket_p) {
    // get local address
    struct addrinfo* hints = (struct addrinfo*) calloc(1, sizeof(struct addrinfo));
    if (hints == nullptr) { return 1; }
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_family = AF_INET;
    hints->ai_flags = AI_PASSIVE;
    
    struct addrinfo* bind_addr;
    getaddrinfo(0, port, hints, &bind_addr);
    
    // socket
    SOCKET server = socket(bind_addr->ai_family, bind_addr->ai_socktype, bind_addr->ai_protocol);
    if (!ISVALIDSOCKET(server)) {
        fprintf(stderr, "socket() failed (%d).\n", GETSOCKETERRNO());
        freeaddrinfo(hints); freeaddrinfo(bind_addr);
        return 1;
    }
    
    // bind
    if (bind(server, bind_addr->ai_addr, bind_addr->ai_addrlen)) {
        fprintf(stderr, "bind() failed (%d). \n", GETSOCKETERRNO());
        free(hints); freeaddrinfo(bind_addr);
        return 1;
    }
    free(hints); freeaddrinfo(bind_addr);
    
    // listen
    if (listen(server, MAX_CONNECTIONS) < 0) {
        fprintf(stderr, "listen() failed (%d).\n", GETSOCKETERRNO());
        return 1;
    }
    
    *socket_p = server;
    return 0;
}

int connect_to_access(char* access_host, char* access_port, SOCKET* socket_p) {
    if (connect_to_peer(access_host, access_port, socket_p)) {
        return 1;  // 1 = ERROR in connecting to access host/peer
    }
    
    /* upon connection, we should send the peer an indication that we need the peer list.
    */
    
    if (b_send(*socket_p, (char*)REQ_NET_ACCESS_STRING) < 0) {
        return 1;
    }
    
    return 0;
}

int connect_to_peer(char* host, char* port, SOCKET* socket_p) {
    // get remote address
    struct addrinfo* hints = (struct addrinfo*) calloc(1, sizeof(struct addrinfo));
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_family = AF_INET;
    hints->ai_flags = AI_PASSIVE;
    
    struct addrinfo* peer_addr;
    if (getaddrinfo(host, port, hints, &peer_addr)) {
        fprintf(stderr, "getaddrinfo() failed (%d).\n", GETSOCKETERRNO());
        return 1;
    }
    
    // socket
    SOCKET peer = socket(peer_addr->ai_family, peer_addr->ai_socktype, peer_addr->ai_protocol);
    if (!ISVALIDSOCKET(peer)) {
        fprintf(stderr, "socket() failed (%d).\n", GETSOCKETERRNO());
        return 1;
    }
    
    // connect
    if (connect(peer, peer_addr->ai_addr, peer_addr->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d).\n", GETSOCKETERRNO());
        return 1;
    }
    *socket_p = peer;
    return 0;
}

int b_recv(SOCKET from, char** buf_p) {
    int msg_size;
    i_recv(from, &msg_size);
    if (msg_size <= 0) { return -1; }
    int bytes_recv = 0;
    char* buf = (char*) malloc((msg_size + 1) * sizeof(char));
    if (buf == nullptr) { return -1; }
    while (bytes_recv < msg_size) {
        int b = (int) recv(from, buf + bytes_recv, msg_size - bytes_recv, 0);
        if (b < 0) { return -1; }
        bytes_recv += b;
		printf("bytes_recv=%d (%d)\n", bytes_recv, msg_size);
    }
    
    buf[bytes_recv] = '\0';  // network sent strings are not null terminators
    *buf_p = buf;
    
    return bytes_recv;
}

int b_send(SOCKET to, char* buf) {
    int msg_size = (int) strlen(buf);
    i_send(to, msg_size);
    
    int bytes_sent = 0;
    while (bytes_sent < msg_size) {
        int b = (int) send(to, buf + bytes_sent, msg_size - bytes_sent, 0);
        if (b < 0) { return -1; }
        bytes_sent += b;
    }
    return 0;
}

/* We'll send and receive integers little endian to not worry about network and host byte order. */
int i_recv(SOCKET from, int* n_p) {
    int n = 0;
    int bytes_recv = 0;
    while (bytes_recv < sizeof(int)) {
    	char k;
		int b = (int) recv(from, &k, sizeof(k), 0);
		if (b < 0) { return -1; }
        n = (k << bytes_recv) | n;
        
        bytes_recv += b;
    }
    *n_p = n;
    return 0;
}

int i_send(SOCKET to, int n) {
    int k = n;
    int bytes_sent = 0;
    while (bytes_sent < sizeof(int)) {
    	char ch = k & 0xFF;  // get the last char in k
    	int b = (int) send(to, &ch, sizeof(char), 0);
        if (b < 0) { return -1; }
        k = k >> (b << 3);  // remove b bytes from n
        bytes_sent += b;
    }
    return 0;
}

char* format_string(char* orig) {
    std::vector<char> f_stack;
    f_stack.push_back('\t');
    f_stack.push_back('\t');
    
    std::vector<char> whitespace_stack;
    int k = 0;
    while (orig[k] != '\0') {
        char c = orig[k++];
        
        if (c == '\n' || c == ' ' || c == '\t') {
            whitespace_stack.push_back(c);
        } else {
            // dump the whitespace stack
            while (whitespace_stack.size() > 0) {
                char w = whitespace_stack.back();
                if (w == '\n') {
                    f_stack.push_back('\n');
                    f_stack.push_back('\t');
                    f_stack.push_back('\t');
                } else { f_stack.push_back(w); }
                
                whitespace_stack.pop_back();
            }
            f_stack.push_back(c);
        }
    }
    
    char* s = (char*) malloc((f_stack.size() + 1) * sizeof(char));
    int length = 0;
    while (length < f_stack.size()) { s[length] = f_stack[length]; length++;}
    s[length] = '\0';
    f_stack.clear();
    whitespace_stack.clear();
    
    return s;
}
