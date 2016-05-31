# -DCMAKE_BUILD_TYPE=Debug
# -DCMAKE_BUILD_TYPE=Release
# -DCMAKE_VERBOSE_MAKEFILE=ON
# -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++
#
# ASan:
#  https://software.intel.com/en-us/articles/address-sanitizer-asan-tool-to-debug-memory-corruptions-in-chrome-os
#  http://www.youtube.com/watch?v=V2_80g0eOMc
#  http://www.youtube.com/watch?v=capbD_aRz40

project(chromium-base-adapt)
cmake_minimum_required(VERSION 2.8)

#set(Extern "../../extern/")
#set(GMock "../../extern/gmock-1.6.0/fused-src")
set(Source "source/")
set(Tests "./tests")
set(BigExtern "/opt/big-3rdparty/")
set(ChromiumRoot "/home/zaqwes/mnt/ssd_hard/src")
set(ChromiumTesting "/home/zaqwes/mnt/ssd_hard/src/testing")

# conf
set (LCB_USE_TSAN Y)

# Test framework
include_directories(${ChromiumRoot})
include_directories(${ChromiumTesting}/gmock/include)
include_directories(${ChromiumTesting}/gmock)
include_directories(${ChromiumTesting}/gtest/include)

include_directories(${Source})
include_directories(${BigExtern}/boost_1_57_0)

# link the libraries to the executable
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")  # need C++11

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
            pthread rt)  # !! pthread and rt last!!
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
            pthread rt)  # !! pthread and rt last!!
    endif()
endif()

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
file( GLOB set0
    ${Source}/mains/*.cc)

file( GLOB chromiumTests
# Units
${ChromiumRoot}/base/bind_unittest.cc
${ChromiumRoot}/base/deferred_sequenced_task_runner_unittest.cc

# Deps
${ChromiumRoot}/base/deferred_sequenced_task_runner.cc
${ChromiumRoot}/base/task_runner.cc

${ChromiumRoot}/base/memory/ref_counted.cc
${ChromiumRoot}/base/memory/weak_ptr.cc
${ChromiumRoot}/base/memory/singleton.cc

${ChromiumRoot}/base/callback*.cc
${ChromiumRoot}/base/logging.cc
${ChromiumRoot}/base/*switches.cc
${ChromiumRoot}/base/*command_line.cc
${ChromiumRoot}/base/vlog.cc
${ChromiumRoot}/base/synchronization/lock.cc
${ChromiumRoot}/base/synchronization/lock_impl_posix.cc
${ChromiumRoot}/base/synchronization/waitable_event_posix.cc
${ChromiumRoot}/base/synchronization/condition_variable_posix.cc

${ChromiumRoot}/base/strings/utf_string_conversions.cc
${ChromiumRoot}/base/strings/utf_string_conversion_utils.cc
${ChromiumRoot}/base/strings/string_util.cc
${ChromiumRoot}/base/strings/string_piece.cc
${ChromiumRoot}/base/strings/string16.cc
${ChromiumRoot}/base/strings/string_split.cc
${ChromiumRoot}/base/strings/sys_string_conversions_posix.cc
${ChromiumRoot}/base/strings/string_number_conversions.cc
${ChromiumRoot}/base/strings/string_util_constants.cc
${ChromiumRoot}/base/strings/stringprintf.cc

${ChromiumRoot}/base/debug/alias.cc
${ChromiumRoot}/base/debug/debugger.cc
${ChromiumRoot}/base/debug/debugger_posix.cc

${ChromiumRoot}/base/process/process_handle_posix.cc
${ChromiumRoot}/base/process/process_metrics.cc
${ChromiumRoot}/base/process/process_metrics_linux.cc
${ChromiumRoot}/base/process/internal_linux.cc

${ChromiumRoot}/base/threading/platform_thread_linux.cc
${ChromiumRoot}/base/threading/platform_thread_posix.cc
${ChromiumRoot}/base/threading/thread_id_name_manager.cc
${ChromiumRoot}/base/threading/thread_local_storage.cc
${ChromiumRoot}/base/threading/thread_local_storage_posix.cc

${ChromiumRoot}/base/profiler/tracked_time.cc
${ChromiumRoot}/base/profiler/alternate_timer.cc

${ChromiumRoot}/base/safe_strerror_posix.cc
${ChromiumRoot}/base/lazy_instance.cc
${ChromiumRoot}/base/pickle.cc
${ChromiumRoot}/base/at_exit.cc
${ChromiumRoot}/base/tracked_objects.cc
${ChromiumRoot}/base/tracking_info.cc
${ChromiumRoot}/base/location.cc
${ChromiumRoot}/base/values.cc
${ChromiumRoot}/base/path_service.cc
${ChromiumRoot}/base/base_paths_posix.cc
${ChromiumRoot}/base/base_paths.cc
${ChromiumRoot}/base/environment.cc
${ChromiumRoot}/base/sys_info.cc
${ChromiumRoot}/base/sys_info_posix.cc
${ChromiumRoot}/base/sys_info_linux.cc

${ChromiumRoot}/base/files/file_path.cc
${ChromiumRoot}/base/files/file_util.cc
${ChromiumRoot}/base/files/file_enumerator.cc
${ChromiumRoot}/base/files/file_enumerator_posix.cc
${ChromiumRoot}/base/files/file.cc
${ChromiumRoot}/base/files/file_util_posix.cc
${ChromiumRoot}/base/files/scoped_file.cc
${ChromiumRoot}/base/files/scoped_temp_dir.cc
${ChromiumRoot}/base/files/file_posix.cc
${ChromiumRoot}/base/files/file_path_constants.cc

${ChromiumRoot}/base/time/time.cc
${ChromiumRoot}/base/time/time_posix.cc

${ChromiumRoot}/base/nix/xdg_util.cc

${ChromiumRoot}/base/metrics/sparse_histogram.cc
${ChromiumRoot}/base/metrics/histogram.cc
${ChromiumRoot}/base/metrics/sample_map.cc
${ChromiumRoot}/base/metrics/histogram_base.cc
${ChromiumRoot}/base/metrics/statistics_recorder.cc
${ChromiumRoot}/base/metrics/histogram_samples.cc
${ChromiumRoot}/base/metrics/bucket_ranges.cc
${ChromiumRoot}/base/metrics/sample_vector.cc

${ChromiumRoot}/base/json/*.cc

${ChromiumRoot}/base/third_party/dmg_fp/g_fmt.cc
${ChromiumRoot}/base/third_party/dmg_fp/dtoa.cc

${ChromiumRoot}/base/third_party/nspr/prtime.cc

${ChromiumRoot}/base/third_party/icu/icu_utf.cc

${ChromiumRoot}/base/third_party/xdg_user_dirs/xdg_user_dir_lookup.cc
)


add_executable(unittest.bin
    ${ChromiumTesting}/gmock/src/gmock-all.cc
    ${ChromiumTesting}/gtest/fused/gtest/gtest-all.cc
    ${set0}
    ${chromiumTests}
)



