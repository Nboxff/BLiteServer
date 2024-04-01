#pragma once
#include <sys/epoll.h>
#include <functional>
class EventLoop;

class Channel {
private:
    EventLoop *loop;
    int fd;

    // what events want to listen for this file descriptor
    uint32_t events;
    uint32_t revents;
    bool inEpoll; 

    std::function<void()> callback;
public:
    Channel(EventLoop *loop, int fd);
    void enableReading();

    int getFd();
    uint32_t getEvents();
    void setRevents(uint32_t revents);
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll(bool _inEpoll = true);

    void setCallback(std::function<void()>);
    void handleEvent();
};