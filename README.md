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
