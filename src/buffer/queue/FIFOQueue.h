#ifndef B_EPSILON_FIFOQUEUE_H
#define B_EPSILON_FIFOQUEUE_H
// --------------------------------------------------------------------------
#include "Queue.h"
#include <utility>
#include <stdexcept>
#include <cassert>
// --------------------------------------------------------------------------
namespace buffer::queue {
// --------------------------------------------------------------------------
template<class K, class V>
class FIFOQueue : public Queue<K, V> {

public:
    explicit FIFOQueue(std::size_t);

    virtual V& find(const K&) override;
};
// --------------------------------------------------------------------------
template<class K, class V>
FIFOQueue<K, V>::FIFOQueue(std::size_t maxEntries) : Queue<K, V>(maxEntries) {
}
// --------------------------------------------------------------------------
template<class K, class V>
V& FIFOQueue<K, V>::find(const K& key) {
    if(!this->contains(key)){
        throw std::runtime_error("Key was not found! (FIFO)");
    }
    return this->pointerMap[key]->second;
}
// --------------------------------------------------------------------------
}//namespace buffer::queue
// --------------------------------------------------------------------------
#endif//B_EPSILON_FIFOQUEUE_H
