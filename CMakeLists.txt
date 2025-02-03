cmake_minimum_required(VERSION 3.19)

project(dragonito C CXX)

option(DRAGONITO_TESTS "Build test program" OFF)
option(DRAGONITO_BENCHMARKS "Build benchmark program" OFF)
option(DRAGONITO_EXAMPLES "Build example program" OFF)

add_library(dragonito INTERFACE "dragonito.h" "dragonito_constants.h" "dragonito_template.h" "dragonito_impl.h")
target_include_directories(dragonito INTERFACE "/")

if (DRAGONITO_TESTS OR DRAGONITO_BENCHMARKS)
	add_subdirectory(tests/ryu)
endif()

if (DRAGONITO_EXAMPLES)
	add_executable(dragonito-examples "tests/example.c")
	
	target_link_libraries(dragonito-examples PRIVATE dragonito)
	target_compile_features(dragonito-examples PRIVATE c_std_99)
	
	if (MSVC)
		target_compile_options(dragonito-examples PRIVATE /Wall /wd5045)
	else()
		target_compile_options(dragonito-examples PRIVATE -Wall)
	endif()
endif()

if (DRAGONITO_TESTS)
	add_executable(dragonito-tests "tests/tests.cpp" "tests/test_bank.h")
	
	target_link_libraries(dragonito-tests PRIVATE dragonito)
	target_link_libraries(dragonito-tests PRIVATE ryu)
	target_compile_features(dragonito-tests PRIVATE cxx_std_17)
endif()

if (DRAGONITO_BENCHMARKS)
	set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
	set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "")
	set(BENCHMARK_INSTALL_DOCS   OFF CACHE BOOL "")
	set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
	set(BENCHMARK_USE_BUNDLED_GTEST  OFF CACHE BOOL "")
	
	add_subdirectory(tests/benchmark)
	add_subdirectory(tests/fmt)
	
	add_executable(dragonito-benchmarks "tests/benchmarks.cpp")
	
	target_link_libraries(dragonito-benchmarks PRIVATE dragonito)
	target_link_libraries(dragonito-benchmarks PRIVATE benchmark fmt ryu)
	target_compile_features(dragonito-benchmarks PRIVATE cxx_std_17)
endif()