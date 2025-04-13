cmake_minimum_required(VERSION 3.10)
project(MyApp)  # 项目名称应与目标名称一致（或不同，但需确保目标存在）
 
# 指定C++标准（可选但推荐）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
 
# 收集所有源文件（推荐显式列出或使用更精确的GLOB）
file(GLOB SOURCES 
    "src/*.c"    # 非递归收集src下的.cpp文件
    "*.c"        # 当前目录下的.cpp文件
    "src/*.cpp"
    "main.cpp"
)
 
# 先创建目标
add_executable(MyApp ${SOURCES})
 
# 再为目标配置头文件路径
target_include_directories(MyApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
