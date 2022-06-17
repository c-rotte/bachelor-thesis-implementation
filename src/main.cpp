#include <array>
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>
#include <vector>
#include <random>
// #include "buffer/OptimalPageBuffer.h" // include this before the trees
#include "btree/BTree.h"
#include "betree/BeTree.h"
#include "thirdparty/ThreadPool/ThreadPool.h"
// --------------------------------------------------------------------------
using namespace std;
using namespace btree;
using namespace betree;
// --------------------------------------------------------------------------
int main() {

    static const string DIRNAME = "/tmp/tester_test_b_tree";
    std::filesystem::remove_all(DIRNAME.c_str());

    constexpr size_t BLOCK_SIZE = 8192;
    constexpr size_t PAGE_AMOUNT = 8789062;
    using Value = std::array<unsigned char, 100>;
    auto tree = std::make_unique<BTree<uint64_t, Value, BLOCK_SIZE, PAGE_AMOUNT>>(DIRNAME, 1.5);
    for(std::size_t i = 0; i < 320000000; i++){
        std::size_t key = rand();
        Value value;
        std::fill(value.begin(), value.end(), key);
        tree->insert(key, std::move(value));
    }
    ThreadPool threadPool(8);
    vector<future<void>> calls;
    {
        for (std::size_t i = 0; i < 8; i++) {
            calls.emplace_back(threadPool.enqueue([&tree]() {
                for(std::size_t i = 0; i < 320000000 / 8; i++){
                    std::size_t key = rand();
                    Value value;
                    std::fill(value.begin(), value.end(), key);
                    tree->insert(key, std::move(value));
                }
            }));
        }
        for (auto& call: calls) {
            call.get();
        }
    }

    //tree->flush();

    return 0;
}
// --------------------------------------------------------------------------
