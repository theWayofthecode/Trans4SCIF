
SRC=../src

TESTS=modules_test.cpp 
SRCS=main.cpp $(TESTS) $(SRC)/ctl_messages.cpp $(SRC)/scifnode.cpp $(SRC)/virt_circbuf.cpp $(SRC)/circbuf.cpp $(SRC)/rmawindow.cpp $(SRC)/trans4node.cpp
HEADERS=catch.hpp $(SRC)/ctl_messages.hpp $(SRC)/scifnode.hpp $(SRC)/virt_circbuf.hpp $(SRC)/circbuf.hpp $(SRC)/rmawindow.hpp $(SRC)/scifepd.hpp $(SRC)/constants.hpp $(SRC)/trans4node.hpp catch.hpp
CFLAGS=-std=c++11

all: mainmmic main

mainmmic: $(SRCS) $(HEADERS)
	/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-g++ $(CFLAGS) -D XEONPHI -o mainmmic $(SRCS) -lscif;

main: $(SRCS) $(HEADERS)
	g++ $(CFLAGS) -o main $(SRCS) -lscif

clean:
	rm -f mainmmic main