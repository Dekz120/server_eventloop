#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include <filesystem>

#include "client.hpp"

Command commandTest(std::string &cmd)
{
  Client client(-1);
  client.updateRequestField(cmd.c_str(), cmd.size());
  client.parseCommand();
  return client.getCommand();
}

TEST_CASE("Command parser test")
{
  std::vector<std::string> input{
    "time\n",
    "echo Hello, my name is Van. I'am performance artist\n",
    "decompress dir/file\ntime\n",
    "compress dir/file",
    "cumpress dir/file\n",
    "time \n",
    "suctiooon",
  };
  for(auto& s : input)
    CHECK(commandTest(s).m_cmd == Command::NotExitsts);
}