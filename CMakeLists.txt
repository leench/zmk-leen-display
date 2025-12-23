zephyr_library()

zephyr_include_directories(include)

if(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  # ===== 核心源文件 =====
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

  # ===== 字体文件：必须也进 zephyr_library，不要碰 app =====
  zephyr_library_sources(
      src/fonts/lv_font_jetbrainsmono_16.c
      src/fonts/lv_font_jetbrainsmono_20.c
      src/fonts/lv_font_jetbrainsmono_24.c
      src/fonts/lv_font_jetbrainsmono_40.c
      src/fonts/lv_font_montserrat_custom_24.c
      src/fonts/nerd_modifiers_28.c
      src/fonts/lv_font_symbol_16.c
  )

  # ===== Display driver（必须受 CONFIG 保护）=====
  add_subdirectory(drivers/display)
# endif()

# ===== LVGL port（必须受 CONFIG 保护）=====
# if(CONFIG_LVGL)
  add_subdirectory(modules/lvgl)
endif()
