#include "InetAddress.h"
#include <cstring>

InetAddress::InetAddress() {
    bzero(&serv_addr, sizeof(serv_addr));
    clnt_addr_len = sizeof(serv_addr);
}

InetAddress::InetAddress(const char *ip, uint16_t port) : ip_address(ip), port(port) {
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);
    clnt_addr_len = sizeof(serv_addr);
}

InetAddress::~InetAddress() {}