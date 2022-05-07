#include <gtest/gtest.h>
// --------------------------------------------------------------------------
#include "external/ThreadPool.h"
#include "src/buffer/PageBuffer.h"
#include "utils/SimpleBinaryTree.h"
#include <filesystem>
#include <new>
#include <random>
#include <ranges>
#include <unordered_set>
// --------------------------------------------------------------------------
using namespace std;
using namespace buffer;
using namespace utils;
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
TEST(PageBuffer, SingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 100;
    PageBuffer<BLOCK_SIZE, PAGE_AMOUNT> pageBuffer(DIRNAME, 1.25);
    vector<size_t> ids;
    {
        unordered_set<size_t> idSet;
        for (int i = 0; i < 1000; i++) {
            ids.push_back(pageBuffer.createPage());
            idSet.insert(ids.back());
        }
        ASSERT_EQ(ids.size(), idSet.size());
    }
    {
        unordered_set<size_t> idSet;
        for (int i = 0; i < 100; i++) {
            size_t id = ids[rand() % ids.size()];
            if (idSet.contains(id)) {
                continue;
            }
            idSet.insert(id);
            pageBuffer.pinPage(id, rand() % 2, true);
        }
        for (size_t id: idSet) {
            pageBuffer.unpinPage(id, rand() % 2);
        }
    }
    for (size_t id: ids) {
        auto& page = pageBuffer.pinPage(id, rand() % 2, true);
        ASSERT_EQ(page.id, id);
        page.data.fill(page.id % 256);
        pageBuffer.unpinPage(id, true);
    }
    {
        unordered_set<size_t> idSet;
        for (int i = 0; i < 100; i++) {
            size_t id = ids[rand() % ids.size()];
            if (idSet.contains(id)) {
                continue;
            }
            idSet.insert(id);
            pageBuffer.pinPage(id, rand() % 2);
        }
        for (size_t id: idSet) {
            pageBuffer.unpinPage(id, rand() % 2);
        }
    }
    for (size_t id: ids) {
        auto& page = pageBuffer.pinPage(id, rand() % 2);
        ASSERT_EQ(page.id, id);
        for (unsigned char c: page.data) {
            ASSERT_EQ(c, id % 256);
        }
        pageBuffer.unpinPage(id, false);
    }
}
// --------------------------------------------------------------------------
TEST(PageBuffer, KeepDataSingleThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 100;
    vector<size_t> ids;
    {
        PageBuffer<BLOCK_SIZE, PAGE_AMOUNT> pageBuffer(DIRNAME, 1.25);
        {
            unordered_set<size_t> idSet;
            for (int i = 0; i < 1000; i++) {
                ids.push_back(pageBuffer.createPage());
                idSet.insert(ids.back());
            }
            ASSERT_EQ(ids.size(), idSet.size());
        }
        {
            unordered_set<size_t> idSet;
            for (int i = 0; i < 100; i++) {
                size_t id = ids[rand() % ids.size()];
                if (idSet.contains(id)) {
                    continue;
                }
                idSet.insert(id);
                pageBuffer.pinPage(id, rand() % 2, true);
            }
            for (size_t id: idSet) {
                pageBuffer.unpinPage(id, rand() % 2);
            }
        }
        for (size_t id: ids) {
            auto& page = pageBuffer.pinPage(id, rand() % 2, true);
            ASSERT_EQ(page.id, id);
            page.data.fill(page.id % 256);
            pageBuffer.unpinPage(id, true);
        }
        {
            unordered_set<size_t> idSet;
            for (int i = 0; i < 100; i++) {
                size_t id = ids[rand() % ids.size()];
                if (idSet.contains(id)) {
                    continue;
                }
                idSet.insert(id);
                pageBuffer.pinPage(id, rand() % 2);
            }
            for (size_t id: idSet) {
                pageBuffer.unpinPage(id, rand() % 2);
            }
        }
        for (size_t id: ids) {
            auto& page = pageBuffer.pinPage(id, rand() % 2);
            ASSERT_EQ(page.id, id);
            for (unsigned char c: page.data) {
                ASSERT_EQ(c, id % 256);
            }
            pageBuffer.unpinPage(id, false);
        }
        pageBuffer.flush();
    }
    PageBuffer<BLOCK_SIZE, PAGE_AMOUNT> pageBuffer(DIRNAME, 1.25);
    for (size_t id: ids) {
        auto& page = pageBuffer.pinPage(id, rand() % 2);
        ASSERT_EQ(page.id, id);
        for (unsigned char c: page.data) {
            ASSERT_EQ(c, id % 256);
        }
        pageBuffer.unpinPage(id, false);
    }
}
// --------------------------------------------------------------------------
TEST(PageBuffer, Full) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 100;
    PageBuffer<BLOCK_SIZE, PAGE_AMOUNT> pageBuffer(DIRNAME, 1.25);
    vector<size_t> ids;
    for (int i = 0; i < 1000; i++) {
        size_t id = pageBuffer.createPage();
        ids.push_back(id);
    }
    for (int i = 0; i < 100; i++) {
        pageBuffer.pinPage(ids[i], rand() % 2, true);
    }
    ASSERT_THROW(pageBuffer.pinPage(ids[100], rand() % 2), std::runtime_error);
}
// --------------------------------------------------------------------------
TEST(PageBuffer, MultiThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 100;
    PageBuffer<BLOCK_SIZE, PAGE_AMOUNT> pageBuffer(DIRNAME, 1.25);
    vector<size_t> ids;
    mutex idsMutex;
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    {
        unordered_set<size_t> idSet;
        for (int i = 0; i < 1000; i++) {
            calls.emplace_back(threadPool.enqueue([&pageBuffer, &ids, &idSet, &idsMutex]() {
                size_t id = pageBuffer.createPage();
                unique_lock idsLock(idsMutex);
                ids.push_back(id);
                idSet.insert(ids.back());
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
        ASSERT_EQ(ids.size(), idSet.size());
    }
    for (size_t id: ids) {
        calls.emplace_back(threadPool.enqueue([id, &pageBuffer]() {
            auto& page = pageBuffer.pinPage(id, true, true);
            ASSERT_EQ(page.id, id);
            page.data.fill(page.id % 256);
            pageBuffer.unpinPage(id, true);
        }));
    }
    for (auto& call: calls) {
        call.get();
    }
    calls.clear();
    {
        for (int i = 0; i < 100; i++) {
            size_t id = ids[rand() % ids.size()];
            calls.emplace_back(threadPool.enqueue([id, &pageBuffer]() {
                pageBuffer.pinPage(id, rand() % 2);
                this_thread::sleep_for(chrono::milliseconds(100));
                pageBuffer.unpinPage(id, rand() % 2);
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
    for (int i = 0; i < 100; i++) {
        size_t id = ids[i];
        calls.emplace_back(threadPool.enqueue([id, &pageBuffer]() {
            auto& page = pageBuffer.pinPage(id, true);
            ASSERT_EQ(page.id, id);
            page.data.fill(page.id % 256);
            this_thread::sleep_for(chrono::milliseconds(100));
            pageBuffer.unpinPage(id, true);
        }));
    }
    for (auto& call: calls) {
        call.get();
    }
    calls.clear();
    {
        for (int i = 0; i < 100; i++) {
            size_t id = ids[rand() % ids.size()];
            calls.emplace_back(threadPool.enqueue([id, &pageBuffer]() {
                pageBuffer.pinPage(id, rand() % 2);
                this_thread::sleep_for(chrono::milliseconds(100));
                pageBuffer.unpinPage(id, rand() % 2);
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
    for (int i = 0; i < 100; i++) {
        size_t id = ids[i];
        calls.emplace_back(threadPool.enqueue([id, &pageBuffer]() {
            auto& page = pageBuffer.pinPage(id, false);
            this_thread::sleep_for(chrono::milliseconds(100));
            ASSERT_EQ(page.id, id);
            for (unsigned char c: page.data) {
                ASSERT_EQ(c, id % 256);
            }
            pageBuffer.unpinPage(id, false);
        }));
    }
    for (int i = 0; i < 20; i++) {
        calls[i].get();
        size_t id = ids[i + 50];
        calls[i] = threadPool.enqueue([id, &pageBuffer]() {
            auto& page = pageBuffer.pinPage(id, false);
            this_thread::sleep_for(chrono::milliseconds(100));
            ASSERT_EQ(page.id, id);
            for (unsigned char c: page.data) {
                ASSERT_EQ(c, id % 256);
            }
            pageBuffer.unpinPage(id, false);
        });
    }
    for (auto& call: calls) {
        call.get();
    }
    calls.clear();
}
// --------------------------------------------------------------------------
TEST(PageBuffer, MultiThreaded2) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 100;
    PageBuffer<BLOCK_SIZE, PAGE_AMOUNT> pageBuffer(DIRNAME, 1.25);
    mutex idsMutex;
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    {
        for (int i = 0; i < 1000; i++) {
            calls.emplace_back(threadPool.enqueue([&pageBuffer]() {
                size_t id = pageBuffer.createPage();
                {
                    auto& page = pageBuffer.pinPage(id, true, true);
                    ASSERT_EQ(page.id, id);
                    page.data.fill(id % 256);
                    pageBuffer.unpinPage(id, true);
                }
                {
                    auto& page = pageBuffer.pinPage(id, false);
                    ASSERT_EQ(page.id, id);
                    for (unsigned char c: page.data) {
                        ASSERT_EQ(c, id % 256);
                    }
                    pageBuffer.unpinPage(id, false);
                }
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
}
// --------------------------------------------------------------------------
TEST(PageBuffer, BinaryTreeSingleThreaded) {
    setup();
    SimpleBinaryTree binaryTree(DIRNAME, 1);// wraps around 64 pages
    ThreadPool threadPool(1);
    {
        atomic_int operations = 0;
        vector<future<void>> calls;
        vector<int> ints(10000);
        iota(ints.begin(), ints.end(), -5000);// fill with -5000..4999
        // shuffle the values to balance the tree
        shuffle(ints.begin(), ints.end(), default_random_engine());
        for (int i: ints) {
            if (i == 0) {
                continue;
            }
            calls.emplace_back(threadPool.enqueue([i, &binaryTree, &operations]() {
                binaryTree.insert(i);
                ASSERT_TRUE(binaryTree.search(i));
                operations++;
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
        ASSERT_EQ(operations, 9999);
    }
    {
        atomic_int operations = 0;
        vector<future<void>> calls;
        for (int i = -5000; i < 5000; i++) {
            if (i == 0) {
                continue;
            }
            calls.emplace_back(threadPool.enqueue([i, &binaryTree, &operations]() {
                ASSERT_TRUE(binaryTree.search(i));
                operations++;
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
        ASSERT_EQ(operations, 9999);
    }
}
// --------------------------------------------------------------------------
TEST(PageBuffer, BinaryTreeMultiThreaded) {
    setup();
    SimpleBinaryTree binaryTree(DIRNAME, 1);// wraps around 64 pages
    ThreadPool threadPool(32);
    {
        atomic_int operations = 0;
        vector<future<void>> calls;
        vector<int> ints(10000);
        iota(ints.begin(), ints.end(), -5000);// fill with -5000..4999
        // shuffle the values to balance the tree
        shuffle(ints.begin(), ints.end(), default_random_engine());
        for (int i: ints) {
            if (i == 0) {
                continue;
            }
            calls.emplace_back(threadPool.enqueue([i, &binaryTree, &operations]() {
                binaryTree.insert(i);
                ASSERT_TRUE(binaryTree.search(i));
                operations++;
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
        ASSERT_EQ(operations, 9999);
    }
    {
        atomic_int operations = 0;
        vector<future<void>> calls;
        for (int i = -5000; i < 5000; i++) {
            if (i == 0) {
                continue;
            }
            calls.emplace_back(threadPool.enqueue([i, &binaryTree, &operations]() {
                ASSERT_TRUE(binaryTree.search(i));
                operations++;
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
        ASSERT_EQ(operations, 9999);
    }
}
// --------------------------------------------------------------------------
TEST(PageBuffer, DeleteMultiThreaded) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 100;
    PageBuffer<BLOCK_SIZE, PAGE_AMOUNT> pageBuffer(DIRNAME, 1.25);
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    {
        for (int i = 0; i < 2000; i++) {
            calls.emplace_back(threadPool.enqueue([&pageBuffer]() {
                size_t id = pageBuffer.createPage();
                if (rand() % 2) {
                    pageBuffer.deletePage(id);
                } else {
                    {
                        auto& page = pageBuffer.pinPage(id, true);
                        ASSERT_EQ(page.id, id);
                        page.data.fill(page.id % 256);
                        pageBuffer.unpinPage(id, true);
                    }
                    {
                        auto& page = pageBuffer.pinPage(id, false);
                        ASSERT_EQ(page.id, id);
                        for (unsigned char c: page.data) {
                            ASSERT_EQ(c, id % 256);
                        }
                        pageBuffer.unpinPage(id, false);
                    }
                }
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
}