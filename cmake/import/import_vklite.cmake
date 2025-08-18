# vklite
# https://github.com/XingRay/vklite
function(import_vklite vklite_install_dir)
    message("import_vklite: vklite_install_dir: ${vklite_install_dir}")
    set(vklite_DIR ${vklite_install_dir}/lib/cmake)
    find_package(vklite)
    if (NOT TARGET vklite)
        message(FATAL_ERROR "vklite is not target !")
    endif ()
endfunction()