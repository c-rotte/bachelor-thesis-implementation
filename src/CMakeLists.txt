set(B_EPSILON_SOURCES
        file/Segment.cpp
        file/SegmentManager.cpp
        buffer/PageBuffer.cpp
        buffer/queue/FIFOQueue.cpp
        buffer/queue/LRUQueue.cpp
        betree/BeTree.cpp
        betree/BeNode.cpp
        btree/BTree.cpp
        btree/BNode.cpp
        util/ErrorHandler.cpp)

add_library(b_epsilon_core ${B_EPSILON_SOURCES})
target_include_directories(b_epsilon_core PUBLIC ${CMAKE_SOURCE_DIR})

add_clang_tidy_target(lint_b_epsilon_core ${B_EPSILON_SOURCES})
add_dependencies(lint lint_b_epsilon_core)

add_executable(b_epsilon main.cpp)
target_link_libraries(b_epsilon
        PUBLIC b_epsilon_core
        TBB::tbb)