include(CMakeParseArguments)

macro(conan_find_apple_frameworks FRAMEWORKS_FOUND FRAMEWORKS SUFFIX BUILD_TYPE)
    if(APPLE)
        if(CMAKE_BUILD_TYPE)
            set(_BTYPE ${CMAKE_BUILD_TYPE})
        elseif(NOT BUILD_TYPE STREQUAL "")
            set(_BTYPE ${BUILD_TYPE})
        endif()
        if(_BTYPE)
            if(${_BTYPE} MATCHES "Debug|_DEBUG")
                set(CONAN_FRAMEWORKS${SUFFIX} ${CONAN_FRAMEWORKS${SUFFIX}_DEBUG} ${CONAN_FRAMEWORKS${SUFFIX}})
                set(CONAN_FRAMEWORK_DIRS${SUFFIX} ${CONAN_FRAMEWORK_DIRS${SUFFIX}_DEBUG} ${CONAN_FRAMEWORK_DIRS${SUFFIX}})
            elseif(${_BTYPE} MATCHES "Release|_RELEASE")
                set(CONAN_FRAMEWORKS${SUFFIX} ${CONAN_FRAMEWORKS${SUFFIX}_RELEASE} ${CONAN_FRAMEWORKS${SUFFIX}})
                set(CONAN_FRAMEWORK_DIRS${SUFFIX} ${CONAN_FRAMEWORK_DIRS${SUFFIX}_RELEASE} ${CONAN_FRAMEWORK_DIRS${SUFFIX}})
            elseif(${_BTYPE} MATCHES "RelWithDebInfo|_RELWITHDEBINFO")
                set(CONAN_FRAMEWORKS${SUFFIX} ${CONAN_FRAMEWORKS${SUFFIX}_RELWITHDEBINFO} ${CONAN_FRAMEWORKS${SUFFIX}})
                set(CONAN_FRAMEWORK_DIRS${SUFFIX} ${CONAN_FRAMEWORK_DIRS${SUFFIX}_RELWITHDEBINFO} ${CONAN_FRAMEWORK_DIRS${SUFFIX}})
            elseif(${_BTYPE} MATCHES "MinSizeRel|_MINSIZEREL")
                set(CONAN_FRAMEWORKS${SUFFIX} ${CONAN_FRAMEWORKS${SUFFIX}_MINSIZEREL} ${CONAN_FRAMEWORKS${SUFFIX}})
                set(CONAN_FRAMEWORK_DIRS${SUFFIX} ${CONAN_FRAMEWORK_DIRS${SUFFIX}_MINSIZEREL} ${CONAN_FRAMEWORK_DIRS${SUFFIX}})
            endif()
        endif()
        foreach(_FRAMEWORK ${FRAMEWORKS})
            # https://cmake.org/pipermail/cmake-developers/2017-August/030199.html
            find_library(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND NAME ${_FRAMEWORK} PATHS ${CONAN_FRAMEWORK_DIRS${SUFFIX}} CMAKE_FIND_ROOT_PATH_BOTH)
            if(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND)
                list(APPEND ${FRAMEWORKS_FOUND} ${CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND})
            else()
                message(FATAL_ERROR "Framework library ${_FRAMEWORK} not found in paths: ${CONAN_FRAMEWORK_DIRS${SUFFIX}}")
            endif()
        endforeach()
    endif()
endmacro()


#################
###  FMT
#################
set(CONAN_FMT_ROOT "/home/chrissi/.conan/data/fmt/8.0.1/_/_/package/2c09c8f84c016041549fcee94e4caae5d89424b6")
set(CONAN_INCLUDE_DIRS_FMT "/home/chrissi/.conan/data/fmt/8.0.1/_/_/package/2c09c8f84c016041549fcee94e4caae5d89424b6/include")
set(CONAN_LIB_DIRS_FMT "/home/chrissi/.conan/data/fmt/8.0.1/_/_/package/2c09c8f84c016041549fcee94e4caae5d89424b6/lib")
set(CONAN_BIN_DIRS_FMT )
set(CONAN_RES_DIRS_FMT )
set(CONAN_SRC_DIRS_FMT )
set(CONAN_BUILD_DIRS_FMT "/home/chrissi/.conan/data/fmt/8.0.1/_/_/package/2c09c8f84c016041549fcee94e4caae5d89424b6/")
set(CONAN_FRAMEWORK_DIRS_FMT )
set(CONAN_LIBS_FMT fmt)
set(CONAN_PKG_LIBS_FMT fmt)
set(CONAN_SYSTEM_LIBS_FMT )
set(CONAN_FRAMEWORKS_FMT )
set(CONAN_FRAMEWORKS_FOUND_FMT "")  # Will be filled later
set(CONAN_DEFINES_FMT )
set(CONAN_BUILD_MODULES_PATHS_FMT )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_FMT )

set(CONAN_C_FLAGS_FMT "")
set(CONAN_CXX_FLAGS_FMT "")
set(CONAN_SHARED_LINKER_FLAGS_FMT "")
set(CONAN_EXE_LINKER_FLAGS_FMT "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_FMT_LIST "")
set(CONAN_CXX_FLAGS_FMT_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_FMT_LIST "")
set(CONAN_EXE_LINKER_FLAGS_FMT_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_FMT "${CONAN_FRAMEWORKS_FMT}" "_FMT" "")
# Append to aggregated values variable
set(CONAN_LIBS_FMT ${CONAN_PKG_LIBS_FMT} ${CONAN_SYSTEM_LIBS_FMT} ${CONAN_FRAMEWORKS_FOUND_FMT})


#################
###  BENCHMARK
#################
set(CONAN_BENCHMARK_ROOT "/home/chrissi/.conan/data/benchmark/1.6.0/_/_/package/1fa499dcb6ccf57733ed1f578fb1e1bd4cf858b1")
set(CONAN_INCLUDE_DIRS_BENCHMARK "/home/chrissi/.conan/data/benchmark/1.6.0/_/_/package/1fa499dcb6ccf57733ed1f578fb1e1bd4cf858b1/include")
set(CONAN_LIB_DIRS_BENCHMARK "/home/chrissi/.conan/data/benchmark/1.6.0/_/_/package/1fa499dcb6ccf57733ed1f578fb1e1bd4cf858b1/lib")
set(CONAN_BIN_DIRS_BENCHMARK )
set(CONAN_RES_DIRS_BENCHMARK )
set(CONAN_SRC_DIRS_BENCHMARK )
set(CONAN_BUILD_DIRS_BENCHMARK "/home/chrissi/.conan/data/benchmark/1.6.0/_/_/package/1fa499dcb6ccf57733ed1f578fb1e1bd4cf858b1/")
set(CONAN_FRAMEWORK_DIRS_BENCHMARK )
set(CONAN_LIBS_BENCHMARK benchmark benchmark_main)
set(CONAN_PKG_LIBS_BENCHMARK benchmark benchmark_main)
set(CONAN_SYSTEM_LIBS_BENCHMARK pthread rt)
set(CONAN_FRAMEWORKS_BENCHMARK )
set(CONAN_FRAMEWORKS_FOUND_BENCHMARK "")  # Will be filled later
set(CONAN_DEFINES_BENCHMARK )
set(CONAN_BUILD_MODULES_PATHS_BENCHMARK )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_BENCHMARK )

set(CONAN_C_FLAGS_BENCHMARK "")
set(CONAN_CXX_FLAGS_BENCHMARK "")
set(CONAN_SHARED_LINKER_FLAGS_BENCHMARK "")
set(CONAN_EXE_LINKER_FLAGS_BENCHMARK "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_BENCHMARK_LIST "")
set(CONAN_CXX_FLAGS_BENCHMARK_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_BENCHMARK_LIST "")
set(CONAN_EXE_LINKER_FLAGS_BENCHMARK_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_BENCHMARK "${CONAN_FRAMEWORKS_BENCHMARK}" "_BENCHMARK" "")
# Append to aggregated values variable
set(CONAN_LIBS_BENCHMARK ${CONAN_PKG_LIBS_BENCHMARK} ${CONAN_SYSTEM_LIBS_BENCHMARK} ${CONAN_FRAMEWORKS_FOUND_BENCHMARK})


#################
###  NLOHMANN_JSON
#################
set(CONAN_NLOHMANN_JSON_ROOT "/home/chrissi/.conan/data/nlohmann_json/3.10.4/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9")
set(CONAN_INCLUDE_DIRS_NLOHMANN_JSON "/home/chrissi/.conan/data/nlohmann_json/3.10.4/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(CONAN_LIB_DIRS_NLOHMANN_JSON )
set(CONAN_BIN_DIRS_NLOHMANN_JSON )
set(CONAN_RES_DIRS_NLOHMANN_JSON )
set(CONAN_SRC_DIRS_NLOHMANN_JSON )
set(CONAN_BUILD_DIRS_NLOHMANN_JSON "/home/chrissi/.conan/data/nlohmann_json/3.10.4/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/")
set(CONAN_FRAMEWORK_DIRS_NLOHMANN_JSON )
set(CONAN_LIBS_NLOHMANN_JSON )
set(CONAN_PKG_LIBS_NLOHMANN_JSON )
set(CONAN_SYSTEM_LIBS_NLOHMANN_JSON )
set(CONAN_FRAMEWORKS_NLOHMANN_JSON )
set(CONAN_FRAMEWORKS_FOUND_NLOHMANN_JSON "")  # Will be filled later
set(CONAN_DEFINES_NLOHMANN_JSON )
set(CONAN_BUILD_MODULES_PATHS_NLOHMANN_JSON )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_NLOHMANN_JSON )

set(CONAN_C_FLAGS_NLOHMANN_JSON "")
set(CONAN_CXX_FLAGS_NLOHMANN_JSON "")
set(CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON "")
set(CONAN_EXE_LINKER_FLAGS_NLOHMANN_JSON "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_NLOHMANN_JSON_LIST "")
set(CONAN_CXX_FLAGS_NLOHMANN_JSON_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_LIST "")
set(CONAN_EXE_LINKER_FLAGS_NLOHMANN_JSON_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_NLOHMANN_JSON "${CONAN_FRAMEWORKS_NLOHMANN_JSON}" "_NLOHMANN_JSON" "")
# Append to aggregated values variable
set(CONAN_LIBS_NLOHMANN_JSON ${CONAN_PKG_LIBS_NLOHMANN_JSON} ${CONAN_SYSTEM_LIBS_NLOHMANN_JSON} ${CONAN_FRAMEWORKS_FOUND_NLOHMANN_JSON})


#################
###  LEVELDB
#################
set(CONAN_LEVELDB_ROOT "/home/chrissi/.conan/data/leveldb/1.23/_/_/package/c520310e11cdd3a91c97b3b85775a9235e9918ad")
set(CONAN_INCLUDE_DIRS_LEVELDB "/home/chrissi/.conan/data/leveldb/1.23/_/_/package/c520310e11cdd3a91c97b3b85775a9235e9918ad/include")
set(CONAN_LIB_DIRS_LEVELDB "/home/chrissi/.conan/data/leveldb/1.23/_/_/package/c520310e11cdd3a91c97b3b85775a9235e9918ad/lib")
set(CONAN_BIN_DIRS_LEVELDB )
set(CONAN_RES_DIRS_LEVELDB )
set(CONAN_SRC_DIRS_LEVELDB )
set(CONAN_BUILD_DIRS_LEVELDB "/home/chrissi/.conan/data/leveldb/1.23/_/_/package/c520310e11cdd3a91c97b3b85775a9235e9918ad/")
set(CONAN_FRAMEWORK_DIRS_LEVELDB )
set(CONAN_LIBS_LEVELDB leveldb)
set(CONAN_PKG_LIBS_LEVELDB leveldb)
set(CONAN_SYSTEM_LIBS_LEVELDB pthread)
set(CONAN_FRAMEWORKS_LEVELDB )
set(CONAN_FRAMEWORKS_FOUND_LEVELDB "")  # Will be filled later
set(CONAN_DEFINES_LEVELDB )
set(CONAN_BUILD_MODULES_PATHS_LEVELDB )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_LEVELDB )

set(CONAN_C_FLAGS_LEVELDB "")
set(CONAN_CXX_FLAGS_LEVELDB "")
set(CONAN_SHARED_LINKER_FLAGS_LEVELDB "")
set(CONAN_EXE_LINKER_FLAGS_LEVELDB "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_LEVELDB_LIST "")
set(CONAN_CXX_FLAGS_LEVELDB_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_LEVELDB_LIST "")
set(CONAN_EXE_LINKER_FLAGS_LEVELDB_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_LEVELDB "${CONAN_FRAMEWORKS_LEVELDB}" "_LEVELDB" "")
# Append to aggregated values variable
set(CONAN_LIBS_LEVELDB ${CONAN_PKG_LIBS_LEVELDB} ${CONAN_SYSTEM_LIBS_LEVELDB} ${CONAN_FRAMEWORKS_FOUND_LEVELDB})


#################
###  LMDB
#################
set(CONAN_LMDB_ROOT "/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681")
set(CONAN_INCLUDE_DIRS_LMDB "/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681/include")
set(CONAN_LIB_DIRS_LMDB "/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681/lib")
set(CONAN_BIN_DIRS_LMDB "/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681/bin")
set(CONAN_RES_DIRS_LMDB )
set(CONAN_SRC_DIRS_LMDB )
set(CONAN_BUILD_DIRS_LMDB "/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681/")
set(CONAN_FRAMEWORK_DIRS_LMDB )
set(CONAN_LIBS_LMDB lmdb)
set(CONAN_PKG_LIBS_LMDB lmdb)
set(CONAN_SYSTEM_LIBS_LMDB pthread)
set(CONAN_FRAMEWORKS_LMDB )
set(CONAN_FRAMEWORKS_FOUND_LMDB "")  # Will be filled later
set(CONAN_DEFINES_LMDB )
set(CONAN_BUILD_MODULES_PATHS_LMDB )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_LMDB )

set(CONAN_C_FLAGS_LMDB "")
set(CONAN_CXX_FLAGS_LMDB "")
set(CONAN_SHARED_LINKER_FLAGS_LMDB "")
set(CONAN_EXE_LINKER_FLAGS_LMDB "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_LMDB_LIST "")
set(CONAN_CXX_FLAGS_LMDB_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_LMDB_LIST "")
set(CONAN_EXE_LINKER_FLAGS_LMDB_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_LMDB "${CONAN_FRAMEWORKS_LMDB}" "_LMDB" "")
# Append to aggregated values variable
set(CONAN_LIBS_LMDB ${CONAN_PKG_LIBS_LMDB} ${CONAN_SYSTEM_LIBS_LMDB} ${CONAN_FRAMEWORKS_FOUND_LMDB})


#################
###  SNAPPY
#################
set(CONAN_SNAPPY_ROOT "/home/chrissi/.conan/data/snappy/1.1.9/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e")
set(CONAN_INCLUDE_DIRS_SNAPPY "/home/chrissi/.conan/data/snappy/1.1.9/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/include")
set(CONAN_LIB_DIRS_SNAPPY "/home/chrissi/.conan/data/snappy/1.1.9/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/lib")
set(CONAN_BIN_DIRS_SNAPPY )
set(CONAN_RES_DIRS_SNAPPY )
set(CONAN_SRC_DIRS_SNAPPY )
set(CONAN_BUILD_DIRS_SNAPPY "/home/chrissi/.conan/data/snappy/1.1.9/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/")
set(CONAN_FRAMEWORK_DIRS_SNAPPY )
set(CONAN_LIBS_SNAPPY snappy)
set(CONAN_PKG_LIBS_SNAPPY snappy)
set(CONAN_SYSTEM_LIBS_SNAPPY stdc++)
set(CONAN_FRAMEWORKS_SNAPPY )
set(CONAN_FRAMEWORKS_FOUND_SNAPPY "")  # Will be filled later
set(CONAN_DEFINES_SNAPPY )
set(CONAN_BUILD_MODULES_PATHS_SNAPPY )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_SNAPPY )

set(CONAN_C_FLAGS_SNAPPY "")
set(CONAN_CXX_FLAGS_SNAPPY "")
set(CONAN_SHARED_LINKER_FLAGS_SNAPPY "")
set(CONAN_EXE_LINKER_FLAGS_SNAPPY "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_SNAPPY_LIST "")
set(CONAN_CXX_FLAGS_SNAPPY_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_SNAPPY_LIST "")
set(CONAN_EXE_LINKER_FLAGS_SNAPPY_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_SNAPPY "${CONAN_FRAMEWORKS_SNAPPY}" "_SNAPPY" "")
# Append to aggregated values variable
set(CONAN_LIBS_SNAPPY ${CONAN_PKG_LIBS_SNAPPY} ${CONAN_SYSTEM_LIBS_SNAPPY} ${CONAN_FRAMEWORKS_FOUND_SNAPPY})


#################
###  CRC32C
#################
set(CONAN_CRC32C_ROOT "/home/chrissi/.conan/data/crc32c/1.1.2/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e")
set(CONAN_INCLUDE_DIRS_CRC32C "/home/chrissi/.conan/data/crc32c/1.1.2/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/include")
set(CONAN_LIB_DIRS_CRC32C "/home/chrissi/.conan/data/crc32c/1.1.2/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/lib")
set(CONAN_BIN_DIRS_CRC32C )
set(CONAN_RES_DIRS_CRC32C )
set(CONAN_SRC_DIRS_CRC32C )
set(CONAN_BUILD_DIRS_CRC32C "/home/chrissi/.conan/data/crc32c/1.1.2/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/")
set(CONAN_FRAMEWORK_DIRS_CRC32C )
set(CONAN_LIBS_CRC32C crc32c)
set(CONAN_PKG_LIBS_CRC32C crc32c)
set(CONAN_SYSTEM_LIBS_CRC32C )
set(CONAN_FRAMEWORKS_CRC32C )
set(CONAN_FRAMEWORKS_FOUND_CRC32C "")  # Will be filled later
set(CONAN_DEFINES_CRC32C )
set(CONAN_BUILD_MODULES_PATHS_CRC32C )
# COMPILE_DEFINITIONS are equal to CONAN_DEFINES without -D, for targets
set(CONAN_COMPILE_DEFINITIONS_CRC32C )

set(CONAN_C_FLAGS_CRC32C "")
set(CONAN_CXX_FLAGS_CRC32C "")
set(CONAN_SHARED_LINKER_FLAGS_CRC32C "")
set(CONAN_EXE_LINKER_FLAGS_CRC32C "")

# For modern cmake targets we use the list variables (separated with ;)
set(CONAN_C_FLAGS_CRC32C_LIST "")
set(CONAN_CXX_FLAGS_CRC32C_LIST "")
set(CONAN_SHARED_LINKER_FLAGS_CRC32C_LIST "")
set(CONAN_EXE_LINKER_FLAGS_CRC32C_LIST "")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND_CRC32C "${CONAN_FRAMEWORKS_CRC32C}" "_CRC32C" "")
# Append to aggregated values variable
set(CONAN_LIBS_CRC32C ${CONAN_PKG_LIBS_CRC32C} ${CONAN_SYSTEM_LIBS_CRC32C} ${CONAN_FRAMEWORKS_FOUND_CRC32C})


### Definition of global aggregated variables ###

set(CONAN_PACKAGE_NAME UCSB)
set(CONAN_PACKAGE_VERSION 0.0.1)

set(CONAN_SETTINGS_ARCH "x86_64")
set(CONAN_SETTINGS_BUILD_TYPE "Release")
set(CONAN_SETTINGS_COMPILER "gcc")
set(CONAN_SETTINGS_COMPILER_LIBCXX "libstdc++11")
set(CONAN_SETTINGS_COMPILER_VERSION "11")
set(CONAN_SETTINGS_OS "Linux")

set(CONAN_DEPENDENCIES fmt benchmark nlohmann_json leveldb lmdb snappy crc32c)
# Storing original command line args (CMake helper) flags
set(CONAN_CMD_CXX_FLAGS ${CONAN_CXX_FLAGS})

set(CONAN_CMD_SHARED_LINKER_FLAGS ${CONAN_SHARED_LINKER_FLAGS})
set(CONAN_CMD_C_FLAGS ${CONAN_C_FLAGS})
# Defining accumulated conan variables for all deps

set(CONAN_INCLUDE_DIRS "/home/chrissi/.conan/data/fmt/8.0.1/_/_/package/2c09c8f84c016041549fcee94e4caae5d89424b6/include"
			"/home/chrissi/.conan/data/benchmark/1.6.0/_/_/package/1fa499dcb6ccf57733ed1f578fb1e1bd4cf858b1/include"
			"/home/chrissi/.conan/data/nlohmann_json/3.10.4/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include"
			"/home/chrissi/.conan/data/leveldb/1.23/_/_/package/c520310e11cdd3a91c97b3b85775a9235e9918ad/include"
			"/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681/include"
			"/home/chrissi/.conan/data/snappy/1.1.9/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/include"
			"/home/chrissi/.conan/data/crc32c/1.1.2/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/include" ${CONAN_INCLUDE_DIRS})
set(CONAN_LIB_DIRS "/home/chrissi/.conan/data/fmt/8.0.1/_/_/package/2c09c8f84c016041549fcee94e4caae5d89424b6/lib"
			"/home/chrissi/.conan/data/benchmark/1.6.0/_/_/package/1fa499dcb6ccf57733ed1f578fb1e1bd4cf858b1/lib"
			"/home/chrissi/.conan/data/leveldb/1.23/_/_/package/c520310e11cdd3a91c97b3b85775a9235e9918ad/lib"
			"/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681/lib"
			"/home/chrissi/.conan/data/snappy/1.1.9/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/lib"
			"/home/chrissi/.conan/data/crc32c/1.1.2/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/lib" ${CONAN_LIB_DIRS})
set(CONAN_BIN_DIRS "/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681/bin" ${CONAN_BIN_DIRS})
set(CONAN_RES_DIRS  ${CONAN_RES_DIRS})
set(CONAN_FRAMEWORK_DIRS  ${CONAN_FRAMEWORK_DIRS})
set(CONAN_LIBS fmt benchmark benchmark_main leveldb lmdb snappy crc32c ${CONAN_LIBS})
set(CONAN_PKG_LIBS fmt benchmark benchmark_main leveldb lmdb snappy crc32c ${CONAN_PKG_LIBS})
set(CONAN_SYSTEM_LIBS rt pthread stdc++ ${CONAN_SYSTEM_LIBS})
set(CONAN_FRAMEWORKS  ${CONAN_FRAMEWORKS})
set(CONAN_FRAMEWORKS_FOUND "")  # Will be filled later
set(CONAN_DEFINES  ${CONAN_DEFINES})
set(CONAN_BUILD_MODULES_PATHS  ${CONAN_BUILD_MODULES_PATHS})
set(CONAN_CMAKE_MODULE_PATH "/home/chrissi/.conan/data/fmt/8.0.1/_/_/package/2c09c8f84c016041549fcee94e4caae5d89424b6/"
			"/home/chrissi/.conan/data/benchmark/1.6.0/_/_/package/1fa499dcb6ccf57733ed1f578fb1e1bd4cf858b1/"
			"/home/chrissi/.conan/data/nlohmann_json/3.10.4/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/"
			"/home/chrissi/.conan/data/leveldb/1.23/_/_/package/c520310e11cdd3a91c97b3b85775a9235e9918ad/"
			"/home/chrissi/.conan/data/lmdb/0.9.29/_/_/package/d2a650a4e4d9d78e47b2c3b961fbd52439cb7681/"
			"/home/chrissi/.conan/data/snappy/1.1.9/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/"
			"/home/chrissi/.conan/data/crc32c/1.1.2/_/_/package/6557f18ca99c0b6a233f43db00e30efaa525e27e/" ${CONAN_CMAKE_MODULE_PATH})

set(CONAN_CXX_FLAGS " ${CONAN_CXX_FLAGS}")
set(CONAN_SHARED_LINKER_FLAGS " ${CONAN_SHARED_LINKER_FLAGS}")
set(CONAN_EXE_LINKER_FLAGS " ${CONAN_EXE_LINKER_FLAGS}")
set(CONAN_C_FLAGS " ${CONAN_C_FLAGS}")

# Apple Frameworks
conan_find_apple_frameworks(CONAN_FRAMEWORKS_FOUND "${CONAN_FRAMEWORKS}" "" "")
# Append to aggregated values variable: Use CONAN_LIBS instead of CONAN_PKG_LIBS to include user appended vars
set(CONAN_LIBS ${CONAN_LIBS} ${CONAN_SYSTEM_LIBS} ${CONAN_FRAMEWORKS_FOUND})


###  Definition of macros and functions ###

macro(conan_define_targets)
    if(${CMAKE_VERSION} VERSION_LESS "3.1.2")
        message(FATAL_ERROR "TARGETS not supported by your CMake version!")
    endif()  # CMAKE > 3.x
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CONAN_CMD_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CONAN_CMD_C_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CONAN_CMD_SHARED_LINKER_FLAGS}")


    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES "${CONAN_SYSTEM_LIBS_FMT} ${CONAN_FRAMEWORKS_FOUND_FMT} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT}" "${CONAN_LIB_DIRS_FMT}"
                                  CONAN_PACKAGE_TARGETS_FMT "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES}"
                                  "" fmt)
    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_FMT_DEBUG} ${CONAN_FRAMEWORKS_FOUND_FMT_DEBUG} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT_DEBUG}" "${CONAN_LIB_DIRS_FMT_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_FMT_DEBUG "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG}"
                                  "debug" fmt)
    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_FMT_RELEASE} ${CONAN_FRAMEWORKS_FOUND_FMT_RELEASE} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT_RELEASE}" "${CONAN_LIB_DIRS_FMT_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_FMT_RELEASE "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE}"
                                  "release" fmt)
    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_FMT_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_FMT_RELWITHDEBINFO} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_FMT_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_FMT_RELWITHDEBINFO "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" fmt)
    set(_CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_FMT_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_FMT_MINSIZEREL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_FMT_MINSIZEREL}" "${CONAN_LIB_DIRS_FMT_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_FMT_MINSIZEREL "${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" fmt)

    add_library(CONAN_PKG::fmt INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::fmt PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_FMT} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_FMT_RELEASE} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_FMT_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_FMT_MINSIZEREL} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_FMT_DEBUG} ${_CONAN_PKG_LIBS_FMT_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_FMT_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_FMT_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::fmt PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_FMT}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_FMT_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_FMT_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_FMT_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_FMT_DEBUG}>)
    set_property(TARGET CONAN_PKG::fmt PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_FMT}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_FMT_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_FMT_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_FMT_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_FMT_DEBUG}>)
    set_property(TARGET CONAN_PKG::fmt PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_FMT_LIST} ${CONAN_CXX_FLAGS_FMT_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_FMT_RELEASE_LIST} ${CONAN_CXX_FLAGS_FMT_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_FMT_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_FMT_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_FMT_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_FMT_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_FMT_DEBUG_LIST}  ${CONAN_CXX_FLAGS_FMT_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES "${CONAN_SYSTEM_LIBS_BENCHMARK} ${CONAN_FRAMEWORKS_FOUND_BENCHMARK} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_BENCHMARK}" "${CONAN_LIB_DIRS_BENCHMARK}"
                                  CONAN_PACKAGE_TARGETS_BENCHMARK "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES}"
                                  "" benchmark)
    set(_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_BENCHMARK_DEBUG} ${CONAN_FRAMEWORKS_FOUND_BENCHMARK_DEBUG} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_BENCHMARK_DEBUG}" "${CONAN_LIB_DIRS_BENCHMARK_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_BENCHMARK_DEBUG "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_DEBUG}"
                                  "debug" benchmark)
    set(_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_BENCHMARK_RELEASE} ${CONAN_FRAMEWORKS_FOUND_BENCHMARK_RELEASE} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_BENCHMARK_RELEASE}" "${CONAN_LIB_DIRS_BENCHMARK_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_BENCHMARK_RELEASE "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELEASE}"
                                  "release" benchmark)
    set(_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_BENCHMARK_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_BENCHMARK_RELWITHDEBINFO} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_BENCHMARK_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_BENCHMARK_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_BENCHMARK_RELWITHDEBINFO "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" benchmark)
    set(_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_BENCHMARK_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_BENCHMARK_MINSIZEREL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_BENCHMARK_MINSIZEREL}" "${CONAN_LIB_DIRS_BENCHMARK_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_BENCHMARK_MINSIZEREL "${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" benchmark)

    add_library(CONAN_PKG::benchmark INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::benchmark PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_BENCHMARK} ${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_BENCHMARK_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_BENCHMARK_RELEASE} ${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_BENCHMARK_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_BENCHMARK_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_BENCHMARK_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_BENCHMARK_MINSIZEREL} ${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_BENCHMARK_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_BENCHMARK_DEBUG} ${_CONAN_PKG_LIBS_BENCHMARK_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_BENCHMARK_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_BENCHMARK_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::benchmark PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_BENCHMARK}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_BENCHMARK_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_BENCHMARK_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_BENCHMARK_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_BENCHMARK_DEBUG}>)
    set_property(TARGET CONAN_PKG::benchmark PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_BENCHMARK}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_BENCHMARK_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_BENCHMARK_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_BENCHMARK_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_BENCHMARK_DEBUG}>)
    set_property(TARGET CONAN_PKG::benchmark PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_BENCHMARK_LIST} ${CONAN_CXX_FLAGS_BENCHMARK_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_BENCHMARK_RELEASE_LIST} ${CONAN_CXX_FLAGS_BENCHMARK_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_BENCHMARK_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_BENCHMARK_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_BENCHMARK_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_BENCHMARK_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_BENCHMARK_DEBUG_LIST}  ${CONAN_CXX_FLAGS_BENCHMARK_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES "${CONAN_SYSTEM_LIBS_NLOHMANN_JSON} ${CONAN_FRAMEWORKS_FOUND_NLOHMANN_JSON} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_NLOHMANN_JSON}" "${CONAN_LIB_DIRS_NLOHMANN_JSON}"
                                  CONAN_PACKAGE_TARGETS_NLOHMANN_JSON "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES}"
                                  "" nlohmann_json)
    set(_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_NLOHMANN_JSON_DEBUG} ${CONAN_FRAMEWORKS_FOUND_NLOHMANN_JSON_DEBUG} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_NLOHMANN_JSON_DEBUG}" "${CONAN_LIB_DIRS_NLOHMANN_JSON_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_NLOHMANN_JSON_DEBUG "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_DEBUG}"
                                  "debug" nlohmann_json)
    set(_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_NLOHMANN_JSON_RELEASE} ${CONAN_FRAMEWORKS_FOUND_NLOHMANN_JSON_RELEASE} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_NLOHMANN_JSON_RELEASE}" "${CONAN_LIB_DIRS_NLOHMANN_JSON_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_NLOHMANN_JSON_RELEASE "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELEASE}"
                                  "release" nlohmann_json)
    set(_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_NLOHMANN_JSON_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_NLOHMANN_JSON_RELWITHDEBINFO} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_NLOHMANN_JSON_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_NLOHMANN_JSON_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_NLOHMANN_JSON_RELWITHDEBINFO "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" nlohmann_json)
    set(_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_NLOHMANN_JSON_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_NLOHMANN_JSON_MINSIZEREL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_NLOHMANN_JSON_MINSIZEREL}" "${CONAN_LIB_DIRS_NLOHMANN_JSON_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_NLOHMANN_JSON_MINSIZEREL "${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" nlohmann_json)

    add_library(CONAN_PKG::nlohmann_json INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::nlohmann_json PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_NLOHMANN_JSON} ${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_NLOHMANN_JSON_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_NLOHMANN_JSON_RELEASE} ${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_NLOHMANN_JSON_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_NLOHMANN_JSON_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_NLOHMANN_JSON_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_NLOHMANN_JSON_MINSIZEREL} ${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_NLOHMANN_JSON_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_NLOHMANN_JSON_DEBUG} ${_CONAN_PKG_LIBS_NLOHMANN_JSON_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_NLOHMANN_JSON_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_NLOHMANN_JSON_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::nlohmann_json PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_NLOHMANN_JSON}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_NLOHMANN_JSON_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_NLOHMANN_JSON_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_NLOHMANN_JSON_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_NLOHMANN_JSON_DEBUG}>)
    set_property(TARGET CONAN_PKG::nlohmann_json PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_NLOHMANN_JSON}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_NLOHMANN_JSON_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_NLOHMANN_JSON_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_NLOHMANN_JSON_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_NLOHMANN_JSON_DEBUG}>)
    set_property(TARGET CONAN_PKG::nlohmann_json PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_NLOHMANN_JSON_LIST} ${CONAN_CXX_FLAGS_NLOHMANN_JSON_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_NLOHMANN_JSON_RELEASE_LIST} ${CONAN_CXX_FLAGS_NLOHMANN_JSON_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_NLOHMANN_JSON_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_NLOHMANN_JSON_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_NLOHMANN_JSON_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_NLOHMANN_JSON_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_NLOHMANN_JSON_DEBUG_LIST}  ${CONAN_CXX_FLAGS_NLOHMANN_JSON_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES "${CONAN_SYSTEM_LIBS_LEVELDB} ${CONAN_FRAMEWORKS_FOUND_LEVELDB} CONAN_PKG::snappy CONAN_PKG::crc32c")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LEVELDB}" "${CONAN_LIB_DIRS_LEVELDB}"
                                  CONAN_PACKAGE_TARGETS_LEVELDB "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES}"
                                  "" leveldb)
    set(_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_LEVELDB_DEBUG} ${CONAN_FRAMEWORKS_FOUND_LEVELDB_DEBUG} CONAN_PKG::snappy CONAN_PKG::crc32c")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LEVELDB_DEBUG}" "${CONAN_LIB_DIRS_LEVELDB_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_LEVELDB_DEBUG "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_DEBUG}"
                                  "debug" leveldb)
    set(_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_LEVELDB_RELEASE} ${CONAN_FRAMEWORKS_FOUND_LEVELDB_RELEASE} CONAN_PKG::snappy CONAN_PKG::crc32c")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LEVELDB_RELEASE}" "${CONAN_LIB_DIRS_LEVELDB_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_LEVELDB_RELEASE "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELEASE}"
                                  "release" leveldb)
    set(_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_LEVELDB_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_LEVELDB_RELWITHDEBINFO} CONAN_PKG::snappy CONAN_PKG::crc32c")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LEVELDB_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_LEVELDB_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_LEVELDB_RELWITHDEBINFO "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" leveldb)
    set(_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_LEVELDB_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_LEVELDB_MINSIZEREL} CONAN_PKG::snappy CONAN_PKG::crc32c")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LEVELDB_MINSIZEREL}" "${CONAN_LIB_DIRS_LEVELDB_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_LEVELDB_MINSIZEREL "${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" leveldb)

    add_library(CONAN_PKG::leveldb INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::leveldb PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_LEVELDB} ${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LEVELDB_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_LEVELDB_RELEASE} ${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LEVELDB_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_LEVELDB_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LEVELDB_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_LEVELDB_MINSIZEREL} ${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LEVELDB_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_LEVELDB_DEBUG} ${_CONAN_PKG_LIBS_LEVELDB_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LEVELDB_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LEVELDB_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::leveldb PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_LEVELDB}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_LEVELDB_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_LEVELDB_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_LEVELDB_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_LEVELDB_DEBUG}>)
    set_property(TARGET CONAN_PKG::leveldb PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_LEVELDB}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_LEVELDB_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_LEVELDB_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_LEVELDB_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_LEVELDB_DEBUG}>)
    set_property(TARGET CONAN_PKG::leveldb PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_LEVELDB_LIST} ${CONAN_CXX_FLAGS_LEVELDB_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_LEVELDB_RELEASE_LIST} ${CONAN_CXX_FLAGS_LEVELDB_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_LEVELDB_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_LEVELDB_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_LEVELDB_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_LEVELDB_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_LEVELDB_DEBUG_LIST}  ${CONAN_CXX_FLAGS_LEVELDB_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_LMDB_DEPENDENCIES "${CONAN_SYSTEM_LIBS_LMDB} ${CONAN_FRAMEWORKS_FOUND_LMDB} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LMDB_DEPENDENCIES "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LMDB}" "${CONAN_LIB_DIRS_LMDB}"
                                  CONAN_PACKAGE_TARGETS_LMDB "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES}"
                                  "" lmdb)
    set(_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_LMDB_DEBUG} ${CONAN_FRAMEWORKS_FOUND_LMDB_DEBUG} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LMDB_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LMDB_DEBUG}" "${CONAN_LIB_DIRS_LMDB_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_LMDB_DEBUG "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_DEBUG}"
                                  "debug" lmdb)
    set(_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_LMDB_RELEASE} ${CONAN_FRAMEWORKS_FOUND_LMDB_RELEASE} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LMDB_RELEASE}" "${CONAN_LIB_DIRS_LMDB_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_LMDB_RELEASE "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELEASE}"
                                  "release" lmdb)
    set(_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_LMDB_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_LMDB_RELWITHDEBINFO} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LMDB_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_LMDB_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_LMDB_RELWITHDEBINFO "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" lmdb)
    set(_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_LMDB_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_LMDB_MINSIZEREL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_LMDB_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_LMDB_MINSIZEREL}" "${CONAN_LIB_DIRS_LMDB_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_LMDB_MINSIZEREL "${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" lmdb)

    add_library(CONAN_PKG::lmdb INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::lmdb PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_LMDB} ${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LMDB_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_LMDB_RELEASE} ${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LMDB_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_LMDB_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LMDB_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_LMDB_MINSIZEREL} ${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LMDB_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_LMDB_DEBUG} ${_CONAN_PKG_LIBS_LMDB_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_LMDB_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_LMDB_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::lmdb PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_LMDB}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_LMDB_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_LMDB_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_LMDB_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_LMDB_DEBUG}>)
    set_property(TARGET CONAN_PKG::lmdb PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_LMDB}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_LMDB_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_LMDB_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_LMDB_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_LMDB_DEBUG}>)
    set_property(TARGET CONAN_PKG::lmdb PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_LMDB_LIST} ${CONAN_CXX_FLAGS_LMDB_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_LMDB_RELEASE_LIST} ${CONAN_CXX_FLAGS_LMDB_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_LMDB_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_LMDB_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_LMDB_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_LMDB_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_LMDB_DEBUG_LIST}  ${CONAN_CXX_FLAGS_LMDB_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES "${CONAN_SYSTEM_LIBS_SNAPPY} ${CONAN_FRAMEWORKS_FOUND_SNAPPY} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SNAPPY}" "${CONAN_LIB_DIRS_SNAPPY}"
                                  CONAN_PACKAGE_TARGETS_SNAPPY "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES}"
                                  "" snappy)
    set(_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_SNAPPY_DEBUG} ${CONAN_FRAMEWORKS_FOUND_SNAPPY_DEBUG} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SNAPPY_DEBUG}" "${CONAN_LIB_DIRS_SNAPPY_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_SNAPPY_DEBUG "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_DEBUG}"
                                  "debug" snappy)
    set(_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_SNAPPY_RELEASE} ${CONAN_FRAMEWORKS_FOUND_SNAPPY_RELEASE} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SNAPPY_RELEASE}" "${CONAN_LIB_DIRS_SNAPPY_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_SNAPPY_RELEASE "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELEASE}"
                                  "release" snappy)
    set(_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_SNAPPY_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_SNAPPY_RELWITHDEBINFO} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SNAPPY_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_SNAPPY_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_SNAPPY_RELWITHDEBINFO "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" snappy)
    set(_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_SNAPPY_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_SNAPPY_MINSIZEREL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_SNAPPY_MINSIZEREL}" "${CONAN_LIB_DIRS_SNAPPY_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_SNAPPY_MINSIZEREL "${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" snappy)

    add_library(CONAN_PKG::snappy INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::snappy PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_SNAPPY} ${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SNAPPY_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_SNAPPY_RELEASE} ${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SNAPPY_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_SNAPPY_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SNAPPY_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_SNAPPY_MINSIZEREL} ${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SNAPPY_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_SNAPPY_DEBUG} ${_CONAN_PKG_LIBS_SNAPPY_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_SNAPPY_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_SNAPPY_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::snappy PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_SNAPPY}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_SNAPPY_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_SNAPPY_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_SNAPPY_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_SNAPPY_DEBUG}>)
    set_property(TARGET CONAN_PKG::snappy PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_SNAPPY}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_SNAPPY_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_SNAPPY_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_SNAPPY_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_SNAPPY_DEBUG}>)
    set_property(TARGET CONAN_PKG::snappy PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_SNAPPY_LIST} ${CONAN_CXX_FLAGS_SNAPPY_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_SNAPPY_RELEASE_LIST} ${CONAN_CXX_FLAGS_SNAPPY_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_SNAPPY_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_SNAPPY_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_SNAPPY_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_SNAPPY_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_SNAPPY_DEBUG_LIST}  ${CONAN_CXX_FLAGS_SNAPPY_DEBUG_LIST}>)


    set(_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES "${CONAN_SYSTEM_LIBS_CRC32C} ${CONAN_FRAMEWORKS_FOUND_CRC32C} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_CRC32C_DEPENDENCIES "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES}")
    conan_package_library_targets("${CONAN_PKG_LIBS_CRC32C}" "${CONAN_LIB_DIRS_CRC32C}"
                                  CONAN_PACKAGE_TARGETS_CRC32C "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES}"
                                  "" crc32c)
    set(_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_DEBUG "${CONAN_SYSTEM_LIBS_CRC32C_DEBUG} ${CONAN_FRAMEWORKS_FOUND_CRC32C_DEBUG} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_DEBUG "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_DEBUG}")
    conan_package_library_targets("${CONAN_PKG_LIBS_CRC32C_DEBUG}" "${CONAN_LIB_DIRS_CRC32C_DEBUG}"
                                  CONAN_PACKAGE_TARGETS_CRC32C_DEBUG "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_DEBUG}"
                                  "debug" crc32c)
    set(_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELEASE "${CONAN_SYSTEM_LIBS_CRC32C_RELEASE} ${CONAN_FRAMEWORKS_FOUND_CRC32C_RELEASE} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELEASE "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELEASE}")
    conan_package_library_targets("${CONAN_PKG_LIBS_CRC32C_RELEASE}" "${CONAN_LIB_DIRS_CRC32C_RELEASE}"
                                  CONAN_PACKAGE_TARGETS_CRC32C_RELEASE "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELEASE}"
                                  "release" crc32c)
    set(_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELWITHDEBINFO "${CONAN_SYSTEM_LIBS_CRC32C_RELWITHDEBINFO} ${CONAN_FRAMEWORKS_FOUND_CRC32C_RELWITHDEBINFO} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELWITHDEBINFO "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELWITHDEBINFO}")
    conan_package_library_targets("${CONAN_PKG_LIBS_CRC32C_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_CRC32C_RELWITHDEBINFO}"
                                  CONAN_PACKAGE_TARGETS_CRC32C_RELWITHDEBINFO "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELWITHDEBINFO}"
                                  "relwithdebinfo" crc32c)
    set(_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_MINSIZEREL "${CONAN_SYSTEM_LIBS_CRC32C_MINSIZEREL} ${CONAN_FRAMEWORKS_FOUND_CRC32C_MINSIZEREL} ")
    string(REPLACE " " ";" _CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_MINSIZEREL "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_MINSIZEREL}")
    conan_package_library_targets("${CONAN_PKG_LIBS_CRC32C_MINSIZEREL}" "${CONAN_LIB_DIRS_CRC32C_MINSIZEREL}"
                                  CONAN_PACKAGE_TARGETS_CRC32C_MINSIZEREL "${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_MINSIZEREL}"
                                  "minsizerel" crc32c)

    add_library(CONAN_PKG::crc32c INTERFACE IMPORTED)

    # Property INTERFACE_LINK_FLAGS do not work, necessary to add to INTERFACE_LINK_LIBRARIES
    set_property(TARGET CONAN_PKG::crc32c PROPERTY INTERFACE_LINK_LIBRARIES ${CONAN_PACKAGE_TARGETS_CRC32C} ${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_CRC32C_LIST}>

                                                                 $<$<CONFIG:Release>:${CONAN_PACKAGE_TARGETS_CRC32C_RELEASE} ${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELEASE}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_RELEASE_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_CRC32C_RELEASE_LIST}>>

                                                                 $<$<CONFIG:RelWithDebInfo>:${CONAN_PACKAGE_TARGETS_CRC32C_RELWITHDEBINFO} ${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_RELWITHDEBINFO}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_RELWITHDEBINFO_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_CRC32C_RELWITHDEBINFO_LIST}>>

                                                                 $<$<CONFIG:MinSizeRel>:${CONAN_PACKAGE_TARGETS_CRC32C_MINSIZEREL} ${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_MINSIZEREL}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_MINSIZEREL_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_CRC32C_MINSIZEREL_LIST}>>

                                                                 $<$<CONFIG:Debug>:${CONAN_PACKAGE_TARGETS_CRC32C_DEBUG} ${_CONAN_PKG_LIBS_CRC32C_DEPENDENCIES_DEBUG}
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${CONAN_SHARED_LINKER_FLAGS_CRC32C_DEBUG_LIST}>
                                                                 $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CONAN_EXE_LINKER_FLAGS_CRC32C_DEBUG_LIST}>>)
    set_property(TARGET CONAN_PKG::crc32c PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CONAN_INCLUDE_DIRS_CRC32C}
                                                                      $<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_CRC32C_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_CRC32C_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_CRC32C_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_CRC32C_DEBUG}>)
    set_property(TARGET CONAN_PKG::crc32c PROPERTY INTERFACE_COMPILE_DEFINITIONS ${CONAN_COMPILE_DEFINITIONS_CRC32C}
                                                                      $<$<CONFIG:Release>:${CONAN_COMPILE_DEFINITIONS_CRC32C_RELEASE}>
                                                                      $<$<CONFIG:RelWithDebInfo>:${CONAN_COMPILE_DEFINITIONS_CRC32C_RELWITHDEBINFO}>
                                                                      $<$<CONFIG:MinSizeRel>:${CONAN_COMPILE_DEFINITIONS_CRC32C_MINSIZEREL}>
                                                                      $<$<CONFIG:Debug>:${CONAN_COMPILE_DEFINITIONS_CRC32C_DEBUG}>)
    set_property(TARGET CONAN_PKG::crc32c PROPERTY INTERFACE_COMPILE_OPTIONS ${CONAN_C_FLAGS_CRC32C_LIST} ${CONAN_CXX_FLAGS_CRC32C_LIST}
                                                                  $<$<CONFIG:Release>:${CONAN_C_FLAGS_CRC32C_RELEASE_LIST} ${CONAN_CXX_FLAGS_CRC32C_RELEASE_LIST}>
                                                                  $<$<CONFIG:RelWithDebInfo>:${CONAN_C_FLAGS_CRC32C_RELWITHDEBINFO_LIST} ${CONAN_CXX_FLAGS_CRC32C_RELWITHDEBINFO_LIST}>
                                                                  $<$<CONFIG:MinSizeRel>:${CONAN_C_FLAGS_CRC32C_MINSIZEREL_LIST} ${CONAN_CXX_FLAGS_CRC32C_MINSIZEREL_LIST}>
                                                                  $<$<CONFIG:Debug>:${CONAN_C_FLAGS_CRC32C_DEBUG_LIST}  ${CONAN_CXX_FLAGS_CRC32C_DEBUG_LIST}>)

    set(CONAN_TARGETS CONAN_PKG::fmt CONAN_PKG::benchmark CONAN_PKG::nlohmann_json CONAN_PKG::leveldb CONAN_PKG::lmdb CONAN_PKG::snappy CONAN_PKG::crc32c)

endmacro()


macro(conan_basic_setup)
    set(options TARGETS NO_OUTPUT_DIRS SKIP_RPATH KEEP_RPATHS SKIP_STD SKIP_FPIC)
    cmake_parse_arguments(ARGUMENTS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if(CONAN_EXPORTED)
        conan_message(STATUS "Conan: called by CMake conan helper")
    endif()

    if(CONAN_IN_LOCAL_CACHE)
        conan_message(STATUS "Conan: called inside local cache")
    endif()

    if(NOT ARGUMENTS_NO_OUTPUT_DIRS)
        conan_message(STATUS "Conan: Adjusting output directories")
        conan_output_dirs_setup()
    endif()

    if(NOT ARGUMENTS_TARGETS)
        conan_message(STATUS "Conan: Using cmake global configuration")
        conan_global_flags()
    else()
        conan_message(STATUS "Conan: Using cmake targets configuration")
        conan_define_targets()
    endif()

    if(ARGUMENTS_SKIP_RPATH)
        # Change by "DEPRECATION" or "SEND_ERROR" when we are ready
        conan_message(WARNING "Conan: SKIP_RPATH is deprecated, it has been renamed to KEEP_RPATHS")
    endif()

    if(NOT ARGUMENTS_SKIP_RPATH AND NOT ARGUMENTS_KEEP_RPATHS)
        # Parameter has renamed, but we keep the compatibility with old SKIP_RPATH
        conan_set_rpath()
    endif()

    if(NOT ARGUMENTS_SKIP_STD)
        conan_set_std()
    endif()

    if(NOT ARGUMENTS_SKIP_FPIC)
        conan_set_fpic()
    endif()

    conan_check_compiler()
    conan_set_libcxx()
    conan_set_vs_runtime()
    conan_set_find_paths()
    conan_include_build_modules()
    conan_set_find_library_paths()
endmacro()


macro(conan_set_find_paths)
    # CMAKE_MODULE_PATH does not have Debug/Release config, but there are variables
    # CONAN_CMAKE_MODULE_PATH_DEBUG to be used by the consumer
    # CMake can find findXXX.cmake files in the root of packages
    set(CMAKE_MODULE_PATH ${CONAN_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})

    # Make find_package() to work
    set(CMAKE_PREFIX_PATH ${CONAN_CMAKE_MODULE_PATH} ${CMAKE_PREFIX_PATH})

    # Set the find root path (cross build)
    set(CMAKE_FIND_ROOT_PATH ${CONAN_CMAKE_FIND_ROOT_PATH} ${CMAKE_FIND_ROOT_PATH})
    if(CONAN_CMAKE_FIND_ROOT_PATH_MODE_PROGRAM)
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ${CONAN_CMAKE_FIND_ROOT_PATH_MODE_PROGRAM})
    endif()
    if(CONAN_CMAKE_FIND_ROOT_PATH_MODE_LIBRARY)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${CONAN_CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})
    endif()
    if(CONAN_CMAKE_FIND_ROOT_PATH_MODE_INCLUDE)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${CONAN_CMAKE_FIND_ROOT_PATH_MODE_INCLUDE})
    endif()
endmacro()


macro(conan_set_find_library_paths)
    # CMAKE_INCLUDE_PATH, CMAKE_LIBRARY_PATH does not have Debug/Release config, but there are variables
    # CONAN_INCLUDE_DIRS_DEBUG/RELEASE CONAN_LIB_DIRS_DEBUG/RELEASE to be used by the consumer
    # For find_library
    set(CMAKE_INCLUDE_PATH ${CONAN_INCLUDE_DIRS} ${CMAKE_INCLUDE_PATH})
    set(CMAKE_LIBRARY_PATH ${CONAN_LIB_DIRS} ${CMAKE_LIBRARY_PATH})
endmacro()


macro(conan_set_vs_runtime)
    if(CONAN_LINK_RUNTIME)
        conan_get_policy(CMP0091 policy_0091)
        if(policy_0091 STREQUAL "NEW")
            if(CONAN_LINK_RUNTIME MATCHES "MTd")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
            elseif(CONAN_LINK_RUNTIME MATCHES "MDd")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
            elseif(CONAN_LINK_RUNTIME MATCHES "MT")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
            elseif(CONAN_LINK_RUNTIME MATCHES "MD")
                set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
            endif()
        else()
            foreach(flag CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE
                         CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO
                         CMAKE_C_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_MINSIZEREL)
                if(DEFINED ${flag})
                    string(REPLACE "/MD" ${CONAN_LINK_RUNTIME} ${flag} "${${flag}}")
                endif()
            endforeach()
            foreach(flag CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
                if(DEFINED ${flag})
                    string(REPLACE "/MDd" ${CONAN_LINK_RUNTIME} ${flag} "${${flag}}")
                endif()
            endforeach()
        endif()
    endif()
endmacro()


macro(conan_flags_setup)
    # Macro maintained for backwards compatibility
    conan_set_find_library_paths()
    conan_global_flags()
    conan_set_rpath()
    conan_set_vs_runtime()
    conan_set_libcxx()
endmacro()


function(conan_message MESSAGE_OUTPUT)
    if(NOT CONAN_CMAKE_SILENT_OUTPUT)
        message(${ARGV${0}})
    endif()
endfunction()


function(conan_get_policy policy_id policy)
    if(POLICY "${policy_id}")
        cmake_policy(GET "${policy_id}" _policy)
        set(${policy} "${_policy}" PARENT_SCOPE)
    else()
        set(${policy} "" PARENT_SCOPE)
    endif()
endfunction()


function(conan_find_libraries_abs_path libraries package_libdir libraries_abs_path)
    foreach(_LIBRARY_NAME ${libraries})
        find_library(CONAN_FOUND_LIBRARY NAME ${_LIBRARY_NAME} PATHS ${package_libdir}
                     NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(CONAN_FOUND_LIBRARY)
            conan_message(STATUS "Library ${_LIBRARY_NAME} found ${CONAN_FOUND_LIBRARY}")
            set(CONAN_FULLPATH_LIBS ${CONAN_FULLPATH_LIBS} ${CONAN_FOUND_LIBRARY})
        else()
            conan_message(STATUS "Library ${_LIBRARY_NAME} not found in package, might be system one")
            set(CONAN_FULLPATH_LIBS ${CONAN_FULLPATH_LIBS} ${_LIBRARY_NAME})
        endif()
        unset(CONAN_FOUND_LIBRARY CACHE)
    endforeach()
    set(${libraries_abs_path} ${CONAN_FULLPATH_LIBS} PARENT_SCOPE)
endfunction()


function(conan_package_library_targets libraries package_libdir libraries_abs_path deps build_type package_name)
    unset(_CONAN_ACTUAL_TARGETS CACHE)
    unset(_CONAN_FOUND_SYSTEM_LIBS CACHE)
    foreach(_LIBRARY_NAME ${libraries})
        find_library(CONAN_FOUND_LIBRARY NAME ${_LIBRARY_NAME} PATHS ${package_libdir}
                     NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(CONAN_FOUND_LIBRARY)
            conan_message(STATUS "Library ${_LIBRARY_NAME} found ${CONAN_FOUND_LIBRARY}")
            set(_LIB_NAME CONAN_LIB::${package_name}_${_LIBRARY_NAME}${build_type})
            add_library(${_LIB_NAME} UNKNOWN IMPORTED)
            set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_LOCATION ${CONAN_FOUND_LIBRARY})
            set(CONAN_FULLPATH_LIBS ${CONAN_FULLPATH_LIBS} ${_LIB_NAME})
            set(_CONAN_ACTUAL_TARGETS ${_CONAN_ACTUAL_TARGETS} ${_LIB_NAME})
        else()
            conan_message(STATUS "Library ${_LIBRARY_NAME} not found in package, might be system one")
            set(CONAN_FULLPATH_LIBS ${CONAN_FULLPATH_LIBS} ${_LIBRARY_NAME})
            set(_CONAN_FOUND_SYSTEM_LIBS "${_CONAN_FOUND_SYSTEM_LIBS};${_LIBRARY_NAME}")
        endif()
        unset(CONAN_FOUND_LIBRARY CACHE)
    endforeach()

    # Add all dependencies to all targets
    string(REPLACE " " ";" deps_list "${deps}")
    foreach(_CONAN_ACTUAL_TARGET ${_CONAN_ACTUAL_TARGETS})
        set_property(TARGET ${_CONAN_ACTUAL_TARGET} PROPERTY INTERFACE_LINK_LIBRARIES "${_CONAN_FOUND_SYSTEM_LIBS};${deps_list}")
    endforeach()

    set(${libraries_abs_path} ${CONAN_FULLPATH_LIBS} PARENT_SCOPE)
endfunction()


macro(conan_set_libcxx)
    if(DEFINED CONAN_LIBCXX)
        conan_message(STATUS "Conan: C++ stdlib: ${CONAN_LIBCXX}")
        if(CONAN_COMPILER STREQUAL "clang" OR CONAN_COMPILER STREQUAL "apple-clang")
            if(CONAN_LIBCXX STREQUAL "libstdc++" OR CONAN_LIBCXX STREQUAL "libstdc++11" )
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
            elseif(CONAN_LIBCXX STREQUAL "libc++")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            endif()
        endif()
        if(CONAN_COMPILER STREQUAL "sun-cc")
            if(CONAN_LIBCXX STREQUAL "libCstd")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=Cstd")
            elseif(CONAN_LIBCXX STREQUAL "libstdcxx")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stdcxx4")
            elseif(CONAN_LIBCXX STREQUAL "libstlport")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stlport4")
            elseif(CONAN_LIBCXX STREQUAL "libstdc++")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stdcpp")
            endif()
        endif()
        if(CONAN_LIBCXX STREQUAL "libstdc++11")
            add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
        elseif(CONAN_LIBCXX STREQUAL "libstdc++")
            add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
        endif()
    endif()
endmacro()


macro(conan_set_std)
    conan_message(STATUS "Conan: Adjusting language standard")
    # Do not warn "Manually-specified variables were not used by the project"
    set(ignorevar "${CONAN_STD_CXX_FLAG}${CONAN_CMAKE_CXX_STANDARD}${CONAN_CMAKE_CXX_EXTENSIONS}")
    if (CMAKE_VERSION VERSION_LESS "3.1" OR
        (CMAKE_VERSION VERSION_LESS "3.12" AND ("${CONAN_CMAKE_CXX_STANDARD}" STREQUAL "20" OR "${CONAN_CMAKE_CXX_STANDARD}" STREQUAL "gnu20")))
        if(CONAN_STD_CXX_FLAG)
            conan_message(STATUS "Conan setting CXX_FLAGS flags: ${CONAN_STD_CXX_FLAG}")
            set(CMAKE_CXX_FLAGS "${CONAN_STD_CXX_FLAG} ${CMAKE_CXX_FLAGS}")
        endif()
    else()
        if(CONAN_CMAKE_CXX_STANDARD)
            conan_message(STATUS "Conan setting CPP STANDARD: ${CONAN_CMAKE_CXX_STANDARD} WITH EXTENSIONS ${CONAN_CMAKE_CXX_EXTENSIONS}")
            set(CMAKE_CXX_STANDARD ${CONAN_CMAKE_CXX_STANDARD})
            set(CMAKE_CXX_EXTENSIONS ${CONAN_CMAKE_CXX_EXTENSIONS})
        endif()
    endif()
endmacro()


macro(conan_set_rpath)
    conan_message(STATUS "Conan: Adjusting default RPATHs Conan policies")
    if(APPLE)
        # https://cmake.org/Wiki/CMake_RPATH_handling
        # CONAN GUIDE: All generated libraries should have the id and dependencies to other
        # dylibs without path, just the name, EX:
        # libMyLib1.dylib:
        #     libMyLib1.dylib (compatibility version 0.0.0, current version 0.0.0)
        #     libMyLib0.dylib (compatibility version 0.0.0, current version 0.0.0)
        #     /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 120.0.0)
        #     /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1197.1.1)
        # AVOID RPATH FOR *.dylib, ALL LIBS BETWEEN THEM AND THE EXE
        # SHOULD BE ON THE LINKER RESOLVER PATH (./ IS ONE OF THEM)
        set(CMAKE_SKIP_RPATH 1 CACHE BOOL "rpaths" FORCE)
        # Policy CMP0068
        # We want the old behavior, in CMake >= 3.9 CMAKE_SKIP_RPATH won't affect the install_name in OSX
        set(CMAKE_INSTALL_NAME_DIR "")
    endif()
endmacro()


macro(conan_set_fpic)
    if(DEFINED CONAN_CMAKE_POSITION_INDEPENDENT_CODE)
        conan_message(STATUS "Conan: Adjusting fPIC flag (${CONAN_CMAKE_POSITION_INDEPENDENT_CODE})")
        set(CMAKE_POSITION_INDEPENDENT_CODE ${CONAN_CMAKE_POSITION_INDEPENDENT_CODE})
    endif()
endmacro()


macro(conan_output_dirs_setup)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endmacro()


macro(conan_split_version VERSION_STRING MAJOR MINOR)
    #make a list from the version string
    string(REPLACE "." ";" VERSION_LIST "${VERSION_STRING}")

    #write output values
    list(LENGTH VERSION_LIST _version_len)
    list(GET VERSION_LIST 0 ${MAJOR})
    if(${_version_len} GREATER 1)
        list(GET VERSION_LIST 1 ${MINOR})
    endif()
endmacro()


macro(conan_error_compiler_version)
    message(FATAL_ERROR "Detected a mismatch for the compiler version between your conan profile settings and CMake: \n"
                        "Compiler version specified in your conan profile: ${CONAN_COMPILER_VERSION}\n"
                        "Compiler version detected in CMake: ${VERSION_MAJOR}.${VERSION_MINOR}\n"
                        "Please check your conan profile settings (conan profile show [default|your_profile_name])\n"
                        "P.S. You may set CONAN_DISABLE_CHECK_COMPILER CMake variable in order to disable this check."
           )
endmacro()

set(_CONAN_CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR})

function(conan_get_compiler CONAN_INFO_COMPILER CONAN_INFO_COMPILER_VERSION)
    conan_message(STATUS "Current conanbuildinfo.cmake directory: " ${_CONAN_CURRENT_DIR})
    if(NOT EXISTS ${_CONAN_CURRENT_DIR}/conaninfo.txt)
        conan_message(STATUS "WARN: conaninfo.txt not found")
        return()
    endif()

    file (READ "${_CONAN_CURRENT_DIR}/conaninfo.txt" CONANINFO)

    # MATCHALL will match all, including the last one, which is the full_settings one
    string(REGEX MATCH "full_settings.*" _FULL_SETTINGS_MATCHED ${CONANINFO})
    string(REGEX MATCH "compiler=([-A-Za-z0-9_ ]+)" _MATCHED ${_FULL_SETTINGS_MATCHED})
    if(DEFINED CMAKE_MATCH_1)
        string(STRIP "${CMAKE_MATCH_1}" _CONAN_INFO_COMPILER)
        set(${CONAN_INFO_COMPILER} ${_CONAN_INFO_COMPILER} PARENT_SCOPE)
    endif()

    string(REGEX MATCH "compiler.version=([-A-Za-z0-9_.]+)" _MATCHED ${_FULL_SETTINGS_MATCHED})
    if(DEFINED CMAKE_MATCH_1)
        string(STRIP "${CMAKE_MATCH_1}" _CONAN_INFO_COMPILER_VERSION)
        set(${CONAN_INFO_COMPILER_VERSION} ${_CONAN_INFO_COMPILER_VERSION} PARENT_SCOPE)
    endif()
endfunction()


function(check_compiler_version)
    conan_split_version(${CMAKE_CXX_COMPILER_VERSION} VERSION_MAJOR VERSION_MINOR)
    if(DEFINED CONAN_SETTINGS_COMPILER_TOOLSET)
       conan_message(STATUS "Conan: Skipping compiler check: Declared 'compiler.toolset'")
       return()
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        # MSVC_VERSION is defined since 2.8.2 at least
        # https://cmake.org/cmake/help/v2.8.2/cmake.html#variable:MSVC_VERSION
        # https://cmake.org/cmake/help/v3.14/variable/MSVC_VERSION.html
        if(
            # 1930 = VS 17.0 (v143 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "17" AND NOT((MSVC_VERSION EQUAL 1930) OR (MSVC_VERSION GREATER 1930))) OR
            # 1920-1929 = VS 16.0 (v142 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "16" AND NOT((MSVC_VERSION GREATER 1919) AND (MSVC_VERSION LESS 1930))) OR
            # 1910-1919 = VS 15.0 (v141 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "15" AND NOT((MSVC_VERSION GREATER 1909) AND (MSVC_VERSION LESS 1920))) OR
            # 1900      = VS 14.0 (v140 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "14" AND NOT(MSVC_VERSION EQUAL 1900)) OR
            # 1800      = VS 12.0 (v120 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "12" AND NOT VERSION_MAJOR STREQUAL "18") OR
            # 1700      = VS 11.0 (v110 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "11" AND NOT VERSION_MAJOR STREQUAL "17") OR
            # 1600      = VS 10.0 (v100 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "10" AND NOT VERSION_MAJOR STREQUAL "16") OR
            # 1500      = VS  9.0 (v90 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "9" AND NOT VERSION_MAJOR STREQUAL "15") OR
            # 1400      = VS  8.0 (v80 toolset)
            (CONAN_COMPILER_VERSION STREQUAL "8" AND NOT VERSION_MAJOR STREQUAL "14") OR
            # 1310      = VS  7.1, 1300      = VS  7.0
            (CONAN_COMPILER_VERSION STREQUAL "7" AND NOT VERSION_MAJOR STREQUAL "13") OR
            # 1200      = VS  6.0
            (CONAN_COMPILER_VERSION STREQUAL "6" AND NOT VERSION_MAJOR STREQUAL "12") )
            conan_error_compiler_version()
        endif()
    elseif(CONAN_COMPILER STREQUAL "gcc")
        conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
        set(_CHECK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
        set(_CONAN_VERSION ${CONAN_COMPILER_MAJOR}.${CONAN_COMPILER_MINOR})
        if(NOT ${CONAN_COMPILER_VERSION} VERSION_LESS 5.0)
            conan_message(STATUS "Conan: Compiler GCC>=5, checking major version ${CONAN_COMPILER_VERSION}")
            conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
            if("${CONAN_COMPILER_MINOR}" STREQUAL "")
                set(_CHECK_VERSION ${VERSION_MAJOR})
                set(_CONAN_VERSION ${CONAN_COMPILER_MAJOR})
            endif()
        endif()
        conan_message(STATUS "Conan: Checking correct version: ${_CHECK_VERSION}")
        if(NOT ${_CHECK_VERSION} VERSION_EQUAL ${_CONAN_VERSION})
            conan_error_compiler_version()
        endif()
    elseif(CONAN_COMPILER STREQUAL "clang")
        conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
        set(_CHECK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
        set(_CONAN_VERSION ${CONAN_COMPILER_MAJOR}.${CONAN_COMPILER_MINOR})
        if(NOT ${CONAN_COMPILER_VERSION} VERSION_LESS 8.0)
            conan_message(STATUS "Conan: Compiler Clang>=8, checking major version ${CONAN_COMPILER_VERSION}")
            if("${CONAN_COMPILER_MINOR}" STREQUAL "")
                set(_CHECK_VERSION ${VERSION_MAJOR})
                set(_CONAN_VERSION ${CONAN_COMPILER_MAJOR})
            endif()
        endif()
        conan_message(STATUS "Conan: Checking correct version: ${_CHECK_VERSION}")
        if(NOT ${_CHECK_VERSION} VERSION_EQUAL ${_CONAN_VERSION})
            conan_error_compiler_version()
        endif()
    elseif(CONAN_COMPILER STREQUAL "apple-clang" OR CONAN_COMPILER STREQUAL "sun-cc" OR CONAN_COMPILER STREQUAL "mcst-lcc")
        conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
        if(${CONAN_COMPILER_MAJOR} VERSION_GREATER_EQUAL "13" AND "${CONAN_COMPILER_MINOR}" STREQUAL "" AND ${CONAN_COMPILER_MAJOR} VERSION_EQUAL ${VERSION_MAJOR})
           # This is correct,  13.X is considered 13
        elseif(NOT ${VERSION_MAJOR}.${VERSION_MINOR} VERSION_EQUAL ${CONAN_COMPILER_MAJOR}.${CONAN_COMPILER_MINOR})
           conan_error_compiler_version()
        endif()
    elseif(CONAN_COMPILER STREQUAL "intel")
        conan_split_version(${CONAN_COMPILER_VERSION} CONAN_COMPILER_MAJOR CONAN_COMPILER_MINOR)
        if(NOT ${CONAN_COMPILER_VERSION} VERSION_LESS 19.1)
            if(NOT ${VERSION_MAJOR}.${VERSION_MINOR} VERSION_EQUAL ${CONAN_COMPILER_MAJOR}.${CONAN_COMPILER_MINOR})
               conan_error_compiler_version()
            endif()
        else()
            if(NOT ${VERSION_MAJOR} VERSION_EQUAL ${CONAN_COMPILER_MAJOR})
               conan_error_compiler_version()
            endif()
        endif()
    else()
        conan_message(STATUS "WARN: Unknown compiler '${CONAN_COMPILER}', skipping the version check...")
    endif()
endfunction()


function(conan_check_compiler)
    if(CONAN_DISABLE_CHECK_COMPILER)
        conan_message(STATUS "WARN: Disabled conan compiler checks")
        return()
    endif()
    if(NOT DEFINED CMAKE_CXX_COMPILER_ID)
        if(DEFINED CMAKE_C_COMPILER_ID)
            conan_message(STATUS "This project seems to be plain C, using '${CMAKE_C_COMPILER_ID}' compiler")
            set(CMAKE_CXX_COMPILER_ID ${CMAKE_C_COMPILER_ID})
            set(CMAKE_CXX_COMPILER_VERSION ${CMAKE_C_COMPILER_VERSION})
        else()
            message(FATAL_ERROR "This project seems to be plain C, but no compiler defined")
        endif()
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID AND NOT CMAKE_C_COMPILER_ID)
        # This use case happens when compiler is not identified by CMake, but the compilers are there and work
        conan_message(STATUS "*** WARN: CMake was not able to identify a C or C++ compiler ***")
        conan_message(STATUS "*** WARN: Disabling compiler checks. Please make sure your settings match your environment ***")
        return()
    endif()
    if(NOT DEFINED CONAN_COMPILER)
        conan_get_compiler(CONAN_COMPILER CONAN_COMPILER_VERSION)
        if(NOT DEFINED CONAN_COMPILER)
            conan_message(STATUS "WARN: CONAN_COMPILER variable not set, please make sure yourself that "
                          "your compiler and version matches your declared settings")
            return()
        endif()
    endif()

    if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL ${CMAKE_SYSTEM_NAME})
        set(CROSS_BUILDING 1)
    endif()

    # If using VS, verify toolset
    if (CONAN_COMPILER STREQUAL "Visual Studio")
        if (CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "LLVM" OR
            CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "llvm" OR
            CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "clang" OR
            CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "Clang")
            set(EXPECTED_CMAKE_CXX_COMPILER_ID "Clang")
        elseif (CONAN_SETTINGS_COMPILER_TOOLSET MATCHES "Intel")
            set(EXPECTED_CMAKE_CXX_COMPILER_ID "Intel")
        else()
            set(EXPECTED_CMAKE_CXX_COMPILER_ID "MSVC")
        endif()

        if (NOT CMAKE_CXX_COMPILER_ID MATCHES ${EXPECTED_CMAKE_CXX_COMPILER_ID})
            message(FATAL_ERROR "Incorrect '${CONAN_COMPILER}'. Toolset specifies compiler as '${EXPECTED_CMAKE_CXX_COMPILER_ID}' "
                                "but CMake detected '${CMAKE_CXX_COMPILER_ID}'")
        endif()

    # Avoid checks when cross compiling, apple-clang crashes because its APPLE but not apple-clang
    # Actually CMake is detecting "clang" when you are using apple-clang, only if CMP0025 is set to NEW will detect apple-clang
    elseif((CONAN_COMPILER STREQUAL "gcc" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR
        (CONAN_COMPILER STREQUAL "apple-clang" AND NOT CROSS_BUILDING AND (NOT APPLE OR NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")) OR
        (CONAN_COMPILER STREQUAL "clang" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR
        (CONAN_COMPILER STREQUAL "sun-cc" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "SunPro") )
        message(FATAL_ERROR "Incorrect '${CONAN_COMPILER}', is not the one detected by CMake: '${CMAKE_CXX_COMPILER_ID}'")
    endif()


    if(NOT DEFINED CONAN_COMPILER_VERSION)
        conan_message(STATUS "WARN: CONAN_COMPILER_VERSION variable not set, please make sure yourself "
                             "that your compiler version matches your declared settings")
        return()
    endif()
    check_compiler_version()
endfunction()


macro(conan_set_flags build_type)
    set(CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}} ${CONAN_CXX_FLAGS${build_type}}")
    set(CMAKE_C_FLAGS${build_type} "${CMAKE_C_FLAGS${build_type}} ${CONAN_C_FLAGS${build_type}}")
    set(CMAKE_SHARED_LINKER_FLAGS${build_type} "${CMAKE_SHARED_LINKER_FLAGS${build_type}} ${CONAN_SHARED_LINKER_FLAGS${build_type}}")
    set(CMAKE_EXE_LINKER_FLAGS${build_type} "${CMAKE_EXE_LINKER_FLAGS${build_type}} ${CONAN_EXE_LINKER_FLAGS${build_type}}")
endmacro()


macro(conan_global_flags)
    if(CONAN_SYSTEM_INCLUDES)
        include_directories(SYSTEM ${CONAN_INCLUDE_DIRS}
                                   "$<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_RELEASE}>"
                                   "$<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_RELWITHDEBINFO}>"
                                   "$<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_MINSIZEREL}>"
                                   "$<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DEBUG}>")
    else()
        include_directories(${CONAN_INCLUDE_DIRS}
                            "$<$<CONFIG:Release>:${CONAN_INCLUDE_DIRS_RELEASE}>"
                            "$<$<CONFIG:RelWithDebInfo>:${CONAN_INCLUDE_DIRS_RELWITHDEBINFO}>"
                            "$<$<CONFIG:MinSizeRel>:${CONAN_INCLUDE_DIRS_MINSIZEREL}>"
                            "$<$<CONFIG:Debug>:${CONAN_INCLUDE_DIRS_DEBUG}>")
    endif()

    link_directories(${CONAN_LIB_DIRS})

    conan_find_libraries_abs_path("${CONAN_LIBS_DEBUG}" "${CONAN_LIB_DIRS_DEBUG}"
                                  CONAN_LIBS_DEBUG)
    conan_find_libraries_abs_path("${CONAN_LIBS_RELEASE}" "${CONAN_LIB_DIRS_RELEASE}"
                                  CONAN_LIBS_RELEASE)
    conan_find_libraries_abs_path("${CONAN_LIBS_RELWITHDEBINFO}" "${CONAN_LIB_DIRS_RELWITHDEBINFO}"
                                  CONAN_LIBS_RELWITHDEBINFO)
    conan_find_libraries_abs_path("${CONAN_LIBS_MINSIZEREL}" "${CONAN_LIB_DIRS_MINSIZEREL}"
                                  CONAN_LIBS_MINSIZEREL)

    add_compile_options(${CONAN_DEFINES}
                        "$<$<CONFIG:Debug>:${CONAN_DEFINES_DEBUG}>"
                        "$<$<CONFIG:Release>:${CONAN_DEFINES_RELEASE}>"
                        "$<$<CONFIG:RelWithDebInfo>:${CONAN_DEFINES_RELWITHDEBINFO}>"
                        "$<$<CONFIG:MinSizeRel>:${CONAN_DEFINES_MINSIZEREL}>")

    conan_set_flags("")
    conan_set_flags("_RELEASE")
    conan_set_flags("_DEBUG")

endmacro()


macro(conan_target_link_libraries target)
    if(CONAN_TARGETS)
        target_link_libraries(${target} ${CONAN_TARGETS})
    else()
        target_link_libraries(${target} ${CONAN_LIBS})
        foreach(_LIB ${CONAN_LIBS_RELEASE})
            target_link_libraries(${target} optimized ${_LIB})
        endforeach()
        foreach(_LIB ${CONAN_LIBS_DEBUG})
            target_link_libraries(${target} debug ${_LIB})
        endforeach()
    endif()
endmacro()


macro(conan_include_build_modules)
    if(CMAKE_BUILD_TYPE)
        if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
            set(CONAN_BUILD_MODULES_PATHS ${CONAN_BUILD_MODULES_PATHS_DEBUG} ${CONAN_BUILD_MODULES_PATHS})
        elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
            set(CONAN_BUILD_MODULES_PATHS ${CONAN_BUILD_MODULES_PATHS_RELEASE} ${CONAN_BUILD_MODULES_PATHS})
        elseif(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
            set(CONAN_BUILD_MODULES_PATHS ${CONAN_BUILD_MODULES_PATHS_RELWITHDEBINFO} ${CONAN_BUILD_MODULES_PATHS})
        elseif(${CMAKE_BUILD_TYPE} MATCHES "MinSizeRel")
            set(CONAN_BUILD_MODULES_PATHS ${CONAN_BUILD_MODULES_PATHS_MINSIZEREL} ${CONAN_BUILD_MODULES_PATHS})
        endif()
    endif()

    foreach(_BUILD_MODULE_PATH ${CONAN_BUILD_MODULES_PATHS})
        include(${_BUILD_MODULE_PATH})
    endforeach()
endmacro()


### Definition of user declared vars (user_info) ###

