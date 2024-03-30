#include "Connection.h"
#include "Channel.h"
#include "Socket.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "util.h"
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
    readBuffer = new Buffer();
}

Connection::~Connection() {
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::echo(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            readBuffer->append(buf, read_bytes);
        } else if (read_bytes == 0) { // EOF
            printf("client fd %d disconnected\n", sockfd);
            deleteConnectionCallback(sock);
            break;
        } else if (read_bytes == -1 && errno == EINTR) {
            printf("continue reading");
            continue;
        } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) { // all data has been read
            printf("message from client fd %d: %s\n", sockfd, readBuffer->c_str());
            errif(write(sockfd, readBuffer->c_str(), readBuffer->size()) == -1, "socket write error");
            readBuffer->clear(); // clear the buffer
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb) {
    this->deleteConnectionCallback = _cb;
}
