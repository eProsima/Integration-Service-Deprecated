project(TIS_NGSIv2)
cmake_minimum_required(VERSION 2.9)

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

find_package(fastcdr)
find_package(fastrtps)

add_definitions(-DEPROSIMA_USER_DLL_EXPORT  -DBUILD_SHARED_LIBS)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS TRUE)

file(GLOB USER_LIB_SOURCES_CXX "*.cxx")
file(GLOB USER_LIB_SOURCES_CPP "*.cpp")

add_executable(tis_ngsiv2 main.cpp)

add_library(dynamicload ../../src/dynamicload/dynamicload.cpp)
#add_library(tinyxml2 ../../src/tinyxml2/tinyxml2.cpp)
add_library(RSManager ../../src/RSManager.cpp)
add_library(RSBridge ../../src/RSBridge.cpp ../../src/RSBridgeFastRTPSToNGSIv2.cpp ../../src/RSBridgeNGSIv2ToFastRTPS.cpp ../../src/NGSIv2/JsonNGSIv2.cxx ../../src/NGSIv2/JsonNGSIv2PubSubTypes.cxx)
add_library(GenericPubSubTypes ../../src/GenericPubSubTypes.cpp)

add_library(userlib SHARED ${USER_LIB_SOURCES_CXX} ${USER_LIB_SOURCES_CPP})

target_link_libraries(userlib fastrtps fastcdr ${CMAKE_DL_LIBS})

target_link_libraries(dynamicload  ${CMAKE_DL_LIBS})
target_link_libraries(GenericPubSubTypes fastrtps fastcdr)
target_link_libraries(RSBridge GenericPubSubTypes dynamicload -L/usr/lib/x86_64-linux-gnu curl curlpp pthread ${Boost_LIBRARIES} boost_system)
target_link_libraries(RSManager RSBridge tinyxml2)

target_link_libraries(tis_ngsiv2 RSManager)
