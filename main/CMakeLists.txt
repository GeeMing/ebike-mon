file(GLOB APP_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/../app/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/../img/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/../fonts/out/*.c"
)

idf_component_register(SRCS
    "mcu_main.c"
    "lvgl_demo_ui.c"
    ${APP_SRCS}
    ${IMG_SRCS}
    INCLUDE_DIRS ".")

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_LVGL_H_INCLUDE_SIMPLE)
