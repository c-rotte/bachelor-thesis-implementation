#ifndef B_EPSILON_FILEMANAGER_H
#define B_EPSILON_FILEMANAGER_H
// --------------------------------------------------------------------------
#include <array>
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <unistd.h>
// --------------------------------------------------------------------------
namespace file {
// --------------------------------------------------------------------------
template<std::uint64_t B>
class FileManager {

    // make sure that the block is capable of storing a list pointer
    static_assert(B > sizeof(std::uint64_t));

    struct alignas(alignof(std::max_align_t)) Header {
        std::uint64_t blockSize;
        std::uint64_t allocatedBlocks;
        std::uint64_t occupiedBlocks;
        std::uint64_t deletedBlocks;
        std::optional<std::uint64_t> freeNodesHead;
    };

private:
    Header header;
    int fd;
    mutable std::mutex mutex; // mutex to safely create and delete blocks

public:
    FileManager(const std::string&);
    virtual ~FileManager() noexcept(false);

private:
    std::uint64_t getIDInBlock(std::uint64_t);
    void storeIDInBlock(std::uint64_t, std::uint64_t);

public:
    constexpr size_t getBlockSize() const;
    std::uint64_t allocatedBlocks() const;

    std::size_t createBlock();
    void deleteBlock(std::uint64_t);
    std::array<unsigned char, B> readBlock(std::uint64_t);
    void writeBlock(std::uint64_t, std::array<unsigned char, B>);

    // TODO: delete copy + move
};
// --------------------------------------------------------------------------
template<std::uint64_t B>
FileManager<B>::FileManager(const std::string& filePath) {
    if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath)) {
        fd = open(filePath.c_str(), O_RDWR);
        if (pread(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
            throw std::runtime_error("Invalid file!");
        }
        if (header.blockSize != B) {
            throw std::runtime_error("Different block sizes!");
        }
    } else {
        fd = open(filePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            throw std::runtime_error("Could not create file!");
        }
        // initialize the file with 10 blocks
        header = {B, 10, 0, 0, std::nullopt};
        if (ftruncate(fd, sizeof(Header) + 10 * B) < 0) {
            throw std::runtime_error("Could not increase the file size.");
        }
    }
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
FileManager<B>::~FileManager() noexcept(false) {
    // flush the header
    if (pwrite(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
        throw std::runtime_error("Could not save the header.");
    }
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t FileManager<B>::getIDInBlock(std::uint64_t id) {
    std::uint64_t result;
    if (pread(fd, &result, sizeof(std::uint64_t),
              sizeof(Header) + id * B) != sizeof(std::uint64_t)) {
        throw std::runtime_error("Could not read id in block.");
    }
    return result;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void FileManager<B>::storeIDInBlock(std::uint64_t id, std::uint64_t idToStore) {
    if (pwrite(fd, &idToStore, sizeof(uint64_t),
               sizeof(Header) + id * B) != sizeof(std::uint64_t)) {
        throw std::runtime_error("Could not write id to block.");
    }
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
constexpr size_t FileManager<B>::getBlockSize() const {
    return B;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t FileManager<B>::allocatedBlocks() const {
    return header.allocatedBlocks;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::size_t FileManager<B>::createBlock() {
    std::unique_lock lock(mutex); // block to prevent race condition
    if (header.deletedBlocks > 0) {
        assert(header.freeNodesHead);
        std::uint64_t result = *header.freeNodesHead;
        if (header.deletedBlocks > 1) {
            std::uint64_t nextID = getIDInBlock(result); // IO
            header.freeNodesHead = nextID;
        } else {
            header.freeNodesHead = std::nullopt;
        }
        header.deletedBlocks--;
        return result;
    }
    assert(!header.freeNodesHead);
    if (header.allocatedBlocks > header.occupiedBlocks) {
        std::uint64_t result = header.occupiedBlocks;
        header.occupiedBlocks++;
        return result;
    }
    std::uint64_t result = header.allocatedBlocks;
    std::size_t newBlocks = 1.25 * header.allocatedBlocks;
    if (ftruncate(fd, sizeof(Header) + newBlocks * B) < 0) { // IO
        throw std::runtime_error("Could not increase the file size.");
    }
    header.allocatedBlocks = newBlocks;
    header.occupiedBlocks++;
    return result;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void FileManager<B>::deleteBlock(std::uint64_t id) {
    std::unique_lock lock(mutex); // block to prevent race condition
    if (header.deletedBlocks > 0) {
        assert(header.freeNodesHead);
        std::uint64_t nextID = *header.freeNodesHead;
        storeIDInBlock(id, nextID); // IO
        header.freeNodesHead = id;
    } else {
        assert(!header.freeNodesHead);
        header.freeNodesHead = id;
    }
    header.deletedBlocks++;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::array<unsigned char, B> FileManager<B>::readBlock(std::uint64_t id) {
    std::array<unsigned char, B> result;
    if (pread(fd, result.data(), B, sizeof(Header) + id * B) != B) {
        throw std::runtime_error("Could not read the block.");
    }
    return result;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void FileManager<B>::writeBlock(std::uint64_t id, std::array<unsigned char, B> data) {
    if (pwrite(fd, data.data(), B, sizeof(Header) + id * B) != B) {
        throw std::runtime_error("Could not write the block.");
    }
}
// --------------------------------------------------------------------------
}// namespace file
// --------------------------------------------------------------------------
#endif//B_EPSILON_FILEMANAGER_H
