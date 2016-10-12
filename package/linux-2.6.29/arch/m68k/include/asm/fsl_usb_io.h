/* Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
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
#ifndef _FSL_USB_IO_H
#define _FSL_USB_IO_H

#define fsl_readl(addr)		in_be32((__force unsigned *)(addr))
#define fsl_writel(val32, addr)	out_be32((__force unsigned *)(addr), (val32))

static inline void fsl_set_usb_accessors(struct fsl_usb2_platform_data *pdata)
{
}

#define cpu_to_hc32(x)	(x)
#define hc32_to_cpu(x)	(x)

#endif /* _FSL_USB_IO_H */

