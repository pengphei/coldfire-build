/*
 * Coldfire M5441x Platform Device Configuration
 *
 * Copyright(C) 2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Lanttor.Guo@freescale.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/mtd/partitions.h>
#ifdef CONFIG_MMC_SPI
#include <linux/mmc/host.h>
#include <linux/spi/mmc_spi.h>
#endif

#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/mcfuart.h>
#include <asm/mcfqspi.h>
#include <asm/mcfdspi.h>
#include <asm/cf_io.h>

/*
 * I2C: only support i2c0 module on m5441x platform
 */
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
static struct resource coldfire_i2c_resources[] = {
	{		/* I/O */
		.start		= 0xFC058000,
		.end		= 0xFC058010,
		.flags		= IORESOURCE_MEM,
	},
	{		/* IRQ */
		.start		= (64 + 30),
		.end		= (64 + 30),
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device coldfire_i2c_device = {
	.name			= "mcf-i2c",
	.id			= 0,	/*bus number*/
	.num_resources		= ARRAY_SIZE(coldfire_i2c_resources),
	.resource		= coldfire_i2c_resources,
};

static void mcf5441x_init_i2c(void)
{
	MCF_PM_PPMCR0 = 22;
	platform_device_register(&coldfire_i2c_device);
}
#endif

#if defined(CONFIG_MMC_ESDHC)
static struct resource sd_mmc_esdhc_resources[] = {
	[0] = {
		.name = "esdhc-module",
		.start  = 0xFC0CC000,
		.end    = 0xFC0CC000 + 0x100,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.name = "esdhc-int-level",
		.start  = (64 + 64 + 64 + 31),
		.end    = (64 + 64 + 64 + 31),
		.flags  = IORESOURCE_IRQ,
	},
	[2] = {
		.name = "esdhc-pin",
		.start  = &MCF_GPIO_PAR_ESDHCH,
		.end    = &MCF_GPIO_PAR_ESDHCL,
		.flags  = IORESOURCE_MEM,
	},
	[3] = {
		.name = "esdhc-slew",
		.start  = &MCF_GPIO_SRCR_SDHC,
		.end    = &MCF_GPIO_SRCR_SDHC,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device sd_mmc_esdhc_device = {
	.name           = "esdhc",
	.id             = -1,
	.resource       = sd_mmc_esdhc_resources,
	.num_resources  = ARRAY_SIZE(sd_mmc_esdhc_resources),
};
#endif

#if defined(CONFIG_MTD_NAND_FSL_NFC)
static struct resource nfc_resources[] = {
	[0] = {
		.name	= "nfc-config",
		.start	= 0xFC0FC000,
		.end	= 0xFC0FFF3B,
		.flags	= IORESOURCE_MEM,

	},
	[1] = {
		.name	= "nfc-int-level",
		.start  = (64 + 64 + 64 + 25),
		.end    = (64 + 64 + 64 + 25),
		.flags  = IORESOURCE_IRQ,
	},

};
static struct platform_device nfc_device = {
	.name           = "fsl_nfc",
	.id             = -1,
	.resource       = nfc_resources,
	.num_resources  = ARRAY_SIZE(nfc_resources),
};
#endif

/*
 * DSPI
 */
#ifdef CONFIG_SPI

#if defined(CONFIG_DSPI0)	/* DSPI0 initialization */
#define M5441X_DSPI0_MCR	0xFC05C000	/* base */

#define M5441X_DSPI0_IRQ_SOURCE	(31)
#define M5441X_DSPI0_IRQ_VECTOR	(64 + M5441X_DSPI0_IRQ_SOURCE)

/* number of supported SPI selects */
#define SPI_NUM_CHIPSELECTS	8

#define M5441X_SPI0_PAR_VAL     (MCF_GPIO_PAR_DSPI0_SCK_DSPI0SCK        | \
				MCF_GPIO_PAR_DSPI0_SOUT_DSPI0SOUT      | \
				MCF_GPIO_PAR_DSPI0_SIN_DSPI0SIN        | \
				MCF_GPIO_PAR_DSPI0_PCS0_DSPI0PCS0)


#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition at26df081a_partitions[] = {
	{
		.name = "at26df081a",
		.size = (1024*64*16),
		.offset = 0x00000000,
		.mask_flags = 0,
	}
};
#endif

#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
static struct flash_platform_data at26df081a_platform_data = {
	.name = "Atmel at26df081a SPI Flash chip",
#ifdef CONFIG_MTD_PARTITIONS
	.parts = at26df081a_partitions,
	.nr_parts = sizeof(at26df081a_partitions)/sizeof(*at26df081a_partitions),
#endif
	.type = "at26df081a"
};

static struct coldfire_dspi_chip at26df081a_chip_info = {
	.mode = SPI_MODE_0,
	.bits_per_word = 16,	/* How many bits per word to transfer
				   per one time (8 or 16) */
	.void_write_data = 0,
	.dbr = 0,
	.pbr = 0,
	.br = 0,
	.pcssck = 1,
	.pasc = 1,
	.pdt = 1,
	.cssck = 4,
	.asc = 4,
	.dt = 14,
};
#endif

#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
static struct coldfire_dspi_chip spidev_chip_info = {
	.bits_per_word = 8,
};
#endif


static struct coldfire_spi_master spi0_master_info = {
	.bus_num = 1,
	.num_chipselect = SPI_NUM_CHIPSELECTS,
	.irq_list = 0,		/* not used */
	.irq_source = M5441X_DSPI0_IRQ_SOURCE,
	.irq_vector = M5441X_DSPI0_IRQ_VECTOR,
	.irq_mask = (1 << M5441X_DSPI0_IRQ_SOURCE),
	.irq_lp = 0x2,		/* irq level */
	.par_val = M5441X_SPI0_PAR_VAL,
	.cs_control = NULL,
};

static struct resource coldfire_spi0_resources[] = {
	[0] = {
		.name = "spi-par",
		.start = (u32)&MCF_GPIO_PAR_DSPI0WH,	/* PAR_DSPI0 */
		.end = (u32)&MCF_GPIO_PAR_DSPI0WH,	/* PAR_DSPI0 */
		.flags = IORESOURCE_MEM
	},

	[1] = {
		.name = "spi-module",
		.start = M5441X_DSPI0_MCR,	/* DSPI MCR Base */
		.end = M5441X_DSPI0_MCR + 0xc0,	/* DSPI mem map end */
		.flags = IORESOURCE_MEM
	},

	[2] = {
		.name = "spi-int-level",
		.start = (u32)&MCF_INTC0_ICR31,	/* ICR start */
		.end = (u32)&MCF_INTC0_ICR31, 	/* ICR end */
		.flags = IORESOURCE_MEM
	},

	[3] = {
		.name = "spi-int-mask",
		.start = (u32)&MCF_INTC0_IMRL,	/* IMRL */
		.end = (u32)&MCF_INTC0_IMRL,		/* IMRL */
		.flags = IORESOURCE_MEM
	}
};

static struct platform_device coldfire_spi0 = {
	.name = "spi_coldfire",
	.id = -1,
	.resource = coldfire_spi0_resources,
	.num_resources = ARRAY_SIZE(coldfire_spi0_resources),
	.dev = {
		.platform_data = &spi0_master_info,
	}
};

#elif defined(CONFIG_DSPI1)	/* DSPI1 initialization */
#define M5441X_DSPI1_MCR	0xFC03C000	/* base */

#define M5441X_DSPI1_IRQ_SOURCE	(54)
#define M5441X_DSPI1_IRQ_VECTOR	(64 + 64 + M5441X_DSPI1_IRQ_SOURCE)

/* number of supported SPI selects */
#define SPI_NUM_CHIPSELECTS	8


#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
static struct coldfire_dspi_chip spidev_chip_info = {
	.bits_per_word = 8,
};
#endif

static struct coldfire_spi_master spi1_master_info = {
	.bus_num = 1,
	.num_chipselect = SPI_NUM_CHIPSELECTS,
	.irq_list = 0,		/* not used */
	.irq_source = M5441X_DSPI1_IRQ_SOURCE,
	.irq_vector = M5441X_DSPI1_IRQ_VECTOR,
	.irq_mask = (1 << (M5441X_DSPI1_IRQ_SOURCE - 32)),
	.irq_lp = 0x2,		/* irq level */
	.cs_control = NULL,
};

static struct resource coldfire_spi1_resources[] = {
	[0] = {
		.name = "spi-par",
		.start = (u32)&MCF_GPIO_PAR_ESDHCH,	/* PAR_ESDHCH */
		.end = (u32)&MCF_GPIO_PAR_ESDHCL,	/* PAR_ESDHCL */
		.flags = IORESOURCE_MEM
	},

	[1] = {
		.name = "spi-module",
		.start = M5441X_DSPI1_MCR,	/* DSPI MCR Base */
		.end = M5441X_DSPI1_MCR + 0xc0,	/* DSPI mem map end */
		.flags = IORESOURCE_MEM
	},

	[2] = {
		.name = "spi-int-level",
		.start = (u32)&MCF_INTC1_ICR54,	/* ICR start */
		.end = (u32)&MCF_INTC1_ICR54, 	/* ICR end */
		.flags = IORESOURCE_MEM
	},

	[3] = {
		.name = "spi-int-mask",
		.start = (u32)&MCF_INTC1_IMRH,	/* IMRL */
		.end = (u32)&MCF_INTC1_IMRH,	/* IMRL */
		.flags = IORESOURCE_MEM
	}
};

static struct platform_device coldfire_spi1 = {
	.name = "spi_coldfire",
	.id = -1,
	.resource = coldfire_spi1_resources,
	.num_resources = ARRAY_SIZE(coldfire_spi1_resources),
	.dev = {
		.platform_data = &spi1_master_info,
	}
};

#ifdef CONFIG_MMC_SPI
static struct coldfire_dspi_chip dspi_sd_chip_info = {
	.mode = SPI_MODE_0,
	.bits_per_word = 8,
	.void_write_data = 0xff,
	.dbr = 0,
	.pbr = 0,
	.br = 1,
	.pcssck = 2,
	.pasc = 2,
	.pdt = 2,
	.cssck = 5,
	.asc = 5,
	.dt = 5,
};

int mmc_spi_init(struct device *mmc_spi_device,
	irqreturn_t mmc_spi_irq_handler(int irq, void *private),
	void *irq_privatedata)
{
	int ret;
#if defined(CONFIG_DETECT_USE_EXTERN_IRQ1)
	int card_detect_extern_irq = 64 + 1;
	/*this is irq1 hardware work round*/
	MCF_GPIO_PAR_IRQ0H |= 0x3;

	MCF_EPORT_EPPAR   = MCF_EPORT_EPPAR | MCF_EPORT_EPPAR_EPPA1_BOTH;
	MCF_EPORT_EPIER   = MCF_EPORT_EPIER | MCF_EPORT_EPIER_EPIE1;

	MCF_INTC0_ICR1  = 7;           /* IRQ1 */
#if debug_mmc_spi
	printk(KERN_INFO "MCF_INTC0_ICR1 %x MCF_EPORT_EPPAR %x "
		"MCF_EPORT_EPFR %x MCF_EPORT_EPIER %x "
		"MCF_INTC0_IMRL %x MCF_INTC0_INTFRCL %x "
		"MCF_INTC0_IPRL %x\n",
		MCF_INTC0_ICR1, MCF_EPORT_EPPAR, MCF_EPORT_EPFR,
		MCF_EPORT_EPIER, MCF_INTC0_IMRL, MCF_INTC0_INTFRCL,
		MCF_INTC0_IPRL);
#endif
#elif defined(CONFIG_DETECT_USE_EXTERN_IRQ7)
	int card_detect_extern_irq = 64 + 7;
	MCF_GPIO_PAR_IRQ0H |= MCF_GPIO_PAR_IRQH_IRQ7;

	MCF_EPORT_EPPAR   = MCF_EPORT_EPPAR | MCF_EPORT_EPPAR_EPPA7_BOTH;
	MCF_EPORT_EPIER   = MCF_EPORT_EPIER | MCF_EPORT_EPIER_EPIE7;

	MCF_INTC0_ICR7  = 2;           /* IRQ7 */
#ifdef debug_mmc_spi
	printk("MCF_INTC0_ICR7 %x MCF_EPORT_EPPAR %x\n",
		MCF_INTC0_ICR7, MCF_EPORT_EPPAR);
#endif
#else
	int card_detect_extern_irq = 64 + 7;
	MCF_GPIO_PAR_IRQ0H |= MCF_GPIO_PAR_IRQH_IRQ7;

	MCF_EPORT_EPPAR   = MCF_EPORT_EPPAR | MCF_EPORT_EPPAR_EPPA7_BOTH;
	MCF_EPORT_EPIER   = MCF_EPORT_EPIER | MCF_EPORT_EPIER_EPIE7;

	MCF_INTC0_ICR7  = 2;           /* IRQ7 */
#ifdef debug_mmc_spi
	printk(KERN_INFO "MCF_INTC0_ICR1 %x MCF_EPORT_EPPAR %x\n",
		MCF_INTC0_ICR7, MCF_EPORT_EPPAR);
#endif
#endif

	ret = request_irq(card_detect_extern_irq,
			mmc_spi_irq_handler, IRQF_DISABLED,
			"mmc_spi_irq", irq_privatedata);
	if (ret) {
		printk(KERN_INFO "%s: request irq fail %x\n", __func__, ret);
		return -EBUSY;
	}

	return 0;
}

static struct mmc_spi_platform_data mcf5441x_mmc_pdata = {
	.ocr_mask = MMC_VDD_33_34,
	.init = mmc_spi_init,
};
#endif

#endif

/* DSPI device */
static struct spi_board_info spi_board_info[] = {

#if defined(CONFIG_DSPI1) && defined(CONFIG_MMC_SPI)
	{
		.modalias = "mmc_spi",
		.max_speed_hz = 50000000,
		.bus_num = 1,
		.chip_select = 0,
		.platform_data = &mcf5441x_mmc_pdata,
		.controller_data = &dspi_sd_chip_info,
	},
#endif
#if defined(CONFIG_DSPI0) && (defined(CONFIG_MTD_M25P80) || \
    defined(CONFIG_MTD_M25P80_MODULE))
	{
		.modalias = "m25p80", /* Name of spi driver for this device */
		.max_speed_hz = 70000000, /* max spi clock in HZ */
		.bus_num = 1,
		.chip_select = CONFIG_DSP0_SBF_CS,
		.platform_data = &at26df081a_platform_data,
		.controller_data = &at26df081a_chip_info
	},
#endif
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
	{
		.modalias = "spidev",
		.max_speed_hz = 16000000, /* max clk (SCK) speed in HZ */
		.bus_num = 1,
		.chip_select = 0,		/* CS0 */
		.controller_data = &spidev_chip_info,
	},
#endif
};

/*
 * m5441x_spi_init - Initialize SPI
 */
static int __init m5441x_spi_init(void)
{
	int retval;

	/* register device */
#if defined(CONFIG_DSPI0)
	MCF_GPIO_PAR_DSPI0WH =
		(MCF_GPIO_PAR_DSPI0WH & MCF_GPIO_PAR_DSPI0_SCK_MASK) |
		MCF_GPIO_PAR_DSPI0_SCK_DSPI0SCK;
	MCF_GPIO_PAR_DSPI0WH =
		(MCF_GPIO_PAR_DSPI0WH & MCF_GPIO_PAR_DSPI0_SOUT_MASK) |
		MCF_GPIO_PAR_DSPI0_SOUT_DSPI0SOUT;
	MCF_GPIO_PAR_DSPI0WH =
		(MCF_GPIO_PAR_DSPI0WH & MCF_GPIO_PAR_DSPI0_SIN_MASK) |
		MCF_GPIO_PAR_DSPI0_SIN_DSPI0SIN;
	MCF_GPIO_PAR_DSPI0WH =
		(MCF_GPIO_PAR_DSPI0WH & MCF_GPIO_PAR_DSPI0_PCS0_MASK) |
		MCF_GPIO_PAR_DSPI0_PCS0_DSPI0PCS0;
	MCF_GPIO_PAR_DSPI0WL = 0x80;
	retval = platform_device_register(&coldfire_spi0);
	if (retval < 0)
		goto out;
#elif defined(CONFIG_DSPI1)
	MCF_PM_PPMCR0 = 0xf;
	MCF_GPIO_PAR_ESDHCH = 0x55;     /* DAT[3:0] */
	MCF_GPIO_PAR_ESDHCL = 0x05;     /* CMD, CLK */
	MCF_GPIO_SRCR_IRQ0 = 3;
	MCF_GPIO_SRCR_SDHC = 3;
	retval = platform_device_register(&coldfire_spi1);
	if (retval < 0)
		goto out;
#endif

	/* register board info */
	if (ARRAY_SIZE(spi_board_info))
		retval = spi_register_board_info(spi_board_info,
						 ARRAY_SIZE(spi_board_info));

out:
	return retval;
}
#endif

#if defined (CONFIG_SERIAL_MCF) || defined(CONFIG_SERIAL_MCF_MODULE)
/*
 * UART initialization
 */
static struct mcf_platform_uart m5441x_uart_platform[] = {
#ifdef CONFIG_SERIAL_MCF_UART0
	{
		.mapbase	= MCFUART_BASE0,
		.irq		= MCFINT0_VECBASE + MCFINT_UART0,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART1
	{
		.mapbase	= MCFUART_BASE1,
		.irq		= MCFINT0_VECBASE + MCFINT_UART1,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART2
	{
		.mapbase	= MCFUART_BASE2,
		.irq		= MCFINT0_VECBASE + MCFINT_UART2,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART3
	{
		.mapbase	= MCFUART_BASE3,
		.irq		= MCFINT0_VECBASE + MCFINT_UART3,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART4
	{
		.mapbase	= MCFUART_BASE4,
		.irq		= MCFINT1_VECBASE + MCFINT_UART4,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART5
	{
		.mapbase	= MCFUART_BASE5,
		.irq		= MCFINT1_VECBASE + MCFINT_UART5,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART6
	{
		.mapbase	= MCFUART_BASE6,
		.irq		= MCFINT1_VECBASE + MCFINT_UART6,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART7
	{
		.mapbase	= MCFUART_BASE7,
		.irq		= MCFINT1_VECBASE + MCFINT_UART7,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART8
	{
		.mapbase	= MCFUART_BASE8,
		.irq		= MCFINT1_VECBASE + MCFINT_UART8,
	},
#endif
#ifdef CONFIG_SERIAL_MCF_UART9
	{
		.mapbase	= MCFUART_BASE9,
		.irq		= MCFINT1_VECBASE + MCFINT_UART9,
	},
#endif
	{}
};

static struct platform_device m5441x_uart = {
	.name			= "mcfuart",
	.id			= 0,
	.dev.platform_data	= m5441x_uart_platform,
};

static void m5441x_uarts_init(void)
{
#ifdef CONFIG_SERIAL_MCF_UART0
	/* enable uart0 clock */
	MCF_PM_PPMCR0 = 24;
	/* gpio pin assignment for uart0 */
	MCF_GPIO_PAR_UART0 =
		(MCF_GPIO_PAR_UART0 & MCF_GPIO_PAR_UART0_U0RXD_MASK) |
		MCF_GPIO_PAR_UART0_U0RXD_U0RXD;
	MCF_GPIO_PAR_UART0 =
		(MCF_GPIO_PAR_UART0 & MCF_GPIO_PAR_UART0_U0TXD_MASK) |
		MCF_GPIO_PAR_UART0_U0TXD_U0TXD;
#endif
#ifdef CONFIG_SERIAL_MCF_UART1
	/* enable uart1 clock */
	MCF_PM_PPMCR0 = 25;
	/* gpio pin assignment for uart1 */
	MCF_GPIO_PAR_UART1 =
		(MCF_GPIO_PAR_UART1 & MCF_GPIO_PAR_UART1_U1RXD_MASK) |
		MCF_GPIO_PAR_UART1_U1RXD_U1RXD;
	MCF_GPIO_PAR_UART1 =
		(MCF_GPIO_PAR_UART1 & MCF_GPIO_PAR_UART1_U1TXD_MASK) |
		MCF_GPIO_PAR_UART1_U1TXD_U1TXD;
#endif
#ifdef CONFIG_SERIAL_MCF_UART2
	/* enable uart2 clock */
	MCF_PM_PPMCR0 = 26;
	/* gpio pin assignment for uart2 */
	MCF_GPIO_PAR_UART2 =
		(MCF_GPIO_PAR_UART2 & MCF_GPIO_PAR_UART2_U2RXD_MASK) |
		MCF_GPIO_PAR_UART2_U2RXD_U2RXD;
	MCF_GPIO_PAR_UART2 =
		(MCF_GPIO_PAR_UART2 & MCF_GPIO_PAR_UART2_U2TXD_MASK) |
		MCF_GPIO_PAR_UART2_U2TXD_U2TXD;
#endif
#ifdef CONFIG_SERIAL_MCF_UART3
	/* enable uart3 clock */
	MCF_PM_PPMCR0 = 27;
	/* gpio pin assignment for uart3 */
	MCF_GPIO_PAR_DSPI0WH =
		(MCF_GPIO_PAR_DSPI0WH & MCF_GPIO_PAR_DSPI0_SIN_MASK) |
		MCF_GPIO_PAR_DSPI0_SIN_U3RXD;
	MCF_GPIO_PAR_DSPI0WH =
		(MCF_GPIO_PAR_DSPI0WH & MCF_GPIO_PAR_DSPI0_SOUT_MASK) |
		MCF_GPIO_PAR_DSPI0_SOUT_U3TXD;
#endif
#ifdef CONFIG_SERIAL_MCF_UART4
	/* enable uart4 clock */
	MCF_PM_PPMCR1 = 24;
	/* gpio pin assignment for uart4 */
	MCF_GPIO_PAR_UART0 =
		(MCF_GPIO_PAR_UART0 & MCF_GPIO_PAR_UART0_U0RTS_MASK) |
		MCF_GPIO_PAR_UART0_U0RTS_U4RXD;
	MCF_GPIO_PAR_UART0 =
		(MCF_GPIO_PAR_UART0 & MCF_GPIO_PAR_UART0_U0CTS_MASK) |
		MCF_GPIO_PAR_UART0_U0CTS_U4TXD;
#endif
#ifdef CONFIG_SERIAL_MCF_UART5
	/* enable uart5 clock */
	MCF_PM_PPMCR1 = 25;
	/* gpio pin assignment for uart5 */
	MCF_GPIO_PAR_UART1 =
		(MCF_GPIO_PAR_UART1 & MCF_GPIO_PAR_UART1_U1RTS_MASK) |
		MCF_GPIO_PAR_UART1_U1RTS_U5RXD;
	MCF_GPIO_PAR_UART1 =
		(MCF_GPIO_PAR_UART1 & MCF_GPIO_PAR_UART1_U1CTS_MASK) |
		MCF_GPIO_PAR_UART1_U1CTS_U5TXD;
#endif
#ifdef CONFIG_SERIAL_MCF_UART6
	/* enable uart6 clock */
	MCF_PM_PPMCR1 = 26;
	/* gpio pin assignment for uart6 */
	MCF_GPIO_PAR_UART2 =
		(MCF_GPIO_PAR_UART2 & MCF_GPIO_PAR_UART2_U2RTS_MASK) |
		MCF_GPIO_PAR_UART2_U2RTS_U6RXD;
	MCF_GPIO_PAR_UART2 =
		(MCF_GPIO_PAR_UART2 & MCF_GPIO_PAR_UART2_U2CTS_MASK) |
		MCF_GPIO_PAR_UART2_U2CTS_U6TXD;
#endif
#ifdef CONFIG_SERIAL_MCF_UART7
	/* enable uart7 clock */
	MCF_PM_PPMCR1 = 27;
	/* gpio pin assignment for uart7 */
	MCF_GPIO_PAR_SSI0H &= (MCF_GPIO_PAR_SSI0H_FS_MASK);
	MCF_GPIO_PAR_SSI0H |= (MCF_GPIO_PAR_SSI0H_FS_U7TXD);
	MCF_GPIO_PAR_SSI0L &= (MCF_GPIO_PAR_SSI0L_BCLK_MASK);
	MCF_GPIO_PAR_SSI0L |= (MCF_GPIO_PAR_SSI0L_BCLK_U7RXD);
#endif
#ifdef CONFIG_SERIAL_MCF_UART8
	/* enable uart8 clock */
	MCF_PM_PPMCR1 = 28;
	/* gpio pin assignment for uart8 */
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_I2C0SCL_MASK) |
		MCF_GPIO_PAR_CANI2C_I2C0SCL_U8TXD;
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_I2C0SDA_MASK) |
		MCF_GPIO_PAR_CANI2C_I2C0SDA_U8RXD;
#endif
#ifdef CONFIG_SERIAL_MCF_UART9
	/* enable uart4 clock */
	MCF_PM_PPMCR1 = 29;
	/* gpio pin assignment for uart9 */
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_CAN1TX_MASK) |
		MCF_GPIO_PAR_CANI2C_CAN1TX_U9TXD;
	MCF_GPIO_PAR_CANI2C =
		(MCF_GPIO_PAR_CANI2C & MCF_GPIO_PAR_CANI2C_CAN1RX_MASK) |
		MCF_GPIO_PAR_CANI2C_CAN1RX_U9RXD;
#endif
}
#endif

#if defined(CONFIG_RTC_M5441X) || defined(CONFIG_RTC_M5441X_MODULE)
/*
 * RTC initialization
 */
static struct platform_device rtc_device = {
	.name = "rtc-m5441x",
	.id   = -1,
};

static void m5441x_rtc_init(void)
{
	/* Power management: enable RTC clock */
	MCF_PM_PPMCR0 = 42;
}
#endif

static struct platform_device *m5441x_devices[] __initdata = {
#if defined(CONFIG_SERIAL_MCF) || defined(CONFIG_SERIAL_MCF_MODULE)
	&m5441x_uart,
#endif
#if defined(CONFIG_RTC_M5441X) || defined(CONFIG_RTC_M5441X_MODULE)
	&rtc_device,
#endif
#if defined(CONFIG_MMC_ESDHC)
	&sd_mmc_esdhc_device,
#endif
#if defined(CONFIG_MTD_NAND_FSL_NFC)
	&nfc_device,
#endif
};

static int __init mcf5441x_init_devices(void)
{
	printk(KERN_INFO "mcf5441x_init_devices: %s.\n", __func__);

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	mcf5441x_init_i2c();
#endif
#if defined(CONFIG_SPI) || defined(CONFIG_SPI_MODULE)
	m5441x_spi_init();
#endif
#if defined(CONFIG_SERIAL_MCF) || defined(CONFIG_SERIAL_MCF_MODULE)
	m5441x_uarts_init();
#endif
#if defined(CONFIG_RTC_M5441X) || defined(CONFIG_RTC_M5441X_MODULE)
	m5441x_rtc_init();
#endif
	platform_add_devices(m5441x_devices, ARRAY_SIZE(m5441x_devices));
	return 0;
}

arch_initcall(mcf5441x_init_devices);


