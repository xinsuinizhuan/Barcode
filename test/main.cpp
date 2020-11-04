/*
 * @Author: your name
 * @Date: 2020-11-04 20:55:54
 * @LastEditTime: 2020-11-04 21:15:34
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \Barcode\test\main.cpp
 */
#include "test_precomp.hpp"

int main(int argc, char **argv) {
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
