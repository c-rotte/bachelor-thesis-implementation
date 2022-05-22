#include <gtest/gtest.h>
// --------------------------------------------------------------------------
#include "src/file/FileManager.h"
#include "thirdparty/ThreadPool/ThreadPool.h"
#include <filesystem>
#include <unordered_set>
// --------------------------------------------------------------------------
using namespace std;
using namespace file;
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
static const string FILENAME = "/tmp/tester_test_file_manager";
void setup() {
    std::filesystem::remove(FILENAME.c_str());
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
TEST(FileManager, StoreSingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    FileManager<BLOCK_SIZE> fileManager(FILENAME, 1000);
    for (int i = 0; i < 1000; i++) {
        size_t id = fileManager.createBlock();
        ASSERT_EQ(id, i);
        array<unsigned char, BLOCK_SIZE> arr;
        fill(arr.begin(), arr.end(), id % 256);
        fileManager.writeBlock(id, move(arr));
    }
    ASSERT_THROW(fileManager.createBlock(), std::runtime_error);
    for (int i = 0; i < 1000; i++) {
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(i);
        for (unsigned char c: arr) {
            ASSERT_EQ(static_cast<int>(c), i % 256);
        }
    }
    ASSERT_EQ(fileManager.allocatedBlocks(), 1000);
}
// --------------------------------------------------------------------------
TEST(FileManager, KeepData) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    {
        FileManager<BLOCK_SIZE> fileManager(FILENAME, 1000);
        for (int i = 0; i < 1000; i++) {
            size_t id = fileManager.createBlock();
            ASSERT_EQ(id, i);
            array<unsigned char, BLOCK_SIZE> arr;
            fill(arr.begin(), arr.end(), id % 256);
            fileManager.writeBlock(id, move(arr));
        }
        fileManager.flush();
    }
    ASSERT_THROW(FileManager<BLOCK_SIZE * 2>(FILENAME, 1000), std::runtime_error);
    FileManager<BLOCK_SIZE> fileManager(FILENAME, 1000);
    for (int i = 0; i < 1000; i++) {
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(i);
        for (unsigned char c: arr) {
            ASSERT_EQ(static_cast<int>(c), i % 256);
        }
    }
    ASSERT_EQ(fileManager.allocatedBlocks(), 1000);
}
// --------------------------------------------------------------------------
TEST(FileManager, DeleteSingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    FileManager<BLOCK_SIZE> fileManager(FILENAME, 1000);
    unordered_set<uint64_t> ids;
    for (int i = 0; i < 1000; i++) {
        size_t id = fileManager.createBlock();
        ids.insert(id);
        array<unsigned char, BLOCK_SIZE> arr;
        fill(arr.begin(), arr.end(), id % 256);
        fileManager.writeBlock(id, move(arr));
        if (i % 5 == 0) {
            fileManager.deleteBlock(id);
            ids.erase(id);
        }
    }
    ASSERT_EQ(ids.size(), 800);
    for (uint64_t id: ids) {
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(id);
        for (unsigned char c: arr) {
            ASSERT_EQ(static_cast<int>(c), id % 256);
        }
    }
    ASSERT_LE(fileManager.allocatedBlocks(), 1000);
}
// --------------------------------------------------------------------------
TEST(FileManager, WriteMultiThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    FileManager<BLOCK_SIZE> fileManager(FILENAME, 10000);
    for (int i = 0; i < 10000; i++) {
        size_t id = fileManager.createBlock();
        ASSERT_EQ(id, i);
        array<unsigned char, BLOCK_SIZE> arr;
        fill(arr.begin(), arr.end(), 1);
        fileManager.writeBlock(id, move(arr));
    }
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    for (int i = 0; i < 10000; i++) {
        calls.emplace_back(threadPool.enqueue([&fileManager]() {
            for (int j = 0; j < 50; j++) {
                size_t randID = rand() % 1000;
                unsigned char randVal = rand() % 256;
                array<unsigned char, BLOCK_SIZE> arr;
                fill(arr.begin(), arr.end(), randVal);
                fileManager.writeBlock(randID, move(arr));
            }
        }));
    }
    for (auto& call: calls) {
        call.get();
    }
    for (int i = 0; i < 10000; i++) {
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(i);
        unsigned char val = arr[0];
        for (unsigned char c: arr) {
            ASSERT_EQ(c, val);
        }
    }
}