#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#ifdef CONFIG_BOARD_NATIVE_SIM
extern int sdl_handle_pending_events(void);
#endif

typedef struct
{
	const struct gpio_dt_spec key;
} keymap_t;

//doom_def keymap
// #define KEYD_ALT       1
// #define KEYD_FIRE      2
// #define KEYD_USE       3
// #define KEYD_CHGW      4
// #define KEYD_CHGWDOWN  5
// #define KEYD_UP        6
// #define KEYD_DOWN      7
// #define KEYD_LEFT      8
// #define KEYD_RIGHT     9
// #define KEYD_SL       10
// #define KEYD_SR       11
// #define KEYD_MENU     12
// #define KEYD_MAP1     13
// #define KEYD_SPEED    14


//shortened keymap
// #define KEY_USE (1 << 0)
// #define KEY_ALT  (1 << 1)    
// #define KEY_FIRE (1 << 2) 
// #define KEY_LEFT (1 << 3)  
// #define KEY_DOWN (1 << 4)  
// #define KEY_RIGHT (1 << 5) 
// #define KEY_UP (1 << 6)
// #define KEY_CHGW (1<< 7)  

#ifdef CONFIG_BOARD_NATIVE_SIM
static keymap_t keys[] =
{
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_enter), gpios)  },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_strafe), gpios) },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_fire), gpios)   },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_left), gpios)   },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_down), gpios)   },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_right), gpios)  },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_up), gpios)     },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_wtog), gpios)   },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_run), gpios)    },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_menu), gpios)   },

/* 	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_right), gpios)  },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_left), gpios)   },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_down), gpios)   },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_up), gpios)     },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_fire), gpios)   },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_enter), gpios)  },
	// { .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_strafe), gpios) },
	// { .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_run), gpios)    },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_menu), gpios)   },
	{ .key = GPIO_DT_SPEC_GET(DT_NODELABEL(key_wtog), gpios)   }, */
};
#endif

int user_contorls_get_state(uint16_t *out_keys)
{
	int ret = 0;

#ifdef CONFIG_BOARD_NATIVE_SIM
	for (size_t i = 0; i < ARRAY_SIZE(keys) && i < NUM_BITS(*out_keys); i++)
	{
		__unused int handled = sdl_handle_pending_events();
		int pin_state = gpio_pin_get_dt(&keys[i].key);
		if (pin_state) 
		{
			*out_keys = *out_keys | (1 << i);
		}
		// SDL_GetRelativeMouseState(&x, &y);
		// printk("%d %d\r\n", x, y);
		// printk("pin_state:%d pin:%d handled:%d\r\n", pin_state, keys[i].key.pin, handled);
	}
#else
	//TODO: implement rpi_pico and esp32 controls logic
#endif

	return ret;
}

int user_controls_init(void)
{
	int ret = 0;

#ifdef CONFIG_BOARD_NATIVE_SIM
	for (size_t i = 0; i < ARRAY_SIZE(keys); i++)
	{
		ret = gpio_pin_configure_dt(&keys[i].key, GPIO_INPUT);  
		if (ret) { printk("failed to init controls, %d\r\n", ret); k_panic(); }
	}
#endif

	return ret;
}