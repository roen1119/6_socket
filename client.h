#ifndef CLIENT_H_
#define CLIENT_H_

#include "basic.h"

using namespace std;


void child_thread(int sfd);

struct message
{
    long int type;
    char content[BUFFER_SIZE];
};

class myClient
{
private:
    bool ifConnected= false;
    int sockfd;                 // 客户端的socket描述字
    sockaddr_in serverAddr;     // 所连接的服务器的socket地址
    
    pthread_t tidp;     // 子线程id
    int msgid;          // 消息队列标识符，用于线程间通信

    void disconnect();
    void printMenu();

    // 新创建的线程从start_rtn函数的地址开始运行
    static void * start_rtn(void * sfd)
    {
        child_thread(*(int*) sfd);
        return nullptr;
    }


public:
    myClient();
    ~myClient();
    void run();
};

#endif