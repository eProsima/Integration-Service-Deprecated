# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.12)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project("domain_change")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}../../cmake/modules)
include(${PROJECT_SOURCE_DIR}/../../cmake/common/eprosima_libraries.cmake)
eprosima_find_package(fastcdr REQUIRED)
eprosima_find_package(fastrtps REQUIRED)

# Set C++11
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

message(STATUS "Configuring DomainExample...")
file(GLOB DOMAINEXAMPLE_SOURCES "*.cxx")

add_definitions(-DBUILD_SHARED_LIBS -Dintegrationservices_EXPORTS)

add_executable(DomainExample ${DOMAINEXAMPLE_SOURCES})
target_include_directories(DomainExample PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(DomainExample fastrtps fastcdr ${Boost_LIBRARIES})

install(TARGETS DomainExample
    RUNTIME DESTINATION examples/${PROJECT_NAME}
    )

install(FILES
    config.xml
    DESTINATION examples/${PROJECT_NAME}
    )