//
//  network.cpp
//  DCI
//
//  Created by Suhas Vittal on 5/22/20.
//

/* MOSTLY LEGACY C CODE */
// TODO convert C code into C++ code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/network.h"

#include <iostream>
#include <vector>


static char* client_port;

int init_local(char* port, SOCKET* socket_p) {
    // set global client_port string to the port passed
    client_port = port;
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
    
    /* upon connection, we should send the peer an indication that we need the peer list and our port for other users to connect to.
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
    free(hints);
    freeaddrinfo(peer_addr);
    *socket_p = peer;
    
    // send the peer our port in case they need to share our address
    if (b_send(*socket_p, (char*)REQ_PORT_SENT) < 0
        || b_send(*socket_p, client_port) < 0) {
        return 1;
    }
    
    return 0;
}

int b_recv(SOCKET from, char** buf_p) {
    int msg_size;
    i_recv(from, &msg_size);
    if (msg_size <= 0) { return -1; }
    int bytes_recv = 0;
    char* buf = new char[msg_size + 1];
    if (buf == nullptr) { return -1; }
    while (bytes_recv < msg_size) {
        int b = (int) recv(from, buf + bytes_recv, msg_size - bytes_recv, 0);
        if (b < 0) { return -1; }
        bytes_recv += b;
    }
    buf[bytes_recv] = '\0';  // network sent strings are not null terminators
    *buf_p = buf;
    return bytes_recv;
}

/* This variant should be used for file data as files may have the NULL character. */
int b_send(SOCKET to, char* buf, unsigned long buf_size) {
    i_send(to, (int) buf_size);
    int bytes_sent = 0;
    while (bytes_sent < buf_size) {
        int b = (int) send(to, buf + bytes_sent, buf_size - bytes_sent, 0);
        if (b < 0) { return -1; }
        bytes_sent += b;
    }
    return 0;
}

int b_send(SOCKET to, char* buf) {
    int msg_size = (int) strlen(buf);
    return b_send(to, buf, msg_size);
}

/* We'll send and receive integers little endian to not worry about network and host byte order. */
int i_recv(SOCKET from, int* n_p) {
    int k = 0;
    int bytes_recv = 0;
    while (bytes_recv < sizeof(int)) {
        char ch[2];
		if (recv(from, ch, sizeof(ch), 0) < 0) { return -1; }
        k = (
             ((ch[1] << 4) | ch[0])
                << (bytes_recv << 3)
             ) | k;
        
        bytes_recv++;
    }
    *n_p = k;
    return 0;
}

int i_send(SOCKET to, int n) {
    int k = n;
    int bytes_sent = 0;
    while (bytes_sent < sizeof(int)) {
        char ch[2];
        // send two four bit chunks to avoid overflow
        ch[0] = k & 0xF; ch[1] = (k >> 4) & 0xF;
        if (send(to, ch, sizeof(ch), 0) < 0) { return -1; }
        k = k >> 8;
        bytes_sent++;
    }
    return 0;
}

