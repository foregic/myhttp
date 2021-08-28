### 个人简单HTTP服务器
> 主要参考了[Tinyhttpd](https://github.com/EZLippi/Tinyhttpd)
>
> 


 

#### 项目结构

```
httpserver
├── CMakeLists.txt
├── README.md
├── bin
│   ├── main
│   └── make.sh
├── include
│   ├── header.h
│   ├── http.h
│   └── utils.h
├── resources
│   ├── file
│   ├── http
│   │   ├── 404.html
│   │   ├── 500.html
│   │   ├── 501.html
│   │   └── index.html
│   └── img
├── src
│   ├── http.cpp
│   └── utils.cpp
└── main.cpp
```

|文件名|内容|
|:--|:--|
|CMakeLists.txt|编译配置|
|bin|编译生成的二进制可执行文件|
|resources|资源文件|
|include|头文件|
|src|源文件|
|http|返回状态码以及对应的html文件|
|utils|工具|
|mian.cpp|主函数|


##### 运行环境
```
WSL Ubuntu 20.04
g++ 9.3.0
cmake 3.16.3
make 4.2.1
```

#### 编译及运行
```shell
$ cd ./bin
$ sh make.sh
$ ./main 12100 # 设置运行端口号为12100
```

#### TODO
1. 绑定服务器和主页
1. 添加更多的网页内容
2. 增加用户注册功能
3. 增加文件系统
4. 增加Redis和MySQL链接功能


#### 1.0
功能：简单的服务器访问，目前仅能通过ip地址+端口号访问
访问解析有点问题，不能返回index.html

