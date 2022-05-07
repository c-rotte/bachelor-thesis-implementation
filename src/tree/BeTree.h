#ifndef B_EPSILON_BETREE_H
#define B_EPSILON_BETREE_H
// --------------------------------------------------------------------------
#include "BeNode.h"
#include "src/buffer/PageBuffer.h"
#include <algorithm>
#include <atomic>
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <new>
#include <optional>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
template<class T>
std::vector<T> splitVector(std::vector<T>& vec, std::size_t index)
// splits vec at index (vec contains [0, index); result contains [index; vec.end))
{
    assert(index < vec.size());
    std::vector<T> result;
    std::move(vec.begin() + index, vec.end(), std::back_inserter(result));
    vec.resize(index);
    return result;
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
    // splits a leaf node by creating a new page and returning it as well
    // as the removed middle key (pivot)
    // note the returned page is still exclusively locked
    PageT& splitInnerNode(typename BeNodeWrapperT::BeInnerNodeT&, K&);
    // inserts pivot elements
    void insertPivots(typename BeNodeWrapperT::BeInnerNodeT&,
                      std::vector<std::tuple<std::size_t, K, std::uint64_t>>);
    // inserts values
    void insertValues(typename BeNodeWrapperT::BeLeafNodeT&,
                      std::vector<std::tuple<std::size_t, K, V>>);
    // sorts the messages and returns a vector of indexes (which point to the
    // removed message blocks returned in the reference)
    std::unordered_map<std::size_t, std::vector<Upsert<K, V>>> removeMessages(
            typename BeNodeWrapperT::BeInnerNodeT&,
            std::vector<Upsert<K, V>>, std::size_t&);

    // helper functions for upsert
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
    innerNode.size = rightInnerNode.size;
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
                              return std::get<0>(a) <= std::get<0>(b);
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
        // move everything to the right
        std::move_backward(innerNode.pivots.begin() + index,
                           innerNode.pivots.begin() + lastFirst,
                           innerNode.pivots.begin() + index + it);
        std::move_backward(innerNode.children.begin() + index + 1,
                           innerNode.children.begin() + lastFirst + 1,
                           innerNode.children.begin() + index + it);
        // insert the pivot and its child
        innerNode.pivots[index + it - 1] = key;
        innerNode.children[index + it] = child;
    }
    // adjust the size
    innerNode.size += newPivots.size();
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::insertValues(typename BeNodeWrapperT::BeLeafNodeT& leafNode,
                                               std::vector<std::tuple<std::size_t, K, V>> newValues) {
    using ValueTuple = std::tuple<std::size_t, K, V>;
    // must have free slots
    assert(leafNode.size + newValues.size() <= leafNode.keys.size());
    // the elements must be sorted and unique
    assert(std::is_sorted(newValues.begin(), newValues.end(),
                          [](const ValueTuple& a, const ValueTuple& b) {
                              return std::get<0>(a) <= std::get<0>(b);
                          }));
    assert(std::is_sorted(newValues.begin(), newValues.end(),
                          [](const ValueTuple& a, const ValueTuple& b) {
                              assert(std::get<1>(a) != std::get<1>(b));
                              return std::get<1>(a) < std::get<1>(b);
                          }));
    // iterate backwards to keep the moves at a minimum
    std::size_t lastFirst = leafNode.size;// keep track of the last move
    for (std::size_t it = newValues.size(); it > 0; it--) {
        ValueTuple& valueTuple = newValues[it - 1];
        std::size_t index = std::get<0>(valueTuple);
        K key = std::get<1>(valueTuple);
        V value = std::move(std::get<2>(valueTuple));
        // must be a valid index
        assert(index + it - 1 < leafNode.keys.size());
        // move everything to the right
        std::move_backward(leafNode.keys.begin() + index,
                           leafNode.keys.begin() + lastFirst,
                           leafNode.keys.begin() + index + it);
        std::move_backward(leafNode.values.begin() + index,
                           leafNode.values.begin() + lastFirst,
                           leafNode.values.begin() + index + it);
        // insert the pivot and its child
        leafNode.keys[index + it] = key;
        leafNode.values[index + it] = std::move(value);
    }
    // adjust the size
    leafNode.size += newValues.size();
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
std::unordered_map<std::size_t, std::vector<Upsert<K, V>>>
BeTree<K, V, B, N, EPSILON>::removeMessages(
        typename BeNodeWrapperT::BeInnerNodeT& innerNode,
        std::vector<Upsert<K, V>> additionalUpserts, std::size_t& addressedChildren) {
    const std::size_t neededUpsertSlots = additionalUpserts.size();
    assert(neededUpsertSlots > innerNode.upserts.upserts.size() - innerNode.upserts.size);
    assert(neededUpsertSlots <= innerNode.upserts.upserts.size());
    // move everything to the vector
    std::move(innerNode.upserts.upserts.begin(),
              innerNode.upserts.upserts.begin() + innerNode.upserts.size,
              std::back_inserter(additionalUpserts));
    // count the number of messages of each child index
    std::unordered_map<std::size_t, std::size_t> childCount;
    std::unordered_map<K, std::size_t> childIndexes;
    for (std::size_t i = 0; i < additionalUpserts.size(); i++) {
        const auto& upsert = additionalUpserts[i];
        std::size_t childIndex;
        if (childIndexes.find(upsert.key) == childIndexes.end()) {
            // find the children of the message using binary search
            auto pivotIt = std::lower_bound(innerNode.pivots.begin(),
                                            innerNode.pivots.begin() + innerNode.size,
                                            upsert.key);
            // the left child is the target
            childIndex = pivotIt - innerNode.pivots.begin();
            childIndexes[upsert.key] = childIndex;
        } else {
            childIndex = childIndexes[upsert.key];
        }
        if (childCount.find(childIndex) == childCount.end()) {
            childCount[childIndex] = 1;
        } else {
            childCount[childIndex]++;
        }
    }
    // sort the messages
    std::sort(additionalUpserts.begin(),
              additionalUpserts.end(),
              [&childCount, &childIndexes](const Upsert<K, V>& a, const Upsert<K, V>& b) {
                  assert(childIndexes.count(a.key) && childIndexes.count(b.key));
                  std::size_t indexA = childIndexes[a.key];
                  std::size_t indexB = childIndexes[b.key];
                  assert(childCount.count(indexA) && childCount.count(indexB));
                  std::size_t countA = childCount[indexA];
                  std::size_t countB = childCount[indexB];
                  if (countA == countB) {
                      return a <= b;
                  }
                  return countA > countB;
              });
    // now the biggest messages are at the start
    std::unordered_map<std::size_t, std::vector<Upsert<K, V>>> result;// child index -> messages
    result[additionalUpserts.front().key] = {std::move(additionalUpserts.front())};
    addressedChildren = 0;
    std::size_t i;
    for (i = 1; i < additionalUpserts.size(); i++) {
        auto& upsert = additionalUpserts[i];
        if (childIndexes[upsert.key] != childIndexes[additionalUpserts[i - 1].key]) {
            if (additionalUpserts.size() - i - 1 >= neededUpsertSlots) {
                addressedChildren++;
                break;
            }
        }
        result[childIndexes[upsert.key]].push_back(std::move(upsert));
    }
    const std::size_t includedMessages = additionalUpserts.size() - i - 1;
    // i marks the beginning of the first included message block
    assert(includedMessages <= innerNode.upserts.upserts.size());
    // move the right messages back to the node
    std::move(additionalUpserts.begin() + i,
              additionalUpserts.end(),
              innerNode.upserts.upserts.begin());
    assert(includedMessages == additionalUpserts.size() - i - 1);
    innerNode.upserts.size = includedMessages;
    // sort the messages in the node
    std::sort(innerNode.upserts.upserts.begin(),
              innerNode.upserts.upserts.begin() + innerNode.upserts.size);
    return result;
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
    using MessageMap = std::unordered_map<std::size_t, std::vector<Upsert<K, V>>>;
    std::queue<std::pair<PageT*, MessageMap>> queue;// <node, messages>
    // handle root
    {
        auto& rootNode = accessNode(*rootPage).asInner();
        std::size_t addressedChildren;
        auto messageMap = std::move(removeMessages(
                rootNode, {std::move(message)}, addressedChildren));
        // check if the root needs slots for the potential splits of the children
        if (rootNode.pivots.size() - rootNode.size < addressedChildren) {
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
                if (childIndex <= newRootNode.size + 1) {
                    assert(!leftMap.count(childIndex));
                    leftMap[childIndex] = std::move(vector);
                } else {
                    const std::size_t adjustedChildIndex = childIndex - newRootNode.size - 1;
                    assert(!rightMap.count(adjustedChildIndex));
                    rightMap[adjustedChildIndex] = std::move(vector);
                }
            }
            queue.push(std::make_pair(&newRootPage, std::move(leftMap)));
            queue.push(std::make_pair(&rightPage, std::move(rightMap)));
        } else {
            // the root has enough space, pass it to the queue
            queue.push(std::make_pair(rootPage, std::move(messageMap)));
        }
    }
    // main action: level order traversal
    while (!queue.empty()) {
        auto [currentPage, messageMap] = std::move(queue.front());
        queue.pop();
        assert(!accessNode(*currentPage).isLeaf());
        // the additional messages are already removed
        auto& currentNode = accessNode(*currentPage).asInner();
        // pin each child and split it if necessary
        std::unordered_map<PageT*, MessageMap> accumulatedMaps;
        // helper map
        std::unordered_map<std::size_t, PageT*> pinnedChildren;
        std::vector<std::tuple<std::size_t, K, std::uint64_t>> pivots;
        for (auto& [childIndex, vector]: messageMap) {
            if (pinnedChildren.find(childIndex) == pinnedChildren.end()) {
                pinnedChildren[childIndex] = &pageBuffer.pinPage(
                        currentNode.children[childIndex], true);
            }
            if (accessNode(*pinnedChildren[childIndex]).isLeaf()) {
                // base case: we arrived at the leaf level
                auto& leafChild = accessNode(*pinnedChildren[childIndex]).asLeaf();
                assert(std::is_sorted(vector.begin(), vector.end()));
                // insert the messages
                std::unordered_map<PageT*, std::vector<std::tuple<std::size_t, K, V>>> accumulatedInserts;
                for (auto& upsert: vector) {
                    if (upsert.type != INSERT) {
                        // flush the inserts
                        for (auto& [pagePtr, inserts]: accumulatedInserts) {
                            assert(accessNode(*pagePtr).isLeaf());
                            insertValues(accessNode(*pagePtr).asLeaf(), std::move(inserts));
                        }
                        accumulatedInserts.clear();
                    }
                    // search for the key index
                    auto keyIt = std::lower_bound(leafChild.keys.begin(),
                                                  leafChild.keys.begin() + leafChild.size,
                                                  upsert.key);
                    std::size_t keyIndex = keyIt - leafChild.keys.begin();
                    if (upsert.type == DELETE) {
                        // the key must exist
                        assert(keyIt != leafChild.keys.end());
                        assert(*keyIt == upsert.key);
                        // delete the key (shift [index; end) one to the left)
                        std::move(leafChild.keys.begin() + keyIndex + 1,
                                  leafChild.keys.begin() + leafChild.size,
                                  leafChild.keys.begin() + keyIndex);
                        // adjust the size
                        leafChild.size--;
                        continue;
                    }
                    if (upsert.type == UPDATE) {
                        // the key must exist
                        assert(keyIt != leafChild.keys.end());
                        assert(*keyIt == upsert.key);
                        // update the key
                        leafChild.values[keyIndex] += upsert.value;
                        continue;
                    }
                    if (upsert.type == INSERT) {
                        // the key must not exist
                        assert(keyIt != leafChild.keys.end());
                        assert(*keyIt != upsert.key);
                        auto* targetPage = currentPage;// page which will receive the insert
                        if (leafChild.size == leafChild.keys.size()) {
                            // full leaf -> split it
                            K middleKey;
                            // <rightPage> is automatically uniquely pinned
                            PageT& rightPage = splitLeafNode(leafChild, middleKey);
                            // insert the pivot into the parent
                            auto pivotIt = std::lower_bound(currentNode.pivots.begin(),
                                                            currentNode.pivots.begin() + currentNode.size,
                                                            middleKey);
                            pivots.push_back(std::tuple<std::size_t, K, std::uint64_t>(
                                    pivotIt - currentNode.pivots.begin(),
                                    middleKey,
                                    rightPage.id));
                            // check which child will receive the insert
                            if (upsert.key <= middleKey) {
                                pageBuffer.unpinPage(currentPage->id, true);
                                targetPage = currentPage;
                            } else {
                                pageBuffer.unpinPage(rightPage.id, true);
                                targetPage = &rightPage;
                                // recalc keyIndex
                                keyIndex -= leafChild.size;
                            }
                        }
                        // from here on only use targetPage
                        assert(accessNode(*targetPage).isLeaf());
                        accumulatedInserts[targetPage].push_back(std::make_tuple(
                                keyIndex, std::move(upsert.key), std::move(upsert.value)));
                        continue;
                    }
                }
                // flush the remaining inserts
                if (!accumulatedInserts.empty()) {
                    for (auto& [pagePtr, inserts]: accumulatedInserts) {
                        assert(accessNode(*pagePtr).isLeaf());
                        insertValues(accessNode(*pagePtr).asLeaf(), std::move(inserts));
                    }
                }
            } else {
                // the children are inner nodes
                PageT* childPage = pinnedChildren[childIndex];
                auto& innerChild = accessNode(*childPage).asInner();
                // remove its messages
                std::size_t addressedChildren;
                auto messageMap = std::move(removeMessages(
                        innerChild, std::move(vector), addressedChildren));
                if (innerChild.pivots.size() - innerChild.size < addressedChildren) {
                    // we need to split the child
                    K midKey;
                    PageT& rightPage = splitInnerNode(innerChild, midKey);
                    pivots.push_back(std::make_tuple(
                            std::lower_bound(currentNode.pivots.begin(),
                                             currentNode.pivots.begin() + currentNode.size,
                                             midKey) -
                                    currentNode.pivots.begin(),
                            midKey, rightPage.id));
                    // split the additional messages
                    for (auto& [childIndex, vector]: messageMap) {
                        if (childIndex <= innerChild.size + 1) {
                            assert(!accumulatedMaps.count(childPage));
                            accumulatedMaps[childPage][childIndex] = std::move(vector);
                        } else {
                            const std::size_t adjustedChildIndex = childIndex - innerChild.size - 1;
                            assert(!accumulatedMaps.count(&rightPage));
                            accumulatedMaps[&rightPage][adjustedChildIndex] = std::move(vector);
                        }
                    }
                } else {
                    assert(!accumulatedMaps.count(childPage));
                    // just pass the child to the map
                    accumulatedMaps[childPage] = std::move(messageMap);
                }
            }
        }
        // insert the pivots into the parent
        insertPivots(currentNode, std::move(pivots));
        // unlock the parent
        pageBuffer.unpinPage(currentPage->id, true);
        // add the children to the queue
        for (auto& [page, messageMap]: accumulatedMaps) {
            queue.push(std::make_pair(page, std::move(messageMap)));
        }
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
        // there is enough space in the root, insert sorted
        auto it = std::lower_bound(innerNode.upserts.upserts.begin(),
                                   innerNode.upserts.upserts.begin() + innerNode.upserts.size,
                                   upsert);
        const std::size_t index = it - innerNode.upserts.upserts.begin();
        // shift to the right
        std::move_backward(it,
                           innerNode.upserts.upserts.begin() + innerNode.upserts.size,
                           it + 1);
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
    const std::size_t keyIndex = keyIt - leafNode.keys.begin();
    if (upsert.type == DELETE) {
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
    if (upsert.type == UPDATE) {
        // the key must exist
        assert(keyIt != leafNode.keys.end());
        assert(*keyIt == upsert.key);
        // update the key
        leafNode.values[keyIndex] += upsert.value;
        pageBuffer.unpinPage(rootPage->id, true);
        return;
    }
    if (upsert.type == INSERT) {
        // the key must not exist
        assert(keyIt != leafNode.keys.end());
        assert(*keyIt != upsert.key);
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
            }
            // update the pointers
            auto* left = &newRoot;
            auto* right = &rightPage;
            {
                auto* parent = rootPage;
                // from here on only use left, right and parent
                // as the other pointers and references may be dangling
                auto& currentParentNode = accessNode(*parent).asInner();
                auto parentPivotIt = std::lower_bound(currentParentNode.pivots.begin(),
                                                      currentParentNode.pivots.begin() + currentParentNode.size,
                                                      middleKey);
                assert(parentPivotIt != currentParentNode.pivots.end());
                // insert the new leaf into the parent
                insertPivots(currentParentNode,
                             {std::tuple<std::size_t, K, std::uint64_t>(
                                     parentPivotIt - currentParentNode.pivots.begin(),
                                     middleKey,
                                     rightPage.id)});
                // free the parent
                pageBuffer.unpinPage(parent->id, true);
            }
            // check which child will receive the insert
            if (upsert.key <= middleKey) {
                pageBuffer.unpinPage(right->id, true);
                targetPage = left;
            } else {
                pageBuffer.unpinPage(left->id, true);
                targetPage = right;
            }
        }
        // from here on only use targetPage
        assert(targetPage != nullptr);
        assert(accessNode(*targetPage).isLeaf());
        auto& targetLeafNode = accessNode(*targetPage).asLeaf();
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
        handleRootLeafUpsert(std::move(upsert), rootPage);
        return;
    }
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
            INSERT};
    upsert(std::move(message));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::update(K key, V value) {
    Upsert<K, V> message{
            std::move(key),
            std::move(value),
            ++header.currentTimeStamp,
            UPDATE};
    upsert(std::move(message));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
void BeTree<K, V, B, N, EPSILON>::erase(const K& key) {
    Upsert<K, V> message{
            key,
            V(),
            ++header.currentTimeStamp,
            DELETE};
    upsert(std::move(message));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N, short EPSILON>
std::optional<V> BeTree<K, V, B, N, EPSILON>::find(const K& key) {
    std::vector<V> accumulatedUpdates;
    std::optional<V> currentValue;
    PageT* currentPage = &pageBuffer.pinPage(header.rootID, false);
    while (!accessNode(*currentPage).isLeaf()) {
        auto& innerNode = accessNode(*currentPage).asInner();
        // find the last element of the message block
        auto lastIt = std::upper_bound(innerNode.upserts.upserts.begin(),
                                       innerNode.upserts.upserts.begin() + innerNode.upserts.size,
                                       key, [](const K& k, const Upsert<K, V>& upsert) {
                                           return k < upsert.key;
                                       });
        // <lastIt> points to the first element greater than <key>
        std::vector<V> localUpdates;
        bool deleted = false;
        for (; lastIt != innerNode.upserts.upserts.begin() && (lastIt - 1)->key == key; --lastIt) {
            const Upsert<K, V>& upsert = *lastIt;
            if (upsert.type == DELETE) {
                localUpdates.clear();
                currentValue = std::nullopt;
                deleted = true;
                continue;
            }
            if (upsert.type == UPDATE) {
                localUpdates.push_back(upsert.value);
                continue;
            }
            if (upsert.type == INSERT) {
                localUpdates.clear();
                currentValue = upsert.value;
                deleted = false;
                continue;
            }
        }
        if (deleted) {
            accumulatedUpdates.clear();
        }
        std::move(localUpdates.rbegin(), localUpdates.rend(), std::back_inserter(accumulatedUpdates));
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
    if (currentPage) {
        // leaf
        if (!currentValue) {
            // we still need a base
            auto& leafNode = accessNode(*currentPage).asLeaf();
            auto childIt = std::lower_bound(leafNode.keys.begin(),
                                            leafNode.keys.begin() + leafNode.size,
                                            key);
            if (*childIt == key) {
                currentValue = leafNode.values[childIt - leafNode.keys.begin()];
            }
        }
        pageBuffer.unpinPage(currentPage->id, false);
    } else {
        // inserted or deleted (delted -> accumulatedUpdates is empty)
        for (auto it = accumulatedUpdates.rbegin(); it != accumulatedUpdates.rend(); ++it) {
            assert(currentValue);
            (*currentValue) += *it;
        }
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
}// namespace tree
// --------------------------------------------------------------------------
#endif//B_EPSILON_BETREE_H
