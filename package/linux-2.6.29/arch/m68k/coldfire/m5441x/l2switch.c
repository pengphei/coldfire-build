/*
 * l2switch.c
 *
 * Sub-architcture dependant initialization code for the Freescale
 * 5441X L2 Switch module.
 *
 * Copyright (C) 2010 Freescale Semiconductor, Inc. All rights reserved.
 * ShrekWu B16972@freescale.com
 *
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>

#include <asm/traps.h>
#include <asm/machdep.h>
#include <asm/coldfire.h>
#include <asm/mcfswitch.h>
#include <asm/mcfsim.h>

static unsigned char    switch_mac_default[] = {
	0x00, 0x04, 0x9F, 0x00, 0xB3, 0x49,
};

static void switch_request_intrs(struct net_device *dev,
	irqreturn_t switch_net_irq_handler(int irq, void *private),
	void *irq_privatedata)
{
	struct switch_enet_private *fep;
	int b;
	static const struct idesc {
		char *name;
		unsigned short irq;
	} *idp, id[] = {
		/*{ "esw_isr(EBERR)", 38 },*/
		{ "esw_isr(RxBuffer)", 39 },
		{ "esw_isr(RxFrame)", 40 },
		{ "esw_isr(TxBuffer)", 41 },
		{ "esw_isr(TxFrame)", 42 },
		{ "esw_isr(QM)", 43 },
		{ "esw_isr(P0OutputDiscard)", 44 },
		{ "esw_isr(P1OutputDiscard)", 45 },
		{ "esw_isr(P2OutputDiscard)", 46 },
		{ "esw_isr(LearningRecord)", 47 },
		{ NULL },
	};

	fep = netdev_priv(dev);
	/*intrruption L2 ethernet SWITCH */
	b = 64 + 64 + 64;

	/* Setup interrupt handlers. */
	for (idp = id; idp->name; idp++) {
		if (request_irq(b+idp->irq,
			switch_net_irq_handler, IRQF_DISABLED,
			idp->name, irq_privatedata) != 0)
			printk(KERN_ERR "FEC: Could not alloc %s IRQ(%d)!\n",
				idp->name, b+idp->irq);
	}

	/* Configure RMII */
	MCF_GPIO_PAR_FEC = (MCF_GPIO_PAR_FEC &
		MCF_GPIO_PAR_FEC_FEC_MASK) |
		MCF_GPIO_PAR_FEC_FEC_RMII0FUL_1FUL;

	MCF_GPIO_PAR_FEC =
		(MCF_GPIO_PAR_FEC &
		MCF_GPIO_PAR_FEC_FEC_MASK) |
		MCF_GPIO_PAR_FEC_FEC_RMII0FUL_1FUL;

	MCF_GPIO_SRCR_FEC = 0x0F;

	MCF_GPIO_PAR_SIMP0H =
		(MCF_GPIO_PAR_SIMP0H &
		MCF_GPIO_PAR_SIMP0H_DAT_MASK) |
		MCF_GPIO_PAR_SIMP0H_DAT_GPIO;

	MCF_GPIO_PDDR_G =
		(MCF_GPIO_PDDR_G &
		MCF_GPIO_PDDR_G4_MASK) |
		MCF_GPIO_PDDR_G4_OUTPUT;

	MCF_GPIO_PODR_G =
		(MCF_GPIO_PODR_G &
		MCF_GPIO_PODR_G4_MASK);
}

static void switch_set_mii(struct net_device *dev)
{
	struct switch_enet_private *fep = netdev_priv(dev);
	switch_t *fecp;

	fecp = fep->hwp;

	MCF_FEC_RCR0 = (MCF_FEC_RCR_PROM | MCF_FEC_RCR_RMII_MODE |
			MCF_FEC_RCR_MAX_FL(1522) | MCF_FEC_RCR_CRC_FWD);
	MCF_FEC_RCR1 = (MCF_FEC_RCR_PROM | MCF_FEC_RCR_RMII_MODE |
			MCF_FEC_RCR_MAX_FL(1522) | MCF_FEC_RCR_CRC_FWD);
	/* TCR */
	MCF_FEC_TCR0 = MCF_FEC_TCR_FDEN;
	MCF_FEC_TCR1 = MCF_FEC_TCR_FDEN;
	/* ECR */
#ifdef MODELO_ENHANCE_BUFFER
	MCF_FEC_ECR0 = MCF_FEC_ECR_ETHER_EN | MCF_FEC_ECR_ENA_1588;
	MCF_FEC_ECR1 = MCF_FEC_ECR_ETHER_EN | MCF_FEC_ECR_ENA_1588;
#else /*legac buffer*/
	MCF_FEC_ECR0 = MCF_FEC_ECR_ETHER_EN;
	MCF_FEC_ECR1 = MCF_FEC_ECR_ETHER_EN;
#endif
	/*
	* Set MII speed to 2.5 MHz
	*/
	MCF_FEC_MSCR0 = ((((MCF_CLK / 2) / (2500000 / 10)) + 5) / 10) * 2;
	MCF_FEC_MSCR1 = ((((MCF_CLK / 2) / (2500000 / 10)) + 5) / 10) * 2;

}

static void switch_get_mac(struct net_device *dev)
{
	struct switch_enet_private *fep = netdev_priv(dev);
	switch_t *fecp;
	unsigned char *iap, tmpaddr[ETH_ALEN];

	fecp = fep->hwp;

	if (FEC_FLASHMAC) {
		/*
		* Get MAC address from FLASH.
		* If it is all 1's or 0's, use the default.
		*/
		iap = FEC_FLASHMAC;
		if ((iap[0] == 0) && (iap[1] == 0) && (iap[2] == 0) &&
			(iap[3] == 0) && (iap[4] == 0) && (iap[5] == 0))
			iap = switch_mac_default;
		if ((iap[0] == 0xff) && (iap[1] == 0xff) &&
			(iap[2] == 0xff) && (iap[3] == 0xff) &&
			(iap[4] == 0xff) && (iap[5] == 0xff))
			iap = switch_mac_default;

	} else {
	/*	*((unsigned long *) &tmpaddr[0]) = fecp->fec_addr_low;
		*((unsigned short *) &tmpaddr[4]) = (fecp->fec_addr_high >> 16);
		iap = &tmpaddr[0];
	*/
	}

	/*memcpy(dev->dev_addr, iap, ETH_ALEN);*/
	memcpy(dev->dev_addr, switch_mac_default, ETH_ALEN);
	/* Adjust MAC if using default MAC address */
	/*if (iap == switch_mac_default)
		dev->dev_addr[ETH_ALEN-1] = switch_mac_default[ETH_ALEN-1] +
						fep->index;*/
}

static void switch_enable_phy_intr(void)
{
}

static void switch_disable_phy_intr(void)
{
}

static void switch_phy_ack_intr(void)
{
}

static void switch_localhw_setup(void)
{
}

static void switch_uncache(unsigned long addr)
{
}

static void switch_platform_flush_cache(void)
{
}

/*
 * Define the fixed address of the FEC hardware.
 */
static unsigned int switch_platform_hw[] = {
	(0xfc0dc000),
	(0xfc0e000),
};

static struct coldfire_switch_platform_data mcf5441x_switch_data = {
	.hash_table = 0,
	.switch_hw = switch_platform_hw,
	.request_intrs = switch_request_intrs,
	.set_mii = switch_set_mii,
	.get_mac = switch_get_mac,
	.enable_phy_intr = switch_enable_phy_intr,
	.disable_phy_intr = switch_disable_phy_intr,
	.phy_ack_intr = switch_phy_ack_intr,
	.localhw_setup = switch_localhw_setup,
	.uncache = switch_uncache,
	.platform_flush_cache = switch_platform_flush_cache,
};

static struct resource l2switch_coldfire_resources[] = {
	[0] = {
		.start  = 0xFC0DC000,
		.end    = 0xFC0DC508,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = (64 + 64 + 64 + 38),
		.end    = (64 + 64 + 64 + 48),
		.flags  = IORESOURCE_IRQ,
	},
	[2] = {
		.start  = 0xFC0E0000,
		.end    = 0xFC0E3FFC,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device l2switch_coldfire_device = {
	.name = "coldfire-switch",
	.id = 0,
	.resource = l2switch_coldfire_resources,
	.num_resources = ARRAY_SIZE(l2switch_coldfire_resources),
	.dev = {
		.platform_data = &mcf5441x_switch_data,
		.coherent_dma_mask = ~0,        /* $$$ REVISIT */
	}
};


static int __init mcf5441x_switch_dev_init(void)
{
	int retval = 0;

	retval = platform_device_register(&l2switch_coldfire_device);

	if (retval < 0) {
		printk(KERN_ERR "MCF5441x L2Switch: platform_device_register"
				" failed with code=%d\n", retval);
	}

	return retval;
}

arch_initcall(mcf5441x_switch_dev_init);
