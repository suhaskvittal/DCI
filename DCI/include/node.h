//
//  node.h
//  DCI
//
//  Created by Suhas Vittal on 5/21/20.
//

#ifndef node_h
#define node_h

#include "network.h"

#include <string>

struct node {
    SOCKET socket;
    std::string ipv4_host;
    std::string ipv4_port;
};

#define NODE(s, h, p) (struct node){s, std::string(h), std::string(p)}

void add_to_network(struct node);

#endif /* node_h */
