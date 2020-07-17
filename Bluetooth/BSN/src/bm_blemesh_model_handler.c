/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include "bm_config.h"
#include "bm_log.h"
#include "bm_simple_buttons_and_leds.h"
#include "bm_timesync.h"
#include "bm_blemesh.h"
#include "bm_blemesh_model_handler.h"


#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>



/** Configuration server definition */
static struct bt_mesh_cfg_srv cfg_srv = {
	.relay = IS_ENABLED(CONFIG_BT_MESH_RELAY),
	.beacon = BT_MESH_BEACON_ENABLED,
	.frnd = IS_ENABLED(CONFIG_BT_MESH_FRIEND),
	.gatt_proxy = IS_ENABLED(CONFIG_BT_MESH_GATT_PROXY),
	.default_ttl = BLE_MESH_TTL,

	/* 3 transmissions with 20ms interval */
	.net_transmit = BT_MESH_TRANSMIT(2, 20),
	.relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

/** Configuration client definition */
static struct bt_mesh_cfg_cli cfg_cli = {
};


/** Generic OnOff client definition */
static void status_handler_onoff_cli(struct bt_mesh_onoff_cli *cli,
						   struct bt_mesh_msg_ctx *ctx,
						   const struct bt_mesh_onoff_status *status)
{
	printk("Ack Recv TID %u\n",cli->tid);
	printk("Ack Recv Time %u%u\n",(uint32_t)(synctimer_getSyncTime() >> 32 ),(uint32_t)synctimer_getSyncTime());
	printk("Ack Recv Hops Took %u\n",BLE_MESH_TTL-ctx->recv_ttl);
	printk("Ack Recv RSSI %d\n",ctx->recv_rssi);
	printk("Ack Recv Src Adr %x\n",ctx->addr);
	printk("Ack Recv Dst Adr %x\n",ctx->recv_dst);
	printk("Ack Recv Group Adr %x\n",ctx->recv_dst);
	printk("Ack Recv Size %u\n",cli->pub.msg->len);
}

struct bt_mesh_onoff_cli on_off_cli = BT_MESH_ONOFF_CLI_INIT(&status_handler_onoff_cli);

static void button0_cb(){
	int err;
	struct bt_mesh_onoff_set set = {
		.on_off = button0_toggle_state_get(),
	};	
	err = bt_mesh_onoff_cli_set_unack(&on_off_cli, NULL, &set);
	//err = bt_mesh_onoff_cli_set(&on_off_cli, NULL, &set, NULL);
	printk("Sent TID %u\n",on_off_cli.tid);
	printk("Sent Time %u%u\n",(uint32_t)(synctimer_getSyncTime() >> 32 ),(uint32_t)synctimer_getSyncTime());
	printk("Sent TTL %u\n",on_off_cli.model->pub->ttl);
	printk("Sent RSSI %d\n",0);
	printk("Sent Src Adr %x\n",addr);
	printk("Sent Dst Adr %x\n",on_off_cli.model->pub->addr);
	printk("Sent Group Adr %x\n",on_off_cli.model->pub->addr);
	printk("Sent Size %u\n",on_off_cli.pub.msg->len);
}

/** ON/OFF Server definition */
static void led_set(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_onoff_set *set,
		    struct bt_mesh_onoff_status *rsp)
{
	// Set DK LED index
	led0_set(set->on_off);
	// Update Response Status
	rsp->present_on_off = set->on_off;
	// Log the Event
	printk("Recv TID %u\n",srv->prev_transaction.tid+1);
	printk("Recv Time %u%u\n",(uint32_t)(synctimer_getSyncTime() >> 32 ),(uint32_t)synctimer_getSyncTime());
	printk("Recv Hops Took %u\n",BLE_MESH_TTL-ctx->recv_ttl);
	printk("Recv RSSI %d\n",ctx->recv_rssi);
	printk("Recv Src Adr %x\n",ctx->addr);
	printk("Recv Dst Adr %x\n",ctx->recv_dst);
	printk("Recv Group Adr %x\n",ctx->recv_dst);
	printk("Recv Size %u\n",srv->model->pub->msg->len);
}

static void led_get(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp)
{
	// Send respond Status
	rsp->present_on_off = led0_get();
}

static const struct bt_mesh_onoff_srv_handlers onoff_handlers = {
	.set = led_set,
	.get = led_get,
};

static struct bt_mesh_onoff_srv on_off_srv = BT_MESH_ONOFF_SRV_INIT(&onoff_handlers);

/** Health Server definition */
static void attention_on(struct bt_mesh_model *model)
{
	printk("attention_on()\n");
}

static void attention_off(struct bt_mesh_model *model)
{
	printk("attention_off()\n");
}
static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

/* Define all Elements */
static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(
		1, BT_MESH_MODEL_LIST(
			BT_MESH_MODEL_CFG_SRV(&cfg_srv),
			BT_MESH_MODEL_CFG_CLI(&cfg_cli),
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
			BT_MESH_MODEL_ONOFF_CLI(&on_off_cli),
			BT_MESH_MODEL_ONOFF_SRV(&on_off_srv)),
		BT_MESH_MODEL_NONE)
};

/* Create Composition */
static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};


const struct bt_mesh_comp *bm_blemesh_model_handler_init(void)
{
	init_leds_buttons(button0_cb); // Init Buttons and LEDs
	return &comp;
}
