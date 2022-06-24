#ifndef B_EPSILON_PAGEBUFFER_H
#define B_EPSILON_PAGEBUFFER_H
// --------------------------------------------------------------------------
#include "queue/FIFOQueue.h"
#include "queue/LRUQueue.h"
#include "src/file/SegmentManager.h"
#include "src/util/ErrorHandler.h"
#include <array>
#include <atomic>
#include <cinttypes>
#include <cstddef>
#include <functional>
#include <iostream>
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
    std::uint64_t id = -1;

private:
    std::shared_mutex mutex;
    std::atomic_size_t pins = 0;// protects the page from eviction
    std::atomic_bool dirty = false;
    // just the buffer can access the metadata
    template<std::size_t BLOCK, std::size_t N>
    friend class PageBuffer;
};
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
class PageBuffer {

private:
    file::SegmentManager<B> segmentManager;
    // pages loaded into memory
    std::array<Page<B>, N> pages;
    // pageTable contains all currently loaded pages
    std::unordered_map<std::uint64_t, std::size_t> pageTable;// id -> index
    std::unordered_set<std::size_t> freeSlots;
    // the 2Q only handle pages with zero pins
    queue::FIFOQueue<std::uint64_t, std::size_t> fifoQueue;// id -> index
    queue::LRUQueue<std::uint64_t, std::size_t> lruQueue;  // id -> index
    mutable std::shared_mutex tableMutex;

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
    // if ModeFunction != nullptr, exclusive is ignored
    using ModeFunction = std::function<bool(Page<B>&)>;
    Page<B>& pinPage(std::uint64_t, bool, bool = false,
                     std::optional<ModeFunction> = std::nullopt);
    void unpinPage(Page<B>&, bool);

    void flush();// not thread-safe

    PageBuffer<B, N>& operator=(const PageBuffer<B, N>&) = delete;
    PageBuffer<B, N>& operator=(PageBuffer<B, N>&&) noexcept = default;
};
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
PageBuffer<B, N>::PageBuffer(const std::string& path, double growthFactor)
    : segmentManager(path, growthFactor), pages() {
    for (std::size_t index = 0; index < N; index++) {
        freeSlots.insert(index);
    }
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::loadPage(std::uint64_t id, std::size_t index) {
    auto& page = pages[index];
    page.data = std::move(segmentManager.readBlock(id));// IO read
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::savePage(std::size_t index) {
    auto& page = pages[index];
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
Page<B>& PageBuffer<B, N>::pinPage(std::uint64_t id, bool exclusive,
                                   bool skipLoad, std::optional<ModeFunction> modeFunction) {
    const auto lockPageTable = [this](bool exclusivePageTableLock) {
        if (exclusivePageTableLock) {
            tableMutex.lock();
        } else {
            tableMutex.lock_shared();
        }
    };
    const auto unlockPageTable = [this](bool exclusivePageTableLock) {
        if (exclusivePageTableLock) {
            tableMutex.unlock();
        } else {
            tableMutex.unlock_shared();
        }
    };
    bool exclusivePageTableLock = false;
    do {
        // lock the pageTable
        lockPageTable(exclusivePageTableLock);
        // 1) the page is already in memory
        auto pageIt = pageTable.find(id);
        if (pageIt != pageTable.end()) {
            auto pagePair = *pageIt;
            // found the page -> pin it
            auto& page = pages[pagePair.second];
            std::size_t pins = ++page.pins;
            // unlock the page table
            assert(pins >= 1);
            if (pins == 1) {// 0 -> 1: update position in 2Q
                if (!exclusivePageTableLock) {
                    --page.pins;
                    unlockPageTable(exclusivePageTableLock);
                    exclusivePageTableLock = true;
                    continue;
                }
                if (fifoQueue.contains(id)) {
                    fifoQueue.remove(id);
                    lruQueue.insert(id, pagePair.second);
                } else if (lruQueue.contains(id)) {
                    lruQueue.find(id, true);
                } else {
                    fifoQueue.insert(id, pagePair.second);
                }
            }
            unlockPageTable(exclusivePageTableLock);
            // lock the page
            if (modeFunction) {
                exclusive = (*modeFunction)(page);
            }
            if (exclusive) {
                page.mutex.lock();
            } else {
                page.mutex.lock_shared();
            }
            return page;
        }
        // 2.1) we still have space in memory
        if (pageTable.size() < pages.size()) {
            // the page needs to be loaded into memory
            if (!exclusivePageTableLock) {
                unlockPageTable(exclusivePageTableLock);
                exclusivePageTableLock = true;
                continue;
            }
            assert(!freeSlots.empty());
            assert(!pageTable.contains(id));
            std::size_t freeIndex = *freeSlots.begin();
            freeSlots.erase(freeIndex);
            pageTable[id] = freeIndex;
            auto& page = pages[freeIndex];
            assert(page.pins == 0);
            // set the metadata
            page.pins = 1;
            page.id = id;
            page.dirty = false;
            // add the page to the 2Q
            fifoQueue.insert(id, freeIndex);
            // load the page
            if (skipLoad) {
                // unlock the queue
                unlockPageTable(exclusivePageTableLock);
            } else {
                // lock the page (instant)
                std::unique_lock pageLock(page.mutex);
                // unlock the table
                unlockPageTable(exclusivePageTableLock);
                // load the page + unlock
                loadPage(id, freeIndex);// IO read
                // unlock the page
            }
            // lock the page again
            if (modeFunction) {
                exclusive = (*modeFunction)(page);
            }
            if (exclusive) {
                page.mutex.lock();
            } else {
                page.mutex.lock_shared();
            }
            return page;
        }
        // 2.2) we have to evict from the fifo queue
        {
            auto key = fifoQueue.findOne([this](const std::size_t& index) {
                return pages[index].pins == 0;
            });
            if (key) {
                std::size_t pageIndex = fifoQueue.find(*key, false);
                // load the page
                auto& page = pages[pageIndex];
                ++page.pins;// set page to pinned
                {
                    if (page.dirty) {
                        // mark the page as clean since we write it to disk
                        page.dirty = false;
                        // lock the page (instant)
                        std::shared_lock pageLock(page.mutex);
                        // unlock the queue
                        unlockPageTable(exclusivePageTableLock);
                        // evict the old page
                        savePage(pageIndex);// IO write
                        // unlock the page
                        pageLock.unlock();
                        // re-lock the table + queue
                        lockPageTable(true);
                    } else {
                        if (!exclusivePageTableLock) {
                            // unlock the queue
                            unlockPageTable(exclusivePageTableLock);
                            // re-lock
                            lockPageTable(true);
                        }
                    }
                }
                if (!pageTable.contains(id) &&
                    fifoQueue.contains(*key) &&
                    page.pins == 1 && !page.dirty) {
                    // the page was not accessed -> we can evict it and use it
                    pageTable.erase(*key);
                    fifoQueue.remove(*key);
                    // store the index in the page table
                    pageTable[id] = pageIndex;
                    fifoQueue.insert(id, pageIndex);
                    // set the metadata
                    page.id = id;
                    page.dirty = false;
                    if (skipLoad) {
                        // unlock the table + queue
                        unlockPageTable(true);
                    } else {
                        // lock the page (instant because we have the only pin)
                        std::unique_lock pageLock(page.mutex);
                        // unlock the table + queue
                        unlockPageTable(true);
                        // load the new page
                        loadPage(id, pageIndex);
                        // unlock the page
                        pageLock.unlock();
                    }
                    // lock the page again
                    if (modeFunction) {
                        exclusive = (*modeFunction)(page);
                    }
                    if (exclusive) {
                        page.mutex.lock();
                    } else {
                        page.mutex.lock_shared();
                    }
                    return page;
                }
                // we can't use this page
                --page.pins;
                // unlock the table + queue
                unlockPageTable(true);
                exclusivePageTableLock = false;
                continue;
            }
        }
        // 2.3) we have to evict from the lru queue
        {
            auto key = lruQueue.findOne([this](const std::size_t& index) {
                return pages[index].pins == 0;
            });
            if (key) {
                std::size_t pageIndex = lruQueue.find(*key, false);
                // load the page
                auto& page = pages[pageIndex];
                ++page.pins;// set page to pinned
                {
                    if (page.dirty) {
                        // mark the page as clean since we write it to disk
                        page.dirty = false;
                        // lock the page (instant)
                        std::shared_lock pageLock(page.mutex);
                        // unlock the queue
                        unlockPageTable(exclusivePageTableLock);
                        // evict the old page
                        savePage(pageIndex);// IO write
                        // unlock the page
                        pageLock.unlock();
                        // re-lock the table + queue
                        lockPageTable(true);
                    } else {
                        if (!exclusivePageTableLock) {
                            // unlock the queue
                            unlockPageTable(exclusivePageTableLock);
                            // re-lock
                            lockPageTable(true);
                        }
                    }
                }
                if (!pageTable.contains(id) &&
                    lruQueue.contains(*key) &&
                    page.pins == 1 && !page.dirty) {
                    // the page was not accessed -> we can evict it and use it
                    pageTable.erase(*key);
                    lruQueue.remove(*key);
                    // store the index in the page table
                    pageTable[id] = pageIndex;
                    fifoQueue.insert(id, pageIndex);
                    // set the metadata
                    page.id = id;
                    page.dirty = false;
                    if (skipLoad) {
                        // unlock the table + queue
                        unlockPageTable(true);
                    } else {
                        // lock the page (instant because we have the only pin)
                        std::unique_lock pageLock(page.mutex);
                        // unlock the table + queue
                        unlockPageTable(true);
                        // load the new page
                        loadPage(id, pageIndex);
                        // unlock the page
                        pageLock.unlock();
                    }
                    // lock the page again
                    if (modeFunction) {
                        exclusive = (*modeFunction)(page);
                    }
                    if (exclusive) {
                        page.mutex.lock();
                    } else {
                        page.mutex.lock_shared();
                    }
                    return page;
                }
                // we can't use this page
                --page.pins;
                // unlock the table + queue
                unlockPageTable(true);
                exclusivePageTableLock = false;
                continue;
            }
        }
        // no free slot was found -> abort
        util::raise("Buffer is full!");
    } while (true);
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::unpinPage(Page<B>& page, bool dirty) {
    assert(page.pins >= 1);
    if (dirty) {
        // set page to dirty
        page.dirty = true;
    }
    // release the page lock
    page.mutex.unlock();
    --page.pins;
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::flush() {
    for (std::size_t index = 0; index < N; index++) {
        if (!freeSlots.contains(index)) {
            auto& page = pages[index];
            if (page.dirty) {
                savePage(index);
            }
        }
    }
    segmentManager.flush();
}
// --------------------------------------------------------------------------
}// namespace buffer
// --------------------------------------------------------------------------
#endif//B_EPSILON_PAGEBUFFER_H
