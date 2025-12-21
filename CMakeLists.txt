zephyr_library()
zephyr_include_directories(include)

add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/drivers/display)
add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/modules/lvgl)

zephyr_library_sources(
    src/custom_status_screen.c
    src/display_rotate_init.c
    src/raw_hid_bridge.c
    src/brightness.c

    src/widgets/clock.c
    src/widgets/volume.c
    src/widgets/battery.c
    src/widgets/layer.c
    src/widgets/connection.c
    src/widgets/bongo_cat.c
    src/widgets/bongo_cat_images_inverted.c
    src/widgets/sysicon.c
    src/widgets/modifiers.c
)

target_sources(app PRIVATE
    src/fonts/lv_font_jetbrainsmono_16.c
    src/fonts/lv_font_jetbrainsmono_20.c
    src/fonts/lv_font_jetbrainsmono_24.c
    src/fonts/lv_font_jetbrainsmono_40.c
    src/fonts/lv_font_montserrat_custom_24.c
    src/fonts/nerd_modifiers_28.c
    src/fonts/lv_font_symbol_16.c
)
