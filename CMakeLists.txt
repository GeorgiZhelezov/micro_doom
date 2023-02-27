# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BOARD esp32)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(esp_doom)

include_directories(
	./inc
)

target_sources(app PRIVATE src/main.c)
