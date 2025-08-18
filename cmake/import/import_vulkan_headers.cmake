# Vulkan-Headers
# https://github.com/KhronosGroup/Vulkan-Headers
# set(vulkan_headers_install_dir "D:/develop/vulkan/Vulkan-Headers/Vulkan-Headers-1.3.275" CACHE STRING "vulkan headers install dir")
function(import_vulkan_headers vulkan_headers_install_dir)
    add_library(vulkan_headers INTERFACE)
    target_include_directories(vulkan_headers
            INTERFACE "${vulkan_headers_install_dir}/include"
    )
endfunction()
