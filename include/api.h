/*
 * @Author       : foregic
 * @Date         : 2021-12-28 22:11:11
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-29 15:29:04
 * @FilePath     : /httpserver/include/api.h
 * @Description  :
 */

#ifndef _API_H
#define _API_H

#include "../lib/md5/md5.h"
#include "dao.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

class Api {
public:
    static void registers(const std::string &username, const std::string &passwd) {
        Query query;

        char buffer[1024];

        sprintf(buffer, "insert user value(null, %s, %s)", username.c_str(), passwd.c_str());
        query.update(string(buffer));
    }

    static bool login_authorization(const std::string &username, const std::string &passwd) {
        Query query;

        char buffer[1024];

        sprintf(buffer, "select passwd from user where username = %s", username.c_str());
        auto result = query.search(string(buffer));

        if (passwd != result[0][0].data()) {
            return false;
        }
        return true;
    }

    /**
     * @description  : 根据传来的字符串进行MD5加密
     * @param         {string} &str
     * @return        {*}
     */
    static std::string gethash(const std::string &str) {
        return MD5(str).toStr();
    }
};

#endif /* _API_H */
