cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME "capture-beward")

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(Boost_USE_STATIC_LIBS ON)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost 1.60.0 REQUIRED COMPONENTS
    thread system date_time chrono regex filesystem atomic program_options)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(ext/application/include)
include_directories(include)

add_definitions(-DBOOST_APPLICATION_FEATURE_NS_SELECT_STD)
add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WS08)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})
