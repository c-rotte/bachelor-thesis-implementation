#ifndef B_EPSILON_SEGMENTMANAGER_H
#define B_EPSILON_SEGMENTMANAGER_H
// --------------------------------------------------------------------------
#include "Segment.h"
#include <cinttypes>
#include <cstddef>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
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

    struct SegmentContainer {
        // wrap it into an optional to delay the io read
        std::optional<Segment<B>> segment;
        std::shared_mutex mutex;
    };

private:
    std::string dirPath;
    int fd;
    Header header;
    std::vector<std::unique_ptr<SegmentContainer>> segments;
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
    std::array<unsigned char, B> readBlock(std::uint64_t);
    void writeBlock(std::uint64_t, std::array<unsigned char, B>);

    std::size_t allocatedBlocks() const;
    void flush();// not thread-safe

    SegmentManager<B>& operator=(const SegmentManager<B>&) = delete;
    SegmentManager<B>& operator=(SegmentManager<B>&&) noexcept = default;
};
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
        std::size_t currentSegmentOffset = B;
        for (std::size_t i = 0; i < header.numberOfSegments; i++) {
            auto segmentContainerPtr = std::make_unique<SegmentContainer>();
            segmentContainerPtr->segment = std::make_optional<Segment<B>>(fd, currentSegmentOffset);
            if (segmentContainerPtr->segment->freeBlocks() > 0) {
                freeSegments.insert(i);
            }
            // increase the offset
            currentSegmentOffset = segmentContainerPtr->segment->getOffset() +
                                   segmentContainerPtr->segment->getTotalSize();
            segments.push_back(std::move(segmentContainerPtr));
        }
    } else {
        std::filesystem::create_directories(dirPath);
        fd = open(headerFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            throw std::runtime_error("Could not create the segment file!");
        }
        header = {0, 0};
        if (ftruncate(fd, B) < 0) {
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
        // here, the segments are created
        if (segments.empty()) {
            assert(header.numberOfSegments == 0);
        }
        assert(freeSegments.empty());
        header.lastAllocatedBlocks = std::max<std::size_t>(10, header.lastAllocatedBlocks * growthFactor);
        // we need these for the segment
        const std::size_t segmentOffset = (header.numberOfSegments == 0)
                                                  ? B
                                                  : segments.back()->segment->getOffset() +
                                                            segments.back()->segment->getTotalSize();
        const std::size_t segmentSize = header.lastAllocatedBlocks;
        auto segmentContainerPtr = std::make_unique<SegmentContainer>();
        segments.push_back(std::move(segmentContainerPtr));
        const std::size_t segmentIndex = segments.size() - 1;
        // mark the new segment as free
        freeSegments.insert(segmentIndex);
        header.numberOfSegments++;
        // now access and load the segment
        std::uint64_t blockID;
        {
            auto& segmentContainer = *segments.back();
            // first, make space for the new segment
            ftruncate(fd, segmentOffset + B + segmentSize * B);
            // lock the segment
            std::unique_lock segmentLock(segmentContainer.mutex);
            // unlock the segment manager
            mainLock.unlock();
            // initialize the segment
            segmentContainer.segment = std::move(Segment<B>(fd, segmentOffset, segmentSize));
            // create a new block
            blockID = segmentContainer.segment->createBlock();
            // unlock the segment
        }
        return (segmentIndex << 48) | blockID;
    }
    const std::size_t segmentIndex = *freeSegments.begin();
    auto& segmentContainer = *segments.at(segmentIndex);
    std::size_t blockID;
    // create a new block
    {
        // lock the segment
        std::unique_lock segmentLock(segmentContainer.mutex);
        assert(segmentContainer.segment->freeBlocks() > 0);
        if (segmentContainer.segment->freeBlocks() == 1) {
            // mark the segment as full
            freeSegments.erase(segmentIndex);// only operation which needs both locks
        }
        // unlock the segment manager
        mainLock.unlock();
        // create the blockID
        blockID = segmentContainer.segment->createBlock();
        // unlock the segment
    }
    return (segmentIndex << 48) | blockID;
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::deleteBlock(std::uint64_t id) {
    const std::size_t segmentIndex = getIndexFromID(id);
    const std::size_t blockID = getBlockFromID(id);
    // lock the segment manager
    std::unique_lock mainLock(mutex);
    auto& segmentContainer = *segments.at(segmentIndex);
    // access the segment
    {
        // lock the segment
        std::unique_lock segmentLock(segmentContainer.mutex);
        assert(segmentContainer.segment);
        // mark the segment as free
        freeSegments.insert(segmentIndex);// only operation which needs both locks
        // unlock the segment manager
        mainLock.unlock();
        // IO write
        segmentContainer.segment->deleteBlock(blockID);
        // unlock the segment
    }
}
// --------------------------------------------------------------------------
template<std::size_t B>
std::array<unsigned char, B> SegmentManager<B>::readBlock(std::uint64_t id) {
    const std::size_t segmentIndex = getIndexFromID(id);
    const std::size_t blockID = getBlockFromID(id);
    // lock the segment manager
    std::shared_lock mainLock(mutex);
    auto& segmentContainer = *segments.at(segmentIndex);
    // unlock the segment manager
    mainLock.unlock();
    std::array<unsigned char, B> result;
    {
        if (!segmentContainer.segment) {
            // the segment has not been initialized -> wait until it has
            {
                std::shared_lock segmentLock(segmentContainer.mutex);
            }
        }
        assert(segmentContainer.segment);
        result = std::move(segmentContainer.segment->readBlock(blockID));
    }
    return result;
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::writeBlock(std::uint64_t id, std::array<unsigned char, B> data) {
    const std::size_t segmentIndex = getIndexFromID(id);
    const std::size_t blockID = getBlockFromID(id);
    // lock the segment manager
    std::shared_lock mainLock(mutex);
    auto& segmentContainer = *segments.at(segmentIndex);
    // unlock the segment manager
    mainLock.unlock();
    std::optional<std::array<unsigned char, B>> result;
    {
        if (!segmentContainer.segment) {
            // the segment has not been initialized -> wait until it has
            {
                std::shared_lock segmentLock(segmentContainer.mutex);
            }
        }
        assert(segmentContainer.segment);
        segmentContainer.segment->writeBlock(blockID, std::move(data));
    }
}
// --------------------------------------------------------------------------
template<std::size_t B>
std::size_t SegmentManager<B>::allocatedBlocks() const {
    std::size_t result = 0;
    for (const auto& segmentContainerPtr: segments) {
        assert(segmentContainerPtr->segment);
        result += segmentContainerPtr->segment->allocatedBlocks();
    }
    return result;
}
// --------------------------------------------------------------------------
template<std::size_t B>
void SegmentManager<B>::flush() {
    for (auto& segmentContainerPtr: segments) {
        assert(segmentContainerPtr->segment);
        segmentContainerPtr->segment->flush();
    }
    if (pwrite(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
        throw std::runtime_error("Could not save the header (segment file).");
    }
}
// --------------------------------------------------------------------------
}// namespace file
// --------------------------------------------------------------------------
#endif//B_EPSILON_SEGMENTMANAGER_H
