# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "esp-idf/esptool_py/flasher_args.json.in"
  "esp-idf/mbedtls/x509_crt_bundle"
  "flash_app_args"
  "flash_bootloader_args"
  "flasher_args.json"
  "https_server.crt.S"
  "ldgen_libraries"
  "ldgen_libraries.in"
  "littlefs_py_venv"
  "micro_speech.map"
  "no_1000ms.wav.S"
  "no_30ms.wav.S"
  "noise_1000ms.wav.S"
  "project_elf_src_esp32s3.c"
  "rmaker_claim_service_server.crt.S"
  "rmaker_mqtt_server.crt.S"
  "rmaker_ota_server.crt.S"
  "silence_1000ms.wav.S"
  "x509_crt_bundle.S"
  "yes_1000ms.wav.S"
  "yes_30ms.wav.S"
  )
endif()
