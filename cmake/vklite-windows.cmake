# Windows 平台 Vulkan SDK 配置

function(import_vulkan)

    set(Vulkan_sdk_dir "")

    if (IS_DIRECTORY ${VULKAN_SDK_PATH})
        set(Vulkan_sdk_dir ${VULKAN_SDK_PATH})
    endif ()

    if (NOT IS_DIRECTORY ${Vulkan_sdk_dir})
        message(FATAL_ERROR "Vulkan_sdk_dir:${Vulkan_sdk_dir} is not dir")
    endif ()
    message("Vulkan_sdk_dir:${Vulkan_sdk_dir}")

    # 创建vulkan 导入库
    add_library(vulkan STATIC IMPORTED)

    message("vulkan include dir: ${Vulkan_sdk_dir}/Include")
    # 引入头文件方式 1
    target_include_directories(
            vulkan
            INTERFACE ${Vulkan_sdk_dir}/Include
    )

    set_target_properties(vulkan PROPERTIES
            IMPORTED_LOCATION "${Vulkan_sdk_dir}/Lib/vulkan-1.lib"
    )

endfunction()


function(import_glfw)
    set(glfw_dir "")

    if (IS_DIRECTORY ${glfw_PATH})
        set(glfw_dir ${glfw_PATH})
    endif ()

    if (NOT IS_DIRECTORY ${glfw_dir})
        message(FATAL_ERROR "glfw_dir:${glfw_dir} is not dir")
    endif ()
    message("glfw_dir:${glfw_dir}")

    set(GLFW_LIB_DIR ${glfw_dir}/lib-vc2022)
    set(GLFW_INCLUDE_DIR ${glfw_dir}/include)

    add_library(glfw STATIC IMPORTED)

    set_target_properties(glfw PROPERTIES
            IMPORTED_LOCATION "${GLFW_LIB_DIR}/glfw3.lib"
    )

    target_include_directories(glfw
            INTERFACE ${GLFW_INCLUDE_DIR}
    )

endfunction()
