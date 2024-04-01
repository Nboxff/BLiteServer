#pragma once

#include <sys/epoll.h>
#include <vector>

class Channel;
class Epoll {
private:
    int epfd;
    struct epoll_event *events;
public:
    Epoll();
    ~Epoll();
    void addFd(int fd, uint32_t op);
    void updateChannel(Channel *channel);
    std::vector<epoll_event> poll_event(int timeout = -1);
    std::vector<Channel*> poll(int timeout = -1);
};