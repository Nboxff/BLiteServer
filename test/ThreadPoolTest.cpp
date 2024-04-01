#include <iostream>
#include <cstring>
#include "src/ThreadPool.h"
#include "assert.h"
#include <mutex>
#define N 100000

#define ANSI_GREEN "\033[92m"
#define ANSI_RED "\033[91m"
#define ANSI_RESET "\033[0m"

int g_num = 0; // protected by g_num_mutex
std::mutex g_num_mutex;

void print(const char *s) {
    g_num_mutex.lock();
    g_num++;
    std::cout << s << std::endl;
    g_num_mutex.unlock();
}

void print_world() {
    g_num_mutex.lock();
    g_num++;
    std::cout << "world" << std::endl;
    g_num_mutex.unlock();
}

int main() {
    ThreadPool *thread_pool = new ThreadPool();
    std::function<void()> func = std::bind(print, "hello");
    for (int i = 0; i < N; i++) {
        thread_pool->add(func);
        thread_pool->add(print_world);
    }
    delete thread_pool;
    std::cout << std::endl << "g_num: " << g_num << std::endl;

    assert(g_num == N * 2);
    std::cout << ANSI_GREEN "ThreadPoolTest Passed." ANSI_RESET << std::endl;

    return 0;
}