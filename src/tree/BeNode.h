#ifndef B_EPSILON_BENODE_H
#define B_EPSILON_BENODE_H
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
namespace tree {
// --------------------------------------------------------------------------
struct UpsertType {
    static const unsigned char INSERT = 0;
    static const unsigned char UPDATE = 1;
    static const unsigned char DELETE = 2;
};
// --------------------------------------------------------------------------
template<class K, class V>
struct Upsert {
    K key;
    V value;// don't wrap it into an optional to save space
    std::uint32_t timeStamp = -1;
    unsigned char type;

    auto operator<=>(const Upsert&) const;
    auto operator<=>(const K&) const;
};
// --------------------------------------------------------------------------
template<class K, class V>
auto Upsert<K, V>::operator<=>(const Upsert<K, V>& other) const {
    // order the upserts first by key and then by timestamp
    if (key == other.key) {
        return timeStamp <=> other.timeStamp;
    }
    return key <=> other.key;
}
// --------------------------------------------------------------------------
template<class K, class V>
auto Upsert<K, V>::operator<=>(const K& otherKey) const {
    return key <=> otherKey;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t N>
struct UpsertBuffer {
    std::array<Upsert<K, V>, N> upserts;
    std::uint64_t size = 0;
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B_N, std::size_t N>
struct BeInnerNode {
    UpsertBuffer<K, V, B_N> upserts;
    std::array<K, N> pivots;
    std::array<std::uint64_t, N + 1> children;
    std::uint64_t size = 0;// amount of pivots
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t N>
struct BeLeafNode {
    std::array<K, N> keys;
    std::array<V, N> values;
    std::uint64_t size = 0;
};
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
// sizeof(buffer) = B - B^epsilon
// ->
// epsilon=0: 100% buffer
// epsilon=100: 0% buffer
// --------------------------------------------------------------------------
template<class K, class V, std::size_t TOTAL_SIZE, short EPSILON>
struct NodeSizes {
private:
    // epsilon must be ∈ [0;1] (divided by 100)
    static_assert(EPSILON >= 0 && EPSILON <= 100);

    static constexpr std::size_t getLeafN();
    static constexpr std::size_t getInnerN();
    static constexpr std::size_t getInnerBN();

    // the page size contains the following:
    static constexpr std::size_t BASE_LEAF_SIZE = TOTAL_SIZE - sizeof(BeLeafNode<K, V, 0>);
    static constexpr std::size_t BASE_INNER_SIZE = TOTAL_SIZE - sizeof(BeInnerNode<K, V, 0, 0>);

    static constexpr std::size_t INNER_DATA_SIZE = BASE_INNER_SIZE * EPSILON / 100;
    static constexpr std::size_t INNER_BUFFER_SIZE = BASE_INNER_SIZE - INNER_DATA_SIZE;

    // n * sizeof(K) + (n + 1) * sizeof(uint64_t) = INNER_DATA_SIZE <->
    // n * (sizeof(K) + sizeof(uint64_t)) = INNER_DATA_SIZE - sizeof(uint64_t) <->
    // n = (INNER_DATA_SIZE - sizeof(uint64_t)) / (sizeof(K) + sizeof(uint64_t))
    static constexpr std::size_t INNER_N_BEFORE_CORRECTION =
            (INNER_DATA_SIZE - sizeof(std::uint64_t)) / (sizeof(K) + sizeof(std::uint64_t));

public:
    static constexpr std::size_t LEAF_N = getLeafN();
    static constexpr std::size_t INNER_N = getInnerN();
    static constexpr std::size_t INNER_B_N = getInnerBN();
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t TOTAL_SIZE, short EPSILON>
constexpr std::size_t NodeSizes<K, V, TOTAL_SIZE, EPSILON>::getLeafN() {
    // space of an empty leaf divided by the size of one pair<Key, Value>
    return BASE_LEAF_SIZE / (sizeof(K) + sizeof(V));
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t TOTAL_SIZE, short EPSILON>
constexpr std::size_t NodeSizes<K, V, TOTAL_SIZE, EPSILON>::getInnerN() {
    // preliminary splits only work with odd N
    return INNER_N_BEFORE_CORRECTION - (1 - INNER_N_BEFORE_CORRECTION % 2);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t TOTAL_SIZE, short EPSILON>
constexpr std::size_t NodeSizes<K, V, TOTAL_SIZE, EPSILON>::getInnerBN() {
    // space of an empty buffer divided by the size of one Upsert<Key, Value>
    constexpr std::size_t bn = INNER_BUFFER_SIZE / sizeof(Upsert<K, V>);
    // check if we can move the removed slot to the buffer
    // (since we removed one slot from the inner node)
    if constexpr (sizeof(BeInnerNode<K, V, bn + 1, INNER_N>) <= TOTAL_SIZE) {
        return bn + 1;
    }
    return bn;
}
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE, short EPSILON>
class BeNodeWrapper {

public:
    // 1 byte is needed for the bool
    using NodeSizesT = NodeSizes<K, V, PAGE_SIZE - 1, EPSILON>;
    using BeInnerNodeT = BeInnerNode<K, V, NodeSizesT::INNER_B_N, NodeSizesT::INNER_N>;
    using BeLeafNodeT = BeLeafNode<K, V, NodeSizesT::LEAF_N>;

    // make sure that everything fits on the page
    static_assert(sizeof(BeInnerNodeT) < PAGE_SIZE);
    static_assert(sizeof(BeLeafNodeT) < PAGE_SIZE);
    static_assert(alignof(bool) == 1);
    // every flush should fit into <= 2 nodes (after splitting a leaf)
    static_assert(NodeSizesT::INNER_B_N <= NodeSizesT::LEAF_N);
    // the tree only works with an INNER_N of >= 3
    static_assert(NodeSizesT::INNER_N >= 3);
    // to perform preliminary splits, we need an odd number of pivots
    static_assert(NodeSizesT::INNER_N % 2 == 1);

private:
    // the node data starts here
    alignas(std::max_align_t) std::array<unsigned char, PAGE_SIZE - 1> data;
    const bool leaf;

public:
    explicit BeNodeWrapper(bool);

    bool isLeaf() const;
    BeInnerNodeT& asInner();
    BeLeafNodeT& asLeaf();
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE, short EPSILON>
BeNodeWrapper<K, V, PAGE_SIZE, EPSILON>::BeNodeWrapper(bool leaf) : leaf(leaf) {
    if (leaf) {
        new (data.data()) BeLeafNodeT;
    } else {
        new (data.data()) BeInnerNodeT;
    }
    std::cout << NodeSizesT::LEAF_N << " " << NodeSizesT::INNER_B_N << " " << NodeSizesT::INNER_N << std::endl;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE, short EPSILON>
bool BeNodeWrapper<K, V, PAGE_SIZE, EPSILON>::isLeaf() const {
    return leaf;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE, short EPSILON>
typename BeNodeWrapper<K, V, PAGE_SIZE, EPSILON>::BeInnerNodeT&
BeNodeWrapper<K, V, PAGE_SIZE, EPSILON>::asInner() {
    assert(!leaf);
    return *reinterpret_cast<BeInnerNodeT*>(data.data());
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE, short EPSILON>
typename BeNodeWrapper<K, V, PAGE_SIZE, EPSILON>::BeLeafNodeT&
BeNodeWrapper<K, V, PAGE_SIZE, EPSILON>::asLeaf() {
    assert(leaf);
    return *reinterpret_cast<BeLeafNodeT*>(data.data());
}
// --------------------------------------------------------------------------
}// namespace tree
// --------------------------------------------------------------------------
#endif//B_EPSILON_BENODE_H
