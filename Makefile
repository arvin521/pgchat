# export C_INCLUDE_PATH=C_INCLUDE_PATH:/home/wwj/workspace/prj/pgchat

# VPATH = src:include

# all :
# 	gcc -o bin/server src/server.c
# 	gcc -o bin/client src/client.c

#VPATH=include:src
vpath %.c src
vpath %.h include

server:server.o
	@gcc -o bin/$@ server.o

client:client.o
	@gcc -o bin/$@ client.o

server.o:server.c server.h
	@gcc -c $< -Iinclude

client.o:client.c client.h
	@gcc -c $< -Iinclude

.PHONY:clean
clean:
	-rm *.o

