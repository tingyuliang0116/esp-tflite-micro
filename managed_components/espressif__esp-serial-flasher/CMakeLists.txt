cmake_minimum_required(VERSION 3.5)

set(srcs
    src/esp_loader.c
    src/esp_targets.c
    src/md5_hash.c
    src/protocol.c
    src/slip.c
    port/esp32_port.c
)

set(priv_requires driver esp_timer arduino esp-sr esp-tflite-micro)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS include port
    PRIV_INCLUDE_DIRS private_include
    PRIV_REQUIRES ${priv_requires}
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    MD5_ENABLED=$<BOOL:${CONFIG_SERIAL_FLASHER_MD5_ENABLED}>
    SERIAL_FLASHER_RESET_HOLD_TIME_MS=$<IF:$<BOOL:${CONFIG_SERIAL_FLASHER_RESET_HOLD_TIME_MS}>,${CONFIG_SERIAL_FLASHER_RESET_HOLD_TIME_MS},100>
    SERIAL_FLASHER_BOOT_HOLD_TIME_MS=$<IF:$<BOOL:${CONFIG_SERIAL_FLASHER_BOOT_HOLD_TIME_MS}>,${CONFIG_SERIAL_FLASHER_BOOT_HOLD_TIME_MS},50>
)