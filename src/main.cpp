#include "tree/BeTree.h"
#include <filesystem>
#include <iostream>
// --------------------------------------------------------------------------
using namespace std;
using namespace file;
// --------------------------------------------------------------------------
int main() {
    static const string DIRNAME = "/tmp/be_tree";
    std::filesystem::remove_all(DIRNAME.c_str());

    return 0;
}
// --------------------------------------------------------------------------
