/*
 * linux/sound/coldfire/coldfire-codec-spi.c
 *
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Author: Kurt Mahan <kmahan@freescale.com>
 *
 * Simple SPI interface for the CODEC.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include <asm/mcfsim.h>
#include <asm/coldfire.h>
#include <linux/delay.h>

#define MCF_CODEC_SPI_DRIVER_NAME	"mcf_codec_spi"

static struct spi_device *mcf_codec_spi;

/*
 * Write CODEC register via SPI
 */
int mcf_codec_spi_write(u8 addr, u16 data)
{
	u16 spi_word;

	if (mcf_codec_spi == NULL)
		return -ENODEV;

	spi_word = ((addr & 0x7F)<<9)|(data & 0x1FF);
	return spi_write(mcf_codec_spi, (const u8 *)&spi_word,
			 sizeof(spi_word));
}
EXPORT_SYMBOL(mcf_codec_spi_write);

static int __devinit mcf_codec_spi_probe(struct spi_device *spi)
{
	spi->dev.power.power_state = PMSG_ON;
	mcf_codec_spi = spi;

	return 0;
}

static int __devexit mcf_codec_spi_remove(struct spi_device *spi)
{
	return 0;
}

static int mcf_codec_spi_suspend(struct spi_device *spi, pm_message_t message)
{
	return 0;
}

static int mcf_codec_spi_resume(struct spi_device *spi)
{
	return 0;
}

static struct spi_driver mcf_codec_spi_driver = {
	.driver = {
		.name	= MCF_CODEC_SPI_DRIVER_NAME,
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe		= mcf_codec_spi_probe,
	.remove		= __devexit_p(mcf_codec_spi_remove),
	.suspend	= mcf_codec_spi_suspend,
	.resume		= mcf_codec_spi_resume,
};

static int __init mcf_codec_spi_init(void)
{
	return spi_register_driver(&mcf_codec_spi_driver);
}
module_init(mcf_codec_spi_init);

static void __exit mcf_codec_spi_exit(void)
{
	spi_unregister_driver(&mcf_codec_spi_driver);
}
module_exit(mcf_codec_spi_exit);


MODULE_DESCRIPTION("Coldfire Codec SPI driver");
MODULE_AUTHOR("Kurt Mahan, Freescale Semiconductor, Inc.");
MODULE_LICENSE("GPL");
