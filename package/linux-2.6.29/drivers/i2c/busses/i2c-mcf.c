/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 * Lanttor.Guo@freescale.com
 *
 * I2C bus driver on mcfv4/mcfv4e platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/i2c.h>
#include "i2c-algo-mcf.h"

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/proc_fs.h>

#include <asm/coldfire.h>
#include <asm/mcfi2c.h>

#if defined(CONFIG_M547X_8X)
#include <asm/m5485sim.h>
#elif defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
#include <asm/mcfsim.h>
#endif

#define get_clock(adap) (clock)
#define get_own(adap) 	(own)

#if defined(CONFIG_M547X_8X)
static int clock = 0x3b;
#elif defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
static int clock = 0x19;
#endif
module_param(clock, int, 0);
MODULE_PARM_DESC(clock,
	"Set I2C clock in kHz: 400=fast mode (default == 100khz)");

static int own = 0x78;
module_param(own, int, 0);
MODULE_PARM_DESC(clock, "Set I2C Master controller address");

static struct i2c_algo_mcf_data i2c_mcf_board_data = {
	.timeout =	10000,
};

static struct i2c_adapter i2c_mcf_board_adapter = {
	.owner = THIS_MODULE,
	.name = "mcf i2c adapter",
	.algo_data = &i2c_mcf_board_data,
	.class = I2C_CLASS_HWMON,
	.timeout = 100,
	.retries = 2
};
/*
 *  static void i2c_start()
 *
 *  Generates START signal
 */
static void
i2c_start(
	struct i2c_algo_mcf_data *adap
) {
	MCF_I2CR |= MCF_I2CR_MSTA;
}


/*
 *  static void i2c_stop()
 *
 *  Generates STOP signal
 */
static void
i2c_stop(
	struct i2c_algo_mcf_data *adap
) {
	MCF_I2CR &= ~MCF_I2CR_MSTA;
}

static int
i2c_getack(
	struct i2c_algo_mcf_data *adap
) {
	return !(MCF_I2SR & MCF_I2SR_RXAK);
}

/*
 *  static void i2c_repstart()
 *
 *  Generates repeated start signal (without STOP while mastering the bus)
 */
static void
i2c_repstart(
	struct i2c_algo_mcf_data *adap
) {
	MCF_I2CR |= MCF_I2CR_RSTA;
	MCF_I2CR |= MCF_I2CR_MTX;
}


/*
 *  static void wait_for_bb()
 *
 *  Wait for bus idle state
 */
static int
wait_for_bb(
	struct i2c_algo_mcf_data *adap
) {
	int i;
	for (i = 0; i < adap->timeout; i++) {
		if (!(MCF_I2SR & MCF_I2SR_IBB))
			return 0;
		udelay(100);
	}
	printk(KERN_ERR "%s: timeout", __func__);
	return -ETIMEDOUT;
}

/*
 *  static void wait_for_not_bb()
 *
 *  Wait for bus busy state
 */
static int
wait_for_not_bb(
	struct i2c_algo_mcf_data *adap
) {
	int i;
	for (i = 0; i < adap->timeout; i++) {
		if (MCF_I2SR & MCF_I2SR_IBB)
			return 0;
		udelay(100);
	}
	printk(KERN_ERR "%s: timeout", __func__);
	return -ETIMEDOUT;
}

/*
 *  static void wait_xfer_done()
 *
 *  Wait for transfer to complete
 */
static int
wait_xfer_done(
	struct i2c_algo_mcf_data *adap
) {
	int i;

	for (i = 0; i < adap->timeout; i++) {
		if (MCF_I2SR & MCF_I2SR_IIF) {
			MCF_I2SR &= ~MCF_I2SR_IIF;
			return 0;
		}
		udelay(10);
	}
	printk(KERN_ERR "%s: timeout", __func__);
	return -ETIMEDOUT;
}


/*
 *  static void i2c_set_addr()
 *
 *  Sets slave address to communicate
 */
static int
i2c_set_addr(
	struct i2c_algo_mcf_data *adap,
	struct i2c_msg *msg,
	int retries
) {
	unsigned short flags = msg->flags;
	unsigned char addr;
	MCF_I2CR |= MCF_I2CR_MTX;
	if ((flags & I2C_M_TEN)) {
		/* 10 bit address not supported yet */
		return -EIO;
	} else {
		/* normal 7bit address */
		addr = (msg->addr << 1);
		if (flags & I2C_M_RD)
			addr |= 1;
		if (flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;

		MCF_I2DR = addr;
	}
	return 0;
}


/*
 *  static void mcf_i2c_init()
 *
 *  Perform ColdFire i2c initialization
 */
static void
mcf_i2c_init(struct i2c_algo_mcf_data *adap)
{
	u8 dummy;

	/* Setup GPIO lines */
#if defined(CONFIG_M547X_8X)
	MCF_PAR_FECI2CIRQ |= MCF_PAR_SDA;
	MCF_PAR_FECI2CIRQ |= MCF_PAR_SCL;
#elif defined(CONFIG_M5445X)
	MCF_GPIO_PAR_FECI2C |= (0
		| MCF_GPIO_PAR_FECI2C_PAR_SDA(3)
		| MCF_GPIO_PAR_FECI2C_PAR_SCL(3));
#elif defined(CONFIG_M5441X)
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_I2C0SCL_MASK) |
		MCF_GPIO_PAR_CANI2C_I2C0SCL_I2C0SCL;
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_I2C0SDA_MASK) |
		MCF_GPIO_PAR_CANI2C_I2C0SDA_I2C0SDA;
#endif

	/*  Ensure slaves are in idle state */
	if (MCF_I2SR & MCF_I2SR_IBB) {
#if defined(CONFIG_M547X_8X)
		MCF_I2ICR = 0x00;
		MCF_I2CR  = 0x00;
		MCF_I2CR  = 0x0A;
		dummy = MCF_I2DR;
		MCF_I2SR  = 0x00;
		MCF_I2CR  = 0x00;
		MCF_I2ICR = 0x01;
#elif defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
		MCF_I2CR = 0x00;
		MCF_I2CR = 0xA0;
		dummy = MCF_I2DR;
		MCF_I2SR = 0x00;
		MCF_I2CR = 0x00;
		MCF_I2CR = 0x80;
#endif
	}

	/* setup SCL clock */
	MCF_I2FDR = get_clock(adap);

	/* set slave address */
	MCF_I2AR = get_own(adap);

	/* enable I2C module */
#if	defined(CONFIG_M5441X)
	MCF_I2CR = (MCF_I2CR_IEN | MCF_I2CR_IIEN);
#else
	MCF_I2CR = MCF_I2CR_IEN;
#endif
}

static int i2c_outb(
	struct i2c_adapter *i2c_adap,
	char c
) {

	struct i2c_algo_mcf_data *adap = i2c_adap->algo_data;
	int timeout;
	/* Put data to be sent */
	MCF_I2DR = c;
	/* Wait for xfer completed*/
	timeout = wait_xfer_done(adap);
	if (timeout) {
		i2c_stop(adap);
		wait_for_bb(adap);
		printk(KERN_ERR "i2c-algo-mcf: %s i2c_write: "
			"error - timeout.\n", i2c_adap->name);
		return -EREMOTEIO; /* got a better one ?? */
	}

	return 0;
}


/*
 *  static void mcf_sendbytes()
 *
 *  Perform tx data transfer
 */
static int
mcf_sendbytes(
	struct i2c_adapter *i2c_adap,
	const char *buf,
	int count, int last
) {
	struct i2c_algo_mcf_data *adap = i2c_adap->algo_data;
	int ret, i;

	/* Set master TX mode */
	MCF_I2CR |= MCF_I2CR_MTX;

	for (i = 0; i < count; ++i) {
		printk(KERN_DEBUG "i2c-algo-mcf: %s i2c_write: writing %2.2X\n",
		      i2c_adap->name, buf[i]&0xff);
		ret = i2c_outb(i2c_adap, buf[i]);
		if (ret < 0)
			return ret;
	}
	if (last) {
		i2c_stop(adap);
		wait_for_bb(adap);
	} else {
	/*	i2c_repstart(adap);*/
	}

	return i;
}


/*
 *  static void mcf_readbytes()
 *
 *  Perform rx data transfer
 */
static int
mcf_readbytes(
	struct i2c_adapter *i2c_adap,
	char *buf,
	int count, int last
) {
	int i;
	struct i2c_algo_mcf_data *adap = i2c_adap->algo_data;
	u8 dummy;

	/* Set master RX mode */
	MCF_I2CR &= ~MCF_I2CR_MTX;
	MCF_I2CR &= ~MCF_I2CR_TXAK;
	dummy = MCF_I2DR;

	for (i = 0; i < count-1; i++) {
		if (wait_xfer_done(adap)) {
			i2c_stop(adap);
			wait_for_bb(adap);
			printk(KERN_DEBUG
			    "i2c-algo-mcf: mcf_readbytes timed out.\n");
			return -1;
		}

		/* store next data byte */
		buf[i] = MCF_I2DR;
	}

	if (wait_xfer_done(adap)) {
		i2c_stop(adap);
		wait_for_bb(adap);
		printk(KERN_DEBUG "i2c-algo-mcf: mcf_readbytes timed out.\n");
		return -1;
	}

	/* Disable acknowlege (set I2CR.TXAK) */
	MCF_I2CR |= MCF_I2CR_TXAK;
	buf[i] = MCF_I2DR;
	if (wait_xfer_done(adap)) {
		i2c_stop(adap);
		wait_for_bb(adap);
		printk(KERN_DEBUG "i2c-algo-mcf: mcf_readbytes timed out.\n");
		return -1;
	}

	if (last) {
		i2c_stop(adap);
		wait_for_bb(adap);
	} else {
	/*	i2c_repstart(adap);*/
	}

	return i+1;
}


/*
 *  static void mcf_xfer()
 *
 *  Perform master data I/O transfer
 */
static int
mcf_xfer(
	struct i2c_adapter *i2c_adap,
	struct i2c_msg *msgs,
	int num)
{
	struct i2c_algo_mcf_data *adap = i2c_adap->algo_data;
	struct i2c_msg *pmsg;
	int i;
	int ret = 0, timeout;

	/* Skip own address */
	if (get_own(adap) == (msgs[0].addr << 1))
		return -EIO;

	/*  Ensure slaves are in idle state */
	if (MCF_I2SR & MCF_I2SR_IBB) {
#if defined(CONFIG_M547X_8X)
		MCF_I2ICR = 0x00;
		MCF_I2CR  = 0x00;
		MCF_I2CR  = 0x0A;
		timeout = MCF_I2DR;
		MCF_I2SR  = 0x00;
		MCF_I2CR  = 0x00;
		MCF_I2ICR = 0x01;
#elif defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
		MCF_I2CR = 0x00;
		MCF_I2CR = 0xA0;
		timeout = MCF_I2DR;
		MCF_I2SR = 0x00;
		MCF_I2CR = 0x00;
		MCF_I2CR = 0x80;
#endif
	}

	/* setup SCL clock */
	MCF_I2FDR = get_clock(adap);
	/* set slave address */
	MCF_I2AR = get_own(adap);
	/* enable I2C module */
#if	defined(CONFIG_M5441X)
	MCF_I2CR = (MCF_I2CR_IEN | MCF_I2CR_IIEN);
#else
	MCF_I2CR = MCF_I2CR_IEN;
#endif
	MCF_I2CR |= MCF_I2CR_TXAK;

	/* Check for bus busy */
	wait_for_bb(adap);

	for (i = 0; ret >= 0 && i < num; i++) {
		if (MCF_I2SR & MCF_I2SR_IBB) {
#if defined(CONFIG_M547X_8X)
			MCF_I2ICR = 0x00;
			MCF_I2CR  = 0x00;
			MCF_I2CR  = 0x0A;
			timeout = MCF_I2DR;
			MCF_I2SR  = 0x00;
			MCF_I2CR  = 0x00;
			MCF_I2ICR = 0x01;
#elif defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
		MCF_I2CR = 0x00;
		MCF_I2CR = 0xA0;
		timeout = MCF_I2DR;
		MCF_I2SR = 0x00;
		MCF_I2CR = 0x00;
		MCF_I2CR = 0x80;
#endif
		}
		/* setup SCL clock */
		MCF_I2FDR = get_clock(adap);
		/* set slave address */
		MCF_I2AR = get_own(adap);
		/* enable I2C module */
#if		defined(CONFIG_M5441X)
		MCF_I2CR = (MCF_I2CR_IEN | MCF_I2CR_IIEN);
#else
		MCF_I2CR = MCF_I2CR_IEN;
#endif
		MCF_I2CR |= MCF_I2CR_TXAK;

		/* Check for bus busy */
		wait_for_bb(adap);

		pmsg = &msgs[i];

		printk(KERN_DEBUG "i2c-algo-mcf: Doing %s %d bytes "
			"to 0x%02x - %d of %d messages\n",
			pmsg->flags & I2C_M_RD ? "read" : "write",
			pmsg->len, pmsg->addr, i + 1, num);

		/* Send START */
		/*if (i == 0)*/
			i2c_start(adap);

		/* Wait for Bus Busy */
		wait_for_not_bb(adap);

		MCF_I2CR |= MCF_I2CR_MTX;

		ret = i2c_set_addr(adap, pmsg, i2c_adap->retries);
		if (ret < 0)
			return ret;

		/* Wait for address transfer completion */
		wait_xfer_done(adap);

		/* Check for ACK */
		if (!i2c_getack(adap)) {
			i2c_stop(adap);
			wait_for_bb(adap);
			printk(KERN_DEBUG "i2c-algo-mcf: No ack after "
				    "send address in mcf_xfer\n");
			return -EREMOTEIO;
		}

		printk(KERN_DEBUG "i2c-algo-mcf: Msg %d, "
				  "addr = 0x%x, flags = 0x%x, len = %d\n",
				i, msgs[i].addr, msgs[i].flags, msgs[i].len);
		/* Read */
		if (pmsg->flags & I2C_M_RD) {
			/* read bytes into buffer*/
			ret = mcf_readbytes(i2c_adap, pmsg->buf, pmsg->len,
						(i + 1 == num));

			if (ret != pmsg->len) {
				printk(KERN_DEBUG "i2c-algo-mcf: fail: "
					    "only read %d bytes.\n", ret);
			} else {
				printk(KERN_DEBUG "i2c-algo-mcf: "
						  "read %d bytes.\n", ret);
			}
		} else {
			/* write bytes into buffer*/
			ret = mcf_sendbytes(i2c_adap, pmsg->buf, pmsg->len,
						(i + 1 == num));
			if (ret != pmsg->len) {
				printk(KERN_DEBUG "i2c-algo-mcf: fail: "
					    "only wrote %d bytes.\n", ret);
			} else {
				printk(KERN_DEBUG "i2c-algo-mcf: wrote"
					"%d bytes.\n", ret);
			}
		}
	MCF_I2CR = 0;
	}

	/* Disable I2C module */
	MCF_I2CR = 0;
	return i;
}


/*
 *  static void mcf_func()
 *
 *  Return algorithm funtionality
 */
static u32
mcf_func(
	struct i2c_adapter *i2c_adap
) {
	return I2C_FUNC_SMBUS_EMUL | I2C_FUNC_I2C;
}

/*
 *  ColdFire bus algorithm callbacks
 */
static struct i2c_algorithm mcf_algo = {
	.master_xfer    = mcf_xfer,
	.functionality  = mcf_func,
};

/***********************************************************/
struct coldfire_i2c {
	void __iomem *base;
	struct resource *irqarea;
	struct resource *ioarea;
	u32 irq;
	struct i2c_adapter *adap;
	u32 flags;
};

/*
 *  registering functions to load algorithms at runtime
 */
int i2c_mcf_add_bus(struct i2c_adapter *adap)
{
	struct i2c_algo_mcf_data *mcf_adap = adap->algo_data;

	/*adap->id |= mcf_algo.id;*/
	adap->algo = &mcf_algo;
	adap->timeout = 100;

	mcf_i2c_init(mcf_adap);

#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
	i2c_add_numbered_adapter(adap);

	return 0;
}

static int mcf_i2c_probe(struct platform_device *pdev)
{
	struct coldfire_i2c *i2c;
	int rc = 0;

	/************************************************************/
	i2c = kzalloc(sizeof(*i2c), GFP_KERNEL);
	if (!i2c) {
		printk(KERN_ERR "%s kzalloc coldfire_i2c faile\n",
				__func__);
		return -ENOMEM;
	}
	/****************************************************************/
	platform_set_drvdata(pdev, i2c);

	i2c->adap = &i2c_mcf_board_adapter;
	i2c->adap->dev.parent = &pdev->dev;
	i2c->adap->nr = pdev->id;
	rc = i2c_mcf_add_bus(i2c->adap);
	if (rc < 0) {
		printk(KERN_ERR "%s - failed to add adapter\n", __func__);
		rc = -ENODEV;
		goto fail_add;
	}

	printk(KERN_INFO "i2c-algo-mcf.o: I2C ColdFire algorithm"
			" module is loaded.\n");
	return rc;

fail_add:
	kfree(i2c);
	return rc;
};

static int mcf_i2c_remove(struct platform_device *pdev)
{
	struct coldfire_i2c *i2c = platform_get_drvdata(pdev);

	i2c_del_adapter(i2c->adap);
	platform_set_drvdata(pdev, NULL);
	iounmap(i2c->base);
	kfree(i2c);
	return 0;
};

/* Structure for a device driver */
static struct platform_driver mcf_i2c_driver = {
	.probe 	= mcf_i2c_probe,
	.remove = mcf_i2c_remove,
	.driver	= {
		.owner = THIS_MODULE,
		.name = "mcf-i2c",
	},
};

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
	int len = gen_i2c_proc_output(page);
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

static int __init coldfire_i2c_init(void)
{
	int retval;

	retval = gen_i2c_proc_init();
	if (retval < 0)
		printk(KERN_INFO "generate /proc/i2c-adaptor-register "
                                 "for i2c master mode failed!\n");

	return platform_driver_register(&mcf_i2c_driver);
}

static void __exit coldfire_i2c_exit(void)
{
	platform_driver_unregister(&mcf_i2c_driver);
}

module_init(coldfire_i2c_init);
module_exit(coldfire_i2c_exit);

MODULE_AUTHOR("Adrian Cox <adrian@humboldt.co.uk>");
MODULE_DESCRIPTION
    ("I2C-Bus adapter for MCFV4/MCFV4E processors");
MODULE_LICENSE("GPL");
