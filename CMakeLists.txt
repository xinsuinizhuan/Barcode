cmake_minimum_required(VERSION 3.16.6)
project(Barcode)

include(cmake/OpenCVPath.cmake) #even opencv itself do not use cmake_module_path
include(cmake/CMakeHelper.cmake)

set(OPEN_DEBUG ON)
if (OPEN_DEBUG)
    set(CMAKE_BUILD_TYPE "Debug")
    #set it to Debug, it will auto choose *d.dll
else ()
    set(CMAKE_BUILD_TYPE "Release")
    #set it to Release, it will auto choose *.dll
endif ()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(OpenCV_LIBS "${OpenCV_DIR}/x64/vc16/lib")
set(OpenCV_BINS "${OpenCV_DIR}/x64/vc16/bin")

Output_Paths()
find_package(OpenCV REQUIRED)
Output_Paths()

include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBS})

add_executable(${PROJECT_NAME}_Detect src/Detect.cpp)
target_link_libraries(${PROJECT_NAME}_Detect ${OpenCV_LIBS})

add_executable(${PROJECT_NAME}_test src/testenv.cpp)
target_link_libraries(${PROJECT_NAME}_test ${OpenCV_LIBS})

