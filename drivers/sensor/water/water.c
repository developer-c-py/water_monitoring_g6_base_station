/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT zephyr_water

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/logging/log.h>

#include <zephyr/types.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include "app/drivers/sensor/water.h"
/* packet buffer size, RX queue size */
#define PKT_BUF_SIZE 32U
#define RX_QUEUE_SIZE 2U

LOG_MODULE_REGISTER(water, CONFIG_SENSOR_LOG_LEVEL);

typedef struct sensor_value sensor_value_t;

struct water_data
{
	/** RX queue buffer. */
	uint8_t rx_queue_buf[PKT_BUF_SIZE * RX_QUEUE_SIZE];
	/** RX/TX buffer. */
	uint8_t buf[PKT_BUF_SIZE];
	/** RX/TX buffer pointer. */
	uint8_t *buf_ptr;
	/** RX/TX buffer counter. */
	uint8_t buf_ctr;
	/** Expected reception data length. */
	uint8_t rx_data_len;
	/** Sensor value buffer. */
	sensor_value_t pH;
	sensor_value_t turb;
	sensor_value_t temperature;
	/** RX queue. */
	struct k_msgq rx_queue;
};

struct water_config
{
	uint32_t addr;
	const struct device *uart;
};

typedef struct water_data water_data_t;
typedef struct water_config water_config_t;

/** @brief sensor UART configuration. */
static const struct uart_config uart_config = {
	.baudrate = 115200U,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	.parity = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_2,
};

/**
 * @defgroup drivers_water water drivers
 * @ingroup drivers
 * @{
 *
 * @brief A custom driver class to blink LEDs
 *
 * This driver class is provided as an example of how to create custom driver
 * classes. It provides an interface to blink an LED at a configurable rate.
 * Implementations could include simple GPIO-controlled LEDs, addressable LEDs,
 * etc.
 */
int update_value(const struct device *dev,
				 enum sensor_channel chan)
{
	water_config_t *config = dev->config;
	water_data_t *data = dev->data;
	switch (chan)
	{
	case WATER_CHAN_PH:
		data->pH.val1 = 0; // write code to get data from rxbuffer and update
		data->pH.val2 = 0;
		break;
	default:
		return -ENOTSUP;
	}
	return 0;
}

/**
 * @defgroup drivers_water_ops water driver operations
 * @{
 *
 * @brief Operations of the blink driver class.
 *
 * Each driver class tipically provides a set of operations that need to be
 * implemented by each driver. These are used to implement the public API. If
 * support for system calls is needed, the operations structure must be tagged
 * with `__subsystem` and follow the `${class}_driver_api` naming scheme.
 */
static int water_sample_fetch(const struct device *dev,
							  enum sensor_channel chan)
{
	if (chan > WATER_CHAN_ALL || chan < WATER_CHAN_PH)
	{
		return -ENOTSUP;
	}

	return update_value(dev, chan);
}

// copy stored value to the pointer -- return value to app
static int water_channel_get(const struct device *dev,
							 enum sensor_channel chan,
							 struct sensor_value *val)
{
	water_data_t *data = dev->data;
	switch (chan)
	{
	case WATER_CHAN_PH:
		val->val1 = data->pH.val1 = 0; // write code to get data from rxbuffer and update
		val->val2 = data->pH.val2 = 0;
		break;
	default:
		return -ENOTSUP;
	}
	return 0;
}

static const struct sensor_driver_api water_api = {
	.sample_fetch = &water_sample_fetch,
	.channel_get = &water_channel_get,
};

static int water_init(const struct device *dev)
{
	const struct water_config *config = dev->config;
	water_data_t *data = dev->data;

	if (!device_is_ready(config->uart))
	{
		LOG_ERR("UART device not ready\n");
		return ENXIO;
	}

	return 0;
}

#define WATER_INIT(i)                                     \
	static struct water_data water_data_##i;              \
                                                          \
	static const struct water_config water_config_##i = { \
		.input = GPIO_DT_SPEC_INST_GET(i, input_gpios),   \
	};                                                    \
                                                          \
	DEVICE_DT_INST_DEFINE(i, water_init, NULL,            \
						  &water_data_##i,                \
						  &water_config_##i, POST_KERNEL, \
						  CONFIG_SENSOR_INIT_PRIORITY, &water_api);

DT_INST_FOREACH_STATUS_OKAY(WATER_INIT)
