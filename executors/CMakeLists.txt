# -DCMAKE_BUILD_TYPE=Debug
# -DCMAKE_BUILD_TYPE=Release
# -DCMAKE_VERBOSE_MAKEFILE=ON
# -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++
#
# ASan:
#  https://software.intel.com/en-us/articles/address-sanitizer-asan-tool-to-debug-memory-corruptions-in-chrome-os
#  http://www.youtube.com/watch?v=V2_80g0eOMc
#  http://www.youtube.com/watch?v=capbD_aRz40

project(actors_and_workers)
cmake_minimum_required(VERSION 2.8)

# conf
set (LCB_USE_TSAN Y)


set(Extern "../../extern/")
set(GMock "../../extern/gmock-1.6.0/fused-src")
set(Source "source/")
set(Tests "./tests")
set(BigExtern "/opt/big-3rdparty/")


# Test framework
include_directories(${GMock})
include_directories(${Source})
include_directories(${BigExtern}/boost_1_57_0)
include_directories(/opt/folly/include)
include_directories(${Source}/../../)


# link the libraries to the executable
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    # Need boost 1.54
    link_directories(${BigExtern}/../folly/lib)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        IF(LCB_USE_TSAN)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -fsanitize=thread")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -fsanitize=thread")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}  -fsanitize=thread")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
            link_directories(${BigExtern}/boost_1_57_0/clang_tsan_lib64/lib)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -analyse")
        endif()

        link_libraries(
            boost_system boost_thread
            folly
            glog
            pthread rt)
    else()
        IF(LCB_USE_TSAN)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -fsanitize=thread -fPIC")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -fsanitize=thread -fPIC")
            #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}  -fsanitize=thread")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread -pie")

            link_directories(${BigExtern}/boost_1_57_0/gcc_tsan_lib64/lib)
        endif()
        link_libraries(
            boost_system boost_thread
            tsan
            folly
            glog
            pthread rt)  # !! pthread and rt last!!
    endif()
endif()

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
file( GLOB set0
    ${Source}/mains/*.cc
    ${Source}/actors_and_workers/*.h
    ${Source}/actors_and_workers/*.cc
    ${Tests}/*.cc
    ${Tests}/sean_parent/*.cc
    ${Tests}/sean_parent/*.cpp)


add_executable(unittest.bin
    ${GMock}/gmock-gtest-all.cc
    ${set0}
)


