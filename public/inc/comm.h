#ifndef __COMM_H__
#define __COMM_H__

#include <errno.h>

// #include "log/log.h"

/************************* Macro definition *************************/
#define OK 0
#define ERR -1

#define IN
#define OUT
#define IO

#define log_d printf
#define log_i printf
#define log_w printf
#define log_e perror

/************************* const definition *************************/


/************************* union definition *************************/

typedef enum MSG_TYPE
{
    //client to server
    MSG_C2S_REGISTER = 100, //register

    //server to client
    MSG_S2C_FRIEND_STATUS, //friends status
}enMSG_TYPE;

/************************* struct definition ************************/

typedef struct MSG_INFO
{
    enMSG_TYPE enMsgType;
    unsigned int pcMsgBuffSize;
    char *pcMsgBuff;
}stMSG_INFO, *pstMSG_INFO;

/************************* Parameter macro definition ***************/

#define CHECK_PARAM_RET(p, ret) \
    if (NULL == p)              \
    {                           \
        log_e("param is Null!");\
        return ret;             \
    }

/************************* API function declaration ******************/



#endif