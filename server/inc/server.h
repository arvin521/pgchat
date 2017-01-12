
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

/************************* Macro definition *************************/

#define LISTEN_NUM 20
#define EPOLL_EVENTS_NUM 20
#define EPOLL_CREATE_NUM 256
#define BUFF_SIZE 256

/************************* const definition *************************/

const int c_portnumber = 5000;

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



#endif
