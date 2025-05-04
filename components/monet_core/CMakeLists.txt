idf_component_register(
    SRCS
        src/msg_bus.c
    INCLUDE_DIRS "include"
    PRIV_REQUIRES
        utils 
        monet_hal
)
