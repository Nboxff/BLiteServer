#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include <functional>
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <unistd.h>

#define READ_BUFFER 1024

Server::Server(EventLoop *loop) : loop(loop) {
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    serv_sock->listen();
    serv_sock->setnonblocking();

    Channel *servChannel = new Channel(loop, serv_sock->getFd());
    std::function<void()> cb = std::bind(&Server::newConnection, this, serv_sock);
    servChannel->setCallback(cb);
    servChannel->enableReading();
}

Server::~Server() {}

void Server::hanleReadEvent(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        } else if (read_bytes == 0) { // EOF
            printf("client fd %d disconnected\n", sockfd);
            close(sockfd);
            break;
        } else if (read_bytes == -1 && errno == EINTR) {
            printf("continue reading");
            continue;
        } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            printf("finish reading once, errno: %d\n", errno);
            break;
        }
    }
}

void Server::newConnection(Socket *serv_sock) {
    InetAddress *clnt_addr = new InetAddress();
    Socket clnt_sock(serv_sock->accept(clnt_addr));
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock.getFd(), inet_ntoa(clnt_addr->serv_addr.sin_addr), ntohs(clnt_addr->serv_addr.sin_port));
    clnt_sock.setnonblocking();
    Channel *clntChannel = new Channel(loop, clnt_sock.getFd()); // TODO: memory leak
    std::function<void()> cb = std::bind(&Server::hanleReadEvent, this, clnt_sock.getFd());
    clntChannel->setCallback(cb);
    clntChannel->enableReading();
}
