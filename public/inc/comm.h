#ifndef __COMM_H__
#define __COMM_H__

#include <errno.h>

// #include "log/log.h"

/************************* 宏定义 *************************/
#define OK 0
#define ERR -1

//定义函数参数输入还是输岀
#define IN
#define OUT
#define IO

/************************* 常量定义 *************************/
const int port = 12345;

/************************* 参数宏定义 *************************/

#define CHECK_PARAM_RET(p, ret) \
    if (NULL == p)              \
    {                           \
        log_e("param is Null!");                \
        return ret;             \
    }

/************************* API函数声明 *************************/

#endif