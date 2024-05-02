/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT zephyr_water

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(water, CONFIG_SENSOR_LOG_LEVEL);

struct water_data {
	int state;
};

struct water_config {
	struct gpio_dt_spec input;
};

static int water_sample_fetch(const struct device *dev,
				      enum sensor_channel chan)
{
	const struct water_config *config = dev->config;
	struct water_data *data = dev->data;

	data->state = gpio_pin_get_dt(&config->input);

	return 0;
}

static int water_channel_get(const struct device *dev,
				     enum sensor_channel chan,
				     struct sensor_value *val)
{
	struct water_data *data = dev->data;

	if (chan != SENSOR_CHAN_PROX) {
		return -ENOTSUP;
	}

	val->val1 = data->state;

	return 0;
}

static const struct sensor_driver_api water_api = {
	.sample_fetch = &water_sample_fetch,
	.channel_get = &water_channel_get,
};

static int water_init(const struct device *dev)
{
	const struct water_config *config = dev->config;

	int ret;

	if (!device_is_ready(config->input.port)) {
		LOG_ERR("Input GPIO not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->input, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Could not configure input GPIO (%d)", ret);
		return ret;
	}

	return 0;
}

#define WATER_INIT(i)						       \
	static struct water_data water_data_##i;	       \
									       \
	static const struct water_config water_config_##i = {\
		.input = GPIO_DT_SPEC_INST_GET(i, input_gpios),		       \
	};								       \
									       \
	DEVICE_DT_INST_DEFINE(i, water_init, NULL,		       \
			      &water_data_##i,			       \
			      &water_config_##i, POST_KERNEL,	       \
			      CONFIG_SENSOR_INIT_PRIORITY, &water_api);

DT_INST_FOREACH_STATUS_OKAY(WATER_INIT)