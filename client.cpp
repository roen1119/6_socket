#include "client.h"

int main()
{
    myClient client;
    client.run();
}

myClient::myClient()
{
    sockfd = -1;
    key_t msgkey = ftok(".",'a');
    msgid = msgget(msgkey, IPC_CREAT | 0666);
    if ( -1 == msgid)
    {
        cout << "[ error] msgget falied" << endl;
    }
}

myClient::~myClient()
{
    if (-1 != sockfd)
    {
        disconnect();
    }
    msgctl(msgid, IPC_RMID, 0);
}

void myClient::run()
{
    printMenu();
    while(true)
    {
        string command;
        cout << "> ";
        getline(cin, command);
        regex whitespace("\\s+");
        vector<string> words(sregex_token_iterator(command.begin(), command.end(), whitespace, -1), sregex_token_iterator());

        string op = words[0];
        if (op == "")
        {
            continue;
        }
        else if (op =="connect")
        {
            if (-1 != sockfd)
            {
                cout << "Already connected to " << inet_ntoa(serverAddr.sin_addr) << ":" << ntohs(serverAddr.sin_port) << endl;
            }
            else
            {
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_port = htons(stoi(words[2]));
                serverAddr.sin_addr.s_addr = inet_addr(words[1].c_str());

                if( connect(sockfd, (sockaddr*)&serverAddr, (socklen_t)sizeof(serverAddr)) < 0 )
                {
                    cout << "Connect failed! Try reopening the server and client.\n";
                    close(sockfd);
                    sockfd = -1;
                }
                else
                {
                    cout << "Connect success!\n";
                    pthread_create(&tidp, nullptr, start_rtn, &sockfd);
                }
            }
        }
        else if (op == "gettime")
        {
            if (-1 != sockfd)
            {
                char buffer = GET_TIME;
                send(sockfd, &buffer, sizeof(buffer), 0);
                message msg_rcv;
                msgrcv(msgid, &msg_rcv, BUFFER_SIZE, RES_TIME, 0);        // 第三个参数表示优先接受11
                time_t time = atol(msg_rcv.content);
                cout << "Time: " << ctime(&time);
            }
            else
            {
                cout << "You are unconnected!\n";
            }
        }
        else if (op == "getname")
        {
            if(-1 != sockfd)
            {
                char buffer = GET_NAME;
                send(sockfd, &buffer, sizeof(buffer), 0);
                message msg_rcv;
                msgrcv(msgid, &msg_rcv, BUFFER_SIZE, RES_NAME, 0);
                cout << "Name: " << msg_rcv.content << endl;
            }
            else
            {
                cout << "You are unconnected!\n";
            }
        }
        else if (op == "getclientlist")
        {
            if(-1 != sockfd)
            {
                char buffer = GET_LIST;
                send(sockfd, &buffer, sizeof(buffer), 0);

                message msg_rcv;
                msgrcv(msgid, &msg_rcv, BUFFER_SIZE, RES_LIST, 0);

                cout << "IP\t\tPort\n";
                char* ptr = msg_rcv.content;
                while (*ptr)
                {
                    if ('#' == *ptr)
                    {
                        cout<<'\t';
                    }
                    else if ('*' == *ptr)
                    {
                        cout<<'\n';
                    }
                    else
                    {
                        cout<<(*ptr);
                    }
                    ptr++;
                }
            }
            else
            {
                cout << "You are unconnected!\n";
            }
        }
        else if (op == "send")
        {
            if (-1 != sockfd)
            {
                char buffer[BUFFER_SIZE] = {0};
                buffer[0] = TRY_SEND;
                sprintf(buffer + strlen(buffer), "%s", words[1].c_str());
                sprintf(buffer + strlen(buffer), "#");
                sprintf(buffer + strlen(buffer), "%s", words[2].c_str());
                sprintf(buffer + strlen(buffer), "*");
                for (int i = 3; i < words.size(); ++i)
                {
                    sprintf(buffer + strlen(buffer), "%s", words[i].c_str());
                    if (i != words.size())
                    {
                        sprintf(buffer + strlen(buffer), " ");
                    }
                    else
                    {
                        sprintf(buffer + strlen(buffer), "\n");
                    }
                }
                cout << "You are tring to send a packet: \n" << buffer << endl;
                send(sockfd, buffer, BUFFER_SIZE, 0);

                message msg_rcv;
                msgrcv(msgid, &msg_rcv, BUFFER_SIZE, RES_SEND, 0);
                cout << msg_rcv.content << endl;
            }
            else
            {
                cout << "You are unconnected!\n";
            }
        }
        else if (op == "close")
        {
            if (-1 != sockfd)
            {
                disconnect();
            }
            else
            {
                cout << "You are unconnected!\n";
            }
        }
        else if (op == "exit")
        {
            if (-1 != sockfd)
            {
                disconnect();
            }
            cout << "Client exit\n";
            exit(0);
        }
        else if (op == "help")
        {
            printMenu();
        }
        else if (op == "debugclose")
        {
            close(sockfd);
        }
        else if (op=="debugthread")
        {
            pthread_cancel(tidp);
        }
        else
        {
            cout << op << ": illegal operation\n";
        }
    }
}

void myClient::printMenu()
{
    cout << "Menu:\n"
         << "        connect [IP] [port]\n"
         << "        gettime\n"
         << "        getname\n"
         << "        getclientlist\n"
         << "        send [IP] [port] [message]\n"
         << "        close\n"
         << "        exit\n"
         << "        help\n";
}

void myClient::disconnect()
{
    char buffer = TRY_CLOSE;
    send(sockfd, &buffer, sizeof(buffer), 0);
    mutex mt;
    mt.lock();
    pthread_cancel(tidp);
    mt.unlock();
    close(sockfd);
    sockfd = -1;
    cout << "Connection closed. \n";
    return;
}

void connection_handle(int sfd)
{
    char buffer[BUFFER_SIZE];
    recv(sfd, buffer, BUFFER_SIZE, 0);
    cout << buffer << "> ";
    fflush(stdout); // 清空缓冲区

    key_t msgkey = ftok(".",'a');
    int msgid = msgget(msgkey, IPC_CREAT | 0666);
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sfd, buffer, BUFFER_SIZE, 0);

        if (INDICATE == buffer[0])
        {
            cout << "\nYou get a new message:\n";
            cout<<buffer + 1<< endl;
            continue;
        }
        
        message msg;
        msg.type = buffer[0];
        strcpy(msg.content, buffer + 1);
        msgsnd(msgid, &msg, BUFFER_SIZE, 0);
    }
}
