#ifndef B_EPSILON_PAGEBUFFER_H
#define B_EPSILON_PAGEBUFFER_H
// --------------------------------------------------------------------------
#include <array>
#include <unordered_set>
#include <cstddef>
#include <cinttypes>
#include <queue>
// --------------------------------------------------------------------------
namespace buffer {
// --------------------------------------------------------------------------
template <std::size_t B>
struct Page {

    alignas(alignof(std::max_align_t)) std::array<unsigned char, B> data;
};
// --------------------------------------------------------------------------
template <std::size_t B, std::size_t N>
class PageBuffer {

private:

    std::queue<Page<B>> fifoQueue;

public:
    PageBuffer();

public:
    uint64_t createPage();
    void deletePage();
    Page<B>& pinPage(uint64_t, bool);
    void unpinPage(uint64_t, bool);

};
// --------------------------------------------------------------------------
}// namespace buffer
// --------------------------------------------------------------------------
#endif//B_EPSILON_PAGEBUFFER_H
