/*
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Author: Huan Wang, b18965@freescale.com, Fri Aug 08 2008
 *
 * Description:
 * CAN bus driver for Freescale Coldfire embedded CPU
 *
 * Changelog:
 * Fri Aug 08 2008 Huan Wang <b18965@freescale.com>
 * - create, support for MCF548x
 *
 * Tue Dec 08 2009 ChengJu Cai <b22600@freescale.com>
 * - support for MCF532x MCF5253 MCF5227x
 *
 * This file is part of the Linux kernel
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>
#include <linux/can.h>
#include <linux/io.h>
#include <linux/can/dev.h>
#include <asm/mcfsim.h>

#include "flexcan.h"

#undef DEBUG
#ifdef DEBUG
# define DBG(x...) printk(x)
#else
# define DBG(x...)
#endif

static int __devinit mcf_flexcan_probe(struct platform_device *pdev)
{
	struct resource *mem;
	struct net_device *dev;
	struct flexcan_platform_data *pdata = pdev->dev.platform_data;
	struct can_priv *can;
	u32 mem_size;
	int ret = -ENODEV;

	if (!pdata)
		return ret;

	dev = alloc_flexcandev();
	if (!dev)
		return -ENOMEM;
	can = netdev_priv(dev);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	dev->irq = platform_get_irq(pdev, 0);
	if (!mem || !dev->irq)
		goto req_error;

	mem_size = mem->end - mem->start + 1;
	if (!request_mem_region(mem->start, mem_size, pdev->dev.driver->name)) {
		dev_err(&pdev->dev, "resource unavailable\n");
		goto req_error;
	}
	SET_NETDEV_DEV(dev, &pdev->dev);

	dev->base_addr = (unsigned long)ioremap_nocache(mem->start, mem_size);
	if (!dev->base_addr) {
		dev_err(&pdev->dev, "failed to map can port\n");
		ret = -ENOMEM;
		goto fail_map;
	}
	can->bittiming.clock = pdata->clock_frq;

	/* set default baud rate to 500K */
	can->bittiming.bitrate = 500000;

	platform_set_drvdata(pdev, dev);
	ret = register_flexcandev(dev, pdata->clock_src);
	if (ret >= 0) {
		dev_info(&pdev->dev, "probing 0x%lX done\n", dev->base_addr);
		return ret;
	}
	iounmap((unsigned long *)dev->base_addr);
fail_map:
	release_mem_region(mem->start, mem_size);
req_error:
	free_candev(dev);
	dev_err(&pdev->dev, "probe failed\n");
	return ret;
}

static int __devexit mcf_flexcan_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct resource *mem;
	DBG(KERN_INFO "Removing the Module\n");
	platform_set_drvdata(pdev, NULL);
	unregister_flexcandev(dev);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(mem->start, mem->end - mem->start + 1);

	free_candev(dev);
	return 0;
 }

void mcf_flexcan_release(struct device *dev)
{
	/* do nothing */
}

static struct platform_driver mcf_flexcan_driver = {
	.driver = {
		.name = "mcf-flexcan",
	},
	.probe = mcf_flexcan_probe,
	.remove = __devexit_p(mcf_flexcan_remove),
 };

#if defined(CONFIG_M548X)
static struct flexcan_platform_data m548x_can_platform = {
	.clock_src = 1,
	.clock_frq = 100000000,
};

static struct resource mcf548x_can0_resources[] = {
	[0] = {
		.start 		= MCF_MBAR + 0x0000A000,
		.end		= MCF_MBAR + 0x0000A7FF,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 49 + 64,
		.end		= 49 + 64,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct resource mcf548x_can1_resources[] = {
	[0] = {
		.start 		= MCF_MBAR + 0x0000A800,
		.end		= MCF_MBAR + 0x0000AFFF,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 55 + 64,
		.end		= 55 + 64,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device mcf_flexcan[PDEV_MAX] = {
	[0] = {
		.name                   = "mcf-flexcan",
		.id                     = 0,
		.num_resources          = ARRAY_SIZE(mcf548x_can1_resources),
		.resource               = mcf548x_can0_resources,
		.dev.platform_data      = &m548x_can_platform,
		.dev.release		= mcf_flexcan_release,
	},
	[1] = {
		.name                   = "mcf-flexcan",
		.id                     = 1,
		.num_resources          = ARRAY_SIZE(mcf548x_can1_resources),
		.resource               = mcf548x_can1_resources,
		.dev.platform_data      = &m548x_can_platform,
		.dev.release		= mcf_flexcan_release,
	},

};

#elif defined(CONFIG_M5441X)
static struct flexcan_platform_data m5441x_can_platform = {
	.clock_src = 1,
	.clock_frq = 125000000,
};

static struct resource mcf5441x_can0_resources[] = {
	[0] = {
		.start 		= 0xFC020000,
		.end		= 0xFC0208C0,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 0 + 64 + 64,
		.end		= 0 + 64 + 64,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct resource mcf5441x_can1_resources[] = {
	[0] = {
		.start 		= 0xFC024000,
		.end		= 0xFC0248C0,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= 4 + 64 + 64,
		.end		= 4 + 64 + 64,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device mcf_flexcan[PDEV_MAX] = {
	[0] = {
		.name                   = "mcf-flexcan",
		.id                     = 0,
		.num_resources          = ARRAY_SIZE(mcf5441x_can0_resources),
		.resource               = mcf5441x_can0_resources,
		.dev.platform_data      = &m5441x_can_platform,
		.dev.release		= mcf_flexcan_release,
	},
	[1] = {
		.name                   = "mcf-flexcan",
		.id                     = 1,
		.num_resources          = ARRAY_SIZE(mcf5441x_can1_resources),
		.resource               = mcf5441x_can1_resources,
		.dev.platform_data      = &m5441x_can_platform,
		.dev.release		= mcf_flexcan_release,
	},

};

#elif defined(CONFIG_M532x)
static struct flexcan_platform_data m532x_flexcan_platform = {
	.clock_src = 1,
	.clock_frq = 80000000,
};

static struct resource mcf532x_flexcan0_resources[] = {
	[0] = {
		.name  = "flexcan0",
		.start = 0xfc020000,
		.end   = 0xfc020fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "flexcan0_irq",
		.start = 128,
		.end   = 128,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device mcf_flexcan[PDEV_MAX] = {
	[0] = {
		.name		   = "mcf-flexcan",
		.id		   = 0,
		.num_resources	   = ARRAY_SIZE(mcf532x_flexcan0_resources),
		.resource	   = mcf532x_flexcan0_resources,
		.dev.platform_data = &m532x_flexcan_platform,
		.dev.release	   = mcf_flexcan_release,
	},

};
#elif defined(CONFIG_M5227x)
static struct flexcan_platform_data m5227x_flexcan_platform = {
	.clock_src = 1,
	.clock_frq = 80000000,
};

static struct resource mcf5227x_flexcan0_resources[] = {
	[0] = {
		.name  = "flexcan0",
		.start = 0xfc020000,
		.end   = 0xfc020fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "flexcan0_irq",
		.start = 128,
		.end   = 128,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device mcf_flexcan[PDEV_MAX] = {
	[0] = {
		.name		   = "mcf-flexcan",
		.id		   = 0,
		.num_resources 	   = ARRAY_SIZE(mcf5227x_flexcan0_resources),
		.resource 	   = mcf5227x_flexcan0_resources,
		.dev.platform_data = &m5227x_flexcan_platform,
		.dev.release	   = mcf_flexcan_release,
	},
};
#elif defined(CONFIG_M5253)
static struct flexcan_platform_data m5253_flexcan_platform = {
	.clock_src = 1,
	.clock_frq = 70000000,
};

static struct resource mcf5253_flexcan0_resources[] = {
	[0] = {
		.name  = "flexcan0",
		.start = 0x80001000,
		.end   = 0x80001fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "flexcan0_irq",
		.start = 142,
		.end   = 142,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource mcf5253_flexcan1_resources[] = {
	[0] = {
		.name  = "flexcan1",
		.start = 0x80002000,
		.end   = 0x80002fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "flexcan1_irq",
		.name  = "can1_irq",
		.start = 140,
		.end   = 140,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device mcf_flexcan[] = {
	[0] = {
		.name			= "mcf-flexcan",
		.id			= 0,
		.num_resources		= ARRAY_SIZE\
					  (mcf5253_flexcan0_resources),
		.resource		= mcf5253_flexcan0_resources,
		.dev.platform_data	= &m5253_flexcan_platform,
		.dev.release		= mcf_flexcan_release,
	},
	[1] = {
		.name			= "mcf-flexcan",
		.id			=  1,
		.num_resources		= ARRAY_SIZE\
					  (mcf5253_flexcan1_resources),
		.resource		= mcf5253_flexcan1_resources,
		.dev.platform_data	= &m5253_flexcan_platform,
		.dev.release		= mcf_flexcan_release,
	},
};
#endif

static int __init flexcan_of_to_pdev(void)
{
	int i, err = -ENODEV;
	for (i = 0; i < PDEV_MAX; i++) {
		err = platform_device_register(&mcf_flexcan[i]);
		if (err)
			return err;
		DBG(KERN_INFO "%s loaded\n", mcf_flexcan_driver.driver.name);
	}
	return err;
}

 int __init mcf_flexcan_init(void)
 {
#if	defined (CONFIG_M5441X)
	MCF_PM_PPMCR0 = 8;	/* enable FlexCAN0 clock */
	MCF_PM_PPMCR0 = 9;	/* enable FlexCAN1 clock */
#endif
	int err = flexcan_of_to_pdev();

	if (err) {
		printk(KERN_ERR "%s init failed with err=%d\n",
			mcf_flexcan_driver.driver.name, err);
		return err;
	}

	return platform_driver_register(&mcf_flexcan_driver);
 }

 void __exit mcf_flexcan_exit(void)
 {
	int i;
	platform_driver_unregister(&mcf_flexcan_driver);
	for (i = 0; i < PDEV_MAX; i++) {
		platform_device_unregister(&mcf_flexcan[i]);
		DBG(KERN_INFO "%s unloaded\n", mcf_flexcan_driver.driver.name);
	}
 }

 module_init(mcf_flexcan_init);
 module_exit(mcf_flexcan_exit);

 MODULE_AUTHOR("Wang Huan<b18965@freescale.com>");
 MODULE_DESCRIPTION("Freescale MCF CAN driver");
 MODULE_LICENSE("GPL v2");
