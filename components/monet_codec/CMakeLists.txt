# components/monet_codec/CMakeLists.txt
idf_component_register(
    SRCS "src/json_encoder.c"
    INCLUDE_DIRS "include"
    REQUIRES monet_core utils
)
