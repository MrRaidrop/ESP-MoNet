# Stable C path — always compiled.
set(srcs
    src/msg_bus.c
    src/service_registry.c)
set(include_dirs include)

# Optional experimental C++ demo layer. Only compiled when the Kconfig
# option is enabled, so the default build has zero C++ cost.
if(CONFIG_MONET_CPP_EXPERIMENTAL)
    list(APPEND srcs
        src_cpp/CameraFrame.cpp
        src_cpp/Logger.cpp
        src_cpp/MessageBus.cpp
        src_cpp/button_isr_demo.cpp
        src_cpp/cpp_demo.cpp)
    list(APPEND include_dirs include_cpp)
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES
        utils
        monet_hal
        esp_driver_gpio     # button_isr_demo: GPIO interrupt
)
