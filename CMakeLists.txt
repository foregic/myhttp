cmake_minimum_required(VERSION 3.0)

project(MYHTTP)

set(CMAKE_CXX_STANDARD 17) # 设置C++编译版本

SET(CMAKE_CXX_COMPILER "g++")         #设置编译器（可选的有gcc，g++）

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -W -pthread -llua -ldl ") 

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}) # 设定编译出来的可执行二进制文件

set(CMAKE_BUILD_TYPE Release) # 设定编译模式，debug模式

file(GLOB_RECURSE native_srcs src/*.cpp)


# include_directories(/usr/include/lua) # 设定需要添加的头文件
# link_directories( ~/lua-5.4.3/install/lib/liblua.a ) # 添加需要连接库文件目录



include_directories(${CMAKE_SOURCE_DIR}/include ) # 设定

add_executable(main  main.cpp ${native_srcs})

# target_link_libraries(main /usr/lib/lua/liblua.a) # 显式指定链接静态库
# find_package(pthread.h) # 引入外部依赖包

# target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT}) # 链接 Thread 库