/*
 * @file unittests_vfs.cpp
 * @author Mateusz Piesta (mateusz.piesta@mudita.com)
 * @date 17.05.19
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "vfs.hpp"

#include <ticks.hpp>

#include <algorithm>

#include <cstdint>

#include <thread.hpp>

class vfs vfs;

struct vfs_initializer
{
    vfs_initializer()
    {
        vfs.Init();
    }
} vfs_initializer;

TEST_CASE("Test vfs case 1")
{


    const size_t testBufferSize = 1024 * 1024;

    uint8_t testBuffer[testBufferSize] = {0};

    auto fd = vfs.fopen("testFile.txt", "w");
    REQUIRE(fd != nullptr);

    auto bytesWritten = vfs.fwrite(testBuffer, 1, testBufferSize, fd);
    REQUIRE(bytesWritten == testBufferSize);

    auto currFilePos = vfs.ftell(fd);
    REQUIRE(currFilePos == testBufferSize);

    auto fileSize = vfs.filelength(fd);
    REQUIRE(fileSize == testBufferSize);

    currFilePos = vfs.ftell(fd);
    REQUIRE(currFilePos == testBufferSize);

    REQUIRE(vfs.fclose(fd) == 0);

    // current directory is the build dir
    // vfs adds sys/ to the path we need to got outside sys (bad!)
    // and look for some files there
    vfs.mkdir("module-vfs");
    vfs.mkdir("module-vfs/test_dir2");

    fd = vfs.fopen("module-vfs/test1.txt", "a");
    REQUIRE(fd != nullptr);
    REQUIRE(vfs.fclose(fd) == 0);

    fd = vfs.fopen("module-vfs/test2.txt", "a");
    REQUIRE(fd != nullptr);
    REQUIRE(vfs.fclose(fd) == 0);

    auto dirList = vfs.listdir("module-vfs");
    REQUIRE(dirList.size() >= 4);
    for (auto &dir : dirList) {
        if (dir.fileName == "test_dir2") {
            REQUIRE(dir.attributes == vfs::FileAttributes::Directory);
        }
        if (dir.fileName == "test1.txt" || dir.fileName == "test2.txt") {
            REQUIRE(dir.attributes == vfs::FileAttributes::Writable);
        }
    }
}

#define RANDDOM_TESTS 4
TEST_CASE("Random strings")
{
    const std::string allowedChars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string randomIds8, randomIds16, randomIds32;
    randomIds8  = vfs.generateRandomId(8);
    randomIds16 = vfs.generateRandomId(16);
    randomIds32 = vfs.generateRandomId(32);

    REQUIRE(randomIds8.size() == 8);
    for (unsigned int i = 0; i < randomIds8.size(); i++)
        REQUIRE(allowedChars.find_first_of(randomIds8[i]) != std::string::npos);

    REQUIRE(randomIds16.size() == 16);
    for (unsigned int i = 0; i < randomIds16.size(); i++)
        REQUIRE(allowedChars.find_first_of(randomIds16[i]) != std::string::npos);

    REQUIRE(randomIds32.size() == 32);
    for (unsigned int i = 0; i < randomIds32.size(); i++)
        REQUIRE(allowedChars.find_first_of(randomIds32[i]) != std::string::npos);
}

TEST_CASE("CRC32 tests")
{
    unsigned long crc32 = 0;
    char crcBuf[purefs::buffer::crc_char_size];
    std::string randomData = vfs.generateRandomId(128);
    auto fd                = vfs.fopen("testFile.txt", "w");
    REQUIRE(fd != nullptr);

    auto bytesWritten = vfs.fwrite(randomData.c_str(), 1, randomData.size(), fd);
    REQUIRE(bytesWritten == randomData.size());
    REQUIRE(vfs.fclose(fd) == 0);

    fd = vfs.fopen("testFile.txt", "r");
    vfs.computeCRC32(fd, &crc32);
    sprintf(crcBuf, "%lX", crc32);
    auto fdCRC   = vfs.fopen("testFile.txt.crc32", "w");
    bytesWritten = vfs.fwrite(&crcBuf, 1, purefs::buffer::crc_char_size, fdCRC);
    REQUIRE(bytesWritten == purefs::buffer::crc_char_size);
    REQUIRE(vfs.fclose(fdCRC) == 0);
    REQUIRE(vfs.fclose(fd) == 0);

    REQUIRE(vfs.verifyCRC("testFile.txt") == true);
    REQUIRE(vfs.remove("testFile.txt") == 0);
    REQUIRE(vfs.remove("testFile.txt.crc32") == 0);
}

TEST_CASE("File loading and saving")
{
    static constexpr auto test_str = "abcd";
    auto fd                        = vfs.fopen("test1.txt", "w");
    REQUIRE(fd != nullptr);
    const auto slen = std::strlen(test_str);
    REQUIRE(vfs.fwrite(test_str, 1, slen, fd) == slen);
    vfs.fclose(fd);
    std::string fileContents = vfs.loadFileAsString("test1.txt");
    REQUIRE(strcmp(fileContents.c_str(), test_str) == 0);
    vfs.mkdir("module-vfs/test_dirx");
    vfs.replaceWithString("module-vfs/test_dirx/testWrite.txt", "this is a test");
    fileContents = vfs.loadFileAsString("module-vfs/test_dirx/testWrite.txt");
    REQUIRE(strcmp(fileContents.c_str(), "this is a test") == 0);
    REQUIRE(vfs.remove("module-vfs/test_dirx/testWrite.txt") == 0);
}
