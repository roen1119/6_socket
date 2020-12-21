#ifndef BASIC_H_
#define BASIC_H_

#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <mutex>

#include <pthread.h>            // 多线程
#include <sys/socket.h>      // socket基本接口
#include <arpa/inet.h>        // htonl，htons，ntohl，ntohs等字节序转换函数
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>           // 消息队列

#define BUFFER_SIZE 256

// 请求数据包
#define GET_TIME 1
#define GET_NAME 2
#define GET_LIST 3
#define TRY_SEND 4
#define TRY_CLOSE 9

// 响应数据包
#define RES_TIME 11
#define RES_NAME 12
#define RES_LIST 13
#define RES_SEND 14

// 指示数据包
#define INDICATE 20 

#endif