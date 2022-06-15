#include <array>
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>
// --------------------------------------------------------------------------
using namespace std;
// --------------------------------------------------------------------------
int main() {
    filesystem::remove("/tmp/12345");
    int fd = open("/tmp/12345", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ftruncate(fd, 1);
    for (size_t i = 1; i <= 10000; i++) {
        thread thread1([fd, i]() {
            for(int a = 0; a < 100; a++){
                ftruncate(fd, i + 1 + (rand() % 5000));
                //std::cout << 1 << std::endl;
            }
        });
        thread thread2([fd, i]() {
            vector<unsigned char> buf;
            for (size_t j = 0; j < i; j++) {
                buf.push_back(static_cast<unsigned char>(j % 256));
            }
            size_t r = pwrite(fd, buf.data(), buf.size(), 0);
            if(r != buf.size()){
                throw std::runtime_error("1");
            }
        });
        thread1.join();
        thread2.join();

        vector<unsigned char> buf(i);
        size_t r = pread(fd, buf.data(), buf.size(), 0);
        if(r != buf.size()){
            throw std::runtime_error("2");
        }
        for (size_t j = 0; j < i; j++) {
            unsigned char c = buf[j];
            if(static_cast<unsigned char>(j % 256) != c){
                throw std::runtime_error("3");
            }
        }
        std::cout << "passed " << i << std::endl;
    }

    return 0;
}
// --------------------------------------------------------------------------
