all:
	-rm bin/server bin/client
	cd server; make
	cd client; make

.PHONY:clean
clean:
	-rm bin/server bin/client
	cd server; make clean
	cd client; make clean
