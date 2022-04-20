#include <gtest/gtest.h>
// --------------------------------------------------------------------------
#include "external/ThreadPool.h"
#include "src/file/SegmentManager.h"
#include <filesystem>
#include <unordered_set>
// --------------------------------------------------------------------------
using namespace std;
using namespace file;
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
static const string DIRNAME = "/tmp/tester_test_segment_manager";
void setup() {
    std::filesystem::remove_all(DIRNAME.c_str());
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
TEST(SegmentManager, StoreSingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    SegmentManager<BLOCK_SIZE> segmentManager(DIRNAME, 1.25);
    std::unordered_set<size_t> ids;
    for (int i = 0; i < 1000; i++) {
        size_t id = segmentManager.createBlock();
        ids.insert(id);
        for (int j = 0; j < 1000; j++) {
            array<unsigned char, BLOCK_SIZE> arr;
            fill(arr.begin(), arr.end(), id % 256);
            segmentManager.writeBlock(id, move(arr));
        }
    }
    ASSERT_EQ(ids.size(), 1000);
    for (size_t id: ids) {
        array<unsigned char, BLOCK_SIZE> arr = segmentManager.readBlock(id);
        for (unsigned char c: arr) {
            ASSERT_EQ(static_cast<int>(c), id % 256);
        }
    }
}
// --------------------------------------------------------------------------
TEST(SegmentManager, StoreMultiThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    SegmentManager<BLOCK_SIZE> segmentManager(DIRNAME, 1.25);
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    for (int i = 0; i < 1000; i++) {
        calls.emplace_back(threadPool.enqueue([&segmentManager]() {
            size_t id = segmentManager.createBlock();
            for (int j = 0; j < 1000; j++) {
                array<unsigned char, BLOCK_SIZE> arr;
                fill(arr.begin(), arr.end(), id % 256);
                segmentManager.writeBlock(id, move(arr));
            }
            array<unsigned char, BLOCK_SIZE> arr2 = segmentManager.readBlock(id);
            for (unsigned char c: arr2) {
                ASSERT_EQ(static_cast<int>(c), id % 256);
            }
        }));
    }
    for (auto& call: calls) {
        call.get();
    }
}
// --------------------------------------------------------------------------
TEST(SegmentManager, KeepData) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    std::unordered_set<size_t> ids;
    {
        SegmentManager<BLOCK_SIZE> segmentManager(DIRNAME, 1.25);
        for (int i = 0; i < 10000; i++) {
            size_t id = segmentManager.createBlock();
            ids.insert(id);
            array<unsigned char, BLOCK_SIZE> arr;
            fill(arr.begin(), arr.end(), id % 256);
            segmentManager.writeBlock(id, move(arr));
        }
        segmentManager.flush();
    }
    SegmentManager<BLOCK_SIZE> segmentManager(DIRNAME, 1.25);
    for (size_t id: ids) {
        array<unsigned char, BLOCK_SIZE> arr = segmentManager.readBlock(id);
        for (unsigned char c: arr) {
            ASSERT_EQ(static_cast<int>(c), id % 256);
        }
    }
}
// --------------------------------------------------------------------------
TEST(SegmentManager, DeleteSingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    SegmentManager<BLOCK_SIZE> segmentManager(DIRNAME, 1.25);
    unordered_set<uint64_t> ids;
    for (int i = 0; i < 10000; i++) {
        size_t id = segmentManager.createBlock();
        ids.insert(id);
        array<unsigned char, BLOCK_SIZE> arr;
        fill(arr.begin(), arr.end(), id % 256);
        segmentManager.writeBlock(id, move(arr));
        if (i % 5 == 0) {
            segmentManager.deleteBlock(id);
            ids.erase(id);
        }
    }
    ASSERT_EQ(ids.size(), 8000);
    for (uint64_t id: ids) {
        array<unsigned char, BLOCK_SIZE> arr = segmentManager.readBlock(id);
        for (unsigned char c: arr) {
            ASSERT_EQ(static_cast<int>(c), id % 256);
        }
    }
}
// --------------------------------------------------------------------------
TEST(SegmentManager, CreateSingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    SegmentManager<BLOCK_SIZE> segmentManager(DIRNAME, 1.25);
    for (int i = 0; i < 100000; i++) {
        segmentManager.createBlock();
    }
}
// --------------------------------------------------------------------------
TEST(SegmentManager, CreateMultiThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    SegmentManager<BLOCK_SIZE> segmentManager(DIRNAME, 1.25);
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    for (int i = 0; i < 100000; i++) {
        calls.emplace_back(threadPool.enqueue([&segmentManager]() {
            segmentManager.createBlock();
        }));
    }
    for (auto& call: calls) {
        call.get();
    }
}
// --------------------------------------------------------------------------
TEST(SegmentManager, DeleteMultiThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    SegmentManager<BLOCK_SIZE> segmentManager(DIRNAME, 1.25);
    unordered_set<uint64_t> ids;
    mutex m;
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    for (int i = 0; i < 10000; i++) {
        calls.emplace_back(threadPool.enqueue([i, &ids, &m, &segmentManager]() {
            size_t id = segmentManager.createBlock();
            {
                unique_lock lock(m);
                ids.insert(id);
            }
            array<unsigned char, BLOCK_SIZE> arr;
            fill(arr.begin(), arr.end(), id % 256);
            segmentManager.writeBlock(id, move(arr));
            if (i % 5 == 0) {
                segmentManager.deleteBlock(id);
                {
                    unique_lock lock(m);
                    ids.erase(id);
                }
            }
        }));
    }
    for (auto& call: calls) {
        call.get();
    }
    for (uint64_t id: ids) {
        array<unsigned char, BLOCK_SIZE> arr = segmentManager.readBlock(id);
        for (unsigned char c: arr) {
            ASSERT_EQ(static_cast<int>(c), id % 256);
        }
    }
}
// --------------------------------------------------------------------------