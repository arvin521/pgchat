#ifdef _cplusplus 
extern "C"{ 
#endif

#include "server.h"
#include "comm.h"

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

int socket_init(OUT pstSOCKET_INFO pstSocketInfo)
{
    int iRet = 0;
    int iSwitch = 1;

    CHECK_PARAM_RET(pstSocketInfo, ERR);

    struct sockaddr_in stServerAddr;

    pstSocketInfo->iSocketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (pstSocketInfo->iSocketfd < 0)
    {
        log_e("create socket failed!");
        return ERR;
    }

    memset(&stServerAddr, 0, sizeof(stServerAddr));
    stServerAddr.sin_family = AF_INET;
    stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stServerAddr.sin_port=htons(c_portnumber);

    // bind and listen
    iRet = bind(pstSocketInfo->iSocketfd,(struct sockaddr *)&stServerAddr, sizeof(stServerAddr));
    if (iRet < 0)
    {   
        log_e("socket bind failed!");
        close(pstSocketInfo->iSocketfd);
        return ERR;
    } 

    //解决 Bind error: Address already in use,使绑定的ip关闭后立刻重新使用
    iRet = setsockopt(pstSocketInfo->iSocketfd, SOL_SOCKET, SO_REUSEADDR, &iSwitch, sizeof(iSwitch));
    if(iRet < 0)
    {    
        log_e("server: setsockopt failed!");
        close(pstSocketInfo->iSocketfd);
        return -1;
    }

    iRet = listen(pstSocketInfo->iSocketfd, LISTEN_NUM);
    if (iRet < 0)
    {
        log_e("socket listen failed!");
        close(pstSocketInfo->iSocketfd);
        return ERR;
    }

    //生成用于处理accept的epoll专用的文件描述符
    pstSocketInfo->iEpollfd = epoll_create(EPOLL_CREATE_NUM);
    if( pstSocketInfo->iEpollfd < 0 ){
        log_e("server: epoll_create failed!");
        close(pstSocketInfo->iSocketfd);
        return ERR;
    }

    //设置与要处理的事件相关的文件描述符
    pstSocketInfo->stEpollEvent.data.fd = pstSocketInfo->iSocketfd;
    //设置要处理的事件类型ET
    pstSocketInfo->stEpollEvent.events = EPOLLIN | EPOLLET;

    //注册epoll事件
    iRet = epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_ADD, pstSocketInfo->iSocketfd, &(pstSocketInfo->stEpollEvent));
    if (iRet < 0)
    {
        close(pstSocketInfo->iSocketfd);
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
        close(pstSocketInfo->iSocketfd);
        return ERR;
    }

    memset(&stClientAddr, 0, sizeof(stClientAddr));

    iAcceptfd = accept(pstSocketInfo->iSocketfd, (struct sockaddr *)&stClientAddr, &clilen);
    if(iAcceptfd < 0){
        log_e("accept failed!");
        close(pstSocketInfo->iSocketfd);
        return ERR;
    }

    printf("accapt a connection from %s\n", inet_ntoa(stClientAddr.sin_addr));

    //设置用于读操作的文件描述符
    set_no_blocking(iAcceptfd);
    pstSocketInfo->stEpollEvent.data.fd=iAcceptfd;
    pstSocketInfo->stEpollEvent.events=EPOLLIN | EPOLLET;

    iRet = epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_ADD, iAcceptfd, &(pstSocketInfo->stEpollEvent));
    if(iRet < 0)
    {
        log_e("epoll_ctl");
        close(pstSocketInfo->iSocketfd);
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
    char *pcBuffHead = pstSocketInfo->acSocketBuff;

    CHECK_PARAM_RET(pstCurrEpollEvent, ERR);
    CHECK_PARAM_RET(pstSocketInfo, ERR);

    log_i("EPOLLIN\n");

    pstSocketInfo->iSocketfd = pstCurrEpollEvent->data.fd;
    if (pstSocketInfo->iSocketfd < 0)
    {
        return OK;
    }

    //暂时清零，多线程的时候再修改
    memset(pstSocketInfo->acSocketBuff, 0, sizeof(pstSocketInfo->acSocketBuff));

    while (1)
    {
        // 确保 pstSocketInfo->iSocketfd 是 nonblocking 的
        iRecvNum = recv(pstSocketInfo->iSocketfd, pcBuffHead + iCounter, BUFF_SIZE, 0);
        if(iRecvNum < 0)
        {
            if(errno == EAGAIN)
            {
                // 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
                // 在这里就当作是该次事件已处理处.
                iIsReadOver = 1;
                break;
            }
            else if (errno == ECONNRESET)
            {
                // 对方发送了RST
                socket_close(pstSocketInfo->iSocketfd, pstCurrEpollEvent);
                printf("counterpart send out RST\n");
                break;
                }
            else if (errno == EINTR)
            {
                // 被信号中断
                continue;
            }
            else
            {
                //其他不可弥补的错误
                socket_close(pstSocketInfo->iSocketfd, pstCurrEpollEvent);
                printf("unrecovable error\n");
                break;
            }
        }
        else if( iRecvNum == 0)
        {
            // 这里表示对端的socket已正常关闭.发送过FIN了。
            socket_close(pstSocketInfo->iSocketfd, pstCurrEpollEvent);
            printf("counterpart has shut off\n");
            break;
        }

        // iRecvNum > 0
        iCounter += iRecvNum;
        if ( iRecvNum == BUFF_SIZE)
        {
            continue;   // 需要再次读取
        }
        else // 0 < iRecvNum < BUFF_SIZE
        {
            // 安全读完
            iIsReadOver = 1;
            break; // 退出while(1),表示已经全部读完数据
        }
    }

    //Read over!
    if (iIsReadOver)
    {
        pstSocketInfo->acSocketBuff[iCounter] = '\0';

        log_i("recv from client : %s\n", pstSocketInfo->acSocketBuff);

        pstSocketInfo->stEpollEvent.data.fd=pstSocketInfo->iSocketfd;
        pstSocketInfo->stEpollEvent.events = EPOLLOUT | EPOLLET;

        iRet = epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_MOD, pstSocketInfo->iSocketfd, &(pstSocketInfo->stEpollEvent));
        if(iRet < 0)
        {
            log_e("epoll_ctl\n");
            socket_close(pstSocketInfo->iSocketfd, pstCurrEpollEvent);
            return ERR;
        }
        
    }

    return OK;
}

int socket_send(IN struct epoll_event *pstCurrEpollEvent, IO pstSOCKET_INFO pstSocketInfo)
{
    const char str[] = "Send :hello!!!\n";
    int iCounter = 0;
    int iIsWritenOver = 0;
    int iIsWritenLen = 0;
    int iCurrSocketfd = pstCurrEpollEvent->data.fd;
    char *pcSendBuffHead = pstSocketInfo->acSocketSendBuff;

    memset(pstSocketInfo->acSocketSendBuff, 0, sizeof(pstSocketInfo->acSocketSendBuff));
    memcpy(pstSocketInfo->acSocketSendBuff, str, sizeof(str));

    log_i("Write %s", pstSocketInfo->acSocketSendBuff);

    while(1)
    {
        // 确保 iCurrSocketfd 是非阻塞的
        iIsWritenLen = send(iCurrSocketfd, pcSendBuffHead + iCounter, BUFF_SIZE, 0);
        if (iIsWritenLen == -1)
        {
            if (errno == EAGAIN)
            {
                // 对于nonblocking 的socket而言，这里说明了已经全部发送成功了
                iIsWritenOver = 1;
                break;
            }
            else if(errno == ECONNRESET)
            {
                // 对端重置,对方发送了RST
                socket_close(iCurrSocketfd, pstCurrEpollEvent);
                printf("counterpart send out RST\n");
                break;
            }
            else if (errno == EINTR)
            {
                // 被信号中断
                continue;
            }
            else
            {
                // 其他错误
            }
        }

        if (iIsWritenLen == 0)
        {
            // 这里表示对端的socket已正常关闭.
            socket_close(iCurrSocketfd, pstCurrEpollEvent);
            printf("counterpart has shut off\n");
            break;
        }

        // 以下的情况是writenLen > 0
        iCounter += iIsWritenLen;
        if (iIsWritenLen == BUFF_SIZE)
        {
            // 可能还没有写完
            continue;
        }
        else // 0 < iIsWritenLen < BUFF_SIZE
        {
            // 已经写完了
            iIsWritenOver = 1;
            break; // 退出while(1)
        }
    }

    if (iIsWritenOver)
    {
        pstSocketInfo->stEpollEvent.data.fd = pstSocketInfo->iSocketfd;
        pstSocketInfo->stEpollEvent.events=EPOLLIN | EPOLLET;

        epoll_ctl(pstSocketInfo->iEpollfd,EPOLL_CTL_MOD, pstSocketInfo->iSocketfd, &(pstSocketInfo->stEpollEvent));
    }

    return OK;
}

int main()
{
    int iRet = 0;
    int iFdNum = 0;
    int i = 0;

    stSOCKET_INFO stSocketInfo;
    memset(&stSocketInfo, 0, sizeof(stSocketInfo));

    iRet = socket_init(&stSocketInfo);
    if (iRet != OK)
    {
        log_e("socket_init failed!");
        return ERR;
    }

    while(1)
    {
        iRet = socket_wait(&stSocketInfo, &iFdNum);
        if (iRet != OK)
        {
            continue;
        }
        //处理所发生的所有事件
        for(i = 0; i < iFdNum; ++i)
        {
            //如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            if(stSocketInfo.astEpollEventsList[i].data.fd == stSocketInfo.iSocketfd)
            {
                log_i("Receive a new link!\n");
                iRet = socket_add(&stSocketInfo);
                if (iRet != OK)
                {
                    log_e("socket_add failed!");
                    return ERR;
                }
            }
            else if(stSocketInfo.astEpollEventsList[i].events & EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
            {
                iRet = socket_recv(&(stSocketInfo.astEpollEventsList[i]), &stSocketInfo);
                if (iRet != OK)
                {
                    log_e("socket_recv failed!");
                    return ERR;
                }
            }
            else if(stSocketInfo.astEpollEventsList[i].events & EPOLLOUT) // 如果有数据发送
            {
                iRet = socket_send(&(stSocketInfo.astEpollEventsList[i]), &stSocketInfo);
                if (iRet != OK)
                {
                    log_e("socket_send failed!");
                    return ERR;
                }
            }
        }
    }
    
    return OK;
}



#ifdef _cplusplus 
} 
#endif
