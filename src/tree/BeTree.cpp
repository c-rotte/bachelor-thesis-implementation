#include "BeTree.h"
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
tree::BeTree<std::uint64_t, std::uint64_t, 4096, 1000, 50> tree("/tmp/be_tree", 1.25);
// --------------------------------------------------------------------------
}// namespace
 // --------------------------------------------------------------------------