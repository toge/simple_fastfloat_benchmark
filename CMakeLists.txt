cmake_minimum_required(VERSION 3.15)

project(SimpleFastFloatBenchmark VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

if (MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
endif()

## optimization
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox")
endif()

include(FetchContent)
set(ABSL_ENABLE_INSTALL ON)
set(ABSL_RUN_TEST OFF CACHE INTERNAL "")
set(ABSL_USE_GOOGLETEST_HEAD OFF CACHE INTERNAL "")

FetchContent_Declare(abseil 
     GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git 
     GIT_TAG "20210324.1")
FetchContent_GetProperties(abseil)
if(NOT abseil_POPULATED)
    set(BUILD_TESTING OFF)
    FetchContent_Populate(abseil)
    add_subdirectory(${abseil_SOURCE_DIR} ${abseil_BINARY_DIR})
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${abseil_SOURCE_DIR}/absl/copts)
    include(${abseil_SOURCE_DIR}/absl/copts/AbseilConfigureCopts.cmake)
endif()

FetchContent_Declare(
  fast_float
  GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
  GIT_TAG "v1.0.0"
  GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(fast_float)

FetchContent_Declare(doubleconversion 
     GIT_REPOSITORY https://github.com/google/double-conversion.git 
     GIT_TAG "v3.1.5")
FetchContent_GetProperties(doubleconversion)
FetchContent_MakeAvailable(doubleconversion)


FetchContent_Declare(boost
    URL https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz)
FetchContent_GetProperties(boost)
FetchContent_MakeAvailable(boost)

include_directories(${boost_SOURCE_DIR})

FetchContent_Declare(ssp
    GIT_REPOSITORY https://github.com/toge/ssp.git
)
FetchContent_GetProperties(ssp)
FetchContent_MakeAvailable(ssp)

include_directories(${ssp_SOURCE_DIR}/include)

if (MSVC OR FROM_CHARS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFROM_CHARS_AVAILABLE_MAYBE")
endif()

add_subdirectory(benchmarks)
