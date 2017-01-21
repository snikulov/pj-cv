#////////////////////////////////////////////////////////////////////////////////////////////////
#///	Copyright (C) 2016-2017, Sergei Nikulov (sergey.nikulov@gmail.com)
#///	All rights reserved.
#///
#///	Redistribution and use in source and binary forms, with or without
#///	modification, are permitted provided that the following conditions are met:
#///	* Redistributions of source code must retain the above copyright
#///	notice, this list of conditions and the following disclaimer.
#///	* Redistributions in binary form must reproduce the above copyright
#///	notice, this list of conditions and the following disclaimer in the
#///	documentation and/or other materials provided with the distribution.
#///	* Neither the name of the <organization> nor the
#///	names of its contributors may be used to endorse or promote products
#///	derived from this software without specific prior written permission.
#///
#///	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
#///	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#///	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#///	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
#///	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#///	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#///	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#///	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#///	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#///	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#////////////////////////////////////////////////////////////////////////////////////////////////


cmake_minimum_required(VERSION 3.2)
set(PROJECT_NAME "train-dnn")

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include(ExtProjectUtils)

# set output dir for binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(TARGET_WINNT 0x0600)

set(DEPS_ROOT "${CMAKE_BINARY_DIR}/deps")

set(Boost_USE_STATIC_LIBS ON)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(Boost 1.60.0 REQUIRED COMPONENTS
#    thread system date_time chrono regex filesystem atomic program_options)

ExtProjectGit("https://github.com/snikulov/tiny-dnn.git" "install_fix" ${DEPS_ROOT} CMAKE_ARGS -DTBB_FIND_REQUIRED=OFF)

include_directories(${OpenCV_INCLUDE_DIRS})
#include_directories(${Boost_INCLUDE_DIRS})
#include_directories(ext/application/include)
#include_directories(ext/date)

include_directories(include)
include_directories(${DEPS_ROOT}/include)

link_directories(${DEPS_ROOT}/lib)
link_directories(${DEPS_ROOT}/lib64)


add_definitions(-DBOOST_APPLICATION_FEATURE_NS_SELECT_STD)
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_WIN32_WINNT=0x0600)
endif()

set(SRC_FILES
    src/main.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OpenCV_LIBS})
add_dependencies(${PROJECT_NAME} tiny-dnn.git)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy_directory "${DEPS_ROOT}/bin" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    VERBATIM)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()
