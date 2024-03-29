#include "Epoll.h"
#include "util.h"
#include "Channel.h"
#include <unistd.h>
#include <cstring>
#include <cstdio>

#define MAX_EVENTS 1024
Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");
    events = new epoll_event[MAX_EVENTS];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete [] events;
}

void Epoll::addFd(int fd, uint32_t op) {
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;
    errif(epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev) == -1, 
        "epoll add event error");
}

void Epoll::updateChannel(Channel *channel) {
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if (!channel->getInEpoll()) {
        errif(epoll_ctl(epfd,EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->setInEpoll(true);
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
    }
}

std::vector<epoll_event> Epoll::poll_event(int timeout) {
    int nfds = epoll_wait(this->epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    std::vector<epoll_event> activeEvents;
    for (int i = 0; i < nfds; i++) {
        activeEvents.push_back(events[i]);
    }
    return activeEvents;
}

std::vector<Channel*> Epoll::poll(int timeout) {
    int nfds = epoll_wait(this->epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    std::vector<Channel*> activeChannels;
    for (int i = 0; i < nfds; i++) {
        Channel* channel = (Channel*) events[i].data.ptr;
        channel->setRevents(events[i].events);
        activeChannels.push_back(channel);
    }
    return activeChannels;
}