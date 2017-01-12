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

#define log_d printf
#define log_i printf
#define log_w printf
#define log_e perror

/************************* 常量定义 *************************/


/************************* 参数宏定义 *************************/

#define CHECK_PARAM_RET(p, ret) \
    if (NULL == p)              \
    {                           \
        log_e("param is Null!");                \
        return ret;             \
    }

/************************* API函数声明 *************************/


#endif