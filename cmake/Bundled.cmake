find_package(GTest)
if (NOT GTEST_FOUND)
   message(STATUS "Adding bundled Google Test")
   set(BUILD_GMOCK OFF CACHE BOOL INTERNAL)
   set(INSTALL_GTEST OFF CACHE BOOL INTERNAL)

   add_subdirectory(../thirdparty/googletest)

   add_library(GTest::GTest ALIAS gtest)
endif ()