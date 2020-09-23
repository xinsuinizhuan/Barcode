#!/bin/bash -e
# 1. You must download cmake (version >=3.9.1) and install it. You must add cmake to PATH variable during installation
# 2. You must install git-bash (version>=2.14.1). Don't add git to PATH variable during installation
# 3. 修改myRepo=本地要安装的地址，默认pwd
# 4. Windows自动配置环境脚本，修改CMAKE_CONFIG_GENERATOR为正在使用的Visual studio 版本
# 5. 在git bash 中运行此脚本 ./installOCV
myRepo=$(pwd)
CMAKE_CONFIG_GENERATOR="Visual Studio 16 2019"
if [  ! -d "$myRepo/opencv"  ]; then
    echo "cloning opencv"
    git clone https://gitclone.com/github.com/opencv/opencv.git
    mkdir -p Build/opencv
    mkdir -p Install/opencv
else
    cd opencv
    git pull --rebase
    cd ..
fi
if [  ! -d "$myRepo/opencv_contrib"  ]; then
    echo "cloning opencv_contrib"
    git clone https://gitclone.com/github.com/opencv/opencv_contrib.git
    mkdir -p Build/opencv_contrib
else
    cd opencv_contrib
    git pull --rebase
    cd ..
fi
# 检查配置目录是否创建好了
if [ ! -d "$myRepo/Build/opencv" ] || [ ! -d "$myRepo/Install/opencv" ] || [ ! -d "$myRepo/Build/opencv_contrib" ]; then
	echo "no env dir. creating..."
	mkdir -p Build/opencv
	mkdir -p Install/opencv
	mkdir -p Build/opencv_contrib
fi
RepoSource=opencv
pushd Build/$RepoSource
CMAKE_OPTIONS='-DBUILD_PERF_TESTS:BOOL=OFF -DBUILD_TESTS:BOOL=OFF -DBUILD_DOCS:BOOL=OFF  -DWITH_CUDA:BOOL=OFF -DBUILD_EXAMPLES:BOOL=ON -DINSTALL_CREATE_DISTRIB=ON'
cmake -G"$CMAKE_CONFIG_GENERATOR" $CMAKE_OPTIONS -DOPENCV_EXTRA_MODULES_PATH="$myRepo"/opencv_contrib/modules -DCMAKE_INSTALL_PREFIX="$myRepo"/install/"$RepoSource" "$myRepo/$RepoSource"
echo "************************* $Source_DIR -->debug"
cmake --build .  --config debug
echo "************************* $Source_DIR -->release"
cmake --build .  --config release
cmake --build .  --target install --config release
cmake --build .  --target install --config debug
popd
