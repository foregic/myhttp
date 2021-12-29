/*
 * @Author       : foregic
 * @Date         : 2021-12-29 20:32:02
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-29 23:06:26
 * @FilePath     : /httpserver/src/log.cpp
 * @Description  :
 */
#include "../include/log.h"
std::ofstream Log::Printer::of("log.txt", std::ios::binary | std::ios::app);
Log *Log::Mylog = new Log(1000);