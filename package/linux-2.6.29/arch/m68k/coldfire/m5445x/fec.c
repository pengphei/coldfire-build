/*
 * fec-mcf5445x.c
 *
 * Sub-architcture dependant initialization code for the Freescale
 * 5445X FEC module.
 *
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Shrek Wu B16972@freescale.com
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
#include <asm/mcffec.h>
#include <asm/mcfsim.h>

static unsigned char    fec_mac_default[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static void fec_request_intrs(struct net_device *dev,
	irqreturn_t fec_net_irq_handler(int irq, void *private),
	void *irq_privatedata)
{
	struct fec_enet_private *fep;
	int b;
	static const struct idesc {
		char *name;
		unsigned short irq;
	} *idp, id[] = {
		{ "fec(TXF)", 36 },
		{ "fec(TXB)", 37 },
		{ "fec(TXFIFO)", 38 },
		{ "fec(TXCR)", 39 },
		{ "fec(RXF)", 40 },
		{ "fec(RXB)", 41 },
		{ "fec(MII)", 42 },
		{ "fec(LC)", 43 },
		{ "fec(HBERR)", 44 },
		{ "fec(GRA)", 45 },
		{ "fec(EBERR)", 46 },
		{ "fec(BABT)", 47 },
		{ "fec(BABR)", 48 },
		{ NULL },
	};

	fep = netdev_priv(dev);
	/*intrruption FEC0 64+x,FEC! 64+13+x*/
	b = (fep->index) ? 77 : 64;

	/* Setup interrupt handlers. */
	for (idp = id; idp->name; idp++) {
		if (request_irq(b+idp->irq,
			fec_net_irq_handler, IRQF_DISABLED,
			idp->name, irq_privatedata) != 0)
			printk(KERN_ERR "FEC: Could not alloc %s IRQ(%d)!\n",
				idp->name, b+idp->irq);
	}

	if (fep->index) {
		/* Configure RMII */
		MCF_GPIO_PAR_FEC = (MCF_GPIO_PAR_FEC &
			MCF_GPIO_PAR_FEC_FEC1_MASK) |
			MCF_GPIO_PAR_FEC_FEC1_RMII_GPIO;
	} else {
		/* Configure RMII */
		MCF_GPIO_PAR_FEC = (MCF_GPIO_PAR_FEC &
			MCF_GPIO_PAR_FEC_FEC0_MASK) |
			MCF_GPIO_PAR_FEC_FEC0_RMII_GPIO;
	}

	/* Set up gpio outputs for MII lines on FEC0 */
	MCF_GPIO_PAR_FECI2C |= (0 |
		MCF_GPIO_PAR_FECI2C_MDIO0_MDIO0 |
		MCF_GPIO_PAR_FECI2C_MDC0_MDC0);
}

static void fec_set_mii(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile fec_t *fecp;

	fecp = fep->hwp;
	fecp->fec_r_cntrl = OPT_FRAME_SIZE | 0x04;
	fecp->fec_x_cntrl = 0x00;

	/*
	* Set MII speed to 2.5 MHz
	*/
	fep->phy_speed = ((((MCF_CLK / 2) / (2500000 / 10)) + 5) / 10) * 2;
	fecp->fec_mii_speed = fep->phy_speed;

}

static void fec_get_mac(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile fec_t *fecp;
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
			iap = fec_mac_default;
		if ((iap[0] == 0xff) && (iap[1] == 0xff) &&
			(iap[2] == 0xff) && (iap[3] == 0xff) &&
			(iap[4] == 0xff) && (iap[5] == 0xff))
			iap = fec_mac_default;
	} else {
		*((unsigned long *) &tmpaddr[0]) = fecp->fec_addr_low;
		*((unsigned short *) &tmpaddr[4]) = (fecp->fec_addr_high >> 16);
		iap = &tmpaddr[0];
	}

	memcpy(dev->dev_addr, iap, ETH_ALEN);

	/* Adjust MAC if using default MAC address */
	if (iap == fec_mac_default)
		dev->dev_addr[ETH_ALEN-1] = fec_mac_default[ETH_ALEN-1] +
						fep->index;
}

static void fec_enable_phy_intr(void)
{
}

static void fec_disable_phy_intr(void)
{
}

static void fec_phy_ack_intr(void)
{
}

static void fec_localhw_setup(void)
{
}

static void fec_uncache(unsigned long addr)
{
}

static void fec_platform_flush_cache(void)
{
}

/*
 * Define the fixed address of the FEC hardware.
 */
static unsigned int fec_platform_hw[] = {
	(MCF_MBAR + 0xfc030000),
#if defined(CONFIG_FEC2)
	(MCF_MBAR + 0xfc034000),
#endif
};

static struct coldfire_fec_platform_data mcf5445x_fec_data = {
	.hash_table = 0,
	.fec_hw = fec_platform_hw,
	.request_intrs = fec_request_intrs,
	.set_mii = fec_set_mii,
	.get_mac = fec_get_mac,
	.enable_phy_intr = fec_enable_phy_intr,
	.disable_phy_intr = fec_disable_phy_intr,
	.phy_ack_intr = fec_phy_ack_intr,
	.localhw_setup = fec_localhw_setup,
	.uncache = fec_uncache,
	.platform_flush_cache = fec_platform_flush_cache,
};

static struct resource fec_coldfire_resources[] = {
	[0] = {
		.start  = MCF_MBAR + 0xfc030000,
		.end    = MCF_MBAR + 0xfc030300,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = (64 + 36),
		.end    = (64 + 48),
		.flags  = IORESOURCE_IRQ,
	},
#if defined(CONFIG_FEC2)
	[2] = {
		.start  = MCF_MBAR + 0xfc034000,
		.end    = MCF_MBAR + 0xfc034300,
		.flags  = IORESOURCE_MEM,
	},
	[3] = {
		.start  = (64 + 49),
		.end    = (64 + 61),
		.flags  = IORESOURCE_IRQ,
	},
#endif
};

static struct platform_device fec_coldfire_device = {
	.name = "coldfire-fec",
	.id = 0,
	.resource = fec_coldfire_resources,
	.num_resources = ARRAY_SIZE(fec_coldfire_resources),
	.dev = {
		.platform_data = &mcf5445x_fec_data,
		.coherent_dma_mask = ~0,        /* $$$ REVISIT */
	}
};


static int __init mcf5445x_fec_dev_init(void)
{
	int retval = 0;

	retval = platform_device_register(&fec_coldfire_device);

	if (retval < 0) {
		printk(KERN_ERR "MCF5445x FEC: platform_device_register failed"
				"with code=%d\n",
			retval);
	}

	return retval;
}

arch_initcall(mcf5445x_fec_dev_init);
