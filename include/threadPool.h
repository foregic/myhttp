/*
 * @Author       : foregic
 * @Date         : 2021-12-20 17:24:11
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-30 01:20:10
 * @FilePath     : /httpserver/include/threadPool.h
 * @Description  :
 */

#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unistd.h>
#include <utility>

#include "log.h"

class threadPool {
private:
    class Worker {
    public:
        Worker() = delete;
        Worker(threadPool *p, int i)
            : pool(p), id(i) {}

        void operator()() {
            while (!pool->shutdown) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(pool->mx);
                    pool->cv_consumer.wait(lock, [&] {
                        // printf("[%s]\tworker %d waiting for work\n", getTime(), id);
                        // Log::write("worker %d waiting for work", id);
                        return pool->shutdown || !pool->taskQue.empty();
                    }); // wait 直到有 task
                    if (pool->shutdown && pool->taskQue.empty())
                        return;
                    task = std::move(pool->taskQue.front());
                    pool->taskQue.pop();
                }
                // printf("[%s]\tworker %d working\n", getTime(), id);
                // Log::write("worker %d working", id);
                task();
            }
        }

    private:
        std::string time;
        int id;
        threadPool *pool;
    };

public:
    threadPool() = delete;
    threadPool(const threadPool &other) = delete;
    threadPool(threadPool &&other) = delete;
    threadPool &operator=(const threadPool &other) = delete;
    threadPool &operator=(threadPool &&other) = delete;

    threadPool(const int &num = 4, const int &maxtasks = 1000)
        : threadNum(num), shutdown(false), maxTasks(maxtasks) {}
    ~threadPool() {
        shutdown = false;
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void run() {
        for (size_t i = 0; i < threadNum; i++) {
            threads.emplace_back(std::move(std::thread(Worker(this, i + 1))));
        }
    }

    void stop() {
        shutdown = false;
        cv_consumer.notify_all();
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    // todo 动态修改线程池大小
    bool increasePoolSize(int num) { return false; }
    bool decreasePoolSize(int num) { return false; }

public:
    template <typename F, typename... Arg>
    auto submit(F &&f, Arg &&...args)
        -> std::future<decltype(f(args...))> {

        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Arg>(args)...);
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        auto ret = task->get_future();

        {
            std::unique_lock<std::mutex> lock(mx);
            cv_producer.wait(lock, [&] { return taskQue.size() < maxTasks; });
            taskQue.emplace([=] {
                (*task)();
            });
            cv_consumer.notify_one();
        }

        return ret;
    }

private:
    int maxTasks;
    std::atomic<bool> shutdown;
    int threadNum;
    std::mutex mx;
    std::queue<std::function<void()>> taskQue;
    std::condition_variable cv_producer, cv_consumer;
    std::vector<std::thread> threads;
    // static threadPool *threadPool;
};

class PoolFactory {
public:
    static threadPool *create(const int num = 1, const int maxtasks = 1000) {
        return new threadPool(num, maxtasks);
    }
};

#endif /* _THREADPOOL_H */
