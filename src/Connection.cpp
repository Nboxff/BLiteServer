#include "Connection.h"
#include "Channel.h"
#include "Socket.h"
#include "EventLoop.h"
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <unistd.h>

#define READ_BUFFER 1024

Connection::Connection(EventLoop* loop, Socket* sock) : loop(loop), sock(sock), channel(nullptr) {
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
}

Connection::~Connection() {
    delete channel;
    delete sock;
}

void Connection::echo(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        } else if (read_bytes == 0) { // EOF
            printf("client fd %d disconnected\n", sockfd);
            deleteConnectionCallback(sock);
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

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb) {
    this->deleteConnectionCallback = _cb;
}
