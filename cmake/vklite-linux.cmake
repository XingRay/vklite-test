# Linux平台配置
message("platform: linux")
target_compile_definitions(${PROJECT_NAME} PRIVATE VKLITE_LINUX)

find_package(Vulkan REQUIRED)

# Linux平台特定配置
function(configure_vklite_target target)
    target_include_directories(${target} PRIVATE
            ${Vulkan_INCLUDE_DIRS}
    )

    target_link_libraries(${target} PRIVATE
            ${Vulkan_LIBRARIES}
    )

    # Linux特定编译选项
    target_compile_definitions(${target} PRIVATE
            LINUX
            VK_USE_PLATFORM_XLIB_KHR
    )
endfunction()