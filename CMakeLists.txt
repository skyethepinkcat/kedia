cmake_minimum_required(VERSION 3.16)

project(kedia_manager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Test Widgets Sql Concurrent)

find_program(FFMPEG ffmpeg)

add_compile_definitions(FFMPEG="${FFMPEG}")

set(CMAKE_AUTOUIC_SEARCH_PATHS ui/)

include(CTest)

include(FetchContent)

qt_standard_project_setup()

add_subdirectory(src/manager)
add_subdirectory(src/lib)
add_subdirectory(src/store)
add_subdirectory(test/lib)
add_subdirectory(test/store)
# add_subdirectory(test/manager)

add_test(NAME SeriesTest COMMAND kedia_test)
add_test(NAME StoreTest COMMAND kedia_store_test)
# add_test(NAME ManagerTest COMMAND kedia_test_manager)
