#include <gtest/gtest.h>
// --------------------------------------------------------------------------
#include "external/ThreadPool.h"
#include "src/tree/BeTree.h"
#include <filesystem>
#include <new>
#include <random>
#include <ranges>
#include <unordered_set>
// --------------------------------------------------------------------------
using namespace std;
using namespace tree;
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
static const string DIRNAME = "/tmp/tester_test_be_tree";
void setup() {
    std::filesystem::remove_all(DIRNAME.c_str());
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
template<std::size_t N>
struct ForSingleThreadedInsertSmall {
    template<std::size_t I>
    static void iteration() {
        constexpr size_t BLOCK_SIZE = 4096;
        constexpr size_t PAGE_AMOUNT = 100;
        BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, I> tree(DIRNAME, 1.25);
        for (uint64_t i = 0; i < 1000; i++) {
            tree.insert(i, i);
        }
        for (uint64_t i = 0; i < 1000; i++) {
            auto find = tree.find(i);
            ASSERT_TRUE(find);
            ASSERT_EQ(*find, i);
        }
        if constexpr (I + 1 < N) ForSingleThreadedInsertSmall<N>::iteration<I + 10>();
    }
};
// --------------------------------------------------------------------------
TEST(BeTree, SingleThreadedInsertSmall) {
    setup();
    //ForSingleThreadedInsertSmall<90>::iteration<10>();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50> tree(DIRNAME, 1.25);
    for (uint64_t i = 0; i < 120; i++) {
        tree.insert(i, i);
        std::cout << tree << "\n" << std::endl;
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}
// --------------------------------------------------------------------------
TEST(BeTree, SingleThreadedInsertSmallReversed) {
    setup();
    //ForSingleThreadedInsertSmall<90>::iteration<10>();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50> tree(DIRNAME, 1.25);
    for (uint64_t i = 120; i > 0; i--) {
        tree.insert(i - 1, i - 1);
        std::cout << tree << "\n" << std::endl;
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}
// --------------------------------------------------------------------------
TEST(BeTree, SingleThreadedInsertSmallRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(120);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i : inserts) {
        tree.insert(i, i);
        //std::cout << tree << "\n" << std::endl;
    }
    for (uint64_t i = 0; i < 120; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}
// --------------------------------------------------------------------------
TEST(BeTree, SingleThreadedInsertLargeRandom) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50> tree(DIRNAME, 1.25);
    vector<uint64_t> inserts(5000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for (uint64_t i : inserts) {
        tree.insert(i, i);
        //std::cout << "inserted " << i << std::endl;
        //std::cout << tree << "\n" << std::endl;
    }
    for (uint64_t i = 0; i < 5000; i++) {
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
        std::cout << "found " << i << std::endl;
    }
}
