/*
 * @Author       : foregic
 * @Date         : 2021-12-20 17:24:11
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-01 20:33:10
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

class ThreadPool {
private:
    class Worker {
    public:
        Worker() = delete;
        Worker(ThreadPool *p, int i)
            : pool(p), id(i) {}

        void operator()() {
            while (!pool->shutdown) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(pool->mx);
                    pool->condVarConsumer.wait(lock, [&] {
                        printf("worker %d waiting for work\n",  id);
                        // Log::write("worker %d waiting for work", id);
                        return pool->shutdown || !pool->taskQue.empty();
                    }); // wait 直到有 task
                    if (pool->shutdown && pool->taskQue.empty())
                        return;
                    task = std::move(pool->taskQue.front());
                    pool->taskQue.pop();
                }
                printf("worker %d working\n",  id);
                // Log::write("worker %d working", id);
                task();
                pool->condVarProducer.notify_one();
            }
        }

    private:
        std::string time;
        int id;
        ThreadPool *pool;
    };

public:
    ThreadPool() = delete;
    ThreadPool(const ThreadPool &other) = delete;
    ThreadPool(ThreadPool &&other) = delete;
    ThreadPool &operator=(const ThreadPool &other) = delete;
    ThreadPool &operator=(ThreadPool &&other) = delete;

    ThreadPool(const int &num = 4, const int &maxtasks = 1000)
        : threadNum(num), shutdown(false), maxTasks(maxtasks) {}
    ~ThreadPool() {
        shutdown = false;
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void run() {
        for (int i = 0; i < threadNum; i++) {
            threads.emplace_back(std::move(std::thread(Worker(this, i + 1))));
        }
    }

    void stop() {
        shutdown = false;
        condVarConsumer.notify_all();
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
            condVarProducer.wait(lock, [&] { return int(taskQue.size()) < maxTasks; });
            taskQue.emplace([=] {
                (*task)();
            });
            condVarConsumer.notify_one();
        }

        return ret;
    }
    // template <typename F>
    // auto submit(F &&f)
    //     -> std::future<decltype(f)> {

    //     // std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Arg>(args)...);
    //     auto task = std::make_shared<std::packaged_task<decltype(f)()>>(f);
    //     auto ret = task->get_future();

    //     {
    //         std::unique_lock<std::mutex> lock(mx);
    //         condVarProducer.wait(lock, [&] { return int(taskQue.size()) < maxTasks; });
    //         taskQue.emplace([=] {
    //             (*task)();
    //         });
    //     }
    //         condVarConsumer.notify_one();

    //     return ret;
    // }

private:
    int maxTasks;
    std::atomic<bool> shutdown;
    int threadNum;
    std::mutex mx;
    std::queue<std::function<void()>> taskQue;
    std::condition_variable condVarProducer, condVarConsumer;
    std::vector<std::thread> threads;
    // static ThreadPool *ThreadPool;
};

class PoolFactory {
public:
    static ThreadPool *create(const int num = 1, const int maxtasks = 1000) {
        return new ThreadPool(num, maxtasks);
    }
};

#endif /* _THREADPOOL_H */
