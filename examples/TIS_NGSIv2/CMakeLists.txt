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
file(GLOB USER_LIB_SOURCES_CPP "jsoncpp.cpp")

##list(REMOVE_ITEM USER_LIB_SOURCES_CPP "userlib_ngsiv2.cpp" "userlib_fastrtps.cpp")
##list(FILTER USER_LIB_SOURCES_CPP EXCLUDE REGEX "**/userlib_*.cpp")

MESSAGE( STATUS "CPP SOURCES: " ${USER_LIB_SOURCES_CPP} )

add_library(userlib_fastrtps SHARED ${USER_LIB_SOURCES_CXX} ${USER_LIB_SOURCES_CPP} "userlib_fastrtps.cpp")
add_library(userlib_ngsiv2 SHARED ${USER_LIB_SOURCES_CXX} ${USER_LIB_SOURCES_CPP} "userlib_ngsiv2.cpp")

target_link_libraries(userlib_fastrtps fastrtps fastcdr ${CMAKE_DL_LIBS})
target_link_libraries(userlib_ngsiv2 fastrtps fastcdr ${CMAKE_DL_LIBS})
