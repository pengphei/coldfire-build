/*
 * dspi_mcf.c - DSPI controller for ColdFire processors
 *
 * Copyright 2008-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 * Author: Matt Waddel Matt.Waddel@freescale.com
 * 	   Kurt Mahan kmahan@freescale.com
 *	   Wang Huan <b18965@freescale.com>
 *	   Jingchang Lu <b22599@freescale.com>
 *	   Lanttor.Guo@freescale.com
 *
 * Based on spi_coldfire.c
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
 *
 ***************************************************************************
 * Changes:
 *   v0.003	M5301x support.
 *   v0.002	M547x/M548x support.
 *   v0.001	Initial version. Coldfire DSPI master driver.
 ****************************************************************************/

/*
 * Includes
 */

#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <asm/mcfsim.h>
#include <asm/mcfqspi.h>
#include <asm/coldfire.h>
#include <linux/io.h>
#include <asm/mcfdspi.h>
#include <linux/dma-mapping.h>

#include <linux/time.h>
#undef DSPI_COLDFIRE_DEBUG

#ifdef DSPI_COLDFIRE_DEBUG
#define DBG(fmt, args...) \
	printk(KERN_INFO "[%s]  " fmt , __func__, ## args)
#else
#define DBG(fmt, args...)	do {} while (0)
#endif

#if defined(CONFIG_M5445X)
#include <asm/mcf5445x_dspi.h>
#if defined(CONFIG_SPI_COLDFIRE_DSPI_EDMA)
	#include <asm/mcf5445x_edma.h>
#endif
#elif defined(CONFIG_M5441X)
#include <asm/mcf5441x_dspi.h>
#if defined(CONFIG_SPI_COLDFIRE_DSPI_EDMA)
#include <asm/mcf5441x_edma.h>
#endif
#endif

#if defined(CONFIG_M547X_8X)
#include <asm/virtconvert.h>
#include <asm/m5485dspi.h>
#endif

#if defined(CONFIG_SPI_COLDFIRE_DSPI_EDMA)
#include <asm/mcf_edma.h>
#define SPI_DSPI_EDMA
#define EDMA_BUFSIZE_KMALLOC	(DSPI_FIFO_SIZE*4)
#if defined(CONFIG_M5445X)
#define DSPI_DMA_RX_TCD		MCF_EDMA_CHAN_DSPI_RX
#define DSPI_DMA_TX_TCD 	MCF_EDMA_CHAN_DSPI_TX
#elif defined(CONFIG_M5441X)
#if defined(CONFIG_DSPI0)
#define DSPI_DMA_RX_TCD		MCF_EDMA_CHAN_DSPI0_RX
#define DSPI_DMA_TX_TCD 	MCF_EDMA_CHAN_DSPI0_TX
#elif defined(CONFIG_DSPI1)
#define DSPI_DMA_RX_TCD		MCF_EDMA_CHAN_DSPI1_RX
#define DSPI_DMA_TX_TCD 	MCF_EDMA_CHAN_DSPI1_TX
#endif
#endif

#undef NEW_DMA_METHOD

#endif

#define DSPI_BITS	MCF_DSPI_DCTAR_FMSZ(0xF)
#define DSPI_BITS_16	MCF_DSPI_DCTAR_FMSZ(0xF)
#define DSPI_BITS_8	MCF_DSPI_DCTAR_FMSZ(0x7)

#ifdef NEW_DMA_METHOD
#define DSPI_FIFO_SIZE  (16)
#else
#define DSPI_FIFO_SIZE	16
#endif

#define DRIVER_NAME "Coldfire DSPI"

/****************************************************************************/

/*
 * Local constants and macros
 */

#define START_STATE ((void *)0)
#define RUNNING_STATE ((void *)1)
#define DONE_STATE ((void *)2)
#define ERROR_STATE ((void *)-1)

#define QUEUE_RUNNING 0
#define QUEUE_STOPPED 1
#define TRAN_STATE_RX_VOID	   0x01
#define TRAN_STATE_TX_VOID 	   0x02
#define TRAN_STATE_WORD_ODD_NUM	   0x04
/****************************************************************************/

/*
 * Local Data Structures
 */

struct DSPI_MCR {
	unsigned master:1;
	unsigned cont_scke:1;
	unsigned dconf:2;
	unsigned frz:1;
	unsigned mtfe:1;
	unsigned pcsse:1;
	unsigned rooe:1;
	unsigned pcsis:8;
	unsigned reserved15:1;
	unsigned mdis:1;
	unsigned dis_tx:1;
	unsigned dis_rxf:1;
	unsigned clr_tx:1;
	unsigned clr_rxf:1;
	unsigned smpl_pt:2;
	unsigned reserved71:7;
	unsigned halt:1;
};

struct DSPI_CTAR {
	unsigned dbr:1;
	unsigned fmsz:4;
	unsigned cpol:1;
	unsigned cpha:1;
	unsigned lsbfe:1;
	unsigned pcssck:2;
	unsigned pasc:2;
	unsigned pdt:2;
	unsigned pbr:2;
	unsigned cssck:4;
	unsigned asc:4;
	unsigned dt:4;
	unsigned br:4;
};

struct chip_data {
	/* dspi data */
	union {
		u32 mcr_val;
		struct DSPI_MCR mcr;
	};
	union {
		u32 ctar_val;
		struct DSPI_CTAR ctar;
	};
	u16 void_write_data;
};


struct driver_data {
	/* Driver model hookup */
	struct platform_device *pdev;

	/* SPI framework hookup */
	struct spi_master *master;

	/* Driver message queue */
	struct workqueue_struct	*workqueue;
	struct work_struct pump_messages;
	spinlock_t lock; /* lock */
	struct list_head queue;
	int busy;
	int run;

	/* Message Transfer pump */
	struct tasklet_struct pump_transfers;

	/* Current message transfer state info */
	struct spi_message *cur_msg;
	struct spi_transfer *cur_transfer;
	struct chip_data *cur_chip;
	size_t len;
	void *tx;
	void *tx_end;
	void *rx;
	void *rx_end;
	char flags;
	u8 cs;
	u16 void_write_data;
	unsigned cs_change:1;

	u32 trans_cnt;
	u32 wce_cnt;
	u32 abrt_cnt;
	volatile u32 *mcr;		/* DSPI MCR register */
	volatile u32 *ctar;		/* DSPI CTAR register */
	volatile u32 *dspi_dtfr;		/* DSPI DTFR register */
	volatile u32 *dspi_drfr;		/* DSPI DRFR register */
	volatile u32 *dspi_rser;		/* DSPI RSER register */
	volatile u32 *dspi_sr;		/* DSPI status register */

#if defined(SPI_DSPI_EDMA)
	volatile void *edma_tx_buf;
	volatile void *edma_rx_buf;
	dma_addr_t edma_tx_buf_pa;
	dma_addr_t edma_rx_buf_pa;
#endif

#if defined(CONFIG_M5301x)
	u8 *parh;
	u8 *parl;
#else
	u8  *par;	   /* Pin assignment register */
#endif
	u8  *int_icr;	   /* Interrupt level and priority register */
	u32 *int_mr;       /* Interrupt mask register */
	void (*cs_control)(u8 cs, u8 command);
};

#define DSPI_CS(cs) ((1<<(cs))<<16)

/****************************************************************************/

/*
 * SPI local functions
 */
static unsigned int first_in_transfer = 1;

static void *next_transfer(struct driver_data *drv_data)
{
	struct spi_message *msg = drv_data->cur_msg;
	struct spi_transfer *trans = drv_data->cur_transfer;

	DBG("\n");
	/* Move to next transfer */
	if (trans->transfer_list.next != &msg->transfers) {
		drv_data->cur_transfer = list_entry(trans->transfer_list.next,
						    struct spi_transfer,
						    transfer_list);

		if (drv_data->cur_transfer->transfer_list.next
			== &msg->transfers) /* last transfer */
			drv_data->cur_transfer->cs_change = 1;

		return RUNNING_STATE;
	} else
		return DONE_STATE;
}


static inline int is_word_transfer(struct driver_data *drv_data)
{
	return ((*(volatile u32 *)(drv_data->ctar+drv_data->cs) & DSPI_BITS_16)
				== DSPI_BITS_8) ? 0 : 1;
}

static inline void set_8bit_transfer_mode(struct driver_data *drv_data)
{
	DBG("\n");
	*(volatile u32 *)(drv_data->ctar+drv_data->cs) =
	((*(volatile u32 *)(drv_data->ctar + drv_data->cs)) & ~DSPI_BITS)
				| DSPI_BITS_8;
}

static inline void set_16bit_transfer_mode(struct driver_data *drv_data)
{
	DBG("\n");
	(*(volatile u32 *)(drv_data->ctar+drv_data->cs)) =
	((*(volatile u32 *)(drv_data->ctar + drv_data->cs)) & ~DSPI_BITS)
				| DSPI_BITS_16;
}

static unsigned char hz_to_spi_baud(int pbr, int dbr, int speed_hz)
{
	int pbr_tbl[4] = {2, 3, 5, 7}; /* Valid baud rate pre-scaler values */
	int brs[16] = {	2,	4,	6,	8,
			16,	32,	64,	128,
			256,	512,	1024,	2048,
			4096,	8192,	16384,	32768 };
	int temp, index = 0;

	if ((pbr < 0) || (pbr > 3) ||
	    (dbr < 0) || (dbr > 1))
		return 15; /* table indexes out of range, go slow */

	temp = ((((MCF_CLK / 2) / pbr_tbl[pbr]) * (1 + dbr)) / speed_hz);

	while (temp > brs[index])
		if (index++ >= 15)
			break;

	DBG("baud rate scaler = 0x%x - %d\n", index, brs[index]);
	return index;
}

static int write(struct driver_data *drv_data)
{
	int tx_count = 0;
	int tx_word = is_word_transfer(drv_data);
	u16 d16;
	u8  d8;
	u32 dspi_pushr = 0;
	int first = 1;
#if defined(SPI_DSPI_EDMA)
	volatile u32 *edma_wr = (volatile u32 *)(drv_data->edma_tx_buf);
#endif

	/* If we are in word mode, but only have a single byte to transfer
	 * then switch to byte mode temporarily.  Will switch back at the
	 * end of the transfer. */
	if (tx_word && ((drv_data->tx_end - drv_data->tx) == 1)) {
		drv_data->flags |= TRAN_STATE_WORD_ODD_NUM;
		set_8bit_transfer_mode(drv_data);
		tx_word = 0;
	}
	while ((drv_data->tx < drv_data->tx_end)
			&& (tx_count < DSPI_FIFO_SIZE)) {
		if (tx_word) {
			if ((drv_data->tx_end - drv_data->tx) == 1)
				break;

			if (!(drv_data->flags & TRAN_STATE_TX_VOID))
				d16 = *(u16 *)drv_data->tx;
			else
				d16 = drv_data->void_write_data;

			dspi_pushr = MCF_DSPI_DTFR_TXDATA(d16) |
				     DSPI_CS(drv_data->cs) |
				     MCF_DSPI_DTFR_CTAS(drv_data->cs) |
				     MCF_DSPI_DTFR_CONT;

			drv_data->tx += 2;
		} else {
			if (!(drv_data->flags & TRAN_STATE_TX_VOID))
				d8 = *(u8 *)drv_data->tx;
			else
				d8 = (u8)drv_data->void_write_data;

			dspi_pushr = MCF_DSPI_DTFR_TXDATA(d8) |
				     DSPI_CS(drv_data->cs) |
				     MCF_DSPI_DTFR_CTAS(drv_data->cs) |
				     MCF_DSPI_DTFR_CONT;

			drv_data->tx++;
		}
#ifdef NEW_DMA_METHOD
		if ((drv_data->cs_change)
				&& (drv_data->tx == drv_data->tx_end))
			dspi_pushr &= ~MCF_DSPI_DTFR_CONT;
#else
		if (drv_data->tx == drv_data->tx_end
			|| tx_count == DSPI_FIFO_SIZE-1) {
			/* last transfer in the queue */
			dspi_pushr |= MCF_DSPI_DTFR_EOQ;
			if ((drv_data->cs_change)
			 && (drv_data->tx == drv_data->tx_end))
				dspi_pushr &= ~MCF_DSPI_DTFR_CONT;
#ifdef CONFIG_M547X_8X
			/* EOQ gets missed if we don't delay */
			udelay(100);
#endif
		} else if (tx_word && ((drv_data->tx_end - drv_data->tx) == 1))
			dspi_pushr |= MCF_DSPI_DTFR_EOQ;
#endif
#if 1
		/*
		 * we don't need this count in NEW_DMA_METHOD,
		 * so let is be.(2009-09-11)
		 */
		if (first) {
			first = 0;
			dspi_pushr |= MCF_DSPI_DTFR_CTCNT; /* clear counter */
		}
#endif
#if defined(SPI_DSPI_EDMA)
		*(volatile u32 *)edma_wr = dspi_pushr;
		edma_wr++;
#else
		*drv_data->dspi_dtfr = dspi_pushr;
#endif
		tx_count++;
	}

#if defined(SPI_DSPI_EDMA)
#ifdef NEW_DMA_METHOD

	if (tx_count > 0) {
		mcf_edma_set_tcd_params(DSPI_DMA_TX_TCD,
			/*virt_to_phys((void *)drv_data->edma_tx_buf),*/
			(u32)drv_data->edma_tx_buf_pa,
			(u32)drv_data->dspi_dtfr,
			MCF_EDMA_TCD_ATTR_SSIZE_32BIT
			| MCF_EDMA_TCD_ATTR_DSIZE_32BIT,
			4,		/* soff */
			4,	/* nbytes */
			0,		/* slast */
			tx_count,		/* citer */
			tx_count,		/* biter */
			0,		/* doff */
			0,		/* dlastsga */
			0,		/* major_int */
			1);		/* disable_req */

		mcf_edma_set_tcd_params(DSPI_DMA_RX_TCD,
			(u32)drv_data->dspi_drfr,
			/*virt_to_phys((void *)drv_data->edma_rx_buf),*/
			(u32)drv_data->edma_rx_buf_pa,
			MCF_EDMA_TCD_ATTR_SSIZE_32BIT
			| MCF_EDMA_TCD_ATTR_DSIZE_32BIT,
			0,		/* soff */
			4,	/* nbytes */
			0,		/* slast */
			tx_count,		/* citer */
			tx_count,		/* biter */
			4,		/* doff */
			0,		/* dlastsga */
			1,		/* major_int */
			1);		/* disable_req */

		mcf_edma_enable_transfer(DSPI_DMA_RX_TCD);
		mcf_edma_enable_transfer(DSPI_DMA_TX_TCD);
	}
#else
	if (tx_count > 0) {

		mcf_edma_set_tcd_params(DSPI_DMA_TX_TCD,
			virt_to_phys((void *)drv_data->edma_tx_buf),
			(u32)drv_data->dspi_dtfr,
			MCF_EDMA_TCD_ATTR_SSIZE_32BIT
			| MCF_EDMA_TCD_ATTR_DSIZE_32BIT,
			4,		/* soff */
			4 * tx_count,	/* nbytes */
			0,		/* slast */
			1,		/* citer */
			1,		/* biter */
			0,		/* doff */
			0,		/* dlastsga */
			0,		/* major_int */
			1);		/* disable_req */

		mcf_edma_set_tcd_params(DSPI_DMA_RX_TCD,
			(u32)drv_data->dspi_drfr,
			virt_to_phys((void *)drv_data->edma_rx_buf),
			MCF_EDMA_TCD_ATTR_SSIZE_32BIT
			| MCF_EDMA_TCD_ATTR_DSIZE_32BIT,
			0,		/* soff */
			4 * tx_count,	/* nbytes */
			0,		/* slast */
			1,		/* citer */
			1,		/* biter */
			4,		/* doff */
			0,		/* dlastsga */
			0,		/* major_int */
			1);		/* disable_req */

		mcf_edma_start_transfer(DSPI_DMA_TX_TCD);
	}
#endif
#endif
	DBG(" send %d[%d]\n", tx_count, tx_count*(tx_word + 1));
	return tx_count * (tx_word + 1);
}

static int read(struct driver_data *drv_data)
{
	int rx_count = 0;
	int rx_word = is_word_transfer(drv_data);
	u16 d;
#if defined(SPI_DSPI_EDMA)
	u32 *rx_edma = (u32 *) drv_data->edma_rx_buf;

	/* receive SPI data */
	udelay(10);
	mcf_edma_start_transfer(DSPI_DMA_RX_TCD);
	udelay(10);
#endif
	while ((drv_data->rx < drv_data->rx_end)
		&& (rx_count < DSPI_FIFO_SIZE)) {

		if (rx_word) {
			if ((drv_data->rx_end - drv_data->rx) == 1)
				break;
#if defined(SPI_DSPI_EDMA)
			d = MCF_DSPI_DRFR_RXDATA(*rx_edma);
			rx_edma++;
#else
			d = MCF_DSPI_DRFR_RXDATA(*drv_data->dspi_drfr);
#endif
			if (!(drv_data->flags & TRAN_STATE_RX_VOID))
				*(u16 *)drv_data->rx = d;
			drv_data->rx += 2;

		} else {
#if defined(SPI_DSPI_EDMA)
			d = MCF_DSPI_DRFR_RXDATA(*rx_edma);
			rx_edma++;
#else
			d = MCF_DSPI_DRFR_RXDATA(*drv_data->dspi_drfr);
#endif
			if (!(drv_data->flags & TRAN_STATE_RX_VOID))
				*(u8 *)drv_data->rx = d;
			drv_data->rx++;
		}
		rx_count++;
		DBG("rxd=0x%x\n", d);
	}
	return rx_count;
}


static inline void dspi_setup_chip(struct driver_data *drv_data)
{
	struct chip_data *chip = drv_data->cur_chip;

	DBG("\n");
	(*(volatile u32 *)drv_data->mcr) = chip->mcr_val;
	(*(volatile u32 *)(drv_data->ctar+drv_data->cs)) = chip->ctar_val;
#ifdef NEW_DMA_METHOD
	/* enable DSPI DMA request function */
	(*(volatile u32 *)drv_data->dspi_rser) =  MCF_DSPI_DRSER_TFFFE
		| MCF_DSPI_DRSER_TFFFS | MCF_DSPI_DRSER_RFDFE
		| MCF_DSPI_DRSER_RFDFS;
#else
	*drv_data->dspi_rser =  MCF_DSPI_DRSER_EOQFE;
#endif
}

#if defined(SPI_DSPI_EDMA)
static int edma_tx_handler(int channel, void *dev)
{
	DBG("\n");
	if (channel == DSPI_DMA_TX_TCD)
		mcf_edma_stop_transfer(DSPI_DMA_TX_TCD);
	return IRQ_HANDLED;
}

static struct driver_data *dspi_drv_data;

static int edma_rx_handler(int channel, void *dev)
{
	struct driver_data *drv_data = dspi_drv_data;
#if 1
	int rx_count = 0;
	int rx_word = is_word_transfer(drv_data);
	u16 d;
	volatile u32 *rx_edma = (volatile u32 *) drv_data->edma_rx_buf;
	struct spi_message *msg = drv_data->cur_msg;
#endif
	DBG("\n");
	if (channel == DSPI_DMA_RX_TCD) {
		mcf_edma_stop_transfer(DSPI_DMA_TX_TCD);
		mcf_edma_stop_transfer(DSPI_DMA_RX_TCD);
	}

#if 1
	if (!(drv_data->flags & TRAN_STATE_RX_VOID)) {
		while ((drv_data->rx < drv_data->rx_end)
				&& (rx_count < DSPI_FIFO_SIZE)) {
			if (rx_word) {
				if ((drv_data->rx_end - drv_data->rx) == 1)
					break;
				d = MCF_DSPI_DRFR_RXDATA(*rx_edma);
				rx_edma++;
				*(u16 *)drv_data->rx = d;
				drv_data->rx += 2;

			} else {
				d = MCF_DSPI_DRFR_RXDATA(*rx_edma);
				rx_edma++;
				*(u8 *)drv_data->rx = d;
				drv_data->rx++;
			}
			rx_count++;
		}
	} else {	/* rx void by upper */
		if ((drv_data->rx_end - drv_data->rx) > DSPI_FIFO_SIZE)
			drv_data->rx += DSPI_FIFO_SIZE;
		else
			drv_data->rx = drv_data->rx_end -
				(drv_data->tx_end - drv_data->tx);
	}
	if (drv_data->rx == drv_data->rx_end) {
		/*
		 * * Finished now - fall through and schedule next
		 * * transfer tasklet
		 * */
		if (drv_data->flags & TRAN_STATE_WORD_ODD_NUM)
			set_16bit_transfer_mode(drv_data);

		msg->state = next_transfer(drv_data);
	} else {
		/* not finished yet - keep going */
		msg->actual_length += write(drv_data);

		return  IRQ_HANDLED;
	}

#endif
	tasklet_schedule(&drv_data->pump_transfers);

	return IRQ_HANDLED;
}
#endif

static irqreturn_t dspi_interrupt(int irq, void *dev_id)
{
	struct driver_data *drv_data = (struct driver_data *)dev_id;
	struct spi_message *msg = drv_data->cur_msg;

	/* Clear all flags immediately */
	*drv_data->dspi_sr = MCF_DSPI_DSR_EOQF;

	if (!drv_data->cur_msg || !drv_data->cur_msg->state) {
#if !defined(SPI_DSPI_EDMA)
		u32 irq_status = *drv_data->dspi_sr;
		/* if eDMA is used it happens some time (at least once)*/
		printk(KERN_ERR "Bad message or transfer state handler. \
				 IRQ status = %x\n", irq_status);
#endif
		return IRQ_NONE;
	}

	DBG("\n");
	/*
	 * Read the data into the buffer and reload and start
	 * queue with new data if not finished.  If finished
	 * then setup the next transfer
	 */
#if defined(SPI_DSPI_EDMA)
	 mcf_edma_start_transfer(DSPI_DMA_RX_TCD);
#endif
	read(drv_data);

	if (drv_data->rx == drv_data->rx_end) {
		/*
		 * Finished now - fall through and schedule next
		 * transfer tasklet
		 */
		if (drv_data->flags & TRAN_STATE_WORD_ODD_NUM)
			set_16bit_transfer_mode(drv_data);

		msg->state = next_transfer(drv_data);
	} else {
		/* not finished yet - keep going */
		msg->actual_length += write(drv_data);
		return IRQ_HANDLED;
	}

	tasklet_schedule(&drv_data->pump_transfers);

	return IRQ_HANDLED;
}

/* caller already set message->status; dma and pio irqs are blocked */
static void giveback(struct driver_data *drv_data)
{
	struct spi_transfer *last_transfer;
	unsigned long flags;
	struct spi_message *msg;
	DBG("\n");

	spin_lock_irqsave(&drv_data->lock, flags);
	msg = drv_data->cur_msg;
	drv_data->cur_msg = NULL;
	drv_data->cur_transfer = NULL;
	drv_data->cur_chip = NULL;
	queue_work(drv_data->workqueue, &drv_data->pump_messages);
	spin_unlock_irqrestore(&drv_data->lock, flags);

	last_transfer = list_entry(msg->transfers.prev,
				   struct spi_transfer,	transfer_list);

	if (!last_transfer->cs_change)
		drv_data->cs_control(drv_data->cs, QSPI_CS_DROP);

	msg->state = NULL;
	if (msg->complete)
		msg->complete(msg->context);
}


static void pump_transfers(unsigned long data)
{
	struct driver_data *drv_data = (struct driver_data *)data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;
	struct chip_data *chip = NULL;
	unsigned long flags;
#if 0
	int rx_count = 0;
	int rx_word = is_word_transfer(drv_data);
	u16 d;
	volatile u32 *rx_edma = (volatile u32 *) drv_data->edma_rx_buf;
	struct spi_message *msg = drv_data->cur_msg;
#endif
	DBG("\n");

#if 0
	if (!first_in_transfer) {
		if (!(drv_data->flags & TRAN_STATE_RX_VOID)) {
			while ((drv_data->rx < drv_data->rx_end)
					&& (rx_count < DSPI_FIFO_SIZE)) {
				if (rx_word) {
					if ((drv_data->rx_end - drv_data->rx)
							== 1)
						break;
					d = MCF_DSPI_DRFR_RXDATA(*rx_edma);
					rx_edma++;
					*(u16 *)drv_data->rx = d;
					drv_data->rx += 2;

				} else {
					d = MCF_DSPI_DRFR_RXDATA(*rx_edma);
					rx_edma++;
					*(u8 *)drv_data->rx = d;
					drv_data->rx++;
				}
				rx_count++;
			}
		} else {	/* rx void by upper */
			if ((drv_data->rx_end - drv_data->rx) > DSPI_FIFO_SIZE)
				drv_data->rx += DSPI_FIFO_SIZE;
			else
				drv_data->rx = drv_data->rx_end -
					(drv_data->tx_end - drv_data->tx);
		}
		if (drv_data->rx == drv_data->rx_end) {
			/*
			 * * Finished now - fall through and schedule next
			 * * transfer tasklet
			 * */
			if (drv_data->flags & TRAN_STATE_WORD_ODD_NUM)
				set_16bit_transfer_mode(drv_data);

			msg->state = next_transfer(drv_data);
		} else {
			/* not finished yet - keep going */
			local_irq_save(flags);
			msg->actual_length += write(drv_data);
			local_irq_restore(flags);
			return;
		}
	} else {
		first_in_transfer = 0;
	}
#endif


	/* Get current state information */
	message = drv_data->cur_msg;
	transfer = drv_data->cur_transfer;
	chip = drv_data->cur_chip;

	/* Handle for abort */
	if (message->state == ERROR_STATE) {
		message->status = -EIO;
		giveback(drv_data);
		return;
	}

	/* Handle end of message */
	if (message->state == DONE_STATE) {
		message->status = 0;
		giveback(drv_data);
		return;
	}
#if 1
		drv_data->cs = message->spi->chip_select;
		drv_data->cs_change = transfer->cs_change;
		drv_data->void_write_data = chip->void_write_data;
#endif

	if (message->state == START_STATE) {
#if 0
		drv_data->cs = message->spi->chip_select;
		drv_data->cs_change = transfer->cs_change;
		drv_data->void_write_data = chip->void_write_data;
#endif

		dspi_setup_chip(drv_data);

		if (drv_data->cs_control)
			drv_data->cs_control(message->spi->chip_select,
				QSPI_CS_ASSERT);
	}

	/* Delay if requested at end of transfer*/
	if (message->state == RUNNING_STATE) {
		previous = list_entry(transfer->transfer_list.prev,
					struct spi_transfer,
					transfer_list);

		if (drv_data->cs_control && transfer->cs_change)
			drv_data->cs_control(message->spi->chip_select,
				QSPI_CS_DROP);

		if (previous->delay_usecs)
			udelay(previous->delay_usecs);

		if (drv_data->cs_control && transfer->cs_change)
			drv_data->cs_control(message->spi->chip_select,
				QSPI_CS_ASSERT);
	}

	drv_data->flags = 0;
	drv_data->tx = (void *)transfer->tx_buf;
	drv_data->tx_end = drv_data->tx + transfer->len;
	drv_data->rx = transfer->rx_buf;
	drv_data->rx_end = drv_data->rx + transfer->len;

	if (!drv_data->rx)
		drv_data->flags |= TRAN_STATE_RX_VOID;

	if (!drv_data->tx)
		drv_data->flags |= TRAN_STATE_TX_VOID;

#if 0
	drv_data->cs = message->spi->chip_select;
	drv_data->cs_change = transfer->cs_change;
	drv_data->void_write_data = chip->void_write_data;
#endif
	if (transfer->speed_hz) {
		*(drv_data->ctar + drv_data->cs) = \
			((chip->ctar_val & ~0xF) | \
			  hz_to_spi_baud(chip->ctar.pbr, \
					 chip->ctar.dbr, \
					 transfer->speed_hz));
	}

	message->state = RUNNING_STATE;

	/* Go baby, go */
	local_irq_save(flags);
	message->actual_length += write(drv_data);
	local_irq_restore(flags);
}


static void pump_messages(struct work_struct *work)
{
	struct driver_data *drv_data;
	unsigned long flags;
	DBG("\n");

	drv_data = container_of(work, struct driver_data, pump_messages);

	/* Lock queue and check for queue work */
	spin_lock_irqsave(&drv_data->lock, flags);
	if (list_empty(&drv_data->queue)
		|| drv_data->run == QUEUE_STOPPED) {
		drv_data->busy = 0;
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return;
	}

	/* Make sure we are not already running a message */
	if (drv_data->cur_msg) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return;
	}

	/* Extract head of queue */
	drv_data->cur_msg = list_entry(drv_data->queue.next,
					struct spi_message, queue);
	list_del_init(&drv_data->cur_msg->queue);

	/* Initial message state*/
	drv_data->cur_msg->state = START_STATE;
	drv_data->cur_transfer = list_entry(drv_data->cur_msg->transfers.next,
						struct spi_transfer,
						transfer_list);

	if (drv_data->cur_transfer->transfer_list.next
		== &drv_data->cur_msg->transfers)
		drv_data->cur_transfer->cs_change = 1; /* last */

#ifdef NEW_DMA_METHOD
	mcf_edma_stop_transfer(DSPI_DMA_TX_TCD);
	mcf_edma_stop_transfer(DSPI_DMA_RX_TCD);
	first_in_transfer = 1;
#endif
	/* Setup the SPI Registers using the per chip configuration */
	drv_data->cur_chip = spi_get_ctldata(drv_data->cur_msg->spi);

	/* Mark as busy and launch transfers */
	tasklet_schedule(&drv_data->pump_transfers);

	drv_data->busy = 1;
	spin_unlock_irqrestore(&drv_data->lock, flags);
}

/****************************************************************************/

/*
 * SPI master implementation
 */

static int transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct driver_data *drv_data = spi_master_get_devdata(spi->master);
	unsigned long flags;

	DBG("\n");
	spin_lock_irqsave(&drv_data->lock, flags);

	if (drv_data->run == QUEUE_STOPPED) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return -ESHUTDOWN;
	}

	msg->actual_length = 0;
	msg->status = -EINPROGRESS;
	msg->state = START_STATE;

	list_add_tail(&msg->queue, &drv_data->queue);

	if (drv_data->run == QUEUE_RUNNING && !drv_data->busy)
		queue_work(drv_data->workqueue, &drv_data->pump_messages);

	spin_unlock_irqrestore(&drv_data->lock, flags);

	return 0;
}


static int setup(struct spi_device *spi)
{
	struct chip_data *chip;
	struct coldfire_dspi_chip *chip_info
		= (struct coldfire_dspi_chip *)spi->controller_data;
	DBG("\n");

	/* Only alloc on first setup */
	chip = spi_get_ctldata(spi);
	if (chip == NULL) {
		chip = kcalloc(1, sizeof(struct chip_data), GFP_KERNEL);
		if (!chip)
			return -ENOMEM;
		spi->mode = chip_info->mode;
		spi->bits_per_word = chip_info->bits_per_word;
	}

	chip->mcr.master = 1;
	chip->mcr.cont_scke = 0;
	chip->mcr.dconf = 0;
	chip->mcr.frz = 0;
	chip->mcr.mtfe = 0;
	chip->mcr.pcsse = 0;
	chip->mcr.rooe = 0;
	chip->mcr.pcsis = 0xFF;
	chip->mcr.reserved15 = 0;
	chip->mcr.mdis = 0;
	chip->mcr.dis_tx = 0;
	chip->mcr.dis_rxf = 0;
	chip->mcr.clr_tx = 1;
	chip->mcr.clr_rxf = 1;
	chip->mcr.smpl_pt = 0;
	chip->mcr.reserved71 = 0;
	chip->mcr.halt = 0;

	if ((spi->bits_per_word >= 4) && (spi->bits_per_word <= 16)) {
		chip->ctar.fmsz = spi->bits_per_word-1;
	} else {
		printk(KERN_ERR "Invalid wordsize\n");
		kfree(chip);
		return -ENODEV;
	}

	chip->void_write_data = chip_info->void_write_data;

	if (spi->max_speed_hz != 0)
		chip_info->br = hz_to_spi_baud(chip_info->pbr, chip_info->dbr, \
					       spi->max_speed_hz);

	chip->ctar.cpha = (spi->mode & SPI_CPHA) ? 1 : 0;
	chip->ctar.cpol = (spi->mode & SPI_CPOL) ? 1 : 0;
	chip->ctar.lsbfe = (spi->mode & SPI_LSB_FIRST) ? 1 : 0;
	chip->ctar.dbr = chip_info->dbr;
	chip->ctar.pbr = chip_info->pbr;
	chip->ctar.br = chip_info->br;
	chip->ctar.pcssck = chip_info->pcssck;
	chip->ctar.pasc = chip_info->pasc;
	chip->ctar.pdt = chip_info->pdt;
	chip->ctar.cssck = chip_info->cssck;
	chip->ctar.asc = chip_info->asc;
	chip->ctar.dt = chip_info->dt;

	spi_set_ctldata(spi, chip);

	return 0;
}

static int init_queue(struct driver_data *drv_data)
{
	INIT_LIST_HEAD(&drv_data->queue);
	spin_lock_init(&drv_data->lock);

	drv_data->run = QUEUE_STOPPED;
	drv_data->busy = 0;

	tasklet_init(&drv_data->pump_transfers,
			pump_transfers,	(unsigned long)drv_data);

	INIT_WORK(&drv_data->pump_messages, pump_messages);

	drv_data->workqueue = create_singlethread_workqueue(
				drv_data->master->dev.parent->bus_id);
	if (drv_data->workqueue == NULL)
		return -EBUSY;

	return 0;
}

static int start_queue(struct driver_data *drv_data)
{
	unsigned long flags;

	spin_lock_irqsave(&drv_data->lock, flags);

	if (drv_data->run == QUEUE_RUNNING || drv_data->busy) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return -EBUSY;
	}

	drv_data->run = QUEUE_RUNNING;
	drv_data->cur_msg = NULL;
	drv_data->cur_transfer = NULL;
	drv_data->cur_chip = NULL;
	spin_unlock_irqrestore(&drv_data->lock, flags);

	queue_work(drv_data->workqueue, &drv_data->pump_messages);

	return 0;
}

static int stop_queue(struct driver_data *drv_data)
{
	unsigned long flags;
	unsigned limit = 500;
	int status = 0;

	spin_lock_irqsave(&drv_data->lock, flags);

	/* This is a bit lame, but is optimized for the common execution path.
	 * A wait_queue on the drv_data->busy could be used, but then the common
	 * execution path (pump_messages) would be required to call wake_up or
	 * friends on every SPI message. Do this instead */
	drv_data->run = QUEUE_STOPPED;
	while (!list_empty(&drv_data->queue) && drv_data->busy && limit--) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		msleep(10);
		spin_lock_irqsave(&drv_data->lock, flags);
	}

	if (!list_empty(&drv_data->queue) || drv_data->busy)
		status = -EBUSY;

	spin_unlock_irqrestore(&drv_data->lock, flags);

	return status;
}

static int destroy_queue(struct driver_data *drv_data)
{
	int status;

	status = stop_queue(drv_data);
	if (status != 0)
		return status;

	destroy_workqueue(drv_data->workqueue);

	return 0;
}


static void cleanup(struct spi_device *spi)
{
	struct chip_data *chip = spi_get_ctldata((struct spi_device *)spi);

	dev_dbg(&spi->dev, "spi_device %u.%u cleanup\n",
		spi->master->bus_num, spi->chip_select);

	kfree(chip);
}


/****************************************************************************/

/*
 * Generic Device driver routines and interface implementation
 */

static int coldfire_spi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct coldfire_spi_master *platform_info;
	struct spi_master *master;
	struct driver_data *drv_data = 0;
	struct resource *memory_resource;
	int irq;
	int status = 0;
	int i;

	platform_info = (struct coldfire_spi_master *)dev->platform_data;

	master = spi_alloc_master(dev, sizeof(struct driver_data));
	if (!master)
		return -ENOMEM;

	drv_data = spi_master_get_devdata(master);
	drv_data->master = master;

	INIT_LIST_HEAD(&drv_data->queue);
	spin_lock_init(&drv_data->lock);

	master->bus_num = platform_info->bus_num;
	master->num_chipselect = platform_info->num_chipselect;
	master->cleanup = cleanup;
	master->setup = setup;
	master->transfer = transfer;

	drv_data->cs_control = platform_info->cs_control;
	if (drv_data->cs_control)
		for (i = 0; i < master->num_chipselect; i++)
			drv_data->cs_control(i, QSPI_CS_INIT | QSPI_CS_DROP);

	/* Setup register addresses */
	memory_resource = platform_get_resource_byname(pdev,
				IORESOURCE_MEM, "spi-module");
	if (!memory_resource) {
		dev_dbg(dev, "can not find platform module memory\n");
		goto out_error_master_alloc;
	}

#if defined(SPI_DSPI_EDMA)
	drv_data->edma_tx_buf = (volatile void *)dma_alloc_coherent(NULL,
						EDMA_BUFSIZE_KMALLOC,
						&drv_data->edma_tx_buf_pa,
						GFP_DMA);
	if (!drv_data->edma_tx_buf) {
		dev_dbg(dev, "cannot allocate eDMA TX memory\n");
		goto out_error_master_alloc;
	}
	drv_data->edma_rx_buf = (volatile void *)dma_alloc_coherent(NULL,
						EDMA_BUFSIZE_KMALLOC,
						&drv_data->edma_rx_buf_pa,
						GFP_DMA);
	if (!drv_data->edma_rx_buf) {
		dma_free_coherent(NULL, EDMA_BUFSIZE_KMALLOC, \
				(void *)drv_data->edma_tx_buf,
				drv_data->edma_tx_buf_pa);
		dev_dbg(dev, "cannot allocate eDMA RX memory\n");
		goto out_error_master_alloc;
	}
	printk(KERN_INFO "Coldfire DSPI DMA addr: Tx-0x%p[0x%x],"
			" Rx-0x%p[0x%x]\n",
			drv_data->edma_tx_buf, drv_data->edma_tx_buf_pa,
			drv_data->edma_rx_buf, drv_data->edma_rx_buf_pa);
#endif
#if defined(CONFIG_DSPI0)
	drv_data->mcr = (volatile u32 *)&MCF_DSPI_DMCR;
	drv_data->ctar = (volatile u32 *)&MCF_DSPI_DCTAR0;
	drv_data->dspi_sr = (volatile u32 *)&MCF_DSPI_DSR;
	drv_data->dspi_rser = (volatile u32 *)&MCF_DSPI_DRSER;
	drv_data->dspi_dtfr = (volatile u32 *)&MCF_DSPI_DTFR;
	drv_data->dspi_drfr = (volatile u32 *)&MCF_DSPI_DRFR;
#elif defined(CONFIG_DSPI1)
	drv_data->mcr = (volatile u32 *)&MCF_DSPI1_DMCR;
	drv_data->ctar = (volatile u32 *)&MCF_DSPI1_DCTAR0;
	drv_data->dspi_sr = (volatile u32 *)&MCF_DSPI1_DSR;
	drv_data->dspi_rser = (volatile u32 *)&MCF_DSPI1_DRSER;
	drv_data->dspi_dtfr = (volatile u32 *)&MCF_DSPI1_DTFR;
	drv_data->dspi_drfr = (volatile u32 *)&MCF_DSPI1_DRFR;

#endif
	memory_resource = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						       "spi-par");
	if (!memory_resource) {
		dev_dbg(dev, "No spi-par memory\n");
		goto out_error_master_alloc;
	}
#ifdef CONFIG_M5301x
	drv_data->parh = (void *)memory_resource->start;
	drv_data->parl = (void *)memory_resource->end;
#elif defined(CONFIG_M5441X)
	/* This configuration has been set at arch scource*/
#else
	drv_data->par = (void *)memory_resource->start;
#endif

	memory_resource = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						       "spi-int-level");
	if (!memory_resource) {
		dev_dbg(dev, "No spi-int-level memory\n");
		goto out_error_master_alloc;
	}
	drv_data->int_icr = (void *)memory_resource->start;

	memory_resource = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						       "spi-int-mask");
	if (!memory_resource) {
		dev_dbg(dev, "No spi-int-mask memory\n");
		goto out_error_master_alloc;
	}
	drv_data->int_mr = (void *)memory_resource->start;
#ifndef NEW_DMA_METHOD
	/*
	 * PIO mode
	 */
	if (platform_info->irq_list) {
		/* multiple IRQs */
		int *irqlist = platform_info->irq_list;
		while ((irq = *irqlist++)) {
			int off = *irqlist++;
			int lvl = *irqlist++;
			int msk = *irqlist++;
			status = request_irq(irq, dspi_interrupt, IRQF_DISABLED,
					     dev->bus_id, drv_data);
			if (status < 0) {
				dev_err(&pdev->dev,
				"Unable to attach ColdFire DSPI interrupt\n");
				goto out_error_master_alloc;
			}

			if (lvl)
				*(drv_data->int_icr + off) = lvl;

			if (msk)
				*drv_data->int_mr &= ~msk;
		}
	} else {
		irq = platform_info->irq_vector;

		status = request_irq(platform_info->irq_vector, dspi_interrupt,
				     IRQF_DISABLED, dev->bus_id, drv_data);
		if (status < 0) {
			dev_err(&pdev->dev,
				"Unable to attach ColdFire DSPI interrupt\n");
			goto out_error_master_alloc;
		}

		*drv_data->int_icr = platform_info->irq_lp;
		*drv_data->int_mr &= ~platform_info->irq_mask;
	}
#endif
	/* Now that we have all the addresses etc.  Let's set it up */
#ifdef CONFIG_M5301x
	*drv_data->parh = (u8) (platform_info->par_val16 >> 8);
	*drv_data->parl = (u8) (platform_info->par_val16 & 0xff);
#elif defined(CONFIG_M5441X)
	/* This configuration has been set at arch source */
#else
	*drv_data->par = platform_info->par_val;
#endif

	/* Initial and start queue */
	status = init_queue(drv_data);
	if (status != 0) {
		dev_err(&pdev->dev, "Problem initializing DSPI queue\n");
		goto out_error_irq_alloc;
	}
	status = start_queue(drv_data);
	if (status != 0) {
		dev_err(&pdev->dev, "Problem starting DSPI queue\n");
		goto out_error_irq_alloc;
	}

#if defined(SPI_DSPI_EDMA)
	DBG("MCF edma request channel for SPI\n");

	if (mcf_edma_request_channel(DSPI_DMA_TX_TCD,
				     edma_tx_handler,
				     NULL,
				     0x00,
				     pdev,
				     NULL, /* spinlock */
				     DRIVER_NAME) < 0){
		dev_err(&pdev->dev, "eDMA transmit channel request\n");
		status = -EINVAL;
		goto out_error_queue_alloc;
	}
/*
 * we only need RX eDMA interrupt to sync a spi transfer,
 * the Tx eDMA interrupt can be ignored, this is determined
 * by SPI communicate machnisim, i.e, is half duplex mode, that is
 * whether read or write, we need write data out to get we wanted.
 */
	if (mcf_edma_request_channel(DSPI_DMA_RX_TCD,
				     edma_rx_handler,
				     NULL,
				     0x06,
				     pdev,
				     NULL, /* spinlock */
				     DRIVER_NAME) < 0){
		dev_err(&pdev->dev, "eDAM receive channel request\n");
		status = -EINVAL;
		mcf_edma_free_channel(DSPI_DMA_TX_TCD, pdev);
		goto out_error_queue_alloc;
	}

	dspi_drv_data = drv_data;
#endif

	/* Register with the SPI framework */
	platform_set_drvdata(pdev, drv_data);
	status = spi_register_master(master);
	if (status != 0) {
		dev_err(&pdev->dev, "Problem registering DSPI master\n");
		status = -EINVAL;
		goto out_error_queue_alloc;
	}

#ifdef NEW_DMA_METHOD
	printk(KERN_INFO "Coldfire DSPI: Using Highspeed eDMA transfer method!\n");
#endif
	printk(KERN_INFO "DSPI: Coldfire master initialized\n");
	return status;

out_error_queue_alloc:
	destroy_queue(drv_data);

out_error_irq_alloc:
	free_irq(platform_info->irq_vector, drv_data);

out_error_master_alloc:
	spi_master_put(master);
	return status;

}

static int coldfire_spi_remove(struct platform_device *pdev)
{
	struct driver_data *drv_data = platform_get_drvdata(pdev);
	int irq;
	int status = 0;

	if (!drv_data)
		return 0;

#if defined(SPI_DSPI_EDMA)
	mcf_edma_free_channel(DSPI_DMA_TX_TCD, pdev);
	mcf_edma_free_channel(DSPI_DMA_RX_TCD, pdev);
#endif

	/* Remove the queue */
	status = destroy_queue(drv_data);
	if (status != 0)
		return status;

	/* Release IRQ */
	irq = platform_get_irq(pdev, 0);
	if (irq >= 0)
		free_irq(irq, drv_data);

	/* Disconnect from the SPI framework */
	spi_unregister_master(drv_data->master);

	/* Prevent double remove */
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static void coldfire_spi_shutdown(struct platform_device *pdev)
{
	int status = coldfire_spi_remove(pdev);

	if (status != 0)
		dev_err(&pdev->dev, "shutdown failed with %d\n", status);
}


#ifdef CONFIG_PM
static int suspend_devices(struct device *dev, void *pm_message)
{
	pm_message_t *state = pm_message;

	if (dev->power.power_state.event != state->event) {
		dev_warn(dev, "pm state does not match request\n");
		return -1;
	}

	return 0;
}

static int coldfire_spi_suspend(struct platform_device *pdev,
				pm_message_t state)
{
	struct driver_data *drv_data = platform_get_drvdata(pdev);
	int status = 0;

	/* Check all childern for current power state */
	if (device_for_each_child(&pdev->dev,
		&state, suspend_devices) != 0) {
		dev_warn(&pdev->dev, "suspend aborted\n");
		return -1;
	}

	status = stop_queue(drv_data);
	if (status != 0)
		return status;

	return 0;
}

static int coldfire_spi_resume(struct platform_device *pdev)
{
	struct driver_data *drv_data = platform_get_drvdata(pdev);
	int status = 0;

	/* Start the queue running */
	status = start_queue(drv_data);
	if (status != 0) {
		dev_err(&pdev->dev, "problem starting queue (%d)\n", status);
		return status;
	}

	return 0;
}
#else
#define coldfire_spi_suspend NULL
#define coldfire_spi_resume NULL
#endif /* CONFIG_PM */

static struct platform_driver driver = {
	.driver = {
		.name = "spi_coldfire",
		.bus = &platform_bus_type,
		.owner = THIS_MODULE,
	},
	.probe = coldfire_spi_probe,
	.remove = __devexit_p(coldfire_spi_remove),
	.shutdown = coldfire_spi_shutdown,
	.suspend = coldfire_spi_suspend,
	.resume = coldfire_spi_resume,
};

static int __init coldfire_spi_init(void)
{
	platform_driver_register(&driver);

	return 0;
}
module_init(coldfire_spi_init);

static void __exit coldfire_spi_exit(void)
{
	platform_driver_unregister(&driver);
}
module_exit(coldfire_spi_exit);

MODULE_AUTHOR("Matt Waddel");
MODULE_DESCRIPTION("ColdFire DSPI Contoller");
MODULE_LICENSE("GPL");
