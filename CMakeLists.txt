# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
# set(EXTRA_PARTITION_TABLE_CSV partitions.csv)
# set(PARTITION_TABLE_CSV ${CMAKE_SOURCE_DIR}/partitions.csv)

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_ble_mqtt_https_sensors)
