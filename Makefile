all:
	mkdir -p bin
	cd server; make
	cd client; make

.PHONY : clean  
clean:  
	cd server; make clean
	cd client; make clean
