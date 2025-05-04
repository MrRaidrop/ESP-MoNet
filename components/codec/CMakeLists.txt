# components/codec/CMakeLists.txt
idf_component_register(
    SRCS "src/json_encoder.c"
    INCLUDE_DIRS "include"
    REQUIRES core utils
)
