cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME "pizza-jones")

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(Boost_USE_STATIC_LIBS ON)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})

add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WS08)

set(SRC_FILES src/PizzaJones.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${CMAKE_THREAD_LIBS_INIT})
