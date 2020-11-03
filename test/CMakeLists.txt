add_executable(
        unit_tests
        basic_test.cpp
)

target_link_libraries(
        unit_tests
        gtest_main
        BarCode
)
add_test(
        NAME
        unit
        COMMAND
        ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/unit_tests
)
target_link_libraries(unit_tests ${OpenCV_LIBS})

# TODO:ADD MORE TEST CASE