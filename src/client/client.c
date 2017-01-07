#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "comm.h"

const char* ipaddr = "127.0.0.1";

int main(void)
{
    int fd;
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket create error");
    printf("client: socket created\n");

    struct sockaddr_in  serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;

    //主机字节序改网络字节序
    //host to network
    serveraddr.sin_port = htons(port);
    //pointer to network
    inet_pton(AF_INET, ipaddr, &serveraddr.sin_addr.s_addr);
    //调用connect指定服务器的ip
    if(connect(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        perror("connect error");
    printf("client: connect OK\n");

    if (send(fd, "Hello Arvin!", 12, 0) < 0){
        perror("send");
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    size_t size;
    if((size = recv(fd, buffer, sizeof(buffer), 0)) < 0)
        perror("read error");
    printf("client read content: %s\n", buffer);

    close(fd);

    return 0;
}