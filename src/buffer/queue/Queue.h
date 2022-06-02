#ifndef B_EPSILON_QUEUE_H
#define B_EPSILON_QUEUE_H
// --------------------------------------------------------------------------
#include <cstddef>
#include <functional>
#include <iterator>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
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
    std::list<Entry<K, V>> entryQueue;
    std::unordered_map<K, typename std::list<Entry<K, V>>::iterator> pointerMap;

public:
    Queue() = default;
    virtual ~Queue() = default;

public:
    std::size_t size() const;
    const std::list<Entry<K, V>>& getList() const;
    // inserts Entry(K, V)
    void insert(K, V);
    Entry<K, V> remove(const K&);
    std::optional<K> findOne(std::function<bool(const V&)>);
    bool contains(const K&) const;
    virtual V& find(const K&, bool) = 0;
};
// --------------------------------------------------------------------------
template<class K, class V>
std::size_t Queue<K, V>::size() const {
    return pointerMap.size();
}
// --------------------------------------------------------------------------
template<class K, class V>
const std::list<Entry<K, V>>& Queue<K, V>::getList() const {
    return entryQueue;
}
// --------------------------------------------------------------------------
template<class K, class V>
void Queue<K, V>::insert(K key, V value) {
    // inserted values are at the front
    entryQueue.push_front(std::make_pair(key, std::move(value)));
    pointerMap[key] = entryQueue.begin();
}
// --------------------------------------------------------------------------
template<class K, class V>
Entry<K, V> Queue<K, V>::remove(const K& key) {
    auto it = pointerMap[key];
    Entry<K, V> removedEntry = std::move(*it);
    entryQueue.erase(it);
    pointerMap.erase(key);
    return removedEntry;
}
// --------------------------------------------------------------------------
template<class K, class V>
std::optional<K> Queue<K, V>::findOne(std::function<bool(const V&)> predicate) {
    std::optional<K> result = std::nullopt;
    // queue -> iterate until we find a fitting entry to remove
    for (auto it = entryQueue.rbegin(); it != entryQueue.rend(); ++it) {
        if (predicate(it->second)) {
            // remove the entry
            result = it->first;
            break;
        }
    }
    return result;
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
