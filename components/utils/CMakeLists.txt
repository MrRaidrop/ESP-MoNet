idf_component_register(
    SRCS
        src/json_utils.c
        src/ble_format_utils.c
    INCLUDE_DIRS "include"
    # pure C API，no out dependency
)
