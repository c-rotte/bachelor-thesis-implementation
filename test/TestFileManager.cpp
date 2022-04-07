#include <gtest/gtest.h>
// --------------------------------------------------------------------------
#include "src/file/FileManager.h"
#include "TesterUtils.h"
#include <unordered_set>
#include <filesystem>
// --------------------------------------------------------------------------
using namespace std;
using namespace file;
using namespace utils;
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
static const string FILENAME = "/tmp/data.txt";
void setup() {
    std::remove(FILENAME.c_str());
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
TEST(FileManager, StoreSingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    FileManager<BLOCK_SIZE> fileManager(FILENAME);
    for(int i = 0; i < 1000; i++){
        size_t id = fileManager.createBlock();
        ASSERT_EQ(id, i);
        array<unsigned char, BLOCK_SIZE> arr;
        fill(arr.begin(), arr.end(), id % 256);
        fileManager.writeBlock(id, move(arr));
    }
    for(int i = 0; i < 1000; i++){
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(i);
        for(unsigned char c : arr){
            ASSERT_EQ(static_cast<int>(c), i % 256);
        }
    }
    size_t blocks = 10;
    for(int i = 0; i < 1000; i++){
        if(i == blocks){
            blocks *= 1.25;
        }
    }
    ASSERT_EQ(fileManager.allocatedBlocks(), blocks);
}
// --------------------------------------------------------------------------
TEST(FileManager, KeepFiles) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    {
        FileManager<BLOCK_SIZE> fileManager(FILENAME);
        for(int i = 0; i < 1000; i++){
            size_t id = fileManager.createBlock();
            ASSERT_EQ(id, i);
            array<unsigned char, BLOCK_SIZE> arr;
            fill(arr.begin(), arr.end(), id % 256);
            fileManager.writeBlock(id, move(arr));
        }
    }
    FileManager<BLOCK_SIZE> fileManager(FILENAME);
    for(int i = 0; i < 1000; i++){
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(i);
        for(unsigned char c : arr){
            ASSERT_EQ(static_cast<int>(c), i % 256);
        }
    }
    size_t blocks = 10;
    for(int i = 0; i < 1000; i++){
        if(i == blocks){
            blocks *= 1.25;
        }
    }
    ASSERT_EQ(fileManager.allocatedBlocks(), blocks);
}
// --------------------------------------------------------------------------
TEST(FileManager, DeleteSingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    FileManager<BLOCK_SIZE> fileManager(FILENAME);
    unordered_set<uint64_t> ids;
    for(int i = 0; i < 1000; i++){
        size_t id = fileManager.createBlock();
        ids.insert(id);
        array<unsigned char, BLOCK_SIZE> arr;
        fill(arr.begin(), arr.end(), id % 256);
        fileManager.writeBlock(id, move(arr));
        if(i % 5 == 0){
            fileManager.deleteBlock(id);
            ids.erase(id);
        }
    }
    for(uint64_t id : ids){
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(id);
        for(unsigned char c : arr){
            ASSERT_EQ(static_cast<int>(c), id % 256);
        }
    }
    size_t blocks = 10;
    for(int i = 0; i < 1000; i++){
        if(i == blocks){
            blocks *= 1.25;
        }
    }
    ASSERT_LE(fileManager.allocatedBlocks(), blocks);
}
// --------------------------------------------------------------------------
TEST(FileManager, StoreMultiThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    FileManager<BLOCK_SIZE> fileManager(FILENAME);
    DummyThreadPool threadPool;
    for(int i = 0; i < 10000; i++){
        threadPool.submit([&fileManager](){
            size_t id = fileManager.createBlock();
            array<unsigned char, BLOCK_SIZE> arr;
            fill(arr.begin(), arr.end(), id % 256);
            fileManager.writeBlock(id, move(arr));
        });
    }
    threadPool.join();
    for(int i = 0; i < 10000; i++){
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(i);
        for(unsigned char c : arr){
            ASSERT_EQ(static_cast<int>(c), i % 256);
        }
    }
    size_t blocks = 10;
    for(int i = 0; i < 10000; i++){
        if(i == blocks){
            blocks *= 1.25;
        }
    }
    ASSERT_EQ(fileManager.allocatedBlocks(), blocks);
    size_t fileSize = filesystem::file_size(FILENAME);
}
// --------------------------------------------------------------------------
TEST(FileManager, DeleteMultiThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    FileManager<BLOCK_SIZE> fileManager(FILENAME);
    DummyThreadPool threadPool;
    unordered_set<uint64_t> ids;
    mutex m;
    for(int i = 0; i < 10000; i++){
        threadPool.submit([i, &fileManager, &ids, &m](){
            size_t id = fileManager.createBlock();
            {
                unique_lock lock(m);
                ids.insert(id);
            }
            array<unsigned char, BLOCK_SIZE> arr;
            fill(arr.begin(), arr.end(), id % 256);
            fileManager.writeBlock(id, move(arr));
            if(i % 5 == 0){
                fileManager.deleteBlock(id);
                {
                    unique_lock lock(m);
                    ids.erase(id);
                }
            }
        });
    }
    threadPool.join();
    for(uint64_t id : ids){
        array<unsigned char, BLOCK_SIZE> arr = fileManager.readBlock(id);
        for(unsigned char c : arr){
            ASSERT_EQ(static_cast<int>(c), id % 256);
        }
    }
}
// --------------------------------------------------------------------------