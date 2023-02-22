# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gez/sdks/zephyr/modules/hal/espressif/components/bootloader/subproject"
  "/home/gez/workspaces/eclipse/esp_doom/build/esp-idf/build/bootloader"
  "/home/gez/workspaces/eclipse/esp_doom/build/esp-idf"
  "/home/gez/workspaces/eclipse/esp_doom/build/esp-idf/tmp"
  "/home/gez/workspaces/eclipse/esp_doom/build/esp-idf/src/EspIdfBootloader-stamp"
  "/home/gez/workspaces/eclipse/esp_doom/build/esp-idf/src"
  "/home/gez/workspaces/eclipse/esp_doom/build/esp-idf/src/EspIdfBootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gez/workspaces/eclipse/esp_doom/build/esp-idf/src/EspIdfBootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gez/workspaces/eclipse/esp_doom/build/esp-idf/src/EspIdfBootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
