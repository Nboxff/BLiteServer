#include "Channel.h"
#include "Epoll.h"

Channel::Channel(Epoll *ep, int fd) : ep(ep), fd(fd) {}

void Channel::enableReading() {
    events = EPOLLIN | EPOLLET;
    ep->updateChannel(this);    
}

int Channel::getFd() {
    return this->fd;
}

uint32_t Channel::getEvents() {
    return this->events;
}

bool Channel::getInEpoll() {
    return this->inEpoll;
}

void Channel::setInEpoll(bool _inEpoll) {
    this->inEpoll=  _inEpoll;
}

void Channel::setRevents(uint32_t revents) {
    this->revents = revents;
}

uint32_t Channel::getRevents() {
    return this->revents;
}