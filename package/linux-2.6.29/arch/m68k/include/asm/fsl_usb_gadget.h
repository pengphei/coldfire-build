/*
 * Copyright 2005-2009 Freescale Semiconductor, Inc. All Rights Reserved.
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

/*
 * USB Gadget side, platform-specific functionality.
 */

#include <linux/usb/fsl_xcvr.h>

/* Needed for i2c/serial transceivers */
static inline void
fsl_platform_set_device_mode(struct fsl_usb2_platform_data *pdata)
{
	if (pdata->xcvr_ops && pdata->xcvr_ops->set_device)
		pdata->xcvr_ops->set_device();
}

static inline void
fsl_platform_pullup_enable(struct fsl_usb2_platform_data *pdata)
{
	if (pdata->xcvr_ops && pdata->xcvr_ops->pullup)
		pdata->xcvr_ops->pullup(1);
}

static inline void
fsl_platform_pullup_disable(struct fsl_usb2_platform_data *pdata)
{
	if (pdata->xcvr_ops && pdata->xcvr_ops->pullup)
		pdata->xcvr_ops->pullup(0);
}
