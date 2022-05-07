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
TEST(BeTree, SingleThreadedInsertSmall) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 100;
    BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50> tree(DIRNAME, 1.25);
    for(uint64_t i = 0; i < 1000; i++){
        tree.insert(i, i);
    }
    for(uint64_t i = 0; i < 1000; i++){
        auto find = tree.find(i);
        ASSERT_TRUE(find);
        ASSERT_EQ(*find, i);
    }
}