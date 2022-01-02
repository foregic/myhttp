/*
 * @Author       : foregic
 * @Date         : 2021-12-28 13:50:06
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-01 01:23:36
 * @FilePath     : /httpserver/include/log.h
 * @Description  :
 */

#ifndef _LOG_H
#define _LOG_H

#include <atomic>
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
#include <unordered_map>

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
        }
        ~Printer() {
            if (log->isclose_) {
            }
        }
        void operator()() {
            log->logFile << "start";
            printf("start\n");
            while (1) {

                std::unique_lock<std::mutex> lock(log->mx);
                log->condVarConsumer.wait(lock, [&] { return log->isclose_ || log->logs.size() > 0; });
                if (log->isclose_) {
                    break;
                }
                auto info = log->logs.front();
                log->logs.pop();
                lock.unlock();

                char buffer[1024];
                memset(buffer, 0, 1024);
                sprintf(buffer, "[%s]\t%s\n", getTime(), info.c_str());
                // if (log->logFile.is_open()) {
                //     printf("文件已打开");
                // } else {
                //     printf("文件已关闭");
                // }
                // printf("输出信息:%s\n", buffer);
                // log->logFile << buffer;
                log->logFile.write(buffer, strlen(buffer));
                // log->logFile.flush();
                // printf("写入信息完毕\n");
                log->condVarProducer.notify_one();
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
        logFile.close();
        post();
        if (printThread.joinable()) {
            printThread.join();
        }
    }

private:
    Log(const int size) : isclose_(false), maxSize(size) {

        logFile.open("log", std::ios::binary);
        while (!logFile) {
            perror("log start failed");
            exit(0);
        }
        run();
    }

public:
    bool full() {
        return int(logs.size()) == maxSize;
    }
    static Log *getInstance() {
        return Mylog;
    }

    void run() {
        printThread = std::thread(Printer(this));
    }

    void post() {
        condVarConsumer.notify_one();
    }

    static void write(const std::string &str) {
        Mylog->put(str);
    }
    static void write(const char *str) {
        Mylog->put(str);
    }
    template <typename... Args>
    static void write(const char *__fmt, Args... args) {
        Mylog->put(__fmt, args...);
    }

    template <typename... Args>
    static void write(int tpye, const char *__fmt, Args... args) {
        switch (tpye) {
        case 0:
            char ch[1024] = "[debug]";
            strcat(ch, __fmt);
            write(ch, args...);
            break;
        }
        Mylog->put(__fmt, args...);
    }

    template <typename... Args>
    void put(const char *ch, Args... args) {
        char buffer[1024];
        sprintf(buffer, ch, args...);
        std::unique_lock<std::mutex> lock(mx);
        condVarProducer.wait(lock, [&] { return int(logs.size()) < maxSize; });
        logs.emplace(buffer);
        post();
    }

    void put(const char *ch) {
        std::unique_lock<std::mutex> lock(mx);
        condVarProducer.wait(lock, [&] { return int(logs.size()) < maxSize; });
        logs.emplace(ch);
        post();
    }

    void put(const std::string &str) {
        std::unique_lock<std::mutex> lock(mx);
        condVarProducer.wait(lock, [&] { return int(logs.size()) < maxSize; });
        logs.emplace(str);
        post();
    }

private:
    std::atomic_bool isclose_;
    std::thread printThread;
    Block<std::string> logs;
    std::mutex mx;
    std::condition_variable condVarProducer, condVarConsumer;
    int maxSize;

public:
    std::ofstream logFile;
    static Log *Mylog;

    // todo 添加信息标识debug、info、warning等
    // static std::unordered_map<int, std::string> info;
};

#endif /* _LOG_H */
