#ifndef B_EPSILON_SEGMENTMANAGER_H
#define B_EPSILON_SEGMENTMANAGER_H
// --------------------------------------------------------------------------
#include "FileManager.h"
#include <cinttypes>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
// --------------------------------------------------------------------------
namespace file {
// --------------------------------------------------------------------------
template<std::size_t B>
class SegmentManager {
    // WARNING: Concurrent Reads + Writes to the same block are not thread-safe

    struct alignas(alignof(std::max_align_t)) Header {
        std::uint64_t numberOfSegments;
        std::size_t lastAllocatedBlocks;
    };

    struct Segment {
        const std::string filePath;
        const std::size_t allocatedBlocks;

    private:
        std::mutex mutex;
        std::optional<FileManager<B>> fileManager;

    public:
        Segment(const std::string&, std::size_t = 0);
        void accessFileManager(std::function<void(std::optional<FileManager<B>>&, std::mutex&)>);
    };

    // wrap the segment into a unique pointer to keep
    // references valid after resizing the vector

private:
    std::string dirPath;
    int fd;
    Header header;
    std::vector<std::unique_ptr<Segment>> segments;
    std::unordered_set<std::size_t> freeSegments;
    const double growthFactor;
    mutable std::shared_mutex mutex;// mutex for block creation and deletion (freelist)

public:
    SegmentManager() = delete;
    SegmentManager(const std::string&, double);
    SegmentManager(const SegmentManager<B>&) = delete;
    SegmentManager(SegmentManager<B>&&) noexcept = default;

private:
    std::size_t getIndexFromID(std::uint64_t) const;
    std::size_t getBlockFromID(std::uint64_t) const;

public:
    std::uint64_t createBlock();
    void deleteBlock(std::uint64_t);
    void lockBlock(std::uint64_t);
    void unlockBlock(std::uint64_t);
    std::array<unsigned char, B> readBlock(std::uint64_t);
    void writeBlock(std::uint64_t, std::array<unsigned char, B>);

    void flush();// not thread-safe

    SegmentManager<B>& operator=(const SegmentManager<B>&) = delete;
    SegmentManager<B>& operator=(SegmentManager<B>&&) noexcept = default;
};
// --------------------------------------------------------------------------
template<std::size_t B>
SegmentManager<B>::Segment::Segment(const std::string& filePath, std::size_t allocatedBlocks)
    : filePath(filePath), allocatedBlocks(allocatedBlocks), fileManager(std::nullopt) {
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::Segment::accessFileManager(std::function<void(std::optional<FileManager<B>>&, std::mutex&)> func) {
    // the caller is responsible for initializing + locking the fileManager
    func(fileManager, mutex);
}
// --------------------------------------------------------------------------
template<std::size_t B>
SegmentManager<B>::SegmentManager(const std::string& dirPath, double growthFactor)
    : dirPath(dirPath), growthFactor(growthFactor) {
    const std::string headerFile = dirPath + "/segments";
    if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath) &&
        std::filesystem::exists(headerFile) && std::filesystem::is_regular_file(headerFile)) {
        fd = open(headerFile.c_str(), O_RDWR);
        if (pread(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
            throw std::runtime_error("Invalid segment file!");
        }
        for (std::size_t i = 0; i < header.numberOfSegments; i++) {
            auto segmentPtr = std::make_unique<Segment>(dirPath + "/" + std::to_string(i));
            auto& segment = *segmentPtr;
            segments.push_back(std::move(segmentPtr));
            // note: initializes the fileManager
            segment.accessFileManager([this, &segment, i](std::optional<FileManager<B>>& fileManager, std::mutex&) {
                // no need to lock since we are still in a single thread (constructor)
                // initialize the file manager
                fileManager = FileManager<B>(segment.filePath, segment.allocatedBlocks);
                if (fileManager->freeBlocks() > 0) {
                    freeSegments.insert(i);
                }
            });
        }
    } else {
        std::filesystem::create_directory(dirPath);
        fd = open(headerFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            throw std::runtime_error("Could not create the segment file!");
        }
        header = {0, 0};
        if (ftruncate(fd, sizeof(Header)) < 0) {
            throw std::runtime_error("Could not increase the file size (segment file).");
        }
    }
}
// --------------------------------------------------------------------------
template<std::size_t B>
std::size_t SegmentManager<B>::getIndexFromID(std::uint64_t id) const {
    return id >> 48;
}
// --------------------------------------------------------------------------
template<std::size_t B>
std::size_t SegmentManager<B>::getBlockFromID(std::uint64_t id) const {
    return id & 0xffffffffffff;
}
// --------------------------------------------------------------------------
template<std::size_t B>
std::uint64_t SegmentManager<B>::createBlock() {
    // lock the segment manager
    std::unique_lock mainLock(mutex);
    if (segments.empty() || freeSegments.empty()) {
        // here, the file managers are created
        if (segments.empty()) {
            assert(header.numberOfSegments == 0);
        }
        assert(freeSegments.empty());
        header.lastAllocatedBlocks = std::max<std::size_t>(10, header.lastAllocatedBlocks * growthFactor);
        auto segmentPtr = std::make_unique<Segment>(dirPath + "/" + std::to_string(header.numberOfSegments), header.lastAllocatedBlocks);
        auto& segment = *segmentPtr;
        segments.push_back(std::move(segmentPtr));
        const std::size_t segmentIndex = segments.size() - 1;
        header.numberOfSegments++;
        // now access the segment
        std::size_t blockID;
        segment.accessFileManager([this, &segment, &mainLock, &blockID, segmentIndex](
                                          std::optional<FileManager<B>>& fileManager, std::mutex& segmentMutex) {
            assert(!fileManager);
            // lock the segment
            std::scoped_lock segmentLock(segmentMutex);
            // mark the segment as free
            freeSegments.insert(segmentIndex);// only operation which needs both locks
            // unlock the segment manager
            mainLock.unlock();
            // initialize the fileManager
            fileManager = FileManager<B>(segment.filePath, segment.allocatedBlocks);
            // create the blockID
            blockID = fileManager->createBlock();
            // unlock the segment
        });
        return (segmentIndex << 48) | blockID;
    }
    const std::size_t segmentIndex = *freeSegments.begin();
    auto& segment = *segments.at(segmentIndex);
    std::size_t blockID;
    segment.accessFileManager([this, &mainLock, segmentIndex, &blockID](
                                      std::optional<FileManager<B>>& fileManager, std::mutex& segmentMutex) {
        // lock the segment
        std::scoped_lock segmentLock(segmentMutex);
        assert(fileManager);
        assert(fileManager->freeBlocks() > 0);
        if (fileManager->freeBlocks() == 1) {
            // mark the segment as full
            freeSegments.erase(segmentIndex);// only operation which needs both locks
        }
        // unlock the segment manager
        mainLock.unlock();
        // create the blockID
        blockID = fileManager->createBlock();
        // unlock the segment
    });
    return (segmentIndex << 48) | blockID;
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::deleteBlock(std::uint64_t id) {
    const std::size_t segmentIndex = getIndexFromID(id);
    const std::size_t blockID = getBlockFromID(id);
    // lock the segment manager
    std::unique_lock mainLock(mutex);
    auto& segment = segments.at(segmentIndex);
    // access the segment
    segment->accessFileManager([this, &mainLock, blockID, segmentIndex](
                                       std::optional<FileManager<B>>& fileManager, std::mutex& segmentMutex) {
        // lock the segment
        std::scoped_lock segmentLock(segmentMutex);
        assert(fileManager);
        // mark the segment as free
        freeSegments.insert(segmentIndex);// only operation which needs both locks
        // unlock the segment manager
        mainLock.unlock();
        // IO write
        fileManager->deleteBlock(blockID);
        // unlock the segment
    });
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::lockBlock(std::uint64_t id) {
    const std::size_t segmentIndex = getIndexFromID(id);
    const std::size_t blockID = getBlockFromID(id);
    // lock the segment manager
    std::shared_lock mainLock(mutex);
    auto& segment = *segments.at(segmentIndex);
    // unlock the segment manager
    mainLock.unlock();
    segment.accessFileManager([blockID](
                                      std::optional<FileManager<B>>& fileManager, std::mutex& segmentMutex) {
        if (!fileManager) {
            // the fileManager has not been initialized -> wait until it has
            std::unique_lock segmentLock(segmentMutex);
        }
        assert(fileManager);
        fileManager->lockBlock(blockID);
    });
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::unlockBlock(std::uint64_t id) {
    const std::size_t segmentIndex = getIndexFromID(id);
    const std::size_t blockID = getBlockFromID(id);
    // lock the segment manager
    std::shared_lock mainLock(mutex);
    auto& segment = *segments.at(segmentIndex);
    // unlock the segment manager
    mainLock.unlock();
    segment.accessFileManager([blockID](
                                      std::optional<FileManager<B>>& fileManager, std::mutex& segmentMutex) {
        if (!fileManager) {
            // the fileManager has not been initialized -> wait until it has
            std::unique_lock segmentLock(segmentMutex);
        }
        assert(fileManager);
        fileManager->unlockBlock(blockID);
    });
}
// --------------------------------------------------------------------------
template<std::size_t B>
std::array<unsigned char, B> SegmentManager<B>::readBlock(std::uint64_t id) {
    const std::size_t segmentIndex = getIndexFromID(id);
    const std::size_t blockID = getBlockFromID(id);
    // lock the segment manager
    std::shared_lock mainLock(mutex);
    auto& segment = *segments.at(segmentIndex);
    // unlock the segment manager
    mainLock.unlock();
    std::optional<std::array<unsigned char, B>> result;
    segment.accessFileManager([this, &mainLock, &result, blockID](
                                      std::optional<FileManager<B>>& fileManager, std::mutex& segmentMutex) {
        // TODO: make segmentMutex shared
        if (!fileManager) {
            // the fileManager has not been initialized -> wait until it has
            std::unique_lock segmentLock(segmentMutex);
        }
        assert(fileManager);
        result = fileManager->readBlock(blockID);
    });
    return std::move(*result);
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::writeBlock(std::uint64_t id, std::array<unsigned char, B> data) {
    const std::size_t segmentIndex = getIndexFromID(id);
    const std::size_t blockID = getBlockFromID(id);
    // lock the segment manager
    std::shared_lock mainLock(mutex);
    auto& segment = *segments.at(segmentIndex);
    // unlock the segment manager
    mainLock.unlock();
    segment.accessFileManager([this, &mainLock, data = std::move(data), blockID](
                                      std::optional<FileManager<B>>& fileManager, std::mutex& segmentMutex) {
        if (!fileManager) {
            // the fileManager has not been initialized -> wait until it has
            std::unique_lock segmentLock(segmentMutex);
        }
        assert(fileManager);
        fileManager->writeBlock(blockID, std::move(data));
    });
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::flush() {
    for (auto& segment: segments) {
        segment->accessFileManager([](std::optional<FileManager<B>>& fileManager, std::mutex&) {
            assert(fileManager);
            fileManager->flush();
        });
    }
    if (pwrite(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
        throw std::runtime_error("Could not save the header (segment file).");
    }
}
// --------------------------------------------------------------------------
}// namespace file
// --------------------------------------------------------------------------
#endif//B_EPSILON_SEGMENTMANAGER_H
