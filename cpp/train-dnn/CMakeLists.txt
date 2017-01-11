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
#find_package(OpenCV REQUIRED)
#find_package(Boost 1.60.0 REQUIRED COMPONENTS
#    thread system date_time chrono regex filesystem atomic program_options)

ExtProjectGit("https://github.com/snikulov/tiny-dnn.git" "install_fix" ${DEPS_ROOT} CMAKE_ARGS -DTBB_FIND_REQUIRED=OFF)

#include_directories(${OpenCV_INCLUDE_DIRS})
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
    ${CMAKE_THREAD_LIBS_INIT})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy_directory "${DEPS_ROOT}/bin" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    VERBATIM)
