if (NOT UNIX)
   message(FATAL_ERROR "Unsupported OS")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

include(EnableAddressSanitizer)
include(EnableUndefinedSanitizer)
include(clang-tidy)
include(Bundled)

add_custom_target(lint)
