#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *loop, int fd) : loop(loop), fd(fd), inEpoll(false) {}

void Channel::enableReading() {
    events = EPOLLIN | EPOLLET;
    loop->updateChannel(this);    
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

void Channel::setCallback(std::function<void()> _callback) {
    this->callback = _callback;
}

void Channel::handleEvent() {
    loop->addThread(callback);
}

void Channel::setRevents(uint32_t revents) {
    this->revents = revents;
}

uint32_t Channel::getRevents() {
    return this->revents;
}