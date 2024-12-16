cmake_minimum_required(VERSION 2.8.2)

# Override flags to enable prepare for linking to static runtime
set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)

# Set a private module find path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(E57RefImpl)

# developer adjustable version numbers
set(${PROJECT_NAME}_MAJOR_VERSION 1)
set(${PROJECT_NAME}_MINOR_VERSION 1)

include(Tags)

# propose a default installation directory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    string(REGEX REPLACE "/${PROJECT_NAME}" "" CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    set(T_ ${PROJECT_NAME})
    set(T_ ${T_}-${${PROJECT_NAME}_MAJOR_VERSION})
    set(T_ ${T_}.${${PROJECT_NAME}_MINOR_VERSION})
    set(T_ ${T_}.${${PROJECT_NAME}_BUILD_VERSION})
    set(T_ ${T_}-${${PROJECT_NAME}_BUILD_TAG})
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${T_}
        CACHE PATH
        "Install path prefix, prepended onto install directories."
        FORCE
    )
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

find_package(Threads REQUIRED)

# Find the Boost, nlohmann_json and Xerces libraries

find_package(nlohmann_json REQUIRED)

set(BOOST_ROOT /opt/boost)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost
    COMPONENTS
        program_options
        system
        thread
        filesystem
    QUIET
)

set(XERCES_ROOT /opt/xerces)
set(Xerces_USE_STATIC_LIBS On)
find_package(Xerces QUIET)

set(XML_LIBRARIES ${Xerces_LIBRARY})
set(XML_INCLUDE_DIRS ${Xerces_INCLUDE_DIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DLINUX)
    find_package(ICU REQUIRED)
    set(XML_LIBRARIES ${XML_LIBRARIES} ${ICU_LIBRARIES})
    set(XML_INCLUDE_DIRS ${XML_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS})
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-DWINDOWS)
endif()

add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
add_definitions(-DBOOST_ALL_NO_LIB -DXERCES_STATIC_LIBRARY)

configure_file (
  "${PROJECT_SOURCE_DIR}/include/config.h.in"
  "${PROJECT_BINARY_DIR}/include/config.h"
)

include_directories(
    ${PROJECT_BINARY_DIR}/include
    include
    include/time_conversion
    src/refimpl
    ${XML_INCLUDE_DIRS}
    /opt/boost/include
)

link_directories(
    /opt/boost/lib
)


#
# The reference implementation
#

add_library( E57RefImpl STATIC
    src/refimpl/E57Foundation.cpp
    src/refimpl/E57FoundationImpl.cpp
    src/refimpl/E57FoundationImpl.h
    src/refimpl/E57Simple.cpp
    src/refimpl/E57SimpleImpl.cpp
    src/refimpl/E57SimpleImpl.h
    include/E57Foundation.h
    include/E57Simple.h
)
set_target_properties( E57RefImpl
    PROPERTIES DEBUG_POSTFIX "-d"
)

#
# Files for LAS format support
#

add_library( LASReader STATIC
    src/LASReader/LASReader.cpp
    include/LASReader.h
)

#
# Time conversion utilities
#

add_library( time_conversion STATIC
    src/time_conversion/time_conversion.c
    include/time_conversion/time_conversion.h
    include/time_conversion/basictypes.h
    include/time_conversion/constants.h
    include/time_conversion/gnss_error.h
)

#
# Example programs
#

add_executable( DemoWrite01
    src/examples/DemoWrite01.cpp
)
target_link_libraries( DemoWrite01
    E57RefImpl
    LASReader
    time_conversion
    nlohmann_json::nlohmann_json
    ${XML_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

#include (InstallRequiredSystemLibraries)

set (CPACK_PACKAGE_VENDOR "Riegl LMS GmbH")
set (CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_MAJOR_VERSION}")
set (CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_MINOR_VERSION}")
set (CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_BUILD_VERSION}")
set (CPACK_SYSTEM_NAME "${${PROJECT_NAME}_BUILD_TAG}")
set (CPACK_GENERATOR "ZIP")
set (CPACK_STRIP_FILES "TRUE")

include (CPack)
