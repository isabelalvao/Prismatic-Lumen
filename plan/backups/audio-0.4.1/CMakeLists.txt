cmake_minimum_required(VERSION 3.24)
project(Fantasynt VERSION 0.4.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(FANTASYNT_BUILD_APP "Build the SDL3/bgfx application" OFF)
include(CTest)
include(cmake/Dependencies.cmake)

add_library(fantasynt_control
    src/control/protocol.cpp
    src/control/scene_controller.cpp)
target_include_directories(fantasynt_control PUBLIC src)
target_link_libraries(fantasynt_control PUBLIC nlohmann_json::nlohmann_json)
if(MSVC)
    target_compile_options(fantasynt_control PRIVATE /W4 /permissive- /utf-8)
endif()

if(FANTASYNT_BUILD_APP)
    include(cmake/GraphicsDependencies.cmake)
    include(cmake/Shaders.cmake)
    add_library(fantasynt_openxr src/vr/openxr_session.cpp)
    target_include_directories(fantasynt_openxr PUBLIC src)
    target_link_libraries(fantasynt_openxr PUBLIC fantasynt_vr_math PRIVATE OpenXR::openxr_loader nlohmann_json::nlohmann_json d3d11 dxgi)
    add_library(fantasynt_render src/render/renderer.cpp src/render/mesh.cpp src/render/post_process.cpp)
    target_include_directories(fantasynt_render PUBLIC src)
    target_link_libraries(fantasynt_render PUBLIC fantasynt_control fantasynt_geometry bgfx bimg bx)
    add_executable(fantasynt src/app/main.cpp)
    target_link_libraries(fantasynt PRIVATE fantasynt_render fantasynt_net fantasynt_openxr SDL3::SDL3-static)
    if(MSVC)
        target_compile_options(fantasynt_render PRIVATE /W4 /permissive- /utf-8)
        target_compile_options(fantasynt PRIVATE /W4 /permissive- /utf-8)
    endif()
    add_custom_target(fantasynt_resources
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fantasynt>/shaders"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fantasynt>/examples"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:openxr_loader> "$<TARGET_FILE_DIR:fantasynt>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FANTASYNT_SHADER_OUTPUTS} "$<TARGET_FILE_DIR:fantasynt>/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/examples/calm.json"
            "${CMAKE_SOURCE_DIR}/examples/balanced.json"
            "${CMAKE_SOURCE_DIR}/examples/energetic.json" "$<TARGET_FILE_DIR:fantasynt>/examples"
        DEPENDS fantasynt_shaders openxr_loader ${FANTASYNT_SHADER_OUTPUTS}
            examples/calm.json examples/balanced.json examples/energetic.json
        VERBATIM)
    add_dependencies(fantasynt fantasynt_resources)
    if(BUILD_TESTING)
        add_library(fantasynt_mock_openxr SHARED tests/mock_openxr_runtime.cpp)
        target_link_libraries(fantasynt_mock_openxr PRIVATE OpenXR::headers d3d11 dxgi)
        if(MSVC)
            target_compile_options(fantasynt_mock_openxr PRIVATE /utf-8)
        endif()
        add_executable(fantasynt_renderer_tests tests/renderer_tests.cpp)
        target_include_directories(fantasynt_renderer_tests PRIVATE tests)
        target_link_libraries(fantasynt_renderer_tests PRIVATE fantasynt_render SDL3::SDL3-static bimg_decode)
        add_dependencies(fantasynt_renderer_tests fantasynt_shaders)
        add_test(NAME renderer_resize COMMAND fantasynt_renderer_tests
            "${CMAKE_BINARY_DIR}/shaders" "${CMAKE_SOURCE_DIR}/plan/m1-resize.png")
        set_tests_properties(renderer_resize PROPERTIES TIMEOUT 20)
    endif()
endif()

add_library(fantasynt_net src/net/line_framer.cpp src/net/control_server.cpp)
target_include_directories(fantasynt_net PUBLIC src)
target_link_libraries(fantasynt_net PRIVATE nlohmann_json::nlohmann_json ws2_32)
if(MSVC)
    target_compile_options(fantasynt_net PRIVATE /W4 /permissive- /utf-8)
endif()

add_library(fantasynt_geometry src/render/geometry.cpp src/render/tunnel_geometry.cpp src/render/surface_geometry.cpp
    src/render/landscape_geometry.cpp src/render/landscape_surfaces.cpp src/render/sequence.cpp)
target_include_directories(fantasynt_geometry PUBLIC src)
add_library(fantasynt_vr_math src/vr/view_math.cpp)
target_include_directories(fantasynt_vr_math PUBLIC src)

if(BUILD_TESTING)
    add_executable(fantasynt_sequence_tests tests/sequence_tests.cpp)
    target_include_directories(fantasynt_sequence_tests PRIVATE tests)
    target_link_libraries(fantasynt_sequence_tests PRIVATE fantasynt_geometry)
    add_test(NAME sequence COMMAND fantasynt_sequence_tests)
    add_executable(fantasynt_vr_math_tests tests/vr_math_tests.cpp)
    target_include_directories(fantasynt_vr_math_tests PRIVATE tests)
    target_link_libraries(fantasynt_vr_math_tests PRIVATE fantasynt_vr_math)
    add_test(NAME vr_math COMMAND fantasynt_vr_math_tests)
    add_executable(fantasynt_control_tests tests/control_tests.cpp)
    target_include_directories(fantasynt_control_tests PRIVATE tests)
    target_link_libraries(fantasynt_control_tests PRIVATE fantasynt_control)
    add_test(NAME control COMMAND fantasynt_control_tests)
    add_executable(fantasynt_network_tests tests/network_tests.cpp)
    target_include_directories(fantasynt_network_tests PRIVATE tests)
    target_link_libraries(fantasynt_network_tests PRIVATE fantasynt_net fantasynt_control ws2_32)
    add_test(NAME network COMMAND fantasynt_network_tests)
    set_tests_properties(network PROPERTIES TIMEOUT 15)
    add_executable(fantasynt_geometry_tests tests/geometry_tests.cpp)
    target_include_directories(fantasynt_geometry_tests PRIVATE tests)
    target_link_libraries(fantasynt_geometry_tests PRIVATE fantasynt_geometry)
    add_test(NAME geometry COMMAND fantasynt_geometry_tests)
endif()
