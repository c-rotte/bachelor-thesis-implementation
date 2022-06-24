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

    struct alignas(alignof(std::max_align_t)) Header {
        std::atomic_size_t idCounter = 0;
    };
    static_assert(sizeof(Header) <= B);

private:
    int fd;
    Header header;
    // pages loaded into memory
    std::array<Page<B>, N> pages;

public:
    PageBuffer() = delete;
    PageBuffer(const std::string&, double);
    PageBuffer(const PageBuffer<B, N>&) = delete;
    PageBuffer(PageBuffer<B, N>&&) noexcept = default;

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
PageBuffer<B, N>::PageBuffer(const std::string& path, double) : pages() {
    std::string filePath = path + "/pages";
    if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(path)) {
        std::size_t fileSize = std::filesystem::file_size(filePath);
        if (fileSize == 0 || fileSize != B + B * N) {
            util::raise("Invalid file size.");
        }
        fd = open(filePath.c_str(), O_RDWR);
        if (pread(fd, pages.data(), B * N, 0) != B * N) {
            util::raise("Invalid file!");
        }
    } else {
        std::filesystem::create_directories(path);
        fd = open(filePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            util::raise("Could not create the buffer file!");
        }
        header.idCounter = 0;
        if (ftruncate(fd, B + B * N) < 0) {
            util::raise("Could not increase the file size (buffer file).");
        }
    }
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
std::uint64_t PageBuffer<B, N>::createPage() {
    std::size_t id = ++header.idCounter;
    if (id >= pages.size()) {
        util::raise("Full Buffer!");
    }
    pages[id].id = id;
    return id;
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
Page<B>& PageBuffer<B, N>::pinPage(std::uint64_t id, bool exclusive,
                                   bool, std::optional<ModeFunction> modeFunction) {
    if (id >= pages.size()) {
        util::raise("Invalid id!");
    }
    auto& page = pages[id];
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
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::unpinPage(Page<B>& page, bool) {
    page.mutex.unlock();
}
// --------------------------------------------------------------------------
template<std::size_t B, std::size_t N>
void PageBuffer<B, N>::flush() {
    std::cout << "flushing " << header.idCounter << " pages" << std::endl;
    if (pwrite(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
        util::raise("Could not save the header (buffer file).");
    }
    if (pwrite(fd, pages.data(), B * header.idCounter, 0) != B * header.idCounter) {
        util::raise("Could not save the pages (buffer file).");
    }
}
// --------------------------------------------------------------------------
}// namespace buffer
// --------------------------------------------------------------------------
#endif//B_EPSILON_PAGEBUFFER_H
