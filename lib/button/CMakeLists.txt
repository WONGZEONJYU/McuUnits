CreateStaticModule(
        TARGET_NAME "XButton"
        SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src
        INCLUDES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc
        LINK_LIB XTools XMemory XContainer WObject
)
