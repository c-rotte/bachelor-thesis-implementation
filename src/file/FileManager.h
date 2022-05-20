#ifndef B_EPSILON_FILEMANAGER_H
#define B_EPSILON_FILEMANAGER_H
// --------------------------------------------------------------------------
#include <array>
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <unistd.h>
#include <unordered_set>
// --------------------------------------------------------------------------
namespace file {
// --------------------------------------------------------------------------
template<std::uint64_t B>
class FileManager {
    // WARNING: file creation + deletion are not thread-safe
    // WARNING: concurrent deletion + write to the same block is undefined

    // make sure that the block is capable of storing a list pointer
    static_assert(B > sizeof(std::uint64_t));

    struct alignas(alignof(std::max_align_t)) Header {
        std::uint64_t blockSize = 0;
        std::uint64_t allocatedBlocks = 0;
        std::uint64_t occupiedBlocks = 0;
        std::uint64_t deletedBlocks = 0;
        std::optional<std::uint64_t> freeNodesHead;
    };
    // use 4096 bytes for the header (operating system block size)
    static constexpr std::size_t HEADER_SIZE = 4096;
    static_assert(sizeof(Header) <= HEADER_SIZE);

private:
    Header header;
    int fd;

public:
    FileManager() = delete;
    explicit FileManager(const std::string&, std::size_t = 0);
    FileManager(const FileManager<B>&) = delete;
    FileManager(FileManager<B>&&) noexcept = default;

private:
    std::uint64_t getIDInBlock(std::uint64_t);
    void storeIDInBlock(std::uint64_t, std::uint64_t);

public:
    constexpr size_t getBlockSize() const;
    std::uint64_t freeBlocks() const;
    std::uint64_t allocatedBlocks() const;

    std::uint64_t createBlock();
    void deleteBlock(std::uint64_t);
    std::array<unsigned char, B> readBlock(std::uint64_t);
    void writeBlock(std::uint64_t, std::array<unsigned char, B>);

    void flush();

public:
    FileManager<B>& operator=(const FileManager<B>&) = delete;
    FileManager<B>& operator=(FileManager<B>&&) noexcept = default;
};
// --------------------------------------------------------------------------
template<std::uint64_t B>
FileManager<B>::FileManager(const std::string& filePath, std::size_t allocatedBlocks) {
    if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath)) {
        fd = open(filePath.c_str(), O_RDWR);// TODO: check O_DIRECT (errno 9)
        if (pread(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
            throw std::runtime_error("Invalid file!");
        }
        if (header.blockSize != B) {
            throw std::runtime_error("Different block sizes!");
        }
    } else {
        if (allocatedBlocks == 0) {
            throw std::runtime_error("<allocatedBlocks> cannot be 0 if the file does not exist yet.");
        }
        // TODO: check O_DIRECT (errno 22)
        fd = open(filePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            throw std::runtime_error("Could not create file!");
        }
        // initialize the file with <allocatedBlocks> blocks
        header = {B, allocatedBlocks, 0, 0, std::nullopt};
        if (ftruncate(fd, HEADER_SIZE + allocatedBlocks * B) < 0) {
            throw std::runtime_error("Could not increase the file size.");
        }
    }
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t FileManager<B>::getIDInBlock(std::uint64_t id) {
    std::uint64_t result;
    if (pread(fd, &result, sizeof(std::uint64_t),
              HEADER_SIZE + id * B) != sizeof(std::uint64_t)) {
        throw std::runtime_error("Could not read id in block.");
    }
    return result;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void FileManager<B>::storeIDInBlock(std::uint64_t id, std::uint64_t idToStore) {
    if (pwrite(fd, &idToStore, sizeof(uint64_t),
               HEADER_SIZE + id * B) != sizeof(std::uint64_t)) {
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
std::uint64_t FileManager<B>::freeBlocks() const {
    return header.allocatedBlocks - header.occupiedBlocks + header.deletedBlocks;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t FileManager<B>::allocatedBlocks() const {
    return header.allocatedBlocks;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
std::uint64_t FileManager<B>::createBlock() {
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
    throw std::runtime_error("All blocks were occupied!");
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void FileManager<B>::deleteBlock(std::uint64_t id) {
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
std::array<unsigned char, B> FileManager<B>::readBlock(std::uint64_t id) {
    std::array<unsigned char, B> result;
    if (pread(fd, result.data(), B, HEADER_SIZE + id * B) != B) {
        throw std::runtime_error("Could not read the block.");
    }
    return result;
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void FileManager<B>::writeBlock(std::uint64_t id, std::array<unsigned char, B> data) {
    if (pwrite(fd, data.data(), B, HEADER_SIZE + id * B) != B) {
        throw std::runtime_error("Could not write the block.");
    }
}
// --------------------------------------------------------------------------
template<std::uint64_t B>
void FileManager<B>::flush() {
    if (pwrite(fd, &header, sizeof(Header), 0) != sizeof(Header)) {
        throw std::runtime_error("Could not save the header.");
    }
}
// --------------------------------------------------------------------------
}// namespace file
// --------------------------------------------------------------------------
#endif//B_EPSILON_FILEMANAGER_H
