cmake_minimum_required(VERSION 3.16)

project(mtp_full_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent)

find_library(LIBMTP_LIBRARY NAMES mtp libmtp HINTS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
if(NOT LIBMTP_LIBRARY)
    message(FATAL_ERROR "libmtp not found. Please install libmtp-dev or specify the path.")
else()
    message(STATUS "Found libmtp: ${LIBMTP_LIBRARY}")
endif()

add_subdirectory(libmtpviewmodel)
add_subdirectory(app)

set(CMAKE_PREFIX_PATH ${Qt6_DIR})
