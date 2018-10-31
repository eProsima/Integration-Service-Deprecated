cmake_minimum_required(VERSION 2.8.3)

###############################################################################
# Version information                                                         #
###############################################################################
set(IS_TOP_LEVEL TRUE)
if(PROJECT_SOURCE_DIR)
    set(IS_TOP_LEVEL FALSE)
endif()

#If not set, copy over product version from configure.ac
file(STRINGS
    configure.ac
    LIB_MAJOR_VERSION_TMP
    REGEX "^m4_define\\(\\[?version_major\\]?, *\\[?[0-9]+\\]?\\)"
    )
file(STRINGS
    configure.ac
    LIB_MINOR_VERSION_TMP
    REGEX "^m4_define\\(\\[?version_minor\\]?, *\\[?[0-9]+\\]?\\)"
    )
file(STRINGS
    configure.ac
    LIB_PATCH_VERSION_TMP
    REGEX "^m4_define\\(\\[?version_micro\\]?, *\\[?[0-9]+\\]?\\)"
    )

string(REGEX REPLACE "m4_define\\(\\[?version_major\\]?, *\\[?([0-9]+)\\]?\\)"
    "\\1"
    LIB_MAJOR_VERSION
    ${LIB_MAJOR_VERSION_TMP}
    )
string(REGEX REPLACE "m4_define\\(\\[?version_minor\\]?, *\\[?([0-9]+)\\]?\\)"
    "\\1"
    LIB_MINOR_VERSION
    ${LIB_MINOR_VERSION_TMP}
    )
string(REGEX REPLACE "m4_define\\(\\[?version_micro\\]?, *\\[?([0-9]+)\\]?\\)"
    "\\1"
    LIB_PATCH_VERSION
    ${LIB_PATCH_VERSION_TMP}
    )

set(LIB_VERSION_STR "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

###############################################################################
# Project                                                                     #
###############################################################################
if(CMAKE_VERSION VERSION_LESS 3.0)
    project("integration_services" C CXX)
    set(PROJECT_VERSION_MAJOR "${LIB_MAJOR_VERSION}")
    set(PROJECT_VERSION_MINOR "${LIB_MINOR_VERSION}")
    set(PROJECT_VERSION_PATCH "${LIB_PATCH_VERSION}")
    set(PROJECT_VERSION "${LIB_VERSION_STR}")
else()
    cmake_policy(SET CMP0048 NEW)
    project("integration_services" VERSION "${LIB_VERSION_STR}" LANGUAGES C CXX)
endif()

set(PROJECT_NAME_STYLED "IntegrationServices")
set(PROJECT_NAME_LARGE "Integration Services")
set(PROJECT_NAME_UPPER "IS")
set(SHAREDLIB_NAME "islib")

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

include(${PROJECT_SOURCE_DIR}/cmake/common/eprosima_libraries.cmake)

###############################################################################
# Test system configuration
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/common/check_configuration.cmake)
check_stdcxx()
check_compile_feature()
check_endianness()

###############################################################################
# Check MSVC architecture
###############################################################################
if(MSVC OR MSVC_IDE)
    check_msvc_arch()
endif()

eprosima_find_package(fastcdr $<$<NOT:$<BOOL:${EPROSIMA_INSTALLER}>>:REQUIRED>)
eprosima_find_package(fastrtps $<$<NOT:$<BOOL:${EPROSIMA_INSTALLER}>>:REQUIRED>)

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
set(INCLUDE_SOURCE_DIR src/ CACHE PATH "Installation directory for C++ sources")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")

if(WIN32 AND EPROSIMA_INSTALLER AND IS_TOP_LEVEL)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

add_definitions(-DBUILD_SHARED_LIBS -Dintegration_services_EXPORTS)
add_definitions(-D${PROJECT_NAME_UPPER}_SOURCE)

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

configure_file(src/log/ISLog.h.in ${PROJECT_SOURCE_DIR}/include/log/ISLog.h)
configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.h)

include(${PROJECT_SOURCE_DIR}/cmake/dev/set_sources.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/dev/generate_msvc_libraries.cmake)

set(${PROJECT_NAME}_header_files
        include/xmlUtils.h
        include/log/ISLog.h
        include/dynamicload/dynamicload.h
        include/ISManager.h
        include/ISBridge.h
        include/RTPSBridge.h
        include/RTPSPublisher.h
        include/RTPSSubscriber.h
        include/GenericPubSubTypes.h
    )

set(${PROJECT_NAME}_source_files
        src/dynamicload/dynamicload.cpp
        src/ISManager.cpp
        src/RTPSBridge.cpp
        src/RTPSPublisher.cpp
        src/RTPSSubscriber.cpp
        src/GenericPubSubTypes.cpp
    )

# Add sources to Makefile.am
set_sources(${${PROJECT_NAME}_source_files})

if(EPROSIMA_INSTALLER)
    set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
    set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} C++ Headers")
endif()

###############################################################################
# Test system configuration
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/common/check_configuration.cmake)
check_stdcxx()
check_compile_feature()
check_endianness()

###############################################################################
# Check MSVC architecture
###############################################################################
if(MSVC OR MSVC_IDE)
    check_msvc_arch()
endif()

# Create libraries
if((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER AND IS_TOP_LEVEL)
    generate_msvc_libraries(${MSVC_ARCH})
elseif(NOT EPROSIMA_INSTALLER)
    # Testing
    enable_testing()
    include(CTest)
    add_subdirectory(test)

    # Executable and library
    add_executable(${PROJECT_NAME} src/main.cpp)

    add_library(${SHAREDLIB_NAME} SHARED ${${PROJECT_NAME}_source_files})
    target_include_directories(${SHAREDLIB_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

    target_link_libraries(${SHAREDLIB_NAME}
            fastrtps
            fastcdr
            ${CMAKE_DL_LIBS}
            $<$<BOOL:${WIN32}>:IPHLPAPI$<SEMICOLON>shlwapi>
            $<$<NOT:$<BOOL:${WIN32}>>:-L/usr/lib/x86_64-linux-gnu>
            )

    target_link_libraries(${PROJECT_NAME} ${SHAREDLIB_NAME})

    # Properties
    set_target_properties(${SHAREDLIB_NAME} PROPERTIES RELEASE_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    set_target_properties(${SHAREDLIB_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    set_target_properties(${SHAREDLIB_NAME} PROPERTIES DEBUG_POSTFIX d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
        set_target_properties(${SHAREDLIB_NAME} PROPERTIES VS_WINRT_COMPONENT "true")
    endif()
endif()

if(UNIX AND EPROSIMA_INSTALLER)
    install(
        FILES
            include/xmlUtils.h
            include/ISBaseClass.h
            include/ISBridge.h
            include/ISPublisher.h
            include/ISSubscriber.h
            include/ISManager.h
            include/GenericPubSubTypes.h
            include/ISManager.h
            include/ISBridge.h
            include/RTPSBridge.h
            include/RTPSPublisher.h
            include/RTPSSubscriber.h
            include/GenericPubSubTypes.h
        COMPONENT headers
        DESTINATION ${INCLUDE_INSTALL_DIR})

    install(
        FILES
            include/dynamicload/dynamicload.h
        COMPONENT headers
        DESTINATION ${INCLUDE_INSTALL_DIR}/dynamicload)

    install(
        FILES
        include/log/ISLog.h
        COMPONENT headers
        DESTINATION ${INCLUDE_INSTALL_DIR}/log)

    install(
        FILES
            src/main.cpp
            src/ISManager.cpp
            src/RTPSBridge.cpp
            src/RTPSPublisher.cpp
            src/RTPSSubscriber.cpp
            src/GenericPubSubTypes.cpp
        COMPONENT sources
        DESTINATION ${INCLUDE_SOURCE_DIR}/src)

    install(
        FILES
            src/dynamicload/dynamicload.cpp
        COMPONENT sources
        DESTINATION ${INCLUDE_SOURCE_DIR}/src/dynamicload)

    # Install licenses
    install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
        DESTINATION ${LICENSE_INSTALL_DIR}
        COMPONENT licenses
        )
    set(CPACK_COMPONENT_LICENSES_HIDDEN 1)

    install(DIRECTORY ${PROJECT_BINARY_DIR}/autotools/
        DESTINATION .
        USE_SOURCE_PERMISSIONS
        OPTIONAL
        COMPONENT sources
        )
endif()

# Install examples
install(DIRECTORY ${PROJECT_SOURCE_DIR}/examples
    DESTINATION ./
    COMPONENT examples
    PATTERN "examples/CMakeLists.txt" EXCLUDE
    )
set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} examples")

if(NOT EPROSIMA_INSTALLER)
    install(
        FILES
            include/xmlUtils.h
            include/ISBaseClass.h
            include/ISBridge.h
            include/ISPublisher.h
            include/ISSubscriber.h
            include/ISManager.h
            include/GenericPubSubTypes.h
            include/ISManager.h
            include/ISBridge.h
            include/RTPSBridge.h
            include/RTPSPublisher.h
            include/RTPSSubscriber.h
            include/GenericPubSubTypes.h
        COMPONENT headers
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME})

    install(
        FILES
            include/dynamicload/dynamicload.h
        COMPONENT headers
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/dynamicload)

    install(
        FILES
            include/log/ISLog.h
        COMPONENT headers
        DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/log)

    if(MSVC OR MSVC_IDE)
        install(
            TARGETS ${SHAREDLIB_NAME}
            EXPORT ${PROJECT_NAME}Targets
            COMPONENT libraries_${MSVC_ARCH}
            DESTINATION ${LIB_INSTALL_DIR})
    else()
        install(
            TARGETS ${SHAREDLIB_NAME}
            EXPORT ${PROJECT_NAME}Targets
            COMPONENT libraries}
            DESTINATION ${LIB_INSTALL_DIR})
    endif()

    export(
        TARGETS ${SHAREDLIB_NAME}
        FILE ${BIN_INSTALL_DIR}cmake/config/${PROJECT_NAME}Targets.cmake)

    install(
        EXPORT ${PROJECT_NAME}Targets
        DESTINATION lib/${PROJECT_NAME}/cmake
        COMPONENT cmake)

    install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        COMPONENT libraries_${MSVC_ARCH})
else()
    set(CPACK_COMPONENTS_ALL headers sources licenses examples)

    if(MSVC OR MSVC_IDE)
        set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} libraries_${MSVC_ARCH})
    else()
        set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} libraries)
    endif()

    include(${PROJECT_SOURCE_DIR}/cmake/packaging/eProsimaPackaging.cmake)
endif()

# Install libraries
if(MSVC OR MSVC_IDE AND EPROSIMA_INSTALLER AND IS_TOP_LEVEL)
    install_msvc_libraries(${MSVC_ARCH})
elseif(NOT EPROSIMA_INSTALLER)
    set(DIR_EXTENSION "")
    if(EPROSIMA_INSTALLER_MINION)
        set(DIR_EXTENSION "/${MSVC_ARCH}")
    endif()

    install(EXPORT ${PROJECT_NAME}Targets
        DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}/${PROJECT_NAME}/cmake
        COMPONENT cmake
        )
else()
    string(TOUPPER "${MSVC_ARCH}" MSVC_ARCH_UPPER)
    set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DISPLAY_NAME "${MSVC_ARCH}")
    set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries for platform ${MSVC_ARCH}")
    set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_GROUP "Libraries")

    set(CPACK_COMPONENT_GROUP_LIBRARIES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries")
endif()
