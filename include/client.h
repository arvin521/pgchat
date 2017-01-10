#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

const char* ipaddr = "127.0.0.1";
#define SERV_PORT 5000