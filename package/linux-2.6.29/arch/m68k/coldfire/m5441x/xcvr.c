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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/usb/fsl_xcvr.h>
#include <asm/mcfsim.h>

/* Use the configured xcvr_ops */
#ifdef CONFIG_USB_M5441X_ULPI
#define XCVR_OPS &xcvr_ops_ulpi
#elif defined CONFIG_USB_M5441X_FSLS
#define XCVR_OPS &xcvr_ops_fsls
#else
#error "Invalid USB transceiver selection."
#endif

#define MCF_FEC_ENET0_RCR	MCF_REG32(0xFC0D4084)

/*
 * Init routine for on-chip FSLS xcvr
 */
static void xcvr_fsls_init(struct fsl_xcvr_ops *this)
{
	pr_debug("%s: name=%s\n", __func__, this->name);

	/* Configure USB_DP_PDOWN/USB_DM_PDOWN (external resistor) */
	MCF_GPIO_PAR_DBGH1 &= ~MCF_GPIO_PAR_DEBUGH1_DAT0;
	MCF_GPIO_PAR_DBGH1 &= ~MCF_GPIO_PAR_DEBUGH1_DAT1;
	MCF_GPIO_PDDR_I |= 0xc0;
	MCF_GPIO_PODR_I &= 0x3f;

	/* Enable VBUS_EN and VBUS_OC signals */
	MCF_GPIO_PAR_TIMER =
		(MCF_GPIO_PAR_TIMER &
		MCF_GPIO_PAR_TIMER_T3IN_MASK) |
		MCF_GPIO_PAR_TIMER_T3IN_USB0_VBUSEN;

	MCF_GPIO_PAR_TIMER =
		(MCF_GPIO_PAR_TIMER &
		MCF_GPIO_PAR_TIMER_T0IN_MASK) |
		MCF_GPIO_PAR_TIMER_T0IN_USBO_VBUSOC;

	/* Setup USB_VBUS_OC signal to be active-low */
	MCF_CCM_MISCCR &= (~MCF_CCM_MISCCR_USBOOC);
	MCF_CCM_MISCCR |= MCF_CCM_MISCCR_USBPUE;

}

/*
 * Init routine for off-chip ULPI xcvr
 */
static void xcvr_ulpi_init(struct fsl_xcvr_ops *this)
{
	pr_debug("%s: name=%s\n", __func__, this->name);

	/* disable FEC2 clock */
	MCF_PM_PPMSR0 = 54;

	/* reset USB3300 */
	asm ("moveb #0x40, %%d0\n\t"
	     "moveb %%d0, 0xec090000\n\t"
	     : : : "%d0");
	mdelay(1);
	asm ("moveb #0x0, %%d0\n\t"
	     "moveb %%d0, 0xec090000\n\t"
	     : : : "%d0");

	/* enable USB OTG clock*/
	MCF_PM_PPMCR0 = 44;

	/* Chip select for ULPI: GPIO G4 as CS between ULPI and RMII1*/
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
		MCF_GPIO_PODR_G4_MASK) |
		MCF_GPIO_PODR_G4_VAL;

	/* Enable ULPI */
	MCF_CCM_MISCCR2 = MCF_CCM_MISCCR2 & (~MCF_CCM_MISCCR2_ULPI);

	/* Slew rate setting for OTG */
	MCF_GPIO_SRCR_FEC = 0x0f;
	MCF_GPIO_SRCR_TIMER = 0xc3;
	MCF_GPIO_SRCR_IRQ0 = 0x03;

	/* Enable the required ULPI signals */
	MCF_GPIO_PAR_TIMER =
		(MCF_GPIO_PAR_TIMER &
		MCF_GPIO_PAR_TIMER_T3IN_MASK) |
		MCF_GPIO_PAR_TIMER_T3IN_ULPI_DIR;

	MCF_GPIO_PAR_TIMER =
		(MCF_GPIO_PAR_TIMER &
		MCF_GPIO_PAR_TIMER_T0IN_MASK) |
		MCF_GPIO_PAR_TIMER_T0IN_ULPI_NXT;

	MCF_FEC_ENET0_RCR = 0x104; /* FEC0 as RMII mode */

	MCF_GPIO_PAR_FEC =
		(MCF_GPIO_PAR_FEC &
		MCF_GPIO_PAR_FEC_FEC_MASK) |
		MCF_GPIO_PAR_FEC_FEC_RMII0FUL_ULPI;


	pr_debug("MCF_GPIO_PAR_TIMER:0x%x, MCF_GPIO_PAR_FEC:0x%x,"
		 "MCF_GPIO_PAR_SIMP0H:0x%x, MCF_GPIO_PDDR_G:0x%x,"
		 "MCF_GPIO_PODR_G:0x%x, MCF_CCM_MISCCR2:0x%x\n\n",
		 MCF_GPIO_PAR_TIMER, MCF_GPIO_PAR_FEC,
		 MCF_GPIO_PAR_SIMP0H, MCF_GPIO_PDDR_G,
		 MCF_GPIO_PODR_G, MCF_CCM_MISCCR2);

	pr_debug("OTGSC:0x%lx, PORTSC:0x%lx, USBMODE:0x%lx, "
		 "USBCMD:0x%lx, USBSTS:0x%lx, USBINTR:0x%lx, "
		 "MCF_CCM_UOCSR:0x%x\n\n",
		MCF_REG32(0xfc0b01a4), MCF_REG32(0xfc0b0184),
		MCF_REG32(0xfc0b01a8), MCF_REG32(0xfc0b0140),
		MCF_REG32(0xfc0b0144), MCF_REG32(0xfc0b0148),
		MCF_CCM_UOCSR);
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
