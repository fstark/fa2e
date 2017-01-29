cmake_minimum_required(VERSION 3.5.1)

set (CMAKE_CXX_STANDARD 14)


project(fa2e)

include_directories(include)

file(GLOB SOURCES "src/fa2e/*.cpp")

add_executable(fa2e ${SOURCES})

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARIES})

