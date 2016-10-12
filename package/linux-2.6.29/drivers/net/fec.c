/*
 * Fast Ethernet Controller (FEC) driver for Motorola MPC8xx.
 * Copyright (c) 1997 Dan Malek (dmalek@jlc.net)
 *
 * Right now, I am very wasteful with the buffers.  I allocate memory
 * pages and then divide them into 2K frame buffers.  This way I know I
 * have buffers large enough to hold one frame within one buffer descriptor.
 * Once I get this working, I will use 64 or 128 byte CPM buffers, which
 * will be much more memory efficient and will easily handle lots of
 * small packets.
 *
 * Much better multiple PHY support by Magnus Damm.
 * Copyright (c) 2000 Ericsson Radio Systems AB.
 *
 * Support for FEC controller of ColdFire processors.
 * Copyright (c) 2001-2005 Greg Ungerer (gerg@snapgear.com)
 *
 * Bug fixes and cleanup by Philippe De Muyter (phdm@macqel.be)
 * Copyright (c) 2004-2006 Macq Electronique SA.
 *
 * Copyright 2007-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 * Author: Kurt Mahan, kmahan@freescale.com
 *         Jason Jin, Jason.jin@freescale.com
 *         Chenghu Wu <b16972@freescale.com>
 * Bug fixes, add Coldfire support, cleanup,
 * move the phy support to the phy level.
 * Add 1588 function into FEC driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <linux/phy.h>

#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <asm/pgtable.h>
#include <asm/cacheflush.h>

#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include "fec.h"
#include "fec_1588.h"

#if defined(CONFIG_FEC2)
#define	FEC_MAX_PORTS	2
#else
#define	FEC_MAX_PORTS	1
#endif

/* The FEC buffer descriptors track the ring buffers.  The rx_bd_base and
 * tx_bd_base always point to the base of the buffer descriptors.  The
 * cur_rx and cur_tx point to the currently available buffer.
 * The dirty_tx tracks the current buffer that is being sent by the
 * controller.  The cur_tx and dirty_tx are equal under both completely
 * empty and completely full conditions.  The empty/ready indicator in
 * the buffer descriptor determines the actual condition.
 */
struct fec_enet_private {
	/* Hardware registers of the FEC device */
	volatile fec_t  *hwp;

	struct net_device *netdev;
	struct platform_device *pdev;
	/* The saved address of a sent-in-place packet/buffer, for skfree(). */
	unsigned char *tx_bounce[TX_RING_SIZE];
	struct  sk_buff *tx_skbuff[TX_RING_SIZE];
	ushort  skb_cur;
	ushort  skb_dirty;

	/* CPM dual port RAM relative addresses.*/
	cbd_t   *rx_bd_base;      /* Address of Rx and Tx buffers. */
	cbd_t   *tx_bd_base;
	cbd_t   *cur_rx, *cur_tx; /* The next free ring entry */
	cbd_t   *dirty_tx;        /* The ring entries to be free()ed. */
	uint    tx_full;
	/* hold while accessing the HW like ringbuffer for tx/rx but not MAC */
	spinlock_t hw_lock;
	/* hold while accessing the mii_list_t() elements */
	spinlock_t mii_lock;
	struct mii_bus *mdio_bus;
	struct phy_device *phydev;

	uint    phy_id;
	uint    phy_id_done;
	uint    phy_status;
	uint    phy_speed;
	phy_info_t const        *phy;
	struct work_struct phy_task;
	volatile fec_t  *phy_hwp;

	uint    sequence_done;
	uint    mii_phy_task_queued;

	uint    phy_addr;

	int     index;
	int     opened;
	int     link;
	int     old_link;
	int     full_duplex;
	int     duplex;
	int     speed;
	int     msg_enable;
#ifdef CONFIG_FEC_1588
	struct fec_ptp_private *ptp_priv;
	uint    ptimer_present;
#endif
};

struct fec_platform_private {
	struct platform_device  *pdev;

	unsigned long           quirks;
	int                     num_slots;      /* Slots on controller */
	struct fec_enet_private *fep_host[0];      /* Pointers to hosts */
};
/* Interrupt events/masks.
*/
#define FEC_ENET_HBERR	((uint)0x80000000)	/* Heartbeat error */
#define FEC_ENET_BABR	((uint)0x40000000)	/* Babbling receiver */
#define FEC_ENET_BABT	((uint)0x20000000)	/* Babbling transmitter */
#define FEC_ENET_GRA	((uint)0x10000000)	/* Graceful stop complete */
#define FEC_ENET_TXF	((uint)0x08000000)	/* Full frame transmitted */
#define FEC_ENET_TXB	((uint)0x04000000)	/* A buffer was transmitted */
#define FEC_ENET_RXF	((uint)0x02000000)	/* Full frame received */
#define FEC_ENET_RXB	((uint)0x01000000)	/* A buffer was received */
#define FEC_ENET_MII	((uint)0x00800000)	/* MII interrupt */
#define FEC_ENET_EBERR	((uint)0x00400000)	/* SDMA bus error */
#define FEC_ENET_TS_AVAIL       ((uint)0x00010000)
#define FEC_ENET_TS_TIMER       ((uint)0x00008000)
static int fec_enet_open(struct net_device *dev);
static int fec_enet_start_xmit(struct sk_buff *skb, struct net_device *dev);
static void fec_enet_mii(struct net_device *dev);
static irqreturn_t fec_enet_interrupt(int irq, void *dev_id);
static void fec_enet_tx(struct net_device *dev);
static void fec_enet_rx(struct net_device *dev);
static int fec_enet_close(struct net_device *dev);
static void set_multicast_list(struct net_device *dev);
static void fec_restart(struct net_device *dev, int duplex);
static void fec_stop(struct net_device *dev);
static void fec_set_mac_address(struct net_device *dev);

#define		NMII	20

/* Make MII read/write commands for the FEC.
*/
#define mk_mii_read(REG)	(0x60020000 | ((REG & 0x1f) << 18))
#define mk_mii_write(REG, VAL)	(0x50020000 | ((REG & 0x1f) << 18) | \
						(VAL & 0xffff))

/* Transmitter timeout.
*/
#define TX_TIMEOUT (2*HZ)

/* ------------------------------------------------------------------- */
static int
fec_enet_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct fec_enet_private *fep;
	volatile fec_t	*fecp;
	volatile cbd_t	*bdp;
	unsigned short	status;
#ifdef CONFIG_FEC_1588
	unsigned long   estatus;
#endif
	unsigned long flags;

	fep = netdev_priv(dev);
	fecp = (volatile fec_t *)dev->base_addr;

	if (!fep->link) {
		/* Link is down or autonegotiation is in progress. */
		return 1;
	}

	spin_lock_irqsave(&fep->hw_lock, flags);
	/* Fill in a Tx ring entry */
	bdp = fep->cur_tx;

	status = bdp->cbd_sc;
#ifndef final_version
	if (status & BD_ENET_TX_READY) {
		/* Ooops.  All transmit buffers are full.  Bail out.
		 * This should not happen, since dev->tbusy should be set.
		 */
		printk(KERN_ERR "%s: tx queue full!.\n", dev->name);
		spin_unlock_irqrestore(&fep->hw_lock, flags);
		return 1;
	}
#endif

	/* Clear all of the status flags.
	 */
	status &= ~BD_ENET_TX_STATS;

	/* Set buffer length and buffer pointer.
	*/
	bdp->cbd_bufaddr = __pa(skb->data);
	bdp->cbd_datlen = skb->len;

	/*
	 *	On some FEC implementations data must be aligned on
	 *	4-byte boundaries. Use bounce buffers to copy data
	 *	and get it aligned. Ugh.
	 */
	if (bdp->cbd_bufaddr & 0x3) {
		unsigned int index1;
		index1 = bdp - fep->tx_bd_base;
		memcpy(fep->tx_bounce[index1],
		       (void *)skb->data, bdp->cbd_datlen);
		bdp->cbd_bufaddr = __pa(fep->tx_bounce[index1]);
	}

	/* Save skb pointer.
	*/
	fep->tx_skbuff[fep->skb_cur] = skb;

	dev->stats.tx_bytes += skb->len;
	fep->skb_cur = (fep->skb_cur+1) & TX_RING_MOD_MASK;

	/* Push the data cache so the CPM does not get stale memory
	 * data.
	 */
	flush_dcache_range((unsigned long)skb->data,
			   (unsigned long)skb->data + skb->len);

	/* Send it on its way.  Tell FEC it's ready, interrupt when done,
	 * it's the last BD of the frame, and to put the CRC on the end.
	 */

	status |= (BD_ENET_TX_READY | BD_ENET_TX_INTR
			| BD_ENET_TX_LAST | BD_ENET_TX_TC);
	bdp->cbd_sc = status;
#ifdef CONFIG_FEC_1588
	if (fep->ptimer_present) {
		if (fec_ptp_do_txstamp(skb))
			estatus = BD_ENET_TX_TS;
		else
			estatus = 0;

		bdp->cbd_esc = (estatus | BD_ENET_TX_INT);
		bdp->cbd_bdu = BD_ENET_TX_BDU;
	}
#endif
	dev->trans_start = jiffies;

	/* Trigger transmission start */
	fecp->fec_x_des_active = 0;

	/* If this was the last BD in the ring, start at the beginning again.
	*/
	if (status & BD_ENET_TX_WRAP)
		bdp = fep->tx_bd_base;
	else
		bdp++;

	if (bdp == fep->dirty_tx) {
		fep->tx_full = 1;
		netif_stop_queue(dev);
	}

	fep->cur_tx = (cbd_t *)bdp;

	spin_unlock_irqrestore(&fep->hw_lock, flags);

	return 0;
}

static void
fec_timeout(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	printk(KERN_INFO "%s: transmit timed out.\n", dev->name);
	dev->stats.tx_errors++;
#ifndef final_version
	{
	int	i;
	cbd_t	*bdp;

	printk(KERN_INFO "Ring data dump: cur_tx %lx%s,"
		"dirty_tx %lx cur_rx: %lx\n",
		(unsigned long)fep->cur_tx, fep->tx_full ? " (full)" : "",
		(unsigned long)fep->dirty_tx,
		(unsigned long)fep->cur_rx);

	bdp = fep->tx_bd_base;
	printk(KERN_INFO " tx: %u buffers\n",  TX_RING_SIZE);
	for (i = 0 ; i < TX_RING_SIZE; i++) {
		printk(KERN_INFO "  %08x: %04x %04x %08x\n",
		       (uint) bdp,
		       bdp->cbd_sc,
		       bdp->cbd_datlen,
		       (int) bdp->cbd_bufaddr);
		bdp++;
	}

	bdp = fep->rx_bd_base;
	printk(KERN_INFO " rx: %lu buffers\n",
			(unsigned long) RX_RING_SIZE);
	for (i = 0 ; i < RX_RING_SIZE; i++) {
		printk(KERN_INFO "  %08x: %04x %04x %08x\n",
		       (uint) bdp,
		       bdp->cbd_sc,
		       bdp->cbd_datlen,
		       (int) bdp->cbd_bufaddr);
		bdp++;
	}
	}
#endif
	fec_restart(dev, fep->full_duplex);
	netif_wake_queue(dev);
}

/* The interrupt handler.
 * This is called from the MPC core interrupt.
 */
static irqreturn_t
fec_enet_interrupt(int irq, void *dev_id)
{
	struct	net_device *dev = dev_id;
	volatile fec_t	*fecp;
	uint	int_events;
	irqreturn_t ret = IRQ_NONE;
#ifdef CONFIG_FEC_1588
	struct fec_enet_private *fep = netdev_priv(dev);
	struct fec_ptp_private *fpp = fep->ptp_priv;
#endif

	fecp = (volatile fec_t *)dev->base_addr;

	/* Get the interrupt events that caused us to be here.
	*/
	do {
		int_events = fecp->fec_ievent;
		fecp->fec_ievent = int_events;
		/* Handle receive event in its own function.
		 */
		if (int_events & FEC_ENET_RXF) {
			ret = IRQ_HANDLED;
			fec_enet_rx(dev);
		}

		/* Transmit OK, or non-fatal error. Update the buffer
		   descriptors. FEC handles all errors, we just discover
		   them as part of the transmit process.
		*/
		if (int_events & FEC_ENET_TXF) {
			ret = IRQ_HANDLED;
			fec_enet_tx(dev);
		}

#ifdef CONFIG_FEC_1588
		if (int_events & FEC_ENET_TS_AVAIL) {
			ret = IRQ_HANDLED;
			fec_ptp_store_txstamp(fep->ptp_priv);
		}

		if (int_events & FEC_ENET_TS_TIMER) {
			ret = IRQ_HANDLED;
			if (fep->ptimer_present)
				fpp->prtc++;
		}

#endif
		if (int_events & FEC_ENET_MII) {
			ret = IRQ_HANDLED;
			fec_enet_mii(dev);
		}

	} while (int_events);

	return ret;
}


static void
fec_enet_tx(struct net_device *dev)
{
	struct	fec_enet_private *fep;
	volatile cbd_t	*bdp;
	unsigned short status;
	struct	sk_buff	*skb;

	fep = netdev_priv(dev);
	spin_lock_irq(&fep->hw_lock);
	bdp = fep->dirty_tx;

	while (((status = bdp->cbd_sc) & BD_ENET_TX_READY) == 0) {
		if (bdp == fep->cur_tx && fep->tx_full == 0)
			break;

		skb = fep->tx_skbuff[fep->skb_dirty];
		/* Check for errors. */
		if (status & (BD_ENET_TX_HB | BD_ENET_TX_LC |
				   BD_ENET_TX_RL | BD_ENET_TX_UN |
				   BD_ENET_TX_CSL)) {
			dev->stats.tx_errors++;
			if (status & BD_ENET_TX_HB)  /* No heartbeat */
				dev->stats.tx_heartbeat_errors++;
			if (status & BD_ENET_TX_LC)  /* Late collision */
				dev->stats.tx_window_errors++;
			if (status & BD_ENET_TX_RL)  /* Retrans limit */
				dev->stats.tx_aborted_errors++;
			if (status & BD_ENET_TX_UN)  /* Underrun */
				dev->stats.tx_fifo_errors++;
			if (status & BD_ENET_TX_CSL) /* Carrier lost */
				dev->stats.tx_carrier_errors++;
		} else {
			dev->stats.tx_packets++;
		}

#ifndef final_version
		if (status & BD_ENET_TX_READY)
			printk(KERN_ERR "HEY! "
				"Enet xmit interrupt and TX_READY.\n");
#endif
		/* Deferred means some collisions occurred during transmit,
		 * but we eventually sent the packet OK.
		 */
		if (status & BD_ENET_TX_DEF)
			dev->stats.collisions++;

		/* Free the sk buffer associated with this last transmit.
		 */
		dev_kfree_skb_any(skb);
		fep->tx_skbuff[fep->skb_dirty] = NULL;
		fep->skb_dirty = (fep->skb_dirty + 1) & TX_RING_MOD_MASK;

		/* Update pointer to next buffer descriptor to be transmitted.
		 */
		if (status & BD_ENET_TX_WRAP)
			bdp = fep->tx_bd_base;
		else
			bdp++;

		/* Since we have freed up a buffer, the ring is no longer
		 * full.
		 */
		if (fep->tx_full) {
			fep->tx_full = 0;
			if (netif_queue_stopped(dev))
				netif_wake_queue(dev);
		}
	}
	fep->dirty_tx = (cbd_t *)bdp;
	spin_unlock_irq(&fep->hw_lock);
}


/* During a receive, the cur_rx points to the current incoming buffer.
 * When we update through the ring, if the next incoming buffer has
 * not been given to the system, we just set the empty indicator,
 * effectively tossing the packet.
 */
static void
fec_enet_rx(struct net_device *dev)
{
	struct	fec_enet_private *fep;
#ifdef CONFIG_FEC_1588
	struct fec_ptp_private *fpp;
#endif
	volatile fec_t	*fecp;
	volatile cbd_t *bdp;
	unsigned short status;
	struct	sk_buff	*skb;
	ushort	pkt_len;
	__u8 *data;

#ifdef CONFIG_M532x
	flush_cache_all();
#endif

	fep = netdev_priv(dev);
#ifdef CONFIG_FEC_1588
	fpp = fep->ptp_priv;
#endif
	fecp = (volatile fec_t *)dev->base_addr;

	spin_lock_irq(&fep->hw_lock);

	/* First, grab all of the stats for the incoming packet.
	 * These get messed up if we get called due to a busy condition.
	 */
	bdp = fep->cur_rx;

while (!((status = bdp->cbd_sc) & BD_ENET_RX_EMPTY)) {

#ifndef final_version
	/* Since we have allocated space to hold a complete frame,
	 * the last indicator should be set.
	 */
	if ((status & BD_ENET_RX_LAST) == 0)
		printk(KERN_INFO "FEC ENET: rcv is not +last\n");
#endif

	if (!fep->opened)
		goto rx_processing_done;

	/* Check for errors. */
	if (status & (BD_ENET_RX_LG | BD_ENET_RX_SH | BD_ENET_RX_NO |
			   BD_ENET_RX_CR | BD_ENET_RX_OV)) {
		dev->stats.rx_errors++;
		if (status & (BD_ENET_RX_LG | BD_ENET_RX_SH)) {
			/* Frame too long or too short. */
			dev->stats.rx_length_errors++;
		}
		if (status & BD_ENET_RX_NO)	/* Frame alignment */
			dev->stats.rx_frame_errors++;
		if (status & BD_ENET_RX_CR)	/* CRC Error */
			dev->stats.rx_crc_errors++;
		if (status & BD_ENET_RX_OV)	/* FIFO overrun */
			dev->stats.rx_fifo_errors++;
	}

	/* Report late collisions as a frame error.
	 * On this error, the BD is closed, but we don't know what we
	 * have in the buffer.  So, just drop this frame on the floor.
	 */
	if (status & BD_ENET_RX_CL) {
		dev->stats.rx_errors++;
		dev->stats.rx_frame_errors++;
		goto rx_processing_done;
	}

	/* Process the incoming frame.
	 */
	dev->stats.rx_packets++;
	pkt_len = bdp->cbd_datlen;
	dev->stats.rx_bytes += pkt_len;
	data = (__u8 *)__va(bdp->cbd_bufaddr);

	/* This does 16 byte alignment, exactly what we need.
	 * The packet length includes FCS, but we don't want to
	 * include that when passing upstream as it messes up
	 * bridging applications.
	 */
	skb = dev_alloc_skb(pkt_len-4);

	if (skb == NULL) {
		printk(KERN_ERR "%s: Memory squeeze, "
			"dropping packet.\n", dev->name);
		dev->stats.rx_dropped++;
	} else {
		skb_put(skb, pkt_len-4);	/* Make room */
		skb_copy_to_linear_data(skb, data, pkt_len-4);
		skb->protocol = eth_type_trans(skb, dev);
#ifdef CONFIG_FEC_1588
		/* 1588 messeage TS handle */
		if (fep->ptimer_present)
			fec_ptp_store_rxstamp(fpp, skb,
					(struct bufdesc *)bdp);
#endif
		netif_rx(skb);
	}
rx_processing_done:

	/* Clear the status flags for this buffer.
	*/
	status &= ~BD_ENET_RX_STATS;

	/* Mark the buffer empty.
	*/
	status |= BD_ENET_RX_EMPTY;
	bdp->cbd_sc = status;
#ifdef CONFIG_FEC_1588
	bdp->cbd_esc = BD_ENET_RX_INT;
	bdp->cbd_prot = 0;
	bdp->cbd_bdu = 0;
#endif

	/* Update BD pointer to next entry.
	*/
	if (status & BD_ENET_RX_WRAP)
		bdp = fep->rx_bd_base;
	else
		bdp++;

#if 1
	/* Doing this here will keep the FEC running while we process
	 * incoming frames.  On a heavily loaded network, we should be
	 * able to keep up at the expense of system resources.
	 */
	fecp->fec_r_des_active = 0;
#endif
   } /* while (!((status = bdp->cbd_sc) & BD_ENET_RX_EMPTY)) */
	fep->cur_rx = (cbd_t *)bdp;

#if 0
	/* Doing this here will allow us to process all frames in the
	 * ring before the FEC is allowed to put more there.  On a heavily
	 * loaded network, some frames may be lost.  Unfortunately, this
	 * increases the interrupt overhead since we can potentially work
	 * our way back to the interrupt return only to come right back
	 * here.
	 */
	fecp->fec_r_des_active = 0;
#endif

	spin_unlock_irq(&fep->hw_lock);
}


/* called from interrupt context */
static void
fec_enet_mii(struct net_device *dev)
{
	struct	fec_enet_private *fep;
	volatile fec_t	*ep;

	fep = netdev_priv(dev);
	spin_lock_irq(&fep->mii_lock);

	ep = fep->phy_hwp; /*FIXME Jason*/

	spin_unlock_irq(&fep->mii_lock);
}

static int fec_mdio_transfer(struct mii_bus *bus, int phy_id,
	int reg, int regval)
{
	struct net_device *dev = bus->priv;
	unsigned long   flags;
	struct fec_enet_private *fep;
	int tries = 100;
	int retval = 0;

	fep = netdev_priv(dev);
	spin_lock_irqsave(&fep->mii_lock, flags);

	regval |= phy_id << 23;
	fep->hwp->fec_mii_data = regval;

	/* wait for it to finish, this takes about 23 us on lite5200b */
	while (!(fep->hwp->fec_ievent & FEC_ENET_MII) && --tries)
		udelay(5);

	if (!tries) {
		printk(KERN_ERR "%s timeout\n", __func__);
			return -ETIMEDOUT;
	}

	fep->hwp->fec_ievent = FEC_ENET_MII;
	retval = (fep->hwp->fec_mii_data);
	spin_unlock_irqrestore(&fep->mii_lock, flags);

	return retval;
}

static int coldfire_fec_mdio_read(struct mii_bus *bus,
	int phy_id, int reg)
{
	int ret;

	ret = fec_mdio_transfer(bus, phy_id, reg,
		mk_mii_read(reg));
	return ret;
}

static int coldfire_fec_mdio_write(struct mii_bus *bus,
	int phy_id, int reg, u16 data)
{
	return fec_mdio_transfer(bus, phy_id, reg,
		mk_mii_write(reg, data));
}

static void fec_adjust_link(struct net_device *dev)
{
	struct fec_enet_private *priv = netdev_priv(dev);
	struct phy_device *phydev = priv->phydev;
	int new_state = 0;

	if (phydev->link != PHY_DOWN) {
		if (phydev->duplex != priv->duplex) {
			new_state = 1;
			priv->duplex = phydev->duplex;
		}

		if (phydev->speed != priv->speed) {
			new_state = 1;
			priv->speed = phydev->speed;
		}

		if (priv->old_link == PHY_DOWN) {
			new_state = 1;
			priv->old_link = phydev->link;
		}

		if (new_state)
			fec_restart(dev, phydev->duplex);
	} else if (priv->old_link) {
		new_state = 1;
		priv->old_link = PHY_DOWN;
		priv->speed = 0;
		priv->duplex = -1;
		fec_stop(dev);
	}

	if (new_state)
		phy_print_status(phydev);
}

static int coldfire_fec_init_phy(struct net_device *dev)
{
	struct fec_enet_private *priv = netdev_priv(dev);
	struct phy_device *phydev = NULL;
	int i, startnode = 0;

	/* search for connect PHY device */
	for (i = 0; i < PHY_MAX_ADDR; i++) {
		struct phy_device *const tmp_phydev =
			priv->mdio_bus->phy_map[i];

		if (!tmp_phydev) {
#ifdef FEC_DEBUG
			printk(KERN_INFO "%s no PHY here at"
				"mii_bus->phy_map[%d]\n",
				__func__, i);
#endif
			continue; /* no PHY here... */
		}

#ifdef CONFIG_FEC_SHARED_PHY
		if (priv->index == 0)
			phydev = tmp_phydev;
		else if (priv->index == 1) {
			if (startnode == 1) {
				phydev = tmp_phydev;
				startnode = 0;
			} else {
				startnode++;
				continue;
			}
		} else
			printk(KERN_INFO "%s now we do not"
				"support (%d) more than"
				"2 phys shared "
				"one mdio bus\n",
				__func__, startnode);
#else
		phydev = tmp_phydev;
#endif
#ifdef FEC_DEBUG
		printk(KERN_INFO "%s find PHY here at"
			"mii_bus->phy_map[%d]\n",
			__func__, i);
#endif
		break; /* found it */
	}

	/* now we are supposed to have a proper phydev, to attach to... */
	if (!phydev) {
		printk(KERN_INFO "%s: Don't found any phy device at all\n",
			dev->name);
		return -ENODEV;
	}

	priv->link = PHY_DOWN;
	priv->old_link = PHY_DOWN;
	priv->speed = 0;
	priv->duplex = -1;
#ifdef FEC_DEBUG
	printk(KERN_INFO "%s phydev_busid %s\n", __func__, phydev->dev.bus_id);
#endif
	phydev = phy_connect(dev, phydev->dev.bus_id,
		&fec_adjust_link, 0, PHY_INTERFACE_MODE_MII);
	if (IS_ERR(phydev)) {
		printk(KERN_ERR " %s phy_connect failed\n", __func__);
		return PTR_ERR(phydev);
	}

	printk(KERN_INFO "attached phy %i to driver %s\n",
		phydev->addr, phydev->drv->name);

	priv->phydev = phydev;

	return 0;
}
/* -----------------------------------------------------------------------*/
static int
fec_enet_open(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	/* I should reset the ring buffers here, but I don't yet know
	 * a simple way to do that.
	 */
	fec_set_mac_address(dev);

	fep->link = 0;
	coldfire_fec_init_phy(dev);
	phy_write(fep->phydev, MII_BMCR, BMCR_RESET);
	phy_start(fep->phydev);
	fep->old_link = 0;
	if (fep->phydev) {
		/* Set the initial link state to true. A lot of hardware
		 * based on this device does not implement a PHY interrupt,
		 * so we are never notified of link change.
		 */
		fep->link = 1;
	} else {
		/* no phy,  go full duplex,  it's most likely a hub chip */
		fec_restart(dev, 1);
	}

	/* if the fec is the fist open, we need to do nothing*/
	/* if the fec is not the fist open, we need to restart the FEC*/
	if (fep->sequence_done == 0)
		fec_restart(dev, 1);
	else
		fep->sequence_done = 0;

	netif_start_queue(dev);
	fep->opened = 1;
	return 0;		/* Success */
}

static int
fec_enet_close(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	/* Don't know what to do yet.
	*/
	fep->opened = 0;
	netif_stop_queue(dev);
	fec_stop(dev);
	phy_disconnect(fep->phydev);
	phy_stop(fep->phydev);
	phy_write(fep->phydev, MII_BMCR, BMCR_PDOWN);
	return 0;
}

/* Set or clear the multicast filter for this adaptor.
 * Skeleton taken from sunlance driver.
 * The CPM Ethernet implementation allows Multicast as well as individual
 * MAC address filtering.  Some of the drivers check to make sure it is
 * a group multicast address, and discard those that are not.  I guess I
 * will do the same for now, but just remove the test if you want
 * individual filtering as well (do the upper net layers want or support
 * this kind of feature?).
 */

#define HASH_BITS	6		/* #bits in hash */
#define CRC32_POLY	0xEDB88320

static void set_multicast_list(struct net_device *dev)
{
	struct fec_enet_private *fep;
	volatile fec_t *ep;
	struct dev_mc_list *dmi;
	unsigned int i, j, bit, data, crc;
	unsigned char hash;

	fep = netdev_priv(dev);
	ep = fep->hwp;

	if (dev->flags&IFF_PROMISC) {
		ep->fec_r_cntrl |= 0x0008;
		printk(KERN_INFO "%s IFF_PROMISC\n", __func__);
	} else {

		ep->fec_r_cntrl &= ~0x0008;

		if (dev->flags & IFF_ALLMULTI) {
			/* Catch all multicast addresses, so set the
			 * filter to all 1's.
			 */
			ep->fec_grp_hash_table_high = 0xffffffff;
			ep->fec_grp_hash_table_low = 0xffffffff;
		} else {
			/* Clear filter and add the addresses in hash register.
			*/
			ep->fec_grp_hash_table_high = 0;
			ep->fec_grp_hash_table_low = 0;

			dmi = dev->mc_list;

			for (j = 0; j < dev->mc_count;
				j++, dmi = dmi->next) {
				/* Only support group multicast for now.
				*/
				if (!(dmi->dmi_addr[0] & 1))
					continue;

				/* calculate crc32 value of mac address
				*/
				crc = 0xffffffff;

				for (i = 0; i < dmi->dmi_addrlen; i++) {
					data = dmi->dmi_addr[i];
					for (bit = 0; bit < 8; bit++,
						data >>= 1) {
						crc = (crc >> 1) ^
						(((crc ^ data) & 1) ?
						CRC32_POLY : 0);
					}
				}

				/* only upper 6 bits (HASH_BITS) are used
				*  which point to specific bit in the
				*  hash registers
				*/
				hash = (crc >> (32 - HASH_BITS)) & 0x3f;

				if (hash > 31)
					ep->fec_grp_hash_table_high |=
						1 << (hash - 32);
				else
					ep->fec_grp_hash_table_low |= 1 << hash;
			}
		}
	}
}

/* Set a MAC change in hardware.
 */
static void
fec_set_mac_address(struct net_device *dev)
{
	volatile fec_t *fecp;

	fecp = ((struct fec_enet_private *)netdev_priv(dev))->hwp;

	/* Set station address. */
	fecp->fec_addr_low = dev->dev_addr[3] | (dev->dev_addr[2] << 8) |
		(dev->dev_addr[1] << 16) | (dev->dev_addr[0] << 24);
	fecp->fec_addr_high = (dev->dev_addr[5] << 16) |
		(dev->dev_addr[4] << 24);

}

/* Initialize the FEC Ethernet.
 */
 /*
  * XXX:  We need to clean up on failure exits here.
  */
int __init fec_enet_init(struct net_device *dev,
	int slot, struct platform_device *pdev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	unsigned long	mem_addr;
	volatile cbd_t	*bdp;
	cbd_t		*cbd_base;
	volatile fec_t	*fecp;
	int 		i, j;
	struct coldfire_fec_platform_data *plat =
		pdev->dev.platform_data;

	/* Only allow us to be probed once. */
	if (slot >= FEC_MAX_PORTS)
		return -ENXIO;

	/* Allocate memory for buffer descriptors.
	*/
	mem_addr = __get_free_page(GFP_DMA);
	if (mem_addr == 0) {
		printk(KERN_ERR "FEC: allocate descriptor memory failed?\n");
		return -ENOMEM;
	}

	spin_lock_init(&fep->hw_lock);
	spin_lock_init(&fep->mii_lock);

	/* Create an Ethernet device instance.
	*/
	fecp = (volatile fec_t *)plat->fec_hw[slot];

	fep->index = slot;
	fep->hwp = fecp;
	fep->netdev = dev;
#ifdef CONFIG_FEC_SHARED_PHY
	fep->phy_hwp = (volatile fec_t *) plat->fec_hw[slot & ~1];
#else
	fep->phy_hwp = fecp;
#endif

	/* Set the Ethernet address.  If using multiple Enets on the 8xx,
	 * this needs some work to get unique addresses.
	 *
	 * This is our default MAC address unless the user changes
	 * it via eth_mac_addr (our dev->set_mac_addr handler).
	 */
	if (plat && plat->get_mac)
		plat->get_mac(dev);

	/* Whack a reset.  We should wait for this.
	*/
	fecp->fec_ecntrl = 1;
	udelay(10);


	cbd_base = (cbd_t *)mem_addr;
	/* XXX: missing check for allocation failure */
	if (plat && plat->uncache)
		plat->uncache(mem_addr);

	/* Set receive and transmit descriptor base.
	*/
	fep->rx_bd_base = cbd_base;
	fep->tx_bd_base = cbd_base + RX_RING_SIZE;

	fep->dirty_tx = fep->cur_tx = fep->tx_bd_base;
	fep->cur_rx = fep->rx_bd_base;

	fep->skb_cur = fep->skb_dirty = 0;

	/* Initialize the receive buffer descriptors.
	*/
	bdp = fep->rx_bd_base;
	for (i = 0; i < FEC_ENET_RX_PAGES; i++) {

		/* Allocate a page.
		*/
		mem_addr = __get_free_page(GFP_DMA);
		/* XXX: missing check for allocation failure */
		if (plat && plat->uncache)
			plat->uncache(mem_addr);

		/* Initialize the BD for every fragment in the page.
		*/
		for (j = 0; j < FEC_ENET_RX_FRPPG; j++) {
			bdp->cbd_sc = BD_ENET_RX_EMPTY;
#ifdef CONFIG_FEC_1588
			bdp->cbd_esc = BD_ENET_RX_INT;
#endif
			bdp->cbd_bufaddr = __pa(mem_addr);
			mem_addr += FEC_ENET_RX_FRSIZE;
			bdp++;
		}
	}

	/* Set the last buffer to wrap.
	*/
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	/* ...and the same for transmmit.
	*/
	bdp = fep->tx_bd_base;
	for (i = 0, j = FEC_ENET_TX_FRPPG; i < TX_RING_SIZE; i++) {
		if (j >= FEC_ENET_TX_FRPPG) {
			mem_addr = __get_free_page(GFP_DMA);
			j = 1;
		} else {
			mem_addr += FEC_ENET_TX_FRSIZE;
			j++;
		}
		fep->tx_bounce[i] = (unsigned char *) mem_addr;

		/* Initialize the BD for every fragment in the page.
		*/
		bdp->cbd_sc = 0;
#ifdef CONFIG_FEC_1588
		bdp->cbd_esc = BD_ENET_RX_INT;
#endif
		bdp->cbd_bufaddr = 0;
		bdp++;
	}

	/* Set the last buffer to wrap.
	*/
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	/* Set receive and transmit descriptor base.
	*/
	fecp->fec_r_des_start = __pa((uint)(fep->rx_bd_base));
	fecp->fec_x_des_start = __pa((uint)(fep->tx_bd_base));

	/* Install our interrupt handlers. This varies depending on
	 * the architecture.
	*/
	if (plat && plat->request_intrs)
		plat->request_intrs(dev, fec_enet_interrupt, dev);

	fecp->fec_grp_hash_table_high = 0;
	fecp->fec_grp_hash_table_low = 0;
	fecp->fec_r_buff_size = PKT_MAXBLR_SIZE;
	fecp->fec_ecntrl = 2;
	fecp->fec_r_des_active = 0;
	if (plat->hash_table == 0) {
		fecp->fec_hash_table_high = 0;
		fecp->fec_hash_table_low = 0;
	}

	dev->base_addr = (unsigned long)fecp;

	/* The FEC Ethernet specific entries in the device structure. */
	dev->open = fec_enet_open;
	dev->hard_start_xmit = fec_enet_start_xmit;
	dev->tx_timeout = fec_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
	dev->stop = fec_enet_close;
	dev->set_multicast_list = set_multicast_list;

	/* setup MII interface */
	if (plat && plat->set_mii)
		plat->set_mii(dev);

	/* Clear and enable interrupts */
	fecp->fec_ievent = 0xffc00000;
	fecp->fec_imask = (FEC_ENET_TXF | FEC_ENET_RXF);

	/* Queue up command to detect the PHY and initialize the
	 * remainder of the interface.
	 */
#ifndef CONFIG_FEC_SHARED_PHY
	fep->phy_addr = 0;
#else
	fep->phy_addr = fep->index;
#endif

	fep->sequence_done = 1;
	return 0;
}

/* This function is called to start or restart the FEC during a link
 * change.  This only happens when switching between half and full
 * duplex.
 */
static void
fec_restart(struct net_device *dev, int duplex)
{
	struct fec_enet_private *fep;
	volatile cbd_t *bdp;
	volatile fec_t *fecp;
	int i;
	struct coldfire_fec_platform_data *plat;

	fep = netdev_priv(dev);
	fecp = fep->hwp;
	plat = fep->pdev->dev.platform_data;
	/* Whack a reset.  We should wait for this.
	*/
	fecp->fec_ecntrl = 1;
	udelay(10);

	/* Clear any outstanding interrupt.
	*/
	fecp->fec_ievent = 0xffc00000;
	if (plat && plat->enable_phy_intr)
		plat->enable_phy_intr();

	/* Set station address.
	*/
	fec_set_mac_address(dev);

	/* Reset all multicast.
	*/
	fecp->fec_grp_hash_table_high = 0;
	fecp->fec_grp_hash_table_low = 0;

	/* Set maximum receive buffer size.
	*/
	fecp->fec_r_buff_size = PKT_MAXBLR_SIZE;

	if (plat && plat->localhw_setup)
		plat->localhw_setup();
	/* Set receive and transmit descriptor base.
	*/
	fecp->fec_r_des_start = __pa((uint)(fep->rx_bd_base));
	fecp->fec_x_des_start = __pa((uint)(fep->tx_bd_base));

	fep->dirty_tx = fep->cur_tx = fep->tx_bd_base;
	fep->cur_rx = fep->rx_bd_base;

	/* Reset SKB transmit buffers.
	*/
	fep->skb_cur = fep->skb_dirty = 0;
	for (i = 0; i <= TX_RING_MOD_MASK; i++) {
		if (fep->tx_skbuff[i] != NULL) {
			dev_kfree_skb_any(fep->tx_skbuff[i]);
			fep->tx_skbuff[i] = NULL;
		}
	}

	/* Initialize the receive buffer descriptors.
	*/
	bdp = fep->rx_bd_base;
	for (i = 0; i < RX_RING_SIZE; i++) {

		/* Initialize the BD for every fragment in the page.
		*/
		bdp->cbd_sc = BD_ENET_RX_EMPTY;
		bdp++;
	}

	/* Set the last buffer to wrap.
	*/
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	/* ...and the same for transmmit.
	*/
	bdp = fep->tx_bd_base;
	for (i = 0; i < TX_RING_SIZE; i++) {

		/* Initialize the BD for every fragment in the page.
		*/
		bdp->cbd_sc = 0;
		bdp->cbd_bufaddr = 0;
		bdp++;
	}

	/* Set the last buffer to wrap.
	*/
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	/* Enable MII mode.
	*/
	if (duplex) {
	#ifdef CONFIG_M5441X
		fecp->fec_r_cntrl = OPT_FRAME_SIZE | 0x104;/* MII enable */
	#else
		fecp->fec_r_cntrl = OPT_FRAME_SIZE | 0x04;/* MII enable */
	#endif
		fecp->fec_x_cntrl = 0x04;		  /* FD enable */
	} else {
		/* MII enable|No Rcv on Xmit */
	#ifdef CONFIG_M5441X
		fecp->fec_r_cntrl = OPT_FRAME_SIZE | 0x106;
	#else
		fecp->fec_r_cntrl = OPT_FRAME_SIZE | 0x06;
	#endif
		fecp->fec_x_cntrl = 0x00;
	}
	fep->full_duplex = duplex;

#if defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
	if (fep->speed == 10)
		fecp->fec_r_cntrl |= 0x0200;	/*RMII_10T*/
	else
		fecp->fec_r_cntrl &= ~0x0200;
#endif

	/* Set MII speed.
	*/
	fecp->fec_mii_speed = fep->phy_speed;
#ifdef CONFIG_FEC_1588
	if (fep->ptimer_present) {
		int ret;
		/* Set Timer count */
		ret = fec_ptp_start(fep->ptp_priv);
		if (ret)
			fep->ptimer_present = 0;
		else
			fecp->fec_ecntrl = 0x00000010;
	}
	fecp->fec_ecntrl |= 2;
#else
	/* And last, enable the transmit and receive processing.
	*/
	fecp->fec_ecntrl = 2;
#endif
	fecp->fec_r_des_active = 0;

	/* Enable interrupts we wish to service.
	*/
#ifdef CONFIG_FEC_1588
	fecp->fec_imask = (FEC_ENET_TXF | FEC_ENET_RXF |
			FEC_ENET_TS_TIMER | FEC_ENET_TS_AVAIL);
#else
	fecp->fec_imask = (FEC_ENET_TXF | FEC_ENET_RXF);
#endif
}

static void
fec_stop(struct net_device *dev)
{
	volatile fec_t *fecp;
	struct fec_enet_private *fep;
	struct coldfire_fec_platform_data *plat;

	fep = netdev_priv(dev);
	fecp = fep->hwp;
	plat = fep->pdev->dev.platform_data;
	/*
	** We cannot expect a graceful transmit stop without link !!!
	*/
	if (fep->link) {
		fecp->fec_x_cntrl = 0x01;	/* Graceful transmit stop */
		udelay(10);
		if (!(fecp->fec_ievent & FEC_ENET_GRA))
			printk(KERN_ERR "fec_stop : Graceful transmit"
				" stop did not complete !\n");
	}

	/* Whack a reset.  We should wait for this.
	*/
	fecp->fec_ecntrl = 1;
	udelay(10);

	/* Clear outstanding MII command interrupts.
	*/
	fecp->fec_ievent = FEC_ENET_MII;
	if (plat && plat->enable_phy_intr)
		plat->enable_phy_intr();

	fecp->fec_imask = 0;
	fecp->fec_mii_speed = fep->phy_speed;
#ifdef CONFIG_FEC_1588
	if (fep->ptimer_present)
		fec_ptp_stop(fep->ptp_priv);
#endif
}

static int fec_mdio_register(struct net_device *dev,
	int slot)
{
	int err = 0;
	struct fec_enet_private *fep = netdev_priv(dev);

	fep->mdio_bus = mdiobus_alloc();
	if (!fep->mdio_bus) {
		printk(KERN_ERR "ethernet mdiobus_alloc fail\n");
		return -ENOMEM;
	}

	if (slot == 0) {
		fep->mdio_bus->name = "Coldfire FEC MII 0 Bus";
		strcpy(fep->mdio_bus->id, "0");
	} else if (slot == 1) {
		fep->mdio_bus->name = "Coldfire FEC MII 1 Bus";
		strcpy(fep->mdio_bus->id, "1");
	} else {
		printk(KERN_ERR "Now coldfire can not"
			"support more than 2 mii bus\n");
	}

	fep->mdio_bus->read = &coldfire_fec_mdio_read;
	fep->mdio_bus->write = &coldfire_fec_mdio_write;
	fep->mdio_bus->priv = dev;
	err = mdiobus_register(fep->mdio_bus);
	if (err) {
		mdiobus_free(fep->mdio_bus);
		printk(KERN_ERR "%s: ethernet mdiobus_register fail\n",
			dev->name);
		return -EIO;
	}

	printk(KERN_INFO "mdiobus_register %s ok\n",
		fep->mdio_bus->name);
	return err;
}

static int __init fec_eth_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	int i, err;
	struct fec_enet_private *fep;

	struct fec_platform_private *chip;

	printk(KERN_INFO "FEC ENET Version 0.2\n");
	chip = kzalloc(sizeof(struct fec_platform_private) +
		sizeof(struct fec_enet_private *) * FEC_MAX_PORTS, GFP_KERNEL);
	if (!chip) {
		err = -ENOMEM;
		printk(KERN_ERR "%s: kzalloc fail %x\n", __func__,
			(unsigned int)chip);
		return err;
	}

	chip->pdev = pdev;
	chip->num_slots = FEC_MAX_PORTS;
	platform_set_drvdata(pdev, chip);

	for (i = 0; (i < chip->num_slots); i++) {
		dev = alloc_etherdev(sizeof(struct fec_enet_private));
		if (!dev) {
			printk(KERN_ERR "%s: ethernet alloc_etherdev fail\n",
				dev->name);
			return -ENOMEM;
		}

		fep = netdev_priv(dev);
		fep->pdev = pdev;

		err = fec_enet_init(dev, i, pdev);
		if (err) {
			free_netdev(dev);
			platform_set_drvdata(pdev, NULL);
			kfree(chip);
			continue;
		}

		chip->fep_host[i] = fep;
#ifdef CONFIG_FEC_SHARED_PHY
		if (i == 0)
			err = fec_mdio_register(dev, 0);
		else {
			fep->mdio_bus = chip->fep_host[0]->mdio_bus;
			printk(KERN_INFO "FEC%d SHARED the %s ok\n",
				i, fep->mdio_bus->name);
		}
#else
		err = fec_mdio_register(dev, i);
#endif
		if (err) {
			printk(KERN_ERR "%s: ethernet fec_mdio_register\n",
				dev->name);
			free_netdev(dev);
			platform_set_drvdata(pdev, NULL);
			kfree(chip);
			return -ENOMEM;
		}

#ifdef CONFIG_FEC_1588
		fep->ptp_priv = kzalloc(sizeof(struct fec_ptp_private),
					GFP_KERNEL);
		if (fep->ptp_priv) {
			fep->ptp_priv->hwp = fep->hwp;
			err = fec_ptp_init(fep->ptp_priv, i);
			if (err)
				printk(KERN_ERR "IEEE1588: "
					"ptp-timer init failed\n");
			else
				fep->ptimer_present = 1;
		} else
			printk(KERN_ERR "IEEE1588: failed to "
					"malloc memory\n");
#endif

		if (register_netdev(dev) != 0) {
			/* XXX: missing cleanup here */
#ifdef CONFIG_FEC_1588
			if (fep->ptimer_present)
				fec_ptp_cleanup(fep->ptp_priv);

			kfree(fep->ptp_priv);
#endif
			free_netdev(dev);
			platform_set_drvdata(pdev, NULL);
			kfree(chip);
			printk(KERN_ERR "%s: ethernet register_netdev fail\n",
				dev->name);
			return -EIO;
		}

		printk(KERN_INFO "%s: ethernet %pM\n",
				dev->name, dev->dev_addr);
	}

	return 0;
}

static int fec_eth_remove(struct platform_device *pdev)
{
	int i;
	struct net_device *dev;
	struct fec_enet_private *fep;
	struct fec_platform_private *chip;

	chip = platform_get_drvdata(pdev);
	if (chip) {
		for (i = 0; i < chip->num_slots; i++) {
			fep = chip->fep_host[i];
			dev = fep->netdev;
			fep->sequence_done = 1;
#ifdef CONFIG_FEC_1588
			if (fep->ptimer_present)
				fec_ptp_cleanup(fep->ptp_priv);

			kfree(fep->ptp_priv);
#endif
			unregister_netdev(dev);
			free_netdev(dev);
		}

		platform_set_drvdata(pdev, NULL);
		kfree(chip);
	} else
		printk(KERN_ERR "%s: can not get the "
			"fec_platform_private %x\n", __func__,
			(unsigned int)chip);

	return 0;
}

static struct platform_driver fec_eth_driver = {
	.probe          = fec_eth_probe,
	.remove         = fec_eth_remove,
	.driver         = {
		.name   = "coldfire-fec",
		.owner  = THIS_MODULE,
	},
};

static int __init coldfire_fec_init(void)
{
	return platform_driver_register(&fec_eth_driver);;
}

static void __exit coldfire_fec_exit(void)
{
	platform_driver_unregister(&fec_eth_driver);
}

module_init(coldfire_fec_init);
module_exit(coldfire_fec_exit);
MODULE_LICENSE("GPL");
