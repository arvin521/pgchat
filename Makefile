CC := gcc
CFLAGS := -g -Wall
 
CUR_DIR := $(shell pwd)
#TOP_DIR := $(CUR_DIR)/..
SRC_DIR := $(CUR_DIR)/src
INC_DIR := $(CUR_DIR)/include
BIN_DIR := $(CUR_DIR)/bin
BUILD_DIR := $(CUR_DIR)/build

TARGETS := server client 

.PHONY:all 
all : $(TARGETS)
	-rm *.o
 
server: server.o
	$(CC) -o $(BIN_DIR)/server server.o 

client: client.o
	$(CC) -o $(BIN_DIR)/client client.o
 
server.o: ${INC_DIR}/server.h
	$(CC) -c ${SRC_DIR}/server.c -I${INC_DIR}

client.o: ${INC_DIR}/client.h
	$(CC) -c ${SRC_DIR}/client.c -I${INC_DIR}

.PHONY: clean 
clean :
	-rm bin/* *.o
