cmake_minimum_required(VERSION 3.16.6)
aux_source_directory(decoder SRC_FILES)
aux_source_directory(detector SRC_FILES)
add_library(BarCode "")
target_sources(
        BarCode
        PRIVATE
        ${SRC_FILES}
        PUBLIC
        ${INCLUDE_DIRECTORIES}
)
target_link_libraries(BarCode ${OpenCV_LIBS})

