/*
 * Copyright (c) 2020 Abram Early
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef ZEPHYR_DRIVERS_CAN_SAM_H_
#define ZEPHYR_DRIVERS_CAN_SAM_H_

#include "can_mcan.h"
#include <soc.h>

#define DEV_DATA(dev) ((struct can_sam_data *)(dev)->data)
#define DEV_CFG(dev) ((const struct can_sam_config *)(dev)->config)

struct can_sam_config {
	struct can_mcan_msg_sram *msg_sram;
	struct can_mcan_config mcan_cfg;
	void (*irq_config)();
	uint32_t periph_id;
	uint32_t num_pins;
	struct soc_gpio_pin pins[];
};

struct can_sam_data {
	struct can_mcan_data mcan_data;
};

#endif /*ZEPHYR_DRIVERS_CAN_SAM_H_*/
