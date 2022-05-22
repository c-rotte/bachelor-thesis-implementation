#ifndef B_EPSILON_BTREE_H
#define B_EPSILON_BTREE_H
// --------------------------------------------------------------------------
#include "BNode.h"
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
    void insertTraversal(K, V, PageT*);
    void handleRootLeafInsert(K, V, PageT*);
    void handleRootInnerInsert(K, V, PageT*);

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
            throw std::runtime_error("Invalid btree header!");
        }
    } else {
        fd = open(headerFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            throw std::runtime_error("Could not create the btree header!");
        }
        header.rootID = pageBuffer.createPage();
        // initialize the root node (leaf)
        initializeNode(pageBuffer.pinPage(header.rootID, true, true), true);
        pageBuffer.unpinPage(header.rootID, true);
        if (ftruncate(fd, sizeof(Header)) < 0) {
            throw std::runtime_error("Could not increase the file size (btree).");
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
void BTree<K, V, B, N>::insertTraversal(K key, V value, PageT* parentPage) {
    // must not be a leaf
    assert(!accessNode(*parentPage).isLeaf());
    auto& parentNode = accessNode(*parentPage).asInner();
    assert(parentNode.size < parentNode.pivots.size());
    // search for the key index
    auto keyIt = std::lower_bound(parentNode.pivots.begin(),
                                  parentNode.pivots.begin() + parentNode.size,
                                  key);
    std::size_t keyIndex = keyIt - parentNode.pivots.begin();
    std::uint64_t childID = parentNode.children[keyIndex];
    // pin the child
    PageT& childPage = pageBuffer.pinPage(childID, true);
    if (accessNode(childPage).isLeaf()) {
        auto* targetPage = &childPage;
        {
            auto& leafNode = accessNode(childPage).asLeaf();
            if (keyIndex >= leafNode.size || leafNode.keys[keyIndex] != key) {
                // the key already exists -> overwrite it
                leafNode.values[keyIndex] = std::move(value);
                // unpin the page
                pageBuffer.unpinPage(targetPage->id, true);
                return;
            }
            // check if we need to split the leaf
            assert(leafNode.size <= leafNode.keys.size());
            if (leafNode.size == leafNode.keys.size()) {
                K middleKey;
                PageT& rightPage = splitLeafNode(leafNode, middleKey);
                // insert the pivot
                std::move_backward(parentNode.pivots.begin() + keyIndex,
                                   parentNode.pivots.begin() + parentNode.size,
                                   parentNode.pivots.begin() + parentNode.size + 1);
                parentNode.pivots[keyIndex] = middleKey;
                std::move_backward(parentNode.children.begin() + keyIndex + 1,
                                   parentNode.children.begin() + parentNode.size + 1,
                                   parentNode.children.begin() + parentNode.size + 2);
                parentNode.children[keyIndex + 1] = rightPage.id;
                // unpin the parent
                pageBuffer.unpinPage(parentPage->id, true);
                // check which child will receive the insert
                if (key <= middleKey) {
                    pageBuffer.unpinPage(rightPage.id, true);
                    targetPage = &childPage;
                } else {
                    // adjust the key index
                    auto& rightNode = accessNode(rightPage).asLeaf();
                    keyIndex -= rightNode.size;
                    pageBuffer.unpinPage(childPage.id, true);
                    targetPage = &rightPage;
                }
            } else {
                // unpin the parent
                pageBuffer.unpinPage(parentPage->id, false);
            }
        }
        // perform the insert
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
        targetLeafNode.keys[keyIndex] = key;
        targetLeafNode.values[keyIndex] = std::move(value);
        // adjust the size
        targetLeafNode.size++;
        // unpin the page
        pageBuffer.unpinPage(targetPage->id, true);
    } else {
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::handleRootInnerInsert(K key, V value, PageT* rootPage) {
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
    if (innerNode.size == innerNode.pivots.size()) {
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
            auto& newRootNode = accessNode(newRoot).asLeaf();
            keyIndex -= newRootNode.size;
            pageBuffer.unpinPage(newRoot.id, true);
            targetPage = &rightPage;
        }
    }
    insertTraversal(key, std::move(value), targetPage);
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
    PageT* rootPage = &pageBuffer.pinPage(header.rootID, true);
    // first case: the root node is a leaf node
    if (accessNode(*rootPage).isLeaf()) {
        handleRootLeafInsert(key, std::move(value), rootPage);
        return;
    }
    // second case: the root node is an inner node
    handleRootInnerInsert(key, std::move(value), rootPage);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::update(K key, V value) {
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::erase(const K& key) {
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
std::optional<V> BTree<K, V, B, N>::find(const K& key) {
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
void BTree<K, V, B, N>::flush() {
    if (pwrite(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
        throw std::runtime_error("Could not save the header (betree).");
    }
    pageBuffer.flush();
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B, std::size_t N>
std::ostream& operator<<(std::ostream& out, BTree<K, V, B, N>& tree) {
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