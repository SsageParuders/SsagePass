# 参考官方文档:https://llvm.org/docs/CMake.html#developing-llvm-passes-out-of-source
## 请确保编译器是来自NDK的clang 自举编译
cmake_minimum_required(VERSION 3.13.4)
# SET (CMAKE_C_COMPILER_WORKS 1)
# SET (CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_C_COMPILER /root/android-ndk-r25/toolchains/llvm/prebuilt/linux-x86_64/bin/clang) # 调用原生NDK的clang
set(CMAKE_CXX_COMPILER /root/android-ndk-r25/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++) # 调用原生NDK的clang++

project(SsagePass) # 项目名称
# we need LLVM_HOME in order not automatically set LLVM_DIR
set(ENV{LLVM_HOME} /root/android-ndk-r25/toolchains/llvm/prebuilt/linux-x86_64) # 指定LLVM_HOME为原生NDK
set(ENV{LLVM_DIR} /root/android_clang_14_06/lib64/cmake/llvm) # 指定cmake文件 如此可以连接到LLVM库

find_package(LLVM REQUIRED CONFIG) # 寻找LLVM依赖包
add_definitions(${LLVM_DEFINITIONS}) # 预定义
include_directories(/root/android_clang_14_06/include) # LLVM的头文件
include_directories("./include") # 包含 ./include 文件夹中的头文件 主要是为Pass项目服务
link_directories(${LLVM_LIBRARY_DIRS}) # LLVM依赖库

## 如果LLVM版本高于10 则需要启用C++14特性
set(CMAKE_CXX_STANDARD 14) # LLVM启用14特性
## 确保使用llvm的libc++ 而不是来自gcc的libstdc++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++") # libc++
include_directories(/root/android_clang_14_06/include/c++/v1) # libc++的头文件包含路径
set(CMAKE_SKIP_RPATH ON)

add_library( SsageObfuscator MODULE # 生成动态库文件
  src/SplitBasicBlock.cpp
  src/Utils.cpp
  src/CryptoUtils.cpp
  src/Flattening.cpp
  src/LegacyLowerSwitch.cpp
  src/PMRegistration.cpp
  src/StringEncryption.cpp
)

# Use C++11 to compile your pass (i.e., supply -std=c++11).
target_compile_features(SsageObfuscator PRIVATE 
  cxx_range_for cxx_auto_type
)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(SsageObfuscator PROPERTIES
  COMPILE_FLAGS "-fno-rtti"
)