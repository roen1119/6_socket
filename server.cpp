#include "server.h"

vector< pair<int, ip_port> > clientList;
mutex mt;

int main()
{
    myServer server;
    server.run();
}

myServer::myServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    cout << "Sockfd: " << sockfd << endl;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5412);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (sockaddr*)&addr, (socklen_t)sizeof(addr));
    listen(sockfd, MAX_CONNECTION);
    cout << "Binding: " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;
}

myServer::~myServer()
{
    close(sockfd);
}

void myServer::run()
{
    cout << "Server listening...\n";
    while(true)
    {
        sockaddr_in clientAddr;
        unsigned int clientAddrLength = sizeof(clientAddr);
        int  connection_fd = accept(sockfd, (sockaddr*)&clientAddr, (socklen_t*)&clientAddrLength);
        clientList.push_back(pair<int, ip_port>(connection_fd, ip_port( inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port))));
        cout << "New connection:\n";
        cout << "         connection_fd: " << connection_fd << endl;
        cout << "         clientAddr: " << inet_ntoa(clientAddr.sin_addr) << endl;
        cout << "         clientPort: " << ntohs(clientAddr.sin_port) << endl;

        pthread_t tidp;
        pthread_create(&tidp, nullptr, start_rtn, &connection_fd);
    }
}


void child_thread(int connection_fd)
{
    char helloMsg[] = "hello\n";
    send(connection_fd, helloMsg, strlen(helloMsg), 0);

    char buffer_recv[BUFFER_SIZE] = {0};
    char buffer_send[BUFFER_SIZE] = {0};
    while (true)
    {
        memset(buffer_send, 0, BUFFER_SIZE);
        recv(connection_fd, buffer_recv, BUFFER_SIZE, 0);
        mt.lock();
        if (TRY_CLOSE == buffer_recv[0])    // close
        {
            cout << "Close connection: " << connection_fd << endl;
            for( auto it = clientList.begin(); it != clientList.end(); ++it)
            {
                if ((*it).first == connection_fd)
                {
                    it = clientList.erase(it);
                    break;
                }
            }
        }
        else if (GET_TIME == buffer_recv[0])   // gettime
        {
            cout << "get time: " << connection_fd << endl;
            time_t t;
            time(&t);
            buffer_send[0] = RES_TIME;
            sprintf(buffer_send + strlen(buffer_send), "%ld", t);
            send(connection_fd, buffer_send, strlen(buffer_send), 0);
        }
        else if (GET_NAME == buffer_recv[0])   // getname
        {
            cout << "get name: " << connection_fd << endl;
            buffer_send[0] = RES_NAME;
            gethostname(buffer_send + strlen(buffer_send), sizeof(buffer_send) - sizeof(char));
            send(connection_fd, buffer_send, strlen(buffer_send), 0);
        }
        else if (GET_LIST == buffer_recv[0])   // getclientlist
        {
            cout << "get client list: " << connection_fd << endl;
            buffer_send[0] = RES_LIST;
            for (auto& it: clientList)
            {
                sprintf(buffer_send + strlen(buffer_send), "%s", it.second.first.c_str());
                sprintf(buffer_send + strlen(buffer_send), "#");
                sprintf(buffer_send + strlen(buffer_send), "%d", it.second.second);
                sprintf(buffer_send + strlen(buffer_send), "*");
            }
            send(connection_fd, buffer_send, strlen(buffer_send), 0);
        }
        else if (TRY_SEND == buffer_recv[0])   // send
        {
            string recv(buffer_recv + 1);
            size_t pos0 = recv.find("#"), pos1 = recv.find("*");
            string ip_addr = recv.substr(0, pos0);
            int port = atoi(recv.substr((pos0 + 1), pos1 - pos0 - 1).c_str());
            string content = recv.substr(pos1 + 1);
            
            cout <<"Sending a message to "<< ip_addr << ":" << port << ". And the content is: \n" << content << endl;

            int dest = -1;
            for (auto it = clientList.begin(); it != clientList.end(); ++it)
            {
                if (it->second.first == ip_addr && it->second.second == port)
                {
                    dest = it->first;
                    break;
                }
            }

            // buffer_send[] sent to connection_fd
            // msg_send[] sent to destination client
            buffer_send[0] = RES_SEND;
            if (-1 == dest)
            {
                cout << "Destination Unconnected.\n";
                sprintf(buffer_send + 1, "Sending falied.");
            }
            else
            {
                cout << "Send success.\n";
                sprintf(buffer_send + 1, "Send success.");
                char msg_send[BUFFER_SIZE] = {0};
                msg_send[0] = INDICATE;
                sprintf(msg_send + 1, "%s", content.c_str());
                send(dest, msg_send, strlen(msg_send), 0);
            }
            send(connection_fd, buffer_send, strlen(buffer_send), 0);
        }
        memset(buffer_recv, 0, BUFFER_SIZE);
        mt.unlock();
    }
}