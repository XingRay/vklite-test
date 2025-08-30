cmake_minimum_required(VERSION 4.0)
project(vklite_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if (WIN32)
    # Windows 特定编译选项
    add_compile_options(
            /W4 /WX

            # 关闭 C4100（未使用参数）警告
            /wd4100
            /wd4267
    )

endif ()


# config
option(BUILD_TEST "build test" ON)

# vulkan sdk
# https://vulkan.org/
set(vulkan_install_dir "D:/develop/vulkan/VulkanSDK/1.4.321.1" CACHE STRING "vulkan install dir")

# Vulkan-Headers
# https://github.com/KhronosGroup/Vulkan-Headers
set(vulkan_headers_install_dir "D:/develop/vulkan/Vulkan-Headers/Vulkan-Headers-1.3.275" CACHE STRING "vulkan headers install dir")

# glfw
# https://github.com/glfw/glfw
set(glfw_install_dir "D:/develop/opengl/glfw/glfw-3.4.bin.WIN64" CACHE STRING "glfw install dir")

# stb
# https://github.com/nothings/stb
set(stb_install_dir "D:/develop/opengl/stb" CACHE STRING "stb install dir")

# glm
# https://github.com/g-truc/glm
set(glm_install_dir "D:/develop/opengl/glm/glm-1.0.1-light" CACHE STRING "glm install dir")

# tinyobjloader
# https://github.com/tinyobjloader/tinyobjloader
set(tinyobjloader_install_dir "D:/develop/opengl/tinyobjloader" CACHE STRING "tinyobjloader install dir")

# vklite
set(vklite_install_dir "D:/code/git/github/vklite/build/dist/vklite")


include(cmake/import/import_vulkan.cmake)
import_vulkan(${vulkan_install_dir})

include(cmake/import/import_vulkan_headers.cmake)
import_vulkan_headers(${vulkan_headers_install_dir})

include(cmake/import/import_glm.cmake)
import_glm(${glm_install_dir})

include(cmake/import/import_glfw.cmake)
import_glfw(${glfw_install_dir})

include(cmake/import/import_stb.cmake)
import_stb(${stb_install_dir})

include(cmake/import/import_tinyobjloader.cmake)
import_tinyobjloader(${tinyobjloader_install_dir})

include(cmake/import/import_vklite.cmake)
import_vklite(${vklite_install_dir})

add_subdirectory(src)