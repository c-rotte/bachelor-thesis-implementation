#ifndef B_EPSILON_ERRORHANDLER_H
#define B_EPSILON_ERRORHANDLER_H
// --------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <thread>
// --------------------------------------------------------------------------
namespace util {
// --------------------------------------------------------------------------
[[noreturn]] inline void raise(const std::string& what) {
    std::cerr << "Thread ["
              << std::this_thread::get_id()
              << "] raised an exception: \""
              << what
              << "\""
              << std::endl;
    throw std::runtime_error(what);
}
// --------------------------------------------------------------------------
}// namespace util
// --------------------------------------------------------------------------
#endif//B_EPSILON_ERRORHANDLER_H
