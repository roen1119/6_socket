#ifndef SERVER_H_
#define SERVER_H_

#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <mutex>

#include <pthread.h>    // 多线程

#include <sys/socket.h> // socket基本接口
#include <arpa/inet.h>  // htonl，htons，ntohl，ntohs等字节序转换函数
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

using namespace std;

#define BUFFER_SIZE 256

// 为什么不能用vector<int connection_fd, sockaddr_in addr>的方式来表示client list呢
typedef pair<string, int> ip_port;

void connection_handle(int connection_fd); // TODO

class myServer
{
private:
    int sockfd;         // 服务器的sockfd
    sockaddr_in addr;   // 服务器的address

    // 为什么不能放在类内呢？
    //      因为connection_handle也需要用！
    // vector<pair<int, ip_port>> clientList;

    const int MAX_CONNECTION = 4;

    static void* start_rtn(void* sfd)
    {
        connection_handle(*(int*)sfd);
        return nullptr;
    }
public:
    myServer();     // 完成socket(), bind(), listen()的操作
    ~myServer();
    void run();     // run只负责接受connect，其他的工作，交给子线程，判断recv到的包是社么
};

#endif