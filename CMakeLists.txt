# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -Oz")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32-doom)