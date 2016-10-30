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
include_directories(ext/date)
include_directories(include)

add_definitions(-DBOOST_APPLICATION_FEATURE_NS_SELECT_STD)
add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WS08)

set(HDR_FILES
    include/monitor_queue.hpp
    include/jpeg_writter.hpp
    include/opencv_frame.hpp
    include/algo/od_interface.hpp
    include/algo/hough_circles.hpp
    include/algo/hog_dlib.hpp
)
set(SRC_FILES src/main.cpp)

# DLIB inclusion
set(DLIB_JPEG_SUPPORT ON)
set(USE_AVX_INSTRUCTIONS ON)
add_subdirectory(ext/dlib)

add_executable(${PROJECT_NAME} ${HDR_FILES} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ext/dlib)
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    dlib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
target_link_libraries(${PROJECT_NAME} Ws2_32)
endif()