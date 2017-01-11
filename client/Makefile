
CFLAGS := -g -Wall -O3
CC := gcc $(CFLAGS)

INCLUDE := -Iinc
  
#Sources := $(shell find -iname *.c )  
Sources := ./src/client.c

Objs := $(Sources:.c=.o)  
  
client:$(Objs)  
	$(CC) -o ../bin/$@ $(Objs)  
  
-include $(Sources:.c=.d)  

%.d: %.c
	@set -e; rm -f $@; /
	$(CC) -M $(CPPFLAGS) $< $(INCLUDE) > $@.; /
	sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@.> $@; /
	rm -f $@.

$(Objs):%o:%c
	$(CC) -c $< -o $@ $(INCLUDE)

.PHONY : clean  
clean:  
	rm $(Sources:.c=.o)
