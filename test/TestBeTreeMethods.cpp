#include <gtest/gtest.h>
// --------------------------------------------------------------------------
#include "src/betree/BeTree.h"
#include "thirdparty/ThreadPool/ThreadPool.h"
#include <filesystem>
#include <new>
#include <random>
#include <ranges>
#include <unordered_set>
// --------------------------------------------------------------------------
using namespace std;
using namespace betree;
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
static const string DIRNAME = "/tmp/tester_test_be_tree_methods";
void setup() {
    std::filesystem::remove_all(DIRNAME.c_str());
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
namespace betree {
// --------------------------------------------------------------------------
TEST(BeTreeMethods, splitLeafNode) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    using BeTreeT = BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50>;
    BeTreeT tree(DIRNAME, 1.25);
    {
        BeTreeT::BeNodeWrapperT::BeLeafNodeT leafNode;
        uint64_t totalXOR = 0;
        for (uint64_t i = 0; i < leafNode.keys.size(); i++) {
            leafNode.keys[i] = i;
            leafNode.values[i] = i;
            leafNode.size++;
            totalXOR ^= i;
        }
        const std::size_t totalSize = leafNode.size;
        uint64_t middleKey;
        BeTreeT::PageT& rightPage = tree.splitLeafNode(leafNode, middleKey);
        BeTreeT::BeNodeWrapperT::BeLeafNodeT& rightLeafNode = tree.accessNode(rightPage).asLeaf();
        ASSERT_EQ(totalSize, leafNode.size + rightLeafNode.size);
        const std::size_t sizeDifference = leafNode.size > rightLeafNode.size
                                                   ? leafNode.size - rightLeafNode.size
                                                   : rightLeafNode.size - leafNode.size;
        ASSERT_LE(sizeDifference, 1);
        for (uint64_t i = 0; i < leafNode.size; i++) {
            ASSERT_EQ(leafNode.keys[i], leafNode.values[i]);
            ASSERT_LE(leafNode.keys[i], middleKey);
            totalXOR ^= leafNode.keys[i];
        }
        for (uint64_t i = 0; i < rightLeafNode.size; i++) {
            ASSERT_EQ(rightLeafNode.keys[i], rightLeafNode.values[i]);
            ASSERT_GT(rightLeafNode.keys[i], middleKey);
            totalXOR ^= rightLeafNode.keys[i];
        }
        ASSERT_EQ(totalXOR, 0);
        tree.pageBuffer.unpinPage(rightPage.id, true);
    }
    {
        BeTreeT::BeNodeWrapperT::BeLeafNodeT leafNode;
        uint64_t totalXOR = 0;
        for (uint64_t i = 0; i < leafNode.keys.size() / 2; i++) {
            leafNode.keys[i] = i;
            leafNode.values[i] = i;
            leafNode.size++;
            totalXOR ^= i;
        }
        const std::size_t totalSize = leafNode.size;
        uint64_t middleKey;
        BeTreeT::PageT& rightPage = tree.splitLeafNode(leafNode, middleKey);
        BeTreeT::BeNodeWrapperT::BeLeafNodeT& rightLeafNode = tree.accessNode(rightPage).asLeaf();
        ASSERT_EQ(totalSize, leafNode.size + rightLeafNode.size);
        const std::size_t sizeDifference = leafNode.size > rightLeafNode.size
                                                   ? leafNode.size - rightLeafNode.size
                                                   : rightLeafNode.size - leafNode.size;
        ASSERT_LE(sizeDifference, 1);
        for (uint64_t i = 0; i < leafNode.size; i++) {
            ASSERT_EQ(leafNode.keys[i], leafNode.values[i]);
            ASSERT_LE(leafNode.keys[i], middleKey);
            totalXOR ^= leafNode.keys[i];
        }
        for (uint64_t i = 0; i < rightLeafNode.size; i++) {
            ASSERT_EQ(rightLeafNode.keys[i], rightLeafNode.values[i]);
            ASSERT_GT(rightLeafNode.keys[i], middleKey);
            totalXOR ^= rightLeafNode.keys[i];
        }
        ASSERT_EQ(totalXOR, 0);
        tree.pageBuffer.unpinPage(rightPage.id, true);
    }
}
// --------------------------------------------------------------------------
TEST(BeTreeMethods, splitInnerNode) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    using BeTreeT = BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50>;
    BeTreeT tree(DIRNAME, 1.25);
    {
        BeTreeT::BeNodeWrapperT::BeInnerNodeT innerNode;
        uint64_t totalXOR = 0;
        for (uint64_t i = 0; i < innerNode.pivots.size(); i++) {
            innerNode.pivots[i] = i;
            innerNode.size++;
            totalXOR ^= i;
        }
        for (uint64_t i = 0; i < innerNode.pivots.size() + 1; i++) {
            innerNode.children[i] = i;
            totalXOR ^= i;
        }
        for (uint64_t i = 0; i < innerNode.upserts.upserts.size(); i++) {
            Upsert<uint64_t, uint64_t> upsert;
            upsert.key = i;
            totalXOR ^= i;
            innerNode.upserts.upserts[i] = std::move(upsert);
            innerNode.upserts.size++;
        }
        const std::size_t totalSize = innerNode.size;
        uint64_t middleKey;
        BeTreeT::PageT& rightPage = tree.splitInnerNode(innerNode, middleKey);
        BeTreeT::BeNodeWrapperT::BeInnerNodeT& rightInnerNode = tree.accessNode(rightPage).asInner();
        ASSERT_EQ(totalSize, innerNode.size + rightInnerNode.size + 1);
        const std::size_t sizeDifference = innerNode.size > rightInnerNode.size
                                                   ? innerNode.size - rightInnerNode.size
                                                   : rightInnerNode.size - innerNode.size;
        ASSERT_LE(sizeDifference, 1);
        for (uint64_t i = 0; i < innerNode.size; i++) {
            totalXOR ^= innerNode.pivots[i];
            ASSERT_LE(innerNode.pivots[i], middleKey);
        }
        for (uint64_t i = 0; i < innerNode.size + 1; i++) {
            totalXOR ^= innerNode.children[i];
        }
        for (uint64_t i = 0; i < innerNode.upserts.size; i++) {
            const auto& upsert = innerNode.upserts.upserts[i];
            totalXOR ^= upsert.key;
        }
        for (uint64_t i = 0; i < rightInnerNode.size; i++) {
            totalXOR ^= rightInnerNode.pivots[i];
            ASSERT_GT(rightInnerNode.pivots[i], middleKey);
        }
        for (uint64_t i = 0; i < rightInnerNode.size + 1; i++) {
            totalXOR ^= rightInnerNode.children[i];
        }
        for (uint64_t i = 0; i < rightInnerNode.upserts.size; i++) {
            const auto& upsert = rightInnerNode.upserts.upserts[i];
            totalXOR ^= upsert.key;
        }
        totalXOR ^= middleKey;
        ASSERT_EQ(totalXOR, 0);
        tree.pageBuffer.unpinPage(rightPage.id, true);
    }
}
// --------------------------------------------------------------------------
TEST(BeTreeMethods, insertPivots) {
    setup();
    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 100;
    using BeTreeT = BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50>;
    BeTreeT tree(DIRNAME, 1.25);
    {
        BeTreeT::BeNodeWrapperT::BeInnerNodeT innerNode;
        for (std::size_t i = 0; i < 5; i++) {
            innerNode.pivots[i] = 5 * (1 + i);
        }
        for (std::size_t i = 0; i <= 5; i++) {
            innerNode.children[i] = 2 + 5 * i;
        }
        innerNode.size = 5;
        // insert the new pivots
        // (index,pivot,child)
        using PivotTuple = std::tuple<std::size_t, uint64_t, std::uint64_t>;
        std::vector<PivotTuple> newPivots;
        newPivots.emplace_back(0, 3, 3);
        newPivots.emplace_back(1, 8, 8);
        newPivots.emplace_back(2, 13, 13);
        newPivots.emplace_back(5, 28, 28);
        tree.insertPivots(innerNode, newPivots);
        ASSERT_EQ(innerNode.size, 9);
        const std::vector<std::size_t> resultPivots = {3, 5, 8, 10, 13, 15, 20, 25, 28};
        for (std::size_t i = 0; i < 9; i++) {
            ASSERT_EQ(innerNode.pivots[i], resultPivots[i]);
        }
        const std::vector<std::size_t> resultChildren = {2, 3, 7, 8, 12, 13, 17, 22, 27, 28};
        for (std::size_t i = 0; i < 10; i++) {
            ASSERT_EQ(innerNode.children[i], resultChildren[i]);
        }
    }
}
// --------------------------------------------------------------------------
// TODO: adjust for different sizeof(Upsert)
/*
TEST(BeTreeMethods, removeMessages) {
    setup();
    constexpr size_t BLOCK_SIZE = 256;
    constexpr size_t PAGE_AMOUNT = 100;
    using BeTreeT = BeTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT, 50>;
    BeTreeT tree(DIRNAME, 1.25);
    {
        BeTreeT::BeNodeWrapperT::BeInnerNodeT innerNode;
        for (std::size_t i = 0; i < 5; i++) {
            Upsert<uint64_t, uint64_t> upsert;
            upsert.key = i * 2;
            innerNode.upserts.upserts[i] = std::move(upsert);
            innerNode.upserts.size++;
        }
        innerNode.pivots[0] = 4;
        innerNode.pivots[1] = 6;
        innerNode.children[0] = 2;
        innerNode.children[1] = 5;
        innerNode.children[2] = 7;
        innerNode.size = 2;
        std::vector<Upsert<uint64_t, uint64_t>> additionalUpserts;
        for (std::size_t i = 10; i <= 12; i++) {
            Upsert<uint64_t, uint64_t> upsert;
            upsert.key = i;
            upsert.timeStamp = 1;
            additionalUpserts.push_back(std::move(upsert));
        }

        BeTreeT::MessageMap messageMap = tree.removeMessages(
                innerNode, additionalUpserts,
                BeTreeT::BeNodeWrapperT::NodeSizesT::LEAF_N / 2);
        ASSERT_EQ(messageMap.size(), 1);
        ASSERT_EQ(messageMap[2].size(), 4);
        const vector<uint64_t> expectedUpserts = {8, 10, 11, 12};
        for (std::size_t i = 0; i < messageMap[1].size(); i++) {
            ASSERT_EQ(messageMap[2][i].key, expectedUpserts[i]);
        }
        ASSERT_EQ(innerNode.upserts.size, 4);
    }
}
 */
// --------------------------------------------------------------------------
}// namespace betree
 // --------------------------------------------------------------------------