cmake_minimum_required(VERSION 2.8.3)

project("integration-services")
set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 0)
set(PROJECT_MICRO_VERSION 0)
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_MICRO_VERSION})

include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(--std=c++11 SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -pedantic")
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

add_definitions(-DBUILD_SHARED_LIBS -Dintegration_services_EXPORTS)

IF (NOT DEFINED LOG_LEVEL_DEBUG)
    SET(LOG_LEVEL_DEBUG OFF)
ENDIF()
ADD_DEFINITIONS(-DLOG_LEVEL_DEBUG=${LOG_LEVEL_DEBUG})

IF (NOT DEFINED LOG_LEVEL_INFO)
    SET(LOG_LEVEL_INFO OFF)
ENDIF()
ADD_DEFINITIONS(-DLOG_LEVEL_INFO=${LOG_LEVEL_INFO})

IF (NOT DEFINED LOG_LEVEL_WARN)
    SET(LOG_LEVEL_WARN OFF)
ENDIF()
ADD_DEFINITIONS(-DLOG_LEVEL_WARN=${LOG_LEVEL_WARN})

IF (NOT DEFINED LOG_LEVEL_ERROR)
    SET(LOG_LEVEL_ERROR ON)
ENDIF()
ADD_DEFINITIONS(-DLOG_LEVEL_ERROR=${LOG_LEVEL_ERROR})

configure_file(src/log/ISLog.h.in ${PROJECT_SOURCE_DIR}/src/log/ISLog.h)

add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty/tinyxml2)

if(WIN32)
    add_library(islib
        src/dynamicload/dynamicload.cpp src/dynamicload/dynamicload.h
        src/ISManager.cpp src/ISManager.h src/log/ISLog.h
        src/ISBridge.cpp src/ISBridge.h
        src/RTPSBridge.cpp src/RTPSBridge.h
        src/RTPSPublisher.cpp src/RTPSPublisher.h
        src/RTPSSubscriber.cpp src/RTPSSubscriber.h
        src/GenericPubSubTypes.cpp src/GenericPubSubTypes.h
        thirdparty/tinyxml2/tinyxml2.cpp thirdparty/tinyxml2/tinyxml2.h)

    target_link_libraries(islib
        fastrtps_static
        fastcdr_static
        ${CMAKE_DL_LIBS}
        IPHLPAPI
        shlwapi)
else()
    add_library(islib SHARED
        src/dynamicload/dynamicload.cpp src/dynamicload/dynamicload.h
        src/ISManager.cpp src/ISManager.h src/log/ISLog.h
        src/ISBridge.cpp src/ISBridge.h
        src/RTPSBridge.cpp src/RTPSBridge.h
        src/RTPSPublisher.cpp src/RTPSPublisher.h
        src/RTPSSubscriber.cpp src/RTPSSubscriber.h
        src/GenericPubSubTypes.cpp src/GenericPubSubTypes.h
        thirdparty/tinyxml2/tinyxml2.cpp thirdparty/tinyxml2/tinyxml2.h)

    target_link_libraries(islib
        fastrtps
        fastcdr
        -L/usr/lib/x86_64-linux-gnu
        ${CMAKE_DL_LIBS})
endif()

target_include_directories(islib PRIVATE thirdparty/tinyxml2)
target_link_libraries(${PROJECT_NAME} islib)

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
    EXPORT ${PROJECT_NAME}Targets
    DESTINATION lib/${PROJECT_NAME}/cmake
    COMPONENT cmake)

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

include(${PROJECT_SOURCE_DIR}/cmake/packaging/eProsimaPackaging.cmake)
