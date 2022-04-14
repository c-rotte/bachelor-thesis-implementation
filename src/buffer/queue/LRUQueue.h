#ifndef B_EPSILON_LRUQUEUE_H
#define B_EPSILON_LRUQUEUE_H
// --------------------------------------------------------------------------
#include "Queue.h"
#include <cassert>
#include <stdexcept>
#include <utility>
// --------------------------------------------------------------------------
namespace buffer::queue {
// --------------------------------------------------------------------------
template<class K, class V>
class LRUQueue : public Queue<K, V> {

public:
    explicit LRUQueue(std::size_t);

    virtual V& find(const K&) override;
};
// --------------------------------------------------------------------------
template<class K, class V>
LRUQueue<K, V>::LRUQueue(std::size_t maxEntries) : Queue<K, V>(maxEntries) {
}
// --------------------------------------------------------------------------
template<class K, class V>
V& LRUQueue<K, V>::find(const K& key) {
    if (!this->contains(key)) {
        throw std::runtime_error("Key was not found! (LRU)");
    }
    auto it = this->pointerMap[key];
    // move entry to the front
    this->entryQueue.splice(this->entryQueue.begin(), this->entryQueue, it);
    return it->second;
}
// --------------------------------------------------------------------------
}//namespace buffer::queue
// --------------------------------------------------------------------------
#endif//B_EPSILON_LRUQUEUE_H
