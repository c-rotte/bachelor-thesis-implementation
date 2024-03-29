#ifndef B_EPSILON_FIFOQUEUE_H
#define B_EPSILON_FIFOQUEUE_H
// --------------------------------------------------------------------------
#include "Queue.h"
#include "src/util/ErrorHandler.h"
#include <cassert>
#include <utility>
// --------------------------------------------------------------------------
namespace buffer::queue {
// --------------------------------------------------------------------------
template<class K, class V>
class FIFOQueue : public Queue<K, V> {

public:
    FIFOQueue() = default;

    virtual V& find(const K&, bool = false) override;
};
// --------------------------------------------------------------------------
template<class K, class V>
V& FIFOQueue<K, V>::find(const K& key, bool) {
    if (!this->contains(key)) {
        util::raise("Key was not found! (FIFO)");
    }
    return this->pointerMap[key]->second;
}
// --------------------------------------------------------------------------
}//namespace buffer::queue
// --------------------------------------------------------------------------
#endif//B_EPSILON_FIFOQUEUE_H
