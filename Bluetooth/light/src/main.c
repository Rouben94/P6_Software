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
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"
#include "const.h"

static void self_Configure()
{

}

static void self_Provision()
{
	int err = 0;
}

static void bt_ready(int err)
{
}

void main(void)
{
	int err;

	printk("Initializing...\n");

	/* ---------- Init Bluetooth ---------- */
	printk("Enabling Bluetooth...\n");
	err = bt_enable(NULL);
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}
	printk("Bluetooth initialized\n");
	/* =====================================*/

	/* --------------- Init Board ------------*/
	printk("Init Buttons and LEDs...\n");
	dk_leds_init();
	dk_buttons_init(NULL);
	printk("Buttons and LEDs initialized\n");
	/* ===================================*/

	/* --------------- Init Mesh ------------*/
	printk("Init Mesh...\n");
	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	//err = bt_mesh_init(&prov, model_handler_init());
	if (err)
	{
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}
	printk("Mesh initialized\n");
	/* ===================================*/

	/* ------------- Provisioning ------------*/
	printk("Provisioning...\n");
	/* This will be a no-op if settings_load() loaded provisioning info */
	//bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
	/*
		if (IS_ENABLED(CONFIG_SETTINGS))
	{
		settings_load();
	}
	*/
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

	/* ------------- Configure Models ------------*/
	printk("Configuring...\n");
	/* Add Application Key */
	bt_mesh_cfg_app_key_add(net_idx, addr, net_idx, app_idx, app_key, &err);
	printk("Err Code: %d\n", err);
	/* Bind to Generic ON/OFF Model */
	bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
							 BT_MESH_MODEL_ID_GEN_ONOFF_SRV, &err);
	printk("Err Code: %d\n", err);
	/* Add model subscription */
	bt_mesh_cfg_mod_sub_add(net_idx, addr, addr, GROUP_ADDR,
							BT_MESH_MODEL_ID_GEN_ONOFF_SRV, &err);
	printk("Err Code: %d\n", err);
	printk("Configuring done\n");
	/* ===================================*/
}
