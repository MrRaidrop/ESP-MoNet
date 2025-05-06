idf_component_register(
    SRCS
        src/msg_bus.c
        src/service_registry.c
    INCLUDE_DIRS "include"
    PRIV_REQUIRES
        utils 
        monet_hal
)
