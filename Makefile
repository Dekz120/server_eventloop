LSOURCES= elooplib/node.cpp elooplib/server.cpp elooplib/client.cpp elooplib/eventloop.cpp
SOURCES = bvnServer/main.cpp
HEADERS = ./elooplib

CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -Wextra #-Werror

all: bvnserver clean

bvnserver: main.o eventloop.a
	$(CXX) $(CXXFLAGS) main.o eventloop.a -o bvnserver

main.o: $(SOURCES)
	$(CXX)  -c $(CXXFLAGS) -I./elooplib $(SOURCES) 

eventloop.a: eventloop.o client.o server.o node.o
	ar rcs eventloop.a eventloop.o client.o server.o node.o
	
eventloop.o: 
	$(CXX) -c $(CXXFLAGS)  $(LSOURCES)
	
client.o: 
	$(CXX) -c $(CXXFLAGS)  elooplib/eventloop.cpp
	
server.o: 
	$(CXX) -c $(CXXFLAGS)  elooplib/server.cpp
node.o: 
	$(CXX) -c $(CXXFLAGS)  elooplib/node.cpp
clean:
	rm *.o *.a bvnserver

