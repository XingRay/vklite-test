# https://vulkan.org/
# https://vulkan.lunarg.com/
# https://vulkan.lunarg.com/sdk/home#windows
# set(vulkan_install_dir "D:/develop/vulkan/VulkanSDK/1.3.296.0" CACHE STRING "vulkan install dir")
function(import_vulkan vulkan_install_dir)
    # 创建vulkan 导入库
    add_library(vulkan STATIC IMPORTED)

    target_include_directories(
            vulkan
            INTERFACE ${vulkan_install_dir}/Include
    )

    set_target_properties(vulkan PROPERTIES
            IMPORTED_LOCATION "${vulkan_install_dir}/Lib/vulkan-1.lib"
    )

endfunction()