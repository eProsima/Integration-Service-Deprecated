cmake_minimum_required(VERSION 2.8.3)

###############################################################################
# Version information                                                         #
###############################################################################
set(IS_TOP_LEVEL TRUE)
if(PROJECT_SOURCE_DIR)
    set(IS_TOP_LEVEL FALSE)
    string(REPLACE "\\" "/" PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR} )
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
    project("integration_service" C CXX)
    set(PROJECT_VERSION_MAJOR "${LIB_MAJOR_VERSION}")
    set(PROJECT_VERSION_MINOR "${LIB_MINOR_VERSION}")
    set(PROJECT_VERSION_PATCH "${LIB_PATCH_VERSION}")
    set(PROJECT_VERSION "${LIB_VERSION_STR}")
else()
    cmake_policy(SET CMP0048 NEW)
    project("integration_service" VERSION "${LIB_VERSION_STR}" LANGUAGES C CXX)
endif()

set(PROJECT_NAME_STYLED "IntegrationService")
set(PROJECT_NAME_LARGE "Integration Service")
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

###############################################################################
# eProsima build options
###############################################################################
option(EPROSIMA_BUILD "Activate internal building" OFF)

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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)
include(${PROJECT_SOURCE_DIR}/cmake/common/eprosima_libraries.cmake)
eprosima_find_package(fastcdr REQUIRED)
eprosima_find_package(fastrtps REQUIRED)

if(NOT TINYXML2_INCLUDE_DIR)
    set(THIRDPARTY_TinyXML2 ON)
    eprosima_find_thirdparty(TinyXML2 tinyxml2)
endif()

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
set(EXAMPLES_INSTALL_DIR examples/ CACHE PATH "Installation directory for examples")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")

add_definitions(-DEPROSIMA_SHAPES_DLL_EXPORT)
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

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND ${PROJECT_NAME}_source_files
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
    set_sources(${TINYXML2_SOURCE_DIR}/tinyxml2.cpp)
endif()

# Add sources to Makefile.am
set_sources(${${PROJECT_NAME}_source_files})

# Create libraries
if(NOT EPROSIMA_INSTALLER)
    # Testing
    option(BUILD_TESTS "Build tests" OFF)
	
    if(EPROSIMA_BUILD)
		SET(BUILD_TESTS ON)
    endif()

    if(BUILD_TESTS)
        SET(GTEST_INDIVIDUAL TRUE)
        enable_testing()
        include(CTest)
        add_subdirectory(test)
    endif()
endif()

# Examples
option(COMPILE_EXAMPLES "Build example" OFF)
if(COMPILE_EXAMPLES)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    endif()
    add_subdirectory(examples)
endif()

# Executable and library
add_executable(${PROJECT_NAME} src/main.cpp)

if (WIN32)
    add_library(${SHAREDLIB_NAME} STATIC
        ${${PROJECT_NAME}_source_files}
        )
else()
    add_library(${SHAREDLIB_NAME} SHARED
#        $<$<NOT:$<BOOL:${WIN32}>>:SHARED>
        ${${PROJECT_NAME}_source_files}
        )
endif()

target_include_directories(${SHAREDLIB_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE $<BUILD_INTERFACE:${TINYXML2_INCLUDE_DIR}>
    )

target_link_libraries(${SHAREDLIB_NAME} PUBLIC
        fastrtps
        fastcdr
        $<$<NOT:$<BOOL:${TINYXML2_INCLUDE_DIR}>>:tinyxml2>
        ${CMAKE_DL_LIBS}
        $<$<BOOL:${WIN32}>:IPHLPAPI$<SEMICOLON>shlwapi>
        $<$<NOT:$<BOOL:${WIN32}>>:-L/usr/lib/x86_64-linux-gnu>
        )

target_link_libraries(${PROJECT_NAME} ${SHAREDLIB_NAME})

if(TINYXML2_LIBRARY)
    target_link_libraries(${PROJECT_NAME} ${PRIVACY}
        ${TINYXML2_LIBRARY}
        )
endif()

# Properties
set_target_properties(${SHAREDLIB_NAME} PROPERTIES RELEASE_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
set_target_properties(${SHAREDLIB_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
set_target_properties(${SHAREDLIB_NAME} PROPERTIES DEBUG_POSTFIX d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
    set_target_properties(${SHAREDLIB_NAME} PROPERTIES VS_WINRT_COMPONENT "true")
endif()

###############################################################################
# Compile options
###############################################################################

###############################################################################
# Packaging                                                                   
###############################################################################

if(EPROSIMA_INSTALLER)
    set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
    set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} C++ Headers")
endif()

if(WIN32 AND EPROSIMA_INSTALLER)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

install(TARGETS islib
    EXPORT islib_Targets
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT libraries
    )

install(FILES ${PROJECT_SOURCE_DIR}/build/Release/integration_service.exe
    COMPONENT bin
    DESTINATION ${BIN_INSTALL_DIR}
    )

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_Targets
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT bin
    )

# Install includes
install(DIRECTORY include/
    COMPONENT headers
    DESTINATION ${INCLUDE_INSTALL_DIR}
    FILES_MATCHING PATTERN "*.h"
    )

# Install examples
install(DIRECTORY ${PROJECT_SOURCE_DIR}/examples/
    COMPONENT examples
    DESTINATION ${EXAMPLES_INSTALL_DIR}
    )

### # Export library
### install(EXPORT ${PROJECT_NAME}Targets
###     DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
###     )

# Sources
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src
    COMPONENT sources
    DESTINATION ${INCLUDE_SOURCE_DIR})

# Install licenses
install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
    COMPONENT licenses
    DESTINATION share
    )

set(CPACK_COMPONENT_LICENSES_HIDDEN 1)
set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} examples")
set(CPACK_COMPONENTS_ALL headers sources licenses examples bin)
set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} libraries)

# Generate Windows Installer.
if(WIN32 AND EPROSIMA_INSTALLER)
    # Set product name.
    set(PRODUCT_NAME_SHORT "Integration Service")
    set(PRODUCT_NAME_LONG "Integration Service")

    set(CPACK_GENERATOR NSIS)

    set(CPACK_PROJECT_CONFIG_FILE ${PROJECT_BINARY_DIR}/cmake/packaging/windows/WindowsPackaging.cmake)
    # Setup generic options.
    
    set(CPACK_PACKAGE_NAME ${PRODUCT_NAME_SHORT})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Integration Service")
    set(CPACK_PACKAGE_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT}")
    set(CPACK_PACKAGE_VENDOR "eProsima")
    set(CPACK_PACKAGE_CONTACT "eProsima Support <support@eprosima.com>")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_MAJOR_VERSION})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_MINOR_VERSION})
    #set(CPACK_PACKAGE_VERSION_PATH ${PROJECT_MICRO_VERSION})
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
    set(CPACK_COMPONENTS_LICENSES_HIDDEN 1)

    # Components display.
    set(CPACK_COMPONENT_CMAKE_DISPLAY_NAME "CMake files")
    set(CPACK_COMPONENT_CMAKE_DESCRIPTION "CMake configuration files")
    set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} C++ Headers")
    set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
    set(CPACK_COMPONENT_BIN_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} Executables")
    set(CPACK_COMPONENT_BIN_DISPLAY_NAME "Executables")
    set(CPACK_COMPONENT_SOURCES_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} Source Files")
    set(CPACK_COMPONENT_SOURCES_DISPLAY_NAME "Source Files")
    set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} Libraries")
    set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
    string(TOUPPER "${MSVC_ARCH}" MSVC_ARCH_UPPER)
    set(CPACK_COMPONENT_LICENSES_DISPLAY_NAME "License")
    set(CPACK_COMPONENT_LICENSES_DESCRIPTION "eProsima ${PRODUCT_NAME_SHORT} License")

    # Setup NSIS installer.
    set(CPACK_NSIS_DISPLAY_NAME "${PRODUCT_NAME_LONG} ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "eProsima ${PRODUCT_NAME_LONG} ${PROJECT_VERSION}")
    set(CPACK_NSIS_URL_INFO_ABOUT "www.eprosima.com")
    set(CPACK_NSIS_CONTACT "support@eprosima.com")
    set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/utils/images/icon/eprosima_icon.ico")
    set(CPACK_NSIS_MENU_LINKS 
        "bin/integration_service.exe" "${PRODUCT_NAME_SHORT}"
        "Uninstall.exe" "Uninstall ${PRODUCT_NAME_SHORT}"
        )

    if(CMAKE_CL_64)
        set(CPACK_NSIS_INSTALL_ROOT "$ENV{PROGRAMFILES}/eProsima")
    else() 
        set(PF86_STR "PROGRAMFILES(x86)")
    set(CPACK_NSIS_INSTALL_ROOT "$ENV{${PF86_STR}}/eProsima")
    endif()
    string(REPLACE "\\" "/" CPACK_NSIS_INSTALL_ROOT ${CPACK_NSIS_INSTALL_ROOT} )

    # Setup configure files for NSIS installer.
    configure_file(${PROJECT_SOURCE_DIR}/cmake/packaging/windows/NSISPackaging.cmake.in
        ${PROJECT_BINARY_DIR}/cmake/packaging/windows/NSISPackaging.cmake @ONLY
        )
    # Setup redistributables installation nsh.
    if(${MSVC_ARCH} STREQUAL "x64Win64VS2015")
        set(REDIST_REGISTER "SOFTWARE\\Classes\\Installer\\Dependencies\\,,amd64,14.0,bundle")
    set(REDIST_VERSION "14.0.23506.0")
    set(REDIST_LINK "http://download.microsoft.com/download/C/E/5/CE514EAE-78A8-4381-86E8-29108D78DBD4/VC_redist.x64.exe")
    set(REDIST_EXEC "vc_redist_x64.exe")
    elseif(${MSVC_ARCH} STREQUAL "i86Win32VS2015")
        set(REDIST_REGISTER "SOFTWARE\\Classes\\Installer\\Dependencies\\,,x86,14.0,bundle")
    set(REDIST_VERSION "14.0.23506.0")
    set(REDIST_LINK "http://download.microsoft.com/download/C/E/5/CE514EAE-78A8-4381-86E8-29108D78DBD4/VC_redist.x86.exe")
    set(REDIST_EXEC "vc_redist_i86.exe")
    endif()
    configure_file(${PROJECT_SOURCE_DIR}/cmake/packaging/windows/InstallRedistributables.nsh.in
        ${PROJECT_BINARY_DIR}/cmake/packaging/windows/InstallRedistributables.nsh @ONLY
        )

    # Setup CPack install script
    set(CPACK_INSTALL_SCRIPT "${PROJECT_BINARY_DIR}/cmake/packaging/windows/NSISPackaging.cmake")

    # Update CMAKE_MODULE_PATH to find NSIS.template.in
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/packaging/windows") 

    include(CPack)
endif()