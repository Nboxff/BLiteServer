#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mtx; // mutex of task queue
    std::condition_variable cv;
    bool stop;
public:
    ThreadPool(int size = 10);
    ~ThreadPool();
    void add(std::function<void()>);
};