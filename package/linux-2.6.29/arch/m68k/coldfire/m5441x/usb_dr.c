/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <asm/mcfsim.h>

#define USB_OTGREGS_BASE MCF_REG32(0xFC0B0000)
#define INT_USB		(64 + 64 + 64 + 17)	/* INTC1:17 17.2.9.1*/
#define INT_UOCSR	(64 + 64 + 64 + 27)	/* INTC1:27 17.2.9.1 */

#include "usb.h"

/*
 * platform data structs
 * 	- Which one to use is determined by CONFIG options in usb.h
 * 	- operating_mode plugged at run time
 */

/* off-chip ULPI transceiver */
static struct fsl_usb2_platform_data __maybe_unused dr_config_ulpi = {
	.name            = "DR",
	.platform_init   = usb_platform_dr_init,
	.platform_uninit = usb_platform_dr_uninit,
	.phy_mode	 = FSL_USB2_PHY_ULPI,
	.transceiver     = "dr-ulpi",
	.power_budget    = 500,
	.es              = 1,
	.big_endian_mmio = 1,
	.big_endian_desc = 1,
	.le_setup_buf    = 1,
};

/* on-chip FS/LS serial transceiver */
static struct fsl_usb2_platform_data __maybe_unused dr_config_fsls = {
	.name            = "DR",
	.platform_init   = usb_platform_dr_init,
	.platform_uninit = usb_platform_dr_uninit,
	.phy_mode        = FSL_USB2_PHY_SERIAL,
	.transceiver     = "dr-fsls",
	.power_budget    = 500,
	.es              = 1,
	.big_endian_mmio = 1,
	.big_endian_desc = 1,
	.le_setup_buf    = 1,
};

/*
 * resources
 */
static struct resource dr_udc_resources[] = {
	[0] = {
		.start = (u32)(&USB_OTGREGS_BASE),
		.end   = (u32)(&USB_OTGREGS_BASE + 0x1ff),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource dr_otg_resources[] = {
	[0] = {
		.start = (u32)(&USB_OTGREGS_BASE),
		.end   = (u32)(&USB_OTGREGS_BASE + 0x1ff),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource dr_host_resources[] = {
	[0] = {
		.start = (u32)(&USB_OTGREGS_BASE),
		.end   = (u32)(&USB_OTGREGS_BASE + 0x1ff),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB,
		.flags = IORESOURCE_IRQ,
	},
};

static u64 dr_udc_dmamask = ~(u32) 0;
static void dr_udc_release(struct device *dev)
{
}

static u64 dr_otg_dmamask = ~(u32) 0;
static void dr_otg_release(struct device *dev)
{
}

/*
 * platform device structs
 * 	dev.platform_data field plugged at run time
 */
static struct platform_device __maybe_unused dr_udc_device = {
	.name = "fsl-usb2-udc",
	.id   = -1,
	.dev  = {
		.release           = dr_udc_release,
		.dma_mask          = &dr_udc_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.resource      = dr_udc_resources,
	.num_resources = ARRAY_SIZE(dr_udc_resources),
};

static struct platform_device __maybe_unused dr_otg_device = {
	.name = "fsl-usb2-otg",
	.id = -1,
	.dev = {
		.release           = dr_otg_release,
		.dma_mask          = &dr_otg_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.resource      = dr_otg_resources,
	.num_resources = ARRAY_SIZE(dr_otg_resources),
};

static int __init usb_dr_init(void)
{
	pr_debug("%s:\n", __func__);

	dr_register_otg();
	dr_register_host(dr_host_resources, ARRAY_SIZE(dr_host_resources));
	dr_register_udc();

	return 0;
}

module_init(usb_dr_init);
