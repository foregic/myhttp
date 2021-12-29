/*
 * @Author       : foregic
 * @Date         : 2021-12-28 13:50:06
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-29 23:07:28
 * @FilePath     : /httpserver/include/log.h
 * @Description  :
 */

#ifndef _LOG_H
#define _LOG_H

#include <condition_variable>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <mutex>
#include <queue>
#include <semaphore.h>
#include <string>
#include <thread>

template <typename T>
class Block {
public:
    Block() {}

    T &front() {
        std::unique_lock<std::mutex> lock(mx);
        return queue.front();
    }
    void pop() {
        std::unique_lock<std::mutex> lock(mx);
        queue.pop();
    }
    void emplace(const T &t) {
        std::unique_lock<std::mutex> lock(mx);
        queue.emplace(t);
    }
    void emplace(T &&t) {
        std::unique_lock<std::mutex> lock(mx);
        queue.emplace(std::forward<T>(t));
    }
    size_t size() {
        std::unique_lock<std::mutex> lock(mx);
        return queue.size();
    }

private:
    std::mutex mx;
    std::queue<T> queue;
};

class Log {
private:
    class Printer {
    public:
        Printer() = delete;
        Printer(Log *_log) : log(_log) {
            if (!of)
                throw std::runtime_error("Log start falied");
        }
        ~Printer() {
            if (log->isclose_) {
                of.close();
            }
        }
        void operator()() {

            while (!log->isclose_) {
                std::unique_lock<std::mutex> lock(log->mx);
                log->cv_consumer.wait(lock, [&] { return log->logs.size() > 0; });

                char buffer[1024];
                memset(buffer, 0, 1024);
                sprintf(buffer, "[%s]\t%s\n", getTime(), log->logs.front().data());
                of.write(buffer, strlen(buffer));
                log->logs.pop();
                log->cv_producer.notify_one();
            }
        }
        const char *getTime() {

            std::time_t result = std::time(NULL);
            time = std::string(asctime(std::localtime(&result)));
            time.pop_back();
            return time.c_str();
        }

    private:
        std::string time;
        static std::ofstream of;
        Log *log;
    };

public:
    Log() = delete;
    Log(const Log &) = delete;
    Log(Log &&) = delete;
    Log &operator=(const Log &) = delete;
    Log &operator=(Log &&) = delete;
    ~Log() {
        isclose_ = true;
        post();
        if (printThread.joinable()) {
            printThread.join();
        }
    }

    Log(const int size = 1000) {
        isclose_ = false;
        maxSize = size;
    }

    bool full() {
        return logs.size() == maxSize;
    }
    static Log *getInstance() {
        return Mylog;
    }

    void run() {
        printThread = std::move(std::thread(Printer(this)));
    }

    void post() {
        cv_consumer.notify_one();
    }

    static void write(const std::string &str) {
        Log *log = Log::getInstance();
        log->put(str);
    }
    static void write(const char *str) {
        Log *log = Log::getInstance();
        log->put(str);
    }
    template <typename... Args>
    static void write(const char *__fmt, Args... args) {
        Log *log = Log::getInstance();
        log->put(__fmt, args...);
    }

    template <typename... Args>
    void put(const char *ch, Args... args) {
        std::unique_lock<std::mutex> lock(mx);
        cv_producer.wait(lock, [&] { return int(logs.size()) < maxSize; });
        char buffer[1024];
        sprintf(buffer, ch, args...);
        logs.emplace(ch);
        post();
    }

    void put(const char *ch) {
        std::unique_lock<std::mutex> lock(mx);
        cv_producer.wait(lock, [&] { return int(logs.size()) < maxSize; });
        logs.emplace(ch);
        post();
    }

    void put(const std::string &str) {
        std::unique_lock<std::mutex> lock(mx);
        cv_producer.wait(lock, [&] { return int(logs.size()) < maxSize; });
        logs.emplace(str);
        post();
    }

private:
    bool isclose_;
    std::thread printThread;
    Block<std::string> logs;
    std::mutex mx;
    std::condition_variable cv_producer, cv_consumer;
    int maxSize;
    static Log *Mylog;
};

#endif /* _LOG_H */
