#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "util.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"

void handleEvent(int clnt_sockfd);

int main() {
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    serv_sock->listen();
    
    Epoll *ep = new Epoll();
    ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET);

    InetAddress *clnt_addr = new InetAddress();

    while (true) {
        std::vector<epoll_event> events = ep->poll();
        for (epoll_event event : events) {
            if (event.data.fd == serv_sock->getFd()) {
                Socket clnt_sock(serv_sock->accept(clnt_addr));
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock.getFd(), inet_ntoa(clnt_addr->serv_addr.sin_addr), ntohs(clnt_addr->serv_addr.sin_port));

                clnt_sock.setnonblocking();
                ep->addFd(clnt_sock.getFd(), EPOLLIN | EPOLLET);
            } else if (event.events & EPOLLIN) {// readable event
                handleEvent(event.data.fd);
            } else {
                printf("something else happened\n");
            }
        }
    }
    
    delete serv_sock, serv_addr, clnt_addr;
    return 0;
}

void handleEvent(int clnt_sockfd) {
    while (true) {
        char buf[1024];
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(clnt_sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", clnt_sockfd, buf);
            write(clnt_sockfd, buf, sizeof(buf));
        } else if (read_bytes == 0) { // EOF
            printf("client fd %d disconnected\n", clnt_sockfd);
            close(clnt_sockfd);
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