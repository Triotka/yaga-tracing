cmake_minimum_required(VERSION 3.17)

project(yaga CXX)

include(FetchContent)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules/")

find_package(Catch2 3.4 QUIET)
find_package(GMP REQUIRED)

if(FOUND_CATCH)
    message(STATUS "Using system install of Catch2")
else()
    message(STATUS "Fetching Catch2")

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.4.0
    )

    FetchContent_MakeAvailable(Catch2)

    message(STATUS "Fetching Catch2 - done")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_library(yaga src/lra/Fraction.h src/lra/Rational.h src/lra/Long_fraction.cpp src/metrics/Metrics.cpp)
target_link_libraries(yaga PUBLIC GMP::GMP)
add_executable(test)
add_executable(sat src/sat_solver.cpp)
add_executable(smt src/smt_solver.cpp)

add_subdirectory(src)
add_subdirectory(tests)

target_link_libraries(test PUBLIC yaga)

target_compile_features(yaga PRIVATE cxx_std_20)
target_compile_features(test PRIVATE cxx_std_20)
target_compile_features(sat PRIVATE cxx_std_20)
target_compile_features(smt PRIVATE cxx_std_20)

target_link_libraries(test PRIVATE yaga Catch2::Catch2WithMain)
target_link_libraries(sat PRIVATE yaga)
target_link_libraries(smt PRIVATE yaga)

# Enable logging for metrics web application
# target_compile_definitions(yaga PRIVATE LOG_TRASHING LOG_SHALLOW_BACKTRACKS LOG_BACKTRACK_CLUSTERING)
# target_compile_definitions(test PRIVATE LOG_TRASHING LOG_SHALLOW_BACKTRACKS LOG_BACKTRACK_CLUSTERING)
# target_compile_definitions(sat PRIVATE LOG_TRASHING LOG_SHALLOW_BACKTRACKS LOG_BACKTRACK_CLUSTERING)
# target_compile_definitions(smt PRIVATE LOG_TRASHING LOG_SHALLOW_BACKTRACKS LOG_BACKTRACK_CLUSTERING)