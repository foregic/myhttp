

#ifndef MYHTTP_THREADPOOL_H
#define MYHTTP_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

template <typename T>
class Task {
public:
    Task() {}
    Task(Task &&) {}
    ~Task() {}

    bool empty() const noexcept {
        std::unique_lock<std::mutex> lock(mx);
        return que.empty();
    }

    size_t size() const noexcept {
        std::unique_lock<std::mutex> lock(mx);
        return que.size();
    }

    void push(T &t) {
        std::unique_lock<std::mutex> lock(mx);
        que.emplace(t);
    }

    bool pop(T &t) {
        if (empty()) {
            return false;
        }
        std::unique_lock<std::mutex> lock(mx);
        t = std::move(que.front());
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
        Worker(threadPool *p) : pool(p) {}

        void operator()() {
            std::function<void()> func;
            bool popResult;

            while (!pool->shutdown) {
                std::unique_lock<std::mutex> lock(pool->mx);

                pool->cv.wait(lock, [&] { return pool->taskQue.size() > 0; });

                popResult = pool->taskQue.pop(func);
                if (popResult) {
                    func();
                }
            }
        }

    private:
        threadPool *pool;
    };

public:
    threadPool(const int num = 4) : threadNum(num), threads(num), shutdown(false) {}
    threadPool(const threadPool &other) = delete;
    threadPool(threadPool &&other) = delete;
    threadPool &operator=(threadPool &&other) = delete;
    threadPool &operator=(const threadPool &other) = delete;
    ~threadPool() {
        shutdown = false;
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void run() {
        for (auto &thread : threads) {
            thread = std::thread(Worker(this));
        }
    }

    void stop() {
        shutdown = false;
        cv.notify_all();
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
    auto submit(F &&f, Arg &&...args) -> std::future<decltype(f(args...))> {
        auto func = std::bind(std::forward<F>(f), std::move(args)...);
        auto task = std::make_unique<std::packaged_task<decltype(f(args...))()>>(func);
        auto ret = task->get_future();
        if (shutdown.load(std::memory_order_acquire)) {
            throw std::runtime_error("thread pool has shutdown");
        }
        taskQue.push([&] { task->operator(); });
        cv.notify_one();
        return ret;
    }

private:
    std::atomic<bool> shutdown;
    int threadNum;
    std::mutex mx;
    Task<std::function<void()>> taskQue;
    std::condition_variable cv;
    std::vector<std::thread> threads;
};

#endif // MYHTTP_THREADPOOL_H
