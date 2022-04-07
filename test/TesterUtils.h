#ifndef B_EPSILON_TESTERUTILS_H
#define B_EPSILON_TESTERUTILS_H
// --------------------------------------------------------------------------
#include <functional>
#include <thread>
#include <vector>
// --------------------------------------------------------------------------
namespace utils {
// --------------------------------------------------------------------------
class DummyThreadPool {

private:

    std::vector<std::thread> threads;

public:

    DummyThreadPool() = default;

    void submit(std::function<void()>);
    void join();

};
// --------------------------------------------------------------------------
}// namespace utils
// --------------------------------------------------------------------------
#endif//B_EPSILON_TESTERUTILS_H
