cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME "capture-brawe")

project(${PROJECT_NAME})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})