idf_component_register(
    SRCS
        src/json_utils.c
        src/ble_format_utils.c
        src/cache.c
    INCLUDE_DIRS "include"
    REQUIRES monet_core monet_hal
)
