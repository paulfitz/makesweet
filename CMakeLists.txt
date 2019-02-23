cmake_minimum_required(VERSION 2.6)

PROJECT(makesweet)

set (CMAKE_CXX_STANDARD 11)

SET(BUILD_SHARED_LIBS ON)

# Find YARP.  Point the YARP_DIR environment variable at your build.
FIND_PACKAGE(YARP)
INCLUDE_DIRECTORIES(${YARP_INCLUDE_DIRS})
LINK_LIBRARIES(${YARP_LIBRARIES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

ADD_SUBDIRECTORY(src)
