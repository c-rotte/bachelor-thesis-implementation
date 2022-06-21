#ifndef B_EPSILON_LRUQUEUE_H
#define B_EPSILON_LRUQUEUE_H
// --------------------------------------------------------------------------
#include "Queue.h"
#include "src/util/ErrorHandler.h"
#include <cassert>
#include <utility>
// --------------------------------------------------------------------------
namespace buffer::queue {
// --------------------------------------------------------------------------
template<class K, class V>
class LRUQueue : public Queue<K, V> {

public:
    LRUQueue() = default;

    virtual V& find(const K&, bool) override;
};
// --------------------------------------------------------------------------
template<class K, class V>
V& LRUQueue<K, V>::find(const K& key, bool modify) {
    if (!this->contains(key)) {
        util::raise("Key was not found! (LRU)");
    }
    auto it = this->pointerMap[key];
    if (modify) {
        // move entry to the front
        this->entryQueue.splice(this->entryQueue.begin(), this->entryQueue, it);
    }
    return it->second;
}
// --------------------------------------------------------------------------
}//namespace buffer::queue
// --------------------------------------------------------------------------
#endif//B_EPSILON_LRUQUEUE_H
