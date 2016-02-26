/* l2cap.c - Bluetooth L2CAP Tester */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bluetooth/bluetooth.h>

#include <errno.h>
#include <bluetooth/l2cap.h>
#include <misc/byteorder.h>
#include "bttester.h"

#define CONTROLLER_INDEX 0
#define DATA_MTU 230
#define CHANNELS 1

static struct nano_fifo data_fifo;
static NET_BUF_POOL(data_pool, 1, DATA_MTU, &data_fifo, NULL,
		    BT_BUF_USER_DATA_MIN);

static struct channel {
	uint8_t chan_id; /* Internal number that identifies L2CAP channel. */
	struct bt_l2cap_le_chan le;
} channels[CHANNELS];

static struct net_buf *alloc_buf_cb(struct bt_l2cap_chan *chan)
{
	return net_buf_get(&data_fifo, 0);
}

static uint8_t recv_cb_buf[DATA_MTU + sizeof(struct l2cap_data_received_ev)];

static void recv_cb(struct bt_l2cap_chan *l2cap_chan, struct net_buf *buf)
{
	struct l2cap_data_received_ev *ev = (void *) recv_cb_buf;
	struct channel *chan = CONTAINER_OF(l2cap_chan, struct channel, le);

	ev->chan_id = chan->chan_id;
	ev->data_length = sys_cpu_to_le16(buf->len);
	memcpy(ev->data, buf->data, buf->len);

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_EV_DATA_RECEIVED,
		    CONTROLLER_INDEX, recv_cb_buf, sizeof(*ev) + buf->len);
}

static void connected_cb(struct bt_l2cap_chan *l2cap_chan)
{
	struct l2cap_connected_ev ev;
	struct channel *chan = CONTAINER_OF(l2cap_chan, struct channel, le);
	struct bt_conn_info info;

	ev.chan_id = chan->chan_id;
	/* TODO: ev.psm */
	if (!bt_conn_get_info(l2cap_chan->conn, &info)) {
		switch (info.type) {
		case BT_CONN_TYPE_LE:
			ev.address_type = info.le.dst->type;
			memcpy(ev.address, info.le.dst->a.val,
			       sizeof(ev.address));
			break;
		case BT_CONN_TYPE_BR:
			memcpy(ev.address, info.br.dst->val,
			       sizeof(ev.address));
			break;
		}
	}

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_EV_CONNECTED, CONTROLLER_INDEX,
		    (uint8_t *) &ev, sizeof(ev));
}

static void disconnected_cb(struct bt_l2cap_chan *l2cap_chan)
{
	struct l2cap_disconnected_ev ev;
	struct channel *chan = CONTAINER_OF(l2cap_chan, struct channel, le);
	struct bt_conn_info info;

	memset(&ev, 0, sizeof(struct l2cap_disconnected_ev));

	/* TODO: ev.result */
	ev.chan_id = chan->chan_id;
	/* TODO: ev.psm */
	if (!bt_conn_get_info(l2cap_chan->conn, &info)) {
		switch (info.type) {
		case BT_CONN_TYPE_LE:
			ev.address_type = info.le.dst->type;
			memcpy(ev.address, info.le.dst->a.val,
			       sizeof(ev.address));
			break;
		case BT_CONN_TYPE_BR:
			memcpy(ev.address, info.br.dst->val,
			       sizeof(ev.address));
			break;
		}
	}

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_EV_DISCONNECTED,
		    CONTROLLER_INDEX, (uint8_t *) &ev, sizeof(ev));
}

static struct bt_l2cap_chan_ops l2cap_ops = {
	.alloc_buf	= alloc_buf_cb,
	.recv		= recv_cb,
	.connected	= connected_cb,
	.disconnected	= disconnected_cb,
};

static struct channel *get_free_channel()
{
	uint8_t i;
	struct channel *chan;

	for (i = 0; i < CHANNELS; i++) {
		if (channels[i].le.chan.state != BT_L2CAP_DISCONNECTED) {
			continue;
		}

		chan = &channels[i];
		chan->chan_id = i;

		return chan;
	}

	return NULL;
}

static void connect(uint8_t *data, uint16_t len)
{
	const struct l2cap_connect_cmd *cmd = (void *) data;
	struct l2cap_connect_rp rp;
	struct bt_conn *conn;
	struct channel *chan;
	int err;

	conn = bt_conn_lookup_addr_le((bt_addr_le_t *) data);
	if (!conn) {
		goto fail;
	}

	chan = get_free_channel();
	if (!chan) {
		goto fail;
	}

	chan->le.chan.ops = &l2cap_ops;
	chan->le.rx.mtu = DATA_MTU;

	err = bt_l2cap_chan_connect(conn, &chan->le.chan, cmd->psm);
	if (err < 0) {
		goto fail;
	}

	rp.chan_id = chan->chan_id;

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_CONNECT, CONTROLLER_INDEX,
		    (uint8_t *) &rp, sizeof(rp));

	return;

fail:
	tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_CONNECT, CONTROLLER_INDEX,
		   BTP_STATUS_FAILED);
}

static void supported_commands(uint8_t *data, uint16_t len)
{
	uint8_t cmds[1];
	struct l2cap_read_supported_commands_rp *rp = (void *) cmds;

	memset(cmds, 0, sizeof(cmds));

	tester_set_bit(cmds, L2CAP_READ_SUPPORTED_COMMANDS);
	tester_set_bit(cmds, L2CAP_CONNECT);

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_READ_SUPPORTED_COMMANDS,
		    CONTROLLER_INDEX, (uint8_t *) rp, sizeof(cmds));
}

void tester_handle_l2cap(uint8_t opcode, uint8_t index, uint8_t *data,
			 uint16_t len)
{
	switch (opcode) {
	case L2CAP_READ_SUPPORTED_COMMANDS:
		supported_commands(data, len);
		return;
	case L2CAP_CONNECT:
		connect(data, len);
		return;
	default:
		tester_rsp(BTP_SERVICE_ID_L2CAP, opcode, index,
			   BTP_STATUS_UNKNOWN_CMD);
		return;
	}
}

uint8_t tester_init_l2cap(void)
{
	net_buf_pool_init(data_pool);

	return BTP_STATUS_SUCCESS;
}
