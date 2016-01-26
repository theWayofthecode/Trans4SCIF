
BASE=../src

SRCS=main_catch.cpp test_trans4_internals.cpp $(BASE)/ctl_messages.cpp $(BASE)/scifnode.cpp $(BASE)/virt_circbuf.cpp $(BASE)/circbuf.cpp $(BASE)/rmawindow.cpp $(BASE)/trans4node.cpp
HEADERS=$(BASE)/ctl_messages.hpp $(BASE)/scifnode.hpp $(BASE)/virt_circbuf.hpp $(BASE)/circbuf.hpp $(BASE)/rmawindow.hpp $(BASE)/scifepd.hpp $(BASE)/constants.hpp $(BASE)/trans4node.hpp catch.hpp
CFLAGS=-std=c++11

all: mainmmic main

mainmmic: $(SRCS) $(HEADERS)
	/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-g++ $(CFLAGS) -D XEONPHI -o mainmmic $(SRCS) -lscif;

main: $(SRCS) $(HEADERS)
	g++ $(CFLAGS) -o main $(SRCS) -lscif

clean:
	rm -f mainmmic main