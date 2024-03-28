#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"

#include <vector>
EventLoop::EventLoop() {
    ep = new Epoll();
}

EventLoop::~EventLoop() {
    delete ep;
}

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel*> chs;
        chs = ep->poll();
        for (Channel* channel : chs) {
            channel->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel * ch) {
    ep->updateChannel(ch);
}
