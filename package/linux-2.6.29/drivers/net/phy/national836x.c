/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc. All rights reserved.
 *	Chenghu Wu <b16972@freescale.com>
 *
 * Driver for National Semiconductor PHYs 83640
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/netdevice.h>

/* DP83640 phy identifier values */
#define DP83640_PHY_ID	0x20005ce0

/* PHY Status Register */
#define MII_DP83640_PHYSTST            16
/* Interrupt Control Register */
#define MII_DP83640_ICR                17
/* Interrupt Status and Interrupt EVEN Enable Register */
#define MII_DP83640_ISR                18

#define MII_DP83640_ICR_IRQEVEN_EN      0x0001
#define MII_DP83640_ICR_IRQOUTPUT_EN    0x0002
#define MII_DP83640_ISR_ENERGY_EVEN     0x0040
#define MII_DP83640_ISR_LINKSTATUS_EVEN 0x0020

static int ns_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		err = phy_write(phydev, MII_DP83640_ICR,
				MII_DP83640_ICR_IRQEVEN_EN |
				MII_DP83640_ICR_IRQOUTPUT_EN);
		err = phy_write(phydev, MII_DP83640_ICR,
				MII_DP83640_ISR_ENERGY_EVEN |
				MII_DP83640_ISR_LINKSTATUS_EVEN);
	} else {
		err = phy_write(phydev, MII_DP83640_ICR, 0);
	}
	return err;
}

static int ns83640_ack_interrupt(struct phy_device *phydev)
{
	int ret = phy_read(phydev, MII_DP83640_ISR);
	if (ret < 0) {
		printk(KERN_INFO "%s MII_DP83640_ISR %x\n",
			__func__, ret);
		return ret;
	}
	return 0;
}

static int ns83640_config_init(struct phy_device *phydev)
{
	int ret = phy_read(phydev, MII_DP83640_PHYSTST);
	if (ret < 0) {
		printk(KERN_INFO "%s MII_DP83640_ISR %x\n",
			__func__, ret);
	}

	return ns83640_ack_interrupt(phydev);
}

static struct phy_driver dp83640_driver = {
	.phy_id = DP83640_PHY_ID,
	.phy_id_mask = 0xfffffff0,
	.name = "NatSemi DP83640",
	.features = PHY_BASIC_FEATURES,
	.flags = PHY_HAS_INTERRUPT,
	.config_init = ns83640_config_init,
	.config_aneg = genphy_config_aneg,
	.read_status = genphy_read_status,
	.ack_interrupt = ns83640_ack_interrupt,
	.config_intr = ns_config_intr,
	.driver = {.owner = THIS_MODULE,}
};

static int __init ns83640_init(void)
{
	return phy_driver_register(&dp83640_driver);
}

static void __exit ns83640_exit(void)
{
	phy_driver_unregister(&dp83640_driver);
}

MODULE_DESCRIPTION("NatSemi PHY driver");
MODULE_AUTHOR("Chenghu Wu <b16972@freescale.com>");
MODULE_LICENSE("GPL v2");

module_init(ns83640_init);
module_exit(ns83640_exit);
