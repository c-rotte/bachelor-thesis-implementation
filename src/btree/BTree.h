#ifndef B_EPSILON_BTREE_H
#define B_EPSILON_BTREE_H
// --------------------------------------------------------------------------
#include "BNode.h"
#include "src/buffer/PageBuffer.h"
#include "src/util/ErrorHandler.h"
#include <algorithm>
#include <atomic>
#include <cinttypes>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <functional>
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
namespace btree {
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
class BTree {

    using BNodeWrapperT = BNodeWrapper<K, V, B>;
    using PageT = buffer::Page<B>;

    // a node must fit onto a page
    static_assert(sizeof(BNodeWrapperT) == B);
    static_assert(alignof(BNodeWrapperT) == alignof(PageT));

    struct alignas(alignof(std::max_align_t)) Header {
        std::uint64_t rootID = 0;
        bool leafRoot = true;
    };

private:
    buffer::PageBuffer<B, N> pageBuffer;
    int fd;
    Header header;

public:
    BTree(const std::string&, double);

private:
    void initializeNode(PageT&, bool) const;
    BNodeWrapperT& accessNode(PageT&) const;

    // splits a leaf node by creating a new page and returning it as well
    // as a copy of the middle key (pivot)
    // note: the returned page is still exclusively locked
    PageT& splitLeafNode(typename BNodeWrapperT::BLeafNodeT&, K&);
    FRIEND_TEST(BTreeMethods, splitLeafNode);
    // splits a leaf node by creating a new page and returning it as well
    // as the removed middle key (pivot)
    // note: the returned page is still exclusively locked
    PageT& splitInnerNode(typename BNodeWrapperT::BInnerNodeT&, K&);
    FRIEND_TEST(BTreeMethods, splitInnerNode);
    // helper methods
    bool insertTraversal(K, V, PageT*, bool, bool);
    void handleRootLeafInsert(K, V, PageT*);
    bool handleRootInnerInsert(K, V, PageT*, bool);

public:
    // inserts (K,V)
    // note: if K already exists, it will get overwritten
    void insert(K, V);
    // updates (K,V_old) with V_old += V
    // note: does nothing if K does not exist
    void update(K, V);
    // removes K
    // note: does nothing if K does not exist
    void erase(const K&);
    // attempts to find (K,V) and returns V
    std::optional<V> find(const K&);
    // saves the btree
    void flush();
    // prints out the btree (dot language)
    // note: this makes use of the page buffer
    template<class K_O, class V_O, std::size_t B_O, std::size_t N_O>
    friend std::ostream& operator<<(std::ostream&, BTree<K_O, V_O, B_O, N_O>&);
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
BTree<K, V, B, N>::BTree(const std::string& path, double growthFactor)
    : pageBuffer(path, growthFactor) {
    const std::string headerFile = path + "/btree";
    if (std::filesystem::exists(headerFile) && std::filesystem::is_regular_file(headerFile)) {
        fd = open(headerFile.c_str(), O_RDWR);
        if (pread(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
            util::raise("Invalid btree header!");
        }
    } else {
        fd = open(headerFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            util::raise("Could not create the btree header!");
        }
        header.rootID = pageBuffer.createPage();
        // initialize the root node (leaf)
        initializeNode(pageBuffer.pinPage(header.rootID, true, true), true);
        pageBuffer.unpinPage(header.rootID, true);
        if (ftruncate(fd, sizeof(Header)) < 0) {
            util::raise("Could not increase the file size (btree).");
        }
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::initializeNode(PageT& page, bool leaf) const {
    new (page.data.data()) BNodeWrapperT(leaf);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
typename BTree<K, V, B, N>::BNodeWrapperT&
BTree<K, V, B, N>::accessNode(PageT& page) const {
    return *reinterpret_cast<BNodeWrapperT*>(page.data.data());
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
typename BTree<K, V, B, N>::PageT&
BTree<K, V, B, N>::splitLeafNode(typename BNodeWrapperT::BLeafNodeT& leafNode,
                                 K& resultKey) {
    assert(leafNode.size >= 2);
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
template<class K, class V, std::size_t B, std::size_t N>
typename BTree<K, V, B, N>::PageT&
BTree<K, V, B, N>::splitInnerNode(typename BNodeWrapperT::BInnerNodeT& innerNode,
                                  K& resultKey) {
    assert(innerNode.size >= 2);
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
    return rightPage;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
bool BTree<K, V, B, N>::insertTraversal(K key, V value, PageT* parentPage,
                                        bool dirtyParent, bool exclusiveMode) {
    // must not be a leaf
    assert(!accessNode(*parentPage).isLeaf());
    auto& parentNode = accessNode(*parentPage).asInner();
    assert(!exclusiveMode || parentNode.size < parentNode.pivots.size());
    // search for the pivot index
    auto pivotIt = std::lower_bound(parentNode.pivots.begin(),
                                    parentNode.pivots.begin() + parentNode.size,
                                    key);
    std::size_t pivotIndex = pivotIt - parentNode.pivots.begin();
    std::uint64_t childID = parentNode.children[pivotIndex];
    // pin the child
    PageT& childPage = pageBuffer.pinPage(childID, false, false,
                                          [exclusiveMode, this](PageT& page) {
                                              return exclusiveMode || accessNode(page).isLeaf();
                                          });
    if (!exclusiveMode) {
        pageBuffer.unpinPage(parentPage->id, false);
    }
    if (accessNode(childPage).isLeaf()) {
        auto* targetPage = &childPage;
        auto& leafNode = accessNode(childPage).asLeaf();
        // search for the key index
        auto keyIt = std::lower_bound(leafNode.keys.begin(),
                                      leafNode.keys.begin() + leafNode.size,
                                      key);
        std::size_t keyIndex = keyIt - leafNode.keys.begin();
        bool split = false;
        {
            // check if we need to split the leaf
            assert(leafNode.size <= leafNode.keys.size());
            if (leafNode.size == leafNode.keys.size()) {
                if (!exclusiveMode) {
                    pageBuffer.unpinPage(childPage.id, false);
                    return false;
                }
                K middleKey;
                PageT& rightPage = splitLeafNode(leafNode, middleKey);
                // insert the pivot
                std::move_backward(parentNode.pivots.begin() + pivotIndex,
                                   parentNode.pivots.begin() + parentNode.size,
                                   parentNode.pivots.begin() + parentNode.size + 1);
                parentNode.pivots[pivotIndex] = middleKey;
                std::move_backward(parentNode.children.begin() + pivotIndex + 1,
                                   parentNode.children.begin() + parentNode.size + 1,
                                   parentNode.children.begin() + parentNode.size + 2);
                parentNode.children[pivotIndex + 1] = rightPage.id;
                parentNode.size++;
                // unpin the parent
                pageBuffer.unpinPage(parentPage->id, true);
                // check which child will receive the insert
                if (key <= middleKey) {
                    pageBuffer.unpinPage(rightPage.id, true);
                    targetPage = &childPage;
                } else {
                    // adjust the key index
                    keyIndex -= leafNode.size;
                    pageBuffer.unpinPage(childPage.id, true);
                    targetPage = &rightPage;
                }
                split = true;
            } else {
                if (exclusiveMode) {
                    // unpin the parent
                    pageBuffer.unpinPage(parentPage->id, dirtyParent);
                }
            }
        }
        // perform the insert
        assert(targetPage != nullptr);
        assert(accessNode(*targetPage).isLeaf());
        auto& targetLeafNode = accessNode(*targetPage).asLeaf();
        if (keyIndex < targetLeafNode.size && * keyIt == key) {
            // the key already exists -> overwrite it
            if (targetLeafNode.values[keyIndex] == value) {
                pageBuffer.unpinPage(targetPage->id, split);
                return true;
            }
            targetLeafNode.values[keyIndex] = std::move(value);
            // unpin the page
            pageBuffer.unpinPage(targetPage->id, true);
            return true;
        }
        // make room for the key (shift [index; end) one to the right)
        std::move_backward(targetLeafNode.keys.begin() + keyIndex,
                           targetLeafNode.keys.begin() + targetLeafNode.size,
                           targetLeafNode.keys.begin() + targetLeafNode.size + 1);
        std::move_backward(targetLeafNode.values.begin() + keyIndex,
                           targetLeafNode.values.begin() + targetLeafNode.size,
                           targetLeafNode.values.begin() + targetLeafNode.size + 1);
        // insert the pair
        targetLeafNode.keys[keyIndex] = key;
        targetLeafNode.values[keyIndex] = std::move(value);
        // adjust the size
        targetLeafNode.size++;
        // unpin the page
        pageBuffer.unpinPage(targetPage->id, true);
        return true;
    } else {
        auto* targetPage = &childPage;
        bool split = false;
        {
            auto& innerNode = accessNode(childPage).asInner();
            assert(innerNode.size <= innerNode.pivots.size());
            // check if we need to split
            if (innerNode.size == innerNode.pivots.size() && exclusiveMode) {
                K middleKey;
                PageT& rightPage = splitInnerNode(innerNode, middleKey);
                // insert the pivot
                std::move_backward(parentNode.pivots.begin() + pivotIndex,
                                   parentNode.pivots.begin() + parentNode.size,
                                   parentNode.pivots.begin() + parentNode.size + 1);
                parentNode.pivots[pivotIndex] = middleKey;
                std::move_backward(parentNode.children.begin() + pivotIndex + 1,
                                   parentNode.children.begin() + parentNode.size + 1,
                                   parentNode.children.begin() + parentNode.size + 2);
                parentNode.children[pivotIndex + 1] = rightPage.id;
                parentNode.size++;
                // unpin the parent
                pageBuffer.unpinPage(parentPage->id, true);
                // check which page receives the insert
                if (key <= middleKey) {
                    pageBuffer.unpinPage(rightPage.id, true);
                    targetPage = &childPage;
                } else {
                    pageBuffer.unpinPage(childPage.id, true);
                    targetPage = &rightPage;
                }
                split = true;
            } else {
                if (exclusiveMode) {
                    // unpin the parent
                    pageBuffer.unpinPage(parentPage->id, dirtyParent);
                }
            }
        }
        // recursive call
        return insertTraversal(key, std::move(value), targetPage, split, exclusiveMode);
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
bool BTree<K, V, B, N>::handleRootInnerInsert(K key, V value,
                                              PageT* rootPage, bool exclusiveMode) {
    // must be called with the root page
    assert(rootPage != nullptr);
    assert(rootPage->id == header.rootID);
    // must not be a leaf
    assert(!accessNode(*rootPage).isLeaf());
    auto& innerNode = accessNode(*rootPage).asInner();
    // search for the key index
    auto keyIt = std::lower_bound(innerNode.pivots.begin(),
                                  innerNode.pivots.begin() + innerNode.size,
                                  key);
    std::size_t keyIndex = keyIt - innerNode.pivots.begin();
    // we need to check for a split
    auto* targetPage = rootPage;// page which will receive the insert
    assert(innerNode.size <= innerNode.pivots.size());
    bool dirtyParent = false;
    if (innerNode.size == innerNode.pivots.size() && exclusiveMode) {
        // full root -> split it preemptively
        K middleKey;
        // <rightPage> is automatically uniquely pinned
        PageT& rightPage = splitInnerNode(innerNode, middleKey);
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
        if (key <= middleKey) {
            pageBuffer.unpinPage(rightPage.id, true);
            targetPage = &newRoot;
        } else {
            // adjust the key index
            auto& newRootNode = accessNode(newRoot).asInner();
            keyIndex -= newRootNode.size;
            pageBuffer.unpinPage(newRoot.id, true);
            targetPage = &rightPage;
        }
        dirtyParent = true;
    }
    return insertTraversal(key, std::move(value), targetPage, dirtyParent, exclusiveMode);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::handleRootLeafInsert(K key, V value, PageT* rootPage) {
    // must be called with the root page
    assert(rootPage != nullptr);
    assert(rootPage->id == header.rootID);
    // must be a leaf
    assert(accessNode(*rootPage).isLeaf());
    auto& leafNode = accessNode(*rootPage).asLeaf();
    // search for the key index
    auto keyIt = std::lower_bound(leafNode.keys.begin(),
                                  leafNode.keys.begin() + leafNode.size,
                                  key);
    std::size_t keyIndex = keyIt - leafNode.keys.begin();
    // perform the insert
    if (keyIndex < leafNode.size && * keyIt == key) {
        // the key does already exist -> overwrite
        if (leafNode.values[keyIndex] != value) {
            leafNode.values[keyIndex] = std::move(value);
            pageBuffer.unpinPage(rootPage->id, true);
        } else {
            pageBuffer.unpinPage(rootPage->id, false);
        }
        return;
    }
    auto* targetPage = rootPage;// page which will receive the insert
    assert(leafNode.size <= leafNode.keys.size());
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
            // set the leaf bool
            header.leafRoot = false;
            // free the parent
            pageBuffer.unpinPage(rootPage->id, true);
        }
        // check which child will receive the insert
        if (key <= middleKey) {
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
    if (keyIndex < targetLeafNode.size && targetLeafNode.keys[keyIndex] == key) {
        // the key already exists -> overwrite it
        targetLeafNode.values[keyIndex] = std::move(value);
        // unpin the page
        pageBuffer.unpinPage(targetPage->id, true);
        return;
    }
    // make room for the key (shift [index; end) one to the right)
    std::move_backward(targetLeafNode.keys.begin() + keyIndex,
                       targetLeafNode.keys.begin() + targetLeafNode.size,
                       targetLeafNode.keys.begin() + targetLeafNode.size + 1);
    std::move_backward(targetLeafNode.values.begin() + keyIndex,
                       targetLeafNode.values.begin() + targetLeafNode.size,
                       targetLeafNode.values.begin() + targetLeafNode.size + 1);
    // insert the pair
    targetLeafNode.keys[keyIndex] = key;
    targetLeafNode.values[keyIndex] = std::move(value);
    // adjust the size
    targetLeafNode.size++;
    // unpin the page
    pageBuffer.unpinPage(targetPage->id, true);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::insert(K key, V value) {
    PageT* rootPage = &pageBuffer.pinPage(header.rootID, header.leafRoot);
    // first case: the root node is a leaf node
    if (accessNode(*rootPage).isLeaf()) {
        assert(header.leafRoot);
        handleRootLeafInsert(key, std::move(value), rootPage);
        return;
    }
    assert(!header.leafRoot);
    // second case: the root node is an inner node
    bool success = handleRootInnerInsert(key, value, rootPage, false);
    if (!success) {
        rootPage = &pageBuffer.pinPage(header.rootID, true);
        handleRootInnerInsert(key, std::move(value), rootPage, true);
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::update(K key, V value) {
    PageT* currentPage = &pageBuffer.pinPage(header.rootID, false);
    // after a few inserts, the root will be an inner node
    if (accessNode(*currentPage).isLeaf()) {
        // repin uniquely
        pageBuffer.unpinPage(header.rootID, false);
        currentPage = &pageBuffer.pinPage(header.rootID, true);
    }
    while (!accessNode(*currentPage).isLeaf()) {
        auto& currentNode = accessNode(*currentPage).asInner();
        // search for the key index
        auto keyIt = std::lower_bound(currentNode.pivots.begin(),
                                      currentNode.pivots.begin() + currentNode.size,
                                      key);
        std::size_t keyIndex = keyIt - currentNode.pivots.begin();
        std::uint64_t childID = currentNode.children[keyIndex];
        // pin the child
        PageT* childPage = &pageBuffer.pinPage(childID, false, false,
                                               [this](PageT& page) {
                                                   return accessNode(page).isLeaf();
                                               });
        // unpin the parent
        pageBuffer.unpinPage(currentPage->id, false);
        // set the current page to the child
        currentPage = childPage;
    }
    // currentPage is now a uniquely pinned leaf node
    auto& leafNode = accessNode(*currentPage).asLeaf();
    // search for the key index
    auto keyIt = std::lower_bound(leafNode.keys.begin(),
                                  leafNode.keys.begin() + leafNode.size,
                                  key);
    std::size_t keyIndex = keyIt - leafNode.keys.begin();
    bool found = false;
    if (keyIndex < leafNode.size && * keyIt == key) {
        // the tree contains the key -> update its value
        leafNode.values[keyIndex] = leafNode.values[keyIndex] + std::move(value);
        found = true;
    }
    pageBuffer.unpinPage(currentPage->id, found);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::erase(const K& key) {
    PageT* currentPage = &pageBuffer.pinPage(header.rootID, false);
    // after a few inserts, the root will be an inner node
    if (accessNode(*currentPage).isLeaf()) {
        // repin uniquely
        pageBuffer.unpinPage(header.rootID, false);
        currentPage = &pageBuffer.pinPage(header.rootID, true);
    }
    while (!accessNode(*currentPage).isLeaf()) {
        auto& currentNode = accessNode(*currentPage).asInner();
        // search for the key index
        auto keyIt = std::lower_bound(currentNode.pivots.begin(),
                                      currentNode.pivots.begin() + currentNode.size,
                                      key);
        std::size_t keyIndex = keyIt - currentNode.pivots.begin();
        std::uint64_t childID = currentNode.children[keyIndex];
        // pin the child
        PageT* childPage = &pageBuffer.pinPage(childID, false, false,
                                               [this](PageT& page) {
                                                   return accessNode(page).isLeaf();
                                               });
        // unpin the parent
        pageBuffer.unpinPage(currentPage->id, false);
        // set the current page to the child
        currentPage = childPage;
    }
    // currentPage is now a uniquely pinned leaf node
    auto& leafNode = accessNode(*currentPage).asLeaf();
    // search for the key index
    auto keyIt = std::lower_bound(leafNode.keys.begin(),
                                  leafNode.keys.begin() + leafNode.size,
                                  key);
    std::size_t keyIndex = keyIt - leafNode.keys.begin();
    bool found = false;
    if (keyIndex < leafNode.size && * keyIt == key) {
        assert(leafNode.size >= 1);
        // the tree contains the key -> erase it
        std::move(leafNode.keys.begin() + keyIndex + 1,
                  leafNode.keys.begin() + leafNode.size,
                  leafNode.keys.begin() + keyIndex);
        std::move(leafNode.values.begin() + keyIndex + 1,
                  leafNode.values.begin() + leafNode.size,
                  leafNode.values.begin() + keyIndex);
        // adjust the size
        leafNode.size--;
        found = true;
    }
    pageBuffer.unpinPage(currentPage->id, found);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
std::optional<V> BTree<K, V, B, N>::find(const K& key) {
    PageT* currentPage = &pageBuffer.pinPage(header.rootID, false);
    while (!accessNode(*currentPage).isLeaf()) {
        auto& currentNode = accessNode(*currentPage).asInner();
        // search for the key index
        auto keyIt = std::lower_bound(currentNode.pivots.begin(),
                                      currentNode.pivots.begin() + currentNode.size,
                                      key);
        std::size_t keyIndex = keyIt - currentNode.pivots.begin();
        std::uint64_t childID = currentNode.children[keyIndex];
        // pin the child
        PageT* childPage = &pageBuffer.pinPage(childID, false);
        // unpin the parent
        pageBuffer.unpinPage(currentPage->id, false);
        // set the current page to the child
        currentPage = childPage;
    }
    // currentPage is now a pinned leaf node (shared)
    auto& leafNode = accessNode(*currentPage).asLeaf();
    // search for the key index
    auto keyIt = std::lower_bound(leafNode.keys.begin(),
                                  leafNode.keys.begin() + leafNode.size,
                                  key);
    std::size_t keyIndex = keyIt - leafNode.keys.begin();
    std::optional<V> result;
    if (keyIndex < leafNode.size && * keyIt == key) {
        // the tree contains the key -> return its value
        result = leafNode.values[keyIndex];
    }
    pageBuffer.unpinPage(currentPage->id, false);
    return result;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::flush() {
    if (pwrite(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
        util::raise("Could not save the header (btree).");
    }
    pageBuffer.flush();
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
std::ostream& operator<<(std::ostream& out, BTree<K, V, B, N>& tree) {
    std::unordered_set<std::uint64_t> visitedIDs;
    // level order traversal
    std::queue<std::uint64_t> queue;
    queue.push(tree.header.rootID);
    std::cout << "digraph{\n";
    while (!queue.empty()) {
        std::uint64_t currentID = queue.front();

        if (visitedIDs.contains(currentID)) {
            exit(-1);
        }
        visitedIDs.insert(currentID);

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
}// namespace btree
// --------------------------------------------------------------------------
#endif//B_EPSILON_BTREE_H
