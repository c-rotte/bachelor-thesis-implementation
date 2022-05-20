#ifndef B_EPSILON_BETREE_H
#define B_EPSILON_BETREE_H
// --------------------------------------------------------------------------
#include "BeNode.h"
#include "src/buffer/PageBuffer.h"
#include <algorithm>
#include <atomic>
#include <cinttypes>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <execution>
#include <fcntl.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <iterator>
#include <new>
#include <numeric>
#include <optional>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
template<class Iterator>
requires std::bidirectional_iterator<Iterator>
std::size_t squashUpserts(Iterator begin, Iterator end)
// squashes [begin; end), moves it to [begin and returns the new range size
{
    assert(end >= begin);
    auto accumulate = [](Iterator begin, Iterator end)
    // accumulate squashes message blocks (same key) into single upserts
    // according to the following rules:
    // II -> I
    // IU -> I
    // ID -> D
    // UI -> I
    // UU -> U
    // UD -> D
    // DI -> I
    // DU -> D
    // DD -> D
    {
        auto initialValue = *begin;
        initialValue.type = tree::UpsertType::INVALID;
        return std::reduce(
                std::execution::par_unseq, begin, end, std::move(initialValue),
                [](const auto& upsertA, const auto& upsertB) {
                    // invalid
                    if (upsertA.type == tree::UpsertType::INVALID) {
                        return upsertB;
                    }
                    if (upsertB.type == tree::UpsertType::INVALID) {
                        return upsertA;
                    }
                    if (upsertA.type == tree::UpsertType::INSERT) {
                        if (upsertB.type == tree::UpsertType::INSERT) {
                            // II -> I
                            auto result = upsertB;
                            return result;
                        }
                        if (upsertB.type == tree::UpsertType::UPDATE) {
                            // IU -> I
                            auto result = upsertA;
                            result.timeStamp = upsertB.timeStamp;
                            result.value += upsertB.value;
                            return result;
                        }
                        if (upsertB.type == tree::UpsertType::DELETE) {
                            // ID -> D
                            return upsertB;
                        }
                    }
                    if (upsertA.type == tree::UpsertType::UPDATE) {
                        if (upsertB.type == tree::UpsertType::INSERT) {
                            // UI -> I
                            auto result = upsertB;
                            return result;
                        }
                        if (upsertB.type == tree::UpsertType::UPDATE) {
                            // UU -> U
                            auto result = upsertB;
                            result.value = upsertA.value + upsertB.value;
                            return result;
                        }
                        if (upsertB.type == tree::UpsertType::DELETE) {
                            // UD -> D
                            return upsertB;
                        }
                    }
                    if (upsertA.type == tree::UpsertType::DELETE) {
                        if (upsertB.type == tree::UpsertType::INSERT) {
                            // DI -> I
                            auto result = upsertB;
                            return result;
                        }
                        if (upsertB.type == tree::UpsertType::UPDATE) {
                            // DU -> D
                            auto result = upsertA;
                            return result;
                        }
                        if (upsertB.type == tree::UpsertType::DELETE) {
                            // DD -> D
                            return upsertB;
                        }
                    }
                    // we should never reach this
                    assert(false);
                    return upsertA;
                });
    };
    // this is where we insert the accumulated messages
    auto currentSlot = begin;
    // start of our current messageBlock
    auto lastBegin = begin;
    for (auto it = begin; it != end; ++it) {
        Iterator currentBegin, currentEnd;
        if (lastBegin->key != it->key) {
            currentBegin = lastBegin;
            currentEnd = it;
            // set lastBegin
            lastBegin = it;
        } else {
            continue;
        }
        assert(currentBegin->key == (currentEnd - 1)->key);
        // found a block, accumulate the block
        *currentSlot = accumulate(currentBegin, currentEnd);
        ++currentSlot;
    }
    if (currentSlot != end) {
        assert(lastBegin->key == (end - 1)->key);
        *currentSlot = accumulate(lastBegin, end);
        ++currentSlot;
    }
    return currentSlot - begin;
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
namespace tree {
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
class BeTree {

    using BeNodeWrapperT = BeNodeWrapper<K, V, B, EPSILON>;
    using PageT = buffer::Page<B>;

    // a node must fit onto a page
    static_assert(sizeof(BeNodeWrapperT) == B);
    static_assert(alignof(BeNodeWrapperT) == alignof(PageT));

    struct alignas(alignof(std::max_align_t)) Header {
        std::uint64_t rootID = 0;
        std::atomic_uint32_t currentTimeStamp = 0;
    };

private:
    buffer::PageBuffer<B, N> pageBuffer;
    int fd;
    Header header;

public:
    BeTree(const std::string&, double);

private:
    void initializeNode(PageT&, bool) const;
    BeNodeWrapperT& accessNode(PageT&) const;

    // splits a leaf node by creating a new page and returning it as well
    // as a copy of the middle key (pivot)
    // note the returned page is still exclusively locked
    PageT& splitLeafNode(typename BeNodeWrapperT::BeLeafNodeT&, K&);
    FRIEND_TEST(BeTreeMethods, splitLeafNode);
    // splits a leaf node by creating a new page and returning it as well
    // as the removed middle key (pivot)
    // note the returned page is still exclusively locked
    PageT& splitInnerNode(typename BeNodeWrapperT::BeInnerNodeT&, K&);
    FRIEND_TEST(BeTreeMethods, splitInnerNode);
    // inserts pivot elements
    // tuples: (index of the split child, midKey, id of the right page)
    void insertPivots(typename BeNodeWrapperT::BeInnerNodeT&,
                      std::vector<std::tuple<std::size_t, K, std::uint64_t>>);
    FRIEND_TEST(BeTreeMethods, insertPivots);
    // sorts the messages and returns a vector of indexes (which point to the
    // removed message blocks returned in the reference)
    using MessageMap = std::unordered_map<std::size_t, std::vector<Upsert<K, V>>>;
    MessageMap removeMessages(typename BeNodeWrapperT::BeInnerNodeT&,
                              std::vector<Upsert<K, V>>, std::size_t);
    FRIEND_TEST(BeTreeMethods, removeMessages);
    // helper functions for upsert
    void handleTraversalNode(PageT*, MessageMap,
                             std::deque<std::pair<PageT*, MessageMap>>&);
    void flushRootNode(PageT*, Upsert<K, V>);
    void handleRootInnerUpsert(Upsert<K, V>, PageT*);
    void handleRootLeafUpsert(Upsert<K, V>, PageT*);
    // inserts a message
    void upsert(Upsert<K, V>);

public:
    void insert(K, V);
    void update(K, V);
    void erase(const K&);
    std::optional<V> find(const K&);

    void flush();

    template<class K_O, class V_O, std::size_t B_O, std::size_t N_O, short EPSILON_O>
    friend std::ostream& operator<<(std::ostream&, BeTree<K_O, V_O, B_O, N_O, EPSILON_O>&);
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
BeTree<K, V, B, N, EPSILON>::BeTree(const std::string& path, double growthFactor)
    : pageBuffer(path, growthFactor) {
    const std::string headerFile = path + "/tree";
    if (std::filesystem::exists(headerFile) && std::filesystem::is_regular_file(headerFile)) {
        fd = open(headerFile.c_str(), O_RDWR);
        if (pread(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
            throw std::runtime_error("Invalid tree header!");
        }
    } else {
        fd = open(headerFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            throw std::runtime_error("Could not create the tree header!");
        }
        header.rootID = pageBuffer.createPage();
        // initialize the root node (leaf)
        initializeNode(pageBuffer.pinPage(header.rootID, true, true), true);
        pageBuffer.unpinPage(header.rootID, true);
        if (ftruncate(fd, sizeof(Header)) < 0) {
            throw std::runtime_error("Could not increase the file size (tree).");
        }
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::initializeNode(PageT& page, bool leaf) const {
    new (page.data.data()) BeNodeWrapperT(leaf);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
typename BeTree<K, V, B, N, EPSILON>::BeNodeWrapperT&
BeTree<K, V, B, N, EPSILON>::accessNode(PageT& page) const {
    return *reinterpret_cast<BeNodeWrapperT*>(page.data.data());
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
typename BeTree<K, V, B, N, EPSILON>::PageT&
BeTree<K, V, B, N, EPSILON>::splitLeafNode(typename BeNodeWrapperT::BeLeafNodeT& leafNode,
                                           K& resultKey) {
    assert(leafNode.size >= 2);// maybe >= 1
    const std::size_t splitIndex = (leafNode.size - 1) / 2;
    resultKey = leafNode.keys[splitIndex];
    // create a new leaf node
    auto& rightPage = pageBuffer.pinPage(pageBuffer.createPage(), true, true);
    initializeNode(rightPage, true);
    assert(accessNode(rightPage).isLeaf());
    auto& rightLeaf = accessNode(rightPage).asLeaf();
    // copy the right pairs to the new node
    std::move(leafNode.keys.begin() + splitIndex + 1,
              leafNode.keys.begin() + leafNode.size,
              rightLeaf.keys.begin());
    std::move(leafNode.values.begin() + splitIndex + 1,
              leafNode.values.begin() + leafNode.size,
              rightLeaf.values.begin());
    // adjust the sizes
    rightLeaf.size = leafNode.size / 2;
    leafNode.size -= rightLeaf.size;
    return rightPage;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
typename BeTree<K, V, B, N, EPSILON>::PageT&
BeTree<K, V, B, N, EPSILON>::splitInnerNode(typename BeNodeWrapperT::BeInnerNodeT& innerNode,
                                            K& resultKey) {
    assert(innerNode.size >= 2);// maybe >= 1
    const std::size_t splitIndex = (innerNode.size - 1) / 2;
    resultKey = innerNode.pivots[splitIndex];
    // create a new inner node
    auto& rightPage = pageBuffer.pinPage(pageBuffer.createPage(), true, true);
    initializeNode(rightPage, false);
    assert(!accessNode(rightPage).isLeaf());
    auto& rightInnerNode = accessNode(rightPage).asInner();
    // copy the right pairs to the new node
    std::move(innerNode.pivots.begin() + splitIndex + 1,
              innerNode.pivots.begin() + innerNode.size,
              rightInnerNode.pivots.begin());
    std::move(innerNode.children.begin() + splitIndex + 1,
              innerNode.children.begin() + innerNode.size + 1,
              rightInnerNode.children.begin());
    // adjust the sizes
    rightInnerNode.size = innerNode.size / 2;
    innerNode.size = innerNode.size - rightInnerNode.size - 1;
    // move the upserts to the right (<it> points to the first removed element)
    auto it = std::remove_if(innerNode.upserts.upserts.begin(),
                             innerNode.upserts.upserts.begin() + innerNode.upserts.size,
                             [&resultKey](const Upsert<K, V>& upsert) {
                                 return upsert.key > resultKey;
                             });
    std::move(it, innerNode.upserts.upserts.begin() + innerNode.upserts.size,
              rightInnerNode.upserts.upserts.begin());
    // adjust the buffer sizes
    rightInnerNode.upserts.size = innerNode.upserts.size -
                                  (it - innerNode.upserts.upserts.begin());
    innerNode.upserts.size -= rightInnerNode.upserts.size;
    return rightPage;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::insertPivots(typename BeNodeWrapperT::BeInnerNodeT& innerNode,
                                               std::vector<std::tuple<std::size_t, K, std::uint64_t>> newPivots) {
    using PivotTuple = std::tuple<std::size_t, K, std::uint64_t>;
    // must have free slots
    assert(innerNode.size + newPivots.size() <= innerNode.pivots.size());
    // the elements must be sorted and unique
    assert(std::is_sorted(newPivots.begin(), newPivots.end(),
                          [](const PivotTuple& a, const PivotTuple& b) {
                              assert(std::get<0>(a) != std::get<0>(b));
                              return std::get<0>(a) < std::get<0>(b);
                          }));
    assert(std::is_sorted(newPivots.begin(), newPivots.end(),
                          [](const PivotTuple& a, const PivotTuple& b) {
                              assert(std::get<1>(a) != std::get<1>(b));
                              return std::get<1>(a) < std::get<1>(b);
                          }));
    // iterate backwards to keep the moves at a minimum
    std::size_t lastFirst = innerNode.size;// keep track of the last move
    for (std::size_t it = newPivots.size(); it > 0; it--) {
        PivotTuple& pivotTuple = newPivots[it - 1];
        std::size_t index = std::get<0>(pivotTuple);
        K key = std::get<1>(pivotTuple);
        std::uint64_t child = std::get<2>(pivotTuple);
        // must be a valid index
        assert(index + it - 1 < innerNode.pivots.size());
        assert(lastFirst + it <= innerNode.pivots.size());
        // move everything to the right
        /*
        std::cout << "it=" << it << " index=" << index << " lastFirst=" << lastFirst << std::endl;
        for (std::size_t i = 0; i < innerNode.size + newPivots.size(); i++) {
            std::cout << innerNode.pivots[i] << " ";
        }
        std::cout << std::endl;
        */
        std::move_backward(innerNode.pivots.begin() + index,
                           innerNode.pivots.begin() + lastFirst,
                           innerNode.pivots.begin() + lastFirst + it);
        std::move_backward(innerNode.children.begin() + index + 1,
                           innerNode.children.begin() + lastFirst + 1,
                           innerNode.children.begin() + lastFirst + it + 1);
        // insert the pivot and its child
        innerNode.pivots[index + it - 1] = key;
        innerNode.children[index + it] = child;
        /*
        for (std::size_t i = 0; i < innerNode.size + newPivots.size(); i++) {
            std::cout << innerNode.pivots[i] << " ";
        }
        std::cout << std::endl;
        */
        lastFirst = index;
    }
    // adjust the size
    innerNode.size += newPivots.size();
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
typename BeTree<K, V, B, N, EPSILON>::MessageMap
BeTree<K, V, B, N, EPSILON>::removeMessages(
        typename BeNodeWrapperT::BeInnerNodeT& innerNode,
        std::vector<Upsert<K, V>> additionalUpserts, std::size_t maxRemoved) {
    // we need to free |all messages| - |buffer size| slots
    const std::size_t neededUpsertSlots = innerNode.upserts.size +
                                          additionalUpserts.size() -
                                          innerNode.upserts.upserts.size();
    assert(maxRemoved >= neededUpsertSlots);
    assert(std::is_sorted(innerNode.upserts.upserts.begin(),
                          innerNode.upserts.upserts.begin() + innerNode.upserts.size));
    assert(std::is_sorted(additionalUpserts.begin(), additionalUpserts.end()));
    // merge the node and the additionalUpserts
    std::vector<Upsert<K, V>> allMessages;
    allMessages.reserve(innerNode.upserts.size + additionalUpserts.size());
    std::merge(innerNode.upserts.upserts.begin(),
               innerNode.upserts.upserts.begin() + innerNode.upserts.size,
               additionalUpserts.begin(),
               additionalUpserts.end(),
               std::back_inserter(allMessages));
    // squash the messages
    allMessages.resize(squashUpserts(allMessages.begin(), allMessages.end()));
    // now scan all messages for blocks
    // first index, last index (exclusive), child index
    using BlockTuple = std::tuple<std::size_t, std::size_t, std::size_t>;
    std::vector<BlockTuple> messageBlocks;
    std::size_t lastStartingIndex = 0;
    K lastKey = allMessages.front().key;
    std::size_t lastChildIndex =
            std::lower_bound(innerNode.pivots.begin(),
                             innerNode.pivots.begin() + innerNode.size,
                             lastKey) -
            innerNode.pivots.begin();
    for (std::size_t i = 1; i < allMessages.size(); i++) {
        const K& currentKey = allMessages[i].key;
        if (currentKey != lastKey) {
            // check if the child is still the same
            const std::size_t currentChildIndex =
                    std::lower_bound(innerNode.pivots.begin(),
                                     innerNode.pivots.begin() + innerNode.size,
                                     currentKey) -
                    innerNode.pivots.begin();
            if (currentChildIndex != lastChildIndex) {
                // found a new message block -> add the last one
                messageBlocks.emplace_back(lastStartingIndex, i, lastChildIndex);
                lastStartingIndex = i;
                lastKey = currentKey;
                lastChildIndex = currentChildIndex;
            }
        }
    }
    // add the last block
    messageBlocks.emplace_back(lastStartingIndex, allMessages.size(), lastChildIndex);
    // create a vector of references (indexes) to the tuples
    std::vector<std::size_t> blockReferences(messageBlocks.size());
    std::iota(blockReferences.begin(), blockReferences.end(), 0);
    // sort the references according to their size (starting with the largest)
    // first index, last index (exclusive), child index
    std::sort(blockReferences.begin(), blockReferences.end(),
              [&messageBlocks](const std::size_t& indexA, const std::size_t& indexB) {
                  const BlockTuple& a = messageBlocks[indexA];
                  const BlockTuple& b = messageBlocks[indexB];
                  const std::size_t sizeA = std::get<1>(a) - std::get<0>(a);
                  const std::size_t sizeB = std::get<1>(b) - std::get<0>(b);
                  return sizeA >= sizeB;
              });
    // now loop through the tuples until we have enough slots
    MessageMap result;
    std::size_t collectedSlots = 0;
    for (auto it = blockReferences.begin();
         collectedSlots < neededUpsertSlots &&
         it != blockReferences.end();
         ++it) {
        auto& messageBlock = messageBlocks[*it];
        // move the messages to the result map
        auto& resultVec = result[std::get<2>(messageBlock)];
        assert(resultVec.empty());
        const std::size_t blockSize = std::get<1>(messageBlock) -
                                      std::get<0>(messageBlock);
        // copy not more than maxRemoved
        std::size_t toRemove = blockSize;
        if (collectedSlots + blockSize > maxRemoved) {
            toRemove = maxRemoved - collectedSlots;
        }
        resultVec.reserve(toRemove);
        std::move(allMessages.begin() + std::get<0>(messageBlock),
                  allMessages.begin() + std::get<0>(messageBlock) + toRemove,
                  std::back_inserter(resultVec));
        collectedSlots += toRemove;
        assert(std::is_sorted(resultVec.begin(), resultVec.end()));
        if (toRemove == blockSize) {
            // invalidate the reference
            *it = blockReferences.size();
        } else {
            // adjust the reference
            // first index, last index (exclusive), child index
            std::get<0>(messageBlock) += toRemove;
        }
    }
    // sort the references again, but this time accordingly to their natural order
    std::sort(blockReferences.begin(), blockReferences.end());
    // move the rest back to the node
    innerNode.upserts.size = 0;
    for (std::size_t index: blockReferences) {
        if (index == blockReferences.size()) {
            // this block was removed -> continue
            continue;
        }
        auto& messageBlock = messageBlocks[index];
        const std::size_t blockSize = std::get<1>(messageBlock) -
                                      std::get<0>(messageBlock);
        std::move(allMessages.begin() + std::get<0>(messageBlock),
                  allMessages.begin() + std::get<1>(messageBlock),
                  innerNode.upserts.upserts.begin() + innerNode.upserts.size);
        innerNode.upserts.size += blockSize;
    }
    assert(std::is_sorted(innerNode.upserts.upserts.begin(),
                          innerNode.upserts.upserts.begin() + innerNode.upserts.size));
    return result;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::handleTraversalNode(PageT* currentPage,
                                                      MessageMap messageMap,
                                                      std::deque<std::pair<PageT*, MessageMap>>& queue) {
    // queue: <node, messages>
    // the additional messages are already removed
    auto& currentNode = accessNode(*currentPage).asInner();
    // pin each child and split it if necessary
    using PivotTuple = std::tuple<std::size_t, K, std::uint64_t>;
    std::vector<PivotTuple> newPivots;
    using SplitResult = std::pair<K, PageT*>;
    std::vector<std::tuple<PageT*, std::optional<SplitResult>, std::vector<Upsert<K, V>>>> leafMessages;
    for (auto& [childIndex, vector]: messageMap) {
        /*
        std::cout << currentPage->id << ": currently at childIndex " << childIndex << std::endl;
        std::cout << "children: ";
        for(int i = 0; i < currentNode.size + 1; i++){
            std::cout << " " << currentNode.children[i];
        }
        std::cout << std::endl;
        */
        assert(childIndex <= currentNode.size);
        assert(std::is_sorted(vector.begin(), vector.end()));
        // squash the message vector
        vector.resize(squashUpserts(vector.begin(), vector.end()));
        // pin the child
        PageT& childPage = pageBuffer.pinPage(
                currentNode.children[childIndex], true);
        if (accessNode(childPage).isLeaf()) {
            // base case: we arrived at the leaf level
            auto& leafChild = accessNode(childPage).asLeaf();
            // count the future node size
            bool needToSplit = false;
            long long futureSize = leafChild.size;
            for (const auto& upsert: vector) {
                auto it = std::lower_bound(leafChild.keys.begin(),
                                           leafChild.keys.begin() + leafChild.size, upsert.key);
                bool foundKey = static_cast<std::size_t>(leafChild.keys.begin() - it) <
                                        leafChild.size &&
                                * it == upsert.key;
                futureSize += (upsert.type == UpsertType::INSERT && !foundKey);
                futureSize -= (upsert.type == UpsertType::DELETE && foundKey);
                futureSize = std::max(futureSize, 0LL);
                if (static_cast<std::size_t>(futureSize) > leafChild.keys.size()) {
                    needToSplit = true;
                    break;
                }
            }
            std::optional<SplitResult> rightSplitResult;
            //std::cout << "inserts=" << (futureSize - leafChild.size) << std::endl;
            //std::cout << "futureSize=" << futureSize << " leafChild.keys.size()=" << leafChild.keys.size() << std::endl;
            if (needToSplit) {
                // we need to split the leaf
                K middleKey;
                // <rightPage> is automatically uniquely pinned
                PageT& rightPage = splitLeafNode(leafChild, middleKey);
                rightSplitResult = std::make_pair(middleKey, &rightPage);
                // add the pivot
                newPivots.emplace_back(childIndex, middleKey, rightPage.id);
            }
            // collect the child (so that we can free the parent and then handle the messages)
            leafMessages.emplace_back(&childPage, std::move(rightSplitResult), std::move(vector));
        } else {
            // the children are inner nodes
            auto& innerChild = accessNode(childPage).asInner();
            if (innerChild.upserts.upserts.size() - innerChild.upserts.size >= vector.size()) {
                // squash the child
                innerChild.upserts.size = squashUpserts(innerChild.upserts.upserts.begin(),
                                                        innerChild.upserts.upserts.begin() +
                                                                innerChild.upserts.size);
            }
            //std::cout << "buffering... " << (innerChild.upserts.upserts.size() - innerChild.upserts.size) << " vs " << vector.size() << std::endl;
            // check again if there are enough free slots
            if (innerChild.upserts.upserts.size() - innerChild.upserts.size >= vector.size()) {
                // the child has enough space for its addressed messages
                // -> store them and continue
                std::vector<Upsert<K, V>> nodeUpserts;
                // we need to copy the upserts since std::merge does not like overlapping ranges
                nodeUpserts.reserve(innerChild.upserts.size);
                std::move(innerChild.upserts.upserts.begin(),
                          innerChild.upserts.upserts.begin() + innerChild.upserts.size,
                          std::back_inserter(nodeUpserts));
                std::merge(nodeUpserts.begin(), nodeUpserts.end(),
                           vector.begin(), vector.end(),
                           innerChild.upserts.upserts.begin());
                innerChild.upserts.size += vector.size();
                // free the page
                //std::cout << "3" << std::endl;
                pageBuffer.unpinPage(childPage.id, true);
                continue;
            }
            // remove its messages
            auto messageMap = std::move(removeMessages(
                    innerChild, std::move(vector),
                    BeNodeWrapperT::NodeSizesT::LEAF_N / 2));
            if (innerChild.pivots.size() - innerChild.size < messageMap.size()) {
                // we need to split the child
                K middleKey;
                PageT& rightPage = splitInnerNode(innerChild, middleKey);
                {
                    //auto& rightNode = accessNode(rightPage).asInner();
                    /*
                    std::cout << "left children: ";
                    for (int i = 0; i < innerChild.size + 1; i++) {
                        std::cout << " " << innerChild.children[i];
                    }
                    std::cout << "right children: ";
                    for (int i = 0; i < rightNode.size + 1; i++) {
                        std::cout << " " << rightNode.children[i];
                    }
                     */
                }
                newPivots.emplace_back(childIndex, middleKey, rightPage.id);
                // split the additional messages
                MessageMap leftMap, rightMap;
                for (auto& [childIndex, vector]: messageMap) {
                    if (childIndex <= innerChild.size) {
                        assert(!leftMap.count(childIndex));
                        leftMap[childIndex] = std::move(vector);
                    } else {
                        const std::size_t adjustedChildIndex = childIndex - innerChild.size - 1;
                        assert(!rightMap.count(adjustedChildIndex));
                        rightMap[adjustedChildIndex] = std::move(vector);
                    }
                }
                assert(!leftMap.empty() || !rightMap.empty());
                if (leftMap.empty()) {
                    pageBuffer.unpinPage(childPage.id, true);
                } else {
                    queue.emplace_back(&childPage, std::move(leftMap));
                }
                if (rightMap.empty()) {
                    pageBuffer.unpinPage(rightPage.id, true);
                } else {
                    queue.emplace_back(&rightPage, std::move(rightMap));
                }
            } else {
                queue.emplace_back(&childPage, std::move(messageMap));
            }
        }
    }
    // insert the pivots into the parent
    std::sort(newPivots.begin(), newPivots.end(),
              [](const PivotTuple& a, const PivotTuple& b) {
                  return std::get<0>(a) < std::get<0>(b);
              });
    insertPivots(currentNode, std::move(newPivots));
    // unlock the parent
    //std::cout << "5" << std::endl;
    pageBuffer.unpinPage(currentPage->id, true);
    // now handle the leaf messages (if there are any)
    for (auto& [childPage, splitResult, vector]: leafMessages) {
        // insert the messages
        for (auto& upsert: vector) {
            // determine the target node
            PageT* targetPage = childPage;
            if (splitResult && upsert.key > splitResult->first) {
                targetPage = splitResult->second;
            }
            auto& targetNode = accessNode(*targetPage).asLeaf();
            //std::cout << "targetNode.size=" << targetNode.size << std::endl;
            // search for the key index
            auto keyIt = std::lower_bound(targetNode.keys.begin(),
                                          targetNode.keys.begin() + targetNode.size,
                                          upsert.key);
            const std::size_t keyIndex = keyIt - targetNode.keys.begin();
            if (upsert.type == UpsertType::DELETE) {
                if (keyIndex >= targetNode.size || *keyIt != upsert.key) {
                    // the key does not exist -> continue
                    continue;
                }
                // delete the entry (shift [index; end) one to the left)
                std::move(targetNode.keys.begin() + keyIndex + 1,
                          targetNode.keys.begin() + targetNode.size,
                          targetNode.keys.begin() + keyIndex);
                std::move(targetNode.values.begin() + keyIndex + 1,
                          targetNode.values.begin() + targetNode.size,
                          targetNode.values.begin() + keyIndex);
                // adjust the size
                targetNode.size--;
                continue;
            }
            if (upsert.type == UpsertType::UPDATE) {
                if (keyIndex >= targetNode.size || *keyIt != upsert.key) {
                    // the key does not exist -> continue
                    continue;
                }
                // update the key
                targetNode.values[keyIndex] += upsert.value;
                continue;
            }
            if (upsert.type == UpsertType::INSERT) {
                if (keyIndex < targetNode.size && * keyIt == upsert.key) {
                    // the key does already exist -> overwrite
                    targetNode.values[keyIndex] = std::move(upsert.value);
                    continue;
                }
                assert(targetNode.size < targetNode.keys.size());
                assert(keyIndex < targetNode.keys.size() && *keyIt != upsert.key);
                // insert the key,value
                std::move_backward(targetNode.keys.begin() + keyIndex,
                                   targetNode.keys.begin() + targetNode.size,
                                   targetNode.keys.begin() + targetNode.size + 1);
                std::move_backward(targetNode.values.begin() + keyIndex,
                                   targetNode.values.begin() + targetNode.size,
                                   targetNode.values.begin() + targetNode.size + 1);
                //std::cout << "inserting " << upsert.key << " at index " << keyIndex << std::endl;
                targetNode.keys[keyIndex] = upsert.key;
                targetNode.values[keyIndex] = std::move(upsert.value);
                targetNode.size++;
                continue;
            }
        }
        // free both the left and right page
        //std::cout << "1" << std::endl;
        pageBuffer.unpinPage(childPage->id, true);
        if (splitResult) {
            //std::cout << "2" << std::endl;
            pageBuffer.unpinPage(splitResult->second->id, true);
        }
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::flushRootNode(PageT* rootPage, Upsert<K, V> message) {
    assert(!accessNode(*rootPage).isLeaf());
    // queue for the level order traversal
    // note: the queue contains <page, additionalMessages> pairs where each
    // node (page) has
    // a) has enough free pivot slots
    // b) has a buffer with messages removed to <additionalMessages>
    // c) is currently locked
    // d) a parent which is unlocked
    std::deque<std::pair<PageT*, MessageMap>> queue;// <node, messages>
    // handle root
    {
        auto& rootNode = accessNode(*rootPage).asInner();
        auto messageMap = std::move(removeMessages(
                rootNode, {std::move(message)},
                BeNodeWrapperT::NodeSizesT::LEAF_N / 2));
        // check if the root needs slots for the potential splits of the children
        if (rootNode.pivots.size() - rootNode.size < messageMap.size()) {
            //std::cout << "rootNode.size: " << rootNode.size << std::endl;
            // we need to create a new root
            K midKey;
            PageT& rightPage = splitInnerNode(rootNode, midKey);
            PageT& newRootPage = pageBuffer.pinPage(pageBuffer.createPage(), true, true);
            initializeNode(newRootPage, false);
            auto& newRootNode = accessNode(newRootPage).asInner();
            // add the pivot and the children
            newRootNode.pivots[0] = midKey;
            newRootNode.children[0] = newRootPage.id;
            newRootNode.children[1] = rightPage.id;
            newRootNode.size = 1;
            // swap the old with the new root
            std::swap(rootPage->data, newRootPage.data);
            // free the parent
            pageBuffer.unpinPage(rootPage->id, true);
            // split the additional messages
            MessageMap leftMap;
            MessageMap rightMap;
            for (auto& [childIndex, vector]: messageMap) {
                if (childIndex <= newRootNode.size) {
                    assert(!leftMap.count(childIndex));
                    //std::cout << "childIndex: " << childIndex << std::endl;
                    leftMap[childIndex] = std::move(vector);
                } else {
                    //std::cout << "node.size: " << node.size << std::endl;
                    //std::cout << "left.size: " << newRootNode.size << std::endl;
                    const std::size_t adjustedChildIndex = childIndex - newRootNode.size - 1;
                    //std::cout << "adjustedChildIndex: " << adjustedChildIndex << std::endl;
                    assert(!rightMap.count(adjustedChildIndex));
                    rightMap[adjustedChildIndex] = std::move(vector);
                }
            }
            queue.emplace_back(&newRootPage, std::move(leftMap));
            queue.emplace_back(&rightPage, std::move(rightMap));
        } else {
            // the root has enough space, pass it to the queue
            queue.emplace_back(rootPage, std::move(messageMap));
        }
    }
    // main action: level order traversal
    while (!queue.empty()) {
        /*
        std::cout << "pages: ";
        for (const auto& pair: queue) {
            std::cout << " " << pair.first->id;
        }
        std::cout << std::endl;
        */
        auto [currentPage, messageMap] = std::move(queue.front());
        queue.pop_front();
        //std::cout << std::endl;
        assert(!accessNode(*currentPage).isLeaf());
        handleTraversalNode(currentPage, std::move(messageMap), queue);
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::handleRootInnerUpsert(Upsert<K, V> upsert, PageT* rootPage) {
    // must be called with the root page
    assert(rootPage != nullptr);
    assert(rootPage->id == header.rootID);
    // must be an inner node
    assert(!accessNode(*rootPage).isLeaf());
    auto& innerNode = accessNode(*rootPage).asInner();
    if (innerNode.upserts.size < innerNode.upserts.upserts.size()) {
        // squash the buffer
        innerNode.upserts.size = squashUpserts(innerNode.upserts.upserts.begin(),
                                               innerNode.upserts.upserts.begin() +
                                                       innerNode.upserts.size);
    }
    // check again if there are enough free slots
    if (innerNode.upserts.size < innerNode.upserts.upserts.size()) {
        // there is enough space in the root, insert sorted
        auto it = std::lower_bound(innerNode.upserts.upserts.begin(),
                                   innerNode.upserts.upserts.begin() + innerNode.upserts.size,
                                   upsert);
        const std::size_t index = it - innerNode.upserts.upserts.begin();
        // shift to the right
        std::move_backward(it,
                           innerNode.upserts.upserts.begin() + innerNode.upserts.size,
                           innerNode.upserts.upserts.begin() + innerNode.upserts.size + 1);
        innerNode.upserts.upserts[index] = std::move(upsert);
        innerNode.upserts.size++;
        pageBuffer.unpinPage(rootPage->id, true);
        return;
    }
    // buffer is full, we need to flush
    flushRootNode(rootPage, std::move(upsert));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::handleRootLeafUpsert(Upsert<K, V> upsert, PageT* rootPage) {
    // must be called with the root page
    assert(rootPage != nullptr);
    assert(rootPage->id == header.rootID);
    // must be a leaf
    assert(accessNode(*rootPage).isLeaf());
    auto& leafNode = accessNode(*rootPage).asLeaf();
    // search for the key index
    auto keyIt = std::lower_bound(leafNode.keys.begin(),
                                  leafNode.keys.begin() + leafNode.size,
                                  upsert.key);
    std::size_t keyIndex = keyIt - leafNode.keys.begin();
    if (upsert.type == UpsertType::DELETE) {
        // the key must exist
        assert(keyIt != leafNode.keys.end());
        assert(*keyIt == upsert.key);
        // delete the key (shift [index; end) one to the left)
        std::move(leafNode.keys.begin() + keyIndex + 1,
                  leafNode.keys.begin() + leafNode.size,
                  leafNode.keys.begin() + keyIndex);
        // adjust the size
        leafNode.size--;
        // unpin the leaf node
        pageBuffer.unpinPage(rootPage->id, true);
        return;
    }
    if (upsert.type == UpsertType::UPDATE) {
        // the key must exist
        assert(keyIt != leafNode.keys.end());
        assert(*keyIt == upsert.key);
        // update the key
        leafNode.values[keyIndex] += upsert.value;
        pageBuffer.unpinPage(rootPage->id, true);
        return;
    }
    if (upsert.type == UpsertType::INSERT) {
        if (keyIndex < leafNode.size) {
            // the key must not exist
            assert(*keyIt != upsert.key);
        }
        auto* targetPage = rootPage;// page which will receive the insert
        if (leafNode.size == leafNode.keys.size()) {
            // full leaf -> split it
            K middleKey;
            // <rightPage> is automatically uniquely pinned
            PageT& rightPage = splitLeafNode(leafNode, middleKey);
            // create a new root
            PageT& newRoot = pageBuffer.pinPage(pageBuffer.createPage(), true, true);
            {
                // first initialize the new inner node
                initializeNode(newRoot, false);
                auto& newRootNode = accessNode(newRoot).asInner();
                newRootNode.pivots[0] = middleKey;
                newRootNode.children[0] = newRoot.id;
                newRootNode.children[1] = rightPage.id;
                newRootNode.size = 1;
                // now swap with the root (this invalidates all references to the old root)
                std::swap(newRoot.data, rootPage->data);
                // free the parent
                pageBuffer.unpinPage(rootPage->id, true);
            }
            // check which child will receive the insert
            if (upsert.key <= middleKey) {
                pageBuffer.unpinPage(rightPage.id, true);
                targetPage = &newRoot;
            } else {
                // adjust the key index
                auto& newRootNode = accessNode(newRoot).asLeaf();
                keyIndex -= newRootNode.size;
                pageBuffer.unpinPage(newRoot.id, true);
                targetPage = &rightPage;
            }
        }
        // from here on only use targetPage
        assert(targetPage != nullptr);
        assert(accessNode(*targetPage).isLeaf());
        auto& targetLeafNode = accessNode(*targetPage).asLeaf();
        //std::cout << targetLeafNode.size << std::endl;
        // make room for the key (shift [index; end) one to the right)
        std::move(targetLeafNode.keys.begin() + keyIndex,
                  targetLeafNode.keys.begin() + targetLeafNode.size,
                  targetLeafNode.keys.begin() + keyIndex + 1);
        std::move(targetLeafNode.values.begin() + keyIndex,
                  targetLeafNode.values.begin() + targetLeafNode.size,
                  targetLeafNode.values.begin() + keyIndex + 1);
        // insert the pair
        targetLeafNode.keys[keyIndex] = upsert.key;
        targetLeafNode.values[keyIndex] = std::move(upsert.value);
        // adjust the size
        targetLeafNode.size++;
        // unpin the page
        pageBuffer.unpinPage(targetPage->id, true);
        return;
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::upsert(Upsert<K, V> upsert) {
    PageT* rootPage = &pageBuffer.pinPage(header.rootID, true);
    // first case: the root node is a leaf node (direct insert)
    if (accessNode(*rootPage).isLeaf()) {
        //std::cout << "leaf " << upsert.key << std::endl;
        handleRootLeafUpsert(std::move(upsert), rootPage);
        return;
    }
    //std::cout << "inner " << upsert.key << std::endl;
    // second case: the root node is an inner node
    handleRootInnerUpsert(std::move(upsert), rootPage);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::insert(K key, V value) {
    Upsert<K, V> message{
            std::move(key),
            std::move(value),
            ++header.currentTimeStamp,
            UpsertType::INSERT};
    upsert(std::move(message));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::update(K key, V value) {
    Upsert<K, V> message{
            std::move(key),
            std::move(value),
            ++header.currentTimeStamp,
            UpsertType::UPDATE};
    upsert(std::move(message));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::erase(const K& key) {
    Upsert<K, V> message{
            key,
            V(),
            ++header.currentTimeStamp,
            UpsertType::DELETE};
    upsert(std::move(message));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
std::optional<V> BeTree<K, V, B, N, EPSILON>::find(const K& key) {
    std::deque<V> accumulatedUpdates;
    std::optional<V> currentValue;
    PageT* currentPage = &pageBuffer.pinPage(header.rootID, false);
    while (!accessNode(*currentPage).isLeaf()) {
        auto& innerNode = accessNode(*currentPage).asInner();
        // find the first element of the message block
        auto firstIt = std::lower_bound(innerNode.upserts.upserts.begin(),
                                        innerNode.upserts.upserts.begin() + innerNode.upserts.size,
                                        key);
        bool deleted = false;
        std::vector<V> localUpdates;
        for (; firstIt != innerNode.upserts.upserts.begin() + innerNode.upserts.size &&
               firstIt->key == key;
             ++firstIt) {
            const Upsert<K, V>& upsert = *firstIt;
            if (upsert.type == UpsertType::DELETE) {
                localUpdates.clear();
                currentValue = std::nullopt;
                deleted = true;
                continue;
            }
            if (upsert.type == UpsertType::UPDATE) {
                localUpdates.push_back(upsert.value);
                continue;
            }
            if (upsert.type == UpsertType::INSERT) {
                localUpdates.clear();
                currentValue = upsert.value;
                deleted = false;
                continue;
            }
        }
        if (deleted) {
            accumulatedUpdates.clear();
        }
        std::move(localUpdates.begin(), localUpdates.end(), std::front_inserter(accumulatedUpdates));
        if (deleted || currentValue) {
            // new insert or new delete -> break
            pageBuffer.unpinPage(currentPage->id, false);
            currentPage = nullptr;
            break;
        }
        auto childIt = std::lower_bound(innerNode.pivots.begin(),
                                        innerNode.pivots.begin() + innerNode.size,
                                        key);
        std::uint64_t childId = innerNode.children[childIt - innerNode.pivots.begin()];
        PageT* nextPage = &pageBuffer.pinPage(childId, false);
        pageBuffer.unpinPage(currentPage->id, false);
        currentPage = nextPage;
    }
    /*
    std::cout << key << " accumulated updates:";
    for (const auto& v: accumulatedUpdates) {
        std::cout << " " << v;
    }
    std::cout << std::endl;
    */
    if (currentPage) {
        // leaf
        if (!currentValue) {
            // we still need a base
            auto& leafNode = accessNode(*currentPage).asLeaf();
            auto childIt = std::lower_bound(leafNode.keys.begin(),
                                            leafNode.keys.begin() + leafNode.size,
                                            key);
            const std::size_t index = childIt - leafNode.keys.begin();
            if (index < leafNode.size && * childIt == key) {
                currentValue = leafNode.values[childIt - leafNode.keys.begin()];
            }
        }
        pageBuffer.unpinPage(currentPage->id, false);
    }
    // inserted or deleted (deleted -> accumulatedUpdates is empty)
    for (const auto& updateValue: accumulatedUpdates) {
        assert(currentValue);
        (*currentValue) += updateValue;
    }
    return currentValue;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::flush() {
    if (pwrite(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
        throw std::runtime_error("Could not save the header (tree).");
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
std::ostream& operator<<(std::ostream& out, BeTree<K, V, B, N, EPSILON>& tree) {
    // level order traversal
    std::queue<std::uint64_t> queue;
    queue.push(tree.header.rootID);
    std::cout << "digraph{\n";
    while (!queue.empty()) {
        std::uint64_t currentID = queue.front();
        queue.pop();
        auto& page = tree.pageBuffer.pinPage(currentID, false);
        std::cout << page.id << "[label=\"";
        if (tree.accessNode(page).isLeaf()) {
            auto& leafNode = tree.accessNode(page).asLeaf();
            for (std::size_t i = 0; i < leafNode.size; i++) {
                std::cout << "(" << leafNode.keys[i] << " => " << leafNode.values[i] << ")";
                if (i != leafNode.size - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "\"];\n";
        } else {
            auto& innerNode = tree.accessNode(page).asInner();
            std::cout << "pivots=(";
            for (std::size_t i = 0; i < innerNode.size; i++) {
                std::cout << innerNode.pivots[i];
                if (i != innerNode.size - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << ") buffer=(";
            for (std::size_t i = 0; i < innerNode.upserts.size; i++) {
                std::cout << "(";
                if (innerNode.upserts.upserts[i].type == 0) {
                    std::cout << "INSERT";
                } else if (innerNode.upserts.upserts[i].type == 1) {
                    std::cout << "UPDATE";
                } else if (innerNode.upserts.upserts[i].type == 2) {
                    std::cout << "DELETE";
                } else {
                    std::cout << "UNKNOWN";
                }
                std::cout << ": " << innerNode.upserts.upserts[i].key << ", ";
                std::cout << innerNode.upserts.upserts[i].value << ")";
                if (i != innerNode.upserts.size - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << ")";
            std::cout << "\"];\n";
            for (std::size_t i = 0; i < innerNode.size + 1; i++) {
                std::uint64_t childID = innerNode.children[i];
                std::cout << currentID << " -> " << childID << ";\n";
                queue.push(childID);
            }
        }
        tree.pageBuffer.unpinPage(currentID, false);
    }
    std::cout << "}";
    return out;
}
// --------------------------------------------------------------------------
}// namespace tree
// --------------------------------------------------------------------------
#endif//B_EPSILON_BETREE_H
