### 个人简单HTTP服务器

#### 项目结构

```
httpserver
├── CMakeLists.txt
├── config.json
├── README.md
├── bin
│   ├── main
├── include
│   ├── lua-5.4.3
│   ├── luascript.h
│   ├── server.h
│   ├── sql.h
│   └── threadPool.h
├── resources
│   ├── file
│   ├── http
│   │   ├── 404.html
│   │   ├── 500.html
│   │   ├── 501.html
│   │   └── index.html
│   └── img
│       ├── epoll测试1.png
│       └── epoll测试2.png
├── script
│   └── post.lua
├── src
│   ├── http.cpp
│   ├── server.cpp
│   └── threadPool.cpp
└── main.cpp
└── make.sh
```

|文件名|内容|
|:--|:--|
|CMakeLists.txt|编译配置|
|bin|编译生成的二进制可执行文件|
|resources|资源文件|
|include|头文件|
|script|脚本文件夹|
|include/lua-5.4.3|lua源文件、头文件、静态库文件|
|src|源文件|
|http.cpp|返回状态码以及对应的html文件|
|mian.cpp|主函数|
|config.json|配置文件|


##### 运行环境
```
WSL2 Ubuntu 20.04
g++ 9.3.0
cmake 3.16.3
make 4.2.1
lua 5.4.3
```

#### 编译及运行
```shell
$ sh make.sh
```



### 1.3
增加了lua脚本

script 文件夹存放脚本文件


### 1.2
增加了线程池


#### 1.1
增加了epoll多路复用，可以正常返回index.html
性能测试：
![](https://github.com/foregic/myhttp/blob/main/resources/img/epoll%E6%B5%8B%E8%AF%951.png)

![](https://github.com/foregic/myhttp/blob/main/resources/img/epoll%E6%B5%8B%E8%AF%952.png)

#### 1.0
功能：简单的服务器访问，目前仅能通过ip地址+端口号访问
访问解析有点问题，不能返回index.html




