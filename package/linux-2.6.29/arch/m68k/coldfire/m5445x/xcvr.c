/*
 * Copyright 2005-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/usb/fsl_xcvr.h>
#include <asm/mcfsim.h>

/* Use the configured xcvr_ops */
#ifdef CONFIG_USB_M5445X_ULPI
#define XCVR_OPS &xcvr_ops_ulpi
#elif defined CONFIG_USB_M5445X_FSLS
#define XCVR_OPS &xcvr_ops_fsls
#else
#error "Invalid USB transceiver selection."
#endif

/*
 * Init routine for on-chip FSLS xcvr
 */
static void xcvr_fsls_init(struct fsl_xcvr_ops *this)
{
	pr_debug("%s: name=%s\n", __func__, this->name);

	/* Enable VBUS_EN and VBUS_OC signals */
	MCF_GPIO_PAR_USB = MCF_GPIO_PAR_USB_VBUSEN_VBUSEN |
			   MCF_GPIO_PAR_USB_VBUSOC_VBUSOC;

	/* Setup USB_VBUS_OC signal to be active-low */
	MCF_CCM_MISCCR |= MCF_CCM_MISCCR_USBOC;
}

/*
 * Init routine for off-chip ULPI xcvr
 */
static void xcvr_ulpi_init(struct fsl_xcvr_ops *this)
{
	pr_debug("%s: name=%s\n", __func__, this->name);

	/* Enable the required ULPI signals */
	MCF_GPIO_PAR_DMA = (MCF_GPIO_PAR_DMA &
			    MCF_GPIO_PAR_DMA_DACK1_MASK) |
			    MCF_GPIO_PAR_DMA_DACK1_ULPI_DIR;

	MCF_GPIO_PAR_USB = MCF_GPIO_PAR_USB_VBUSEN_ULPI_NXT |
			   MCF_GPIO_PAR_USB_VBUSOC_ULPI_STP;

	MCF_GPIO_PAR_FEC = (MCF_GPIO_PAR_FEC &
			    MCF_GPIO_PAR_FEC_FEC0_MASK) |
			    MCF_GPIO_PAR_FEC_FEC0_RMII_ULPI;
}

static void xcvr_uninit(struct fsl_xcvr_ops *this)
{
	pr_debug("%s\n", __func__);
}

static void xcvr_pullup(int on)
{
	unsigned short ccm = in_be16(&MCF_CCM_UOCSR);

	pr_debug("%s(%d)\n", __func__, on);

	ccm = in_be16(&MCF_CCM_UOCSR);
	if (on) {
		ccm |= MCF_CCM_UOCSR_BVLD;
		ccm &= ~MCF_CCM_UOCSR_SEND;
		out_be16(&MCF_CCM_UOCSR, ccm);
	} else {
		ccm &= ~MCF_CCM_UOCSR_BVLD;
		out_be16(&MCF_CCM_UOCSR, ccm);
	}
}

struct fsl_xcvr_ops xcvr_ops_ulpi = {
	.init   = xcvr_ulpi_init,
	.uninit = xcvr_uninit,
	.pullup = xcvr_pullup,
	.name   = "dr-ulpi",
};

struct fsl_xcvr_ops xcvr_ops_fsls = {
	.init   = xcvr_fsls_init,
	.uninit = xcvr_uninit,
	.pullup = xcvr_pullup,
	.name   = "dr-fsls",
};

static int __init usb_xcvr_init(void)
{
	struct fsl_xcvr_ops *xops = XCVR_OPS;

	pr_debug("%s %s\n", __func__, xops->name);
	fsl_usb_xcvr_register(xops);

	return 0;
}

static void __exit usb_xcvr_exit(void)
{
	fsl_usb_xcvr_unregister(XCVR_OPS);
}

module_init(usb_xcvr_init);
module_exit(usb_xcvr_exit);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("External ULPI xcvr driver");
MODULE_LICENSE("GPL");
