cmake_minimum_required(VERSION 4.0)
project(corsair-780t-fan-control-mod LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)

find_package(CURL REQUIRED)

add_executable(corsair-780t-fan-control-mod main.cpp)

target_link_libraries(corsair-780t-fan-control-mod
        PRIVATE
        PkgConfig::LIBUSB
        CURL::libcurl
)
