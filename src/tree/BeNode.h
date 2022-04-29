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
enum UpsertType {
    INSERT,
    UPDATE,
    DELETE
};
// --------------------------------------------------------------------------
template<class K, class V>
struct Upsert {
    K key;
    V value;// don't wrap it into an optional to save space
    std::uint32_t timeStamp = -1;
    UpsertType type;

    auto operator<=>(const Upsert&) const;
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
template<class K, class V, std::size_t N>
struct UpsertBuffer {
    std::array<Upsert<K, V>, N> upserts;
    std::uint64_t size = 0;
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t B_N, std::size_t N>
struct InnerNodeData {
    UpsertBuffer<K, V, B_N> upserts;
    std::array<std::uint64_t, N + 1> children;
    std::array<K, N> pivots;
    std::uint64_t size = 0;
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t N>
struct LeafNodeData {
    std::array<K, N> keys;
    std::array<V, N> values;
    std::uint64_t rightNeighbour = -1;
    std::uint64_t size = 0;
};
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
// sizeof(buffer) = B - B^epsilon
// ->
// 0: 100% buffer
// 1: 0% buffer
constexpr double EPSILON = 0.5;// a double can't be a template parameter
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
struct NodeSizes {
private:
    // the page size contains the following:
    static constexpr std::size_t BASE_LEAF_SIZE = PAGE_SIZE - sizeof(LeafNodeData<K, V, 0>);
    static constexpr std::size_t BASE_INNER_SIZE = PAGE_SIZE - sizeof(InnerNodeData<K, V, 0, 0>);

    static constexpr std::size_t INNER_DATA_SIZE = BASE_INNER_SIZE * EPSILON;
    static constexpr std::size_t INNER_BUFFER_SIZE = BASE_INNER_SIZE - INNER_DATA_SIZE;

    // epsilon must be ∈ [0;1]
    static_assert(EPSILON >= 0 && EPSILON <= 1);

public:
    // space of an empty leaf divided by the size of one pair<Key, Value>
    static constexpr std::size_t LEAF_N =
            BASE_LEAF_SIZE / (sizeof(K) + sizeof(V));
    // n * sizeof(K) + (n + 1) * sizeof(uint64_t) = INNER_DATA_SIZE <->
    // n * (sizeof(K) + sizeof(uint64_t)) = INNER_DATA_SIZE - sizeof(uint64_t) <->
    // n = (INNER_DATA_SIZE - sizeof(uint64_t)) / (sizeof(K) + sizeof(uint64_t))
    static constexpr std::size_t INNER_N =
            (INNER_DATA_SIZE - sizeof(std::uint64_t)) /
            (sizeof(K) + sizeof(std::uint64_t));
    // space of an empty buffer divided by the size of one Upsert<Key, Value>
    static constexpr std::size_t INNER_B_N =
            INNER_BUFFER_SIZE / sizeof(Upsert<K, V>);
};
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
class BeNode {

    // PAGE_SIZE - 1 because we need one byte for the leaf bool
    using NodeSizesT = NodeSizes<K, V, PAGE_SIZE - 1>;
    using InnerNodeDataT = InnerNodeData<K, V, NodeSizesT::INNER_B_N, NodeSizesT::INNER_N>;
    using LeafNodeDataT = LeafNodeData<K, V, NodeSizesT::LEAF_N>;

private:
    alignas(std::max_align_t) std::array<unsigned char, PAGE_SIZE - 1> data;// the node data starts here
    const bool leaf;

public:
    explicit BeNode(bool);

public:
    InnerNodeDataT& accessAsInner();
    LeafNodeDataT& accessAsLeaf();
    bool isLeaf() const;
};
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>// TODO: remove name of unused template params
BeNode<K, V, PAGE_SIZE>::BeNode(bool leaf) : leaf(leaf) {
    if (leaf) {
        new (data.data()) LeafNodeDataT;
    } else {
        new (data.data()) InnerNodeDataT;
    }
    std::cout << NodeSizesT::LEAF_N << " " << NodeSizesT::INNER_B_N << " " << NodeSizesT::INNER_N << std::endl;
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
typename BeNode<K, V, PAGE_SIZE>::InnerNodeDataT& BeNode<K, V, PAGE_SIZE>::accessAsInner() {
    assert(!leaf);
    return *reinterpret_cast<InnerNodeDataT*>(&data);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
typename BeNode<K, V, PAGE_SIZE>::LeafNodeDataT& BeNode<K, V, PAGE_SIZE>::accessAsLeaf() {
    assert(leaf);
    return *reinterpret_cast<LeafNodeDataT*>(&data);
}
// --------------------------------------------------------------------------
template<class K, class V, std::size_t PAGE_SIZE>
bool BeNode<K, V, PAGE_SIZE>::isLeaf() const {
    return leaf;
}
// --------------------------------------------------------------------------
}// namespace tree
// --------------------------------------------------------------------------
#endif//B_EPSILON_BENODE_H
