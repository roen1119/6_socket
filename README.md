# mysocket


### 要求

- 自定义协议

  定义包的type、header等

- 使用Socket编程接口

  C语言形式

- 实现网络应用软件

  发送、接受网络数据包

- 传输层：TCP

### 服务器

并发处理多个客户端的请求。请求的类型如下：

socket()

bind() 绑定监听端口5412

listen()，设置连接等待队列长度(MAX_CONNECTION)

循环调用accept()，接收到socket句柄后创建新的客户端项目，而后创建子进程继续调用accept()，send() hello给刚才的客户端。接着，循环调用receive()

- 时间 time(), send()
- 名称 GetComputerName(), send()
- 连接的所有客户端信息 send()
- 转发send()
- 异步多线程

### 客户端

人机交互：命令行；与服务器的通信。以下功能都通过发包、收包的流程实现。

socket()

- connect() 连接

  连接到指定地址和端口的服务器。

  连接成功后，创建子线程循环调用reveive().

- close() 断开连接

- send() 获取时间 - 响应

  请求服务器给出时间

- send() 获取名字 - 响应

  请求服务器给出其机器名称

- send() 活动连接列表 - 响应

  当前连接的所有客户端：编号、IP地址、端口

- send() 发消息

  要求服务器转发消息给对应编号的客户端

- receive() 收消息  - 指示消息

  收到服务器转发的消息后显示在屏幕上

- 退出

  判断是否连接，若是则先断开连接。退出程序。

##### Pipeling

<img src="assets/20190108140458505.png">

##### Server

|      函数      |   描述   |
| :------------: | :------: |
|    socket()    |   创建   |
|     bind()     |   绑定   |
|    listen()    |   监听   |
|    accept()    | 等待连接 |
| read()/write() |          |
|    close()     |          |

##### Client

|      函数      | 描述 |
| :------------: | :--: |
|    socket()    |      |
|   connect()    |      |
| read()/write() |      |
|    close()     |      |





##### socket()

```c++
int socket(int domain,int type,int protocol);
```

返回值，成功返回非负描述符，失败返回-1

e.g.

```c++
socket(AF_INET, SOCK_STREAM, 0); //建立基于IPV4的TCP套接字
```

当我们调用socket创建一个socket时，返回的socket描述字它存在于协议族（address family，AF_XXX）空间中，但**没有一个具体的地址**。

如果想要给它赋值一个地址，就必须调用bind()函数，否则就当调用connect()、listen()时系统会自动随机分配一个端口。



##### addr (IPv4)

```c++
struct sockaddr_in {
    sa_family_t    sin_family; /* address family: AF_INET */
    in_port_t      sin_port;   /* port in network byte order */
    struct in_addr sin_addr;   /* internet address */
};

struct in_addr {
    uint32_t       s_addr;     /* address in network byte order */
};
```

<img src="./assets/struct.png">

实际中使用`sockaddr_in`，在传入函数时转换成`sockaddr`.

##### 主机字节序

就是我们平常说的大端和小端模式：不同的CPU有不同的字节序类型，这些字节序是指整数在内存中保存的顺序，这个叫做主机序。引用标准的Big-Endian和Little-Endian的定义如下：

- Little-Endian就是低位字节排放在内存的低地址端，高位字节排放在内存的高地址端。
- Big-Endian就是高位字节排放在内存的低地址端，低位字节排放在内存的高地址端。

##### 网络字节序

4个字节的32 bit值以下面的次序传输：首先是0～7bit，其次8～15bit，然后16～23bit，最后是24~31bit。这种传输次序称作大端字节序。由于TCP/IP首部中所有的二进制整数在网络中传输时都要求以这种次序，因此它又称作网络字节序。字节序，顾名思义字节的顺序，就是大于一个字节类型的数据在内存中的存放顺序，一个字节的数据没有顺序的问题了。

##### 字节序的转换

将地址绑定到socket的时候，先将主机字节序转换成为网络字节序



##### bind()

```c++
int bind(int sockfd,const struct sockaddr *myaddr,socklen_t addrlen);
```

成功返回0，失败返回-1

bind()函数把一个地址族中的特定地址赋给socket.

服务器在启动的时候都会绑定一个众所周知的地址（如ip地址+端口号），用于提供服务。

> 客户端就不用指定，有系统自动分配一个端口号和自身的ip地址组合。这就是为什么通常服务器端在listen之前会调用bind()，而客户端就不会调用，而是在connect()时由系统随机生成一个。



##### connect()

```c++
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

参数详解：

- 客户端的socket描述字
- 服务器的socket地址
- socket地址的长度

客户端通过调用connect函数来建立与TCP服务器的连接。

##### 笔记

要判断socket是否连接，还需要其他的socket函数，很麻烦。所以在这里设置为：

在输入命令connect之后再进行socket()初始化，默认获得了socketfd就等同于已连接。



##### listen()

```c++
int listen(int sockfd,int backlog);
```

如果作为一个服务器，在调用socket()、bind()之后就会调用listen()来监听这个socket，如果客户端这时调用connect()发出连接请求，服务器端就会接收到这个请求。

listen函数的第一个参数即为要监听的socket描述字，第二个参数为相应socket可以排队的最大连接个数。socket()函数创建的socket默认是一个主动类型的(客户端)，listen函数将socket变为被动类型的(服务器)，等待客户的连接请求。



##### accept()

TCP服务器端依次调用socket()、bind()、listen()之后，就会监听指定的socket地址了。TCP客户端依次调用socket()、connect()之后就想TCP服务器发送了一个连接请求。TCP服务器监听到这个请求之后，就会调用accept()函数取接收请求，这样连接就建立好了。

```c++
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

参数：

- 服务器的socket描述字，是服务器开始调用socket()函数时生成的
- 用于返回客户端的协议地址
- 协议地址的长度。

如果accpet成功，那么其返回值是由内核自动生成的一个全新的描述字，代表与返回客户的TCP连接。

> accept的第一个参数为服务器的socket描述字，是服务器开始调用socket()函数生成的，称为监听socket描述字；而accept函数返回的是已连接的socket描述字。一个服务器通常通常仅仅只创建一个监听socket描述字，它在该服务器的生命周期内一直存在。内核为每个由服务器进程接受的客户连接创建了一个已连接socket描述字，当服务器完成了对某个客户的服务，相应的已连接socket描述字就被关闭。



##### close()

在服务器与客户端建立连接之后，会进行一些读写操作，完成了读写操作就要关闭相应的socket描述字，好比操作完打开的文件要调用fclose关闭打开的文件。

```c++
#include <unistd.h>
int close(int fd);
```













##### pthread_create()

```c++
int pthread_create(
                 pthread_t *restrict tidp,
                 const pthread_attr_t *restrict attr,
                 void *(*start_rtn)(void *),
                 void *restrict arg
                  );
```

参数详解：

- 新创建的线程ID指向的内存单元
- 线程属性，默认为NULL
- 新创建的线程从start_rtn函数的地址开始运行
- 默认为NULL。若上述函数需要参数，将参数放入结构中并将地址作为arg传入

