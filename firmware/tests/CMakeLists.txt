cmake_minimum_required(VERSION 3.16)

project(stackchan_firmware_tests LANGUAGES CXX)

enable_testing()

add_executable(motion_math_test
    motion_math_test.cpp
    ../main/stackchan/motion/motion_math.cpp
)

target_include_directories(motion_math_test PRIVATE
    ../main
)

target_compile_features(motion_math_test PRIVATE cxx_std_17)

add_test(NAME motion_math_test COMMAND $<TARGET_FILE:motion_math_test>)

# Embody Mode drawing on the host: the real LVGL (as configured for the robot, see
# sim/lv_conf.h) and the real SpriteLayer render into memory; the test checks pixels.
set(LVGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../managed_components/lvgl__lvgl)
if(EXISTS ${LVGL_DIR}/lvgl.h)
    project(stackchan_sim LANGUAGES C CXX)
    file(GLOB_RECURSE LVGL_SOURCES ${LVGL_DIR}/src/*.c)
    add_library(lvgl_sim STATIC ${LVGL_SOURCES})
    target_include_directories(lvgl_sim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/sim ${LVGL_DIR})
    target_compile_definitions(lvgl_sim PUBLIC LV_CONF_INCLUDE_SIMPLE)

    add_executable(sprite_layer_test
        sprite_layer_test.cpp
        sim/sim_support.cpp
        ../main/apps/app_embody_mode/sprite_layer.cpp
    )
    target_include_directories(sprite_layer_test PRIVATE
        sim/stubs
        ../main
        ../components/ArduinoJson/src
    )
    target_link_libraries(sprite_layer_test PRIVATE lvgl_sim m)
    target_compile_features(sprite_layer_test PRIVATE cxx_std_17)
    add_test(NAME sprite_layer_test COMMAND $<TARGET_FILE:sprite_layer_test>)
    set_tests_properties(sprite_layer_test PROPERTIES ENVIRONMENT "SIM_ASSETS=${CMAKE_CURRENT_SOURCE_DIR}/data")

    add_executable(readings_view_test
        readings_view_test.cpp
        ../main/apps/app_embody_mode/readings_view.cpp
    )
    target_include_directories(readings_view_test PRIVATE ../main)
    target_link_libraries(readings_view_test PRIVATE lvgl_sim m)
    target_compile_features(readings_view_test PRIVATE cxx_std_17)
    add_test(NAME readings_view_test COMMAND $<TARGET_FILE:readings_view_test>)
endif()

add_executable(bounded_send_test bounded_send_test.cpp)
target_include_directories(bounded_send_test PRIVATE ../vendor/esp-ml307/include)
target_compile_features(bounded_send_test PRIVATE cxx_std_17)
add_test(NAME bounded_send_test COMMAND $<TARGET_FILE:bounded_send_test>)

add_executable(sample_ring_test sample_ring_test.cpp)
target_include_directories(sample_ring_test PRIVATE sim/stubs ../main)
target_compile_features(sample_ring_test PRIVATE cxx_std_17)
add_test(NAME sample_ring_test COMMAND $<TARGET_FILE:sample_ring_test>)
