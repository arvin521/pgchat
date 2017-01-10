export C_INCLUDE_PATH=C_INCLUDE_PATH:/home/wwj/workspace/prj/pgchat

VPATH = src:include

all :
	gcc -o bin/server src/server.c
	gcc -o bin/client src/client.c
