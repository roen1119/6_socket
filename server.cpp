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
        unsigned int clientAddrLength = sizeof(clientAddr);
        int  connection_fd = accept(sockfd, (sockaddr*)&clientAddr, (socklen_t*)&clientAddrLength);
        clientList.push_back(pair<int, ip_port>(connection_fd, ip_port( inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port))));
        cout << "[ debug] connection_fd: " << connection_fd << endl;
        cout << "         clientAddr: " << inet_ntoa(clientAddr.sin_addr) << endl;
        cout << "         clientPort: " << ntohs(clientAddr.sin_port) << endl;

        pthread_t tidp;
        pthread_create(&tidp, nullptr, start_rtn, &connection_fd);
    }
}

void connection_handle(int connection_fd)
{
    char helloMsg[] = "hello\n";
    send(connection_fd, helloMsg, strlen(helloMsg), 0);

    char buffer_recv[BUFFER_SIZE] = {0};
    char buffer_send[BUFFER_SIZE] = {0};
    int flag = 1;
    while (true)
    {

        memset(buffer_send, 0, BUFFER_SIZE);

        recv(connection_fd, buffer_recv, BUFFER_SIZE, 0);


        mt.lock();
        if (9 == buffer_recv[0])    // close
        {
            // client在调用disconnect时，已经通过close将自己的socket断开，
            // 服务器不需要再处理连接的问题，只需要维护好自己的clientlist，将该client从表中删去即可。
            cout << "close connection: " << connection_fd << endl;
            for( auto it = clientList.begin(); it != clientList.end(); ++it)
            {
                if ((*it).first == connection_fd)
                {
                    it = clientList.erase(it);
                    cout << "[ debug] erase from list\n";
                    break;
                }
            }
        }
        else if (1 == buffer_recv[0])   // gettime
        {
            cout << "gettime: " << connection_fd << endl;
            time_t t;
            time(&t);
            buffer_send[0] = 11;
            sprintf(buffer_send + strlen(buffer_send), "%ld", t);
            send(connection_fd, buffer_send, strlen(buffer_send), 0);
        }
        else if (2 == buffer_recv[0])   // getname
        {
            cout << "getname: " << connection_fd << endl;
            buffer_send[0] = 12;
            gethostname(buffer_send + strlen(buffer_send), sizeof(buffer_send) - sizeof(char));
            send(connection_fd, buffer_send, strlen(buffer_send), 0);
        }
        else if (3 == buffer_recv[0])   // getclientlist
        {
            cout << "getclientlist: " << connection_fd << endl;
            buffer_send[0] = 13;
            for (auto& it: clientList)
            {
                sprintf(buffer_send + strlen(buffer_send), "%s", it.second.first.c_str());
                sprintf(buffer_send + strlen(buffer_send), "#");
                sprintf(buffer_send + strlen(buffer_send), "%d", it.second.second);
                sprintf(buffer_send + strlen(buffer_send), "*");
            }
            send(connection_fd, buffer_send, strlen(buffer_send), 0);
        }
        else if (4 == buffer_recv[0])   // send
        {
            // 分析包
            string recv(buffer_recv + 1);
            size_t pos0 = recv.find("#"), pos1 = recv.find("*");
            string ip_addr = recv.substr(0, pos0);
            int port = atoi(recv.substr((pos0 + 1), pos1 - pos0 - 1).c_str());
            string content = recv.substr(pos1 + 1);
            
            cout <<"send to "<< ip_addr << ":" << port << ':' << connection_fd << endl;

            int dest = -1;
            for (auto it = clientList.begin(); it != clientList.end(); ++it)
            {
                if (it->second.first == ip_addr && it->second.second == port)
                {
                    dest = it->first;
                    break;
                }
            }

            // buffer_send[] to connection_fd
            // msg_send[] to destination client
            buffer_send[0] = 14;
            if (-1 == dest)
            {
                cout << "destination unconnected\n";
                sprintf(buffer_send + 1, "Sending falied.\n");
            }
            else
            {
                sprintf(buffer_send + 1, "Send success.\n");
                char msg_send[BUFFER_SIZE] = {0};
                msg_send[0] = 20;
                sprintf(msg_send + 1, "%s", content.c_str());
                send(dest, msg_send, strlen(msg_send), 0);
            }
            send(connection_fd, buffer_send, strlen(buffer_send), 0);
        }
        else if (0 == buffer_recv[0])
        {
            if(flag)
            {
                cout << "ghost0\n";
                flag=0;
            }
        }
        memset(buffer_recv, 0, BUFFER_SIZE);
        mt.unlock();
    }
}