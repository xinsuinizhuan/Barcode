# 编译说明

在`OpenCV_Debug.props` 和 `OpenCV_Release.props` 两个文件中将**OPENCV_DIR**改成本地环境opencv所在目录的环境变量

```xml
<ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>$(OPENCV_DIR)\..\..\include</AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
      <AdditionalLibraryDirectories>$(OPENCV_DIR)\lib</AdditionalLibraryDirectories>
      <AdditionalDependencies>opencv_world450d.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
```

如果能正常运行example程序就没啥问题了

### How To use Cmake
在.CMake下面建一个`OpenCVPath.cmake`,
里面放

``` cmake
set(OpenCV_DIR ${YOUR_OPENCV_DIR})
# 比如"C:/repo/opencv_build/Install/opencv"
```

同时确认环境变量配置了.
然后用visual studio-文件-打开-cmake,打开`CMakeLists.txt`.
即可跑通`BarCodeTest_Dected.exe`