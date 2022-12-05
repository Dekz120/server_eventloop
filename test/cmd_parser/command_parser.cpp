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

TEST_CASE("Valid command parser test")
{
  std::vector<std::string> input{
      "time\n",
      "echo Hello, my name is Van. I'am performance artist\n",
      "compress dir/file\n",
      "decompress dir/file\ntime\n"};
  for (auto &s : input)
    CHECK(commandTest(s).cmd != Command::Unknown);
}

TEST_CASE("Invalid command parser test")
{
  std::vector<std::string> input{
      "cumpress dir/file\n",
      "time \n",
      "suctiooon",
      "compress dir/file",
  };
  for (auto &s : input)
    CHECK(commandTest(s).cmd == Command::Unknown);
}