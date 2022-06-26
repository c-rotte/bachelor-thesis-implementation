#include "betree/BeNode.h"
#include "btree/BNode.h"
#include <iostream>
// --------------------------------------------------------------------------
using namespace std;
namespace besizes = betree::sizes;
namespace bsizes = btree::sizes;
// --------------------------------------------------------------------------
constexpr std::size_t BLOCK_SIZE = 16384;
using Key = std::uint64_t;
using Value = std::array<unsigned char, 100>;
// --------------------------------------------------------------------------
template<std::size_t N>
struct BeNodeForLoop {
    template<std::size_t EPSILON>
    static void iteration() {
        using NodeSizesT = besizes::NodeSizes<Key, Value, BLOCK_SIZE, EPSILON>;
        std::cout << "epsilon=" << EPSILON
                  << ": LeafN=" << NodeSizesT::LEAF_N
                  << ", InnerN=" << NodeSizesT::INNER_N
                  << ", InnerBN=" << NodeSizesT::INNER_B_N
                  << ", RootN=" << NodeSizesT::ROOT_N
                  << std::endl;
        if constexpr (EPSILON + 10 < N) {
            BeNodeForLoop<N>::iteration<EPSILON + 10>();
        }
    }
};
// --------------------------------------------------------------------------
int main() {
    std::cout << "BeNode:\n";
    BeNodeForLoop<101>::iteration<0>();
    std::cout << "\n";
    using NodeSizesT = bsizes::NodeSizes<Key, Value, BLOCK_SIZE>;
    std::cout << "BNode:\n"
              << "LeafN=" << NodeSizesT::LEAF_N
              << ", InnerN=" << NodeSizesT::INNER_N
              << std::endl;
    return 0;
}
// --------------------------------------------------------------------------
