#include <gtest/gtest.h>
// --------------------------------------------------------------------------
#include "src/btree/BTree.h"
#include "thirdparty/ThreadPool/ThreadPool.h"
#include <filesystem>
#include <new>
#include <random>
#include <ranges>
// --------------------------------------------------------------------------
using namespace std;
using namespace btree;
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
static const string DIRNAME = "/tmp/tester_test_b_tree";
void setup() {
    std::filesystem::remove_all(DIRNAME.c_str());
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedInsertSmall) {
    setup();
    //ForSingleThreadedInsertSmall<90>::iteration<10>();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    for (uint64_t i = 0; i < 120; i++) {
        tree.insert(i, i);
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedInsertSmallReversed) {
    setup();
    //ForSingleThreadedInsertSmall<90>::iteration<10>();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    for (uint64_t i = 120; i > 0; i--) {
        tree.insert(i - 1, i - 1);
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedInsertSmallRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(120);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedInsertBigRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(5000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
    }
    for (uint64_t i = 0; i < 5000; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedInsertLargeRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(50000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
    }
    for (uint64_t i = 0; i < 50000; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, MultiThreadedInsertLargeRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(50000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                tree.insert(i, i);
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                auto find = tree.find(i);
                ASSERT_TRUE(find);
                ASSERT_EQ(*find, i);
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedUpdateSmallRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(120);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
        tree.update(i, i);
        tree.update(i, 1);
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, 2 * i + 1);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedUpdateBigRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(5000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
        tree.update(i, i);
        tree.update(i, 1);
    }
    for (uint64_t i = 0; i < 5000; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, 2 * i + 1);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedUpdateLargeRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 1024;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(50000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
        tree.update(i, i);
        tree.update(i, 1);
    }
    for (uint64_t i = 0; i < 50000; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, 2 * i + 1);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, MultiThreadedUpdateLargeRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(50000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                tree.insert(i, i);
                tree.update(i, i);
                tree.update(i, 1);
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                auto find = tree.find(i);
                ASSERT_TRUE(find);
                ASSERT_EQ(*find, 2 * i + 1);
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedDeleteSmallRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(120);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
        tree.update(i, 1);
    }
    for (uint64_t i: inserts) {
        tree.update(i, 2);
        tree.erase(i);
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        ASSERT_FALSE(find);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedDeleteBigRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(5000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
        tree.update(i, 1);
    }
    for (uint64_t i: inserts) {
        tree.update(i, 2);
        tree.erase(i);
    }
    for (uint64_t i = 0; i < 5000; i++) {
        auto find = tree.find(i);
        ASSERT_FALSE(find);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedDeleteLargeRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(50000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
        tree.update(i, 1);
    }
    for (uint64_t i: inserts) {
        tree.update(i, 2);
        tree.erase(i);
    }
    for (uint64_t i = 0; i < 50000; i++) {
        auto find = tree.find(i);
        ASSERT_FALSE(find);
    }
}
// --------------------------------------------------------------------------
TEST(BTree, MultiThreadedDeleteLargeRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(50000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                tree.insert(i, i);
                tree.update(i, 1);
                tree.erase(i);
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                auto find = tree.find(i);
                ASSERT_FALSE(find);
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
}
// --------------------------------------------------------------------------
TEST(BTree, SingleThreadedDuplicatesSmallRandom)
// assumptions:
// INSERT overwrites existing entries
// UPDATE does nothing if there is no entry
// DELETE does nothing if there is no entry
{
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(120);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i: inserts) {
        tree.insert(i, i);
        tree.update(i, 1);
        tree.insert(i, i);
        tree.update(i, 1);
        if (i % 2 == 0) {
            tree.erase(i);
            tree.erase(i);
        }
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        if (i % 2 == 0) {
            ASSERT_FALSE(find);
        } else {
            ASSERT_TRUE(find);
            ASSERT_EQ(*find, i + 1);
        }
    }
}
// --------------------------------------------------------------------------
TEST(BTree, MultiThreadedDuplicatesBigRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(5000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                tree.insert(i, i);
                tree.update(i, 1);
                tree.insert(i, i);
                tree.update(i, 1);
                if (i % 2 == 0) {
                    tree.erase(i);
                    tree.erase(i);
                }
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                auto find = tree.find(i);
                if (i % 2 == 0) {
                    ASSERT_FALSE(find);
                } else {
                    ASSERT_TRUE(find);
                    ASSERT_EQ(*find, i + 1);
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
TEST(BTree, MultiThreadedDuplicatesLargeRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 512;
    constexpr size_t PAGE_AMOUNT = 100;
    BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(50000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    ThreadPool threadPool(32);
    vector<future<void>> calls;
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                tree.insert(i, i);
                tree.update(i, 1);
                tree.insert(i, i);
                tree.update(i, 1);
                if (i % 2 == 0) {
                    tree.erase(i);
                    tree.erase(i);
                }
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
        calls.clear();
    }
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                auto find = tree.find(i);
                if (i % 2 == 0) {
                    ASSERT_FALSE(find);
                } else {
                    ASSERT_TRUE(find);
                    ASSERT_EQ(*find, i + 1);
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
TEST(BTree, KeepData) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    vector<uint64_t> inserts(5000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    {
        BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
        for (uint64_t i: inserts) {
            tree.insert(i, i);
            tree.update(i, i);
            tree.update(i, 1);
        }
        tree.flush();
    }
    {
        BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT> tree(DIRNAME, 1.25);
        for (uint64_t i = 0; i < 120; i++) {
            auto find = tree.find(i);
            ASSERT_TRUE(find);
            ASSERT_EQ(*find, 2 * i + 1);
        }
    }
}