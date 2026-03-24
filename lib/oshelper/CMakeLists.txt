set(FREERTOS_DETECTED FALSE)
CreateStaticModule(
        TARGET_NAME "OsHelper"
        SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src
        INCLUDES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc
        LINK_LIB XContainer XCallable FreeRTOS
)
# 方法3：检查是否存在FreeRTOS库目标
if(TARGET FreeRTOS)
    set(FREERTOS_DETECTED TRUE)
endif()

# 如果找到FreeRTOS.h文件，也认为检测到了FreeRTOS
if(FREERTOS_H_FOUND)
    set(FREERTOS_DETECTED TRUE)
endif()

# 根据检测结果设置编译定义
if(FREERTOS_DETECTED)
    target_compile_definitions(OsHelper PUBLIC USE_FREERTOS)
    message(STATUS "Unit库: 检测到FreeRTOS，已自动启用USE_FREERTOS宏")
else()
    message(STATUS "Unit库: 未检测到FreeRTOS，某些功能可能不可用")
endif()
