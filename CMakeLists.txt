cmake_minimum_required(VERSION 2.8.3)

###############################################################################
# Version information                                                         #
###############################################################################

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
#set(PROJECT_MAJOR_VERSION 0)
#set(PROJECT_MINOR_VERSION 0)
#set(PROJECT_VERSION_PATCH 0)
#set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_MICRO_VERSION})
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

find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)

SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

# Testing
enable_testing()
include(CTest)
add_subdirectory(test)

set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR include/ CACHE PATH "Installation directory for C++ headers")
set(INCLUDE_SOURCE_DIR src/ CACHE PATH "Installation directory for C++ sources")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")
#set(LICENSE_INSTALL_DIR ./ CACHE PATH "Installation directory for licenses")

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

configure_file(src/log/ISLog.h.in ${PROJECT_SOURCE_DIR}/src/log/ISLog.h)
configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.h)

include(${PROJECT_SOURCE_DIR}/cmake/dev/set_sources.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/dev/generate_msvc_libraries.cmake)

set(${PROJECT_NAME}_source_files
        src/xmlUtils.h
        src/dynamicload/dynamicload.cpp src/dynamicload/dynamicload.h
        src/ISManager.cpp src/ISManager.h src/log/ISLog.h
        src/ISBridge.h
        src/RTPSBridge.cpp src/RTPSBridge.h
        src/RTPSPublisher.cpp src/RTPSPublisher.h
        src/RTPSSubscriber.cpp src/RTPSSubscriber.h
        src/GenericPubSubTypes.cpp src/GenericPubSubTypes.h
        thirdparty/tinyxml2/tinyxml2.cpp thirdparty/tinyxml2/tinyxml2.h
    )

add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty/tinyxml2)

if(WIN32)
    add_library(${SHAREDLIB_NAME} ${${PROJECT_NAME}_source_files})

    include_directories("${fastrtps_INCLUDE_DIR}")
    include_directories("${fastcdr_INCLUDE_DIR}")

    # Made linked libraries PRIVATE to prevent local directories in Windows installer.
    #set(PRIVACY "")
    #if(EPROSIMA_INSTALLER_MINION)
    #    set(PRIVACY "PRIVATE")
    #endif()

    target_link_libraries(${SHAREDLIB_NAME}
        fastrtps
        fastcdr
        ${CMAKE_DL_LIBS}
        IPHLPAPI
        shlwapi)
else()
    add_library(${SHAREDLIB_NAME} SHARED ${${PROJECT_NAME}_source_files})

    target_link_libraries(${SHAREDLIB_NAME}
        fastrtps
        fastcdr
        -L/usr/lib/x86_64-linux-gnu
        ${CMAKE_DL_LIBS})
endif()

# Add sources to Makefile.am
set_sources(${${PROJECT_NAME}_source_files})

target_include_directories(${SHAREDLIB_NAME} PRIVATE thirdparty/tinyxml2)
target_link_libraries(${PROJECT_NAME} ${SHAREDLIB_NAME})

set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers" PARENT_SCOPE)
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} C++ Headers" PARENT_SCOPE)


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
if(MSVC OR MSVC_IDE)
    if(EPROSIMA_INSTALLER)
        generate_msvc_libraries(${MSVC_ARCH})
        #generate_msvc_libraries(x64Win64VS2015)
    else()
        set_target_properties(${SHAREDLIB_NAME} PROPERTIES RELEASE_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
        set_target_properties(${SHAREDLIB_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
        set_target_properties(${SHAREDLIB_NAME} PROPERTIES DEBUG_POSTFIX d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

        if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
            set_target_properties(${SHAREDLIB_NAME} PROPERTIES VS_WINRT_COMPONENT "true")
        endif()
    endif()
endif()

install(
    FILES
        src/xmlUtils.h
        src/ISBridge.h
        src/ISManager.h
        src/GenericPubSubTypes.h
        src/ISManager.h
        src/ISBridge.h
        src/RTPSBridge.h
        src/RTPSPublisher.h
        src/RTPSSubscriber.h
        src/GenericPubSubTypes.h
    COMPONENT headers
    DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME})

install(
    FILES
        src/dynamicload/dynamicload.h
    COMPONENT headers
    DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/dynamicload)


if(EPROSIMA_INSTALLER)
    install(
        FILES
            src/xmlUtils.h
            src/ISBridge.h
            src/ISManager.h
            src/GenericPubSubTypes.h
            src/ISManager.h
            src/ISBridge.h
            src/RTPSBridge.h
            src/RTPSPublisher.h
            src/RTPSSubscriber.h
            src/GenericPubSubTypes.h
        COMPONENT sources
        DESTINATION ${INCLUDE_SOURCE_DIR}/src)

    install(
        FILES
            src/dynamicload/dynamicload.h
        COMPONENT sources
        DESTINATION ${INCLUDE_SOURCE_DIR}/src/dynamicload)

    install(
        FILES
            src/log/ISLog.h
        COMPONENT sources
        DESTINATION ${INCLUDE_SOURCE_DIR}/src/log)

    install(
        FILES
            thirdparty/tinyxml2/tinyxml2.h
        COMPONENT sources
        DESTINATION ${INCLUDE_SOURCE_DIR}/thirdparty/tinyxml2)

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

    install(
        FILES
            thirdparty/tinyxml2/tinyxml2.cpp
        COMPONENT sources
        DESTINATION ${INCLUDE_SOURCE_DIR}/thirdparty/tinyxml2)

    # Install licenses
    install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
        DESTINATION ./
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

set(CPACK_COMPONENTS_ALL headers sources licenses examples)

if(MSVC OR MSVC_IDE)
    #if(EPROSIMA_INSTALLER)
    #    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL}
    #        libraries_i86Win32VS2015
    #        libraries_x64Win64VS2015
    #        )
    #else()
    #    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} libraries_${MSVC_ARCH})
    #endif()
    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} libraries_${MSVC_ARCH})
else()
    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} libraries)
endif()

include(${PROJECT_SOURCE_DIR}/cmake/packaging/eProsimaPackaging.cmake)

# Install libraries
if(MSVC OR MSVC_IDE)
    if(EPROSIMA_INSTALLER)
        #install_msvc_libraries(i86Win32VS2015)
        #install_msvc_libraries(x64Win64VS2015)
        install_msvc_libraries(${MSVC_ARCH})
    else()
        set(DIR_EXTENSION "")
        if(EPROSIMA_INSTALLER_MINION)
            set(DIR_EXTENSION "/${MSVC_ARCH}")
        endif()

        install(EXPORT ${PROJECT_NAME}Targets
            DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}/${PROJECT_NAME}/cmake
            COMPONENT cmake
            )


        # Install pdb files
        #install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug/
        #    DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}
        #    COMPONENT libraries_${MSVC_ARCH}
        #    CONFIGURATIONS Debug
        #    FILES_MATCHING
        #    PATTERN "*.pdb"
        #    )

        string(TOUPPER "${MSVC_ARCH}" MSVC_ARCH_UPPER)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DISPLAY_NAME "${MSVC_ARCH}" PARENT_SCOPE)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries for platform ${MSVC_ARCH}" PARENT_SCOPE)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_GROUP "Libraries" PARENT_SCOPE)
    endif()

    set(CPACK_COMPONENT_GROUP_LIBRARIES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries" PARENT_SCOPE)
endif()

