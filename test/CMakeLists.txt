cmake_minimum_required(VERSION 3.5)
project(test_ble_format)

# 添加测试文件和待测模块
add_executable(test_ble
    test_ble.c
    ../main/utils/ble_format_utils.c
)

# 添加头文件路径
target_include_directories(test_ble PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../main/utils
    unity/src
)
