# Running DOOM on [Zephyr](https://zephyrproject.org/)

The goal is to make a port of [next-hack](https://github.com/next-hack/nRF52840Doom)'s port of [GBADoom](https://github.com/doomhack/GBADoom) for 
[his nRF52840 board](https://next-hack.com/index.php/2021/11/13/porting-doom-to-an-nrf52840-based-usb-bluetooth-le-dongle), to my
Raspberry Pi Pico 
(
	[T-Pico shop](https://lilygo.cc/en-bg/products/t-pico?variant=42295946641589) /
	[repo](https://github.com/Xinyuan-LilyGO/T-PicoC3)
),
ESP32 
(
	[T-Display shop](https://lilygo.cc/en-bg/products/lilygo%C2%AE-ttgo-t-display-1-14-inch-lcd-esp32-control-board) /
	[repo](https://github.com/Xinyuan-LilyGO/TTGO-T-Display)
),
ESP32S3
(
	[T-Display-S3 shop](https://lilygo.cc/products/t-display-s3?variant=42284559827125) /
	[repo](https://github.com/Xinyuan-LilyGO/T-Display-S3)
)
boards using the Zephyr RTOS. 

I found these boards on AliExpress form LYLGO and decided that it would be cool to try this, 
given that they come in a variety displays 
(
	[ST7789V](https://newhavendisplay.com/content/datasheets/ST7789V.pdf),
	[ST7735](https://www.displayfuture.com/Display/datasheet/controller/ST7735.pdf)
	...
)
with different resolutions (135x240, 170x320 ...).
It is important to 
point out that I have the 16MB flash variants because I would like to make the whole game playable on 
them with some controller over Bluetooth (maybe a BLE mouse or I write my own gamepad app with Kotlin?). 
Additionally, the ESP32 doesn't have a lot of RAM, further optimizations will be needed in order to 
play more complex levels in addition to the BLE stack on top of all this. The Pico doesn't have any RF
capabilities, but the board does have an ESP32C3 on the other side so some "transport layer" between both
will be needed (I2C, UART?).

> [!NOTE]
> This project is still in a work-in-progress phase.

## Some things to do:
- make it fully run on `esp32_devkitc_wroom`, `esp32s3_devkitc` and `native_sim`
	- fix all the prohibited loads from pointer dereferencing to FLASH
	- optimize for RAM to fit levels and BLE stack
- add some sort of controller support via BLE
- make audio work
- stream audio via BLE (would require custom implementation) ?
- optimize and cleanup the code
- fix some demos not being in sync compared to other ports
- fix, at times, weird sprite rendering
- fix statusbar for wider screens (320px)

## Building

Firstly, you will have to get the Zephyr SDK on your system 
([Getting Started guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)).
After you can successfully compile, say their hello_world example, you can build this project by doing the following in your terminal:


for the [rpi_pico](https://docs.zephyrproject.org/latest/boards/raspberrypi/rpi_pico/doc/index.html) target:
```
west build -p=always -b rpi_pico
```

for the [esp32_devkitc_wroom](https://docs.zephyrproject.org/latest/boards/espressif/esp32_devkitc_wroom/doc/index.html) target:
```
west build -p=always -b esp32_devkitc_wroom/esp32/procpu
```

for the [esp32s3_devkitc](https://docs.zephyrproject.org/latest/boards/espressif/esp32s3_devkitc/doc/index.html) target:
```
west build -p=always -b esp32s3_devkitc/esp32s3/procpu
```

for the [native_sim](https://docs.zephyrproject.org/latest/boards/native/native_sim/doc/index.html) target:
```
west build -p=always -b native_sim
```

## Flashing the app

By default `west` uses a J-LINK debugger for the `rpi_pico` and for the `esp32_devkitc_wroom` and `esp32s3_devkitc` - the on-board USB to UART converter. You can, of course use whatever method you like with the `-r` argument.
```
west flash
```

## Flashing the demo wad

Use the [supplied shareware wad](wad/doom1_shareware.wad), since it includes modifications from the `MCUDoomWadUtil` 
tool in `next-hack`'s port. 

> [!IMPORTANT]
> All board targets expect the wad to be at flash offset `0x22b000`.

## Running

The `native_sim` target uses the SDL2 library for display drawing and can be ran using:
```
west build -t run
```

## Demo

Ammo shows FPS * 10.

https://github.com/user-attachments/assets/5b878f2c-c894-4278-942b-954c7286ba15
