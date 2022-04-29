#include "file/SegmentManager.h"
#include <filesystem>
#include "tree/BeNode.h"
#include <iostream>
// --------------------------------------------------------------------------
using namespace std;
using namespace file;
// --------------------------------------------------------------------------
int main() {
    //static const string DIRNAME = "/tmp/tester_test_segment_manager";
    // std::filesystem::remove_all(DIRNAME.c_str());
    //SegmentManager<4096> segmentManager(DIRNAME, 1.25);
    tree::BeNode<std::uint64_t, std::pair<std::uint64_t, std::uint64_t>, 4096> node(false);
    return 0;
}
// --------------------------------------------------------------------------
