
#ifndef __SERVER_SOCKET_H__
#define __SERVER_SOCKET_H__


#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "comm.h"

/************************* Macro definition *************************/

#define LISTEN_NUM 20
#define EPOLL_EVENTS_NUM 20
#define EPOLL_CREATE_NUM 256
#define BUFF_SIZE 256

/************************* const definition *************************/

/************************* Parameter macro definition ***************/

typedef struct SOCKET_INFO
{
    int iSocketfdListen;

    int iEpollfd;
    struct epoll_event stEpollEvent;
    struct epoll_event astEpollEventsList[EPOLL_EVENTS_NUM];

    char acSocketBuff[BUFF_SIZE];
    char acSocketSendBuff[BUFF_SIZE];
} stSOCKET_INFO, *pstSOCKET_INFO;

/************************* union definition *************************/

/************************* struct definition ************************/

/************************* local function definition ****************/


/************************* API function definition ******************/

int socket_init(OUT pstSOCKET_INFO pstSocketInfo);
int socket_wait(IN pstSOCKET_INFO pstSocketInfo, OUT int *piFdNum);
int socket_add(IN pstSOCKET_INFO pstSocketInfo);
int socket_recv(struct epoll_event *pstCurrEpollEvent, IO pstSOCKET_INFO pstSocketInfo);
int socket_send(IN struct epoll_event *pstCurrEpollEvent, IO pstSOCKET_INFO pstSocketInfo);

#endif