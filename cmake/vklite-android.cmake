# Android 平台 Vulkan 配置

message("platform: android")
target_compile_definitions(${PROJECT_NAME}
        PUBLIC VKLITE_ANDROID ANDROID VK_USE_PLATFORM_ANDROID_KHR
)

function(import_vulkan_headers)
    set(VulkanHeadersPath "")

    if (IS_DIRECTORY ${VULKAN_HEADERS_PATH})
        set(VulkanHeadersPath ${VULKAN_HEADERS_PATH})
    endif ()
    if (NOT IS_DIRECTORY ${VulkanHeadersPath})
        set(VulkanHeadersPath "D:/develop/vulkan/Vulkan-Headers/Vulkan-Headers-1.3.275")
    endif ()

    add_library(VulkanHeaders INTERFACE)
    target_include_directories(VulkanHeaders
            INTERFACE "${VulkanHeadersPath}/include"
    )
endfunction()