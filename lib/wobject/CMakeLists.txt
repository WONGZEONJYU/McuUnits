CreateStaticModule(
        TARGET_NAME "WObject"
        SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src
        INCLUDES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc
        LINK_LIB XGlobal
)

target_compile_options(WObject PUBLIC
        -Wno-unused-local-typedefs
        -Wno-ignored-qualifiers
        -Wno-address
)
