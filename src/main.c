/*
 * Author: Aidan Stanford
 */

#include <zephyr/kernel.h>
// #include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

// for generating images
#include <zephyr/random/random.h>

#include "graphics_common.h"
#include "pong_core.h"

/* Get the i2c device handle */
const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(oled_dev));
const struct i2c_dt_spec *shared_dev = &dev_i2c;

LOG_MODULE_REGISTER(main);

/* Thread objects */
static struct k_thread pong_obj;
K_THREAD_STACK_DEFINE(pong_stack, PONG_MAIN_STACK_SIZE);

int main(void) {
  // send config to device to display ram
  oled_core_cfg_t my_cfg = {
  	.screen_mod = 0,
  	.mem_mode = HORIZ_MODE,
  };
  
  // initializes the config and powers on the device 
	// in ram mode
  oled_init(&my_cfg, SET_ADDR_MODE | SET_SCREEN_RAM_MOD);
  
	k_sleep(K_MSEC(1000));

	// once we're ready, create pong_main
	k_tid_t pong_tid = k_thread_create(&pong_obj, pong_stack, K_THREAD_STACK_SIZEOF(pong_stack),
                                       pong_main, NULL, NULL, NULL,
                                       PONG_MAIN_PRIORITY, 0, K_NO_WAIT);

  while (1) {
    // sleep for some time
    k_sleep(K_MSEC(10000));
  }
  
  return 0;
}
