#ifndef B_EPSILON_SIMPLEBINARYTREE_H
#define B_EPSILON_SIMPLEBINARYTREE_H
// --------------------------------------------------------------------------
#include "src/buffer/PageBuffer.h"
#include <optional>
#include <string>
// --------------------------------------------------------------------------
namespace utils {
// --------------------------------------------------------------------------
class SimpleBinaryTree {

    struct Node {
        std::optional<int> value = std::nullopt;
        std::optional<std::uint64_t> leftID = std::nullopt;
        std::optional<std::uint64_t> rightID = std::nullopt;
    };

private:
    std::size_t slowDownUS;
    buffer::PageBuffer<4096, 64> pageBuffer;
    std::uint64_t rootID;

public:
    SimpleBinaryTree(const std::string&, std::size_t);

private:
    void initializeNode(buffer::Page<4096>&) const;
    Node& getNode(buffer::Page<4096>&) const;

public:
    void insert(int);
    bool search(int);
};
// --------------------------------------------------------------------------
}// namespace utils
// --------------------------------------------------------------------------
#endif//B_EPSILON_SIMPLEBINARYTREE_H
