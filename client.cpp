#include "client.h"

int main()
{
    myClient client;
    client.run();
}

myClient::myClient()
{
    cout << "[ debug] in myClient::myClinet\n";
    sockfd = -1;
    // 创建消息队列，实现进程间通信
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
    cout << "[ debug] in myClient::run()\n";
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
                /**
                 * TODO:
                 * 从serverAddr中获得
                 *  sin_addr.s_addr -ip地址
                 *  sin_port        -端口号
                 * 并通过ntohl函数转换成string，然后打印
                 */
                cout << "Already connected to \n";
            }
            else
            {
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                cout << "[ debug] sockfd = " << sockfd << endl;
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_port = htons(stoi(words[2]));
                serverAddr.sin_addr.s_addr = inet_addr(words[1].c_str());

                if( connect(sockfd, (sockaddr*)&serverAddr, (socklen_t)sizeof(serverAddr)) < 0 )
                {
                    cout << "Connect failed!\n";
                    close(sockfd);
                    sockfd = -1;
                }
                else
                {
                    cout << "[ debug] Connect success...\n";
                    pthread_create(&tidp, nullptr, start_rtn, &sockfd);
                    // note: 
                    //  功能：创建新进程
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
            if(-1 == sockfd)
            {
                cout << "No connection.\n";
            }
            else
            {
                cout << "[ debug] disconnect()...\n";
                disconnect();
                // TODO: 临界区互斥问题？
                mutex mt;
                mt.lock();
                pthread_cancel(tidp);
                mt.unlock();
                sockfd = -1;
                cout << "Connection closed. \n";
            }
        }
        else if (op == "exit")
        {
            if(-1 != sockfd)
            {
                disconnect();
            }
            cout << "Exit Clinet\n";
            exit(0);
        }
        else if (op == "help")
        {
            printMenu();
        }
        else if (op=="debug")
        {
            if(words[1] == "sockfd")
            {
                cout << "[ debug] sockfd: " <<  sockfd << endl;
            }
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
    cout << buffer << endl;
    fflush(stdout); // 清空缓冲区

    // 消息队列
    key_t msgkey = ftok("/",'a');
    int msgid = msgget(msgkey, IPC_CREAT | 0666);
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sfd, buffer, BUFFER_SIZE, 0);  // 调用recv接受server的响应消息
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
         << "- getclients\n"
         << "- send [IP] [port] [message]\n"
         << "- close\n"
         << "- exit\n"
         << "- help\n";
}

void myClient::disconnect()
{
    char buffer = 50;
    send(sockfd, &buffer, sizeof(buffer), 0);       // 发送包通知服务器断开连接
    close(sockfd);  // 关闭socket
    return;
}