#include <gtest/gtest.h>
// --------------------------------------------------------------------------
#include "src/buffer/queue/FIFOQueue.h"
#include "src/buffer/queue/LRUQueue.h"
#include <memory>
// --------------------------------------------------------------------------
using namespace std;
using namespace buffer::queue;
// --------------------------------------------------------------------------
namespace {
// --------------------------------------------------------------------------
}// namespace
// --------------------------------------------------------------------------
TEST(FIFOQueue, Store) {
    FIFOQueue<int, unique_ptr<int>> queue;
    for (int i = 0; i < 32; i++) {
        ASSERT_NO_THROW(queue.insert(i, make_unique<int>(i)));
    }
    for (int i = 31; i >= 0; i--) {
        ASSERT_EQ(*queue.find(i), i);
        ASSERT_TRUE(queue.contains(i));
    }
    ASSERT_FALSE(queue.contains(32));
    ASSERT_THROW(queue.find(32), std::runtime_error);
    {
        queue.insert(32, make_unique<int>(32));
        auto found = queue.findOne([](const unique_ptr<int>&) {
            return true;
        });
        ASSERT_TRUE(found);
        auto removed = queue.remove(*found);
        ASSERT_EQ(removed.first, 0);
        ASSERT_EQ(*removed.second, 0);
    }
    for (int i = 32; i >= 1; i--) {
        ASSERT_NO_THROW(queue.find(i));
        ASSERT_TRUE(queue.contains(i));
    }
    {
        auto found = queue.findOne([](const unique_ptr<int>& v) {
            return *v == 5;
        });
        ASSERT_TRUE(found);
        auto removed = queue.remove(*found);
        ASSERT_EQ(removed.first, 5);
        ASSERT_EQ(*removed.second, 5);
    }
    {
        auto removed = queue.remove(15);
        ASSERT_EQ(removed.first, 15);
        ASSERT_EQ(*removed.second, 15);
        ASSERT_FALSE(queue.contains(15));
    }
}
// --------------------------------------------------------------------------
TEST(LIFOQueue, Store) {
    LRUQueue<int, unique_ptr<int>> queue;
    for (int i = 0; i < 32; i++) {
        ASSERT_NO_THROW(queue.insert(
                i, make_unique<int>(i)));
    }
    for (int i = 0; i < 32; i++) {
        ASSERT_TRUE(queue.contains(i));
    }
    ASSERT_FALSE(queue.contains(32));
    ASSERT_THROW(queue.find(32, true), std::runtime_error);
    {
        queue.insert(32, make_unique<int>(32));
        auto found = queue.findOne([](const unique_ptr<int>&) {
            return true;
        });
        ASSERT_TRUE(found);
        auto removed = queue.remove(*found);
        ASSERT_EQ(removed.first, 0);
        ASSERT_EQ(*removed.second, 0);
    }
    {
        queue.insert(33, make_unique<int>(33));
        auto found = queue.findOne([](const unique_ptr<int>& v) {
            return *v == 5;
        });
        ASSERT_TRUE(found);
        auto removed = queue.remove(*found);
        ASSERT_EQ(removed.first, 5);
        ASSERT_EQ(*removed.second, 5);
    }
    auto& ptr = queue.find(10, true);
    ASSERT_EQ(*ptr, 10);
    for (int i = 1; i <= 33; i++) {
        if (i != 10 && i != 5) {
            queue.find(i, true);
        }
        if (i == 10) {
            queue.find(i, false);
        }
    }
    {
        queue.insert(34, make_unique<int>(34));
        auto found = queue.findOne([](const unique_ptr<int>&) {
            return true;
        });
        ASSERT_TRUE(found);
        auto removed = queue.remove(*found);
        ASSERT_EQ(removed.first, 10);
        ASSERT_EQ(*removed.second, 10);
    }
    {
        auto removed = queue.remove(15);
        ASSERT_EQ(removed.first, 15);
        ASSERT_EQ(*removed.second, 15);
        ASSERT_FALSE(queue.contains(15));
    }
}
// --------------------------------------------------------------------------
