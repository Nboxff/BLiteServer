#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

Socket::Socket() {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");
}

Socket::Socket(int fd) : fd(fd) {
    errif(fd == -1, "socket create error");
}

Socket::~Socket() {}

void Socket::bind(InetAddress* inet_address) {
    // call global scope bind function
    int bind_ret = ::bind(fd, (sockaddr*)&inet_address->serv_addr, sizeof(inet_address->serv_addr));
    errif(bind_ret == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

int Socket::accept(InetAddress* clnt_addr) {
    int clnt_sockfd = ::accept(fd, (sockaddr *) &clnt_addr->serv_addr, &clnt_addr->clnt_addr_len);
    errif(clnt_sockfd == -1, "socket accept error");
    return clnt_sockfd;
}

int Socket::getFd() {
    return this->fd;
}

void Socket::setnonblocking() {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}
