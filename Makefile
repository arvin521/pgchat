all:
	mkdir -p bin
	cd server; make
	cd client; make

.PHONY:clean
clean:
	-rm bin/*
