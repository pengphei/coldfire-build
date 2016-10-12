/*
 * drivers/watchdog/mcf_wdt.c
 *
 * Watchdog driver for ColdFire processors
 *
 * Copyright 2006-2007, 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * Author: Shrek Wu<B16972@freesale.com>
 * 	   ChengJu Cai<B22600@freesale.com>
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/bitops.h>

#include <linux/uaccess.h>
#include <asm/coldfire.h>
#include <asm/mcfsim.h>

static int nowayout;
static unsigned int heartbeat = MCF_GPT_MAX_TIMEOUT;
static unsigned long wdt_status;

#define	WDT_IN_USE		0
#define	WDT_OK_TO_CLOSE		1

static unsigned long wdt_tick_rate;

#ifdef CONFIG_M547X_8X
static int
wdt_enable(int time)
{
	if (time > 30 || time < 1)
		return -EINVAL;

	heartbeat = time;

	MCF_GPT_GMS0 = 0;
	MCF_GPT_GCIR0 =  MCF_GPT_GCIR_PRE(heartbeat * wdt_tick_rate) |
				MCF_GPT_GCIR_CNT(0xffff);
	MCF_GPT_GMS0 = MCF_GPT_GMS_OCPW(0xA5) | MCF_GPT_GMS_WDEN |
				MCF_GPT_GMS_CE | MCF_GPT_GMS_TMS_GPIO;

	return 0;
}

static void
wdt_disable(void)
{
	MCF_GPT_GMS0 = 0;
}

static void
wdt_keepalive(void)
{
	MCF_GPT_GMS0 = MCF_GPT_GMS_OCPW(0xA5) | MCF_GPT_GMS0;
}

#elif defined(CONFIG_M5445X) || defined(CONFIG_M5441X)

/* Enable watchdog and set time-out */
static int
wdt_enable(int time)
{
	unsigned int sign = 0x01 << 31;
	int i = 0, timeout_n = 31;
	int max_timeout = sign / MCF_BUSCLK;
	unsigned int count;

	if (time > max_timeout || time < 1)
		return -EINVAL;

	count = time * MCF_BUSCLK;

	for (i = 0; i < 31; i++) {
		if (count & (sign >> i)) {
			if ((count & (~(sign >> i))) == 0)
				timeout_n = 31 - i;
			else
				timeout_n = 31 - i + 1;
			break;
		}
	}

	heartbeat = (unsigned int)(0x01 << timeout_n) / MCF_BUSCLK;

	MCF_SCM_CWCR = MCF_SCM_CWCR_CWE
			| MCF_SCM_CWCR_CWRI(0x02)
			| MCF_SCM_CWCR_CWT(timeout_n);

	return 0;
}

/* Disable the watchdog */
static void
wdt_disable(void)
{
	MCF_SCM_CWCR = 0x00;
}

/* Reset the watchdog timer counter */
static void
wdt_keepalive(void)
{
	MCF_SCM_CWSR = 0x55;
	MCF_SCM_CWSR = 0xAA;
}
#endif

static int
mcf_wdt_open(struct inode *inode, struct file *file)
{
	int ret;
	if (test_and_set_bit(WDT_IN_USE, &wdt_status))
		return -EBUSY;

	clear_bit(WDT_OK_TO_CLOSE, &wdt_status);

	ret = wdt_enable(heartbeat);
	if (ret)
		return ret;

	return nonseekable_open(inode, file);
}

static ssize_t
mcf_wdt_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	if (len) {
		if (!nowayout) {
			size_t i;

			clear_bit(WDT_OK_TO_CLOSE, &wdt_status);

			for (i = 0; i != len; i++) {
				char c;

				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					set_bit(WDT_OK_TO_CLOSE, &wdt_status);
			}
		}
		wdt_keepalive();
	}

	return len;
}


static struct watchdog_info ident = {
	.options	= WDIOF_MAGICCLOSE | WDIOF_SETTIMEOUT |
				WDIOF_KEEPALIVEPING,
	.identity	= "Coldfire Watchdog",
};

static int
mcf_wdt_ioctl(struct inode *inode, struct file *file, unsigned int cmd,\
		unsigned long __user arg)
{
	int ret = -ENOIOCTLCMD;
	int time = 0;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		ret = copy_to_user((struct watchdog_info *)arg, &ident,
				   sizeof(ident)) ? -EFAULT : 0;
		break;

	case WDIOC_GETSTATUS:
		ret = put_user(0, (int *)arg);
		break;

	case WDIOC_GETBOOTSTATUS:
		ret = put_user(0, (int *)arg);
		break;

	case WDIOC_SETTIMEOUT:
		ret = get_user(time, (int *)arg);
		if (ret)
			break;

		ret = wdt_enable(time);
		if (ret)
			break;
		ret = put_user(heartbeat, (int *)arg);
		break;

	case WDIOC_GETTIMEOUT:
		ret = put_user(heartbeat, (int *)arg);
		break;

	case WDIOC_KEEPALIVE:
		wdt_keepalive();
		ret = 0;
		break;
	}

	return ret;
}

static int
mcf_wdt_release(struct inode *inode, struct file *file)
{
	if (test_bit(WDT_OK_TO_CLOSE, &wdt_status)) {
		wdt_keepalive();
	} else {
		printk(KERN_CRIT "WATCHDOG: Device closed unexpectdly\n");
		wdt_disable();
	}

	clear_bit(WDT_IN_USE, &wdt_status);
	clear_bit(WDT_OK_TO_CLOSE, &wdt_status);

	return 0;
}


static const struct file_operations mcf_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= mcf_wdt_write,
	.ioctl		= mcf_wdt_ioctl,
	.open		= mcf_wdt_open,
	.release	= mcf_wdt_release,
};

static struct miscdevice mcf_wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &mcf_wdt_fops,
};

static int __init mcf_wdt_init(void)
{
	wdt_tick_rate = MCF_BUSCLK/0xffff;
#ifdef CONFIG_WATCHDOG_NOWAYOUT
	nowayout = 1;
#else
	nowayout = 0;
#endif
	printk("ColdFire watchdog driver is loaded.\n");

	return misc_register(&mcf_wdt_miscdev);
}

static void __exit mcf_wdt_exit(void)
{
	misc_deregister(&mcf_wdt_miscdev);
}

module_init(mcf_wdt_init);
module_exit(mcf_wdt_exit);

MODULE_AUTHOR("Deepak Saxena");
MODULE_DESCRIPTION("ColdFire Watchdog");

module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat, "Watchdog heartbeat in seconds (default 60s)");

module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started");

MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);

