### How To use Cmake:编译说明
在CMake下面建一个`OpenCVPath.cmake`,
里面放

``` cmake
set(OpenCV_DIR ${YOUR_OPENCV_DIR})
# 比如"C:/repo/opencv_build/Install/opencv"
```

同时确认环境变量配置了.(*/opencv/x64/vc16/bin)  
然后用visual studio-文件-打开-cmake,打开`CMakeLists.txt`.  
即可跑通`BarCodeTest_test.exe`.