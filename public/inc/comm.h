#ifndef __COMM_H__
#define __COMM_H__

#include <errno.h>

// #include "log/log.h"

/************************* Macro definition *************************/
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

/************************* const definition *************************/


/************************* union definition *************************/

/************************* struct definition ************************/

/************************* Parameter macro definition ***************/

#define CHECK_PARAM_RET(p, ret) \
    if (NULL == p)              \
    {                           \
        log_e("param is Null!");\
        return ret;             \
    }

/************************* API function declaration ******************/



#endif