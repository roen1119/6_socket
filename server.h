#ifndef SERVER_H_
#define SERVER_H_

#include "basic.h"

using namespace std;

typedef pair<string, int> ip_port;

void connection_handle(int connection_fd);

class myServer
{
private:
    int sockfd;
    sockaddr_in addr;

    const int MAX_CONNECTION = 4;

    static void* start_rtn(void* sfd)
    {
        connection_handle(*(int*)sfd);
        return nullptr;
    }
    
public:
    myServer();
    ~myServer();
    void run();
};

#endif