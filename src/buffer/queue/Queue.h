#ifndef B_EPSILON_QUEUE_H
#define B_EPSILON_QUEUE_H
// --------------------------------------------------------------------------
#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <iterator>
// --------------------------------------------------------------------------
namespace buffer::queue {
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
template<class K, class V>
using Entry = std::pair<K, V>;
// --------------------------------------------------------------------------
}//namespace
// --------------------------------------------------------------------------
template<class K, class V>
class Queue {

protected:
    const std::size_t maxEntries;
    std::list<Entry<K, V>> entryQueue;
    std::unordered_map<K, typename std::list<Entry<K, V>>::iterator> pointerMap;

public:
    explicit Queue(std::size_t);
    virtual ~Queue() = default;

public:
    const std::list<Entry<K, V>>& getList() const;
    std::optional<Entry<K, V>> insert(K, V, std::function<bool(const V&)>);
    bool contains(const K&) const;
    virtual V& find(const K&) = 0;
};
// --------------------------------------------------------------------------
template<class K, class V>
Queue<K, V>::Queue(std::size_t maxEntries) : maxEntries(maxEntries) {
}
// --------------------------------------------------------------------------
template<class K, class V>
const std::list<Entry<K, V>>& Queue<K, V>::getList() const {
    return entryQueue;
}
// --------------------------------------------------------------------------
template<class K, class V>
std::optional<Entry<K, V>> Queue<K, V>::insert(K key, V value,
                                                   std::function<bool(const V&)> predicate) {
    if(contains(key)){
        throw std::runtime_error("Key was already in the queue!");
    }
    std::optional<Entry<K, V>> removedEntry = std::nullopt;
    assert(pointerMap.size() <= maxEntries);
    if(pointerMap.size() == maxEntries){
        // queue -> iterate until we find a fitting entry to remove
        for(auto it = entryQueue.rbegin(); it != entryQueue.rend(); ++it){
            if(predicate(it->second)){
                // remove the entry
                removedEntry = std::move(*it);
                entryQueue.erase(std::next(it).base());
                pointerMap.erase(removedEntry->first);
                break;
            }
        }
        if(!removedEntry){
            throw std::runtime_error("Queue is full! (FIFO)");
        }
    }
    // inserted values are at the front
    entryQueue.push_front(std::make_pair(key, std::move(value)));
    pointerMap[key] = entryQueue.begin();
    return removedEntry;
}
// --------------------------------------------------------------------------
template<class K, class V>
bool Queue<K, V>::contains(const K& key) const {
    return pointerMap.find(key) != pointerMap.end();
}
// --------------------------------------------------------------------------
}// namespace buffer::queue
// --------------------------------------------------------------------------
#endif//B_EPSILON_QUEUE_H
