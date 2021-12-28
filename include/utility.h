/*
 * @Author       : foregic
 * @Date         : 2021-12-28 13:50:06
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-28 14:24:53
 * @FilePath     : /httpserver/include/utility.h
 * @Description  :
 */

#ifndef _UTILITY_H
#define _UTILITY_H

#include <ctime>
#include <queue>
#include <string>

#include <condition_variable>
#include <mutex>
#include <semaphore.h>

class Log {

    class Print {
        Print() = delete;
        Print(Log *_log) : log(_log) {}
        ~Print() = default;

        void operator()() {
            std::string time;
            while (true) {
                sem_wait(&log->sem);
                std::time_t result = std::time(NULL);
                time = std::move(asctime(std::localtime(&result)));
                time.pop_back();
                printf("[%s] %s\n", time.c_str(), log->logs.front().data());
                log->logs.pop();
            }
        }
        Log *log;
    };

public:
    Log() = delete;
    ~Log() {
        sem_destroy(&sem);
    }

    Log(const int size) {

        Log::maxSize = size;
        sem_init(&sem, 0, size);
    }

    template <typename T, typename... Arg>
    static void print(T t, Arg... args) {
        std::cout << t << ' ';
        print(args...);
    }

    static void print() {
        return;
    }

    static bool full() {
        return logs.size() >= maxSize;
    }

    static void post() {
        if (full()) {
            sem_post(&sem);
        }
    }

    template <typename T>
    static void put(T &&t) {
        logs.emplace(t);
        post();
    }

    static void put(const char *ch) {
        logs.emplace(ch);
        post();
    }

    static void put(std::string &str) {
        logs.emplace(str);
        post();
    }

private:
    static std::queue<std::string> logs;
    static sem_t sem;
    // static std::mutex;
    // static std::condition_variable cv;
    static int maxSize;
};

#endif /* _UTILITY_H */
