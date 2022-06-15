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

    constexpr size_t BLOCK_SIZE = 4096;
    constexpr size_t PAGE_AMOUNT = 6000;
    auto tree = std::make_unique<BTree<uint64_t, uint64_t, BLOCK_SIZE, PAGE_AMOUNT>>(DIRNAME, 1.25);
    vector<uint64_t> inserts(1000000);
    iota(inserts.begin(), inserts.end(), 0);
    shuffle(inserts.begin(), inserts.end(), default_random_engine());
    for(uint64_t i : inserts){
        tree->insert(i, i);
    }
    ThreadPool threadPool(4);
    vector<future<void>> calls;
    {
        for (uint64_t i: inserts) {
            calls.emplace_back(threadPool.enqueue([&tree, i]() {
                auto find = tree->find(i);
                if(!find){
                    throw std::runtime_error("not found");
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
