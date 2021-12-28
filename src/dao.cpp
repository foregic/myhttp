/*
 * @Author       : foregic
 * @Date         : 2021-12-26 18:06:20
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-28 21:35:36
 * @FilePath     : /httpserver/src/dao.cpp
 * @Description  :
 */

#include "../include/dao.h"

ConnPool *ConnPool::connPool = new ConnPool("http", "127.0.0.1", "root", "123456", 3306);

bool Conn::connected() const {
    return conn.connected();
}

void Conn::disconnect() {
    if (conn.connected()) {
        conn.disconnect();
    }
}

void Conn::connect(const char *db, const char *server, const char *user,
                   const char *password, unsigned int port) {
    if (conn.connected()) {
        conn.disconnect();
    }
    conn.connect(db, server, user, password, port);
}

Conn::~Conn() {
    this->disconnect(); // 断开和服务器的连接
}