# mysocket

### 要求

- 自定义协议

    定义包的type、header等

- 使用Socket编程接口

    C语言形式

- 实现网络应用软件

    发送、接受网络数据包

- 传输层：TCP

### 服务器

并发处理多个客户端的请求。请求的类型如下：

socket()

bind() 绑定监听端口5412

listen()，设置连接等待队列长度

循环调用accept()，接收到socket句柄后创建新的客户端项目，而后创建子进程继续调用accept()，send() hello给刚才的客户端。接着，循环调用receive()

- 时间 time(), send()
- 名称 GetComputerName(), send()
- 连接的所有客户端信息 send()
- 转发send()
- 异步多线程

### 客户端

人机交互：命令行；与服务器的通信。以下功能都通过发包、收包的流程实现。

socket()

- connect() 连接

    连接到指定地址和端口的服务器。

    连接成功后，创建子线程循环调用reveive().

- close() 断开连接
- send() 获取时间 - 响应

    请求服务器给出时间

- send() 获取名字 - 响应

    请求服务器给出其机器名称

- send() 活动连接列表 - 响应

    当前连接的所有客户端：编号、IP地址、端口

- send() 发消息

    要求服务器转发消息给对应编号的客户端

- receive() 收消息  - 指示消息

    收到服务器转发的消息后显示在屏幕上

- 退出

    判断是否连接，若是则先断开连接。退出程序。