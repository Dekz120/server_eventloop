SOURCES = elooplib/archive.cpp elooplib/client_task.cpp elooplib/client.cpp\
elooplib/eventloop.cpp elooplib/node.cpp elooplib/server.cpp elooplib/threadpool.cpp\
elooplib/tp_tasks.cpp
INCLUDES =  ./elooplib/
OBJECTS=$(SOURCES:.cpp=.o)

CXX = g++
CXXFLAGS = -std=c++2a -g -fsanitize=address -Wall -Wextra #-Werror

bvnserver: main.o eventloop.a
	$(CXX) $(CXXFLAGS) -lz main.o eventloop.a -o $@

main.o: bvnServer/main.cpp $(INCLUDES)
	$(CXX)  -c $(CXXFLAGS) -I$(INCLUDES) bvnServer/main.cpp

eventloop.a: $(OBJECTS) 
	ar rcs eventloop.a $(OBJECTS)
	
elooplib/%.o: %.cpp
	$(CXX)  -c $(CXXFLAGS) -I$(INCLUDES) $?

clean:
	rm -f elooplib/*.o *.o *.a bvnserver

