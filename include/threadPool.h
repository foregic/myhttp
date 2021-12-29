/*
 * @Author       : foregic
 * @Date         : 2021-12-20 17:24:11
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-29 23:33:01
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

template <typename T>
class Task {
public:
    Task() {}
    Task(Task &&) {}
    ~Task() {}

    bool empty() noexcept {
        std::unique_lock<std::mutex> lock(mx);
        return que.empty();
    }

    size_t size() noexcept {
        std::unique_lock<std::mutex> lock(mx);
        return que.size();
    }

    void emplace(T &t) noexcept {
        std::unique_lock<std::mutex> lock(mx);
        que.emplace(t);
    }

    void emplace(T &&t) noexcept {
        std::unique_lock<std::mutex> lock(mx);
        que.emplace(std::forward<T>(t));
    }

    bool pop(T &t) noexcept {
        if (empty()) {
            return false;
        }
        std::unique_lock<std::mutex> lock(mx);
        t = std::move(que.front());

        que.pop();
        return true;
    }

private:
    std::mutex mx;
    std::queue<T> que;
};

class threadPool {
private:
    class Worker {
    public:
        Worker() = delete;
        Worker(threadPool *p, int i)
            : pool(p), id(i) {}

        void operator()() {
            std::function<void()> func;
            bool popResult;

            while (!pool->shutdown) {
                std::unique_lock<std::mutex> lock(pool->mx);

                pool->cv_consumer.wait(lock, [&] { 
                    printf("[%s]\tworker %d waiting for work\n",getTime(),id); 
                    Log::write("worker %d waiting for work\n",id);
                    
                    return pool->taskQue.size() > 0; });

                popResult = pool->taskQue.pop(func);
                if (popResult) {
                    try {
                        pool->cv_producer.notify_one();
                        printf("[%s]\tworker %d working\n", getTime(), id);
                        Log::write("worker %d working\n", id);
                        func();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
        }

        const char *getTime() {
            std::time_t result = std::time(NULL);
            time = std::move(asctime(std::localtime(&result)));
            time.pop_back();
            return time.c_str();
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
        : threadNum(num), threads(std::vector<std::thread>(num)), shutdown(false), maxTasks(maxtasks) {}
    ~threadPool() {
        shutdown = false;
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void run() {
        for (size_t i = 0; i < threads.size(); i++) {
            threads[i] = std::move(std::thread(Worker(this, i + 1)));
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
    void submit(F &&f, Arg &&...args)
    // -> std::future<decltype(f(args...))>
    {

        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Arg>(args)...);
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        auto ret = task->get_future();

        // if (shutdown.load(std::memory_order_acquire)) {
        //     throw std::runtime_error("thread pool has shutdown");
        // }
        // taskQue.push(std::move(func));
        // printf("push task\n");
        std::function<void()>
            warpper_func = [&] {
                (*task)();
            };
        std::unique_lock<std::mutex> lock(mx);
        cv_producer.wait(lock, [&] { return taskQue.size() < maxTasks; });
        {

            taskQue.emplace(std::move(warpper_func));
        }
        cv_consumer.notify_one();

        // usleep(1000);

        // return ret;
    }

private:
    int maxTasks;
    std::atomic<bool> shutdown;
    int threadNum;
    std::mutex mx;
    Task<std::function<void()>> taskQue;
    std::condition_variable cv_producer, cv_consumer;
    std::vector<std::thread> threads;
};

class PoolFactory {
public:
    static threadPool *create(const int &num = 4, const int &maxtasks = 1000) {
        return new threadPool(num, maxtasks);
    }
};

#endif /* _THREADPOOL_H */
