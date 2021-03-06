/*
 * @Author       : foregic
 * @Date         : 2021-12-24 12:57:42
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-01 11:59:54
 * @FilePath     : /httpserver/include/dao.h
 * @Description  :
 */

#ifndef _DAO_H
#define _DAO_H

#define MYSQLPP_MYSQL_HEADERS_BURIED // 使用mysqlpp库

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <mysql++/mysql++.h>

class Conn {
    using Connection = mysqlpp::Connection;
    using Driver = mysqlpp::DBDriver;
    using Query = mysqlpp::Query;

public:
    Conn();
    Conn(const std::string &db, const std::string &server, const std::string &user,
         const std::string &password, unsigned int &port)
        : conn(Connection(db.c_str(), server.c_str(), user.c_str(), password.c_str(), port)) {}
    ~Conn();

    virtual void connect(const char *db, const char *server, const char *user,
                         const char *password, unsigned int port);

    void disconnect();

    bool connected() const;

    bool createDb(const std::string &str) {
        return conn.create_db(str);
    }
    bool selectDb(const std::string &str) {
        return conn.select_db(str);
    }
    Driver *driver() {
        return conn.driver();
    }
    Query acquireQuery() { // 获得query对象
        return conn.query();
    }

private:
    Connection conn;
};

class ConnPool {
public:
    ConnPool() = delete;
    ConnPool(const ConnPool &) = delete;
    ConnPool(ConnPool &&) = delete;
    ConnPool &operator=(const ConnPool &) = delete;
    ConnPool &operator=(ConnPool &&) = delete;

private:
    ConnPool(const char *db, const char *server = 0, const char *user = 0,
             const char *password = 0, unsigned int port = 0, int maxSize = 20) //构造方法
        : db(db), server(server), user(user), password(password), port(port), maxSize(maxSize) {
        std::unique_lock<std::mutex> lock(mx);
        if (connPool != nullptr) {
            throw std::runtime_error("connect pool has built");
        }
    }

public:
    ~ConnPool() {
        this->destoryConnectionPool();
    }

    void destoryConnectionPool() {

        for (auto &connection : conns) {
            connection.disconnect();
        }
    }

    static ConnPool *getInstance() { return connPool; }

    Conn createConnection() {
        return Conn(db, server, user, password, port);
    }

    Conn getConnection() {
        std::unique_lock<std::mutex> lock(mx);
        if (conns.size() > 0) {
            auto con = conns.front();

            conns.pop_front();
            if (!con.connected()) {

                con = this->createConnection();
            }
            return con;

        } else {

            if (curSize < maxSize) {
                auto con = this->createConnection();
                if (con.connected()) {
                    ++curSize;
                    return con;
                } else {
                    throw std::runtime_error("build connection failed");
                }
            } else {
                // 建立的连接数已经达到maxSize
                cv.wait(lock, [&] { return conns.size() > 0; });
                auto con = conns.front();
                return con;
            }
        }
    }

    void releaseConnect(Conn &conn) {
        std::unique_lock<std::mutex> lock(mx);
        conns.push_back(conn);
        cv.notify_one();
    }

private:
    std::condition_variable cv;
    std::string db, server, user, password;
    unsigned int port;
    int curSize, maxSize;
    std::mutex mx;
    std::list<Conn> conns;
    static ConnPool *connPool;
};

class Query {
    using dbQuery = mysqlpp::Query;
    using Row = mysqlpp::Row;

public:
    Query() {
    }
    ~Query() {}

    /**
     * @description  : 传入要执行的语句，执行成功结果为真
     * @param         {*}
     * @return        {*}
     * 删,增,改
     */
    bool update(const std::string &str) {
        ConnPool *connPool = ConnPool::getInstance();
        Conn conn = connPool->getConnection();
        dbQuery query = conn.acquireQuery();
        query << str;
        auto result = query.exec();
        connPool->releaseConnect(conn);
        return result;
    }

    /**
     * @description  : 传入要执行的查询语句，如果未执行成功，则结果为{}
     * @param         {*}
     * @return        {*}
     */
    // 查
    std::vector<Row> search(const std::string &str) {
        ConnPool *connPool = ConnPool::getInstance();
        Conn conn = connPool->getConnection();
        dbQuery query = conn.acquireQuery();

        query << str;
        std::vector<Row> result;
        if (mysqlpp::StoreQueryResult res = query.store()) {
            result = res;
        } else {
            return {};
        }
        connPool->releaseConnect(conn);
        return result;
    }

private:
};

#endif /* _DAO_H */
