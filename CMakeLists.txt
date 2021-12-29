cmake_minimum_required(VERSION 3.0)

project(MYHTTP)

set(CMAKE_CXX_STANDARD 17) # 设置C++编译版本

SET(CMAKE_CXX_COMPILER "g++")         #设置编译器（可选的有gcc，g++）

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall ") 

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}) # 设定编译出来的可执行二进制文件

set(CMAKE_BUILD_TYPE Debug) # 设定编译模式，debug模式

file(GLOB_RECURSE native_srcs src/*.cpp)

add_library(MD  SHARED
 lib/md5/src/md5.cpp)

 target_include_directories(MD
 PUBLIC 
     ${PROJECT_SOURCE_DIR}/lib
)

include_directories(
    lib/lua-5.4.3/install/include
    lib/md5
)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib/lua-5.4.3/install/lib
    
)


# link_libraries( ${CMAKE_SOURCE_DIR}/lib/lua-5.4.3/install/lib/liblua.a 
# ) # 添加需要连接库文件目录

# target_link_libraries(MYHTTP mysqlpp)

include_directories(${CMAKE_SOURCE_DIR}/include ) # 设定

add_executable(main  main.cpp ${native_srcs})

# target_link_libraries(main /usr/lib/lua/liblua.a) # 显式指定链接静态库
# find_package(pthread.h) # 引入外部依赖包

target_link_libraries (
    main 
    MD 
    mysqlpp 
    pthread 
    liblua.a
    ${CMAKE_DL_LIBS}
) # 链接


# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
# aux_source_directory(. DIR_LIB_SRCS)

# 生成链接库
# add_library (MathFunctions ${DIR_LIB_SRCS})

# 添加 math 子目录
# add_subdirectory(math)


# 指定生成目标 
# add_executable(Demo main.cc)

# 添加链接库
# target_link_libraries(Demo MathFunctions)

