#ifndef __SOCKET_H__
#define __SOCKET_H__


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>


/************************* 宏定义 *************************/
#define LISTEN_NUM 20
#define EVENTS_NUM 20
#define EPOLL_CREATE_NUM 256

#define DATA_BUFF_SIZE 256

/************************* 常量定义 *************************/

const int coiPortNum = 12345;

/************************* 参数宏定义 *************************/

/************************* 联合体定义 *************************/

typedef struct SOCKET_INFO
{
    int iSockfd;

    int iEpollfd;
    struct epoll_event stEpollEvent;
    struct epoll_event astEpollEvents[EVENTS_NUM];

    char acDataBuff[DATA_BUFF_SIZE];

}stSOCKET_INFO, *pstSOCKET_INFO;

/************************* 结构体定义 *************************/

/************************* 本地函数声明 *************************/

static void close_and_event(int iSockfd, struct epoll_event pstEvent);
static void set_no_block(int iSock);

/************************* API函数声明 *************************/

int socket_init(OUT pstSOCKET_INFO pstSocketInfo);
int socket_wait(IN pstSOCKET_INFO pstSocketInfo);
int socket_add_link(IO pstSOCKET_INFO pstSocketInfo);
int socket_recv(IN struct epoll_event *pstCurrEpollEvent, IO pstSOCKET_INFO pstSocketInfo);
int socket_send(IN struct epoll_event *pstCurrEpollEvent);

#endif