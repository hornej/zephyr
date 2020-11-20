/*
 * Copyright (c) 2018 Texas Instruments, Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief General WiFi Definitions
 */

#ifndef ZEPHYR_INCLUDE_NET_WIFI_H_
#define ZEPHYR_INCLUDE_NET_WIFI_H_

enum wifi_security_type {
	WIFI_SECURITY_TYPE_NONE = 0,
	WIFI_SECURITY_TYPE_PSK,
#if CONFIG_NET_L2_WIFI_ENTERPRISE
	WIFI_SECURITY_TYPE_802_1X,
#endif
};

#if CONFIG_NET_L2_WIFI_ENTERPRISE
// TODO: I really don't like how this is being done, but it will work fine for now
#define WIFI_EAP_NONE 0
#define WIFI_EAP_TLS 1
#define WIFI_EAP_TTLS 2
#define WIFI_EAP_PEAP 3
#define WIFI_EAP_MSCHAPV2 4

#define WIFI_EAP_MODE(phase1, phase2) (phase1 | (phase2 << 4))
#define WIFI_EAP_PHASE1(mode) ((mode >> 0) & 0x0F)
#define WIFI_EAP_PHASE2(mode) ((mode >> 4) & 0x0F)

enum wifi_eap_mode {
	WIFI_EAP_MODE_TLS = WIFI_EAP_MODE(WIFI_EAP_TLS, WIFI_EAP_NONE),
	WIFI_EAP_MODE_TTLS_TLS = WIFI_EAP_MODE(WIFI_EAP_TTLS, WIFI_EAP_TLS),
	WIFI_EAP_MODE_PEAP_TLS = WIFI_EAP_MODE(WIFI_EAP_PEAP, WIFI_EAP_TLS),
	WIFI_EAP_MODE_TTLS_MSCHAPV2 = WIFI_EAP_MODE(WIFI_EAP_TTLS, WIFI_EAP_MSCHAPV2),
	WIFI_EAP_MODE_PEAP_MSCHAPV2 = WIFI_EAP_MODE(WIFI_EAP_PEAP, WIFI_EAP_MSCHAPV2),
};
#endif

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PSK_MAX_LEN 64

#define WIFI_CHANNEL_MAX 14
#define WIFI_CHANNEL_ANY 255

#endif /* ZEPHYR_INCLUDE_NET_WIFI_H_ */
