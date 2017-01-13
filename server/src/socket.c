
#ifdef _cplusplus 
extern "C"{ 
#endif

#include "socket.h"

static const int c_iServPort = 5000;

/*************** local function definition **************/

static void set_no_blocking(int iSocketfd)
{
    int iOpts = 0;

    iOpts = fcntl(iSocketfd, F_GETFL);
    if(iOpts < 0)
    {
        log_e("fcntl(iSocketfd,GETFL)");
        return;
    }

    iOpts = iOpts | O_NONBLOCK;
    if(fcntl(iSocketfd, F_SETFL, iOpts) < 0)
    {
        log_e("fcntl(iSocketfd, SETFL, iOpts)");
        return;
    }
}

static void socket_close(int sockid, struct epoll_event *pstEpollEv)
{
    close(sockid);
    pstEpollEv->data.fd = -1;
}

/*************** API function definition *****************/

int socket_init(OUT pstSOCKET_INFO pstSocketInfo)
{
    int iRet = 0;
    int iSwitch = 1;

    CHECK_PARAM_RET(pstSocketInfo, ERR);

    struct sockaddr_in stServerAddr;

    pstSocketInfo->iSocketfdListen = socket(AF_INET, SOCK_STREAM, 0);
    if (pstSocketInfo->iSocketfdListen < 0)
    {
        log_e("create socket failed!");
        return ERR;
    }

    memset(&stServerAddr, 0, sizeof(stServerAddr));
    stServerAddr.sin_family = AF_INET;
    stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stServerAddr.sin_port = htons(c_iServPort);
    iRet = bind(pstSocketInfo->iSocketfdListen, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr));
    if (iRet < 0)
    {   
        log_e("socket bind failed!");
        close(pstSocketInfo->iSocketfdListen);
        return ERR;
    } 

    //fix bug: Bind error: Address already in use
    iRet = setsockopt(pstSocketInfo->iSocketfdListen, SOL_SOCKET, SO_REUSEADDR, &iSwitch, sizeof(iSwitch));
    if(iRet < 0)
    {    
        log_e("server: setsockopt failed!");
        close(pstSocketInfo->iSocketfdListen);
        return -1;
    }

    iRet = listen(pstSocketInfo->iSocketfdListen, LISTEN_NUM);
    if (iRet < 0)
    {
        log_e("socket listen failed!");
        close(pstSocketInfo->iSocketfdListen);
        return ERR;
    }

    pstSocketInfo->iEpollfd = epoll_create(EPOLL_CREATE_NUM);
    if( pstSocketInfo->iEpollfd < 0 ){
        log_e("server: epoll_create failed!");
        close(pstSocketInfo->iSocketfdListen);
        return ERR;
    }

    pstSocketInfo->stEpollEvent.data.fd = pstSocketInfo->iSocketfdListen;
    pstSocketInfo->stEpollEvent.events = EPOLLIN | EPOLLET; //events type is : ET

    //register epoll event
    iRet = epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_ADD, pstSocketInfo->iSocketfdListen, &(pstSocketInfo->stEpollEvent));
    if (iRet < 0)
    {
        close(pstSocketInfo->iSocketfdListen);
        log_e("epoll_ctl");
        return ERR;
    }

    return OK;
}

int socket_wait(IN pstSOCKET_INFO pstSocketInfo, OUT int *piFdNum)
{
    CHECK_PARAM_RET(pstSocketInfo, ERR);
    *piFdNum = epoll_wait(pstSocketInfo->iEpollfd, pstSocketInfo->astEpollEventsList, EPOLL_EVENTS_NUM, -1);
    return OK;
}

int socket_add(IN pstSOCKET_INFO pstSocketInfo)
{
    int iRet = 0;
    int iAcceptfd = 0;
    socklen_t clilen;
    struct sockaddr_in stClientAddr;
    //int iSize = 0;

    if (NULL == pstSocketInfo)
    {
        close(pstSocketInfo->iSocketfdListen);
        return ERR;
    }

    memset(&stClientAddr, 0, sizeof(stClientAddr));

    iAcceptfd = accept(pstSocketInfo->iSocketfdListen, (struct sockaddr *)&stClientAddr, &clilen);
    if(iAcceptfd < 0){
        log_e("accept failed!");
        close(pstSocketInfo->iSocketfdListen);
        return ERR;
    }

    printf("accapt a connection from %s\n", inet_ntoa(stClientAddr.sin_addr));

    set_no_blocking(iAcceptfd);

    pstSocketInfo->stEpollEvent.data.fd = iAcceptfd;
    pstSocketInfo->stEpollEvent.events  = EPOLLIN | EPOLLET;

    iRet = epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_ADD, iAcceptfd, &(pstSocketInfo->stEpollEvent));
    if(iRet < 0)
    {
        log_e("epoll_ctl\n");
        close(pstSocketInfo->iSocketfdListen);
        return ERR;
    }

    return OK;
}

int socket_recv(struct epoll_event *pstCurrEpollEvent,IO pstSOCKET_INFO pstSocketInfo)
{
    int iRet = 0;
    int iRecvNum = 0;
    int iCounter = 0;
    int iIsReadOver = 0;
    int iCurrSocketfd = 0;
    char *pcBuffHead = NULL;

    CHECK_PARAM_RET(pstCurrEpollEvent, ERR);
    CHECK_PARAM_RET(pstSocketInfo, ERR);

    log_i("EPOLLIN\n");

    pcBuffHead = pstSocketInfo->acSocketBuff;

    iCurrSocketfd = pstCurrEpollEvent->data.fd;
    if (iCurrSocketfd < 0)
    {
        return OK;
    }

    //TODO: Temporarily cleared, consider multi threading and then modify
    memset(pstSocketInfo->acSocketBuff, 0, sizeof(pstSocketInfo->acSocketBuff));

    while (1)
    {
        // make sure "iCurrSocketfd" is no blocking
        iRecvNum = recv(iCurrSocketfd, pcBuffHead + iCounter, BUFF_SIZE, 0);
        if(iRecvNum < 0)
        {
            if(errno == EAGAIN)
            {
                // errno is equal to EAGAIN indicates that current buff has no data to be read.
                iIsReadOver = 1;
                break;
            }
            else if (errno == ECONNRESET)
            {
                // client had sent RST
                socket_close(iCurrSocketfd, pstCurrEpollEvent);
                printf("counterpart send out RST\n");
                break;
                }
            else if (errno == EINTR)
            {
                // interrupt by signal
                continue;
            }
            else
            {
                // Other irreparable mistakes
                socket_close(iCurrSocketfd, pstCurrEpollEvent);
                printf("unrecovable error\n");
                break;
            }
        }
        else if( iRecvNum == 0)
        {
            // receive FIN indicates that client socket had closed
            socket_close(iCurrSocketfd, pstCurrEpollEvent);
            printf("counterpart has shut off\n");
            break;
        }

        // iRecvNum > 0
        iCounter += iRecvNum;
        if ( iRecvNum == BUFF_SIZE)
        {
            continue;   // need read again
        }
        else
        {
            // Read all the data
            iIsReadOver = 1;
            break;
        }
    }

    //Read over!
    if (iIsReadOver)
    {
        pstSocketInfo->acSocketBuff[iCounter] = '\0';

        log_i("recv from client : %s\n", pstSocketInfo->acSocketBuff);

        pstSocketInfo->stEpollEvent.data.fd = iCurrSocketfd;
        pstSocketInfo->stEpollEvent.events  = EPOLLOUT | EPOLLET;

        iRet = epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_MOD, iCurrSocketfd, &(pstSocketInfo->stEpollEvent));
        if(iRet < 0)
        {
            log_e("epoll_ctl\n");
            socket_close(iCurrSocketfd, pstCurrEpollEvent);
            return ERR;
        }
        
    }

    return OK;
}

int socket_send(IN struct epoll_event *pstCurrEpollEvent, IN char *pcSendBuff,
                IN int iSendBuffSize, IO pstSOCKET_INFO pstSocketInfo)
{
    int iRet = 0;
    int iCounter = 0;
    int iIsWritenOver = 0;
    int iIsWritenLen = 0;
    int iCurrSocketfd = 0;
    char *pcSendBuffHead = pcSendBuff;

    CHECK_PARAM_RET(pstCurrEpollEvent, ERR);
    CHECK_PARAM_RET(pstSocketInfo, ERR);

    iCurrSocketfd = pstCurrEpollEvent->data.fd;

    log_i("Write %s", pcSendBuff);

    while(1)
    {
        // make sure "iCurrSocketfd" is no blocking
        iIsWritenLen = send(iCurrSocketfd, pcSendBuffHead + iCounter, iSendBuffSize, 0);
        if (iIsWritenLen == -1)
        {
            if (errno == EAGAIN)
            {
                // send all the data
                iIsWritenOver = 1;
                break;
            }
            else if(errno == ECONNRESET)
            {
                // client reset, and send RST
                socket_close(iCurrSocketfd, pstCurrEpollEvent);
                printf("counterpart send out RST\n");
                break;
            }
            else if (errno == EINTR)
            {
                // interrupt by signal
                continue;
            }
            else
            {
                // other mistake
            }
        }

        if (iIsWritenLen == 0)
        {
            // client socket is closed!
            socket_close(iCurrSocketfd, pstCurrEpollEvent);
            printf("counterpart has shut off\n");
            break;
        }

        iCounter += iIsWritenLen;
        if (iIsWritenLen == iSendBuffSize)
        {
            // continue write
            continue;
        }
        else
        {
            iIsWritenOver = 1;
            break;
        }
    }

    if (iIsWritenOver)
    {
        pstSocketInfo->stEpollEvent.data.fd = iCurrSocketfd;
        pstSocketInfo->stEpollEvent.events=EPOLLIN | EPOLLET;

        iRet = epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_MOD, iCurrSocketfd, &(pstSocketInfo->stEpollEvent));
        if(iRet < 0)
        {
            log_e("epoll_ctl\n");
            socket_close(iCurrSocketfd, pstCurrEpollEvent);
            return ERR;
        }
    }

    return OK;
}


#ifdef _cplusplus 
} 
#endif