cmake_minimum_required(VERSION 3.20)

project(Netforge
    VERSION 0.1.0
    DESCRIPTION "Lightweight asynchronous networking library for modern C++"
    LANGUAGES CXX
)

# ------------------------------------------------------------------------------
# C++
# ------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(NETFORGE_BUILD_EXAMPLES "Build Netforge examples" ON)
option(NETFORGE_BUILD_TESTS "Build Netforge tests" OFF)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

find_package(Boost CONFIG REQUIRED)

message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "Boost directory: ${Boost_DIR}")

# ------------------------------------------------------------------------------
# Netforge
# ------------------------------------------------------------------------------

add_library(Netforge
    src/Session.cpp
    src/TcpSession.cpp
    src/HttpSession.cpp
)

add_library(Netforge::Netforge ALIAS Netforge)

set_target_properties(Netforge PROPERTIES
    OUTPUT_NAME "Netforge$<$<CONFIG:Debug>:_d>"
)

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

if(NETFORGE_BUILD_EXAMPLES)

    add_executable(netforge_tcp_server
        examples/tcp_server/main.cpp
    )

    target_link_libraries(netforge_tcp_server
        PRIVATE
            Netforge::Netforge
    )


    add_executable(netforge_http_server
        examples/http_server/main.cpp
    )

    target_link_libraries(netforge_http_server
        PRIVATE
            Netforge::Netforge
    )

endif()

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if(NETFORGE_BUILD_TESTS)

    enable_testing()

    # Add tests here.

endif()