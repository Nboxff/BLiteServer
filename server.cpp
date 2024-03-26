#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "util.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

const int MAX_EVENTS = 1024;

void setnonblocking(int fd){
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void handleEvent(int clnt_sockfd);

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // ipv4
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    errif(bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
    
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");

    struct epoll_event events[MAX_EVENTS], ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == sockfd) {
                int clnt_sockfd = accept(sockfd, (sockaddr *) &clnt_addr, &clnt_addr_len);
                errif(clnt_sockfd == -1, "socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                ev.data.fd = clnt_sockfd;
                ev.events = EPOLLIN | EPOLLET;
                setnonblocking(clnt_sockfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev);
            } else if (events[i].events & EPOLLIN) {// readable event
                handleEvent(events[i].data.fd);
            } else {
                printf("something else happened\n");
            }
        }
    }
    

    close(sockfd);
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