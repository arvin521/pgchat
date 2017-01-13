#ifdef _cplusplus 
extern "C"{ 
#endif

#include "client.h"

const char* ipaddr = "127.0.0.1";
const int c_iClientPort = 5000;

void * recv_thread_proc(void *arg)
{
    int iSocketfd = *((int *)arg);
    char buffer[1024];

    log_i("Recv Thread Start!\n");

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        
        if(recv(iSocketfd, buffer, sizeof(buffer), 0) < 0)
        {
            log_e("client read failed!\n");
        }
        
        log_i("client read content: %s\n", buffer);
    }

    pthread_exit(NULL);
}

void * send_thread_proc(void *arg)
{
    int iSocketfd = *((int *)arg);

    log_i("Send Thread Start!\n");

    while (1)
    {
        if (send(iSocketfd, "Hello Arvin!\n", 13, 0) < 0)
        {
            log_e("client send failed!\n");
        }
    }

    pthread_exit(NULL);
}

int main(void)
{
    int iSocketfd = 0;

    pthread_t RecvThreadHandle = 0;
    pthread_t SendThreadHandle = 0;

    if((iSocketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log_e("socket create error");
    }

    struct sockaddr_in  serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(c_iClientPort);
    inet_pton(AF_INET, ipaddr, &serveraddr.sin_addr.s_addr);
    if(connect(iSocketfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
        log_e("connect error");
        close(iSocketfd);
        return ERR;
    }

    if(pthread_create(&RecvThreadHandle, NULL, recv_thread_proc, (void *)(&iSocketfd))) //comment2
    {
        log_e("create recv thread failed!");
        close(iSocketfd);
        return ERR;
    }

    if(pthread_create(&SendThreadHandle, NULL, send_thread_proc, (void *)(&iSocketfd))) //comment2
    {
        log_e("create send thread failed!");
        close(iSocketfd);
        return ERR;
    }

    // wait recv thread exit
    if(RecvThreadHandle) 
    {
        log_i("Recv Thread Exit!\n");
        pthread_join(RecvThreadHandle, NULL);
    }

    // wait send thread exit
    if(SendThreadHandle) 
    {
        log_i("Send Thread Exit!\n");
        pthread_join(SendThreadHandle, NULL);
    }

    log_i("all thread exit, close socket!");
    close(iSocketfd);
    return 0;
}

#ifdef _cplusplus 
} 
#endif
