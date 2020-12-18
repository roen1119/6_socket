#ifndef CLIENT_H_
#define CLIENT_H_

#include <iostream>
#include <vector>
#include <string>
#include <regex>

#include <pthread.h>    // 多线程

#include <sys/socket.h> // socket基本接口
#include <arpa/inet.h>  // htonl，htons，ntohl，ntohs等字节序转换函数
#include <unistd.h>

using namespace std;

#define BUFFER_SIZE 256
void connection_handle(int sfd); // TODO

class myClient
{
private:
    int sockfd;               // 客户端的socket描述字
    sockaddr_in serverAddr;     // 所连接的服务器的socket地址
    // Note:
    //  sin_family:对应的是地址类型：AF_INET代表ipv4。
    //  sin_port:代表端口号。
    //  sin_addr.s_addr:代表我们所建立的ip地址。
    
    pthread_t tidp;             // 新创建的线程ID指向的内存单元

    void init();    // 通过socket获得sockfd，打印菜单
    void disconnect();  // 断开连接，发送
    void printMenu();

    // TODO: why static?
    static void * start_rtn(void * sfd) // 新创建的线程从start_rtn函数的地址开始运行
    {
        connection_handle(*(int*) sfd); // TODO
        return nullptr;
    }


public:
    myClient(); // 初始化一些类成员变量
    ~myClient();

    void run(); // 对输入命令进行响应
};

#endif