cmake_minimum_required(VERSION 3.5)

project(CppHttpServer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-Wall -pedantic)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG)
endif()

find_package(spdlog REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED CONFIG)
find_package(Threads)
set(Libevent_DIR ~/Tools/usr/lib/cmake/libevent)
find_package(Libevent REQUIRED CONFIG)
include_directories(Include Src test ${LIBEVENT_INCLUDE_DIRS})
link_directories(Lib)
set(LOCAL_LINK_LIB spdlog::spdlog nlohmann_json::nlohmann_json ${LIBEVENT_LIBRARIES} pthread)

add_subdirectory(Src)
add_subdirectory(test)