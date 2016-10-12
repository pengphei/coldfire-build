/*
 * Copyright (C) 2005-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/usb/fsl_xcvr.h>
#include <linux/usb/fsl_usb2.h>
#include <asm/mcfsim.h>

/* ehci_arc_hc_driver.flags value */
#define FSL_PLATFORM_HC_FLAGS (HCD_USB2 | HCD_MEMORY)

static void fsl_setup_phy(struct ehci_hcd *ehci,
			  enum fsl_usb2_phy_modes phy_mode,
			  int port_offset);


static inline void fsl_platform_usb_setup(struct ehci_hcd *ehci)
{
	struct fsl_usb2_platform_data *pdata;

	pdata = ehci_to_hcd(ehci)->self.controller->platform_data;
	fsl_setup_phy(ehci, pdata->phy_mode, 0);
}

static inline void fsl_platform_set_host_mode(struct usb_hcd *hcd)
{
	unsigned int temp;
	struct fsl_usb2_platform_data *pdata;
	struct fsl_usb_host_regs *regs;

	pdata = hcd->self.controller->platform_data;
	regs = pdata->regs;

	if (pdata->xcvr_ops && pdata->xcvr_ops->set_host)
		pdata->xcvr_ops->set_host();

	/* set host mode and select "big endian" */
	temp = in_be32(&regs->usbmode);
	temp |= USBMODE_CM_HOST | (pdata->es ? USBMODE_ES : 0);
	out_be32(&regs->usbmode, temp);

	pr_debug("%s: set usbmode to 0x%x\n\n", __FUNCTION__,
		in_be32(&regs->usbmode));

}

static inline void
fsl_platform_set_vbus_power(struct fsl_usb2_platform_data *pdata, int on)
{
#ifdef CONFIG_USB_M5441X_FSLS
	if (on)
		MCF_CCM_UOCSR |= MCF_CCM_UOCSR_DRV_VBUS;
	else
		MCF_CCM_UOCSR &= (~MCF_CCM_UOCSR_DRV_VBUS);
#endif
}
