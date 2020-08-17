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
#include "bm_rand.h"
#include "bm_timesync.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <drivers/hwinfo.h>
#include "bm_blemesh_model_handler.h"
#include "bm_blemesh.h"
#include <bluetooth/mesh/main.h>
#include "bm_simple_buttons_and_leds.h"




/** Defines for Self Provisioning **/
static const u8_t net_key[16] = {
	0x66,
	0xef,
	0x1e,
	0x3a,
	0x18,
	0x0d,
	0x7d,
	0xec,
	0x15,
	0x45,
	0xe6,
	0xae,
	0x77,
	0x47,
	0xb3,
	0xfe,
};
static u8_t dev_key[16] = {
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xab,
	0xcd,
	0xef,
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xab,
	0xcd,
	0xef,
};
static const u8_t app_key[16] = {
	0xa3,
	0x01,
	0x77,
	0x4b,
	0xe8,
	0x02,
	0x2c,
	0xd8,
	0x12,
	0x3f,
	0xf3,
	0x27,
	0x42,
	0xa7,
	0x36,
	0x94,
};

static const u16_t net_idx=0;
static const u16_t app_idx=0;
static const u32_t iv_index=0;

#define GROUP_ADDR 0xc000 //Range from 0xC000-0xFEFF 


static u8_t flags =0;
u16_t addr = 0x0b0d; // Unicast Address Range from 0-0x7FFF, 0-32767 ->Assigned Random
static bool initialized=false; // Flag if the Stack is initialized


// Init the rest while Bluetooth is enabled
static void bt_ready(int err){

	/* --------------- Init Board ------------*/
	printk("Init Buttons and LEDs...\n");
	
	printk("Buttons and LEDs initialized\n");
	/* ===================================*/
	
	/* --------------- Init Mesh ------------*/
	printk("Init Mesh...\n");
	u8_t dev_uuid[16];
	hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));
	// Do catch for nrf53 -> no HWINFO set (0xFFFF)
	if (dev_uuid[0] == 0xFF && dev_uuid[1] == 0xFF && dev_uuid[5] == 0xFF){
		u8_t *p0 = (u8_t*)&NRF_FICR->DEVICEADDR[0]; // Use MAC Address for UUID
		dev_uuid[0] = p0[0];
		dev_uuid[1] = p0[1];
		dev_uuid[2] = p0[2];
		dev_uuid[3] = p0[3];
		u8_t *p1 = (u8_t*)&NRF_FICR->DEVICEADDR[1]; // Use MAC Address for UUID
		dev_uuid[4] = p1[0];
		dev_uuid[5] = p1[1];
	}
	struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	};
	err = bt_mesh_init(&prov, bm_blemesh_model_handler_init());
	if (err)
	{
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}
	printk("Mesh initialized\n");
	/* ===================================*/

	/* ------------- Provisioning ------------*/
	printk("Provisioning...\n");
	addr = (dev_uuid[1] << 8) | dev_uuid[0];
	addr &= ~(1U << 15); // Limit the Address Range from 0-0x7FFF, 0-32767	
	if (IS_ENABLED(CONFIG_SETTINGS))
	{
		settings_load();
	}
	err = bt_mesh_provision(net_key, net_idx, flags, iv_index, addr,
							dev_key);
	if (err == -EALREADY)
	{
		printk("Already Provisioned (Restored Settings)\n");
	}
	else if (err)
	{
		printk("Provisioning failed (err %d)\n", err);
		return;
	}
	else
	{
		printk("Provisioning completed\n");
	}
	/* ===================================*/

	initialized = true;
}

void bm_blemesh_enable(void)
{
	int err;

	/* ---------- Init Bluetooth ---------- */
	printk("Enabling Bluetooth...\n");
	err = bt_enable(bt_ready);
	while (initialized == false){
		k_sleep(K_MSEC(10));
	}
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	} 
	printk("Bluetooth initialized\n");
	/* =====================================*/

	/* ------------- Configure Models ------------*/
	printk("Configuring...\n");
	u8_t stat;
	/* Add Application Key */
	bt_mesh_cfg_app_key_add(net_idx, addr, net_idx, app_idx, app_key, &stat);
	printk("Err Code: %d\n", stat);
	/* Bind to Generic ON/OFF Model */
#ifdef BENCHMARK_SERVER
	bm_led3_set(true); // Signal that the Configuring was sucessfull
	bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
							 BT_MESH_MODEL_ID_GEN_ONOFF_SRV, &stat);
	printk("Err Code: %d\n", stat);
	/* Add model subscription */
	bt_mesh_cfg_mod_sub_add(net_idx, addr, addr, (uint16_t) GROUP_ADDR + bm_params.GroupAddress,
							BT_MESH_MODEL_ID_GEN_ONOFF_SRV, &stat);
	printk("Err Code: %d\n", stat);
#endif
#ifdef BENCHMARK_CLIENT
	bm_led2_set(true); // Signal that the Configuring was sucessfull
	bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
							 BT_MESH_MODEL_ID_GEN_ONOFF_CLI, &stat);
	bt_mesh_cfg_friend_set(net_idx,addr,false,&stat);
	printk("Err Code: %d\n", stat);
	/* Add model publishing */
	struct bt_mesh_cfg_mod_pub pub = {
		.addr = (uint16_t) GROUP_ADDR + bm_params.GroupAddress,
		.app_idx = app_idx,
		.ttl = BLE_MESH_TTL
	};
	bt_mesh_cfg_mod_pub_set(net_idx, addr, addr, BT_MESH_MODEL_ID_GEN_ONOFF_CLI,
						 &pub, &stat);
	printk("Err Code: %d\n",stat);
	//bm_sleep(bm_rand_32 % 5000); // Sleep Random Time for LPN Activation
	/*
	bt_mesh_cfg_friend_set(net_idx,addr,false,&stat);
	printk("Err Code: %d\n", stat);
	bt_mesh_lpn_set(true); // Enable Low Power Node
	*/
#endif
	
	printk("Configuring done\n");

	bm_led0_set(true); // Signal that the Configuring was sucessfull
	/* ===================================*/	
}
