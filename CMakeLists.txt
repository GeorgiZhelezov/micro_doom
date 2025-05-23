cmake_minimum_required(VERSION 3.20.0)

if(BOARD MATCHES "^esp32s3_devkitc")
	set(SEG_REV_LIST "dongle" "nodongle")

	string(REGEX MATCH "^[^@]+" SEG_BOARD ${BOARD})
	string(REGEX MATCH "/.*$" SEG_QUALIFIERS ${BOARD})

	string(REGEX MATCH "@([^/]+)" SEG_REV ${BOARD})

	if(SEG_REV)
		set(SEG_REV_STRIPPED "${CMAKE_MATCH_1}")

		string(REGEX REPLACE "@[^/]+" "" BOARD ${BOARD})

		set(BOARD ${BOARD} CACHE STRING "Modified board without specifier" FORCE)

		set(TEMP_CONF "${SEG_BOARD}_${SEG_REV_STRIPPED}.conf")
		string(REPLACE "/" "_" TEMP_CONF ${TEMP_CONF})
		set(TEMP_CONF "boards/${TEMP_CONF}")

		set(TEMP_OVERLAY "${SEG_BOARD}_${SEG_REV_STRIPPED}.overlay")
		string(REPLACE "/" "_" TEMP_OVERLAY ${TEMP_OVERLAY})
		set(TEMP_OVERLAY "boards/${TEMP_OVERLAY}")

		set(DTC_OVERLAY_FILE ${TEMP_OVERLAY})
		set(CONF_FILE ${TEMP_CONF})
	else()
		message(FATAL_ERROR "No revision given for ${BOARD} from list: ${SEG_REV_LIST}")
	endif()
endif()

# message(STATUS "FINAL BOARD: ${BOARD}")
# message(STATUS "FINAL overlay: ${DTC_OVERLAY_FILE}")
# message(STATUS "FINAL conf: ${CONF_FILE}")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(micro_doom)

if(GITHUB_ACTIONS_BUILD)
	add_definitions(-DCONFIG_GITHUB_ACTIONS_BUILD)
endif()

include_directories(
	inc
	wad
	port/src
	port/Doom/include
)

target_sources(app PRIVATE 
	src/user_main.c
	src/user_display.c
	src/user_flash.c
	src/user_time.c
	src/user_debug.c
	src/user_controls.c
	src/user_game.c

	port/src/display.c
	port/src/main.c
	port/src/printf.c
	# port/src/usb.c
	# port/src/usbCdc.c
	# port/src/usbDeviceImplementation.c
	# port/src/ymodem.c
	port/src/display.c
	port/src/font8x8.c
	port/src/graphics.c
	port/src/pwm_audio.c
	port/src/qspi.c
	port/src/keyboard.c

	port/Doom/source/am_map.c
	port/Doom/source/d_client.c
	port/Doom/source/d_items.c
	port/Doom/source/d_main.c
	port/Doom/source/doom_iwad.c
	port/Doom/source/f_finale.c
	port/Doom/source/f_wipe.c
	port/Doom/source/g_game.c
	port/Doom/source/global_data.c
	port/Doom/source/hu_lib.c
	port/Doom/source/hu_stuff.c
	port/Doom/source/i_audio.c
	port/Doom/source/i_main.c
	port/Doom/source/i_spi_support.c
	port/Doom/source/i_video.c
	port/Doom/source/info.c
	port/Doom/source/m_bbox.c
	port/Doom/source/m_cheat.c
	port/Doom/source/m_menu.c
	port/Doom/source/m_random.c
	port/Doom/source/p_ceilng.c
	port/Doom/source/p_doors.c
	port/Doom/source/p_enemy.c
	port/Doom/source/p_floor.c
	port/Doom/source/p_genlin.c
	port/Doom/source/p_inter.c
	port/Doom/source/p_lights.c
	port/Doom/source/p_map.c
	port/Doom/source/p_maputl.c
	port/Doom/source/p_mobj.c
	port/Doom/source/p_plats.c
	port/Doom/source/p_pspr.c
	port/Doom/source/p_setup.c
	port/Doom/source/p_sight.c
	port/Doom/source/p_spec.c
	port/Doom/source/p_switch.c
	port/Doom/source/p_telept.c
	port/Doom/source/p_tick.c
	port/Doom/source/p_user.c
	port/Doom/source/r_data.c
	port/Doom/source/r_draw.c
	port/Doom/source/r_fast_stuff.c
	port/Doom/source/r_main.c
	port/Doom/source/r_patch.c
	port/Doom/source/r_plane.c
	port/Doom/source/r_things.c
	port/Doom/source/s_sound.c
	port/Doom/source/sounds.c
	port/Doom/source/st_gfx.c
	port/Doom/source/st_lib.c
	port/Doom/source/st_stuff.c
	port/Doom/source/tables.c
	port/Doom/source/v_video.c
	port/Doom/source/version.c
	port/Doom/source/w_wad.c
	port/Doom/source/wi_stuff.c
	port/Doom/source/z_bmalloc.c
	port/Doom/source/z_zone.c
)