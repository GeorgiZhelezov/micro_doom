#include <zephyr/device.h>
#include <zephyr/toolchain.h>

/* 1 : /soc/rtc@3ff48000:
 * Supported:
 *    - /soc/uart@3ff40000
 *    - /soc/spi@3ff65000
 *    - /soc/spi@3ff64000
 */
extern const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2"))) __devicehdl_dts_ord_5[6];
const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_dts_ord_5[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 5, 6, 7, DEVICE_HANDLE_ENDS };

/* 2 : /soc/gpio/gpio@3ff44800:
 */
extern const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2"))) __devicehdl_dts_ord_74[3];
const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_dts_ord_74[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 3 : /soc/gpio/gpio@3ff44000:
 * Supported:
 *    - /soc/spi@3ff64000/st7789v@0
 */
extern const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2"))) __devicehdl_dts_ord_11[4];
const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_dts_ord_11[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 8, DEVICE_HANDLE_ENDS };

/* 4 : /soc/trng@3ff75144:
 */
extern const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2"))) __devicehdl_dts_ord_60[3];
const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_dts_ord_60[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 5 : /soc/uart@3ff40000:
 * Direct Dependencies:
 *    - /soc/rtc@3ff48000
 */
extern const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2"))) __devicehdl_dts_ord_61[4];
const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_dts_ord_61[] = { 1, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 6 : /soc/spi@3ff65000:
 * Direct Dependencies:
 *    - /soc/rtc@3ff48000
 */
extern const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2"))) __devicehdl_dts_ord_59[4];
const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_dts_ord_59[] = { 1, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 7 : /soc/spi@3ff64000:
 * Direct Dependencies:
 *    - /soc/rtc@3ff48000
 * Supported:
 *    - /soc/spi@3ff64000/st7789v@0
 */
extern const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2"))) __devicehdl_dts_ord_76[5];
const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_dts_ord_76[] = { 1, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 8, DEVICE_HANDLE_ENDS };

/* 8 : /soc/spi@3ff64000/st7789v@0:
 * Direct Dependencies:
 *    - /soc/gpio/gpio@3ff44000
 *    - /soc/spi@3ff64000
 */
extern const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2"))) __devicehdl_dts_ord_77[5];
const Z_DECL_ALIGN(device_handle_t) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_dts_ord_77[] = { 3, 7, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };
