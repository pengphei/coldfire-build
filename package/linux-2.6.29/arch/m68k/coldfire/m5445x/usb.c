/*
 * Copyright 2004-2009 Freescale Semiconductor, Inc. All Rights Reserved.
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
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <linux/usb/fsl_xcvr.h>
#include <asm/mcfsim.h>

#define MCF_SCM_BCR		MCF_REG32(0xFC040024)
#define MCF_SCM_BCR_GBR		(1 << 9)	/* global bursts for read */
#define MCF_SCM_BCR_GBW		(1 << 8)	/* global bursts for write */
#define MCF_SCM_BCR_SBE_ALL	(0xff << 0)	/* slave burst enable */

#define MAX_XCVR 3
struct fsl_xcvr_ops *g_xc_ops[MAX_XCVR] = { NULL };

#ifdef ULPI_DEBUG
void print_ulpi_regs(void)
{
	pr_debug("MCF_SCM_BCR=0x%08lx  MCF_CCM_MISCCR=0x%08x  "
		 "MCF_GPIO_PAR_DMA=0x%08x  MCF_GPIO_PAR_USB=08%08x  "
		 "MCF_GPIO_PAR_FEC=08%08x\n",
		 MCF_SCM_BCR, MCF_CCM_MISCCR, MCF_GPIO_PAR_DMA,
		 MCF_GPIO_PAR_USB, MCF_GPIO_PAR_FEC);
}
EXPORT_SYMBOL(print_ulpi_regs);
#endif

static inline void fsl_usb_enable_clk(void)
{
	pr_debug("%s\n", __func__);

	/* Use external clock source if PLL isn't a multiple of 60MHz */
	MCF_CCM_MISCCR &= ~MCF_CCM_MISCCR_USBSRC;

	/* Initialize the USB Clock: use USB input clock */
	MCF_GPIO_PAR_DMA = (MCF_GPIO_PAR_DMA & MCF_GPIO_PAR_DMA_DREQ1_MASK) |
			   MCF_GPIO_PAR_DMA_DREQ1_USB_CLKIN;
}

static inline void fsl_usb_disable_clk(void)
{
	pr_debug("%s\n", __func__);
}

void fsl_usb_xcvr_register(struct fsl_xcvr_ops *xcvr_ops)
{
	int i;

	pr_debug("%s '%s'\n", __func__, xcvr_ops->name);
	for (i = 0; i < MAX_XCVR; i++) {
		if (g_xc_ops[i] == NULL) {
			g_xc_ops[i] = xcvr_ops;
			return;
		}
	}

	pr_debug("%s failed\n", __func__);
}
EXPORT_SYMBOL_GPL(fsl_usb_xcvr_register);

void fsl_usb_xcvr_unregister(struct fsl_xcvr_ops *xcvr_ops)
{
	int i;

	pr_debug("%s '%s'\n", __func__, xcvr_ops->name);
	for (i = 0; i < MAX_XCVR; i++) {
		if (g_xc_ops[i] == xcvr_ops) {
			g_xc_ops[i] = NULL;
			return;
		}
	}

	pr_debug("%s failed\n", __func__);
}
EXPORT_SYMBOL_GPL(fsl_usb_xcvr_unregister);

static struct fsl_xcvr_ops *fsl_usb_get_xcvr(char *name)
{
	int i;

	pr_debug("%s '%s'\n", __func__, name);
	if (name == NULL) {
		printk(KERN_ERR "get_xcvr(): No tranceiver name\n");
		return NULL;
	}

	for (i = 0; i < MAX_XCVR; i++) {
		if (strcmp(g_xc_ops[i]->name, name) == 0)
			return g_xc_ops[i];
	}
	pr_debug("Failed %s\n", __func__);
	return NULL;
}

/* The dmamask must be set for EHCI to work */
static u64 ehci_dmamask = ~(u32) 0;

/*!
 * Register an instance of a USB host platform device.
 *
 * @param	res:	resource pointer
 * @param       n_res:	number of resources
 * @param       config: config pointer
 *
 * @return      newly-registered platform_device
 *
 * Each supported host interface is registered as an instance
 * of the "fsl-ehci" device.  Call this function multiple times
 * to register each host interface.
 */
static int instance_id;
struct platform_device *host_pdev_register(struct resource *res, int n_res,
					   struct fsl_usb2_platform_data *config)
{
	struct platform_device *pdev;
	int rc;

	pr_debug("register host res=0x%p, size=%d\n", res, n_res);

	pdev = platform_device_register_simple("fsl-ehci",
					       instance_id, res, n_res);
	if (IS_ERR(pdev)) {
		printk(KERN_ERR "usb: can't register %s Host, %ld\n",
		       config->name, PTR_ERR(pdev));
		return NULL;
	}

	pdev->dev.coherent_dma_mask = 0xffffffff;
	pdev->dev.dma_mask = &ehci_dmamask;

	rc = platform_device_add_data(pdev, config,
				      sizeof(struct fsl_usb2_platform_data));
	if (rc) {
		platform_device_unregister(pdev);
		return NULL;
	}

	printk(KERN_INFO "usb: %s host (%s) registered\n", config->name,
	       config->transceiver);
	pr_debug("pdev=0x%p  dev=0x%p  resources=0x%p  pdata=0x%p\n",
		 pdev, &pdev->dev, pdev->resource, pdev->dev.platform_data);

	instance_id++;

	return pdev;
}


static int dr_used;

int usb_platform_dr_init(struct platform_device *pdev)
{
	struct fsl_usb2_platform_data *pdata = pdev->dev.platform_data;
	struct fsl_xcvr_ops *xops;

	pr_debug("%s: pdev=0x%p  pdata=0x%p\n", __func__, pdev, pdata);

	xops = fsl_usb_get_xcvr(pdata->transceiver);
	if (!xops) {
		printk(KERN_ERR "DR transceiver ops missing\n");
		return -EINVAL;
	}
	pdata->xcvr_ops = xops;
	pdata->xcvr_type = xops->xcvr_type;
	pdata->pdev = pdev;
	xops->pdata = pdata;

	/* enable USB read, write and slave bursts */
	MCF_SCM_BCR = MCF_SCM_BCR_GBR | MCF_SCM_BCR_GBW | MCF_SCM_BCR_SBE_ALL;

	if (!dr_used) {
		fsl_usb_enable_clk();

		if (xops->init)
			xops->init(xops);
	}

	dr_used++;
	pr_debug("%s: success\n", __func__);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_platform_dr_init);

void usb_platform_dr_uninit(struct fsl_usb2_platform_data *pdata)
{
	pr_debug("%s\n", __func__);

	dr_used--;
	if (!dr_used) {
		if (pdata->xcvr_ops && pdata->xcvr_ops->uninit)
			pdata->xcvr_ops->uninit(pdata->xcvr_ops);

		pdata->regs = NULL;
		fsl_usb_disable_clk();
	}
}
EXPORT_SYMBOL_GPL(usb_platform_dr_uninit);
