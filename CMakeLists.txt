zephyr_library()
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)

if(CONFIG_SHIELD_ROBA_R AND CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  zephyr_library_sources(src/pointer_guard.c)
endif()
