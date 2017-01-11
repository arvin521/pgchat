
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

/************************* 宏定义 *************************/

#define MAXLINE 256
#define OPEN_MAX 100
#define LISTENQ 20

#define INFTIM 1000

/************************* 常量定义 *************************/
const int c_portnumber = 5000;
/************************* 参数宏定义 *************************/

/************************* 联合体定义 *************************/


/************************* 结构体定义 *************************/

/************************* 本地函数声明 *************************/


/************************* API函数声明 *************************/



#endif
