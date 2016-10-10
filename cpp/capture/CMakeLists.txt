cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME "capture-beward")

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${CMAKE_THREAD_LIBS_INIT})
