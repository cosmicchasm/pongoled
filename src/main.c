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

#include "pong_core.h"


/* Get the i2c device handle */
const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(oled_dev));
const struct i2c_dt_spec *shared_dev = &dev_i2c;

LOG_MODULE_REGISTER(main);

/*
 * @brief Callback which is called when a write is received from the master.
 * @param config Pointer to the target configuration.
 * @param val The byte received from the master.
 */

int main(void)
{
  // send config to device to display ram
  oled_core_cfg_t my_cfg = {
  	.screen_mod = 0,
  	.mem_mode = HORIZ_MODE,
  };
  
  // initializes the config and powers on the device
  // ram mode
  oled_init(&my_cfg, SET_ADDR_MODE | SET_SCREEN_RAM_MOD);
  
	// run test now that everything's initialized
	// no return value since this is purely a visual test
	k_sleep(K_MSEC(1000));

	// branch to pong now
	pong_main();

  while (1) {
    // sleep for some time
    k_sleep(K_MSEC(1000));
  }
  
  return 0;
}
