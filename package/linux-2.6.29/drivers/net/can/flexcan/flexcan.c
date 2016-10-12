/*
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Author: Huan Wang, b18965@freescale.com, Fri Aug 08 2008
 *
 * Description:
 * CAN bus driver for Freescale Coldfire FlexCAN controller
 *
 * Changelog:
 * Fri Aug 08 2008 Huan Wang <b18965@freescale.com>
 * - create, support for MCF548x's FlexCAN
 *
 * Tue Dec 08 2009 ChengJu Cai <b22600@freescale.com>
 * - support for MCF532x MCF5253 MCF5227x's FlexCAN
 *
 * Thu May 20 2010 Lanttor Guo <Lanttor.Guo@freescale.com>
 * - support for MCF5441X's FlexCAN
 *
 * This file is part of the Linux kernel
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/io.h>
#include <linux/can.h>
#include <asm/mcfsim.h>

#include <linux/can/dev.h>
#include <linux/can/error.h>

#include "flexcan.h"

#define CAN_FILTER	3

#ifdef CONFIG_CAN_DEBUG_DEVICES
# define DBG(x...) printk(x)
#else
# define DBG(x...)
#endif

struct flexcan_priv {
	struct can_priv can;
	long open_time;
	unsigned long flags;
	u32 cur_pri;
	struct list_head tx_head;
	struct net_device *dev;
};
static int flexcan_close(struct net_device *dev);
static int flexcan_hard_start_xmit(struct sk_buff *skb, \
		struct net_device *dev);
static void flexcan_tx_timeout(struct net_device *dev);
static irqreturn_t flexcan_isr(int irq, void *dev_id);
static irqreturn_t flexcan_err_isr(int irq, void *dev_id);

static struct can_bittiming_const flexcan_bittiming_const = {
	.tseg1_min = 1,
	.tseg1_max = 8,
	.tseg2_min = 2,
	.tseg2_max = 8,
	.sjw_max = 4,
	.brp_min = 1,
	.brp_max = 256,
	.brp_inc = 1,
};

static int set_reset_mode(struct net_device *dev)
{
	struct flexcan_priv *priv = netdev_priv(dev);
	volatile struct flexcan_regs *regs = \
			(volatile struct flexcan_regs *)dev->base_addr;
	int i;

	/* disable all MB interrupts */
	regs->canimask = IMASK_BUFF_DISABLE_ALL;

	/* disable error and bus off interrupt */
	regs->canctrl &= ~(CANCTRL_ERRMSK | CANCTRL_BOFFMSK);

	/* reset chip */
	regs->canmcr &= ~(CANMCR_MDIS);
	regs->canmcr |= CANMCR_SOFTRST;

	for (i = 0; i < FLEXCAN_SET_MODE_RETRIES; i++) {
		/* check reset bit */
		if ((regs->canmcr & CANMCR_SOFTRST) == 0x00) {
			priv->can.state = CAN_STATE_STOPPED;
			return 0;
		}

		udelay(10);
	}

	dev_err(ND2D(dev), "setting FlexCAN into reset mode failed!\n");
	return 1;

}

static int set_normal_mode(struct net_device *dev)
{
	struct flexcan_priv *priv = netdev_priv(dev);
	volatile struct flexcan_regs *regs = \
			(volatile struct flexcan_regs *)dev->base_addr;
	int i;

	/* enable the module */
	regs->canmcr &= ~CANMCR_MDIS;

	/* synchronize with can bus*/
	regs->canmcr &= ~CANMCR_HALT;
	for (i = 0; i < FLEXCAN_SET_MODE_RETRIES; i++) {
		/* check freeze and disabled bit */
		if (!(regs->canmcr & (CANMCR_NOTRDY | CANMCR_FRZACK))) {
			priv->can.state = CAN_STATE_ACTIVE;

			/* enable all MB interrupts */
			regs->canimask = IMASK_BUFF_ENABLE_ALL;
			/* Enable error and bus off interrupt */
			regs->canctrl |= (CANCTRL_ERRMSK | CANCTRL_BOFFMSK);

			return 0;
		}
		udelay(10);
	}

	dev_err(ND2D(dev), "setting FlexCAN into normal mode failed!\n");
	return 1;

}

static void flexcan_start(struct net_device *dev)
{
	struct flexcan_priv *priv = netdev_priv(dev);

	/* leave reset mode */
	if (priv->can.state != CAN_STATE_STOPPED)
		set_reset_mode(dev);

	/* leave reset mode */
	set_normal_mode(dev);
}

static int flexcan_set_mode(struct net_device *dev, enum can_mode mode)
{
	switch (mode) {
	case CAN_MODE_START:

		flexcan_start(dev);
		if (netif_queue_stopped(dev))
			netif_wake_queue(dev);
		break;

	default:
		return -EOPNOTSUPP;
	}

	return 0;
}
/*
 * initialize flexcan:
 *   - set clock source
 *   - set output mode
 *   - set baudrate
 *   - enable interrupts
 *   - start operating mode
 */
static void chipset_init(struct net_device *dev, int clock_src)
{
	volatile struct flexcan_regs *regs = \
		(volatile struct flexcan_regs *)dev->base_addr;
	int i, j;

	/* Setting the Clk */
	if (clock_src)
		regs->canctrl |= CANCTRL_CLKSRC;
	else
		regs->canctrl &= ~CANCTRL_CLKSRC;

	/* set LBUF to sending from lower buffer*/
	regs->canctrl |= CANCTRL_LBUF;

	/* initialize message buffers */
	for (i = 0; i < CAN_MB; i++) {
		regs->cantxfg[i].can_dlc = 0;
		regs->cantxfg[i].can_id = 0;
		for (j = 0; j < 8; j++)
			regs->cantxfg[i].data[j] = 0;

		/* put MB into rx queue,
		 * MB[SEND_BUF] just use to send message
		 */
		if (i == SEND_BUF)
			continue;
		regs->cantxfg[i].can_dlc = MB_CNT_CODE(0x04);
	}

	/* Global registers instead of individual regsiters */
	regs->canrxgmsk = 0x00000000;
	regs->canrx14msk = 0x00000000;
	regs->canrx15msk = 0x00000000;

	/* Setting MB[15] to receive the extended frame */
	regs->cantxfg[15].can_dlc |= 0x600000;

	/* setting the loop back */
	/* regs->canctrl |= CANCTRL_LPB; */
}

static int flexcan_get_state(struct net_device *dev, enum can_state *state)
{
	struct flexcan_priv *priv = netdev_priv(dev);
	volatile struct flexcan_regs *regs = \
		(volatile struct flexcan_regs *)dev->base_addr;
	u8 status, fltconf;

	/* FIXME: inspecting the status register to get the current state
	 * is not really necessary, because state changes are handled by
	 * in the ISR and the variable priv->can.state gets updated. The
	 * CAN devicde interface needs fixing!
	 */

	spin_lock_irq(&priv->can.irq_lock);

	if (priv->can.state == CAN_STATE_STOPPED) {
		*state =  CAN_STATE_STOPPED;
	} else {
		status = regs->canerrstat;
		fltconf = (status & ERRSTAT_FLTCONF(3)) >> 4;
		if (fltconf & 0x02)
			*state = CAN_STATE_BUS_OFF;
		else if (fltconf & 0x01)
			*state = CAN_STATE_BUS_PASSIVE;
		else if ((status & ERRSTAT_TXWRN) || (status & ERRSTAT_RXWRN))
			*state = CAN_STATE_BUS_WARNING;
		else
			*state = CAN_STATE_ACTIVE;
	}
	/* Check state */
	if (*state != priv->can.state)
		dev_err(ND2D(dev),
			"Oops, state mismatch: hard %d != soft %d\n",
			*state, priv->can.state);
	spin_unlock_irq(&priv->can.irq_lock);

	return 0;
}

/* Initialization of the FLEXCAN controller.
 */
static int flexcan_open(struct net_device *dev)
{
	int ret;
#if defined(CONFIG_M547X_8X)
	int i;
#endif
	struct flexcan_priv *priv = netdev_priv(dev);

	DBG(KERN_INFO"  Initializing FLEXCAN\n");

#if defined(CONFIG_M547X_8X)
	MCF_PAR_TIMER = MCF_PAR_TIMER | 0x28;
	MCF_PAR_TIMER = MCF_PAR_TIMER & 0xf8;
	MCF_PAR_DSPI = MCF_PAR_DSPI | 0x0a00;
	MCF_PAR_FECI2CIRQ = MCF_PAR_FECI2CIRQ | 0x0283;
	MCF_PAR_PSCn(2) = MCF_PAR_PSCn(2) & 0x0f;
	MCF_PAR_PSCn(2) = MCF_PAR_PSCn(2) | 0x50;
#elif defined(CONFIG_M5441X)
	MCF_PM_PPMCR0 = 8;	/* enable FlexCAN0 clock */
	MCF_PM_PPMCR0 = 9;	/* enable FlexCAN1 clock */

	/* CAN0 */
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_I2C0SCL_MASK) |
		MCF_GPIO_PAR_CANI2C_I2C0SCL_CAN0TX;
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_I2C0SDA_MASK) |
		MCF_GPIO_PAR_CANI2C_I2C0SDA_CAN0RX;
	/* CAN1 */
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_CAN1TX_MASK) |
		MCF_GPIO_PAR_CANI2C_CAN1TX_CAN1TX;
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_CAN1RX_MASK) |
		MCF_GPIO_PAR_CANI2C_CAN1RX_CAN1RX;
#elif defined(CONFIG_M532x)
	MCF_GPIO_PAR_FECI2C &= 0xF0;
	MCF_GPIO_PAR_FECI2C |= MCF_GPIO_PAR_FECI2C_PAR_SDA(0x2) \
		| MCF_GPIO_PAR_FECI2C_PAR_SCL(0x2);
#elif defined(CONFIG_M5227x)
	MCF_GPIO_PAR_I2C = (0 | MCF_GPIO_PAR_I2C_PAR_SCL_CANTX
			      | MCF_GPIO_PAR_I2C_PAR_SDA_CANRX);
#endif

	/* set chip into reset mode */
	set_reset_mode(dev);

	/* determine and set bittime */
	ret = can_set_bittiming(dev);
	if (ret)
		return ret;

#if defined(CONFIG_M547X_8X)
	for (i = 0; i < 2; i++) {
		MCF_ICR(ISC_CANn_MBOR(i)) = 0x33 + 0x01 * i;
		MCF_ICR(ISC_CANn_ERR(i)) = 0x33 + 0x01 * i;
		MCF_ICR(ISC_CANn_BUSOFF(i)) = 0x33 + 0x01 * i;
	}
	ret = request_irq(dev->irq, flexcan_err_isr, \
			IRQF_DISABLED, dev->name, dev);
	ret = request_irq(dev->irq + 1, flexcan_err_isr, \
			IRQF_DISABLED, dev->name, dev);
	ret = request_irq(dev->irq + 2, flexcan_isr, \
			IRQF_DISABLED, dev->name, dev);
#elif defined(CONFIG_M5441X)
	ret = request_irq(dev->irq, flexcan_isr, \
			IRQF_DISABLED, dev->name, dev);
	ret = request_irq(dev->irq + 1, flexcan_err_isr, \
			IRQF_DISABLED, dev->name, dev);
	ret = request_irq(dev->irq + 3, flexcan_err_isr, \
			IRQF_DISABLED, dev->name, dev);
#elif defined(CONFIG_M532x) || defined(CONFIG_M5227x)
	ret = request_irq(dev->irq, flexcan_isr, \
			IRQF_DISABLED, dev->name, dev);
	ret = request_irq(dev->irq + 1, flexcan_err_isr, \
			IRQF_DISABLED, dev->name, dev);
	ret = request_irq(dev->irq + 3, flexcan_err_isr, \
			IRQF_DISABLED, dev->name, dev);
#elif defined(CONFIG_M5253)
	ret = request_irq(dev->irq, flexcan_isr, \
			IRQF_DISABLED, dev->name, dev);
	ret = request_irq(dev->irq - 1, flexcan_err_isr, \
			IRQF_DISABLED, dev->name, dev);
#endif
	if (ret < 0) {
		DBG(KERN_ERR "%s - failed to attach interrupt\n", dev->name);
		return ret;
	}

	/* init and start flexcan */
	chipset_init(dev, 1);
	flexcan_start(dev);
	priv->open_time = jiffies;

	netif_start_queue(dev);
	return 0;
}

/* Set FlexCAN bit time.
 */
static int flexcan_do_set_bittiming(struct net_device *dev)
{
	volatile struct flexcan_regs *regs = \
		(struct flexcan_regs *)dev->base_addr;
	struct flexcan_priv *priv = netdev_priv(dev);
	struct can_bittiming *bt = &priv->can.bittiming;
	volatile u32 reg = 0;

	/* Clear the old bittiming */
	reg = CANCTRL_PRESDIV(0xFF) | CANCTRL_RJW(0x03);
	reg |= (CANCTRL_PROPSEG(0x07) |
		CANCTRL_PSEG1(0x07) |
		CANCTRL_PSEG2(0x07) |
		CANCTRL_SAMP(0x01));
	regs->canctrl &= ~reg;

	/* Set the new bittiming */
	reg = CANCTRL_PRESDIV(bt->brp - 1) | CANCTRL_RJW(bt->sjw - 1);
	reg |= (CANCTRL_PROPSEG(bt->prop_seg - 1) |
		CANCTRL_PSEG1(bt->phase_seg1 - 1) |
		CANCTRL_PSEG2(bt->phase_seg2 - 1) |
		CANCTRL_SAMP(priv->can.ctrlmode & CAN_CTRLMODE_3_SAMPLES));
	regs->canctrl |= reg;

	return 0;
}

#ifdef CONFIG_FLEXCAN_HW_FILTER
/* Set the CAN MB hardware filter.
 */
static int flexcan_do_set_hw_filter(struct net_device *dev, \
		struct can_filter *can_hw_filter)
{
	int i = 0, extended_sign = 0;
	u32 tmp = 0, tmp1 = 0, can_id, can_mask[CAN_FILTER];
	volatile struct flexcan_regs *regs = \
		(struct flexcan_regs *)dev->base_addr;

	/* Into Freezemode */
	regs->canmcr |= CANMCR_HALT;
	while ((regs->canmcr & CANMCR_FRZACK) != CANMCR_FRZACK)
		;

	/* Find out the extended frame MASK and ID. */
	for (i = 0; i < CAN_FILTER; i++) {
		if (can_hw_filter[i].can_id & CAN_EFF_FLAG) {
			tmp = can_hw_filter[CAN_FILTER - 1].can_id;
			can_hw_filter[CAN_FILTER - 1].can_id = \
					can_hw_filter[i].can_id;
			can_hw_filter[i].can_id = tmp;

			tmp = can_hw_filter[CAN_FILTER - 1].can_mask;
			can_hw_filter[CAN_FILTER - 1].can_mask = \
					can_hw_filter[i].can_mask;
			can_hw_filter[i].can_mask = tmp;

			extended_sign = 1;
		}
	}

	/* Set CAN Message Buffer(MB) hardware filter Mask,
	 * canrxgmsk is Global Mask(for all MBs except the MB[14] and MB[15]),
	 * canrx14msk is special for MB[14]
	 * canrx15msk is special for MB[15]
	 */
	for (i = 0; i < CAN_FILTER; i++) {
		can_mask[i] = can_hw_filter[i].can_mask;
		DBG(KERN_INFO "can_mask[%d] = 0x%x\n", i, can_mask[i]);
		tmp = (can_mask[i] & CAN_SFF_MASK) << 18;
		tmp1 = can_mask[i] >> 11;
		can_mask[i] = tmp | tmp1;
	}
	regs->canrxgmsk = can_mask[0];
	regs->canrx14msk = can_mask[1];
	regs->canrx15msk = can_mask[2];
	if (regs->canrx15msk == 0)
		extended_sign = 1;

	/* Set CAN Message Buffer(MB) hardware filter ID,
	 * Note: MB[0] configure as Tx MB and
	 * others configure as Rx MB in CAN driver.
	 * MB[15] only can receive the extended frame and
	 * others Rx MB only can receive the standard frame.
	 */
	for (i = 0; i < CAN_MB; i++) {
		if (i == SEND_BUF)
			continue;

		regs->cantxfg[i].can_dlc = MB_CNT_CODE(0);

		if (i == 14) {
			can_id = (can_hw_filter[1].can_id & MB_ID_STD);
		} else if (i == 15) {
			if (extended_sign == 1) {
				/* Set MB[15] to
				 * receive and filter extended frame
				 */
				can_id = can_hw_filter[2].can_id & MB_ID_EXT;
				regs->cantxfg[i].can_dlc |= 0x600000;
			} else {
				/* Set MB[15] to
				 * receive and filter standard frame
				 */
				can_id = can_hw_filter[2].can_id & MB_ID_STD;
			}
		} else {
			can_id = (can_hw_filter[0].can_id & MB_ID_STD);
		}

		DBG(KERN_INFO "can_id = 0x%x\n", can_id);
		tmp = (can_id & CAN_SFF_MASK) << 18;
		tmp1 = can_id >> 11;
		can_id = tmp | tmp1;
		regs->cantxfg[i].can_id = can_id;
		DBG(KERN_INFO "regs->cantxfg[%d].can_id = 0x%x\n", \
			i, regs->cantxfg[i].can_id);

		regs->cantxfg[i].can_dlc |= MB_CNT_CODE(0x04);
	}

	/* Got out from Freezemode */
	regs->canmcr &= ~CANMCR_HALT;
	while (regs->canmcr & CANMCR_FRZACK)
		;

	return 0;
}

/* Get the CAN MB hardware filter.
 */
static int flexcan_do_get_hw_filter(struct net_device *dev, \
		struct can_filter *can_hw_filter)
{
	int i = 0, tmp = 0, tmp1 = 0, can_mask[CAN_FILTER];
	volatile struct flexcan_regs *regs = \
		(struct flexcan_regs *)dev->base_addr;

	/* Get CAN MB Mask */
	can_mask[0] = regs->canrxgmsk;
	can_mask[1] = regs->canrx14msk;
	can_mask[2] = regs->canrx15msk;
	for (i = 0; i < CAN_FILTER; i++) {
		tmp1 = can_mask[i];
		DBG(KERN_INFO "can_mask[%d] = 0x%x\n", i, can_mask[i]);
		tmp = tmp1;
		tmp = (can_mask[i] >> 18) & CAN_SFF_MASK;
		can_hw_filter[i].can_mask = ((tmp1 << 11) | tmp) & CAN_EFF_MASK;
	}

	/* Get CAN MB ID */
	can_hw_filter[0].can_id = regs->cantxfg[1].can_id >> 18;
	can_hw_filter[1].can_id = regs->cantxfg[14].can_id >> 18;

	tmp1 = regs->cantxfg[15].can_id & MB_ID_EXT;
	tmp = tmp1;
	tmp = (tmp >> 18) & CAN_SFF_MASK;
	can_hw_filter[2].can_id = ((tmp1 << 11) | tmp) & MB_ID_EXT;

	return 0;
}
#endif

/* Shutdown the FLEXCAN controller. Called on close().
 */
static int flexcan_close(struct net_device *dev)
{
	struct flexcan_priv *priv = netdev_priv(dev);

	set_reset_mode(dev);
	netif_stop_queue(dev);
	priv->open_time = 0;
	can_close_cleanup(dev);

#if defined(CONFIG_M547X_8X)
	free_irq(dev->irq, dev);
	free_irq(dev->irq + 1, dev);
	free_irq(dev->irq + 2, dev);
#elif defined(CONFIG_M5441X)
	free_irq(dev->irq, dev);
	free_irq(dev->irq + 1, dev);
	free_irq(dev->irq + 3, dev);
#elif defined(CONFIG_M532x) || defined(CONFIG_M5227x)
	free_irq(dev->irq, dev);
	free_irq(dev->irq + 1, dev);
	free_irq(dev->irq + 3, dev);
#elif defined(CONFIG_M5253)
	free_irq(dev->irq, dev);
	free_irq(dev->irq - 1, dev);
#endif

	return 0;
}

/* Get message from FIFO and send it.
 */
static int flexcan_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	volatile struct flexcan_regs *regs = \
		(volatile struct flexcan_regs *)dev->base_addr;
	struct can_frame *frame = (struct can_frame *)skb->data;
	struct net_device_stats *stats = (struct net_device_stats *)&dev->stats;
	int i, len;
	u32 can_id, can_ext, tmp, tmp1;
	int txbuf = SEND_BUF;

	len = frame->can_dlc & 0xF;

	/* Transmission inactive */
	regs->cantxfg[txbuf].can_dlc = MB_CNT_CODE(0x08);

	can_ext = frame->can_id;
	if (can_ext & CAN_EFF_FLAG) {
		/* Frame format is extended */
		regs->cantxfg[txbuf].can_dlc |= (1 << 21);
		regs->cantxfg[txbuf].can_dlc |= (1 << 22);
		can_id = frame->can_id & MB_ID_EXT;
		if (frame->can_id & CAN_RTR_FLAG)
			regs->cantxfg[txbuf].can_dlc |= (1 << 20);

		tmp = (can_id & CAN_SFF_MASK) << 18;
		tmp1 = can_id >> 11;
		can_id = tmp | tmp1;
		regs->cantxfg[txbuf].can_id = can_id;
	} else {
		/* Frame format is standard */
		can_id = frame->can_id & MB_ID_EXT;
		if (frame->can_id & CAN_RTR_FLAG)
			regs->cantxfg[txbuf].can_dlc |= (1 << 20);

		regs->cantxfg[txbuf].can_id = can_id << 18;
	}

	/* Data transfer */
	for (i = 0; i < len; i++)
		regs->cantxfg[txbuf].data[i] = frame->data[i];
	/* Get the Length */
	regs->cantxfg[txbuf].can_dlc |= len << 16;

	/*Control/status word to hold Tx MB active */
	regs->cantxfg[txbuf].can_dlc |= MB_CNT_CODE(0x0c);
	DBG("Transmission code active 0x%x\n", regs->cantxfg[txbuf].can_dlc);

	stats->tx_packets++;
	stats->tx_bytes += len;

	kfree_skb(skb);
	return NETDEV_TX_OK;
}

/* Called when TX timeout happens
 */
static void flexcan_tx_timeout(struct net_device *dev)
{
	struct sk_buff *skb;
	volatile struct flexcan_regs *regs = \
		(volatile struct flexcan_regs *)dev->base_addr;
	struct can_frame *frame;

	/* Diable the interuppts */
	regs->canimask = IMASK_BUFF_DISABLE_ALL;

	skb = dev_alloc_skb(sizeof(struct can_frame));
	if (!skb) {
		if (printk_ratelimit())
			dev_notice(ND2D(dev), "TIMEOUT packet dropped\n");
		DBG(" TIMEOUT happened here\n");
		return;
	}
	frame = (struct can_frame *)skb_put(skb, sizeof(struct can_frame));

	frame->can_id = CAN_ERR_FLAG;
	frame->can_dlc = CAN_ERR_DLC;

	skb->dev = dev;
	skb->protocol = __constant_htons(ETH_P_CAN);
	skb->pkt_type = PACKET_BROADCAST;
	skb->ip_summed = CHECKSUM_UNNECESSARY;

	netif_rx(skb);
}

/* FlexCAN error and bus-off isr
 */
static irqreturn_t flexcan_err_isr(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct net_device_stats *stats = &dev->stats;

	volatile struct flexcan_regs *regs = \
			(struct flexcan_regs *)dev->base_addr;
	struct flexcan_priv *priv = netdev_priv(dev);
	enum can_state state = priv->can.state;

	u32 errstate = regs->canerrstat;
	u32 fltconf = (errstate & 0x30) >> 4;

	struct can_frame *cf;
	struct sk_buff *skb;

	skb = dev_alloc_skb(sizeof(struct can_frame));
	if (skb == NULL)
		return -ENOMEM;
	skb->dev = dev;
	skb->protocol = htons(ETH_P_CAN);
	cf = (struct can_frame *)skb_put(skb, sizeof(struct can_frame));
	memset(cf, 0, sizeof(struct can_frame));
	cf->can_id = CAN_ERR_FLAG;
	cf->can_dlc = CAN_ERR_DLC;

	if (errstate & 0x04) {
		/* error warning interrupt */
		dev_dbg(ND2D(dev), "error warning interrupt\n");
		priv->can.can_stats.error_warning++;

		state = CAN_STATE_BUS_OFF;
		cf->can_id |= CAN_ERR_BUSOFF;
		can_bus_off(dev);
		regs->canerrstat |= 0x04;
	}
	if (errstate & 0x02) {
		if (errstate & 0xFC00)
			/* bus error interrupt */
			priv->can.can_stats.bus_error++;

		cf->can_id |= CAN_ERR_PROT | CAN_ERR_BUSERROR;

		if (errstate & 0x8000)
			cf->data[2] |= CAN_ERR_PROT_BIT0;
		if (errstate & 0x4000)
			cf->data[2] |= CAN_ERR_PROT_BIT1;
		if (errstate & 0x2000)
			cf->can_id |= CAN_ERR_ACK;
		if (errstate & 0x0800)
			cf->data[2] |= CAN_ERR_PROT_FORM;
		if (errstate & 0x0400)
			cf->data[2] |= CAN_ERR_PROT_STUFF;
		if (errstate & 0x1000) {
			cf->data[2] |= CAN_ERR_PROT_UNSPEC;
			cf->data[3] = errstate & 0xFC00;
		}

		switch (fltconf) {
		case 0:
			state = CAN_STATE_ACTIVE;
			break;
		case 1:
			dev_dbg(ND2D(dev), "error passive interrupt\n");
			priv->can.can_stats.error_passive++;
			state = CAN_STATE_BUS_PASSIVE;
			break;
		default:
			break;
		}

		regs->canerrstat |= 0x02;

	}

	if (state != priv->can.state && (state == CAN_STATE_BUS_WARNING ||
					 state == CAN_STATE_BUS_PASSIVE)) {
		uint8_t rxerr = (regs->canerrcnt & 0xFF00) >> 8;
		uint8_t txerr = (regs->canerrcnt & 0xFF);
		cf->can_id |= CAN_ERR_CRTL;
		if (state == CAN_STATE_BUS_WARNING)
			cf->data[1] = (txerr > rxerr) ?
				CAN_ERR_CRTL_TX_WARNING :
				CAN_ERR_CRTL_RX_WARNING;
		else
			cf->data[1] = (txerr > rxerr) ?
				CAN_ERR_CRTL_TX_PASSIVE :
				CAN_ERR_CRTL_RX_PASSIVE;
	}

	priv->can.state = state;

	netif_rx(skb);

	dev->last_rx = jiffies;
	stats->rx_packets++;
	stats->rx_bytes += cf->can_dlc;

	return IRQ_HANDLED;
}

/* FlexCAN OVERRUN process
 */
static int flexcan_overrun(struct net_device *dev)
{
	struct flexcan_priv *priv = netdev_priv(dev);
	struct net_device_stats *stats = &dev->stats;

	struct can_frame *cf;
	struct sk_buff *skb;

	DBG(KERN_INFO "OVERRUN\n");
	skb = dev_alloc_skb(sizeof(struct can_frame));
	if (skb == NULL)
		return -ENOMEM;

	skb->dev = dev;
	skb->protocol = htons(ETH_P_CAN);
	cf = (struct can_frame *)skb_put(skb, sizeof(struct can_frame));
	memset(cf, 0, sizeof(struct can_frame));
	cf->can_id = CAN_ERR_FLAG;
	cf->can_dlc = CAN_ERR_DLC;

	/* data overrun interrupt */
	dev_dbg(ND2D(dev), "data overrun interrupt\n");
	cf->can_id |= CAN_ERR_CRTL;
	cf->data[1] = CAN_ERR_CRTL_RX_OVERFLOW;
	priv->can.can_stats.data_overrun++;

	netif_rx(skb);

	dev->last_rx = jiffies;
	stats->rx_packets++;
	stats->rx_bytes += cf->can_dlc;

	return 0;
}

/* FlexCAN Message Tx and Rx isr
 */
static irqreturn_t flexcan_isr(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	volatile struct flexcan_regs *regs = \
		(volatile struct flexcan_regs *)dev->base_addr;
	struct net_device_stats *stats = (struct net_device_stats *)&dev->stats;
	struct sk_buff *skb;
	struct can_frame *frame;
	int i, j, k;
	u32 flags, oflags;
	int retval = 1;

	while ((flags = regs->caniflg)) {
		DBG(KERN_INFO "reg->caniflg = 0x%x\n", flags);
		oflags = flags;
		for (i = 0; i < CAN_MB; i++) {
			/* Message Buffer SEND_BUF only used to send data */
			if (i == SEND_BUF) {
				flags = flags >> 1;
				continue;
			}
			if (flags & 0x01) {
				struct flexcan_mb *mb = \
					(struct flexcan_mb *)&regs->cantxfg[i];
				int ctrl = mb->can_dlc;
				int code = (ctrl >> 24) & 0x0f;
				int len = (ctrl >> 16) & 0x0f;
				u32 tmp, tmp1;
				DBG(KERN_INFO "** MB[%d] : CODE=0x%x LEN=%d\n",
					i, code, len);

				if (code < 8 && (len > 0)) {
					/* Rx packet */
					DBG(KERN_INFO "** RX\n");

					/* OVERRUN:A frame
					 * was overwritten into a full buffer.
					 */
					if (code == 6)
						flexcan_overrun(dev);

					for (j = 0; j < len; j++)
						DBG(KERN_INFO "DATA[%d]\
						= 0x%x\n", j, mb->data[j]);
					skb = dev_alloc_skb\
					      (sizeof(struct can_frame));
					if (!skb)
						dev_notice(ND2D(dev),\
							"pakts dropped\n");

					skb->dev = dev;
					frame = (struct can_frame *)skb_put\
					(skb, sizeof(struct can_frame));

					frame->can_dlc = len;
					tmp1 = mb->can_id & MB_ID_EXT;
					if (ctrl & MB_CNT_IDE) {
						tmp = tmp1;
						tmp = (tmp >> 18)\
						      & CAN_SFF_MASK;
						frame->can_id = (tmp1 << 11)\
								| tmp;
						frame->can_id &= CAN_EFF_MASK;
						frame->can_id |= CAN_EFF_FLAG;
						if (ctrl & MB_CNT_RTR)
							frame->can_id |= \
								CAN_RTR_FLAG;
					} else {
						frame->can_id = tmp1 >> 18;
						if (ctrl & MB_CNT_RTR)
							frame->can_id |=\
								CAN_RTR_FLAG;
					}

					for (k = 0; k < 8; k++)
						frame->data[k] =\
						regs->cantxfg[i].data[k];

					stats->rx_packets++;
					stats->rx_bytes += len;

					/* unlock the MB */
					j = regs->cantimer;

					DBG("ID = 0x%x\n", mb->can_id);
					skb->dev = dev;
					skb->protocol = __constant_htons\
							(ETH_P_CAN);
					skb->ip_summed = CHECKSUM_UNNECESSARY;

					retval = netif_rx(skb);
					if (retval == NET_RX_DROP)
						printk(KERN_INFO"\n\
							Packet dropped");

				} else {
					/* Tx packet */
					DBG(KERN_INFO "** TX Completed\n");
				}

			}
			flags = flags >> 1;
		}

		/* clear iflag bits */
		regs->caniflg = oflags;
	}

	return IRQ_HANDLED;
}

/* Register FlexCAN
 */
int register_flexcandev(struct net_device *dev, int clock_src)
{
	int err;

	err = register_candev(dev);
	if (err) {
		printk(KERN_INFO"FlexCAN: registering netdev failed\n");
		free_netdev(dev);
		return err;
	}

	set_reset_mode(dev);
	chipset_init(dev, 1);

	return 0;
}
EXPORT_SYMBOL(register_flexcandev);

/* Unregister FlexCAN
 */
void unregister_flexcandev(struct net_device *dev)
{
	set_reset_mode(dev);
	unregister_netdev(dev);
}
EXPORT_SYMBOL(unregister_flexcandev);

static const struct net_device_ops flexcan_netdev_ops = {
	.ndo_open		= flexcan_open,
	.ndo_stop		= flexcan_close,

	.ndo_start_xmit		= flexcan_hard_start_xmit,
	.ndo_tx_timeout		= flexcan_tx_timeout,
	/* .ndo_do_ioctl	= can_ioctl, */
};

/* This function is called first in this module.
 */
struct net_device *alloc_flexcandev(void)
{
	struct net_device *dev;
	struct flexcan_priv *priv;

	dev = alloc_candev(sizeof(struct flexcan_priv));
	if (!dev)
		return NULL;

	priv = netdev_priv(dev);
	priv->dev = dev;

	dev->netdev_ops = &flexcan_netdev_ops;
	dev->flags |= IFF_NOARP;

	priv->can.bittiming_const = &flexcan_bittiming_const;

	priv->can.do_set_bittiming = flexcan_do_set_bittiming;
#ifdef CONFIG_FLEXCAN_HW_FILTER
	priv->can.do_set_hw_filter = flexcan_do_set_hw_filter;
	priv->can.do_get_hw_filter = flexcan_do_get_hw_filter;
#endif
	priv->can.do_set_mode = flexcan_set_mode;
	priv->can.do_get_state = flexcan_get_state;

	return dev;
}
EXPORT_SYMBOL(alloc_flexcandev);
