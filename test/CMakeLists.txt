Output_variable(INCLUDE_DIRECTORIES)
set(INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES};./test_precomp.hpp")
Output_variable(INCLUDE_DIRECTORIES)

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
        ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/unit_tests.exe
)
target_link_libraries(unit_tests ${OpenCV_LIBS})

# TODO:ADD MORE TEST CASE