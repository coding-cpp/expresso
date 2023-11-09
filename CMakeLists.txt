cmake_minimum_required(VERSION 3.16)
project(backend VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug 
    CACHE STRING 
      "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(USE_INSTALL_RESOURCE_PATH "Set resource path to install location" OFF)

add_executable(hello_router
  ./src/server.cpp
  ./src/router.cpp
  ./src/response.cpp
  ./src/process.cpp

  ./example/hello_router.cpp
)

target_include_directories(hello_router
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)