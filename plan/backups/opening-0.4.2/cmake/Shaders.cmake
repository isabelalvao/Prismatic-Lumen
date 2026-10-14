set(FANTASYNT_SHADER_OUTPUTS)
foreach(SHADER vs_scene fs_scene fs_ground vs_quad fs_extract fs_blur fs_composite fs_copy)
    if(SHADER MATCHES "^vs_")
        set(SHADER_TYPE vertex)
    else()
        set(SHADER_TYPE fragment)
    endif()
    set(SHADER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER}.bin")
    add_custom_command(OUTPUT "${SHADER_OUTPUT}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders"
        COMMAND $<TARGET_FILE:shaderc>
            -f "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER}.sc"
            -o "${SHADER_OUTPUT}" --type ${SHADER_TYPE} --platform windows -p s_5_0
            --varyingdef "${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc"
            -i "${bgfx_cmake_SOURCE_DIR}/bgfx/src"
        DEPENDS shaderc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER}.sc"
            "${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc"
        VERBATIM)
    list(APPEND FANTASYNT_SHADER_OUTPUTS "${SHADER_OUTPUT}")
endforeach()
add_custom_target(fantasynt_shaders DEPENDS ${FANTASYNT_SHADER_OUTPUTS})
