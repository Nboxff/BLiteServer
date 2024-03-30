#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include <vector>

EventLoop::EventLoop() {
    ep = new Epoll();
    threadPool = new ThreadPool();
}

EventLoop::~EventLoop() {
    delete ep;
    delete threadPool;
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

void EventLoop::addThread(std::function<void()> func) {
    threadPool->add(func);
}
