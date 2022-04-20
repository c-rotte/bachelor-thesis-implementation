#ifndef B_EPSILON_LOCKSET_H
#define B_EPSILON_LOCKSET_H
// --------------------------------------------------------------------------
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
// --------------------------------------------------------------------------
namespace buffer::lock {
// --------------------------------------------------------------------------
template<class K>
class LockSet {

    struct Lock {
        std::size_t references = 0;
        std::shared_mutex mutex;
    };

private:
    std::unordered_map<K, std::unique_ptr<Lock>> mutexMap;
    mutable std::shared_mutex mutex;

public:
    LockSet() = default;

private:
    // removes the lock if it is not referenced anymore
    void checkForCleanup(const K&);

public:
    // locks K (announce a write operation)
    void lock(const K&);
    // unlock K (the write operation has finished)
    void unlock(const K&);
    // wait for the announced operation to finish
    void wait(const K&);
};
// --------------------------------------------------------------------------
template<class K>
void LockSet<K>::checkForCleanup(const K& key) {
    Lock& lock = *mutexMap[key];
    if (lock.references == 0) {
        mutexMap.erase(key);
    }
}
// --------------------------------------------------------------------------
template<class K>
void LockSet<K>::lock(const K& key) {
    std::unique_lock mapLock(mutex);
    if (mutexMap.find(key) == mutexMap.end()) {
        mutexMap[key] = std::make_unique<Lock>();
    }
    Lock& lock = *mutexMap[key];
    ++lock.references;
    mapLock.unlock();
    lock.mutex.lock();
}
// --------------------------------------------------------------------------
template<class K>
void LockSet<K>::unlock(const K& key) {
    std::unique_lock mapLock(mutex);
    if (mutexMap.find(key) == mutexMap.end()) {
        throw std::runtime_error("Key was not found!");
    }
    Lock& lock = *mutexMap[key];
    lock.mutex.unlock();
    --lock.references;
    checkForCleanup(key);
}
// --------------------------------------------------------------------------
template<class K>
void LockSet<K>::wait(const K& key) {
    {
        std::shared_lock mapLock(mutex);
        if (mutexMap.find(key) == mutexMap.end()) {
            return;
        }
        Lock& lock = *mutexMap[key];
        ++lock.references;
        mapLock.unlock();
        {
            std::shared_lock waitingLock(lock.mutex);
        }
    }
    std::unique_lock mapLock(mutex);
    if (mutexMap.find(key) == mutexMap.end()) {
        throw std::runtime_error("Key was not found!");
    }
    Lock& lock = *mutexMap[key];
    --lock.references;
    checkForCleanup(key);
}
// --------------------------------------------------------------------------
}// namespace buffer::lock
// --------------------------------------------------------------------------
#endif//B_EPSILON_LOCKSET_H
