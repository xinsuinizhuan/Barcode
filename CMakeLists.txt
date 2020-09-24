# CMakeList.txt: testCmakeOpencv 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.16.6)

project ("BarcodeTest")
include(OpenCVPath.cmake)
MESSAGE(${OpenCV_DIR})

set(OpenCV_LIBS "${OpenCV_DIR}/x64/vc16/lib")
set(OpenCV_BINS "${OpenCV_DIR}/x64/vc16/bin")
MESSAGE("OpenCV_DIR: " ${OpenCV_DIR})
MESSAGE("OpenCV_INCLUDE_DIRS: " ${OpenCV_INCLUDE_DIRS})
MESSAGE("OpenCV_LIBS: " ${OpenCV_LIBS})
MESSAGE("OpenCV_BINS: " ${OpenCV_BINS})

find_package(OpenCV REQUIRED)
MESSAGE("OpenCV_DIR: " ${OpenCV_DIR})
MESSAGE("OpenCV_INCLUDE_DIRS: " ${OpenCV_INCLUDE_DIRS})
MESSAGE("OpenCV_LIBS: " ${OpenCV_LIBS})
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBS})
# 将源代码添加到此项目的可执行文件。
add_executable (${PROJECT_NAME}_Detect "Detect.cpp")

#SET(ENV{CMAKE_INCLUDE_PATH} ${OpenCV_BINS})
#SET(ENV{CMAKE_LIBRARY_PATH} ${OpenCV_BINS})
#SET(ENV{PATH} "${OpenCV_BINS}")
#MESSAGE($ENV{PATH} " System path")
#SET(ENV{PATH} "$ENV{PATH};${OpenCV_BINS}")
#MESSAGE($ENV{PATH} " System path")

#execute_process(
#  COMMAND ${CMAKE_COMMAND} -E environment
#)
# TODO: 如有需要，请添加测试并安装目标。
target_link_libraries(${PROJECT_NAME}_Detect opencv_world450d)