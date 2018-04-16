cmake_minimum_required(VERSION 2.8.3)

project("integrationservices")
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 0)
set(PROJECT_MICRO_VERSION 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_MICRO_VERSION})

include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(--std=c++11 SUPPORTS_CXX11)
    if(SUPPORTS_CXX11)
        add_compile_options(--std=c++11)
    else()
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)

SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")


set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR include/ CACHE PATH "Installation directory for C++ headers")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")
if(WIN32)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

add_definitions(-DBUILD_SHARED_LIBS -Dintegrationservices_EXPORTS)

add_executable(integration-services src/main.cpp)

if(WIN32)
	add_library(islib
	    src/dynamicload/dynamicload.cpp
	    src/ISManager.cpp
	    src/GenericPubSubTypes.cpp
	    thirdparty/tinyxml2/tinyxml2.cpp thirdparty/tinyxml2/tinyxml2.h)

	target_include_directories(islib PUBLIC thirdparty/tinyxml2)
else()
	add_library(islib SHARED
	    src/dynamicload/dynamicload.cpp
	    src/ISManager.cpp
	    src/GenericPubSubTypes.cpp)
endif()

add_library(isrtpsbridgelib SHARED
    src/builtin/RTPS/ISBridgeRTPS.cpp
    src/builtin/RTPS/bridgertpslib.cpp)

if(WIN32)
	target_link_libraries(islib
	    fastrtps_static
	    fastcdr_static
	    ${CMAKE_DL_LIBS}
	    IPHLPAPI
	    shlwapi)
else()
	target_link_libraries(islib
	    fastrtps
	    fastcdr
	    tinyxml2
	    -L/usr/lib/x86_64-linux-gnu
	    curl
	    curlpp
	    ${Boost_LIBRARIES}
	    boost_system
	    ${CMAKE_DL_LIBS})
endif()

target_link_libraries(isrtpsbridgelib islib)

target_link_libraries(integration-services islib)

install(
    FILES
        src/ISBridge.h
        src/ISManager.h
        src/GenericPubSubTypes.h
    COMPONENT headers
    DESTINATION ${INCLUDE_INSTALL_DIR}${PROJECT_NAME})

install(
    FILES
        src/dynamicload/dynamicload.h
    COMPONENT headers
    DESTINATION ${INCLUDE_INSTALL_DIR}${PROJECT_NAME}/dynamicload)

install(
    TARGETS islib
    EXPORT ${PROJECT_NAME}Targets
    COMPONENT libraries
    DESTINATION ${LIB_INSTALL_DIR})

export(
    TARGETS islib
    FILE ${BIN_INSTALL_DIR}cmake/config/${PROJECT_NAME}Targets.cmake)

install(
    TARGETS isrtpsbridgelib
    EXPORT ${PROJECT_NAME}Targets
    COMPONENT libraries
    DESTINATION ${LIB_INSTALL_DIR})

export(
    TARGETS isrtpsbridgelib islib
    FILE ${BIN_INSTALL_DIR}cmake/config/${PROJECT_NAME}Targets.cmake)

install(
    EXPORT ${PROJECT_NAME}Targets
    DESTINATION lib/integrationservices/cmake
    COMPONENT cmake)

install(
    TARGETS integration-services
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

include(${PROJECT_SOURCE_DIR}/cmake/packaging/eProsimaPackaging.cmake)
