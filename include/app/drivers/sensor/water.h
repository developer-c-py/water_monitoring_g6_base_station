/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_ZEPHYR_DRIVERS_SENSOR_H_
#define ZEPHYR_INCLUDE_ZEPHYR_DRIVERS_SENSOR_H_

#include <zephyr/drivers/sensor.h>

/** @brief water_sensor custom channels. */
enum water_channel
{
  /** Fingerprint verification. */
  WATER_CHAN_PH = SENSOR_CHAN_PRIV_START,
  WATER_CHAN_TURB,
  WATER_CHAN_TEMP,
  WATER_CHAN_ALL

};

#endif /* ZEPHYR_INCLUDE */