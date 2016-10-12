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


extern int usb_platform_dr_init(struct platform_device *pdev);
extern void usb_platform_dr_uninit(struct fsl_usb2_platform_data *pdata);
extern struct platform_device *host_pdev_register(struct resource *res,
		  int n_res, struct fsl_usb2_platform_data *config);


/*
 * Determine which platform_data struct to use, based on which
 * transceiver is configured.
 * PDATA is a pointer to it.
 */
#ifdef CONFIG_USB_M5445X_ULPI
static struct fsl_usb2_platform_data dr_config_ulpi;
#define PDATA (&dr_config_ulpi)
#else
static struct fsl_usb2_platform_data dr_config_fsls;
#define PDATA (&dr_config_fsls)
#endif

/*
 * Used to set pdata->operating_mode before registering the platform_device.
 * If OTG is configured, the controller operates in OTG mode,
 * otherwise it's either host or device.
 */
#ifdef CONFIG_USB_OTG
#define DR_UDC_MODE	FSL_USB2_DR_OTG
#define DR_HOST_MODE	FSL_USB2_DR_OTG
#else
#define DR_UDC_MODE	FSL_USB2_DR_DEVICE
#define DR_HOST_MODE	FSL_USB2_DR_HOST
#endif


#if defined CONFIG_USB_EHCI_HCD || defined CONFIG_USB_EHCI_HCD_MODULE
static inline void dr_register_host(struct resource *r, int rs)
{
	PDATA->operating_mode = DR_HOST_MODE;
	host_pdev_register(r, rs, PDATA);
}
#else
static inline void dr_register_host(struct resource *r, int rs)
{
}
#endif

#ifdef CONFIG_USB_GADGET_FSL_USB2
static struct platform_device dr_udc_device;

static inline void dr_register_udc(void)
{
	PDATA->operating_mode = DR_UDC_MODE;
	dr_udc_device.dev.platform_data = PDATA;

	if (platform_device_register(&dr_udc_device))
		printk(KERN_ERR "usb: can't register DR gadget\n");
	else
		printk(KERN_INFO "usb: DR gadget (%s) registered\n",
		       PDATA->transceiver);
}
#else
static inline void dr_register_udc(void)
{
}
#endif

#ifdef CONFIG_USB_OTG
static struct platform_device dr_otg_device;

/*
 * set the proper operating_mode and
 * platform_data pointer, then register the
 * device.
 */
static inline void dr_register_otg(void)
{
	PDATA->operating_mode = FSL_USB2_DR_OTG;
	dr_otg_device.dev.platform_data = PDATA;

	if (platform_device_register(&dr_otg_device))
		printk(KERN_ERR "usb: can't register otg device\n");
	else
		printk(KERN_INFO "usb: DR OTG registered\n");
}
#else
static inline void dr_register_otg(void)
{
}
#endif
