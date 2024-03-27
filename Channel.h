#pragma once
#include <sys/epoll.h>

class Epoll;
class Channel {
private:
    Epoll *ep;
    int fd;

    // what events want to listen for this file descriptor
    uint32_t events;
    uint32_t revents;
    bool inEpoll; 

public:
    Channel(Epoll *ep, int fd);
    void enableReading();
    int getFd();
    uint32_t getEvents();
    void setRevents(uint32_t revents);
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll(bool _inEpoll = true);
};