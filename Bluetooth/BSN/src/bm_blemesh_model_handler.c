/*
This file is part of Bluetooth-Benchamrk.

Bluetooth-Benchamrk is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Bluetooth-Benchamrk is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bluetooth-Benchamrk.  If not, see <http://www.gnu.org/licenses/>.
*/

/* AUTHOR 	 :    Raffael Anklin       */

#include "bm_config.h"
#include "bm_log.h"
#include "bm_cli.h"
#include "bm_simple_buttons_and_leds.h"
#include "bm_timesync.h"
#include "bm_blemesh.h"
#include "bm_blemesh_model_handler.h"


#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>

// Message for Save the Data
bm_message_info msg;

// TID OVerflow Handler -> Allows an Overflow of of the TID when the next TID is dropping by 250... (uint8 overflows at 255 so there is a margin of 5)
// To Handle such case for all possibel Src. Adresses create the following struct and a array of the struct to save the last seen TID of each Addr.
typedef struct
{
  uint16_t src_addr;
  uint8_t last_TID_seen;
  uint8_t TID_OverflowCnt;
} __attribute__((packed)) bm_tid_overflow_handler_t;

#define max_number_of_nodes 100

bm_tid_overflow_handler_t bm_tid_overflow_handler[max_number_of_nodes]; // Excpect not more than max_number_of_nodes Different Adresses

// Insert the tid and src address to get the merged tid with the tid overlfow cnt -> resulting in a uint16_t
uint16_t bm_get_overflow_tid_from_overflow_handler(uint8_t tid, uint16_t src_addr){
	// Get the TID in array
	for (int i = 0; i < max_number_of_nodes; i++)
	{
		if(bm_tid_overflow_handler[i].src_addr == src_addr){
			// Check if Overflow happend
			if((bm_tid_overflow_handler[i].last_TID_seen - tid) > 250){
				bm_tid_overflow_handler[i].TID_OverflowCnt++;
			}
			// Add the last seen TID
			bm_tid_overflow_handler[i].last_TID_seen = tid;	
			return (uint16_t) (bm_tid_overflow_handler[i].TID_OverflowCnt << 8 ) | (tid & 0xff);		
			//return bm_tid_overflow_handler[i].TID_OverflowCnt;
		} else if(bm_tid_overflow_handler[i].src_addr == 0){
			// Add the Src Adress
			bm_tid_overflow_handler[i].src_addr = src_addr;
			bm_tid_overflow_handler[i].last_TID_seen = tid;
			bm_tid_overflow_handler[i].TID_OverflowCnt = 0;
			return (uint16_t) (bm_tid_overflow_handler[i].TID_OverflowCnt << 8 ) | (tid & 0xff);
		}
	}
	return 0; // Default return 0	
}


/** Configuration server definition */
static struct bt_mesh_cfg_srv cfg_srv = {
	.relay = IS_ENABLED(CONFIG_BT_MESH_RELAY),
	.beacon = BT_MESH_BEACON_ENABLED,
	.frnd = IS_ENABLED(CONFIG_BT_MESH_FRIEND),
	.gatt_proxy = IS_ENABLED(CONFIG_BT_MESH_GATT_PROXY),
	.default_ttl = BLE_MESH_TTL,

	/* 3 retransmissions with 20ms interval 
	.net_transmit = BT_MESH_TRANSMIT(2, 20),
	.relay_retransmit = BT_MESH_TRANSMIT(2, 20),
	*/

	/* 0 retransmissions with 20ms interval */
	.net_transmit = BT_MESH_TRANSMIT(0, 20),
	.relay_retransmit = BT_MESH_TRANSMIT(0, 20),
	

};

/** Configuration client definition */
static struct bt_mesh_cfg_cli cfg_cli = {
};

#ifdef BENCHMARK_CLIENT
/** Generic OnOff client definition */
static void status_handler_onoff_cli(struct bt_mesh_onoff_cli *cli,
						   struct bt_mesh_msg_ctx *ctx,
						   const struct bt_mesh_onoff_status *status)
{
	// ToDO Appropriate Hanlding of Ack Messages
}

struct bt_mesh_onoff_cli on_off_cli = BT_MESH_ONOFF_CLI_INIT(&status_handler_onoff_cli);

static void button0_cb(){
	int err;
	struct bt_mesh_onoff_set set = {
		.on_off = bm_button0_toggle_state_get(),
	};
	msg.net_time = synctimer_getSyncTime();
	msg.ack_net_time = 0;
	err = bt_mesh_onoff_cli_set_unack(&on_off_cli, NULL, &set);
	bm_led3_set(!bm_led3_get()); // Toggle the Blue LED
	//err = bt_mesh_onoff_cli_set(&on_off_cli, NULL, &set, NULL);
	msg.message_id = bm_get_overflow_tid_from_overflow_handler(on_off_cli.tid,addr);
	msg.number_of_hops = on_off_cli.model->pub->ttl;
	msg.rssi = 0;
	msg.src_addr = addr;
	msg.dst_addr = on_off_cli.model->pub->addr;
	msg.group_addr = on_off_cli.model->pub->addr;
	msg.data_size = on_off_cli.pub.msg->len;
	//msg.data_size = 4; // Fix the Size because its only available after the first pub mesg...
	bm_log_append_ram(msg);	
	/*
	printk("Sent TID %u\n",on_off_cli.tid);
	printk("Sent Time %u%u\n",(uint32_t)(synctimer_getSyncTime() >> 32 ),(uint32_t)synctimer_getSyncTime());
	printk("Sent TTL %u\n",on_off_cli.model->pub->ttl);
	printk("Sent RSSI %d\n",0);
	printk("Sent Src Adr %x\n",addr);
	printk("Sent Dst Adr %x\n",on_off_cli.model->pub->addr);
	printk("Sent Group Adr %x\n",on_off_cli.model->pub->addr);
	printk("Sent Size %u\n",on_off_cli.pub.msg->len);
	*/
}

void bm_send_message(){
	bm_button0_toggle_state_set(!bm_button0_toggle_state_get()); // Simulate toggle of button
	button0_cb();
}

#endif
#ifdef BENCHMARK_SERVER
/** ON/OFF Server definition */
static void led_set(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_onoff_set *set,
		    struct bt_mesh_onoff_status *rsp)
{
	// Set DK LED index
	bm_led3_set(set->on_off);
	// Update Response Status
	//rsp->present_on_off = set->on_off;
	// Log the Event
	msg.message_id = bm_get_overflow_tid_from_overflow_handler(srv->prev_transaction.tid+1,ctx->addr);
	msg.net_time = synctimer_getSyncTime();
	msg.ack_net_time = 0;
	msg.number_of_hops = (uint8_t)BLE_MESH_TTL-ctx->recv_ttl;
	msg.rssi = (uint8_t)ctx->recv_rssi;
	msg.src_addr = ctx->addr;
	msg.dst_addr = addr;
	msg.group_addr = ctx->recv_dst;
	msg.data_size = 4; // Fix the Size to the same as sent size...
	bm_log_append_ram(msg);
	/*
	printk("Recv TID %u\n",srv->prev_transaction.tid+1);
	printk("Recv Time %u%u\n",(uint32_t)(synctimer_getSyncTime() >> 32 ),(uint32_t)synctimer_getSyncTime());
	printk("Recv Hops Took %u\n",BLE_MESH_TTL-ctx->recv_ttl);
	printk("Recv RSSI %d\n",ctx->recv_rssi);
	printk("Recv Src Adr %x\n",ctx->addr);
	printk("Recv Dst Adr %x\n",ctx->recv_dst);
	printk("Recv Group Adr %x\n",ctx->recv_dst);
	printk("Recv Size %u\n",srv->model->pub->msg->len);
	*/
}

static void led_get(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp)
{
	// Send respond Status
	rsp->present_on_off = bm_led3_get();
}

static const struct bt_mesh_onoff_srv_handlers onoff_handlers = {
	.set = led_set,
	.get = led_get,
};

static struct bt_mesh_onoff_srv on_off_srv = BT_MESH_ONOFF_SRV_INIT(&onoff_handlers);

#endif

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
#ifdef BENCHMARK_MASTER
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
#endif
#ifdef BENCHMARK_CLIENT
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
			BT_MESH_MODEL_ONOFF_CLI(&on_off_cli)),
#endif
#ifdef BENCHMARK_SERVER
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
			BT_MESH_MODEL_ONOFF_SRV(&on_off_srv)),
#endif
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
#ifdef BENCHMARK_CLIENT
	bm_init_buttons(button0_cb); // Init Buttons and LEDs
#endif
	return &comp;
}
