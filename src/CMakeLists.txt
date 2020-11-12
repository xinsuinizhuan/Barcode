cmake_minimum_required(VERSION 3.16.6)
aux_source_directory(decoder SRC_FILES)
aux_source_directory(detector SRC_FILES)
set(SRC_FILES "${SRC_FILES};./barcode.cpp")

add_library(BarCode "")
target_sources(
        BarCode
        PRIVATE
        ${SRC_FILES}
        PUBLIC
        ${INCLUDE_DIRECTORIES}
)
target_link_libraries(BarCode ${OpenCV_LIBS})


add_executable(${PROJECT_NAME}_main_debug
        ${SRC_FILES}
        ./main_debug.cpp
        )

target_link_libraries(${PROJECT_NAME}_main_debug ${OpenCV_LIBS})
