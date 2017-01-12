#ifdef _cplusplus 
extern "C"{ 
#endif

#include "server.h"

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
            log_e("socket_wait failed, continue!\n");
            continue;
        }
        // process all things
        for(i = 0; i < iFdNum; ++i)
        {
            // receive a new connect
            if(stSocketInfo.astEpollEventsList[i].data.fd == stSocketInfo.iSocketfdListen)
            {
                log_i("Receive a new link!\n");
                iRet = socket_add(&stSocketInfo);
                if (iRet != OK)
                {
                    log_e("socket_add failed!");
                    return ERR;
                }
            }
            //Already connected users, and receive data, then read.
            else if(stSocketInfo.astEpollEventsList[i].events & EPOLLIN)
            {
                iRet = socket_recv(&(stSocketInfo.astEpollEventsList[i]), &stSocketInfo);
                if (iRet != OK)
                {
                    log_e("socket_recv failed!");
                    return ERR;
                }
            }
            // no data to send.
            else if(stSocketInfo.astEpollEventsList[i].events & EPOLLOUT)
            {
                char acStr[] = "aaaabbbb\n";
                int iBuffSize = sizeof(acStr);

                iRet = socket_send(&(stSocketInfo.astEpollEventsList[i]), acStr, iBuffSize, &stSocketInfo);
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
