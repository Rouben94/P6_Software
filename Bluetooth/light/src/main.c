/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *  @brief Nordic Mesh light sample
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <drivers/hwinfo.h>
#include "model_handler.h"
#include "const.h"

bool initialized=false;

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
	err = bt_mesh_init(&prov, model_handler_init());
	if (err)
	{
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}
	printk("Mesh initialized\n");
	/* ===================================*/

	/* ------------- Provisioning ------------*/
	printk("Provisioning...\n");
	//sys_rand_get(dev_key,sizeof(dev_key)); // Generate Random Dev Key
	//addr = (dev_uuid[1] << 8) | dev_uuid[0];
	addr = (dev_uuid[1] << 8) | dev_uuid[0];
	addr &= ~(1U << 15); // Limit the Address Range from 0-0x7FFF, 0-32767
	/* This will be a no-op if settings_load() loaded provisioning info */
	//bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);	
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

void main(void)
{
	int err;

	printk("Initializing...\n");

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
	bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
							 BT_MESH_MODEL_ID_GEN_ONOFF_SRV, &stat);
	bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
							 BT_MESH_MODEL_ID_GEN_ONOFF_CLI, &stat);
	printk("Err Code: %d\n", stat);
	/* Add model subscription */
	bt_mesh_cfg_mod_sub_add(net_idx, addr, addr, GROUP_ADDR,
							BT_MESH_MODEL_ID_GEN_ONOFF_SRV, &stat);
	printk("Err Code: %d\n", stat);
	/* Add model publishing */
	struct bt_mesh_cfg_mod_pub pub = {
		.addr = GROUP_ADDR,
		.app_idx = app_idx
	};
	bt_mesh_cfg_mod_pub_set(net_idx, addr, addr, BT_MESH_MODEL_ID_GEN_ONOFF_CLI,
						 &pub, &stat);
	printk("Err Code: %d\n",stat);
	
	printk("Configuring done\n");
	/* ===================================*/
	
}
