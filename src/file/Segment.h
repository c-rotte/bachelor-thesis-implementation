#ifndef B_EPSILON_SEGMENT_H
#define B_EPSILON_SEGMENT_H
// --------------------------------------------------------------------------
#include "src/util/ErrorHandler.h"
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
// --------------------------------------------------------------------------
namespace file {
// --------------------------------------------------------------------------
template<std::size_t B>
class Segment {

    // make sure that the block is capable of storing a list pointer
    static_assert(B > sizeof(std::uint64_t));

    struct alignas(alignof(std::max_align_t)) Header {
        std::uint64_t blockSize = 0;
        std::uint64_t allocatedBlocks = 0;
        std::uint64_t occupiedBlocks = 0;
        std::uint64_t deletedBlocks = 0;
        std::optional<std::uint64_t> freeNodesHead;
    };
    // use B bytes for the header
    static_assert(sizeof(Header) <= B);

private:
    int fd;
    std::size_t fileOffset;// marks the begin of the header
    Header header;

public:
    Segment(int, std::size_t);               // construct an existing segment
    Segment(int, std::size_t, std::uint64_t);// construct a new segment
    Segment(const Segment<B>&) = delete;
    Segment(Segment<B>&&) noexcept = default;

private:
    std::uint64_t getIDInBlock(std::uint64_t);
    void storeIDInBlock(std::uint64_t, std::uint64_t);

public:
    constexpr std::size_t getBlockSize() const;
    std::size_t getOffset() const;
    std::size_t getTotalSize() const;

    std::uint64_t freeBlocks() const;
    std::uint64_t allocatedBlocks() const;

    std::uint64_t createBlock();
    void deleteBlock(std::uint64_t);
    std::array<unsigned char, B> readBlock(std::uint64_t);
    void writeBlock(std::uint64_t, std::array<unsigned char, B>);

    void flush();

public:
    Segment<B>& operator=(const Segment<B>&) = delete;
    Segment<B>& operator=(Segment<B>&&) noexcept = default;
};
// --------------------------------------------------------------------------
template<std::size_t B>
Segment<B>::Segment(int fd, std::size_t fileOffset)
    : fd(fd), fileOffset(fileOffset) {
    assert(fd >= 0);
    // load the header
    if (pread(fd, &header, sizeof(Header), fileOffset) != sizeof(Header)) {
        util::raise("Invalid segment!");
    }
    if (header.blockSize != B) {
        util::raise("Different block sizes (segment)!");
    }
}
// --------------------------------------------------------------------------
template<std::size_t B>
Segment<B>::Segment(int fd, std::size_t fileOffset, std::uint64_t segmentSize)
    : fd(fd), fileOffset(fileOffset) {
    assert(fd >= 0);
    // create a new header
    header = {B, segmentSize, 0, 0, std::nullopt};
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t Segment<B>::getIDInBlock(std::uint64_t id) {
    std::uint64_t result;
    if (pread(fd, &result, sizeof(std::uint64_t),
              fileOffset + B + id * B) != sizeof(std::uint64_t)) {
        util::raise("Could not read id in block.");
    }
    return result;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void Segment<B>::storeIDInBlock(std::uint64_t id, std::uint64_t idToStore) {
    if (pwrite(fd, &idToStore, sizeof(uint64_t),
               fileOffset + B + id * B) != sizeof(std::uint64_t)) {
        util::raise("Could not write id to block.");
    }
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
constexpr std::size_t Segment<B>::getBlockSize() const {
    return B;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::size_t Segment<B>::getOffset() const {
    return fileOffset;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::size_t Segment<B>::getTotalSize() const {
    // header + blocks
    return B + header.allocatedBlocks * B;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t Segment<B>::freeBlocks() const {
    return header.allocatedBlocks - header.occupiedBlocks + header.deletedBlocks;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t Segment<B>::allocatedBlocks() const {
    return header.allocatedBlocks;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t Segment<B>::createBlock() {
    if (header.deletedBlocks > 0) {
        // default case if operations are delete-heavy
        assert(header.freeNodesHead);
        std::uint64_t result = *header.freeNodesHead;
        if (header.deletedBlocks > 1) {
            std::uint64_t nextID = getIDInBlock(result);// IO read
            header.freeNodesHead = nextID;
        } else {
            header.freeNodesHead = std::nullopt;
        }
        header.deletedBlocks--;
        return result;
    }
    assert(!header.freeNodesHead);
    if (header.allocatedBlocks > header.occupiedBlocks) {
        // default case if operations are create-heavy
        std::uint64_t result = header.occupiedBlocks;
        header.occupiedBlocks++;
        return result;
    }
    util::raise("All blocks were occupied!");
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void Segment<B>::deleteBlock(std::uint64_t id) {
    if (header.deletedBlocks > 0) {
        assert(header.freeNodesHead);
        std::uint64_t nextID = *header.freeNodesHead;
        storeIDInBlock(id, nextID);// IO write
        header.freeNodesHead = id;
    } else {
        assert(!header.freeNodesHead);
        header.freeNodesHead = id;
    }
    header.deletedBlocks++;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::array<unsigned char, B> Segment<B>::readBlock(std::uint64_t id) {
    std::array<unsigned char, B> result;
    if (pread(fd, result.data(), B, fileOffset + B + id * B) != B) {
        util::raise("Could not read the block.");
    }
    return result;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void Segment<B>::writeBlock(std::uint64_t id, std::array<unsigned char, B> data) {
    if (pwrite(fd, data.data(), B, fileOffset + B + id * B) != B) {
        util::raise("Could not write the block.");
    }
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void Segment<B>::flush() {
    if (pwrite(fd, &header, sizeof(Header), fileOffset) != sizeof(Header)) {
        util::raise("Could not save the header.");
    }
}
// --------------------------------------------------------------------------
}// namespace file
// --------------------------------------------------------------------------
#endif//B_EPSILON_SEGMENT_H
