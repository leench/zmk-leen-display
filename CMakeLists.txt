if(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  zephyr_library()
  zephyr_library_sources(${ZEPHYR_BASE}/misc/empty_file.c)
  zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
  zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
  zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_CMAKE_DIR}/include)
  zephyr_library_include_directories(include)

  # ===== Display driver（必须受 CONFIG 保护）=====
  add_subdirectory(drivers)

  # ===== 核心源文件 =====
  zephyr_library_sources(
      src/custom_status_screen.c
      src/display_rotate_init.c
      src/raw_hid_bridge.c
      src/brightness.c
      src/touch_func.c
  )

  zephyr_library_sources_ifdef(CONFIG_ZMK_WIDGET_CLOCK      src/widgets/clock.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_WIDGET_VOLUME     src/widgets/volume.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_WIDGET_BATTERY    src/widgets/battery.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_WIDGET_LAYER      src/widgets/layer.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_WIDGET_CONNECTION src/widgets/connection.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_WIDGET_SYSICON    src/widgets/sysicon.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_WIDGET_MODIFIERS  src/widgets/modifiers.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_WIDGET_WPM_STATUS src/widgets/wpm_status.c)

  if(CONFIG_ZMK_WIDGET_BONGO_CAT)
    zephyr_library_sources(
        src/widgets/bongo_cat.c
        src/widgets/bongo_cat_images.c
        src/widgets/bongo_cat_images_inverted.c
    )
  endif()

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
# endif()

# ===== LVGL port（必须受 CONFIG 保护）=====
# if(CONFIG_LVGL)
  # add_subdirectory(modules/lvgl)
endif()
