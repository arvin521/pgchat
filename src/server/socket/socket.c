#ifdef _cplusplus 
extern "C"{ 
#endif

#include "inc/comm.h"
#include "inc/server/socket/socket.h"

int socket_init(OUT pstSOCKET_INFO pstSocketInfo)
{
    int iRet = 0;
    int iSwitchOn = 1;
    struct sockaddr_in serveraddr;

    CHECK_PARAM_RET(pstSocketInfo, ERR);

    pstSocketInfo->iSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (pstSocketInfo->iSockfd < 0)
    {
        log_e("socket create failed!");
        return ERR;
    }

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port=htons(coiPortNum);
    iRet = bind(pstSocketInfo->iSockfd, (sockaddr *)&serveraddr, sizeof(serveraddr));
    if (iRet < 0)
    {
        log_e("socket bind failed!");
        close(pstSocketInfo->iSockfd);
        return ERR;
    }

    //解决 Bind error: Address already in use,使绑定的ip关闭后立刻重新使用
    iRet = setsockopt(pstSocketInfo->iSockfd, SOL_SOCKET, SO_REUSEADDR, &iSwitchOn, sizeof(iSwitchOn));
    if(iRet < 0)
    {    
        log_e("setsockopt failed!");
        close(pstSocketInfo->iSockfd);
        return ERR;
    }

    iRet = listen(pstSocketInfo->iSockfd, LISTEN_NUM);
    if (iRet < 0)
    {
        log_e("socket listen failed!");
        close(pstSocketInfo->iSockfd);
        return ERR;
    }

    pstSocketInfo->iEpollfd = epoll_create(EPOLL_CREATE_NUM);
    if( pstSocketInfo->iEpollfd < 0 ){
        log_e("server: epoll_create failed!");
        close(pstSocketInfo->iSockfd);
        return ERR;
    }

    //设置事件类型为ET
    pstSocketInfo->stEpollEvent.events = EPOLLIN | EPOLLET;
    pstSocketInfo->stEpollEvent.data.fd = pstSocketInfo->iSockfd;

    //注册epoll事件
    iRet = epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_ADD, pstSocketInfo->iSockfd, &(pstSocketInfo->stEpollEvent)
    if(iRet < 0)
    { 
        log_e("server: epoll_ctl failed!");
        close(pstSocketInfo->iSockfd);   
    }

    return OK;
}

int socket_wait(IN pstSOCKET_INFO pstSocketInfo)
{
    CHECK_PARAM_RET(pstSocketInfo, ERR);

    return epoll_wait(pstSocketInfo->iEpollfd, pstSocketInfo->astEpollEvents, EVENTS_NUM, -1);
}

int socket_add_link(IO pstSOCKET_INFO pstSocketInfo)
{
    int iConnfd = 0;
    int iRet = 0;
    struct sockaddr_in stClientAddr;
    int iClientLen = sizeof(stClientAddr);

    CHECK_PARAM_RET(pstSocketInfo, ERR);

    iConnfd = accept(pstSocketInfo->iSockfd, (sockaddr *)&stClientAddr, &iClientLen);
    if(iConnfd < 0)
    { 
        log_e("socket accept failed!");
        close(pstSocketInfo->iSockfd);
        return ERR;
    }

    log_i("remote addr : %s", inet_ntoa(stClientAddr.sin_addr));

    set_no_block(iConnfd);

    //设置事件类型为ET
    pstSocketInfo->stEpollEvent.data.fd = iConnfd;
    pstSocketInfo->stEpollEvent.events = EPOLLIN | EPOLLET;

    //注册epoll事件
    epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_ADD, iConnfd, &(pstSocketInfo->stEpollEvent));

    return OK;
}

int socket_recv(IN struct epoll_event *pstCurrEpollEvent, IO pstSOCKET_INFO pstSocketInfo)
{
    int iSockfd = 0;
    char *pBuffHead = pstSocketInfo->acDataBuff;
    int iRecvLen = 0;
    int iIdx = 0;
    int iIsReadOver = 0;

    CHECK_PARAM_RET(pstSocketInfo, ERR);
    CHECK_PARAM_RET(pstCurrEpollEvent, ERR);

    iSockfd = pstCurrEpollEvent->data.fd;
    if (iSockfd < 0)
    {
        return OK;
    }

    while(1)
    {
        iRecvLen = recv(iSockfd, pBuffHead + iIdx, DATA_BUFF_SIZE, 0);
        if(iRecvLen < 0)
        {
            switch (errno)
            {
                case EAGAIN:
                {
                    // 非阻塞的模式, 当errno为EAGAIN时,表明当前缓冲区已无数据可读
                    iIsReadOver = 1;
                    break;
                }
                case ECONNRESET:
                {
                    // 对方发送了RST
                    close_and_event(iSockfd, pstCurrEpollEvent);
                    log_i("Recv RST, so close link!!!");
                    break;
                }
                case EINTR:
                {
                    continue;
                }
                default:
                {
                    //其他不可弥补的错误
                    close_and_event(iSockfd, pstCurrEpollEvent);
                    log_i("An unrecoverable error occured, so close link!!!");
                    break;
                }
            }
        }
        else if(iRecvLen == 0)
        {
            // 这里表示对端的socket已正常关闭.发送过FIN了。
            close_and_event(iSockfd, pstCurrEpollEvent);
            log_i("Rec FIN, so close link!!!");
            break;
        }
        else if (iRecvLen >= DATA_BUFF_SIZE)
        {
            iIdx += iRecvLen;
            continue;   // 需要再次读取
        }
        else
        {
            // 安全读完
            iIdx += iRecvLen;
            iIsReadOver = 1;
            break; // 退出while(1),表示已经全部读完数据
        }
    }

    if (iIsReadOver)
    {
        // 安全读完了数据
        pBuffHead[iIdx] = '\0';

        log_i("recv data for remote: %s", pBuffHead);

        //修改sockfd上要处理的事件为EPOLLOUT, 设置事件类型为ET
        pstSocketInfo->stEpollEvent.data.fd=iSockfd;
        pstSocketInfo->stEpollEvent.events = EPOLLOUT | EPOLLET;
        epoll_ctl(pstSocketInfo->iEpollfd, EPOLL_CTL_MOD, iSockfd, &(pstSocketInfo->stEpollEvent));
    }

    return OK;
}

int socket_send(IN struct epoll_event *pstCurrEpollEvent)
{
    char acStr[] = "hello from epoll : this is a long string which may be cut by the net\n";

    int isSendOk = 0;
    int iSendDataLen = 0;
    int iIdex = 0;
    int iSocketfd = 0;
    //char *pBuffHead = pstSocketInfo->acDataBuff;
    char *pBuffHead = acStr;

    log_i("send data: %s", line);

    iSocketfd = pstCurrEpollEvent->data.fd;

    while(1)
    {
        // 确保 iSocketfd 是非阻塞的
        iSendDataLen = send(iSocketfd, pBuffHead + iIdex, sizeof(acStr), 0);
        if (iSendDataLen < 0)
        {
            switch (errno)
            {
                case EAGAIN:
                {
                    // 对于非塞的socket而言，这里说明了已经全部发送成功了
                    isSendOk = 1;
                    break;
                }
                case ECONNRESET:
                {
                    // 对端重置,对方发送了RST
                    close_and_event(iSocketfd, pstCurrEpollEvent);
                    log_i("send RST, and close link!!!");
                    break;
                }
                case EINTR:
                {
                    // 被信号中断
                    continue;
                }
                default:
                {
                    // 其他错误
                    log_e("unknow error!");
                }
            }
        }
        else if (iSendDataLen == 0)
        {
            // 这里表示对端的socket已正常关闭.
            close_and_event(iSocketfd, pstCurrEpollEvent);
            log_i("FIN close link!!!");
            break;
        }
        else if (iSendDataLen >= MAXLINE)
        {
            // 可能还没有写完
            iIdex += iSendDataLen;
            continue;
        }
        else
        {
            // 已经写完了
            isSendOk = 1;
            break; // 退出while(1)
        }
    }

    if (isSendOk)
    {
        ev.data.fd=iSocketfd;
        ev.events=EPOLLIN | EPOLLET;

        epoll_ctl(epfd,EPOLL_CTL_MOD,iSocketfd,&ev);
    }

    return OK;
}

/************************* local fun *************************/

static void set_no_block(int iSock)
{
    int opts = 0;

    opts = fcntl(iSock, F_GETFL);
    if(opts < 0)
    {
        log_e("fcntl failed!");
        return;
    }

    opts = opts | O_NONBLOCK;
    if(fcntl(iSock, F_SETFL, opts) < 0)
    {
        log_e("fcntl failed!!");
        return;
    }

    return;
}

static void close_and_event(int iSockfd, struct epoll_event pstEvent)
{
    close(iSockfd);
    pstEvent->data.fd = -1;
}



#ifdef _cplusplus 
} 
#endif