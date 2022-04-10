
#include "file/SegmentManager.h"
#include <filesystem>
#include <iostream>
// --------------------------------------------------------------------------
using namespace std;
using namespace file;
// --------------------------------------------------------------------------
int main() {
    static const string DIRNAME = "/tmp/tester_test_segment_manager";
    // std::filesystem::remove_all(DIRNAME.c_str());
    SegmentManager<4096> segmentManager(DIRNAME, 1.25);
    return 0;
}
// --------------------------------------------------------------------------
