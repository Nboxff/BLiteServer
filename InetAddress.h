#pragma once
#include <arpa/inet.h>

class InetAddress {
public:
    struct sockaddr_in serv_addr;
    socklen_t clnt_addr_len;
    const char * ip_address;
    uint16_t port;
    
    InetAddress();
    InetAddress(const char *ip, uint16_t port);
    ~InetAddress();
};