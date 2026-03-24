function(CreateStaticModule)
    set(options "")
    set(oneValueArgs
            TARGET_NAME
            SOURCES_PATH
            INCLUDES_PATH
    )
    set(multiValueArgs
            LINK_LIB
            EXT_LINK_LIB
            EXT_INC_PATH
    )

    cmake_parse_arguments(MOD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    file(GLOB SrcFiles ${MOD_SOURCES_PATH}/*.c*)
    file(GLOB InternalHeaderFiles ${MOD_SOURCES_PATH}/*.h*)
    file(GLOB HeaderFiles ${MOD_INCLUDES_PATH}/*.h*)

    add_library(${MOD_TARGET_NAME} STATIC)

    target_sources(${MOD_TARGET_NAME} PRIVATE
            ${SrcFiles}
            ${InternalHeaderFiles}
            ${HeaderFiles}
    )

    target_link_libraries(${MOD_TARGET_NAME}
            PUBLIC
                ${MOD_LINK_LIB}
            PRIVATE
                ${MOD_EXT_LINK_LIB}
    )

    target_include_directories(${MOD_TARGET_NAME}
            PUBLIC
                ${MOD_INCLUDES_PATH}
            PRIVATE
                ${MOD_EXT_INC_PATH}
                ${MOD_SOURCES_PATH}
    )

endfunction()

function(CreateInterface)
    set(options "")
    set(oneValueArgs
            TARGET_NAME
    )
    set(multiValueArgs INCLUDES_PATH LINK_LIB)

    cmake_parse_arguments(MOD
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )

    add_library(${MOD_TARGET_NAME} INTERFACE)

    target_link_libraries(${MOD_TARGET_NAME} INTERFACE ${MOD_LINK_LIB})

    target_include_directories(${MOD_TARGET_NAME} INTERFACE ${MOD_INCLUDES_PATH})

endfunction()
