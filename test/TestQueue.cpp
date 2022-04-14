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
    FIFOQueue<int, unique_ptr<int>> queue(32);
    for (int i = 0; i < 32; i++) {
        auto removed = queue.insert(i, make_unique<int>(i),
                                    [](const unique_ptr<int>&) {
                                        return true;
                                    });
        ASSERT_FALSE(removed);
    }
    for (int i = 31; i >= 0; i--) {
        ASSERT_EQ(*queue.find(i), i);
        ASSERT_TRUE(queue.contains(i));
    }
    ASSERT_FALSE(queue.contains(32));
    ASSERT_THROW(queue.find(32), std::runtime_error);
    {
        auto removed = queue.insert(32, make_unique<int>(32),
                                    [](const unique_ptr<int>&) {
                                        return true;
                                    });
        ASSERT_TRUE(removed);
        ASSERT_EQ(removed->first, 0);
        ASSERT_EQ(*removed->second, 0);
        for (int i = 32; i >= 1; i--) {
            ASSERT_NO_THROW(queue.find(i));
            ASSERT_TRUE(queue.contains(i));
        }
    }
    {
        auto removed = queue.insert(33, make_unique<int>(32),
                                    [](const unique_ptr<int>& v) {
                                        return *v == 5;
                                    });
        ASSERT_TRUE(removed);
        ASSERT_EQ(removed->first, 5);
        ASSERT_EQ(*removed->second, 5);
    }
}
// --------------------------------------------------------------------------
TEST(LIFOQueue, Store) {
    LRUQueue<int, unique_ptr<int>> queue(32);
    for (int i = 0; i < 32; i++) {
        auto removed = queue.insert(i, make_unique<int>(i),
                                    [](const unique_ptr<int>&) {
                                        return true;
                                    });
        ASSERT_FALSE(removed);
    }
    for (int i = 0; i < 32; i++) {
        ASSERT_TRUE(queue.contains(i));
    }
    ASSERT_FALSE(queue.contains(32));
    ASSERT_THROW(queue.find(32), std::runtime_error);
    {
        auto removed = queue.insert(32, make_unique<int>(32),
                                    [](const unique_ptr<int>&) {
                                        return true;
                                    });
        ASSERT_TRUE(removed);
        ASSERT_EQ(removed->first, 0);
        ASSERT_EQ(*removed->second, 0);
    }
    {
        auto removed = queue.insert(33, make_unique<int>(32),
                                    [](const unique_ptr<int>& v) {
                                        return *v == 5;
                                    });
        ASSERT_TRUE(removed);
        ASSERT_EQ(removed->first, 5);
        ASSERT_EQ(*removed->second, 5);
    }
    {
        auto& ptr = queue.find(10);
        ASSERT_EQ(*ptr, 10);
        for (int i = 1; i <= 33; i++) {
            if (i != 10 && i != 5) {
                queue.find(i);
            }
        }
        auto removed = queue.insert(34, make_unique<int>(32),
                                    [](const unique_ptr<int>&) {
                                        return true;
                                    });
        ASSERT_TRUE(removed);
        ASSERT_EQ(removed->first, 10);
        ASSERT_EQ(*removed->second, 10);
    }
}
// --------------------------------------------------------------------------
