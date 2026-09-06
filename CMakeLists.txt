cmake_minimum_required(VERSION 3.20)

project(Netforge
    VERSION 0.1.0
    DESCRIPTION "Lightweight asynchronous networking library for modern C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(NETFORGE_BUILD_EXAMPLES "Build Netforge examples" ON)
option(NETFORGE_BUILD_TESTS "Build Netforge tests" OFF)

# ------------------------------------------------------------------------------
# Boost
# ------------------------------------------------------------------------------

find_package(Boost QUIET CONFIG)

if (NOT Boost_FOUND)
    message(STATUS "Boost not found. Downloading Boost...")

    FetchContent_Declare(
        boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.92.0/boost-1.92.0-cmake.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    FetchContent_MakeAvailable(boost)
endif()

# ------------------------------------------------------------------------------
# Netforge
# ------------------------------------------------------------------------------

add_library(Netforge
    src/Session.cpp
    src/TcpSession.cpp
)

set_target_properties(Netforge PROPERTIES
    OUTPUT_NAME "Netforge$<$<CONFIG:Debug>:_d>"
)

add_library(Netforge::Netforge ALIAS Netforge)

target_compile_features(Netforge
    PUBLIC
        cxx_std_20
)

target_include_directories(Netforge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Netforge
    PUBLIC
        Boost::headers
)

# ------------------------------------------------------------------------------
# Examples
# ------------------------------------------------------------------------------

if (NETFORGE_BUILD_EXAMPLES)

    add_executable(netforge_echo
        examples/echo/Main.cpp
    )

    target_link_libraries(netforge_echo
        PRIVATE
            Netforge::Netforge
    )

endif()