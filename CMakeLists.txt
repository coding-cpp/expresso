cmake_minimum_required(VERSION 3.16)
project(expresso VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_COMPILER "g++")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release 
    CACHE STRING 
      "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(USE_INSTALL_RESOURCE_PATH "Set resource path to install location" OFF)

add_executable(server
  ./src/core/request.cpp
  ./src/core/response.cpp
  ./src/core/router.cpp
  ./src/core/server.cpp

  ./src/middleware/cors.cpp
  ./src/middleware/cookie_parser.cpp
  ./src/middleware/middleware.cpp
  ./src/middleware/static_serve.cpp
  
  ./src/utils/common.cpp
  ./src/utils/file.cpp
  ./src/utils/print.cpp
  ./src/utils/process.cpp
  ./src/utils/sterlize.cpp

  ./example/main.cpp
)

target_include_directories(server
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
