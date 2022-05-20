set(TEST_SOURCES
        Tester.cpp
        TesterUtils.cpp
        TestFileManager.cpp
        TestSegmentManager.cpp
        TestQueue.cpp
        TestPageBuffer.cpp
        utils/SimpleBinaryTree.cpp
        TestBeTreeMethods.cpp
        TestBeTree.cpp)

add_clang_tidy_target(lint_tester_core ${TEST_SOURCES})
add_dependencies(lint lint_tester_core)

add_executable(tester ${TEST_SOURCES})
target_link_libraries(tester PUBLIC
        b_epsilon_core
        GTest::GTest
        PRIVATE TBB::tbb)
