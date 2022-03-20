add_subdirectory_ifdef(CONFIG_ZMOD4510 zmod4510)
add_subdirectory_ifdef(CONFIG_SPS30 sps30)

if(CONFIG_CUSTOM_SENSOR_SHELL)
  zephyr_library_sources_ifdef(CONFIG_CUSTOM_SENSOR_SHELL sensor_shell.c)
endif()

