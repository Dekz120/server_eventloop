#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include <filesystem>

#include "archive.hpp"

TEST_CASE("Compression test") {
    FILE* file = fopen("testfile1.txt", "r");
    FILE* cfile = fopen("testfile1.txt.gz", "w");
    CHECK(compress(file, cfile, 7) == 0);
    fclose(file);
    fclose(cfile);

    auto size_before = std::filesystem::file_size("testfile1.txt");

    FILE* cfile2 = fopen("testfile1.txt.gz", "r");
    FILE* dcfile = fopen("testfile1_2.txt", "w");
    CHECK(decompress(cfile2, dcfile) == 0);
    fclose(cfile2);
    fclose(dcfile);

    auto size_after = std::filesystem::file_size("testfile1_2.txt");
    CHECK(size_before == size_after);
}