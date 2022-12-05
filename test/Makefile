CXX = g++
CXXFLAGS = -std=c++2a
INCLUDES = ../elooplib/

all: cmd_parser_ut file_cmd_ut
##
cmd_parser_ut: ./cmd_parser/command_parser.cpp
	$(CXX) $(CXXFLAGS) -g -I$(INCLUDES) ../elooplib/*.cpp $? \
	-pthread -lz -o ./cmd_parser/$@

file_cmd_ut: ./file_cmd/file_cmd.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDES) ../elooplib/archive.cpp -lz $? -o ./file_cmd/file_cmd_ut
##
clean:
	rm -f ./*/*.gz ./file_cmd/*1_2.txt ./*/*_ut