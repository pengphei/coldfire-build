/*
 * Copyright (C) 2008-2009 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Author: Huan Wang, b18965@freescale.com, Fri Aug 08 2008
 *
 * Description:
 * FlexCAN defines
 *
 * Changelog:
 * Fri Aug 08 2008 Huan Wang <b18965@freescale.com>
 * - create, support for MCF548x's FlexCAN
 *
 * Tue Dec 08 2009 ChengJu Cai <b22600@freescale.com>
 * - support for MCF532x MCF5253 MCF5227x's FlexCAN
 *
 * This file is part of the Linux kernel
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef __MCF_FLEXCAN_H__
#define __MCF_FLEXCAN_H__

/* Message type access macros.*/
#define FLEXCAN_SET_MODE_RETRIES  255

/* Message Buffer 0  configure as Tx */
#define SEND_BUF	0

/* Structure of the message buffer */
struct flexcan_mb {
	volatile canid_t	can_dlc;
	volatile canid_t	can_id;
	__u8		data[8];
};

struct flexcan_regs {

	volatile u32  canmcr;           /* FLEXCAN 0x00 */
	volatile u32  canctrl;          /* FLEXCAN 0x04 */
	volatile u32  cantimer;         /* FLEXCAN 0x08 */
	volatile u32  reserved1;
	volatile u32  canrxgmsk;        /* FLEXCAN 0x10 */
	volatile u32  canrx14msk;       /* FLEXCAN 0x14 */
	volatile u32  canrx15msk;       /* FLEXCAN 0x18 */
	volatile u32  canerrcnt;        /* FLEXCAN 0x1C */
	volatile u32  canerrstat;       /* FLEXCAN 0x20 */
	volatile u32  reserved2;
	volatile u32  canimask;         /* FLEXCAN 0x28 */
	volatile u32  reserved3;
	volatile u32  caniflg;          /* FLEXCAN 0x30 */

	volatile u32  reserved4[19];
	struct  flexcan_mb cantxfg[CAN_MB];

};

/* @clock_src:
	1 = The FLEXCAN clock source is the onchip Bus Clock.
	0 = The FLEXCAN clock source is the chip Oscillator Clock.*/
struct flexcan_platform_data {
	unsigned int clock_src; /* FLEXCAN_CLKSRC_BUS or FLEXCAN_CLKSRC_XTAL */
	unsigned int clock_frq; /* can ref. clock, in Hz */
};

struct net_device *alloc_flexcandev(void);

extern int register_flexcandev(struct net_device *dev, int clock_src);
extern void unregister_flexcandev(struct net_device *dev);
#endif  /* __MCF_FLEXCAN__ */
