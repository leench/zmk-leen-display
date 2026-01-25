zephyr_library()

zephyr_library_sources_ifdef(
  CONFIG_INPUT_CST816D
  input_cst816d.c
)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})