#include "client.h"

int main()
{
    myClient client;
    client.run();
}

myClient::myClient()
{
    ifConnected = false;
    sockfd = -1;
    // 创建消息队列，实现进程间通信
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
    close(sockfd);
    // msgctl(msgid, IPC_RMID, 0);      // 删除消息队列
}

void myClient::run()
{
    printMenu();
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    cout << "[ debug] sockfd = " << sockfd << endl;
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
            if (true == ifConnected)
            {
                cout << "Already connected to " << inet_ntoa(serverAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
            }
            else
            {
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_port = htons(stoi(words[2]));
                serverAddr.sin_addr.s_addr = inet_addr(words[1].c_str());

                if( connect(sockfd, (sockaddr*)&serverAddr, (socklen_t)sizeof(serverAddr)) < 0 )
                {
                    cout << "Connect failed!\n";
                    ifConnected = false;
                }
                else
                {
                    cout << "Connect success!\n";
                    ifConnected = true;
                    pthread_create(&tidp, nullptr, start_rtn, &sockfd);
                    // note: 
                    //  参数：新线程id，null指针，新线程开始的地方，传入参数给start_rtn
                }
            }
        }
        else if (op == "gettime")
        {
            // 调用函数send
            cout << "404 not done. \n";
        }
        else if (op == "getname")
        {
            // 调用函数send
            cout << "404 not done. \n";
        }
        else if (op == "getclients")
        {
            // 调用函数send
            cout << "404 not done. \n";
        }
        else if (op == "send")
        {
            // 调用函数send
            cout << "404 not done. \n";
        }
        else if (op == "close")
        {
            if (false == ifConnected)
            {
                cout << "No connection now.\n";
            }
            else
            {
                disconnect();
                cout << "Connection closed. \n";
            }
        }
        else if (op == "exit")
        {
            if (true == ifConnected)
            {
                disconnect();
            }
            cout << "Goodbye\n";
            exit(0);
        }
        else if (op == "help")
        {
            printMenu();
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
    cout << buffer << "\n> ";
    fflush(stdout); // 清空缓冲区

    // 消息队列
    key_t msgkey = ftok("/",'a');
    int msgid = msgget(msgkey, IPC_CREAT | 0666);
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sfd, buffer, BUFFER_SIZE, 0);  // 调用recv接受server的响应消息

        // 不适宜交给父线程让它操作，因为client接收消息是被动
        // 而父线程只能在主动发送请求下，才能接受消息做出响应
        if (20 == buffer[0])    // 类型20：直接打印
        {
            std::cout<<buffer + 1<<'\n';
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
    char buffer = 50;
    // TODO: 发送fin包，是否有问题？
    send(sockfd, &buffer, sizeof(buffer), 0);       // 发送包通知服务器断开连接
    mutex mt;
    mt.lock();
    pthread_cancel(tidp);
    mt.unlock();
    ifConnected = false;
    return;
}