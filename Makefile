LSOURCES= elooplib/server.cpp elooplib/client.cpp elooplib/eventloop.cpp
SOURCES = elooplib/threadpool.hpp bvnServer/main.cpp
HEADDIR = ./elooplib
HEADERS = ./elooplib/*.hpp
OBJFILES = eventloop.o client.o server.o node.o

CXX = g++
CXXFLAGS = -std=c++17 -g -fsanitize=address -Wall -Wextra #-Werror

#all: bvnserver clean

bvnserver: main.o eventloop.a
	$(CXX) $(CXXFLAGS) main.o eventloop.a -o $@

main.o: $(SOURCES) $(HEADERS)
	$(CXX)  -c $(CXXFLAGS) -I$(HEADDIR) $(SOURCES) 

eventloop.a: $(OBJFILES)
	ar rcs eventloop.a $(OBJFILES)
	
eventloop.o: $(LSOURCES) 
	$(CXX) -c $(CXXFLAGS)  elooplib/eventloop.cpp
	
client.o: elooplib/client.cpp elooplib/node.cpp elooplib/threadpool.hpp elooplib/archive.hpp
	$(CXX) -c $(CXXFLAGS) elooplib/client.cpp
	
server.o: elooplib/server.cpp elooplib/node.cpp
	$(CXX) -c $(CXXFLAGS)  elooplib/server.cpp
node.o: elooplib/node.cpp 
	$(CXX) -c $(CXXFLAGS)  $?

clean:
	rm *.o *.a 

