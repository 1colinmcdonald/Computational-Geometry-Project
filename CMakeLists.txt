cmake_minimum_required(VERSION 3.16)
project(proj LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CGAL REQUIRED)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

enable_testing()


FetchContent_MakeAvailable(googletest)


add_executable(benchmark benchmark.cpp)

# Works if CGAL provides imported targets
if(TARGET CGAL::CGAL)
  target_link_libraries(benchmark CGAL::CGAL)
else()
  # Fallback for older CGAL CMake configs
  include(${CGAL_USE_FILE})
  target_link_libraries(project_lib PUBLIC ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})
endif()

add_executable(bin_search_test bin_search_test.cpp)
target_link_libraries(bin_search_test PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(bin_search_test)