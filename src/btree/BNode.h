#ifndef B_EPSILON_BNODE_H
#define B_EPSILON_BNODE_H
// --------------------------------------------------------------------------
#include <array>
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <optional>
// --------------------------------------------------------------------------
namespace btree {
// --------------------------------------------------------------------------
template<class K, std::size_t N>
struct BInnerNode {
    std::array<K, N> pivots;
    std::array<std::uint64_t, N + 1> children;
    std::uint64_t size = 0;// amount of pivots
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t N>
struct BLeafNode {
    std::array<K, N> keys;
    std::array<V, N> values;
    std::uint64_t size = 0;
};
// --------------------------------------------------------------------------
namespace sizes {
// --------------------------------------------------------------------------
template<class K, class V, std::size_t TOTAL_SIZE>
struct NodeSizes {
private:
    static constexpr std::size_t getLeafN();
    static constexpr std::size_t getInnerN();

    // the page size contains the following:
    static constexpr std::size_t BASE_LEAF_SIZE = TOTAL_SIZE - sizeof(BLeafNode<K, V, 0>);
    static constexpr std::size_t BASE_INNER_SIZE = TOTAL_SIZE - sizeof(BInnerNode<K, 0>);

    // n * sizeof(K) + (n + 1) * sizeof(uint64_t) = INNER_DATA_SIZE <->
    // n * (sizeof(K) + sizeof(uint64_t)) = INNER_DATA_SIZE - sizeof(uint64_t) <->
    // n = (INNER_DATA_SIZE - sizeof(uint64_t)) / (sizeof(K) + sizeof(uint64_t))
    static constexpr std::size_t INNER_N_BEFORE_CORRECTION =
            (BASE_INNER_SIZE - sizeof(std::uint64_t)) / (sizeof(K) + sizeof(std::uint64_t));

public:
    static constexpr std::size_t LEAF_N = getLeafN();
    static constexpr std::size_t INNER_N = getInnerN();
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t TOTAL_SIZE>
constexpr std::size_t NodeSizes<K, V, TOTAL_SIZE>::getLeafN() {
    // space of an empty leaf divided by the size of one pair<Key, Value>
    return BASE_LEAF_SIZE / (sizeof(K) + sizeof(V));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t TOTAL_SIZE>
constexpr std::size_t NodeSizes<K, V, TOTAL_SIZE>::getInnerN() {
    // preliminary splits only work with odd N
    return INNER_N_BEFORE_CORRECTION - (1 - INNER_N_BEFORE_CORRECTION % 2);
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
class BNodeWrapper {

public:
    // 1 byte is needed for the bool
    using NodeSizesT = sizes::NodeSizes<K, V, PAGE_SIZE - 1>;
    using BInnerNodeT = BInnerNode<K, NodeSizesT::INNER_N>;
    using BLeafNodeT = BLeafNode<K, V, NodeSizesT::LEAF_N>;

    // make sure that everything fits on the page
    static_assert(sizeof(BInnerNodeT) < PAGE_SIZE);
    static_assert(sizeof(BLeafNodeT) < PAGE_SIZE);
    static_assert(alignof(bool) == 1);
    // the btree only works with an INNER_N of >= 3
    static_assert(NodeSizesT::INNER_N >= 3);
    // to perform preliminary splits, we need an odd number of pivots
    static_assert(NodeSizesT::INNER_N % 2 == 1);

private:
    // the node data starts here
    alignas(std::max_align_t) std::array<unsigned char, PAGE_SIZE - 1> data;
    const bool leaf;

public:
    explicit BNodeWrapper(bool);

    bool isLeaf() const;
    BInnerNodeT& asInner();
    BLeafNodeT& asLeaf();
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
BNodeWrapper<K, V, PAGE_SIZE>::BNodeWrapper(bool leaf) : leaf(leaf) {
    if (leaf) {
        new (data.data()) BLeafNodeT;
    } else {
        new (data.data()) BInnerNodeT;
    }
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
bool BNodeWrapper<K, V, PAGE_SIZE>::isLeaf() const {
    return leaf;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
typename BNodeWrapper<K, V, PAGE_SIZE>::BInnerNodeT&
BNodeWrapper<K, V, PAGE_SIZE>::asInner() {
    assert(!leaf);
    return *reinterpret_cast<BInnerNodeT*>(data.data());
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
typename BNodeWrapper<K, V, PAGE_SIZE>::BLeafNodeT&
BNodeWrapper<K, V, PAGE_SIZE>::asLeaf() {
    assert(leaf);
    return *reinterpret_cast<BLeafNodeT*>(data.data());
}
// --------------------------------------------------------------------------
}// namespace btree
// --------------------------------------------------------------------------
#endif//B_EPSILON_BNODE_H
