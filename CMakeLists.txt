cmake_minimum_required(VERSION 3.10)
project(gitCI)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MHD REQUIRED libmicrohttpd)

include_directories(${MHD_INCLUDE_DIRS})
link_directories(${MHD_LIBRARY_DIRS})

add_executable(gitCI main.cpp
        webhooks/gitCIServer.cpp
        webhooks/gitCIServer.h)

target_link_libraries(gitCI ${MHD_LIBRARIES})