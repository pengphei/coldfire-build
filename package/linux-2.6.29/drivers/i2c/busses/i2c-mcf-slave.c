/*
 * i2c-mcf-slave.c - support adpater slave mode, now only support mcf5445x platform
 *
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Lanttor Guo <lanttor.guo@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
*/

#ifdef CONFIG_I2C_SLAVE_TEST
#define DEBUG
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/types.h>
#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/irq.h>
#include <asm/mcfi2c.h>
#if defined(CONFIG_M5445X)
#include <asm/mcf5445x_intc.h>
#endif

#define IRQ (64+30)
#define SLAVE_HANDLER_NAME "mcf-i2c slave handler"
#define	I2C_BUFFER_SIZE	50

/* Structure for storing I2C transfer data */
struct i2c_buffer {
    int tx_index;			/* TX index */
    int rx_index;			/* RX index */
    u16 length;			/* Length of the buffer in bytes */
    u8 buf[I2C_BUFFER_SIZE];		/* Data buffer */
};

struct i2c_buffer i2c_tx_buffer;
struct i2c_buffer i2c_rx_buffer;

u8 *tx_string = "abcdefghijklmnopqrstuvwxyz0123456789)!@#$%^&*([].";

/*
 * I2C slave mode interrupt handler
 *
 */
static irqreturn_t i2c_slave_handler(int this_irq, void *dev_id)
{
	u8 dummy_read;

#ifdef DEBUG
	printk("i2c adapter slave mode irq handler.\n");
#endif

	/* Clear I2C interupt flag */
	MCF_I2SR = ~MCF_I2SR_IIF;

	/* Check if this device is in Master or Slave Mode. */
	if (MCF_I2CR & MCF_I2CR_MSTA) {
		/* Master mode, do nothing here */
		printk(KERN_INFO "i2c master mode at %s(), do nothing!\n", __func__);
		return IRQ_NONE;
	} else {
		/* Slave Mode - Check if Arbitration Lost. */
		if (MCF_I2SR & MCF_I2SR_IAL) {

		#ifdef DEBUG
			printk("Arbitration Lost.\n");
		#endif

			/* Clear IAL bit */
			MCF_I2SR &= ~MCF_I2SR_IAL;

			/* Arbitration Lost -
			 * Check if this device is being addressed as slave.
			 *(If not, nothing more needs to be done.)
			 */
			if (MCF_I2SR & MCF_I2SR_IAAS) {
				/* Addressed as slave -
				 * Check if master was reading from slave or
				 * writing to slave.
				 */
				if (MCF_I2SR & MCF_I2SR_SRW) {
					/* Set tx_index to 0 */
					if (i2c_tx_buffer.length == 0) {
						i2c_tx_buffer.length = I2C_BUFFER_SIZE;
						i2c_tx_buffer.tx_index = 0;
					}

					/* Master was reading from slave -
					 * Set Transmit Mode.
					 */
					MCF_I2CR |= MCF_I2CR_MTX;

					/* Write data to MBDR. */
					MCF_I2DR =
					i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++];
					i2c_tx_buffer.length--;

				#ifdef DEBUG
					printk("Arbitration Lost: Addressed as slave - TX mode.\n");
				#endif
				} else {
					/* Set rx_index to 0 */
					i2c_rx_buffer.rx_index = 0;

					/* Master was writing to slave - Set Receive Mode. */
					MCF_I2CR &= ~MCF_I2CR_MTX;

					/* Dummy read from MBDR, to clear the ICF bit. */
					dummy_read = MCF_I2DR;

				#ifdef DEBUG
					printk("Arbitration Lost: Addressed as slave - RX mode.\n");
				#endif
				}
			}

		} else {
			/* Arbitration Not Lost - Check if data byte is this devices's
			 * Slave Address byte. */
			if (MCF_I2SR & MCF_I2SR_IAAS) {
				/* Data byte is Slave Address byte - Check Slave Read/Write bit. */
				if (MCF_I2SR & MCF_I2SR_SRW) {
					/* Set tx_index to 0 */
					if (i2c_tx_buffer.length == 0) {
						i2c_tx_buffer.length = I2C_BUFFER_SIZE;
						i2c_tx_buffer.tx_index = 0;
					}

					/* Master was reading from slave - Set Transmit Mode. */
					MCF_I2CR |= MCF_I2CR_MTX;

					/* Write data to MBDR. */
					MCF_I2DR =
					i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++];
					i2c_tx_buffer.length--;

				#ifdef DEBUG
					printk("Slave TX: First byte - 0x%02X\n",
						i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
				#endif
				} else {
					/* Master has specified Slave Receive Mode.
					   Set Receive Mode.  (Writing to MBCR clears IAAS.) */

					/* Set rx_index to 0 */
					i2c_rx_buffer.rx_index = 0;

					MCF_I2CR &= ~MCF_I2CR_MTX;

					/* Dummy read from MBDR, to clear the ICF bit. */
					dummy_read = MCF_I2DR;

				#ifdef DEBUG
					printk("Slave RX: Receive address.\n");
				#endif
				}
			} else {
				/* Data byte received is not Slave Address byte -
				   Check if this device is in Transmit or Receive Mode. */
				if (MCF_I2CR & MCF_I2CR_MTX) {
					/* Last byte received? */
					if (MCF_I2SR & MCF_I2SR_RXAK) {
						MCF_I2CR &= ~MCF_I2CR_MTX;
						dummy_read = MCF_I2DR;

					#ifdef DEBUG
						printk("Slave TX: Last byte has been sent.\n");
					#endif
					} else {
						/* Write data to MBDR. */
						MCF_I2DR =
						i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++];
						i2c_tx_buffer.length--;

					#ifdef DEBUG
						printk("Slave TX: Send byte - 0x%02X\n",
							i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
					#endif
						if (i2c_tx_buffer.length == 0) {
							i2c_tx_buffer.length = I2C_BUFFER_SIZE;
							i2c_tx_buffer.tx_index = 0;
						}

					}
				} else {
					/* Receive Mode - Read data from MBDR and store it. */
					i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = MCF_I2DR;
					i2c_rx_buffer.length++;

				#ifdef DEBUG
					printk("Slave RX: Receive byte - 0x%02X\n",
						i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
				#endif
				}
			}
		}
		return IRQ_HANDLED;
	}
}

#ifdef CONFIG_PROC_FS

/*
 *	Info exported via "/proc/driver/i2c".
 */

static int gen_i2c_proc_output(char *buf)
{
	char *p;

	p = buf;
	p += sprintf(p,
		     "I2CR: 0x%x\n"
		     "I2SR: 0x%x\n"
		     "I2DR: 0x%x\n",
		     MCF_I2CR, MCF_I2SR, MCF_I2DR);

	return p - buf;
}

static int gen_i2c_read_proc(char *page, char **start, off_t off,
			     int count, int *eof, void *data)
{
	int len = gen_i2c_proc_output (page);
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

static int __init gen_i2c_proc_init(void)
{
	struct proc_dir_entry *r;

	r = create_proc_read_entry("driver/i2c-adaptor-register", 0, NULL, gen_i2c_read_proc, NULL);
	if (!r)
		return -ENOMEM;
	return 0;
}
#else
static inline int gen_i2c_proc_init(void) { return 0; }
#endif /* CONFIG_PROC_FS */

/*
 *  Initalize I2C module
 */
static int __init i2c_coldfire_init(void)
{
	int retval;
	u8  dummy_read;

#ifdef DEBUG
	printk(KERN_INFO "init i2c adaptor slave mode!\n");
#endif

	/* Initialize the tx buffer */
	strcpy((char *)&i2c_tx_buffer.buf, (const char *)tx_string);
	i2c_tx_buffer.length = I2C_BUFFER_SIZE;

#if defined(CONFIG_M5445X)
	/*
	 * Initialize the GPIOs for I2C
	 */
	MCF_GPIO_PAR_FECI2C |= (0
			| MCF_GPIO_PAR_FECI2C_PAR_SDA(3)
			| MCF_GPIO_PAR_FECI2C_PAR_SCL(3));
#endif

	/* Set transmission frequency 0x19 = ~100kHz */
	MCF_I2FDR = 0x19;

	/* set the I2C slave address */
	MCF_I2AR = 0x6A;

	/* Enable I2C module and if IBB is set, do the special initialzation */
	/* procedures as are documented */

	if ((MCF_I2SR & MCF_I2SR_IBB) == 1) {
		printk(KERN_INFO "%s - do special I2C init procedures \n",
			__func__);
		MCF_I2CR = 0x00;
		MCF_I2CR = 0xA0;
		dummy_read = MCF_I2DR;
		MCF_I2SR = 0x00;
		MCF_I2CR = 0x00;
	}

	MCF_I2CR |= (MCF_I2CR_IEN  | MCF_I2CR_IIEN);

	/* default I2C mode is - slave and receive */
	MCF_I2CR &= ~(MCF_I2CR_MSTA | MCF_I2CR_MTX);

	retval = request_irq(IRQ, i2c_slave_handler, IRQF_DISABLED,
			     SLAVE_HANDLER_NAME, NULL);
	if (retval < 0)
		printk(KERN_INFO "request_irq for i2c slave mode failed!\n");

	retval = gen_i2c_proc_init();

	if (retval < 0)
		printk(KERN_INFO "gen /proc/i2c-adaptor-register for i2c slave mode failed!\n");

	return retval;
};

/*
 *  I2C module exit function
 */

static void __exit i2c_coldfire_exit(void)
{
	/* disable I2C and Interrupt */
	MCF_I2CR &= ~(MCF_I2CR_IEN | MCF_I2CR_IIEN);
	free_irq(IRQ, NULL);

};

MODULE_DESCRIPTION("MCF5445x I2C adaptor slave mode support");
MODULE_LICENSE("GPL");

module_init(i2c_coldfire_init);
module_exit(i2c_coldfire_exit);
