cmake_minimum_required(VERSION 3.15)
project(QueueBenchmarks
    VERSION 1.0
    DESCRIPTION "Benchmarking different queue implementations"
    LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(queue_benchmarks 
    main.cpp
    BenchmarkDequeue.cpp
    BenchmarkQueue.cpp
    BenchmarkList.cpp
)

add_executable(stm_tests
    stm_tests.cpp
)

add_executable(stm_unit_tests
    stm_unit_tests.cpp
)

add_executable(stm_smoke_tests
    stm_smoke_tests.cpp
)

add_executable(stm_sanity_tests
    stm_sanity_tests.cpp
)

add_executable(stm_differential_tests
    stm_differential_tests.cpp
)

add_executable(stm_api_coverage_tests
    stm_api_coverage_tests.cpp
)

add_test(NAME stm_tests COMMAND stm_tests)
add_test(NAME stm_unit_tests COMMAND stm_unit_tests)
add_test(NAME stm_smoke_tests COMMAND stm_smoke_tests)
add_test(NAME stm_sanity_tests COMMAND stm_sanity_tests)
add_test(NAME stm_differential_tests COMMAND stm_differential_tests)
add_test(NAME stm_api_coverage_tests COMMAND stm_api_coverage_tests)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(queue_benchmarks PRIVATE -O3)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(queue_benchmarks PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(stm_tests PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(stm_unit_tests PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(stm_smoke_tests PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(stm_sanity_tests PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(stm_differential_tests PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(stm_api_coverage_tests PRIVATE -Wall -Wextra -pedantic)
endif()

target_include_directories(queue_benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stm_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stm_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stm_smoke_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stm_sanity_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stm_differential_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stm_api_coverage_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
