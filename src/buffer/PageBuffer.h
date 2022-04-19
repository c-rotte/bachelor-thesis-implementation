#ifndef B_EPSILON_PAGEBUFFER_H
#define B_EPSILON_PAGEBUFFER_H
// --------------------------------------------------------------------------
#include "queue/FIFOQueue.h"
#include "queue/LRUQueue.h"
#include "src/file/SegmentManager.h"
#include <array>
#include <atomic>
#include <cinttypes>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_set>
// --------------------------------------------------------------------------
namespace buffer {
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
class PageBuffer;
// --------------------------------------------------------------------------
template<std::size_t B>
struct Page {
    alignas(alignof(std::max_align_t)) std::array<unsigned char, B> data = {};
    std::uint64_t id;

private:
    std::shared_mutex mutex;
    std::size_t pins = 0;// protects the page from eviction
    bool dirty = false;
    // just the buffer can access the metadata
    template<std::size_t BLOCK, std::size_t N>
    friend class PageBuffer;
};
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
class PageBuffer {

private:
    file::SegmentManager<B> segmentManager;
    std::array<Page<B>, N> pages;
    std::unordered_set<std::size_t> freeSlots;

    queue::FIFOQueue<std::uint64_t, std::size_t> fifoQueue;// id -> index
    queue::LRUQueue<std::uint64_t, std::size_t> lruQueue;  // id -> index
    mutable std::shared_mutex queueMutex;

public:
    PageBuffer() = delete;
    PageBuffer(const std::string&, double);
    PageBuffer(const PageBuffer<B, N>&) = delete;
    PageBuffer(PageBuffer<B, N>&&) noexcept = default;

private:
    void loadPage(std::uint64_t, std::size_t);
    void savePage(std::size_t);

public:
    std::uint64_t createPage();
    void deletePage(std::uint64_t);// assumes that the page is not pinned
    Page<B>& pinPage(std::uint64_t, bool);
    void unpinPage(std::uint64_t, bool);

    void flush();// not thread-safe

    PageBuffer<B, N>& operator=(const PageBuffer<B, N>&) = delete;
    PageBuffer<B, N>& operator=(PageBuffer<B, N>&&) noexcept = default;
};
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
PageBuffer<B, N>::PageBuffer(const std::string& path, double growthFactor)
    : segmentManager(path, growthFactor), pages({}) {
    for (std::size_t index = 0; index < N; index++) {
        freeSlots.insert(index);
    }
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::loadPage(std::uint64_t id, std::size_t index) {
    auto& page = pages[index];
    page.data = std::move(segmentManager.readBlock(id));// IO read
    page.id = id;
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::savePage(std::size_t index) {
    auto& page = pages[index];
    if (!page.dirty) {
        // page is not dirty -> nothing to do
        return;
    }
    // don't move the array to keep the page in memory valid
    segmentManager.writeBlock(page.id, page.data);// IO write
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
std::uint64_t PageBuffer<B, N>::createPage() {
    return segmentManager.createBlock();// locked segment + potential IO write
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::deletePage(std::uint64_t id) {
    std::size_t pageIndex;
    {
        // lock the queue
        std::unique_lock queueLock(queueMutex);
        if (lruQueue.contains(id)) {
            assert(!fifoQueue.contains(id));
            // page is in memory (LRU)
            pageIndex = lruQueue.find(id, false);
            assert(pages[pageIndex].pins == 0);
            lruQueue.remove(id);
            // mark the index as free
            assert(!freeSlots.contains(pageIndex));
            freeSlots.insert(pageIndex);
        } else if (fifoQueue.contains(id)) {
            // page is in memory (FIFO)
            pageIndex = fifoQueue.find(id);
            assert(pages[pageIndex].pins == 0);
            fifoQueue.remove(id);
            // mark the index as free
            assert(!freeSlots.contains(pageIndex));
            freeSlots.insert(pageIndex);
        }
    }
    // delete the page
    segmentManager.deleteBlock(id);// locked segment + IO read
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
Page<B>& PageBuffer<B, N>::pinPage(std::uint64_t id, bool exclusive) {
    // lock the queue
    std::unique_lock queueLock(queueMutex);
    assert(lruQueue.size() + fifoQueue.size() <= N);
    if (lruQueue.contains(id)) {
        assert(!fifoQueue.contains(id));
        // page is in memory (LRU)
        std::size_t pageIndex = lruQueue.find(id, true);
        auto& page = pages[pageIndex];
        ++page.pins;
        // unlock the queue
        queueLock.unlock();
        // lock the page
        if (exclusive) {
            page.mutex.lock();
        } else {
            page.mutex.lock_shared();
        }
        return page;
    }
    if (fifoQueue.contains(id)) {
        // page is in memory (FIFO) -> move to LRU
        std::size_t pageIndex = fifoQueue.remove(id).second;
        lruQueue.insert(id, pageIndex);
        auto& page = pages[pageIndex];
        ++page.pins;
        // unlock the queue
        queueLock.unlock();
        // lock the page
        if (exclusive) {
            page.mutex.lock();
        } else {
            page.mutex.lock_shared();
        }
        return page;
    }
    if (lruQueue.size() + fifoQueue.size() < N) {
        assert(!freeSlots.empty());
        // buffer is not full -> get a free index and load it into the FIFO queue
        std::size_t freeIndex = *freeSlots.begin();
        freeSlots.erase(freeIndex);
        fifoQueue.insert(id, freeIndex);
        auto& page = pages[freeIndex];
        assert(page.pins == 0);
        page.pins = 1;// set page to pinned
        {
            // lock the page (instant)
            std::unique_lock pageLock(page.mutex);
            // unlock the queue
            queueLock.unlock();
            // load the page + unlock
            loadPage(id, freeIndex);// IO read
            // unlock the page
        }
        // lock the page again
        if (exclusive) {
            page.mutex.lock();
        } else {
            page.mutex.lock_shared();
        }
        return page;
    }
    {
        assert(lruQueue.size() + fifoQueue.size() == N);
        // buffer is full -> find a free slot in the FIFO queue
        auto removedEntry = fifoQueue.removeOne([this](const std::size_t& index) {
            return pages[index].pins == 0;
        });
        if (removedEntry) {
            std::size_t pageIndex = removedEntry->second;
            // store the index in the FIFO queue
            fifoQueue.insert(id, pageIndex);
            // load the new one
            auto& page = pages[pageIndex];
            assert(page.pins == 0);
            page.pins = 1;// set page to pinned
            {
                // lock the page (instant)
                std::unique_lock pageLock(page.mutex);
                // lock the saved block
                segmentManager.lockBlock(removedEntry->first);
                // unlock the queue
                queueLock.unlock();
                // evict the old page and load the new one
                savePage(pageIndex);    // potential IO write
                segmentManager.unlockBlock(removedEntry->first);
                loadPage(id, pageIndex);// IO read
                // unlock the page
            }
            // lock the page again
            if (exclusive) {
                page.mutex.lock();
            } else {
                page.mutex.lock_shared();
            }
            return page;
        }
    }
    {
        assert(lruQueue.size() + fifoQueue.size() == N);
        // buffer is full -> find a free slot in the LRU queue
        auto removedEntry = lruQueue.removeOne([this](const std::size_t& index) {
            return pages[index].pins == 0;
        });
        if (removedEntry) {
            std::size_t pageIndex = removedEntry->second;
            // store the index in the FIFO queue
            fifoQueue.insert(id, pageIndex);
            // load the new one
            auto& page = pages[pageIndex];
            assert(page.pins == 0);
            page.pins = 1;// set page to pinned
            {
                // lock the page (instant)
                std::unique_lock pageLock(page.mutex);
                // lock the saved block
                segmentManager.lockBlock(removedEntry->first);
                // unlock the queue
                queueLock.unlock();
                // evict the old page and load the new one
                savePage(pageIndex);    // potential IO write
                segmentManager.unlockBlock(removedEntry->first);
                loadPage(id, pageIndex);// IO read
                // unlock the page
            }
            // lock the page again
            if (exclusive) {
                page.mutex.lock();
            } else {
                page.mutex.lock_shared();
            }
            return page;
        }
    }
    throw std::runtime_error("Buffer is full!");// unlock the queue
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::unpinPage(std::uint64_t id, bool dirty) {
    // lock the queue
    std::unique_lock queueLock(queueMutex);
    if (fifoQueue.contains(id)) {
        std::size_t index = fifoQueue.find(id);
        auto& page = pages[index];
        page.mutex.unlock();// release the page lock
        if (dirty) {
            page.dirty = dirty;// set page to dirty
        }
        assert(page.pins >= 1);
        --page.pins;// set page to unpinned
        return;     // unlock the queue
    }
    if (lruQueue.contains(id)) {
        std::size_t index = lruQueue.find(id, false);
        auto& page = pages[index];
        page.mutex.unlock();// release the page lock
        if (dirty) {
            page.dirty = dirty;// set page to dirty
        }
        assert(page.pins >= 1);
        --page.pins;// set page to unpinned
        return;     // unlock the queue
    }
    throw std::runtime_error("Invalid page id!");// unlock the queue
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::flush() {
    for (std::size_t index = 0; index < N; index++) {
        if (!freeSlots.contains(index)) {
            savePage(index);
        }
    }
    segmentManager.flush();
}
// --------------------------------------------------------------------------
}// namespace buffer
// --------------------------------------------------------------------------
#endif//B_EPSILON_PAGEBUFFER_H
