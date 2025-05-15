cmake_minimum_required(VERSION 3.10)
project(CppPaperTrading LANGUAGES CXX)

# — C++17 —
set(CMAKE_CXX_STANDARD     17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS    OFF)

# — Your headers (OrderEngine.hpp, CommunicationLayer.hpp, json.hpp etc.) —
include_directories(${CMAKE_SOURCE_DIR}/include)

# — Find hiredis via Homebrew —
execute_process(
  COMMAND brew --prefix hiredis
  OUTPUT_VARIABLE HIREDIS_PREFIX
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT HIREDIS_PREFIX)
  message(FATAL_ERROR "hiredis not found. brew install hiredis")
endif()

# — JSON (nlohmann) —
find_package(nlohmann_json REQUIRED)

# — Gather all your backend sources (omit BinanceFuturesFeed.cpp which you deleted) —
file(GLOB_RECURSE BACKEND_SOURCES src/*.cpp)

# — Main executable —
add_executable(trading_engine ${BACKEND_SOURCES})

target_include_directories(trading_engine PRIVATE
  ${HIREDIS_PREFIX}/include
)

target_link_libraries(trading_engine PRIVATE
  ${HIREDIS_PREFIX}/lib/libhiredis.dylib
  nlohmann_json::nlohmann_json
)

# — Unit tests (unchanged) —
option(BUILD_TESTS "Build unit tests" ON)
if (BUILD_TESTS)
  find_package(GTest REQUIRED)
  enable_testing()

  add_executable(OrderEngineTests
    tests/OrderEngineTests.cpp
    src/OrderEngine.cpp
    src/CommunicationLayer.cpp
  )

  target_include_directories(OrderEngineTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${HIREDIS_PREFIX}/include
  )

  target_link_libraries(OrderEngineTests PRIVATE
    GTest::gtest_main
    ${HIREDIS_PREFIX}/lib/libhiredis.dylib
  )

  add_test(NAME OrderEngineTests COMMAND OrderEngineTests)
endif()
