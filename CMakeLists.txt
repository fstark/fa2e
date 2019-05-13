cmake_minimum_required(VERSION 3.5.1)

set (CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(fa2e)



include_directories(include)
# Including custom cmake rules (clang-tidy + clang-format)
include(cmake/clang-cxx-dev-tools.cmake)
include(cmake/FindSDL2.cmake)
find_package(SDL2)

file(GLOB SOURCES "src/fa2e/*.cpp")

add_executable(fa2e ${SOURCES})

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARIES})

#INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} "C:\\Users\\fred\\fa2e\\SDL2-2.0.5\\include")
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARIES}  "C:\\Users\\fred\\fa2e\\SDL2-2.0.5\\lib\\x86\\SDL2.lib" "C:\\Users\\fred\\fa2e\\SDL2-2.0.5\\lib\\x86\\SDL2main.lib")


#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

#message(STATUS "XXXXXXXXXXXXXXXXXXX")

#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#  message(STATUS "dir='${dir}'")
#endforeach()

