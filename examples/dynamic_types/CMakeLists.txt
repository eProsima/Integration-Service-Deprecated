project(helloworldlib)
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

find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)

add_definitions(-DEPROSIMA_SHAPES_DLL_EXPORT  -DBUILD_SHARED_LIBS)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS TRUE)

file(GLOB USER_LIB_SOURCES_CXX "*.cxx")
file(GLOB USER_LIB_SOURCES_CPP "*.cpp")

add_library(keyhelloworldlib SHARED ${USER_LIB_SOURCES_CXX} ${USER_LIB_SOURCES_CPP})
if(WIN32)
	target_include_directories(keyhelloworldlib PUBLIC ../../../../../install/include)
endif()

target_link_libraries(keyhelloworldlib fastrtps fastcdr ${CMAKE_DL_LIBS})
