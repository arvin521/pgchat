
#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/************************* 宏定义 *************************/
#define LISTEN_NUM 20
#define EPOLL_EVENTS_NUM 20
#define EPOLL_CREATE_NUM 256
#define BUFF_SIZE 256

/************************* 常量定义 *************************/
const int c_portnumber = 5000;
/************************* 参数宏定义 *************************/

typedef struct SOCKET_INFO
{
    int iSocketfd;

    int iEpollfd;
    struct epoll_event stEpollEvent;
    struct epoll_event astEpollEventsList[EPOLL_EVENTS_NUM];

    char acSocketBuff[BUFF_SIZE];
    char acSocketSendBuff[BUFF_SIZE];
} stSOCKET_INFO, *pstSOCKET_INFO;

/************************* 联合体定义 *************************/

/************************* 结构体定义 *************************/

/************************* 本地函数声明 *************************/


/************************* API函数声明 *************************/



#endif
