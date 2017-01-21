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


cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME "capture-beward")

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
find_package(Boost 1.60.0 REQUIRED COMPONENTS
    thread system date_time chrono regex filesystem atomic program_options)

if(WIN32)

ExtProjectGit("https://github.com/log4cplus/log4cplus.git" "REL_1_2_0" ${DEPS_ROOT}
    CMAKE_ARGS -DBUILD_SHARED_LIBS=ON -DUNICODE=OFF -DLOG4CPLUS_BUILD_LOGGINGSERVER=OFF -DLOG4CPLUS_BUILD_TESTING=OFF -D_WIN32_WINNT=${TARGET_WINNT})
else()
ExtProjectGit("https://github.com/log4cplus/log4cplus.git" "REL_1_2_0" ${DEPS_ROOT}
    CMAKE_ARGS -DBUILD_SHARED_LIBS=ON -DUNICODE=OFF -DLOG4CPLUS_BUILD_LOGGINGSERVER=OFF -DLOG4CPLUS_BUILD_TESTING=OFF)
endif()
ExtProjectGit("https://github.com/snikulov/soci.git" "soci_mysql" ${DEPS_ROOT}
    CMAKE_ARGS -DSOCI_DB2=OFF -DSOCI_EMPTY=OFF -DSOCI_FIREBIRD=OFF -DSOCI_MYSQL=ON -DSOCI_ODBC=OFF -DSOCI_ORACLE=OFF -DSOCI_POSTGRESQL=OFF -DSOCI_SQLITE3=OFF)
ExtProjectGit("https://github.com/snikulov/tiny-dnn.git" "install_fix" ${DEPS_ROOT} CMAKE_ARGS -DTBB_FIND_REQUIRED=OFF)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(ext/application/include)
include_directories(ext/date)
include_directories(include)
include_directories(${DEPS_ROOT}/include)

link_directories(${DEPS_ROOT}/lib)
link_directories(${DEPS_ROOT}/lib64)
if(NOT WIN32)
link_directories(/usr/lib64/mysql)
endif()


add_definitions(-DBOOST_APPLICATION_FEATURE_NS_SELECT_STD)
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_WIN32_WINNT=0x0600)
endif()
set(HDR_FILES
    include/monitor_queue.hpp
    include/jpeg_writter.hpp
    include/jpeg_writter_ex.hpp
    include/jpeg_writter_sql.hpp
    include/opencv_frame.hpp
    include/algorithm_factory.hpp
    include/algo/od_interface.hpp
    include/algo/hough_circles.hpp
    include/algo/hog_dlib.hpp
    include/pipe/filter.hpp
    include/pipe/pipeline.hpp
    include/pipe/producer.hpp
    )
set(SRC_FILES
    src/algorithm_factory.cpp
    src/main.cpp)

# DLIB inclusion
set(DLIB_JPEG_SUPPORT ON)
set(USE_AVX_INSTRUCTIONS ON)
add_subdirectory(ext/dlib)

#set(LOG4CPLUS_BUILD_TESTING OFF)
#set(LOG4CPLUS_BUILD_LOGGINGSERVER OFF)
#set(UNICODE OFF)
#set(LOG4CPLUS_HAVE_CXX11_SUPPORT ON)
#add_subdirectory(ext/log4cplus)

add_executable(pipeline src/pipeline.cpp)
add_dependencies(pipeline log4cplus.git soci.git tiny-dnn.git)
target_include_directories(pipeline PUBLIC ext/dlib)
target_link_libraries(pipeline
    ${OpenCV_LIBS}
    dlib
    ${Boost_LIBRARIES}
    soci_core
    soci_mysql
    mysqlclient
    ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pipeline
    debug log4cplusD
    optimized log4cplus)
if(WIN32)
    target_link_libraries(pipeline Ws2_32)
else()
    target_link_libraries(pipeline rt)
endif()

add_executable(${PROJECT_NAME} ${HDR_FILES} ${SRC_FILES})
add_dependencies(${PROJECT_NAME} log4cplus.git)
target_include_directories(${PROJECT_NAME} PUBLIC ext/dlib)
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    dlib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME}
    debug log4cplusD
    optimized log4cplus)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} Ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} rt)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy_directory "${DEPS_ROOT}/bin" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    VERBATIM)

add_executable(mysqlstore ${HDR_FILES} src/mysqlstore.cpp)
add_dependencies(mysqlstore log4cplus.git soci.git)
target_link_libraries(mysqlstore
    ${Boost_LIBRARIES}
    soci_core
    soci_mysql
    mysqlclient
    ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(mysqlstore
    debug log4cplusD
    optimized log4cplus)
if(WIN32)
    target_link_libraries(mysqlstore Ws2_32)
else()
    target_link_libraries(mysqlstore rt)
endif()
