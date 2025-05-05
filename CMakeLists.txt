# Minimum CMake version required
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(araneid VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
set(SOURCES
    src/base/log.cpp
    src/base/simulator.cpp
    src/base/thread-pool.cpp
    src/base/time.cpp
    src/base/units.cpp
    src/network/device.cpp
    src/network/packet.cpp
    src/network/transmission.cpp
    src/system/bridge.cpp
    src/system/fd-reader.cpp
    src/system/virtual-machine.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

