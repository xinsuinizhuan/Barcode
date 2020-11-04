cmake_minimum_required(VERSION 3.16.6)
project(Barcode)
set(CMAKE_CXX_STANDARD 11)

include(cmake/OpenCVPath.cmake) #even opencv itself do not use cmake_module_path
include(cmake/CMakeHelper.cmake)
include(cmake/crossPlatform.cmake)
include(cmake/buildType.cmake)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

Output_variable(CMAKE_BUILD_TYPE)

#set(OpenCV_LIBS "${OpenCV_DIR}/x64/vc16/lib")
set(OpenCV_BINS "${OpenCV_DIR}/x64/vc16/bin")

Output_Paths()
find_package(OpenCV REQUIRED)
Output_Paths()

include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBS})
add_subdirectory(src)

include(cmake/googletest.cmake)
fetch_googletest(
        ${PROJECT_SOURCE_DIR}/cmake
        ${PROJECT_BINARY_DIR}/googletest
)

enable_testing()
add_subdirectory(test)

