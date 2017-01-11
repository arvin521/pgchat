
CFLAGS := -g -Wall -O3
CC := gcc $(CFLAGS)

Pwd := $(shell pwd)
INCLUDE := -Iinc
INCLUDE += -I$(Pwd)/../public/inc

#Sources := $(shell find -iname *.c )
Sources := ./src/server.c

Objs := $(Sources:.c=.o)
  
server:$(Objs)  
	$(CC) -o ../bin/$@ $(Objs)
	-rm $(Sources:.c=.o)
  
-include $(Sources:.c=.d)  

%.d: %.c
	@set -e; rm -f $@; /
	$(CC) -M $(CPPFLAGS) $< $(INCLUDE) > $@.; /
	sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@.> $@; /
	rm -f $@.

$(Objs):%o:%c
	$(CC) -c $< -o $@ $(INCLUDE)

.PHONY:clean
clean:
	-rm $(Sources:.c=.o)
