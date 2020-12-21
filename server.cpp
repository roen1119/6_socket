#include "server.h"

vector< pair<int, ip_port> > clientList;
mutex mt;

int main()
{
    // TODO:设置异步
    // ios::sync_with_stdio(false);
    myServer server;
    server.run();
}

myServer::myServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5412);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (sockaddr*)&addr, (socklen_t)sizeof(addr));
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
        // TODO: why not use this directly?
        // socklen_t* addrLength;
        unsigned int clientAddrLength = sizeof(clientAddr);
        
        int  connection_fd = accept(sockfd, (sockaddr*)&clientAddr, (socklen_t*)&clientAddrLength);
        clientList.push_back(pair<int, ip_port>(connection_fd, ip_port( inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port))));
        cout << "[ debug] connection_fd: " << connection_fd << endl;
        cout << "         clientAddr: " << inet_ntoa(clientAddr.sin_addr) << endl;
        cout << "         clientPort: " << ntohs(clientAddr.sin_port) << endl;

        // note: 为什么不像client一样把tidp放在类内？
        //       因为一个server会连接很多client
        //       对每个连接，会新建一个子线程
        pthread_t tidp;
        pthread_create(&tidp, nullptr, start_rtn, &connection_fd);
        // 参数：新线程id，null指针，新线程开始的地方，传入参数给start_rtn
    }
}

/**
 * 子线程：继续循环accept，并send hello给刚刚的客户
 */
void connection_handle(int connection_fd)
{
    char helloMsg[] = "hello\n";
    send(connection_fd, helloMsg, strlen(helloMsg), 0);

    char buffer_recv[BUFFER_SIZE];
    while (true)
    {
        recv(connection_fd, buffer_recv, BUFFER_SIZE, 0);

        // TODO: 为什么在分析收到的包的时候需要临界区互斥？
        mt.lock();
        switch(buffer_recv[0])
        {
        case 0: // close
            // client在调用disconnect时，已经通过close将自己的socket断开，
            // 服务器不需要再处理连接的问题，只需要维护好自己的clientlist，将该client从表中删去即可。
            for( auto it = clientList.begin(); it != clientList.end(); ++it)
            {
                if ((*it).first == connection_fd)
                {
                    it = clientList.erase(it);
                    cout << "[ debug] erase from list\n";
                    break;
                }
            }
            break;
        case 1: // getime
            break;
        case 2: // getname
            break;
        case 3: // getclients
            break;
        case 4: // send
            break;
        case 5:
            break;
        }
        memset(buffer_recv, 0, BUFFER_SIZE);    // 重新分配内存
        mt.unlock();
    }
}