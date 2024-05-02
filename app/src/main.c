// /* Application Code */

#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>


int main() {
  const struct device *const dev = DEVICE_DT_GETONE(water);

  struct my_struct {
    int val;
  } temp;

  if (!device_is_ready(dev)) {
    printk("not ready");
    return 0;
  }

  while (1) {
    print_values(&temp);
    printk("value = %d", temp.val);
    k_sleep(K_MSEC(1000));
  }
}
