/*
 * @Author       : foregic
 * @Date         : 2021-12-29 20:32:02
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-30 01:48:11
 * @FilePath     : /httpserver/src/log.cpp
 * @Description  :
 */
#include "../include/log.h"

std::ofstream Log::Printer::of("log", std::ios::binary);

Log *Log::Mylog = new Log(1000);