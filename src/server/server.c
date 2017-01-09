#ifdef _cplusplus 
extern "C"{ 
#endif

#include "inc/server/server.h"

int main(int argc, char *argv[])  
{
    int i = 0;
    int iRet = 0;
    int iActiveLinkNum = 0;

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
        iActiveLinkNum = socket_wait(&stSocketInfo);
        for(i = 0; i < iActiveLinkNum; ++i)
        {
            //如果检测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            if(stSocketInfo.astEpollEvents[i].data.fd == stSocketInfo.iSockfd)
            {
                log_i("add new connect.");
                iRet = socket_add_link(&stSocketInfo);
                if (iRet != OK)
                {
                    log_e("socket_add_link failed!");
                    return ERR;
                }
            }
            else if(stSocketInfo.astEpollEvents[i].events & EPOLLIN) // 如果是已经连接的用户，并且收到数据，则读入
            {
                log_i("recv data.");
                iRet = socket_recv(&(stSocketInfo.astEpollEvents[i]), &stSocketInfo);
                if (iRet != OK)
                {
                    log_e("socket_recv failed!");
                    return ERR;
                }
            }
            else if(stSocketInfo.astEpollEvents[i].events & EPOLLOUT) // 如果有数据发送，则发送
            {
                log_i("send data.");
                iRet = socket_send(&(stSocketInfo.astEpollEvents[i]));
                if (iRet != OK)
                {
                    log_e("socket_send failed!");
                    return ERR;
                }
            }
        }
    }

    return 0;
}




#ifdef _cplusplus 
} 
#endif