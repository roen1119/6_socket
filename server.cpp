#include "server.h"

vector< pair<int, ip_port> > clientList;

int main()
{
    // 设置异步
    // ios::sync_with_stdio(false);
    myServer server;
    server.run();
}

myServer::myServer()
{
    cout << "[ debug] socket()...\n";
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    cout << "[ debug] sockfd: " << sockfd << endl;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5412);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    cout << "[ debug] bind()...\n";
    bind(sockfd, (sockaddr*)&addr, (socklen_t)sizeof(addr));
    cout << "[ debug] listen()...\n";
    listen(sockfd, MAX_CONNECTION);
    cout << "listening\n";
}

myServer::~myServer()
{
    close(sockfd);
}

void myServer::run()
{
    while(true)
    {
        sockaddr_in clientAddr;
        // socklen_t* addrLength;   // why not use this directly?
        unsigned int clientAddrLength = sizeof(clientAddr);
        int connection_fd = accept(sockfd, (sockaddr*)&client, (socklen_t*)&clientAddrLength);
        clientList.push_back(pair<int, ip_port>(connection_fd, (ip_port)( inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.cin_port))));
        cout << "[ debug] connection_fd: " << connection_fd << endl;
        cout << "[ debug] clientAddr: " << inet_ntoa(clientAddr.sin_addr) << endl;
        cout << "[ debug] clientPort: " << ntohs(clientAddr.cin_port) << endl;
        /**
         * TODO
         * 子线程：继续循环accept，并send hello给刚刚的客户
         */
        char hello[] = "hello\n";
        send(connection_fd, hello, strlen(hello), 0);
    }
}