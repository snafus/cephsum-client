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

add_executable(cephsumclient src/cephsumclient.cpp )
target_link_libraries(cephsumclient PRIVATE OpenSSL::SSL OpenSSL::Crypto)
add_dependencies(cephsumclient rapidjson)
install(TARGETS cephsumclient RUNTIME DESTINATION bin LIBRARY DESTINATION lib)

set(VERSION "0.0.1")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "cephserve-client")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "JW")
set(CPACK_PACKAGE_VENDOR "My Company")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)

