#-------------------------------------------------------------------------------
# Project description
#-------------------------------------------------------------------------------
cmake_minimum_required( VERSION 3.7 )

project( cephserve-client )

set(CMAKE_CXX_STANDARD 14)
include(ExternalProject)
include(FindOpenSSL)

include("${CMAKE_SOURCE_DIR}/vendor/rapidjson.cmake")
if( OPENSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
else()
    message(STATUS "No openssl")
# Error; with REQUIRED, pkg_search_module() will throw an error by it's own
endif()

include_directories(${RAPIDJSON_INCLUDE_DIR})

add_executable(cephsumclient src/cksum.cpp )
target_link_libraries(cephsumclient PRIVATE OpenSSL::SSL OpenSSL::Crypto)
add_dependencies(cephsumclient rapidjson)
