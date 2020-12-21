#include "client.h"

int main()
{
    myClient client;
    client.run();
}

myClient::myClient()
{
    sockfd = -1;
    // TODO: 为什么是ftok("/",'a')
    key_t msgkey = ftok("/",'a');
    msgid = msgget(msgkey, IPC_CREAT | 0666);
    if ( -1 == msgid)
    {
        cout << "[ debug] msgget falied" << endl;
    }
}

myClient::~myClient()
{
    if (-1 != sockfd)
    {
        disconnect();
    }
    // msgctl(msgid, IPC_RMID, 0);      // 删除消息队列
}

void myClient::run()
{
    printMenu();
    while(true)
    {
        string command;
        cout << "> ";
        getline(cin, command);
        /**
         * TODO:  
         * my split 
         */
            regex whitespace("\\s+");
            vector<string> words(sregex_token_iterator(command.begin(), command.end(), whitespace, -1),
                             sregex_token_iterator());
        /**
         * TODO:
         * remember to check the length of `words[]` to prevent segment error
         */
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
                    cout << "Connect failed!\n";
                    cout << "[ debug] sockfd = " << sockfd << endl;
                    close(sockfd);
                    sockfd = -1;
                }
                else
                {
                    cout << "Connect success!\n";
                    pthread_create(&tidp, nullptr, start_rtn, &sockfd);
                    // note: 
                    //  参数：新线程id，null指针，新线程开始的地方，传入参数给start_rtn
                }
            }
        }
        else if (op == "gettime")
        {
            // 调用函数send
            char buffer = 1;
            send(sockfd, &buffer, sizeof(buffer), 0);
            message msg_rcv;
            msgrcv(msgid, &msg_rcv, BUFFER_SIZE, 11, 0);    // 优先接受11
            time_t time = atol(msg_rcv.content);
            cout << "Time: " << ctime(&time);
        }
        else if (op == "getname")
        {
            char buffer = 2;
            send(sockfd, &buffer, sizeof(buffer), 0);
            message msg_rcv;
            msgrcv(msgid, &msg_rcv, BUFFER_SIZE, 12, 0);    // 优先接受12
            cout << "Name: " << msg_rcv.content << endl;
        }
        else if (op == "getclientlist")
        {
            cout << "[ debug] in get client list\n";
            char buffer = 3;
            send(sockfd, &buffer, sizeof(buffer), 0);

            cout << "[ debug] have sent to server\n";
            message msg_rcv;
            msgrcv(msgid, &msg_rcv, BUFFER_SIZE, 13, 0);    // 优先接受13

            cout << "[ debug] have received from son\n";

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
        else if (op == "send")
        {
            char buffer[BUFFER_SIZE] = {0};
            buffer[0] = 4;
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
            cout << "[ debug] send msg: \n" << buffer << "\n[ debug] end\n";
            send(sockfd, buffer, BUFFER_SIZE, 0);
            message msg_rcv;
            msgrcv(msgid, &msg_rcv, BUFFER_SIZE, 14, 0);
            cout << msg_rcv.content << endl;
        }
        else if (op == "close")
        {
            if (-1 != sockfd)
            {
                disconnect();
                cout << "Connection closed. \n";
            }
            else
            {
                cout << "No connection now.\n";
            }
        }
        else if (op == "exit")
        {
            if (-1 != sockfd)
            {
                disconnect();
                cout << "Connection closed. \n";
            }
            cout << "Goodbye\n";
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

// 新创建的子进程要做什么？
void connection_handle(int sfd)
{
    // 创建时，第一次connect成功，应该print提示消息
    char buffer[BUFFER_SIZE];
    recv(sfd, buffer, BUFFER_SIZE, 0);  // 接收消息
    cout << buffer << "> ";
    fflush(stdout); // 清空缓冲区

    // 消息队列
    key_t msgkey = ftok("/",'a');
    int msgid = msgget(msgkey, IPC_CREAT | 0666);
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sfd, buffer, BUFFER_SIZE, 0);  // 调用recv接受server的响应消息

        if (20 == buffer[0])
        {
            cout << "\n Get A Message:\n";
            cout<<buffer + 1<<'\n';
            continue;
        }
        // 将消息类型和内容保存到msg中
        message msg;
        msg.type = buffer[0];
        strcpy(msg.content, buffer + 1);
        msgsnd(msgid, &msg, BUFFER_SIZE, 0);    // 发送给父进程
    }
}

void myClient::printMenu()
{
    cout << "Please input a command:\n"
         << "- connect [IP] [port]\n"
         << "- gettime\n"
         << "- getname\n"
         << "- getclientlist\n"
         << "- send [IP] [port] [message]\n"
         << "- close\n"
         << "- exit\n"
         << "- help\n";
}

void myClient::disconnect()
{
    char buffer = 9;
    // TODO: 发送fin包，是否有问题？
    send(sockfd, &buffer, sizeof(buffer), 0);       // 发送包通知服务器delete it from clientList
    mutex mt;
    mt.lock();
    pthread_cancel(tidp);
    mt.unlock();
    close(sockfd);
    sockfd = -1;
    return;
}