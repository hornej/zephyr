/*
 * Copyright (c) 2020 Abram Early
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef ZEPHYR_DRIVERS_CAN_SAM0_H_
#define ZEPHYR_DRIVERS_CAN_SAM0_H_

#include "can_mcan.h"
#include <soc.h>

#define DEV_DATA(dev) ((struct can_sam0_data *)(dev)->data)
#define DEV_CFG(dev) ((const struct can_sam0_config *)(dev)->config)

struct can_sam0_config {
	struct can_mcan_msg_sram *msg_sram;
	struct can_mcan_config mcan_cfg;
	void (*irq_config)();
	volatile uint32_t *mclk;
	uint32_t mclk_mask;
	uint16_t gclk_core_id;
};

struct can_sam0_data {
	struct can_mcan_data mcan_data;
};

#endif /*ZEPHYR_DRIVERS_CAN_SAM0_H_*/
