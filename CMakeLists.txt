# 创建Unit静态库
set(LIBName Units)

# 自动收集所有子目录的源文件和头文件
file(GLOB_RECURSE UNIT_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
)

file(GLOB_RECURSE UNIT_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.hpp
)

#list(FILTER UNIT_HEADERS EXCLUDE REGEX ".*_p\\.h(pp)?$")

# 创建静态库
add_library(${LIBName} STATIC ${UNIT_SOURCES} ${UNIT_HEADERS})

# 自动发现所有子目录并设置为包含路径
file(GLOB UNIT_SUBDIRS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*/include)
message(STATUS ${UNIT_SUBDIRS})
set(UNIT_INCLUDE_DIRS "")
foreach(subdir ${UNIT_SUBDIRS})
    if(IS_DIRECTORY ${subdir})
        list(APPEND UNIT_INCLUDE_DIRS ${subdir})
    endif()
endforeach()

# 设置包含目录
target_include_directories(${LIBName} PUBLIC ${UNIT_INCLUDE_DIRS})

# 链接依赖
target_link_libraries(${LIBName} PUBLIC stm32cubemx)

# 自动检测FreeRTOS并设置编译定义
set(FREERTOS_DETECTED FALSE)

# 方法1：检查是否存在FreeRTOS.h头文件
find_file(FREERTOS_H_FOUND
    NAMES FreeRTOS.h
    PATHS 
        ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
        ${CMAKE_SOURCE_DIR}/Core/Inc
        ${CMAKE_SOURCE_DIR}/Inc
        ${CMAKE_SOURCE_DIR}/include
    NO_DEFAULT_PATH
)

# 方法2：检查是否已经定义了USE_FREERTOS宏
get_target_property(EXISTING_DEFINITIONS ${CMAKE_PROJECT_NAME} COMPILE_DEFINITIONS)
if(EXISTING_DEFINITIONS)
    foreach(def ${EXISTING_DEFINITIONS})
        if(def STREQUAL "USE_FREERTOS")
            set(FREERTOS_DETECTED TRUE)
            break()
        endif()
    endforeach()
endif()

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
    target_compile_definitions(${LIBName} PUBLIC USE_FREERTOS)
    message(STATUS "Unit库: 检测到FreeRTOS，已自动启用USE_FREERTOS宏")
else()
    message(STATUS "Unit库: 未检测到FreeRTOS，某些功能可能不可用")
endif()

# 设置编译选项
target_compile_options(${LIBName} PUBLIC
    -Wno-unused-local-typedefs
    -Wno-ignored-qualifiers
    -Wno-address
)

unset(LIBName)
