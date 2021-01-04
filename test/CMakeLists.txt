Output_variable(INCLUDE_DIRECTORIES)
set(INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES};./test_precomp.hpp ./verifyer.hpp")
Output_variable(INCLUDE_DIRECTORIES)

add_executable(
        unit_tests
        #basic_test.cpp
        integration_testing.cpp

        #patternMatchTest.cpp
)
add_executable(${PROJECT_NAME}_main_debug
        main_debug.cpp
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

target_link_libraries(unit_tests BarCode)
target_link_libraries(${PROJECT_NAME}_main_debug BarCode)

SET_TARGET_PROPERTIES(unit_tests PROPERTIES LINK_FLAGS "/PROFILE")
# TODO:ADD MORE TEST CASE