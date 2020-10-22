

### 开发流程说明

分支管理：

**dev/feature** : 开发分支，最新的功能、优化等统一合入此分支(简称dev)，前身是test

**alpha/v01** : 经集成测试后确认稳定的分支(可以展示的分支)，v01,v02代表版本号，每次迭代在dev分支测试后拉出一个新alpha分支，例如本周日完成一次迭代，三个人一起对dev分支确认测试通过后拉出一个新分支alpha/v02，并将这次迭代新增的功能、优化等添加到开发记录中。例如：

alpha/v01 2020/10/22 识别EAN13

往alpha分支合入bugfix需要通过pr形式，2人以上review



**master**: 经大量测试及代码规范检查之后确认可以合入opencv的分支，由稳定的alpha版本合入。



### How To use Cmake:编译说明

在CMake下面建一个`OpenCVPath.cmake`,
里面放

``` cmake
set(OpenCV_DIR ${YOUR_OPENCV_DIR})
# 比如"C:/repo/opencv_build/Install/opencv"
```

示例

```cmake
set(OpenCV_DIR "C://repo//opencv_build//Install//opencv")
```

同时确认环境变量配置了.(*/opencv/x64/vc16/bin)  
然后用visual studio-文件-打开-cmake,打开`CMakeLists.txt`.  
即可跑通`BarCodeTest_test.exe`.