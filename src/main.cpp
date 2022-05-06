#include "file/SegmentManager.h"
#include "tree/BeNode.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
// --------------------------------------------------------------------------
using namespace std;
using namespace file;
// --------------------------------------------------------------------------
template<size_t N>
struct For {
    template<size_t I>
    static void iteration() {
        std::cout << "epsilon=" << std::fixed << std::setprecision(2) << (I / 100.0) << ": ";
        tree::BeNodeWrapper<std::uint64_t, std::uint64_t, 4096, I> node(false);
        if constexpr (I + 1 < N) For<N>::iteration<I + 1>();
    }
};
// --------------------------------------------------------------------------
int main() {
    //static const string DIRNAME = "/tmp/tester_test_segment_manager";
    // std::filesystem::remove_all(DIRNAME.c_str());
    //SegmentManager<4096> segmentManager(DIRNAME, 1.25);
    For<101>::iteration<0>();
    return 0;
}
// --------------------------------------------------------------------------
