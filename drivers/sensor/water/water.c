/** @file water.c
 *  @brief Function prototypes for the sensornode driver
 *
 *  This contains the prototypes for the sensor node
 *  driver and eventually any macros, constants,
 *  or global variables you will need.
 *
 *  @author Amrith Harijayanthan Namboodiri
 *  @author Abdel Mujeeb
 *  @author Adithya
 *  @bug No known bugs.
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
/*
 * Buffer size and Queue size
 */
#define PKT_BUF_SIZE 32U
#define RX_QUEUE_SIZE 2U

LOG_MODULE_REGISTER(water, CONFIG_SENSOR_LOG_LEVEL);

/*
 * typedef of sensor_value
 */
typedef struct sensor_value sensor_value_t;

/*
 * water_data structure
 */
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
	/** Sensor value pH. */
	sensor_value_t pH;
	/** Sensor value turb. */
	sensor_value_t turb;
	/** Sensor value temperature. */
	sensor_value_t temperature;
	/** RX queue. */
	struct k_msgq rx_queue;
};

/*
 * water_config structure
 */
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
 * @defgroup drivers_water Blink drivers
 * @ingroup drivers
 * @{
 *
 */

/**
 * @brief helper funtion to update values in the struct
 *
 *  format : [ 8bit  | 16bit 16bit | 16bit 16bit | 16bit 16bit | 8bit CRC]
 *
 *  @param dev device struct
 *  @param chan channel to update
 *  @return int 0 if success
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

/** @brief water_sample fetch function to update the values from sensor to struct
 *
 *  This function will fetch values from sensor and update it in the struct
 *
 *  @param dev device struct
 *  @param chan channel
 *  @return int returns 0 if succeeful otherwise Error code
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

/** @brief water_sample get function to store the values from sensor to struct from user
 *
 *  This function will fetch values from the stored stuct memory and copy it to the user passed stuct
 *
 *  @param dev device
 *  @param chan channel
 *  @param val struct sensor_value type struct to retrieve sensor value
 *  @return int returns 0 if succeeful otherwise Error code
 */
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

/** @brief water_init function checks for uart device and copies all configuration
 *
 *  This function takes a struct with all configuration and initilises the driver and sensor with it.
 *
 *  @param dev device
 *  @return int returns 0 if succeeful otherwise Error code
 */
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
/// @} // end of drivers_water
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
