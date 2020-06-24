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



static void bt_ready(int err)
{
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	dk_leds_init();
	dk_buttons_init(NULL);

	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	if (err)
	{
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS))
	{
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

static void self_Provision()
{
	int err = 0;
	err = bt_mesh_provision(net_key, net_idx, flags, iv_index, addr,
							dev_key);
	if (err == -EALREADY)
	{
		printk("Already Provisioned (Stored Settings)\n");
	}
	else if (err)
	{
		printk("Provisioning failed (err %d)\n", err);
		return;
	}
	else
	{
		printk("Provisioning completed\n");
		//configure();
	}
}

static void self_Configure()
{
	printk("Configuring...\n");

	/* Add Application Key */
	bt_mesh_cfg_app_key_add(net_idx, addr, net_idx, app_idx, app_key, NULL);
	/* Bind to Generic ON/OFF Model */
	bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
				     BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);
	/* Add model subscription */
	bt_mesh_cfg_mod_sub_add(net_idx, addr, addr, GROUP_ADDR,
				    BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);

	printk("Configuring done\n");
}



void main(void)
{
	int err;

	printk("Initializing...\n");

	err = bt_enable(bt_ready);
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
	}

	// Do Self Provisioning
	self_Provision();
}
