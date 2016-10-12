/*
 * arch/m68k/coldfire/mcf5445x-devices.c
 *
 * Coldfire M5445x Platform Device Configuration
 *
 * Based on the Freescale MXC devices.c
 *
 * Copyright 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *	Kurt Mahan <kmahan@freescale.com>
 *      Jason Jin Jason.Jin@freescale.com
 *      Shrek Wu B16972@freescale.com
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <linux/spi/spi.h>

#include <linux/interrupt.h>
#include <linux/init.h>
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

/* ATA Interrupt */
#define IRQ_ATA		(64 + 64 + 54)

/* ATA Base */
void __iomem * io_ata_map;
unsigned long io_ata_mapsize = 16 * 1024 * 1024;
#define	BASE_IO_ATA	((unsigned int)(io_ata_map))
#define	BASE_IO_ATA_PHY	0x90000000
#define IO_ATA_MAP_OFFSET	(BASE_IO_ATA-BASE_IO_ATA_PHY)

/* conversion function between virtual address and physical address */
unsigned int io_ata_virt2phys(void* x) { return ((unsigned int)x - IO_ATA_MAP_OFFSET); }
void* io_ata_phys2virt(unsigned int x) { return (void* )(x + IO_ATA_MAP_OFFSET); }

#define ATA_IER		MCF_REG08(BASE_IO_ATA+0x2c)	/* int enable reg */
#define ATA_ICR		MCF_REG08(BASE_IO_ATA+0x30)	/* int clear reg */

/* DSPI */
#define M5445X_DSPI_MCR	0xfc05c000			/* base */

#define M5445X_DSPI_IRQ_SOURCE	(31)
#define M5445X_DSPI_IRQ_VECTOR	(64 + M5445X_DSPI_IRQ_SOURCE)

#define M5445X_AUDIO_IRQ_SOURCE	(49) /* SSI interrupt */
#define M5445X_AUDIO_IRQ_VECTOR	(128+M5445X_AUDIO_IRQ_SOURCE)
#define M5445X_AUDIO_IRQ_LEVEL	(4)

/* number of supported SPI selects */
#define SPI_NUM_CHIPSELECTS	8

#define M5445X_SPI_PAR_VAL	(MCF_GPIO_PAR_DSPI_SCK | 	\
				MCF_GPIO_PAR_DSPI_SOUT |	\
				MCF_GPIO_PAR_DSPI_SIN |	\
				MCF_GPIO_PAR_DSPI_PCS5_PCS5 |  \
				MCF_GPIO_PAR_DSPI_PCS1_PCS1 |  \
				MCF_GPIO_PAR_DSPI_PCS0_PCS0)

#ifdef CONFIG_SPI
/*
 * DSPI
 */

#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
static struct coldfire_dspi_chip spidev_chip_info = {
	.bits_per_word = 8,
};
#endif

#if defined(CONFIG_SND_COLDFIRE) || defined(CONFIG_SND_COLDFIRE_MODULE)
static struct coldfire_dspi_chip codec_spi_chip_info = {
	.mode = SPI_MODE_0,
	.bits_per_word = 16,
	.void_write_data = 0,
	.dbr = 0,
	.pbr = 2,
	.br = 8,
	.pcssck = 1,
	.pasc = 1,
	.pdt = 1,
	.cssck = 0,
	.asc = 1,
	.dt = 1,
};
#endif

#if defined(CONFIG_MMC_SPI)
int mmc_spi_init(struct device *mmc_spi_device,
		irqreturn_t mmc_spi_irq_handler(int irq, void *private),
		void *irq_privatedata)
{
#if 0
	MCF_GPIO_PAR_IRQ = MCF_GPIO_PAR_IRQ | MCF_GPIO_PAR_IRQ_IRQ4_GPIO;
	/* Set trailing edge for PCI interrupts */
	MCF_EPORT_EPPAR &= ~MCF_EPORT_EPPAR_EPPA4(3);
	MCF_EPORT_EPPAR |= MCF_EPORT_EPPAR_EPPA4_LEVEL;
	/* Turn on irq line in eport */
	MCF_EPORT_EPIER = MCF_EPORT_EPIER | MCF_EPORT_EPIER_EPIE4;
#endif
	MCF_EPORT_EPPAR &= ~MCF_EPORT_EPPAR_EPPA7(3);
	MCF_EPORT_EPPAR |= MCF_EPORT_EPPAR_EPPA7_BOTH;
	MCF_EPORT_EPIER = MCF_EPORT_EPIER | MCF_EPORT_EPIER_EPIE7;
#if defined(CONFIG_M54451_SD_HW_DETECT)
	MCF_INTC0_ICR7 = 2;
	if (request_irq(64 + 7, mmc_spi_irq_handler,
			IRQF_DISABLED, "mmc_spi_irq", irq_privatedata)) {
		printk(KERN_INFO "Cannot allocate "
				"eport_irq_lines[%d] IRQ\n", 7);
		return -EBUSY;
	}
#endif
	return 0;
}

static struct coldfire_dspi_chip dspi_sd_chip_info = {
	.mode = SPI_MODE_0,
	.bits_per_word = 8,
	.void_write_data = 0xff,
	.dbr = 0,
	.pbr = 1,
	.br = 1,
	.pcssck = 2,
	.pasc = 2,
	.pdt = 2,
	.cssck = 5,
	.asc = 5,
	.dt = 5,
};

static struct mmc_spi_platform_data mcf54451_mmc_pdata = {
	.ocr_mask = MMC_VDD_33_34,
	.init = mmc_spi_init,
};
#endif

static struct spi_board_info spi_board_info[] = {
#if defined(CONFIG_SND_COLDFIRE) || defined(CONFIG_SND_COLDFIRE_MODULE)
	{
		.modalias = "mcf_codec_spi",
		.max_speed_hz = 300000,
		.bus_num = 1,
		.chip_select = 5,
		.irq = M5445X_AUDIO_IRQ_VECTOR,
		.platform_data = NULL,
		.controller_data = &codec_spi_chip_info
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

#if defined(CONFIG_MMC_SPI)
	{
		.modalias = "mmc_spi",
		.max_speed_hz = 25000000,
		.bus_num = 1,
		.chip_select = 0,
		.platform_data = &mcf54451_mmc_pdata,
		.controller_data = &dspi_sd_chip_info,
	},
#endif
};


static struct coldfire_spi_master coldfire_master_info = {
	.bus_num = 1,
	.num_chipselect = SPI_NUM_CHIPSELECTS,
	.irq_list = 0,		/* not used */
	.irq_source = M5445X_DSPI_IRQ_SOURCE,
	.irq_vector = M5445X_DSPI_IRQ_VECTOR,
	.irq_mask = (1 << M5445X_DSPI_IRQ_SOURCE),
	.irq_lp = 0x2,		/* irq level */
	.par_val = M5445X_SPI_PAR_VAL,
	.cs_control = NULL,
};

static struct resource coldfire_spi_resources[] = {
	[0] = {
		.name = "spi-par",
		.start = &MCF_GPIO_PAR_DSPI,	/* PAR_DSPI */
		.end = &MCF_GPIO_PAR_DSPI,	/* PAR_DSPI */
		.flags = IORESOURCE_MEM
	},

	[1] = {
		.name = "spi-module",
		.start = M5445X_DSPI_MCR,	/* DSPI MCR Base */
		.end = M5445X_DSPI_MCR + 0xc0,	/* DSPI mem map end */
		.flags = IORESOURCE_MEM
	},

	[2] = {
		.name = "spi-int-level",
		.start = &MCF_INTC0_ICR31,	/* ICR start */
		.end = &MCF_INTC0_ICR31, 	/* ICR end */
		.flags = IORESOURCE_MEM
	},

	[3] = {
		.name = "spi-int-mask",
		.start = &MCF_INTC0_IMRL,	/* IMRL */
		.end = &MCF_INTC0_IMRL,		/* IMRL */
		.flags = IORESOURCE_MEM
	}
};

static struct platform_device coldfire_spi = {
	.name = "spi_coldfire",
	.id = -1,
	.resource = coldfire_spi_resources,
	.num_resources = ARRAY_SIZE(coldfire_spi_resources),
	.dev = {
		.platform_data = &coldfire_master_info,
	}
};

/**
 * m5445x_spi_init - Initialize SPI
 */
static int __init m5445x_spi_init(void)
{
	int retval;

	/* register device */
	retval = platform_device_register(&coldfire_spi);
	if (retval < 0)
		goto out;

	/* register board info */
	if (ARRAY_SIZE(spi_board_info))
		retval = spi_register_board_info(spi_board_info,
						 ARRAY_SIZE(spi_board_info));

out:
	return retval;
}
#endif

/*
 * On-chip PATA
 */
#if defined(CONFIG_PATA_FSL) || defined(CONFIG_PATA_FSL_MODULE)
static int ata_init(struct platform_device *pdev)
{
	/* clear ints */
	ATA_IER = 0x00;
	ATA_ICR = 0xff;

	/* setup shared pins */
	MCF_GPIO_PAR_FEC = (MCF_GPIO_PAR_FEC & MCF_GPIO_PAR_FEC_FEC1_MASK) |
			   MCF_GPIO_PAR_FEC_FEC1_ATA;

	MCF_GPIO_PAR_FECI2C = (MCF_GPIO_PAR_FECI2C &
			(MCF_GPIO_PAR_FECI2C_MDC1_MASK &
			 MCF_GPIO_PAR_FECI2C_MDIO1_MASK)) |
			MCF_GPIO_PAR_FECI2C_MDC1_ATA_DIOR |
			MCF_GPIO_PAR_FECI2C_MDIO1_ATA_DIOW;

	MCF_GPIO_PAR_ATA = MCF_GPIO_PAR_ATA_BUFEN |
			MCF_GPIO_PAR_ATA_CS1 |
			MCF_GPIO_PAR_ATA_CS0 |
			MCF_GPIO_PAR_ATA_DA2 |
			MCF_GPIO_PAR_ATA_DA1 |
			MCF_GPIO_PAR_ATA_DA0 |
			MCF_GPIO_PAR_ATA_RESET_RESET |
			MCF_GPIO_PAR_ATA_DMARQ_DMARQ |
			MCF_GPIO_PAR_ATA_IORDY_IORDY;

	MCF_GPIO_PAR_PCI = (MCF_GPIO_PAR_PCI &
			(MCF_GPIO_PAR_PCI_GNT3_MASK &
			 MCF_GPIO_PAR_PCI_REQ3_MASK)) |
			MCF_GPIO_PAR_PCI_GNT3_ATA_DMACK |
			MCF_GPIO_PAR_PCI_REQ3_ATA_INTRQ;

	return 0;
}

static void ata_exit(void)
{
	printk(KERN_INFO "** ata_exit\n");
}

static int ata_get_clk_rate(void)
{
	return MCF_BUSCLK;
}


static struct fsl_ata_platform_data ata_data = {
	.init             = ata_init,
	.exit             = ata_exit,
	.get_clk_rate     = ata_get_clk_rate,
};

static struct resource pata_fsl_resources[] = {
	[0] = {		/* I/O */
		.flags		= IORESOURCE_MEM,
	},
	[2] = {		/* IRQ */
		.start		= IRQ_ATA,
		.end		= IRQ_ATA,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device pata_fsl_device = {
	.name			= "pata_fsl",
	.id			= -1,
	.num_resources		= ARRAY_SIZE(pata_fsl_resources),
	.resource		= pata_fsl_resources,
	.dev			= {
		.platform_data	= &ata_data,
		.coherent_dma_mask = ~0,	/* $$$ REVISIT */
	},
};

static inline void mcf5445x_init_pata(void)
{
	io_ata_map = ioremap(BASE_IO_ATA_PHY, io_ata_mapsize);
	if (!io_ata_map) {
		printk(KERN_ERR "IO ATA map failed by ioremap!\n");
		return;
	}
	printk(KERN_INFO "BASE_IO_ATA value is 0x%x\n", BASE_IO_ATA);

	pata_fsl_resources[0].start = BASE_IO_ATA;
	pata_fsl_resources[0].end = BASE_IO_ATA + 0x000000d8;

	(void)platform_device_register(&pata_fsl_device);
}
#endif

/*
 * I2C
 */
#if defined(CONFIG_I2C)
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

static void mcf5445x_init_i2c(void)
{
	platform_device_register(&coldfire_i2c_device);
}
#endif

static int __init mcf5445x_init_devices(void)
{
	printk(KERN_INFO "mcf5445x_init_devices: %s.\n", __func__);
#ifdef CONFIG_SPI
	m5445x_spi_init();
#endif

#if defined(CONFIG_PATA_FSL) || defined(CONFIG_PATA_FSL_MODULE)
	mcf5445x_init_pata();
#endif

#if defined(CONFIG_I2C)
	mcf5445x_init_i2c();
#endif
	return 0;
}
arch_initcall(mcf5445x_init_devices);

/***************************************************************************/
static struct mcf_platform_uart m5445x_uart_platform[] = {
	{
		.mapbase	= MCFUART_BASE1,
		.irq		= MCFINT_VECBASE + MCFINT_UART0,
	},
	{ },
};

static struct platform_device m5445x_uart = {
	.name			= "mcfuart",
	.id			= 0,
	.dev.platform_data	= m5445x_uart_platform,
};

#if defined(CONFIG_RTC_MCF) || defined(CONFIG_RTC_MCF_MODULE)
static struct platform_device rtc_device = {
	.name = "mcf-rtc",
	.id   = -1,
};
#endif

static struct platform_device *m5445x_devices[] __initdata = {
	&m5445x_uart,
#if defined(CONFIG_RTC_MCF) || defined(CONFIG_RTC_MCF_MODULE)
	&rtc_device,
#endif
};


static void m5445x_uart_init_line(int line, int irq)
{
	if (line == 0) {
		MCF_GPIO_PAR_UART = (0 | 0x000F);
	} else if (line == 1) {
		MCF_GPIO_PAR_UART |= (0 | 0x0FF0);
	} 
}

void m5445x_uarts_init(void)
{
	const int nrlines = ARRAY_SIZE(m5445x_uart_platform);
	int line;

	for (line = 0; (line < nrlines); line++)
		m5445x_uart_init_line(line, m5445x_uart_platform[line].irq);
}
/****************************************************************************/
/***************************************************************************/

static int __init init_BSP(void)
{
	m5445x_uarts_init();
	platform_add_devices(m5445x_devices, ARRAY_SIZE(m5445x_devices));
	return 0;
}

arch_initcall(init_BSP);


