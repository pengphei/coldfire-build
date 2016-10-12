/*
 * Copyright 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

/**
 * struct fsl_xcvr_ops - USB transceiver operations
 *
 * @xcvr_type: one of PORTSCX_PTS_{UTMI,SERIAL,ULPI}
 * @init: transceiver- and board-specific initialization function
 * @uninit: transceiver- and board-specific uninitialization function
 * @set_host:
 * @set_device:
 *
 */
struct fsl_xcvr_ops {
	char *name;
	u32 xcvr_type;
	struct fsl_usb2_platform_data *pdata;

	void (*init)(struct fsl_xcvr_ops *ops);
	void (*uninit)(struct fsl_xcvr_ops *ops);
	void (*suspend)(struct fsl_xcvr_ops *ops);
	void (*set_host)(void);
	void (*set_device)(void);
	void (*set_vbus_power)(struct fsl_xcvr_ops *ops, int on);
	void (*set_remote_wakeup)(u32 *view);
	void (*pullup)(int on);
};

extern void fsl_usb_xcvr_register(struct fsl_xcvr_ops *xcvr_ops);
extern void fsl_usb_xcvr_unregister(struct fsl_xcvr_ops *xcvr_ops);
