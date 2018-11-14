project(helloworld_to_file)
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

if(WIN32)
    set(liblist)
else()
	set(liblist -L/usr/lib/x86_64-linux-gnu)
endif()

file(GLOB USER_LIB_SOURCES_CXX "*.cxx")

add_library(isfile SHARED ${USER_LIB_SOURCES_CXX} ${USER_LIB_SOURCES_CPP}
    FilePublisher.cpp
    isfile.cpp)

target_include_directories(isfile PRIVATE ../../include)

target_link_libraries(isfile fastrtps fastcdr ${liblist} ${CMAKE_DL_LIBS})


install(TARGETS isfile
    LIBRARY DESTINATION examples/${PROJECT_NAME}/build$<$<BOOL:${WIN32}>:/${CMAKE_BUILD_TYPE}>
    )

install(FILES
    config$<$<BOOL:${WIN32}>:_win>.xml
    DESTINATION examples/${PROJECT_NAME}
    )