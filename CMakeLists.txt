cmake_minimum_required(VERSION 3.5...4.0)
project(KMER_SETS_BENCHMARK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

# === Step 1: Add SBWT and SSHash as subdirectories ===
# This ensures SBWT gets built.
add_subdirectory(external/SBWT ${CMAKE_BINARY_DIR}/sbwt_build)
# This ensures SSHash gets built.
add_subdirectory(external/sshash ${CMAKE_BINARY_DIR}/sshash_build)

# Add ESSENTIALS
add_subdirectory(external/essentials)

# === Step 2: Include directories ===
include_directories(
    ${CMAKE_SOURCE_DIR}/external/SBWT/SeqIO/include
    ${CMAKE_SOURCE_DIR}/external/SBWT/include/sbwt
    ${CMAKE_BINARY_DIR}/sbwt_build/external/sdsl-lite/build/external/libdivsufsort/include
    ${CMAKE_SOURCE_DIR}/external/sshash/include
)

if (UNIX)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

endif()

MESSAGE(STATUS "Compiling for processor: ${CMAKE_SYSTEM_PROCESSOR}")
MESSAGE(STATUS "Compiling with flags:${CMAKE_CXX_FLAGS}")

# === Step 3: Executable ===
add_executable(sbwt_benchmark src/sbwt_benchmark.cpp)
add_executable(sshash_benchmark src/sshash_benchmark.cpp)

# === Step 4: Link libraries ===

target_link_libraries(sbwt_benchmark
    sbwt_static
    ${CMAKE_BINARY_DIR}/sbwt_build/external/KMC/build/libkmc_core.a
    ${CMAKE_BINARY_DIR}/sbwt_build/external/KMC/build/libkmc_tools.a
    sdsl
    ESSENTIALS
)

target_link_libraries(sshash_benchmark
    sshash_static
    ESSENTIALS
)
