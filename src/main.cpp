#include <iostream>
#include "file/FileManager.h"
// --------------------------------------------------------------------------
int main() {
    file::FileManager<4096> f("/tmp/tree.txt");
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
// --------------------------------------------------------------------------
